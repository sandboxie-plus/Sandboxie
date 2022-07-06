; *** Inno Setup version 6.1.0+ Swedish messages ***
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/files/istrans/
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; Translated by stefan@bodingh.se (Stefan Bodingh)
;

; The following three entries are very important. Be sure to read and 
; understand the '[LangOptions] section' topic in the help file.


[LangOptions]
LanguageName=Svenska	
LanguageID=$041D
LanguageCodePage=1252
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=
;DialogFontSize=8
;WelcomeFontName=Verdana
;WelcomeFontSize=12
;TitleFontName=Arial
;TitleFontSize=29
;CopyrightFontName=Arial
;CopyrightFontSize=8


; *** Application titles


[Messages]
SetupAppTitle=Installationsprogram
SetupWindowTitle=Installationsprogram för %1
UninstallAppTitle=Avinstallation
UninstallAppFullTitle=%1 Avinstallation

; *** Misc. common


InformationTitle=Information
ConfirmTitle=Bekräfta
ErrorTitle=Fel

; *** SetupLdr messages


SetupLdrStartupMessage=%1 kommer att installeras. Vill du fortsätta?
LdrCannotCreateTemp=Kan inte skapa en temporär fil. Installationen avbryts
LdrCannotExecTemp=Kan inte köra fil i temporär katalog. Installationen avbryts
HelpTextNote=

; *** Startup error messages


LastErrorMessage=%1.%n%nFel %2: %3
SetupFileMissing=Filen %1 saknas i installationskatalogen. Rätta till problemet eller hämta en ny kopia av programmet.
SetupFileCorrupt=Installationsfilerna är felaktiga. Hämta en ny kopia av programmet
SetupFileCorruptOrWrongVer=Installationsfilerna är felaktiga, eller stämmer ej överens med denna version av installationsprogrammet. Rätta till felet eller hämta en ny programkopia.
InvalidParameter=En ogiltig parameter angavs på kommandoraden:%n%n%1
SetupAlreadyRunning=Setup körs redan.
WindowsVersionNotSupported=Programmet stödjer inte den version av Windows som körs på datorn.
WindowsServicePackRequired=Programmet kräver %1 Service Pack %2 eller nyare.
NotOnThisPlatform=Detta program kan ej köras på %1.
OnlyOnThisPlatform=Detta program måste ha %1.
OnlyOnTheseArchitectures=Detta program kan bara installeras på Windows versioner med följande processorarkitekturer:%n%n%1
WinVersionTooLowError=Detta program kräver %1, version %2 eller senare.
WinVersionTooHighError=Programmet kan inte installeras på %1 version %2 eller senare.
AdminPrivilegesRequired=Du måste vara inloggad som administratör när du installerar detta program.
PowerUserPrivilegesRequired=Du måste vara inloggad som administratör eller medlem av gruppen Privilegierade användare (Power Users) när du installerar  detta program.
SetupAppRunningError=Installationsprogrammet har upptäckt att %1 är igång.%n%nAvsluta det angivna programmet nu. Klicka sedan på OK för att gå vidare, eller på Avbryt för att avsluta.
UninstallAppRunningError=Avinstalleraren har upptäckt att %1 körs för tillfället.%n%nStäng all öppna instanser av det nu, klicka sedan på OK för att gå vidare, eller på Avbryt för att avsluta.
PrivilegesRequiredOverrideTitle=Installationstyp
PrivilegesRequiredOverrideInstruction=Välj installationstyp
PrivilegesRequiredOverrideText1=%1 kan installeras för alla användare (kräver administratons-rättigheter), eller bara för dig.
PrivilegesRequiredOverrideText2=%1 kan installeras bara för dig, eller för alla användare (kräver administratons-rättigheter).
PrivilegesRequiredOverrideAllUsers=Installera för &alla användare
PrivilegesRequiredOverrideAllUsersRecommended=Installera för &alla användare (rekommenderas)
PrivilegesRequiredOverrideCurrentUser=Installera för &mig enbart
PrivilegesRequiredOverrideCurrentUserRecommended=Installera för &mig enbart (rekommenderas)

