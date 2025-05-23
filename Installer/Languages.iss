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
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "Italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "Korean"; MessagesFile: "isl\Korean.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "swedish"; MessagesFile: "isl\Swedish.isl"
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"
Name: "vietnamese"; MessagesFile: "isl\Vietnamese.isl"
;Name: "armenian"; MessagesFile: "compiler:Languages\Armenian.isl"
;Name: "bulgarian"; MessagesFile: "compiler:Languages\Bulgarian.isl"
;Name: "catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
;Name: "corsican"; MessagesFile: "compiler:Languages\Corsican.isl"
;Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
;Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
;Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
;Name: "hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"
;Name: "icelandic"; MessagesFile: "compiler:Languages\Icelandic.isl"
;Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
;Name: "slovak"; MessagesFile: "compiler:Languages\Slovak.isl"
;Name: "slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"


[CustomMessages]

; English
english.AddSandboxedMenu=Add "Run Sandboxed" to context menu
english.AddSandboxedBrowser=Add desktop shortcut for starting Web browser under Sandboxie
english.SandboxieStartMenu1=Run any program sandboxed
english.SandboxieStartMenu2=Run Web browser sandboxed
english.SandboxieStartMenu3=Sandboxie Start Menu
english.SandboxedBrowser=Sandboxed Web Browser
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
english.RefreshBuild=Download latest Templates.ini and translations
english.InstallImDisk=Install (or Update) ImDisk 3.0 driver (for RamDisk and Encrypted Sandboxes)

