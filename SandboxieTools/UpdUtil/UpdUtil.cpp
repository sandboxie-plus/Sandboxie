/*
 * Copyright 2022-2023 David Xanatos, xanasoft.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "framework.h"
#include <shellapi.h>
#include <io.h>
#include <fcntl.h>
#include <aclapi.h>
#include <iostream>
#include "../Common/helpers.h"
#include "../Common/WebUtils.h"
#include "../Common/json/JSON.h"
#include "UpdUtil.h"

bool GetDriverInfo(DWORD InfoClass, void* pBuffer, size_t Size);

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

struct SFiles
{
	std::map<std::wstring, std::shared_ptr<SFile>> Map;
	std::wstring Sign;
};

struct SRelease: SFiles
{
	//SRelease() : Status(eLoaded) {}
	std::wstring Version;
	int iUpdate;
	std::wstring CI;
	//enum EStatus
	//{
	//	eLoaded = 0,
	//	eScanned,
	//	ePrepared
	//}				Status;
	std::wstring AgentArch;
	std::wstring Framework;
};

typedef std::list<std::wstring> TScope;

struct SAddon : SFiles
{
	std::wstring Id;

	/*std::wstring Name;
	std::wstring Icon;
	std::wstring Description;
	std::wstring Version;
	std::wstring InfoUrl;*/

	JSONObject Data;

	bool IsDefault;

	std::wstring InstallPath;
	std::wstring Installer;
	std::wstring UninstallKey;
};

typedef std::map<std::wstring, std::shared_ptr<SAddon>> TAddonMap;

std::wstring Arch2Str(ULONG architecture)
{
	switch (architecture)
	{
	case IMAGE_FILE_MACHINE_ARM64:	return L"a64";
	case IMAGE_FILE_MACHINE_AMD64:	return L"x64";
	case IMAGE_FILE_MACHINE_I386:	return L"x86";
	default: return L"";
	}
}

extern "C"
{
NTSYSCALLAPI NTSTATUS NTAPI NtQuerySystemInformationEx(
    _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _In_reads_bytes_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_opt_(SystemInformationLength) PVOID SystemInformation,
    _In_ ULONG SystemInformationLength,
    _Out_opt_ PULONG ReturnLength
    );
}

ULONG GetSysArch()
{
    USHORT architecture = 0;
    NTSTATUS status;
	HANDLE ProcessHandle = GetCurrentProcess();
	ULONG bufferLength;
	SYSTEM_SUPPORTED_PROCESSOR_ARCHITECTURES_INFORMATION* buffer;
    ULONG returnLength;

	bufferLength = sizeof(SYSTEM_SUPPORTED_PROCESSOR_ARCHITECTURES_INFORMATION[5]);
	buffer = (SYSTEM_SUPPORTED_PROCESSOR_ARCHITECTURES_INFORMATION*)malloc(bufferLength);

	const ULONG SystemSupportedProcessorArchitectures = 181;
    status = NtQuerySystemInformationEx((SYSTEM_INFORMATION_CLASS)SystemSupportedProcessorArchitectures, &ProcessHandle, sizeof(ProcessHandle), buffer, bufferLength, &returnLength);

    if (NT_SUCCESS(status))
    {
        for (ULONG i = 0; i < returnLength / sizeof(SYSTEM_SUPPORTED_PROCESSOR_ARCHITECTURES_INFORMATION); i++)
        {
            if (buffer[i].Native)
            {
                architecture = (USHORT)buffer[i].Machine;
                break;
            }
        }
    }
    else // windows 7 fallback
    {
        SYSTEM_INFO SystemInfo = {0};
        GetNativeSystemInfo(&SystemInfo);
        switch (SystemInfo.wProcessorArchitecture)
        {
        case PROCESSOR_ARCHITECTURE_AMD64:  architecture = IMAGE_FILE_MACHINE_AMD64; break;
		//case PROCESSOR_ARCHITECTURE_ARM:    architecture = ; break;
        case PROCESSOR_ARCHITECTURE_ARM64:  architecture = IMAGE_FILE_MACHINE_ARM64; break;
        //case PROCESSOR_ARCHITECTURE_IA64:   architecture = IMAGE_FILE_MACHINE_IA64; break; // itanium
        case PROCESSOR_ARCHITECTURE_INTEL:  architecture = IMAGE_FILE_MACHINE_I386; break;
        }
    }

    free(buffer);

	return architecture;
}

