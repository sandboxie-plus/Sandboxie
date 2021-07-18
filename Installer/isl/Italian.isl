; bovirus@gmail.com
; *** Inno Setup version 6.1.0+ Italian messages ***
;
; To download user-contributed translations of this file, go to:
;    https://jrsoftware.org/files/istrans/
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; Italian.isl - Last Update: 25.07.2020  by bovirus (bovirus@gmail.com)
;
; Translator name:   bovirus
; Translator e-mail: bovirus@gmail.com
; Based on previous translations of Rinaldo M. aka Whiteshark (based on ale5000 5.1.11+ translation)
;
[LangOptions]
; The following three entries are very important. Be sure to read and 
; understand the '[LangOptions] section' topic in the help file.
LanguageName=Italiano
LanguageID=$0410
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

[Messages]

; *** Application titles
SetupAppTitle=Installazione
SetupWindowTitle=Installazione di %1
UninstallAppTitle=Disinstallazione
UninstallAppFullTitle=Disinstallazione di %1

; *** Misc. common
InformationTitle=Informazioni
ConfirmTitle=Conferma
ErrorTitle=Errore

; *** SetupLdr messages
SetupLdrStartupMessage=Questa è l'installazione di %1.%n%nVuoi continuare?
LdrCannotCreateTemp=Impossibile creare un file temporaneo.%n%nInstallazione annullata.
LdrCannotExecTemp=Impossibile eseguire un file nella cartella temporanea.%n%nInstallazione annullata.

; *** Startup error messages
LastErrorMessage=%1.%n%nErrore %2: %3
SetupFileMissing=File %1 non trovato nella cartella di installazione.%n%nCorreggi il problema o richiedi una nuova copia del programma.
SetupFileCorrupt=I file di installazione sono danneggiati.%n%nRichiedi una nuova copia del programma.
SetupFileCorruptOrWrongVer=I file di installazione sono danneggiati, o sono incompatibili con questa versione del programma di installazione.%n%nCorreggi il problema o richiedi una nuova copia del programma.
InvalidParameter=È stato inserito nella riga di comando un parametro non valido:%n%n%1
SetupAlreadyRunning=Il processo di installazione è già in funzione.
WindowsVersionNotSupported=Questo programma non supporta la versione di Windows installata nel computer.
WindowsServicePackRequired=Questo programma richiede %1 Service Pack %2 o successivo.
NotOnThisPlatform=Questo programma non è compatibile con %1.
OnlyOnThisPlatform=Questo programma richiede %1.
OnlyOnTheseArchitectures=Questo programma può essere installato solo su versioni di Windows progettate per le seguenti architetture della CPU:%n%n%1
WinVersionTooLowError=Questo programma richiede %1 versione %2 o successiva.
WinVersionTooHighError=Questo programma non può essere installato su %1 versione %2 o successiva.
AdminPrivilegesRequired=Per installare questo programma sono richiesti privilegi di amministratore.
PowerUserPrivilegesRequired=Per poter installare questo programma sono richiesti i privilegi di amministratore o di Power Users.
SetupAppRunningError=%1 è attualmente in esecuzione.%n%nChiudi adesso tutte le istanze del programma e poi seleziona "OK", o seleziona "Annulla" per uscire.
UninstallAppRunningError=%1 è attualmente in esecuzione.%n%nChiudi adesso tutte le istanze del programma e poi seleziona "OK", o seleziona "Annulla" per uscire.

; *** Startup questions
PrivilegesRequiredOverrideTitle=Seleziona modo installazione
PrivilegesRequiredOverrideInstruction=Seleziona modo installazione
PrivilegesRequiredOverrideText1=%1 può essere installato per tutti gli utenti (richiede privilegi di amministratore), o solo per l'utente attuale.
PrivilegesRequiredOverrideText2=%1 può essere installato solo per l'utente attuale, o per tutti gli utenti (richiede privilegi di amministratore).
PrivilegesRequiredOverrideAllUsers=Inst&alla per tutti gli utenti
PrivilegesRequiredOverrideAllUsersRecommended=Inst&alla per tutti gli utenti (suggerito)
PrivilegesRequiredOverrideCurrentUser=Installa solo per l'&utente attuale
PrivilegesRequiredOverrideCurrentUserRecommended=Installa solo per l'&utente attuale (suggerito)

