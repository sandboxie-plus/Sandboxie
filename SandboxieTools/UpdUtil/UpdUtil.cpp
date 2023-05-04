// UpdUtil.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include <shellapi.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include "common/helpers.h"
#include "common/WebUtils.h"
#include "common/json/JSON.h"
#include "UpdUtil.h"


extern "C" {
	NTSTATUS CreateKeyPair(_In_ PCWSTR PrivFile, _In_ PCWSTR PubFile);

	NTSTATUS SignHash(_In_ PVOID Hash, _In_ ULONG HashSize, _In_ PVOID PrivKey, _In_ ULONG PrivKeySize, _Out_ PVOID* Signature, _Out_ ULONG* SignatureSize);
	NTSTATUS VerifyHashSignature(PVOID Hash, ULONG HashSize, PVOID Signature, ULONG SignatureSize);
	NTSTATUS VerifyFileSignature(const wchar_t* FilePath);

	NTSTATUS MyHashBuffer(_In_ PVOID pData, _In_ SIZE_T uSize, _Out_ PVOID* Hash, _Out_ PULONG HashSize);
	NTSTATUS MyHashFile(_In_ PCWSTR FileName, _Out_ PVOID* Hash, _Out_ PULONG HashSize);

	NTSTATUS MyReadFile(_In_ PWSTR FileName, _In_ ULONG FileSizeLimit, _Out_ PVOID* Buffer, _Out_ PULONG FileSize);
	NTSTATUS MyWriteFile(_In_ PWSTR FileName, _In_ PVOID Buffer, _In_ ULONG BufferSize);

    size_t b64_encoded_size(size_t inlen);
    wchar_t* b64_encode(const unsigned char* in, size_t inlen);
    size_t b64_decoded_size(const wchar_t* in);
    int b64_decode(const wchar_t* in, unsigned char* out, size_t outlen);
}

struct SFile
{
	SFile() : State(eNone) {}

	std::wstring	Path;
	std::wstring	Url;
	std::wstring	Hash;
	enum EState
	{
		eNone = 0,
		eChanged,
		ePending	// downloaded
	}				State;
};

struct SFileMap
{
	//SFileMap() : Status(eLoaded) {}

	std::map<std::wstring, std::shared_ptr<SFile>> Map;
	std::wstring Sign;
	std::wstring Version;
	int iUpdate;
	std::wstring CI;
	//enum EStatus
	//{
	//	eLoaded = 0,
	//	eScanned,
	//	ePrepared
	//}				Status;
};

typedef std::list<std::wstring> TScope;


std::shared_ptr<SFileMap> ScanDir(std::wstring Path) 
{
	if (Path.back() != L'\\') 
		Path.push_back(L'\\');

	std::shared_ptr<SFileMap> pFiles = std::make_shared<SFileMap>();

	std::vector<std::wstring> Entries;
	Entries.push_back(Path);

	for (int i = 0; i < Entries.size(); i++)
	{
		if (Entries[i].back() == '\\') {
			ListDir(Entries[i], Entries);
			continue;
		}

		ULONG hashSize;
		PVOID hash = NULL;
		if (NT_SUCCESS(MyHashFile(Entries[i].c_str(), &hash, &hashSize)))
		{
			std::shared_ptr<SFile> pFile = std::make_shared<SFile>();
			pFile->Path = Entries[i].substr(Path.length());
			pFile->Hash = hexStr((unsigned char*)hash, hashSize);
			pFiles->Map[pFile->Path] = pFile;

			free(hash);
		}
	}

	return pFiles;
}

std::wstring GetJSONStringSafe(const JSONObject& root, const std::wstring& key, const std::wstring& def = L"")
{
	auto I = root.find(key);
	if (I == root.end() || !I->second->IsString())
		return def;
	return I->second->AsString();
}

int GetJSONIntSafe(const JSONObject& root, const std::wstring& key, int def = 0)
{
	auto I = root.find(key);
	if (I == root.end() || !I->second->IsNumber())
		return def;
	return (int)I->second->AsNumber();
}

JSONObject GetJSONObjectSafe(const JSONObject& root, const std::wstring& key)
{
	auto I = root.find(key);
	if (I == root.end() || !I->second->IsObject())
		return JSONObject();
	return I->second->AsObject();
}

