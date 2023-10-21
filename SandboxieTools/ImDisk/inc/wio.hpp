/*
    Win32 overlapped I/O API functions encapsulated in C++ classes.

    Copyright (C) 2005-2007 Olof Lagerkvist.
*/

#ifndef _WIO_HPP
#define _WIO_HPP

__inline LPSTR
WideToByteAlloc(LPCWSTR lpSrc)
{
  LPSTR lpDst;
  int iReqSize =
    WideCharToMultiByte(CP_ACP, 0, lpSrc, -1, NULL, 0, NULL, NULL);
  if (iReqSize == 0)
    return NULL;

  lpDst = (LPSTR) malloc(iReqSize);
  if (lpDst == NULL)
    return NULL;

  if (WideCharToMultiByte(CP_ACP, 0, lpSrc, -1, lpDst, iReqSize, NULL, NULL)
      != iReqSize)
    {
      free(lpDst);
      return NULL;
    }

  return lpDst;
}

__inline SOCKET
ConnectTCP(u_long ulAddress, u_short usPort)
{
  // Open socket
  SOCKET sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sd == INVALID_SOCKET)
    return INVALID_SOCKET;

  sockaddr_in addr = { 0 };
  addr.sin_family = AF_INET;
  addr.sin_port = usPort;
  addr.sin_addr.s_addr = ulAddress;

  if (connect(sd, (sockaddr*)&addr, sizeof addr) == SOCKET_ERROR)
    {
      int __h_errno = WSAGetLastError();
      closesocket(sd);
      WSASetLastError(__h_errno);
      return INVALID_SOCKET;
    }

  return sd;
}

__inline SOCKET
ConnectTCP(LPCWSTR wszServer, u_short usPort)
{
  if (wszServer == NULL)
    return INVALID_SOCKET;

  if (usPort == 0)
    return INVALID_SOCKET;

  LPSTR szServer = WideToByteAlloc(wszServer);

  // Get server address
  u_long haddr = inet_addr(szServer);

  // Wasn't IP? Lookup host.
  if (haddr == INADDR_NONE)
    {
      hostent *hent = gethostbyname(szServer);
      if (hent == NULL)
	{
	  free(szServer);
	  return INVALID_SOCKET;
	}

      haddr = *(u_long*)hent->h_addr;
    }

  free(szServer);

  return ConnectTCP(haddr, usPort);
}

__inline SOCKET
ConnectTCP(LPCWSTR wszServer, LPCWSTR wszService)
{
  if (wszServer == NULL)
    return INVALID_SOCKET;

  if (wszService == NULL)
    return INVALID_SOCKET;

  u_short usPort = htons((u_short)wcstoul(wszService, NULL, 0));
  if (usPort == 0)
    {
      // Get port name for service
      LPSTR szService = WideToByteAlloc(wszService);
      servent *service = getservbyname(szService, "tcp");
      free(szService);
      if (service == NULL)
	return INVALID_SOCKET;

      usPort = service->s_port;
    }

  return ConnectTCP(wszServer, usPort);
}

/// Enhanced OVERLAPPED structure with encapsulated API functions.
struct WOverlapped : public OVERLAPPED
{
  BOOL Read(HANDLE hFile, LPVOID lpBuf, DWORD dwLength, DWORDLONG dwStart = 0)
  {
    if (!ResetEvent())
      return FALSE;

    Offset = (DWORD) dwStart;
    OffsetHigh = (DWORD) (dwStart >> 32);
    DWORD dw;
    return ReadFile(hFile, lpBuf, dwLength, &dw, this);
  }

  BOOL Write(HANDLE hFile, LPCVOID lpBuf, DWORD dwLength,
	     DWORDLONG dwStart = 0)
  {
    if (!ResetEvent())
      return FALSE;

    Offset = (DWORD) dwStart;
    OffsetHigh = (DWORD) (dwStart >> 32);
    DWORD dw;
    return WriteFile(hFile, lpBuf, dwLength, &dw, this);
  }

  DWORD BufRecv(HANDLE hFile, PVOID pBuf, DWORD dwBufSize)
  {
    DWORD dwDone = 0;
    bool bGood = true;

    for (PVOID ptr = pBuf; dwDone < dwBufSize; )
      {
	if (!Read(hFile, ptr, dwBufSize-dwDone))
	  if (GetLastError() != ERROR_IO_PENDING)
	    {
	      bGood = false;
	      break;
	    }

	DWORD dwReadLen;
	if (!GetResult(hFile, &dwReadLen))
	  {
	    bGood = false;
	    break;
	  }

	if (dwReadLen == 0)
	  break;

	dwDone += dwReadLen;
	(*(LPBYTE*) &ptr) += dwReadLen;
      }

    if (bGood & (dwDone != dwBufSize))
      SetLastError(ERROR_HANDLE_EOF);

    return dwDone;
  }

  BOOL BufSend(HANDLE hFile, const void *pBuf, DWORD dwBufSize)
  {
    DWORD dwDone = 0;
    for (const void *ptr = pBuf; dwDone < dwBufSize; )
      {
	if (!Write(hFile, ptr, dwBufSize-dwDone))
	  if (GetLastError() != ERROR_IO_PENDING)
	    break;

	DWORD dwWriteLen;
	if (!GetResult(hFile, &dwWriteLen))
	  break;

	if (dwWriteLen == 0)
	  break;

	dwDone += dwWriteLen;
	*(CONST BYTE**) &ptr += dwWriteLen;
      }

    return dwDone == dwBufSize;
  }

  BOOL ConnectNamedPipe(HANDLE hNamedPipe)
  {
    return ::ConnectNamedPipe(hNamedPipe, this);
  }

  BOOL WaitCommEvent(HANDLE hFile, LPDWORD lpEvtMask)
  {
    return ::WaitCommEvent(hFile, lpEvtMask, this);
  }

  BOOL GetResult(HANDLE hFile, LPDWORD lpNumberOfBytesTransferred,
		 BOOL bWait = TRUE)
  {
    return GetOverlappedResult(hFile, this, lpNumberOfBytesTransferred, bWait);
  }

  bool Wait(DWORD dwTimeout = INFINITE)
  {
    return WaitForSingleObject(hEvent, dwTimeout) == WAIT_OBJECT_0;
  }

  bool IsComplete()
  {
    return WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0;
  }

  BOOL SetEvent()
  {
    return ::SetEvent(hEvent);
  }

  BOOL ResetEvent()
  {
    return ::ResetEvent(hEvent);
  }

  BOOL PulseEvent()
  {
    return ::PulseEvent(hEvent);
  }

  operator bool() const
  {
    return hEvent != NULL;
  }

  bool operator!() const
  {
    return hEvent == NULL;
  }

  explicit WOverlapped(OVERLAPPED &ol)
  {
    *(OVERLAPPED*)this = ol;
  }

  explicit WOverlapped(BOOL bManualReset = true, BOOL bSignalled = false)
  {
    ZeroMemory(this, sizeof *this);
    hEvent = CreateEvent(NULL, bManualReset, bSignalled, NULL);
  }

  explicit WOverlapped(LPCTSTR lpName)
  {
    ZeroMemory(this, sizeof *this);
    hEvent = OpenEvent(EVENT_ALL_ACCESS, false, lpName);
  }

  ~WOverlapped()
  {
    if (hEvent != NULL)
      CloseHandle(hEvent);
  }
};

#else  // __cplusplus

#endif // _WIO_HPP
