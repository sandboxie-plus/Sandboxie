#include "../framework.h"
#include "helpers.h"
#include <winhttp.h>

void GetWebPayload(PVOID RequestHandle, PSTR* pData, ULONG* pDataLength)
{
	PVOID result = NULL;
	ULONG allocatedLength;
	ULONG dataLength;
	ULONG returnLength;
	BYTE buffer[0x1000];

	if (pData == NULL)
		return;

	allocatedLength = sizeof(buffer);
	*pData = (PSTR)malloc(allocatedLength);
	dataLength = 0;

	while (WinHttpReadData(RequestHandle, buffer, sizeof(buffer), &returnLength))
	{
		if (returnLength == 0)
			break;

		if (allocatedLength < dataLength + returnLength)
		{
			allocatedLength *= 2;
			*pData = (PSTR)realloc(*pData, allocatedLength);
		}

		memcpy(*pData + dataLength, buffer, returnLength);

		dataLength += returnLength;
	}

	if (allocatedLength < dataLength + 1)
	{
		allocatedLength++;
		*pData = (PSTR)realloc(*pData, allocatedLength);
	}

	// Ensure that the buffer is null-terminated.
	(*pData)[dataLength] = 0;

	if (pDataLength != NULL)
		*pDataLength = dataLength;
}


BOOLEAN WebDownload(const WCHAR* Host, const WCHAR* Path, 
	PSTR* pData, ULONG* pDataLength)
{
	BOOLEAN success = FALSE;

	PVOID SessionHandle = NULL;
	PVOID ConnectionHandle = NULL;
	PVOID RequestHandle = NULL;

	{
		SessionHandle = WinHttpOpen(NULL,
			g_osvi.dwMajorVersion >= 8 ? WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY : WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (!SessionHandle)
			goto CleanupExit;

		if (g_osvi.dwMajorVersion >= 8) {
			ULONG Options = WINHTTP_DECOMPRESSION_FLAG_GZIP | WINHTTP_DECOMPRESSION_FLAG_DEFLATE;
			WinHttpSetOption(SessionHandle, WINHTTP_OPTION_DECOMPRESSION, &Options, sizeof(Options));
		}
	}

	{
		ConnectionHandle = WinHttpConnect(SessionHandle, Host, 443, 0); // ssl port
		if (!ConnectionHandle)
			goto CleanupExit;
	}

	{
		ULONG httpFlags = WINHTTP_FLAG_SECURE | WINHTTP_FLAG_REFRESH;
		RequestHandle = WinHttpOpenRequest(ConnectionHandle,
			NULL, Path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, httpFlags);

		if (!RequestHandle)
			goto CleanupExit;

		ULONG Options = WINHTTP_DISABLE_KEEP_ALIVE;
		WinHttpSetOption(RequestHandle, WINHTTP_OPTION_DISABLE_FEATURE, &Options, sizeof(Options));
	}

	if (!WinHttpSendRequest(RequestHandle, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0))
		goto CleanupExit;

	if (!WinHttpReceiveResponse(RequestHandle, NULL))
		goto CleanupExit;

	GetWebPayload(RequestHandle, pData, pDataLength);

	success = TRUE;

CleanupExit:
	if (RequestHandle)
		WinHttpCloseHandle(RequestHandle);
	if (ConnectionHandle)
		WinHttpCloseHandle(ConnectionHandle);
	if (SessionHandle)
		WinHttpCloseHandle(SessionHandle);

	return success;
}


BOOLEAN WebUpload(const WCHAR* Host, const WCHAR* Path, 
	const WCHAR * FileName, PSTR* pFileData, ULONG FileLength, 
	PSTR* pData, ULONG* pDataLength,
	std::map<std::string, std::string> params)
{
	BOOLEAN success = FALSE;

	PVOID SessionHandle = NULL;
	PVOID ConnectionHandle = NULL;
	PVOID RequestHandle = NULL;

	{
		SessionHandle = WinHttpOpen(NULL,
			g_osvi.dwMajorVersion >= 8 ? WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY : WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (!SessionHandle)
			goto CleanupExit;

		if (g_osvi.dwMajorVersion >= 8) {
			ULONG Options = WINHTTP_DECOMPRESSION_FLAG_GZIP | WINHTTP_DECOMPRESSION_FLAG_DEFLATE;
			WinHttpSetOption(SessionHandle, WINHTTP_OPTION_DECOMPRESSION, &Options, sizeof(Options));
		}
	}

	{
		ConnectionHandle = WinHttpConnect(SessionHandle, Host, 443, 0); // ssl port
		if (!ConnectionHandle)
			goto CleanupExit;
	}

	{
		ULONG httpFlags = WINHTTP_FLAG_SECURE | WINHTTP_FLAG_REFRESH;
		RequestHandle = WinHttpOpenRequest(ConnectionHandle,
			L"POST", Path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, httpFlags);

		if (!RequestHandle)
			goto CleanupExit;

		ULONG Options = WINHTTP_DISABLE_KEEP_ALIVE;
		WinHttpSetOption(RequestHandle, WINHTTP_OPTION_DISABLE_FEATURE, &Options, sizeof(Options));
	}

	{
		std::string Boundary = "-------------------------" + std::to_string(GetTickCount64()) + "-" + std::to_string(rand());
		std::wstring wBoundary = g_str_conv.from_bytes(Boundary);
		std::wstring wContentHeader = L"Content-Type: multipart/form-data; boundary=" + wBoundary;

		WinHttpAddRequestHeaders(RequestHandle, wContentHeader.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
		WinHttpAddRequestHeaders(RequestHandle, L"Connection: close", (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);

		std::string Payload;

		for (auto I = params.begin(); I != params.end(); ++I) {

			Payload.append("--" + Boundary + "\r\nContent-Disposition: form-data; name=\"" + I->first + "\"\r\n\r\n");
			Payload.append(I->second);
			Payload.append("\r\n");
		}

		Payload.append("--" + Boundary + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"" + g_str_conv.to_bytes(FileName) + "\"\r\n\r\n");
		Payload.append((char*)pFileData, FileLength);
		Payload.append("\r\n--" + Boundary + "--");

		std::wstring wContentLength = L"Content-Length: " + std::to_wstring(Payload.size());
		
		if(!WinHttpSendRequest(RequestHandle, wContentLength.c_str(), -1, (LPVOID)Payload.c_str(), Payload.size(), Payload.size(), 0))
			goto CleanupExit;
	}

	if (!WinHttpReceiveResponse(RequestHandle, NULL))
		goto CleanupExit;

	GetWebPayload(RequestHandle, pData, pDataLength);

	success = TRUE;

CleanupExit:
	if (RequestHandle)
		WinHttpCloseHandle(RequestHandle);
	if (ConnectionHandle)
		WinHttpCloseHandle(ConnectionHandle);
	if (SessionHandle)
		WinHttpCloseHandle(SessionHandle);

	return success;
}