JSONArray GetJSONArraySafe(const JSONObject& root, const std::wstring& key)
{
	auto I = root.find(key);
	if (I == root.end() || !I->second->IsArray())
		return JSONArray();
	return I->second->AsArray();
}

std::string WriteUpdate(std::shared_ptr<SFileMap> pFiles)
{
	JSONObject root;

	JSONArray files;
	for (auto I = pFiles->Map.begin(); I != pFiles->Map.end(); ++I) 
	{
		JSONObject file;
		file[L"path"] = new JSONValue(I->second->Path);
		file[L"hash"] = new JSONValue(I->second->Hash);
		file[L"url"] = new JSONValue(I->second->Url);
		//if(I->second->State == SFile::eChanged)
		//	file[L"state"] = new JSONValue(L"changed");
		//else if(I->second->State == SFile::ePending)
		//	file[L"state"] = new JSONValue(L"pending");
		files.push_back(new JSONValue(file));
	}
	root[L"files"] = new JSONValue(files);

	root[L"signature"] = new JSONValue(pFiles->Sign);

	root[L"version"] = new JSONValue(pFiles->Version);
	root[L"update"] = new JSONValue(pFiles->iUpdate);

	root[L"ci"] = new JSONValue(pFiles->CI);
			
	//if(pFiles->Status == SFileMap::eScanned)
	//	root[L"status"] = new JSONValue(L"scanned");
	//else if(pFiles->Status == SFileMap::ePrepared)
	//	root[L"status"] = new JSONValue(L"prepared");

	JSONValue *value = new JSONValue(root);
	auto wJson = value->Stringify();
	delete value;

	return g_str_conv.to_bytes(wJson);
}

std::shared_ptr<SFileMap> ReadUpdate(const JSONObject& jsonObject)
{
	if (jsonObject.find(L"files") == jsonObject.end())
		return std::shared_ptr<SFileMap>();

	std::shared_ptr<SFileMap> pFiles = std::make_shared<SFileMap>();

	JSONArray jsonFiles = GetJSONArraySafe(jsonObject, L"files");
	for (auto I = jsonFiles.begin(); I != jsonFiles.end(); ++I) {
		if ((*I)->IsObject()) {
			JSONObject jsonFile = (*I)->AsObject();

			std::shared_ptr<SFile> pFile = std::make_shared<SFile>();
			pFile->Path = GetJSONStringSafe(jsonFile, L"path");
			pFile->Hash = GetJSONStringSafe(jsonFile, L"hash");
			pFile->Url = GetJSONStringSafe(jsonFile, L"url");
			//std::wstring state = GetJSONStringSafe(jsonObject, L"state");
			//if(state == L"changed")
			//	pFile->State = SFile::eChanged;
			//else if(state == L"pending")
			//	pFile->State = SFile::ePending;
			pFiles->Map[pFile->Path] = pFile;
		}
	}

	pFiles->Sign = GetJSONStringSafe(jsonObject, L"signature");

	pFiles->Version = GetJSONStringSafe(jsonObject, L"version");
	pFiles->iUpdate = GetJSONIntSafe(jsonObject, L"update", -1);

	pFiles->CI = GetJSONStringSafe(jsonObject, L"ci");

	std::wstring status = GetJSONStringSafe(jsonObject, L"status");
	//if(status == L"scanned")
	//	pFiles->Status = SFileMap::eScanned;
	//else if(status == L"prepared")
	//	pFiles->Status = SFileMap::ePrepared;

	return pFiles;
}

bool VerifyUpdate(std::shared_ptr<SFileMap> pFiles)
{
	bool pass = false;

	std::set<std::wstring> hash_set;
	for (auto I = pFiles->Map.begin(); I != pFiles->Map.end(); ++I)
		hash_set.insert(I->second->Path + L":" + I->second->Hash);

	std::string hashes;
	for (auto I = hash_set.begin(); I != hash_set.end(); ++I)
		hashes += g_str_conv.to_bytes(*I) + "\n";

	ULONG hashSize;
	PVOID hash = NULL;
	if (NT_SUCCESS(MyHashBuffer((void*)hashes.c_str(), hashes.length(), &hash, &hashSize)))
	{
		ULONG signatureSize = b64_decoded_size(pFiles->Sign.c_str());
		if (signatureSize)
		{
			PUCHAR signature = (PUCHAR)malloc(signatureSize);
			b64_decode(pFiles->Sign.c_str(), signature, signatureSize);

			if (NT_SUCCESS(VerifyHashSignature((PUCHAR)hash, hashSize, (PUCHAR)signature, signatureSize)))
			{
				pass = true;
			}

			free(signature);
		}

		free(hash);
	}

	return pass;
}

