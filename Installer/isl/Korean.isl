; *** Inno Setup version 6.1.0+ Korean messages ***

; ▒ Translator: VenusGirl (venusgirl@outlook.com)
;
; 이 파일의 사용자 제공 번역을 다운로드하려면 다음으로 이동하십시오:
;   https://jrsoftware.org/files/istrans/

; 참고: 이 텍스트를 번역할 때는 InnoSetup 메시지에
; 마침표가 자동으로 추가되므로 아직 없는 메시지의 끝에
; 마침표(.)를 추가하지 마십시오 (마침표를 추가하면
; 두 개의 마침표가 표시됩니다).

[LangOptions]
; 다음 세 항목은 매우 중요합니다. 도움말 파일의
; '[LangOptions] 섹션' 항목을 읽고 이해하십시오.
LanguageName=한국어
LanguageID=$0412
LanguageCodePage=949
; 번역할 언어가 특수 글꼴 또는 크기를 필요로 하는 경우
; 다음 항목 중 하나를 주석 해제하고 적절하게 변경하십시오.
;DialogFontName=
;DialogFontSize=8
;WelcomeFontName=Verdana
;WelcomeFontSize=12
;TitleFontName=Arial
;TitleFontSize=29
;CopyrightFontName=Arial
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=설치
SetupWindowTitle=%1 설치
UninstallAppTitle=제거
UninstallAppFullTitle=%1 제거

; *** Misc. common
InformationTitle=정보
ConfirmTitle=확인
ErrorTitle=오류

; *** SetupLdr messages
SetupLdrStartupMessage=%1을(를) 설치합니다, 계속하시겠습니까?
LdrCannotCreateTemp=임시 파일을 만들 수 없습니다. 설치가 중단되었습니다.
LdrCannotExecTemp=임시 디렉터리에서 파일을 실행할 수 없습니다. 설치가 중단되었습니다.
HelpTextNote=

; *** Startup error messages
LastErrorMessage=%1.%n%n오류 %2: %3
SetupFileMissing=%1 파일이 설치 디렉터리에 없습니다. 문제를 해결하거나 프로그램의 새 사본을 구하십시오.
SetupFileCorrupt=설치 파일이 손상되었습니다. 프로그램의 새 사본을 구하십시오.
SetupFileCorruptOrWrongVer=설치 파일이 손상되었거나 이 버전의 설치 프로그램과 호환되지 않습니다. 문제를 해결하거나 프로그램의 새 복사본을 구하십시오.
InvalidParameter=명령줄에 잘못된 매개변수가 전달되었습니다:%n%n%1
SetupAlreadyRunning=설치가 이미 실행 중입니다.
WindowsVersionNotSupported=이 프로그램은 컴퓨터에서 실행 중인 Windows 버전을 지원하지 않습니다.
WindowsServicePackRequired=이 프로그램을 사용하려면 %1 서비스 팩 %2 이상이 필요합니다.
NotOnThisPlatform=이 프로그램은 %1에서 실행되지 않습니다.
OnlyOnThisPlatform=이 프로그램은 %1에서 실행되어야 합니다.
OnlyOnTheseArchitectures=이 프로그램은 다음 프로세서 아키텍처용으로 설계된 Windows 버전에만 설치할 수 있습니다:%n%n%1
WinVersionTooLowError=이 프로그램에는 %1 버전 %2 이상이 필요합니다.
WinVersionTooHighError=%1 버전 %2 이상에 이 프로그램을 설치할 수 없습니다.
AdminPrivilegesRequired=이 프로그램을 설치할 때 관리자로 로그인해야 합니다.
PowerUserPrivilegesRequired=이 프로그램을 설치할 때 관리자 또는 Power Users 그룹의 구성원으로 로그인해야 합니다.
SetupAppRunningError=설치에서 %1이(가) 현재 실행 중임을 감지했습니다.%n%n지금 모든 인스턴스를 닫은 다음 확인을 클릭하여 계속하거나 취소를 클릭하여 종료하십시오.
UninstallAppRunningError=제거에서 %1이(가) 현재 실행 중임을 감지했습니다.%n%n지금 모든 인스턴스를 닫은 다음 확인을 클릭하여 계속하거나 취소를 클릭하여 종료하십시오.

