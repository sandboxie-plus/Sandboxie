set version=1.1.1q

mkdir %~dp0\OpenSSL
curl -LsSO --output-dir %~dp0\OpenSSL\ http://wiki.overbyte.eu/arch/openssl-%version%-win64.zip
"C:\Program Files\7-Zip\7z.exe" x -bd -o%~dp0\OpenSSL\x64\ %~dp0\OpenSSL\openssl-%version%-win64.zip

curl -LsSO --output-dir %~dp0\OpenSSL\ http://wiki.overbyte.eu/arch/openssl-%version%-win32.zip
"C:\Program Files\7-Zip\7z.exe" x -bd -o%~dp0\OpenSSL\Win32\ %~dp0\OpenSSL\openssl-%version%-win32.zip
