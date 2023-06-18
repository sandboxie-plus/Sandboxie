<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU">
<context>
    <name>CAdvancedPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="428"/>
        <source>Advanced Sandbox options</source>
        <translation>Расширенные параметры песочницы</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="429"/>
        <source>On this page advanced sandbox options can be configured.</source>
        <translation>На этой странице можно настроить дополнительные параметры песочницы.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="434"/>
        <source>Network Access</source>
        <translation>Доступ к сети</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="442"/>
        <source>Allow network/internet access</source>
        <translation>Разрешить доступ к сети/интернету</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="443"/>
        <source>Block network/internet by denying access to Network devices</source>
        <translation>Блокировать доступ к сети/интернету, запрещая доступ к сетевым устройствам</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="445"/>
        <source>Block network/internet using Windows Filtering Platform</source>
        <translation>Блокировка доступа к сети/интернету с помощью Windows Filtering Platform</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="450"/>
        <source>Allow access to network files and folders</source>
        <translation>Разрешить доступ к сетевым файлам и папкам</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="451"/>
        <location filename="Wizards/NewBoxWizard.cpp" line="467"/>
        <source>This option is not recommended for Hardened boxes</source>
        <translation>Этот параметр не рекомендуется для песочниц с усиленной изоляцией</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="457"/>
        <source>Admin Options</source>
        <translation>Параметры администратора</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="461"/>
        <source>Make applications think they are running elevated</source>
        <translation>Заставить приложения думать, что они работают с повышенными правами</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="466"/>
        <source>Allow MSIServer to run with a sandboxed system token</source>
        <translation>Разрешить запуск MSIServer с изолированным системным токеном</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="472"/>
        <source>Box Options</source>
        <translation>Опции песочницы</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="476"/>
        <source>Use a Sandboxie login instead of an anonymous token</source>
        <translation>Использовать логин Sandboxie вместо анонимного токена</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="482"/>
        <source>Prevent sandboxes programs installed on host from loading dll&apos;s from the sandbox</source>
        <translation>Запретить программам в песочнице, установленным на хосте, загружать dll из песочницы</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="483"/>
        <source>This feature may reduce compatibility as it also prevents box located processes from writing to host located ones and even starting them.</source>
        <translation>Эта функция может снизить совместимость, поскольку она также не позволяет процессам, расположенным в песочнице, записывать данные в процессы, расположенные на хосте, и даже запускать их.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="477"/>
        <source>Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.</source>
        <translation>Использование пользовательского токена Sandboxie позволяет лучше изолировать отдельные песочницы друг от друга, а также показывает в пользовательском столбце диспетчеров задач имя песочницы, к которой принадлежит процесс. Однако у некоторых сторонних решений безопасности могут быть проблемы с пользовательскими токенами.</translation>
    </message>
</context>
<context>
    <name>CBoxTypePage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="168"/>
        <source>Create new Sandbox</source>
        <translation>Создать новую песочницу</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="175"/>
        <source>A sandbox isolates your host system from processes running within the box, it prevents them from making permanent changes to other programs and data in your computer. The level of isolation impacts your security as well as the compatibility with applications, hence there will be a different level of isolation depending on the selected Box Type. Sandboxie can also protect your personal data from being accessed by processes running under its supervision.</source>
        <translation>Песочница изолирует вашу хост-систему от процессов, запущенных в ней, и не позволяет им вносить постоянные изменения в другие программы и данные на вашем компьютере. Уровень изоляции влияет на вашу безопасность, а также на совместимость с приложениями, поэтому от типа выбранной песочницы зависит уровень ее изоляции. Sandboxie также может защитить ваши личные данные от доступа со стороны процессов, запущенных под его контролем.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="184"/>
        <source>Enter box name:</source>
        <translation>Введите имя песочницы:</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="202"/>
        <source>Select box type:</source>
        <translation>Выберите тип песочницы:</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="205"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>Песочница с усиленной изоляцией и защитой данных</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="206"/>
        <source>Security Hardened Sandbox</source>
        <translation>Песочница с усиленной изоляцией</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="207"/>
        <source>Sandbox with Data Protection</source>
        <translation>Песочница с защитой данных</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="208"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>Песочница со стандартной изоляцией (по умолчанию)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="210"/>
        <source>Application Compartment with Data Protection</source>
        <translation>Контейнер для приложений с защитой данных</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="211"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>Контейнер для приложений (БЕЗ изоляции)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="229"/>
        <source>Remove after use</source>
        <translation>Удалить после использования</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="230"/>
        <source>After the last process in the box terminates, all data in the box will be deleted and the box itself will be removed.</source>
        <translation>После завершения последнего процесса в песочнице, эта песочница и все данные в ней будут удалены.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="235"/>
        <source>Configure advanced options</source>
        <translation>Настроить дополнительные параметры</translation>
    </message>
</context>
<context>
    <name>CBrowserOptionsPage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="844"/>
        <source>Create Web Browser Template</source>
        <translation>Создать шаблон веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="845"/>
        <source>Configure web browser template options.</source>
        <translation>Настроить параметры шаблона веб-браузера.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="855"/>
        <source>Force the Web Browser to run in this sandbox</source>
        <translation>Заставить веб-браузер работать в этой песочнице</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="859"/>
        <source>Allow direct access to the entire Web Browser profile folder</source>
        <translation>Разрешить прямой доступ ко всей папке профиля веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="863"/>
        <source>Allow direct access to Web Browser&apos;s phishing database</source>
        <translation>Разрешить прямой доступ к фишинговой базе данных веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="867"/>
        <source>Allow direct access to Web Browser&apos;s session management</source>
        <translation>Разрешить прямой доступ к управлению сеансом веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="871"/>
        <source>Allow direct access to Web Browser&apos;s sync data</source>
        <translation>Разрешить прямой доступ к данным синхронизации веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="875"/>
        <source>Allow direct access to Web Browser&apos;s preferences</source>
        <translation>Разрешить прямой доступ к настройкам веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="879"/>
        <source>Allow direct access to Web Browser&apos;s passwords</source>
        <translation>Разрешить прямой доступ к паролям веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="883"/>
        <source>Allow direct access to Web Browser&apos;s cookies</source>
        <translation>Разрешить прямой доступ к файлам cookie веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="887"/>
        <source>Allow direct access to Web Browser&apos;s bookmarks</source>
        <translation>Разрешить прямой доступ к закладкам веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="891"/>
        <source>Allow direct access to Web Browser&apos;s bookmark and history database</source>
        <translation>Разрешить прямой доступ к базе данных закладок и истории веб-браузера</translation>
    </message>
</context>
<context>
    <name>CBrowserPathsPage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="517"/>
        <source>Create Web Browser Template</source>
        <translation>Создать шаблон веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="518"/>
        <source>Configure your Web Browser&apos;s profile directories.</source>
        <translation>Настроить каталоги профиля вашего веб-браузера.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="526"/>
        <source>User profile(s) directory:</source>
        <translation>Каталог профилей пользователей:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="528"/>
        <source>Show also imperfect matches</source>
        <translation>Показать также неполные совпадения</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="534"/>
        <source>Browser Executable (*.exe)</source>
        <translation>Исполняемый файл браузера (*.exe)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="551"/>
        <source>Continue without browser profile</source>
        <translation>Продолжить без профиля браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="712"/>
        <source>Configure your Gecko based Browsers profile directories.</source>
        <translation>Настроить каталоги профилей браузеров на основе Gecko.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="716"/>
        <source>Configure your Chromium based Browsers profile directories.</source>
        <translation>Настроить каталоги профилей браузеров на основе Chromium.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="796"/>
        <source>No suitable folders have been found.
Note: you need to run the browser unsandboxed for them to get created.
Please browse to the correct user profile directory.</source>
        <translation>Подходящие папки не найдены.
Примечание: вам нужно запустить браузер без песочницы, чтобы они были созданы.
Пожалуйста, перейдите в правильный каталог профиля пользователя.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="801"/>
        <source>Please choose the correct user profile directory, if it is not listed you may need to browse to it.</source>
        <translation>Пожалуйста, выберите правильный каталог профиля пользователя, если его нет в списке, вам может потребоваться перейти к нему.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="807"/>
        <source>Please ensure the selected directory is correct, the wizard is not confident in all the presented options.</source>
        <translation>Пожалуйста, убедитесь, что выбранный каталог правильный, мастер настройки не уверен во всех представленных параметрах.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="810"/>
        <source>Please ensure the selected directory is correct.</source>
        <translation>Убедитесь, что выбран правильный каталог.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="815"/>
        <source>This path does not look like a valid profile directory.</source>
        <translation>Этот путь не похож на допустимый каталог профиля.</translation>
    </message>