; *** Startup questions
PrivilegesRequiredOverrideTitle=설치 모드 선택
PrivilegesRequiredOverrideInstruction=설치 모드를 선택해 주십시오
PrivilegesRequiredOverrideText1=%1은 모든 사용자 (관리자 권한 필요) 또는 사용자용으로 설치합니다.
PrivilegesRequiredOverrideText2=%1은 현재 사용자 또는 모든 사용자 (관리자 권한 필요)용으로 설치합니다.
PrivilegesRequiredOverrideAllUsers=모든 사용자용으로 설치(&A)
PrivilegesRequiredOverrideAllUsersRecommended=모든 사용자용으로 설치 (추천)(&A)
PrivilegesRequiredOverrideCurrentUser=현재 사용자용으로 설치(&M)
PrivilegesRequiredOverrideCurrentUserRecommended=현재 사용자용으로 설치 (추천)(&M)

; *** Misc. errors
ErrorCreatingDir=설치 프로그램에서 "%1" 디렉터리를 만들지 못했습니다.
ErrorTooManyFilesInDir="%1" 디렉터리에 파일이 너무 많아서 파일을 만들 수 없습니다

; *** Setup common messages
ExitSetupTitle=설치 종료
ExitSetupMessage=설치가 완료되지 않았습니다. 지금 종료하면 프로그램이 설치되지 않습니다.%n%n설치를 다시 실행하여 설치를 완료할 수 있습니다.%n%n설치를 종료하시겠습니까?
AboutSetupMenuItem=설치 정보(&A)...
AboutSetupTitle=설치 정보
AboutSetupMessage=%1 버전 %2%n%3%n%n%1 홈 페이지:%n%4
AboutSetupNote=
TranslatorNote=

; *** Buttons
ButtonBack=< 뒤로(&B)
ButtonNext=다음(&N) >
ButtonInstall=설치(&I)
ButtonOK=확인
ButtonCancel=취소
ButtonYes=예(&Y)
ButtonYesToAll=모두 예(&A)
ButtonNo=아니오(&N)
ButtonNoToAll=모두 아니오(&O)
ButtonFinish=마침(&F)
ButtonBrowse=찾아보기(&B)...
ButtonWizardBrowse=찾아보기(&R)...
ButtonNewFolder=새 폴더 만들기(&M)

; *** "Select Language" dialog messages
SelectLanguageTitle=설치 언어 선택
SelectLanguageLabel=설치 중에 사용할 언어를 선택하십시오.

; *** Common wizard text
ClickNext=다음을 클릭하여 계속하거나 취소를 클릭하여 설치를 종료합니다.
BeveledLabel=
BrowseDialogTitle=폴더 찾아보기
BrowseDialogLabel=아래 목록에서 폴더를 선택한 후 확인을 클릭하십시오.
NewFolderName=새 폴더

; *** "Welcome" wizard page
WelcomeLabel1=[name] 설치 마법사에 오신 것을 환영합니다
WelcomeLabel2=컴퓨터에 [name/ver]가 설치됩니다.%n%n계속하기 전에 다른 모든 응용 프로그램을 닫는 것이 좋습니다.

; *** "Password" wizard page
WizardPassword=암호
PasswordLabel1=이 설치는 암호로 보호됩니다.
PasswordLabel3=암호를 입력한 후 다음을 클릭하여 계속하십시오. 암호는 대소문자를 구분합니다.
PasswordEditLabel=암호(&P):
IncorrectPassword=입력한 암호가 올바르지 않습니다. 다시 시도하십시오.