; *** Misc. errors
ErrorCreatingDir=Impossibile creare la cartella "%1"
ErrorTooManyFilesInDir=Impossibile creare i file nella cartella "%1" perché contiene troppi file.

; *** Setup common messages
ExitSetupTitle=Uscita dall'installazione
ExitSetupMessage=L'installazione non è completa.%n%nUscendo dall'installazione in questo momento, il programma non sarà installato.%n%nÈ possibile eseguire l'installazione in un secondo tempo.%n%nVuoi uscire dall'installazione?
AboutSetupMenuItem=&Informazioni sull'installazione...
AboutSetupTitle=Informazioni sull'installazione
AboutSetupMessage=%1 versione %2%n%3%n%n%1 sito web:%n%4
AboutSetupNote=
TranslatorNote=Traduzione italiana a cura di Rinaldo M. aka Whiteshark e bovirus (v. 11.09.2018)

; *** Buttons
ButtonBack=< &Indietro
ButtonNext=&Avanti >
ButtonInstall=Inst&alla
ButtonOK=OK
ButtonCancel=Annulla
ButtonYes=&Si
ButtonYesToAll=Sì a &tutto
ButtonNo=&No
ButtonNoToAll=N&o a tutto
ButtonFinish=&Fine
ButtonBrowse=&Sfoglia...
ButtonWizardBrowse=S&foglia...
ButtonNewFolder=&Crea nuova cartella

; *** "Select Language" dialog messages
SelectLanguageTitle=Seleziona la lingua dell'installazione
SelectLanguageLabel=Seleziona la lingua da usare durante l'installazione.

; *** Common wizard text
ClickNext=Seleziona "Avanti" per continuare, o "Annulla" per uscire.
BeveledLabel=
BrowseDialogTitle=Sfoglia cartelle
BrowseDialogLabel=Seleziona una cartella nell'elenco, e quindi seleziona "OK".
NewFolderName=Nuova cartella

; *** "Welcome" wizard page
WelcomeLabel1=Installazione di [name]
WelcomeLabel2=[name/ver] sarà installato sul computer.%n%nPrima di procedere chiudi tutte le applicazioni attive.

; *** "Password" wizard page
WizardPassword=Password
PasswordLabel1=Questa installazione è protetta da password.
PasswordLabel3=Inserisci la password, quindi per continuare seleziona "Avanti".%nLe password sono sensibili alle maiuscole/minuscole.
PasswordEditLabel=&Password:
IncorrectPassword=La password inserita non è corretta. Riprova.

; *** "License Agreement" wizard page
WizardLicense=Contratto di licenza
LicenseLabel=Prima di procedere leggi con attenzione le informazioni che seguono.
LicenseLabel3=Leggi il seguente contratto di licenza.%nPer procedere con l'installazione è necessario accettare tutti i termini del contratto.
LicenseAccepted=Accetto i termini del &contratto di licenza 
LicenseNotAccepted=&Non accetto i termini del contratto di licenza

; *** "Information" wizard pages
WizardInfoBefore=Informazioni
InfoBeforeLabel=Prima di procedere leggi le importanti informazioni che seguono.
InfoBeforeClickLabel=Quando sei pronto per proseguire, seleziona "Avanti".
WizardInfoAfter=Informazioni
InfoAfterLabel=Prima di procedere leggi le importanti informazioni che seguono.
InfoAfterClickLabel=Quando sei pronto per proseguire, seleziona "Avanti".

; *** "User Information" wizard page
WizardUserInfo=Informazioni utente
UserInfoDesc=Inserisci le seguenti informazioni.
UserInfoName=&Nome:
UserInfoOrg=&Società:
UserInfoSerial=&Numero di serie:
UserInfoNameRequired=È necessario inserire un nome.

