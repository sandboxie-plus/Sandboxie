
#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include <bcrypt.h>
//#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fileapi.h>

#include "..\..\Sandboxie\common\win32_ntddk.h"



static UCHAR KphpTrustedPublicKey[] =
{
    0x45, 0x43, 0x53, 0x31, 0x20, 0x00, 0x00, 0x00, 0x05, 0x7A, 0x12, 0x5A, 0xF8, 0x54, 0x01, 0x42,
    0xDB, 0x19, 0x87, 0xFC, 0xC4, 0xE3, 0xD3, 0x8D, 0x46, 0x7B, 0x74, 0x01, 0x12, 0xFC, 0x78, 0xEB,
    0xEF, 0x7F, 0xF6, 0xAF, 0x4D, 0x9A, 0x3A, 0xF6, 0x64, 0x90, 0xDB, 0xE3, 0x48, 0xAB, 0x3E, 0xA7,
    0x2F, 0xC1, 0x18, 0x32, 0xBD, 0x23, 0x02, 0x9D, 0x3F, 0xF3, 0x27, 0x86, 0x71, 0x45, 0x26, 0x14,
    0x14, 0xF5, 0x19, 0xAA, 0x2D, 0xEE, 0x50, 0x10
};


#define CST_SIGN_ALGORITHM BCRYPT_ECDSA_P256_ALGORITHM
#define CST_SIGN_ALGORITHM_BITS 256
#define CST_HASH_ALGORITHM BCRYPT_SHA256_ALGORITHM
#define CST_BLOB_PRIVATE BCRYPT_ECCPRIVATE_BLOB
#define CST_BLOB_PUBLIC BCRYPT_ECCPUBLIC_BLOB

#define KPH_SIGNATURE_MAX_SIZE (128 * 1024) // 128 kB

#define FILE_BUFFER_SIZE 4096


NTSTATUS PhCreateFileWin32(_Out_ PHANDLE FileHandle, _In_ PCWSTR FileName, _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ ULONG FileAttributes, _In_ ULONG ShareAccess, _In_ ULONG CreateDisposition, _In_ ULONG CreateOptions)
{
    UNICODE_STRING uni;
	OBJECT_ATTRIBUTES attr;
    WCHAR wszBuffer[MAX_PATH];
    _snwprintf(wszBuffer, MAX_PATH, L"\\??\\%s", FileName);
	RtlInitUnicodeString(&uni, wszBuffer);
	InitializeObjectAttributes(&attr, &uni, OBJ_CASE_INSENSITIVE, NULL, 0);

	IO_STATUS_BLOCK Iosb;
	return NtCreateFile(FileHandle, DesiredAccess, &attr, &Iosb, NULL, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, NULL, 0);
}

NTSTATUS PhGetFileSize(_In_ HANDLE FileHandle, _Out_ PLARGE_INTEGER Size)
{
    if (GetFileSizeEx(FileHandle, Size))
        return STATUS_SUCCESS;
    return STATUS_UNSUCCESSFUL;
}


static NTSTATUS CstReadFile(
    _In_ PWSTR FileName,
    _In_ ULONG FileSizeLimit,
    _Out_ PVOID* Buffer,
    _Out_ PULONG FileSize
    )
{
    NTSTATUS status;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    LARGE_INTEGER fileSize;
    PVOID buffer;
    IO_STATUS_BLOCK iosb;

    if (!NT_SUCCESS(status = PhCreateFileWin32(&fileHandle, FileName, FILE_GENERIC_READ, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE)))
        goto CleanupExit;

    if (!NT_SUCCESS(status = PhGetFileSize(fileHandle, &fileSize)))
        goto CleanupExit;

    if (fileSize.QuadPart > FileSizeLimit)
        goto CleanupExit;

    buffer = malloc((ULONG)fileSize.QuadPart);
    if (!NT_SUCCESS(status = NtReadFile(fileHandle, NULL, NULL, NULL, &iosb, buffer, (ULONG)fileSize.QuadPart, NULL, NULL)))
        goto CleanupExit;

    *Buffer = buffer;
    *FileSize = (ULONG)fileSize.QuadPart;

CleanupExit:
    if(fileHandle != INVALID_HANDLE_VALUE)
        NtClose(fileHandle);

    return status;
}