; *** "License Agreement" wizard page
WizardLicense=사용권 계약
LicenseLabel=계속하기 전에 다음 중요한 정보를 읽어보십시오.
LicenseLabel3=다음 사용권 계약을 읽어보십시오. 설치를 계속하기 전에 이 계약 조건에 동의해야 합니다.
LicenseAccepted=동의합니다(&A)
LicenseNotAccepted=동의하지 않습니다(&D)

; *** "Information" wizard pages
WizardInfoBefore=정보
InfoBeforeLabel=계속하기 전에 다음 중요한 정보를 읽어보십시오.
InfoBeforeClickLabel=설치를 계속할 준비가 되었으면 다음을 클릭합니다.
WizardInfoAfter=정보
InfoAfterLabel=계속하기 전에 다음 중요한 정보를 읽어보십시오.
InfoAfterClickLabel=설치를 계속할 준비가 되었으면 다음을 클릭합니다.

; *** "User Information" wizard page
WizardUserInfo=사용자 정보
UserInfoDesc=사용자 정보를 입력하십시오.
UserInfoName=사용자 이름(&U):
UserInfoOrg=조직(&O):
UserInfoSerial=일련 번호:(&S):
UserInfoNameRequired=이름을 입력해야 합니다.

; *** "Select Destination Location" wizard page
WizardSelectDir=대상 위치 선택
SelectDirDesc=[name]을(를) 어디에 설치하시겠습니까?
SelectDirLabel3=다음 폴더에 [name]을(를) 설치합니다.
SelectDirBrowseLabel=계속하려면 다음을 클릭합니다. 다른 폴더를 선택하려면 찾아보기를 클릭합니다.
DiskSpaceGBLabel=이 프로그램은 최소 [gb] GB의 디스크 여유 공간이 필요합니다.
DiskSpaceMBLabel=이 프로그램은 최소 [mb] MB의 디스크 여유 공간이 필요합니다.
CannotInstallToNetworkDrive=네트워크 드라이브에 설치할 수 없습니다.
CannotInstallToUNCPath=UNC 경로에 설치할 수 없습니다.
InvalidPath=드라이브 문자를 포함한 전체 경로를 입력해야 합니다. 예:%n%nC:\APP%n%n 또는 UNC 경로 형식:%n%n\\server\share
InvalidDrive=선택한 드라이브 또는 UNC 공유가 존재하지 않거나 액세스할 수 없습니다, 다른 경로를 선택하십시오.
DiskSpaceWarningTitle=디스크 공간이 부족합니다
DiskSpaceWarning=설치 시 최소 %1 KB 디스크 공간이 필요하지만, 선택한 드라이브의 여유 공간은 %2 KB 밖에 없습니다.%n%n그래도 계속하시겠습니까?
DirNameTooLong=폴더 이름 또는 경로가 너무 깁니다.
InvalidDirName=폴더 이름이 유효하지 않습니다.
BadDirName32=폴더 이름은 다음 문자를 포함할 수 없습니다:%n%n%1
DirExistsTitle=폴더가 존재합니다
DirExists=폴더 %n%n%1%n%n이(가) 이미 존재합니다, 그래도 해당 폴더에 설치하시겠습니까?
DirDoesntExistTitle=폴더가 존재하지 않습니다
DirDoesntExist=폴더 %n%n%1%n%n이(가) 존재하지 않습니다, 폴더를 만드시겠습니까?