; Chinese Simplified
ChineseSimplified.AddSandboxedMenu=添加“在沙箱中运行”右键菜单
ChineseSimplified.AddSandboxedBrowser=添加用于在 Sandboxie 下启动网页浏览器的桌面快捷方式
ChineseSimplified.SandboxieStartMenu1=在沙箱中运行程序
ChineseSimplified.SandboxieStartMenu2=在沙箱中运行网页浏览器
ChineseSimplified.SandboxieStartMenu3=Sandboxie 开始菜单
ChineseSimplified.SandboxedBrowser=在沙箱中运行网页浏览器
ChineseSimplified.CustomPageLabel1=选择安装方式
ChineseSimplified.CustomPageLabel2=应当如何安装
ChineseSimplified.CustomPageLabel3=选择安装模式
ChineseSimplified.CustomPageInstallMode=在这台计算机上安装 {#MyAppName}
ChineseSimplified.CustomPageUpgradeMode=更新当前已安装的 {#MyAppName}
ChineseSimplified.CustomPagePortableMode=提取全部文件到指定目录供便携化使用
ChineseSimplified.RequiresWin7OrLater=Sandboxie-Plus 需要 Windows 7 或更高版本。
ChineseSimplified.ClassicFound=检测到已安装 Sandboxie Classic 版本，如要继续安装必须先将其卸载，是否开始卸载？
ChineseSimplified.RunSandboxedMenu=在沙箱中运行(&X)
ChineseSimplified.UninstallTaskLabel1=选择卸载方式
ChineseSimplified.UninstallTaskLabel2=要如何卸载 Sandboxie-Plus？
ChineseSimplified.UninstallTaskLabel3=保留配置文件和所有沙箱(&K)%n如果您打算保留当前配置并重新安装 Sandboxie-Plus，请选择此项。
ChineseSimplified.UninstallTaskLabel4=移除配置文件(&C)%n选择此选项将移除 Sandboxie.ini 和 Sandboxie-Plus.ini 配置文件，但会保留您在 Sandboxie-Plus 中创建的沙箱。
ChineseSimplified.UninstallTaskLabel5=移除配置文件和所有沙箱(&S)%n选择此选项将会移除所有配置文件和沙箱，包括 FileRootPath 所定义的路径下的 Sandbox 文件夹。
ChineseSimplified.RefreshBuild=下载最新的 Templates.ini 模板和翻译
ChineseSimplified.InstallImDisk=安装（或更新）ImDisk 3.0驱动程序（用于内存盘和加密沙箱）。

; Chinese Traditional
ChineseTraditional.AddSandboxedMenu=加入「在沙箱中執行」選單
ChineseTraditional.AddSandboxedBrowser=在桌面放置從沙盤開啟網頁瀏覽器的捷徑
ChineseTraditional.SandboxieStartMenu1=在沙盤中執行應用程式
ChineseTraditional.SandboxieStartMenu2=在沙盤中開啟網頁瀏覽器
ChineseTraditional.SandboxieStartMenu3=Sandboxie 開始功能表
ChineseTraditional.SandboxedBrowser=從沙盤開啟網頁瀏覽器
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
ChineseTraditional.RefreshBuild=下載最新的 Templates.ini 和翻譯

; Dutch
dutch.AddSandboxedMenu="In sandbox uitvoeren" aan contextmenu toevoegen
dutch.AddSandboxedBrowser=Bureaubladsnelkoppeling toevoegen om webbrowser onder Sandboxie te starten
dutch.SandboxieStartMenu1=Programma gesandboxt uitvoeren
dutch.SandboxieStartMenu2=Webbrowser gesandboxt uitvoeren
dutch.SandboxieStartMenu3=Sandboxie-startmenu
dutch.SandboxedBrowser=Gesandboxte webbrowser
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
dutch.RefreshBuild=Download de nieuwste Templates.ini en vertalingen

; French
french.AddSandboxedMenu=Ajouter « Exécuter dans un bac à sable » au menu contextuel
french.AddSandboxedBrowser=Ajouter un raccourci sur le bureau pour exécuter le navigateur web dans Sandboxie
french.SandboxieStartMenu1=Exécuter un programme dans un bac à sable
french.SandboxieStartMenu2=Exécuter le navigateur web dans un bac à sable
french.SandboxieStartMenu3=Menu Démarrer de Sandboxie
french.SandboxedBrowser=Navigateur web dans un bac à sable
french.CustomPageLabel1=Sélectionner le type d'installation
french.CustomPageLabel2=Comment doit-on l'installer ?
french.CustomPageLabel3=Choisir le mode d'installation
french.CustomPageInstallMode=Installer {#MyAppName} sur cet ordinateur
french.CustomPageUpgradeMode=Mettre à jour l'installation de {#MyAppName} existante
french.CustomPagePortableMode=Extraire tous les fichiers dans un répertoire pour une utilisation portable
french.RequiresWin7OrLater=Sandboxie-Plus nécessite Windows 7 ou une version ultérieure.
french.ClassicFound=Installation de Sandboxie Classic détectée ; il doit être désinstallé d'abord, voulez-vous le désinstaller maintenant ?
french.RunSandboxedMenu=Exécuter dans un &bac à sable
french.UninstallTaskLabel1=Sélectionner le type de désinstallation
french.UninstallTaskLabel2=Comment voulez-vous désinstaller Sandboxie-Plus ?
french.UninstallTaskLabel3=&Conserver les fichiers de configuration et les bacs à sable%nCette option est la plus recommandée si vous prévoyez de réinstaller Sandboxie-Plus tout en conservant vos fichiers de configuration et vos bacs à sable.
french.UninstallTaskLabel4=Supprimer les &fichiers de configuration%nSélectionnez cette option pour supprimer les fichiers de configuration « Sandboxie.ini » et « Sandboxie-Plus.ini » tout en conservant les bacs à sable inchangés.
french.UninstallTaskLabel5=Supprimer les fichiers de configuration et les &bacs à sable%nSélectionnez cette option pour supprimer tous les fichiers de configuration et les bacs à sable, y compris le dossier « Sandbox » situé dans les chemins personnalisés avec « FileRootPath ».
french.RefreshBuild=Télécharger les dernières versions de Templates.ini et des traductions
french.InstallImDisk=Installer (ou mettre à jour) le pilote ImDisk 3.0 (pour disque de mémoire vive ou bac à sable chiffré)

; German
german.AddSandboxedMenu=Füge "Starte Sandgeboxt" zum Kontextmenü hinzu
german.AddSandboxedBrowser=Erstelle Desktop-Verknüpfung zum Starten des Standard-Webbrowsers mit Sandboxie
german.SandboxieStartMenu1=Programm mit Sandboxie starten
german.SandboxieStartMenu2=Webbrowser mit Sandboxie starten
german.SandboxieStartMenu3=Sandboxie Startmenü
german.SandboxedBrowser=Sandgeboxter Webbrowser
german.CustomPageLabel1=Wählen Sie die Installationsart aus
german.CustomPageLabel2=Wie soll installiert werden?
german.CustomPageLabel3=Wählen Sie die Installationsmethode aus
german.CustomPageInstallMode=Installiere {#MyAppName} auf diesem Computer
german.CustomPageUpgradeMode=Aktualisiere bestehende {#MyAppName} Installation
german.CustomPagePortableMode=Extrahiere alle Dateien in einen Ordner zur portablen Verwendung
german.RequiresWin7OrLater=Sandboxie-Plus erfordert Windows 7 oder neuer.
german.ClassicFound=Sandboxie Classic Installation gefunden, welche vorher deinstalliert werden muss. Möchten Sie diese nun deinstallieren?
german.RunSandboxedMenu=Starte &Sandgeboxt
german.UninstallTaskLabel1=Wählen Sie die Deinstallationsart aus
german.UninstallTaskLabel2=Wie möchten Sie Sandboxie-Plus deinstallieren?
german.UninstallTaskLabel3=&Behalte Konfigurationsdateien und Sandboxen%nDies ist die empfohlene Option, wenn Sie vorhaben Sandboxie-Plus erneut zu installieren und dabei Ihre Konfigurationsdateien und Sandboxen behalten möchten.
german.UninstallTaskLabel4=Entferne &Konfigurationsdateien%nWählen Sie diese Option, um die Konfigurationsdateien Sandboxie.ini und Sandboxie-Plus.ini zu entfernen, während die Sandboxen unverändert bleiben.
german.UninstallTaskLabel5=Entferne Konfigurationsdateien und &Sandboxen%nWählen Sie diese Option, um alle Konfigurationsdateien und Sandboxen zu entfernen, einschließlich des Sandboxordners im, durch die Einstellung Dateisystemquelle (FileRootPath), benutzerdefinierten Pfad.
german.RefreshBuild=Neueste Templates.ini und Übersetzungen herunterladen
german.InstallImDisk=Installiere (oder Aktualisiere) ImDisk 3.0 Treiber (für Ramdisk und verschlüsselte Sandboxen)
; Fix issue #1333 with a space inserted before %1
german.AutoStartProgram=Starte automatisch %1

; Hungarian
hungarian.AddSandboxedMenu=Az "Izolált módú futtatás" hozzáadása a helyi menühöz
hungarian.AddSandboxedBrowser=Asztali parancsikon hozzáadása a böngésző izolált módú indításához
hungarian.SandboxieStartMenu1=Bármilyen program futtatása izolált módban
hungarian.SandboxieStartMenu2=Webböngésző futtatása izolált módban
hungarian.SandboxieStartMenu3=Sandboxie start menü
hungarian.SandboxedBrowser=Izolált böngésző
hungarian.CustomPageLabel1=Válassza ki a telepítés típusát
hungarian.CustomPageLabel2=Hogyan kell telepíteni?
hungarian.CustomPageLabel3=Válassza ki a telepítési módot
hungarian.CustomPageInstallMode=Telepítse a {#MyAppName} alkalmazást erre a számítógépre
hungarian.CustomPageUpgradeMode=Frissítse a meglévő {#MyAppName} telepítést
hungarian.CustomPagePortableMode=Csomagolja ki az összes fájlt egy könyvtárba hordozható használatra
hungarian.RequiresWin7OrLater=Sandboxie-Plus alkalmazáshoz Windows 7 vagy újabb rendszer szükséges.
hungarian.ClassicFound=A Sandboxie Classic telepítése észlelve, melyet először el kell távolítani. Eltávolítja most?
hungarian.RunSandboxedMenu=Futtatás izolált módban
hungarian.UninstallTaskLabel1=Válassza ki az eltávolítás típusát
hungarian.UninstallTaskLabel2=Hogyan távolítja el a Sandboxie-Plus-t?
hungarian.UninstallTaskLabel3=A konfigurációs fájlok és homokozók megtartása%nEz a leginkább ajánlott lehetőség, ha a Sandboxie-Plus újratelepítését tervezi, miközben megtartja konfigurációs fájljait és homokozóit.
hungarian.UninstallTaskLabel4=A konfigurációs fájlok eltávolítása%nVálassza ezt a lehetőséget a Sandboxie.ini és a Sandboxie-Plus.ini konfigurációs fájlok eltávolításához, miközben a homokozók változatlanok maradnak.
hungarian.UninstallTaskLabel5=A konfigurációs fájlok és homokozók eltávolítása%nVálassza ezt a lehetőséget az összes konfigurációs fájl és homokozó eltávolításához, beleértve a Sandbox mappát is, amely a FileRootPath egyéni elérési útjain található.
hungarian.RefreshBuild=Töltse le a legújabb Templates.ini-t és a fordításokat

; Italian
Italian.AddSandboxedMenu=Aggiungi l'opzione "Avvia nell'area virtuale" al menu contestuale
Italian.AddSandboxedBrowser=Aggiungi un collegamento sul desktop per l'avvio del browser Web in Sandboxie
Italian.SandboxieStartMenu1=Avvia un programma nell'area virtuale
Italian.SandboxieStartMenu2=Avvia il browser Web nell'area virtuale
Italian.SandboxieStartMenu3=Menu Start di Sandboxie
Italian.SandboxedBrowser=Browser Web nell'area virtuale
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
Italian.RefreshBuild=Aggiorna Templates.ini e translations.7z

; Japanese
japanese.AddSandboxedMenu="サンドボックス化して実行" を右クリックに追加
japanese.AddSandboxedBrowser=Sandboxie 下で Web ブラウザを起動するショートカットをデスクトップに追加する
japanese.SandboxieStartMenu1=プログラムを指定してサンドボックス化して実行
japanese.SandboxieStartMenu2=Web ブラウザをサンドボックス化して実行
japanese.SandboxieStartMenu3=Sandboxie スタートメニュー
japanese.SandboxedBrowser=サンドボックス化した Web ブラウザ
japanese.CustomPageLabel1=インストールタイプを選択
japanese.CustomPageLabel2=どのようにインストールしますか
japanese.CustomPageLabel3=インストールモードを選択してください
japanese.CustomPageInstallMode=このコンピューターに {#MyAppName} をインストール
japanese.CustomPageUpgradeMode=既存の {#MyAppName} インストールを上書き
japanese.CustomPagePortableMode=ポータブル利用のためにすべてのファイルを展開する
japanese.RequiresWin7OrLater=Sandboxie-Plus は Windows7 以降が必要です
japanese.ClassicFound=Sandboxie Classic がインストールされています。最初にそれをアンインストールしなければなりません。今すぐアンインストールしますか？
japanese.RunSandboxedMenu=サンドボックス化して実行(&S)
japanese.UninstallTaskLabel1=アンインストールの種類を選択
japanese.UninstallTaskLabel2=どのように Sandboxie-Plus をアンインストールしますか？
japanese.UninstallTaskLabel3=構成ファイルとサンドボックスを保持する(&K)%n構成ファイルとサンドボックスを保持したまま Sandboxie-Plus を再インストールする予定なら、これが最も推奨されるオプションです。
japanese.UninstallTaskLabel4=構成ファイルを削除する(&C)%nSandboxie.ini と Sandboxie-Plus.ini 構成ファイルを削除するには、このオプションを選択します。サンドボックスは変更しません。
japanese.UninstallTaskLabel5=構成とサンドボックスを削除する(&S)%n全ての構成ファイルとサンドボックスを削除するには、このオプションを選択します。FileRootPath のカスタムパスにサンドボックスフォルダーも含めて削除します。
japanese.RefreshBuild=最新の Templates.ini と翻訳をダウンロードする

; Korean
Korean.AddSandboxedMenu=상황에 맞는 메뉴에 "샌드박스에서 실행" 추가
Korean.AddSandboxedBrowser=Sandboxie에서 웹 브라우저를 시작하기 위한 바탕 화면 바로 가기 추가
Korean.SandboxieStartMenu1=샌드박스에서 모든 프로그램 실행
Korean.SandboxieStartMenu2=샌드박스에서 웹 브라우저 실행
Korean.SandboxieStartMenu3=Sandboxie 시작 메뉴
Korean.SandboxedBrowser=샌드박스에서 웹 브라우저
Korean.CustomPageLabel1=설치 유형 선택
Korean.CustomPageLabel2=설치 방법
Korean.CustomPageLabel3=설치 모드 선택
Korean.CustomPageInstallMode=이 컴퓨터에 {#MyAppName} 설치
Korean.CustomPageUpgradeMode=기존 {#MyAppName} 설치 업데이트
Korean.CustomPagePortableMode=휴대용 사용을 위해 모든 파일을 디렉터리에 추출
Korean.RequiresWin7OrLater=Sandboxie-Plus를 사용하려면 Windows 7 이상이 필요합니다.
Korean.ClassicFound=Sandboxie Classic 설치가 탐지되었습니다. 먼저 제거해야 합니다. 지금 제거하시겠습니까?
Korean.RunSandboxedMenu=샌드박스에서 실행(&S)
Korean.UninstallTaskLabel1=제거 유형 선택
Korean.UninstallTaskLabel2=Sandboxie-Plus를 제거하시겠습니까?
Korean.UninstallTaskLabel3=구성 파일 및 샌드박스 유지(&K)%n구성 파일과 샌드박스를 유지하면서 Sandboxie-Plus를 다시 설치하려는 경우 이 옵션을 사용하는 것이 가장 좋습니다.
Korean.UninstallTaskLabel4=구성 파일 제거(&C)%n샌드박스를 변경하지 않고 Sandboxie.ini 및 Sandboxie-Plus.ini 구성 파일을 제거하려면 이 옵션을 선택합니다.
Korean.UninstallTaskLabel5=구성 파일 및 샌드박스 제거(&S)%nFileRootPath를 사용하여 사용자 지정 경로에 있는 샌드박스 폴더를 포함하여 모든 구성 파일과 샌드박스를 제거하려면 이 옵션을 선택하십시오.
Korean.RefreshBuild=최신 Templates.ini 및 번역 다운로드

; Polish
polish.AddSandboxedMenu=Dodaj "Uruchom w piaskownicy" do menu kontekstowego
polish.AddSandboxedBrowser=Dodaj skrót pulpitu dla przeglądarki internetowej uruchamianej w Sandboxie
polish.SandboxieStartMenu1=Uruchom dowolny program w piaskownicy
polish.SandboxieStartMenu2=Uruchom przeglądarkę internetową w piaskownicy
polish.SandboxieStartMenu3=Menu start w Sandboxie
polish.SandboxedBrowser=Przeglądarka internetowa w piaskownicy
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
polish.RefreshBuild=Pobierz najnowsze Templates.ini i tłumaczenia
polish.InstallImDisk=Zainstaluj (lub Uaktualnij) sterownik ImDisk 3.0 (dla RamDisk i zaszyfrowanych sandboxów)

; Brazilian Portuguese
brazilianportuguese.AddSandboxedMenu=Adicionar "Executar na Caixa de Areia" no menu de contexto
brazilianportuguese.AddSandboxedBrowser=Adicionar um atalho na área de trabalho para iniciar o Navegador web sob o Sandboxie
brazilianportuguese.SandboxieStartMenu1=Executar qualquer programa em uma caixa de areia
brazilianportuguese.SandboxieStartMenu2=Executar o Navegador web em uma caixa de areia
brazilianportuguese.SandboxieStartMenu3=Menu Iniciar no Sandboxie
brazilianportuguese.SandboxedBrowser=Navegador web em uma caixa
brazilianportuguese.CustomPageLabel1=Selecione Tipo de Instalação
brazilianportuguese.CustomPageLabel2=Como deve ser instalado
brazilianportuguese.CustomPageLabel3=Escolha o modo de instalação
brazilianportuguese.CustomPageInstallMode=Instalar {#MyAppName} neste computador
brazilianportuguese.CustomPageUpgradeMode=Atualizar instalação existente do {#MyAppName}
brazilianportuguese.CustomPagePortableMode=Extrair todos os arquivos para um diretório para uso portable
brazilianportuguese.RequiresWin7OrLater=Sandboxie-Plus requer o Windows 7 ou posterior.
brazilianportuguese.ClassicFound=Instalação do Classic Sandboxie detectada, deve ser desinstalado primeiro, você quer desinstalá-lo agora?
brazilianportuguese.RunSandboxedMenu=Executar na &Caixa de Areia
brazilianportuguese.UninstallTaskLabel1=Selecionar Tipo de Desinstalar
brazilianportuguese.UninstallTaskLabel2=Como você gostaria de desinstalar Sandboxie-Plus?
brazilianportuguese.UninstallTaskLabel3=&Manter os arquivos de configuração do sandboxes%nEsta é a opção mais recomendada se você planeja reinstalar o Sandboxie-Plus enquanto mantém seus arquivos de configuração e caixas de areia.
brazilianportuguese.UninstallTaskLabel4=&Remover arquivos de configuração%nSelecione essa opção para remover os arquivos de configuração Sandboxie.ini e Sandboxie-Plus.ini mantendo as caixas de areia inalteradas.
brazilianportuguese.UninstallTaskLabel5=Remover arquivos de configuração e &caixas de areia%nSelecione essa opção para remover todos os arquivos de configuração e caixas de areia, incluindo a pasta Sandbox localizada em caminhos personalizados com FileRootPath.
brazilianportuguese.RefreshBuild=Baixar mais recentes Templates.ini e traduções

; Portuguese
portuguese.AddSandboxedMenu=Adicionar "Roda na Caixa de Areia" para o menu de contexto
portuguese.AddSandboxedBrowser=Adicionar um atalho na área de trabalho para iniciar o Navegador web sob o Sandboxie
portuguese.SandboxieStartMenu1=Rodar qualquer programa em uma caixa de areia
portuguese.SandboxieStartMenu2=Rodar o Navegador web em uma caixa de areia
portuguese.SandboxieStartMenu3=Menu Iniciar no Sandboxie
portuguese.SandboxedBrowser=Navegador web em uma caixa
portuguese.CustomPageLabel1=Selecione Tipo de Instalação
portuguese.CustomPageLabel2=Como deve ser instalado
portuguese.CustomPageLabel3=Escolha o modo de instalação
portuguese.CustomPageInstallMode=Instalar {#MyAppName} neste computador
portuguese.CustomPageUpgradeMode=Atualizar instalação existente do {#MyAppName}
portuguese.CustomPagePortableMode=Extrair todos os ficheiro para um diretório para uso portable
portuguese.RequiresWin7OrLater=Sandboxie-Plus requer o Windows 7 ou posterior.
portuguese.ClassicFound=Instalação do Classic Sandboxie detectada, deve ser desinstalado primeiro, você quer desinstalá-lo agora?
portuguese.RunSandboxedMenu=Rodar na &Caixa de Areia
portuguese.UninstallTaskLabel1=Selecionar Tipo de Desinstalar
portuguese.UninstallTaskLabel2=Como você gostaria de desinstalar Sandboxie-Plus?
portuguese.UninstallTaskLabel3=&Manter os ficheiro de definição do sandboxes%nEsta é a opção mais recomendada se você planeja reinstalar o Sandboxie-Plus enquanto mantém seus ficheiro de definição e caixas de areia.
portuguese.UninstallTaskLabel4=&Remover ficheiro de definição%nSelecione essa opção para remover os ficheiro de definição Sandboxie.ini e Sandboxie-Plus.ini mantendo as caixas de areia inalteradas.
portuguese.UninstallTaskLabel5=Remover ficheiro de definição e &caixas de areia%nSelecione essa opção para remover todos os ficheiro de definição e caixas de areia, incluindo a pasta Sandbox localizada em caminhos personalizados com FileRootPath.
portuguese.RefreshBuild=Descarregar mais novos Templates.ini e traduções

; Russian
russian.AddSandboxedMenu=Добавить "Запустить в песочнице" в контекстное меню
russian.AddSandboxedBrowser=Добавить ярлык на рабочий стол для запуска браузера в Sandboxie
russian.SandboxieStartMenu1=Запустить программу в песочнице
russian.SandboxieStartMenu2=Запустить веб браузер в песочнице
russian.SandboxieStartMenu3=Начальное меню Sandboxie
russian.SandboxedBrowser=Браузер в песочнице
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
russian.RefreshBuild=Скачать последние Templates.ini и переводы

; Spanish
spanish.AddSandboxedMenu=Agregar "Ejecutar Aislado en una Sandbox" al menú contextual
spanish.AddSandboxedBrowser=Agregar Acceso Directo al Escritorio para iniciar el Navegador Web supervisado por Sandboxie
spanish.SandboxieStartMenu1=Ejecutar un programa aislado en la sandbox
spanish.SandboxieStartMenu2=Ejecutar Navegador Web aislado en una sandbox
spanish.SandboxieStartMenu3=Menú de Inicio de Sandboxie
spanish.SandboxedBrowser=Navegador Web Aislado en una Sandbox
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
spanish.RefreshBuild=Descargar la última versión de Templates.ini y traducciones

; Swedish
swedish.AddSandboxedMenu=Lägg till Kör sandlådad, till snabbmenyn
swedish.AddSandboxedBrowser=Lägg till skrivbordsgenväg för att starta webbläsaren under Sandboxie
swedish.SandboxieStartMenu1=Kör valfritt program sandlådat
swedish.SandboxieStartMenu2=Kör webbläsaren sandlådad
swedish.SandboxieStartMenu3=Sandboxie startmeny
swedish.SandboxedBrowser=Sandlådad webbläsare
swedish.CustomPageLabel1=Välj installationstyp
swedish.CustomPageLabel2=Hur ska installeringen ske
swedish.CustomPageLabel3=Välj installationsläget
swedish.CustomPageInstallMode=Installera {#MyAppName} på denna dator
swedish.CustomPageUpgradeMode=Uppdatera existerande {#MyAppName} installation
swedish.CustomPagePortableMode=Extrahera alla filer till en katalog för portabel användning
swedish.RequiresWin7OrLater=Sandboxie-Plus kräver Windows 7 eller senare.
swedish.ClassicFound=Upptäckt en installation av Sandboxie Classic, den behöver avinstalleras först. Vill du avinstallera den nu?
swedish.RunSandboxedMenu=Kör &sandlådad
swedish.UninstallTaskLabel1=Välj avinstallationstyp
swedish.UninstallTaskLabel2=Hur vill du avinstallera Sandboxie-Plus?
swedish.UninstallTaskLabel3=&Behåll konfigureringsfiler och sandlådor%nDetta är det rekommenderade alternativet om du tänker installera om Sandboxie-Plus, men vill behålla dina konfigurationsfiler och sandlådor.
swedish.UninstallTaskLabel4=Ta bort &konfigurationsfilerna%nVälj detta alternativ för att ta bort Sandboxie.ini och Sandboxie-Plus.ini:s konfigurationsfiler men behålla sandlådorna oförändrade.
swedish.UninstallTaskLabel5=Ta bort konfigurationsfiler och &sandlådor%nVälj detta alternativ för att ta bort alla konfigurationsfiler och sandlådor, även sandlådemappen lokaliserad i anpassade sökvägar med filrootsökvägen.
swedish.RefreshBuild=Nedladda senaste Mallar.ini och översättningar
swedish.InstallImDisk=Installera (eller uppdatera) ImDisk 3.0-drivrutinen (för RamDisk och krypterade sandlådor)

; Turkish
turkish.AddSandboxedMenu=Bağlam menüsüne "Korumalı Alanda Çalıştır" seçeneği ekle
turkish.AddSandboxedBrowser=Sandboxie altında Web tarayıcısını başlatmak için masaüstü kısayolu ekle
turkish.SandboxieStartMenu1=Korumalı alanda herhangi bir program çalıştır
turkish.SandboxieStartMenu2=Korumalı alanda Web tarayıcısını çalıştır
turkish.SandboxieStartMenu3=Sandboxie Başlangıç Menüsü
turkish.SandboxedBrowser=Korumalı Web Tarayıcısı
turkish.CustomPageLabel1=Kurulum Türünü Seçin
turkish.CustomPageLabel2=Nasıl kurulacağını seçin
turkish.CustomPageLabel3=Yükleme modunu seçin
turkish.CustomPageInstallMode={#MyAppName} uygulamasını bu bilgisayara kur
turkish.CustomPageUpgradeMode=Mevcut {#MyAppName} kurulumunu güncelle
turkish.CustomPagePortableMode=Taşınabilir kullanım için tüm dosyaları bir dizine çıkar
turkish.RequiresWin7OrLater=Sandboxie-Plus, Windows 7 veya sonraki bir sürümünü gerektirir.
turkish.ClassicFound=Sandboxie Classic kurulumu algılandı, önce kaldırılması gerekiyor, şimdi kaldırmak istiyor musunuz?
turkish.RunSandboxedMenu=&Korumalı Alanda Çalıştır
turkish.UninstallTaskLabel1=Kaldırma Türünü Seçin
turkish.UninstallTaskLabel2=Sandboxie-Plus'ı nasıl kaldırmak istersiniz?
turkish.UninstallTaskLabel3=Yapılandırma dosyalarını ve korumalı alanları &sakla%nYapılandırma dosyalarınızı ve korumalı alanlarınızı saklarken Sandboxie-Plus'ı yeniden yüklemeyi planlıyorsanız, bu en çok önerilen seçenektir.
turkish.UninstallTaskLabel4=&Yapılandırma dosyalarını kaldır%nSandboxie.ini ve Sandboxie-Plus.ini yapılandırma dosyalarını, korumalı alanları değiştirmeden kaldırmak için bunu seçin.
turkish.UninstallTaskLabel5=Yapılandırma dosyalarını ve &korumalı alanları kaldır%nFileRootPath ile özel yollarda bulunan Sandbox klasörü dahil tüm yapılandırma dosyalarını ve korumalı alanları kaldırmak için bunu seçin.
turkish.RefreshBuild=En son Templates.ini ve çeviriler indirilsin
turkish.InstallImDisk=ImDisk 3.0 sürücüsü yüklensin (veya güncellensin) (RamDisk ve Şifrelenmiş Korumalı Alanlar için)

; Ukrainian
ukrainian.AddSandboxedMenu=Додати "Виконати в пісочниці" в контекстне меню
ukrainian.AddSandboxedBrowser=Додати ярлик на робочий стіл для запуску браузера в Sandboxie
ukrainian.SandboxieStartMenu1=Запустити програму у пісочниці
ukrainian.SandboxieStartMenu2=Запустити веб-браузер у пісочниці
ukrainian.SandboxieStartMenu3=Початкове меню Sandboxie
ukrainian.SandboxedBrowser=Браузер у пісочниці
ukrainian.CustomPageLabel1=Оберіть тип встановки
ukrainian.CustomPageLabel2=Як потрібно встановити
ukrainian.CustomPageLabel3=Оберіть режим встановки
ukrainian.CustomPageInstallMode=Встановити {#MyAppName} на цьому пристрої
ukrainian.CustomPageUpgradeMode=Оновити існуючу встановку {#MyAppName}
ukrainian.CustomPagePortableMode=Витягти всі файли у каталог для портативного режиму
ukrainian.RequiresWin7OrLater=Для Sandboxie-Plus потрібно Windows 7 або новіше.
ukrainian.ClassicFound=Виявлена версія Sandboxie Classic, на початку встановлення її потрібно видалити. Видалити зараз?
ukrainian.RunSandboxedMenu=Виконати в пісочниці (&S)
ukrainian.UninstallTaskLabel1=Оберіть тип видалення
ukrainian.UninstallTaskLabel2=Як ви хочете видалити Sandboxie-Plus?
ukrainian.UninstallTaskLabel3=Зберігти файли конфігурації и пісочниць (&K)%nЦе найбільш рекомендований варіант, якщо ви плануете перевстановити Sandboxie-Plus, зберігши файли конфігурації и пісочниць.
ukrainian.UninstallTaskLabel4=Видалити файли конфігурації (&C)%nОберіть цей параметр, щоб видалити файли конфігурації Sandboxie.ini та Sandboxie-Plus.ini, залишивши файли пісочниць без змін.
ukrainian.UninstallTaskLabel5=Видалити файли конфігурації та пісочниць (&S)%nОберіть цей параметр, щоб видалити всі файли конфігурації та пісочниць, включаючи папку Sandbox, яка має місцерозташування в FileRootPath.
ukrainian.RefreshBuild=Завантажити останні Templates.ini та переклади

; Vietnamese
vietnamese.AddSandboxedMenu=Thêm "Chạy trong Sandbox" vào menu ngữ cảnh
vietnamese.AddSandboxedBrowser=Thêm shortcut trên màn hình để khởi động trình duyệt Web trong Sandboxie
vietnamese.SandboxieStartMenu1=Chạy chương trình bất kỳ trong sandbox
vietnamese.SandboxieStartMenu2=Chạy trình duyệt web trong Sandbox
vietnamese.SandboxieStartMenu3=Sandboxie Start Menu
vietnamese.SandboxedBrowser=Trình duyệt web trong Sandbox
vietnamese.CustomPageLabel1=Chọn loại cài đặt
vietnamese.CustomPageLabel2=Cài đặt như thế nào
vietnamese.CustomPageLabel3=Chọn chế độ cài đặt
vietnamese.CustomPageInstallMode=Cài đặt {#MyAppName} trên máy tính này
vietnamese.CustomPageUpgradeMode=Cập nhật cài đặt hiện có của {#MyAppName}
vietnamese.CustomPagePortableMode=Giải nén tất cả các tệp vào một thư mục để sử dụng di động
vietnamese.RequiresWin7OrLater=Sandboxie-Plus yêu cầu Windows 7 trở lên.
vietnamese.ClassicFound=Đã phát hiện cài đặt Sandboxie Classic, nó phải được gỡ cài đặt trước, bạn có muốn gỡ cài đặt nó ngay bây giờ không?
vietnamese.RunSandboxedMenu=Chạy trong &Sandbox
vietnamese.UninstallTaskLabel1=Chọn loại gỡ cài đặt
vietnamese.UninstallTaskLabel2=Bạn muốn gỡ cài đặt Sandboxie-Plus như thế nào?
vietnamese.UninstallTaskLabel3=&Giữ các tệp cấu hình và Sandbox%nĐây là tùy chọn được đề xuất nhiều nhất nếu bạn định cài đặt lại Sandboxie-Plus trong khi vẫn giữ các tệp cấu hình và các Sandbox của mình.
vietnamese.UninstallTaskLabel4=Xoá &tập tin cấu hình%nChọn tùy chọn này để xóa các tệp cấu hình Sandboxie.ini và Sandboxie-Plus.ini trong khi vẫn giữ nguyên các Sandbox.
vietnamese.UninstallTaskLabel5=Xóa các tệp cấu hình và các &Sandbox%nChọn tùy chọn này để xóa tất cả các tệp cấu hình và Sandbox, bao gồm cả thư mục Sandbox nằm trong đường dẫn tùy chỉnh với FileRootPath.
vietnamese.RefreshBuild=Tải xuống Templates.ini mới nhất và các bản dịch
