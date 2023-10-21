///
/// Some additional native user-mode API functions we use
///

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)

typedef LONG NTSTATUS;
typedef NTSTATUS *PNTSTATUS;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };

    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define OBJ_CASE_INSENSITIVE    0x00000040L

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES);	      \
    (p)->RootDirectory = r;			      \
    (p)->Attributes = a;			      \
    (p)->ObjectName = n;			      \
    (p)->SecurityDescriptor = s;		      \
    (p)->SecurityQualityOfService = NULL;	      \
} 

#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040

#define SYMLINK_FLAG_RELATIVE   1

typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG Flags;
            WCHAR PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR  DataBuffer[1];
        } GenericReparseBuffer;
    } DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#define REPARSE_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)


#define REPARSE_GUID_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_GUID_DATA_BUFFER, GenericReparseBuffer)

//
// Maximum allowed size of the reparse data.
//

#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE      ( 16 * 1024 )

//
// Predefined reparse tags.
// These tags need to avoid conflicting with IO_REMOUNT defined in ntos\inc\io.h
//

#define IO_REPARSE_TAG_RESERVED_ZERO             (0)
#define IO_REPARSE_TAG_RESERVED_ONE              (1)

//
// The value of the following constant needs to satisfy the following conditions:
//  (1) Be at least as large as the largest of the reserved tags.
//  (2) Be strictly smaller than all the tags in use.
//

#define IO_REPARSE_TAG_RESERVED_RANGE            IO_REPARSE_TAG_RESERVED_ONE

//
// The reparse tags are a ULONG. The 32 bits are laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-+-+-+-+-----------------------+-------------------------------+
//  |M|R|N|R|     Reserved bits     |       Reparse Tag Value       |
//  +-+-+-+-+-----------------------+-------------------------------+
//
// M is the Microsoft bit. When set to 1, it denotes a tag owned by Microsoft.
//   All ISVs must use a tag with a 0 in this position.
//   Note: If a Microsoft tag is used by non-Microsoft software, the
//   behavior is not defined.
//
// R is reserved.  Must be zero for non-Microsoft tags.
//
// N is name surrogate. When set to 1, the file represents another named
//   entity in the system.
//
// The M and N bits are OR-able.
// The following macros check for the M and N bit values:
//

//
// Macro to determine whether a reparse point tag corresponds to a tag
// owned by Microsoft.
//

#define IsReparseTagMicrosoft(_tag) (              \
                           ((_tag) & 0x80000000)   \
                           )

//
// Macro to determine whether a reparse point tag is a name surrogate
//

#define IsReparseTagNameSurrogate(_tag) (          \
                           ((_tag) & 0x20000000)   \
                           )

// end_winnt

//
// The following constant represents the bits that are valid to use in
// reparse tags.
//

#define IO_REPARSE_TAG_VALID_VALUES     (0xF000FFFF)

//
// Macro to determine whether a reparse tag is a valid tag.
//

#define IsReparseTagValid(_tag) (                               \
                  !((_tag) & ~IO_REPARSE_TAG_VALID_VALUES) &&   \
                  ((_tag) > IO_REPARSE_TAG_RESERVED_RANGE)      \
                 )

///////////////////////////////////////////////////////////////////////////////
//
// Microsoft tags for reparse points.
//
///////////////////////////////////////////////////////////////////////////////