; *** "Select Components" wizard page
WizardSelectComponents=구성 요소 선택
SelectComponentsDesc=어떤 구성 요소를 설치해야 합니까?
SelectComponentsLabel2=설치할 구성 요소를 선택하고 설치하지 않을 구성 요소를 지웁니다. 계속할 준비가 되면 다음을 클릭합니다.
FullInstallation=모두 설치
; 가능하면 'Compact'를 'Minimal'로 번역하지 마십시오 (귀하의 언어로 '최소'를 의미합니다).
CompactInstallation=최소 설치
CustomInstallation=사용자 지정 설치
NoUninstallWarningTitle=구성 요소가 존재합니다
NoUninstallWarning=다음 구성 요소가 컴퓨터에 이미 설치되어 있습니다: %n%n%1%n%n이러한 구성 요소를 선택해도 제거되지 않습니다.%n%n계속하시겠습니까?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceGBLabel=현재 선택은 최소 [gb] GB의 디스크 여유 공간이 필요합니다.
ComponentsDiskSpaceMBLabel=현재 선택은 최소 [mb] MB의 디스크 여유 공간이 필요합니다.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=추가 작업 선택
SelectTasksDesc=어떤 추가 작업을 수행해야 합니까?
SelectTasksLabel2=[name]을(를) 설치하는 동안 수행할 추가 작업을 선택하고 다음을 클릭합니다.

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=시작 메뉴 폴더 선택
SelectStartMenuFolderDesc=프로그램의 바로가기를 어디에 설치하시겠습니까?
SelectStartMenuFolderLabel3=설치는 다음 시작 메뉴 폴더에 프로그램 바로가기를 만듭니다.
SelectStartMenuFolderBrowseLabel=계속하려면 다음을 클릭합니다. 다른 폴더를 선택하려면 찾아보기를 클릭합니다.
MustEnterGroupName=폴더 이름을 입력하십시오.
GroupNameTooLong=폴더 이름 또는 경로가 너무 깁니다.
InvalidGroupName=폴더 이름이 유효하지 않습니다.
BadGroupName=폴더 이름은 다음 문자를 포함할 수 없습니다:%n%n%1
NoProgramGroupCheck2=시작 메뉴 폴더를 만들지 않음(&D)

; *** "Ready to Install" wizard page
WizardReady=설치 준비 완료
ReadyLabel1=[name]을(를) 컴퓨터에 설치할 준비가 되었습니다.
ReadyLabel2a=설치를 클릭하여 설치를 계속하거나 설정을 검토하거나 변경하려면 뒤로를 클릭합니다.
ReadyLabel2b=설치를 클릭하여 설치를 계속합니다.
ReadyMemoUserInfo=사용자 정보:
ReadyMemoDir=대상 위치:
ReadyMemoType=설치 유형:
ReadyMemoComponents=선택한 구성 요소:
ReadyMemoGroup=시작 메뉴 폴더:
ReadyMemoTasks=추가 작업:

; *** TDownloadWizardPage wizard page and DownloadTemporaryFile
DownloadingLabel=추가 파일 다운로드 중...
ButtonStopDownload=다운로드 중지(&S)
StopDownload=다운로드를 중지하시겠습니까?
ErrorDownloadAborted=다운로드가 중지되었습니다
ErrorDownloadFailed=다운로드에 실패했습니다: %1 %2
ErrorDownloadSizeFailed=크기를 가져오지 못했습니다: %1 %2
ErrorFileHash1=파일 해시에 실패했습니다: %1
ErrorFileHash2=잘못된 파일 해시: 예상 %1, 찾음 %2
ErrorProgress=잘못된 진행 상황: %1 / %2
ErrorFileSize=잘못된 파일 크기: 예상 %1, 찾음 %2