; *** "Select Destination Location" wizard page
WizardSelectDir=Selezione cartella di installazione
SelectDirDesc=Dove vuoi installare [name]?
SelectDirLabel3=[name] sarà installato nella seguente cartella.
SelectDirBrowseLabel=Per continuare seleziona "Avanti".%nPer scegliere un'altra cartella seleziona "Sfoglia".
DiskSpaceGBLabel=Sono richiesti almeno [gb] GB di spazio libero nel disco.
DiskSpaceMBLabel=Sono richiesti almeno [mb] MB di spazio libero nel disco.
CannotInstallToNetworkDrive=Non è possibile effettuare l'installazione in un disco in rete.
CannotInstallToUNCPath=Non è possibile effettuare l'installazione in un percorso UNC.
InvalidPath=Va inserito un percorso completo di lettera di unità; per esempio:%n%nC:\APP%n%no un percorso di rete nella forma:%n%n\\server\condivisione
InvalidDrive=L'unità o il percorso di rete selezionato non esiste o non è accessibile.%n%nSelezionane un altro.
DiskSpaceWarningTitle=Spazio su disco insufficiente
DiskSpaceWarning=L'installazione richiede per eseguire l'installazione almeno %1 KB di spazio libero, ma l'unità selezionata ha solo %2 KB disponibili.%n%nVuoi continuare comunque?
DirNameTooLong=Il nome della cartella o il percorso sono troppo lunghi.
InvalidDirName=Il nome della cartella non è valido.
BadDirName32=Il nome della cartella non può includere nessuno dei seguenti caratteri:%n%n%1
DirExistsTitle=Cartella già esistente
DirExists=La cartella%n%n  %1%n%nesiste già.%n%nVuoi comunque installare l'applicazione in questa cartella?
DirDoesntExistTitle=Cartella inesistente
DirDoesntExist=La cartella%n%n  %1%n%nnon esiste. Vuoi creare la cartella?

; *** "Select Components" wizard page
WizardSelectComponents=Selezione componenti
SelectComponentsDesc=Quali componenti vuoi installare?
SelectComponentsLabel2=Seleziona i componenti da installare, deseleziona quelli che non vuoi installare.%nPer continuare seleziona "Avanti".
FullInstallation=Installazione completa
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Installazione compatta
CustomInstallation=Installazione personalizzata
NoUninstallWarningTitle=Componente esistente
NoUninstallWarning=I seguenti componenti sono già installati nel computer:%n%n%1%n%nDeselezionando questi componenti essi non verranno rimossi.%n%nVuoi continuare comunque?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceGBLabel=La selezione attuale richiede almeno [gb] GB di spazio nel disco.
ComponentsDiskSpaceMBLabel=La selezione attuale richiede almeno [mb] MB di spazio nel disco.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Selezione processi aggiuntivi
SelectTasksDesc=Quali processi aggiuntivi vuoi eseguire?
SelectTasksLabel2=Seleziona i processi aggiuntivi che verranno eseguiti durante l'installazione di [name], quindi seleziona "Avanti".

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Selezione della cartella nel menu Avvio/Start
SelectStartMenuFolderDesc=Dove vuoi inserire i collegamenti al programma?
SelectStartMenuFolderLabel3=Verranno creati i collegamenti al programma nella seguente cartella del menu Avvio/Start.
SelectStartMenuFolderBrowseLabel=Per continuare, seleziona "Avanti".%nPer selezionare un'altra cartella, seleziona "Sfoglia".
MustEnterGroupName=Devi inserire il nome della cartella.
GroupNameTooLong=Il nome della cartella o il percorso sono troppo lunghi.
InvalidGroupName=Il nome della cartella non è valido.
BadGroupName=Il nome della cartella non può includere nessuno dei seguenti caratteri:%n%n%1
NoProgramGroupCheck2=&Non creare una cartella nel menu Avvio/Start

