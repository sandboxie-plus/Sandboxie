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
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "Italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
;Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"
;Name: "armenian"; MessagesFile: "compiler:Languages\Armenian.isl"
;Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
;Name: "bulgarian"; MessagesFile: "compiler:Languages\Bulgarian.isl"
;Name: "catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
;Name: "corsican"; MessagesFile: "compiler:Languages\Corsican.isl"
;Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
;Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
;Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
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
ChineseSimplified.UninstallTaskLabel1=选择卸载方式
ChineseSimplified.UninstallTaskLabel2=您想卸载 Sandboxie-Plus 到什么程度？
ChineseSimplified.UninstallTaskLabel3=保留配置文件和所有沙盘(&K)%n如果您是打算重新安装 Sandboxie-Plus，这是很好的选择，不会丢失您的配置文件和沙盘。
ChineseSimplified.UninstallTaskLabel4=移除配置文件(&C)%n选择此选项将会移除 Sandboxie.ini 和 Sandboxie-Plus.ini 配置文件，但保留您的沙盘。
ChineseSimplified.UninstallTaskLabel5=移除配置文件和所有沙盘(&S)%n选择此选项将会移除所有配置文件和沙盘，包括 FileRootPath 所定义的路径下的 Sandbox 文件夹。

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
ChineseTraditional.UninstallTaskLabel1=選擇移除類型
ChineseTraditional.UninstallTaskLabel2=要如何移除 Sandboxie-Plus？
ChineseTraditional.UninstallTaskLabel3=保留設定檔和沙箱(&K)%n如果您打算保留設定檔和沙箱的同時，並重新安裝 Sandboxie-Plus，這是最推薦的選項。
ChineseTraditional.UninstallTaskLabel4=刪除設定檔案(&C)%n選擇此選項可刪除 Sandboxie.ini 和 Sandboxie-Plus.ini 設定檔，並完整保留沙箱。
ChineseTraditional.UninstallTaskLabel5=刪除設定檔和沙箱(&S)%n選擇此選項可刪除所有設定檔和沙箱，包含位於 FileRootPath 自訂路徑中的 Sandbox 資料夾。

