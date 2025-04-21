#include "stdafx.h"
#include "WinHelper.h"

#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtWin>
#else
#include <windows.h>
#endif

#include <Shlwapi.h>
#include <Shlobj.h>


QVariantMap ResolveShortcut(const QString& LinkPath)
{
    QVariantMap Link;

    HRESULT hRes = E_FAIL;
    IShellLinkW* psl = NULL;

    // buffer that receives the null-terminated string
    // for the drive and path
    WCHAR szPath[0x1000];
    // structure that receives the information about the shortcut
    WIN32_FIND_DATAW wfd;

    // Get a pointer to the IShellLink interface
    hRes = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);

    if (SUCCEEDED(hRes))
    {
        // Get a pointer to the IPersistFile interface
        IPersistFile*  ppf     = NULL;
        psl->QueryInterface(IID_IPersistFile, (void **) &ppf);

        // Open the shortcut file and initialize it from its contents
        hRes = ppf->Load(LinkPath.toStdWString().c_str(), STGM_READ);
        if (SUCCEEDED(hRes))
        {
            hRes = psl->Resolve(NULL, SLR_NO_UI | SLR_NOSEARCH | SLR_NOUPDATE);
            if (SUCCEEDED(hRes))
            {
                // Get the path to the shortcut target
                hRes = psl->GetPath(szPath, ARRAYSIZE(szPath), &wfd, SLGP_RAWPATH);
                if (hRes == S_OK)
                    Link["Path"] = QString::fromWCharArray(szPath);
                else
                {
                    PIDLIST_ABSOLUTE pidl;
                    hRes = psl->GetIDList(&pidl);

                    if (SUCCEEDED(hRes)) 
                    {
                        LPWSTR url = nullptr;
                        SHGetNameFromIDList(pidl, SIGDN_URL, &url);
                    
                        if (url) 
                        {
                            QUrl Url = QString::fromWCharArray(url);

                            if (Url.isLocalFile())
                                Link["Path"] = Url.path().mid(1).replace("/", "\\");
                            else
                                Link["Path"] = Url.toString();
                        
                            CoTaskMemFree(url);
                        }

                        CoTaskMemFree(pidl);
                    }
                }

                hRes = psl->GetArguments(szPath, ARRAYSIZE(szPath));
                if (!FAILED(hRes))
                    Link["Arguments"] = QString::fromWCharArray(szPath);

                hRes = psl->GetWorkingDirectory(szPath, ARRAYSIZE(szPath));
                if (!FAILED(hRes))
				    Link["WorkingDir"] = QString::fromWCharArray(szPath);

				int IconIndex;
                hRes = psl->GetIconLocation(szPath, ARRAYSIZE(szPath), &IconIndex);
                if (FAILED(hRes))
                    return Link;
				Link["IconPath"] = QString::fromWCharArray(szPath);
				Link["IconIndex"] = IconIndex;

                // Get the description of the target
                hRes = psl->GetDescription(szPath, ARRAYSIZE(szPath));
                if (FAILED(hRes))
                    return Link;
                Link["Info"] = QString::fromWCharArray(szPath);
            }
        }
    }

    return Link;
}

QPixmap LoadWindowsIcon(const QString& Path, quint32 Index)
{
	std::wstring path = QString(Path).replace("/", "\\").toStdWString();
	HICON icon = ExtractIconW(NULL, path.c_str(), Index);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QPixmap Icon = QtWin::fromHICON(icon);
#else
	QPixmap Icon = QPixmap::fromImage(QImage::fromHICON(icon));
#endif
	DestroyIcon(icon);
	return Icon;
}

bool PickWindowsIcon(QWidget* pParent, QString& Path, quint32& Index)
{
	wchar_t iconPath[MAX_PATH] = { 0 };
	Path.toWCharArray(iconPath);
	BOOL Ret = PickIconDlg((HWND)pParent->window()->winId(), iconPath, MAX_PATH, (int*)&Index);
	Path = QString::fromWCharArray(iconPath);
	return !!Ret;
}

void ProtectWindow(void* hWnd)
{
    typedef BOOL(*LPSETWINDOWDISPLAYAFFINITY)(HWND, DWORD);
    static LPSETWINDOWDISPLAYAFFINITY pSetWindowDisplayAffinity = NULL;
    if (!pSetWindowDisplayAffinity)
        pSetWindowDisplayAffinity = (LPSETWINDOWDISPLAYAFFINITY)GetProcAddress(LoadLibraryA("user32.dll"), "SetWindowDisplayAffinity");
    if (pSetWindowDisplayAffinity)
        pSetWindowDisplayAffinity((HWND)hWnd, 0x00000011);
}

