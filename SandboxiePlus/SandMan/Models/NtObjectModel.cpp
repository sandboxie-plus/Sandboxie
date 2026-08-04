#include "stdafx.h"
#include "NtObjectModel.h"
#include "../../MiscHelpers/Common/Common.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;
#include <Windows.h>
#include <Winternl.h>


CNtObjectModel::CNtObjectModel(QObject *parent)
:CTreeItemModel(parent)
{
	m_Root = MkNode(QVariant());
	((SNtObjectNode*)m_Root)->ObjectPath = "\\";
	((SNtObjectNode*)m_Root)->ObjectType = "Directory";
	m_Root->Values.resize(columnCount());
	m_Root->Values[eName].Raw = "";
	m_Root->Values[eType].Raw = "Root";

	m_Icons.insert("ActivationObject", QPixmap(":/NtObjects/ACTIVATIONOBJECT.png"));
	m_Icons.insert("ActivityReference", QPixmap(":/NtObjects/ACTIVITYREFERENCE.png"));
	m_Icons.insert("Adapter", QPixmap(":/NtObjects/ADAPTER.png"));
	m_Icons.insert("ALPC Port", QPixmap(":/NtObjects/PORT.png"));
	m_Icons.insert("Callback", QPixmap(":/NtObjects/CALLBACK.png"));
	m_Icons.insert("Composition", QPixmap(":/NtObjects/COMPOSITION.png"));
	m_Icons.insert("Controller", QPixmap(":/NtObjects/CONTROLLER.png"));
	m_Icons.insert("CoreMessaging", QPixmap(":/NtObjects/COREMESSAGING.png"));
	m_Icons.insert("CoverageSampler", QPixmap(":/NtObjects/COVERAGESAMPLER.png"));
	m_Icons.insert("DebugObject", QPixmap(":/NtObjects/DEBUGOBJECT.png"));
	m_Icons.insert("Desktop", QPixmap(":/NtObjects/DESKTOP.png"));
	m_Icons.insert("Device", QPixmap(":/NtObjects/DEVICE.png"));
	m_Icons.insert("Directory", QPixmap(":/NtObjects/DIRECTORY.png"));
	m_Icons.insert("Folder", QPixmap(":/NtObjects/FOLDER.png"));
	m_Icons.insert("DmaAdapter", QPixmap(":/NtObjects/HALDMA.png"));
	m_Icons.insert("DmaDomain", QPixmap(":/NtObjects/HALDMA.png"));
	m_Icons.insert("Driver", QPixmap(":/NtObjects/DRIVER.png"));
	m_Icons.insert("DxgkCompositionObject", QPixmap(":/NtObjects/DXOBJECT.png"));
	m_Icons.insert("DxgkCurrentDxgProcessObject", QPixmap(":/NtObjects/DXOBJECT.png"));
	m_Icons.insert("DxgkCurrentDxgThreadObject", QPixmap(":/NtObjects/DXOBJECT.png"));
	m_Icons.insert("DxgkDisplayManagerObject", QPixmap(":/NtObjects/DXOBJECT.png"));
	m_Icons.insert("DxgkSharedBundleObject", QPixmap(":/NtObjects/DXOBJECT.png"));
	m_Icons.insert("DxgkSharedKeyedMutexObject", QPixmap(":/NtObjects/DXOBJECT.png"));
	m_Icons.insert("DxgkSharedProtectedSessionObject", QPixmap(":/NtObjects/DXOBJECT.png"));
	m_Icons.insert("DxgkSharedResource", QPixmap(":/NtObjects/DXOBJECT.png"));
	m_Icons.insert("DxgkSharedSwapChainObject", QPixmap(":/NtObjects/DXOBJECT.png"));
	m_Icons.insert("DxgkSharedSyncObject", QPixmap(":/NtObjects/DXOBJECT.png"));
	m_Icons.insert("EnergyTracker", QPixmap(":/NtObjects/ENERGYTRACKER.png"));
	m_Icons.insert("EtwConsumer", QPixmap(":/NtObjects/ETWCONSUMER.png"));
	m_Icons.insert("EtwRegistration", QPixmap(":/NtObjects/ETWREGISTRATION.png"));
	m_Icons.insert("EtwSessionDemuxEntry", QPixmap(":/NtObjects/ETWSESSIONDEMUXENTRY.png"));
	m_Icons.insert("Event", QPixmap(":/NtObjects/EVENT.png"));
	m_Icons.insert("EventPair", QPixmap(":/NtObjects/EVENTPAIR.png"));
	m_Icons.insert("File", QPixmap(":/NtObjects/FILE.png"));
	m_Icons.insert("FilterCommunicationPort", QPixmap(":/NtObjects/FLTCOMMPORT.png"));
	m_Icons.insert("FilterConnectionPort", QPixmap(":/NtObjects/FLTCONNPORT.png"));
	m_Icons.insert("IoCompletion", QPixmap(":/NtObjects/IOCOMPLETION.png"));
	m_Icons.insert("IoCompletionReserve", QPixmap(":/NtObjects/IOCOMPLETION_RESERVE.png"));
	m_Icons.insert("IRTimer", QPixmap(":/NtObjects/IRTIMER.png"));
	m_Icons.insert("Job", QPixmap(":/NtObjects/JOB.png"));
	m_Icons.insert("Key", QPixmap(":/NtObjects/KEY.png"));
	m_Icons.insert("KeyedEvent", QPixmap(":/NtObjects/KEYEDEVENT.png"));
	m_Icons.insert("Mutant", QPixmap(":/NtObjects/MUTANT.png"));
	m_Icons.insert("NdisCmState", QPixmap(":/NtObjects/NDISCMSTATE.png"));
	m_Icons.insert("Partition", QPixmap(":/NtObjects/MEMORYPARTITION.png"));
	m_Icons.insert("PcwObject", QPixmap(":/NtObjects/PCWOBJECT.png"));
	m_Icons.insert("Pipe", QPixmap(":/NtObjects/Pipe.png"));
	m_Icons.insert("PowerRequest", QPixmap(":/NtObjects/POWERREQUEST.png"));
	m_Icons.insert("Process", QPixmap(":/NtObjects/PROCESS.png"));
	m_Icons.insert("Profile", QPixmap(":/NtObjects/PROFILE.png"));
	m_Icons.insert("PsSiloContextNonPaged", QPixmap(":/NtObjects/PSSILOCONTEXT.png"));
	m_Icons.insert("PsSiloContextPaged", QPixmap(":/NtObjects/PSSILOCONTEXT.png"));
	m_Icons.insert("RawInputManager", QPixmap(":/NtObjects/RAWINPUTMANAGER.png"));
	m_Icons.insert("RegistryTransaction", QPixmap(":/NtObjects/KEY.png"));
	m_Icons.insert("Section", QPixmap(":/NtObjects/SECTION.png"));
	m_Icons.insert("Semaphore", QPixmap(":/NtObjects/SEMAPHORE.png"));
	m_Icons.insert("Session", QPixmap(":/NtObjects/SESSION.png"));
	m_Icons.insert("SymbolicLink", QPixmap(":/NtObjects/SYMLINK.png"));
	m_Icons.insert("Thread", QPixmap(":/NtObjects/THREAD.png"));
	m_Icons.insert("Timer", QPixmap(":/NtObjects/TIMER.png"));
	m_Icons.insert("TmEn", QPixmap(":/NtObjects/TMEN.png"));
	m_Icons.insert("TmRm", QPixmap(":/NtObjects/TMRM.png"));
	m_Icons.insert("TmTm", QPixmap(":/NtObjects/TMTM.png"));
	m_Icons.insert("TmTx", QPixmap(":/NtObjects/TMTX.png"));
	m_Icons.insert("Token", QPixmap(":/NtObjects/TOKEN.png"));
	m_Icons.insert("TpWorkerFactory", QPixmap(":/NtObjects/TPWORKERFACTORY.png"));
	m_Icons.insert("Type", QPixmap(":/NtObjects/TYPE.png"));
	m_Icons.insert("UserApcReserve", QPixmap(":/NtObjects/USERAPCRESERVE.png"));
	m_Icons.insert("Value", QPixmap(":/NtObjects/VALUE.png"));
	m_Icons.insert("VirtualKey", QPixmap(":/NtObjects/VIRTUALKEY.png"));
	m_Icons.insert("VRegConfigurationContext", QPixmap(":/NtObjects/VREGCFGCTX.png"));
	m_Icons.insert("WaitablePort", QPixmap(":/NtObjects/WAITABLEPORT.png"));
	m_Icons.insert("WaitCompletionPacket", QPixmap(":/NtObjects/WAITCOMPLETIONPACKET.png"));
	m_Icons.insert("WindowStation", QPixmap(":/NtObjects/WINSTATION.png"));
	m_Icons.insert("WmiGuid", QPixmap(":/NtObjects/WMIGUID.png"));
	m_DefaultIcon = QPixmap(":/NtObjects/UNKNOWN.png");
}