std::wstring ReadRegistryStringValue(std::wstring key, const std::wstring& valueName) 
{
	auto RootPath = Split2(key, L"\\");

	HKEY hKey;
    if (_wcsicmp(RootPath.first.c_str(), L"HKEY_LOCAL_MACHINE") == 0)
        hKey = HKEY_LOCAL_MACHINE;
    else if (_wcsicmp(RootPath.first.c_str(), L"HKEY_CURRENT_USER") == 0)
        hKey = HKEY_CURRENT_USER;
    else 
        return L"";

    if (RegOpenKeyEx(hKey, RootPath.second.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) 
	{
        wchar_t valueData[0x1000] = L"";
        DWORD dataSize = sizeof(valueData);
        DWORD valueType;

		RegQueryValueExW(hKey, valueName.c_str(), NULL, &valueType, (LPBYTE)valueData, &dataSize);

        RegCloseKey(hKey);

		return valueData;
    }
	return L"";
}

ULONG GetBinaryArch(const std::wstring& file)
{
	ULONG arch = 0;

    HANDLE hFile = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
		goto finish;
    
    IMAGE_DOS_HEADER dosHeader;
    DWORD bytesRead;
    if (!ReadFile(hFile, &dosHeader, sizeof(IMAGE_DOS_HEADER), &bytesRead, NULL) || bytesRead != sizeof(IMAGE_DOS_HEADER))
        goto finish;
    
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
        goto finish;
    
    SetFilePointer(hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN);
    IMAGE_NT_HEADERS ntHeader;
    if (!ReadFile(hFile, &ntHeader, sizeof(IMAGE_NT_HEADERS), &bytesRead, NULL) || bytesRead != sizeof(IMAGE_NT_HEADERS))
        goto finish;
    
    if (ntHeader.Signature != IMAGE_NT_SIGNATURE)
        goto finish;
    
	arch = ntHeader.FileHeader.Machine;
    
finish:
    if(hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	return arch;
}

std::wstring GetFileVersion(const std::wstring& file)
{
	LPVOID versionInfo = NULL;
	VS_FIXEDFILEINFO *fixedFileInfo = NULL;
    UINT fixedFileInfoSize;

    DWORD versionHandle;
    DWORD versionSize = GetFileVersionInfoSizeW(file.c_str(), &versionHandle);
	if (versionSize == 0)
		goto finish;

	versionInfo = malloc(versionSize);

    if (!GetFileVersionInfo(file.c_str(), versionHandle, versionSize, versionInfo)) 
		goto finish;

    if (!VerQueryValue(versionInfo, L"\\", (LPVOID *)&fixedFileInfo, &fixedFileInfoSize)) 
		goto finish;

finish:
	std::wstring version;

	if (fixedFileInfo) 
	{
		DWORD fileVersionMS = fixedFileInfo->dwFileVersionMS;
		DWORD fileVersionLS = fixedFileInfo->dwFileVersionLS;

		WORD majorVersion = HIWORD(fileVersionMS);
		WORD minorVersion = LOWORD(fileVersionMS);
		WORD buildNumber = HIWORD(fileVersionLS);
		WORD revisionNumber = LOWORD(fileVersionLS);

		version = std::to_wstring(majorVersion) + L"." + std::to_wstring(minorVersion) + L"." + std::to_wstring(buildNumber);
		if(revisionNumber) version += L"." + std::to_wstring(revisionNumber);
	}

	if (versionInfo)free(versionInfo);

	return version;
}

void CreateDirectoryTree(const std::wstring& root, const std::wstring& path) 
{
    wchar_t* pathCopy = _wcsdup(path.c_str());
    wchar_t* token = _wcstok(pathCopy, L"\\");

    wchar_t currentPath[MAX_PATH] = L"";
	wcscpy(currentPath, root.c_str());

	for (; token != NULL;) {

		wcscat(currentPath, L"\\");
		wcscat(currentPath, token);

		if (CreateDirectoryW(currentPath, NULL) == 0) {
			if (GetLastError() != ERROR_ALREADY_EXISTS) {
				printf("Error creating directory %S\n", currentPath);
			}
		}

		token = _wcstok(NULL, L"\\");
	}

    free(pathCopy);
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

bool DeleteDirectoryRecursively(const std::wstring& root, const std::wstring& path, bool bWithFiles)
{
	std::vector<std::wstring> Entries;
	ListDir(root + path, Entries);

	for (int i = 0; i < Entries.size(); i++)
	{
		auto path_name = SplitName(Entries[i]);
		if (path_name.second.empty()) {
			if (!DeleteDirectoryRecursively(Entries[i], L"", bWithFiles))
				return false;
		}
		else if (bWithFiles) {
			if (!DeleteFileW(Entries[i].c_str()))
				return false;
		}
		else
			return false;
	}

	return !!RemoveDirectoryW((root + path).c_str());
}


std::shared_ptr<SRelease> ScanDir(std::wstring Path) 
{
	if (Path.back() != L'\\') 
		Path.push_back(L'\\');

	std::shared_ptr<SRelease> pFiles = std::make_shared<SRelease>();

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

int GetJSONBoolSafe(const JSONObject& root, const std::wstring& key, bool def = false)
{
	auto I = root.find(key);
	if (I == root.end() || !I->second->IsBool())
		return def;
	return I->second->AsBool();
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

std::string WriteUpdate(std::shared_ptr<SRelease> pFiles)
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
			
	//if(pFiles->Status == SRelease::eScanned)
	//	root[L"status"] = new JSONValue(L"scanned");
	//else if(pFiles->Status == SRelease::ePrepared)
	//	root[L"status"] = new JSONValue(L"prepared");

	root[L"framework"] = new JSONValue(pFiles->Framework);
	root[L"agent_arch"] = new JSONValue(pFiles->AgentArch);

	JSONValue *value = new JSONValue(root);
	auto wJson = value->Stringify();
	delete value;

	return g_str_conv.to_bytes(wJson);
}

void ReadFiles(const JSONArray& jsonFiles, std::shared_ptr<SFiles> pFiles)
{
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
}

std::shared_ptr<SRelease> ReadUpdate(const JSONObject& jsonObject)
{
	if (jsonObject.find(L"files") == jsonObject.end())
		return std::shared_ptr<SRelease>();

	std::shared_ptr<SRelease> pFiles = std::make_shared<SRelease>();

	JSONArray jsonFiles = GetJSONArraySafe(jsonObject, L"files");
	ReadFiles(jsonFiles, pFiles);

	pFiles->Sign = GetJSONStringSafe(jsonObject, L"signature");

	pFiles->Version = GetJSONStringSafe(jsonObject, L"version");
	pFiles->iUpdate = GetJSONIntSafe(jsonObject, L"update", -1);

	pFiles->CI = GetJSONStringSafe(jsonObject, L"ci");

	std::wstring status = GetJSONStringSafe(jsonObject, L"status");
	//if(status == L"scanned")
	//	pFiles->Status = SRelease::eScanned;
	//else if(status == L"prepared")
	//	pFiles->Status = SRelease::ePrepared;

	pFiles->Framework = GetJSONStringSafe(jsonObject, L"framework");
	pFiles->AgentArch = GetJSONStringSafe(jsonObject, L"agent_arch");

	return pFiles;
}

bool VerifyUpdate(std::shared_ptr<SFiles> pFiles)
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

int FindChanges(std::shared_ptr<SRelease> pNewFiles, std::wstring base_dir, std::wstring temp_dir, std::shared_ptr<TScope> pScope)
{
	std::shared_ptr<SRelease> pOldFiles = ScanDir(base_dir);
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

int DownloadUpdate(std::wstring temp_dir, std::shared_ptr<SFiles> pNewFiles)
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
			CreateDirectoryTree(temp_dir, path_name.first);

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

int ApplyUpdate(std::wstring base_dir, std::wstring temp_dir, std::shared_ptr<SFiles> pNewFiles)
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
			CreateDirectoryTree(base_dir, path_name.first);

		if (MoveFileExW(src.c_str(), dest.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {

			// inherit parent folder permissions
			ACL g_null_acl = { 0 };
			InitializeAcl(&g_null_acl, sizeof(g_null_acl), ACL_REVISION);
			DWORD error = SetNamedSecurityInfoW((wchar_t*)dest.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION, NULL, NULL, (PACL)&g_null_acl, NULL);

			std::wcout << L" done" << std::endl;
		} else
			std::wcout << L" FAILED" << std::endl;
	}

	//std::wstring src = temp_dir + L"\\" _T(UPDATE_FILE);
	//std::wstring dest = base_dir + L"\\" _T(UPDATE_FILE);
	//MoveFileExW(src.c_str(), dest.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);

	return Count;
}

void Execute(std::wstring wFile, std::wstring wParams)
{
	SHELLEXECUTEINFO si = { sizeof(SHELLEXECUTEINFO) };
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpVerb = L"runas";
	si.lpFile = wFile.c_str();
	si.lpParameters = wParams.c_str();
	si.nShow = SW_HIDE;
		
	std::wcout << L"KmdUtil.exe " << si.lpParameters << std::endl;
	if (ShellExecuteEx(&si)) {
		WaitForSingleObject(si.hProcess, INFINITE);
		CloseHandle(si.hProcess);
	}
}

int ProcessUpdate(std::shared_ptr<SRelease>& pFiles, const std::wstring& step, const std::wstring& temp_dir, const std::wstring& base_dir, const std::wstring& scope)
{
	int ret = 0;

	if (!pFiles || pFiles->Map.empty())
		return ERROR_INTERNAL;

	//if (step.empty() || step == L"scan" || pFiles->Status < SRelease::eScanned)
	if (step.empty() || step == L"scan" || step == L"prepare" || step == L"apply")
	{
		std::shared_ptr<TScope> pScope;
		if (!scope.empty()) pScope = GetScope(scope);

		ret = FindChanges(pFiles, base_dir, temp_dir, pScope);
		//pFiles->Status = SRelease::eScanned;
		if (ret <= 0)
			return ret; // error or nothing todo
		if (step == L"scan")
			return ret;
	}

	//if (step.empty() || step == L"prepare" || pFiles->Status < SRelease::ePrepared)
	if (step.empty() || step == L"prepare" || step == L"apply")
	{
		ret = DownloadUpdate(temp_dir, pFiles);
		//pFiles->Status = SRelease::ePrepared;
		if (ret <= 0)
			return ret; // error or nothing todo
		if (step == L"prepare")
			return ret;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// addon stuff
//

std::wstring GetSpecificEntry(const JSONObject& root, const std::wstring& name, const std::wstring& core_arch, const std::wstring& agent_arch, const std::wstring& framework)
{
	if (!framework.empty() && root.find(name + L"-" + framework) != root.end())
		return name + L"-" + framework;

	std::wstring match = GetJSONStringSafe(root, L"matchArch");
	std::wstring arch = (match != L"agent") ? core_arch : agent_arch;
	if (!arch.empty() && root.find(name + L"-" + arch) != root.end())
		return name + L"-" + arch;

	return name;
}

std::wstring GetSpecificEntryValue(const JSONObject& root, const std::wstring& name, const std::wstring& core_arch, const std::wstring& agent_arch, const std::wstring& framework)
{
	std::wstring entry = GetSpecificEntry(root, name, core_arch, agent_arch, framework);
	return GetJSONStringSafe(root, entry);
}

std::shared_ptr<SAddon> ReadAddon(const JSONObject& addon, const std::wstring& core_arch, const std::wstring& agent_arch, const std::wstring& framework)
{
	std::shared_ptr<SAddon> pAddon = std::make_shared<SAddon>();
	pAddon->Id = GetJSONStringSafe(addon, L"id");

	std::wstring entry = GetSpecificEntry(addon, L"files", core_arch, agent_arch, framework);

	JSONArray jsonFiles = GetJSONArraySafe(addon, entry);
	ReadFiles(jsonFiles, pAddon);

	pAddon->Sign = GetJSONStringSafe(addon, entry + L"-sig");

	/*pAddon->Name = GetJSONStringSafe(addon, L"name");
	pAddon->Icon = GetJSONStringSafe(addon, L"icon");
	pAddon->Description = GetJSONStringSafe(addon, L"description");
	pAddon->Version = GetJSONStringSafe(addon, L"version");
	pAddon->InfoUrl = GetJSONStringSafe(addon, L"info_url");*/

	pAddon->IsDefault = GetJSONBoolSafe(addon, L"default");

	pAddon->InstallPath = GetSpecificEntryValue(addon, L"installPath", core_arch, agent_arch, framework);
	pAddon->Installer = GetSpecificEntryValue(addon, L"installer", core_arch, agent_arch, framework);
	pAddon->UninstallKey = GetSpecificEntryValue(addon, L"uninstallKey", core_arch, agent_arch, framework);

	for (auto I = addon.begin(); I != addon.end(); ++I) {
		if (I->first.find(L'-') != std::wstring::npos)
			continue; // skip all entries containing "-"
		pAddon->Data.insert(*I);
	}

	return pAddon;
}

JSONObject WriteAddon(std::shared_ptr<SAddon> pAddon)
{
	JSONObject addon;

	addon[L"id"] = new JSONValue(pAddon->Id);
		
	JSONArray files;
	for (auto J = pAddon->Map.begin(); J != pAddon->Map.end(); ++J)
	{
		JSONObject file;
		file[L"path"] = new JSONValue(J->second->Path);
		file[L"hash"] = new JSONValue(J->second->Hash);
		file[L"url"] = new JSONValue(J->second->Url);
		files.push_back(new JSONValue(file));
	}
	if (!files.empty()) {
		addon[L"files"] = new JSONValue(files);

		addon[L"files-sig"] = new JSONValue(pAddon->Sign);
	}

	/*addon[L"name"] = new JSONValue(pAddon->Name);
	addon[L"icon"] = new JSONValue(pAddon->Icon);
	addon[L"description"] = new JSONValue(pAddon->Description);
	addon[L"version"] = new JSONValue(pAddon->Version);
	addon[L"info_url"] = new JSONValue(pAddon->InfoUrl);*/

	addon[L"default"] = new JSONValue(pAddon->IsDefault);

	if (!pAddon->InstallPath.empty()) addon[L"installPath"] = new JSONValue(pAddon->InstallPath);
	if (!pAddon->Installer.empty()) addon[L"installer"] = new JSONValue(pAddon->Installer);
	if (!pAddon->UninstallKey.empty()) addon[L"uninstallKey"] = new JSONValue(pAddon->UninstallKey);
		
	for (auto I = pAddon->Data.begin(); I != pAddon->Data.end(); ++I) {
		addon.insert(*I);
	}

	return addon;
}

std::shared_ptr<TAddonMap> ReadAddons(const JSONObject& jsonObject, const std::wstring& core_arch, const std::wstring& agent_arch, const std::wstring& framework)
{
	if (jsonObject.find(L"list") == jsonObject.end())
		return std::shared_ptr<TAddonMap>();

	std::shared_ptr<TAddonMap> pAddons = std::make_shared<TAddonMap>();

	JSONArray list = GetJSONArraySafe(jsonObject, L"list");
	for (auto I = list.begin(); I != list.end(); ++I) {
		if ((*I)->IsObject()) {
			JSONObject addon = (*I)->AsObject();

			std::shared_ptr<SAddon> pAddon = ReadAddon(addon, core_arch, agent_arch, framework);

			(*pAddons)[MkLower(pAddon->Id)] = pAddon;
		}
	}

	return pAddons;
}

std::string WriteAddons(std::shared_ptr<TAddonMap> pAddons)
{
	JSONObject root;

	JSONArray list;
	for (auto I = pAddons->begin(); I != pAddons->end(); ++I) 
	{
		list.push_back(new JSONValue(WriteAddon(I->second)));
	}
	root[L"list"] = new JSONValue(list);

	JSONValue *value = new JSONValue(root);
	auto wJson = value->Stringify();
	delete value;

	return g_str_conv.to_bytes(wJson);
}

bool VerifyAddons(std::shared_ptr<TAddonMap> pAddons)
{
	for (auto I = pAddons->begin(); I != pAddons->end(); ++I)
	{
		if (I->second->Map.empty())
			continue;
		if (!VerifyUpdate(I->second))
			return false;
	}
	return true;
}

int DownloadAddon(std::shared_ptr<SAddon> pAddon, const std::wstring& step, const std::wstring& temp_dir, const std::wstring& base_dir)
{
	int ret = 0;

	if (!pAddon || pAddon->Map.empty()) {
		std::wcout << L"Addon is not available for this platform" << std::endl;
		return ERROR_NO_ADDON2;
	}

	// always mark all files for download
	for (auto I = pAddon->Map.begin(); I != pAddon->Map.end(); ++I)
		I->second->State = SFile::eChanged;

	//if (step.empty() || step == L"prepare" || pFiles->Status < SRelease::ePrepared)
	if (step.empty() || step == L"prepare" || step == L"apply")
	{
		CreateDirectoryTree(temp_dir, pAddon->Id);
		ret = DownloadUpdate(temp_dir + L"\\" + pAddon->Id, pAddon);
		//pFiles->Status = SRelease::ePrepared;
		if (ret <= 0)
			return ret; // error or nothing todo
		if (step == L"prepare")
			return ret;
	}

	return ret;
}

int InstallAddon(std::shared_ptr<SAddon> pAddon, const std::wstring& temp_dir, const std::wstring& base_dir)
{
	int ret = 0;

	if (!pAddon->Installer.empty() && FileExists((temp_dir + L"\\" + pAddon->Id + pAddon->Installer).c_str())) {

		LPWCH environmentStrings = GetEnvironmentStrings();

		DWORD environmentLen = 0;
		for (LPWCH current = environmentStrings; *current; current += wcslen(current) + 1)
			environmentLen += wcslen(current) + 1;

		LPWCH modifiedEnvironment = (LPWCH)LocalAlloc(0, (environmentLen + 32 + base_dir.length() + 1 + 1) * sizeof(wchar_t));
		memcpy(modifiedEnvironment, environmentStrings, (environmentLen + 1) * sizeof(wchar_t));
				
		FreeEnvironmentStrings(environmentStrings);

		LPWCH modifiedEnvironmentEnd = modifiedEnvironment + environmentLen;

		wcscpy(modifiedEnvironmentEnd, L"SBIEHOME=");
		wcscat(modifiedEnvironmentEnd, base_dir.c_str());
		modifiedEnvironmentEnd += wcslen(modifiedEnvironmentEnd) + 1;

		*modifiedEnvironmentEnd = 0;

		STARTUPINFO si = { sizeof(si), 0 };
		PROCESS_INFORMATION pi = { 0 };
		std::wstring cmdLine = temp_dir + L"\\" + pAddon->Id + pAddon->Installer;
		if (CreateProcessW(NULL, (wchar_t*)cmdLine.c_str(), NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, modifiedEnvironment, NULL, &si, &pi))
		{
			while (WaitForSingleObject(pi.hProcess, 1000) == WAIT_TIMEOUT);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else
			ret = ERROR_BAD_ADDON2;

		LocalFree(modifiedEnvironment);

		if (ret >= 0 && !pAddon->UninstallKey.empty()) {
			std::wstring cmd = ReadRegistryStringValue(pAddon->UninstallKey, L"UninstallString");
			if (cmd.empty()) // when the expected uninstall key is not present,
				ret = ERROR_BAD_ADDON2; // it means the installation failed
		}

		return ret;
	}

	if (pAddon->InstallPath.empty())
		return ERROR_BAD_ADDON;

	// install addon
	CreateDirectoryTree(base_dir, pAddon->InstallPath);
	ret = ApplyUpdate(base_dir + pAddon->InstallPath, temp_dir + L"\\" + pAddon->Id, pAddon);

	return ret;
}

std::shared_ptr<SAddon> LoadAddon(const std::wstring& base_dir, const std::wstring& id)
{
	std::shared_ptr<SAddon> pAddon;

	char* aJson = NULL;
	std::wstring file_path = base_dir + _T(ADDONS_PATH) + id + L".json";
	if (NT_SUCCESS(MyReadFile((wchar_t*)file_path.c_str(), 1024 * 1024, (PVOID*)&aJson, NULL)) && aJson != NULL) 
	{	
		JSONValue* jsonObject = JSON::Parse(aJson);
		if (jsonObject) {
			if (jsonObject->IsObject())
				pAddon = ReadAddon(jsonObject->AsObject(), L"", L"", L"");
			delete jsonObject;
		}
		free(aJson);
	}

	return pAddon;
}

int RemoveAddon(std::shared_ptr<SAddon> pAddon, const std::wstring& base_dir)
{
	int ret = 0;

	if (!pAddon->UninstallKey.empty())
	{
		std::wstring cmdLine = ReadRegistryStringValue(pAddon->UninstallKey, L"UninstallString");
		if(cmdLine.empty()) // when the expected uninstall key is not present,
			return ret; // then it seems the addon was already uninstalled

		STARTUPINFO si = { sizeof(si), 0 };
		PROCESS_INFORMATION pi = { 0 };
		if (CreateProcessW(NULL, (wchar_t*)cmdLine.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			while (WaitForSingleObject(pi.hProcess, 1000) == WAIT_TIMEOUT);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else
			ret = ERROR_BAD_ADDON2;

		if (ret >= 0) {
			std::wstring cmd = ReadRegistryStringValue(pAddon->UninstallKey, L"UninstallString");
			if (!cmd.empty()) // when the expected uninstall key is still present,
				ret = ERROR_BAD_ADDON2; // it means the uninstallation failed
		}

		return ret;
	}

	if (pAddon->InstallPath.empty())
		return ERROR_BAD_ADDON;

	for (auto I = pAddon->Map.begin(); I != pAddon->Map.end(); ++I)
	{
		std::wstring file = base_dir + pAddon->InstallPath + I->second->Path;
		if (!DeleteFileW(file.c_str()) && FileExists(file.c_str())) {
			ret = ERROR_DELETE;
			break;
		}
	}

	if (ret >= 0 && pAddon->InstallPath != L"\\")
		DeleteDirectoryRecursively(base_dir, pAddon->InstallPath, false);

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// other stuff
//

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

std::wstring GetArgument(const std::vector<std::wstring>& arguments, std::wstring name, std::wstring mod = L"/") 
{
	std::wstring prefix = mod + name + L":";
	for (size_t i = 0; i < arguments.size(); i++) {
		if (_wcsicmp(arguments[i].substr(0, prefix.length()).c_str(), prefix.c_str()) == 0) {
			return arguments[i].substr(prefix.length());
		}
	}
	return L"";
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

	// install sandboxie-plus /version:1.6.1 /path:C:\Projects\Sandboxie\SandboxieLive\x64\Debug\Sandboxie_test

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
		if (arch == L"x86_64")
			arch = L"x64";
		else if (arch == L"i386")
			arch = L"x86";
		else if (arch == L"ARM64" || arch == L"A64" || arch == L"arm64")
			arch = L"a64";
	} else
		arch = Arch2Str(GetSysArch());

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
	else if ((arguments.size() >= 2 && (arguments[0] == L"update" || arguments[0] == L"upgrade" || arguments[0] == L"install")) || (arguments.size() >= 1 && arguments[0] == L"modify"))
	{
		int ret = 0;

		bool bModify = arguments[0] == L"modify";
		bool bDoAddons = (bModify || arguments[0] == L"install");

		std::wstring software = bModify ? L"sandboxie-addons" : arguments[1];

		std::shared_ptr<SRelease> pFiles;
		JSONValue* jsonAddons = NULL;

		auto add_addons = SplitStr(GetArgument(arguments, L"add", L""), L",", false);
		auto remove_addons = SplitStr(GetArgument(arguments, L"remove", L""), L",", false);

		std::wstring step = GetArgument(arguments, L"step");

		//
		// Prepare update data, load from file or download
		//

		bool bSave = false;
		if (!step.empty() && step != L"get") // load from file
		{
			if (!bModify) {

				char* aJson = NULL;
				std::wstring file_path = temp_dir + L"\\" _T(UPDATE_FILE);
				if (NT_SUCCESS(MyReadFile((wchar_t*)file_path.c_str(), 1024 * 1024, (PVOID*)&aJson, NULL)) && aJson != NULL) 
				{	
					JSONValue* jsonObject = JSON::Parse(aJson);
					if (jsonObject) {
						if (jsonObject->IsObject())
							pFiles = ReadUpdate(jsonObject->AsObject());
						delete jsonObject;
					}
					free(aJson);
				}

				if (!pFiles) {
					std::wcout << L"No pending update found!" << std::endl;
					return ERROR_LOAD;
				}
			}

			if ((bModify && !add_addons.empty()) || arguments[0] == L"install") {

				char* aJson = NULL;
				std::wstring file_path = temp_dir + L"\\" _T(ADDONS_FILE);
				if (NT_SUCCESS(MyReadFile((wchar_t*)file_path.c_str(), 1024 * 1024, (PVOID*)&aJson, NULL)) && aJson != NULL) 
				{	
					JSONValue* jsonObject = JSON::Parse(aJson);
					if (jsonObject) {
						if (jsonObject->IsObject())
							jsonAddons = jsonObject;
					}
					free(aJson);
				}

				if (!jsonAddons && arguments[0] != L"install") { // only fail here when we want to add addons
					std::wcout << L"No addons found!" << std::endl;
					return ERROR_LOAD;
				}
			}
		}
		else // load from server
		{
			std::wstringstream params;
			params << L"&action=" << arguments[0];

			std::wstring channel;
			std::wstring version;
			if (!bModify)
			{
				channel = GetArgument(arguments, L"channel");
				if (channel.empty()) channel = GetArgument(arguments, L"release");
				if (!channel.empty())
					params << L"&channel=" << channel;

				version = GetArgument(arguments, L"version");
				if (!version.empty()) {
					params << L"&version=" << version;
					std::wstring update = GetArgument(arguments, L"update");
					if (!update.empty())
						params << L"&update=" << update;
				}

				// version or channel must be specified
				if (version.empty() && channel.empty())
					return ERROR_INVALID;
			}

			params << L"&system=windows-" << g_osvi.dwMajorVersion << L"." << g_osvi.dwMinorVersion << L"." << g_osvi.dwBuildNumber << "-" << arch;

			wchar_t StrLang[16];
			LCIDToLocaleName(GetUserDefaultLCID(), StrLang, ARRAYSIZE(StrLang), 0);
			if (StrLang[2] == L'-') StrLang[2] = '_';

			params << L"&language=" << StrLang;

			std::wstring update_key = GetArgument(arguments, L"update_key");
			if (!update_key.empty()) params << L"&update_key=" + update_key;

			CreateDirectoryW(temp_dir.c_str(), NULL);

			char* aJson = NULL;
			std::wstring path = L"/update.php?software=" + software + params.str();
			if (WebDownload(_T(UPDATE_DOMAIN), path.c_str(), &aJson, NULL) && aJson != NULL)
			{
				JSONValue* jsonObject = JSON::Parse(aJson);
				if (jsonObject) {
					if (jsonObject->IsObject()) {

						JSONObject root = jsonObject->AsObject();

						if (!bModify) {
							JSONObject update;
							if (!version.empty() && channel.empty())
								update = GetJSONObjectSafe(root, L"update");
							else
								update = GetJSONObjectSafe(root, L"release");
							pFiles = ReadUpdate(update);
						}
						
						if (bDoAddons) {
							auto I = root.find(L"addons");
							if (I != root.end() && I->second->IsObject()) {
								jsonAddons = I->second;
								root.erase(I);
							}
						}

					}
					delete jsonObject;
				}
				free(aJson);

				if (!bModify && !pFiles) {
					std::wcout << L"No update found !!!" << std::endl;
					return ERROR_GET;
				}
			}

			ret = 0;

			bSave = true;
		}

		if (pFiles && !VerifyUpdate(pFiles)) {
			std::wcout << L"INVALID Update SIGNATURE in " _T(UPDATE_FILE) "!!!" << std::endl;
			return ERROR_SIGN;
		}

		//
		// when needed shutdown the software
		//

		bool bRestart = false;
		if (ret >= 0 && (step.empty() || step == L"apply"))
			bRestart = HasFlag(arguments, L"restart");

		if (bRestart) {
			Execute(base_dir + L"\\KmdUtil.exe", L"scandll_silent");
			Execute(base_dir + L"\\KmdUtil.exe", L"stop SbieSvc");
			Execute(base_dir + L"\\KmdUtil.exe", L"stop SbieDrv");
			Sleep(3000);
			Execute(base_dir + L"\\KmdUtil.exe", L"stop SbieDrv");
		}

		//
		// apply update
		//

		if (step != L"get" && !bModify)
		{
			std::wstring scope = GetArgument(arguments, L"scope");
			//if(software == L"sandboxie" && scope.empty())
			//	scope = L"core";

			ret = ProcessUpdate(pFiles, step, temp_dir, base_dir, scope);

			if (ret >= 0 && (step.empty() || step == L"apply"))
			{
				ret = ApplyUpdate(base_dir, temp_dir, pFiles);
				if (ret <= 0)
					return ret; // error or nothing todo
			}
		}

		//
		// load addons appropriate for the current installation
		//

		std::shared_ptr<TAddonMap> pAddons;
		if (jsonAddons) 
		{
			std::wstring core_arch = Arch2Str(GetBinaryArch(base_dir + L"\\sbiesvc.exe"));

			std::wstring agent_arch = GetArgument(arguments, L"agent_arch");
			if (agent_arch.empty()) {
				agent_arch = Arch2Str(GetBinaryArch(base_dir + L"\\SandMan.exe"));
				if (agent_arch.empty()) agent_arch = Arch2Str(GetBinaryArch(base_dir + L"\\SbieCtrl.exe"));
			}

			std::wstring framework = GetArgument(arguments, L"framework");
			if (framework.empty()) {
				framework = GetFileVersion(base_dir + L"\\Qt5Core.dll");
				if (framework.empty()) framework = GetFileVersion(base_dir + L"\\Qt6Core.dll");
				if (!framework.empty()) framework = L"qt" + framework + L"_" + agent_arch;
			}

			if (pFiles) {
				if (agent_arch.empty()) agent_arch = pFiles->AgentArch;
				if (framework.empty()) agent_arch = pFiles->Framework;
			}

			pAddons = ReadAddons(jsonAddons->AsObject(), core_arch, agent_arch, framework);

			if(!pAddons){
				std::wcout << L"No addons found!" << std::endl;
				if(bModify)
					return ERROR_LOAD;
			}
			else if (!VerifyAddons(pAddons)) {
				pAddons.reset(); // clear untrusted addon data
				std::wcout << L"INVALID " _T(ADDONS_FILE) L" SIGNATURE !!!" << std::endl;
				if(bModify)
					return ERROR_SIGN;
			}
		}

		//
		// install addons
		//

		if (step != L"get" && pAddons)
		{
			if (!bModify) { // install case

				for (auto I = pAddons->begin(); I != pAddons->end(); ++I)
				{
					if (!I->second->IsDefault)
						continue;

					// don't add default addons marked to be removed
					auto F = std::find(remove_addons.begin(), remove_addons.end(), I->first);
					if (F != remove_addons.end())
						continue;

					// don't add already added addons
					F = std::find(add_addons.begin(), add_addons.end(), I->first);
					if (F != add_addons.end())
						continue;

					add_addons.push_back(I->first);
				}
			}

			for (auto I = add_addons.begin(); I != add_addons.end(); ++I)
			{
				auto F = pAddons->find(MkLower(*I));
				if (F != pAddons->end()) {
					std::wcout << L"Downloading addon " << *I << std::endl;
					ret = DownloadAddon(F->second, step, temp_dir, base_dir);
				}
				else {
					std::wcout << L"Addon Not Found" << std::endl;
					ret = ERROR_NO_ADDON;
				}

				if (ret >= 0 && (step.empty() || step == L"apply"))
				{
					std::shared_ptr<SAddon> pAddon = LoadAddon(base_dir, *I);
					if (pAddon && !pAddon->InstallPath.empty()) {
						std::wcout << L"Updating addon " << *I << std::endl;
						RemoveAddon(pAddon, base_dir);
					} else
						std::wcout << L"Installing addon " << *I << std::endl;
					ret = InstallAddon(F->second, temp_dir, base_dir);

					// register addon
					if (ret >= 0)
					{
						JSONValue* value = new JSONValue(WriteAddon(F->second));
						auto wJson = value->Stringify();
						delete value;

						std::string aJson = g_str_conv.to_bytes(wJson);
						MyWriteFile((wchar_t*)(base_dir + _T(ADDONS_PATH) + F->second->Id + L".json").c_str(), (char*)aJson.c_str(), aJson.length());

						if (ret >= 0)
							DeleteDirectoryRecursively(temp_dir + L"\\", F->second->Id + L"\\", true);
					}
				}

				if (ret < 0 && !bModify) // install case
					ret = 0; // ignore addon errors
			}
		}

		//
		// remove addons
		//

		if(step != L"get" && !remove_addons.empty())
		{
			for (auto I = remove_addons.begin(); I != remove_addons.end(); ++I)
			{
				std::shared_ptr<SAddon> pAddon = LoadAddon(base_dir, *I);
				if (!pAddon)
					ret = ERROR_NO_ADDON;

				if (ret >= 0 && (step.empty() || step == L"apply"))
				{
					std::wcout << L"Removing addon " << *I << std::endl;
					ret = RemoveAddon(pAddon, base_dir);

					// unregister addon
					if (ret >= 0)
						DeleteFileW((base_dir + _T(ADDONS_PATH) + pAddon->Id + L".json").c_str());
				}

				if (ret < 0 && !bModify) // install case
					ret = 0; // ignore addon errors
			}
		}

		//
		// restart software
		//

		if (bRestart) {
			Sleep(1000);
			Execute(base_dir + L"\\KmdUtil.exe", L"start SbieSvc");
		}

		std::wstring wOpen = GetArgument(arguments, L"open");
		if (!wOpen.empty()) {
			Execute(base_dir + L"\\start.exe", L"open_agent:" + wOpen);
		}

		//
		// save data to file if needed
		//

		if (ret >= 0) 
		{
			if (!bModify) {
				std::wstring file_path = temp_dir + L"\\" _T(UPDATE_FILE);
				if (step.empty() || step == L"apply" || (ret == 0 && step != L"get"))
					DeleteFileW(file_path.c_str()); // cleanup after apply or if there are no updates
				//else { // store partial state to file
				else if (bSave) {
					std::string Json = WriteUpdate(pFiles);
					MyWriteFile((wchar_t*)file_path.c_str(), (char*)Json.c_str(), Json.length());
				}
			}

			if (bDoAddons && pAddons) {
				std::wstring file_path = temp_dir + L"\\" _T(ADDONS_FILE);
				if (step.empty() || step == L"apply" /*|| (ret == 0 && step != L"get")*/)
					DeleteFileW(file_path.c_str());
				else if (bSave) {
					std::string Json = WriteAddons(pAddons);
					MyWriteFile((wchar_t*)file_path.c_str(), (char*)Json.c_str(), Json.length());
				}
			}
		}

		return ret;
	}
	else if (arguments.size() >= 2 && (arguments[0] == L"get_cert" || arguments[0] == L"get_cert_lr"))
	{
		int ret = 0;

		std::wstring file_path = base_dir + L"\\Certificate.dat";

		std::wstring path;

		std::wstring serial;

		std::wstring hwid = GetArgument(arguments, L"hwid");

		if (arguments[0] == L"get_cert_lr") 
		{
			path = L"/get_cert.php?LR=1";
			serial = GetArgument(arguments, L"serial");
			if(!serial.empty())
				path += L"&SN=" + serial;
		}
		else
		{
			serial = arguments[1];

			path = L"/get_cert.php?SN=" + serial;
		}
		

		if (hwid.empty()) {
			if (serial.length() > 5 && toupper(serial[4]) == 'N') { // node locked business use
				wchar_t uuid_str[40];
				GetDriverInfo(-2, uuid_str, sizeof(uuid_str));
				hwid = std::wstring(uuid_str);
			}
		}
		else // when hwid is specified manually, assume it's not for this system
			file_path.clear();
		if (!hwid.empty())
			path += L"&HwId=" + hwid;

		std::wstring update_key = GetArgument(arguments, L"update_key");
		if (!file_path.empty() && update_key.empty()) {
			char* aCert = NULL;
			if (NT_SUCCESS(MyReadFile((wchar_t*)file_path.c_str(), 1024 * 1024, (PVOID*)&aCert, NULL)) && aCert != NULL) {
				std::string sCert = aCert;
				free(aCert);
				auto Cert = GetArguments(std::wstring(sCert.begin(), sCert.end()), L'\n', L':');
				auto F = Cert.find(L"UPDATEKEY");
				if (F != Cert.end())
					update_key = F->second;
			}
		}
		if(!update_key.empty())
			path += L"&UpdateKey=" + update_key;

		char* aCert = NULL;
		ULONG lCert = 0;
		if (WebDownload(_T(UPDATE_DOMAIN), path.c_str(), &aCert, &lCert) && aCert != NULL && *aCert) 
		{
			if (aCert[0] == L'{') {
				JSONValue* jsonObject = JSON::Parse(aCert);
				if (jsonObject) {
					if (jsonObject->IsObject() && GetJSONBoolSafe(jsonObject->AsObject(), L"error"))
					{
						std::wcout << GetJSONStringSafe(jsonObject->AsObject(), L"errorMsg") << std::endl;

						ret = ERROR_GET_CERT;
					}
					delete jsonObject;
				}
				free(aCert);
			}
		}
		else
		{
			std::wcout << L"FAILED to call get_cert.php" << std::endl;

			ret = ERROR_GET_CERT;
		}

		if (ret == 0) 
		{
			if (file_path.empty())
				printf("== CERTIFICATE ==\r\n%s\r\n== END ==", aCert);
			else if(!NT_SUCCESS(MyWriteFile((wchar_t*)file_path.c_str(), aCert, lCert)))
				ret = ERROR_INTERNAL;
		}

		return ret;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//


	PrintUsage();
	return ERROR_INVALID;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver API

#define SANDBOXIE		L"Sandboxie"

#define SBIESVC_PORT	L"\\RPC Control\\SbieSvcPort"
#define MAX_REQUEST_LENGTH      (2048 * 1024)


typedef struct _UNICODE_STRING64 {
    USHORT Length;
    USHORT MaximumLength;
    __declspec(align(8)) unsigned __int64 Buffer;
} UNICODE_STRING64;

#include "..\..\Sandboxie\common\defines.h"

#include "..\..\Sandboxie\core\drv\api_defs.h"
#include "..\..\Sandboxie\core\drv\api_flags.h"

HANDLE SbieApi_DeviceHandle = INVALID_HANDLE_VALUE;

NTSTATUS SbieApi_Ioctl(ULONG64 *parms)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING uni;
    IO_STATUS_BLOCK MyIoStatusBlock;

    if (parms == NULL) {

		if (SbieApi_DeviceHandle != INVALID_HANDLE_VALUE)
			NtClose(SbieApi_DeviceHandle);

        SbieApi_DeviceHandle = INVALID_HANDLE_VALUE;
		return STATUS_SUCCESS;
    }

    if (SbieApi_DeviceHandle == INVALID_HANDLE_VALUE) {

        RtlInitUnicodeString(&uni, API_DEVICE_NAME);
        InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = NtOpenFile(
            &SbieApi_DeviceHandle, FILE_GENERIC_READ, &objattrs, &MyIoStatusBlock,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0);

        if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_NO_SUCH_DEVICE)
            status = STATUS_SERVER_DISABLED;

    } else
        status = STATUS_SUCCESS;

    if (status != STATUS_SUCCESS) {

        SbieApi_DeviceHandle = INVALID_HANDLE_VALUE;

    } else {

        status = NtDeviceIoControlFile(
            SbieApi_DeviceHandle, NULL, NULL, NULL, &MyIoStatusBlock,
            API_SBIEDRV_CTLCODE, parms, sizeof(ULONG64) * 8, NULL, 0);
    }

    return status;
}

//LONG SbieApi_Call(ULONG api_code, LONG arg_num, ...) 
//{
//    va_list valist;
//    NTSTATUS status;
//    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
//
//    memzero(parms, sizeof(parms));
//    parms[0] = api_code;
//
//    if (arg_num >= (API_NUM_ARGS - 1))
//        return STATUS_INVALID_PARAMETER;
//
//    va_start(valist, arg_num);
//    for (LONG i = 1; i <= arg_num; i++)
//        parms[i] = (ULONG64)va_arg(valist, ULONG_PTR);
//    va_end(valist);
//
//    status = SbieApi_Ioctl(parms);
//
//    return status;
//}

bool GetDriverInfo(DWORD InfoClass, void* pBuffer, size_t Size)
{
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_QUERY_DRIVER_INFO_ARGS *args = (API_QUERY_DRIVER_INFO_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_QUERY_DRIVER_INFO;
	args->info_class.val = InfoClass;
	args->info_data.val = pBuffer;
	args->info_len.val = Size;

	NTSTATUS status = SbieApi_Ioctl(parms);
	if (!NT_SUCCESS(status)) {
		memset(pBuffer, 0, Size);
		return false;
	}
	return true;
}