; Dutch
dutch.AddSandboxedMenu="In sandbox uitvoeren" aan contextmenu toevoegen
dutch.CustomPageLabel1=Installatietype selecteren
dutch.CustomPageLabel2=Hoe moet er geïnstalleerd worden
dutch.CustomPageLabel3=Installatiemodus kiezen
dutch.CustomPageInstallMode={#MyAppName} op deze computer installeren
dutch.CustomPageUpgradeMode=Bestaande installatie van {#MyAppName} bijwerken
dutch.CustomPagePortableMode=Alle bestanden naar een map uitpakken voor draagbaar gebruik
dutch.RequiresWin7OrLater=Sandboxie-Plus vereist Windows 7 of later.
dutch.ClassicFound=Installatie van Sandboxie Classic gedetecteerd. Het moet eerst verwijderd worden. Wilt u het nu verwijderen?
dutch.RunSandboxedMenu=In &sandbox uitvoeren
dutch.UninstallTaskLabel1=Type verwijdering selecteren
dutch.UninstallTaskLabel2=Hoe wilt u Sandboxie-Plus verwijderen?
dutch.UninstallTaskLabel3=Configuratiebestanden en sandboxen behouden%nDit is de aanbevolen optie als u Sandboxie-Plus opnieuw wilt installeren en uw configuratiebestanden en sandboxen wilt behouden.
dutch.UninstallTaskLabel4=Configuratiebestanden verwijderen%nSelecteer deze optie om de configuratiebestanden Sandboxie.ini en Sandboxie-Plus.ini wilt verwijderen en de sandboxen ongewijzigd wilt laten.
dutch.UninstallTaskLabel5=Configuratiebestanden en sandboxen verwijderen%nSelecteer deze optie om alle configuratiebestanden en sandboxen te verwijderen, inclusief de Sandbox-map die zich in aangepaste paden met FileRootPath bevindt.

; French
french.AddSandboxedMenu=Ajouter « Exécuter dans un bac à sable » au menu contextuel
french.CustomPageLabel1=Sélectionnez le type d'installation
french.CustomPageLabel2=Comment doit-on l'installer
french.CustomPageLabel3=Choisissez le mode d'installation
french.CustomPageInstallMode=Installer {#MyAppName} sur cet ordinateur
french.CustomPageUpgradeMode=Mettre à jour l'installation de {#MyAppName} existante
french.CustomPagePortableMode=Extraire tous les fichiers dans un répertoire pour une utilisation portable
french.RequiresWin7OrLater=Sandboxie-Plus nécessite Windows 7 ou une version ultérieure.
french.ClassicFound=Installation de Sandboxie Classic détectée, il doit être désinstallé d'abord, voulez-vous le désinstaller maintenant ?
french.RunSandboxedMenu=Exécuter dans un &bac à sable
french.UninstallTaskLabel1=Sélectionnez le type de désinstallation
french.UninstallTaskLabel2=Comment voulez-vous désinstaller Sandboxie-Plus ?
french.UninstallTaskLabel3=&Conserver les fichiers de configuration et les bacs à sable%nCette option est la plus recommandée si vous prévoyez de réinstaller Sandboxie-Plus tout en conservant vos fichiers de configuration et vos bacs à sable.
french.UninstallTaskLabel4=Supprimer les &fichiers de configuration%nSélectionnez cette option pour supprimer les fichiers de configuration « Sandboxie.ini » et « Sandboxie-Plus.ini » tout en conservant les bacs à sable inchangés.
french.UninstallTaskLabel5=Supprimer les fichiers de configuration et les &bacs à sable%nSélectionnez cette option pour supprimer tous les fichiers de configuration et les bacs à sable, y compris le dossier « Sandbox » situé dans les chemins personnalisés avec « FileRootPath ».

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
german.UninstallTaskLabel3=&Konfiguration und Sandboxen behalten%nDies ist die empfohlene Option, wenn Sie vorhaben Sandboxie-Plus erneut zu intallieren, während Sandboxie-Plus alle Sandboxen und Konfigurationsdateien behält.
german.UninstallTaskLabel4=&Entferne die Konfiguration%nWählen Sie diese Option um die Sandboxie.ini und Sandboxie-Plus.ini Dateien zu entfernen, während die Sandboxen unverändert bleiben.
german.UninstallTaskLabel5=Entferne die Konfiguration und alle &Sandboxen%nWählen Sie diese Option um alle Konfigurationsdateien und alle Sandboxen, inklusive des Sandboxordners im, durch die Dateisystemquelle, benutzerdefinierten Pfad.
; Fix issue #1333 with a space inserted before %1
german.AutoStartProgram=Starte automatisch %1

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
Italian.UninstallTaskLabel3=&Mantieni i file di configurazione e le aree virtuali%nSelezionare questa opzione se si prevede di reinstallare Sandboxie in un secondo momento, conservando le impostazioni e le aree virtuali.
Italian.UninstallTaskLabel4=Rimuovi i file di &configurazione%nSelezionare questa opzione per rimuovere i file di configurazione Sandboxie.ini e Sandboxie-Plus.ini, lasciando invariate le aree virtuali presenti.
Italian.UninstallTaskLabel5=Rimuovi i file di configurazione e le &aree virtuali%nSelezionare questa opzione per rimuovere tutti i file di configurazione e le aree virtuali, inclusa la cartella Sandbox nei percorsi personalizzati mediante FileRootPath.

; Polish
polish.AddSandboxedMenu=Dodaj "Uruchom w piaskownicy" do menu kontekstowego
polish.CustomPageLabel1=Wybierz typ instalacji
polish.CustomPageLabel2=Jak należy instalować
polish.CustomPageLabel3=Wybierz tryb instalacji
polish.CustomPageInstallMode=Zainstaluj {#MyAppName} na tym komputerze
polish.CustomPageUpgradeMode=Aktualizacja istniejącej instalacji {#MyAppName}
polish.CustomPagePortableMode=Wypakuj wszystkie pliki do katalogu, aby użyć trybu przenośnego
polish.RequiresWin7OrLater=Sandboxie-Plus wymaga systemu Windows 7 lub nowszego.
polish.ClassicFound=Wykryto instalację Sandboxie Classic, która musi być najpierw odinstalowana, czy chcesz ją teraz odinstalować?
polish.RunSandboxedMenu=Uruchom w &Sandboxie
polish.UninstallTaskLabel1=Wybierz typ dezinstalacji
polish.UninstallTaskLabel2=W jaki sposób chcesz odinstalować Sandboxie-Plus?
polish.UninstallTaskLabel3=Zachowaj pliki &konfiguracyjne i piaskownice%nJest to najbardziej zalecana opcja, jeśli planujesz reinstalować Sandboxie-Plus, zachowując pliki konfiguracyjne i piaskownice.
polish.UninstallTaskLabel4=Usuń pliki konfigura&cyjne%nWybierz tę opcję, aby usunąć pliki konfiguracyjne Sandboxie.ini i Sandboxie-Plus.ini, zachowując piaskownice bez zmian.
polish.UninstallTaskLabel5=Usuń pliki konfiguracyjne i pia&skownice%nWybierz tę opcję, aby usunąć wszystkie pliki konfiguracyjne i piaskownice, w tym folder Sandbox znajdujący się w niestandardowych ścieżkach z FileRootPath.

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
portuguese.UninstallTaskLabel1=Selecionar Tipo de Desinstalar
portuguese.UninstallTaskLabel2=Como você gostaria de desinstalar Sandboxie-Plus?
portuguese.UninstallTaskLabel3=&Manter os arquivos de configuração do sandboxes%nEsta é a opção mais recomendada se você planeja reinstalar o Sandboxie-Plus enquanto mantém seus arquivos de configuração e caixas de areia.
portuguese.UninstallTaskLabel4=&Remover arquivos de configuração%nSelecione essa opção para remover os arquivos de configuração Sandboxie.ini e Sandboxie-Plus.ini mantendo as caixas de areia inalteradas.
portuguese.UninstallTaskLabel5=Remover arquivos de configuração e &caixas de areia%nSelecione essa opção para remover todos os arquivos de configuração e caixas de areia, incluindo a pasta Sandbox localizada em caminhos personalizados com FileRootPath.

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
russian.UninstallTaskLabel1=Выберите тип удаления
russian.UninstallTaskLabel2=Как вы хотите удалить Sandboxie-Plus?
russian.UninstallTaskLabel3=Сохранять файлы конфигурации и песочницы (&K)%nЭто наиболее рекомендуемый вариант, если вы планируете переустановить Sandboxie-Plus, сохранив файлы конфигурации и песочницы.
russian.UninstallTaskLabel4=Удалить файлы конфигурации (&C)%nВыберите этот параметр, чтобы удалить файлы конфигурации Sandboxie.ini и Sandboxie-Plus.ini, оставив файлы песочницы без изменений.
russian.UninstallTaskLabel5=Удалить файлы конфигурации и песочницы (&S)%nВыберите этот параметр, чтобы удалить все файлы конфигурации и песочницы, включая папку Sandbox, расположение которой заданно в FileRootPath.

; Spanish
spanish.AddSandboxedMenu=Agregar "Ejecutar Aislado en una Sandbox" al menú contextual
spanish.CustomPageLabel1=Seleccione el tipo de instalación
spanish.CustomPageLabel2=¿Cómo se debe instalar?
spanish.CustomPageLabel3=Elija el modo de instalación
spanish.CustomPageInstallMode=Instalar {#MyAppName} en esta computadora
spanish.CustomPageUpgradeMode=Actualizar la instalación existente de {#MyAppName}
spanish.CustomPagePortableMode=Extraiga todos los archivos a un directorio para uso portátil
spanish.RequiresWin7OrLater=Sandboxie-Plus requiere Windows 7 o posterior.
spanish.ClassicFound=Se detectó la instalación de Sandboxie Classic, primero debe desinstalarse. ¿Quieres desinstalarlo ahora?
spanish.RunSandboxedMenu=&Ejecutar Aislado en una Sandbox
spanish.UninstallTaskLabel1=Seleccione el tipo de desinstalación
spanish.UninstallTaskLabel2=¿Cómo le gustaría desinstalar Sandboxie-Plus?
spanish.UninstallTaskLabel3=&Mantenga los archivos de configuración y las sandboxes%nEsta es la opción más recomendada si planea reinstalar Sandboxie-Plus mientras mantiene sus archivos de configuración y las sandboxes.
spanish.UninstallTaskLabel4=Eliminar archivos de &configuración%nSeleccione esta opción para eliminar los archivos de configuración Sandboxie.ini y Sandboxie-Plus.ini manteniendo las sandboxes sin cambios.
spanish.UninstallTaskLabel5=Eliminar archivos de configuración y las &sandboxes%nSeleccione esta opción para eliminar todos los archivos de configuración y las sandboxes, incluida la carpeta Sandbox ubicada en rutas personalizadas mediante FileRootPath.