#define IO_REPARSE_TAG_MOUNT_POINT              (0xA0000003L)       // winnt
#define IO_REPARSE_TAG_HSM                      (0xC0000004L)       // winnt
#define IO_REPARSE_TAG_DRIVE_EXTENDER           (0x80000005L)
#define IO_REPARSE_TAG_HSM2                     (0x80000006L)       // winnt
#define IO_REPARSE_TAG_SIS                      (0x80000007L)       // winnt
#define IO_REPARSE_TAG_WIM                      (0x80000008L)       // winnt
#define IO_REPARSE_TAG_CSV                      (0x80000009L)       // winnt
#define IO_REPARSE_TAG_DFS                      (0x8000000AL)       // winnt
#define IO_REPARSE_TAG_FILTER_MANAGER           (0x8000000BL)
#define IO_REPARSE_TAG_SYMLINK                  (0xA000000CL)       // winnt
#define IO_REPARSE_TAG_IIS_CACHE                (0xA0000010L)
#define IO_REPARSE_TAG_DFSR                     (0x80000012L)       // winnt
#define IO_REPARSE_TAG_DEDUP                    (0x80000013L)       // winnt
#define IO_REPARSE_TAG_APPXSTRM                 (0xC0000014L)
#define IO_REPARSE_TAG_NFS                      (0x80000014L)       // winnt
#define IO_REPARSE_TAG_FILE_PLACEHOLDER         (0x80000015L)       // winnt
#define IO_REPARSE_TAG_DFM                      (0x80000016L)

///////////////////////////////////////////////////////////////////////////////
//
// Non-Microsoft tags for reparse points
//
///////////////////////////////////////////////////////////////////////////////

//
// Tag allocated to CONGRUENT, May 2000. Used by IFSTEST
//

#define IO_REPARSE_TAG_IFSTEST_CONGRUENT        (0x00000009L)

//
//  Tag allocated to Moonwalk Universal for HSM
//  GUID: 257ABE42-5A28-4C8C-AC46-8FEA5619F18F
//

#define IO_REPARSE_TAG_MOONWALK_HSM             (0x0000000AL)

//
//  Tag allocated to Tsinghua University for Research purposes
//  No released products should use this tag
//  GUID: b86dff51-a31e-4bac-b3cf-e8cfe75c9fc2
//

#define IO_REPARSE_TAG_TSINGHUA_UNIVERSITY_RESEARCH (0x0000000BL)

//
// Tag allocated to ARKIVIO for HSM
//

#define IO_REPARSE_TAG_ARKIVIO                  (0x0000000CL)

//
//  Tag allocated to SOLUTIONSOFT for name surrogate
//

#define IO_REPARSE_TAG_SOLUTIONSOFT             (0x2000000DL)

//
//  Tag allocated to COMMVAULT for HSM
//

#define IO_REPARSE_TAG_COMMVAULT                (0x0000000EL)

//
//  Tag allocated to Overtone Software for HSM
//

#define IO_REPARSE_TAG_OVERTONE                 (0x0000000FL)

//
//  Tag allocated to Symantec (formerly to KVS Inc) for HSM
//  GUID: A49F7BF6-77CA-493c-A0AA-18DBB28D1098
//

#define IO_REPARSE_TAG_SYMANTEC_HSM2            (0x00000010L)

//
//  Tag allocated to Enigma Data for HSM
//

#define IO_REPARSE_TAG_ENIGMA_HSM               (0x00000011L)

//
//  Tag allocated to Symantec for HSM
//  GUID: B99F4235-CF1C-48dd-9E6C-459FA289F8C7
//

#define IO_REPARSE_TAG_SYMANTEC_HSM             (0x00000012L)

//
//  Tag allocated to INTERCOPE for HSM
//  GUID: F204BE2D-AEEB-4728-A31C-C7F4E9BEA758}
//

#define IO_REPARSE_TAG_INTERCOPE_HSM            (0x00000013L)

//
//  Tag allocated to KOM Networks for HSM
//

#define IO_REPARSE_TAG_KOM_NETWORKS_HSM         (0x00000014L)

//
//  Tag allocated to MEMORY_TECH for HSM
//  GUID: E51BA456-046C-43ea-AEC7-DC1A87E1FD49
//

#define IO_REPARSE_TAG_MEMORY_TECH_HSM          (0x00000015L)

//
//  Tag allocated to BridgeHead Software for HSM
//  GUID: EBAFF6E3-F21D-4496-8342-58144B3D2BD0
//

#define IO_REPARSE_TAG_BRIDGEHEAD_HSM           (0x00000016L)

//
//  Tag allocated to OSR for samples reparse point filter
//  GUID: 3740c860-b19b-11d9-9669-0800200c9a66
//