; *** Misc. errors


ErrorCreatingDir=Kunde inte skapa katalogen "%1"
ErrorTooManyFilesInDir=Kunde inte skapa en fil i katalogen "%1" därför att den innehåller för många filer

; *** Setup common messages


ExitSetupTitle=Avsluta installationen
ExitSetupMessage=Installationen är inte färdig. Om du avslutar nu, kommer programmet inte att installeras.%n%nDu kan köra installationsprogrammet  vid ett senare tillfälle för att slutföra installationen.%n%nVill du avbryta installationen?
AboutSetupMenuItem=&Om installationsprogrammet...
AboutSetupTitle=Om installationsprogrammet
AboutSetupMessage=%1 version %2%n%3%n%n%1 hemsida:%n%4
AboutSetupNote=Svensk översättning är gjord av dickg@go.to 1999, 2002%n%nUppdatering till 3.0.2+ av peter@peterandlinda.com, 4.+ av stefan@bodingh.se
TranslatorNote=

; *** Buttons


ButtonBack=< &Tillbaka
ButtonNext=&Nästa >
ButtonInstall=&Installera
ButtonOK=OK
ButtonCancel=Avbryt
ButtonYes=&Ja
ButtonYesToAll=Ja till &Allt
ButtonNo=&Nej
ButtonNoToAll=N&ej till allt
ButtonFinish=&Slutför
ButtonBrowse=&Bläddra...
ButtonWizardBrowse=&Bläddra...
ButtonNewFolder=Skapa ny katalog

; *** "Select Language" dialog messages


SelectLanguageTitle=Välj språk för installationen
SelectLanguageLabel=Välj språk som skall användas under installationen:

; *** Common wizard text


ClickNext=Klicka på Nästa för att fortsätta eller på Avbryt för att avsluta installationen.
BeveledLabel=
BrowseDialogTitle=Välj katalog
BrowseDialogLabel=Välj en katalog i listan nedan, klicka sedan på OK.
NewFolderName=Ny katalog

; *** "Welcome" wizard page


WelcomeLabel1=Välkommen till installationsprogrammet för [name].
WelcomeLabel2=Detta kommer att installera [name/ver] på din dator.%n%nDet rekommenderas att du avslutar alla andra program innan du fortsätter. Det förebygger konflikter under installationens gång.

; *** "Password" wizard page


WizardPassword=Lösenord
PasswordLabel1=Denna installation är skyddad med lösenord.
PasswordLabel3=Var god ange lösenordet, klicka sedan på Nästa för att fortsätta. Lösenord skiljer på versaler/gemener.
PasswordEditLabel=&Lösenord:
IncorrectPassword=Lösenordet du angav är inkorrekt. Försök igen.

; *** "License Agreement" wizard page





WizardLicense=Licensavtal
LicenseLabel=Var god och läs följande viktiga information innan du fortsätter.
LicenseLabel3=Var god och läs följande licensavtal. Du måste acceptera villkoren i avtalet innan du kan fortsätta med installationen.
LicenseAccepted=Jag &accepterar avtalet
LicenseNotAccepted=Jag accepterar &inte avtalet

; *** "Information" wizard pages





WizardInfoBefore=Information
InfoBeforeLabel=Var god läs följande viktiga information innan du fortsätter.
InfoBeforeClickLabel=När du är klar att fortsätta med installationen klickar du på Nästa.
WizardInfoAfter=Information
InfoAfterLabel=Var god läs följande viktiga information innan du fortsätter.
InfoAfterClickLabel=När du är klar att fortsätta med installationen klickar du på Nästa.

; *** "User Information" wizard page





WizardUserInfo=Användarinformation
UserInfoDesc=Var god och fyll i följande uppgifter.
UserInfoName=&Namn:
UserInfoOrg=&Organisation:
UserInfoSerial=&Serienummer:
UserInfoNameRequired=Du måste fylla i ett namn.

; *** "Select Destination Directory" wizard page