; *** "Ready to Install" wizard page
WizardReady=Pronto per l'installazione
ReadyLabel1=Il programma è pronto per iniziare l'installazione di [name] nel computer.
ReadyLabel2a=Seleziona "Installa" per continuare con l'installazione, o "Indietro" per rivedere o modificare le impostazioni.
ReadyLabel2b=Per procedere con l'installazione seleziona "Installa".
ReadyMemoUserInfo=Informazioni utente:
ReadyMemoDir=Cartella di installazione:
ReadyMemoType=Tipo di installazione:
ReadyMemoComponents=Componenti selezionati:
ReadyMemoGroup=Cartella del menu Avvio/Start:
ReadyMemoTasks=Processi aggiuntivi:

; *** TDownloadWizardPage wizard page and DownloadTemporaryFile
DownloadingLabel=Download file aggiuntivi...
ButtonStopDownload=&Stop download
StopDownload=Sei sicuro di voler interrompere il download?
ErrorDownloadAborted=Download annullato
ErrorDownloadFailed=Download fallito: %1 %2
ErrorDownloadSizeFailed=Rilevamento dimensione fallito: %1 %2
ErrorFileHash1=Errore hash file: %1
ErrorFileHash2=Hash file non valido: atteso %1, trovato %2
ErrorProgress=Progresso non valido: %1 di %2
ErrorFileSize=Dimensione file non valida: attesa %1, trovata %2

; *** "Preparing to Install" wizard page
WizardPreparing=Preparazione all'installazione
PreparingDesc=Preparazione all'installazione di [name] nel computer.
PreviousInstallNotCompleted=L'installazione/rimozione precedente del programma non è stata completata.%n%nÈ necessario riavviare il sistema per completare l'installazione.%n%nDopo il riavvio del sistema esegui di nuovo l'installazione di [name].
CannotContinue=L'installazione non può continuare. Seleziona "Annulla" per uscire.
ApplicationsFound=Le seguenti applicazioni stanno usando file che devono essere aggiornati dall'installazione.%n%nTi consigliamo di permettere al processo di chiudere automaticamente queste applicazioni.
ApplicationsFound2=Le seguenti applicazioni stanno usando file che devono essere aggiornati dall'installazione.%n%nTi consigliamo di permettere al processo di chiudere automaticamente queste applicazioni.%n%nAl completamento dell'installazione, il processo tenterà di riavviare le applicazioni.
CloseApplications=Chiudi &automaticamente le applicazioni
DontCloseApplications=&Non chiudere le applicazioni
ErrorCloseApplications=L'installazione non è riuscita a chiudere automaticamente tutte le applicazioni.%n%nPrima di proseguire ti raccomandiamo di chiudere tutte le applicazioni che usano file che devono essere aggiornati durante l'installazione.
PrepareToInstallNeedsRestart=Il programma di installazione deve riavviare il computer.%nDopo aver riavviato il computer esegui di nuovo il programma di installazione per completare l'installazione di [name].%n%nVuoi riavviare il computer ora?

; *** "Installing" wizard page
WizardInstalling=Installazione in corso
InstallingLabel=Attendi il completamento dell'installazione di [name] nel computer.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Installazione di [name] completata
FinishedLabelNoIcons=Installazione di [name] completata.
FinishedLabel=Installazione di [name] completata.%n%nL'applicazione può essere eseguita selezionando le relative icone.
ClickFinish=Seleziona "Fine" per uscire dall'installazione.
FinishedRestartLabel=Per completare l'installazione di [name], è necessario riavviare il sistema.%n%nVuoi riavviare adesso?
FinishedRestartMessage=Per completare l'installazione di [name], è necessario riavviare il sistema.%n%nVuoi riavviare adesso?
ShowReadmeCheck=Si, visualizza ora il file LEGGIMI
YesRadio=&Si, riavvia il sistema adesso
NoRadio=&No, riavvia il sistema più tardi
; used for example as 'Run MyProg.exe'
RunEntryExec=Esegui %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Visualizza %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=L'installazione necessita del disco successivo
SelectDiskLabel2=Inserisci il disco %1 e seleziona "OK".%n%nSe i file di questo disco si trovano in una cartella diversa da quella visualizzata sotto, inserisci il percorso corretto o seleziona "Sfoglia".
PathLabel=&Percorso:
FileNotInDir2=Il file "%1" non è stato trovato in "%2".%n%nInserisci il disco corretto o seleziona un'altra cartella.
SelectDirectoryLabel=Specifica il percorso del prossimo disco.

