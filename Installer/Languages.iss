﻿; English is default language and can be translated to any
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

; If you are going to provide a new translation in your language,
; please add it after the last one in this section.

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