CNtObjectModel::~CNtObjectModel()
{
	FreeNode(m_Root);
	m_Root = NULL;
}

CNtObjectModel::SNtObjectNode* CNtObjectModel::GetNode(const QModelIndex &index) const
{
    if (index.column() > 0)
        return NULL;

	SNtObjectNode* pNode;
    if (!index.isValid())
        pNode = (SNtObjectNode*)m_Root;
    else
        pNode = static_cast<SNtObjectNode*>(index.internalPointer());
	return pNode;
}

extern "C" {

#define DIRECTORY_QUERY 0x0001

NTSYSCALLAPI NTSTATUS NTAPI NtOpenDirectoryObject(_Out_ PHANDLE DirectoryHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes);

typedef struct _OBJECT_DIRECTORY_INFORMATION
{
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

NTSYSCALLAPI NTSTATUS NTAPI NtQueryDirectoryObject(_In_ HANDLE DirectoryHandle, _Out_writes_bytes_opt_(Length) PVOID Buffer, _In_ ULONG Length,
    _In_ BOOLEAN ReturnSingleEntry, _In_ BOOLEAN RestartScan, _Inout_ PULONG Context, _Out_opt_ PULONG ReturnLength);

NTSYSCALLAPI NTSTATUS NTAPI NtQueryDirectoryFile(_In_ HANDLE FileHandle, _In_opt_ HANDLE Event, _In_opt_ PIO_APC_ROUTINE ApcRoutine, _In_opt_ PVOID ApcContext,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock, _Out_writes_bytes_(Length) PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass,
    _In_ BOOLEAN ReturnSingleEntry, _In_opt_ PUNICODE_STRING FileName, _In_ BOOLEAN RestartScan);

typedef struct _FILE_BOTH_DIR_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	ULONG EaSize;
	CCHAR ShortNameLength;
	WCHAR ShortName[12];
	WCHAR FileName[1];
} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION; 

#define SYMBOLIC_LINK_QUERY (0x0001)

NTSYSCALLAPI NTSTATUS NTAPI NtOpenSymbolicLinkObject(_Out_ PHANDLE LinkHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes);

NTSYSCALLAPI NTSTATUS NTAPI NtQuerySymbolicLinkObject(_In_ HANDLE LinkHandle, _Inout_ PUNICODE_STRING LinkTarget, _Out_opt_ PULONG ReturnedLength);


typedef struct _KEY_BASIC_INFORMATION {
  LARGE_INTEGER LastWriteTime;
  ULONG         TitleIndex;
  ULONG         NameLength;
  WCHAR         Name[1];
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef enum _KEY_INFORMATION_CLASS {
  KeyBasicInformation,
  KeyNodeInformation,
  KeyFullInformation,
  KeyNameInformation,
  KeyCachedInformation,
  KeyFlagsInformation,
  KeyVirtualizationInformation,
  KeyHandleTagsInformation,
  KeyTrustInformation,
  KeyLayerInformation,
  MaxKeyInfoClass
} KEY_INFORMATION_CLASS;

NTSYSCALLAPI NTSTATUS NTAPI NtOpenKey(_Out_ PHANDLE KeyHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes);
NTSYSCALLAPI NTSTATUS NTAPI NtQueryKey(_In_ HANDLE KeyHandle, _In_ KEY_INFORMATION_CLASS KeyInformationClass, 
	_Out_writes_bytes_opt_(Length) PVOID KeyInformation, _In_ ULONG Length, _Out_ PULONG ResultLength);
NTSYSCALLAPI NTSTATUS NTAPI NtEnumerateKey(_In_ HANDLE KeyHandle, _In_ ULONG Index, _In_ KEY_INFORMATION_CLASS KeyInformationClass,
    _Out_writes_bytes_opt_(Length) PVOID KeyInformation, _In_ ULONG Length, _Out_ PULONG ResultLength);

typedef struct _KEY_VALUE_FULL_INFORMATION {
  ULONG TitleIndex;
  ULONG Type;
  ULONG DataOffset;
  ULONG DataLength;
  ULONG NameLength;
  WCHAR Name[1];
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
  KeyValueBasicInformation,
  KeyValueFullInformation,
  KeyValuePartialInformation,
  KeyValueFullInformationAlign64,
  KeyValuePartialInformationAlign64,
  KeyValueLayerInformation,
  MaxKeyValueInfoClass
} KEY_VALUE_INFORMATION_CLASS;

NTSYSCALLAPI NTSTATUS NTAPI NtEnumerateValueKey(_In_ HANDLE KeyHandle, _In_ ULONG Index, _In_ KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    _Out_writes_bytes_opt_(Length) PVOID KeyValueInformation, _In_ ULONG Length, _Out_ PULONG ResultLength);

}

bool FilterObjects(const QString& ObjectPath) 
{
	QStringList FilterList;
	FilterList.append("\\REGISTRY");
	FilterList.append("\\Device\\HarddiskVolume");
	foreach(const QString & Filter, FilterList) {
		int len = min(Filter.length(), ObjectPath.length());
		if (ObjectPath.left(len).compare(Filter.left(len), Qt::CaseInsensitive) == 0)
			return true;
	}
	return false;
}

#define MATCH_FLAG_EXACT    0x01
#define MATCH_FLAG_AUX      0x02

int MatchPatternX(const SAccessRule& rule, const QString& Path)
{
	auto match = rule.Exp.match(Path);
	if (!match.hasMatch())
		return 0;
	int len = match.capturedLength();
	return len;
}

int MatchPathList(const QString& Path, const QList<SAccessRule>& list, ULONG* plevel, ULONG* pflags, USHORT* pwildc, SAccessRule& found)
{
    int match_len = 0;
    ULONG level = plevel ? *plevel : -1; // lower is better, 3 is max value
    ULONG flags = pflags ? *pflags : 0;
    USHORT wildc = pwildc ? *pwildc : -1; // lower is better

    foreach(const SAccessRule& rule, list)
	{
		ULONG cur_level = 3; // Pattern_Level(pat);
        if (cur_level > level)
            continue; // no point testing patterns with a to weak level

		BOOLEAN cur_exact = rule.Pattern.right(1) != "*";
        if (!cur_exact && (flags & MATCH_FLAG_EXACT))
            continue;

		USHORT cur_wildc = rule.Pattern.count("*");

        int cur_len = MatchPatternX(rule, Path);
        if (cur_len > match_len) {
            match_len = cur_len;
            level = cur_level;
            flags = cur_exact ? MATCH_FLAG_EXACT : 0;
            wildc = cur_wildc;
            found = rule;
            
            // we need to test all entries to find the best match, so we don't break here
            // unless we found an exact match, than there can't be a batter one
            if (cur_exact)
                break;
        }

        //
        // if we have a pattern like C:\Windows\,
        // we still want it to match a path like C:\Windows,
        // hence we add a L'\\' to the path and check again
        //

        else if (Path.right(1) != "\\") { 
            cur_len = MatchPatternX(rule, Path + "\\");
            if (cur_len > match_len) {
                match_len = cur_len;
                level = cur_level;
                flags = MATCH_FLAG_AUX | (cur_exact ? MATCH_FLAG_EXACT : 0);
                wildc = cur_wildc;
                found = rule;
            }
        }
    }

    if (plevel) *plevel = level;
    if (pflags) *pflags = flags;
    if (pwildc) *pwildc = wildc;
    return match_len;
}

bool MatchPathListEx(const QString& Path, const QList<SAccessRule>& list, ULONG* plevel, int* pmatch_len, ULONG* pflags, USHORT* pwildc, QString& patsrc)
{
    SAccessRule found;
    ULONG cur_level;
    ULONG cur_flags;
    USHORT cur_wildc;
    int cur_len;

    cur_level = *plevel;
    cur_flags = *pflags;
    cur_wildc = *pwildc;
    cur_len = MatchPathList(Path, list, &cur_level, &cur_flags, &cur_wildc, found);
    if (cur_level <= *plevel && (
        ((*pflags & MATCH_FLAG_EXACT) == 0 && (cur_flags & MATCH_FLAG_EXACT) != 0) || // an exact match overrules any non-exact match
        ((*pflags & MATCH_FLAG_AUX) != 0 && (cur_flags & MATCH_FLAG_AUX) == 0) || // a rule with a primary match overrules auxiliary matches
        (cur_len > *pmatch_len) || // the longer the match the more specific the rule, hence the highest priority it has
        ((cur_len == *pmatch_len && cur_len > 0) && (cur_wildc < *pwildc)) // given the same match length, a rule with less wildcards wins
        )) {
        *plevel = cur_level;
        *pflags = cur_flags;
        *pwildc = cur_wildc;
        *pmatch_len = cur_len;
		patsrc = found.Pattern;

        return true;
    }
    return false;
}

COptionsWindow::EAccessMode MatchPath(const QString& Path, const SAccessLists& RuleLists, bool RuleSpecificity)
{
	COptionsWindow::EAccessMode mp_flags = COptionsWindow::eNormal;

    QString patsrc;
    int match_len = 0;
    ULONG level = 3; 
	// 3 - global default - lower is better, 3 is max value
	// 2 - match all 
	// 1 - match by negation
	// 0 - exact match
    ULONG flags = 0;
    USHORT wildc = -1; // lower is better

    if (MatchPathListEx(Path, RuleLists.Rules.values(COptionsWindow::eClosed), &level, &match_len, &flags, &wildc, patsrc)) {
        mp_flags = COptionsWindow::eClosed;
        if (!RuleSpecificity) goto finish;
    }
    
    if (MatchPathListEx(Path, RuleLists.Rules.values(COptionsWindow::eBoxOnly), &level, &match_len, &flags, &wildc, patsrc)) {
        mp_flags = COptionsWindow::eBoxOnly;
        if (!RuleSpecificity) goto finish;
    }
    
    if (MatchPathListEx(Path, RuleLists.Rules.values(COptionsWindow::eReadOnly), &level, &match_len, &flags, &wildc, patsrc)) {
        mp_flags = COptionsWindow::eReadOnly;
        if (!RuleSpecificity) goto finish;
    }
    
    if (MatchPathListEx(Path, RuleLists.Rules.values(COptionsWindow::eNormal), &level, &match_len, &flags, &wildc, patsrc)) {
        mp_flags = COptionsWindow::eNormal;
        // don't goto finish as open can overwrite this 
    }

    if (MatchPathListEx(Path, RuleLists.Rules.values(COptionsWindow::eOpen), &level, &match_len, &flags, &wildc, patsrc)) {
        mp_flags = COptionsWindow::eOpen;
    }

finish:
	return mp_flags;
}

void CNtObjectModel::AddEntryToList(const QString& ObjectName, const QString &ObjectType, const QString& ObjectPath, QList<SNtObjectInfo>& list)
{
	QString ObjectPrefix = ObjectPath;
	if (ObjectPrefix != "\\")
		ObjectPrefix += "\\";
	QString FullPath = ObjectPrefix + ObjectName;
	//if (!FilterObjects(FullPath))
	//	return;

	COptionsWindow::EAccessType Type;
	if (ObjectType == "Key" || ObjectType == "Value")
		Type = COptionsWindow::eKey;
	else if (ObjectType == "Device" || ObjectType == "Folder" || ObjectType == "SymbolicLink" || ObjectType == "File" || ObjectType == "Pipe" || FullPath.left(7) == "\\Device")
		Type = COptionsWindow::eFile;
	else
		Type = COptionsWindow::eIPC;

	SNtObjectInfo NtObject;
	NtObject.Name = ObjectName;
	NtObject.Type = ObjectType;
	NtObject.Directive = MatchPath(FullPath, m_Rules.Lists[Type], m_Rules.RuleSpecificity);
	list.append(NtObject);
}

QList<SNtObjectInfo> CNtObjectModel::EnumDirectoryObjects(const QString& ObjectPath, const QString& ObjectType)
{
	QList<SNtObjectInfo> list;

	QString Path = ObjectPath;
	std::wstring Name = Path.replace("/", "\\").toStdWString();
	if (ObjectType == "Device" || ObjectType == "SymbolicLink")
		Name += L"\\";

	NTSTATUS status;
	UNICODE_STRING name;
	OBJECT_ATTRIBUTES oa;
	IO_STATUS_BLOCK iosb;

	RtlInitUnicodeString(&name, Name.c_str());
	InitializeObjectAttributes(&oa, &name, OBJ_CASE_INSENSITIVE, NULL, NULL);

	if (ObjectType == "Key")
	{
		HANDLE hKey;
		status = NtOpenKey(&hKey, MAXIMUM_ALLOWED, &oa);
		if (NT_SUCCESS(status))
		{
			ULONG Index, Length;
			const ULONG BufferSize = 0x1000;
			BYTE Buffer[BufferSize];
			PKEY_BASIC_INFORMATION KeyInformation = (PKEY_BASIC_INFORMATION)Buffer;
			PKEY_VALUE_FULL_INFORMATION KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buffer;

			//status = NtQueryKey(hKey, KeyBasicInformation, &KeyInformation[0], sizeof(KeyInformation), &Length);

			Index = 0;
			for (;;)
			{
				status = NtEnumerateKey(hKey, Index++, KeyBasicInformation, &KeyInformation[0], BufferSize, &Length);
				if (!NT_SUCCESS(status))
					break;

				AddEntryToList(QString::fromWCharArray(KeyInformation[0].Name, KeyInformation[0].NameLength / sizeof(wchar_t)), "Key", ObjectPath, list);
			}

			Index = 0;
			for (;;)
			{
				status = NtEnumerateValueKey(hKey, Index++, KeyValueFullInformation, &KeyValueInformation[0], BufferSize, &Length);
				if (!NT_SUCCESS(status))
					break;

				AddEntryToList(QString::fromWCharArray(KeyValueInformation[0].Name, KeyValueInformation[0].NameLength / sizeof(wchar_t)), "Value", ObjectPath, list);
			}

			NtClose(hKey);
		}
	}
	else if (ObjectType == "Device" || ObjectType == "Folder" || ObjectType == "SymbolicLink") 
	{
		bool bIsPipe = ObjectPath == "\\Device\\NamedPipe";

		HANDLE DirectoryHandle;
		status = NtCreateFile(&DirectoryHandle, GENERIC_READ | FILE_LIST_DIRECTORY, &oa, &iosb,
			0, FILE_ATTRIBUTE_DIRECTORY, FILE_SHARE_READ, FILE_OPEN_IF, FILE_DIRECTORY_FILE, 0, 0);
		if (NT_SUCCESS(status)) {

			PFILE_BOTH_DIR_INFORMATION DirInfo = (PFILE_BOTH_DIR_INFORMATION)malloc(0x10000);

			BOOL bReset = TRUE;
			status = NtQueryDirectoryFile(DirectoryHandle, NULL, NULL, NULL, &iosb, DirInfo, 0x10000, (FILE_INFORMATION_CLASS)3, FALSE, NULL, bReset);

			if (status == STATUS_PENDING) {
				status = NtWaitForSingleObject(DirectoryHandle, FALSE, NULL);
				if (NT_SUCCESS(status))
					status = iosb.Status;
			}

			if (NT_SUCCESS(status))
			{
				PFILE_BOTH_DIR_INFORMATION DirInformation = DirInfo;
				while (1)
				{
					QString Name = QString::fromWCharArray(DirInformation->FileName, DirInformation->FileNameLength / sizeof(wchar_t));
					QString Type;
					if (bIsPipe)
						Type = "Pipe";
					else if ((DirInformation->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
						Type = "Folder";
					else
						Type = "File";

					if (Name != "." && Name != "..")
						AddEntryToList(Name, Type, ObjectPath, list);

					if (DirInformation->NextEntryOffset == 0)
						break;
					DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
				}
			}

			free(DirInfo);

			NtClose(DirectoryHandle);
		}
	}
	else
	{
		HANDLE DirectoryHandle;
		status = NtOpenDirectoryObject(&DirectoryHandle, DIRECTORY_QUERY, &oa);
		if (NT_SUCCESS(status))
		{
			BOOLEAN firstTime = TRUE;
			ULONG indexCounter = 0;
			ULONG bufferSize = 0x1000;
			POBJECT_DIRECTORY_INFORMATION buffer = (POBJECT_DIRECTORY_INFORMATION)malloc(bufferSize);

			do
			{
				while ((status = NtQueryDirectoryObject(DirectoryHandle, buffer, bufferSize, FALSE, firstTime, &indexCounter, NULL)) == STATUS_MORE_ENTRIES)
				{
					if (buffer[0].Name.Buffer)
						break;

					free(buffer);
					bufferSize *= 2;
					buffer = (POBJECT_DIRECTORY_INFORMATION)malloc(bufferSize);
					if (!buffer) {
						status = STATUS_INSUFFICIENT_RESOURCES;
						break;
					}
				}

				if (NT_SUCCESS(status))
				{
					for (ULONG i = 0; ; i++)
					{
						POBJECT_DIRECTORY_INFORMATION info;
						info = &buffer[i];
						if (!info->Name.Buffer)
							break;
						AddEntryToList(QString::fromWCharArray(info->Name.Buffer), QString::fromWCharArray(info->TypeName.Buffer), ObjectPath, list);
					}
				}

				firstTime = FALSE;

			} while (status == STATUS_MORE_ENTRIES);

			if (buffer) free(buffer);

			NtClose(DirectoryHandle);
		}
	}

	return list;
}

void CNtObjectModel::FillNode(const struct SNtObjectInfo* pNtObject, SNtObjectNode* pChildNode)
{
	pChildNode->Values.resize(columnCount());

	pChildNode->Values[eName].Raw = pNtObject->Name;
	if (pNtObject->Type == "SymbolicLink")
	{
		std::wstring		path = pChildNode->ObjectPath.toStdWString();

		NTSTATUS			status;
		UNICODE_STRING		ObjectName;
		UNICODE_STRING		LinkTarget;
		OBJECT_ATTRIBUTES   ObjectAttributes;
		HANDLE				hLink;
		ULONG				bytesNeeded = 0x7FFF;

		RtlInitUnicodeString(&ObjectName, (wchar_t*)path.c_str());

		LinkTarget.Buffer = (PWSTR)malloc(bytesNeeded);
		LinkTarget.Length = 0;
		LinkTarget.MaximumLength = bytesNeeded;

		InitializeObjectAttributes(&ObjectAttributes, &ObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL);
		status = NtOpenSymbolicLinkObject(&hLink, SYMBOLIC_LINK_QUERY, &ObjectAttributes);
		if (NT_SUCCESS(status))
		{
			status = NtQuerySymbolicLinkObject(hLink, &LinkTarget, &bytesNeeded);

			NtClose(hLink);
		}

		if(NT_SUCCESS(status))
			pChildNode->Values[eType].Formatted = pNtObject->Type + QString(" (%1)").arg(QString::fromWCharArray(LinkTarget.Buffer, LinkTarget.Length / sizeof(wchar_t)));
		else
			pChildNode->Values[eType].Formatted = pNtObject->Type;

		free(LinkTarget.Buffer);
	}
	else if (pNtObject->Type == "Directory" || pNtObject->Type == "Folder" || pNtObject->Type == "Key")
	{
		pChildNode->Values[eType].Raw = QVariant(); // sort directories first
		pChildNode->Values[eType].Formatted = pNtObject->Type;
	}
	else if (pNtObject->Type == "Device" && (pNtObject->Name == "NamedPipe" || pNtObject->Name.indexOf("HarddiskVolume") == 0
		|| pNtObject->Name.indexOf("CdRom") == 0 || pNtObject->Name.indexOf("Floppy") == 0))
	{
		pChildNode->Values[eType].Raw = pNtObject->Type;
	}
	else
	{
		pChildNode->Values[eType].Raw = pNtObject->Type;
		pChildNode->State = -1;
	}
	pChildNode->Values[eDirective].Raw = pNtObject->Directive;
	pChildNode->Values[eDirective].Formatted = COptionsWindow::GetAccessModeStr(pNtObject->Directive);
	pChildNode->Icon = m_Icons.value(pNtObject->Type);
}

void CNtObjectModel::fetchMore(const QModelIndex &parent)
{
	SNtObjectNode* pNode = GetNode(parent);
	if (pNode->State != 0)
		return;

	QList<SNtObjectInfo> FoundObjects = EnumDirectoryObjects(pNode->ObjectPath, pNode->ObjectType);

	QMap<QList<QVariant>, QList<STreeNode*> > New;

	QString ObjectPrefix = pNode->ObjectPath;
	if (pNode->ObjectPath != "\\")
		ObjectPrefix += "\\";

	foreach(const SNtObjectInfo& NtObject, FoundObjects)
	{
		QString ObjectPath = ObjectPrefix + NtObject.Name;

		SNtObjectNode* pChildNode = static_cast<SNtObjectNode*>(MkNode(ObjectPath));
		pChildNode->ObjectPath = ObjectPath;
		pChildNode->ObjectType = NtObject.Type;
		if(pNode->Parent)
		{
			pChildNode->Path = pNode->Path;
			pChildNode->Path.append(pNode->ObjectPath);
		}
		//pChildNode->pNtObject = 
		New[pChildNode->Path].append(pChildNode);

		FillNode(&NtObject, pChildNode);
	}

	pNode->State = 1;

	if(!New.isEmpty())
	{
		beginInsertRows(parent, 0, FoundObjects.size()-1);
		for(QMap<QList<QVariant>, QList<STreeNode*> >::const_iterator I = New.begin(); I != New.end(); I++)
			Fill(m_Root, /*QModelIndex(),*/ I.key(), 0, I.value(), NULL);
		endInsertRows();
	}
}

void CNtObjectModel::Refresh()
{ 
	QMap<QList<QVariant>, QList<STreeNode*> > New;
	QHash<QVariant, STreeNode*> Old = m_Map;

	Refresh((SNtObjectNode*)m_Root, New, Old); 

	CTreeItemModel::Sync(New, Old);
}

void CNtObjectModel::Refresh(SNtObjectNode* pNode, QMap<QList<QVariant>, QList<STreeNode*> >& New, QHash<QVariant, STreeNode*>& Old)
{
	QList<SNtObjectInfo> FoundObjects = EnumDirectoryObjects(pNode->ObjectPath, pNode->ObjectType);

	QString ObjectPrefix = pNode->ObjectPath;
	if (pNode->ObjectPath != "\\")
		ObjectPrefix += "\\";

	foreach(const SNtObjectInfo& NtObject, FoundObjects)
	{
		QString ObjectPath = ObjectPrefix + NtObject.Name;

		//QModelIndex Index;
		
		SNtObjectNode* pChildNode = static_cast<SNtObjectNode*>(Old.value(ObjectPath));
		if(!pChildNode)
		{
			pChildNode = static_cast<SNtObjectNode*>(MkNode(ObjectPath));
			pChildNode->ObjectPath = ObjectPath;
			pChildNode->ObjectType = NtObject.Type;
			if(pNode->Parent)
			{
				pChildNode->Path = pNode->Path;
				pChildNode->Path.append(pNode->ObjectPath);
			}
			//pChildNode->pNtObject = 
			New[pChildNode->Path].append(pChildNode);

			FillNode(&NtObject, pChildNode);
		}
		else
		{
			Old[ObjectPath] = NULL;
			//Index = Find(m_Root, pChildNode);

			if (pChildNode->State == 1)
				Refresh(pChildNode, New, Old);
		}

		//if(Index.isValid()) // this is to slow, be more precise
		//	emit dataChanged(createIndex(Index.row(), 0, pChildNode), createIndex(Index.row(), columnCount()-1, pChildNode));
	}
}

bool CNtObjectModel::canFetchMore(const QModelIndex &parent) const
{
	SNtObjectNode* pNode = GetNode(parent);
	if (!pNode || pNode->State != 0)
		return false;
	return true;
}

bool CNtObjectModel::hasChildren(const QModelIndex &parent) const
{
	SNtObjectNode* pNode = GetNode(parent);
	if (pNode && pNode->State == 0)
		return true;
	return CTreeItemModel::hasChildren(parent);
}

int CNtObjectModel::columnCount(const QModelIndex &parent) const
{
	return eCount;
}

QVariant CNtObjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)
		{
			case eName:				return tr("Name");
			case eType:				return tr("Type");
			case eDirective:		return tr("Access");
		}
	}
    return QVariant();
}