typedef struct {
    BCRYPT_ALG_HANDLE hashAlgHandle;
    ULONG hashObjectSize;
    ULONG hashSize;
    PVOID hashObject;
    BCRYPT_HASH_HANDLE hashHandle;
    
} CST_HASH_OBJ;

static VOID CstFreeHash(CST_HASH_OBJ* pHashObj)
{
    if (pHashObj->hashHandle)
        BCryptDestroyHash(pHashObj->hashHandle);
    if (pHashObj->hashObject)
        free(pHashObj->hashObject); // must be freed after destroying hash object
    if (pHashObj->hashAlgHandle)
        BCryptCloseAlgorithmProvider(pHashObj->hashAlgHandle, 0);
}

static NTSTATUS CstInitHash(CST_HASH_OBJ* pHashObj)
{
    NTSTATUS status;
    ULONG querySize;
    memset(pHashObj, 0, sizeof(CST_HASH_OBJ));

    if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(&pHashObj->hashAlgHandle, CST_HASH_ALGORITHM, NULL, 0)))
        goto CleanupExit;

    if (!NT_SUCCESS(status = BCryptGetProperty(pHashObj->hashAlgHandle, BCRYPT_OBJECT_LENGTH, (PUCHAR)&pHashObj->hashObjectSize, sizeof(ULONG), &querySize, 0)))
        goto CleanupExit;

    if (!NT_SUCCESS(status = BCryptGetProperty(pHashObj->hashAlgHandle, BCRYPT_HASH_LENGTH, (PUCHAR)&pHashObj->hashSize, sizeof(ULONG), &querySize, 0)))
        goto CleanupExit;

    pHashObj->hashObject = malloc(pHashObj->hashObjectSize);
    if (!pHashObj->hashObject) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanupExit;
    }

    if (!NT_SUCCESS(status = BCryptCreateHash(pHashObj->hashAlgHandle, &pHashObj->hashHandle, (PUCHAR)pHashObj->hashObject, pHashObj->hashObjectSize, NULL, 0, 0)))
        goto CleanupExit;

CleanupExit:
    if (!NT_SUCCESS(status))
        CstFreeHash(pHashObj);

    return status;
}

static NTSTATUS CstHashData(
    CST_HASH_OBJ* pHashObj,
    _Out_ PVOID Data,
    _Out_ ULONG DataSize
    )
{
    return BCryptHashData(pHashObj->hashHandle, (PUCHAR)Data, DataSize, 0);
}

static NTSTATUS CstFinishHash(
    CST_HASH_OBJ* pHashObj,
    _Out_ PVOID* Hash,
    _Out_ PULONG HashSize
    )
{
    NTSTATUS status;
    PVOID hash;

    hash = malloc(pHashObj->hashSize);
    if (!hash) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanupExit;
    }

    if (!NT_SUCCESS(status = BCryptFinishHash(pHashObj->hashHandle, (PUCHAR)hash, pHashObj->hashSize, 0)))
        goto CleanupExit;

    *HashSize = pHashObj->hashSize;
    *Hash = hash;
    return STATUS_SUCCESS;

CleanupExit:
    if (hash)
        free(hash);

    return status;
}