#define IO_REPARSE_TAG_OSR_SAMPLE               (0x20000017L)

//
//  Tag allocated to Global 360 for HSM
//  GUID: C4B51F66-7F00-4c55-9441-8A1B159F209B
//

#define IO_REPARSE_TAG_GLOBAL360_HSM            (0x00000018L)

//
//  Tag allocated to Altiris for HSM
//  GUID: fc1047eb-fb2d-45f2-a2f4-a71c1032fa2dB
//

#define IO_REPARSE_TAG_ALTIRIS_HSM              (0x00000019L)

//
//  Tag allocated to Hermes for HSM
//  GUID: 437E0FD5-FCB4-42fe-877A-C785DA662AC2
//

#define IO_REPARSE_TAG_HERMES_HSM               (0x0000001AL)

//
//  Tag allocated to PointSoft for HSM
//  GUID: 547BC7FD-9604-4deb-AE07-B6514DF5FBC6
//

#define IO_REPARSE_TAG_POINTSOFT_HSM            (0x0000001BL)

//
//  Tag allocated to GRAU Data Storage for HSM
//  GUID: 6662D310-5653-4D10-8C31-F8E166D1A1BD
//

#define IO_REPARSE_TAG_GRAU_DATASTORAGE_HSM     (0x0000001CL)

//
//  Tag allocated to CommVault for HSM
//  GUID: cc38adf3-c583-4efa-b183-72c1671941de
//

#define IO_REPARSE_TAG_COMMVAULT_HSM            (0x0000001DL)


//
//  Tag allocated to Data Storage Group for single instance storage
//  GUID: C1182673-0562-447a-8E40-4F0549FDF817
//

#define IO_REPARSE_TAG_DATASTOR_SIS             (0x0000001EL)


//
//  Tag allocated to Enterprise Data Solutions, Inc. for HSM
//  GUID: EB63DF9D-8874-41cd-999A-A197542CDAFC
//

#define IO_REPARSE_TAG_EDSI_HSM                 (0x0000001FL)


//
//  Tag allocated to HP StorageWorks Reference Information Manager for Files (HSM)
//  GUID: 3B0F6B23-0C2E-4281-9C19-C6AEEBC88CD8
//

#define IO_REPARSE_TAG_HP_HSM                   (0x00000020L)


//
//  Tag allocated to SER Beteiligung Solutions Deutschland GmbH (HSM)
//  GUID: 55B673F0-978E-41c5-9ADB-AF99640BE90E
//

#define IO_REPARSE_TAG_SER_HSM                  (0x00000021L)


//
//  Tag allocated to Double-Take Software (formerly NSI Software, Inc.) for HSM
//  GUID: f7cb0ce8-453a-4ae1-9c56-db41b55f6ed4
//

#define IO_REPARSE_TAG_DOUBLE_TAKE_HSM          (0x00000022L)


//
//  Tag allocated to Beijing Wisdata Systems CO, LTD for HSM
//  GUID: d546500a-2aeb-45f6-9482-f4b1799c3177
//

#define IO_REPARSE_TAG_WISDATA_HSM              (0x00000023L)


//
//  Tag allocated to Mimosa Systems Inc for HSM
//  GUID: 8ddd4144-1a22-404b-8a5a-fcd91c6ee9f3
//

#define IO_REPARSE_TAG_MIMOSA_HSM               (0x00000024L)


//
//  Tag allocated to H&S Heilig und Schubert Software AG for HSM
//  GUID: 77CA30C0-E5EC-43df-9E44-A4910378E284
//

#define IO_REPARSE_TAG_HSAG_HSM                 (0x00000025L)


//
//  Tag allocated to Atempo Inc. (Atempo Digital Archive)  for HSM
//  GUID: 9B64518A-D6A4-495f-8D01-392F38862F0C
//

#define IO_REPARSE_TAG_ADA_HSM                  (0x00000026L)


//
//  Tag allocated to Autonomy Corporation for HSM
//  GUID: EB112A57-10FC-4b42-B590-A61897FDC432
//

