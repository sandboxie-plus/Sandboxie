rem @echo off

mkdir %~dp0\Assets


mkdir %~dp0\Assets\Plus
copy %~dp0\Languages.iss %~dp0\Assets\Plus\
copy %~dp0\license.txt %~dp0\Assets\Plus\
copy %~dp0\Sandboxie.ini %~dp0\Assets\Plus\
copy %~dp0\Sandboxie-Plus.ini %~dp0\Assets\Plus\
copy %~dp0\Sandboxie-Plus.iss %~dp0\Assets\Plus\
copy %~dp0\SandManInstall.ico %~dp0\Assets\Plus\
mkdir %~dp0\Assets\Plus\isl
copy %~dp0\isl\* %~dp0\Assets\Plus\isl\


mkdir %~dp0\Assets\Classic
mkdir %~dp0\Assets\Classic\apps
mkdir %~dp0\Assets\Classic\apps\res
copy %~dp0..\sandboxie\apps\res\sandbox-full.ico %~dp0\Assets\Classic\apps\res\
mkdir %~dp0\Assets\Classic\common
copy %~dp0..\sandboxie\common\my_version.h %~dp0\Assets\Classic\common\
mkdir %~dp0\Assets\Classic\install
copy %~dp0..\sandboxie\install\Cmdline.nsh %~dp0\Assets\Classic\install\
copy %~dp0..\sandboxie\install\InstallType.ini %~dp0\Assets\Classic\install\
copy %~dp0..\sandboxie\install\IsAdmin.nsh %~dp0\Assets\Classic\install\
copy %~dp0..\sandboxie\install\IsWow64.nsh %~dp0\Assets\Classic\install\
copy %~dp0..\sandboxie\install\LICENSE.TXT %~dp0\Assets\Classic\install\
copy %~dp0..\sandboxie\install\ParseVersion.bat %~dp0\Assets\Classic\install\
copy %~dp0..\sandboxie\install\Registry.nsh %~dp0\Assets\Classic\install\
copy %~dp0..\sandboxie\install\SandboxieVS.nsi %~dp0\Assets\Classic\install\
copy %~dp0..\sandboxie\install\Warning.ini %~dp0\Assets\Classic\install\
copy %~dp0..\sandboxie\install\whatsnew.html %~dp0\Assets\Classic\install\
mkdir %~dp0\Assets\Classic\msgs
mkdir %~dp0\Assets\Classic\msgs\SbieRelease
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Albanian.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Arabic.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Bulgarian.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Croatian.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Czech.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Danish.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Dutch.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_English.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Estonian.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Farsi.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Finnish.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_French.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_German.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Greek.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Hebrew.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Hungarian.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Indonesian.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Italian.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Japanese.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Korean.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Macedonian.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Polish.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Portuguese.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_PortugueseBr.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Russian.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_SimpChinese.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Slovak.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Spanish.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Swedish.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_TradChinese.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Turkish.txt %~dp0\Assets\Classic\msgs\SbieRelease\
copy %~dp0..\sandboxie\msgs\SbieRelease\NsisText_Ukrainian.txt %~dp0\Assets\Classic\msgs\SbieRelease\