; *** Installation phase messages
SetupAborted=L'installazione non è stata completata.%n%nCorreggi il problema e riesegui nuovamente l'installazione.
AbortRetryIgnoreSelectAction=Seleziona azione
AbortRetryIgnoreRetry=&Riprova
AbortRetryIgnoreIgnore=&Ignora questo errore e continua
AbortRetryIgnoreCancel=Annulla installazione

; *** Installation status messages
StatusClosingApplications=Chiusura applicazioni...
StatusCreateDirs=Creazione cartelle...
StatusExtractFiles=Estrazione file...
StatusCreateIcons=Creazione icone...
StatusCreateIniEntries=Creazione voci nei file INI...
StatusCreateRegistryEntries=Creazione voci di registro...
StatusRegisterFiles=Registrazione file...
StatusSavingUninstall=Salvataggio delle informazioni di disinstallazione...
StatusRunProgram=Termine dell'installazione...
StatusRestartingApplications=Riavvio applicazioni...
StatusRollback=Recupero delle modifiche...

; *** Misc. errors
ErrorInternal2=Errore interno %1
ErrorFunctionFailedNoCode=%1 fallito
ErrorFunctionFailed=%1 fallito; codice %2
ErrorFunctionFailedWithMessage=%1 fallito; codice %2.%n%3
ErrorExecutingProgram=Impossibile eseguire il file:%n%1

; *** Registry errors
ErrorRegOpenKey=Errore di apertura della chiave di registro:%n%1\%2
ErrorRegCreateKey=Errore di creazione della chiave di registro:%n%1\%2
ErrorRegWriteKey=Errore di scrittura della chiave di registro:%n%1\%2

; *** INI errors
ErrorIniEntry=Errore nella creazione delle voci INI nel file "%1".

; *** File copying errors
FileAbortRetryIgnoreSkipNotRecommended=&Salta questo file (non suggerito)
FileAbortRetryIgnoreIgnoreNotRecommended=&Ignora questo errore e continua (non suggerito)
SourceIsCorrupted=Il file sorgente è danneggiato
SourceDoesntExist=Il file sorgente "%1" non esiste
ExistingFileReadOnly2=Il file esistente non può essere sostituito in quanto segnato come in sola lettura.
ExistingFileReadOnlyRetry=&Rimuovi attributo di sola lettura e riprova
ExistingFileReadOnlyKeepExisting=&Mantieni il file esistente
ErrorReadingExistingDest=Si è verificato un errore durante la lettura del file esistente:
FileExistsSelectAction=Seleziona azione
FileExists2=Il file esiste già.
FileExistsOverwriteExisting=S&ovrascrivi il file esistente
FileExistsKeepExisting=&Mantieni il file esistente
FileExistsOverwriteOrKeepAll=&Applica questa azione per i prossimi conflitti
ExistingFileNewerSelectAction=Seleziona azione
ExistingFileNewer2=Il file esistente è più recente del file che si sta cercando di installare.
ExistingFileNewerOverwriteExisting=S&ovrascrivi il file esistente
ExistingFileNewerKeepExisting=&Mantieni il file esistente (suggerito)
ExistingFileNewerOverwriteOrKeepAll=&Applica questa azione per i prossimi conflitti
ErrorChangingAttr=Si è verificato un errore durante il tentativo di modifica dell'attributo del file esistente:
ErrorCreatingTemp=Si è verificato un errore durante la creazione di un file nella cartella di installazione:
ErrorReadingSource=Si è verificato un errore durante la lettura del file sorgente:
ErrorCopying=Si è verificato un errore durante la copia di un file:
ErrorReplacingExistingFile=Si è verificato un errore durante la sovrascrittura del file esistente:
ErrorRestartReplace=Errore durante riavvio o sostituzione:
ErrorRenamingTemp=Si è verificato un errore durante il tentativo di rinominare un file nella cartella di installazione:
ErrorRegisterServer=Impossibile registrare la DLL/OCX: %1
ErrorRegSvr32Failed=RegSvr32 è fallito con codice di uscita %1
ErrorRegisterTypeLib=Impossibile registrare la libreria di tipo: %1

