#include "../framework.h"
#include "dirent.h"

OSVERSIONINFOW g_osvi;

#pragma warning(disable : 4996)
void InitOsVersionInfo() 
{
	g_osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOW);
	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetVersion");
	if (RtlGetVersion == NULL || !NT_SUCCESS(RtlGetVersion(&g_osvi)))
		GetVersionExW(&g_osvi);
}
		
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> g_str_conv;

bool ListDir(std::wstring Path, std::vector<std::wstring>& Entries)
{
	if (Path.back() != L'\\')
		return false;

	//std::string narrow = g_str_conv.to_bytes(wide_utf16_source_string);
	//std::wstring wide = g_str_conv.from_bytes(narrow_utf8_source_string);
	std::string aPath = g_str_conv.to_bytes(Path);

	DIR* d = opendir (aPath.c_str());
	if (d == NULL)
		return false;

	dirent* e;
	while ((e = readdir(d)) != NULL) 
	{
		std::string aName = e->d_name;
		std::wstring Name = std::wstring(aName.begin(), aName.end());

		switch (e->d_type) 
		{
			case DT_DIR:
				if(Name.compare(L"..") == 0 || Name.compare(L".") == 0)
					continue;
				Entries.push_back(Path + Name + L"\\");
				break;
			default:
				Entries.push_back(Path + Name);
		}
	}
	closedir (d);
	return true;
}

std::wstring hexStr(unsigned char* data, int len)
{
	static constexpr wchar_t hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	std::wstring s(len * 2, ' ');
	for (int i = 0; i < len; ++i) {
		s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
		s[2 * i + 1] = hexmap[data[i] & 0x0F];
	}
	return s;
}