std::pair<std::wstring, std::wstring> SplitName(std::wstring path)
{
	std::wstring name;
	size_t pos = path.find_last_of(L"\\");
	if (pos == -1) {
		name = path;
		path.clear();
	}
	else {
		name = path.substr(pos + 1);
		path.erase(pos);
		path = L"\\" + path;
	}
	return std::make_pair(path, name);
}

void AddFilesToScope(std::shared_ptr<TScope> pScope, const WCHAR* pFiles)
{
	for (const WCHAR* pFile = pFiles; *pFile; pFile += wcslen(pFile) + 1)
		pScope->push_back(pFile);
}

// full|core|meta|lang|tmpl
std::shared_ptr<TScope> GetScope(std::wstring scope)
{
	std::shared_ptr<TScope> pScope = std::make_shared<TScope>();

	//if (scope == L"full") // everything
	
	if (scope == L"core" || scope == L"full") // core
		AddFilesToScope(pScope, SCOPE_CORE_FILES);
	
	if (scope == L"lang" || scope == L"meta" || scope == L"core" || scope == L"full") 
		AddFilesToScope(pScope, SCOPE_LANG_FILES);
	
	if (scope == L"tmpl" || scope == L"meta" || scope == L"core" || scope == L"full")
		AddFilesToScope(pScope, SCOPE_TMPL_FILES);

	// when there are no entries it means everything
	if (pScope->empty()) pScope.reset(); 
	return pScope;
}

bool InScope(std::shared_ptr<TScope> pScope, std::wstring name) 
{
	if (!pScope) return true;

	auto len = name.length();
	if (len > 4 && name.substr(len - 4) == L".sig")
		name = name.substr(0, len - 4);

	for (auto I = pScope->begin(); I != pScope->end(); ++I) {
		if (wildstrcmp(I->c_str(), name.c_str()) != NULL)
			return true;
	}

	return false;
}

int FindChanges(std::shared_ptr<SFileMap> pNewFiles, std::wstring base_dir, std::wstring temp_dir, std::shared_ptr<TScope> pScope)
{
	std::shared_ptr<SFileMap> pOldFiles = ScanDir(base_dir);
	if (!pOldFiles)
		return ERROR_SCAN;

	int Count = 0;
	for (auto I = pNewFiles->Map.begin(); I != pNewFiles->Map.end(); ++I)
	{
		I->second->State = SFile::eNone;
		if (!InScope(pScope, I->second->Path))
			continue;

		auto J = pOldFiles->Map.find(I->first);
		if (J == pOldFiles->Map.end() || J->second->Hash != I->second->Hash) {
			I->second->State = SFile::eChanged;
			Count++;
		}
	}

	return Count;
}


BOOLEAN WebDownload(std::wstring url, PSTR* pData, ULONG* pDataLength)
{
	size_t pos = url.find_first_of(L'/', 8);
	if (pos == std::wstring::npos)
		return FALSE;
	std::wstring path = url.substr(pos);
	std::wstring domain = url.substr(8, pos-8);

	return WebDownload(domain.c_str(), path.c_str(), pData, pDataLength);
}

