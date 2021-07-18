#define MyAppName "Sandboxie-Plus"
;
; use commandline to populate:
; ISCC.exe /ORelease Sandboxie-Plus.iss /DMyAppVersion=%SbiePlusVer% /DMyDrvVersion=%SbieVer% /DMyAppArch=x64 /DMyAppSrc=SbiePlus64
;
;#define MyAppVersion "0.7.5"
;#define MyDrvVersion "5.49.8"
;#define MyAppArch "x64"
;#define MyAppSrc "SbiePlus64"

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppName} v{#MyAppVersion}
AppId=Sandboxie-Plus
AppVersion={#MyAppVersion}
AppPublisher=http://xanasoft.com/
AppPublisherURL=http://sandboxie-plus.com/
AppMutex=SBIEPLUS_MUTEX
DefaultDirName={code:InstallPath}
; Handled in code section as always want DirPage for portable mode.
DisableDirPage=no
DefaultGroupName={#MyAppName}
Uninstallable=not IsPortable
UninstallDisplayIcon={app}\SandMan.exe
OutputBaseFilename={#MyAppName}-{#MyAppArch}-v{#MyAppVersion}
Compression=lzma
ArchitecturesAllowed={#MyAppArch}
ArchitecturesInstallIn64BitMode=x64
AllowNoIcons=yes
AlwaysRestart=no
LicenseFile=.\license.txt
VersionInfoCopyright=Copyright (C) 2020-2021 by David Xanatos (xanasoft.com)
VersionInfoVersion={#MyAppVersion}
;WizardImageFile=WizardImage0.bmp
;WizardSmallImageFile=WizardSmallImage0.bmp

; Allow /CURRENTUSER to be used with /PORTABLE=1 to avoid admin requirement.
PrivilegesRequiredOverridesAllowed=commandline

[Files]
; Both portable and install.
Source: ".\Release\{#MyAppSrc}\*"; DestDir: "{app}"; MinVersion: 0.0,5.0; Flags: recursesubdirs ignoreversion; Excludes: "*.pdb";
; Only if portable.
Source: ".\Sandboxie.ini"; DestDir: "{app}"; Flags: ignoreversion onlyifdoesntexist; Check: IsPortable
Source: ".\Sandboxie-Plus.ini"; DestDir: "{app}"; Flags: ignoreversion onlyifdoesntexist; Check: IsPortable

[Icons]
Name: "{group}\Sandboxie-Plus"; Filename: "{app}\SandMan.exe"; MinVersion: 0.0,5.0;
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "http://sandboxie-plus.com/"; MinVersion: 0.0,5.0;
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"; MinVersion: 0.0,5.0;
Name: "{userdesktop}\Sandboxie-Plus"; Filename: "{app}\SandMan.exe"; Tasks: DesktopIcon; MinVersion: 0.0,5.0;
;Name: "{userstartup}\Sandboxie-Plus"; Filename: "{app}\SandMan.exe"; Tasks: AutoStartEntry;

;[Registry]
;Root: HKCU; Subkey: "Software\{#MyAppName}"; ValueName: "{#MyAppName}_Autorun"; ValueType: string; ValueData: "1"; Flags: uninsdeletekey; Tasks: AutoStartEntry

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "armenian"; MessagesFile: "compiler:Languages\Armenian.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "bulgarian"; MessagesFile: "compiler:Languages\Bulgarian.isl"
Name: "catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "ChineseSimplified"; MessagesFile: "isl\ChineseSimplified.isl"
Name: "ChineseTraditional"; MessagesFile: "isl\ChineseTraditional.isl"
Name: "corsican"; MessagesFile: "compiler:Languages\Corsican.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: "icelandic"; MessagesFile: "compiler:Languages\Icelandic.isl"
Name: "Italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "slovak"; MessagesFile: "compiler:Languages\Slovak.isl"
Name: "slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"

[CustomMessages]
; Note: The prefix of the default language can be omitted.
english.AddSandboxedMenu=Add "Run Sandboxed" to context menu

ChineseSimplified.AddSandboxedMenu=添加“在沙盘中运行”菜单

Italian.AddSandboxedMenu=Aggiungi l'opzione "Avvia nell'area virtuale" al menu contestuale

[Tasks]
Name: "DesktopIcon"; Description: "{cm:CreateDesktopIcon}"; MinVersion: 0.0,5.0; Check: not IsPortable 
Name: "AutoStartEntry"; Description: "{cm:AutoStartProgram,{#MyAppName}}"; MinVersion: 0.0,5.0; Check: not IsPortable 
Name: "AddRunSandboxed"; Description: "{cm:AddSandboxedMenu}"; MinVersion: 0.0,5.0; Check: not IsPortable 

[Messages]
; Include with commandline /? message.
HelpTextNote=/PORTABLE=1%nEnable portable mode.%n

;[Languages]

[CustomMessages]
english.CustomPageLabel1=Select Installation Type
english.CustomPageLabel2=How should be installed
english.CustomPageLabel3=Choose the installation mode
english.CustomPageInstallMode=Install {#MyAppName} on this computer
english.CustomPageUpgradeMode=Update existing {#MyAppName} installation
english.CustomPagePortableMode=Extract all files to a directory for portable use
english.RequiresWin7OrLater=Sandboxie-Plus requires Windows 7 or later.
english.ClassicFound=Sandboxie Classic installation detected, it must be uninstalled first, do you want to uninstall it now?
english.RunSandboxedMenu=Run &Sandboxed

ChineseSimplified.CustomPageLabel1=选择安装方式
ChineseSimplified.CustomPageLabel2=应当如何安装
ChineseSimplified.CustomPageLabel3=选择安装模式
ChineseSimplified.CustomPageInstallMode=在这台计算机上安装 {#MyAppName}
ChineseSimplified.CustomPageUpgradeMode=更新当前已安装的 {#MyAppName}
ChineseSimplified.CustomPagePortableMode=提取全部文件到指定目录供便携化使用
ChineseSimplified.RequiresWin7OrLater=Sandboxie-Plus 需要 Windows 7 或更高版本。
ChineseSimplified.ClassicFound=检测到已安装 Sandboxie 原版，继续安装必须先将其卸载，是否开始卸载？
ChineseSimplified.RunSandboxedMenu=在沙盘中运行(&S)

Italian.CustomPageLabel1=Seleziona tipo di installazione
Italian.CustomPageLabel2=Come dovrebbe essere installato
Italian.CustomPageLabel3=Scegli la modalità di installazione
Italian.CustomPageInstallMode=Installa {#MyAppName} su questo computer
Italian.CustomPageUpgradeMode=Aggiorna l'installazione esistente di {#MyAppName}
Italian.CustomPagePortableMode=Estrai tutti i file in una directory per uso portabile
Italian.RequiresWin7OrLater=Sandboxie-Plus richiede Windows 7 o superiore.
Italian.ClassicFound=È stata rilevata una installazione di Sandboxie Classic che potrebbe causare incompatibilità. Rimuoverla ora?
Italian.RunSandboxedMenu=Avvia nell'area virtuale

[Code]
var
  CustomPage: TInputOptionWizardPage;
  Portable: boolean;
  //w7drv: string;
  //CompatVer: String;
  IsInstalled: boolean;
//  SbiePath: String;


// Return True or False for the value of Check.
function IsPortable(): boolean;
begin
  if (ExpandConstant('{param:portable|0}') = '1') or Portable then begin
    result := True;
  end;
end;

// Return the path to use for the value of DefaultDirName.
function InstallPath(dummy: string): string;
var 
  DrvPath: string;
  SbiePath: string;
begin
  
  //if SbiePath <> '' then begin
  //  result := SbiePath;
  //  exit;
  //end;

  IsInstalled := False;
  SbiePath := ExpandConstant('{param:dir}');
  
  if SbiePath = '' then begin
    if (ExpandConstant('{param:portable|0}') = '1') or Portable then begin
      SbiePath := ExpandConstant('{src}') + '\{#MyAppName}';
    end else begin
      if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Services\SbieDrv', 'ImagePath', DrvPath) then begin
        IsInstalled := True;
        DrvPath := ExtractFilePath(DrvPath);
        if Copy(DrvPath,1,4) = '\??\' then begin
          DrvPath := Copy(DrvPath, 5, Length(DrvPath)-5);
        end;
        SbiePath := DrvPath;
      end else begin
        SbiePath := ExpandConstant('{autopf}') + '\{#MyAppName}';
      end;
    end;
  end;

  result := SbiePath;
end;

// Create the custom page.
// Source: https://timesheetsandstuff.wordpress.com/2008/06/27/the-joy-of-part-2/
procedure InitializeWizard;
begin
  CustomPage := CreateInputOptionPage(wpLicense,
                                      CustomMessage('CustomPageLabel1'),
                                      CustomMessage('CustomPageLabel2'),
                                      CustomMessage('CustomPageLabel3'), True, False);

  if IsInstalled = True then begin
    CustomPage.Add(CustomMessage('CustomPageUpgradeMode'));
  end else begin
    CustomPage.Add(CustomMessage('CustomPageInstallMode'));
  end;
  CustomPage.Add(CustomMessage('CustomPagePortableMode'));

  // Default - Normal Installation if not argument /PORTABLE=1.
  if ExpandConstant('{param:portable|0}') = '1' then begin
    WizardForm.NoIconsCheck.Checked := True;
    CustomPage.SelectedValueIndex := 1;
  end else begin
    CustomPage.SelectedValueIndex := 0;
  end;
end;

// Get mode setting from Custom page and set path for the Dir page.
function NextButtonClick2(CurPageID: integer): boolean;
begin
  if CurPageID = CustomPage.ID then begin
    Portable := not (CustomPage.SelectedValueIndex = 0);
    WizardForm.DirEdit.Text := InstallPath('');
    
    // No Start Menu folder setting on Ready page if portable.
    if Portable then begin
      WizardForm.NoIconsCheck.Checked := True;
    end else begin
      WizardForm.NoIconsCheck.Checked := False;
    end;
  end;
  result := True;
end;


// Detect if already installed.
// Source: https://stackoverflow.com/a/30568071
function IsUpgrade: boolean;
var
  S: string;
  InnoSetupReg: string;
  AppPathName: string;
begin
  InnoSetupReg := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#SetupSetting("AppName")}_is1');
  AppPathName := 'Inno Setup: App Path';
  result := RegQueryStringValue(HKLM, InnoSetupReg, AppPathName, S) or
            RegQueryStringValue(HKCU, InnoSetupReg, AppPathName, S);
end;

// Skip Custom page and Group page if portable.
function ShouldSkipPage(PageID: integer): boolean;
begin
  if PageID = CustomPage.ID then begin
    if ExpandConstant('{param:portable|0}') = '1' then
      result := True;
  end else if PageID = wpSelectDir then begin
    if not IsPortable and IsUpgrade then
      result := True;
  end else if PageID = wpSelectProgramGroup then begin
    if IsPortable then
      result := True;
  end;
end;

//////////////////////////////////////////////////////
// Installation
//

function InitializeSetup(): Boolean;
var
  Version: TWindowsVersion;
  DeleteFlag: Cardinal;
  ExecRet: Integer;
  DrvVersion: Cardinal;
  UninstallString: string;
begin

  //CompatVer := '{#MyDrvVersion}.{#MyAppArch}';

  GetWindowsVersionEx(Version);

  if (Version.NTPlatform = False) or (Version.Major < 6) then
  begin
    SuppressibleMsgBox(CustomMessage('RequiresWin7OrLater'), mbError, MB_OK, MB_OK);
    Result := False;
    exit;
  end;

  ExecRet := IDYES;
  while (ExecRet = IDYES) do
  begin
  
      if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Sandboxie', 'UninstallString', UninstallString) then begin
        
        ExecRet := MsgBox(CustomMessage('ClassicFound'), mbConfirmation, MB_YESNOCANCEL);
        if ExecRet = IDCANCEL then
        begin
          Result := False;
          exit;
        end;

        if ExecRet = IDYES then
        begin
          Exec('cmd.exe', '/c ' + UninstallString, '', SW_HIDE, ewWaitUntilTerminated, ExecRet);
          ExecRet := IDYES;
        end;

      end else begin
        //SuppressibleMsgBox('test', mbError, MB_OK, MB_OK);
        //ExecRet := IDNO;
        break;
      end;
  end;

  Result := True;
end;

function ShutdownSbie(): Boolean;
var
  ExecRet: Integer;
begin

  if(FileExists(ExpandConstant('{app}\kmdutil.exe')) = False) then
  begin
    Result := True;
    exit;
  end;

  Exec(ExpandConstant('{app}\kmdutil.exe'), 'scandll', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
  if(ExecRet <> 0) then
  begin
    Result := False;
    exit;
  end;

  Exec('taskkill', '/IM sandman.exe /F', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
  Exec('taskkill', '/IM sbiectrl.exe /F', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
  Exec('taskkill', '/IM start.exe /F', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);

  Exec(ExpandConstant('{app}\kmdutil.exe'), 'stop SbieSvc', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
  Exec(ExpandConstant('{app}\kmdutil.exe'), 'stop SbieDrv', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);

  Result := True;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
  ExecRet: Integer;
  ErrCode: Integer;
  Version: TWindowsVersion;
begin

  NextButtonClick2(CurPageID);

  if (CurPageID = wpSelectTasks) then
  begin
    //GetWindowsVersionEx(Version);
    //
    //if (Version.NTPlatform = False) or (Version.Major < 8) then
    //begin
    //  ExecRet := MsgBox('Windows 7 requires a provisional driver. You will have to download it from the GitHub release page https://github.com/sandboxie-plus/Sandboxie/releases/' + #13#10 + 'Do you have it already downloaded? Select No to open browser, or Cancel to abort installation.', mbConfirmation, MB_YESNOCANCEL);
    //  if ExecRet = IDCANCEL then
    //  begin
    //    Result := False;
    //    exit;
    //  end;
    //
    //  if ExecRet = IDNO then
    //  begin
    //    ShellExec('open', 'https://github.com/sandboxie-plus/Sandboxie/releases/', '', '', SW_SHOW, ewNoWait, ErrCode);
    //  end;
    //
    //  if (GetOpenFileName('', w7drv, '', 'Driver binary (*.'+CompatVer+'.rc4)|*.'+CompatVer+'.rc4|All Files|*.*', CompatVer + '.rc4') = False) then
    //  begin
    //    Result := False;
    //    exit;
    //  end;
    //end;

  end;

  if ((CurPageID = wpReady) and (not IsPortable())) then
  begin
      Result := ShutdownSbie();
      exit;
  end;

  Result := True;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ExecRet: Integer;
  //params: String;
begin

  // after the instalation
  if (CurStep <> ssPostInstall) then  
    exit;

  //if (w7drv <> '') then
  //begin
  //    DeleteFile(ExpandConstant('{app}\SbieDrv.sys.w10'));
  //    RenameFile(ExpandConstant('{app}\SbieDrv.sys'), ExpandConstant('{app}\SbieDrv.sys.w10'));
  //    FileCopy(w7drv, ExpandConstant('{app}\SbieDrv.sys.rc4'), False);
  //end;

  if (IsPortable()) then  
    exit;

  // install the driver
  Exec(ExpandConstant('{app}\kmdutil.exe'), ExpandConstant('install SbieDrv "{app}\SbieDrv.sys" type=kernel start=demand altitude=86900'), '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
  // install the service
  Exec(ExpandConstant('{app}\kmdutil.exe'), ExpandConstant('install SbieSvc "{app}\SbieSvc.exe" type=own start=auto display="Sandboxie Service" group=UIGroup'), '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);

  Exec(ExpandConstant('{app}\kmdutil.exe'), 'start SbieSvc', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
 
  Exec('reg.exe', 'ADD HKLM\SYSTEM\ControlSet001\Services\SbieSvc /v PreferExternalManifest /t REG_DWORD /d 1 /f', '', SW_HIDE, ewWaitUntilTerminated, ExecRet);

  if WizardIsTaskSelected('AutoStartEntry') then
  begin
    RegWriteStringValue(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Run', 'SandboxiePlus_AutoRun', ExpandConstant('"{app}\SandMan.exe" -autorun'));
  end;

  if WizardIsTaskSelected('AddRunSandboxed') then
  begin
    RegWriteStringValue(HKEY_CURRENT_USER, 'software\classes\*\shell\sandbox', '', CustomMessage('RunSandboxedMenu'));
    RegWriteStringValue(HKEY_CURRENT_USER, 'software\classes\*\shell\sandbox', 'Icon', ExpandConstant('"{app}\start.exe"'));
    RegWriteStringValue(HKEY_CURRENT_USER, 'software\classes\*\shell\sandbox\command', '', ExpandConstant('"{app}\SandMan.exe"') +' /box:__ask__ "%1" %*');

    RegWriteStringValue(HKEY_CURRENT_USER, 'software\classes\Folder\shell\sandbox', '', 'Run &Sandboxed');
    RegWriteStringValue(HKEY_CURRENT_USER, 'software\classes\Folder\shell\sandbox', 'Icon', ExpandConstant('"{app}\start.exe"'));
    RegWriteStringValue(HKEY_CURRENT_USER, 'software\classes\Folder\shell\sandbox\command', '', ExpandConstant('"{app}\SandMan.exe"') +' /box:__ask__ C:\WINDOWS\Explorer.exe "%1"');
  end;

end;

//////////////////////////////////////////////////////
// Uninstallation
//

function InitializeUninstall(): Boolean;
begin
  Result := True;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ExecRet: Integer;
begin
  
  // before the uninstalation
  if (CurUninstallStep <> usUninstall) then
    exit;

  if (ShutdownSbie() = False) then
  begin
    Abort();
    exit;
  end;

  // uninstall the driver
  Exec(ExpandConstant('{app}\kmdutil.exe'), 'delete SbieSvc', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
  Exec(ExpandConstant('{app}\kmdutil.exe'), 'delete SbieDrv', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);

  // remove from autostart
  RegDeleteValue(HKEY_CURRENT_USER, 'Software\Microsoft\Windows\CurrentVersion\Run', 'SandboxiePlus_AutoRun');
  
  // remove shell integration
  RegDeleteValue(HKEY_CURRENT_USER, 'software\classes\*\shell', 'sandbox');
  RegDeleteValue(HKEY_CURRENT_USER, 'software\classes\folder\shell', 'sandbox');

  // delete other left overs
  DeleteFile(ExpandConstant('{app}\SbieDrv.sys.w10'));
  DeleteFile(ExpandConstant('{app}\SbieDrv.sys.rc4'));

end;