WizardSelectDir=Välj installationsplats
SelectDirDesc=Var skall [name] installeras?
SelectDirLabel3=Installationsprogrammet kommer att installera [name] i följande katalog
SelectDirBrowseLabel=För att fortsätta klickar du på Nästa. Om du vill välja en annan katalog klickar du på Bläddra.
DiskSpaceGBLabel=Programmet kräver minst [gb] MB hårddiskutrymme.
DiskSpaceMBLabel=Programmet kräver minst [mb] MB hårddiskutrymme.
CannotInstallToNetworkDrive=Setup kan inte installeras på nätverksdisk.
CannotInstallToUNCPath=Setup kan inte installeras på UNC sökväg.
InvalidPath=Du måste skriva en fullständig sökväg med enhetsbeteckning; till exempel:%n%nC:\Program%n%neller en UNC-sökväg i formatet:%n%n\\server\resurs
InvalidDrive=Enheten du har valt finns inte eller är inte tillgänglig. Välj en annan.
DiskSpaceWarningTitle=Ej tillräckligt med diskutrymme
DiskSpaceWarning=Installationsprogrammet behöver åtminstone %1 KB ledigt diskutrymme för installationen, men den valda enheten har bara %2  KB tillgängligt.%n%nVill du fortsätta ändå?
DirNameTooLong=Katalogens namn eller sökväg är för lång.
InvalidDirName=Katalogen du har valt är inte tillgänglig.
BadDirName32=Katalogens namn får ej innehålla något av följande tecken:%n%n%1
DirExistsTitle=Katalogen finns
DirExists=Katalogen:%n%n%1%n%nfinns redan. Vill du ändå fortsätta installationen till den valda katalogen?
DirDoesntExistTitle=Katalogen finns inte
DirDoesntExist=Katalogen:%n%n%1%n%nfinns inte. Vill du skapa den?

; *** "Select Components" wizard page





WizardSelectComponents=Välj komponenter
SelectComponentsDesc=Vilka komponenter skall installeras?
SelectComponentsLabel2=Välj de komponenter som du vill ska installeras; avmarkera de komponenter som du inte vill ha. Klicka sedan på Nästa när du är klar att  fortsätta.
FullInstallation=Fullständig installation
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)





CompactInstallation=Kompakt installation
CustomInstallation=Anpassad installation
NoUninstallWarningTitle=Komponenter finns
NoUninstallWarning=Installationsprogrammet har upptäckt att följande komponenter redan finns installerade på din dator:%n%n%1%n%nAtt  avmarkera dessa komponenter kommer inte att avinstallera dom.%n%nVill du fortsätta ändå?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceGBLabel=Aktuella val kräver minst [gb] GB diskutrymme.
ComponentsDiskSpaceMBLabel=Aktuella val kräver minst [mb] MB diskutrymme.

; *** "Select Additional Tasks" wizard page





WizardSelectTasks=Välj extra uppgifter
SelectTasksDesc=Vilka extra uppgifter skall utföras?
SelectTasksLabel2=Markera ytterligare uppgifter att utföra vid installation av [name], tryck sedan på Nästa.

; *** "Select Start Menu Folder" wizard page





WizardSelectProgramGroup=Välj Startmenykatalogen
SelectStartMenuFolderDesc=Var skall installationsprogrammet placera programmets genvägar?
SelectStartMenuFolderLabel3=Installationsprogrammet kommer att skapa programmets genvägar i följande katalog.
SelectStartMenuFolderBrowseLabel=För att fortsätta klickar du på Nästa. Om du vill välja en annan katalog, klickar du på Bläddra.
MustEnterGroupName=Du måste ange en katalog.
GroupNameTooLong=Katalogens namn eller sökväg är för lång.
InvalidGroupName=Katalogen du har valt är inte tillgänglig.
BadGroupName=Katalognamnet kan inte innehålla någon av följande tecken:%n%n%1
NoProgramGroupCheck2=&Skapa ingen Startmenykatalog

; *** "Ready to Install" wizard page