static NTSTATUS CstHashFile(
    _In_ PCWSTR FileName,
    _Out_ PVOID* Hash,
    _Out_ PULONG HashSize
    )
{
    NTSTATUS status;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    PVOID buffer = NULL;
    IO_STATUS_BLOCK iosb;
    CST_HASH_OBJ hashObj;

    if (!NT_SUCCESS(status = CstInitHash(&hashObj)))
        goto CleanupExit;

    if (!NT_SUCCESS(status = PhCreateFileWin32(&fileHandle, FileName, FILE_GENERIC_READ, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE)))
        goto CleanupExit;

    buffer = malloc(FILE_BUFFER_SIZE);
    if (!buffer) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanupExit;
    }

    while (TRUE)
    {
        if (!NT_SUCCESS(status = NtReadFile(fileHandle, NULL, NULL, NULL, &iosb, buffer, FILE_BUFFER_SIZE, NULL, NULL)))
        {
            if (status == STATUS_END_OF_FILE)
                break;

            goto CleanupExit;
        }

        if (!NT_SUCCESS(status = CstHashData(&hashObj, buffer, (ULONG)iosb.Information)))
            goto CleanupExit;
    }

    if (!NT_SUCCESS(status = CstFinishHash(&hashObj, Hash, HashSize)))
        goto CleanupExit;

CleanupExit:
    if(buffer)
        free(buffer);
    if(fileHandle != INVALID_HANDLE_VALUE)
        NtClose(fileHandle);
    CstFreeHash(&hashObj);

    return status;
}

static NTSTATUS VerifyHashSignature(
    _In_ PVOID Hash,
    _In_ ULONG HashSize,
    _In_ PVOID Signature,
    _In_ ULONG SignatureSize
    )
{
    NTSTATUS status;
    BCRYPT_ALG_HANDLE signAlgHandle = NULL;
    BCRYPT_KEY_HANDLE keyHandle = NULL;
    
    // Import the trusted public key.

    if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(&signAlgHandle, CST_SIGN_ALGORITHM, NULL, 0)))
        goto CleanupExit;
    if (!NT_SUCCESS(status = BCryptImportKeyPair(signAlgHandle, NULL, CST_BLOB_PUBLIC, &keyHandle,
        KphpTrustedPublicKey, sizeof(KphpTrustedPublicKey), 0)))
    {
        goto CleanupExit;
    }

    // Verify the hash.

    if (!NT_SUCCESS(status = BCryptVerifySignature(keyHandle, NULL, (PUCHAR)Hash, HashSize, (PUCHAR)Signature,
        SignatureSize, 0)))
    {
        goto CleanupExit;
    }

CleanupExit:
    if (keyHandle != NULL)
        BCryptDestroyKey(keyHandle);
    if (signAlgHandle)
        BCryptCloseAlgorithmProvider(signAlgHandle, 0);

    return status;
}

NTSTATUS VerifyFileSignature(const wchar_t* FilePath)
{
    NTSTATUS status;
    ULONG hashSize;
    PVOID hash = NULL;
    ULONG signatureSize;
    PVOID signature = NULL;
    WCHAR* signatureFileName = NULL;


    // Read the signature.
    signatureFileName = (WCHAR*)malloc((wcslen(FilePath) + 4 + 1) * sizeof(WCHAR));
    if(!signatureFileName) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CleanupExit;
    }
    wcscpy(signatureFileName, FilePath);
    wcscat(signatureFileName, L".sig");

    // Read the signature file.

    if (!NT_SUCCESS(status = CstReadFile(signatureFileName, KPH_SIGNATURE_MAX_SIZE, &signature, &signatureSize)))
        goto CleanupExit;

    // Hash the file.

    if (!NT_SUCCESS(status = CstHashFile(FilePath, &hash, &hashSize)))
        goto CleanupExit;

    // Verify the hash.

    if (!NT_SUCCESS(status = VerifyHashSignature((PUCHAR)hash, hashSize, (PUCHAR)signature, signatureSize)))
    {
        goto CleanupExit;
    }

CleanupExit:
    if (signature)
        free(signature);
    if (hash)
        free(hash);
    if (signatureFileName)
        free(signatureFileName);

    return status;
}

