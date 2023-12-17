#define MyAppName "Sandboxie-Plus"
#include "Languages.iss"

; Use commandline to populate:
; ISCC.exe /ORelease Sandboxie-Plus.iss /DMyAppVersion=%SbiePlusVer% /DMyAppArch=x64 /DMyAppSrc=SbiePlus64
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
DefaultGroupName={#MyAppName}
Uninstallable=not IsPortable
UninstallDisplayIcon={app}\SandMan.exe
OutputBaseFilename={#MyAppName}-{#MyAppArch}-v{#MyAppVersion}
Compression=lzma
ArchitecturesAllowed={#MyAppArch}
ArchitecturesInstallIn64BitMode=x64 arm64
AllowNoIcons=yes
AlwaysRestart=no
LicenseFile=license.txt
UsedUserAreasWarning=no
VersionInfoCopyright=Copyright (C) 2020-2023 by David Xanatos (xanasoft.com)
VersionInfoVersion={#MyAppVersion}
SetupIconFile=SandManInstall.ico

; Handled in code section as always want DirPage for portable mode.
DisableDirPage=no

; Allow /CURRENTUSER to be used with /PORTABLE=1 to avoid admin requirement.
PrivilegesRequiredOverridesAllowed=commandline


[Tasks]
Name: "DesktopIcon"; Description: "{cm:CreateDesktopIcon}"; MinVersion: 0.0,5.0; Check: not IsPortable
;Name: "DesktopIcon2"; Description: "{cm:AddSandboxedBrowser}"; MinVersion: 0.0,5.0; Check: not IsPortable
;Name: "AutoStartEntry"; Description: "{cm:AutoStartProgram,{#MyAppName}}"; MinVersion: 0.0,5.0; Check: not IsPortable
;Name: "AddRunSandboxed"; Description: "{cm:AddSandboxedMenu}"; MinVersion: 0.0,5.0; Check: not IsPortable
Name: "RefreshBuild"; Description: "{cm:RefreshBuild}"; MinVersion: 0.0,5.0; Check: not IsPortable


[Files]
; Both portable and install.
Source: ".\Release\{#MyAppSrc}\*"; DestDir: "{app}"; MinVersion: 0.0,5.0; Flags: recursesubdirs ignoreversion; Excludes: "*.pdb"
; include the driver pdb
Source: ".\Release\{#MyAppSrc}\SbieDrv.pdb"; DestDir: "{app}"; MinVersion: 0.0,5.0; Flags: ignoreversion
Source: ".\Release\{#MyAppSrc}\SbieDll.pdb"; DestDir: "{app}"; MinVersion: 0.0,5.0; Flags: ignoreversion

; Only if portable.
Source: ".\Sandboxie.ini"; DestDir: "{app}"; Flags: ignoreversion onlyifdoesntexist; Check: IsPortable
Source: ".\Sandboxie-Plus.ini"; DestDir: "{app}"; Flags: ignoreversion onlyifdoesntexist; Check: IsPortable


[Icons]
Name: "{group}\Sandboxie-Plus"; Filename: "{app}\SandMan.exe"; MinVersion: 0.0,5.0
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "http://sandboxie-plus.com/"; MinVersion: 0.0,5.0
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"; MinVersion: 0.0,5.0
Name: "{group}\{cm:SandboxieStartMenu1}"; Filename: "{app}\Start.exe"; Parameters: "/box:__ask__ run_dialog"; MinVersion: 0.0,5.0
Name: "{group}\{cm:SandboxieStartMenu2}"; Filename: "{app}\Start.exe"; Parameters: "default_browser"; MinVersion: 0.0,5.0
Name: "{group}\{cm:SandboxieStartMenu3}"; Filename: "{app}\Start.exe"; Parameters: "/box:__ask__ start_menu"; MinVersion: 0.0,5.0
Name: "{userdesktop}\Sandboxie-Plus"; Filename: "{app}\SandMan.exe"; Tasks: DesktopIcon; MinVersion: 0.0,5.0
;Name: "{userdesktop}\{cm:SandboxedBrowser}"; Filename: "{app}\Start.exe"; Parameters: "default_browser"; Tasks: DesktopIcon2; MinVersion: 0.0,5.0


[INI]
; Set Sandman language.
Filename: "{localappdata}\{#MyAppName}\{#MyAppName}.ini"; Section: "Options"; Key: "UiLanguage"; String: "{code:SandmanLanguage|{language}}"; Check: (not IsPortable) and (not IsUpgrade)
Filename: "{app}\{#MyAppName}.ini"; Section: "Options"; Key: "UiLanguage"; String: "{code:SandmanLanguage|{language}}"; Check: IsPortable


[InstallDelete]
; Remove deprecated files at install time.
Type: filesandordirs; Name: "{app}\translations"
Type: files; Name: "{app}\SbieDrv.sys.w10"
Type: files; Name: "{app}\SbieDrv.sys.rc4"
Type: files; Name: "{app}\SbieIni.exe.sig"


[Registry]
; Autostart Sandman.
;Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueName: "SandboxiePlus_AutoRun"; ValueType: string; ValueData: """{app}\SandMan.exe"" -autorun"; Flags: uninsdeletevalue; Tasks: AutoStartEntry

; AddRunSandboxed all files.
;Root: HKCU; Subkey: "Software\Classes\*\shell"; Flags: uninsdeletekeyifempty; Tasks: AddRunSandboxed
;Root: HKCU; Subkey: "Software\Classes\*\shell\sandbox"; ValueName: ""; ValueType: string; ValueData: "{cm:RunSandboxedMenu}"; Flags: uninsdeletekey; Tasks: AddRunSandboxed
;Root: HKCU; Subkey: "Software\Classes\*\shell\sandbox"; ValueName: "Icon"; ValueType: string; ValueData: """{app}\start.exe"""; Tasks: AddRunSandboxed
;Root: HKCU; Subkey: "Software\Classes\*\shell\sandbox\command"; ValueName: ""; ValueType: string; ValueData: """{app}\SandMan.exe"" /box:__ask__ ""%1"" %*"; Tasks: AddRunSandboxed

; AddRunSandboxed folder.
;Root: HKCU; Subkey: "Software\Classes\Folder\shell"; Flags: uninsdeletekeyifempty; Tasks: AddRunSandboxed
;Root: HKCU; Subkey: "Software\Classes\Folder\shell\sandbox"; ValueName: ""; ValueType: string; ValueData: "{cm:RunSandboxedMenu}"; Flags: uninsdeletekey; Tasks: AddRunSandboxed
;Root: HKCU; Subkey: "Software\Classes\Folder\shell\sandbox"; ValueName: "Icon"; ValueType: string; ValueData: """{app}\start.exe"""; Tasks: AddRunSandboxed
;Root: HKCU; Subkey: "Software\Classes\Folder\shell\sandbox\command"; ValueName: ""; ValueType: string; ValueData: """{app}\SandMan.exe"" /box:__ask__ ""%1"" %*"; Tasks: AddRunSandboxed

; External manifest for Sbie service.
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\SbieSvc"; ValueName: "PreferExternalManifest"; ValueType: dword; ValueData: "1"; Check: not IsPortable

; Set language for Sbie service.
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\SbieSvc"; ValueType: dword; ValueName: "Language"; ValueData: "{code:SystemLanguage}"; Check: not IsPortable

; Set IniPath for Sbie driver.
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\SbieDrv"; ValueType: string; ValueName: "IniPath"; ValueData: "{code:GetIniPath}"; Check: IsUpgrade and IsIniPathSet


[Run]
; Install the Sbie driver.
Filename: "{app}\KmdUtil.exe"; Parameters: "install SbieDrv ""{app}\SbieDrv.sys"" type=kernel start=demand msgfile=""{app}\SbieMsg.dll"" altitude=86900"; StatusMsg: "KmdUtil install SbieDrv..."; Check: not IsPortable

; Install the Sbie service.
Filename: "{app}\KmdUtil.exe"; Parameters: "install SbieSvc ""{app}\SbieSvc.exe"" type=own start=auto msgfile=""{app}\SbieMsg.dll"" display=""Sandboxie Service"" group=UIGroup"; StatusMsg: "KmdUtil install SbieSvc..."; Check: not IsPortable

; Update metadata (templates and translations)
Filename: "{app}\UpdUtil.exe"; Parameters: {code:GetParams}; StatusMsg: "UpdUtill checking for updates..."; Check: IsRefresh

; Start the Sbie service.
Filename: "{app}\KmdUtil.exe"; Parameters: "start SbieSvc"; StatusMsg: "KmdUtil start SbieSvc"; Check: not IsPortable

; Start the Sandman UI.
Filename: "{app}\Start.exe"; Parameters: "open_agent:sandman.exe"; Description: "Start Sandboxie-Plus UI"; StatusMsg: "Launch SandMan UI..."; Flags: postinstall nowait; Check: IsOpenSandMan
;Filename: "{app}\SandMan.exe"; Parameters: "-autorun"; StatusMsg: "Launch SandMan UI..."; Flags: runasoriginaluser nowait; Check: not IsPortable


[UninstallDelete]
Type: dirifempty; Name: "{app}"
Type: dirifempty; Name: "{localappdata}\{#MyAppName}"


[Messages]
; Include with commandline /? message.
HelpTextNote=/PORTABLE=1%nEnable portable mode.%n


[Code]
var
  CustomPage: TInputOptionWizardPage;
  IsInstalled: Boolean;
  Portable: Boolean;
  IniPathExist: Boolean;
  IniPath: String;

function IsPortable(): Boolean;
begin

  // Return True or False for the value of Check.
  if (ExpandConstant('{param:portable|0}') = '1') or Portable then begin
    Result := True;
  end;
end;

function IsOpenSandMan(): Boolean;
begin

  // Return True or False for the value of Check.
  if (ExpandConstant('{param:open_agent|0}') = '1') or ((not IsPortable) and (not WizardSilent)) then begin
    Result := True;
  end;
end;

function IsUpgrade(): Boolean;
var
  S: String;
  InnoSetupReg: String;
  AppPathName: String;
begin

  // Detect if already installed.
  // Source: https://stackoverflow.com/a/30568071
  InnoSetupReg := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#SetupSetting("AppName")}_is1');
  AppPathName := 'Inno Setup: App Path';

  Result := RegQueryStringValue(HKLM, InnoSetupReg, AppPathName, S) or
            RegQueryStringValue(HKCU, InnoSetupReg, AppPathName, S);
end;


function InstallPath(Dummy: String): String;
var
  DrvPath: String;
  SbiePath: String;
begin

  // Return the path to use for the value of DefaultDirName.
  IsInstalled := False;
  SbiePath := ExpandConstant('{param:dir}');

  if SbiePath = '' then begin
    if (ExpandConstant('{param:portable|0}') = '1') or Portable then begin
      SbiePath := ExpandConstant('{src}') + '\{#MyAppName}';
    end else begin
      if RegQueryStringValue(HKLM, 'SYSTEM\CurrentControlSet\Services\SbieDrv', 'ImagePath', DrvPath) then begin
        IsInstalled := True;
        DrvPath := ExtractFilePath(DrvPath);

        if Copy(DrvPath, 1, 4) = '\??\' then begin
          DrvPath := Copy(DrvPath, 5, Length(DrvPath) - 5);
        end;

        SbiePath := DrvPath;
      end else begin
        SbiePath := ExpandConstant('{autopf}') + '\{#MyAppName}';
      end;
    end;
  end;

  Result := SbiePath;
end;


function SandmanLanguage(Language: String): String;
begin

  // Language values for Sandboxie-Plus.ini.
  case Lowercase(Language) of
    'english': Result := 'en';
    'chinesesimplified': Result := 'zh_CN';
    'chinesetraditional': Result := 'zh_TW';
    'dutch': Result := 'nl';
    'french': Result := 'fr';
    'german': Result := 'de';
    'hungarian': Result := 'hu';
    'italian': Result := 'it';
    'korean': Result := 'ko';
    'polish': Result := 'pl';
    'brazilianportuguese': Result := 'pt_BR';
    'portuguese': Result := 'pt_PT';
    'russian': Result := 'ru';
    'spanish': Result := 'es';
    'swedish': Result := 'sv_SE';
    'turkish': Result := 'tr';
    'ukrainian': Result := 'uk';
    'vietnamese': Result := 'vi';
  end;
end;


function SystemLanguage(Dummy: String): String;
begin

  // Language identifier for the System Eventlog messages.
  Result := IntToStr(GetUILanguage());
end;


procedure UpdateStatus(OutputProgressPage: TOutputProgressWizardPage; Text: String; Percentage: Integer);
begin

  // Called by ShutdownSbie() to update status or progress.
  if IsUninstaller() then
    UninstallProgressForm.StatusLabel.Caption := Text
  else begin
    OutputProgressPage.SetProgress(Percentage, 100);
    OutputProgressPage.SetText(Text, '');
  end;

  // Output status information to log.
  Log('Debug: ' + Text);
end;


function ShutdownSbie(): Boolean;
var
  ExecRet: Integer;
  StatusText: String;
  OutputProgressPage: TOutputProgressWizardPage;
begin

  // Require KmdUtil.exe to continue.
  if (FileExists(ExpandConstant('{app}\KmdUtil.exe')) = False) then
  begin
    Result := True;
    exit;
  end;

  try

    // Backup status text (uninstall). Prepare progress page (install).
    if IsUninstaller() then
      StatusText := UninstallProgressForm.StatusLabel.Caption
    else begin
      OutputProgressPage := CreateOutputProgressPage(SetupMessage(msgWizardPreparing), SetupMessage(msgPreparingDesc));
      OutputProgressPage.Show();
    end;

    // Run KmdUtil scandll.
    UpdateStatus(OutputProgressPage, 'KmdUtil scandll', 5);
    Exec(ExpandConstant('{app}\KmdUtil.exe'), 'scandll_silent', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);

    if (ExecRet <> 0) then
    begin
      Result := False;
      exit;
    end;

    // Stop service and driver.
    UpdateStatus(OutputProgressPage, 'KmdUtil stop SbieSvc', 55);
    Exec(ExpandConstant('{app}\KmdUtil.exe'), 'stop SbieSvc', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);

    UpdateStatus(OutputProgressPage, 'KmdUtil stop SbieDrv', 85);
    Exec(ExpandConstant('{app}\KmdUtil.exe'), 'stop SbieDrv', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);

    // Uninstall service and driver.
    UpdateStatus(OutputProgressPage, 'KmdUtil delete SbieSvc', 95);
    Exec(ExpandConstant('{app}\KmdUtil.exe'), 'delete SbieSvc', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);

    UpdateStatus(OutputProgressPage, 'KmdUtil delete SbieDrv', 100);
    Exec(ExpandConstant('{app}\KmdUtil.exe'), 'delete SbieDrv', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);

    // Query driver which can remove SbieDrv key if exist.
    if RegKeyExists(HKLM, 'SYSTEM\CurrentControlSet\services\SbieDrv') then begin
      UpdateStatus(OutputProgressPage, 'SC query SbieDrv', 100);
      Exec(ExpandConstant('{sys}\sc.exe'), 'query SbieDrv', '', SW_HIDE, ewWaitUntilTerminated, ExecRet);
    end;
  finally

    // Restore status text (uninstall). Hide Prepare progress page (install).
    if IsUninstaller() then
      UninstallProgressForm.StatusLabel.Caption := StatusText
    else begin
      OutputProgressPage.SetProgress(0, 100);
      OutputProgressPage.Hide();
    end;
  end;

  Result := True;
end;


//////////////////////////////////////////////////////
// Installation Events
//


function NextButtonClick(CurPageID: Integer): Boolean;
var
  ExecRet: Integer;
begin

  // Get mode setting from Custom page and set path for the Dir page.
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

  // Shutdown service, driver and processes as ready to install.
  if ((CurPageID = wpReady) and (not IsPortable())) then
  begin

    // Stop processes.
    Exec(ExpandConstant('{sys}\taskkill.exe'), '/IM Sandman.exe /IM SbieCtrl.exe /IM Start.exe /F', '', SW_HIDE, ewWaitUntilTerminated, ExecRet);

    Result := ShutdownSbie();
    exit;
  end;

  Result := True;
end;


function ShouldSkipPage(PageID: Integer): Boolean;
begin

  // Skip Custom page and Group page if portable.
  if PageID = CustomPage.ID then begin
    if ExpandConstant('{param:portable|0}') = '1' then
      Result := True;
  end else if PageID = wpSelectDir then begin
    if not IsPortable and IsUpgrade then
      Result := True;
  end else if PageID = wpSelectProgramGroup then begin
    if IsPortable then
      Result := True;
  end;
end;


procedure InitializeWizard();
begin

  // Create the custom page.
  // Source: https://timesheetsandstuff.wordpress.com/2008/06/27/the-joy-of-part-2/
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

  // Default to Normal Installation if not argument /PORTABLE=1.
  if ExpandConstant('{param:portable|0}') = '1' then begin
    WizardForm.NoIconsCheck.Checked := True;
    CustomPage.SelectedValueIndex := 1;
  end else begin
    CustomPage.SelectedValueIndex := 0;
  end;
end;


function InitializeSetup(): Boolean;
var
  Version: TWindowsVersion;
  ExecRet: Integer;
  UninstallString: String;
begin

  // Require Windows 7 or later.
  GetWindowsVersionEx(Version);

  if (Version.NTPlatform = False) or (Version.Major < 6) then
  begin
    SuppressibleMsgBox(CustomMessage('RequiresWin7OrLater'), mbError, MB_OK, MB_OK);
    Result := False;
    exit;
  end;

  // Ask to uninstall Sandboxie Classic if found.
  ExecRet := IDYES;

  while (ExecRet = IDYES) do
  begin
      if RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Sandboxie', 'UninstallString', UninstallString) then begin

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
        break;
      end;
  end;

  begin
  
    // Return the path to use for the value of IniPath.
    if RegQueryStringValue(HKLM, 'SYSTEM\CurrentControlSet\Services\SbieDrv', 'IniPath', IniPath) then
    begin
      if Copy(IniPath, 1, 4) = '\??\' then
      begin
        IniPathExist := True;
      end;
    end;
  end;

  Result := True;
end;

function IsIniPathSet(): Boolean;
begin

  // Return True or False for the value of Check.
  if (IniPathExist) then
  begin
      Result := True;
  end;
end;

function GetIniPath(Dummy: String): String;
begin

  // Return the path to use for the value of IniPath.
  if (IniPathExist) then
  begin
      Result := IniPath;
  end;
end;

//procedure CurStepChanged(CurStep: TSetupStep);
//var
//  ExecRet: Integer;
//  //params: String;
//begin
//
//  // after the installation
//  if (CurStep <> ssPostInstall) then
//    exit;
//
//  if WizardIsTaskSelected('RefreshBuild') then
//  begin
//    SuppressibleMsgBox('test', mbError, MB_OK, MB_OK);
//  end;
//
//end;

function IsRefresh(): Boolean;
begin

  if WizardIsTaskSelected('RefreshBuild') then begin
    Result := True;
  end;
end;

function GetParams(Value: string): string;
begin
  if IsInstalled = True then begin
    Result := 'upgrade sandboxie-plus /embedded /scope:meta /version:{#MyAppVersion}';
  end else begin
    Result := 'install sandboxie-plus /embedded /scope:meta /version:{#MyAppVersion}';
  end;
end;


//////////////////////////////////////////////////////
// Uninstallation Exclusive
//


procedure UninstallCleanup(ButtonCount: Integer);
var
  Buttons: Cardinal;
  ButtonLabels: TArrayOfString;
  ExecRet: Integer;
  I: Integer;
  Paths: TStringList;
  ShowMsgbox: Boolean;
  TaskRet: Integer;
begin

  // Require 2 or 3 for button count.
  if (ButtonCount < 2) or (ButtonCount > 3) then
    exit;

  // Require Sandman.exe to continue.
  //if not FileExists(ExpandConstant('{app}\Sandman.exe')) then
  //  exit;

  // Make a list.
  Paths := TStringList.Create;

  // Append file paths to the list for removal.
  Paths.Append('{localappdata}\{#MyAppName}\{#MyAppName}.ini');
  Paths.Append('{win}\Sandboxie.ini');
  Paths.Append('{app}\{#MyAppName}.ini');
  Paths.Append('{app}\Sandboxie.ini');

  // Expand paths and detect if any file exist.
  for I := 0 to Paths.Count - 1 do begin
    Paths[I] := ExpandConstant(Paths[I]);

    if FileExists(Paths[I]) then
      ShowMsgbox := True;
  end;

  // Delete the config files and the sandboxes.
  if ShowMsgbox then begin
    case ButtonCount of
      2: begin
        Buttons := MB_YESNO;
        ButtonLabels := [CustomMessage('UninstallTaskLabel3'),
                         CustomMessage('UninstallTaskLabel4')];
      end;

      3: begin
        Buttons := MB_ABORTRETRYIGNORE;
        ButtonLabels := [CustomMessage('UninstallTaskLabel3'),
                         CustomMessage('UninstallTaskLabel4'),
                         CustomMessage('UninstallTaskLabel5')];
      end;
    end;

    case TaskDialogMsgBox(CustomMessage('UninstallTaskLabel1'),
                          CustomMessage('UninstallTaskLabel2'),
                          mbConfirmation, Buttons, ButtonLabels, 0) of

      IDRETRY: TaskRet := 1;
      IDIGNORE: TaskRet := 2;
      IDABORT: TaskRet := 3;
      IDYES: TaskRet := 1;
      IDNO: TaskRet := 2;
    end;

    if TaskRet > 2 then begin
      Log('Debug: Start terminate_all');
      Exec(ExpandConstant('{app}\start.exe'), '/terminate_all', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
    end;

    if TaskRet > 2 then begin
      Log('Debug: Start delete_all_sandboxes');
      Exec(ExpandConstant('{app}\start.exe'), 'delete_all_sandboxes', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
    end;

    if TaskRet > 1 then begin
      for I := 0 to Paths.Count - 1 do
        if FileExists(Paths[I]) then begin
          Log('Debug: DeleteFile(' + Paths[I] + ')');
          DeleteFile(Paths[I]);
        end;
    end;
  end;

  // Release the list.
  Paths.Free;
end;

procedure ShellUninstall();
var
  ExecRet: Integer;
begin

  if FileExists(ExpandConstant('{app}\Sandman.exe')) then begin
    Log('Debug: SandMan /ShellUninstall');
    Exec(ExpandConstant('{app}\Sandman.exe'), '/ShellUninstall', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
  end else begin
    Log('Debug: SbieCtrl /uninstall');
    Exec(ExpandConstant('{app}\sbiectrl.exe'), '/uninstall', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ExecRet);
  end;
end;


//////////////////////////////////////////////////////
// Uninstallation Events
//


procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ExecRet: Integer;
begin

  // Before the uninstallation.
  if (CurUninstallStep <> usUninstall) then
    exit;

  // Stop processes.
  Exec(ExpandConstant('{sys}\taskkill.exe'), '/IM Sandman.exe /IM SbieCtrl.exe /IM Start.exe /F', '', SW_HIDE, ewWaitUntilTerminated, ExecRet);

  // User to confirm extra files to remove.
  if not UninstallSilent then
    UninstallCleanup(3);

  // Shutdown service, driver and processes.
  if (ShutdownSbie() = False) then
  begin
    Abort();
    exit;
  end;

  // remove shell integration.
  ShellUninstall();

end;