WizardReady=Redo att installera
ReadyLabel1=Installationsprogrammet är nu redo att installera [name] på din dator.
ReadyLabel2a=Tryck på Installera om du vill fortsätta, eller på gå Tillbaka om du vill granska eller ändra på något.
ReadyLabel2b=Välj Installera för att påbörja installationen.
ReadyMemoUserInfo=Användarinformation:
ReadyMemoDir=Installationsplats:
ReadyMemoType=Installationstyp:
ReadyMemoComponents=Valda komponenter:
ReadyMemoGroup=Startmenykatalog:
ReadyMemoTasks=Extra uppgifter:
DownloadingLabel=Laddar ner ytterligare filer...
ButtonStopDownload=&Stoppa nedladdning
StopDownload=Är du säker på att du vill stoppa nedladdningen?
ErrorDownloadAborted=Nedladdningen avbruten
ErrorDownloadFailed=Nedladdningen misslyckades: %1 %2
ErrorDownloadSizeFailed=Få storlek misslyckades: %1 %2
ErrorFileHash1=Filhash misslyckades: %1
ErrorFileHash2=Ogiltig filhash: förväntat %1, hittat %2
ErrorProgress=Ogiltig framfart: %1 of %2
ErrorFileSize=Ogiltig filstorlek: förväntad %1, hittad %2

; *** "Preparing to Install" wizard page





WizardPreparing=Förbereder installationen
PreparingDesc=Installationsprogrammet förbereder installationen av [name] på din dator.
PreviousInstallNotCompleted=Installationen/avinstallationen av ett tidigare program har inte slutförts. Du måste starta om datorn för att avsluta den installationen.%n%nEfter att ha startat om datorn kör du installationsprogrammet igen för att slutföra installationen av [name].
CannotContinue=Installationsprogrammet kan inte fortsätta. Klicka på Avbryt för att avsluta.
ApplicationsFound=Följande program använder filer som måste uppdateras av Setup. Vi rekommenderar att du låter Setup automatiskt stänga dessa program.
ApplicationsFound2=Följande program använder filer som måste uppdateras av Setup. Vi rekommenderar att du låter Setup automatiskt stänga dessa program. Efter installationen kommer Setup att försöka starta programmen igen.
CloseApplications=&Stäng programmen automatiskt
DontCloseApplications=&Stäng inte programmen
ErrorCloseApplications=Installationsprogrammet kunde inte stänga alla program. Innan installationen fortsätter rekommenderar vi att du stänger alla program som använder filer som Setup behöver uppdatera. 
PrepareToInstallNeedsRestart=Installationen måste starta om din dator. När du har startat om datorn kör du Setup igen för att slutföra installationen av [name].%n%nVill du starta om nu?

; *** "Installing" wizard page





WizardInstalling=Installerar
InstallingLabel=Vänta medan [name] installeras på din dator.

; *** "Setup Completed" wizard page





FinishedHeadingLabel=Avslutar installationen av [name]
FinishedLabelNoIcons=[name] har nu installerats på din dator.
FinishedLabel=[name] har nu installerats på din dator. Programmet kan startas genom att välja någon av ikonerna.
ClickFinish=Välj Slutför för att avsluta installationen.
FinishedRestartLabel=För att slutföra installationen av [name], måste datorn startas om. Vill du starta om nu?
FinishedRestartMessage=För att slutföra installationen av [name], måste datorn startas om.%n%nVill du starta om datorn nu?
ShowReadmeCheck=Ja, jag vill se filen LÄS MIG
YesRadio=&Ja, jag vill starta om datorn nu
NoRadio=&Nej, jag startar själv om datorn senare
; used for example as 'Run MyProg.exe'





RunEntryExec=Kör %1
; used for example as 'View Readme.txt'





RunEntryShellExec=Läs %1

; *** "Setup Needs the Next Disk" stuff





