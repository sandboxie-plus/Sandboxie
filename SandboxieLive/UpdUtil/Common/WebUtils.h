#pragma once

BOOLEAN WebDownload(const WCHAR* Host, const WCHAR* Path, 
	PSTR* pData, ULONG* pDataLength);

BOOLEAN WebUpload(const WCHAR* Host, const WCHAR* Path,
	const WCHAR* FileName, PSTR* pFileData, ULONG FileLength,
	PSTR* pData, ULONG* pDataLength,
	std::map<std::string, std::string> params = std::map<std::string, std::string>());