QString GetProductVersion(const QString &filePath) 
{
    QFileInfo check_file(filePath);

    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile()) {
        DWORD  verHandle = 0;
        UINT   size      = 0;
        LPBYTE lpBuffer  = NULL;
        DWORD  verSize   = GetFileVersionInfoSizeW(filePath.toStdWString().c_str(), &verHandle);

        if (verSize != NULL) {
            LPSTR verData = new char[verSize];

            if (GetFileVersionInfoW(filePath.toStdWString().c_str(), verHandle, verSize, verData)) {
                if (VerQueryValueW(verData, L"\\", (VOID FAR* FAR*)&lpBuffer, &size)) {
                    if (size) {
                        VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                        if (verInfo->dwSignature == 0xfeef04bd) {
                            // Doesn't matter if you are on 32 bit or 64 bit,
                            // DWORD is always 32 bits, so first two revision numbers
                            // come from dwFileVersionMS, last two come from dwFileVersionLS
                            QString Version = QString("%1.%2.%3")
                                .arg((verInfo->dwFileVersionMS >> 16) & 0xffff)
                                .arg((verInfo->dwFileVersionMS >>  0) & 0xffff)
                                .arg((verInfo->dwFileVersionLS >> 16) & 0xffff)
                                //.arg((verInfo->dwFileVersionLS >>  0) & 0xffff)
                                ;
                            DWORD Update = (verInfo->dwFileVersionLS >> 0) & 0xffff;
                            if (Update)
                                Version += QString("%1").arg(QChar((char)('a' + (Update - 1))));
                            return Version;
                        }
                    }
                }
            }
            delete[] verData;
        }
    }
    return QString();
}

bool KillProcessById(DWORD processId) 
{
    bool ok = false;
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess && hProcess != INVALID_HANDLE_VALUE) {
        if (TerminateProcess(hProcess, 0))
            ok = true;
        CloseHandle(hProcess);
    }
    return ok;
}

bool KillProcessByWnd(const QString& WndName)
{
    HWND hwnd = FindWindowW(WndName.toStdWString().c_str(), 0);
    if (hwnd) {
        DWORD processId;
        if (GetWindowThreadProcessId(hwnd, &processId))
            return KillProcessById(processId);
    }
    return false;
}

#include <netlistmgr.h>

bool CheckInternet()
{
    bool bRet = false;

    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        INetworkListManager* pNetworkListManager = nullptr;
        hr = CoCreateInstance(CLSID_NetworkListManager, NULL, CLSCTX_ALL, IID_INetworkListManager, (void**)&pNetworkListManager);
        if (SUCCEEDED(hr)) 
        {
            NLM_CONNECTIVITY connectivity = NLM_CONNECTIVITY_DISCONNECTED;
            hr = pNetworkListManager->GetConnectivity(&connectivity);
            if (SUCCEEDED(hr)) {
                if (connectivity & NLM_CONNECTIVITY_IPV4_INTERNET || connectivity & NLM_CONNECTIVITY_IPV6_INTERNET) {
                    bRet = true;
                }
            }

            pNetworkListManager->Release();
        }
        CoUninitialize();
    }

    return bRet;
}

QVariantList EnumNICs()
{
    QVariantList NICs;

    ULONG bufferSize = 0;
    GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &bufferSize);
    std::vector<byte> buffer;
    buffer.resize(bufferSize * 10 / 8);
    IP_ADAPTER_ADDRESSES* adapters = (IP_ADAPTER_ADDRESSES*)buffer.data();
    if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapters, &bufferSize) == NO_ERROR)
    {
        for (IP_ADAPTER_ADDRESSES* adapter = adapters; adapter != NULL; adapter = adapter->Next)
        {
            QVariantMap Data;
            Data["Adapter"] = QString::fromWCharArray(adapter->FriendlyName);
            Data["Device"] = QString::fromLatin1(adapter->AdapterName);
            Data["Index"] = (quint32)adapter->IfIndex;
            Data["MAC"] = QByteArray((char*)adapter->PhysicalAddress, adapter->PhysicalAddressLength);

            QStringList Ip4;
            QStringList Ip6;
            for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast != NULL; unicast = unicast->Next)
            {
                char addrStr[INET6_ADDRSTRLEN] = { 0 };

                if (unicast->Address.lpSockaddr->sa_family == AF_INET) {
                    struct sockaddr_in* sa4 = (struct sockaddr_in*)unicast->Address.lpSockaddr;
                    inet_ntop(AF_INET, &(sa4->sin_addr), addrStr, sizeof(addrStr));
                    Ip4.append(QString::fromLatin1(addrStr));
                }
                else if (unicast->Address.lpSockaddr->sa_family == AF_INET6) {
                    struct sockaddr_in6* sa6 = (struct sockaddr_in6*)unicast->Address.lpSockaddr;
                    inet_ntop(AF_INET6, &(sa6->sin6_addr), addrStr, sizeof(addrStr));
                    Ip6.append(QString::fromLatin1(addrStr));
                }
            }
            Data["Ip4"] = Ip4;
            Data["Ip6"] = Ip6;

            NICs.append(Data);
        }
    }

    return NICs;
}