#define IO_REPARSE_TAG_AUTN_HSM                 (0x00000027L)


//
//  Tag allocated to Nexsan for HSM
//  GUID: d35eba9a-e722-445d-865f-dde1120acf16
//

#define IO_REPARSE_TAG_NEXSAN_HSM               (0x00000028L)


//
//  Tag allocated to Double-Take for SIS
//  GUID: BDA506C2-F74D-4495-9A8D-44FD8D5B4F42
//

#define IO_REPARSE_TAG_DOUBLE_TAKE_SIS          (0x00000029L)


//
//  Tag allocated to Sony for HSM
//  GUID: E95032E4-FD81-4e15-A8E2-A1F078061C4E
//

#define IO_REPARSE_TAG_SONY_HSM                 (0x0000002AL)


//
//  Tag allocated to Eltan Comm for HSM
//  GUID: E1596D9F-44D8-43f4-A2D6-E9FE8D3E28FB
//

#define IO_REPARSE_TAG_ELTAN_HSM                (0x0000002BL)


//
//  Tag allocated to Utixo LLC for HSM
//  GUID: 5401F960-2F95-46D0-BBA6-052929FE2C32
//

#define IO_REPARSE_TAG_UTIXO_HSM                (0x0000002CL)


//
//  Tag allocated to Quest Software for HSM
//  GUID: D546500A-2AEB-45F6-9482-F4B1799C3177
//

#define IO_REPARSE_TAG_QUEST_HSM                (0x0000002DL)


//
//  Tag allocated to DataGlobal GmbH for HSM
//  GUID: 7A09CA83-B7B1-4614-ADFD-0BD5F4F989C9
//

#define IO_REPARSE_TAG_DATAGLOBAL_HSM           (0x0000002EL)


//
//  Tag allocated to Qi Tech LLC for HSM
//  GUID: C8110B39-A4CE-432E-B58A-FBEAD296DF03
//

#define IO_REPARSE_TAG_QI_TECH_HSM              (0x2000002FL)

//
//  Tag allocated to DataFirst Corporation for HSM
//  GUID: E0E40591-6434-479f-94AC-DECF6DAEFB5C
//

#define IO_REPARSE_TAG_DATAFIRST_HSM            (0x00000030L)

//
//  Tag allocated to C2C Systems for HSM
//  GUID: 6F2F829C-36AE-4E88-A3B6-E2C24377EA1C
//

#define IO_REPARSE_TAG_C2CSYSTEMS_HSM           (0x00000031L)

//
//  Tag allocated to Waterford Technologies for deduplication
//  GUID: 0AF8B999-B8E8-408b-805F-5448E68F9274
//

#define IO_REPARSE_TAG_WATERFORD                (0x00000032L)

//
//  Tag allocated to Riverbed Technology for HSM
//  GUID: 3336274-255B-4038-9D39-14B0EC3F8256
//

#define IO_REPARSE_TAG_RIVERBED_HSM             (0x00000033L)

//
//  Tag allocated to Caringo, Inc.  for HSM
//  GUID: B92426FA-D35F-48DB-A452-8FD557A23353
//

#define IO_REPARSE_TAG_CARINGO_HSM              (0x00000034L)

//
//  Tag allocated to MaxiScale, Inc. for HSM
//  GUID: 643B4714-BA13-427b-B771-C5BFDE787BB7
//

#define IO_REPARSE_TAG_MAXISCALE_HSM            (0x20000035L)

//
//  Tag allocated to Citrix Systems for profile management
//  GUID: B9150EDE-5845-4818-841B-5BCBB3B848E3
//

#define IO_REPARSE_TAG_CITRIX_PM                (0x00000036L)

//
//  Tag allocated to OpenAFS for DFS
//  GUID: EF21A155-5C92-4470-AB3B-370403D96369
//

#define IO_REPARSE_TAG_OPENAFS_DFS              (0x00000037L)

//
//  Tag allocated to ZL Technologies Inc for HSM
//  GUID: A521FE7A-EB10-4148-BAC7-264359827B7E
//