; *** Uninstall display name markings
; used for example as 'My Program (32-bit)'
UninstallDisplayNameMark=%1 (%2)
; used for example as 'My Program (32-bit, All users)'
UninstallDisplayNameMarks=%1 (%2, %3)
UninstallDisplayNameMark32Bit=32bit
UninstallDisplayNameMark64Bit=64bit
UninstallDisplayNameMarkAllUsers=Tutti gli utenti
UninstallDisplayNameMarkCurrentUser=Utente attuale

; *** Post-installation errors
ErrorOpeningReadme=Si è verificato un errore durante l'apertura del file LEGGIMI.
ErrorRestartingComputer=Impossibile riavviare il sistema. Riavvia il sistema manualmente.

; *** Uninstaller messages
UninstallNotFound=Il file "%1" non esiste.%n%nImpossibile disinstallare.
UninstallOpenError=Il file "%1" non può essere aperto.%n%nImpossibile disinstallare
UninstallUnsupportedVer=Il file registro di disinstallazione "%1" è in un formato non riconosciuto da questa versione del programma di disinstallazione.%n%nImpossibile disinstallare
UninstallUnknownEntry=Trovata una voce sconosciuta (%1) nel file registro di disinstallazione
ConfirmUninstall=Vuoi rimuovere completamente %1 e tutti i suoi componenti?
UninstallOnlyOnWin64=Questa applicazione può essere disinstallata solo in Windows a 64-bit.
OnlyAdminCanUninstall=Questa applicazione può essere disinstallata solo da un utente con privilegi di amministratore.
UninstallStatusLabel=Attendi fino a che %1 è stato rimosso dal computer.
UninstalledAll=Disinstallazione di %1 completata.
UninstalledMost=Disinstallazione di %1 completata.%n%nAlcuni elementi non possono essere rimossi.%n%nDovranno essere rimossi manualmente.
UninstalledAndNeedsRestart=Per completare la disinstallazione di %1, è necessario riavviare il sistema.%n%nVuoi riavviare il sistema adesso?
UninstallDataCorrupted=Il file "%1" è danneggiato. Impossibile disinstallare

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Vuoi rimuovere il file condiviso?
ConfirmDeleteSharedFile2=Il sistema indica che il seguente file condiviso non è più usato da nessun programma.%nVuoi rimuovere questo file condiviso?%nSe qualche programma usasse questo file, potrebbe non funzionare più correttamente.%nSe non sei sicuro, seleziona "No".%nLasciare il file nel sistema non può causare danni.
SharedFileNameLabel=Nome del file:
SharedFileLocationLabel=Percorso:
WizardUninstalling=Stato disinstallazione
StatusUninstalling=Disinstallazione di %1...

; *** Shutdown block reasons
ShutdownBlockReasonInstallingApp=Installazione di %1.
ShutdownBlockReasonUninstallingApp=Disinstallazione di %1.

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1 versione %2
AdditionalIcons=Icone aggiuntive:
CreateDesktopIcon=Crea un'icona sul &desktop
CreateQuickLaunchIcon=Crea un'icona nella &barra 'Avvio veloce'
ProgramOnTheWeb=Sito web di %1
UninstallProgram=Disinstalla %1
LaunchProgram=Avvia %1
AssocFileExtension=&Associa i file con estensione %2 a %1
AssocingFileExtension=Associazione dei file con estensione %2 a %1...
AutoStartProgramGroupDescription=Esecuzione automatica:
AutoStartProgram=Esegui automaticamente %1
AddonHostProgramNotFound=Impossibile individuare %1 nella cartella selezionata.%n%nVuoi continuare ugualmente?