; *** "Preparing to Install" wizard page
WizardPreparing=설치 준비 중
PreparingDesc=컴퓨터에 [name] 설치를 준비하는 중입니다.
PreviousInstallNotCompleted=이전 프로그램의 설치/제거가 완료되지 않았습니다. 설치를 완료하려면 컴퓨터를 다시 시작해야 합니다.%n%n컴퓨터를 재시작한 후 설치를 다시 실행하여 [name] 설치를 완료하십시오.
CannotContinue=설치를 계속할 수 없습니다. 종료하려면 취소를 클릭하십시오.
ApplicationsFound=다음 응용 프로그램에서 설치 프로그램에서 업데이트해야 하는 파일을 사용하고 있습니다. 이러한 응용 프로그램을 자동으로 닫도록 허용하는 것이 좋습니다.
ApplicationsFound2=다음 응용 프로그램에서 설치 프로그램에서 업데이트해야 하는 파일을 사용하고 있습니다. 이러한 응용 프로그램을 자동으로 닫도록 허용하는 것이 좋습니다. 설치가 완료되면 응용 프로그램을 다시 시작하려고 시도합니다.
CloseApplications=응용 프로그램 자동 닫기(&A)
DontCloseApplications=응용 프로그램을 닫지 않음(&D)
ErrorCloseApplications=모든 응용 프로그램을 자동으로 닫지 못했습니다. 계속하기 전에 설치 프로그램에서 업데이트해야 하는 파일을 사용하여 모든 응용 프로그램을 닫는 것이 좋습니다.
PrepareToInstallNeedsRestart=컴퓨터를 다시 시작해야 합니다. 컴퓨터를 다시 시작한 후 설치를 다시 실행하여 [name] 설치를 완료하십시오.%n%n지금 다시 시작하시겠습니까?

; *** "Installing" wizard page
WizardInstalling=설치 중
InstallingLabel=컴퓨터에 [name]을(를) 설치하는 동안 잠시 기다려 주십시오.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=[name] 설치 마법사 완료
FinishedLabelNoIcons=컴퓨터에 [name] 설치를 완료했습니다.
FinishedLabel=컴퓨터에 [name] 설치를 완료했습니다. 설치된 바로가기를 선택하여 응용 프로그램을 시작할 수 있습니다.
ClickFinish=설치를 종료하려면 마침을 클릭하십시오.
FinishedRestartLabel=[name] 설치를 완료하려면 컴퓨터를 다시 시작해야 합니다. 지금 다시 시작하시겠습니까?
FinishedRestartMessage=[name] 설치를 완료하려면 컴퓨터를 다시 시작해야 합니다.%n%n지금 다시 시작하시겠습니까?
ShowReadmeCheck=예, README 파일을 보고 싶습니다.
YesRadio=예, 지금 컴퓨터를 다시 시작합니다(&Y)
NoRadio=아니오, 나중에 컴퓨터를 다시 시작하겠습니다(&N)
; 예를 들어 'Run MyProg.exe'로 사용됩니다'
RunEntryExec=%1 실행
; 예를 들어 'Readme.txt 보기'로 사용됩니다'
RunEntryShellExec=%1 보기

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=설치에 다음 디스크가 필요합니다
SelectDiskLabel2=디스크 %1을(를) 삽입하고 확인을 클릭하십시오.%n%n이 디스크의 파일을 아래에 표시된 폴더 이외의 폴더에서 찾을 수 있으면 올바른 경로를 입력하거나 찾아보기를 클릭하십시오.
PathLabel=경로(&P):
FileNotInDir2="%1" 파일을 "%2"에서 찾을 수 없습니다. 올바른 디스크를 넣거나 다른 폴더를 선택하십시오.
SelectDirectoryLabel=다음 디스크의 위치를 지정하십시오.

; *** Installation phase messages
SetupAborted=설치가 완료되지 않았습니다.%n%n문제를 해결한 후 설치를 다시 실행하십시오.
AbortRetryIgnoreSelectAction=작업 선택
AbortRetryIgnoreRetry=재시도(&T)
AbortRetryIgnoreIgnore=오류를 무시하고 진행(&I)
AbortRetryIgnoreCancel=설치 취소

; *** Installation status messages
StatusClosingApplications=응용 프로그램을 닫는 중...
StatusCreateDirs=디렉터리를 만드는 중...
StatusExtractFiles=파일을 추출하는 중...
StatusCreateIcons=바로가기를 만드는 중...
StatusCreateIniEntries=INI 항목을 만드는 중...
StatusCreateRegistryEntries=레지스트리 항목을 만드는 중...
StatusRegisterFiles=파일을 등록하는 중...
StatusSavingUninstall=제거 정보를 저장하는 중...
StatusRunProgram=설치를 완료하는 중...
StatusRestartingApplications=응용 프로그램을 다시 시작하는 중...
StatusRollback=변경 내용을 롤백하는 중...