ChangeDiskTitle=Installationsprogrammet behöver nästa diskett
SelectDiskLabel2=Var god sätt i diskett %1 och tryck OK.%n%nOm filerna kan hittas i en annan katalog än den som visas nedan, skriv in rätt sökväg eller välj Bläddra.
PathLabel=&Sökväg:
FileNotInDir2=Kunde inte hitta filen "%1" i "%2". Var god sätt i korrekt diskett eller välj en annan katalog.
SelectDirectoryLabel=Var god ange sökvägen för nästa diskett.

; *** Installation phase messages





SetupAborted=Installationen slutfördes inte.%n%nVar god rätta till felet och kör installationen igen.
AbortRetryIgnoreSelectAction=Välj åtgärd
AbortRetryIgnoreRetry=&Försök igen
AbortRetryIgnoreIgnore=&Ignorera felet och fortsätt
AbortRetryIgnoreCancel=Avbryt installationen

; *** Installation status messages





StatusClosingApplications=Stänger program...
StatusCreateDirs=Skapar kataloger...
StatusExtractFiles=Packar upp filer...
StatusCreateIcons=Skapar programikoner...
StatusCreateIniEntries=Skriver INI-värden...
StatusCreateRegistryEntries=Skriver register-värden...
StatusRegisterFiles=Registrerar filer...
StatusSavingUninstall=Sparar information för avinstallation...
StatusRunProgram=Slutför installationen...
StatusRestartingApplications=Startar om program...
StatusRollback=Återställer ändringar...

; *** Misc. errors





ErrorInternal2=Internt fel: %1
ErrorFunctionFailedNoCode=%1 misslyckades
ErrorFunctionFailed=%1 misslyckades; kod %2
ErrorFunctionFailedWithMessage=%1 misslyckades; kod %2.%n%3
ErrorExecutingProgram=Kan inte köra filen:%n%1

; *** Registry errors





ErrorRegOpenKey=Fel vid öppning av registernyckel:%n%1\%2
ErrorRegCreateKey=Kan ej skapa registernyckel:%n%1\%2
ErrorRegWriteKey=Kan ej skriva till registernyckel:%n%1\%2

; *** INI errors





ErrorIniEntry=Kan inte skriva nytt INI-värde i filen "%1".
FileAbortRetryIgnoreSkipNotRecommended=&Hoppa över den här filen (rekommenderas inte)
FileAbortRetryIgnoreIgnoreNotRecommended=&Ignorera felet och fortsätt (rekommenderas inte)
SourceIsCorrupted=Källfilen är felaktig
SourceDoesntExist=Källfilen "%1" finns inte
ExistingFileReadOnly2=Den befintliga filen kunde inte bytas ut eftersom den är markerad skrivskyddad.
ExistingFileReadOnlyRetry=&Ta bort skrivskyddad attributet och försök igen
ExistingFileReadOnlyKeepExisting=&Behåll den befintliga filen
ErrorReadingExistingDest=Ett fel uppstod vid försök att läsa den befintliga filen:
FileExistsSelectAction=Välj åtgärd
FileExists2=Filen finns redan.
FileExistsOverwriteExisting=&Skriv över den befintliga filen
FileExistsKeepExisting=&Behåll befintlig fil
FileExistsOverwriteOrKeepAll=&Gör detta för nästa konflikt
ExistingFileNewerSelectAction=Välj åtgärd
ExistingFileNewer2=Den befintliga filen är nyare än den som Setup försöker installera.
ExistingFileNewerOverwriteExisting=&Skriv över den befintliga filen
ExistingFileNewerKeepExisting=&Behåll befintlig fil (rekommenderas)
ExistingFileNewerOverwriteOrKeepAll=&Gör detta för nästa konflikt
ErrorChangingAttr=Ett fel uppstod vid försök att ändra attribut på den befintliga filen:
ErrorCreatingTemp=Ett fel uppstod vid ett försök att skapa installationskatalogen:
ErrorReadingSource=Ett fel uppstod vid ett försök att läsa källfilen:
ErrorCopying=Ett fel uppstod vid kopiering av filen:
ErrorReplacingExistingFile=Ett fel uppstod vid ett försök att ersätta den befintliga filen:
ErrorRestartReplace=ÅterstartaErsätt misslyckades:
ErrorRenamingTemp=Ett fel uppstod vid ett försök att byta namn på en fil i installationskatalogen:
ErrorRegisterServer=Kunde inte registrera DLL/OCX: %1
ErrorRegSvr32Failed=RegSvr32 misslyckades med felkod %1
ErrorRegisterTypeLib=Kunde inte registrera typbibliotek: %1
UninstallDisplayNameMark=%1 (%2)
UninstallDisplayNameMarks=%1 (%2, %3)
UninstallDisplayNameMark32Bit=32-bit
UninstallDisplayNameMark64Bit=64-bit
UninstallDisplayNameMarkAllUsers=Alla användare
UninstallDisplayNameMarkCurrentUser=Nuvarande användare

