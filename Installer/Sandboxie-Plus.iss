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
Name: "{group}\Sandboxie-Plus Website"; Filename: "http://sandboxie-plus.com/"; MinVersion: 0.0,5.0;
Name: "{group}\Uninstall Sandboxie-Plus"; Filename: "{uninstallexe}"; MinVersion: 0.0,5.0;
Name: "{userdesktop}\Sandboxie-Plus"; Filename: "{app}\SandMan.exe"; Tasks: DesktopIcon; MinVersion: 0.0,5.0;
;Name: "{userstartup}\Sandboxie-Plus"; Filename: "{app}\SandMan.exe"; Tasks: AutoStartEntry;

;[Registry]
;Root: HKCU; Subkey: "Software\{#MyAppName}"; ValueName: "{#MyAppName}_Autorun"; ValueType: string; ValueData: "1"; Flags: uninsdeletekey; Tasks: AutoStartEntry

[Tasks]
Name: "DesktopIcon"; Description: "Create a &desktop icon"; MinVersion: 0.0,5.0; Check: not IsPortable 
Name: "AutoStartEntry"; Description: "Start when Windows starts"; MinVersion: 0.0,5.0; Check: not IsPortable 
Name: "AddRunSandboxed"; Description: "Add Run Sandboxed"; MinVersion: 0.0,5.0; Check: not IsPortable 

[Messages]
; Include with commandline /? message.
HelpTextNote=/PORTABLE=1%nEnable portable mode.%n

;[Languages]

[CustomMessages]
CustomPageLabel1=Select Installation Type
CustomPageLabel2=How should be installed
CustomPageLabel3=Choose the installation mode
CustomPageInstallMode=Install {#MyAppName} on this computer
CustomPageUpgradeMode=Update existing {#MyAppName} installation
CustomPagePortableMode=Extract all files to a directory for portable use


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
    SuppressibleMsgBox('Sandboxie-Plus requires Windows 7 or later.', mbError, MB_OK, MB_OK);
    Result := False;
    exit;
  end;

  ExecRet := IDYES;
  while (ExecRet = IDYES) do
  begin
  
      if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Sandboxie', 'UninstallString', UninstallString) then begin
        
        ExecRet := MsgBox('Sandboxie Classic installation detected, it must be uninstalled first, do you want to uninstall it now?', mbConfirmation, MB_YESNOCANCEL);
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
    RegWriteStringValue(HKEY_CURRENT_USER, 'software\classes\*\shell\sandbox', '', 'Run &Sandboxed');
    RegWriteStringValue(HKEY_CURRENT_USER, 'software\classes\*\shell\sandbox', 'Icon', ExpandConstant('"{app}\start.exe"'));
    RegWriteStringValue(HKEY_CURRENT_USER, 'software\classes\*\shell\sandbox\command', '', ExpandConstant('"{app}\start.exe"') +' /box:__ask__ "%1" %*');
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