; *** Misc. errors
ErrorInternal2=내부 오류: %1
ErrorFunctionFailedNoCode=%1 실패
ErrorFunctionFailed=%1 실패; 코드 %2
ErrorFunctionFailedWithMessage=%1 실패, 코드: %2.%n%3
ErrorExecutingProgram=파일 실행 오류:%n%1

; *** Registry errors
ErrorRegOpenKey=레지스트리 키 열기 오류:%n%1\%2
ErrorRegCreateKey=레지스트리 키 생성 오류:%n%1\%2
ErrorRegWriteKey=레지스트리 키 쓰기 오류:%n%1\%2

; *** INI errors
ErrorIniEntry="%1" 파일에 INI 항목 만들기 오류입니다.

; *** File copying errors
FileAbortRetryIgnoreSkipNotRecommended=이 파일 건너뛰기 (추천하지 않음)(&S)
FileAbortRetryIgnoreIgnoreNotRecommended=오류를 무시하고 계속 (추천하지 않음)(&I)
SourceIsCorrupted=원본 파일이 손상되었습니다
SourceDoesntExist=원본 파일 "%1"이(가) 없습니다
ExistingFileReadOnly2=읽기 전용으로 표시되어 있으므로 기존 파일을 교체할 수 없습니다.
ExistingFileReadOnlyRetry=읽기 전용 속성을 제거하고 다시 시도(&R)
ExistingFileReadOnlyKeepExisting=기존 파일 유지(&K)
ErrorReadingExistingDest=기존 파일을 읽는 동안 오류 발생:
FileExistsSelectAction=작업 선택
FileExists2=파일이 이미 존재합니다.
FileExistsOverwriteExisting=기존 파일 덮어쓰기(&O)
FileExistsKeepExisting=기존 파일 유지(&K)
FileExistsOverwriteOrKeepAll=다음 충돌에 대해 이 작업 수행(&D)
ExistingFileNewerSelectAction=작업 선택
ExistingFileNewer2=설치 프로그램에서 설치하려는 파일보다 기존 파일이 더 최신입니다.
ExistingFileNewerOverwriteExisting=기존 파일 덮어쓰기(&O)
ExistingFileNewerKeepExisting=기존 파일 유지 (추천)(&K)
ExistingFileNewerOverwriteOrKeepAll=다음 충돌에 대해 이 작업 수행(&D)
ErrorChangingAttr=기존 파일의 속성을 변경하는 동안 오류 발생:
ErrorCreatingTemp=대상 디렉터리에 파일을 만드는 동안 오류 발생:
ErrorReadingSource=원본 파일을 읽는 동안 오류 발생:
ErrorCopying=파일을 복사하는 동안 오류 발생:
ErrorReplacingExistingFile=기존 파일을 교체하는 동안 오류 발생:
ErrorRestartReplace=RestartReplace 실패:
ErrorRenamingTemp=대상 디렉터리 내의 파일 이름을 바꾸는 동안 오류 발생:
ErrorRegisterServer=DLL/OCX를 등록할 수 없습니다: %1
ErrorRegSvr32Failed=종료 코드 %1로 인해 RegSvr32가 실패했습니다
ErrorRegisterTypeLib=유형 라이브러리를 등록할 수 없습니다: %1

; *** Uninstall display name markings
; 예를 들어 '내 프로그램'으로 사용됩니다 (32비트)'
UninstallDisplayNameMark=%1 (%2)비트
; 예를 들어 '내 프로그램'으로 사용됩니다 (32비트, 모든 사용자)'
UninstallDisplayNameMarks=%1 (%2, %3)
UninstallDisplayNameMark32Bit=32비트
UninstallDisplayNameMark64Bit=64비트
UninstallDisplayNameMarkAllUsers=모든 사용자
UninstallDisplayNameMarkCurrentUser=현재 사용자

