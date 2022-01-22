mkdir %~dp0\OpenSSL

curl --ssl-no-revoke -L http://wiki.overbyte.eu/arch/openssl-1.1.1m-win64.zip -o %~dp0\OpenSSL\openssl-1.1.1m-win64.zip
"C:\Program Files\7-Zip\7z.exe" x -o%~dp0\OpenSSL\x64\ %~dp0\OpenSSL\openssl-1.1.1m-win64.zip

curl --ssl-no-revoke -L http://wiki.overbyte.eu/arch/openssl-1.1.1m-win32.zip -o %~dp0\OpenSSL\openssl-1.1.1m-win32.zip
"C:\Program Files\7-Zip\7z.exe" x -o%~dp0\OpenSSL\Win32\ %~dp0\OpenSSL\openssl-1.1.1m-win32.zip

