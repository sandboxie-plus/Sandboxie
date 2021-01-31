mkdir %~dp0\OpenSSL

curl --ssl-no-revoke -L https://indy.fulgan.com/SSL/Archive/Experimental/openssl-1.1.0h-x64-VC2017.zip -o %~dp0\OpenSSL\openssl-1.1.0h-x64-VC2017.zip
"C:\Program Files\7-Zip\7z.exe" x -o%~dp0\OpenSSL\ %~dp0\OpenSSL\openssl-1.1.0h-x64-VC2017.zip

curl --ssl-no-revoke -L https://indy.fulgan.com/SSL/Archive/Experimental/openssl-1.1.0h-x32-VC2017.zip -o %~dp0\OpenSSL\openssl-1.1.0h-x32-VC2017.zip
"C:\Program Files\7-Zip\7z.exe" x -o%~dp0\OpenSSL\ %~dp0\OpenSSL\openssl-1.1.0h-x32-VC2017.zip

