/*++

Copyright (c) Microsoft Corporation, 1992 -

Module Name:

    lsalookupi.h

Abstract:

    LSA Policy Lookup internal API

--*/

#ifndef _LSALOOKUPI_
#define _LSALOOKUPI_

#ifdef MIDL_PASS
#define SIZE_IS(x)          [size_is(x)]
#define SWITCH_IS(x)        [switch_is(x)]
#define SWITCH_TYPE(x)      [switch_type(x)]
#define CASE(x)             [case(x)]
#define RANGE(x,y)          [range(x,y)]
#define VAR_SIZE_ARRAY
#define SID_POINTER         PISID
#define REF                 [ref]
#else
#define SIZE_IS(x)
#define SWITCH_IS(x)
#define SWITCH_TYPE(x)
#define CASE(x)
#define RANGE(x,y)          __in_range(x,y)
#define VAR_SIZE_ARRAY      (1)
#define SID_POINTER         PSID
#define REF
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// Generic negative values for unknown IDs, inapplicable indices etc.
//

#define LSA_UNKNOWN_ID      ((ULONG) 0xFFFFFFFFL)
#define LSA_UNKNOWN_INDEX   ((LONG) -1)

typedef enum _LSA_SID_NAME_MAPPING_OPERATION_TYPE {

    LsaSidNameMappingOperation_Add,
    LsaSidNameMappingOperation_Remove,
    LsaSidNameMappingOperation_AddMultiple,

} LSA_SID_NAME_MAPPING_OPERATION_TYPE, *PLSA_SID_NAME_MAPPING_OPERATION_TYPE;

#define LSA_MAXIMUM_NUMBER_OF_CHARACTERS_IN_NAMES_FOR_SNMC  0x400

typedef struct _LSA_SID_NAME_MAPPING_OPERATION_ADD_INPUT {

    UNICODE_STRING      DomainName;
    UNICODE_STRING      AccountName;
    REF SID_POINTER     Sid;
    ULONG               Flags;

} LSA_SID_NAME_MAPPING_OPERATION_ADD_INPUT, *PLSA_SID_NAME_MAPPING_OPERATION_ADD_INPUT;

typedef struct _LSA_SID_NAME_MAPPING_OPERATION_REMOVE_INPUT {

    UNICODE_STRING  DomainName;
    UNICODE_STRING  AccountName;

} LSA_SID_NAME_MAPPING_OPERATION_REMOVE_INPUT, *PLSA_SID_NAME_MAPPING_OPERATION_REMOVE_INPUT;

#define LSA_MAXIMUM_NUMBER_OF_MAPPINGS_IN_ADD_MULTIPLE_INPUT    0x1000

typedef struct _LSA_SID_NAME_MAPPING_OPERATION_ADD_MULTIPLE_INPUT {

    RANGE( 1, LSA_MAXIMUM_NUMBER_OF_MAPPINGS_IN_ADD_MULTIPLE_INPUT )
                            ULONG                                       Count;
    REF SIZE_IS( Count )    PLSA_SID_NAME_MAPPING_OPERATION_ADD_INPUT   Mappings;

} LSA_SID_NAME_MAPPING_OPERATION_ADD_MULTIPLE_INPUT, *PLSA_SID_NAME_MAPPING_OPERATION_ADD_MULTIPLE_INPUT;

typedef SWITCH_TYPE( LSA_SID_NAME_MAPPING_OPERATION_TYPE ) union _LSA_SID_NAME_MAPPING_OPERATION_INPUT {

    CASE( LsaSidNameMappingOperation_Add )
        LSA_SID_NAME_MAPPING_OPERATION_ADD_INPUT            AddInput;
    CASE( LsaSidNameMappingOperation_Remove )
        LSA_SID_NAME_MAPPING_OPERATION_REMOVE_INPUT         RemoveInput;
    CASE( LsaSidNameMappingOperation_AddMultiple )
        LSA_SID_NAME_MAPPING_OPERATION_ADD_MULTIPLE_INPUT   AddMultipleInput;

} LSA_SID_NAME_MAPPING_OPERATION_INPUT, *PLSA_SID_NAME_MAPPING_OPERATION_INPUT;

typedef enum _LSA_SID_NAME_MAPPING_OPERATION_ERROR {

    LsaSidNameMappingOperation_Success,
    LsaSidNameMappingOperation_NonMappingError,
    LsaSidNameMappingOperation_NameCollision,
    LsaSidNameMappingOperation_SidCollision,
    LsaSidNameMappingOperation_DomainNotFound,
    LsaSidNameMappingOperation_DomainSidPrefixMismatch,
    LsaSidNameMappingOperation_MappingNotFound,

} LSA_SID_NAME_MAPPING_OPERATION_ERROR, *PLSA_SID_NAME_MAPPING_OPERATION_ERROR;

typedef struct _LSA_SID_NAME_MAPPING_OPERATION_GENERIC_OUTPUT {

    LSA_SID_NAME_MAPPING_OPERATION_ERROR    ErrorCode;

} LSA_SID_NAME_MAPPING_OPERATION_GENERIC_OUTPUT, *PLSA_SID_NAME_MAPPING_OPERATION_GENERIC_OUTPUT;

typedef LSA_SID_NAME_MAPPING_OPERATION_GENERIC_OUTPUT LSA_SID_NAME_MAPPING_OPERATION_ADD_OUTPUT, *PLSA_SID_NAME_MAPPING_OPERATION_ADD_OUTPUT;
typedef LSA_SID_NAME_MAPPING_OPERATION_GENERIC_OUTPUT LSA_SID_NAME_MAPPING_OPERATION_REMOVE_OUTPUT, *PLSA_SID_NAME_MAPPING_OPERATION_REMOVE_OUTPUT;
typedef LSA_SID_NAME_MAPPING_OPERATION_GENERIC_OUTPUT LSA_SID_NAME_MAPPING_OPERATION_ADD_MULTIPLE_OUTPUT, *PLSA_SID_NAME_MAPPING_OPERATION_ADD_MULTIPLE_OUTPUT;

typedef SWITCH_TYPE( LSA_SID_NAME_MAPPING_OPERATION_TYPE ) union _LSA_SID_NAME_MAPPING_OPERATION_OUTPUT {

    CASE( LsaSidNameMappingOperation_Add )
        LSA_SID_NAME_MAPPING_OPERATION_ADD_OUTPUT           AddOutput;
    CASE( LsaSidNameMappingOperation_Remove )
        LSA_SID_NAME_MAPPING_OPERATION_REMOVE_OUTPUT        RemoveOutput;
    CASE( LsaSidNameMappingOperation_AddMultiple )
        LSA_SID_NAME_MAPPING_OPERATION_ADD_MULTIPLE_OUTPUT  AddMultipleOutput;

} LSA_SID_NAME_MAPPING_OPERATION_OUTPUT, *PLSA_SID_NAME_MAPPING_OPERATION_OUTPUT;

NTSTATUS
LsaLookupManageSidNameMapping(
    __in    LSA_SID_NAME_MAPPING_OPERATION_TYPE     OperationType,
    __in    PLSA_SID_NAME_MAPPING_OPERATION_INPUT   OperationInput,
    __out   PLSA_SID_NAME_MAPPING_OPERATION_OUTPUT  *OperationOutput
    );

#ifdef __cplusplus
}
#endif

#undef SIZE_IS
#undef SWITCH_IS
#undef SWITCH_TYPE
#undef CASE
#undef RANGE
#undef VAR_SIZE_ARRAY
#undef SID_POINTER
#undef REF

#endif // _LSALOOKUPI_