; *** Post-installation errors





ErrorOpeningReadme=Ett fel uppstod vid öppnandet av LÄS MIG-filen.
ErrorRestartingComputer=Installationsprogrammet kunde inte starta om datorn. Var god gör det manuellt.

; *** Uninstaller messages





UninstallNotFound=Filen "%1" finns inte. Kan inte avinstallera.
UninstallOpenError=Filen "%1" kan inte öppnas. Kan inte avinstallera.
UninstallUnsupportedVer=Avinstallationsloggen "%1" är i ett format som denna version inte känner igen. Kan ej avinstallera
UninstallUnknownEntry=En okänd rad (%1) hittades i avinstallationsloggen
ConfirmUninstall=Är du säker på att du vill ta bort %1 och alla tillhörande komponenter?
UninstallOnlyOnWin64=Denna installation kan endast avinstalleras på en 64-bitarsversion av Windows.
OnlyAdminCanUninstall=Denna installation kan endast avinstalleras av en användare med administrativa rättigheter.
UninstallStatusLabel=Var god och vänta medan %1 tas bort från din dator.
UninstalledAll=%1 är nu borttaget från din dator.
UninstalledMost=Avinstallationen av %1 är nu klar.%n%nEn del filer/kataloger gick ej att ta bort. Dessa kan tas bort manuellt.
UninstalledAndNeedsRestart=För att slutföra avinstallationen av %1 måste datorn startas om.%n%nVill du starta om nu?
UninstallDataCorrupted=Filen "%1" är felaktig. Kan inte avinstallera

; *** Uninstallation phase messages





ConfirmDeleteSharedFileTitle=Ta bort delad fil?
ConfirmDeleteSharedFile2=Systemet indikerar att följande delade fil inte längre används av några program. Vill du ta bort den delade filen?%n%n%1%n%nOm något program fortfarande använder denna fil och den raderas, kommer programmet kanske att sluta fungera. Om du är osäker, välj Nej. Att låta filen ligga kvar i systemet kommer inte att orsaka någon skada.
SharedFileNameLabel=Filnamn:
SharedFileLocationLabel=Plats:
WizardUninstalling=Avinstallationsstatus
StatusUninstalling=Avinstallerar %1...

; *** Shutdown block reasons





ShutdownBlockReasonInstallingApp=Installerar %1.
ShutdownBlockReasonUninstallingApp=Avinstallerar %1.

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.







[CustomMessages]
NameAndVersion=%1 version %2
AdditionalIcons=Återstående ikoner:
CreateDesktopIcon=Skapa en genväg på skrivbordet
CreateQuickLaunchIcon=Skapa en genväg i Snabbstartfältet
ProgramOnTheWeb=%1 på Webben
UninstallProgram=Avinstallera %1
LaunchProgram=Starta %1
AssocFileExtension=Associera %1 med %2 filnamnstillägg
AssocingFileExtension=Associerar %1 med %2 filnamnstillägg...
AutoStartProgramGroupDescription=Autostart:
AutoStartProgram=Starta automatiskt %1
AddonHostProgramNotFound=%1 kunde inte hittas i katalogen du valde.%n%nVill du fortsätta ändå?