int DownloadUpdate(std::wstring temp_dir, std::shared_ptr<SFileMap> pNewFiles)
{
	std::wcout << L"Downloading" << std::endl;

	int Count = 0;
	for (auto I = pNewFiles->Map.begin(); I != pNewFiles->Map.end(); ++I)
	{
		if (I->second->State != SFile::eChanged)
			continue;
		Count++;

		auto path_name = SplitName(I->second->Path);

		if (!path_name.first.empty())
			CreateDirectoryW((temp_dir + path_name.first).c_str(), NULL);

		ULONG hashSize;
		PVOID hash = NULL;
		if (NT_SUCCESS(MyHashFile((wchar_t*)(temp_dir + L"\\" + I->second->Path).c_str(), &hash, &hashSize)))
		{
			std::wstring Hash = hexStr((unsigned char*)hash, hashSize);
			free(hash);

			if (I->second->Hash == Hash) {
				I->second->State = SFile::ePending;
				continue; // already downloaded and up to date
			}
		}

		std::wcout << L"\tDownloading: " << I->second->Path << L" ...";

		char* pData = NULL;
		ULONG uDataLen = 0;
		if (WebDownload(I->second->Url, &pData, &uDataLen)) {

			ULONG hashSize;
			PVOID hash = NULL;
			if(NT_SUCCESS(MyHashBuffer(pData, uDataLen, &hash, &hashSize)))
			{
				std::wstring Hash = hexStr((unsigned char*)hash, hashSize);
				free(hash);

				if (I->second->Hash != Hash) 
				{
					free(pData);
					std::wcout << L" BAD!!!" << std::endl;
					return ERROR_HASH;
				}

				MyWriteFile((wchar_t*)(temp_dir + L"\\" + I->second->Path).c_str(), pData, uDataLen);
				I->second->State = SFile::ePending;
			}

			free(pData);
			std::wcout << L" done" << std::endl;
		}
		else {

			std::wcout << L" FAILED" << std::endl;

			return ERROR_DOWNLOAD;
		}
	}

	return Count;
}

int LoadUpdate(std::wstring temp_dir, std::shared_ptr<SFileMap>& pNewFiles)
{	
	char* aJson = NULL;
	std::wstring file_path = temp_dir + L"\\" _T(UPDATE_FILE);
	if (NT_SUCCESS(MyReadFile((wchar_t*)file_path.c_str(), 1024 * 1024, (PVOID*)&aJson, NULL)) && aJson != NULL) 
	{	
		JSONValue* jsonObject = JSON::Parse(aJson);
		if (jsonObject) {
			if (jsonObject->IsObject())
				pNewFiles = ReadUpdate(jsonObject->AsObject());
			delete jsonObject;
		}
		free(aJson);
	}
	
	if(!pNewFiles){
		std::wcout << L"No pending update found!" << std::endl;
		return ERROR_LOAD;
	}

	if (!VerifyUpdate(pNewFiles)) {
		std::wcout << L"INVALID " _T(UPDATE_FILE) L" SIGNATURE !!!" << std::endl;
		return ERROR_SIGN;
	}

	return 0;
}