#define IO_REPARSE_TAG_ZLTI_HSM                 (0x00000038L)

//
//  Tag allocated to EMC Corporation for HSM
//  GUID: 119EA2B9-8979-48b9-B4CE-5082AF2D81E5
//

#define IO_REPARSE_TAG_EMC_HSM                  (0x00000039L)

//
//  Tag allocated to VMware for profile management
//  GUID: 6D020A57-C9BB-4DA4-A43F-49686D8D5E77
//

#define IO_REPARSE_TAG_VMWARE_PM                (0x0000003AL)

//
//  Tag allocated to Arco Computer Products for backup
//  GUID: C933F72B-A64D-44d9-8CD9-F339D12390CC
//

#define IO_REPARSE_TAG_ARCO_BACKUP              (0x0000003BL)

//
//  Tag allocated to Carroll-Net for HSM
//  GUID: 805EB191-564B-415a-A78C-9ED0AF8E02FF
//

#define IO_REPARSE_TAG_CARROLL_HSM              (0x0000003CL)

//
//  Tag allocated to ComTrade for HSM
//  GUID: D546500A-2AEB-45F6-9482-F4B1799C3177
//

#define IO_REPARSE_TAG_COMTRADE_HSM             (0x0000003DL)

//
//  Tag allocated to EaseVault for HSM
//  GUID: BBA65D6F-F8A0-48CC-B748-DBD5FFFCCFB1
//

#define IO_REPARSE_TAG_EASEVAULT_HSM            (0x0000003EL)

//
//  Tag allocated to Hitachi Data Systems for HSM
//  GUID: DC095FD2-AC3C-46BA-9E58-DD182BE86AF4
//

#define IO_REPARSE_TAG_HDS_HSM                  (0x0000003FL)


//
//  Reparse point index keys.
//
//  The index with all the reparse points that exist in a volume at a
//  given time contains entries with keys of the form
//                        <reparse tag, file record id>.
//  The data part of these records is empty.
//

#pragma pack(4)

typedef struct _REPARSE_INDEX_KEY {

    //
    //  The tag of the reparse point.
    //

    ULONG FileReparseTag;

    //
    //  The file record Id where the reparse point is set.
    //

    LARGE_INTEGER FileId;

} REPARSE_INDEX_KEY, *PREPARSE_INDEX_KEY;

#pragma pack()

NTSYSAPI
BOOLEAN 
NTAPI 
RtlDosPathNameToNtPathName_U(IN PCWSTR DosName,
			     OUT PUNICODE_STRING NtName,
			     OUT PCWSTR *DosFilePath OPTIONAL,
			     OUT PUNICODE_STRING NtFilePath OPTIONAL);

NTSYSAPI 
BOOLEAN 
NTAPI 
RtlCreateUnicodeString(OUT PUNICODE_STRING DestinationString,
		       IN PCWSTR SourceString);

NTSYSAPI 
VOID
NTAPI
RtlFreeUnicodeString(IN PUNICODE_STRING UnicodeString);

NTSYSAPI 
VOID
NTAPI
RtlInitUnicodeString(IN OUT PUNICODE_STRING DestinationString,
		     IN PCWSTR SourceString);

NTSYSAPI 
NTSTATUS
NTAPI
NtOpenFile(OUT PHANDLE  FileHandle,
	   IN ACCESS_MASK  DesiredAccess,
	   IN POBJECT_ATTRIBUTES  ObjectAttributes,
	   OUT PIO_STATUS_BLOCK  IoStatusBlock,
	   IN ULONG  ShareAccess,
	   IN ULONG  OpenOptions);

NTSYSAPI 
BOOLEAN 
NTAPI 
NtClose(IN HANDLE hObject);

NTSYSAPI 
ULONG
NTAPI 
RtlNtStatusToDosError(NTSTATUS Status);

NTSYSAPI
NTSTATUS
WINAPI
RtlGetVersion(
__inout __deref
POSVERSIONINFOW lpVersionInformation);

#ifdef __cplusplus
}
#endif
