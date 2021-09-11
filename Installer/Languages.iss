; English is default language and can be translated to any
; of the languages listed in the languages section.
; The translations can be added to the CustomMessages section.
; The content of this script will be included by Sandboxie-Plus.iss,
; so please edit with care.


[Languages]

Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "ChineseSimplified"; MessagesFile: "isl\ChineseSimplified.isl"
Name: "ChineseTraditional"; MessagesFile: "isl\ChineseTraditional.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "Italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"
;Name: "armenian"; MessagesFile: "compiler:Languages\Armenian.isl"
;Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
;Name: "bulgarian"; MessagesFile: "compiler:Languages\Bulgarian.isl"
;Name: "catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
;Name: "corsican"; MessagesFile: "compiler:Languages\Corsican.isl"
;Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
;Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
;Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
;Name: "french"; MessagesFile: "compiler:Languages\French.isl"
;Name: "hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"
;Name: "icelandic"; MessagesFile: "compiler:Languages\Icelandic.isl"
;Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
;Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
;Name: "slovak"; MessagesFile: "compiler:Languages\Slovak.isl"
;Name: "slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"
;Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"


[CustomMessages]

; English
english.AddSandboxedMenu=Add "Run Sandboxed" to context menu
english.CustomPageLabel1=Select Installation Type
english.CustomPageLabel2=How should be installed
english.CustomPageLabel3=Choose the installation mode
english.CustomPageInstallMode=Install {#MyAppName} on this computer
english.CustomPageUpgradeMode=Update existing {#MyAppName} installation
english.CustomPagePortableMode=Extract all files to a directory for portable use
english.RequiresWin7OrLater=Sandboxie-Plus requires Windows 7 or later.
english.ClassicFound=Sandboxie Classic installation detected, it must be uninstalled first, do you want to uninstall it now?
english.RunSandboxedMenu=Run &Sandboxed
english.UninstallTaskLabel1=Select Uninstall Type
english.UninstallTaskLabel2=How would you like to uninstall Sandboxie-Plus?
english.UninstallTaskLabel3=&Keep configuration files and sandboxes%nThis is the most recommended option if you plan to reinstall Sandboxie-Plus while keeping your configuration files and sandboxes.
english.UninstallTaskLabel4=Remove &configuration files%nSelect this option to remove the Sandboxie.ini and Sandboxie-Plus.ini configuration files while keeping the sandboxes unchanged.
english.UninstallTaskLabel5=Remove configuration files and &sandboxes%nSelect this option to remove all configuration files and sandboxes, including the Sandbox folder located in custom paths with FileRootPath.

; Chinese Simplified
ChineseSimplified.AddSandboxedMenu=添加“在沙盘中运行”菜单
ChineseSimplified.CustomPageLabel1=选择安装方式
ChineseSimplified.CustomPageLabel2=应当如何安装
ChineseSimplified.CustomPageLabel3=选择安装模式
ChineseSimplified.CustomPageInstallMode=在这台计算机上安装 {#MyAppName}
ChineseSimplified.CustomPageUpgradeMode=更新当前已安装的 {#MyAppName}
ChineseSimplified.CustomPagePortableMode=提取全部文件到指定目录供便携化使用
ChineseSimplified.RequiresWin7OrLater=Sandboxie-Plus 需要 Windows 7 或更高版本。
ChineseSimplified.ClassicFound=检测到已安装 Sandboxie 原版，继续安装必须先将其卸载，是否开始卸载？
ChineseSimplified.RunSandboxedMenu=在沙盘中运行(&S)
;ChineseSimplified.UninstallTaskLabel1=
;ChineseSimplified.UninstallTaskLabel2=
;ChineseSimplified.UninstallTaskLabel3=
;ChineseSimplified.UninstallTaskLabel4=
;ChineseSimplified.UninstallTaskLabel5=

; Chinese Traditional
ChineseTraditional.AddSandboxedMenu=加入「在沙箱中執行」選單
ChineseTraditional.CustomPageLabel1=選擇安裝方式
ChineseTraditional.CustomPageLabel2=應該如何安裝
ChineseTraditional.CustomPageLabel3=選擇安裝模式
ChineseTraditional.CustomPageInstallMode=在這台電腦上安裝 {#MyAppName}
ChineseTraditional.CustomPageUpgradeMode=更新目前已安裝的 {#MyAppName}
ChineseTraditional.CustomPagePortableMode=解壓縮所有檔案到指定目錄供便攜化使用
ChineseTraditional.RequiresWin7OrLater=Sandboxie-Plus 需要 Windows 7 或更高版本。
ChineseTraditional.ClassicFound=偵測到已安裝 Sandboxie 原版，要繼續安裝必須先將其移除，是否開始移除？
ChineseTraditional.RunSandboxedMenu=在沙箱中執行(&S)
;ChineseTraditional.UninstallTaskLabel1=
;ChineseTraditional.UninstallTaskLabel2=
;ChineseTraditional.UninstallTaskLabel3=
;ChineseTraditional.UninstallTaskLabel4=
;ChineseTraditional.UninstallTaskLabel5=

; Dutch
dutch.AddSandboxedMenu="In sandbox uitvoeren" aan contextmenu toevoegen
dutch.CustomPageLabel1=Installatietype selecteren
dutch.CustomPageLabel2=Hoe moet er geïnstalleerd worden
dutch.CustomPageLabel3=Installatiemodus kiezen
dutch.CustomPageInstallMode={#MyAppName} op deze computer installeren
dutch.CustomPageUpgradeMode=Bestaande installatie van {#MyAppName} bijwerken
dutch.CustomPagePortableMode=Alle bestanden naar een map uitpakken voor draagbaar gebruik
dutch.RequiresWin7OrLater=Sandboxie-Plus vereist Windows 7 of later.
dutch.ClassicFound=Installatie van Sandboxie Classic gedetecteerd, die eerst moet verwijderd worden. Wilt u deze nu verwijderen?
dutch.RunSandboxedMenu=In &sandbox uitvoeren
;dutch.UninstallTaskLabel1=
;dutch.UninstallTaskLabel2=
;dutch.UninstallTaskLabel3=
;dutch.UninstallTaskLabel4=
;dutch.UninstallTaskLabel5=

; German
german.AddSandboxedMenu=Füge "In Sandbox ausführen" zum Kontextmenü hinzu
german.CustomPageLabel1=Wählen Sie die Installationsart aus
german.CustomPageLabel2=Wie soll installiert werden
german.CustomPageLabel3=Wählen Sie die Installationsmethode
german.CustomPageInstallMode=Installiere {#MyAppName} auf diesem Computer
german.CustomPageUpgradeMode=Update der bestehenden {#MyAppName} Installation
german.CustomPagePortableMode=Extrahiere alle Dateien in einen Ordner zur tragbaren Verwendung
german.RequiresWin7OrLater=Sandboxie-Plus benötigt Windows 7 oder neuer.
german.ClassicFound=Sandboxie Classic Installation gefunden, welche vorher deinstalliert werden muss. Möchten Sie diese nun deinstallieren?
german.RunSandboxedMenu=In &Sandbox ausführen
german.UninstallTaskLabel1=Wählen Sie die De-Installationsart aus
german.UninstallTaskLabel2=Wie möchten Sie Sandboxie-Plus de-installieren?
german.UninstallTaskLabel3=&Konfiguration und Sandboxen behalten%nDies it die empfohlene Option, wenn Sie vorhaben Sandboxie-Plus erneut zu intallieren, während Sandboxie-Plus alle Sandboxen und Konfigurationsdateien behält.
german.UninstallTaskLabel4=&Entferne die Konfiguration%nWählen Sie diese Option um die Sandboxie.ini und Sandboxie-Plus.ini Dateien zu entfernen, während die Sandboxen unverändert bleiben.
german.UninstallTaskLabel5=Entferne die Konfiguration und alle &Sandboxen%nWählen Sie diese Option um alle Konfigurationsdateien und alle Sandboxen, inklusive des Sandboxordners im, durch die Dateisystemquelle, benutzerdefinierten Pfad.

; Italian
Italian.AddSandboxedMenu=Aggiungi l'opzione "Avvia nell'area virtuale" al menu contestuale
Italian.CustomPageLabel1=Seleziona tipo di installazione
Italian.CustomPageLabel2=Come dovrebbe essere installato
Italian.CustomPageLabel3=Scegli la modalità di installazione
Italian.CustomPageInstallMode=Installa {#MyAppName} su questo computer
Italian.CustomPageUpgradeMode=Aggiorna l'installazione esistente di {#MyAppName}
Italian.CustomPagePortableMode=Estrai tutti i file in una directory per uso portabile
Italian.RequiresWin7OrLater=Sandboxie-Plus richiede Windows 7 o superiore.
Italian.ClassicFound=È stata rilevata una installazione di Sandboxie Classic che potrebbe causare incompatibilità. Rimuoverla ora?
Italian.RunSandboxedMenu=Avvia nell'&area virtuale
Italian.UninstallTaskLabel1=Seleziona tipo di disinstallazione
Italian.UninstallTaskLabel2=Come vorresti disinstallare Sandboxie-Plus?
Italian.UninstallTaskLabel3=&Mantieni i file di configurazione e le aree virtuali%nSeleziona questa opzione se prevedi di reinstallare Sandboxie-Plus mantenendo i file di configurazione e le aree virtuali.
Italian.UninstallTaskLabel4=Rimuovi i file di &configurazione%nSeleziona questa opzione per rimuovere i file di configurazione Sandboxie.ini e Sandboxie-Plus.ini mantenendo le aree virtuali invariate.
Italian.UninstallTaskLabel5=Rimuovi i file di configurazione e le &aree virtuali%nSeleziona questa opzione per rimuovere tutti i file di configurazione e le aree virtuali, compresa la cartella Sandbox presente nei percorsi personalizzati mediante FileRootPath.

; Portuguese
portuguese.AddSandboxedMenu=Adicionar "Roda na Caixa de Areia" para o menu de contexto
portuguese.CustomPageLabel1=Selecione Tipo de Instalação
portuguese.CustomPageLabel2=Como deve ser instalado
portuguese.CustomPageLabel3=Escolha o modo de instalação
portuguese.CustomPageInstallMode=Instalar {#MyAppName} neste computador
portuguese.CustomPageUpgradeMode=Atualizar instalação existente do {#MyAppName}
portuguese.CustomPagePortableMode=Extrair todos os arquivos para um diretório para uso portable
portuguese.RequiresWin7OrLater=Sandboxie-Plus requer o Windows 7 ou posterior.
portuguese.ClassicFound=Instalação do Classic Sandboxie detectada, deve ser desinstalado primeiro, você quer desinstalá-lo agora?
portuguese.RunSandboxedMenu=Rodar na &Caixa de Areia
;portuguese.UninstallTaskLabel1=
;portuguese.UninstallTaskLabel2=
;portuguese.UninstallTaskLabel3=
;portuguese.UninstallTaskLabel4=
;portuguese.UninstallTaskLabel5=

; Russian
russian.AddSandboxedMenu=Добавить "Запустить в песочнице" в контекстное меню
russian.CustomPageLabel1=Выберите тип установки
russian.CustomPageLabel2=Как следует установить
russian.CustomPageLabel3=Выберите режим установки
russian.CustomPageInstallMode=Установить {#MyAppName} на этом компьютере
russian.CustomPageUpgradeMode=Обновить существующую установку {#MyAppName}
russian.CustomPagePortableMode=Извлечь все файлы в каталог для портативного использования
russian.RequiresWin7OrLater=Sandboxie-Plus требует Windows 7 или новее.
russian.ClassicFound=Обнаружена установка Sandboxie Classic, перед установкой ее необходимо удалить. Удалить сейчас?
russian.RunSandboxedMenu=Запустить в песочнице (&S)
;russian.UninstallTaskLabel1=
;russian.UninstallTaskLabel2=
;russian.UninstallTaskLabel3=
;russian.UninstallTaskLabel4=
;russian.UninstallTaskLabel5=