</context>
<context>
    <name>CBrowserTypePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="325"/>
        <source>Create Web Browser Template</source>
        <translation>Создать шаблон веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="326"/>
        <source>Select your Web Browsers main executable, this will allow Sandboxie to identify the browser.</source>
        <translation>Выберите основной исполняемый файл веб-браузера, это позволит Sandboxie идентифицировать браузер.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="336"/>
        <source>Enter browser name:</source>
        <translation>Введите имя браузера:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="347"/>
        <source>Main executable (eg. firefox.exe, chrome.exe, msedge.exe, etc...):</source>
        <translation>Основной исполняемый файл (например, firefox.exe, chrome.exe, msedge.exe и т.&#xa0;д.):</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="351"/>
        <source>Browser executable (*.exe)</source>
        <translation>Исполняемый файл браузера (*.exe)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="451"/>
        <source>The browser appears to be Gecko based, like Mozilla Firefox and its derivatives.</source>
        <translation>Браузер, похоже, основан на Gecko, например Mozilla Firefox и его производные.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="454"/>
        <source>The browser appears to be Chromium based, like Microsoft Edge or Google Chrome and its derivatives.</source>
        <translation>Браузер, похоже, основан на Chromium, например Microsoft Edge или Google Chrome и его производных.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="457"/>
        <source>Browser could not be recognized, template cannot be created.</source>
        <translation>Браузер не может быть распознан, шаблон не может быть создан.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="504"/>
        <source>This browser name is already in use, please choose an other one.</source>
        <translation>Это имя браузера уже используется, выберите другое.</translation>
    </message>
</context>
<context>
    <name>CCertificatePage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="195"/>
        <source>Install your &lt;b&gt;Sandboxie-Plus&lt;/b&gt; support certificate</source>
        <translation>Укажите ваш сертификат сторонника &lt;b&gt;Sandboxie-Plus&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="196"/>
        <source>If you have a supporter certificate, please fill it into the field below.</source>
        <translation>Если у вас есть сертификат сторонника, пожалуйста, добавьте его в поле ниже.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="218"/>
        <source>Start evaluation without a certificate for a limited period of time.</source>
        <translation>Начать оценку без сертификата, в течение ограниченного периода времени.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="246"/>
        <source>To use &lt;b&gt;Sandboxie-Plus&lt;/b&gt; in a business setting, an appropriate &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;support certificate&lt;/a&gt; for business use is required. If you do not yet have the required certificate(s), you can get those from the &lt;a href=&quot;https://xanasoft.com/shop/&quot;&gt;xanasoft.com web shop&lt;/a&gt;.</source>
        <translation>Для использования &lt;b&gt;Sandboxie-Plus&lt;/b&gt; в бизнес-среде, необходима бизнес версия &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;сертификата сторонника&lt;/a&gt;. Если у вас еще нет необходимых сертификатов, вы можете получить их в &lt;a href=&quot;https://xanasoft.com/shop/&quot;&gt;интернет-магазине xanasoft.com&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="260"/>
        <source>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; provides additional features and box types exclusively to &lt;u&gt;project supporters&lt;/u&gt;. Boxes like the Privacy Enhanced boxes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;protect user data from illicit access&lt;/font&gt;&lt;/b&gt; by the sandboxed programs. If you are not yet a supporter, then please consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt; to ensure further development of Sandboxie and to receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;.</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; предоставляет дополнительные функции и типы песочниц исключительно &lt;u&gt;сторонникам проекта&lt;/u&gt;. Песочницы с улучшенной конфиденциальностью, &lt;b&gt;&lt;font color=&apos;red&apos;&gt;защищают пользовательские данные от несанкционированного доступа&lt;/font&gt;&lt;/b&gt; изолированных программ. Если вы еще не являетесь сторонником, рассмотрите возможность &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;поддержать проект&lt;/a&gt;, чтобы обеспечить дальнейшее развитие Sandboxie и получить &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;сертификат сторонника&lt;/a&gt;.</translation>
    </message>
</context>
<context>
    <name>CCleanUpJob</name>
    <message>
        <location filename="BoxJob.h" line="36"/>
        <source>Deleting Content</source>
        <translation>Удаление содержимого</translation>
    </message>
</context>
<context>
    <name>CFileBrowserWindow</name>
    <message>
        <location filename="Views/FileView.cpp" line="398"/>
        <source>%1 - Files</source>
        <translation>%1 - Файлы</translation>
    </message>
</context>
<context>
    <name>CFileView</name>
    <message>
        <location filename="Views/FileView.cpp" line="188"/>
        <source>Create Shortcut</source>
        <translation>Создать ярлык</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="210"/>
        <source>Pin to Box Run Menu</source>
        <translation>Закрепить в меню &quot;Запустить&quot; песочницы</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="217"/>
        <source>Recover to Any Folder</source>
        <translation>Восстановить в любую папку</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="219"/>
        <source>Recover to Same Folder</source>
        <translation>Восстановить в ту же папку</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="223"/>
        <source>Run Recovery Checks</source>
        <translation>Выполнить проверки восстановления</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="287"/>
        <source>Select Directory</source>
        <translation>Выбрать каталог</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="353"/>
        <source>Create Shortcut to sandbox %1</source>
        <translation>Создать ярлык для песочницы %1</translation>
    </message>
</context>
<context>
    <name>CFilesPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="302"/>
        <source>Sandbox location and behavior</source>
        <translation>Местоположение и поведение песочницы</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="303"/>
        <source>On this page the sandbox location and its behavior can be customized.
You can use %USER% to save each users sandbox to an own folder.</source>
        <translation>На этой странице можно настроить расположение песочницы и ее поведение.
Вы можете использовать %USER%, чтобы сохранить песочницу каждого пользователя в отдельной папке.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="308"/>
        <source>Sandboxed Files</source>
        <translation>Файлы в песочнице</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="331"/>
        <source>Select Directory</source>
        <translation>Выбрать каталог</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="339"/>
        <source>Virtualization scheme</source>
        <translation>Схема виртуализации</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="343"/>
        <source>Version 1</source>
        <translation>Версия 1</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="344"/>
        <source>Version 2</source>
        <translation>Версия 2</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="350"/>
        <source>Separate user folders</source>
        <translation>Раздельные папки пользователей</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="355"/>
        <source>Use volume serial numbers for drives</source>
        <translation>Использовать серийные номера томов для дисков</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="360"/>
        <source>Auto delete content when last process terminates</source>
        <translation>Автоматическое удаление содержимого при завершении последнего процесса</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="367"/>
        <source>Enable Immediate Recovery of files from recovery locations</source>
        <translation>Включить немедленное восстановление файлов из мест восстановления</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="402"/>
        <source>The selected box location is not a valid path.</source>
        <translation>Выбранное расположение песочницы не является допустимым путем.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="407"/>
        <source>The selected box location exists and is not empty, it is recommended to pick a new or empty folder. Are you sure you want to use an existing folder?</source>
        <translation>Выбранное местоположение песочницы существует и не является пустым, рекомендуется выбрать новую или пустую папку. Вы уверены, что хотите использовать существующую папку?</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="412"/>
        <source>The selected box location is not placed on a currently available drive.</source>
        <translation>Выбранное расположение песочницы не размещено на доступном в данный момент диске.</translation>
    </message>
</context>
<context>
    <name>CFinishPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="489"/>
        <source>Complete your configuration</source>
        <translation>Завершите настройку</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="496"/>
        <source>Almost complete, click Finish to apply all selected options and conclude the wizard.</source>
        <translation>Почти завершено, нажмите &quot;Завершить&quot;, чтобы применить все выбранные параметры и завершить работу мастера.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="508"/>
        <source>Keep Sandboxie-Plus up to date.</source>
        <translation>Поддерживать Sandboxie-Plus в актуальном состоянии.</translation>
    </message>
</context>
<context>
    <name>CFinishTemplatePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="936"/>
        <source>Create Web Browser Template</source>
        <translation>Создать шаблон веб-браузера</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="944"/>
        <source>Almost complete, click Finish to create a new  Web Browser Template and conclude the wizard.</source>
        <translation>Почти завершено, нажмите &quot;Готово&quot;, чтобы создать новый шаблон веб-браузера и завершить работу мастера.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="962"/>
        <source>Browser name: %1
</source>
        <translation>Имя браузера: %1
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="966"/>
        <source>Browser Type: Gecko (Mozilla Firefox)
</source>
        <translation>Тип браузера: Gecko (Mozilla Firefox)
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="967"/>
        <source>Browser Type: Chromium (Google Chrome)
</source>
        <translation>Тип браузера: Chromium (Google Chrome)
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="969"/>
        <source>
</source>
        <translation>
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="970"/>
        <source>Browser executable path: %1
</source>
        <translation>Путь к исполняемому файлу браузера: %1
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="980"/>
        <source>Browser profile path: %1
</source>
        <translation>Путь к профилю браузера: %1
</translation>
    </message>
</context>
<context>
    <name>CIntroPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="117"/>
        <source>Introduction</source>
        <translation>Введение</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="121"/>
        <source>Welcome to the Setup Wizard. This wizard will help you to configure your copy of &lt;b&gt;Sandboxie-Plus&lt;/b&gt;. You can start this wizard at any time from the Sandbox-&gt;Maintenance menu if you do not wish to complete it now.</source>
        <translation>Добро пожаловать в мастер настройки. Этот мастер поможет вам настроить вашу копию &lt;b&gt;Sandboxie-Plus&lt;/b&gt;. Вы можете запустить этот мастер в любое время из меню Песочница-&gt;Обслуживание, если не хотите завершать его сейчас.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="130"/>
        <source>Select how you would like to use Sandboxie-Plus</source>
        <translation>Выберите, как вы хотели бы использовать Sandboxie-Plus</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="133"/>
        <source>&amp;Personally, for private non-commercial use</source>
        <translation>Персональный, для частного некоммерческого использования (&amp;P)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="138"/>
        <source>&amp;Commercially, for business or enterprise use</source>
        <translation>Коммерческий, для бизнеса или корпоративного использования (&amp;C)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="143"/>
        <source>Note: this option is persistent</source>
        <translation>Примечание: этот параметр является постоянным</translation>
    </message>
</context>
<context>
    <name>CMonitorModel</name>
    <message>
        <location filename="Models/MonitorModel.cpp" line="147"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="148"/>
        <source>Status</source>
        <translation>Статус</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="149"/>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="150"/>
        <source>Count</source>
        <translation>Количество</translation>
    </message>
</context>
<context>
    <name>CMultiErrorDialog</name>
    <message>
        <location filename="Dialogs/MultiErrorDialog.cpp" line="10"/>
        <source>Sandboxie-Plus - Error</source>
        <translation>Sandboxie-Plus - Ошибка</translation>
    </message>
    <message>
        <location filename="Dialogs/MultiErrorDialog.cpp" line="19"/>
        <source>Message</source>
        <translation>Сообщение</translation>
    </message>
</context>
<context>
    <name>CNewBoxWindow</name>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="23"/>
        <source>Sandboxie-Plus - Create New Box</source>
        <translation>Sandboxie-Plus - Создать новую песочницу</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="39"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>Песочница с усиленной изоляцией и защитой данных</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="40"/>
        <source>Security Hardened Sandbox</source>
        <translation>Песочница с усиленной изоляцией</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="41"/>
        <source>Sandbox with Data Protection</source>
        <translation>Песочница с защитой данных</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="42"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>Песочница со стандартной изоляцией (по умолчанию)</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="44"/>
        <source>Application Compartment with Data Protection</source>
        <translation>Контейнер для приложений с защитой данных</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="45"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>Контейнер для приложений (БЕЗ изоляции)</translation>
    </message>
</context>
<context>
    <name>CNewBoxWizard</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="30"/>
        <source>New Box Wizard</source>
        <translation>Мастер создания новой песочницы</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="138"/>
        <source>The new sandbox has been created using the new &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;Virtualization Scheme Version 2&lt;/a&gt;, if you experience any unexpected issues with this box, please switch to the Virtualization Scheme to Version 1 and report the issue, the option to change this preset can be found in the Box Options in the Box Structure group.</source>
        <translation>Новая песочница была создана с использованием новой &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;схемы виртуализации версии&#xa0;2&lt;/a&gt;, если у вас возникнут какие-либо непредвиденные проблемы с этой песочницей, переключитесь на схему виртуализации до версии 1 и сообщите о проблеме, возможность изменить этот пресет можно найти в параметрах песочницы в группе &quot;Структура песочницы&quot;.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="141"/>
        <source>Don&apos;t show this message again.</source>
        <translation>Больше не показывать это сообщение.</translation>
    </message>
</context>
<context>
    <name>COnDeleteJob</name>
    <message>
        <location filename="BoxJob.h" line="58"/>
        <source>OnDelete: %1</source>
        <translation>При удалении: %1</translation>
    </message>
</context>
<context>
    <name>COnlineUpdater</name>
    <message>
        <location filename="OnlineUpdater.cpp" line="99"/>
        <source>Do you want to check if there is a new version of Sandboxie-Plus?</source>
        <translation>Вы хотите проверить, есть ли новая версия Sandboxie-Plus?</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="100"/>
        <source>Don&apos;t show this message again.</source>
        <translation>Больше не показывать это сообщение.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="209"/>
        <source>Checking for updates...</source>
        <translation>Проверка обновлений...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="248"/>
        <source>server not reachable</source>
        <translation>сервер недоступен</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="249"/>
        <location filename="OnlineUpdater.cpp" line="251"/>
        <source>Failed to check for updates, error: %1</source>
        <translation>Не удалось проверить обновления, ошибка: %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="404"/>
        <source>&lt;p&gt;Do you want to download the installer?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Вы хотите загрузить установщик?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="406"/>
        <source>&lt;p&gt;Do you want to download the updates?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Вы хотите загрузить обновления?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="408"/>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;update page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Вы хотите перейти на &lt;a href=&quot;%1&quot;&gt;страницу обновления&lt;/a&gt;?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="416"/>
        <source>Don&apos;t show this update anymore.</source>
        <translation>Больше не показывать это обновление.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="534"/>
        <source>Downloading updates...</source>
        <translation>Загрузка обновлений...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="560"/>
        <source>invalid parameter</source>
        <translation>неверный параметр</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="561"/>
        <source>failed to download updated information</source>
        <translation>не удалось загрузить обновленную информацию</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="562"/>
        <source>failed to load updated json file</source>
        <translation>не удалось загрузить обновленный json файл</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="563"/>
        <source>failed to download a particular file</source>
        <translation>не удалось загрузить определенный файл</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="564"/>
        <source>failed to scan existing installation</source>
        <translation>не удалось просканировать существующую установку</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="565"/>
        <source>updated signature is invalid !!!</source>
        <translation>обновленная подпись недействительна !!!</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="566"/>
        <source>downloaded file is corrupted</source>
        <translation>загруженный файл поврежден</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="567"/>
        <source>internal error</source>
        <translation>внутренняя ошибка</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="568"/>
        <source>unknown error</source>
        <translation>неизвестная ошибка</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="592"/>
        <source>Failed to download updates from server, error %1</source>
        <translation>Не удалось загрузить обновления с сервера, ошибка %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="611"/>
        <source>&lt;p&gt;Updates for Sandboxie-Plus have been downloaded.&lt;/p&gt;&lt;p&gt;Do you want to apply these updates? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Обновления для Sandboxie-Plus были загружены.&lt;/p&gt;&lt;p&gt;Применить эти обновления? Если какие-либо программы работают в песочнице, они будут завершены.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="715"/>
        <source>Downloading installer...</source>
        <translation>Загрузка установщика...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="757"/>
        <source>Failed to download installer from: %1</source>
        <translation>Не удалось загрузить установщик из: %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="783"/>
        <source>&lt;p&gt;A new Sandboxie-Plus installer has been downloaded to the following location:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Do you want to begin the installation? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Новый установщик Sandboxie-Plus загружен в следующую папку:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Вы хотите начать установку? Если какие-либо программы работают в песочнице, они будут завершены.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="848"/>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;info page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Вы хотите перейти на &lt;a href=&quot;%1&quot;&gt;страницу с информацией&lt;/a&gt;?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="856"/>
        <source>Don&apos;t show this announcement in the future.</source>
        <translation>Не показывать это объявление в будущем.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="398"/>
        <source>&lt;p&gt;There is a new version of Sandboxie-Plus available.&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;&lt;b&gt;New version:&lt;/b&gt;&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;Доступна новая версия Sandboxie-Plus.&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;&lt;b&gt;Новая версия:&lt;/b&gt;&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="274"/>
        <source>No new updates found, your Sandboxie-Plus is up-to-date.

Note: The update check is often behind the latest GitHub release to ensure that only tested updates are offered.</source>
        <translation>Новых обновлений не найдено, у вас установлена последняя версия Sandboxie-Plus.

Примечание: Найденное обновлениие часто отстает от версии доступной на GitHub, чтобы гарантировать, что предлагаются только проверенные обновления.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="964"/>
        <source>Checking for certificate...</source>
        <translation>Проверка сертификата...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="1004"/>
        <source>No certificate found on server!</source>
        <translation>Сертификат не найден на сервере!</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="1009"/>
        <source>There is no updated certificate available.</source>
        <translation>Обновленный сертификат недоступен.</translation>
    </message>
</context>
<context>
    <name>COptionsWindow</name>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="446"/>
        <location filename="Windows/OptionsWindow.cpp" line="460"/>
        <location filename="Windows/OptionsAccess.cpp" line="24"/>
        <source>Browse for File</source>
        <translation>Выбрать файл</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="25"/>
        <source>Browse for Folder</source>
        <translation>Выбрать папку</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="312"/>
        <source>Closed</source>
        <translation>Закрытый</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="313"/>
        <source>Closed RT</source>
        <translation>Закрытый RT</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="314"/>
        <source>Read Only</source>
        <translation>Только чтение</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="308"/>
        <source>Normal</source>
        <translation>Нормальный</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="309"/>
        <source>Open</source>
        <translation>Открытый</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="310"/>
        <source>Open for All</source>
        <translation>Открытый для всех</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="311"/>
        <source>No Rename</source>
        <translation>Нет переименования</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="315"/>
        <source>Box Only (Write Only)</source>
        <translation>Только песочница (только запись)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="316"/>
        <source>Ignore UIPI</source>
        <translation>Игнорировать UIPI</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="318"/>
        <location filename="Windows/OptionsAccess.cpp" line="335"/>
        <location filename="Windows/OptionsAccess.cpp" line="348"/>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="325"/>
        <source>Regular Sandboxie behavior - allow read and also copy on write.</source>
        <translation>Обычное поведение Sandboxie - разрешить чтение, а также копирование при записи.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="326"/>
        <source>Allow write-access outside the sandbox.</source>
        <translation>Разрешить доступ на запись вне песочницы.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="327"/>
        <source>Allow write-access outside the sandbox, also for applications installed inside the sandbox.</source>
        <translation>Разрешить доступ на запись за пределами песочницы, а также для приложений, установленных внутри песочницы.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="328"/>
        <source>Don&apos;t rename window classes.</source>
        <translation>Не переименовывать классы окон.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="329"/>
        <source>Deny access to host location and prevent creation of sandboxed copies.</source>
        <translation>Запретить доступ к местоположению хоста и предотвратить создание изолированных копий.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="330"/>
        <source>Block access to WinRT class.</source>
        <translation>Заблокировать доступ к классу WinRT.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="331"/>
        <source>Allow read-only access only.</source>
        <translation>Разрешить доступ только для чтения.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="332"/>
        <source>Hide host files, folders or registry keys from sandboxed processes.</source>
        <translation>Скрыть файлы, папки или ключи реестра хоста от изолированных процессов.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="333"/>
        <source>Ignore UIPI restrictions for processes.</source>
        <translation>Игнорировать ограничения UIPI для процессов.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="342"/>
        <source>File/Folder</source>
        <translation>Файл/Папка</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="343"/>
        <source>Registry</source>
        <translation>Реестр</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="344"/>
        <source>IPC Path</source>
        <translation>Путь IPC</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="345"/>
        <source>Wnd Class</source>
        <translation>Wnd класс</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="346"/>
        <source>COM Object</source>
        <translation>COM объект</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="353"/>
        <source>Select File</source>
        <translation>Выбрать файл</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="353"/>
        <source>All Files (*.*)</source>
        <translation>Все файлы (*.*)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="365"/>
        <location filename="Windows/OptionsForce.cpp" line="242"/>
        <location filename="Windows/OptionsForce.cpp" line="252"/>
        <location filename="Windows/OptionsRecovery.cpp" line="128"/>
        <location filename="Windows/OptionsRecovery.cpp" line="139"/>
        <source>Select Directory</source>
        <translation>Выбрать каталог</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="996"/>
        <location filename="Windows/OptionsAccess.cpp" line="399"/>
        <location filename="Windows/OptionsAccess.cpp" line="632"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="554"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="629"/>
        <location filename="Windows/OptionsGeneral.cpp" line="512"/>
        <location filename="Windows/OptionsGeneral.cpp" line="608"/>
        <location filename="Windows/OptionsNetwork.cpp" line="461"/>
        <location filename="Windows/OptionsNetwork.cpp" line="554"/>
        <location filename="Windows/OptionsNetwork.cpp" line="667"/>
        <source>All Programs</source>
        <translation>Все программы</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="994"/>
        <location filename="Windows/OptionsWindow.cpp" line="1012"/>
        <location filename="Windows/OptionsAccess.cpp" line="404"/>
        <location filename="Windows/OptionsAccess.cpp" line="636"/>
        <location filename="Windows/OptionsGeneral.cpp" line="517"/>
        <location filename="Windows/OptionsGeneral.cpp" line="612"/>
        <location filename="Windows/OptionsNetwork.cpp" line="193"/>
        <location filename="Windows/OptionsNetwork.cpp" line="466"/>
        <location filename="Windows/OptionsNetwork.cpp" line="558"/>
        <source>Group: %1</source>
        <translation>Группа: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="537"/>
        <source>COM objects must be specified by their GUID, like: {00000000-0000-0000-0000-000000000000}</source>
        <translation>COM-объекты должны быть указаны по их GUID, например: {00000000-0000-0000-0000-000000000000}</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="543"/>
        <source>RT interfaces must be specified by their name.</source>
        <translation>Интерфейсы RT должны быть указаны по их имени.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="557"/>
        <source>Opening all IPC access also opens COM access, do you still want to restrict COM to the sandbox?</source>
        <translation>Открытие всего доступа к IPC также открывает доступ к COM, вы все еще хотите ограничить COM песочницей?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="558"/>
        <source>Don&apos;t ask in future</source>
        <translation>Не спрашивать в будущем</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="567"/>
        <source>&apos;OpenWinClass=program.exe,#&apos; is not supported, use &apos;NoRenameWinClass=program.exe,*&apos; instead</source>
        <translation>&apos;OpenWinClass=program.exe,#&apos; не поддерживается, используйте &apos;NoRenameWinClass=program.exe,*&apos; вместо него</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="611"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="605"/>
        <location filename="Windows/OptionsGeneral.cpp" line="579"/>
        <location filename="Windows/OptionsGrouping.cpp" line="234"/>
        <location filename="Windows/OptionsGrouping.cpp" line="258"/>
        <location filename="Windows/OptionsNetwork.cpp" line="533"/>
        <source>Template values can not be edited.</source>
        <translation>Значения шаблона нельзя редактировать.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="679"/>
        <source>Template values can not be removed.</source>
        <translation>Значения шаблона удалить нельзя.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="41"/>
        <source>Enable the use of win32 hooks for selected processes. Note: You need to enable win32k syscall hook support globally first.</source>
        <translation>Включить использование win32 hooks для выбранных процессов. Примечание. Сначала необходимо глобально включить поддержку win32k syscall hook.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="42"/>
        <source>Enable crash dump creation in the sandbox folder</source>
        <translation>Включить создание аварийного дампа в папке песочницы</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="43"/>
        <source>Always use ElevateCreateProcess fix, as sometimes applied by the Program Compatibility Assistant.</source>
        <translation>Всегда использовать исправление ElevateCreateProcess, которое иногда применяется помощником по совместимости программ.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="45"/>
        <source>Enable special inconsistent PreferExternalManifest behaviour, as needed for some Edge fixes</source>
        <translation>Включить специальное непоследовательное поведение PreferExternalManifest, необходимое для некоторых исправлений Edge</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="46"/>
        <source>Set RpcMgmtSetComTimeout usage for specific processes</source>
        <translation>Использовать RpcMgmtSetComTimeout для определенных процессов</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="47"/>
        <source>Makes a write open call to a file that won&apos;t be copied fail instead of turning it read-only.</source>
        <translation>Вызывает сбой при открытии записи в файл, который не будет скопирован, вместо того, чтобы сделать его доступным только для чтения.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="50"/>
        <source>Make specified processes think they have admin permissions.</source>
        <translation>Заставить указанные процессы думать, что у них есть права администратора.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="51"/>
        <source>Force specified processes to wait for a debugger to attach.</source>
        <translation>Заставить указанные процессы ждать подключения отладчика.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="53"/>
        <source>Sandbox file system root</source>
        <translation>Корень файловой системы песочницы</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="54"/>
        <source>Sandbox registry root</source>
        <translation>Корень реестра песочницы</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="55"/>
        <source>Sandbox ipc root</source>
        <translation>Корень IPC песочницы</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="575"/>
        <source>Add special option:</source>
        <translation>Добавить специальную опцию:</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="726"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="730"/>
        <source>On Start</source>
        <translation>При запуске</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="727"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="735"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="739"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="743"/>
        <source>Run Command</source>
        <translation>Выполнить комманду</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="731"/>
        <source>Start Service</source>
        <translation>Запустить службу</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="734"/>
        <source>On Init</source>
        <translation>При инициализации</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="738"/>
        <source>On File Recovery</source>
        <translation>При восстановлении файлов</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="742"/>
        <source>On Delete Content</source>
        <translation>При удалении контента</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="753"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="775"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="786"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="797"/>
        <source>Please enter the command line to be executed</source>
        <translation>Пожалуйста, введите командную строку для выполнения</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="835"/>
        <source>Please enter a program file name</source>
        <translation>Пожалуйста, введите имя файла программы</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="917"/>
        <source>Deny</source>
        <translation>Отклонить</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="1022"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="111"/>
        <location filename="Windows/OptionsForce.cpp" line="122"/>
        <source>Process</source>
        <translation>Процесс</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="111"/>
        <location filename="Windows/OptionsForce.cpp" line="122"/>
        <source>Folder</source>
        <translation>Папка</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="222"/>
        <location filename="Windows/OptionsForce.cpp" line="232"/>
        <source>Select Executable File</source>
        <translation>Выбор исполняемого файла</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="222"/>
        <location filename="Windows/OptionsForce.cpp" line="232"/>
        <source>Executable Files (*.exe)</source>
        <translation>Исполняемые файлы (*.exe)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="19"/>
        <source>This option requires a valid supporter certificate</source>
        <translation>Для этой опции требуется действующий сертификат сторонника</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="22"/>
        <source>Supporter exclusive option</source>
        <translation>Эксклюзивная опция для сторонников</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="47"/>
        <source>Don&apos;t alter the window title</source>
        <translation>Не менять заголовок окна</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="48"/>
        <source>Display [#] indicator only</source>
        <translation>Отображать только индикатор [#]</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="49"/>
        <source>Display box name in title</source>
        <translation>Отображать имя песочницы в заголовке</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="51"/>
        <source>Border disabled</source>
        <translation>Рамка отключена</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="52"/>
        <source>Show only when title is in focus</source>
        <translation>Отображать, только когда заголовок в фокусе</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="53"/>
        <source>Always show</source>
        <translation>Всегда отображать</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="56"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>Песочница с усиленной изоляцией и защитой данных</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="57"/>
        <source>Security Hardened Sandbox</source>
        <translation>Песочница с усиленной изоляцией</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="58"/>
        <source>Sandbox with Data Protection</source>
        <translation>Песочница с защитой данных</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="59"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>Песочница со стандартной изоляцией (по умолчанию)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="61"/>
        <source>Application Compartment with Data Protection</source>
        <translation>Контейнер для приложений с защитой данных</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="62"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>Контейнер для приложений (БЕЗ изоляции)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="109"/>
        <source>Custom icon</source>
        <translation>Пользовательская иконка</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="161"/>
        <source>Version 1</source>
        <translation>Версия 1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="162"/>
        <source>Version 2</source>
        <translation>Версия 2</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="198"/>
        <source>Browse for Program</source>
        <translation>Выбрать программу</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="258"/>
        <source>Open Box Options</source>
        <translation>Открыть параметры песочницы</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="259"/>
        <source>Browse Content</source>
        <translation>Просмотр содержимого</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="260"/>
        <source>Start File Recovery</source>
        <translation>Начать восстановление файлов</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="261"/>
        <source>Show Run Dialog</source>
        <translation>Показать диалог запуска</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="294"/>
        <source>Indeterminate</source>
        <translation>Неопределено</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="484"/>
        <location filename="Windows/OptionsGeneral.cpp" line="584"/>
        <source>Always copy</source>
        <translation>Всегда копировать</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="485"/>
        <location filename="Windows/OptionsGeneral.cpp" line="585"/>
        <source>Don&apos;t copy</source>
        <translation>Не копировать</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="486"/>
        <location filename="Windows/OptionsGeneral.cpp" line="586"/>
        <source>Copy empty</source>
        <translation>Копировать пустой</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="712"/>
        <source>kilobytes (%1)</source>
        <translation>килобайт (%1)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="818"/>
        <source>Select color</source>
        <translation>Выбрать цвет</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="893"/>
        <source>Select Program</source>
        <translation>Выбрать программу</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="764"/>
        <source>Please enter a service identifier</source>
        <translation>Пожалуйста, введите идентификатор службы</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="893"/>
        <source>Executables (*.exe *.cmd)</source>
        <translation>Исполняемые файлы (*.exe *.cmd)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="897"/>
        <location filename="Windows/OptionsGeneral.cpp" line="913"/>
        <source>Please enter a menu title</source>
        <translation>Пожалуйста, введите заголовок меню</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="909"/>
        <source>Please enter a command</source>
        <translation>Пожалуйста, введите команду</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGrouping.cpp" line="141"/>
        <source>Please enter a name for the new group</source>
        <translation>Пожалуйста, введите имя для новой группы</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGrouping.cpp" line="229"/>
        <source>Please select group first.</source>
        <translation>Пожалуйста, сначала выберите группу.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="37"/>
        <location filename="Windows/OptionsNetwork.cpp" line="588"/>
        <source>Any</source>
        <translation>Любой</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="38"/>
        <location filename="Windows/OptionsNetwork.cpp" line="589"/>
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="39"/>
        <location filename="Windows/OptionsNetwork.cpp" line="590"/>
        <source>UDP</source>
        <translation>UDP</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="40"/>
        <location filename="Windows/OptionsNetwork.cpp" line="591"/>
        <source>ICMP</source>
        <translation>ICMP</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="52"/>
        <source>Allow access</source>
        <translation>Разрешить доступ</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="53"/>
        <source>Block using Windows Filtering Platform</source>
        <translation>Блокировать с помощью платформы фильтрации Windows</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="54"/>
        <source>Block by denying access to Network devices</source>
        <translation>Блокировать путем запрета доступа к сетевым устройствам</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="917"/>
        <location filename="Windows/OptionsNetwork.cpp" line="171"/>
        <location filename="Windows/OptionsNetwork.cpp" line="574"/>
        <source>Allow</source>
        <translation>Разрешить</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="172"/>
        <source>Block (WFP)</source>
        <translation>Block (WFP)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="173"/>
        <source>Block (NDev)</source>
        <translation>Block (NDev)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="278"/>
        <source>A non empty program name is required.</source>
        <translation>Требуется непустое имя программы.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="575"/>
        <source>Block</source>
        <translation>Блокировать</translation>
    </message>
    <message>
        <location filename="Windows/OptionsRecovery.cpp" line="150"/>
        <source>Please enter a file extension to be excluded</source>
        <translation>Пожалуйста, введите расширение файла, которое нужно исключить</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="56"/>
        <source>All Categories</source>
        <translation>Все категории</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="95"/>
        <source>Custom Templates</source>
        <translation>Пользовательские шаблоны</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="96"/>
        <source>Email Reader</source>
        <translation>Почтовый клиент</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="97"/>
        <source>PDF/Print</source>
        <translation>PDF/Печать</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="98"/>
        <source>Security/Privacy</source>
        <translation>Безопасность/Конфиденциальность</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="99"/>
        <source>Desktop Utilities</source>
        <translation>Настольные утилиты</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="100"/>
        <source>Download Managers</source>
        <translation>Менеджеры загрузок</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="101"/>
        <source>Miscellaneous</source>
        <translation>Разное</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="102"/>
        <source>Web Browser</source>
        <translation>Веб-браузер</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="103"/>
        <source>Media Player</source>
        <translation>Медиа плеер</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="104"/>
        <source>Torrent Client</source>
        <translation>Торрент-клиент</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="151"/>
        <source>This template is enabled globally. To configure it, use the global options.</source>
        <translation>Этот шаблон включен глобально. Для его настройки используйте глобальные параметры.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="187"/>
        <source>Please enter the template identifier</source>
        <translation>Пожалуйста, введите идентификатор шаблона</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="195"/>
        <source>Error: %1</source>
        <translation>Ошибка: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="222"/>
        <source>Do you really want to delete the selected local template(s)?</source>
        <translation>Вы действительно хотите удалить выбранные локальные шаблоны?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="228"/>
        <source>Only local templates can be removed!</source>
        <translation>Только локальные шаблоны могут быть удалены!</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="181"/>
        <source>Sandboxie Plus - &apos;%1&apos; Options</source>
        <translation>Sandboxie Plus - Опции &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="287"/>
        <source>File Options</source>
        <translation>Параметры файла</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="319"/>
        <source>Grouping</source>
        <translation>Группировка</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="525"/>
        <source>Add %1 Template</source>
        <translation>Добавить шаблон %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="625"/>
        <source>Search for options</source>
        <translation>Поиск вариантов</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="755"/>
        <source>Box: %1</source>
        <translation>Песочница: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="756"/>
        <source>Template: %1</source>
        <translation>Шаблон: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="757"/>
        <source>Global: %1</source>
        <translation>Глобально: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="758"/>
        <source>Default: %1</source>
        <translation>По умолчанию: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="935"/>
        <source>This sandbox has been deleted hence configuration can not be saved.</source>
        <translation>Эта песочница была удалена, поэтому сохранить конфигурацию невозможно.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="982"/>
        <source>Some changes haven&apos;t been saved yet, do you really want to close this options window?</source>
        <translation>Некоторые изменения еще не были сохранены, вы действительно хотите закрыть окно параметров?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="1005"/>
        <source>Enter program:</source>
        <translation>Введите программу:</translation>
    </message>
</context>
<context>
    <name>CPopUpMessage</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="49"/>
        <source>?</source>
        <translation>?</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="50"/>
        <source>Visit %1 for a detailed explanation.</source>
        <translation>Посетите %1 для получения подробных сведений.</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="57"/>
        <source>Dismiss</source>
        <translation>Отклонить</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="58"/>
        <source>Remove this message from the list</source>
        <translation>Удалить это сообщение из списка</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="61"/>
        <source>Hide all such messages</source>
        <translation>Скрыть все подобные сообщения</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="75"/>
        <source> (%1)</source>
        <translation> (%1)</translation>
    </message>
</context>
<context>
    <name>CPopUpProgress</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="366"/>
        <source>Dismiss</source>
        <translation>Отклонить</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="367"/>
        <source>Remove this progress indicator from the list</source>
        <translation>Удалить этот индикатор прогресса из списка</translation>
    </message>
</context>
<context>
    <name>CPopUpPrompt</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="108"/>
        <source>Remember for this process</source>
        <translation>Запомнить для этого процесса</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="118"/>
        <source>Yes</source>
        <translation>Да</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="123"/>
        <source>No</source>
        <translation>Нет</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="128"/>
        <source>Terminate</source>
        <translation>Прервать</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="149"/>
        <source>Yes and add to allowed programs</source>
        <translation>Да, и добавить в разрешенные программы</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="184"/>
        <source>Requesting process terminated</source>
        <translation>Процесс запроса прерван</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="189"/>
        <source>Request will time out in %1 sec</source>
        <translation>Срок действия запроса истекает через %1 сек</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="191"/>
        <source>Request timed out</source>
        <translation>Срок действия запроса истек</translation>
    </message>
</context>
<context>
    <name>CPopUpRecovery</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="245"/>
        <source>Recover to:</source>
        <translation>Восстановить в:</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="250"/>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="251"/>
        <source>Clear folder list</source>
        <translation>Очистить список папок</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="259"/>
        <source>Recover</source>
        <translation>Восстановить</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="260"/>
        <source>Recover the file to original location</source>
        <translation>Восстановить файл в исходное место</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="263"/>
        <source>Recover &amp;&amp; Explore</source>
        <translation>Восстановить и показать</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="264"/>
        <source>Recover &amp;&amp; Open/Run</source>
        <translation>Восстановить и открыть/запустить</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="266"/>
        <source>Open file recovery for this box</source>
        <translation>Открыть восстановление файлов для этой песочницы</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="274"/>
        <source>Dismiss</source>
        <translation>Отклонить</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="275"/>
        <source>Don&apos;t recover this file right now</source>
        <translation>Не восстанавливать этот файл прямо сейчас</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="278"/>
        <source>Dismiss all from this box</source>
        <translation>Отклонить все из этой песочницы</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="279"/>
        <source>Disable quick recovery until the box restarts</source>
        <translation>Отключить быстрое восстановление до перезапуска песочницы</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="303"/>
        <source>Select Directory</source>
        <translation>Выбрать каталог</translation>
    </message>
</context>
<context>
    <name>CPopUpWindow</name>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="25"/>
        <source>Sandboxie-Plus Notifications</source>
        <translation>Уведомления Sandboxie-Plus</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="171"/>
        <source>Do you want to allow the print spooler to write outside the sandbox for %1 (%2)?</source>
        <translation>Вы хотите, чтобы диспетчер очереди печати мог писать вне песочницы для %1 (%2)?</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="279"/>
        <source>Do you want to allow %4 (%5) to copy a %1 large file into sandbox: %2?
File name: %3</source>
        <translation>Разрешить %4 (%5) копировать большой файл %1 в песочницу: %2?
Имя файла: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="285"/>
        <source>Do you want to allow %1 (%2) access to the internet?
Full path: %3</source>
        <translation>Вы хотите разрешить %1 (%2) доступ к Интернет?
Полный путь: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="344"/>
        <source>%1 is eligible for quick recovery from %2.
The file was written by: %3</source>
        <translation>%1 может быть быстро восстановлен из %2.
Файл был записан: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="346"/>
        <source>an UNKNOWN process.</source>
        <translation>НЕИЗВЕСТНЫЙ процесс.</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="346"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="430"/>
        <location filename="Windows/PopUpWindow.cpp" line="431"/>
        <source>UNKNOWN</source>
        <translation>НЕИЗВЕСТНО</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="437"/>
        <source>Migrating a large file %1 into the sandbox %2, %3 left.
Full path: %4</source>
        <translation>Перенос большого файла %1 в песочницу %2, осталось %3.
Полный путь: %4</translation>
    </message>
</context>
<context>
    <name>CRecoveryLogWnd</name>
    <message>
        <location filename="SandManRecovery.cpp" line="306"/>
        <source>Sandboxie-Plus - Recovery Log</source>
        <translation>Sandboxie-Plus - Журнал восстановления</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="317"/>
        <source>Time|Box Name|File Path</source>
        <translation>Время|Имя песочницы|Путь к файлу</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="319"/>
        <source>Cleanup Recovery Log</source>
        <translation>Очистить журнал восстановления</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="329"/>
        <source>The following files were recently recovered and moved out of a sandbox.</source>
        <translation>Следующие файлы были недавно восстановлены и перемещены из песочницы.</translation>
    </message>
</context>
<context>
    <name>CRecoveryWindow</name>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="46"/>
        <source>%1 - File Recovery</source>
        <translation>%1 - Восстановление файла</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="70"/>
        <source>File Name</source>
        <translation>Имя файла</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="71"/>
        <source>File Size</source>
        <translation>Размер файла</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="72"/>
        <source>Full Path</source>
        <translation>Полный путь</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="106"/>
        <source>Remember target selection</source>
        <translation>Запомнить выбор</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="112"/>
        <source>Delete everything, including all snapshots</source>
        <translation>Удалить все, включая все снимки</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="138"/>
        <source>Original location</source>
        <translation>Исходное местоположение</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="139"/>
        <source>Browse for location</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="140"/>
        <source>Clear folder list</source>
        <translation>Очистить список папок</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="180"/>
        <location filename="Windows/RecoveryWindow.cpp" line="201"/>
        <location filename="Windows/RecoveryWindow.cpp" line="552"/>
        <source>Select Directory</source>
        <translation>Выбрать каталог</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="248"/>
        <source>Do you really want to delete %1 selected files?</source>
        <translation>Вы действительно хотите удалить %1 выбранных файлов?</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="276"/>
        <source>Close until all programs stop in this box</source>
        <translation>Закрыть, пока все программы в этой песочнице не остановятся</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="277"/>
        <source>Close and Disable Immediate Recovery for this box</source>
        <translation>Закрыть и отключить немедленное восстановление для этой песочницы</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="297"/>
        <source>There are %1 new files available to recover.</source>
        <translation>Доступно %1 новых файлов для восстановления.</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="588"/>
        <source>There are %1 files and %2 folders in the sandbox, occupying %3 of disk space.</source>
        <translation>В песочнице %1 файлов и %2 папок, которые занимают %3 дискового пространства.</translation>
    </message>
</context>
<context>
    <name>CSandBox</name>
    <message>
        <location filename="SandMan.cpp" line="3614"/>
        <source>Waiting for folder: %1</source>
        <translation>Ожидание папки: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3615"/>
        <source>Deleting folder: %1</source>
        <translation>Удаление папки: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3616"/>
        <source>Merging folders: %1 &amp;gt;&amp;gt; %2</source>
        <translation>Слияние папок: %1 и %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3617"/>
        <source>Finishing Snapshot Merge...</source>
        <translation>Завершение слияния снимков...</translation>
    </message>
</context>
<context>
    <name>CSandBoxPlus</name>
    <message>
        <location filename="SbiePlusAPI.cpp" line="652"/>
        <source>Disabled</source>
        <translation>Отключено</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="663"/>
        <source>OPEN Root Access</source>
        <translation>ОТКРЫТЬ Root-доступ</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="665"/>
        <source>Application Compartment</source>
        <translation>Контейнер для приложений</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="667"/>
        <source>NOT SECURE</source>
        <translation>НЕ БЕЗОПАСНЫЙ</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="669"/>
        <source>Reduced Isolation</source>
        <translation>Сниженная изоляция</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="671"/>
        <source>Enhanced Isolation</source>
        <translation>Повышенная изоляция</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="674"/>
        <source>Privacy Enhanced</source>
        <translation>Повышенная конфиденциальность</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="677"/>
        <source>API Log</source>
        <translation>Журнал API</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="680"/>
        <source>No INet (with Exceptions)</source>
        <translation>Нет INet (с исключениями)</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="682"/>
        <source>No INet</source>
        <translation>Без интернета</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="685"/>
        <source>Net Share</source>
        <translation>Общая сеть</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="687"/>
        <source>No Admin</source>
        <translation>Без прав администратора</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="690"/>
        <source>Auto Delete</source>
        <translation>Автоудаление</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="693"/>
        <source>Normal</source>
        <translation>Нормальный</translation>
    </message>
</context>
<context>
    <name>CSandMan</name>
    <message>
        <location filename="SandMan.cpp" line="177"/>
        <location filename="SandMan.cpp" line="1997"/>
        <source>Sandboxie-Plus v%1</source>
        <translation>Sandboxie-Plus v%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3503"/>
        <source>Reset Columns</source>
        <translation>Сбросить столбцы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3504"/>
        <source>Copy Cell</source>
        <translation>Копировать ячейку</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3505"/>
        <source>Copy Row</source>
        <translation>Копировать строку</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3506"/>
        <source>Copy Panel</source>
        <translation>Копировать панель</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1185"/>
        <source>Time|Message</source>
        <translation>Время|Сообщение</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1194"/>
        <source>Sbie Messages</source>
        <translation>Cообщения sbie</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1201"/>
        <source>Trace Log</source>
        <translation>Журнал трассировки</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="45"/>
        <source>Show/Hide</source>
        <translation>Показать/Скрыть</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="566"/>
        <location filename="SandMan.cpp" line="568"/>
        <location filename="SandMan.cpp" line="616"/>
        <location filename="SandMan.cpp" line="618"/>
        <source>Pause Forcing Programs</source>
        <translation>Приостановить принудительные программы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="468"/>
        <location filename="SandMan.cpp" line="684"/>
        <source>&amp;Sandbox</source>
        <translation>Песочница (&amp;S)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="469"/>
        <location filename="SandMan.cpp" line="942"/>
        <location filename="SandMan.cpp" line="943"/>
        <source>Create New Box</source>
        <translation>Создать новую песочницу</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="470"/>
        <source>Create Box Group</source>
        <translation>Создать группу песочниц</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="474"/>
        <location filename="SandMan.cpp" line="615"/>
        <source>Terminate All Processes</source>
        <translation>Завершить все процессы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="410"/>
        <source>&amp;Maintenance</source>
        <translation>Обслуживание (&amp;M)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="411"/>
        <source>Connect</source>
        <translation>Подключить</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="412"/>
        <source>Disconnect</source>
        <translation>Отключить</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="414"/>
        <source>Stop All</source>
        <translation>Остановить все</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="415"/>
        <source>&amp;Advanced</source>
        <translation>Расширенный (&amp;A)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="416"/>
        <source>Install Driver</source>
        <translation>Установить драйвер</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="417"/>
        <source>Start Driver</source>
        <translation>Запустить драйвер</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="418"/>
        <source>Stop Driver</source>
        <translation>Остановить драйвер</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="419"/>
        <source>Uninstall Driver</source>
        <translation>Удалить драйвер</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="421"/>
        <source>Install Service</source>
        <translation>Установить службу</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="422"/>
        <source>Start Service</source>
        <translation>Запустить службу</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="423"/>
        <source>Stop Service</source>
        <translation>Остановить службу</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="424"/>
        <source>Uninstall Service</source>
        <translation>Удалить службу</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="429"/>
        <source>Uninstall All</source>
        <translation>Удалить все</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="500"/>
        <location filename="SandMan.cpp" line="654"/>
        <source>Exit</source>
        <translation>Выход</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="503"/>
        <location filename="SandMan.cpp" line="656"/>
        <source>&amp;View</source>
        <translation>Вид (&amp;V)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="435"/>
        <source>Simple View</source>
        <translation>Простой вид</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="436"/>
        <source>Advanced View</source>
        <translation>Расширенный вид</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="441"/>
        <source>Always on Top</source>
        <translation>Поверх всех окон</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="510"/>
        <source>Show Hidden Boxes</source>
        <translation>Показать скрытые песочницы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="512"/>
        <source>Show All Sessions</source>
        <translation>Показать все сеансы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="535"/>
        <source>Clean Up</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="536"/>
        <source>Cleanup Processes</source>
        <translation>Очистить процессы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="538"/>
        <source>Cleanup Message Log</source>
        <translation>Очистить журнал сообщений</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="539"/>
        <source>Cleanup Trace Log</source>
        <translation>Очистить журнал трассировки</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="542"/>
        <source>Keep terminated</source>
        <translation>Держать завершенным</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="562"/>
        <source>&amp;Options</source>
        <translation>Опции (&amp;O)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="563"/>
        <location filename="SandMan.cpp" line="705"/>
        <source>Global Settings</source>
        <translation>Глобальные настройки</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="597"/>
        <location filename="SandMan.cpp" line="715"/>
        <source>Reset all hidden messages</source>
        <translation>Сбросить все скрытые сообщения</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="555"/>
        <source>Trace Logging</source>
        <translation>Ведение журнала трассировки</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="447"/>
        <source>&amp;Help</source>
        <translation>Помощь (&amp;H)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="456"/>
        <source>Visit Support Forum</source>
        <translation>Посетить форум поддержки</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="455"/>
        <source>Online Documentation</source>
        <translation>Онлайн-документация</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="458"/>
        <source>Check for Updates</source>
        <translation>Проверить обновления</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="460"/>
        <source>About the Qt Framework</source>
        <translation>О Qt Framework</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="461"/>
        <location filename="SandMan.cpp" line="3588"/>
        <source>About Sandboxie-Plus</source>
        <translation>О Sandboxie-Plus</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="598"/>
        <location filename="SandMan.cpp" line="716"/>
        <source>Reset all GUI options</source>
        <translation>Сбросить все настройки GUI</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="689"/>
        <source>Create New Sandbox</source>
        <translation>Создать новую песочницу</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="690"/>
        <source>Create New Group</source>
        <translation>Создать новую группу</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="792"/>
        <location filename="SandMan.cpp" line="930"/>
        <location filename="SandMan.cpp" line="931"/>
        <source>Cleanup</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1015"/>
        <source>Click to run installer</source>
        <translation>Нажмите, чтобы запустить установщик</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1019"/>
        <source>Click to apply update</source>
        <translation>Нажмите, чтобы применить обновление</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1031"/>
        <source>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Support Sandboxie-Plus on Patreon&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Поддержите Sandboxie-Plus на Patreon&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1265"/>
        <source>Do you want to close Sandboxie Manager?</source>
        <translation>Вы хотите закрыть Sandboxie Manager?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1282"/>
        <source>Sandboxie-Plus was running in portable mode, now it has to clean up the created services. This will prompt for administrative privileges.

Do you want to do the clean up?</source>
        <translation>Sandboxie-Plus работала в портативном режиме, теперь ей нужно очистить созданные службы. Это потребует административных привилегий.

Вы хотите сделать уборку?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1283"/>
        <location filename="SandMan.cpp" line="1696"/>
        <location filename="SandMan.cpp" line="2027"/>
        <location filename="SandMan.cpp" line="2635"/>
        <location filename="SandMan.cpp" line="3063"/>
        <location filename="SandMan.cpp" line="3079"/>
        <source>Don&apos;t show this message again.</source>
        <translation>Больше не показывать это сообщение.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1452"/>
        <source>This box provides &lt;a href=&quot;sbie://docs/security-mode&quot;&gt;enhanced security isolation&lt;/a&gt;, it is suitable to test untrusted software.</source>
        <translation>Эта песочница обеспечивает &lt;a href=&quot;sbie://docs/security-mode&quot;&gt;улучшенную изоляцию безопасности&lt;/a&gt; и подходит для тестирования ненадежного программного обеспечения.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1456"/>
        <source>This box provides standard isolation, it is suitable to run your software to enhance security.</source>
        <translation>Эта песочница обеспечивает стандартную изоляцию, она подходит для запуска вашего программного обеспечения для повышения безопасности.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1460"/>
        <source>This box does not enforce isolation, it is intended to be used as an &lt;a href=&quot;sbie://docs/compartment-mode&quot;&gt;application compartment&lt;/a&gt; for software virtualization only.</source>
        <translation>Эта песочница не обеспечивает изоляцию, она предназначена для использования в качестве &lt;a href=&quot;sbie://docs/compartment-mode&quot;&gt;контейнера приложений&lt;/a&gt; только для программной виртуализации.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1465"/>
        <source>&lt;br /&gt;&lt;br /&gt;This box &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;prevents access to all user data&lt;/a&gt; locations, except explicitly granted in the Resource Access options.</source>
        <translation>&lt;br /&gt;&lt;br /&gt;Эта песочница &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;запрещает доступ ко всем данным пользователя&lt;/a&gt;, кроме тех, которые явно разрешены в параметрах доступа к ресурсам.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1553"/>
        <source>Unknown operation &apos;%1&apos; requested via command line</source>
        <translation>Неизвестная операция &apos;%1&apos; запрошена из командной строки</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2771"/>
        <location filename="SandMan.cpp" line="2773"/>
        <location filename="SandMan.cpp" line="3322"/>
        <source>Sandboxie-Plus - Error</source>
        <translation>Sandboxie-Plus - Ошибка</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2771"/>
        <source>Failed to stop all Sandboxie components</source>
        <translation>Не удалось остановить все компоненты Sandboxie</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2773"/>
        <source>Failed to start required Sandboxie components</source>
        <translation>Не удалось запустить необходимые компоненты Sandboxie</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1694"/>
        <source>Some compatibility templates (%1) are missing, probably deleted, do you want to remove them from all boxes?</source>
        <translation>Некоторые шаблоны совместимости (%1) отсутствуют, вероятно, удалены. Удалить их из всех песочниц?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1711"/>
        <source>Cleaned up removed templates...</source>
        <translation>Убраны удаленные шаблоны...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2024"/>
        <source>Sandboxie-Plus was started in portable mode, do you want to put the Sandbox folder into its parent directory?
Yes will choose: %1
No will choose: %2</source>
        <translation>Sandboxie-Plus был запущен в портативном режиме, вы хотите поместить папку Sandbox в ее родительский каталог?
Да, выберет: %1
Нет, выберет: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3277"/>
        <source>A sandbox must be emptied before it can be deleted.</source>
        <translation>Песочницу необходимо очистить, прежде чем ее можно будет удалить.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2481"/>
        <source>The supporter certificate is not valid for this build, please get an updated certificate</source>
        <translation>Сертификат сторонника недействителен для этой сборки, получите обновленный сертификат</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2484"/>
        <source>The supporter certificate has expired%1, please get an updated certificate</source>
        <translation>Срок действия сертификата сторонника истек%1, пожалуйста получите обновленный сертификат</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2485"/>
        <source>, but it remains valid for the current build</source>
        <translation>, но остается действительным для текущей сборки</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2487"/>
        <source>The supporter certificate will expire in %1 days, please get an updated certificate</source>
        <translation>Срок действия сертификата сторонника истекает через %1 дн., получите обновленный сертификат</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2100"/>
        <source>Default sandbox not found; creating: %1</source>
        <translation>Песочница по умолчанию не найдена; создание: %1</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="105"/>
        <source>Dismiss Update Notification</source>
        <translation>Отклонить уведомление об обновлении</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="181"/>
        <source> - Driver/Service NOT Running!</source>
        <translation> - Драйвер/служба НЕ работает!</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="183"/>
        <source> - Deleting Sandbox Content</source>
        <translation> - Удаление содержимого песочницы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1784"/>
        <source>Auto Deleting %1 Content</source>
        <translation>Автоудаление содержимого %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2140"/>
        <source>   -   NOT connected</source>
        <translation>   -   НЕ подключено</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2376"/>
        <source>PID %1: </source>
        <translation>ID процесса %1: </translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2378"/>
        <source>%1 (%2): </source>
        <translation>%1 (%2): </translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2407"/>
        <source>The selected feature set is only available to project supporters. Processes started in a box with this feature set enabled without a supporter certificate will be terminated after 5 minutes.&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Become a project supporter&lt;/a&gt;, and receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;</source>
        <translation>Выбранный набор функций доступен только сторонникам проекта. Процессы, запущенные в песочнице с этим набором функций без сертификата сторонника, будут прекращены через 5 минут.&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Станьте сторонником проекта&lt;/a&gt;, и получите &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;сертификат сторонника&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="202"/>
        <source>Recovering file %1 to %2</source>
        <translation>Восстановление файла %1 в %2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="214"/>
        <source>The file %1 already exists, do you want to overwrite it?</source>
        <translation>Файл %1 уже существует. Вы хотите его перезаписать?</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="179"/>
        <location filename="SandManRecovery.cpp" line="215"/>
        <source>Do this for all files!</source>
        <translation>Сделать это для всех файлов!</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="99"/>
        <location filename="SandManRecovery.cpp" line="159"/>
        <source>Checking file %1</source>
        <translation>Проверка файла %1</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="107"/>
        <source>The file %1 failed a security check!

%2</source>
        <translation>Файл %1 не прошел проверку безопасности!

%2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="115"/>
        <source>All files passed the checks</source>
        <translation>Все файлы прошли проверку</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="178"/>
        <source>The file %1 failed a security check, do you want to recover it anyway?

%2</source>
        <translation>Файл %1 не прошел проверку безопасности, вы все равно хотите его восстановить?

%2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="243"/>
        <source>Failed to recover some files: 
</source>
        <translation>Не удалось восстановить некоторые файлы: 
</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2528"/>
        <source>Only Administrators can change the config.</source>
        <translation>Только администраторы могут изменять конфигурацию.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2538"/>
        <source>Please enter the configuration password.</source>
        <translation>Пожалуйста, введите пароль конфигурации.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2546"/>
        <source>Login Failed: %1</source>
        <translation>Ошибка входа: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2575"/>
        <source>Do you want to terminate all processes in all sandboxes?</source>
        <translation>Вы хотите завершить все процессы во всех песочницах?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2576"/>
        <source>Terminate all without asking</source>
        <translation>Завершить все, не спрашивая</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2634"/>
        <source>Sandboxie-Plus was started in portable mode and it needs to create necessary services. This will prompt for administrative privileges.</source>
        <translation>Sandboxie-Plus запущен в портативном режиме, и ему нужно создать необходимые службы. Это потребует административных привилегий.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2668"/>
        <source>CAUTION: Another agent (probably SbieCtrl.exe) is already managing this Sandboxie session, please close it first and reconnect to take over.</source>
        <translation>ВНИМАНИЕ: другой агент (вероятно, SbieCtrl.exe) уже управляет этим сеансом Sandboxie, пожалуйста, сначала закройте его и подключитесь повторно, чтобы взять на себя управление.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2800"/>
        <source>Executing maintenance operation, please wait...</source>
        <translation>Выполняется операция обслуживания, подождите...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2980"/>
        <source>Do you also want to reset hidden message boxes (yes), or only all log messages (no)?</source>
        <translation>Вы также хотите сбросить скрытые окна сообщений (да) или только все сообщения журнала (нет)?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3077"/>
        <source>The changes will be applied automatically whenever the file gets saved.</source>
        <translation>Изменения будут применяться автоматически при сохранении файла.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3078"/>
        <source>The changes will be applied automatically as soon as the editor is closed.</source>
        <translation>Изменения вступят в силу автоматически после закрытия редактора.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3257"/>
        <source>Error Status: 0x%1 (%2)</source>
        <translation>Состояние ошибки: 0x%1 (%2)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3258"/>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3281"/>
        <source>Failed to copy box data files</source>
        <translation>Не удалось скопировать файлы данных песочницы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3286"/>
        <source>Failed to remove old box data files</source>
        <translation>Не удалось удалить старые файлы данных песочницы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3300"/>
        <source>Unknown Error Status: 0x%1</source>
        <translation>Неизвестный статус ошибки: 0x%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3507"/>
        <source>Case Sensitive</source>
        <translation>Чувствительный к регистру</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3508"/>
        <source>RegExp</source>
        <translation>Регулярное выражение</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3509"/>
        <source>Highlight</source>
        <translation>Подсветить</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3510"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3511"/>
        <source>&amp;Find ...</source>
        <translation>Найти (&amp;) ...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3512"/>
        <source>All columns</source>
        <translation>Все столбцы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3561"/>
        <source>&lt;h3&gt;About Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;Version %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2023 by DavidXanatos&lt;/p&gt;</source>
        <translation>&lt;h3&gt;О Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;Версия %1&lt;/p&gt;&lt;p&gt; Авторское право (c) 2020-2023 DavidXanatos&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3569"/>
        <source>This copy of Sandboxie+ is certified for: %1</source>
        <translation>Эта копия Sandboxie+ сертифицирована для: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3571"/>
        <source>Sandboxie+ is free for personal and non-commercial use.</source>
        <translation>Sandboxie+ бесплатен для личного и некоммерческого использования.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3574"/>
        <source>Sandboxie-Plus is an open source continuation of Sandboxie.&lt;br /&gt;Visit &lt;a href=&quot;https://sandboxie-plus.com&quot;&gt;sandboxie-plus.com&lt;/a&gt; for more information.&lt;br /&gt;&lt;br /&gt;%3&lt;br /&gt;&lt;br /&gt;Driver version: %1&lt;br /&gt;Features: %2&lt;br /&gt;&lt;br /&gt;Icons from &lt;a href=&quot;https://icons8.com&quot;&gt;icons8.com&lt;/a&gt;</source>
        <translation>Sandboxie-Plus - это продолжение Sandboxie с открытым исходным кодом.&lt;br /&gt;Посетите &lt;a href=&quot;https://sandboxie-plus.com&quot;&gt;sandboxie-plus.com&lt;/a&gt; для получения дополнительной информации.&lt;br /&gt;&lt;br /&gt;%3&lt;br /&gt;&lt;br /&gt;Версия драйвера: %1&lt;br /&gt;Возможности: %2&lt;br /&gt;&lt;br /&gt;Иконки взяты с &lt;a href=&quot;https://icons8.com&quot;&gt;icons8.com&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3260"/>
        <source>Administrator rights are required for this operation.</source>
        <translation>Для этой операции требуются права администратора.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="165"/>
        <source>WARNING: Sandboxie-Plus.ini in %1 cannot be written to, settings will not be saved.</source>
        <translation>ВНИМАНИЕ: Sandboxie-Plus.ini в %1 не может быть записан, настройки не будут сохранены.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="572"/>
        <source>Disable File Recovery</source>
        <translation>Отключить восстановление файлов</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="575"/>
        <source>Disable Message Popup</source>
        <translation>Отключить всплывающее окно сообщения</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="426"/>
        <source>Setup Wizard</source>
        <translation>Мастер настройки</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="437"/>
        <source>Vintage View (like SbieCtrl)</source>
        <translation>Винтажный вид (как в SbieCtrl)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="476"/>
        <location filename="SandMan.cpp" line="628"/>
        <source>Is Window Sandboxed?</source>
        <translation>Находится ли окно в песочнице?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="529"/>
        <source>Refresh View</source>
        <translation>Обновить вид</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="540"/>
        <source>Cleanup Recovery Log</source>
        <translation>Очистить журнал восстановления</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="613"/>
        <source>&amp;File</source>
        <translation>Файл (&amp;F)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="629"/>
        <source>Resource Access Monitor</source>
        <translation>Монитор доступа к ресурсам</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="693"/>
        <source>Set Container Folder</source>
        <translation>Установить папку контейнера</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="696"/>
        <source>Set Layout and Groups</source>
        <translation>Установить макет и группы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="698"/>
        <source>Reveal Hidden Boxes</source>
        <translation>Отобразить скрытые песочницы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="704"/>
        <source>&amp;Configure</source>
        <translation>Настроить (&amp;C)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="708"/>
        <source>Program Alerts</source>
        <translation>Оповещения программы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="710"/>
        <source>Windows Shell Integration</source>
        <translation>Интеграция с оболочкой Windows</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="712"/>
        <source>Software Compatibility</source>
        <translation>Программная совместимость</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="721"/>
        <source>Lock Configuration</source>
        <translation>Заблокировать конфигурацию</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1014"/>
        <source>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus release %1 ready&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;Готова новая версия Sandboxie-Plus %1&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1018"/>
        <source>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update %1 ready&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;Готово новое обновление Sandboxie-Plus %1&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1022"/>
        <source>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update v%1 available&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;Доступно новое обновление Sandboxie-Plus v%1&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1032"/>
        <source>Click to open web browser</source>
        <translation>Нажмите, чтобы открыть веб-браузер</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1211"/>
        <source>Time|Box Name|File Path</source>
        <translation>Время|Имя песочницы|Путь к файлу</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="559"/>
        <location filename="SandMan.cpp" line="667"/>
        <location filename="SandMan.cpp" line="1221"/>
        <source>Recovery Log</source>
        <translation>Журнал восстановления</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1763"/>
        <source>Executing OnBoxDelete: %1</source>
        <translation>Выполнение OnBoxDelete: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1969"/>
        <source>Auto deleting content of %1</source>
        <translation>Автоудаление содержимого %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2004"/>
        <source>Sandboxie-Plus Version: %1 (%2)</source>
        <translation>Sandboxie-Plus версия: %1 (%2)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2005"/>
        <source>Current Config: %1</source>
        <translation>Текущая конфигурация: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2006"/>
        <source>Data Directory: %1</source>
        <translation>Каталог данных: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2338"/>
        <source>The program %1 started in box %2 will be terminated in 5 minutes because the box was configured to use features exclusively available to project supporters.</source>
        <translation>Программа %1, запущенная в песочнице %2, будет завершена через 5 минут, поскольку песочница была настроена на использование функций, доступных исключительно для сторонников проекта.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2340"/>
        <source>The box %1 is configured to use features exclusively available to project supporters, these presets will be ignored.</source>
        <translation>Песочница %1 настроена на использование функций, доступных исключительно для сторонников проекта, эти предустановки будут игнорироваться.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2341"/>
        <source>&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Become a project supporter&lt;/a&gt;, and receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;</source>
        <translation>&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Стань сторонником проекта&lt;/a&gt;, и получи &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;сертификат сторонника&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2595"/>
        <source>Please enter the duration, in seconds, for disabling Forced Programs rules.</source>
        <translation>Введите продолжительность в секундах, для отключения правил принудительных программ.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="521"/>
        <source>Show File Panel</source>
        <translation>Показать панель файлов</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="454"/>
        <source>Contribute to Sandboxie-Plus</source>
        <translation>Внесите вклад в Sandboxie-Plus</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="471"/>
        <source>Import Box</source>
        <translation>Импорт контейнера</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="473"/>
        <location filename="SandMan.cpp" line="614"/>
        <source>Run Sandboxed</source>
        <translation>Запуск в песочнице</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="585"/>
        <location filename="SandMan.cpp" line="723"/>
        <location filename="SandMan.cpp" line="958"/>
        <location filename="SandMan.cpp" line="959"/>
        <source>Edit Sandboxie.ini</source>
        <translation>Редактировать Sandboxie.ini</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="588"/>
        <source>Edit Templates.ini</source>
        <translation>Редактировать Templates.ini</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="590"/>
        <source>Edit Sandboxie-Plus.ini</source>
        <translation>Редактировать Sandboxie-Plus.ini</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="594"/>
        <location filename="SandMan.cpp" line="726"/>
        <source>Reload configuration</source>
        <translation>Перезагрузить конфигурацию</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="662"/>
        <source>Programs</source>
        <translation>Программы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="663"/>
        <source>Files and Folders</source>
        <translation>Файлы и папки</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="691"/>
        <source>Import Sandbox</source>
        <translation>Импорт песочницы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="740"/>
        <source>Sandbox %1</source>
        <translation>Песочница %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="783"/>
        <source>New-Box Menu</source>
        <translation>Меню новой песочницы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="800"/>
        <source>Edit-ini Menu</source>
        <translation>Меню редактирования ini</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="844"/>
        <source>Toolbar Items</source>
        <translation>Элементы панели инструментов</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="846"/>
        <source>Reset Toolbar</source>
        <translation>Сбросить панель инструментов</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1023"/>
        <source>Click to download update</source>
        <translation>Нажмите, чтобы скачать обновление</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1639"/>
        <source>No Force Process</source>
        <translation>Не принудительный процесс</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1856"/>
        <source>Removed Shortcut: %1</source>
        <translation>Удаленный ярлык: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1935"/>
        <source>Updated Shortcut to: %1</source>
        <translation>Ярлык обновлен: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1937"/>
        <source>Added Shortcut to: %1</source>
        <translation>Добавлен ярлык для: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1988"/>
        <source>Auto removing sandbox %1</source>
        <translation>Автоудаление песочницы %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>%1 Directory: %2</source>
        <translation>%1 Каталог: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>Application</source>
        <translation>Приложение</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>Installation</source>
        <translation>Установка</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2069"/>
        <source> for Personal use</source>
        <translation> для личного использования</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2071"/>
        <source>   -   for Non-Commercial use ONLY</source>
        <translation>   -   ТОЛЬКО для некоммерческого использования</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2285"/>
        <location filename="SandMan.cpp" line="2287"/>
        <source> (%1)</source>
        <translation> (%1)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2475"/>
        <source>The evaluation period has expired!!!</source>
        <translation>Период оценки истек!!!</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2610"/>
        <source>No Recovery</source>
        <translation>Нет файлов для восстановления</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2616"/>
        <source>No Messages</source>
        <translation>Нет сообщений</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2672"/>
        <source>&lt;b&gt;ERROR:&lt;/b&gt; The Sandboxie-Plus Manager (SandMan.exe) does not have a valid signature (SandMan.exe.sig). Please download a trusted release from the &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get&quot;&gt;official Download page&lt;/a&gt;.</source>
        <translation>&lt;b&gt;ОШИБКА:&lt;/b&gt; Sandboxie-Plus Manager (SandMan.exe) не имеет действительной подписи (SandMan.exe.sig). Загрузите надежную версию с &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get&quot;&gt;официальной страницы загрузки&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2775"/>
        <source>Maintenance operation failed (%1)</source>
        <translation>Операция обслуживания не удалась (%1)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2780"/>
        <source>Maintenance operation completed</source>
        <translation>Операция технического обслуживания завершена</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2911"/>
        <source>In the Plus UI, this functionality has been integrated into the main sandbox list view.</source>
        <translation>В интерфейсе Plus, эта функция была интегрирована в основное представление списка песочницы.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2912"/>
        <source>Using the box/group context menu, you can move boxes and groups to other groups. You can also use drag and drop to move the items around. Alternatively, you can also use the arrow keys while holding ALT down to move items up and down within their group.&lt;br /&gt;You can create new boxes and groups from the Sandbox menu.</source>
        <translation>Используя контекстное меню песочницы/группы, вы можете перемещать песочницы и группы в другие группы. Вы также можете использовать перетаскивание для перемещения элементов. В качестве альтернативы вы также можете использовать клавиши со стрелками, удерживая нажатой клавишу ALT, чтобы перемещать элементы вверх и вниз в пределах группы.&lt;br /&gt;Вы можете создавать новые песочницы и группы из меню &quot;Песочница&quot;.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3061"/>
        <source>You are about to edit the Templates.ini, this is generally not recommended.
This file is part of Sandboxie and all change done to it will be reverted next time Sandboxie is updated.</source>
        <translation>Вы собираетесь редактировать Templates.ini, обычно это не рекомендуется.
Этот файл является частью Sandboxie, и все внесенные в него изменения будут отменены при следующем обновлении Sandboxie.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3136"/>
        <source>Sandboxie config has been reloaded</source>
        <translation>Конфигурация Sandboxie перезагружена</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3261"/>
        <source>Failed to execute: %1</source>
        <translation>Не удалось выполнить: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3262"/>
        <source>Failed to connect to the driver</source>
        <translation>Не удалось подключиться к драйверу</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3263"/>
        <source>Failed to communicate with Sandboxie Service: %1</source>
        <translation>Не удалось связаться со службой Sandboxie: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3264"/>
        <source>An incompatible Sandboxie %1 was found. Compatible versions: %2</source>
        <translation>Обнаружена несовместимая песочница %1. Совместимые версии: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3265"/>
        <source>Can&apos;t find Sandboxie installation path.</source>
        <translation>Не удается найти путь установки Sandboxie.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3266"/>
        <source>Failed to copy configuration from sandbox %1: %2</source>
        <translation>Не удалось скопировать конфигурацию из песочницы %1: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3267"/>
        <source>A sandbox of the name %1 already exists</source>
        <translation>Песочница с именем %1 уже существует</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3268"/>
        <source>Failed to delete sandbox %1: %2</source>
        <translation>Не удалось удалить песочницу %1: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3269"/>
        <source>The sandbox name can not be longer than 32 characters.</source>
        <translation>Имя песочницы не может быть длиннее 32 символов.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3270"/>
        <source>The sandbox name can not be a device name.</source>
        <translation>Имя песочницы не может быть именем устройства.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3271"/>
        <source>The sandbox name can contain only letters, digits and underscores which are displayed as spaces.</source>
        <translation>Имя песочницы может содержать только буквы, цифры и символы подчеркивания, которые отображаются как пробелы.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3272"/>
        <source>Failed to terminate all processes</source>
        <translation>Не удалось завершить все процессы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3273"/>
        <source>Delete protection is enabled for the sandbox</source>
        <translation>Для этой песочницы включена защита от удаления</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3274"/>
        <source>All sandbox processes must be stopped before the box content can be deleted</source>
        <translation>Все процессы песочницы должны быть остановлены, перед удалением ее содержимого</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3275"/>
        <source>Error deleting sandbox folder: %1</source>
        <translation>Ошибка при удалении папки песочницы: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3278"/>
        <source>Failed to move directory &apos;%1&apos; to &apos;%2&apos;</source>
        <translation>Не удалось переместить каталог &apos;%1&apos; в &apos;%2&apos;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3279"/>
        <source>This Snapshot operation can not be performed while processes are still running in the box.</source>
        <translation>Операция снимка не может быть выполнена, пока в песочнице еще выполняются процессы.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3280"/>
        <source>Failed to create directory for new snapshot</source>
        <translation>Не удалось создать каталог для нового снимка</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3282"/>
        <source>Snapshot not found</source>
        <translation>Снимок не найден</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3283"/>
        <source>Error merging snapshot directories &apos;%1&apos; with &apos;%2&apos;, the snapshot has not been fully merged.</source>
        <translation>Ошибка при объединении каталогов снимков &apos;%1&apos; с &apos;%2&apos;, снимок не был объединен полностью.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3284"/>
        <source>Failed to remove old snapshot directory &apos;%1&apos;</source>
        <translation>Не удалось удалить старый каталог снимков &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3285"/>
        <source>Can&apos;t remove a snapshot that is shared by multiple later snapshots</source>
        <translation>Невозможно удалить снимок, который используется несколькими более поздними снимками</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3287"/>
        <source>You are not authorized to update configuration in section &apos;%1&apos;</source>
        <translation>У вас нет прав для обновления конфигурации в разделе &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3288"/>
        <source>Failed to set configuration setting %1 in section %2: %3</source>
        <translation>Не удалось установить параметр конфигурации %1 в секции %2: %3</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3289"/>
        <source>Can not create snapshot of an empty sandbox</source>
        <translation>Невозможно создать снимок пустой песочницы</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3290"/>
        <source>A sandbox with that name already exists</source>
        <translation>Песочница с таким именем уже существует</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3291"/>
        <source>The config password must not be longer than 64 characters</source>
        <translation>Пароль конфигурации не должен быть длиннее 64 символов</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3292"/>
        <source>The operation was canceled by the user</source>
        <translation>Операция отменена пользователем</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3294"/>
        <source>Import/Export not available, 7z.dll could not be loaded</source>
        <translation>Импорт/экспорт недоступен, не удалось загрузить 7z.dll</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3295"/>
        <source>Failed to create the box archive</source>
        <translation>Не удалось создать архив контейнера</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3296"/>
        <source>Failed to open the 7z archive</source>
        <translation>Не удалось открыть 7z архив</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3297"/>
        <source>Failed to unpack the box archive</source>
        <translation>Не удалось распаковать архив контейнера</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3298"/>
        <source>The selected 7z file is NOT a box archive</source>
        <translation>Выбранный 7z файл НЕ является архивом контейнера</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3324"/>
        <source>Operation failed for %1 item(s).</source>
        <translation>Операция не удалась для %1 элемента(ов).</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3355"/>
        <source>Do you want to open %1 in a sandboxed (yes) or unsandboxed (no) Web browser?</source>
        <translation>Вы хотите открыть %1 в изолированном (да) или не изолированном (нет) браузере?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3356"/>
        <source>Remember choice for later.</source>
        <translation>Запомнить выбор.</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="88"/>
        <source>The selected window is running as part of program %1 in sandbox %2</source>
        <translation>Выбранное окно работает как часть программы %1 в песочнице %2</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="95"/>
        <source>The selected window is not running as part of any sandboxed program.</source>
        <translation>Выбранное окно не запущено как часть какой-либо изолированной программы.</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="134"/>
        <source>Drag the Finder Tool over a window to select it, then release the mouse to check if the window is sandboxed.</source>
        <translation>Перетащите инструмент поиска на окно, чтобы выбрать его, затем отпустите кнопку мыши, чтобы проверить, является ли окно изолированным.</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="204"/>
        <source>Sandboxie-Plus - Window Finder</source>
        <translation>Sandboxie-Plus - Поиск окна</translation>
    </message>
    <message>
        <location filename="main.cpp" line="123"/>
        <source>Sandboxie Manager can not be run sandboxed!</source>
        <translation>Sandboxie Manager нельзя запускать в песочнице!</translation>
    </message>
</context>
<context>
    <name>CSbieModel</name>
    <message>
        <location filename="Models/SbieModel.cpp" line="159"/>
        <source>Box Group</source>
        <translation>Группа песочницы</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="315"/>
        <source>Empty</source>
        <translation>Пусто</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="559"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="560"/>
        <source>Process ID</source>
        <translation>ID процесса</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="561"/>
        <source>Status</source>
        <translation>Статус</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="562"/>
        <source>Title</source>
        <translation>Заголовок</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="563"/>
        <source>Info</source>
        <translation>Информация</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="567"/>
        <source>Path / Command Line</source>
        <translation>Путь / Командная строка</translation>
    </message>
</context>
<context>
    <name>CSbieProcess</name>
    <message>
        <location filename="SbieProcess.cpp" line="59"/>
        <source>Sbie RpcSs</source>
        <translation>Sbie RpcSs (удаленный вызов процедур)</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="60"/>
        <source>Sbie DcomLaunch</source>
        <translation>Sbie DcomLaunch (plug and play)</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="61"/>
        <source>Sbie Crypto</source>
        <translation>Sbie Crypto (CSP)</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="62"/>
        <source>Sbie WuauServ</source>
        <translation>Sbie WuauServ</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="63"/>
        <source>Sbie BITS</source>
        <translation>Sbie BITS (фоновая интеллектуальная служба передачи)</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="64"/>
        <source>Sbie Svc</source>
        <translation>Sbie Svc</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="65"/>
        <source>MSI Installer</source>
        <translation>MSI Installer</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="66"/>
        <source>Trusted Installer</source>
        <translation>Trusted Installer</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="67"/>
        <source>Windows Update</source>
        <translation>Центр обновления Windows</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="68"/>
        <source>Windows Explorer</source>
        <translation>Проводник</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="69"/>
        <source>Internet Explorer</source>
        <translation>Internet Explorer</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="70"/>
        <source>Firefox</source>
        <translation>Firefox</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="71"/>
        <source>Windows Media Player</source>
        <translation>Проигрыватель Windows Media</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="72"/>
        <source>Winamp</source>
        <translation>Winamp</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="73"/>
        <source>KMPlayer</source>
        <translation>KMPlayer</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="74"/>
        <source>Windows Live Mail</source>
        <translation>Почта Windows Live</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="75"/>
        <source>Service Model Reg</source>
        <translation>Service Model Reg</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="76"/>
        <source>RunDll32</source>
        <translation>RunDll32</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="77"/>
        <location filename="SbieProcess.cpp" line="78"/>
        <source>DllHost</source>
        <translation>DllHost</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="79"/>
        <source>Windows Ink Services</source>
        <translation>Службы Windows Ink</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="80"/>
        <source>Chromium Based</source>
        <translation>На основе Chromium</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="81"/>
        <source>Google Updater</source>
        <translation>Программа обновлений Google</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="82"/>
        <source>Acrobat Reader</source>
        <translation>Acrobat Reader</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="83"/>
        <source>MS Outlook</source>
        <translation>MS Outlook</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="84"/>
        <source>MS Excel</source>
        <translation>MS Excel</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="85"/>
        <source>Flash Player</source>
        <translation>Flash Player</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="86"/>
        <source>Firefox Plugin Container</source>
        <translation>Firefox контейнер плагина</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="87"/>
        <source>Generic Web Browser</source>
        <translation>Стандартный веб-браузер</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="88"/>
        <source>Generic Mail Client</source>
        <translation>Стандартный почтовый клиент</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="89"/>
        <source>Thunderbird</source>
        <translation>Thunderbird</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="99"/>
        <source>Terminated</source>
        <translation>Прекращено</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="105"/>
        <source>Forced </source>
        <translation>Принудительно </translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="103"/>
        <source>Running</source>
        <translation>Выполняется</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="109"/>
        <source> Elevated</source>
        <translation> Повышенный</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="111"/>
        <source> as System</source>
        <translation> как система</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="114"/>
        <source> in session %1</source>
        <translation> в сеансе %1</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="120"/>
        <source> (%1)</source>
        <translation> (%1)</translation>
    </message>
</context>
<context>
    <name>CSbieView</name>
    <message>
        <location filename="Views/SbieView.cpp" line="151"/>
        <location filename="Views/SbieView.cpp" line="281"/>
        <source>Create New Box</source>
        <translation>Создать новую песочницу</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="397"/>
        <source>Remove Group</source>
        <translation>Удалить группу</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="161"/>
        <location filename="Views/SbieView.cpp" line="291"/>
        <source>Run</source>
        <translation>Запустить</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="166"/>
        <source>Run Program</source>
        <translation>Запустить программу</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="167"/>
        <source>Run from Start Menu</source>
        <translation>Запустить из меню &apos;Пуск&apos;</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="178"/>
        <source>Default Web Browser</source>
        <translation>Веб-браузер по умолчанию</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="179"/>
        <source>Default eMail Client</source>
        <translation>Почтовый клиент по умолчанию</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="181"/>
        <source>Windows Explorer</source>
        <translation>Проводник</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="182"/>
        <source>Registry Editor</source>
        <translation>Редактор реестра</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="183"/>
        <source>Programs and Features</source>
        <translation>Программы и компоненты</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="194"/>
        <source>Terminate All Programs</source>
        <translation>Завершить все программы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="102"/>
        <location filename="Views/SbieView.cpp" line="200"/>
        <location filename="Views/SbieView.cpp" line="259"/>
        <location filename="Views/SbieView.cpp" line="339"/>
        <location filename="Views/SbieView.cpp" line="378"/>
        <source>Create Shortcut</source>
        <translation>Создать ярлык</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="202"/>
        <location filename="Views/SbieView.cpp" line="324"/>
        <source>Explore Content</source>
        <translation>Открыть в проводнике</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="204"/>
        <location filename="Views/SbieView.cpp" line="331"/>
        <source>Snapshots Manager</source>
        <translation>Менеджер снимков</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="205"/>
        <source>Recover Files</source>
        <translation>Восстановить файлы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="206"/>
        <location filename="Views/SbieView.cpp" line="323"/>
        <source>Delete Content</source>
        <translation>Удалить содержимое</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="213"/>
        <source>Sandbox Presets</source>
        <translation>Предустановки песочницы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="215"/>
        <source>Ask for UAC Elevation</source>
        <translation>Запросить повышение уровня UAC</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="216"/>
        <source>Drop Admin Rights</source>
        <translation>Отбросить права администратора</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="217"/>
        <source>Emulate Admin Rights</source>
        <translation>Эмуляция прав администратора</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="225"/>
        <source>Block Internet Access</source>
        <translation>Блокировать доступ в Интернет</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="227"/>
        <source>Allow Network Shares</source>
        <translation>Разрешить сетевые ресурсы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="208"/>
        <source>Sandbox Options</source>
        <translation>Опции песочницы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="177"/>
        <source>Standard Applications</source>
        <translation>Стандартные приложения</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="197"/>
        <source>Browse Files</source>
        <translation>Просмотр файлов</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="236"/>
        <location filename="Views/SbieView.cpp" line="329"/>
        <source>Sandbox Tools</source>
        <translation>Инструменты песочницы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="237"/>
        <source>Duplicate Box Config</source>
        <translation>Дублировать конфигурацию песочницы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="240"/>
        <location filename="Views/SbieView.cpp" line="342"/>
        <source>Rename Sandbox</source>
        <translation>Переименовать песочницу</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="241"/>
        <location filename="Views/SbieView.cpp" line="343"/>
        <source>Move Sandbox</source>
        <translation>Переместить песочницу</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="252"/>
        <location filename="Views/SbieView.cpp" line="354"/>
        <source>Remove Sandbox</source>
        <translation>Удалить песочницу</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="257"/>
        <location filename="Views/SbieView.cpp" line="376"/>
        <source>Terminate</source>
        <translation>Завершить</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="260"/>
        <source>Preset</source>
        <translation>Предустановка</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="100"/>
        <location filename="Views/SbieView.cpp" line="261"/>
        <source>Pin to Run Menu</source>
        <translation>Закрепить в меню &apos;Запустить&apos;</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="263"/>
        <source>Block and Terminate</source>
        <translation>Заблокировать и завершить</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="267"/>
        <source>Allow internet access</source>
        <translation>Разрешить доступ в Интернет</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="269"/>
        <source>Force into this sandbox</source>
        <translation>Принудительно в этой песочнице</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="271"/>
        <source>Set Linger Process</source>
        <translation>Установить вторичный процесс</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="273"/>
        <source>Set Leader Process</source>
        <translation>Установить первичный процесс</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="536"/>
        <source>    File root: %1
</source>
        <translation>    Корень файла: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="537"/>
        <source>    Registry root: %1
</source>
        <translation>    Корень реестра: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="538"/>
        <source>    IPC root: %1
</source>
        <translation>    Корень IPC: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="540"/>
        <source>Options:
    </source>
        <translation>Опции:
    </translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="748"/>
        <source>[None]</source>
        <translation>[Нет]</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1059"/>
        <source>Please enter a new group name</source>
        <translation>Пожалуйста, введите новое имя группы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="857"/>
        <source>Do you really want to remove the selected group(s)?</source>
        <translation>Вы действительно хотите удалить выбранные группы?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="152"/>
        <location filename="Views/SbieView.cpp" line="282"/>
        <source>Create Box Group</source>
        <translation>Создать группу песочниц</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="395"/>
        <source>Rename Group</source>
        <translation>Переименовать группу</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="157"/>
        <location filename="Views/SbieView.cpp" line="287"/>
        <source>Stop Operations</source>
        <translation>Остановить операции</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="186"/>
        <source>Command Prompt</source>
        <translation>Командная строка</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="187"/>
        <source>Command Prompt (as Admin)</source>
        <translation>Командная строка (от администратора)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="191"/>
        <source>Command Prompt (32-bit)</source>
        <translation>Командная строка (32-бит)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="176"/>
        <source>Execute Autorun Entries</source>
        <translation>Выполнить записи автозапуска</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="330"/>
        <source>Browse Content</source>
        <translation>Просмотр содержимого</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="196"/>
        <source>Box Content</source>
        <translation>Содержимое песочницы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="203"/>
        <source>Open Registry</source>
        <translation>Открыть в редакторе реестра</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="199"/>
        <location filename="Views/SbieView.cpp" line="338"/>
        <source>Refresh Info</source>
        <translation>Обновить информацию</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="153"/>
        <location filename="Views/SbieView.cpp" line="283"/>
        <source>Import Box</source>
        <translation>Импорт песочницы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="170"/>
        <location filename="Views/SbieView.cpp" line="302"/>
        <source>(Host) Start Menu</source>
        <translation>(Хост) Меню &quot;Пуск&quot;</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="231"/>
        <source>Immediate Recovery</source>
        <translation>Немедленное восстановление</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="233"/>
        <source>Disable Force Rules</source>
        <translation>Отключить принудительные правила</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="238"/>
        <source>Export Box</source>
        <translation>Экспорт контейнера</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="242"/>
        <location filename="Views/SbieView.cpp" line="344"/>
        <source>Move Up</source>
        <translation>Сдвинуть вверх</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="247"/>
        <location filename="Views/SbieView.cpp" line="349"/>
        <source>Move Down</source>
        <translation>Сдвинуть вниз</translation>
    </message>
    <message>
        <source>Run Sandboxed</source>
        <translation type="vanished">Запустить в песочнице</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="296"/>
        <source>Run Web Browser</source>
        <translation>Запустить веб-браузер</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="297"/>
        <source>Run eMail Reader</source>
        <translation>Запустить почтовый клиент</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="298"/>
        <source>Run Any Program</source>
        <translation>Запустить любую программу</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="299"/>
        <source>Run From Start Menu</source>
        <translation>Запустить из меню &quot;Пуск&quot;</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="307"/>
        <source>Run Windows Explorer</source>
        <translation>Запустить проводник Windows</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="321"/>
        <source>Terminate Programs</source>
        <translation>Завершить программы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="322"/>
        <source>Quick Recover</source>
        <translation>Быстрое восстановление</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="327"/>
        <source>Sandbox Settings</source>
        <translation>Настройки песочницы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="334"/>
        <source>Duplicate Sandbox Config</source>
        <translation>Дублировать конфигурацию песочницы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="335"/>
        <source>Export Sandbox</source>
        <translation>Экспорт песочницы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="396"/>
        <source>Move Group</source>
        <translation>Переместить группу</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="847"/>
        <source>Please enter a new name for the Group.</source>
        <translation>Пожалуйста, введите новое имя для группы.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="894"/>
        <source>Move entries by (negative values move up, positive values move down):</source>
        <translation>Сдвинуть записи на (отрицательные значения сдвигают вверх, положительные значения сдвигают вниз):</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="942"/>
        <source>A group can not be its own parent.</source>
        <translation>Группа не может быть собственным родителем.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1031"/>
        <source>This name is already in use, please select an alternative box name</source>
        <translation>Это имя уже используется, выберите другое имя песочницы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1045"/>
        <source>Importing: %1</source>
        <translation>Импорт: %1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1085"/>
        <source>The Sandbox name and Box Group name cannot use the &apos;,()&apos; symbol.</source>
        <translation>Имя песочницы и имя группы песочниц не могут использовать символы &apos;,()&apos;.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1090"/>
        <source>This name is already used for a Box Group.</source>
        <translation>Это имя уже используется для группы песочниц.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1095"/>
        <source>This name is already used for a Sandbox.</source>
        <translation>Это имя уже используется для песочницы.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1145"/>
        <location filename="Views/SbieView.cpp" line="1215"/>
        <location filename="Views/SbieView.cpp" line="1462"/>
        <source>Don&apos;t show this message again.</source>
        <translation>Больше не показывать это сообщение.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1206"/>
        <location filename="Views/SbieView.cpp" line="1226"/>
        <location filename="Views/SbieView.cpp" line="1630"/>
        <source>This Sandbox is empty.</source>
        <translation>Эта песочница пуста.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1233"/>
        <source>WARNING: The opened registry editor is not sandboxed, please be careful and only do changes to the pre-selected sandbox locations.</source>
        <translation>ПРЕДУПРЕЖДЕНИЕ: Редактор реестра запускается вне песочницы, будьте осторожны и вносите изменения только в предварительно выбранные местоположения песочницы.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1234"/>
        <source>Don&apos;t show this warning in future</source>
        <translation>Не показывать это предупреждение в будущем</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1290"/>
        <source>Please enter a new name for the duplicated Sandbox.</source>
        <translation>Введите новое имя копии песочницы.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1290"/>
        <source>%1 Copy</source>
        <translation>%1 Копия</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1018"/>
        <location filename="Views/SbieView.cpp" line="1329"/>
        <source>Select file name</source>
        <translation>Выберите имя файла</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1018"/>
        <location filename="Views/SbieView.cpp" line="1329"/>
        <source>7-zip Archive (*.7z)</source>
        <translation>7-zip архив (*.7z)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1337"/>
        <source>Exporting: %1</source>
        <translation>Экспорт: %1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1344"/>
        <source>Please enter a new name for the Sandbox.</source>
        <translation>Введите новое имя для песочницы.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1365"/>
        <source>Do you really want to remove the selected sandbox(es)?&lt;br /&gt;&lt;br /&gt;Warning: The box content will also be deleted!</source>
        <translation>Вы действительно хотите удалить выбранные песочницы?&lt;br /&gt;&lt;br /&gt;Внимание: содержимое песочницы также будет удалено!</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1403"/>
        <source>This Sandbox is already empty.</source>
        <translation>Эта песочница уже пуста.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1413"/>
        <source>Do you want to delete the content of the selected sandbox?</source>
        <translation>Вы хотите удалить содержимое выбранной песочницы?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1414"/>
        <location filename="Views/SbieView.cpp" line="1418"/>
        <source>Also delete all Snapshots</source>
        <translation>Также удалить все снимки</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1417"/>
        <source>Do you really want to delete the content of all selected sandboxes?</source>
        <translation>Вы действительно хотите удалить содержимое всех выбранных песочниц?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1444"/>
        <source>Do you want to terminate all processes in the selected sandbox(es)?</source>
        <translation>Вы хотите завершить все процессы в выбранных песочницах?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1445"/>
        <location filename="Views/SbieView.cpp" line="1539"/>
        <source>Terminate without asking</source>
        <translation>Завершить без запроса</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1460"/>
        <source>The Sandboxie Start Menu will now be displayed. Select an application from the menu, and Sandboxie will create a new shortcut icon on your real desktop, which you can use to invoke the selected application under the supervision of Sandboxie.</source>
        <translation>Cтартовое меню Sandboxie теперь будет отображено. Выберите приложение из меню, и Sandboxie создаст новый ярлык на вашем реальном рабочем столе, который вы можете использовать для вызова выбранного приложения под контролем Sandboxie.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1514"/>
        <location filename="Views/SbieView.cpp" line="1566"/>
        <source>Create Shortcut to sandbox %1</source>
        <translation>Создать ярлык для песочницы %1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1538"/>
        <source>Do you want to terminate %1?</source>
        <translation>Вы хотите завершить %1?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1538"/>
        <source>the selected processes</source>
        <translation>выбранные процессы</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1589"/>
        <source>This box does not have Internet restrictions in place, do you want to enable them?</source>
        <translation>В этой песочнице нет ограничений на доступ к Интернет, вы хотите их включить?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1673"/>
        <source>This sandbox is disabled, do you want to enable it?</source>
        <translation>Эта песочница отключена, вы хотите ее включить?</translation>
    </message>
</context>
<context>
    <name>CSelectBoxWindow</name>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="89"/>
        <source>Sandboxie-Plus - Run Sandboxed</source>
        <translation>Sandboxie-Plus - Запуск в песочнице</translation>
    </message>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="206"/>
        <source>Are you sure you want to run the program outside the sandbox?</source>
        <translation>Вы уверены, что хотите запустить программу вне песочницы?</translation>
    </message>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="219"/>
        <source>Please select a sandbox.</source>
        <translation>Выберите песочницу.</translation>
    </message>
</context>
<context>
    <name>CSettingsWindow</name>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="130"/>
        <source>Sandboxie Plus - Global Settings</source>
        <translation>Sandboxie Plus - Глобальные настройки</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="216"/>
        <source>Auto Detection</source>
        <translation>Автоопределение</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="217"/>
        <source>No Translation</source>
        <translation>Нет перевода</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="237"/>
        <source>Don&apos;t integrate links</source>
        <translation>Не интегрировать ссылки</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="238"/>
        <source>As sub group</source>
        <translation>Как подгруппа</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="239"/>
        <source>Fully integrate</source>
        <translation>Полностью интегрировать</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="241"/>
        <source>Don&apos;t show any icon</source>
        <translation>Не показывать никаких значков</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="242"/>
        <source>Show Plus icon</source>
        <translation>Показать Plus значок</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="243"/>
        <source>Show Classic icon</source>
        <translation>Показать классический значок</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="245"/>
        <source>All Boxes</source>
        <translation>Все песочницы</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="246"/>
        <source>Active + Pinned</source>
        <translation>Активные + закрепленные</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="247"/>
        <source>Pinned Only</source>
        <translation>Только закрепленные</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="249"/>
        <source>None</source>
        <translation>Нет</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="250"/>
        <source>Native</source>
        <translation>Родной</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="251"/>
        <source>Qt</source>
        <translation>Qt</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="253"/>
        <source>Ignore</source>
        <translation>Игнорировать</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="265"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="496"/>
        <source>Search for settings</source>
        <translation>Поиск настроек</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="719"/>
        <location filename="Windows/SettingsWindow.cpp" line="720"/>
        <location filename="Windows/SettingsWindow.cpp" line="730"/>
        <source>Run &amp;Sandboxed</source>
        <translation>Запуск в песочнице (&amp;S)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="751"/>
        <source>Sandboxed Web Browser</source>
        <translation>Веб-браузер в песочнице</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="254"/>
        <location filename="Windows/SettingsWindow.cpp" line="259"/>
        <source>Notify</source>
        <translation>Уведомлять</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="255"/>
        <location filename="Windows/SettingsWindow.cpp" line="260"/>
        <source>Download &amp; Notify</source>
        <translation>Загрузить и уведомить</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="256"/>
        <location filename="Windows/SettingsWindow.cpp" line="261"/>
        <source>Download &amp; Install</source>
        <translation>Загрузить и установить</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="345"/>
        <source>Browse for Program</source>
        <translation>Выбрать программу</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="412"/>
        <source>Add %1 Template</source>
        <translation>Добавить шаблон %1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="597"/>
        <source>Please enter message</source>
        <translation>Пожалуйста, введите сообщение</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="629"/>
        <source>Select Program</source>
        <translation>Выбрать программу</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="629"/>
        <source>Executables (*.exe *.cmd)</source>
        <translation>Исполняемые файлы (*.exe *.cmd)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="633"/>
        <location filename="Windows/SettingsWindow.cpp" line="646"/>
        <source>Please enter a menu title</source>
        <translation>Введите название меню</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="642"/>
        <source>Please enter a command</source>
        <translation>Введите команду</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="964"/>
        <source>This supporter certificate has expired, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>Срок действия этого сертификата сторонника истек, &lt;a href=&quot;sbie://update/cert&quot;&gt;получите обновленный сертификат&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="967"/>
        <source>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Plus features will be disabled in %1 days.&lt;/font&gt;</source>
        <translation>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Дополнительные функции будут отключены через %1 дн.&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="969"/>
        <source>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;For this build Plus features remain enabled.&lt;/font&gt;</source>
        <translation>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Для этой сборки функции Plus остаются включенными.&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="971"/>
        <source>&lt;br /&gt;Plus features are no longer enabled.</source>
        <translation>&lt;br /&gt;Дополнительные функции больше не включены.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="977"/>
        <source>This supporter certificate will &lt;font color=&apos;red&apos;&gt;expire in %1 days&lt;/font&gt;, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>Срок действия этого сертификата сторонника &lt;font color=&apos;red&apos;&gt;истечет через %1 дн.&lt;/font&gt;, пожалуйста, &lt;a href=&quot;sbie://update/cert&quot;&gt;получите обновленный сертификат&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1006"/>
        <source>Supporter certificate required</source>
        <translation>Требуется сертификат сторонника</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1112"/>
        <source>Run &amp;Un-Sandboxed</source>
        <translation>Запуск вне песочницы (&amp;U)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1371"/>
        <source>This does not look like a certificate. Please enter the entire certificate, not just a portion of it.</source>
        <translation>Это не похоже на сертификат. Пожалуйста, введите весь сертификат, а не только его часть.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1390"/>
        <source>This certificate is unfortunately expired.</source>
        <translation>К сожалению, срок действия этого сертификата истек.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1392"/>
        <source>This certificate is unfortunately outdated.</source>
        <translation>Этот сертификат, к сожалению, устарел.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1395"/>
        <source>Thank you for supporting the development of Sandboxie-Plus.</source>
        <translation>Спасибо за поддержку разработки Sandboxie-Plus.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1402"/>
        <source>This support certificate is not valid.</source>
        <translation>Этот сертификат поддержки недействителен.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1440"/>
        <location filename="Windows/SettingsWindow.cpp" line="1576"/>
        <source>Select Directory</source>
        <translation>Выбрать каталог</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1469"/>
        <source>&lt;a href=&quot;check&quot;&gt;Check Now&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;check&quot;&gt;Проверить сейчас&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1536"/>
        <source>Please enter the new configuration password.</source>
        <translation>Пожалуйста, введите новый пароль конфигурации.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1540"/>
        <source>Please re-enter the new configuration password.</source>
        <translation>Пожалуйста, повторно введите новый пароль конфигурации.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1545"/>
        <source>Passwords did not match, please retry.</source>
        <translation>Пароли не совпадают, повторите попытку.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1557"/>
        <source>Process</source>
        <translation>Процесс</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1557"/>
        <source>Folder</source>
        <translation>Папка</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1567"/>
        <source>Please enter a program file name</source>
        <translation>Пожалуйста, введите имя файла программы</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1635"/>
        <source>Please enter the template identifier</source>
        <translation>Пожалуйста, введите идентификатор шаблона</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1643"/>
        <source>Error: %1</source>
        <translation>Ошибка: %1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1668"/>
        <source>Do you really want to delete the selected local template(s)?</source>
        <translation>Вы действительно хотите удалить выбранные локальные шаблоны?</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1824"/>
        <source>%1 (Current)</source>
        <translation>%1 (Текущая)</translation>
    </message>
</context>
<context>
    <name>CSetupWizard</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="29"/>
        <source>Setup Wizard</source>
        <translation>Мастер настройки</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="40"/>
        <source>The decision you make here will affect which page you get to see next.</source>
        <translation>Решение, которое вы примете здесь, повлияет на то, какую страницу вы увидите следующей.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="43"/>
        <source>This help is likely not to be of any help.</source>
        <translation>Эта помощь, скорее всего, ничем не поможет.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="47"/>
        <source>Sorry, I already gave all the help I could.</source>
        <translation>Извините, я уже оказал всю помощь, которую мог.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="49"/>
        <source>Setup Wizard Help</source>
        <translation>Справка мастера настройки</translation>
    </message>
</context>
<context>
    <name>CShellPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="417"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; shell integration</source>
        <translation>Настройка интеграции оболочки &lt;b&gt;Sandboxie-Plus&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="418"/>
        <source>Configure how Sandboxie-Plus should integrate with your system.</source>
        <translation>Настройка интеграции Sandboxie-Plus с вашей системой.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="422"/>
        <source>Start UI with Windows</source>
        <translation>Запуск пользовательского интерфейса с Windows</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="427"/>
        <source>Add &apos;Run Sandboxed&apos; to the explorer context menu</source>
        <translation>Добавить &apos;Запустить в песочнице&apos; в контекстное меню проводника</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="432"/>
        <source>Add desktop shortcut for starting Web browser under Sandboxie</source>
        <translation>Добавить ярлык для запуска веб-браузера в Sandboxie на рабочий стол</translation>
    </message>
</context>
<context>
    <name>CSnapshotsWindow</name>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="24"/>
        <source>%1 - Snapshots</source>
        <translation>%1 - Снимки</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="38"/>
        <source>Snapshot</source>
        <translation>Снимок</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="56"/>
        <source>Revert to empty box</source>
        <translation>Возврат к пустой песочнице</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="103"/>
        <source> (default)</source>
        <translation> (по умолчанию)</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="173"/>
        <source>Please enter a name for the new Snapshot.</source>
        <translation>Пожалуйста, введите имя для нового снимка.</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="173"/>
        <source>New Snapshot</source>
        <translation>Новый снимок</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="196"/>
        <source>Do you really want to switch the active snapshot? Doing so will delete the current state!</source>
        <translation>Вы действительно хотите переключить активный снимок? Это приведет к удалению текущего состояния!</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="230"/>
        <source>Do you really want to delete the selected snapshot?</source>
        <translation>Вы действительно хотите удалить выбранный снимок?</translation>
    </message>
</context>
<context>
    <name>CStackView</name>
    <message>
        <location filename="Views/StackView.cpp" line="17"/>
        <source>#|Symbol</source>
        <translation>#|Символ</translation>
    </message>
</context>
<context>
    <name>CSummaryPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="527"/>
        <source>Create the new Sandbox</source>
        <translation>Создать новую песочницу</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="535"/>
        <source>Almost complete, click Finish to create a new sandbox and conclude the wizard.</source>
        <translation>Почти завершено, нажмите &quot;Готово&quot;, чтобы создать новую песочницу и завершить работу мастера.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="544"/>
        <source>Save options as new defaults</source>
        <translation>Сохранить параметры как новые значения по умолчанию</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="555"/>
        <source>Don&apos;t show the summary page in future (unless advanced options were set)</source>
        <translation>Не показывать страницу со сводкой в будущем (если не установлены дополнительные параметры)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="574"/>
        <source>
This Sandbox will be saved to: %1</source>
        <translation>
Эта песочница будет сохранена в: %1</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="577"/>
        <source>
This box&apos;s content will be DISCARDED when it&apos;s closed, and the box will be removed.</source>
        <translation>
Содержимое этой песочницы будет ПОТЕРЯНО, когда она будет закрыта, и песочница будет удалена.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="579"/>
        <source>
This box will DISCARD its content when its closed, its suitable only for temporary data.</source>
        <translation>
Эта песочница ТЕРЯЕТ свое содержимое, при закрытии, она подходит только для временных данных.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="581"/>
        <source>
Processes in this box will not be able to access the internet or the local network, this ensures all accessed data to stay confidential.</source>
        <translation>
Процессы в этой песочнице не смогут получить доступ к Интернету или локальной сети, это гарантирует, что все данные, к которым осуществляется доступ, останутся конфиденциальными.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="583"/>
        <source>
This box will run the MSIServer (*.msi installer service) with a system token, this improves the compatibility but reduces the security isolation.</source>
        <translation>
В этой песочнице будет запускаться MSIServer (служба установщика *.msi) с системным токеном, это улучшает совместимость, но снижает изоляцию безопасности.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="585"/>
        <source>
Processes in this box will think they are run with administrative privileges, without actually having them, hence installers can be used even in a security hardened box.</source>
        <translation>
Процессы в этой песочнице будут думать, что они запущены с правами администратора, но на самом деле не имеют их, поэтому установщики можно использовать даже в защищенной песочнице.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="587"/>
        <source>
Processes in this box will be running with a custom process token indicating the sandbox they belong to.</source>
        <translation>
Процессы в этой песочнице будут запускаться с пользовательским токеном процесса, указывающим на песочницу, к которой они принадлежат.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="620"/>
        <source>Failed to create new box: %1</source>
        <translation>Не удалось создать новую песочницу: %1</translation>
    </message>
</context>
<context>
    <name>CSupportDialog</name>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="115"/>
        <source>The installed supporter certificate &lt;b&gt;has expired %1 days ago&lt;/b&gt; and &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;must be renewed&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Срок действия установленного сертификата поддержки &lt;b&gt;истек %1&#xa0;дн. назад&lt;/b&gt; и его &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;необходимо обновить&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="117"/>
        <source>&lt;b&gt;You have installed Sandboxie-Plus more than %1 days ago.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</source>
        <translation>&lt;b&gt;Вы установили Sandboxie-Plus более %1 дн. назад.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="119"/>
        <source>&lt;u&gt;Commercial use of Sandboxie past the evaluation period&lt;/u&gt;, requires a valid &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;support certificate&lt;/a&gt;.</source>
        <translation>&lt;u&gt;Для коммерческого использования Sandboxie по истечении ознакомительного периода&lt;/u&gt;, требуется действующий &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;сертификат поддержки&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="126"/>
        <source>The installed supporter certificate is &lt;b&gt;outdated&lt;/b&gt; and it is &lt;u&gt;not valid for&lt;b&gt; this version&lt;/b&gt;&lt;/u&gt; of Sandboxie-Plus.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Установленный сертификат поддержки &lt;b&gt;устарел&lt;/b&gt; и &lt;u&gt;недействителен для&lt;b&gt; этой версии&lt;/b&gt;&lt;/u&gt; Sandboxie-Plus.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="128"/>
        <source>The installed supporter certificate is &lt;b&gt;expired&lt;/b&gt; and &lt;u&gt;should be renewed&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Срок действия установленного сертификата поддержки &lt;b&gt;истек&lt;/b&gt;, и его &lt;u&gt;следует обновить&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="130"/>
        <source>&lt;b&gt;You have been using Sandboxie-Plus for more than %1 days now.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</source>
        <translation>&lt;b&gt;Вы используете Sandboxie-Plus уже более %1 дн.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="133"/>
        <source>Sandboxie on ARM64 requires a valid supporter certificate for continued use.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Для дальнейшего использования Sandboxie на ARM64 требуется действующий сертификат поддержки.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="135"/>
        <source>Personal use of Sandboxie is free of charge on x86/x64, although some functionality is only available to project supporters.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Персональное использование Sandboxie на платформах x86/x64 бесплатно, хотя некоторые функции доступны только для сторонников проекта.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="138"/>
        <source>Please continue &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;supporting the project&lt;/a&gt; by renewing your &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; and continue using the &lt;b&gt;enhanced functionality&lt;/b&gt; in new builds.</source>
        <translation>Пожалуйста, продолжайте &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;поддерживать проект&lt;/a&gt;, обновив свой &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;сертификат поддержки&lt;/a&gt; и продолжать использовать &lt;b&gt;расширенные функции&lt;/b&gt; в новых сборках.</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="141"/>
        <source>Sandboxie &lt;u&gt;without&lt;/u&gt; a valid supporter certificate will sometimes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;pause for a few seconds&lt;/font&gt;&lt;/b&gt;. This pause allows you to consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;purchasing a supporter certificate&lt;/a&gt; or &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;earning one by contributing&lt;/a&gt; to the project. &lt;br /&gt;&lt;br /&gt;A &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; not just removes this reminder, but also enables &lt;b&gt;exclusive enhanced functionality&lt;/b&gt; providing better security and compatibility.</source>
        <translation>Sandboxie &lt;u&gt;без&lt;/u&gt; действительного сертификата поддержки иногда &lt;b&gt;&lt;font color=&apos;red&apos;&gt;приостанавливается на несколько секунд&lt;/font&gt;&lt;/b&gt;. Эта пауза позволяет рассмотреть возможность &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;приобретения сертификата поддержки&lt;/a&gt; или &lt;a href=&quot;https: //sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;заработать один, внося свой вклад&lt;/a&gt; в проект. &lt;br /&gt;&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;сертификат поддержки&lt;/a&gt; не только удаляет это напоминание, но и включает &lt;b&gt;эксклюзивную расширенную функциональность&lt;/b&gt;, обеспечивающую лучшую безопасность и совместимость.</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="172"/>
        <source>Sandboxie-Plus - Support Reminder</source>
        <translation>Sandboxie-Plus - Напоминание о поддержке</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="251"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="258"/>
        <source>Quit</source>
        <translation>Выход</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="259"/>
        <source>Continue</source>
        <translation>Продолжить</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="260"/>
        <source>Get Certificate</source>
        <translation>Получить сертификат</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="261"/>
        <source>Enter Certificate</source>
        <translation>Ввести сертификат</translation>
    </message>
</context>
<context>
    <name>CTemplateTypePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="258"/>
        <source>Create new Template</source>
        <translation>Создать новый шаблон</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="267"/>
        <source>Select template type:</source>
        <translation>Выберите тип шаблона:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="271"/>
        <source>%1 template</source>
        <translation>%1 шаблон</translation>
    </message>
</context>
<context>
    <name>CTemplateWizard</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="37"/>
        <source>Compatibility Template Wizard</source>
        <translation>Мастер шаблонов совместимости</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="48"/>
        <source>Custom</source>
        <translation>Пользовательский</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="49"/>
        <source>Web Browser</source>
        <translation>Веб-браузер</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="82"/>
        <source>Force %1 to run in this sandbox</source>
        <translation>Заставить %1 работать в этой песочнице</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="100"/>
        <source>Allow direct access to the entire %1 profile folder</source>
        <translation>Разрешить прямой доступ ко всей папке профиля %1</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="113"/>
        <location filename="Wizards/TemplateWizard.cpp" line="168"/>
        <source>Allow direct access to %1 phishing database</source>
        <translation>Разрешить прямой доступ к фишинговой базе данных %1</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="127"/>
        <source>Allow direct access to %1 session management</source>
        <translation>Разрешить прямой доступ к управлению сеансом %1</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="136"/>
        <location filename="Wizards/TemplateWizard.cpp" line="199"/>
        <source>Allow direct access to %1 passwords</source>
        <translation>Разрешить прямой доступ к паролям %1</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="146"/>
        <location filename="Wizards/TemplateWizard.cpp" line="208"/>
        <source>Allow direct access to %1 cookies</source>
        <translation>Разрешить прямой доступ к %1 файлам cookie</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="155"/>
        <location filename="Wizards/TemplateWizard.cpp" line="227"/>
        <source>Allow direct access to %1 bookmark and history database</source>
        <translation>Разрешить прямой доступ к %1 базе данных закладок и истории</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="180"/>
        <source>Allow direct access to %1 sync data</source>
        <translation>Разрешить прямой доступ к данным синхронизации %1</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="190"/>
        <source>Allow direct access to %1 preferences</source>
        <translation>Разрешить прямой доступ к настройкам %1</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="217"/>
        <source>Allow direct access to %1 bookmarks</source>
        <translation>Разрешить прямой доступ к закладкам %1</translation>
    </message>
</context>
<context>
    <name>CTraceModel</name>
    <message>
        <location filename="Models/TraceModel.cpp" line="196"/>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="175"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="176"/>
        <source>Process %1</source>
        <translation>Процесс %1</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="179"/>
        <source>Thread %1</source>
        <translation>Поток %1</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="333"/>
        <source>Process</source>
        <translation>Процесс</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="335"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="336"/>
        <source>Status</source>
        <translation>Статус</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="337"/>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
</context>
<context>
    <name>CTraceView</name>
    <message>
        <location filename="Views/TraceView.cpp" line="255"/>
        <source>Show as task tree</source>
        <translation>Показать как дерево задач</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="259"/>
        <source>Show NT Object Tree</source>
        <translation>Показать дерево объектов NT</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="266"/>
        <source>PID:</source>
        <translation>ID процесса:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="268"/>
        <location filename="Views/TraceView.cpp" line="275"/>
        <location filename="Views/TraceView.cpp" line="284"/>
        <location filename="Views/TraceView.cpp" line="285"/>
        <location filename="Views/TraceView.cpp" line="294"/>
        <location filename="Views/TraceView.cpp" line="550"/>
        <location filename="Views/TraceView.cpp" line="553"/>
        <location filename="Views/TraceView.cpp" line="651"/>
        <source>[All]</source>
        <translation>[Все]</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="273"/>
        <source>TID:</source>
        <translation>ID потока:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="281"/>
        <source>Type:</source>
        <translation>Тип:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="292"/>
        <source>Status:</source>
        <translation>Статус:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="295"/>
        <source>Open</source>
        <translation>Открытый</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="296"/>
        <source>Closed</source>
        <translation>Закрытый</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="297"/>
        <source>Trace</source>
        <translation>Трассировка</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="298"/>
        <source>Other</source>
        <translation>Другое</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="306"/>
        <source>Show All Boxes</source>
        <translation>Показать все песочницы</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="310"/>
        <source>Show Stack Trace</source>
        <translation>Показать трассировку стека</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="315"/>
        <source>Save to file</source>
        <translation>Сохранить в файл</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="329"/>
        <source>Cleanup Trace Log</source>
        <translation>Очистить журнал трассировки</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="694"/>
        <source>Save trace log to file</source>
        <translation>Сохранить журнал трассировки в файл</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="700"/>
        <source>Failed to open log file for writing</source>
        <translation>Не удалось открыть файл журнала для записи</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="720"/>
        <source>Unknown</source>
        <translation>Неизвестно</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="624"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="251"/>
        <source>Monitor mode</source>
        <translation>Режим монитора</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="631"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
</context>
<context>
    <name>CTraceWindow</name>
    <message>
        <location filename="Views/TraceView.cpp" line="753"/>
        <source>Sandboxie-Plus - Trace Monitor</source>
        <translation>Sandboxie-Plus - Монитор трассировки</translation>
    </message>
</context>
<context>
    <name>CUIPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="302"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; UI</source>
        <translation>Настройка интерфейса &lt;b&gt;Sandboxie-Plus&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="303"/>
        <source>Select the user interface style you prefer.</source>
        <translation>Выберите стиль интерфейса, который вы предпочитаете.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="307"/>
        <source>&amp;Advanced UI for experts</source>
        <translation>Расширенный интерфейс для экспертов (&amp;A)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="312"/>
        <source>&amp;Simple UI for beginners</source>
        <translation>&amp;Простой интерфейс для начинающих (&amp;S)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="317"/>
        <source>&amp;Vintage SbieCtrl.exe UI</source>
        <translation>Винтажный интерфейс SbieCtrl.exe (&amp;V)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="342"/>
        <source>Use Bright Mode</source>
        <translation>Использовать светлый режим</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="346"/>
        <source>Use Dark Mode</source>
        <translation>Использовать темный режим</translation>
    </message>
</context>
<context>
    <name>CWFPPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="453"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; network filtering</source>
        <translation>Настройка сетевой фильтрации &lt;b&gt;Sandboxie-Plus&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="454"/>
        <source>Sandboxie can use the Windows Filtering Platform (WFP) to restrict network access.</source>
        <translation>Sandboxie может использовать платформу фильтрации Windows (WFP) для ограничения доступа к сети.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="460"/>
        <source>Using WFP allows Sandboxie to reliably enforce IP/Port based rules for network access. Unlike system level application firewalls, Sandboxie can use different rules in each box for the same application. If you already have a good and reliable application firewall and do not need per box rules, you can leave this option unchecked. Without WFP enabled, Sandboxie will still be able to reliably and entirely block processes from accessing the network. However, this can cause the process to crash, as the driver blocks the required network device endpoints. Even with WFP disabled, Sandboxie offers to set IP/Port based rules, however these will be applied in user mode only and not be enforced by the driver. Hence, without WFP enabled, an intentionally malicious process could bypass those rules, but not the entire network block.</source>
        <translation>Использование WFP позволяет Sandboxie надежно применять правила доступа к сети на основе IP/порта. В отличие от брандмауэров приложений системного уровня, Sandboxie может использовать разные правила в каждой песочнице для одного и того же приложения. Если у вас уже есть хороший и надежный брандмауэр приложений и вам не нужны правила для каждой песочницы, вы можете оставить этот параметр не отмеченным. Без включения WFP Sandboxie по-прежнему сможет надежно и полностью блокировать доступ процессов к сети. Однако это может привести к сбою процесса, поскольку драйвер блокирует необходимые конечные точки сетевых устройств. Даже при отключенном WFP Sandboxie предлагает установить правила на основе IP/порта, однако они будут применяться только в пользовательском режиме и не будут применяться драйвером. Следовательно, без включенного WFP намеренно вредоносный процесс может обойти эти правила, но не весь сетевой блок.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="469"/>
        <source>Enable Windows Filtering Platform (WFP) support</source>
        <translation>Включить поддержку Windows Filtering Platform (WFP)</translation>
    </message>
</context>
<context>
    <name>NewBoxWindow</name>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="32"/>
        <source>SandboxiePlus new box</source>
        <translation>Sandboxie-Plus новая песочница</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="60"/>
        <source>Box Type Preset:</source>
        <translation>Предустановка типа песочницы:</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="91"/>
        <source>A sandbox isolates your host system from processes running within the box, it prevents them from making permanent changes to other programs and data in your computer. The level of isolation impacts your security as well as the compatibility with applications, hence there will be a different level of isolation depending on the selected Box Type. Sandboxie can also protect your personal data from being accessed by processes running under its supervision.</source>
        <translation>Песочница изолирует вашу хост-систему от процессов, запущенных в ней, и не позволяет им вносить постоянные изменения в другие программы и данные на вашем компьютере. Уровень изоляции влияет на вашу безопасность, а также на совместимость с приложениями, поэтому от типа выбранной песочницы зависит уровень ее изоляции. Sandboxie также может защитить ваши личные данные от доступа со стороны процессов, запущенных под его контролем.</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="101"/>
        <source>Box info</source>
        <translation>Информация о песочнице</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="67"/>
        <source>Sandbox Name:</source>
        <translation>Имя песочницы:</translation>
    </message>
</context>
<context>
    <name>OptionsWindow</name>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="32"/>
        <source>SandboxiePlus Options</source>
        <translation>Опции SandboxiePlus</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="52"/>
        <source>General Options</source>
        <translation>Общие настройки</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="62"/>
        <source>Box Options</source>
        <translation>Опции песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="96"/>
        <source>Sandbox Indicator in title:</source>
        <translation>Индикатор песочницы в заголовке:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="106"/>
        <source>Sandboxed window border:</source>
        <translation>Рамка изолированного окна:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="259"/>
        <source>Double click action:</source>
        <translation>Действие двойного щелчка:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="359"/>
        <source>Separate user folders</source>
        <translation>Раздельные папки пользователей</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="396"/>
        <source>Box Structure</source>
        <translation>Структура песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="953"/>
        <source>Security Options</source>
        <translation>Параметры безопасности</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="963"/>
        <source>Security Hardening</source>
        <translation>Усиление безопасности</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="730"/>
        <location filename="Forms/OptionsWindow.ui" line="766"/>
        <location filename="Forms/OptionsWindow.ui" line="783"/>
        <location filename="Forms/OptionsWindow.ui" line="992"/>
        <location filename="Forms/OptionsWindow.ui" line="1037"/>
        <source>Protect the system from sandboxed processes</source>
        <translation>Защита системы от изолированных процессов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="995"/>
        <source>Elevation restrictions</source>
        <translation>Ограничения повышение уровня</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1080"/>
        <source>Drop rights from Administrators and Power Users groups</source>
        <translation>Удаление прав из групп администраторов и опытных пользователей</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="221"/>
        <source>px Width</source>
        <translation>px ширина</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1002"/>
        <source>Make applications think they are running elevated (allows to run installers safely)</source>
        <translation>Заставить приложения думать, что они работают с повышенными правами (позволяет безопасно запускать установщики)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1094"/>
        <source>CAUTION: When running under the built in administrator, processes can not drop administrative privileges.</source>
        <translation>ВНИМАНИЕ: При запуске под встроенным администратором процессы не могут терять административные привилегии.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="252"/>
        <source>Appearance</source>
        <translation>Внешний вид</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1016"/>
        <source>(Recommended)</source>
        <translation>(Рекомендуемые)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="172"/>
        <source>Show this box in the &apos;run in box&apos; selection prompt</source>
        <translation>Показывать эту песочницу в окне выбора &apos;Выполнить в песочнице&apos;</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="279"/>
        <source>File Options</source>
        <translation>Параметры файла</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="324"/>
        <source>Auto delete content when last sandboxed process terminates</source>
        <translation>Автоматическое удаление содержимого при завершении последнего изолированного процесса</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="453"/>
        <source>Copy file size limit:</source>
        <translation>Максимальный размер копируемого файла:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="410"/>
        <source>Box Delete options</source>
        <translation>Параметры удаления песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="294"/>
        <source>Protect this sandbox from deletion or emptying</source>
        <translation>Защитить эту песочницу от удаления или очистки</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="433"/>
        <location filename="Forms/OptionsWindow.ui" line="474"/>
        <source>File Migration</source>
        <translation>Перенос файлов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="301"/>
        <source>Allow elevated sandboxed applications to read the harddrive</source>
        <translation>Разрешить изолированным приложениям с повышенными правами читать жесткий диск</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="287"/>
        <source>Warn when an application opens a harddrive handle</source>
        <translation>Предупреждать, когда приложение открывает дескриптор жесткого диска</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="503"/>
        <source>kilobytes</source>
        <translation>килобайт</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="596"/>
        <source>Issue message 2102 when a file is too large</source>
        <translation>Сообщение о проблеме 2102, когда файл слишком большой</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="460"/>
        <source>Prompt user for large file migration</source>
        <translation>Запрашивать пользователя о переносе больших файлов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="688"/>
        <source>Allow the print spooler to print to files outside the sandbox</source>
        <translation>Разрешить диспетчеру печати печатать файлы вне песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="702"/>
        <source>Remove spooler restriction, printers can be installed outside the sandbox</source>
        <translation>Снять ограничение диспетчера печати, принтеры можно устанавливать вне песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="641"/>
        <source>Block read access to the clipboard</source>
        <translation>Заблокировать доступ на чтение буфера обмена</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="709"/>
        <source>Open System Protected Storage</source>
        <translation>Открыть системное защищенное хранилище</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="675"/>
        <source>Block access to the printer spooler</source>
        <translation>Заблокировать доступ к диспетчеру печати</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="733"/>
        <source>Other restrictions</source>
        <translation>Прочие ограничения</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="769"/>
        <source>Printing restrictions</source>
        <translation>Ограничения печати</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="786"/>
        <source>Network restrictions</source>
        <translation>Сетевые ограничения</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="695"/>
        <source>Block network files and folders, unless specifically opened.</source>
        <translation>Блокировать сетевые файлы и папки, если они специально не открываются.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="796"/>
        <source>Run Menu</source>
        <translation>Меню запуска</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="925"/>
        <source>You can configure custom entries for the sandbox run menu.</source>
        <translation>Вы можете настроить пользовательские записи для меню запуска песочницы.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="869"/>
        <location filename="Forms/OptionsWindow.ui" line="1615"/>
        <location filename="Forms/OptionsWindow.ui" line="1757"/>
        <location filename="Forms/OptionsWindow.ui" line="1843"/>
        <location filename="Forms/OptionsWindow.ui" line="1988"/>
        <location filename="Forms/OptionsWindow.ui" line="2033"/>
        <location filename="Forms/OptionsWindow.ui" line="2111"/>
        <location filename="Forms/OptionsWindow.ui" line="2963"/>
        <location filename="Forms/OptionsWindow.ui" line="3185"/>
        <location filename="Forms/OptionsWindow.ui" line="3301"/>
        <location filename="Forms/OptionsWindow.ui" line="4438"/>
        <location filename="Forms/OptionsWindow.ui" line="4514"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="879"/>
        <source>Command Line</source>
        <translation>Командная строка</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="845"/>
        <source>Add program</source>
        <translation>Добавить программу</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="826"/>
        <location filename="Forms/OptionsWindow.ui" line="1640"/>
        <location filename="Forms/OptionsWindow.ui" line="1784"/>
        <location filename="Forms/OptionsWindow.ui" line="1851"/>
        <location filename="Forms/OptionsWindow.ui" line="1963"/>
        <location filename="Forms/OptionsWindow.ui" line="2061"/>
        <location filename="Forms/OptionsWindow.ui" line="2139"/>
        <location filename="Forms/OptionsWindow.ui" line="2231"/>
        <location filename="Forms/OptionsWindow.ui" line="2365"/>
        <location filename="Forms/OptionsWindow.ui" line="2464"/>
        <location filename="Forms/OptionsWindow.ui" line="2545"/>
        <location filename="Forms/OptionsWindow.ui" line="2658"/>
        <location filename="Forms/OptionsWindow.ui" line="2932"/>
        <location filename="Forms/OptionsWindow.ui" line="3074"/>
        <location filename="Forms/OptionsWindow.ui" line="3223"/>
        <location filename="Forms/OptionsWindow.ui" line="3309"/>
        <location filename="Forms/OptionsWindow.ui" line="3603"/>
        <location filename="Forms/OptionsWindow.ui" line="3736"/>
        <location filename="Forms/OptionsWindow.ui" line="3805"/>
        <location filename="Forms/OptionsWindow.ui" line="3930"/>
        <location filename="Forms/OptionsWindow.ui" line="3989"/>
        <location filename="Forms/OptionsWindow.ui" line="4458"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1752"/>
        <location filename="Forms/OptionsWindow.ui" line="1838"/>
        <location filename="Forms/OptionsWindow.ui" line="2242"/>
        <location filename="Forms/OptionsWindow.ui" line="2342"/>
        <location filename="Forms/OptionsWindow.ui" line="2475"/>
        <location filename="Forms/OptionsWindow.ui" line="2595"/>
        <location filename="Forms/OptionsWindow.ui" line="2669"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1583"/>
        <source>Program Groups</source>
        <translation>Группы программ</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1591"/>
        <source>Add Group</source>
        <translation>Добавить группу</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1623"/>
        <location filename="Forms/OptionsWindow.ui" line="1977"/>
        <location filename="Forms/OptionsWindow.ui" line="2054"/>
        <location filename="Forms/OptionsWindow.ui" line="2132"/>
        <location filename="Forms/OptionsWindow.ui" line="2918"/>
        <source>Add Program</source>
        <translation>Добавить программу</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1734"/>
        <source>Force Folder</source>
        <translation>Принудительная папка</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2257"/>
        <location filename="Forms/OptionsWindow.ui" line="2357"/>
        <location filename="Forms/OptionsWindow.ui" line="2490"/>
        <source>Path</source>
        <translation>Путь</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1777"/>
        <source>Force Program</source>
        <translation>Принудительная программа</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="550"/>
        <location filename="Forms/OptionsWindow.ui" line="1647"/>
        <location filename="Forms/OptionsWindow.ui" line="1741"/>
        <location filename="Forms/OptionsWindow.ui" line="1877"/>
        <location filename="Forms/OptionsWindow.ui" line="1970"/>
        <location filename="Forms/OptionsWindow.ui" line="2068"/>
        <location filename="Forms/OptionsWindow.ui" line="2184"/>
        <location filename="Forms/OptionsWindow.ui" line="2224"/>
        <location filename="Forms/OptionsWindow.ui" line="2331"/>
        <location filename="Forms/OptionsWindow.ui" line="2457"/>
        <location filename="Forms/OptionsWindow.ui" line="2552"/>
        <location filename="Forms/OptionsWindow.ui" line="2692"/>
        <location filename="Forms/OptionsWindow.ui" line="3120"/>
        <location filename="Forms/OptionsWindow.ui" line="3216"/>
        <location filename="Forms/OptionsWindow.ui" line="3323"/>
        <location filename="Forms/OptionsWindow.ui" line="3562"/>
        <location filename="Forms/OptionsWindow.ui" line="3753"/>
        <location filename="Forms/OptionsWindow.ui" line="3760"/>
        <location filename="Forms/OptionsWindow.ui" line="4025"/>
        <source>Show Templates</source>
        <translation>Показать шаблоны</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1121"/>
        <source>Security note: Elevated applications running under the supervision of Sandboxie, with an admin or system token, have more opportunities to bypass isolation and modify the system outside the sandbox.</source>
        <translation>Примечание по безопасности: расширенные приложения, работающие под контролем Sandboxie, с токеном администратора или системным токеном, имеют больше возможностей для обхода изоляции и изменения системы вне песочницы.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1047"/>
        <source>Allow MSIServer to run with a sandboxed system token and apply other exceptions if required</source>
        <translation>Разрешить MSIServer работать с изолированным системным токеном и при необходимости применить другие исключения</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1104"/>
        <source>Note: Msi Installer Exemptions should not be required, but if you encounter issues installing a msi package which you trust, this option may help the installation complete successfully. You can also try disabling drop admin rights.</source>
        <translation>Примечание: Исключения для установщика Msi не требуются, но если вы столкнетесь с проблемами при установке пакета msi, которому вы доверяете, этот параметр может помочь успешно завершить установку. Вы также можете попробовать отключить сброс прав администратора.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="238"/>
        <source>General Configuration</source>
        <translation>Общая конфигурация</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="211"/>
        <source>Box Type Preset:</source>
        <translation>Предустановка типа песочницы:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="179"/>
        <source>Box info</source>
        <translation>Информация о песочнице</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="142"/>
        <source>&lt;b&gt;More Box Types&lt;/b&gt; are exclusively available to &lt;u&gt;project supporters&lt;/u&gt;, the Privacy Enhanced boxes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;protect user data from illicit access&lt;/font&gt;&lt;/b&gt; by the sandboxed programs.&lt;br /&gt;If you are not yet a supporter, then please consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt;, to receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;.&lt;br /&gt;You can test the other box types by creating new sandboxes of those types, however processes in these will be auto terminated after 5 minutes.</source>
        <translation>&lt;b&gt;Больше типов песочниц&lt;/b&gt; доступны исключительно для &lt;u&gt;сторонников проекта&lt;/u&gt;, песочницы с улучшенной конфиденциальностью &lt;b&gt;&lt;font color=&apos;red&apos;&gt;защищают данные пользователей от несанкционированного доступа&lt;/font&gt;&lt;/b&gt; программ в песочнице.&lt;br /&gt;Если вы еще не являетесь сторонником, то рассмотрите &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;возможность поддержки проекта&lt;/a&gt;, для получения &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;сертификата сторонника&lt;/a&gt;.&lt;br /&gt;Вы можете протестировать другие типы песочниц, создав новые песочницы этих типов, однако процессы в них будут автоматически завершены через 5 минут.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="338"/>
        <source>Use volume serial numbers for drives, like: \drive\C~1234-ABCD</source>
        <translation>Использовать серийные номера томов для дисков, например: \drive\C~1234-ABCD</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="382"/>
        <source>The box structure can only be changed when the sandbox is empty</source>
        <translation>Структуру песочницы можно изменить только тогда, когда она пуста</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="352"/>
        <source>Disk/File access</source>
        <translation>Доступ к диску/файлу</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="331"/>
        <source>Virtualization scheme</source>
        <translation>Схема виртуализации</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="481"/>
        <source>2113: Content of migrated file was discarded
2114: File was not migrated, write access to file was denied
2115: File was not migrated, file will be opened read only</source>
        <translation>2113: Содержимое перенесенного файла было удалено.
2114: Файл не был перенесен, доступ на запись в файл запрещен.
2115: Файл не был перенесен, файл будет открыт только для чтения</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="486"/>
        <source>Issue message 2113/2114/2115 when a file is not fully migrated</source>
        <translation>Сообщение о проблеме 2113/2114/2115, когда файл не полностью перенесен</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="510"/>
        <source>Add Pattern</source>
        <translation>Добавить шаблон</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="517"/>
        <source>Remove Pattern</source>
        <translation>Удалить шаблон</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="571"/>
        <source>Pattern</source>
        <translation>Шаблон</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="579"/>
        <source>Sandboxie does not allow writing to host files, unless permitted by the user. When a sandboxed application attempts to modify a file, the entire file must be copied into the sandbox, for large files this can take a significate amount of time. Sandboxie offers options for handling these cases, which can be configured on this page.</source>
        <translation>Sandboxie не позволяет записывать файлы хоста, если это не разрешено пользователем. Когда приложение в песочнице пытается изменить файл, весь файл должен быть скопирован в песочницу, для больших файлов это может занять значительное время. Sandboxie предлагает варианты обработки таких случаев, которые можно настроить на этой странице.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="589"/>
        <source>Using wildcard patterns file specific behavior can be configured in the list below:</source>
        <translation>Использование файлов с шаблонами подстановочных знаков может быть настроено в списке ниже:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="603"/>
        <source>When a file cannot be migrated, open it in read-only mode instead</source>
        <translation>Если файл невозможно перенести, вместо этого откройте его в режиме только для чтения</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="621"/>
        <source>Open Windows Credentials Store (user mode)</source>
        <translation>Открыть хранилище учетных данных Windows (пользовательский режим)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="661"/>
        <source>Prevent change to network and firewall parameters (user mode)</source>
        <translation>Запретить изменение параметров сети и брандмауэра (пользовательский режим)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="874"/>
        <source>Icon</source>
        <translation>Иконка</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="899"/>
        <source>Move Up</source>
        <translation>Сдвинуть вверх</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="918"/>
        <source>Move Down</source>
        <translation>Сдвинуть вниз</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1153"/>
        <source>Security Isolation</source>
        <translation>Изоляция безопасности</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1174"/>
        <source>Various isolation features can break compatibility with some applications. If you are using this sandbox &lt;b&gt;NOT for Security&lt;/b&gt; but for application portability, by changing these options you can restore compatibility by sacrificing some security.</source>
        <translation>Различные функции изоляции могут нарушить совместимость с некоторыми приложениями. Если вы используете эту песочницу &lt;b&gt;НЕ для безопасности&lt;/b&gt;, а для переносимости приложений, изменив эти параметры, вы можете восстановить совместимость, пожертвовав некоторой безопасностью.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1272"/>
        <source>Access Isolation</source>
        <translation>Изоляция доступа</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1299"/>
        <location filename="Forms/OptionsWindow.ui" line="1357"/>
        <source>Image Protection</source>
        <translation>Защита изображения</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1307"/>
        <source>Issue message 1305 when a program tries to load a sandboxed dll</source>
        <translation>Выдать сообщение 1305, когда программа пытается загрузить изолированную dll</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1340"/>
        <source>Prevent sandboxes programs installed on host from loading dll&apos;s from the sandbox</source>
        <translation>Запретить программам в песочнице, установленным на хосте, загружать библиотеки dll из песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1383"/>
        <source>Sandboxie’s resource access rules often discriminate against program binaries located inside the sandbox. OpenFilePath and OpenKeyPath work only for application binaries located on the host natively. In order to define a rule without this restriction, OpenPipePath or OpenConfPath must be used. Likewise, all Closed(File|Key|Ipc)Path directives which are defined by negation e.g. ‘ClosedFilePath=! iexplore.exe,C:Users*’ will be always closed for binaries located inside a sandbox. Both restriction policies can be disabled on the “Access policies” page.
This is done to prevent rogue processes inside the sandbox from creating a renamed copy of themselves and accessing protected resources. Another exploit vector is the injection of a library into an authorized process to get access to everything it is allowed to access. Using Host Image Protection, this can be prevented by blocking applications (installed on the host) running inside a sandbox from loading libraries from the sandbox itself.</source>
        <translation>Правила доступа к ресурсам Sandboxie часто дискриминируют двоичные файлы программ, расположенные внутри песочницы. OpenFilePath и OpenKeyPath работают только для двоичных файлов приложений, изначально расположенных на хосте. Чтобы определить правило без этого ограничения, необходимо использовать OpenPipePath или OpenConfPath. Аналогично, все директивы Closed(File|Key|Ipc)Path, которые определены отрицанием, например. ‘ClosedFilePath=! iexplore.exe,C:Users*’ всегда будет закрыт для двоичных файлов, находящихся внутри песочницы. Обе политики ограничения можно отключить на странице &quot;Политики доступа&quot;.
Это делается для того, чтобы мошеннические процессы внутри песочницы не создавали переименованные копии самих себя и не получали доступ к защищенным ресурсам. Другой вектор эксплойта — внедрение библиотеки в авторизованный процесс, чтобы получить доступ ко всему, к чему ему разрешен доступ. С помощью Host Image Protection это можно предотвратить, заблокировав приложения (установленные на хосте), работающие внутри песочницы, от загрузки библиотек из самой песочницы.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1404"/>
        <source>Advanced Security</source>
        <translation>Расширенная безопасность</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1412"/>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation>Использовать логин Sandboxie вместо анонимного токена (экспериментально)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1429"/>
        <source>Other isolation</source>
        <translation>Другая изоляция</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1453"/>
        <source>Privilege isolation</source>
        <translation>Изоляция привилегий</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1477"/>
        <source>Sandboxie token</source>
        <translation>Токен Sandboxie</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1566"/>
        <source>Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.</source>
        <translation>Использование пользовательского токена Sandboxie позволяет лучше изолировать отдельные песочницы друг от друга, а также показывает в пользовательском столбце диспетчеров задач имя песочницы, к которой принадлежит процесс. Однако у некоторых сторонних решений безопасности могут быть проблемы с пользовательскими токенами.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1630"/>
        <source>You can group programs together and give them a group name.  Program groups can be used with some of the settings instead of program names. Groups defined for the box overwrite groups defined in templates.</source>
        <translation>Вы можете сгруппировать программы и дать группе название. Группы программ могут использоваться с некоторыми настройками вместо названий программ. Группы, определенные для песочницы, перезаписывают группы, определенные в шаблонах.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1657"/>
        <source>Program Control</source>
        <translation>Контроль программ</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1679"/>
        <source>Force Programs</source>
        <translation>Принудительные программы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1699"/>
        <source>Programs entered here, or programs started from entered locations, will be put in this sandbox automatically, unless they are explicitly started in another sandbox.</source>
        <translation>Введенные здесь программы, или программы запущенные из указанных мест, будут автоматически помещены в эту песочницу, если они явно не запущены в другой песочнице.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1791"/>
        <source>Disable forced Process and Folder for this sandbox</source>
        <translation>Отключить принудительный процесс и папку для этой песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1801"/>
        <source>Breakout Programs</source>
        <translation>Программы вне песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1870"/>
        <source>Breakout Program</source>
        <translation>Программа вне песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1896"/>
        <source>Breakout Folder</source>
        <translation>Папка вне песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1903"/>
        <source>Programs entered here will be allowed to break out of this sandbox when they start. It is also possible to capture them into another sandbox, for example to have your web browser always open in a dedicated box.</source>
        <translation>Программам, указанным здесь, будет разрешено выйти из этой песочницы при запуске. Также можно захватить их в другую песочницу, например, чтобы ваш веб-браузер всегда был открыт в определенной песочнице.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1920"/>
        <source>Stop Behaviour</source>
        <translation>Поведение остановки</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2092"/>
        <source>Start Restrictions</source>
        <translation>Ограничения на запуск</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2100"/>
        <source>Issue message 1308 when a program fails to start</source>
        <translation>Сообщение о проблеме 1308, когда программа не запускается</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2151"/>
        <source>Allow only selected programs to start in this sandbox. *</source>
        <translation>Разрешить запуск только выбранных программ в этой песочнице. *</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2158"/>
        <source>Prevent selected programs from starting in this sandbox.</source>
        <translation>Запретить запуск выбранных программ в этой песочнице.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2165"/>
        <source>Allow all programs to start in this sandbox.</source>
        <translation>Разрешить запуск всех программ в этой песочнице.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2172"/>
        <source>* Note: Programs installed to this sandbox won&apos;t be able to start at all.</source>
        <translation>* Примечание: Программы, установленные в этой песочнице, вообще не запустятся.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2891"/>
        <source>Process Restrictions</source>
        <translation>Ограничения процесса</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2911"/>
        <source>Issue message 1307 when a program is denied internet access</source>
        <translation>Сообщение о проблеме 1307, когда программе запрещен доступ в Интернет</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2925"/>
        <source>Prompt user whether to allow an exemption from the blockade.</source>
        <translation>Подсказка пользователю, разрешить ли освобождение от блокировки.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2939"/>
        <source>Note: Programs installed to this sandbox won&apos;t be able to access the internet at all.</source>
        <translation>Примечание: Программы, установленные в этой песочнице, вообще не смогут получить доступ к Интернету.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2252"/>
        <location filename="Forms/OptionsWindow.ui" line="2352"/>
        <location filename="Forms/OptionsWindow.ui" line="2485"/>
        <location filename="Forms/OptionsWindow.ui" line="2605"/>
        <location filename="Forms/OptionsWindow.ui" line="2679"/>
        <location filename="Forms/OptionsWindow.ui" line="2968"/>
        <source>Access</source>
        <translation>Доступ</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="613"/>
        <source>Restrictions</source>
        <translation>Ограничения</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1930"/>
        <source>Lingering Programs</source>
        <translation>Вторичные программы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1996"/>
        <source>Lingering programs will be automatically terminated if they are still running after all other processes have been terminated.</source>
        <translation>Вторичные программы будут автоматически завершены, если они все еще работают после завершения всех других процессов.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2009"/>
        <source>Leader Programs</source>
        <translation>Первичные программы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2075"/>
        <source>If leader processes are defined, all others are treated as lingering processes.</source>
        <translation>Если первичные процессы определены, все остальные рассматриваются как вторичные процессы.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2204"/>
        <source>Files</source>
        <translation>Файлы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2297"/>
        <source>Configure which processes can access Files, Folders and Pipes. 
&apos;Open&apos; access only applies to program binaries located outside the sandbox, you can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behavior in the Policies tab.</source>
        <translation>Настройте, какие процессы могут получить доступ к файлам, папкам и каналам. 
&apos;Открытый&apos; доступ применяется только к двоичным файлам программы, расположенным за пределами песочницы. Вместо этого вы можете использовать &apos;Открытый для всех&apos;, чтобы применить его ко всем программам, или изменить это поведение на вкладке политик.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2311"/>
        <source>Registry</source>
        <translation>Реестр</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2404"/>
        <source>Configure which processes can access the Registry. 
&apos;Open&apos; access only applies to program binaries located outside the sandbox, you can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behavior in the Policies tab.</source>
        <translation>Настройте, какие процессы могут получить доступ к реестру. 
&apos;Открытый&apos; доступ применяется только к двоичным файлам программы, расположенным за пределами песочницы. Вместо этого вы можете использовать &apos;Открытый для всех&apos;, чтобы применить его ко всем программам, или изменить это поведение на вкладке политик.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2418"/>
        <source>IPC</source>
        <translation>IPC</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2511"/>
        <source>Configure which processes can access NT IPC objects like ALPC ports and other processes memory and context.
To specify a process use &apos;$:program.exe&apos; as path.</source>
        <translation>Настройте, какие процессы могут получить доступ к объектам NT IPC, таким как порты ALPC и другие процессы, память и контекст. Чтобы указать процесс, используйте  &apos;$:program.exe&apos; в качестве пути.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2525"/>
        <source>Wnd</source>
        <translation>Wnd</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2610"/>
        <source>Wnd Class</source>
        <translation>Wnd класс</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2625"/>
        <source>Configure which processes can access desktop objects like windows and alike.</source>
        <translation>Настройте, какие процессы могут получить доступ к объектам рабочего стола, таким как окна и т.п.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2638"/>
        <source>COM</source>
        <translation>COM</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2684"/>
        <source>Class Id</source>
        <translation>Id класса</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2731"/>
        <source>Configure which processes can access COM objects.</source>
        <translation>Настройте, какие процессы могут получить доступ к COM-объектам.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2743"/>
        <source>Don&apos;t use virtualized COM, Open access to hosts COM infrastructure (not recommended)</source>
        <translation>Не использовать виртуализированный COM, открыть доступ к инфраструктуре COM хостов (не рекомендуется)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2751"/>
        <source>Access Policies</source>
        <translation>Политики доступа</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2785"/>
        <source>Apply Close...=!&lt;program&gt;,... rules also to all binaries located in the sandbox.</source>
        <translation>Применить правила Close...=!&lt;program&gt;,... также ко всем двоичным файлам, находящимся в песочнице.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2881"/>
        <source>Network Options</source>
        <translation>Параметры сети</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2978"/>
        <source>Set network/internet access for unlisted processes:</source>
        <translation>Настроить доступ к сети/Интернету для неуказанных процессов:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3025"/>
        <source>Test Rules, Program:</source>
        <translation>Правила тестирования, программа:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3035"/>
        <source>Port:</source>
        <translation>Порт:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3045"/>
        <source>IP:</source>
        <translation>IP:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3055"/>
        <source>Protocol:</source>
        <translation>Протокол:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3065"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3081"/>
        <source>Add Rule</source>
        <translation>Добавить правило</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="566"/>
        <location filename="Forms/OptionsWindow.ui" line="2247"/>
        <location filename="Forms/OptionsWindow.ui" line="2347"/>
        <location filename="Forms/OptionsWindow.ui" line="2480"/>
        <location filename="Forms/OptionsWindow.ui" line="2600"/>
        <location filename="Forms/OptionsWindow.ui" line="2674"/>
        <location filename="Forms/OptionsWindow.ui" line="3092"/>
        <location filename="Forms/OptionsWindow.ui" line="4012"/>
        <source>Program</source>
        <translation>Программа</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="561"/>
        <location filename="Forms/OptionsWindow.ui" line="3097"/>
        <location filename="Forms/OptionsWindow.ui" line="3536"/>
        <location filename="Forms/OptionsWindow.ui" line="3861"/>
        <source>Action</source>
        <translation>Действие</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3102"/>
        <source>Port</source>
        <translation>Порт</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3107"/>
        <source>IP</source>
        <translation>IP</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3112"/>
        <source>Protocol</source>
        <translation>Протокол</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3134"/>
        <source>CAUTION: Windows Filtering Platform is not enabled with the driver, therefore these rules will be applied only in user mode and can not be enforced!!! This means that malicious applications may bypass them.</source>
        <translation>ВНИМАНИЕ: Платформа фильтрации Windows не включена с драйвером, поэтому эти правила будут применяться только в пользовательском режиме и не могут быть применены!!! Это означает, что вредоносные приложения могут их обойти.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2194"/>
        <source>Resource Access</source>
        <translation>Доступ к ресурсам</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2277"/>
        <source>Add File/Folder</source>
        <translation>Добавить файл/папку</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2571"/>
        <source>Add Wnd Class</source>
        <translation>Добавить Wnd класс</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2450"/>
        <source>Add IPC Path</source>
        <translation>Добавит путь IPC</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2384"/>
        <source>Add Reg Key</source>
        <translation>Добавить ключ реестра</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2711"/>
        <source>Add COM Object</source>
        <translation>Добавить COM объект</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3151"/>
        <source>File Recovery</source>
        <translation>Восстановление файлов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3161"/>
        <source>Quick Recovery</source>
        <translation>Быстрое восстановление</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3230"/>
        <source>Add Folder</source>
        <translation>Добавить папку</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3240"/>
        <source>Immediate Recovery</source>
        <translation>Немедленное восстановление</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3270"/>
        <source>Ignore Extension</source>
        <translation>Игнорировать расширение</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3316"/>
        <source>Ignore Folder</source>
        <translation>Игнорировать папку</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3277"/>
        <source>Enable Immediate Recovery prompt to be able to recover files as soon as they are created.</source>
        <translation>Включить запрос немедленного восстановления, чтобы иметь возможность восстанавливать файлы сразу после их создания.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3260"/>
        <source>You can exclude folders and file types (or file extensions) from Immediate Recovery.</source>
        <translation>Вы можете исключить папки и типы файлов (или расширения файлов) из немедленного восстановления.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3193"/>
        <source>When the Quick Recovery function is invoked, the following folders will be checked for sandboxed content. </source>
        <translation>При вызове функции быстрого восстановления следующие папки будут проверяться на наличие изолированного содержимого. </translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3358"/>
        <source>Advanced Options</source>
        <translation>Расширенные настройки</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3950"/>
        <source>Miscellaneous</source>
        <translation>Разное</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2618"/>
        <source>Don&apos;t alter window class names created by sandboxed programs</source>
        <translation>Не изменять имена классов окон, созданные изолированными программами</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1505"/>
        <source>Do not start sandboxed services using a system token (recommended)</source>
        <translation>Не запускать изолированные службы с использованием системного токена (рекомендуется)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1252"/>
        <location filename="Forms/OptionsWindow.ui" line="1269"/>
        <location filename="Forms/OptionsWindow.ui" line="1354"/>
        <location filename="Forms/OptionsWindow.ui" line="1426"/>
        <location filename="Forms/OptionsWindow.ui" line="1450"/>
        <location filename="Forms/OptionsWindow.ui" line="1474"/>
        <source>Protect the sandbox integrity itself</source>
        <translation>Защитить целостность самой песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1498"/>
        <source>Drop critical privileges from processes running with a SYSTEM token</source>
        <translation>Отбросить критические привилегии от процессов, работающих с токеном SYSTEM</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1491"/>
        <location filename="Forms/OptionsWindow.ui" line="1539"/>
        <source>(Security Critical)</source>
        <translation>(Критично для безопасности)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1460"/>
        <source>Protect sandboxed SYSTEM processes from unprivileged processes</source>
        <translation>Защитить изолированные процессы SYSTEM от непривилегированных процессов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3403"/>
        <source>Force usage of custom dummy Manifest files (legacy behaviour)</source>
        <translation>Принудительное использование пользовательских фиктивных файлов манифеста (устаревшее поведение)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2833"/>
        <source>The rule specificity is a measure to how well a given rule matches a particular path, simply put the specificity is the length of characters from the begin of the path up to and including the last matching non-wildcard substring. A rule which matches only file types like &quot;*.tmp&quot; would have the highest specificity as it would always match the entire file path.
The process match level has a higher priority than the specificity and describes how a rule applies to a given process. Rules applying by process name or group have the strongest match level, followed by the match by negation (i.e. rules applying to all processes but the given one), while the lowest match levels have global matches, i.e. rules that apply to any process.</source>
        <translation>Специфика правила - это мера того, насколько хорошо данное правило соответствует определенному пути, проще говоря, специфичность - это длина символов от начала пути до последней совпадающей подстроки без подстановочных знаков включительно. Правило, которое соответствует только таким типам файлов, как &quot;*.tmp&quot; будет иметь наивысшую специфичность, так как всегда будет соответствовать всему пути к файлу.
Уровень соответствия процесса имеет более высокий приоритет, чем специфичность, и описывает, как правило применяется к данному процессу. Правила, применяемые по имени процесса или группе, имеют самый строгий уровень соответствия, за которым следует соответствие по отрицанию (т.е. правила, применяемые ко всем процессам, кроме данного), в то время как самые низкие уровни соответствия имеют глобальные совпадения, то есть правила, которые применяются к любому процессу.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2792"/>
        <source>Prioritize rules based on their Specificity and Process Match Level</source>
        <translation>Приоритет правил на основе их специфики и уровня соответствия процесса</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2844"/>
        <source>Privacy Mode, block file and registry access to all locations except the generic system ones</source>
        <translation>Режим конфиденциальности, блокировка доступа к файлам и реестру для всех мест, кроме общих системных</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2826"/>
        <source>Access Mode</source>
        <translation>Режим доступа</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2864"/>
        <source>When the Privacy Mode is enabled, sandboxed processes will be only able to read C:\Windows\*, C:\Program Files\*, and parts of the HKLM registry, all other locations will need explicit access to be readable and/or writable. In this mode, Rule Specificity is always enabled.</source>
        <translation>Когда включен режим конфиденциальности, изолированные процессы смогут читать только C:\Windows\*, C:\Program Files\* и части реестра HKLM, для всех остальных мест потребуется явный доступ, для чтения и/или записи. В этом режиме всегда включена специфика правила.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2778"/>
        <source>Rule Policies</source>
        <translation>Правила политик</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2799"/>
        <source>Apply File and Key Open directives only to binaries located outside the sandbox.</source>
        <translation>Применить директивы открытия файлов и ключей только к двоичным файлам, расположенным вне песочницы.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1436"/>
        <source>Start the sandboxed RpcSs as a SYSTEM process (not recommended)</source>
        <translation>Запускать изолированный RpcSs как СИСТЕМНЫЙ процесс (не рекомендуется)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1525"/>
        <source>Allow only privileged processes to access the Service Control Manager</source>
        <translation>Разрешить доступ к диспетчеру управления службами только привилегированным процессам</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3375"/>
        <location filename="Forms/OptionsWindow.ui" line="3424"/>
        <source>Compatibility</source>
        <translation>Совместимость</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1559"/>
        <source>Add sandboxed processes to job objects (recommended)</source>
        <translation>Добавить изолированные процессы к объектам задания (рекомендуется)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3410"/>
        <source>Emulate sandboxed window station for all processes</source>
        <translation>Эмуляция оконной станции в песочнице для всех процессов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3431"/>
        <source>Allow use of nested job objects (works on Windows 8 and later)</source>
        <translation>Разрешить использование вложенных объектов заданий (работает в Windows 8 и новее)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1228"/>
        <source>Security Isolation through the usage of a heavily restricted process token is Sandboxie&apos;s primary means of enforcing sandbox restrictions, when this is disabled the box is operated in the application compartment mode, i.e. it’s no longer providing reliable security, just simple application compartmentalization.</source>
        <translation>Изоляция безопасности за счет использования сильно ограниченного токена процесса - это основное средство Sandboxie для принудительного применения ограничений песочницы, когда она отключена, песочница работает в режиме контейнера для приложения, то есть она больше не обеспечивает надежную безопасность, а только простое разделение приложений.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1279"/>
        <source>Allow sandboxed programs to manage Hardware/Devices</source>
        <translation>Разрешить изолированным программам управлять оборудованием/устройствами</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1197"/>
        <source>Disable Security Isolation (experimental)</source>
        <translation>Отключить изоляцию безопасности (экспериментально)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1204"/>
        <source>Open access to Windows Security Account Manager</source>
        <translation>Открытый доступ к диспетчеру учетных записей безопасности Windows</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1238"/>
        <source>Open access to Windows Local Security Authority</source>
        <translation>Открыть доступ к серверу проверки подлинности локальной системы безопасности</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3451"/>
        <source>Disable the use of RpcMgmtSetComTimeout by default (this may resolve compatibility issues)</source>
        <translation>Отключить использование RpcMgmtSetComTimeout по умолчанию (это может решить проблемы совместимости)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1255"/>
        <source>Security Isolation &amp; Filtering</source>
        <translation>Изоляция безопасности &amp; Фильтрация</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1211"/>
        <source>Disable Security Filtering (not recommended)</source>
        <translation>Отключить фильтрацию безопасности (не рекомендуется)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1218"/>
        <source>Security Filtering used by Sandboxie to enforce filesystem and registry access restrictions, as well as to restrict process access.</source>
        <translation>Фильтрация безопасности, используется Sandboxie для наложения ограничений на доступ к файловой системе и реестру, а также для ограничения доступа к процессам.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1286"/>
        <source>The below options can be used safely when you don&apos;t grant admin rights.</source>
        <translation>Приведенные ниже параметры можно безопасно использовать, если вы не предоставляете прав администратора.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3718"/>
        <source>Hide Processes</source>
        <translation>Скрыть процессы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3767"/>
        <location filename="Forms/OptionsWindow.ui" line="3825"/>
        <source>Add Process</source>
        <translation>Добавить процесс</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3743"/>
        <source>Hide host processes from processes running in the sandbox.</source>
        <translation>Скрыть хост-процессы от процессов, запущенных в песочнице.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3774"/>
        <source>Don&apos;t allow sandboxed processes to see processes running in other boxes</source>
        <translation>Не позволять изолированным процессам видеть процессы, запущенные в других песочницах</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3882"/>
        <source>Users</source>
        <translation>Пользователи</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3900"/>
        <source>Restrict Resource Access monitor to administrators only</source>
        <translation>Ограничить мониторинг доступа к ресурсам только администраторам</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3907"/>
        <source>Add User</source>
        <translation>Добавить пользователя</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3937"/>
        <source>Add user accounts and user groups to the list below to limit use of the sandbox to only those accounts.  If the list is empty, the sandbox can be used by all user accounts.

Note:  Forced Programs and Force Folders settings for a sandbox do not apply to user accounts which cannot use the sandbox.</source>
        <translation>Добавьте учетные записи пользователей и группы пользователей в список ниже, чтобы ограничить использование песочницы только этими учетными записями. Если список пуст, песочница может использоваться всеми учетными записями пользователей.

Примечание. Параметры принудительных программ и принудительных папок для песочницы не применяются к учетным записям пользователей, которые не могут использовать эту песочницу.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4048"/>
        <source>Tracing</source>
        <translation>Трассировка</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4056"/>
        <source>API call trace (requires LogAPI to be installed in the Sbie directory)</source>
        <translation>Трассировка вызовов API (требуется, чтобы LogAPI был установлен в каталоге Sbie)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4076"/>
        <source>Pipe Trace</source>
        <translation>Трассировка pipe</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4096"/>
        <source>Log all SetError&apos;s to Trace log (creates a lot of output)</source>
        <translation>Записывать все SetError в журнал трассировки (создает много выходных данных)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4116"/>
        <source>Log Debug Output to the Trace Log</source>
        <translation>Записывать вывод отладки в журнал трассировки</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="89"/>
        <source>Always show this sandbox in the systray list (Pinned)</source>
        <translation>Всегда показывать эту песочницу в системном лотке (закреплено)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1040"/>
        <source>Security enhancements</source>
        <translation>Улучшения безопасности</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="971"/>
        <source>Use the original token only for approved NT system calls</source>
        <translation>Использовать исходный токен только для разрешенных системных вызовов NT</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1023"/>
        <source>Restrict driver/device access to only approved ones</source>
        <translation>Ограничить доступ к драйверу/устройству только утвержденными</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="978"/>
        <source>Enable all security enhancements (make security hardened box)</source>
        <translation>Включить все улучшения безопасности (сделать защищенную песочницу)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="668"/>
        <source>Allow to read memory of unsandboxed processes (not recommended)</source>
        <translation>Разрешить чтение памяти процессов вне песочницы (не рекомендуется)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="716"/>
        <source>Issue message 2111 when a process access is denied</source>
        <translation>Выдать сообщение 2111, когда доступ к процессу запрещен</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3468"/>
        <source>Triggers</source>
        <translation>Триггеры</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3531"/>
        <source>Event</source>
        <translation>Событие</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3501"/>
        <location filename="Forms/OptionsWindow.ui" line="3520"/>
        <location filename="Forms/OptionsWindow.ui" line="3658"/>
        <source>Run Command</source>
        <translation>Выполнить комманду</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3690"/>
        <source>Start Service</source>
        <translation>Запустить службу</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3665"/>
        <source>These events are executed each time a box is started</source>
        <translation>Эти события выполняются каждый раз при запуске песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3668"/>
        <source>On Box Start</source>
        <translation>При запуске песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3549"/>
        <location filename="Forms/OptionsWindow.ui" line="3697"/>
        <source>These commands are run UNBOXED just before the box content is deleted</source>
        <translation>Эти команды запускаются вне песочницы непосредственно перед удалением содержимого песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3396"/>
        <source>Apply ElevateCreateProcess Workaround (legacy behaviour)</source>
        <translation>Применение обходного пути ElevateCreateProcess (устаревшее поведение)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3458"/>
        <source>Use desktop object workaround for all processes</source>
        <translation>Использовать обходной путь для объектов рабочего стола для всех процессов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3623"/>
        <source>These commands are executed only when a box is initialized. To make them run again, the box content must be deleted.</source>
        <translation>Эти команды выполняются только при инициализации песочницы. Чтобы они снова запустились, содержимое песочницы должно быть удалено.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3626"/>
        <source>On Box Init</source>
        <translation>При инициализации песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3636"/>
        <source>Here you can specify actions to be executed automatically on various box events.</source>
        <translation>Здесь вы можете указать действия, которые будут выполняться автоматически при различных событиях песочницы.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3700"/>
        <source>On Delete Content</source>
        <translation>При удалении контента</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3781"/>
        <source>Protect processes in this box from being accessed by specified unsandboxed host processes.</source>
        <translation>Защитить процессы в этой песочнице от доступа указанными неизолированным хост-процессами.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3792"/>
        <location filename="Forms/OptionsWindow.ui" line="3856"/>
        <source>Process</source>
        <translation>Процесс</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3832"/>
        <source>Block also read access to processes in this sandbox</source>
        <translation>Блокировать также доступ для чтения к процессам в этой песочнице</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3982"/>
        <source>Add Option</source>
        <translation>Добавить опцию</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3996"/>
        <source>Here you can configure advanced per process options to improve compatibility and/or customize sandboxing behavior.</source>
        <translation>Здесь вы можете настроить расширенные параметры для каждого процесса, чтобы улучшить совместимость и/или настроить поведение песочницы.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4007"/>
        <source>Option</source>
        <translation>Опция</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4123"/>
        <source>Log all access events as seen by the driver to the resource access log.

This options set the event mask to &quot;*&quot; - All access events
You can customize the logging using the ini by specifying
&quot;A&quot; - Allowed accesses
&quot;D&quot; - Denied accesses
&quot;I&quot; - Ignore access requests
instead of &quot;*&quot;.</source>
        <translation>Записывайте все события доступа, видимые драйвером, в журнал доступа к ресурсам.

Эти параметры устанавливают маску события на &quot;*&quot; - Все события доступа
Вы можете настроить ведение журнала с помощью ini, указав
&quot;A&quot; - Разрешенный доступ
&quot;D&quot; - Запрещенный доступ
&quot;I&quot; - Игнорированные запросы доступа
вместо &quot;*&quot;.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4143"/>
        <source>Ntdll syscall Trace (creates a lot of output)</source>
        <translation>Трассировка системных вызовов Ntdll (создает много выходных данных)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4150"/>
        <source>File Trace</source>
        <translation>Трассировка файлов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4157"/>
        <source>Disable Resource Access Monitor</source>
        <translation>Отключить монитор доступа к ресурсам</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4164"/>
        <source>IPC Trace</source>
        <translation>Трассировка IPC</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4171"/>
        <source>GUI Trace</source>
        <translation>Трассировка GUI</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4185"/>
        <source>Resource Access Monitor</source>
        <translation>Монитор доступа к ресурсам</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4212"/>
        <source>Access Tracing</source>
        <translation>Отслеживание доступа</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4219"/>
        <source>COM Class Trace</source>
        <translation>Трассировка COM класса</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4226"/>
        <source>Key Trace</source>
        <translation>Трассировка ключей</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2993"/>
        <location filename="Forms/OptionsWindow.ui" line="4233"/>
        <source>Network Firewall</source>
        <translation>Сетевой брандмауэр</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4250"/>
        <source>Debug</source>
        <translation>Отладка</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4306"/>
        <source>WARNING, these options can disable core security guarantees and break sandbox security!!!</source>
        <translation>ВНИМАНИЕ, эти параметры могут отключить основные гарантии безопасности и нарушить безопасность песочницы!!!</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4316"/>
        <source>These options are intended for debugging compatibility issues, please do not use them in production use. </source>
        <translation>Эти параметры предназначены для устранения проблем совместимости, не используйте их в продакшен среде. </translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4331"/>
        <source>App Templates</source>
        <translation>Шаблоны приложений</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4416"/>
        <source>Filter Categories</source>
        <translation>Категории фильтров</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4387"/>
        <source>Text Filter</source>
        <translation>Текстовый фильтр</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4409"/>
        <source>Add Template</source>
        <translation>Добавить шаблон</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4364"/>
        <source>This list contains a large amount of sandbox compatibility enhancing templates</source>
        <translation>Этот список содержит большое количество шаблонов для улучшения совместимости песочницы</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4433"/>
        <source>Category</source>
        <translation>Категория</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4468"/>
        <source>Template Folders</source>
        <translation>Папки шаблонов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4488"/>
        <source>Configure the folder locations used by your other applications.

Please note that this values are currently user specific and saved globally for all boxes.</source>
        <translation>Настройте расположение папок, используемых другими вашими приложениями.

Обратите внимание, что эти значения в настоящее время специфичны для пользователя и сохраняются глобально для всех песочниц.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4017"/>
        <location filename="Forms/OptionsWindow.ui" line="4519"/>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4530"/>
        <source>Accessibility</source>
        <translation>Доступность</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4626"/>
        <source>To compensate for the lost protection, please consult the Drop Rights settings page in the Restrictions settings group.</source>
        <translation>Чтобы компенсировать потерю защиты, обратитесь к странице настроек Сброс прав в группе настроек Ограничения.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4550"/>
        <source>Screen Readers: JAWS, NVDA, Window-Eyes, System Access</source>
        <translation>Чтение экрана: JAWS, NVDA, Window-Eyes, System Access</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3337"/>
        <source>Various Options</source>
        <translation>Различные опции</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3498"/>
        <source>This command will be run before the box content will be deleted</source>
        <translation>Эта команда будет запущена до того, как содержимое песочницы будет удалено</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3552"/>
        <source>On File Recovery</source>
        <translation>При восстановлении файлов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3581"/>
        <source>This command will be run before a file is being recovered and the file path will be passed as the first argument. If this command returns anything other than 0, the recovery will be blocked</source>
        <translation>Эта команда будет запущена перед восстановлением файла, и путь к файлу будет передан в качестве первого аргумента. Если эта команда возвращает значение, отличное от 0, восстановление будет заблокировано</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3584"/>
        <source>Run File Checker</source>
        <translation>Запустить проверку файлов</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4341"/>
        <source>Templates</source>
        <translation>Шаблоны</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4590"/>
        <source>The following settings enable the use of Sandboxie in combination with accessibility software.  Please note that some measure of Sandboxie protection is necessarily lost when these settings are in effect.</source>
        <translation>Следующие настройки позволяют использовать Sandboxie в сочетании с программным обеспечением специальных возможностей. Обратите внимание, что когда действуют эти настройки, определенная степень защиты Sandboxie обязательно теряется.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4643"/>
        <source>Edit ini Section</source>
        <translation>Редактировать раздел ini</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4649"/>
        <source>Edit ini</source>
        <translation>Редактировать ini</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4662"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4685"/>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
</context>
<context>
    <name>PopUpWindow</name>
    <message>
        <location filename="Forms/PopUpWindow.ui" line="32"/>
        <source>SandboxiePlus Notifications</source>
        <translation>Уведомления SandboxiePlus</translation>
    </message>
</context>
<context>
    <name>ProgramsDelegate</name>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="55"/>
        <source>Group: %1</source>
        <translation>Группа: %1</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="Views/SbieView.cpp" line="1497"/>
        <source>Drive %1</source>
        <translation>Диск %1</translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="SandMan.cpp" line="3604"/>
        <source>OK</source>
        <translation>ОК</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3605"/>
        <source>Apply</source>
        <translation>Применить</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3606"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3607"/>
        <source>&amp;Yes</source>
        <translation>Да (&amp;Y)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3608"/>
        <source>&amp;No</source>
        <translation>Нет (&amp;N)</translation>
    </message>
</context>
<context>
    <name>RecoveryWindow</name>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="32"/>
        <source>SandboxiePlus - Recovery</source>
        <translation>SandboxiePlus - Восстановление</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="164"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="68"/>
        <source>Recover target:</source>
        <translation>Восстановить:</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="42"/>
        <source>Add Folder</source>
        <translation>Добавить папку</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="61"/>
        <source>Delete Content</source>
        <translation>Удалить содержимое</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="110"/>
        <source>Recover</source>
        <translation>Восстановить</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="78"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="142"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="171"/>
        <source>Show All Files</source>
        <translation>Показать все файлы</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="184"/>
        <source>TextLabel</source>
        <translation>Текстовая метка</translation>
    </message>
</context>
<context>
    <name>SelectBoxWindow</name>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="32"/>
        <source>SandboxiePlus select box</source>
        <translation>SandboxiePlus выбор песочницы</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="87"/>
        <source>Select the sandbox in which to start the program, installer or document.</source>
        <translation>Выберите песочницу, в которой следует запустить программу, установщик или документ.</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="97"/>
        <source>Run in a new Sandbox</source>
        <translation>Запустить в новой песочнице</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="65"/>
        <source>Sandbox</source>
        <translation>Песочница</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="80"/>
        <source>Run As UAC Administrator</source>
        <translation>Запуск от имени администратора UAC</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="47"/>
        <source>Run Sandboxed</source>
        <translation>Запустить в песочнице</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="73"/>
        <source>Run Outside the Sandbox</source>
        <translation>Запустить вне песочницы</translation>
    </message>
</context>
<context>
    <name>SettingsWindow</name>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="32"/>
        <source>SandboxiePlus Settings</source>
        <translation>Настройки SandboxiePlus</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="55"/>
        <source>General Config</source>
        <translation>Общая конфигурация</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="80"/>
        <source>Show file recovery window when emptying sandboxes</source>
        <translation>Показывать окно восстановления файлов при очистке песочниц</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="195"/>
        <source>Open urls from this ui sandboxed</source>
        <translation>Открывать URL-адреса из этого пользовательского интерфейса в песочнице</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="492"/>
        <source>Systray options</source>
        <translation>Параметры системного лотка</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="185"/>
        <source>UI Language:</source>
        <translation>Язык пользовательского интерфейса:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="394"/>
        <source>Shell Integration</source>
        <translation>Интеграция с оболочкой</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="513"/>
        <source>Run Sandboxed - Actions</source>
        <translation>Запуск в песочнице - Действия</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="577"/>
        <source>Start Sandbox Manager</source>
        <translation>Запустить диспетчер песочницы</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="633"/>
        <source>Start UI when a sandboxed process is started</source>
        <translation>Запуск пользовательского интерфейса при запуске изолированного процесса</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="626"/>
        <source>Start UI with Windows</source>
        <translation>Запуск пользовательского интерфейса с Windows</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="534"/>
        <source>Add &apos;Run Sandboxed&apos; to the explorer context menu</source>
        <translation>Добавить &apos;Запустить в песочнице&apos; в контекстное меню проводника</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="173"/>
        <source>Hotkey for terminating all boxed processes:</source>
        <translation>Горячая клавиша для завершения всех процессов в песочнице:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="499"/>
        <source>Always use DefaultBox</source>
        <translation>Всегда использовать DefaultBox</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="442"/>
        <source>Show a tray notification when automatic box operations are started</source>
        <translation>Показывать уведомление в трее при запуске автоматических операций с песочницами</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1337"/>
        <source>Advanced Config</source>
        <translation>Расширенная конфигурация</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1531"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/filerootpath&quot;&gt;file system root&lt;/a&gt;: </source>
        <translation>Песочница &lt;a href=&quot;sbie://docs/filerootpath&quot;&gt;корень файловой системы&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1616"/>
        <source>Clear password when main window becomes hidden</source>
        <translation>Очистить пароль, когда главное окно сворачивается</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1371"/>
        <source>Activate Kernel Mode Object Filtering</source>
        <translation>Активировать фильтрацию объектов в режиме ядра</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1488"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/ipcrootpath&quot;&gt;ipc root&lt;/a&gt;: </source>
        <translation>Песочница &lt;a href=&quot;sbie://docs/ipcrootpath&quot;&gt;корень ipc&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1411"/>
        <source>Sandbox default</source>
        <translation>Песочница по умолчанию</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1602"/>
        <source>Config protection</source>
        <translation>Защита конфигурации</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1481"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="94"/>
        <source>SandMan Options</source>
        <translation>Параметры SandMan</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="208"/>
        <source>Notifications</source>
        <translation>Уведомления</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="240"/>
        <source>Add Entry</source>
        <translation>Добавить запись</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="357"/>
        <source>Show file migration progress when copying large files into a sandbox</source>
        <translation>Показывать прогресс переноса файлов при копировании больших файлов в песочницу</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="248"/>
        <source>Message ID</source>
        <translation>ID сообщения</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="253"/>
        <source>Message Text (optional)</source>
        <translation>Текст сообщения (необязательно)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="268"/>
        <source>SBIE Messages</source>
        <translation>Сообщения SBIE</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="287"/>
        <source>Delete Entry</source>
        <translation>Удалить запись</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="301"/>
        <source>Notification Options</source>
        <translation>Параметры уведомлений</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="327"/>
        <source>Sandboxie may be issue &lt;a href=&quot;sbie://docs/sbiemessages&quot;&gt;SBIE Messages&lt;/a&gt; to the Message Log and shown them as Popups. Some messages are informational and notify of a common, or in some cases special, event that has occurred, other messages indicate an error condition.&lt;br /&gt;You can hide selected SBIE messages from being popped up, using the below list:</source>
        <translation>Sandboxie может выдавать &lt;a href= &quot;sbie://docs/sbiemessages&quot;&gt;сообщения SBIE&lt;/a&gt; в журнал сообщений и отображать их как всплывающие окна. Некоторые сообщения носят информационный характер и уведомляют об общих или, в некоторых случаях, особых событиях, которые произошли, другие сообщения указывают на состояние ошибки.&lt;br /&gt;Вы можете скрыть выбранные сообщения SBIE от всплывающих окон, используя приведенный ниже список:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="340"/>
        <source>Disable SBIE messages popups (they will still be logged to the Messages tab)</source>
        <translation>Отключить всплывающие окна сообщений SBIE (они по-прежнему будут регистрироваться на вкладке &quot;Сообщения&quot;)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="404"/>
        <source>Windows Shell</source>
        <translation>Оболочка Windows</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="670"/>
        <source>Icon</source>
        <translation>Иконка</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="727"/>
        <source>Move Up</source>
        <translation>Сдвинуть вверх</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="746"/>
        <source>Move Down</source>
        <translation>Сдвинуть вниз</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="949"/>
        <source>Show overlay icons for boxes and processes</source>
        <translation>Показать оверлей иконки для песочниц и процессов</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="959"/>
        <source>Display Options</source>
        <translation>Параметры отображения</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1083"/>
        <source>Graphic Options</source>
        <translation>Параметры графики</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1228"/>
        <source>Supporters of the Sandboxie-Plus project can receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;. It&apos;s like a license key but for awesome people using open source software. :-)</source>
        <translation>Сторонники проекта Sandboxie-Plus могут получить &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;сертификат поддержки&lt;/a&gt;. Это похоже на лицензионный ключ, но для замечательных людей, использующих программное обеспечение с открытым исходным кодом. :-)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1321"/>
        <source>Keeping Sandboxie up to date with the rolling releases of Windows and compatible with all web browsers is a never-ending endeavor. You can support the development by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;directly contributing to the project&lt;/a&gt;, showing your support by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;purchasing a supporter certificate&lt;/a&gt;, becoming a patron by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;subscribing on Patreon&lt;/a&gt;, or through a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;PayPal donation&lt;/a&gt;.&lt;br /&gt;Your support plays a vital role in the advancement and maintenance of Sandboxie.</source>
        <translation>Поддержание Sandboxie в актуальном состоянии со скользящими выпусками Windows и совместимости со всеми веб-браузерами - это бесконечная работа. Вы можете поддержать разработку, &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;внеся непосредственный вклад в проект&lt;/a&gt; и проявив свою поддержку &lt;a href= &quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;купив сертификат поддержки&lt;/a&gt;, стать покровителем &lt;a href=&quot;https://sandboxie-plus. com/go.php?to=patreon&quot;&gt;подписавшись на Patreon&lt;/a&gt; или через &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;пожертвование PayPal&lt;/ a&gt;.&lt;br /&gt;Ваша поддержка играет жизненно важную роль в развитии и обслуживании Sandboxie.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1391"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/keyrootpath&quot;&gt;registry root&lt;/a&gt;: </source>
        <translation>Песочница &lt;a href=&quot;sbie://docs/keyrootpath&quot;&gt;корень реестра&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1451"/>
        <source>Sandboxing features</source>
        <translation>Возможности песочницы</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1560"/>
        <source>Sandboxie.ini Presets</source>
        <translation>Пресеты Sandboxie.ini</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1568"/>
        <source>Change Password</source>
        <translation>Изменить пароль</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1575"/>
        <source>Password must be entered in order to make changes</source>
        <translation>Для внесения изменений необходимо ввести пароль</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1623"/>
        <source>Only Administrator user accounts can make changes</source>
        <translation>Только учетная запись администратора может вносить изменения</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1630"/>
        <source>Watch Sandboxie.ini for changes</source>
        <translation>Следить за изменениями в Sandboxie.ini</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1748"/>
        <source>App Templates</source>
        <translation>Шаблоны приложений</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1758"/>
        <source>App Compatibility</source>
        <translation>Совместимость приложений</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1609"/>
        <source>Only Administrator user accounts can use Pause Forcing Programs command</source>
        <translation>Только учетная запись администратора может использовать команду &apos;Приостановить принудительные программы&apos;</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="151"/>
        <source>Run box operations asynchronously whenever possible (like content deletion)</source>
        <translation>Выполнять операции с песочницами асинхронно, когда это возможно (например, удаление содержимого)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1501"/>
        <source>Portable root folder</source>
        <translation>Корневая папка портативной версии</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="350"/>
        <source>Show recoverable files as notifications</source>
        <translation>Показывать восстанавливаемые файлы в виде уведомлений</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="600"/>
        <source>Show Icon in Systray:</source>
        <translation>Показать значок в системном лотке:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="412"/>
        <source>Show boxes in tray list:</source>
        <translation>Показать песочницы в списке лотка:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="65"/>
        <source>General Options</source>
        <translation>Общие настройки</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="527"/>
        <source>Add &apos;Run Un-Sandboxed&apos; to the context menu</source>
        <translation>Добавить &apos;Запустить без песочницы&apos; в контекстное меню</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1461"/>
        <source>Use Windows Filtering Platform to restrict network access</source>
        <translation>Использовать платформу фильтрации Windows для ограничения доступа к сети</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1468"/>
        <source>Hook selected Win32k system calls to enable GPU acceleration (experimental)</source>
        <translation>Перехватить выбранные системные вызовы Win32k, чтобы включить ускорение графического процессора (экспериментально)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="73"/>
        <source>Count and display the disk space occupied by each sandbox</source>
        <translation>Подсчитать и отобразить дисковое пространство, занимаемое каждой песочницей</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="475"/>
        <source>Use Compact Box List</source>
        <translation>Использовать компактный список песочниц</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="800"/>
        <source>Interface Config</source>
        <translation>Конфигурация интерфейса</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="868"/>
        <source>Show &quot;Pizza&quot; Background in box list *</source>
        <translation>Показать фон &quot;Пицца&quot; в списке песочниц *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="925"/>
        <source>Make Box Icons match the Border Color</source>
        <translation>Окрасить значки песочниц цветом рамки</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="915"/>
        <source>Use a Page Tree in the Box Options instead of Nested Tabs *</source>
        <translation>Использовать дерево страниц в опциях песочницы вместо вложенных вкладок *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="908"/>
        <source>Interface Options</source>
        <translation>Опции интерфейса</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="828"/>
        <source>Use large icons in box list *</source>
        <translation>Использовать большие значки в списке песочниц *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="970"/>
        <source>High DPI Scaling</source>
        <translation>Масштабирование с высоким разрешением</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="891"/>
        <source>Don&apos;t show icons in menus *</source>
        <translation>Не показывать значки в меню *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="932"/>
        <source>Use Dark Theme</source>
        <translation>Использовать тёмную тему</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="980"/>
        <source>Font Scaling</source>
        <translation>Масштабирование шрифта</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1042"/>
        <source>(Restart required)</source>
        <translation>(требуется перезагрузка)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="838"/>
        <source>* a partially checked checkbox will leave the behavior to be determined by the view mode.</source>
        <translation>* частично установленный флажок оставит поведение, определяемое режимом отображения.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="144"/>
        <source>Show the Recovery Window as Always on Top</source>
        <translation>Показывать окно восстановления поверх других</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1056"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="942"/>
        <source>Alternate row background in lists</source>
        <translation>Альтернативный фон строки в списках</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="818"/>
        <source>Use Fusion Theme</source>
        <translation>Использовать тему Fusion</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1521"/>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation>Использовать логин Sandboxie вместо анонимного токена (экспериментально)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="665"/>
        <location filename="Forms/SettingsWindow.ui" line="1676"/>
        <location filename="Forms/SettingsWindow.ui" line="1786"/>
        <location filename="Forms/SettingsWindow.ui" line="1919"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1681"/>
        <source>Path</source>
        <translation>Путь</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1703"/>
        <source>Remove Program</source>
        <translation>Удалить программу</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1689"/>
        <source>Add Program</source>
        <translation>Добавить программу</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1724"/>
        <source>When any of the following programs is launched outside any sandbox, Sandboxie will issue message SBIE1301.</source>
        <translation>Когда любая из следующих программ запускается вне любой песочницы, Sandboxie выдаст сообщение SBIE1301.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1734"/>
        <source>Add Folder</source>
        <translation>Добавить папку</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1717"/>
        <source>Prevent the listed programs from starting on this system</source>
        <translation>Запретить запуск перечисленных программ в этой системе</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1696"/>
        <source>Issue message 1308 when a program fails to start</source>
        <translation>Сообщение о проблеме 1308, когда программа не запускается</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="137"/>
        <source>Recovery Options</source>
        <translation>Параметры восстановления</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="435"/>
        <source>Start Menu Integration</source>
        <translation>Интеграция меню &quot;Пуск&quot;</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="520"/>
        <source>Scan shell folders and offer links in run menu</source>
        <translation>Сканировать папки оболочки и предлагать ссылки в меню запуска</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="541"/>
        <source>Integrate with Host Start Menu</source>
        <translation>Интеграция с меню &quot;Пуск&quot; хоста</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="858"/>
        <source>Use new config dialog layout *</source>
        <translation>Использовать новый макет диалогового окна конфигурации *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1644"/>
        <source>Program Control</source>
        <translation>Контроль программ</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1654"/>
        <source>Program Alerts</source>
        <translation>Оповещения программы</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1710"/>
        <source>Issue message 1301 when forced processes has been disabled</source>
        <translation>Выдать сообщение 1301, когда принудительные процессы были отключены</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1347"/>
        <source>Sandboxie Config</source>
        <translation>Конфигурация Sandboxie</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="364"/>
        <source>This option also enables asynchronous operation when needed and suspends updates.</source>
        <translation>Этот параметр также включает асинхронную работу, когда это необходимо, и приостанавливает обновления.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="367"/>
        <source>Suppress pop-up notifications when in game / presentation mode</source>
        <translation>Подавление всплывающих уведомлений в режиме игры/презентации</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="810"/>
        <source>User Interface</source>
        <translation>Пользовательский интерфейс</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="643"/>
        <source>Run Menu</source>
        <translation>Меню запуска</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="695"/>
        <source>Add program</source>
        <translation>Добавить программу</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="753"/>
        <source>You can configure custom entries for all sandboxes run menus.</source>
        <translation>Вы можете настроить пользовательские записи для всех меню запуска песочниц.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="775"/>
        <location filename="Forms/SettingsWindow.ui" line="1866"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="675"/>
        <source>Command Line</source>
        <translation>Командная строка</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1097"/>
        <source>Support &amp;&amp; Updates</source>
        <translation>Поддержка и бновления</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1418"/>
        <source>Default sandbox:</source>
        <translation>Песочница по умолчанию:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1778"/>
        <source>In the future, don&apos;t check software compatibility</source>
        <translation>В будущем не проверять совместимость программного обеспечения</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1794"/>
        <source>Enable</source>
        <translation>Включить</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1814"/>
        <source>Disable</source>
        <translation>Отключить</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1821"/>
        <source>Sandboxie has detected the following software applications in your system. Click OK to apply configuration settings, which will improve compatibility with these applications. These configuration settings will have effect in all existing sandboxes and in any new sandboxes.</source>
        <translation>Sandboxie обнаружила в вашей системе следующие программы. Нажмите OK, чтобы применить настройки конфигурации, которые улучшат совместимость с этими приложениями. Эти параметры конфигурации будут действовать во всех существующих песочницах и в любых новых песочницах.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1834"/>
        <source>Local Templates</source>
        <translation>Локальные шаблоны</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1885"/>
        <source>Add Template</source>
        <translation>Добавить шаблон</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1905"/>
        <source>Text Filter</source>
        <translation>Текстовый фильтр</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1927"/>
        <source>This list contains user created custom templates for sandbox options</source>
        <translation>Этот список содержит созданные пользователем настраиваемые шаблоны для параметров песочницы</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1944"/>
        <source>Edit ini Section</source>
        <translation>Редактировать раздел ini</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1953"/>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1960"/>
        <source>Edit ini</source>
        <translation>Редактировать ini</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1986"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1147"/>
        <source>Version Updates</source>
        <translation>Обновления версий</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1208"/>
        <source>New full versions from the selected release channel.</source>
        <translation>Новые полные версии из выбранного канала выпуска.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1144"/>
        <source>Hotpatches for the installed version, updates to the Templates.ini and translations.</source>
        <translation>Хотпатчи для установленной версии, обновления Templates.ini и переводов.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1161"/>
        <source>This supporter certificate has expired, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>Срок действия этого сертификата сторонника истек, пожалуйста &lt;a href=&quot;sbie://update/cert&quot;&gt;получите обновленный сертификат&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1198"/>
        <source>The preview channel contains the latest GitHub pre-releases.</source>
        <translation>Канал Preview содержит последние предварительные выпуски GitHub.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1211"/>
        <source>New Versions</source>
        <translation>Новые версии</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1218"/>
        <source>The stable channel contains the latest stable GitHub releases.</source>
        <translation>Канал Stable содержит последние стабильные выпуски GitHub.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1221"/>
        <source>Search in the Stable channel</source>
        <translation>Поиск в канале Stable</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1201"/>
        <source>Search in the Preview channel</source>
        <translation>Поиск в канале Preview</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1171"/>
        <source>In the future, don&apos;t notify about certificate expiration</source>
        <translation>В будущем не уведомлять об истечении срока действия сертификата</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1184"/>
        <source>Enter the support certificate here</source>
        <translation>Введите здесь сертификат сторонника</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1268"/>
        <source>Support Settings</source>
        <translation>Настройки поддержки</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1241"/>
        <source>Check periodically for updates of Sandboxie-Plus</source>
        <translation>Периодически проверять наличие обновлений Sandboxie-Plus</translation>
    </message>
</context>
<context>
    <name>SnapshotsWindow</name>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="32"/>
        <source>SandboxiePlus - Snapshots</source>
        <translation>SandboxiePlus - Снимки</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="60"/>
        <source>Selected Snapshot Details</source>
        <translation>Сведения о выбранном снимке</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="66"/>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="120"/>
        <source>Description:</source>
        <translation>Описание:</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="101"/>
        <source>When deleting a snapshot content, it will be returned to this snapshot instead of none.</source>
        <translation>При удалении содержимого снимка, оно будет возвращено этому снимку, а не пустому.</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="104"/>
        <source>Default snapshot</source>
        <translation>Снимок по умолчанию</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="139"/>
        <source>Snapshot Actions</source>
        <translation>Действия со снимками</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="189"/>
        <source>Remove Snapshot</source>
        <translation>Удалить снимок</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="208"/>
        <source>Go to Snapshot</source>
        <translation>Перейти к снимку</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="157"/>
        <source>Take Snapshot</source>
        <translation>Сделать снимок</translation>
    </message>
</context>
</TS>
