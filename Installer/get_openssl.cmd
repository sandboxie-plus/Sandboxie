set version=1_1_1p

mkdir %~dp0\OpenSSL
curl -L --url https://github.com/DavidXanatos/openssl/releases/download/OpenSSL_%version%/OpenSSL-%version%.zip -o %~dp0\OpenSSL\OpenSSL-%version%.zip --ssl-no-revoke
"C:\Program Files\7-Zip\7z.exe" x -bd -o%~dp0\OpenSSL\ %~dp0\OpenSSL\OpenSSL-%version%.zip