int ApplyUpdate(std::wstring base_dir, std::wstring temp_dir, std::shared_ptr<SFileMap> pNewFiles)
{
	std::wcout << L"Applying updates" << std::endl;

	int Count = 0;
	for (auto I = pNewFiles->Map.begin(); I != pNewFiles->Map.end(); ++I)
	{
		if (I->second->State != SFile::ePending)
			continue;
		if(_wcsicmp(I->second->Path.c_str(), L"UpdUtil.exe") == 0)
			continue; // don't try overwriting ourselves
		Count++;

		auto path_name = SplitName(I->second->Path);

		std::wcout << L"\tInstalling: " << I->second->Path << L" ...";

		std::wstring src = temp_dir + L"\\" + I->second->Path;
		std::wstring dest = base_dir + L"\\" + I->second->Path;

		if (!path_name.first.empty())
			CreateDirectoryW((base_dir + path_name.first).c_str(), NULL);

		if(MoveFileExW(src.c_str(), dest.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
			std::wcout << L" done" << std::endl;
		else
			std::wcout << L" FAILED" << std::endl;
	}

	//std::wstring src = temp_dir + L"\\" _T(UPDATE_FILE);
	//std::wstring dest = base_dir + L"\\" _T(UPDATE_FILE);
	//MoveFileExW(src.c_str(), dest.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);

	return Count;
}

DWORD Execute(std::wstring wFile, std::wstring wParams)
{
	SHELLEXECUTEINFO si = { sizeof(SHELLEXECUTEINFO) };
	DWORD ret = 1;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpVerb = L"runas";
	si.lpFile = wFile.c_str();
	si.lpParameters = wParams.c_str();
	si.nShow = SW_HIDE;
		
	std::wcout << L"KmdUtil.exe " << si.lpParameters << std::endl;
	if (ShellExecuteEx(&si)) {
		WaitForSingleObject(si.hProcess, INFINITE);
		GetExitCodeProcess(si.hProcess, &ret);
		CloseHandle(si.hProcess);
	}

	return ret;
}

int ProcessUpdate(std::shared_ptr<SFileMap>& pFiles, const std::wstring& step, const std::wstring& temp_dir, const std::wstring& base_dir, const std::wstring& scope)
{
	int ret = 0;

	if (!pFiles || pFiles->Map.empty())
		return ERROR_INTERNAL;

	//if (step.empty() || step == L"scan" || pFiles->Status < SFileMap::eScanned)
	if (step.empty() || step == L"scan" || step == L"prepare" || step == L"apply")
	{
		std::shared_ptr<TScope> pScope;
		if (!scope.empty()) pScope = GetScope(scope);

		ret = FindChanges(pFiles, base_dir, temp_dir, pScope);
		//pFiles->Status = SFileMap::eScanned;
		if (ret <= 0)
			return ret; // error or nothing todo
		if (step == L"scan")
			return ret;
	}

	//if (step.empty() || step == L"prepare" || pFiles->Status < SFileMap::ePrepared)
	if (step.empty() || step == L"prepare" || step == L"apply")
	{
		ret = DownloadUpdate(temp_dir, pFiles);
		//pFiles->Status = SFileMap::ePrepared;
		if (ret <= 0)
			return ret; // error or nothing todo
		if (step == L"prepare")
			return ret;
	}

	return ret;
}

int DownloadFile(std::wstring url, std::wstring file_path)
{
	char* pData = NULL;
	ULONG uDataLen = 0;

	if (WebDownload(url, &pData, &uDataLen)) {

		MyWriteFile((wchar_t*)file_path.c_str(), pData, uDataLen);

		free(pData);
		std::wcout << L" done" << std::endl;
		return 0;
	}
	std::wcout << L" FAILED" << std::endl;
	return ERROR_DOWNLOAD;
}

int PrintFile(std::wstring url)
{
	char* pData = NULL;
	ULONG uDataLen = 0;

	if (WebDownload(url, &pData, &uDataLen)) {

		std::wcout << pData;
		free(pData);
		return 0;
	}
	return ERROR_DOWNLOAD;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void PrintUsage() 
{
	std::wcout << L"Sandboxie Update Utility - Usage" << std::endl;
	std::wcout << L"================================" << std::endl;
	std::wcout << std::endl;
	std::wcout << L"Update to the latest version in a given release channel" << std::endl;
	std::wcout << L"\tUpdUtil.exe [update|upgrade] software (/channel:[stable|preview|live]) {Options}" << std::endl;
	std::wcout << std::endl;
	std::wcout << L"Update to the latest update of a specific version" << std::endl;
	std::wcout << L"\tUpdUtil.exe [update|upgrade] software (/version:[a.bb.c]) (/update:[d]) {Options}" << std::endl;
	std::wcout << std::endl;
	std::wcout << L"Options:" << std::endl;
	//std::wcout << L"\t/arch:[ARM64|a64|x86_64|x64|i386|x86]" << std::endl;
	std::wcout << L"\t/scope:[full|core|meta|lang|tmpl]" << std::endl;
	std::wcout << L"\t\tfull - update all components" << std::endl;
	std::wcout << L"\t\tcore - core components (for classic, same as full)" << std::endl;
	std::wcout << L"\t\tmeta - update metadata (lang and tmpl)" << std::endl;
	std::wcout << L"\t\tlang - update language files" << std::endl;
	std::wcout << L"\t\ttmpl - update Templates.ini" << std::endl;
	std::wcout << L"\t/step:[get|scan|prepare|apply]" << std::endl;
	std::wcout << L"\t\tget - download updated information to " _T(UPDATE_FILE) << std::endl;
	std::wcout << L"\t\tscan - check for updates, use existing " _T(UPDATE_FILE) " if present" << std::endl;
	std::wcout << L"\t\tprepare - download updates, but don't install" << std::endl;
	std::wcout << L"\t\tapply - install updates" << std::endl;
	std::wcout << L"" << std::endl;
}

bool HasFlag(const std::vector<std::wstring>& arguments, std::wstring name)
{
	return std::find(arguments.begin(), arguments.end(), L"/" + name) != arguments.end();
}

std::wstring GetArgument(const std::vector<std::wstring>& arguments, std::wstring name) 
{
	std::wstring prefix = L"/" + name + L":";
	for (int i = 0; i < arguments.size(); i++) {
		if (_wcsicmp(arguments[i].substr(0, prefix.length()).c_str(), prefix.c_str()) == 0) {
			return arguments[i].substr(prefix.length());
		}
	}
	return L"";
}

bool FileExists(const wchar_t* path)
{
	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND)
		return false;
	return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	InitOsVersionInfo();

	wchar_t szPath[MAX_PATH];
	GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));
	*wcsrchr(szPath, L'\\') = L'\0';
	std::wstring wPath = szPath;

	int nArgs = 0;
	LPWSTR* szArglist = CommandLineToArgvW(lpCmdLine, &nArgs); // GetCommandLineW()
	std::vector<std::wstring> arguments;
	for (int i = 0; i < nArgs; i++)
		arguments.push_back(szArglist[i]);
	LocalFree(szArglist);

	if (!HasFlag(arguments, L"embedded")) {
		if (AttachConsole(ATTACH_PARENT_PROCESS) == FALSE)
			AllocConsole();
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		if (HasFlag(arguments, L"pause")) {
			std::cout << "Sandboxie Update Utility" << std::endl;
			std::wcout << lpCmdLine << std::endl;
			std::cout << std::endl << "Press enter to continue..." << std::endl;
			std::cin.get();
		}
	}

	std::wstring temp_dir = GetArgument(arguments, L"temp");
	if (temp_dir.empty()) {
		wchar_t szTemp[MAX_PATH];
		GetTempPath(MAX_PATH, (LPWSTR)&szTemp);
		temp_dir = std::wstring(szTemp) + L"sandboxie-updater";
	}
	else if (temp_dir.back() == L'\\')
		temp_dir.pop_back();

	std::wstring base_dir = GetArgument(arguments, L"path");
	if (base_dir.empty())
		base_dir = wPath;

	std::wstring arch = GetArgument(arguments, L"arch");
	if (!arch.empty()) {
		// normalize architecture
		if (arch == L"x64")
			arch = L"x86_64";
		else if (arch == L"x86")
			arch = L"i386";
		else if (arch == L"ARM64" || arch == L"A64" || arch == L"a64")
			arch = L"arm64";
	}
	else
#ifdef _M_ARM64
		arch = L"ARM64";
#elif _WIN64
		arch = L"x86_64";
#else
		arch = L"i386";
#endif

	if (arguments.size() >= 2 && arguments[0] == L"download") // download file to disk
	{
		std::wstring url = arguments[1];
		std::wstring name = GetArgument(arguments, L"name");
		if (name.empty()) {
			size_t end = url.find_last_of(L'?');
			size_t pos = url.find_last_of(L'/', end);
			if (pos == std::wstring::npos)
				return ERROR_INVALID;
			name = url.substr(++pos, end - pos);
		}
		return DownloadFile(url, temp_dir + L"\\" + name);
	}
	else if (arguments.size() >= 2 && arguments[0] == L"print") // download file and print to stdout
	{
		std::wstring url = arguments[1];
		return PrintFile(url);
	}
	else if (arguments.size() >= 2 && arguments[0] == L"run_setup") // run a signed setup file
	{
		std::wstring wFile = arguments[1];

		if(!NT_SUCCESS(VerifyFileSignature(wFile.c_str())))
			return ERROR_SIGN;

		std::wstring wParams;
		wParams = L"/open_agent";
		if (HasFlag(arguments, L"embedded"))
			wParams = L" /SILENT";

		SHELLEXECUTEINFO si = { sizeof(SHELLEXECUTEINFO) };
		//si.lpVerb = L"runas";
		si.lpFile = wFile.c_str();
		si.lpParameters = wParams.c_str();
		si.nShow = SW_SHOW;

		if (!ShellExecuteEx(&si)) {
			//DWORD dwError = GetLastError();
			//if (dwError == ERROR_CANCELLED)
			//	return ERROR_CANCELED;
			return ERROR_EXEC;
		}
		return 0;
	}
	else if (arguments.size() >= 2 && (arguments[0] == L"update" || arguments[0] == L"upgrade" || arguments[0] == L"install"))
	{
		std::wstring software = arguments[1];
	
		int ret;

		std::shared_ptr<SFileMap> pFiles;

		std::wstring step = GetArgument(arguments, L"step");

		if (!step.empty() && step != L"get")
		{
			ret = LoadUpdate(temp_dir, pFiles);
			if (ret < 0)
				return ret;
		}

		bool bSave = false;
		if(!pFiles || pFiles->Map.empty())
		{
			std::wstringstream params;
			params << L"&action=" << arguments[0];

			std::wstring channel = GetArgument(arguments, L"channel");
			if (channel.empty()) channel = GetArgument(arguments, L"release");
			if (!channel.empty())
				params << L"&channel=" << channel;

			std::wstring version = GetArgument(arguments, L"version");
			if (!version.empty()) {
				params << L"&version=" << version;
				std::wstring update = GetArgument(arguments, L"update");
				if (!update.empty())
					params << L"&update=" << update;
			}

			params << L"&system=windows-" << g_osvi.dwMajorVersion << L"." << g_osvi.dwMinorVersion << L"." << g_osvi.dwBuildNumber << "-" << arch;

			wchar_t StrLang[16];
			LCIDToLocaleName(GetUserDefaultLCID(), StrLang, ARRAYSIZE(StrLang), 0);
			if (StrLang[2] == L'-') StrLang[2] = '_';

			params << L"&language=" << StrLang;

			std::wstring update_key = GetArgument(arguments, L"update_key");
			if (!update_key.empty()) params << L"&update_key=" + update_key;

			// version or channel must be specified
			if (version.empty() && channel.empty())
				return ERROR_INVALID;

			CreateDirectoryW(temp_dir.c_str(), NULL);

			char* aJson = NULL;
			std::wstring path = L"/update.php?software=" + software + params.str();
			if (WebDownload(_T(UPDATE_DOMAIN), path.c_str(), &aJson, NULL) && aJson != NULL)
			{
				JSONValue* jsonObject = JSON::Parse(aJson);
				if (jsonObject) {
					if (jsonObject->IsObject()) {
						JSONObject update;
						if (!version.empty() && channel.empty()) 
							update = GetJSONObjectSafe(jsonObject->AsObject(), L"update");
						else 
							update = GetJSONObjectSafe(jsonObject->AsObject(), L"release");
						pFiles = ReadUpdate(update);
					}
					delete jsonObject;
				}
				free(aJson);

				if (!pFiles) {
					std::wcout << L"No update found !!!" << std::endl;
					return ERROR_GET;
				}

				if (!VerifyUpdate(pFiles)) {
					std::wcout << L"INVALID " _T(UPDATE_FILE) L" SIGNATURE !!!" << std::endl;
					return ERROR_SIGN;
				}
			}

			ret = 0;

			bSave = true;
		}

		if (step != L"get")
		{
			std::wstring scope = GetArgument(arguments, L"scope");
			//if(software == L"sandboxie" && scope.empty())
			//	scope = L"core";

			ret = ProcessUpdate(pFiles, step, temp_dir, base_dir, scope);

			if (ret >= 0 && (step.empty() || step == L"apply"))
			{
				bool bRestart = HasFlag(arguments, L"restart");

				if (bRestart) {
					Execute(base_dir + L"\\KmdUtil.exe", L"scandll_silent");
					Execute(base_dir + L"\\KmdUtil.exe", L"stop SbieSvc");

					if (Execute(base_dir + L"\\KmdUtil.exe", L"stop SbieDrv"))
					{
						Sleep(3000);
						Execute(base_dir + L"\\KmdUtil.exe", L"stop SbieDrv");
					}
				}

				ret = ApplyUpdate(base_dir, temp_dir, pFiles);
				if (ret <= 0)
					return ret; // error or nothing todo

				if (bRestart) {
					Sleep(1000);
					Execute(base_dir + L"\\KmdUtil.exe", L"start SbieSvc");
				}

				std::wstring wOpen = GetArgument(arguments, L"open");
				if (!wOpen.empty()) {
					Execute(base_dir + L"\\start.exe", L"open_agent:" + wOpen);
				}
			}
		}

		if (ret >= 0) {
			std::wstring file_path = temp_dir + L"\\" _T(UPDATE_FILE);
			if (step.empty() || step == L"apply" || (ret == 0 && step != L"get"))
				DeleteFileW(file_path.c_str()); // cleanup after apply or if there are no updates
			//else { // store partial state to file
			else if(bSave) {
				std::string Json = WriteUpdate(pFiles);
				MyWriteFile((wchar_t*)file_path.c_str(), (char*)Json.c_str(), Json.length());
			}
		}

		return ret;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//

	PrintUsage();
	return ERROR_INVALID;
}