; *** Post-installation errors
ErrorOpeningReadme=README 파일을 여는 동안 오류가 발생했습니다.
ErrorRestartingComputer=컴퓨터를 다시 시작하지 못했습니다. 이 작업을 수동으로 수행하십시오.

; *** Uninstaller messages
UninstallNotFound="%1" 파일이 없습니다. 제거할 수 없습니다.
UninstallOpenError="%1" 파일을 열 수 없습니다. 제거할 수 없습니다
UninstallUnsupportedVer="%1" 제거 로그 파일이 현재 버전의 제거 프로그램에서 인식할 수 없는 형식입니다. 제거할 수 없습니다
UninstallUnknownEntry=제거 로그에 알 수 없는 항목 (%1)이 있습니다
ConfirmUninstall=%1 및 해당 구성 요소를 모두 제거하시겠습니까?
UninstallOnlyOnWin64=이 설치는 64비트 Windows에서만 제거할 수 있습니다.
OnlyAdminCanUninstall=이 설치는 관리자 권한이 있는 사용자만 제거할 수 있습니다.
UninstallStatusLabel=%1이(가) 컴퓨터에서 제거되는 동안 기다려 주십시오.
UninstalledAll=%1이(가) 컴퓨터에서 성공적으로 제거되었습니다.
UninstalledMost=%1 제거가 완료되었습니다.%n%n일부 요소를 제거할 수 없습니다. 수동으로 제거할 수 있습니다.
UninstalledAndNeedsRestart=%1 제거를 완료하려면 컴퓨터를 다시 시작해야 합니다.%n%n지금 다시 시작하시겠습니까?
UninstallDataCorrupted="%1" 파일이 손상되었습니다. 제거할 수 없습니다.

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=공유 파일을 제거하시겠습니까?
ConfirmDeleteSharedFile2=시스템에 다음 공유 파일이 더 이상 어떤 프로그램에서도 사용되지 않는 것으로 표시됩니다. 제거에서 이 공유 파일을 제거하시겠습니까?%n%n이 파일을 계속 사용하고 있고 파일이 제거된 프로그램이 있으면 해당 프로그램이 제대로 작동하지 않을 수 있습니다. 확실하지 않은 경우 아니요를 선택합니다. 파일을 시스템에 남겨두어도 아무런 해가 되지 않습니다.
SharedFileNameLabel=파일 이름:
SharedFileLocationLabel=위치:
WizardUninstalling=제거 상태
StatusUninstalling=%1을(를) 제거하는 중...

; *** Shutdown block reasons
ShutdownBlockReasonInstallingApp=%1을(를) 설치하는 중입니다.
ShutdownBlockReasonUninstallingApp=%1을(를) 제거하는 중입니다.

; 아래 사용자 지정 메시지는 설치 프로그램 자체에서 사용하지 않지만
; 스크립트에서 사용할 경우 해당 메시지를 번역할 수 있습니다.

[CustomMessages]

NameAndVersion=%1 버전 %2
AdditionalIcons=바로가기 추가:
CreateDesktopIcon=바탕 화면에 바로가기 만들기(&D)
CreateQuickLaunchIcon=빠른 실행 아이콘 만들기(&Q)
ProgramOnTheWeb=%1 웹페이지
UninstallProgram=%1 제거
LaunchProgram=%1 실행
AssocFileExtension=%1을 %2 파일 확장자에 연결
AssocingFileExtension=%1을 %2 파일 확장자와 연결하는 중...
AutoStartProgramGroupDescription=시작:
AutoStartProgram=%1 자동 시작
AddonHostProgramNotFound=%1을(를) 선택한 폴더에서 찾을 수 없습니다.%n%n계속하시겠습니까?
