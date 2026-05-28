/*
 * Copyright (C) 2016 wj32
 * Copyright (C) 2021-2025 David Xanatos, xanasoft.com
 *
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "driver.h"
#include "util.h"

NTSTATUS NTAPI ZwQueryInstallUILanguage(LANGID* LanguageId);

#include "api_defs.h"
NTSTATUS Api_GetSecureParamImpl(const wchar_t* name, PVOID* data_ptr, ULONG* data_len, BOOLEAN verify);

#include <bcrypt.h>

#ifdef __BCRYPT_H__
#define KPH_SIGN_ALGORITHM BCRYPT_ECDSA_P256_ALGORITHM
#define KPH_SIGN_ALGORITHM_BITS 256
#define KPH_HASH_ALGORITHM BCRYPT_SHA256_ALGORITHM
#define KPH_BLOB_PUBLIC BCRYPT_ECCPUBLIC_BLOB
#endif

#define KPH_SIGNATURE_MAX_SIZE (128 * 1024) // 128 kB

#define FILE_BUFFER_SIZE (2 * PAGE_SIZE)
#define FILE_MAX_SIZE (128 * 1024 * 1024) // 128 MB

static UCHAR KphpTrustedPublicKey[] =
{
    0x45, 0x43, 0x53, 0x31, 0x20, 0x00, 0x00, 0x00, 0x05, 0x7A, 0x12, 0x5A, 0xF8, 0x54, 0x01, 0x42,
    0xDB, 0x19, 0x87, 0xFC, 0xC4, 0xE3, 0xD3, 0x8D, 0x46, 0x7B, 0x74, 0x01, 0x12, 0xFC, 0x78, 0xEB,
    0xEF, 0x7F, 0xF6, 0xAF, 0x4D, 0x9A, 0x3A, 0xF6, 0x64, 0x90, 0xDB, 0xE3, 0x48, 0xAB, 0x3E, 0xA7,
    0x2F, 0xC1, 0x18, 0x32, 0xBD, 0x23, 0x02, 0x9D, 0x3F, 0xF3, 0x27, 0x86, 0x71, 0x45, 0x26, 0x14,
    0x14, 0xF5, 0x19, 0xAA, 0x2D, 0xEE, 0x50, 0x10
};


typedef struct {
    BCRYPT_ALG_HANDLE algHandle;
    BCRYPT_HASH_HANDLE handle;
    PVOID object;
} MY_HASH_OBJ;

VOID MyFreeHash(MY_HASH_OBJ* pHashObj)
{
    if (pHashObj->handle)
        BCryptDestroyHash(pHashObj->handle);
    if (pHashObj->object)
        ExFreePoolWithTag(pHashObj->object, 'vhpK');
    if (pHashObj->algHandle)
        BCryptCloseAlgorithmProvider(pHashObj->algHandle, 0);
}

NTSTATUS MyInitHash(MY_HASH_OBJ* pHashObj)
{
    NTSTATUS status;
    ULONG hashObjectSize;
    ULONG querySize;
    memset(pHashObj, 0, sizeof(MY_HASH_OBJ));

    if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(&pHashObj->algHandle, KPH_HASH_ALGORITHM, NULL, 0)))
        goto CleanupExit;

    if (!NT_SUCCESS(status = BCryptGetProperty(pHashObj->algHandle, BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashObjectSize, sizeof(ULONG), &querySize, 0)))
        goto CleanupExit;

    pHashObj->object = ExAllocatePoolWithTag(PagedPool, hashObjectSize, 'vhpK');
    if (!pHashObj->object) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanupExit;
    }

    if (!NT_SUCCESS(status = BCryptCreateHash(pHashObj->algHandle, &pHashObj->handle, (PUCHAR)pHashObj->object, hashObjectSize, NULL, 0, 0)))
        goto CleanupExit;

CleanupExit:
    // on failure the caller must call MyFreeHash

    return status;
}

NTSTATUS MyHashData(MY_HASH_OBJ* pHashObj, PVOID Data, ULONG DataSize)
{
    return BCryptHashData(pHashObj->handle, (PUCHAR)Data, DataSize, 0);
}

NTSTATUS MyFinishHash(MY_HASH_OBJ* pHashObj, PVOID* Hash, PULONG HashSize)
{
    NTSTATUS status;
    ULONG querySize;

    if (!NT_SUCCESS(status = BCryptGetProperty(pHashObj->algHandle, BCRYPT_HASH_LENGTH, (PUCHAR)HashSize, sizeof(ULONG), &querySize, 0)))
        goto CleanupExit;

    *Hash = ExAllocatePoolWithTag(PagedPool, *HashSize, 'vhpK');
    if (!*Hash) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanupExit;
    }

    if (!NT_SUCCESS(status = BCryptFinishHash(pHashObj->handle, (PUCHAR)*Hash, *HashSize, 0)))
        goto CleanupExit;

    return STATUS_SUCCESS;

CleanupExit:
    if (*Hash) {
        ExFreePoolWithTag(*Hash, 'vhpK');
        *Hash = NULL;
    }

    return status;
}

NTSTATUS KphHashFile(
    _In_ PUNICODE_STRING FileName,
    _Out_ PVOID *Hash,
    _Out_ PULONG HashSize
    )
{
    NTSTATUS status;
    MY_HASH_OBJ hashObj;
    ULONG querySize;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;
    HANDLE fileHandle = NULL;
    FILE_STANDARD_INFORMATION standardInfo;
    ULONG remainingBytes;
    ULONG bytesToRead;
    PVOID buffer = NULL;

    if(!NT_SUCCESS(status = MyInitHash(&hashObj)))
        goto CleanupExit;

    // Open the file and compute the hash.

    InitializeObjectAttributes(&objectAttributes, FileName, OBJ_KERNEL_HANDLE, NULL, NULL);

    if (!NT_SUCCESS(status = ZwCreateFile(&fileHandle, FILE_GENERIC_READ, &objectAttributes,
        &iosb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0)))
    {
        goto CleanupExit;
    }

    if (!NT_SUCCESS(status = ZwQueryInformationFile(fileHandle, &iosb, &standardInfo,
        sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation)))
    {
        goto CleanupExit;
    }

    if (standardInfo.EndOfFile.QuadPart <= 0)
    {
        status = STATUS_UNSUCCESSFUL;
        goto CleanupExit;
    }
    if (standardInfo.EndOfFile.QuadPart > FILE_MAX_SIZE)
    {
        status = STATUS_FILE_TOO_LARGE;
        goto CleanupExit;
    }

    if (!(buffer = ExAllocatePoolWithTag(PagedPool, FILE_BUFFER_SIZE, 'vhpK')))
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanupExit;
    }

    remainingBytes = (ULONG)standardInfo.EndOfFile.QuadPart;

    while (remainingBytes != 0)
    {
        bytesToRead = FILE_BUFFER_SIZE;
        if (bytesToRead > remainingBytes)
            bytesToRead = remainingBytes;

        if (!NT_SUCCESS(status = ZwReadFile(fileHandle, NULL, NULL, NULL, &iosb, buffer, bytesToRead,
            NULL, NULL)))
        {
            goto CleanupExit;
        }
        if ((ULONG)iosb.Information != bytesToRead)
        {
            status = STATUS_INTERNAL_ERROR;
            goto CleanupExit;
        }

        if (!NT_SUCCESS(status = MyHashData(&hashObj, buffer, bytesToRead)))
            goto CleanupExit;

        remainingBytes -= bytesToRead;
    }

    if (!NT_SUCCESS(status = MyFinishHash(&hashObj, Hash, HashSize)))
        goto CleanupExit;

CleanupExit:
    if (buffer)
        ExFreePoolWithTag(buffer, 'vhpK');
    if (fileHandle)
        ZwClose(fileHandle);
    MyFreeHash(&hashObj);

    return status;
}

NTSTATUS KphVerifySignature(
    _In_ PVOID Hash,
    _In_ ULONG HashSize,
    _In_ PUCHAR Signature,
    _In_ ULONG SignatureSize
    )
{
    NTSTATUS status;
    BCRYPT_ALG_HANDLE signAlgHandle = NULL;
    BCRYPT_KEY_HANDLE keyHandle = NULL;
    PVOID hash = NULL;
    ULONG hashSize;

    // Import the trusted public key.

    if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(&signAlgHandle, KPH_SIGN_ALGORITHM, NULL, 0)))
        goto CleanupExit;
    if (!NT_SUCCESS(status = BCryptImportKeyPair(signAlgHandle, NULL, KPH_BLOB_PUBLIC, &keyHandle,
        KphpTrustedPublicKey, sizeof(KphpTrustedPublicKey), 0)))
    {
        goto CleanupExit;
    }

    // Verify the hash.

    if (!NT_SUCCESS(status = BCryptVerifySignature(keyHandle, NULL, Hash, HashSize, Signature,
        SignatureSize, 0)))
    {
        goto CleanupExit;
    }

CleanupExit:
    if (keyHandle)
        BCryptDestroyKey(keyHandle);
    if (signAlgHandle)
        BCryptCloseAlgorithmProvider(signAlgHandle, 0);

    return status;
}

NTSTATUS KphVerifyFile(
    _In_ PUNICODE_STRING FileName,
    _In_ PUCHAR Signature,
    _In_ ULONG SignatureSize
    )
{
    NTSTATUS status;
    PVOID hash = NULL;
    ULONG hashSize;

    // Hash the file.

    if (!NT_SUCCESS(status = KphHashFile(FileName, &hash, &hashSize)))
        goto CleanupExit;

    // Verify the hash.

    if (!NT_SUCCESS(status = KphVerifySignature(hash, hashSize, Signature, SignatureSize)))
    {
        goto CleanupExit;
    }

CleanupExit:
    if (hash)
        ExFreePoolWithTag(hash, 'vhpK');
 
    return status;
}

NTSTATUS KphVerifyBuffer(
    _In_ PUCHAR Buffer,
    _In_ ULONG BufferSize,
    _In_ PUCHAR Signature,
    _In_ ULONG SignatureSize
    )
{
    NTSTATUS status;
    MY_HASH_OBJ hashObj;
    PVOID hash = NULL;
    ULONG hashSize;

    // Hash the Buffer.

    if(!NT_SUCCESS(status = MyInitHash(&hashObj)))
        goto CleanupExit;

    MyHashData(&hashObj, Buffer, BufferSize);

	if(!NT_SUCCESS(status = MyFinishHash(&hashObj, &hash, &hashSize)))
        goto CleanupExit;

    // Verify the hash.

    if (!NT_SUCCESS(status = KphVerifySignature(hash, hashSize, Signature, SignatureSize)))
    {
        goto CleanupExit;
    }

CleanupExit:

    if (hash)
        ExFreePoolWithTag(hash, 'vhpK');
 
    MyFreeHash(&hashObj);

    return status;
}

NTSTATUS KphReadSignature(    
    _In_ PUNICODE_STRING FileName,
    _Out_ PUCHAR *Signature,
    _Out_ ULONG *SignatureSize
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;
    HANDLE fileHandle = NULL;
    FILE_STANDARD_INFORMATION standardInfo;

    // Open the file and read it.

    InitializeObjectAttributes(&objectAttributes, FileName, OBJ_KERNEL_HANDLE, NULL, NULL);

    if (!NT_SUCCESS(status = ZwCreateFile(&fileHandle, FILE_GENERIC_READ, &objectAttributes,
        &iosb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0)))
    {
        goto CleanupExit;
    }

    if (!NT_SUCCESS(status = ZwQueryInformationFile(fileHandle, &iosb, &standardInfo,
        sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation)))
    {
        goto CleanupExit;
    }

    if (standardInfo.EndOfFile.QuadPart <= 0)
    {
        status = STATUS_UNSUCCESSFUL;
        goto CleanupExit;
    }
    if (standardInfo.EndOfFile.QuadPart > KPH_SIGNATURE_MAX_SIZE)
    {
        status = STATUS_FILE_TOO_LARGE;
        goto CleanupExit;
    }

    *SignatureSize = (ULONG)standardInfo.EndOfFile.QuadPart;

    *Signature = ExAllocatePoolWithTag(PagedPool, *SignatureSize, tzuk);
    if(!*Signature)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanupExit;
    }

    if (!NT_SUCCESS(status = ZwReadFile(fileHandle, NULL, NULL, NULL, &iosb, *Signature, *SignatureSize, 
        NULL, NULL)))
    {
        goto CleanupExit;
    }

CleanupExit:
    if (fileHandle)
        ZwClose(fileHandle);
    
    return status;
}

NTSTATUS KphVerifyCurrentProcess()
{
    NTSTATUS status;
    PUNICODE_STRING processFileName = NULL;
    PUNICODE_STRING signatureFileName = NULL;
    ULONG signatureSize = 0;
    PUCHAR signature = NULL;
    
    if (!NT_SUCCESS(status = SeLocateProcessImageName(PsGetCurrentProcess(), &processFileName)))
        goto CleanupExit;


    //RtlCreateUnicodeString
    signatureFileName = ExAllocatePoolWithTag(PagedPool, sizeof(UNICODE_STRING) + processFileName->MaximumLength + 4 * sizeof(WCHAR), tzuk);
    if (!signatureFileName) 
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanupExit;
    }
    signatureFileName->Buffer = (PWCH)(((PUCHAR)signatureFileName) + sizeof(UNICODE_STRING));
    signatureFileName->MaximumLength = processFileName->MaximumLength + 5 * sizeof(WCHAR);

    //RtlCopyUnicodeString
    wcscpy(signatureFileName->Buffer, processFileName->Buffer);
    signatureFileName->Length = processFileName->Length;

    //RtlUnicodeStringCat
    wcscat(signatureFileName->Buffer, L".sig");
    signatureFileName->Length += 4 * sizeof(WCHAR);


    if (!NT_SUCCESS(status = KphReadSignature(signatureFileName, &signature, &signatureSize)))
        goto CleanupExit;

    status = KphVerifyFile(processFileName, signature, signatureSize); 


CleanupExit:
    if (signature)
        ExFreePoolWithTag(signature, tzuk);
    if (processFileName)
        ExFreePool(processFileName);
    if (signatureFileName)
        ExFreePoolWithTag(signatureFileName, tzuk);

    return status;
}


//---------------------------------------------------------------------------

#define KERNEL_MODE
#include "common/stream.h"
#include "common/base64.c"
extern POOL *Driver_Pool;

NTSTATUS Conf_Read_Line(STREAM *stream, WCHAR *line, int *linenum);

_FX BOOLEAN KphParseDate(const WCHAR* date_str, LARGE_INTEGER* date)
{
    TIME_FIELDS timeFiled = { 0 };
    const WCHAR* ptr = date_str;
    for (; *ptr == ' '; ptr++); // trim left
    const WCHAR* end = wcschr(ptr, L'.');
    if (end) {
        //*end = L'\0';
        timeFiled.Day = (CSHORT)_wtoi(ptr);
        //*end = L'.';
        ptr = end + 1;

        end = wcschr(ptr, L'.');
        if (end) {
            //*end++ = L'\0';
            timeFiled.Month = (CSHORT)_wtoi(ptr);
            //*end = L'.';
            ptr = end + 1;

            timeFiled.Year = (CSHORT)_wtoi(ptr);

            RtlTimeFieldsToTime(&timeFiled, date);

            return TRUE;
        }
    }
    return FALSE;
}

// Example of __DATE__ string: "Jul 27 2012"
//                              0123456789A

#define BUILD_YEAR_CH0 (__DATE__[ 7])
#define BUILD_YEAR_CH1 (__DATE__[ 8])
#define BUILD_YEAR_CH2 (__DATE__[ 9])
#define BUILD_YEAR_CH3 (__DATE__[10])

#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')

#define BUILD_DAY_CH0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define BUILD_DAY_CH1 (__DATE__[ 5])

#define CH2N(c) (c - '0')

_FX VOID KphGetBuildDate(LARGE_INTEGER* date)
{
    TIME_FIELDS timeFiled = { 0 };
    timeFiled.Day = CH2N(BUILD_DAY_CH0) * 10 + CH2N(BUILD_DAY_CH1);
    timeFiled.Month = (
        (BUILD_MONTH_IS_JAN) ?  1 : (BUILD_MONTH_IS_FEB) ?  2 : (BUILD_MONTH_IS_MAR) ?  3 :
        (BUILD_MONTH_IS_APR) ?  4 : (BUILD_MONTH_IS_MAY) ?  5 : (BUILD_MONTH_IS_JUN) ?  6 :
        (BUILD_MONTH_IS_JUL) ?  7 : (BUILD_MONTH_IS_AUG) ?  8 : (BUILD_MONTH_IS_SEP) ?  9 :
        (BUILD_MONTH_IS_OCT) ? 10 : (BUILD_MONTH_IS_NOV) ? 11 : (BUILD_MONTH_IS_DEC) ? 12 : 0);
    timeFiled.Year = CH2N(BUILD_YEAR_CH0) * 1000 + CH2N(BUILD_YEAR_CH1) * 100 + CH2N(BUILD_YEAR_CH2) * 10 + CH2N(BUILD_YEAR_CH3);
    RtlTimeFieldsToTime(&timeFiled, date);
}

_FX LONGLONG KphGetDate(CSHORT days, CSHORT months, CSHORT years)
{
    LARGE_INTEGER date;
    TIME_FIELDS timeFiled = { 0 };
    timeFiled.Day = days;
    timeFiled.Month = months;
    timeFiled.Year = years;
    RtlTimeFieldsToTime(&timeFiled, &date);
    return date.QuadPart;
}

_FX LONGLONG KphGetDateInterval(CSHORT days, CSHORT months, CSHORT years)
{
    return ((LONGLONG)days + (LONGLONG)months * 30ll + (LONGLONG)years * 365ll) * 24ll * 3600ll * 10000000ll; // 100ns steps -> 1sec
}

#include "verify.h"

SCertInfo Verify_CertInfo = { 0 };

_FX NTSTATUS KphValidateCertificate()
{
    Verify_CertInfo.active = 1;
    Verify_CertInfo.expired = 0;
    Verify_CertInfo.outdated = 0;
    Verify_CertInfo.grace_period = 0;
    Verify_CertInfo.locked = 1;
    Verify_CertInfo.lock_req = 0;
    Verify_CertInfo.type = eCertEternal;
    Verify_CertInfo.level = eCertMaxLevel;
    Verify_CertInfo.opt_desk = 1;
    Verify_CertInfo.opt_net = 1;
    Verify_CertInfo.opt_enc = 1;
    Verify_CertInfo.opt_sec = 1;
    Verify_CertInfo.expirers_in_sec = 99999999;

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------


// SMBIOS Structure header as described at
// see https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf (para 6.1.2)
typedef struct _dmi_header
{
  UCHAR type;
  UCHAR length;
  USHORT handle;
  UCHAR data[1];
} dmi_header;

// Structure needed to get the SMBIOS table using GetSystemFirmwareTable API.
// see https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemfirmwaretable
typedef struct _RawSMBIOSData {
  UCHAR  Used20CallingMethod;
  UCHAR  SMBIOSMajorVersion;
  UCHAR  SMBIOSMinorVersion;
  UCHAR  DmiRevision;
  DWORD  Length;
  UCHAR  SMBIOSTableData[1];
} RawSMBIOSData;

#define SystemFirmwareTableInformation 76 

BOOLEAN GetFwUuid(unsigned char* uuid)
{
    BOOLEAN result = FALSE;

    SYSTEM_FIRMWARE_TABLE_INFORMATION sfti;
    sfti.Action = SystemFirmwareTable_Get;
    sfti.ProviderSignature = 'RSMB';
    sfti.TableID = 0;
    sfti.TableBufferLength = 0;

    ULONG Length = sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION);
    NTSTATUS status = ZwQuerySystemInformation(SystemFirmwareTableInformation, &sfti, Length, &Length);
    if (status != STATUS_BUFFER_TOO_SMALL)
        return result;

    ULONG BufferSize = sfti.TableBufferLength;

    Length = BufferSize + sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION);
    SYSTEM_FIRMWARE_TABLE_INFORMATION* pSfti = ExAllocatePoolWithTag(PagedPool, Length, 'vhpK');
    if (!pSfti)
        return result;
    *pSfti = sfti;
    pSfti->TableBufferLength = BufferSize;

    status = ZwQuerySystemInformation(SystemFirmwareTableInformation, pSfti, Length, &Length);
    if (NT_SUCCESS(status)) 
    {
        RawSMBIOSData* smb = (RawSMBIOSData*)pSfti->TableBuffer;

        for (UCHAR* data = smb->SMBIOSTableData; data < smb->SMBIOSTableData + smb->Length;)
        {
            dmi_header* h = (dmi_header*)data;
            if (h->length < 4)
                break;

            //Search for System Information structure with type 0x01 (see para 7.2)
            if (h->type == 0x01 && h->length >= 0x19)
            {
                data += 0x08; //UUID is at offset 0x08

                // check if there is a valid UUID (not all 0x00 or all 0xff)
                BOOLEAN all_zero = TRUE, all_one = TRUE;
                for (int i = 0; i < 16 && (all_zero || all_one); i++)
                {
                    if (data[i] != 0x00) all_zero = FALSE;
                    if (data[i] != 0xFF) all_one = FALSE;
                }

                if (!all_zero && !all_one)
                {
                    // As off version 2.6 of the SMBIOS specification, the first 3 fields
                    // of the UUID are supposed to be encoded on little-endian. (para 7.2.1)
                    *uuid++ = data[3];
                    *uuid++ = data[2];
                    *uuid++ = data[1];
                    *uuid++ = data[0];
                    *uuid++ = data[5];
                    *uuid++ = data[4];
                    *uuid++ = data[7];
                    *uuid++ = data[6];
                    for (int i = 8; i < 16; i++)
                        *uuid++ = data[i];

                    result = TRUE;
                }

                break;
            }

            //skip over formatted area
            UCHAR* next = data + h->length;

            //skip over unformatted area of the structure (marker is 0000h)
            while (next < smb->SMBIOSTableData + smb->Length && (next[0] != 0 || next[1] != 0))
                next++;

            next += 2;

            data = next;
        }
    }

    ExFreePoolWithTag(pSfti, 'vhpK');

    return result;
}

wchar_t* hexbyte(UCHAR b, wchar_t* ptr)
{
    static const wchar_t* digits = L"0123456789ABCDEF";
    *ptr++ = digits[b >> 4];
    *ptr++ = digits[b & 0x0f];
    return ptr;
}

wchar_t g_uuid_str[40] = { 0 };

void InitFwUuid()
{
    UCHAR uuid[16];
    if (GetFwUuid(uuid))
    {
        wchar_t* ptr = g_uuid_str;
        int i;
        for (i = 0; i < 4; i++)
            ptr = hexbyte(uuid[i], ptr);
        *ptr++ = '-';
        for (; i < 6; i++)
            ptr = hexbyte(uuid[i], ptr);
        *ptr++ = '-';
        for (; i < 8; i++)
            ptr = hexbyte(uuid[i], ptr);
        *ptr++ = '-';
        for (; i < 10; i++)
            ptr = hexbyte(uuid[i], ptr);
        *ptr++ = '-';
        for (; i < 16; i++)
            ptr = hexbyte(uuid[i], ptr);
        *ptr++ = 0;
    }
    else // fallback to null guid on error
        wcscpy(g_uuid_str, L"00000000-0000-0000-0000-000000000000");
    
    DbgPrint("sbie FW-UUID: %S\n", g_uuid_str);
}
