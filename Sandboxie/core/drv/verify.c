/*
 * Copyright (C) 2016 wj32
 * Copyright (C) 2021 David Xanatos, xanasoft.com
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

_FX VOID KphParseDate(const WCHAR* date_str, LARGE_INTEGER* date)
{
    TIME_FIELDS timeFiled = { 0 };
    const WCHAR* ptr = date_str;
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
        }
    }
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
    return KphGetDate(1 + days, 1 + months, 1601 + years);
}

#define SOFTWARE_NAME L"Sandboxie-Plus"

union SCertInfo {
    ULONGLONG	State;
    struct {
        ULONG
            valid     : 1, // certificate is active
            expired   : 1, // certificate is expired but may be active
            outdated  : 1, // certificate is expired, not anymore valid for the current build
            business  : 1, // certificate is siutable for business use
            reservd_1 : 4,
            reservd_2 : 8,
            reservd_3 : 8,
            reservd_4 : 8;
        ULONG expirers_in_sec;
    };
} Verify_CertInfo = {0};

_FX NTSTATUS KphValidateCertificate(void)
{
    BOOLEAN CertDbg = FALSE;

    static const WCHAR *path_cert = L"%s\\Certificate.dat";
    NTSTATUS status;
    ULONG path_len = 0;
    WCHAR *path = NULL;
    STREAM *stream = NULL;

    MY_HASH_OBJ hashObj;
    ULONG hashSize;
    PUCHAR hash = NULL;
    ULONG signatureSize = 0;
    PUCHAR signature = NULL;

    const int line_size = 1024 * sizeof(WCHAR);
    WCHAR *line = NULL; //512 wchars
    char *temp = NULL; //1024 chars, utf8 encoded
    int line_num = 0;

    WCHAR* type = NULL;
    WCHAR* level = NULL;
    //WCHAR* key = NULL;
    LARGE_INTEGER cert_date = { 0 };

    if(!NT_SUCCESS(status = MyInitHash(&hashObj)))
        goto CleanupExit;

    path_len = wcslen(Driver_HomePathDos) * sizeof(WCHAR);
    path_len += 64;     // room for \Certificate.ini
    path = Mem_Alloc(Driver_Pool, path_len);
    line = Mem_Alloc(Driver_Pool, line_size);
    temp = Mem_Alloc(Driver_Pool, line_size);
    if (!path || !line || !temp) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanupExit;
    }

    //
    // read (Home Path)\Certificate.dat
    //

    RtlStringCbPrintfW(path, path_len, path_cert, Driver_HomePathDos);

    status = Stream_Open(&stream, path, FILE_GENERIC_READ, 0, FILE_SHARE_READ, FILE_OPEN, 0);
    if (!NT_SUCCESS(status)) {
        status = STATUS_NOT_FOUND;
        goto CleanupExit;
    }

    if(!NT_SUCCESS(status = Stream_Read_BOM(stream, NULL)))
        goto CleanupExit;

    status = Conf_Read_Line(stream, line, &line_num);
    while (NT_SUCCESS(status)) {

        WCHAR *ptr;
        WCHAR *name;
        WCHAR *value;
        ULONG temp_len;

        // parse tag name: value

        ptr = wcschr(line, L':');
        if ((! ptr) || ptr == line) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        value = &ptr[1];

        // eliminate trailing whitespace in the tag name

        while (ptr > line) {
            --ptr;
            if (*ptr > 32) {
                ++ptr;
                break;
            }
        }
        *ptr = L'\0';

        name = line;

        // eliminate leading and trailing whitespace in value

        while (*value <= 32) {
            if (! (*value))
                break;
            ++value;
        }

        if (*value == L'\0') {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        ptr = value + wcslen(value);
        while (ptr > value) {
            --ptr;
            if (*ptr > 32) {
                ++ptr;
                break;
            }
        }
        *ptr = L'\0';

        /*if (*value == '"') {
            value++;
            value[wcslen(value) - 1] = 0;
        }*/

        //
        // Extract and decode the signature
        //

        if (_wcsicmp(L"SIGNATURE", name) == 0 && signature == NULL) {
            signatureSize = b64_decoded_size(value);
            signature = Mem_Alloc(Driver_Pool, signatureSize);
            if (!signature) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto CleanupExit;
            }
            b64_decode(value, signature, signatureSize);
            goto next;
        }

        //
        // Hash the tag
        //

        if (NT_SUCCESS(RtlUnicodeToUTF8N(temp, line_size, &temp_len, name, wcslen(name) * sizeof(wchar_t))))
            MyHashData(&hashObj, temp, temp_len);
        
        if (NT_SUCCESS(RtlUnicodeToUTF8N(temp, line_size, &temp_len, value, wcslen(value) * sizeof(wchar_t))))
            MyHashData(&hashObj, temp, temp_len);

        //
        // Note: when parsing we may change the value of value, by adding \0's, hence we do all that after the hashing
        //

        if (_wcsicmp(L"DATE", name) == 0 && cert_date.QuadPart == 0) {
            // DD.MM.YYYY
            KphParseDate(value, &cert_date);
        }
        else if (_wcsicmp(L"TYPE", name) == 0 && type == NULL) {
            // TYPE-LEVEL
            WCHAR* ptr = wcschr(value, L'-');
            if (ptr != NULL) {
                *ptr++ = L'\0';
                if(level == NULL) level = Mem_AllocString(Driver_Pool, ptr);
            }
            type = Mem_AllocString(Driver_Pool, value);
        }
        else if (_wcsicmp(L"LEVEL", name) == 0 && level == NULL) {
            level = Mem_AllocString(Driver_Pool, value);
        }
        //else if (_wcsicmp(L"UPDATEKEY", name) == 0 && key == NULL) {
        //    key = Mem_AllocString(Driver_Pool, value);
        //}
        else if (_wcsicmp(L"SOFTWARE", name) == 0) { // if software is specified it must be the right one
            if (_wcsicmp(value, SOFTWARE_NAME) != 0) {
                status = STATUS_OBJECT_TYPE_MISMATCH;
                goto CleanupExit;
            }
        }
            
    next:
        status = Conf_Read_Line(stream, line, &line_num);
    }

    Stream_Close(stream);
    

    if(!NT_SUCCESS(status = MyFinishHash(&hashObj, &hash, &hashSize)))
        goto CleanupExit;

    if (!signature) {
        status = STATUS_INVALID_SECURITY_DESCR;
        goto CleanupExit;
    }

    status = KphVerifySignature(hash, hashSize, signature, signatureSize);

    Verify_CertInfo.State = 0; // clear
    if (NT_SUCCESS(status)) {

        Verify_CertInfo.valid = 1;

        if(CertDbg) DbgPrint("Sbie Cert type: %S-%S\n", type, level);

        TIME_FIELDS timeFiled = { 0 };
        if (CertDbg) {
            RtlTimeToTimeFields(&cert_date, &timeFiled);
            DbgPrint("Sbie Cert date: %02d.%02d.%d\n", timeFiled.Day, timeFiled.Month, timeFiled.Year);
        }

        LARGE_INTEGER BuildDate = { 0 };
        KphGetBuildDate(&BuildDate);

        if (CertDbg) {
            RtlTimeToTimeFields(&BuildDate, &timeFiled);
            if (CertDbg) DbgPrint("Sbie Build date: %02d.%02d.%d\n", timeFiled.Day, timeFiled.Month, timeFiled.Year);
        }

        LARGE_INTEGER SystemTime;
        LARGE_INTEGER LocalTime;
        KeQuerySystemTime(&SystemTime);
        ExSystemTimeToLocalTime(&SystemTime, &LocalTime);
        if (CertDbg) {
            RtlTimeToTimeFields(&LocalTime, &timeFiled);
            DbgPrint("Sbie Current time: %02d:%02d:%02d %02d.%02d.%d\n"
                , timeFiled.Hour, timeFiled.Minute, timeFiled.Second, timeFiled.Day, timeFiled.Month, timeFiled.Year);
        }

        if (!type && level) { // fix for some early hand crafted contributor certificates
            type = level;
            level = NULL;
        }

        // Checks if the certi if within its validity periode, failing that has no effect except ui notification
#define TEST_CERT_DATE(days, months, years) \
            if ((cert_date.QuadPart + KphGetDateInterval(days, months, years)) < LocalTime.QuadPart){ \
                Verify_CertInfo.expired = 1; \
            } else \
                Verify_CertInfo.expirers_in_sec = (ULONG)(((cert_date.QuadPart + KphGetDateInterval(0, 0, 1)) - LocalTime.QuadPart) / 10000000ll); // 100ns steps -> 1sec

        // Check if the certificate is valid for the current build, failing this locks features out
#define TEST_VALIDITY(days, months, years) \
            TEST_CERT_DATE(days, months, years) \
            if ((cert_date.QuadPart + KphGetDateInterval(days, months, years)) < BuildDate.QuadPart){ \
                Verify_CertInfo.outdated = 1; \
                Verify_CertInfo.valid = 0; \
                status = STATUS_ACCOUNT_EXPIRED; \
            }

        // Check if the certificate is expired, failing this locks features out
#define TEST_EXPIRATION(days, months, years) \
            TEST_CERT_DATE(days, months, years) \
            if(Verify_CertInfo.expired == 1) { \
                Verify_CertInfo.valid = 0; \
                status = STATUS_ACCOUNT_EXPIRED; \
            }


        if (type && _wcsicmp(type, L"CONTRIBUTOR") == 0) {
            // forever - nothing to check here
        }
        else if (type && _wcsicmp(type, L"BUSINESS") == 0) {
            Verify_CertInfo.business = 1;
            TEST_EXPIRATION(0, 0, 1);
        }
        else /*if (!type || _wcsicmp(type, L"PERSONAL") == 0 || _wcsicmp(type, L"PATREON") == 0 || _wcsicmp(type, L"SUPPORTER") == 0) */ {
            // persistent
            if (level && _wcsicmp(level, L"HUGE") == 0) {
                // 
            } 
            else if (level && _wcsicmp(level, L"LARGE") == 0 && cert_date.QuadPart < KphGetDate(1,04,2022)) { // valid for all builds released with 2 years
                TEST_CERT_DATE(0, 0, 2); // no real expiration just ui reminder
            }
            else if (level && _wcsicmp(level, L"LARGE") == 0) { // valid for all builds released with 2 years
                TEST_VALIDITY(0, 0, 2);
            }
            else if (level && _wcsicmp(level, L"MEDIUM") == 0) { // valid for all builds released with 1 year 
                TEST_VALIDITY(0, 0, 1);
            }
            // subscriptions
            else if (level && _wcsicmp(level, L"TEST") == 0) { // test certificate 5 days only
                TEST_EXPIRATION(5, 0, 0);
            }
            else if (level && _wcsicmp(level, L"ENTRY") == 0) { // patreon entry level, first 3 monts, later longer
                TEST_EXPIRATION(0, 3, 0);
            }
            else /*if (!level || _wcsicmp(level, L"SMALL") == 0)*/ { // valid for 1 year
                TEST_EXPIRATION(0, 0, 1);
            }
        }
    }

CleanupExit:
    if(CertDbg)     DbgPrint("Sbie Cert status: %08x\n", status);


    if(path)        Mem_Free(path, path_len);    
    if(line)        Mem_Free(line, line_size);
    if(temp)        Mem_Free(temp, line_size);

    if (type)       Mem_FreeString(type);
    if (level)      Mem_FreeString(level);
    //if (key)        Mem_FreeString(key);

                    MyFreeHash(&hashObj);
    if(hash)        ExFreePoolWithTag(hash, 'vhpK');
    if(signature)   Mem_Free(signature, signatureSize);

    return status;
}
