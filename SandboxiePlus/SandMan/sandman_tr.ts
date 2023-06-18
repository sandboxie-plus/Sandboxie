<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="tr_TR">
<context>
    <name>CAdvancedPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="428"/>
        <source>Advanced Sandbox options</source>
        <translation>Gelişmiş Korumalı Alan Seçenekleri</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="429"/>
        <source>On this page advanced sandbox options can be configured.</source>
        <translation>Bu sayfada gelişmiş korumalı alan seçenekleri yapılandırılabilir.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="434"/>
        <source>Network Access</source>
        <translation>Ağ Erişimi</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="442"/>
        <source>Allow network/internet access</source>
        <translation>Ağ/internet erişimine izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="443"/>
        <source>Block network/internet by denying access to Network devices</source>
        <translation>Ağ cihazlarına erişimi engelleyerek ağı/interneti engelle</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="445"/>
        <source>Block network/internet using Windows Filtering Platform</source>
        <translation>Windows Filtreleme Platformunu kullanarak ağı/interneti engelle</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="450"/>
        <source>Allow access to network files and folders</source>
        <translation>Ağ dosyalarına ve klasörlerine erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="451"/>
        <location filename="Wizards/NewBoxWizard.cpp" line="467"/>
        <source>This option is not recommended for Hardened boxes</source>
        <translation>Güçlendirilmiş alanlar için bu seçenek önerilmez</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="457"/>
        <source>Admin Options</source>
        <translation>Yönetici Seçenekleri</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="461"/>
        <source>Make applications think they are running elevated</source>
        <translation>Uygulamaların yetkilendirilmiş şekilde çalıştıklarını düşünmelerini sağla</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="466"/>
        <source>Allow MSIServer to run with a sandboxed system token</source>
        <translation>MSIServer&apos;ın korumalı alan sistem belirteci ile çalışmasına izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="472"/>
        <source>Box Options</source>
        <translation>Alan Seçenekleri</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="476"/>
        <source>Use a Sandboxie login instead of an anonymous token</source>
        <translation>Anonim kullanıcı yerine Sandboxie oturum açma belirteci kullan</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="482"/>
        <source>Prevent sandboxes programs installed on host from loading dll&apos;s from the sandbox</source>
        <translation>Ana bilgisayarda kurulu korumalı alanda çalışan programların korumalı alandan dll dosyaları yüklemesini önle</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="483"/>
        <source>This feature may reduce compatibility as it also prevents box located processes from writing to host located ones and even starting them.</source>
        <translation>Bu özellik, alanda konumlu işlemlerin ana bilgisayarda konumlu olanlara yazmasını ve hatta onları başlatmasını da engellediği için uyumluluğu azaltabilir.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="477"/>
        <source>Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.</source>
        <translation>Özel bir Sandboxie belirteci kullanmak, birbirinden ayrı korumalı alanların daha iyi yalıtılmasını sağlar ve görev yöneticilerinin kullanıcı sütununda bir işlemin hangi alana ait olduğu gösterir. Ancak bazı 3. parti güvenlik çözümleri özel belirteçlerle sorun yaşayabilir.</translation>
    </message>
</context>
<context>
    <name>CBoxTypePage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="168"/>
        <source>Create new Sandbox</source>
        <translation>Yeni Korumalı Alan Oluştur</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="175"/>
        <source>A sandbox isolates your host system from processes running within the box, it prevents them from making permanent changes to other programs and data in your computer. The level of isolation impacts your security as well as the compatibility with applications, hence there will be a different level of isolation depending on the selected Box Type. Sandboxie can also protect your personal data from being accessed by processes running under its supervision.</source>
        <translation>Korumalı alan, ana bilgisayar sisteminizi korumalı alan içinde çalışan işlemlerden yalıtır ve onların diğer programlarda ve bilgisayarınızdaki verilerde kalıcı değişiklikler yapmasını engeller. Yalıtım düzeyi, güvenliği ve uygulama uyumluluğunu etkiler, dolayısıyla seçili &apos;Alan Türüne&apos; bağlı olarak farklı bir yalıtım düzeyi sağlayacaktır. Sandboxie ayrıca kişisel verilerinize, kendi gözetimi altında çalışan işlemler tarafından erişilmesine karşı da koruyabilir.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="184"/>
        <source>Enter box name:</source>
        <translatorcomment>&apos;Sandbox&apos; için &apos;Korumalı Alan&apos; kullanıldığından dolayı &apos;Box&apos; için de &apos;Alan&apos; kullanıldı.</translatorcomment>
        <translation>Alan adı girin:</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="202"/>
        <source>Select box type:</source>
        <translation>Alan türü seçin:</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="205"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>Veri Korumalı Güçlendirilmiş Alan</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="206"/>
        <source>Security Hardened Sandbox</source>
        <translation>Güvenliği Güçlendirilmiş Alan</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="207"/>
        <source>Sandbox with Data Protection</source>
        <translation>Veri Korumalı Alan</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="208"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>Standart Yalıtımlı Korumalı Alan (Varsayılan)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="210"/>
        <source>Application Compartment with Data Protection</source>
        <translation>Veri Korumalı Uygulama Bölmesi</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="211"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>Uygulama Bölmesi (Yalıtım YOK)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="229"/>
        <source>Remove after use</source>
        <translation>Kullanıldıktan sonra kaldır</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="230"/>
        <source>After the last process in the box terminates, all data in the box will be deleted and the box itself will be removed.</source>
        <translation>Alandaki son işlem sona erdikten sonra alandaki tüm verilerle birlikte alanın kendisi de kaldırılacaktır.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="235"/>
        <source>Configure advanced options</source>
        <translation>Gelişmiş seçenekleri yapılandır</translation>
    </message>
</context>
<context>
    <name>CBrowserOptionsPage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="844"/>
        <source>Create Web Browser Template</source>
        <translation>Web Tarayıcı Şablonu Oluşturun</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="845"/>
        <source>Configure web browser template options.</source>
        <translation>Web tarayıcı şablonu seçeneklerini yapılandırın.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="855"/>
        <source>Force the Web Browser to run in this sandbox</source>
        <translation>Web Tarayıcısını bu korumalı alanda çalışmaya zorla</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="859"/>
        <source>Allow direct access to the entire Web Browser profile folder</source>
        <translation>Web Tarayıcısının profil klasörünün tamamına doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="863"/>
        <source>Allow direct access to Web Browser&apos;s phishing database</source>
        <translation>Web Tarayıcısının kimlik avı veritabanına doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="867"/>
        <source>Allow direct access to Web Browser&apos;s session management</source>
        <translation>Web Tarayıcısının oturum yönetimine doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="871"/>
        <source>Allow direct access to Web Browser&apos;s sync data</source>
        <translation>Web Tarayıcısının eşitleme verilerine doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="875"/>
        <source>Allow direct access to Web Browser&apos;s preferences</source>
        <translation>Web Tarayıcısının tercihlerine doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="879"/>
        <source>Allow direct access to Web Browser&apos;s passwords</source>
        <translation>Web Tarayıcısının parolalarına doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="883"/>
        <source>Allow direct access to Web Browser&apos;s cookies</source>
        <translation>Web Tarayıcısının tanımlama bilgilerine doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="887"/>
        <source>Allow direct access to Web Browser&apos;s bookmarks</source>
        <translation>Web Tarayıcısının yer imlerine doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="891"/>
        <source>Allow direct access to Web Browser&apos;s bookmark and history database</source>
        <translation>Web Tarayıcısının yer imi ve geçmiş veritabanına doğrudan erişime izin ver</translation>
    </message>
</context>
<context>
    <name>CBrowserPathsPage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="517"/>
        <source>Create Web Browser Template</source>
        <translation>Web Tarayıcı Şablonu Oluşturun</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="518"/>
        <source>Configure your Web Browser&apos;s profile directories.</source>
        <translation>Web Tarayıcılarınızın profil dizinlerini yapılandırın.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="526"/>
        <source>User profile(s) directory:</source>
        <translation>Kullanıcı profil(ler)i dizini:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="528"/>
        <source>Show also imperfect matches</source>
        <translation>Eksik eşleşmeleri de göster</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="534"/>
        <source>Browser Executable (*.exe)</source>
        <translation>Yürütülebilir Tarayıcı Dosyası (*.exe)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="551"/>
        <source>Continue without browser profile</source>
        <translation>Tarayıcı profili olmadan devam et</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="712"/>
        <source>Configure your Gecko based Browsers profile directories.</source>
        <translation>Gecko tabanlı Tarayıcıların profil dizinlerini yapılandırın.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="716"/>
        <source>Configure your Chromium based Browsers profile directories.</source>
        <translation>Chromium tabanlı Tarayıcıların profil dizinlerini yapılandırın.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="796"/>
        <source>No suitable folders have been found.
Note: you need to run the browser unsandboxed for them to get created.
Please browse to the correct user profile directory.</source>
        <translation>Uygun klasör bulunamadı.
Not: Oluşturulmaları için tarayıcıyı korumalı alanda dışında çalıştırmanız gerekir.
Lütfen doğru kullanıcı profili dizinine göz atın.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="801"/>
        <source>Please choose the correct user profile directory, if it is not listed you may need to browse to it.</source>
        <translation>Lütfen doğru kullanıcı profili dizinini seçin, listelenmemişse ona göz atmanız gerekebilir.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="807"/>
        <source>Please ensure the selected directory is correct, the wizard is not confident in all the presented options.</source>
        <translation>Lütfen seçili dizinin doğru olduğundan emin olun, sihirbaz sunulan tüm seçeneklerden emin değil.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="810"/>
        <source>Please ensure the selected directory is correct.</source>
        <translation>Lütfen seçili dizinin doğru olduğundan emin olun.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="815"/>
        <source>This path does not look like a valid profile directory.</source>
        <translation>Bu yol geçerli bir profil dizini gibi görünmüyor.</translation>
    </message>
</context>
<context>
    <name>CBrowserTypePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="325"/>
        <source>Create Web Browser Template</source>
        <translation>Web Tarayıcı Şablonu Oluşturun</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="326"/>
        <source>Select your Web Browsers main executable, this will allow Sandboxie to identify the browser.</source>
        <translation>Web Tarayıcınızın ana yürütülebilir dosyasını seçin, bu, Sandboxie&apos;nin tarayıcıyı tanımlamasını sağlayacaktır.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="336"/>
        <source>Enter browser name:</source>
        <translation>Tarayıcı adını girin:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="347"/>
        <source>Main executable (eg. firefox.exe, chrome.exe, msedge.exe, etc...):</source>
        <translation>Ana yürütülebilir dosya (ör. firefox.exe, chrome.exe, msedge.exe, vb...):</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="351"/>
        <source>Browser executable (*.exe)</source>
        <translation>Yürütülebilir tarayıcı dosyası (*.exe)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="451"/>
        <source>The browser appears to be Gecko based, like Mozilla Firefox and its derivatives.</source>
        <translation>Tarayıcı, Mozilla Firefox ve türevleri gibi Gecko tabanlı görünüyor.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="454"/>
        <source>The browser appears to be Chromium based, like Microsoft Edge or Google Chrome and its derivatives.</source>
        <translation>Tarayıcı, Microsoft Edge veya Google Chrome ve türevleri gibi Chromium tabanlı görünüyor.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="457"/>
        <source>Browser could not be recognized, template cannot be created.</source>
        <translation>Tarayıcı tanınamadı, şablon oluşturulamıyor.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="504"/>
        <source>This browser name is already in use, please choose an other one.</source>
        <translation>Bu tarayıcı adı zaten kullanımda, lütfen başka bir ad seçin.</translation>
    </message>
</context>
<context>
    <name>CCertificatePage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="195"/>
        <source>Install your &lt;b&gt;Sandboxie-Plus&lt;/b&gt; support certificate</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; destek sertifikanızı yükleyin</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="196"/>
        <source>If you have a supporter certificate, please fill it into the field below.</source>
        <translation>Destekçi sertifikanız varsa lütfen aşağıdaki alana doldurunuz.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="218"/>
        <source>Start evaluation without a certificate for a limited period of time.</source>
        <translation>Sınırlı bir süre için sertifika olmadan değerlendirmeye başlayın.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="246"/>
        <source>To use &lt;b&gt;Sandboxie-Plus&lt;/b&gt; in a business setting, an appropriate &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;support certificate&lt;/a&gt; for business use is required. If you do not yet have the required certificate(s), you can get those from the &lt;a href=&quot;https://xanasoft.com/shop/&quot;&gt;xanasoft.com web shop&lt;/a&gt;.</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt;&apos;ı bir iş ortamında kullanmak için uygun bir &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;destek sertifikası&lt;/a&gt; iş kullanımı için gereklidir. Henüz gerekli sertifika(lara) sahip değilseniz, bunları &lt;a href=&quot;https://xanasoft.com/shop/&quot;&gt;xanasoft.com web mağazasından&lt;/a&gt; alabilirsiniz.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="260"/>
        <source>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; provides additional features and box types exclusively to &lt;u&gt;project supporters&lt;/u&gt;. Boxes like the Privacy Enhanced boxes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;protect user data from illicit access&lt;/font&gt;&lt;/b&gt; by the sandboxed programs. If you are not yet a supporter, then please consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt; to ensure further development of Sandboxie and to receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;.</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt;, yalnızca &lt;u&gt;proje destekçilerine&lt;/u&gt; ek özellikler ve alan türleri sunar. Gizliliği geliştirilmiş alanlarda, &lt;b&gt;&lt;font color=&apos;red&apos;&gt;kullanıcı verileriniz korumalı alanda çalışan programların yetkisiz erişimlerine karşı daha iyi korunur.&lt;/font&gt;&lt;/b&gt; Henüz bir destekçimiz değilseniz, Sandboxie&apos;nin daha iyi geliştirilmesine katkıda bulunmak için bir &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;destekçi sertifikası&lt;/a&gt; alarak lütfen &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;projeyi desteklemeyi&lt;/a&gt; düşünün.</translation>
    </message>
</context>
<context>
    <name>CCleanUpJob</name>
    <message>
        <location filename="BoxJob.h" line="36"/>
        <source>Deleting Content</source>
        <translation>İçerik Silme</translation>
    </message>
</context>
<context>
    <name>CFileBrowserWindow</name>
    <message>
        <location filename="Views/FileView.cpp" line="398"/>
        <source>%1 - Files</source>
        <translation>%1 - Dosyalar</translation>
    </message>
</context>
<context>
    <name>CFileView</name>
    <message>
        <location filename="Views/FileView.cpp" line="188"/>
        <source>Create Shortcut</source>
        <translation>Kısayol Oluştur</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="210"/>
        <source>Pin to Box Run Menu</source>
        <translation>Alanın Çalıştır Menüsüne Sabitle</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="217"/>
        <source>Recover to Any Folder</source>
        <translation>Herhangi Bir Klasöre Kurtar</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="219"/>
        <source>Recover to Same Folder</source>
        <translation>Aynı Klasöre Kurtar</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="223"/>
        <source>Run Recovery Checks</source>
        <translation>Kurtarma Denetimlerini Çalıştır</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="287"/>
        <source>Select Directory</source>
        <translation>Dizin Seç</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="353"/>
        <source>Create Shortcut to sandbox %1</source>
        <translation>%1 korumalı alanına kısayol oluştur</translation>
    </message>
</context>
<context>
    <name>CFilesPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="302"/>
        <source>Sandbox location and behavior</source>
        <translation>Korumalı Alan Konumu ve Davranışı</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="303"/>
        <source>On this page the sandbox location and its behavior can be customized.
You can use %USER% to save each users sandbox to an own folder.</source>
        <translation>Bu sayfadan korumalı alan konumu ve davranışı özelleştirilebilir.
Her kullanıcının korumalı alanını kendi klasörüne kaydetmek için %USER% kullanabilirsiniz.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="308"/>
        <source>Sandboxed Files</source>
        <translation>Korumalı Alan Dosyaları</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="331"/>
        <source>Select Directory</source>
        <translation>Dizin Seç</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="339"/>
        <source>Virtualization scheme</source>
        <translation>Sanallaştırma şeması</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="343"/>
        <source>Version 1</source>
        <translation>Sürüm 1</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="344"/>
        <source>Version 2</source>
        <translation>Sürüm 2</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="350"/>
        <source>Separate user folders</source>
        <translation>Ayrı kullanıcı klasörleri</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="355"/>
        <source>Use volume serial numbers for drives</source>
        <translation>Sürücüler için birim seri numaralarını kullan</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="360"/>
        <source>Auto delete content when last process terminates</source>
        <translation>Son işlem sona erdiğinde içeriği otomatik olarak sil</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="367"/>
        <source>Enable Immediate Recovery of files from recovery locations</source>
        <translation>Kurtarma konumlarındaki dosyalar için Anında Kurtarmayı etkinleştir</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="402"/>
        <source>The selected box location is not a valid path.</source>
        <translation>Seçili alan konumu geçerli bir yol değil.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="407"/>
        <source>The selected box location exists and is not empty, it is recommended to pick a new or empty folder. Are you sure you want to use an existing folder?</source>
        <translation>Seçili alan konumu var ve boş değil, yeni veya boş bir klasör seçmeniz önerilir. Var olan klasörü kullanmak istediğinizden emin misiniz?</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="412"/>
        <source>The selected box location is not placed on a currently available drive.</source>
        <translation>Seçili alan konumu şu anda kullanılabilir bir sürücüde bulunmuyor.</translation>
    </message>
</context>
<context>
    <name>CFinishPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="489"/>
        <source>Complete your configuration</source>
        <translation>Yapılandırmanızı tamamlayın</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="496"/>
        <source>Almost complete, click Finish to apply all selected options and conclude the wizard.</source>
        <translation>Neredeyse tamamlandı, tüm seçimleri uygulamak ve sihirbazı tamamlamak için Bitir&apos;e tıklayın.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="508"/>
        <source>Keep Sandboxie-Plus up to date.</source>
        <translation>Sandboxie-Plus&apos;ı güncel tut.</translation>
    </message>
</context>
<context>
    <name>CFinishTemplatePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="936"/>
        <source>Create Web Browser Template</source>
        <translation>Web Tarayıcı Şablonu Oluşturun</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="944"/>
        <source>Almost complete, click Finish to create a new  Web Browser Template and conclude the wizard.</source>
        <translation>Neredeyse tamamlandı, yeni bir Web Tarayıcı Şablonu oluşturmak ve sihirbazı sonlandırmak için Bitiş&apos;e tıklayın.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="962"/>
        <source>Browser name: %1
</source>
        <translation>Tarayıcı adı: %1
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="966"/>
        <source>Browser Type: Gecko (Mozilla Firefox)
</source>
        <translation>Tarayıcı türü: Gecko (Mozilla Firefox)
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="967"/>
        <source>Browser Type: Chromium (Google Chrome)
</source>
        <translation>Tarayıcı türü: Chromium (Google Chrome)
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
        <translation>Tarayıcı yürütülebilir dosya yolu: %1
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="980"/>
        <source>Browser profile path: %1
</source>
        <translation>Tarayıcı profili yolu: %1
</translation>
    </message>
</context>
<context>
    <name>CIntroPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="117"/>
        <source>Introduction</source>
        <translation>Giriş</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="121"/>
        <source>Welcome to the Setup Wizard. This wizard will help you to configure your copy of &lt;b&gt;Sandboxie-Plus&lt;/b&gt;. You can start this wizard at any time from the Sandbox-&gt;Maintenance menu if you do not wish to complete it now.</source>
        <translation>Kurulum Sihirbazına hoş geldiniz. Bu sihirbaz, &lt;b&gt;Sandboxie-Plus&lt;/b&gt; kopyanızı yapılandırmanıza yardımcı olacaktır. Bu sihirbazı, şimdi tamamlamak istemiyorsanız, Korumalı Alan-&gt;Bakım menüsünden istediğiniz zaman başlatabilirsiniz.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="130"/>
        <source>Select how you would like to use Sandboxie-Plus</source>
        <translation>Sandboxie-Plus&apos;ı nasıl kullanmak istediğinizi seçin</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="133"/>
        <source>&amp;Personally, for private non-commercial use</source>
        <translation>&amp;Kişisel olarak, ticari olmayan özel kullanım için</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="138"/>
        <source>&amp;Commercially, for business or enterprise use</source>
        <translation>&amp;Ticari olarak, ticari veya kurumsal kullanım için</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="143"/>
        <source>Note: this option is persistent</source>
        <translation>Not: Bu seçenek kalıcıdır</translation>
    </message>
</context>
<context>
    <name>CMonitorModel</name>
    <message>
        <location filename="Models/MonitorModel.cpp" line="147"/>
        <source>Type</source>
        <translation>Tür</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="148"/>
        <source>Status</source>
        <translation>Durum</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="149"/>
        <source>Value</source>
        <translation>Değer</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="150"/>
        <source>Count</source>
        <translation>Yineleme</translation>
    </message>
</context>
<context>
    <name>CMultiErrorDialog</name>
    <message>
        <location filename="Dialogs/MultiErrorDialog.cpp" line="19"/>
        <source>Message</source>
        <translation>Mesaj</translation>
    </message>
    <message>
        <location filename="Dialogs/MultiErrorDialog.cpp" line="10"/>
        <source>Sandboxie-Plus - Error</source>
        <translation>Sandboxie-Plus - Hata</translation>
    </message>
</context>
<context>
    <name>CNewBoxWindow</name>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="23"/>
        <source>Sandboxie-Plus - Create New Box</source>
        <translation>Sandboxie-Plus - Yeni Alan Oluştur</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="39"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>Veri Korumalı Güçlendirilmiş Alan</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="40"/>
        <source>Security Hardened Sandbox</source>
        <translation>Güvenliği Güçlendirilmiş Alan</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="41"/>
        <source>Sandbox with Data Protection</source>
        <translation>Veri Korumalı Alan</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="42"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>Standart Yalıtımlı Korumalı Alan (Varsayılan)</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="44"/>
        <source>Application Compartment with Data Protection</source>
        <translation>Veri Korumalı Uygulama Bölmesi</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="45"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>Uygulama Bölmesi (Yalıtım YOK)</translation>
    </message>
</context>
<context>
    <name>CNewBoxWizard</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="30"/>
        <source>New Box Wizard</source>
        <translation>Yeni Alan Sihirbazı</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="138"/>
        <source>The new sandbox has been created using the new &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;Virtualization Scheme Version 2&lt;/a&gt;, if you experience any unexpected issues with this box, please switch to the Virtualization Scheme to Version 1 and report the issue, the option to change this preset can be found in the Box Options in the Box Structure group.</source>
        <translation>Yeni korumalı alan, yeni &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;Sanallaştırma Şeması Sürüm 2&lt;/a&gt; kullanılarak oluşturulmuştur. Bu alanla ilgili herhangi bir beklenmeyen sorunla karşılaşırsanız, lütfen Sanallaştırma Şeması Sürüm 1&apos;e geçip sorunu bize bildirin. Bu ön ayarı değiştirmek için Alan Seçenekleri sayfasındaki Dosya Seçenekleri grubunda bulunan Alan Yapısı bölümüne bakabilirsiniz.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="141"/>
        <source>Don&apos;t show this message again.</source>
        <translation>Bu mesajı bir daha gösterme.</translation>
    </message>
</context>
<context>
    <name>COnDeleteJob</name>
    <message>
        <location filename="BoxJob.h" line="58"/>
        <source>OnDelete: %1</source>
        <translation>Silmede: %1</translation>
    </message>
</context>
<context>
    <name>COnlineUpdater</name>
    <message>
        <location filename="OnlineUpdater.cpp" line="99"/>
        <source>Do you want to check if there is a new version of Sandboxie-Plus?</source>
        <translation>Sandboxie-Plus&apos;ın yeni sürümünü denetlemek istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="100"/>
        <source>Don&apos;t show this message again.</source>
        <translation>Bu mesajı bir daha gösterme.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="209"/>
        <source>Checking for updates...</source>
        <translation>Güncellemeler denetleniyor...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="248"/>
        <source>server not reachable</source>
        <translation>sunucuya ulaşılamıyor</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="249"/>
        <location filename="OnlineUpdater.cpp" line="251"/>
        <source>Failed to check for updates, error: %1</source>
        <translation>Güncellemeler denetlenemedi, hata: %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="404"/>
        <source>&lt;p&gt;Do you want to download the installer?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Yükleyiciyi indirmek istiyor musunuz?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="406"/>
        <source>&lt;p&gt;Do you want to download the updates?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Güncellemeleri indirmek istiyor musunuz?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="408"/>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;update page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation>&lt;p&gt;&lt;a href=&quot;%1&quot;&gt; güncelleme sayfasına&lt;/a&gt; gitmek istiyor musunuz?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="416"/>
        <source>Don&apos;t show this update anymore.</source>
        <translation>Bu güncellemeyi artık gösterme.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="534"/>
        <source>Downloading updates...</source>
        <translation>Güncellemeler indiriliyor...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="560"/>
        <source>invalid parameter</source>
        <translation>geçersiz parametre</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="561"/>
        <source>failed to download updated information</source>
        <translation>güncelleme bilgisi indirilemedi</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="562"/>
        <source>failed to load updated json file</source>
        <translation>güncel json dosyası yüklenemedi</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="563"/>
        <source>failed to download a particular file</source>
        <translation>belirli bir dosya indirilemedi</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="564"/>
        <source>failed to scan existing installation</source>
        <translation>mevcut kurulum taranamadı</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="565"/>
        <source>updated signature is invalid !!!</source>
        <translation>güncelleme imzası geçersiz!</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="566"/>
        <source>downloaded file is corrupted</source>
        <translation>indirilen dosya bozuk</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="567"/>
        <source>internal error</source>
        <translation>iç hata</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="568"/>
        <source>unknown error</source>
        <translation>bilinmeyen hata</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="592"/>
        <source>Failed to download updates from server, error %1</source>
        <translation>Güncellemeler sunucudan indirilemedi, hata %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="611"/>
        <source>&lt;p&gt;Updates for Sandboxie-Plus have been downloaded.&lt;/p&gt;&lt;p&gt;Do you want to apply these updates? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Sandboxie-Plus güncellemeleri indirildi.&lt;/p&gt;&lt;p&gt;Bu güncellemeleri uygulamak istiyor musunuz? Herhangi bir program korumalı alanda çalışıyorsa sonlandırılacaktır.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="715"/>
        <source>Downloading installer...</source>
        <translation>Yükleyici indiriliyor...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="757"/>
        <source>Failed to download installer from: %1</source>
        <translation>Şuradan yükleyici indirilemedi: %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="783"/>
        <source>&lt;p&gt;A new Sandboxie-Plus installer has been downloaded to the following location:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Do you want to begin the installation? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Yeni bir Sandboxie-Plus yükleyicisi şu konuma indirildi:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Kuruluma başlamak istiyor musunuz? Herhangi bir program korumalı alanda çalışıyorsa sonlandırılacaktır.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="848"/>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;info page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Bilgi sayfasına &lt;a href=&quot;%1&quot;&gt;gitmek istiyor musunuz&lt;/a&gt;?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="856"/>
        <source>Don&apos;t show this announcement in the future.</source>
        <translation>Bu duyuruyu gelecekte gösterme.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="398"/>
        <source>&lt;p&gt;There is a new version of Sandboxie-Plus available.&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;&lt;b&gt;New version:&lt;/b&gt;&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;Sandboxie-Plus&apos;ın yeni bir sürümü mevcut.&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;&lt;b&gt;Yeni sürüm:&lt;/b&gt;&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="274"/>
        <source>No new updates found, your Sandboxie-Plus is up-to-date.

Note: The update check is often behind the latest GitHub release to ensure that only tested updates are offered.</source>
        <translation>Yeni güncelleme bulunamadı, Sandboxie-Plus&apos;ınız güncel.

Not: Güncelleme denetimi, yalnızca test edilen güncellemelerin sunulmasını sağlamak için, genellikle en son GitHub sürümünün gerisindedir.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="964"/>
        <source>Checking for certificate...</source>
        <translation>Sertifika denetleniyor...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="1004"/>
        <source>No certificate found on server!</source>
        <translation>Sunucuda sertifika bulunamadı!</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="1009"/>
        <source>There is no updated certificate available.</source>
        <translation>Yenilenmiş bir sertifika yok.</translation>
    </message>
</context>
<context>
    <name>COptionsWindow</name>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="53"/>
        <source>Always show</source>
        <translation>Her zaman göster</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="611"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="605"/>
        <location filename="Windows/OptionsGeneral.cpp" line="579"/>
        <location filename="Windows/OptionsGrouping.cpp" line="234"/>
        <location filename="Windows/OptionsGrouping.cpp" line="258"/>
        <location filename="Windows/OptionsNetwork.cpp" line="533"/>
        <source>Template values can not be edited.</source>
        <translation>Şablon değerleri düzenlenemez.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="446"/>
        <location filename="Windows/OptionsWindow.cpp" line="460"/>
        <location filename="Windows/OptionsAccess.cpp" line="24"/>
        <source>Browse for File</source>
        <translation>Dosya için Göz At</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="897"/>
        <location filename="Windows/OptionsGeneral.cpp" line="913"/>
        <source>Please enter a menu title</source>
        <translation>Lütfen bir menü başlığı girin</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="365"/>
        <location filename="Windows/OptionsForce.cpp" line="242"/>
        <location filename="Windows/OptionsForce.cpp" line="252"/>
        <location filename="Windows/OptionsRecovery.cpp" line="128"/>
        <location filename="Windows/OptionsRecovery.cpp" line="139"/>
        <source>Select Directory</source>
        <translation>Dizin Seç</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGrouping.cpp" line="141"/>
        <source>Please enter a name for the new group</source>
        <translation>Lütfen yeni grup için bir isim girin</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="835"/>
        <source>Please enter a program file name</source>
        <translation>Lütfen bir program dosyası adı girin</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="679"/>
        <source>Template values can not be removed.</source>
        <translation>Şablon değerleri kaldırılamaz.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="49"/>
        <source>Display box name in title</source>
        <translation>Başlıkta korumalı alan adını göster</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="111"/>
        <location filename="Windows/OptionsForce.cpp" line="122"/>
        <source>Folder</source>
        <translation>Klasör</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="222"/>
        <location filename="Windows/OptionsForce.cpp" line="232"/>
        <source>Select Executable File</source>
        <translation>Yürütülebilir Dosya Seçin</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="222"/>
        <location filename="Windows/OptionsForce.cpp" line="232"/>
        <source>Executable Files (*.exe)</source>
        <translation>Yürütülebilir Dosyalar (*.exe)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="181"/>
        <source>Sandboxie Plus - &apos;%1&apos; Options</source>
        <translation>Sandboxie Plus - &apos;%1&apos; Ayarlar</translation>
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
        <translation>Grup: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="111"/>
        <location filename="Windows/OptionsForce.cpp" line="122"/>
        <source>Process</source>
        <translation>İşlem</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="48"/>
        <source>Display [#] indicator only</source>
        <translation>Yalnızca [#] göstergesini görüntüle</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="1022"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="51"/>
        <source>Border disabled</source>
        <translation>Kenarlık devre dışı</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="56"/>
        <source>All Categories</source>
        <translation>Tüm Kategoriler</translation>
    </message>
    <message>
        <location filename="Windows/OptionsRecovery.cpp" line="150"/>
        <source>Please enter a file extension to be excluded</source>
        <translation>Lütfen hariç tutulacak bir dosya uzantısı girin</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="353"/>
        <source>Select File</source>
        <translation>Dosya Seç</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="151"/>
        <source>This template is enabled globally. To configure it, use the global options.</source>
        <translation>Bu şablon genel (global) olarak etkindir. Yapılandırmak için genel ayarları kullanın.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGrouping.cpp" line="229"/>
        <source>Please select group first.</source>
        <translation>Lütfen önce grubu seçin.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="353"/>
        <source>All Files (*.*)</source>
        <translation>Tüm Dosyalar (*.*)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="52"/>
        <source>Show only when title is in focus</source>
        <translation>Yalnızca başlık odaktayken göster</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="893"/>
        <source>Select Program</source>
        <translation>Program Seç</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="909"/>
        <source>Please enter a command</source>
        <translation>Lütfen bir komut girin</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="712"/>
        <source>kilobytes (%1)</source>
        <translation>kilobayt (%1)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="19"/>
        <source>This option requires a valid supporter certificate</source>
        <translation>Bu seçenek geçerli bir destekçi sertifikası gerektirir</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="22"/>
        <source>Supporter exclusive option</source>
        <translation>Destekçiye özel seçenek</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="47"/>
        <source>Don&apos;t alter the window title</source>
        <translation>Pencere başlığını değiştirme</translation>
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
        <translation>Tüm Programlar</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="25"/>
        <source>Browse for Folder</source>
        <translation>Klasör için Göz At</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="1005"/>
        <source>Enter program:</source>
        <translation>Program girin:</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="537"/>
        <source>COM objects must be specified by their GUID, like: {00000000-0000-0000-0000-000000000000}</source>
        <translation>COM nesneleri GUID&apos;lerine göre belirtilmelidir, bunun gibi: {00000000-0000-0000-0000-000000000000}</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="543"/>
        <source>RT interfaces must be specified by their name.</source>
        <translation>RT arayüzleri isimleriyle belirtilmelidir.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="198"/>
        <source>Browse for Program</source>
        <translation>Program için Göz At</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="764"/>
        <source>Please enter a service identifier</source>
        <translation>Lütfen bir hizmet tanımlayıcısı girin</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="287"/>
        <source>File Options</source>
        <translation>Dosya Seçenekleri</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="319"/>
        <source>Grouping</source>
        <translation>Gruplama</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="525"/>
        <source>Add %1 Template</source>
        <translation>%1 Şablonu Ekle</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="625"/>
        <source>Search for options</source>
        <translation>Seçeneklerde ara</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="755"/>
        <source>Box: %1</source>
        <translation>Alan: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="756"/>
        <source>Template: %1</source>
        <translation>Şablon: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="757"/>
        <source>Global: %1</source>
        <translation>Genel: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="758"/>
        <source>Default: %1</source>
        <translation>Varsayılan: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="935"/>
        <source>This sandbox has been deleted hence configuration can not be saved.</source>
        <translation>Bu korumalı alan silindi, bu nedenle yapılandırma kaydedilemiyor.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="982"/>
        <source>Some changes haven&apos;t been saved yet, do you really want to close this options window?</source>
        <translation>Bazı değişiklikler henüz kaydedilmedi, bu seçenekler penceresini gerçekten kapatmak istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="311"/>
        <source>No Rename</source>
        <translation>Yeniden Adlandırma Yok</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="312"/>
        <source>Closed</source>
        <translation>Kapalı</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="313"/>
        <source>Closed RT</source>
        <translation>Kapalı RT</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="314"/>
        <source>Read Only</source>
        <translation>Salt Okunur</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="316"/>
        <source>Ignore UIPI</source>
        <translation>UIPI Yok Say</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="318"/>
        <location filename="Windows/OptionsAccess.cpp" line="335"/>
        <location filename="Windows/OptionsAccess.cpp" line="348"/>
        <source>Unknown</source>
        <translation>Bilinmeyen</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="325"/>
        <source>Regular Sandboxie behavior - allow read and also copy on write.</source>
        <translation>Normal Sandboxie davranışı - okumaya izin verir ve ayrıca yazma üzerine kopyalar.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="326"/>
        <source>Allow write-access outside the sandbox.</source>
        <translation>Korumalı alanın dışına yazma erişimine izin verir.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="327"/>
        <source>Allow write-access outside the sandbox, also for applications installed inside the sandbox.</source>
        <translation>Korumalı alanın içine yüklenen uygulamalar için de korumalı alanın dışına yazma erişimine izin verir.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="328"/>
        <source>Don&apos;t rename window classes.</source>
        <translation>Pencere sınıfları yeniden adlandırmaz.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="329"/>
        <source>Deny access to host location and prevent creation of sandboxed copies.</source>
        <translation>Ana bilgisayar konumuna erişimi reddeder ve korumalı alan kopyalarının oluşturulmasını önler.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="330"/>
        <source>Block access to WinRT class.</source>
        <translation>WinRT sınıfına erişimi engeller.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="331"/>
        <source>Allow read-only access only.</source>
        <translation>Yalnızca salt okunur erişime izin verir.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="332"/>
        <source>Hide host files, folders or registry keys from sandboxed processes.</source>
        <translation>Ana bilgisayar dosyalarını, klasörlerini veya kayıt defteri anahtarlarını korumalı alan işlemlerinden gizler.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="333"/>
        <source>Ignore UIPI restrictions for processes.</source>
        <translation>İşlemler için UIPI kısıtlamalarını yok sayar.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="342"/>
        <source>File/Folder</source>
        <translation>Dosya/Klasör</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="343"/>
        <source>Registry</source>
        <translation>Kayıt</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="344"/>
        <source>IPC Path</source>
        <translation>IPC Yolu</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="345"/>
        <source>Wnd Class</source>
        <translation>Wnd Sınıfı</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="346"/>
        <source>COM Object</source>
        <translation>COM Objesi</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="557"/>
        <source>Opening all IPC access also opens COM access, do you still want to restrict COM to the sandbox?</source>
        <translation>Tüm IPC erişimini açmak COM erişimini de açar, COM&apos;u yine de korumalı alanla sınırlandırmak istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="558"/>
        <source>Don&apos;t ask in future</source>
        <translation>Gelecekte sorma</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="567"/>
        <source>&apos;OpenWinClass=program.exe,#&apos; is not supported, use &apos;NoRenameWinClass=program.exe,*&apos; instead</source>
        <translation>&apos;OpenWinClass=program.exe,#&apos; desteklenmiyor, bunun yerine &apos;NoRenameWinClass=program.exe,*&apos; kullanın</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="95"/>
        <source>Custom Templates</source>
        <translation>Özel Şablonlar</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="96"/>
        <source>Email Reader</source>
        <translation>E-posta Okuyucu</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="97"/>
        <source>PDF/Print</source>
        <translation>PDF/Baskı</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="98"/>
        <source>Security/Privacy</source>
        <translation>Güvenlik/Gizlilik</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="99"/>
        <source>Desktop Utilities</source>
        <translation>Masaüstü Yardımcı Programları</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="100"/>
        <source>Download Managers</source>
        <translation>İndirme Yöneticileri</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="101"/>
        <source>Miscellaneous</source>
        <translation>Çeşitli</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="102"/>
        <source>Web Browser</source>
        <translation>Web Tarayıcısı</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="103"/>
        <source>Media Player</source>
        <translation>Medya Oynatıcı</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="104"/>
        <source>Torrent Client</source>
        <translation>Torrent İstemcisi</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="187"/>
        <source>Please enter the template identifier</source>
        <translation>Lütfen şablon tanımlayıcısını girin</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="195"/>
        <source>Error: %1</source>
        <translation>Hata: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="222"/>
        <source>Do you really want to delete the selected local template(s)?</source>
        <translation>Seçili yerel şablonları gerçekten silmek istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="228"/>
        <source>Only local templates can be removed!</source>
        <translation>Yalnızca yerel şablonlar kaldırılabilir!</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="37"/>
        <location filename="Windows/OptionsNetwork.cpp" line="588"/>
        <source>Any</source>
        <translation>Herhangi</translation>
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
        <translation>Erişime izin ver</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="53"/>
        <source>Block using Windows Filtering Platform</source>
        <translation>Windows Filtreleme Platformunu kullanarak engelle</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="54"/>
        <source>Block by denying access to Network devices</source>
        <translation>Ağ cihazlarına erişimi reddederek engelle</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="917"/>
        <location filename="Windows/OptionsNetwork.cpp" line="171"/>
        <location filename="Windows/OptionsNetwork.cpp" line="574"/>
        <source>Allow</source>
        <translation>İzin ver</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="172"/>
        <source>Block (WFP)</source>
        <translation>Engelle (WFP)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="173"/>
        <source>Block (NDev)</source>
        <translation>Engelle (NDev)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="278"/>
        <source>A non empty program name is required.</source>
        <translation>Boş olmayan bir program adı gereklidir.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="575"/>
        <source>Block</source>
        <translation>Engelle</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="308"/>
        <source>Normal</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="309"/>
        <source>Open</source>
        <translation>Açık</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="310"/>
        <source>Open for All</source>
        <translation>Hepsine Açık</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="56"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>Veri Korumalı Güçlendirilmiş Alan</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="57"/>
        <source>Security Hardened Sandbox</source>
        <translation>Güvenliği Güçlendirilmiş Alan</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="58"/>
        <source>Sandbox with Data Protection</source>
        <translation>Veri Korumalı Alan</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="59"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>Standart Yalıtımlı Alan (Varsayılan)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="61"/>
        <source>Application Compartment with Data Protection</source>
        <translation>Veri Korumalı Uygulama Bölmesi</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="62"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>Uygulama Bölmesi (Yalıtım YOK)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="109"/>
        <source>Custom icon</source>
        <translation>Özel simge</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="161"/>
        <source>Version 1</source>
        <translation>Sürüm 1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="162"/>
        <source>Version 2</source>
        <translation>Sürüm 2</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="258"/>
        <source>Open Box Options</source>
        <translation>Alan Seçeneklerini Aç</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="259"/>
        <source>Browse Content</source>
        <translation>İçeriğe Göz At</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="260"/>
        <source>Start File Recovery</source>
        <translation>Dosya Kurtarmayı Başlatın</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="261"/>
        <source>Show Run Dialog</source>
        <translation>Çalıştır Diyaloğunu Göster</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="294"/>
        <source>Indeterminate</source>
        <translation>Belirsiz</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="484"/>
        <location filename="Windows/OptionsGeneral.cpp" line="584"/>
        <source>Always copy</source>
        <translation>Her zaman kopyala</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="485"/>
        <location filename="Windows/OptionsGeneral.cpp" line="585"/>
        <source>Don&apos;t copy</source>
        <translation>Kopyalama</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="486"/>
        <location filename="Windows/OptionsGeneral.cpp" line="586"/>
        <source>Copy empty</source>
        <translation>Boş kopyala</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="818"/>
        <source>Select color</source>
        <translation>Renk seç</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="893"/>
        <source>Executables (*.exe *.cmd)</source>
        <translation>Yürütülebilir dosyalar (*.exe *.cmd)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="315"/>
        <source>Box Only (Write Only)</source>
        <translation>Yalnızca Alan (Salt Yazma)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="41"/>
        <source>Enable the use of win32 hooks for selected processes. Note: You need to enable win32k syscall hook support globally first.</source>
        <translation>Seçili işlemler için win32 kancalarının kullanımını etkinleştirir. Not: Önce win32k syscall kanca desteğini global olarak etkinleştirmeniz gerekir.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="42"/>
        <source>Enable crash dump creation in the sandbox folder</source>
        <translation>Korumalı alan klasöründe kilitlenme döküm dosyası oluşturmayı etkinleştirir</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="43"/>
        <source>Always use ElevateCreateProcess fix, as sometimes applied by the Program Compatibility Assistant.</source>
        <translation>Bazen Program Uyumluluk Yardımcısı tarafından uygulandığı gibi her zaman ElevateCreateProcess düzeltmesini kullanır.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="45"/>
        <source>Enable special inconsistent PreferExternalManifest behaviour, as needed for some Edge fixes</source>
        <translation>Bazı Edge düzeltmeleri için gerektiği gibi özel tutarsız PreferExternalManifest davranışını etkinleştirir</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="46"/>
        <source>Set RpcMgmtSetComTimeout usage for specific processes</source>
        <translation>Belirli işlemler için RpcMgmtSetComTimeout kullanımını ayarlar</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="47"/>
        <source>Makes a write open call to a file that won&apos;t be copied fail instead of turning it read-only.</source>
        <translation>Kopyalanmayacak bir dosyaya yapılan yazma açma çağrısını salt okunur hale getirmek yerine başarısız kılar.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="50"/>
        <source>Make specified processes think they have admin permissions.</source>
        <translation>Belirtilen işlemlerin yönetici izinlerine sahip olduklarını düşünmelerini sağlar.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="51"/>
        <source>Force specified processes to wait for a debugger to attach.</source>
        <translation>Belirtilen işlemleri bir hata ayıklayıcının bağlanmasına beklemeye zorlar.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="53"/>
        <source>Sandbox file system root</source>
        <translation>Korumalı alan dosya sistemi kökü</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="54"/>
        <source>Sandbox registry root</source>
        <translation>Korumalı alan kayıt defteri kökü</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="55"/>
        <source>Sandbox ipc root</source>
        <translation>Korumalı alan ipc kökü</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="575"/>
        <source>Add special option:</source>
        <translation>Özel seçenek ekle:</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="726"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="730"/>
        <source>On Start</source>
        <translation>Başlangıçta</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="727"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="735"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="739"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="743"/>
        <source>Run Command</source>
        <translation>Komutu Çalıştır</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="731"/>
        <source>Start Service</source>
        <translation>Hizmeti Başlat</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="734"/>
        <source>On Init</source>
        <translation>İlk Kullanımda</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="738"/>
        <source>On File Recovery</source>
        <translation>Doysa Kurtarmada</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="742"/>
        <source>On Delete Content</source>
        <translation>İçerik Silmede</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="917"/>
        <source>Deny</source>
        <translation>Reddet</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="753"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="775"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="786"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="797"/>
        <source>Please enter the command line to be executed</source>
        <translation>Lütfen çalıştırılacak komut satırını girin</translation>
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
        <location filename="Windows/PopUpWindow.h" line="61"/>
        <source>Hide all such messages</source>
        <translation>Tüm bu tür mesajları gizle</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="75"/>
        <source> (%1)</source>
        <translation> (%1)</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="58"/>
        <source>Remove this message from the list</source>
        <translation>Bu mesajı listeden kaldırır</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="57"/>
        <source>Dismiss</source>
        <translation>Yok Say</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="50"/>
        <source>Visit %1 for a detailed explanation.</source>
        <translation>Ayrıntılı açıklama için %1 bağlantısını ziyaret edin.</translation>
    </message>
</context>
<context>
    <name>CPopUpProgress</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="367"/>
        <source>Remove this progress indicator from the list</source>
        <translation>Bu ilerleme göstergesini listeden kaldırır</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="366"/>
        <source>Dismiss</source>
        <translation>Kaldır</translation>
    </message>
</context>
<context>
    <name>CPopUpPrompt</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="123"/>
        <source>No</source>
        <translation>Hayır</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="118"/>
        <source>Yes</source>
        <translation>Evet</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="184"/>
        <source>Requesting process terminated</source>
        <translation>İstek süreci sonlandırıldı</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="108"/>
        <source>Remember for this process</source>
        <translation>Bu işlem için hatırla</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="128"/>
        <source>Terminate</source>
        <translation>Sonlandır</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="189"/>
        <source>Request will time out in %1 sec</source>
        <translation>İsteğin süresi %1 saniye içinde dolacak</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="191"/>
        <source>Request timed out</source>
        <translation>İsteğin süresi doldu</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="149"/>
        <source>Yes and add to allowed programs</source>
        <translation>Evet ve izin verilen programlara ekle</translation>
    </message>
</context>
<context>
    <name>CPopUpRecovery</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="279"/>
        <source>Disable quick recovery until the box restarts</source>
        <translation>Korumalı alan yeniden başlatılana kadar hızlı kurtarmayı devre dışı bırak</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="259"/>
        <source>Recover</source>
        <translation>Kurtar</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="260"/>
        <source>Recover the file to original location</source>
        <translation>Dosyayı gerçek konumuna kurtarır</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="274"/>
        <source>Dismiss</source>
        <translation>Reddet</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="275"/>
        <source>Don&apos;t recover this file right now</source>
        <translation>Bu dosya şimdilik kurtarılmaz</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="266"/>
        <source>Open file recovery for this box</source>
        <translation>Bu korumalı alan için dosya kurtarma penceresini aç</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="278"/>
        <source>Dismiss all from this box</source>
        <translation>Bu korumalı alandan gelen bildirimleri kaldır</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="245"/>
        <source>Recover to:</source>
        <translation>Şuraya geri yükle:</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="250"/>
        <source>Browse</source>
        <translation>Göz At</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="251"/>
        <source>Clear folder list</source>
        <translation>Klasör listesini temizle</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="263"/>
        <source>Recover &amp;&amp; Explore</source>
        <translation>Kurtar &amp;&amp; Keşfet</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="264"/>
        <source>Recover &amp;&amp; Open/Run</source>
        <translation>Kurtar &amp;&amp; Aç/Çalıştır</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="303"/>
        <source>Select Directory</source>
        <translation>Dizin Seç</translation>
    </message>
</context>
<context>
    <name>CPopUpWindow</name>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="346"/>
        <source>an UNKNOWN process.</source>
        <translation>BİLİNMEYEN bir işlem.</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="25"/>
        <source>Sandboxie-Plus Notifications</source>
        <translation>Sandboxie-Plus Bildirimleri</translation>
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
        <translation>BİLİNMEYEN</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="171"/>
        <source>Do you want to allow the print spooler to write outside the sandbox for %1 (%2)?</source>
        <translation>Yazdırma biriktiricisinin %1 (%2) için korumalı alanın dışına yazmasına izin vermek istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="279"/>
        <source>Do you want to allow %4 (%5) to copy a %1 large file into sandbox: %2?
File name: %3</source>
        <translation>%4&apos;nin (%5) %1 boyutundaki dosyayı korumalı alana kopyalanmasına izin vermek istiyor musunuz: %2?
Dosya adı: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="285"/>
        <source>Do you want to allow %1 (%2) access to the internet?
Full path: %3</source>
        <translation>%1&apos;nin (%2) internete erişmesine izin vermek istiyor musunuz?
Tam yol: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="344"/>
        <source>%1 is eligible for quick recovery from %2.
The file was written by: %3</source>
        <translation>%1 dosyası %2 içinden hızlı kurtarılmaya uygundur.
Dosyayı yazan: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="437"/>
        <source>Migrating a large file %1 into the sandbox %2, %3 left.
Full path: %4</source>
        <translation>%1 boyutundaki dosya %2 korumalı alanına taşınıyor, %3 kaldı.
Tam yol: %4</translation>
    </message>
</context>
<context>
    <name>CRecoveryLogWnd</name>
    <message>
        <location filename="SandManRecovery.cpp" line="306"/>
        <source>Sandboxie-Plus - Recovery Log</source>
        <translation>Sandboxie-Plus - Kurtarma Günlüğü</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="317"/>
        <source>Time|Box Name|File Path</source>
        <translation>Zaman|Alan Adı|Dosya Yolu</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="319"/>
        <source>Cleanup Recovery Log</source>
        <translation>Temizleme Kurtarma Günlüğü</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="329"/>
        <source>The following files were recently recovered and moved out of a sandbox.</source>
        <translation>Şu dosyalar yakın zamanda kurtarıldı ve bir korumalı alandan taşındı.</translation>
    </message>
</context>
<context>
    <name>CRecoveryWindow</name>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="70"/>
        <source>File Name</source>
        <translation>Dosya Adı</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="71"/>
        <source>File Size</source>
        <translation>Dosya Boyutu</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="72"/>
        <source>Full Path</source>
        <translation>Tam yol</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="180"/>
        <location filename="Windows/RecoveryWindow.cpp" line="201"/>
        <location filename="Windows/RecoveryWindow.cpp" line="552"/>
        <source>Select Directory</source>
        <translation>Dizin Seç</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="46"/>
        <source>%1 - File Recovery</source>
        <translation>%1 - Dosya Kurtarma</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="588"/>
        <source>There are %1 files and %2 folders in the sandbox, occupying %3 of disk space.</source>
        <translation>Korumalı alanda %3 disk alanı kaplayan %1 dosya ve %2 klasör var.</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="297"/>
        <source>There are %1 new files available to recover.</source>
        <translation>Kurtarılabilecek %1 yeni dosya var.</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="106"/>
        <source>Remember target selection</source>
        <translation>Hedef seçimini hatırla</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="138"/>
        <source>Original location</source>
        <translation>Gerçek konum</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="139"/>
        <source>Browse for location</source>
        <translation>Konum için göz atın</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="140"/>
        <source>Clear folder list</source>
        <translation>Klasör listesini temizle</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="248"/>
        <source>Do you really want to delete %1 selected files?</source>
        <translation>Seçili %1 dosyayı gerçekten silmek istiyor musun?</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="276"/>
        <source>Close until all programs stop in this box</source>
        <translation>Bu alandaki tüm programlar sonlanıncaya kadar kapat</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="112"/>
        <source>Delete everything, including all snapshots</source>
        <translation>Tüm anlık görüntüler dahil her şeyi sil</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="277"/>
        <source>Close and Disable Immediate Recovery for this box</source>
        <translation>Kapat ve bu alan için Anında Kurtarmayı devre dışı bırak</translation>
    </message>
</context>
<context>
    <name>CSandBox</name>
    <message>
        <location filename="SandMan.cpp" line="3614"/>
        <source>Waiting for folder: %1</source>
        <translation>Klasör bekleniyor: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3615"/>
        <source>Deleting folder: %1</source>
        <translation>Klasör siliniyor: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3616"/>
        <source>Merging folders: %1 &amp;gt;&amp;gt; %2</source>
        <translation>Klasörler birleştiriliyor: %1 &amp;gt;&amp;gt; %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3617"/>
        <source>Finishing Snapshot Merge...</source>
        <translation>Anlık Görüntü Birleştirme Tamamlanıyor...</translation>
    </message>
</context>
<context>
    <name>CSandBoxPlus</name>
    <message>
        <location filename="SbiePlusAPI.cpp" line="687"/>
        <source>No Admin</source>
        <translation>Yönetici Yok</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="682"/>
        <source>No INet</source>
        <translation>INet Yok</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="680"/>
        <source>No INet (with Exceptions)</source>
        <translation>INet Yok (İstisnalarla)</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="690"/>
        <source>Auto Delete</source>
        <translation>Oto Silme</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="693"/>
        <source>Normal</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="677"/>
        <source>API Log</source>
        <translation>API Günlüğü</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="685"/>
        <source>Net Share</source>
        <translation>Net Paylaşımı</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="671"/>
        <source>Enhanced Isolation</source>
        <translation>Geliştirilmiş Yalıtım</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="669"/>
        <source>Reduced Isolation</source>
        <translation>Azaltılmış Yalıtım</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="652"/>
        <source>Disabled</source>
        <translation>Devre Dışı</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="663"/>
        <source>OPEN Root Access</source>
        <translation>AÇIK Kök Erişimi</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="665"/>
        <source>Application Compartment</source>
        <translation>Uygulama Bölmesi</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="667"/>
        <source>NOT SECURE</source>
        <translation>GÜVENLİ DEĞİL</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="674"/>
        <source>Privacy Enhanced</source>
        <translation>Gelişmiş Gizlilik</translation>
    </message>
</context>
<context>
    <name>CSandMan</name>
    <message>
        <location filename="SandMan.cpp" line="500"/>
        <location filename="SandMan.cpp" line="654"/>
        <source>Exit</source>
        <translation>Çıkış</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2634"/>
        <source>Sandboxie-Plus was started in portable mode and it needs to create necessary services. This will prompt for administrative privileges.</source>
        <translation>Sandboxie-Plus taşınabilir modda başlatıldı ve gerekli hizmetleri oluşturması gerekiyor. Bunun için yönetici ayrıcalıkları isteyecektir.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="536"/>
        <source>Cleanup Processes</source>
        <translation>İşlemleri Temizle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="447"/>
        <source>&amp;Help</source>
        <translation>&amp;Yardım</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="503"/>
        <location filename="SandMan.cpp" line="656"/>
        <source>&amp;View</source>
        <translation>&amp;Görünüm</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3275"/>
        <source>Error deleting sandbox folder: %1</source>
        <translation>Korumalı alan klasörü silinirken hata: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="461"/>
        <location filename="SandMan.cpp" line="3588"/>
        <source>About Sandboxie-Plus</source>
        <translation>Sandboxie-Plus Hakkında</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="177"/>
        <location filename="SandMan.cpp" line="1997"/>
        <source>Sandboxie-Plus v%1</source>
        <translation>Sandboxie-Plus v%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="417"/>
        <source>Start Driver</source>
        <translation>Sürücüyü Başlat</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="416"/>
        <source>Install Driver</source>
        <translation>Sürücüyü Yükle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="419"/>
        <source>Uninstall Driver</source>
        <translation>Sürücüyü Kaldır</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="458"/>
        <source>Check for Updates</source>
        <translation>Güncellemeleri Denetle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="456"/>
        <source>Visit Support Forum</source>
        <translation>Destek Forumu&apos;nu Ziyaret Et</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3266"/>
        <source>Failed to copy configuration from sandbox %1: %2</source>
        <translation>%1 korumalı alanından yapılandırma kopyalaması başarısız oldu: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="435"/>
        <source>Simple View</source>
        <translation>Basit Görünüm</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2378"/>
        <source>%1 (%2): </source>
        <translation>%1 (%2): </translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2546"/>
        <source>Login Failed: %1</source>
        <translation>Giriş başarısız: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="535"/>
        <source>Clean Up</source>
        <translation>Temizle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1283"/>
        <location filename="SandMan.cpp" line="1696"/>
        <location filename="SandMan.cpp" line="2027"/>
        <location filename="SandMan.cpp" line="2635"/>
        <location filename="SandMan.cpp" line="3063"/>
        <location filename="SandMan.cpp" line="3079"/>
        <source>Don&apos;t show this message again.</source>
        <translation>Bu mesajı bir daha gösterme.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="424"/>
        <source>Uninstall Service</source>
        <translation>Hizmeti Kaldır</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="422"/>
        <source>Start Service</source>
        <translation>Hizmeti Başlat</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="421"/>
        <source>Install Service</source>
        <translation>Hizmeti Yükle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3284"/>
        <source>Failed to remove old snapshot directory &apos;%1&apos;</source>
        <translation>Eski anlık görüntü dizini kaldırılamadı &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3078"/>
        <source>The changes will be applied automatically as soon as the editor is closed.</source>
        <translation>Düzenleyici kapatılır kapatılmaz değişiklikler otomatik olarak uygulanacaktır.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1265"/>
        <source>Do you want to close Sandboxie Manager?</source>
        <translation>Sandboxie Yöneticisi&apos;ni kapatmak istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3280"/>
        <source>Failed to create directory for new snapshot</source>
        <translation>Yeni anlık görüntü için dizin oluşturulamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="418"/>
        <source>Stop Driver</source>
        <translation>Sürücüyü Durdur</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1194"/>
        <source>Sbie Messages</source>
        <translation>Sbie Mesajları</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="99"/>
        <location filename="SandManRecovery.cpp" line="159"/>
        <source>Checking file %1</source>
        <translation>%1 dosyası denetleniyor</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="107"/>
        <source>The file %1 failed a security check!

%2</source>
        <translation>%1 dosyası için güvenlik denetimi başarısız oldu!

%2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="115"/>
        <source>All files passed the checks</source>
        <translation>Tüm dosyalar denetimi geçti</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="178"/>
        <source>The file %1 failed a security check, do you want to recover it anyway?

%2</source>
        <translation>%1 dosyası için güvenlik denetimi başarısız oldu, dosyayı yine de kurtarmak istiyor musunuz?

%2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="243"/>
        <source>Failed to recover some files: 
</source>
        <translation>Bazı dosyalar kurtarılamadı: 
</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3278"/>
        <source>Failed to move directory &apos;%1&apos; to &apos;%2&apos;</source>
        <translation>&apos;%1&apos; dizini, &apos;%2&apos; dizinine taşınamadı</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="202"/>
        <source>Recovering file %1 to %2</source>
        <translation>%1&apos;dan %2&apos;a dosya kurtarılıyor</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="455"/>
        <source>Online Documentation</source>
        <translation>Çevrimiçi Belgeler</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2771"/>
        <location filename="SandMan.cpp" line="2773"/>
        <location filename="SandMan.cpp" line="3322"/>
        <source>Sandboxie-Plus - Error</source>
        <translation>Sandboxie-Plus - Hata</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1185"/>
        <source>Time|Message</source>
        <translation>Zaman|Mesaj</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="562"/>
        <source>&amp;Options</source>
        <translation>&amp;Seçenekler</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="45"/>
        <source>Show/Hide</source>
        <translation>Göster/Gizle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3277"/>
        <source>A sandbox must be emptied before it can be deleted.</source>
        <translation>Bir korumalı alan, silinmeden önce boşaltılmalıdır.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3271"/>
        <source>The sandbox name can contain only letters, digits and underscores which are displayed as spaces.</source>
        <translation>Korumalı alan adı yalnızca harf, rakam ve alt çizgi içerebilir.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="410"/>
        <source>&amp;Maintenance</source>
        <translation>&amp;Bakım</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3270"/>
        <source>The sandbox name can not be a device name.</source>
        <translation>Korumalı alan adı bir cihaz adı olamaz.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3324"/>
        <source>Operation failed for %1 item(s).</source>
        <translation>%1 öge için işlem başarısız oldu.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="563"/>
        <location filename="SandMan.cpp" line="705"/>
        <source>Global Settings</source>
        <translation>Genel Ayarlar</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="468"/>
        <location filename="SandMan.cpp" line="684"/>
        <source>&amp;Sandbox</source>
        <translation>&amp;Korumalı Alan</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="792"/>
        <location filename="SandMan.cpp" line="930"/>
        <location filename="SandMan.cpp" line="931"/>
        <source>Cleanup</source>
        <translation>Temizle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="412"/>
        <source>Disconnect</source>
        <translation>Bağlantıyı Kes</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="411"/>
        <source>Connect</source>
        <translation>Bağlan</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2528"/>
        <source>Only Administrators can change the config.</source>
        <translation>Yalnızca Yöneticiler yapılandırmayı değiştirebilir.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3282"/>
        <source>Snapshot not found</source>
        <translation>Anlık görüntü bulunamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="414"/>
        <source>Stop All</source>
        <translation>Tümünü Durdur</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3273"/>
        <source>Delete protection is enabled for the sandbox</source>
        <translation>Korumalı alan için silme koruması etkinleştirilmiş</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="415"/>
        <source>&amp;Advanced</source>
        <translation>&amp;Gelişmiş</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2800"/>
        <source>Executing maintenance operation, please wait...</source>
        <translation>Bakım işlemi yapılıyor, lütfen bekleyin...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="423"/>
        <source>Stop Service</source>
        <translation>Hizmeti Durdur</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="469"/>
        <location filename="SandMan.cpp" line="942"/>
        <location filename="SandMan.cpp" line="943"/>
        <source>Create New Box</source>
        <translation>Yeni Alan Oluştur</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3272"/>
        <source>Failed to terminate all processes</source>
        <translation>Tüm işlemler sonlandırılamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="436"/>
        <source>Advanced View</source>
        <translation>Gelişmiş Görünüm</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3268"/>
        <source>Failed to delete sandbox %1: %2</source>
        <translation>%1: %2 Korumalı alanı silinemedi</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2376"/>
        <source>PID %1: </source>
        <translation>PID %1: </translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="474"/>
        <location filename="SandMan.cpp" line="615"/>
        <source>Terminate All Processes</source>
        <translation>Tüm İşlemleri Sonlandır</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2538"/>
        <source>Please enter the configuration password.</source>
        <translation>Lütfen yapılandırma parolasını girin.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3287"/>
        <source>You are not authorized to update configuration in section &apos;%1&apos;</source>
        <translation>Bölümdeki yapılandırmayı güncelleme yetkiniz yok &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3283"/>
        <source>Error merging snapshot directories &apos;%1&apos; with &apos;%2&apos;, the snapshot has not been fully merged.</source>
        <translation>&apos;%1&apos; ve &apos;%2&apos; anlık görüntü dizinleri birleştirilirken hata oluştu, anlık görüntü tam olarak birleştirilmedi.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="538"/>
        <source>Cleanup Message Log</source>
        <translation>Mesaj Günlüğünü Temizle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="460"/>
        <source>About the Qt Framework</source>
        <translation>Qt Framework Hakkında</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="542"/>
        <source>Keep terminated</source>
        <translation>Sonlandırılmışları Tut</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3267"/>
        <source>A sandbox of the name %1 already exists</source>
        <translation>%1 adında bir korumalı alan zaten var</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3288"/>
        <source>Failed to set configuration setting %1 in section %2: %3</source>
        <translation>%2: %3 bölümünde %1 yapılandırma parametresi ayarlanamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="597"/>
        <location filename="SandMan.cpp" line="715"/>
        <source>Reset all hidden messages</source>
        <translation>Tüm Gizlenmiş Mesajları Sıfırla</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="471"/>
        <source>Import Box</source>
        <translation>Alanı İçe Aktar</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="572"/>
        <source>Disable File Recovery</source>
        <translation>Dosya Kurtarmayı Devre Dışı Bırak</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="575"/>
        <source>Disable Message Popup</source>
        <translation>Açılır Mesaj Penceresini Devre Dışı Bırak</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="691"/>
        <source>Import Sandbox</source>
        <translation>Korumalı Alanı İçe Aktar</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1023"/>
        <source>Click to download update</source>
        <translation>Güncellemeyi indirmek için tıklayın</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1856"/>
        <source>Removed Shortcut: %1</source>
        <translation>Kaldırılan Kısayol: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1935"/>
        <source>Updated Shortcut to: %1</source>
        <translation>Kısayol şu şekilde güncellendi: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1937"/>
        <source>Added Shortcut to: %1</source>
        <translation>Eklenen Kısayol: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1988"/>
        <source>Auto removing sandbox %1</source>
        <translation>%1 korumalı alanı otomatik olarak kaldırıyor</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>%1 Directory: %2</source>
        <translation>%1 Dizini: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>Application</source>
        <translation>Uygulama</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>Installation</source>
        <translation>Kurulum</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2069"/>
        <source> for Personal use</source>
        <translation> Kişisel kullanım için</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2071"/>
        <source>   -   for Non-Commercial use ONLY</source>
        <translation>   -   Ticari Olmayan kullanım için</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2140"/>
        <source>   -   NOT connected</source>
        <translation>   -   Bağlı DEĞİL</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2475"/>
        <source>The evaluation period has expired!!!</source>
        <translation>Değerlendirme süresi dolmuştur!</translation>
    </message>
    <message>
        <source>Select file name</source>
        <translation type="vanished">Dosya adı seçin</translation>
    </message>
    <message>
        <source>7-zip Archive (*.7z)</source>
        <translation type="vanished">7-zip Arşivi (*.7z)</translation>
    </message>
    <message>
        <source>This name is already in use, please select an alternative box name</source>
        <translatorcomment>Sandbox için Korumalı Alan kullanıldığından dolayı box için de alan kullandım.</translatorcomment>
        <translation type="vanished">Bu ad zaten kullanılıyor, lütfen alternatif bir alan adı seçin</translation>
    </message>
    <message>
        <source>Importing: %1</source>
        <translation type="vanished">İçe aktarılıyor: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2610"/>
        <source>No Recovery</source>
        <translation>Kurtarma Yok</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2616"/>
        <source>No Messages</source>
        <translation>Mesaj Yok</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2672"/>
        <source>&lt;b&gt;ERROR:&lt;/b&gt; The Sandboxie-Plus Manager (SandMan.exe) does not have a valid signature (SandMan.exe.sig). Please download a trusted release from the &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get&quot;&gt;official Download page&lt;/a&gt;.</source>
        <translation>&lt;b&gt;HATA:&lt;/b&gt; Sandboxie-Plus Yöneticisinin (SandMan.exe) geçerli bir imzaya sahip değil (SandMan.exe.sig). Lütfen &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get&quot;&gt;resmi İndirme sayfasından&lt;/a&gt; güvenilir bir sürüm indirin.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2780"/>
        <source>Maintenance operation completed</source>
        <translation>Bakım işlemi tamamlandı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2911"/>
        <source>In the Plus UI, this functionality has been integrated into the main sandbox list view.</source>
        <translation>Bu işlevsellik, Plus kullanıcı arayüzünde korumalı alan liste görünümüne entegre edilmiştir.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2912"/>
        <source>Using the box/group context menu, you can move boxes and groups to other groups. You can also use drag and drop to move the items around. Alternatively, you can also use the arrow keys while holding ALT down to move items up and down within their group.&lt;br /&gt;You can create new boxes and groups from the Sandbox menu.</source>
        <translation>Alan/grup bağlam menüsünü kullanarak alanları ve grupları diğer gruplara taşıyabilirsiniz. Öğeleri hareket ettirmek için sürükle ve bırak özelliğini de kullanabilirsiniz. Alternatif olarak, öğeleri grupları içinde yukarı ve aşağı taşımak için ALT tuşunu basılı tutarken ok tuşlarını da kullanabilirsiniz.&lt;br /&gt;Korumalı Alan menüsünden yeni alanlar ve gruplar oluşturabilirsiniz.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2980"/>
        <source>Do you also want to reset hidden message boxes (yes), or only all log messages (no)?</source>
        <translation>Gizlenmiş mesaj kutuları dahil her şeyi (evet) veya yalnızca tüm günlük mesajlarını (hayır) sıfırlamak mı istiyorsunuz?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3061"/>
        <source>You are about to edit the Templates.ini, this is generally not recommended.
This file is part of Sandboxie and all change done to it will be reverted next time Sandboxie is updated.</source>
        <translation>Templates.ini dosyasını düzenlemek üzeresiniz, bu genellikle önerilmez. Bu dosya Sandboxie&apos;nin bir parçasıdır ve üzerinde yapılan tüm değişiklikler Sandboxie güncellendiğinde kaybolacaktır.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3077"/>
        <source>The changes will be applied automatically whenever the file gets saved.</source>
        <translation>Dosya her kaydedildiğinde değişiklikler otomatik olarak uygulanacaktır.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3260"/>
        <source>Administrator rights are required for this operation.</source>
        <translation>Bu işlem için yönetici hakları gereklidir.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3261"/>
        <source>Failed to execute: %1</source>
        <translation>%1 çalıştırılamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3262"/>
        <source>Failed to connect to the driver</source>
        <translation>Sürücüye bağlanılamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3263"/>
        <source>Failed to communicate with Sandboxie Service: %1</source>
        <translation>Sandboxie Hizmeti ile iletişim kurulamadı: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3264"/>
        <source>An incompatible Sandboxie %1 was found. Compatible versions: %2</source>
        <translation>Uyumsuz bir Sandboxie %1 bulundu. Uyumlu sürümler: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3265"/>
        <source>Can&apos;t find Sandboxie installation path.</source>
        <translation>Sandboxie kurulum yolu bulunamıyor.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3269"/>
        <source>The sandbox name can not be longer than 32 characters.</source>
        <translation>Korumalı alan adı 32 karakterden uzun olamaz.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3279"/>
        <source>This Snapshot operation can not be performed while processes are still running in the box.</source>
        <translation>Bu Anlık Görüntü işlemi, alan içinde işlemler çalışırken gerçekleştirilemez.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3285"/>
        <source>Can&apos;t remove a snapshot that is shared by multiple later snapshots</source>
        <translation>Birden çok anlık görüntü tarafından paylaşılan bir anlık görüntü kaldırılamaz</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3294"/>
        <source>Import/Export not available, 7z.dll could not be loaded</source>
        <translation>İçe/Dışa Aktarma kullanılamıyor, 7z.dll yüklenemedi</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3295"/>
        <source>Failed to create the box archive</source>
        <translation>Alan arşivi oluşturulamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3296"/>
        <source>Failed to open the 7z archive</source>
        <translation>7z arşivi açılamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3297"/>
        <source>Failed to unpack the box archive</source>
        <translation>Alan arşivi açılamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3298"/>
        <source>The selected 7z file is NOT a box archive</source>
        <translation>Seçili 7z dosyası bir alan arşivi DEĞİLDİR</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3355"/>
        <source>Do you want to open %1 in a sandboxed (yes) or unsandboxed (no) Web browser?</source>
        <translation>%1 bağlantısını korumalı (evet) veya korumasız (hayır) bir Web tarayıcısında mı açmak istiyorsunuz?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3356"/>
        <source>Remember choice for later.</source>
        <translation>Seçimi sonrası için hatırla.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3504"/>
        <source>Copy Cell</source>
        <translation>Hücreyi Kopyala</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3505"/>
        <source>Copy Row</source>
        <translation>Satırı Kopyala</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3506"/>
        <source>Copy Panel</source>
        <translation>Paneli Kopyala</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2771"/>
        <source>Failed to stop all Sandboxie components</source>
        <translation>Tüm Sandboxie bileşenleri durdurulamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2773"/>
        <source>Failed to start required Sandboxie components</source>
        <translation>Gerekli Sandboxie bileşenleri başlatılamadı</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="214"/>
        <source>The file %1 already exists, do you want to overwrite it?</source>
        <translation>%1 dosyası zaten var, üzerine yazmak istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="179"/>
        <location filename="SandManRecovery.cpp" line="215"/>
        <source>Do this for all files!</source>
        <translation>Bunu tüm dosyalara uygula!</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="441"/>
        <source>Always on Top</source>
        <translation>Her Zaman Üstte</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1711"/>
        <source>Cleaned up removed templates...</source>
        <translation>Kaldırılan şablonlar temizlendi...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3289"/>
        <source>Can not create snapshot of an empty sandbox</source>
        <translation>Boş bir korumalı alanın anlık görüntüsü oluşturulamaz</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3290"/>
        <source>A sandbox with that name already exists</source>
        <translation>Bu adda bir korumalı alan zaten var</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3503"/>
        <source>Reset Columns</source>
        <translation>Sütunları Sıfırla</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="510"/>
        <source>Show Hidden Boxes</source>
        <translation>Gizli Alanları Göster</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1694"/>
        <source>Some compatibility templates (%1) are missing, probably deleted, do you want to remove them from all boxes?</source>
        <translation>Bazı uyumluluk şablonları (%1) eksik, büyük olasılıkla silinmiş, bunları tüm alanlardan kaldırmak istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2575"/>
        <source>Do you want to terminate all processes in all sandboxes?</source>
        <translation>Tüm korumalı alanlardaki tüm işlemleri sonlandırmak istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2576"/>
        <source>Terminate all without asking</source>
        <translation>Hepsini sormadan sonlandır</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="88"/>
        <source>The selected window is running as part of program %1 in sandbox %2</source>
        <translation>Seçili pencere %2 korumalı alanında %1 programının bir parçası olarak çalışıyor</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="95"/>
        <source>The selected window is not running as part of any sandboxed program.</source>
        <translation>Seçili pencere, herhangi bir alandaki programın parçası olarak çalışmıyor.</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="134"/>
        <source>Drag the Finder Tool over a window to select it, then release the mouse to check if the window is sandboxed.</source>
        <translation>Soldaki Bulucu Aracını, farenin sol tuşuyla sürükleyip bir pencerenin üzerine bırakarak, o pencerenin korumalı alanda olup olmadığını bulabilirsiniz.</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="204"/>
        <source>Sandboxie-Plus - Window Finder</source>
        <translation>Sandboxie-Plus - Pencere Bulucu</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2100"/>
        <source>Default sandbox not found; creating: %1</source>
        <translation>Varsayılan korumalı alan bulunamadı; oluşturuluyor: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="539"/>
        <source>Cleanup Trace Log</source>
        <translation>İzleme Günlüğünü Temizle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1201"/>
        <source>Trace Log</source>
        <translation>İzleme Günlüğü</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="555"/>
        <source>Trace Logging</source>
        <translation>İzlemeleri Günlükle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1553"/>
        <source>Unknown operation &apos;%1&apos; requested via command line</source>
        <translation>Bilinmeyen işlem &apos;%1&apos; komut satırı aracılığıyla istendi</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2668"/>
        <source>CAUTION: Another agent (probably SbieCtrl.exe) is already managing this Sandboxie session, please close it first and reconnect to take over.</source>
        <translation>DİKKAT: Bu Sandboxie oturumunu başka bir aracı (muhtemelen SbieCtrl.exe) zaten yönetiyor, lütfen önce onu kapatın ve devralmak için yeniden bağlanın.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3291"/>
        <source>The config password must not be longer than 64 characters</source>
        <translation>Yapılandırma parolası 64 karakterden uzun olmamalıdır</translation>
    </message>
    <message>
        <location filename="main.cpp" line="123"/>
        <source>Sandboxie Manager can not be run sandboxed!</source>
        <translation>Sandboxie Yöneticisi korumalı alanda çalıştırılamaz!</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="470"/>
        <source>Create Box Group</source>
        <translation>Alan Grubu Oluştur</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="512"/>
        <source>Show All Sessions</source>
        <translation>Tüm Oturumları Göster</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3257"/>
        <source>Error Status: 0x%1 (%2)</source>
        <translation>Hata Durumu: 0x%1 (%2)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3258"/>
        <source>Unknown</source>
        <translation>Bilinmeyen</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3300"/>
        <source>Unknown Error Status: 0x%1</source>
        <translation>Bilinmeyen Hata Durumu: 0x%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1031"/>
        <source>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Support Sandboxie-Plus on Patreon&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Patreon&apos;da Sandboxie-Plus&apos;ı destekleyin&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1282"/>
        <source>Sandboxie-Plus was running in portable mode, now it has to clean up the created services. This will prompt for administrative privileges.

Do you want to do the clean up?</source>
        <translation>Sandboxie-Plus taşınabilir modda çalışıyordu, şimdi oluşturulan hizmetleri temizlemesi gerekiyor. Bunun için yönetici ayrıcalıkları isteyecektir.

Temizlik yapmak ister misin?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1452"/>
        <source>This box provides &lt;a href=&quot;sbie://docs/security-mode&quot;&gt;enhanced security isolation&lt;/a&gt;, it is suitable to test untrusted software.</source>
        <translation>Bu alan &lt;a href=&quot;sbie://docs/security-mode&quot;&gt;gelişmiş güvenlik yalıtımı&lt;/a&gt; sağlar, güvenilmeyen yazılımları test etmek için uygundur.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1456"/>
        <source>This box provides standard isolation, it is suitable to run your software to enhance security.</source>
        <translation>Bu alan standart yalıtım sağlar, güvenliği artırmak için yazılımınızı çalıştırmaya uygundur.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1460"/>
        <source>This box does not enforce isolation, it is intended to be used as an &lt;a href=&quot;sbie://docs/compartment-mode&quot;&gt;application compartment&lt;/a&gt; for software virtualization only.</source>
        <translation>Bu alan, yalıtımı zorla uygulamaz. Yalnızca yazılım sanallaştırması için &lt;a href=&quot;sbie://docs/compartment-mode&quot;&gt;uygulama bölmesi&lt;/a&gt; olarak kullanımı amaçlanmıştır.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1465"/>
        <source>&lt;br /&gt;&lt;br /&gt;This box &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;prevents access to all user data&lt;/a&gt; locations, except explicitly granted in the Resource Access options.</source>
        <translation>&lt;br /&gt;&lt;br /&gt;Bu alan, Kaynak Erişimi seçeneklerinde açıkça verilen konumlar dışında &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;tüm kullanıcı verileri&lt;/a&gt; konumlarına erişimi engeller.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1639"/>
        <source>No Force Process</source>
        <translation>Zorlanmış İşlem Yok</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2024"/>
        <source>Sandboxie-Plus was started in portable mode, do you want to put the Sandbox folder into its parent directory?
Yes will choose: %1
No will choose: %2</source>
        <translation>Sandboxie-Plus taşınabilir modda başlatıldı, Korumalı Alan klasörünü ana dizine koymak istiyor musunuz?
Evet şunları seçer: %1
Hayır şunları seçer: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2407"/>
        <source>The selected feature set is only available to project supporters. Processes started in a box with this feature set enabled without a supporter certificate will be terminated after 5 minutes.&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Become a project supporter&lt;/a&gt;, and receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;</source>
        <translation>Seçili özellik seti yalnızca proje destekçileri tarafından kullanılabilir. Bu özellik setinin destekçi sertifikası olmadan etkinleştirildiği bir alanda başlatılan işlemler 5 dakika sonra sonlandırılacaktır.&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get- cert&quot;&gt;Proje destekçisi olun&lt;/a&gt; ve bir &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;destekçi sertifikası&lt;/a&gt; alın</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3569"/>
        <source>This copy of Sandboxie+ is certified for: %1</source>
        <translation>Sandboxie+&apos;nın bu kopyası şu kişiler için sertifikalandırılmıştır: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3571"/>
        <source>Sandboxie+ is free for personal and non-commercial use.</source>
        <translation>Sandboxie+, kişisel ve ticari olmayan kullanım için ücretsizdir.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3574"/>
        <source>Sandboxie-Plus is an open source continuation of Sandboxie.&lt;br /&gt;Visit &lt;a href=&quot;https://sandboxie-plus.com&quot;&gt;sandboxie-plus.com&lt;/a&gt; for more information.&lt;br /&gt;&lt;br /&gt;%3&lt;br /&gt;&lt;br /&gt;Driver version: %1&lt;br /&gt;Features: %2&lt;br /&gt;&lt;br /&gt;Icons from &lt;a href=&quot;https://icons8.com&quot;&gt;icons8.com&lt;/a&gt;</source>
        <translation>Sandboxie-Plus, Sandboxie&apos;nin açık kaynaklı bir devamıdır.&lt;br /&gt;Daha fazla bilgi için &lt;a href=&quot;https://sandboxie-plus.com&quot;&gt;sandboxie-plus.com&lt;/a&gt; adresini ziyaret edin.&lt;br /&gt;&lt;br /&gt;%3&lt;br /&gt;&lt;br /&gt;Sürücü versiyonu: %1&lt;br /&gt;Özellikler: %2&lt;br /&gt;&lt;br /&gt;Simgeler için &lt;a href=&quot;https://icons8.com&quot;&gt;icons8.com&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="429"/>
        <source>Uninstall All</source>
        <translation>Tümünü Kaldır</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="437"/>
        <source>Vintage View (like SbieCtrl)</source>
        <translation>Eski Görünüm (SbieCtrl gibi)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="454"/>
        <source>Contribute to Sandboxie-Plus</source>
        <translation>Sandboxie-Plus&apos;a Katkıda Bulunun</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="473"/>
        <location filename="SandMan.cpp" line="614"/>
        <source>Run Sandboxed</source>
        <translation>Korumalı Alanda Çalıştır</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="476"/>
        <location filename="SandMan.cpp" line="628"/>
        <source>Is Window Sandboxed?</source>
        <translation>Pencere Korumalı Alanda Mı?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="521"/>
        <source>Show File Panel</source>
        <translation>Dosya Panelini Göster</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="540"/>
        <source>Cleanup Recovery Log</source>
        <translation>Kurtarma Günlüğünü Temizle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="598"/>
        <location filename="SandMan.cpp" line="716"/>
        <source>Reset all GUI options</source>
        <translation>Tüm Arayüz Seçeneklerini Sıfırla</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="585"/>
        <location filename="SandMan.cpp" line="723"/>
        <location filename="SandMan.cpp" line="958"/>
        <location filename="SandMan.cpp" line="959"/>
        <source>Edit Sandboxie.ini</source>
        <translation>Sandboxie.ini&apos;yi Düzenle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="588"/>
        <source>Edit Templates.ini</source>
        <translation>Templates.ini&apos;yi Düzenle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="590"/>
        <source>Edit Sandboxie-Plus.ini</source>
        <translation>Sandboxie-Plus.ini&apos;yi Düzenle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="594"/>
        <location filename="SandMan.cpp" line="726"/>
        <source>Reload configuration</source>
        <translation>Yapılandırmayı Yeniden Yükle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="613"/>
        <source>&amp;File</source>
        <translation>&amp;Dosya</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="629"/>
        <source>Resource Access Monitor</source>
        <translation>Kaynak Erişim İzleyicisi</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="662"/>
        <source>Programs</source>
        <translation>Programlar</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="663"/>
        <source>Files and Folders</source>
        <translation>Dosyalar ve Klasörler</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="689"/>
        <source>Create New Sandbox</source>
        <translation>Yeni Korumalı Alan Oluştur</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="690"/>
        <source>Create New Group</source>
        <translation>Yeni Grup Oluştur</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="693"/>
        <source>Set Container Folder</source>
        <translation>Kapsayıcı Klasörünü Ayarla</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="696"/>
        <source>Set Layout and Groups</source>
        <translation>Düzeni ve Grupları Ayarla</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="698"/>
        <source>Reveal Hidden Boxes</source>
        <translation>Gizli Alanları Göster</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="704"/>
        <source>&amp;Configure</source>
        <translation>&amp;Yapılandır</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="708"/>
        <source>Program Alerts</source>
        <translation>Program Uyarıları</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="710"/>
        <source>Windows Shell Integration</source>
        <translation>Windows Kabuk Entegrasyonu</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="712"/>
        <source>Software Compatibility</source>
        <translation>Yazılım Uyumluluğu</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="721"/>
        <source>Lock Configuration</source>
        <translation>Yapılandırmayı Kilitle</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="740"/>
        <source>Sandbox %1</source>
        <translation>Korumalı Alan %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="783"/>
        <source>New-Box Menu</source>
        <translation>Yeni Alan Menüsü</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="800"/>
        <source>Edit-ini Menu</source>
        <translation>Ini Düzenleme Menüsü</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="844"/>
        <source>Toolbar Items</source>
        <translation>Araç Çubuğu Öğeleri</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="846"/>
        <source>Reset Toolbar</source>
        <translation>Araç Çubuğunu Sıfırla</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1015"/>
        <source>Click to run installer</source>
        <translation>Yükleyiciyi çalıştırmak için tıklayın</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1019"/>
        <source>Click to apply update</source>
        <translation>Güncellemeyi uygulamak için tıklayın</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1211"/>
        <source>Time|Box Name|File Path</source>
        <translation>Zaman|Alan Adı|Dosya Yolu</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="559"/>
        <location filename="SandMan.cpp" line="667"/>
        <location filename="SandMan.cpp" line="1221"/>
        <source>Recovery Log</source>
        <translation>Kurtarma Günlüğü</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1014"/>
        <source>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus release %1 ready&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;Yeni bir Sandboxie-Plus sürümü %1 hazır&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1018"/>
        <source>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update %1 ready&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;Yeni bir Sandboxie-Plus güncellemesi %1 hazır&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1022"/>
        <source>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update v%1 available&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;Yeni bir Sandboxie-Plus güncellemesi v%1 mevcut&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1763"/>
        <source>Executing OnBoxDelete: %1</source>
        <translation>AlandaSilme yürütülüyor: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1969"/>
        <source>Auto deleting content of %1</source>
        <translation>%1 içeriği otomatik olarak siliniyor</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2004"/>
        <source>Sandboxie-Plus Version: %1 (%2)</source>
        <translation>Sandboxie-Plus Sürümü: %1 (%2)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2006"/>
        <source>Data Directory: %1</source>
        <translation>Veri Dizini: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2285"/>
        <location filename="SandMan.cpp" line="2287"/>
        <source> (%1)</source>
        <translation> (%1)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2338"/>
        <source>The program %1 started in box %2 will be terminated in 5 minutes because the box was configured to use features exclusively available to project supporters.</source>
        <translation>%2 alanında başlatılan %1 programı, alan yalnızca proje destekçilerine sunulan özellikleri kullanacak şekilde yapılandırıldığından 5 dakika içinde sonlandırılacak.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2340"/>
        <source>The box %1 is configured to use features exclusively available to project supporters, these presets will be ignored.</source>
        <translation>%1 alanı, yalnızca proje destekçilerine sunulan özellikleri kullanacak şekilde yapılandırılmıştır, bu ön ayarlar yok sayılacaktır.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2341"/>
        <source>&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Become a project supporter&lt;/a&gt;, and receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;</source>
        <translation>&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Proje destekçisi olun&lt;/a&gt; ve bir &lt;a href=&quot;https kazanın ://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;destekçi sertifikası&lt;/a&gt;alın</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2595"/>
        <source>Please enter the duration, in seconds, for disabling Forced Programs rules.</source>
        <translation>Lütfen Zorunlu Programlar kurallarını devre dışı bırakmak için süreyi saniye cinsinden girin.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2775"/>
        <source>Maintenance operation failed (%1)</source>
        <translation>Bakım işlemi başarısız oldu (%1)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3274"/>
        <source>All sandbox processes must be stopped before the box content can be deleted</source>
        <translation>Alan içeriği silinmeden önce tüm korumalı alan işlemleri durdurulmalıdır</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3281"/>
        <source>Failed to copy box data files</source>
        <translation>Alan veri dosyaları kopyalanamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3286"/>
        <source>Failed to remove old box data files</source>
        <translation>Eski alan veri dosyaları kaldırılamadı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3292"/>
        <source>The operation was canceled by the user</source>
        <translation>İşlem kullanıcı tarafından iptal edildi</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3507"/>
        <source>Case Sensitive</source>
        <translation>Harfe Duyarlı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3508"/>
        <source>RegExp</source>
        <translation>Düzİfa</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3509"/>
        <source>Highlight</source>
        <translation>Vurgula</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3510"/>
        <source>Close</source>
        <translation>Kapat</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3511"/>
        <source>&amp;Find ...</source>
        <translation>&amp;Bul ...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3512"/>
        <source>All columns</source>
        <translation>Tüm Sütunlar</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3561"/>
        <source>&lt;h3&gt;About Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;Version %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2023 by DavidXanatos&lt;/p&gt;</source>
        <translation>&lt;h3&gt;Sandboxie-Plus hakkında&lt;/h3&gt;&lt;p&gt;Sürüm %1&lt;/p&gt;&lt;p&gt;Telif hakkı (c) 2020-2023, DavidXanatos&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2481"/>
        <source>The supporter certificate is not valid for this build, please get an updated certificate</source>
        <translation>Destekçi sertifikası bu derleme için geçerli değil, lütfen yenilenmiş bir sertifika alın</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2484"/>
        <source>The supporter certificate has expired%1, please get an updated certificate</source>
        <translation>Destekçi sertifikasının süresi doldu%1, lütfen yenilenmiş bir sertifika alın</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2485"/>
        <source>, but it remains valid for the current build</source>
        <translation>, ancak mevcut derleme için geçerli kalır</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2487"/>
        <source>The supporter certificate will expire in %1 days, please get an updated certificate</source>
        <translation>Destekçi sertifikasının süresi %1 gün içinde dolacak, lütfen yenilenmiş bir sertifika alın</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="165"/>
        <source>WARNING: Sandboxie-Plus.ini in %1 cannot be written to, settings will not be saved.</source>
        <translation>UYARI: %1 içindeki Sandboxie-Plus.ini&apos;ye yazılamıyor, ayarlar kaydedilmeyecek.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="566"/>
        <location filename="SandMan.cpp" line="568"/>
        <location filename="SandMan.cpp" line="616"/>
        <location filename="SandMan.cpp" line="618"/>
        <source>Pause Forcing Programs</source>
        <translation>Programları Zorlamayı Duraklat</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="426"/>
        <source>Setup Wizard</source>
        <translation>Kurulum Sihirbazı</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="529"/>
        <source>Refresh View</source>
        <translation>Görünümü Yenile</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1032"/>
        <source>Click to open web browser</source>
        <translation>Web tarayıcısını açmak için tıklayın</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="105"/>
        <source>Dismiss Update Notification</source>
        <translation>Güncelleme Bildirimini Kapat</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="181"/>
        <source> - Driver/Service NOT Running!</source>
        <translation> - Sürücü/Hizmet ÇALIŞMIYOR!</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="183"/>
        <source> - Deleting Sandbox Content</source>
        <translation> - Korumalı Alan İçeriğini Silme</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1784"/>
        <source>Auto Deleting %1 Content</source>
        <translation>%1 İçeriği Otomatik Siliniyor</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2005"/>
        <source>Current Config: %1</source>
        <translation>Geçerli Yapılandırma: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3136"/>
        <source>Sandboxie config has been reloaded</source>
        <translation>Sandboxie yapılandırması yeniden yüklendi</translation>
    </message>
</context>
<context>
    <name>CSbieModel</name>
    <message>
        <location filename="Models/SbieModel.cpp" line="559"/>
        <source>Name</source>
        <translation>Ad</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="159"/>
        <source>Box Group</source>
        <translation>Alan Grubu</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="315"/>
        <source>Empty</source>
        <translation>Boş</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="561"/>
        <source>Status</source>
        <translation>Durum</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="560"/>
        <source>Process ID</source>
        <translation>İşlem Kimliği</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="567"/>
        <source>Path / Command Line</source>
        <translation>Yol / Komut Satırı</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="562"/>
        <source>Title</source>
        <translation>Başlık</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="563"/>
        <source>Info</source>
        <translation>Bilgi</translation>
    </message>
</context>
<context>
    <name>CSbieProcess</name>
    <message>
        <location filename="SbieProcess.cpp" line="99"/>
        <source>Terminated</source>
        <translation>Sonlandırılmış</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="103"/>
        <source>Running</source>
        <translation>Çalışıyor</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="59"/>
        <source>Sbie RpcSs</source>
        <translation>Sbie RpcSs</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="60"/>
        <source>Sbie DcomLaunch</source>
        <translation>Sbie DcomLaunch</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="61"/>
        <source>Sbie Crypto</source>
        <translation>Sbie Kripto</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="63"/>
        <source>Sbie BITS</source>
        <translation>Sbie BITS</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="64"/>
        <source>Sbie Svc</source>
        <translation>Sbie Svc</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="66"/>
        <source>Trusted Installer</source>
        <translation>Güvenilir Yükleyici</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="67"/>
        <source>Windows Update</source>
        <translation>Windows Güncelleme</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="68"/>
        <source>Windows Explorer</source>
        <translation>Windows Gezgini</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="69"/>
        <source>Internet Explorer</source>
        <translation>Internet Explorer</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="71"/>
        <source>Windows Media Player</source>
        <translatorcomment>Windows Media Player</translatorcomment>
        <translation>Windows Media Player</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="73"/>
        <source>KMPlayer</source>
        <translation>KMPlayer</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="74"/>
        <source>Windows Live Mail</source>
        <translation>Windows Live Mail</translation>
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
        <translation>Windows Ink Hizmetleri</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="80"/>
        <source>Chromium Based</source>
        <translation>Chromium Tabanlı</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="81"/>
        <source>Google Updater</source>
        <translation>Google Güncelleyici</translation>
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
        <location filename="SbieProcess.cpp" line="87"/>
        <source>Generic Web Browser</source>
        <translation>Genel Web Tarayıcısı</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="88"/>
        <source>Generic Mail Client</source>
        <translation>Genel Posta İstemcisi</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="109"/>
        <source> Elevated</source>
        <translation> Yetkilendirilmiş</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="111"/>
        <source> as System</source>
        <translation> Sistem olarak</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="120"/>
        <source> (%1)</source>
        <translation> (%1)</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="65"/>
        <source>MSI Installer</source>
        <translation>MSI Yükleyici</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="70"/>
        <source>Firefox</source>
        <translation>Firefox</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="72"/>
        <source>Winamp</source>
        <translation>Winamp</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="86"/>
        <source>Firefox Plugin Container</source>
        <translation>Firefox Eklenti Konteyneri</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="62"/>
        <source>Sbie WuauServ</source>
        <translation>Sbie WuauServ</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="114"/>
        <source> in session %1</source>
        <translation> %1 oturumunda</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="89"/>
        <source>Thunderbird</source>
        <translation>Thunderbird</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="105"/>
        <source>Forced </source>
        <translation>Zorunlu </translation>
    </message>
</context>
<context>
    <name>CSbieView</name>
    <message>
        <location filename="Views/SbieView.cpp" line="161"/>
        <location filename="Views/SbieView.cpp" line="291"/>
        <source>Run</source>
        <translation>Çalıştır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1514"/>
        <location filename="Views/SbieView.cpp" line="1566"/>
        <source>Create Shortcut to sandbox %1</source>
        <translation>%1 korumalı alanına kısayol oluştur</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="540"/>
        <source>Options:
    </source>
        <translation>Seçenekler:
    </translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="216"/>
        <source>Drop Admin Rights</source>
        <translation>Yönetici Haklarını Bırak</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="397"/>
        <source>Remove Group</source>
        <translation>Grubu Kaldır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="208"/>
        <source>Sandbox Options</source>
        <translation>Korumalı Alan Seçenekleri</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="213"/>
        <source>Sandbox Presets</source>
        <translation>Korumalı Alan Ön Ayarları</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="252"/>
        <location filename="Views/SbieView.cpp" line="354"/>
        <source>Remove Sandbox</source>
        <translation>Korumalı Alanı Kaldır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="240"/>
        <location filename="Views/SbieView.cpp" line="342"/>
        <source>Rename Sandbox</source>
        <translation>Korumalı Alanı Yeniden Adlandır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="167"/>
        <source>Run from Start Menu</source>
        <translation>Başlat Menüsünden Çalıştır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="260"/>
        <source>Preset</source>
        <translation>Ön Ayar</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1059"/>
        <source>Please enter a new group name</source>
        <translation>Lütfen yeni bir grup adı girin</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="748"/>
        <source>[None]</source>
        <translation>[Yok]</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1344"/>
        <source>Please enter a new name for the Sandbox.</source>
        <translation>Lütfen Korumalı Alan için yeni bir ad girin.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="206"/>
        <location filename="Views/SbieView.cpp" line="323"/>
        <source>Delete Content</source>
        <translation>İçeriği Sil</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="166"/>
        <source>Run Program</source>
        <translation>Program Çalıştır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="538"/>
        <source>    IPC root: %1
</source>
        <translation>    IPC kökü: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="263"/>
        <source>Block and Terminate</source>
        <translation>Engelle ve Sonlandır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="537"/>
        <source>    Registry root: %1
</source>
        <translation>    Kayıt kökü: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="536"/>
        <source>    File root: %1
</source>
        <translation>    Dosya kökü: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="257"/>
        <location filename="Views/SbieView.cpp" line="376"/>
        <source>Terminate</source>
        <translation>Sonlandır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="273"/>
        <source>Set Leader Process</source>
        <translation>Lider İşlem Olarak Ayarla</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="194"/>
        <source>Terminate All Programs</source>
        <translation>Tüm Programları Sonlandır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="857"/>
        <source>Do you really want to remove the selected group(s)?</source>
        <translation>Seçili grup(lar)ı gerçekten kaldırmak istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="296"/>
        <source>Run Web Browser</source>
        <translation>Web Tarayıcısını Çalıştır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="227"/>
        <source>Allow Network Shares</source>
        <translation>Ağ Paylaşımlarına İzin Ver</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="204"/>
        <location filename="Views/SbieView.cpp" line="331"/>
        <source>Snapshots Manager</source>
        <translation>Anlık Görüntü Yöneticisi</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="225"/>
        <source>Block Internet Access</source>
        <translation>İnternet Erişimini Engelle</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="271"/>
        <source>Set Linger Process</source>
        <translation>Oyalayıcı İşlem Olarak Ayarla</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="151"/>
        <location filename="Views/SbieView.cpp" line="281"/>
        <source>Create New Box</source>
        <translation>Yeni Alan Oluştur</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="100"/>
        <location filename="Views/SbieView.cpp" line="261"/>
        <source>Pin to Run Menu</source>
        <translation>Çalıştır Menüsüne Sabitle</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="205"/>
        <source>Recover Files</source>
        <translation>Dosyaları Kurtar</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="202"/>
        <location filename="Views/SbieView.cpp" line="324"/>
        <source>Explore Content</source>
        <translation>İçeriği Keşfet</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="102"/>
        <location filename="Views/SbieView.cpp" line="200"/>
        <location filename="Views/SbieView.cpp" line="259"/>
        <location filename="Views/SbieView.cpp" line="339"/>
        <location filename="Views/SbieView.cpp" line="378"/>
        <source>Create Shortcut</source>
        <translation>Kısayol Oluştur</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="267"/>
        <source>Allow internet access</source>
        <translation>İnternet Erişimine İzin Ver</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="269"/>
        <source>Force into this sandbox</source>
        <translation>Bu Korumalı Alana Zorla</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1589"/>
        <source>This box does not have Internet restrictions in place, do you want to enable them?</source>
        <translation>Bu alanda İnternet kısıtlamaları yok, bunları etkinleştirmek istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1145"/>
        <location filename="Views/SbieView.cpp" line="1215"/>
        <location filename="Views/SbieView.cpp" line="1462"/>
        <source>Don&apos;t show this message again.</source>
        <translation>Bu mesajı bir daha gösterme.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1403"/>
        <source>This Sandbox is already empty.</source>
        <translation>Bu Korumalı Alan zaten boş.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1413"/>
        <source>Do you want to delete the content of the selected sandbox?</source>
        <translation>Seçili korumalı alanın içeriğini silmek istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1444"/>
        <source>Do you want to terminate all processes in the selected sandbox(es)?</source>
        <translation>Seçili alan(lar)daki tüm işlemleri sonlandırmak istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1673"/>
        <source>This sandbox is disabled, do you want to enable it?</source>
        <translation>Bu alan devre dışı, etkinleştirmek istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1206"/>
        <location filename="Views/SbieView.cpp" line="1226"/>
        <location filename="Views/SbieView.cpp" line="1630"/>
        <source>This Sandbox is empty.</source>
        <translation>Bu Korumalı Alan boş.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="942"/>
        <source>A group can not be its own parent.</source>
        <translation>Bir grup kendi ebeveyni olamaz.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="215"/>
        <source>Ask for UAC Elevation</source>
        <translation>UAC Yetkilendirmesi İste</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="217"/>
        <source>Emulate Admin Rights</source>
        <translation>Yönetici Haklarını Taklit Et</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="178"/>
        <source>Default Web Browser</source>
        <translation>Varsayılan Web Tarayıcısı</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="179"/>
        <source>Default eMail Client</source>
        <translation>Varsayılan e-Posta İstemcisi</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="181"/>
        <source>Windows Explorer</source>
        <translation>Windows Gezgini</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="182"/>
        <source>Registry Editor</source>
        <translation>Kayıt Defteri Düzenleyicisi</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="183"/>
        <source>Programs and Features</source>
        <translation>Programlar ve Özellikler</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="152"/>
        <location filename="Views/SbieView.cpp" line="282"/>
        <source>Create Box Group</source>
        <translation>Alan Grubu Oluştur</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="395"/>
        <source>Rename Group</source>
        <translation>Grubu Yeniden Adlandır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="186"/>
        <source>Command Prompt</source>
        <translation>Komut İstemi</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="187"/>
        <source>Command Prompt (as Admin)</source>
        <translation>Komut İstemi (Yönetici olarak)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="191"/>
        <source>Command Prompt (32-bit)</source>
        <translation>Komut İstemi (32-bit)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="176"/>
        <source>Execute Autorun Entries</source>
        <translation>Otomatik Çalıştırma Girişlerini Yürüt</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="197"/>
        <source>Browse Files</source>
        <translation>Dosyalara Gözat</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="241"/>
        <location filename="Views/SbieView.cpp" line="343"/>
        <source>Move Sandbox</source>
        <translation>Korumalı Alanı Taşı</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="330"/>
        <source>Browse Content</source>
        <translation>İçeriğe Göz At</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="196"/>
        <source>Box Content</source>
        <translation>Alan İçeriği</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="203"/>
        <source>Open Registry</source>
        <translation>Kayıt Defterini Aç</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="199"/>
        <location filename="Views/SbieView.cpp" line="338"/>
        <source>Refresh Info</source>
        <translation>Bilgileri Yenile</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="153"/>
        <location filename="Views/SbieView.cpp" line="283"/>
        <source>Import Box</source>
        <translation>Alanı İçe Aktar</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="170"/>
        <location filename="Views/SbieView.cpp" line="302"/>
        <source>(Host) Start Menu</source>
        <translation>(Ana Bilgisayar) Başlat Menüsü</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="231"/>
        <source>Immediate Recovery</source>
        <translation>Anında Kurtarma</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="233"/>
        <source>Disable Force Rules</source>
        <translation>Zorlama Kurallarını Kapat</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="236"/>
        <location filename="Views/SbieView.cpp" line="329"/>
        <source>Sandbox Tools</source>
        <translation>Korumalı Alan Araçları</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="237"/>
        <source>Duplicate Box Config</source>
        <translation>Alan Yapılandırmasını Çoğalt</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="238"/>
        <source>Export Box</source>
        <translation>Alanı Dışa Aktar</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="242"/>
        <location filename="Views/SbieView.cpp" line="344"/>
        <source>Move Up</source>
        <translation>Yukarı Taşı</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="247"/>
        <location filename="Views/SbieView.cpp" line="349"/>
        <source>Move Down</source>
        <translation>Aşağı Taşı</translation>
    </message>
    <message>
        <source>Run Sandboxed</source>
        <translation type="vanished">Korumalı Alanda Çalıştır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="297"/>
        <source>Run eMail Reader</source>
        <translation>E-posta Okuyucuyu Çalıştır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="298"/>
        <source>Run Any Program</source>
        <translation>Herhangi Bir Program Çalıştır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="299"/>
        <source>Run From Start Menu</source>
        <translation>Başlat Menüsünden Çalıştır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="307"/>
        <source>Run Windows Explorer</source>
        <translation>Windows Gezginini Çalıştır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="321"/>
        <source>Terminate Programs</source>
        <translation>Programları Sonlandır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="322"/>
        <source>Quick Recover</source>
        <translation>Hızlı Kurtarma</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="327"/>
        <source>Sandbox Settings</source>
        <translation>Korumalı Alan Ayarları</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="334"/>
        <source>Duplicate Sandbox Config</source>
        <translation>Korumalı Alan Yapılandırmasını Çoğalt</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="335"/>
        <source>Export Sandbox</source>
        <translation>Korumalı Alanı Dışa Aktar</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="396"/>
        <source>Move Group</source>
        <translation>Grubu Taşı</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="847"/>
        <source>Please enter a new name for the Group.</source>
        <translation>Lütfen Grup için yeni bir ad girin.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="894"/>
        <source>Move entries by (negative values move up, positive values move down):</source>
        <translation>Girişleri hareket ettir (negatif değerler yukarı, pozitif değerler aşağı hareket eder):</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1031"/>
        <source>This name is already in use, please select an alternative box name</source>
        <translation>Bu ad zaten kullanılıyor, lütfen alan için alternatif bir ad seçin</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1045"/>
        <source>Importing: %1</source>
        <translation>İçe aktarılıyor: %1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1085"/>
        <source>The Sandbox name and Box Group name cannot use the &apos;,()&apos; symbol.</source>
        <translation>Korumalı Alan adı ve Alan Grubu adı &apos;,()&apos; sembolünü kullanamaz.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1018"/>
        <location filename="Views/SbieView.cpp" line="1329"/>
        <source>Select file name</source>
        <translation>Dosya adı seçin</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1018"/>
        <location filename="Views/SbieView.cpp" line="1329"/>
        <source>7-zip Archive (*.7z)</source>
        <translation>7-zip Arşivi (*.7z)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1337"/>
        <source>Exporting: %1</source>
        <translation>Dışa aktarılıyor: %1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1445"/>
        <location filename="Views/SbieView.cpp" line="1539"/>
        <source>Terminate without asking</source>
        <translation>Sormadan sonlandır</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1460"/>
        <source>The Sandboxie Start Menu will now be displayed. Select an application from the menu, and Sandboxie will create a new shortcut icon on your real desktop, which you can use to invoke the selected application under the supervision of Sandboxie.</source>
        <translation>Sandboxie Başlat Menüsü şimdi görüntülenecektir. Menüden bir uygulama seçtikten sonra gerçek masaüstünüzde o uygulamayı Sandboxie&apos;nin gözetiminde çalıştırmak için kullanabileceğiniz yeni bir kısayol simgesi oluşturulacaktır.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1538"/>
        <source>Do you want to terminate %1?</source>
        <translation>%1&apos;(y)i sonlandırmak istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1538"/>
        <source>the selected processes</source>
        <translation>Seçili İşlemler</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1290"/>
        <source>Please enter a new name for the duplicated Sandbox.</source>
        <translation>Lütfen kopyalanan Korumalı Alan için yeni bir ad girin.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1290"/>
        <source>%1 Copy</source>
        <translation>%1 Kopya</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="157"/>
        <location filename="Views/SbieView.cpp" line="287"/>
        <source>Stop Operations</source>
        <translation>İşlemleri Durdur</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="177"/>
        <source>Standard Applications</source>
        <translation>Standart Uygulamalar</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1233"/>
        <source>WARNING: The opened registry editor is not sandboxed, please be careful and only do changes to the pre-selected sandbox locations.</source>
        <translation>UYARI: Açılan kayıt defteri düzenleyicisi korumalı alanda değildir, lütfen dikkatli olun ve yalnızca önceden seçilmiş korumalı alan konumlarında değişiklik yapın.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1234"/>
        <source>Don&apos;t show this warning in future</source>
        <translation>Bu uyarıyı gelecekte gösterme</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1365"/>
        <source>Do you really want to remove the selected sandbox(es)?&lt;br /&gt;&lt;br /&gt;Warning: The box content will also be deleted!</source>
        <translation>Seçili korumalı alan(lar)ı gerçekten kaldırmak istiyor musunuz?&lt;br /&gt;&lt;br /&gt;Uyarı: Alan içeriği de silinecek!</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1414"/>
        <location filename="Views/SbieView.cpp" line="1418"/>
        <source>Also delete all Snapshots</source>
        <translation>Tüm Anlık Görüntüleri de sil</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1417"/>
        <source>Do you really want to delete the content of all selected sandboxes?</source>
        <translation>Seçili tüm korumalı alanların içeriğini gerçekten silmek istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1090"/>
        <source>This name is already used for a Box Group.</source>
        <translation>Bu ad zaten bir Alan Grubu için kullanılıyor.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1095"/>
        <source>This name is already used for a Sandbox.</source>
        <translation>Bu ad zaten bir Korumalı Alan için kullanılıyor.</translation>
    </message>
</context>
<context>
    <name>CSelectBoxWindow</name>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="89"/>
        <source>Sandboxie-Plus - Run Sandboxed</source>
        <translation>Sandboxie-Plus - Korumalı Alanda Çalıştır</translation>
    </message>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="206"/>
        <source>Are you sure you want to run the program outside the sandbox?</source>
        <translation>Programı korumalı alanın dışında çalıştırmak istediğinizden emin misiniz?</translation>
    </message>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="219"/>
        <source>Please select a sandbox.</source>
        <translation>Lütfen bir korumalı alan seçiniz.</translation>
    </message>
</context>
<context>
    <name>CSettingsWindow</name>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1536"/>
        <source>Please enter the new configuration password.</source>
        <translation>Lütfen yeni yapılandırma parolasını girin.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1440"/>
        <location filename="Windows/SettingsWindow.cpp" line="1576"/>
        <source>Select Directory</source>
        <translation>Dizin Seç</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1567"/>
        <source>Please enter a program file name</source>
        <translation>Lütfen bir program dosyası adı girin</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1557"/>
        <source>Folder</source>
        <translation>Klasör</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1557"/>
        <source>Process</source>
        <translation>İşlem</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="130"/>
        <source>Sandboxie Plus - Global Settings</source>
        <translation>Sandboxie Plus - Genel Ayarlar</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="496"/>
        <source>Search for settings</source>
        <translation>Ayarlarda ara</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="967"/>
        <source>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Plus features will be disabled in %1 days.&lt;/font&gt;</source>
        <translation>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Plus özellikleri %1 gün içinde devre dışı bırakılacak.&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="969"/>
        <source>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;For this build Plus features remain enabled.&lt;/font&gt;</source>
        <translation>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Bu derleme için Plus özellikleri etkin durumda kalır.&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="971"/>
        <source>&lt;br /&gt;Plus features are no longer enabled.</source>
        <translation>&lt;br /&gt;Plus özellikleri artık etkin değil.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1469"/>
        <source>&lt;a href=&quot;check&quot;&gt;Check Now&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;check&quot;&gt;Şimdi Denetle&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1540"/>
        <source>Please re-enter the new configuration password.</source>
        <translation>Lütfen yeni yapılandırma parolasını tekrar girin.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1545"/>
        <source>Passwords did not match, please retry.</source>
        <translation>Parolalar eşleşmedi, lütfen tekrar deneyin.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="216"/>
        <source>Auto Detection</source>
        <translation>Otomatik Algıla</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1395"/>
        <source>Thank you for supporting the development of Sandboxie-Plus.</source>
        <translation>Sandboxie-Plus&apos;ın gelişimini desteklediğiniz için teşekkür ederiz.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1402"/>
        <source>This support certificate is not valid.</source>
        <translation>Bu destek sertifikası geçerli değil.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="241"/>
        <source>Don&apos;t show any icon</source>
        <translation>Herhangi bir simge gösterme</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="242"/>
        <source>Show Plus icon</source>
        <translation>Plus simgesini göster</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="243"/>
        <source>Show Classic icon</source>
        <translation>Classic simgesini göster</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1390"/>
        <source>This certificate is unfortunately expired.</source>
        <translation>Bu sertifikanın ne yazık ki süresi dolmuş.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="217"/>
        <source>No Translation</source>
        <translation>Çeviri yok</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="237"/>
        <source>Don&apos;t integrate links</source>
        <translation>Kısayolları entegre etme</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="238"/>
        <source>As sub group</source>
        <translation>Alt grup olarak</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="239"/>
        <source>Fully integrate</source>
        <translation>Tamamen entegre et</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="245"/>
        <source>All Boxes</source>
        <translation>Tüm Alanlar</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="246"/>
        <source>Active + Pinned</source>
        <translation>Etkin + Sabitlenmiş</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="247"/>
        <source>Pinned Only</source>
        <translation>Yalnızca Sabitlenmiş</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="249"/>
        <source>None</source>
        <translation>Hiçbiri</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="250"/>
        <source>Native</source>
        <translation>Doğal</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="251"/>
        <source>Qt</source>
        <translation>Qt</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="253"/>
        <source>Ignore</source>
        <translation>Yok Say</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="265"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="345"/>
        <source>Browse for Program</source>
        <translation>Program için Göz At</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="412"/>
        <source>Add %1 Template</source>
        <translation>%1 Şablonu Ekle</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="597"/>
        <source>Please enter message</source>
        <translation>Lütfen mesaj giriniz</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="629"/>
        <source>Select Program</source>
        <translation>Program Seç</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="629"/>
        <source>Executables (*.exe *.cmd)</source>
        <translation>Yürütülebilir dosyalar (*.exe *.cmd)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="633"/>
        <location filename="Windows/SettingsWindow.cpp" line="646"/>
        <source>Please enter a menu title</source>
        <translation>Lütfen bir menü başlığı girin</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="642"/>
        <source>Please enter a command</source>
        <translation>Lütfen bir komut girin</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="719"/>
        <location filename="Windows/SettingsWindow.cpp" line="720"/>
        <location filename="Windows/SettingsWindow.cpp" line="730"/>
        <source>Run &amp;Sandboxed</source>
        <translation>&amp;Korumalı Alanda Çalıştır</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="254"/>
        <location filename="Windows/SettingsWindow.cpp" line="259"/>
        <source>Notify</source>
        <translation>Bildir</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="255"/>
        <location filename="Windows/SettingsWindow.cpp" line="260"/>
        <source>Download &amp; Notify</source>
        <translation>İndir &amp; Bildir</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="256"/>
        <location filename="Windows/SettingsWindow.cpp" line="261"/>
        <source>Download &amp; Install</source>
        <translation>İndir &amp; Yükle</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="964"/>
        <source>This supporter certificate has expired, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>Bu destekçi sertifikasının süresi doldu, lütfen &lt;a href=&quot;sbie://update/cert&quot;&gt;yenilenmiş bir sertifika alın&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="977"/>
        <source>This supporter certificate will &lt;font color=&apos;red&apos;&gt;expire in %1 days&lt;/font&gt;, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>Bu destekçi sertifikası &lt;font color=&apos;red&apos;&gt;%1 gün içinde sona erecek&lt;/font&gt;, lütfen &lt;a href=&quot;sbie://update/cert&quot;&gt;yenilenmiş bir sertifika alın&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1006"/>
        <source>Supporter certificate required</source>
        <translation>Destekçi sertifikası gereklidir</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1112"/>
        <source>Run &amp;Un-Sandboxed</source>
        <translation>&amp;Korumalı Alanın Dışında Çalıştır</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1392"/>
        <source>This certificate is unfortunately outdated.</source>
        <translation>Bu sertifika maalesef güncel değil.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1635"/>
        <source>Please enter the template identifier</source>
        <translation>Lütfen şablon tanımlayıcısını girin</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1643"/>
        <source>Error: %1</source>
        <translation>Hata: %1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1668"/>
        <source>Do you really want to delete the selected local template(s)?</source>
        <translation>Seçili yerel şablonları gerçekten silmek istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1824"/>
        <source>%1 (Current)</source>
        <translation>%1 (Kullanılan)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="751"/>
        <source>Sandboxed Web Browser</source>
        <translation>Korumalı Web Tarayıcısı</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1371"/>
        <source>This does not look like a certificate. Please enter the entire certificate, not just a portion of it.</source>
        <translation>Bu bir sertifikaya benzemiyor. Lütfen sertifikanın sadece bir kısmını değil tamamını girin.</translation>
    </message>
</context>
<context>
    <name>CSetupWizard</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="29"/>
        <source>Setup Wizard</source>
        <translation>Kurulum Sihirbazı</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="40"/>
        <source>The decision you make here will affect which page you get to see next.</source>
        <translation>Burada vereceğiniz karar, bir sonraki adımda hangi sayfayı göreceğinizi etkileyecektir.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="43"/>
        <source>This help is likely not to be of any help.</source>
        <translation>Bu yardım muhtemelen herhangi bir yardım sağlamayacaktır.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="47"/>
        <source>Sorry, I already gave all the help I could.</source>
        <translation>Üzgünüm, elimden gelen tüm yardımı zaten yaptım.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="49"/>
        <source>Setup Wizard Help</source>
        <translation>Kurulum Sihirbazı Yardımı</translation>
    </message>
</context>
<context>
    <name>CShellPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="417"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; shell integration</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; kabuk entegrasyonunu yapılandırın</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="418"/>
        <source>Configure how Sandboxie-Plus should integrate with your system.</source>
        <translation>Sandboxie-Plus&apos;ın sisteminizle nasıl entegre olacağını yapılandırın.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="422"/>
        <source>Start UI with Windows</source>
        <translation>Windows başlangıcında kullanıcı arayüzünü başlat</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="427"/>
        <source>Add &apos;Run Sandboxed&apos; to the explorer context menu</source>
        <translation>Dosya gezgini bağlam menüsüne &apos;Korumalı Alanda Çalıştır&apos; seçeneği ekle</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="432"/>
        <source>Add desktop shortcut for starting Web browser under Sandboxie</source>
        <translation>Sandboxie altında Web tarayıcısını başlatmak için masaüstü kısayolu ekle</translation>
    </message>
</context>
<context>
    <name>CSnapshotsWindow</name>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="230"/>
        <source>Do you really want to delete the selected snapshot?</source>
        <translation>Seçili anlık görüntüyü gerçekten silmek istiyor musunuz?</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="173"/>
        <source>New Snapshot</source>
        <translation>Yeni Anlık Görüntü</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="38"/>
        <source>Snapshot</source>
        <translation>Anlık Görüntü</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="196"/>
        <source>Do you really want to switch the active snapshot? Doing so will delete the current state!</source>
        <translation>Aktif anlık görüntüyü gerçekten değiştirmek istiyor musunuz? Bunu yapmak mevcut durumu siler!</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="24"/>
        <source>%1 - Snapshots</source>
        <translation>%1 - Anlık görüntüler</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="173"/>
        <source>Please enter a name for the new Snapshot.</source>
        <translation>Lütfen yeni Anlık Görüntü için bir ad girin.</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="56"/>
        <source>Revert to empty box</source>
        <translation>Boş alan haline geri dön</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="103"/>
        <source> (default)</source>
        <translation> (varsayılan)</translation>
    </message>
</context>
<context>
    <name>CStackView</name>
    <message>
        <location filename="Views/StackView.cpp" line="17"/>
        <source>#|Symbol</source>
        <translation>#|Sembol</translation>
    </message>
</context>
<context>
    <name>CSummaryPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="527"/>
        <source>Create the new Sandbox</source>
        <translation>Yeni Korumalı Alanı Oluştur</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="535"/>
        <source>Almost complete, click Finish to create a new sandbox and conclude the wizard.</source>
        <translation>Neredeyse tamamlandı, yeni bir korumalı alan oluşturmak ve sihirbazı tamamlamak için Bitiş&apos;e tıklayın.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="544"/>
        <source>Save options as new defaults</source>
        <translation>Seçenekleri yeni varsayılanlar olarak kaydet</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="555"/>
        <source>Don&apos;t show the summary page in future (unless advanced options were set)</source>
        <translation>Özet sayfasını ileride gösterme (gelişmiş seçenekler ayarlanmadıkça)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="574"/>
        <source>
This Sandbox will be saved to: %1</source>
        <translation>
Bu Korumalı Alan şuraya kaydedilecek: %1</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="577"/>
        <source>
This box&apos;s content will be DISCARDED when it&apos;s closed, and the box will be removed.</source>
        <translation>
Bu alandaki son işlem sona erdikten sonra alanın içeriği ATILACAKTIR ve alan kaldırılacaktır.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="579"/>
        <source>
This box will DISCARD its content when its closed, its suitable only for temporary data.</source>
        <translation>
Bu alan kapandığında kendi içeriğini ATACAKTIR, yalnızca geçici veriler için uygundur.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="581"/>
        <source>
Processes in this box will not be able to access the internet or the local network, this ensures all accessed data to stay confidential.</source>
        <translation>
Bu alandaki işlemler internete veya yerel ağa erişemez, böylece erişilen tüm verilerin gizli kalmasını sağlar.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="583"/>
        <source>
This box will run the MSIServer (*.msi installer service) with a system token, this improves the compatibility but reduces the security isolation.</source>
        <translation>
Bu alan, MSIServer&apos;ı (*.msi yükleyici hizmeti) bir sistem belirteci ile çalıştıracaktır, bu uyumluluğu artırır ancak güvenlik yalıtımını azaltır.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="585"/>
        <source>
Processes in this box will think they are run with administrative privileges, without actually having them, hence installers can be used even in a security hardened box.</source>
        <translation>
Bu alandaki işlemler, aslında yönetici ayrıcalıklarına sahip olmadan, yönetici ayrıcalıklarıyla çalıştırıldıklarını düşünecektir. Böylece yükleyiciler güvenliği güçlendirilmiş bir alanda bile kullanılabilir.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="587"/>
        <source>
Processes in this box will be running with a custom process token indicating the sandbox they belong to.</source>
        <translation>
Bu alandaki işlemler, ait oldukları korumalı alanı belirten özel bir işlem belirteci ile çalışacaktır.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="620"/>
        <source>Failed to create new box: %1</source>
        <translation>Yeni alan oluşturulamadı: %1</translation>
    </message>
</context>
<context>
    <name>CSupportDialog</name>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="115"/>
        <source>The installed supporter certificate &lt;b&gt;has expired %1 days ago&lt;/b&gt; and &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;must be renewed&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Yüklü destekçi sertifikası &lt;b&gt;%1 gün önce sona erdi&lt;/b&gt; ve &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;yenilenmesi gerekiyor&lt; /a&gt;.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="117"/>
        <source>&lt;b&gt;You have installed Sandboxie-Plus more than %1 days ago.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</source>
        <translation>&lt;b&gt;Sandboxie-Plus&apos;ı %1 günden daha uzun süre önce yüklediniz.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="119"/>
        <source>&lt;u&gt;Commercial use of Sandboxie past the evaluation period&lt;/u&gt;, requires a valid &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;support certificate&lt;/a&gt;.</source>
        <translation>&lt;u&gt;Sandboxie&apos;nin değerlendirme döneminin ötesinde ticari kullanımı&lt;/u&gt;, geçerli bir &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;destekçi sertifikası gerektirir&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="126"/>
        <source>The installed supporter certificate is &lt;b&gt;outdated&lt;/b&gt; and it is &lt;u&gt;not valid for&lt;b&gt; this version&lt;/b&gt;&lt;/u&gt; of Sandboxie-Plus.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Yüklenen destekçi sertifikası &lt;b&gt;eski&lt;/b&gt; ve Sandboxie-Plus&apos;ın &lt;u&gt;bu sürüm için&lt;b&gt;geçerli değil&lt;/b&gt;&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="128"/>
        <source>The installed supporter certificate is &lt;b&gt;expired&lt;/b&gt; and &lt;u&gt;should be renewed&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Yüklenen destekçi sertifikasının &lt;b&gt;süresi dolmuş&lt;/b&gt; ve &lt;u&gt;yenilenmesi gerekiyor&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="130"/>
        <source>&lt;b&gt;You have been using Sandboxie-Plus for more than %1 days now.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</source>
        <translation>&lt;b&gt;Sandboxie-Plus&apos;ı %1 günden fazla süredir kullanıyorsunuz.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="133"/>
        <source>Sandboxie on ARM64 requires a valid supporter certificate for continued use.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>ARM64&apos;teki Sandboxie, sürekli kullanım için geçerli bir destekçi sertifikası gerektirir.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="135"/>
        <source>Personal use of Sandboxie is free of charge on x86/x64, although some functionality is only available to project supporters.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Sandboxie&apos;nin kişisel kullanımı, x86/x64&apos;te ücretsizdir, ancak bazı işlevler yalnızca proje destekçileri tarafından kullanılabilir.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="138"/>
        <source>Please continue &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;supporting the project&lt;/a&gt; by renewing your &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; and continue using the &lt;b&gt;enhanced functionality&lt;/b&gt; in new builds.</source>
        <translation>Lütfen &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;destekçi sertifikanızı&lt;/a&gt; yenileyerek &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;bu projeyi desteklemeye&lt;/a&gt; devam edin ve yeni sürümlerde &lt;b&gt;gelişmiş işlevselliği&lt;/b&gt; kullanmaya devam edin.</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="141"/>
        <source>Sandboxie &lt;u&gt;without&lt;/u&gt; a valid supporter certificate will sometimes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;pause for a few seconds&lt;/font&gt;&lt;/b&gt;. This pause allows you to consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;purchasing a supporter certificate&lt;/a&gt; or &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;earning one by contributing&lt;/a&gt; to the project. &lt;br /&gt;&lt;br /&gt;A &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; not just removes this reminder, but also enables &lt;b&gt;exclusive enhanced functionality&lt;/b&gt; providing better security and compatibility.</source>
        <translation>Sandboxie geçerli bir destekçi sertifikası &lt;u&gt;olmadığında&lt;/u&gt; bazen &lt;b&gt;&lt;font color=&apos;red&apos;&gt;birkaç saniye duraklar&lt;/font&gt;&lt;/b&gt;. Bu duraklama, &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;bir destekçi sertifikası satın almayı&lt;/a&gt; veya &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;projeye katkıda bulunarak bir tane kazanmayı&lt;/a&gt; düşünmenizi sağlar. &lt;br /&gt;&lt;br /&gt;Bir &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;destekçi sertifikası&lt;/a&gt; yalnızca bu hatırlatıcıyı kaldırmakla kalmaz, aynı zamanda daha iyi güvenlik ve uyumluluk sağlayan &lt;b&gt;özel geliştirilmiş işlevsellik&lt;/b&gt; sunar.</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="172"/>
        <source>Sandboxie-Plus - Support Reminder</source>
        <translation>Sandboxie-Plus - Destek Hatırlatıcısı</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="251"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="258"/>
        <source>Quit</source>
        <translation>Çıkış</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="259"/>
        <source>Continue</source>
        <translation>Devam et</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="260"/>
        <source>Get Certificate</source>
        <translation>Sertifika Al</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="261"/>
        <source>Enter Certificate</source>
        <translation>Sertifika Girin</translation>
    </message>
</context>
<context>
    <name>CTemplateTypePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="258"/>
        <source>Create new Template</source>
        <translation>Yeni Şablon oluştur</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="267"/>
        <source>Select template type:</source>
        <translation>Şablon türünü seçin:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="271"/>
        <source>%1 template</source>
        <translation>%1 şablonu</translation>
    </message>
</context>
<context>
    <name>CTemplateWizard</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="37"/>
        <source>Compatibility Template Wizard</source>
        <translation>Uyumluluk Şablonu Sihirbazı</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="48"/>
        <source>Custom</source>
        <translation>Özel</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="49"/>
        <source>Web Browser</source>
        <translation>Web Tarayıcısı</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="82"/>
        <source>Force %1 to run in this sandbox</source>
        <translation>%1 programını bu korumalı alanda çalışmaya zorla</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="100"/>
        <source>Allow direct access to the entire %1 profile folder</source>
        <translation>%1 profil klasörünün tamamına doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="113"/>
        <location filename="Wizards/TemplateWizard.cpp" line="168"/>
        <source>Allow direct access to %1 phishing database</source>
        <translation>%1 kimlik avı veritabanına doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="127"/>
        <source>Allow direct access to %1 session management</source>
        <translation>%1 oturum yönetimine doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="136"/>
        <location filename="Wizards/TemplateWizard.cpp" line="199"/>
        <source>Allow direct access to %1 passwords</source>
        <translation>%1 parolalarına doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="146"/>
        <location filename="Wizards/TemplateWizard.cpp" line="208"/>
        <source>Allow direct access to %1 cookies</source>
        <translation>%1 tanımlama bilgilerine doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="155"/>
        <location filename="Wizards/TemplateWizard.cpp" line="227"/>
        <source>Allow direct access to %1 bookmark and history database</source>
        <translation>%1 yer imi ve geçmiş veritabanına doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="180"/>
        <source>Allow direct access to %1 sync data</source>
        <translation>%1 eşitleme verisine doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="190"/>
        <source>Allow direct access to %1 preferences</source>
        <translation>%1 tercihlerine doğrudan erişime izin ver</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="217"/>
        <source>Allow direct access to %1 bookmarks</source>
        <translation>%1 yer imlerine doğrudan erişime izin ver</translation>
    </message>
</context>
<context>
    <name>CTraceModel</name>
    <message>
        <location filename="Models/TraceModel.cpp" line="196"/>
        <source>Unknown</source>
        <translation>Bilinmeyen</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="175"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="176"/>
        <source>Process %1</source>
        <translation>İşlem %1</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="179"/>
        <source>Thread %1</source>
        <translation>İş Parçacığı %1</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="333"/>
        <source>Process</source>
        <translation>İşlem</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="335"/>
        <source>Type</source>
        <translation>Tür</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="336"/>
        <source>Status</source>
        <translation>Durum</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="337"/>
        <source>Value</source>
        <translation>Değer</translation>
    </message>
</context>
<context>
    <name>CTraceView</name>
    <message>
        <location filename="Views/TraceView.cpp" line="255"/>
        <source>Show as task tree</source>
        <translation>Görev ağacı olarak göster</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="266"/>
        <source>PID:</source>
        <translation>PID:</translation>
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
        <translation>[Tümü]</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="273"/>
        <source>TID:</source>
        <translation>TID:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="624"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="631"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="281"/>
        <source>Type:</source>
        <translation>Tür:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="259"/>
        <source>Show NT Object Tree</source>
        <translation>NT Nesne Ağacını Göster</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="292"/>
        <source>Status:</source>
        <translation>Durum:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="295"/>
        <source>Open</source>
        <translation>Açık</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="296"/>
        <source>Closed</source>
        <translation>Kapalı</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="297"/>
        <source>Trace</source>
        <translation>İzleme</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="298"/>
        <source>Other</source>
        <translation>Diğer</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="306"/>
        <source>Show All Boxes</source>
        <translation>Tüm Alanları Göster</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="310"/>
        <source>Show Stack Trace</source>
        <translation>Yığın İzlemeyi Göster</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="315"/>
        <source>Save to file</source>
        <translation>Dosyaya kaydet</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="329"/>
        <source>Cleanup Trace Log</source>
        <translation>İzleme Günlüğünü Temizle</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="694"/>
        <source>Save trace log to file</source>
        <translation>İzleme günlüğünü dosyaya kaydet</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="700"/>
        <source>Failed to open log file for writing</source>
        <translation>Günlük dosyası yazmak için açılamadı</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="720"/>
        <source>Unknown</source>
        <translation>Bilinmeyen</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="251"/>
        <source>Monitor mode</source>
        <translation>İzleme modu</translation>
    </message>
</context>
<context>
    <name>CTraceWindow</name>
    <message>
        <location filename="Views/TraceView.cpp" line="753"/>
        <source>Sandboxie-Plus - Trace Monitor</source>
        <translation>Sandboxie-Plus - İzleme Monitörü</translation>
    </message>
</context>
<context>
    <name>CUIPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="302"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; UI</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; kullanıcı arayüzünü yapılandırın</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="303"/>
        <source>Select the user interface style you prefer.</source>
        <translation>Tercih ettiğiniz kullanıcı arayüzü stilini seçin.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="307"/>
        <source>&amp;Advanced UI for experts</source>
        <translation>&amp;Deneyimliler için gelişmiş kullanıcı arayüzü</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="312"/>
        <source>&amp;Simple UI for beginners</source>
        <translation>&amp;Yeni başlayanlar için basit kullanıcı arayüzü</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="317"/>
        <source>&amp;Vintage SbieCtrl.exe UI</source>
        <translation>&amp;Eski SbieCtrl.exe Kullanıcı Arayüzü</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="342"/>
        <source>Use Bright Mode</source>
        <translation>Açık Modu Kullan</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="346"/>
        <source>Use Dark Mode</source>
        <translation>Koyu Modu Kullan</translation>
    </message>
</context>
<context>
    <name>CWFPPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="453"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; network filtering</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; ağ filtrelemesini yapılandırın</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="454"/>
        <source>Sandboxie can use the Windows Filtering Platform (WFP) to restrict network access.</source>
        <translation>Sandboxie, ağ erişimini kısıtlamak için Windows Filtreleme Platformunu (WFP) kullanabilir.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="460"/>
        <source>Using WFP allows Sandboxie to reliably enforce IP/Port based rules for network access. Unlike system level application firewalls, Sandboxie can use different rules in each box for the same application. If you already have a good and reliable application firewall and do not need per box rules, you can leave this option unchecked. Without WFP enabled, Sandboxie will still be able to reliably and entirely block processes from accessing the network. However, this can cause the process to crash, as the driver blocks the required network device endpoints. Even with WFP disabled, Sandboxie offers to set IP/Port based rules, however these will be applied in user mode only and not be enforced by the driver. Hence, without WFP enabled, an intentionally malicious process could bypass those rules, but not the entire network block.</source>
        <translation>WFP&apos;nin kullanılması, Sandboxie&apos;nin ağ erişimi için IP/Port tabanlı kuralları güvenilir bir şekilde uygulamasına olanak sağlar. Sistem düzeyinde çalışan uygulama güvenlik duvarlarının aksine, Sandboxie aynı uygulama için her alanda farklı kurallar kullanabilir. Halihazırda iyi ve güvenilir bir uygulama güvenlik duvarınız varsa ve alan başına kurallara ihtiyacınız yoksa, bu seçeneği işaretlemeden bırakabilirsiniz. WFP etkinleştirilmeden bile Sandboxie, işlemlerin ağa erişmesini güvenilir bir şekilde ve tamamen engelleyebilir. Ancak, Sandboxie sürücüsü gerekli ağ aygıtı uç noktalarını engellediğinden bu durum bazı işlemlerin çökmesine neden olabilir. Sandboxie, WFP devre dışı bırakılmış olsa bile IP/Port tabanlı kurallar belirlemeyi sağlar, ancak bunlar yalnızca kullanıcı modunda uygulanacak ve sürücü tarafından uygulanmaya zorlanmayacaktır. Bu nedenle, WFP etkinleştirilmezse kötü amaçlı bir işlem bu kuralları atlatabilir ancak tüm ağ engellemesini atlatamaz.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="469"/>
        <source>Enable Windows Filtering Platform (WFP) support</source>
        <translation>Windows Filtreleme Platformu (WFP) desteğini etkinleştir</translation>
    </message>
</context>
<context>
    <name>NewBoxWindow</name>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="32"/>
        <source>SandboxiePlus new box</source>
        <translation>SandboxiePlus yeni alan</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="67"/>
        <source>Sandbox Name:</source>
        <translation>Korumalı Alan Adı:</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="60"/>
        <source>Box Type Preset:</source>
        <translation>Alan Türü Ön Ayarı:</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="91"/>
        <source>A sandbox isolates your host system from processes running within the box, it prevents them from making permanent changes to other programs and data in your computer. The level of isolation impacts your security as well as the compatibility with applications, hence there will be a different level of isolation depending on the selected Box Type. Sandboxie can also protect your personal data from being accessed by processes running under its supervision.</source>
        <translation>Korumalı alan, ana bilgisayar sisteminizi korumalı alan içinde çalışan işlemlerden yalıtır ve diğer programlarda ve bilgisayarınızdaki verilerde kalıcı değişiklikler yapmalarını engeller. Yalıtım düzeyi, güvenliği ve uygulamalarla uyumluluğu etkiler, dolayısıyla seçili &apos;Alan Türüne&apos; bağlı olarak farklı bir yalıtım düzeyi sağlayacaktır. Sandboxie ayrıca kişisel verilerinize, kendi gözetimi altında çalışan işlemler tarafından erişilmesine karşı da koruyabilir.</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="101"/>
        <source>Box info</source>
        <translation>Alan bilgisi</translation>
    </message>
</context>
<context>
    <name>OptionsWindow</name>
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
        <translation>Ad</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2257"/>
        <location filename="Forms/OptionsWindow.ui" line="2357"/>
        <location filename="Forms/OptionsWindow.ui" line="2490"/>
        <source>Path</source>
        <translation>Yol</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4685"/>
        <source>Save</source>
        <translation>Kaydet</translation>
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
        <translation>Tür</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2151"/>
        <source>Allow only selected programs to start in this sandbox. *</source>
        <translation>Bu korumalı alanda yalnızca seçili programların başlamasına izin ver. *</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1734"/>
        <source>Force Folder</source>
        <translation>Klasörü Zorla</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2450"/>
        <source>Add IPC Path</source>
        <translation>IPC Yolu Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="96"/>
        <source>Sandbox Indicator in title:</source>
        <translation>Başlıktaki korumalı alan göstergesi:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4250"/>
        <source>Debug</source>
        <translation>Hata Ayıklama</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3882"/>
        <source>Users</source>
        <translation>Kullanıcılar</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="695"/>
        <source>Block network files and folders, unless specifically opened.</source>
        <translation>Özel olarak açılmadıkça ağ dosyalarını ve klasörlerini engelle.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="879"/>
        <source>Command Line</source>
        <translation>Komut Satırı</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2618"/>
        <source>Don&apos;t alter window class names created by sandboxed programs</source>
        <translation>Korumalı alandaki programlar tarafından oluşturulan pencere sınıfı adları değiştirilmesin</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4116"/>
        <source>Log Debug Output to the Trace Log</source>
        <translation>Hata Ayıklama Çıktısını İzleme Günlüğüne Kaydet</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2571"/>
        <source>Add Wnd Class</source>
        <translation>Wnd Sınıfı Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4212"/>
        <source>Access Tracing</source>
        <translation>Erişim İzleme</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="279"/>
        <source>File Options</source>
        <translation>Dosya Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="52"/>
        <source>General Options</source>
        <translation>Genel Seçenekler</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="503"/>
        <source>kilobytes</source>
        <translation>kilobayt</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2165"/>
        <source>Allow all programs to start in this sandbox.</source>
        <translation>Tüm programların bu alanda başlamasına izin ver.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3277"/>
        <source>Enable Immediate Recovery prompt to be able to recover files as soon as they are created.</source>
        <translation>Dosyalar oluşturulur oluşturulmaz kurtarabilmek için Anında Kurtarma istemini etkinleştir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2252"/>
        <location filename="Forms/OptionsWindow.ui" line="2352"/>
        <location filename="Forms/OptionsWindow.ui" line="2485"/>
        <location filename="Forms/OptionsWindow.ui" line="2605"/>
        <location filename="Forms/OptionsWindow.ui" line="2679"/>
        <location filename="Forms/OptionsWindow.ui" line="2968"/>
        <source>Access</source>
        <translation>Erişim</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4316"/>
        <source>These options are intended for debugging compatibility issues, please do not use them in production use. </source>
        <translation>Bu seçenekler uyumluluk sorunlarındaki hataları ayıklamaya yönelik tasarlanmıştır, lütfen bu ayarları üretim amaçlı kullanmayın. </translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4341"/>
        <source>Templates</source>
        <translation>Şablonlar</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4387"/>
        <source>Text Filter</source>
        <translation>Metin Filtresi</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4662"/>
        <source>Cancel</source>
        <translation>İptal</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3900"/>
        <source>Restrict Resource Access monitor to administrators only</source>
        <translation>Kaynak Erişimi İzleyicisini yalnızca yöneticilerle kısıtla</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1252"/>
        <location filename="Forms/OptionsWindow.ui" line="1269"/>
        <location filename="Forms/OptionsWindow.ui" line="1354"/>
        <location filename="Forms/OptionsWindow.ui" line="1426"/>
        <location filename="Forms/OptionsWindow.ui" line="1450"/>
        <location filename="Forms/OptionsWindow.ui" line="1474"/>
        <source>Protect the sandbox integrity itself</source>
        <translation>Korumalı alan bütünlüğünün kendisini koruyun</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3230"/>
        <source>Add Folder</source>
        <translation>Klasör Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2925"/>
        <source>Prompt user whether to allow an exemption from the blockade.</source>
        <translation>Kullanıcıya ablukadan muafiyete izin verip vermeyeceğini sor.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4164"/>
        <source>IPC Trace</source>
        <translation>IPC İzleme</translation>
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
        <translation>Kaldır</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2277"/>
        <source>Add File/Folder</source>
        <translation>Dosya/Klasör Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2911"/>
        <source>Issue message 1307 when a program is denied internet access</source>
        <translation>Bir programın internet erişimi reddedildiğinde 1307 mesajını yayınla</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3375"/>
        <location filename="Forms/OptionsWindow.ui" line="3424"/>
        <source>Compatibility</source>
        <translation>Uyumluluk</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1920"/>
        <source>Stop Behaviour</source>
        <translation>Durma Davranışı</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2939"/>
        <source>Note: Programs installed to this sandbox won&apos;t be able to access the internet at all.</source>
        <translation>Not: Bu alana yüklenen programlar internete hiçbir şekilde erişemez.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="62"/>
        <source>Box Options</source>
        <translation>Alan Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3774"/>
        <source>Don&apos;t allow sandboxed processes to see processes running in other boxes</source>
        <translation>Korumalı alandaki işlemlerin diğer alanlarda çalışan işlemleri görmesine izin verilmesin</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1591"/>
        <source>Add Group</source>
        <translation>Grup Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="106"/>
        <source>Sandboxed window border:</source>
        <translation>Korumalı alana sahip pencere kenarlığı:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2158"/>
        <source>Prevent selected programs from starting in this sandbox.</source>
        <translation>Seçili programların bu alanda başlamasını önle.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3950"/>
        <source>Miscellaneous</source>
        <translation>Çeşitli</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="596"/>
        <source>Issue message 2102 when a file is too large</source>
        <translation>Dosya çok büyükse 2102 mesajını yayınla</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3151"/>
        <source>File Recovery</source>
        <translation>Dosya Kurtarma</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="410"/>
        <source>Box Delete options</source>
        <translation>Alan Silme Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4076"/>
        <source>Pipe Trace</source>
        <translation>Boru İzleme</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4150"/>
        <source>File Trace</source>
        <translation>Dosya İzleme</translation>
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
        <translation>Program</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3767"/>
        <location filename="Forms/OptionsWindow.ui" line="3825"/>
        <source>Add Process</source>
        <translation>İşlem Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1623"/>
        <location filename="Forms/OptionsWindow.ui" line="1977"/>
        <location filename="Forms/OptionsWindow.ui" line="2054"/>
        <location filename="Forms/OptionsWindow.ui" line="2132"/>
        <location filename="Forms/OptionsWindow.ui" line="2918"/>
        <source>Add Program</source>
        <translation>Program Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4416"/>
        <source>Filter Categories</source>
        <translation>Kategorileri Filtrele</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="453"/>
        <source>Copy file size limit:</source>
        <translation>Dosya boyutu kopyalama sınırı:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="709"/>
        <source>Open System Protected Storage</source>
        <translation>Sistem Korumalı Depolamayı aç</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="730"/>
        <location filename="Forms/OptionsWindow.ui" line="766"/>
        <location filename="Forms/OptionsWindow.ui" line="783"/>
        <location filename="Forms/OptionsWindow.ui" line="992"/>
        <location filename="Forms/OptionsWindow.ui" line="1037"/>
        <source>Protect the system from sandboxed processes</source>
        <translation>Sistemi korumalı alandaki işlemlerden koru</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="32"/>
        <source>SandboxiePlus Options</source>
        <translation>SandboxiePlus Ayarları</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4433"/>
        <source>Category</source>
        <translation>Kategori</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1080"/>
        <source>Drop rights from Administrators and Power Users groups</source>
        <translation>Yöneticiler ve Yetkili Kullanıcılar grupları haklarını bırak</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2384"/>
        <source>Add Reg Key</source>
        <translation>Kayıt Anahtarı Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3718"/>
        <source>Hide Processes</source>
        <translation>İşlemleri Gizle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3193"/>
        <source>When the Quick Recovery function is invoked, the following folders will be checked for sandboxed content. </source>
        <translation>Hızlı Kurtarma işlevi çağrıldığında, aşağıdaki klasörler korumalı alan içeriği için denetlenecektir. </translation>
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
        <translation>Tüm erişim olaylarını sürücü tarafından görüldüğü gibi kaynak erişim günlüğüne kaydet.

Bu seçenekler olay maskesini &quot;*&quot; olarak ayarlar - Tüm erişim olayları
Ini dosyasından &quot;*&quot; yerine aşağıdakiler belirtilerek
günlüğe kaydetme özelleştirilebilir.
&quot;A&quot; - İzin verilen erişim(ler)
&quot;D&quot; - Reddedilen erişim(ler)
&quot;I&quot; - Yok sayılan erişim(ler).</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="221"/>
        <source>px Width</source>
        <translation>px Genişliği</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3907"/>
        <source>Add User</source>
        <translation>Kullanıcı Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1699"/>
        <source>Programs entered here, or programs started from entered locations, will be put in this sandbox automatically, unless they are explicitly started in another sandbox.</source>
        <translation>Buraya girilen programlar veya girilen konumlardan başlatılan programlar, başka bir korumalı alanda açıkça belirtilmedikçe otomatik olarak bu alana yerleştirilecektir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1777"/>
        <source>Force Program</source>
        <translation>Program Zorla</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4306"/>
        <source>WARNING, these options can disable core security guarantees and break sandbox security!!!</source>
        <translation>UYARI, bu seçenekler temel güvenlik garantilerini devre dışı bırakabilir ve korumalı alan güvenliğini bozabilir!</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4649"/>
        <source>Edit ini</source>
        <translation>Ini Düzenle</translation>
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
        <translation>Şablonları Göster</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3316"/>
        <source>Ignore Folder</source>
        <translation>Klasörü Yok Say</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4171"/>
        <source>GUI Trace</source>
        <translation>GKA İzleme</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4226"/>
        <source>Key Trace</source>
        <translation>Tuş İzleme</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4048"/>
        <source>Tracing</source>
        <translation>İzleme</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="252"/>
        <source>Appearance</source>
        <translation>Görünüm</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1559"/>
        <source>Add sandboxed processes to job objects (recommended)</source>
        <translation>Korumalı alan işlemlerini iş nesnelerine ekle (Önerilir)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3260"/>
        <source>You can exclude folders and file types (or file extensions) from Immediate Recovery.</source>
        <translation>Klasörleri ve dosya türlerini (veya dosya uzantılarını) Anında Kurtarma&apos;nın dışında bırakabilirsiniz.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="796"/>
        <source>Run Menu</source>
        <translation>Çalıştır Menüsü</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4331"/>
        <source>App Templates</source>
        <translation>Uygulama Şablonları</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3270"/>
        <source>Ignore Extension</source>
        <translation>Uzantıyı Yok Say</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="294"/>
        <source>Protect this sandbox from deletion or emptying</source>
        <translation>Bu korumalı alanı silinmeye veya boşaltılmaya karşı koru</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3937"/>
        <source>Add user accounts and user groups to the list below to limit use of the sandbox to only those accounts.  If the list is empty, the sandbox can be used by all user accounts.

Note:  Forced Programs and Force Folders settings for a sandbox do not apply to user accounts which cannot use the sandbox.</source>
        <translation>Korumalı alanın kullanımını yalnızca bu hesaplarla sınırlamak için aşağıdaki listeye kullanıcı hesaplarını ve kullanıcı gruplarını ekleyin. Liste boşsa, korumalı alan tüm kullanıcı hesapları tarafından kullanılabilir.

Not: Bir korumalı alana ilişkin Zorunlu Programlar ve Zorunlu Klasörler ayarları, korumalı alanı kullanamayan kullanıcı hesapları için geçerli değildir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2172"/>
        <source>* Note: Programs installed to this sandbox won&apos;t be able to start at all.</source>
        <translation>* Not: Bu korumalı alana yüklenen programlar hiçbir şekilde başlatılamaz.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4364"/>
        <source>This list contains a large amount of sandbox compatibility enhancing templates</source>
        <translation>Bu liste, korumalı alan uyumluluğunu geliştiren çok sayıda şablonlar içerir</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1583"/>
        <source>Program Groups</source>
        <translation>Program Grupları</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2100"/>
        <source>Issue message 1308 when a program fails to start</source>
        <translation>Bir program başlatılamadığında 1308 mesajını yayınla</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2194"/>
        <source>Resource Access</source>
        <translation>Kaynak Erişimi</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3358"/>
        <source>Advanced Options</source>
        <translation>Gelişmiş Seçenekler</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3743"/>
        <source>Hide host processes from processes running in the sandbox.</source>
        <translation>Korumalı alanda çalışan işlemlerden ana bilgisayar işlemlerini gizler.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="433"/>
        <location filename="Forms/OptionsWindow.ui" line="474"/>
        <source>File Migration</source>
        <translation>Dosya Taşıma</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="324"/>
        <source>Auto delete content when last sandboxed process terminates</source>
        <translation>Korumalı alandaki son işlem sonlandırıldığında içeriği otomatik olarak sil</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2711"/>
        <source>Add COM Object</source>
        <translation>COM Nesnesi Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="925"/>
        <source>You can configure custom entries for the sandbox run menu.</source>
        <translation>Korumalı alanın çalıştırma menüsünde görünecek özel girişleri yapılandırabilirsiniz.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2092"/>
        <source>Start Restrictions</source>
        <translation>Başlatma Kısıtlamaları</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3403"/>
        <source>Force usage of custom dummy Manifest files (legacy behaviour)</source>
        <translation>Özel sahte Manifest dosyalarının kullanımını zorla (Eski davranış)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4643"/>
        <source>Edit ini Section</source>
        <translation>Ini Düzenleme Bölümü</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4219"/>
        <source>COM Class Trace</source>
        <translation>COM Sınıf İzleme</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="675"/>
        <source>Block access to the printer spooler</source>
        <translation>Yazıcı biriktiricisine erişimi engelle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="688"/>
        <source>Allow the print spooler to print to files outside the sandbox</source>
        <translation>Yazdırma biriktiricisinin korumalı alanın dışındaki dosyalara yazdırmasına izin ver</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="702"/>
        <source>Remove spooler restriction, printers can be installed outside the sandbox</source>
        <translation>Biriktirici kısıtlamasını kaldır, yazıcılar korumalı alanın dışına kurulabilir</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="845"/>
        <source>Add program</source>
        <translation>Program Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1505"/>
        <source>Do not start sandboxed services using a system token (recommended)</source>
        <translation>Korumalı alandaki hizmetleri bir sistem belirteci kullanarak başlatılmasın (Önerilir)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="995"/>
        <source>Elevation restrictions</source>
        <translation>Yetkilendirme Kısıtlamaları</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1002"/>
        <source>Make applications think they are running elevated (allows to run installers safely)</source>
        <translation>Uygulamaların yetkilendirilmiş şekilde çalıştıklarını düşünmelerini sağla (Yükleyicileri güvenli bir şekilde çalıştırmanıza izin verir)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="786"/>
        <source>Network restrictions</source>
        <translation>Ağ Kısıtlamaları</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1016"/>
        <source>(Recommended)</source>
        <translation>(Önerilen)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="259"/>
        <source>Double click action:</source>
        <translation>Çift tıklama eylemi:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="359"/>
        <source>Separate user folders</source>
        <translation>Ayrı kullanıcı klasörleri</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="301"/>
        <source>Allow elevated sandboxed applications to read the harddrive</source>
        <translation>Yetkilendirilmiş korumalı alan uygulamalarının sabit sürücüyü okumasına izin ver</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="287"/>
        <source>Warn when an application opens a harddrive handle</source>
        <translation>Bir uygulama sabit sürücü tanıtıcısı açtığında uyar</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="396"/>
        <source>Box Structure</source>
        <translation>Alan Yapısı</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="613"/>
        <source>Restrictions</source>
        <translation>Kısıtlamalar</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="953"/>
        <source>Security Options</source>
        <translation>Güvenlik Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="963"/>
        <source>Security Hardening</source>
        <translation>Güvenlik Sıkılaştırması</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="733"/>
        <source>Other restrictions</source>
        <translation>Diğer Kısıtlamalar</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="338"/>
        <source>Use volume serial numbers for drives, like: \drive\C~1234-ABCD</source>
        <translation>Sürücüler için birim seri numaralarını kullan, örneğin: \drive\C~1234-ABCD</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="382"/>
        <source>The box structure can only be changed when the sandbox is empty</source>
        <translation>Alan yapısı yalnızca korumalı alan boşken değiştirilebilir</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="352"/>
        <source>Disk/File access</source>
        <translation>Sürücü/Dosya Erişimi</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="331"/>
        <source>Virtualization scheme</source>
        <translation>Sanallaştırma şeması</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="481"/>
        <source>2113: Content of migrated file was discarded
2114: File was not migrated, write access to file was denied
2115: File was not migrated, file will be opened read only</source>
        <translation>2113: Taşınan dosyanın içeriği atıldı
2114: Dosya taşınamadı, dosyaya yazma erişimi reddedildi
2115: Dosya taşınamadı, dosya salt okunur olarak açılacak</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="486"/>
        <source>Issue message 2113/2114/2115 when a file is not fully migrated</source>
        <translation>Bir dosya tam olarak taşınamadığında 2113/2114/2115 mesajını yayınla</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="510"/>
        <source>Add Pattern</source>
        <translation>Desen Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="517"/>
        <source>Remove Pattern</source>
        <translation>Deseni Kaldır</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="571"/>
        <source>Pattern</source>
        <translation>Desen</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="579"/>
        <source>Sandboxie does not allow writing to host files, unless permitted by the user. When a sandboxed application attempts to modify a file, the entire file must be copied into the sandbox, for large files this can take a significate amount of time. Sandboxie offers options for handling these cases, which can be configured on this page.</source>
        <translation>Sandboxie, kullanıcı izin vermediği sürece ana sistemdeki dosyalara yazmaya izin vermez. Korumalı alandaki bir uygulama bir dosyayı değiştirmeye çalıştığında, tüm dosyanın korumalı alana kopyalanması gerekir, büyük dosyalar için bu işlem çok uzun sürebilir. Sandboxie, bu tür durumlar için bu sayfada yapılandırılabilen seçenekler sunar.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="589"/>
        <source>Using wildcard patterns file specific behavior can be configured in the list below:</source>
        <translation>Özel karakter desenleri kullanılarak dosyaya özgü davranış aşağıdaki listeden yapılandırılabilir:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="603"/>
        <source>When a file cannot be migrated, open it in read-only mode instead</source>
        <translation>Bir dosya taşınamadığı zaman salt okunur modda açılsın</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="769"/>
        <source>Printing restrictions</source>
        <translation>Yazdırma Kısıtlamaları</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="874"/>
        <source>Icon</source>
        <translation>Simge</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="899"/>
        <source>Move Up</source>
        <translation>Yukarı Taşı</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="918"/>
        <source>Move Down</source>
        <translation>Aşağı Taşı</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1153"/>
        <source>Security Isolation</source>
        <translation>Güvenlik Yalıtımı</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1174"/>
        <source>Various isolation features can break compatibility with some applications. If you are using this sandbox &lt;b&gt;NOT for Security&lt;/b&gt; but for application portability, by changing these options you can restore compatibility by sacrificing some security.</source>
        <translation>Çeşitli yalıtım özellikleri, bazı uygulamalarla uyumluluğu bozabilir. Bu korumalı alanı &lt;b&gt;Güvenlik için DEĞİL&lt;/b&gt; ancak uygulama taşınabilirliği için kullanıyorsanız, bu seçenekleri değiştirip biraz da güvenlikten ödün vererek uyumluluğu arttırabilirsiniz.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1272"/>
        <source>Access Isolation</source>
        <translation>Erişim Yalıtımı</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1299"/>
        <location filename="Forms/OptionsWindow.ui" line="1357"/>
        <source>Image Protection</source>
        <translation>Görüntü Koruması</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1307"/>
        <source>Issue message 1305 when a program tries to load a sandboxed dll</source>
        <translation>Bir program korumalı alandan bir dll dosyası yüklemeye çalıştığında 1305 mesajını yayınla</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1340"/>
        <source>Prevent sandboxes programs installed on host from loading dll&apos;s from the sandbox</source>
        <translation>Ana bilgisayarda kurulu korumalı alanda çalışan programların korumalı alandan dll dosyaları yüklemesini önle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1383"/>
        <source>Sandboxie’s resource access rules often discriminate against program binaries located inside the sandbox. OpenFilePath and OpenKeyPath work only for application binaries located on the host natively. In order to define a rule without this restriction, OpenPipePath or OpenConfPath must be used. Likewise, all Closed(File|Key|Ipc)Path directives which are defined by negation e.g. ‘ClosedFilePath=! iexplore.exe,C:Users*’ will be always closed for binaries located inside a sandbox. Both restriction policies can be disabled on the “Access policies” page.
This is done to prevent rogue processes inside the sandbox from creating a renamed copy of themselves and accessing protected resources. Another exploit vector is the injection of a library into an authorized process to get access to everything it is allowed to access. Using Host Image Protection, this can be prevented by blocking applications (installed on the host) running inside a sandbox from loading libraries from the sandbox itself.</source>
        <translation>Sandboxie&apos;nin kaynak erişim kuralları, genellikle korumalı alan içinde bulunan program ikili dosyalarına göre ayrım yapar. OpenFilePath ve OpenKeyPath yalnızca ana bilgisayarda yerel olarak bulunan uygulama ikili dosyaları için çalışır. 
Bu kısıtlama olmaksızın bir kural tanımlamak için OpenPipePath veya OpenConfPath kullanılmalıdır. Aynı şekilde, olumsuzlama ile tanımlanan tüm Closed(File|Key|Ipc)Path yönergeleri, örn. &apos;ClosedFilePath=! iexplore.exe,C:Users*&apos;, korumalı alan içinde bulunan ikili dosyalar için her zaman kapalı olacaktır. Her iki kısıtlama ilkesi de &quot;Erişim İlkeleri&quot; sayfasından devre dışı bırakılabilir.
Bu, korumalı alan içindeki haydut işlemlerin kendilerinin yeniden adlandırılmış bir kopyasını oluşturmasını ve korunan kaynaklara erişmesini önlemek için yapılır. Başka bir istismar vektörü de bir kütüphanenin yetkili bir işleme yerleşerek ona izin verilen her şeye erişim hakkı elde etmesidir. Ana Bilgisayar Görüntü Koruması kullanılarak, bir korumalı alanda çalışan uygulamaların (ana bilgisayarda yüklü) korumalı alanda bulunan kütüphanleri yüklemesi engellenerek bu durum önlenebilir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1404"/>
        <source>Advanced Security</source>
        <translation>Gelişmiş Güvenlik</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1412"/>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation>Anonim bir belirteç (deneysel) yerine Sandboxie oturum açmayı kullan</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1429"/>
        <source>Other isolation</source>
        <translation>Diğer Yalıtım</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1453"/>
        <source>Privilege isolation</source>
        <translation>Ayrıcalık Yalıtımı</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1477"/>
        <source>Sandboxie token</source>
        <translation>Sandboxie Belirteci</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1566"/>
        <source>Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.</source>
        <translation>Özel bir Sandboxie belirteci kullanmak, birbirinden ayrı korumalı alanların daha iyi yalıtılmasını sağlar ve görev yöneticilerinin kullanıcı sütununda bir işlemin hangi alana ait olduğu gösterir. Ancak bazı 3. parti güvenlik çözümleri özel belirteçlerle sorun yaşayabilir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1657"/>
        <source>Program Control</source>
        <translation>Program Denetimi</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1679"/>
        <source>Force Programs</source>
        <translation>Zorunlu Programlar</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1791"/>
        <source>Disable forced Process and Folder for this sandbox</source>
        <translation>Bu korumalı alan için İşlem ve Klasör zorlamayı devre dışı bırak</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1801"/>
        <source>Breakout Programs</source>
        <translation>Çıkabilen Programlar</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1870"/>
        <source>Breakout Program</source>
        <translation>Çıkabilen Program</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1896"/>
        <source>Breakout Folder</source>
        <translation>Çıkabilen Klasör</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1903"/>
        <source>Programs entered here will be allowed to break out of this sandbox when they start. It is also possible to capture them into another sandbox, for example to have your web browser always open in a dedicated box.</source>
        <translation>Buraya girilen programlar, başlatıldıklarında, bu kormalı alanın dışına çıkmalarına izin verilecektir. Bunları başka bir korumalı alana geçirmek de mümkündür, örneğin web tarayıcınızın her zaman özel bir alanda açık olması gibi.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1930"/>
        <source>Lingering Programs</source>
        <translation>Oyalayıcı Programlar</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1996"/>
        <source>Lingering programs will be automatically terminated if they are still running after all other processes have been terminated.</source>
        <translation>Oyalayıcı programlar, diğer tüm işlemler sonlandırıldıktan sonra hala çalışıyorlarsa otomatik olarak sonlandırılacaktır.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2009"/>
        <source>Leader Programs</source>
        <translation>Lider Programlar</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2075"/>
        <source>If leader processes are defined, all others are treated as lingering processes.</source>
        <translation>Eğer lider işlemler tanımlanırsa, diğer tüm işlemlere oyalayıcı olarak davranılır.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2204"/>
        <source>Files</source>
        <translation>Dosyalar</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2297"/>
        <source>Configure which processes can access Files, Folders and Pipes. 
&apos;Open&apos; access only applies to program binaries located outside the sandbox, you can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behavior in the Policies tab.</source>
        <translation>Hangi işlemlerin Dosyalara, Klasörlere ve Borulara erişebileceğini yapılandırın. 
&apos;Açık&apos; erişimi yalnızca korumalı alanın dışında bulunan program dosyaları için geçerlidir, bunun yerine tüm programlara uygulanmasını sağlamak için &apos;Hepsine Açık&apos; kullanabilir veya bu davranışı İlkeler sekmesinden değiştirebilirsiniz.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2311"/>
        <source>Registry</source>
        <translation>Kayıt</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2404"/>
        <source>Configure which processes can access the Registry. 
&apos;Open&apos; access only applies to program binaries located outside the sandbox, you can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behavior in the Policies tab.</source>
        <translation>Hangi işlemlerin Kayıt Defterine erişebileceğini yapılandırın. 
&apos;Açık&apos; erişimi yalnızca korumalı alanın dışında bulunan program dosyaları için geçerlidir, bunun yerine tüm programlara uygulanmasını sağlamak için &apos;Hepsine Açık&apos; kullanabilir veya bu davranışı İlkeler sekmesinden değiştirebilirsiniz.</translation>
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
        <translation>Hangi işlemlerin ALPC bağlantı noktaları ve diğer işlemlerin belleği ve bağlamı gibi NT IPC nesnelerine erişebileceğini yapılandırın.
Bir işlemi belirtmek için yol olarak &apos;$:program.exe&apos; kullanın.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2525"/>
        <source>Wnd</source>
        <translation>Wnd</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2610"/>
        <source>Wnd Class</source>
        <translation>Wnd Sınıfı</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2625"/>
        <source>Configure which processes can access desktop objects like windows and alike.</source>
        <translation>Hangi işlemlerin pencereler ve benzeri masaüstü nesnelerine erişebileceğini yapılandırın.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2638"/>
        <source>COM</source>
        <translation>COM</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2684"/>
        <source>Class Id</source>
        <translation>Sınıf Id</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2731"/>
        <source>Configure which processes can access COM objects.</source>
        <translation>Hangi işlemlerin COM nesnelerine erişebileceğini yapılandırın.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2743"/>
        <source>Don&apos;t use virtualized COM, Open access to hosts COM infrastructure (not recommended)</source>
        <translation>Sanallaştırılmış COM kullanılmasın, Ana bilgisayar COM altyapısına açık erişim sağlar (Önerilmez)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2751"/>
        <source>Access Policies</source>
        <translation>Erişim İlkeleri</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2785"/>
        <source>Apply Close...=!&lt;program&gt;,... rules also to all binaries located in the sandbox.</source>
        <translation>Close...=!&lt;program&gt;,... kurallarını korumalı alanda bulunan tüm ikili dosyalara da uygula.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2881"/>
        <source>Network Options</source>
        <translation>Ağ Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3161"/>
        <source>Quick Recovery</source>
        <translation>Hızlı Kurtarma</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3240"/>
        <source>Immediate Recovery</source>
        <translation>Anında Kurtarma</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3337"/>
        <source>Various Options</source>
        <translation>Çeşitli Seçenekler</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3396"/>
        <source>Apply ElevateCreateProcess Workaround (legacy behaviour)</source>
        <translation>ElevateCreateProcess geçici çözümünü uygula (Eski davranış)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3458"/>
        <source>Use desktop object workaround for all processes</source>
        <translation>Masaüstü nesnesi geçici çözümünü tüm işlemler için kullan</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3498"/>
        <source>This command will be run before the box content will be deleted</source>
        <translation>Bu komut, alan içeriği silinmeden önce çalıştırılacaktır</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3552"/>
        <source>On File Recovery</source>
        <translation>Dosya Kurtarmada</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3581"/>
        <source>This command will be run before a file is being recovered and the file path will be passed as the first argument. If this command returns anything other than 0, the recovery will be blocked</source>
        <translation>Bu komut, bir dosya kurtarılmadan önce çalıştırılacak ve dosya yolu ilk bağımsız değişken olarak aktarılacaktır. Bu komut 0&apos;dan başka bir değer döndürürse, kurtarma işlemi engellenecektir</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3584"/>
        <source>Run File Checker</source>
        <translation>Dosya Denetleyicisini Çalıştır</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3700"/>
        <source>On Delete Content</source>
        <translation>İçerik Silmede</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3781"/>
        <source>Protect processes in this box from being accessed by specified unsandboxed host processes.</source>
        <translation>Bu alandaki işlemlere korumalı alan dışındaki ana bilgisayar işlemleri tarafından erişilmesini engeller.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3792"/>
        <location filename="Forms/OptionsWindow.ui" line="3856"/>
        <source>Process</source>
        <translation>İşlem</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3832"/>
        <source>Block also read access to processes in this sandbox</source>
        <translation>Bu korumalı alandaki işlemlere okuma erişimini de engelle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3982"/>
        <source>Add Option</source>
        <translation>Seçenek Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3996"/>
        <source>Here you can configure advanced per process options to improve compatibility and/or customize sandboxing behavior.</source>
        <translation>Burada, uyumluluğu artırmak veya korumalı alan davranışını özelleştirmek için işlem başına gelişmiş seçenekleri yapılandırabilirsiniz.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4007"/>
        <source>Option</source>
        <translation>Seçenek</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4056"/>
        <source>API call trace (requires LogAPI to be installed in the Sbie directory)</source>
        <translation>API çağrı izleme (sbie dizinine logAPI yüklenmesini gerektirir)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4096"/>
        <source>Log all SetError&apos;s to Trace log (creates a lot of output)</source>
        <translation>Tüm SetError Mesajlarını İzleme Günlüğüne Kaydet (Çok fazla çıktı oluşturur)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4143"/>
        <source>Ntdll syscall Trace (creates a lot of output)</source>
        <translation>Ntdll Syscall İzleme (Çok fazla çıktı oluşturur)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="460"/>
        <source>Prompt user for large file migration</source>
        <translation>Büyük dosya taşınması için kullanıcıya sor</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="641"/>
        <source>Block read access to the clipboard</source>
        <translation>Panoya okuma erişimini engelle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3410"/>
        <source>Emulate sandboxed window station for all processes</source>
        <translation>Tüm işlemler için korumalı alan pencere istasyonunu taklit et</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4409"/>
        <source>Add Template</source>
        <translation>Şablon Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4468"/>
        <source>Template Folders</source>
        <translation>Şablon Klasörleri</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4488"/>
        <source>Configure the folder locations used by your other applications.

Please note that this values are currently user specific and saved globally for all boxes.</source>
        <translation>Diğer uygulamalarınız tarafından kullanılan klasör konumlarını yapılandırın.

Lütfen bu değerlerin kullanıcıya özel olduğunu ve tüm alanlar için global olarak kaydedildiğini unutmayın.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4017"/>
        <location filename="Forms/OptionsWindow.ui" line="4519"/>
        <source>Value</source>
        <translation>Değer</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4530"/>
        <source>Accessibility</source>
        <translation>Erişilebilirlik</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4626"/>
        <source>To compensate for the lost protection, please consult the Drop Rights settings page in the Restrictions settings group.</source>
        <translation>Kaybedilen korumayı telafi etmek için lütfen Güvenlik Seçenekleri &gt; Güvenlik Sıkılaştırması altındaki Yetkilendirme Kısıtlamaları bölümü Hak Bırakma ayarlarına bakın.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4550"/>
        <source>Screen Readers: JAWS, NVDA, Window-Eyes, System Access</source>
        <translation>Ekran Okuyucuları: JAWS, NVDA, Window-Eyes, System Access</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4590"/>
        <source>The following settings enable the use of Sandboxie in combination with accessibility software.  Please note that some measure of Sandboxie protection is necessarily lost when these settings are in effect.</source>
        <translation>Aşağıdaki ayarlar, Sandboxie&apos;nin erişilebilirlik yazılımıyla birlikte kullanılmasını sağlar. Lütfen bu ayarlar etkin olduğunda Sandboxie korumasının bir kısmının ister istemez kaybedildiğini unutmayın.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1094"/>
        <source>CAUTION: When running under the built in administrator, processes can not drop administrative privileges.</source>
        <translation>DİKKAT: Yerleşik yönetici altında çalışırken, işlemler yönetici ayrıcalıklarını bırakamaz.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1204"/>
        <source>Open access to Windows Security Account Manager</source>
        <translation>Windows Güvenlik Hesap Yöneticisine açık erişim</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4157"/>
        <source>Disable Resource Access Monitor</source>
        <translation>Kaynak Erişim İzleyicisini Devre Dışı Bırak</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4185"/>
        <source>Resource Access Monitor</source>
        <translation>Kaynak Erişim İzleyicisi</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="172"/>
        <source>Show this box in the &apos;run in box&apos; selection prompt</source>
        <translation>Bu alanı &apos;alanda çalıştır&apos; seçim isteminde göster</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1121"/>
        <source>Security note: Elevated applications running under the supervision of Sandboxie, with an admin or system token, have more opportunities to bypass isolation and modify the system outside the sandbox.</source>
        <translation>Güvenlik notu: Sandboxie&apos;nin gözetimi altında, bir yönetici veya sistem belirteci ile çalışan yetkilendirilmiş uygulamalar, yalıtımı atlamak ve sistemi korumalı alanın dışında değiştirmek için daha fazla fırsata sahiptir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1047"/>
        <source>Allow MSIServer to run with a sandboxed system token and apply other exceptions if required</source>
        <translation>MSIServer&apos;ın korumalı alan sistem belirteci ile çalışmasına ve gerekirse diğer istisnaları uygulamasına izin ver</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1104"/>
        <source>Note: Msi Installer Exemptions should not be required, but if you encounter issues installing a msi package which you trust, this option may help the installation complete successfully. You can also try disabling drop admin rights.</source>
        <translation>Not: Msi Yükleyici İstisnaları gerekli olmamalıdır, ancak güvendiğiniz bir msi paketini kurarken sorunlarla karşılaşırsanız, bu seçenek kurulumun başarıyla tamamlanmasına yardımcı olabilir. Yönetici haklarını bırakmayı devre dışı hale getirmeyi de deneyebilirsiniz.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1040"/>
        <source>Security enhancements</source>
        <translation>Güvenlik Geliştirmeleri</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="971"/>
        <source>Use the original token only for approved NT system calls</source>
        <translation>Asıl belirteci yalnızca onaylı NT sistem çağrıları için kullan</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1023"/>
        <source>Restrict driver/device access to only approved ones</source>
        <translation>Sürücü/cihaz erişimini yalnızca onaylanmış olanlarla kısıtla</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="978"/>
        <source>Enable all security enhancements (make security hardened box)</source>
        <translation>Tüm güvenlik geliştirmelerini etkinleştir (Güvenliği güçlendirilmiş alan yapar)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1630"/>
        <source>You can group programs together and give them a group name.  Program groups can be used with some of the settings instead of program names. Groups defined for the box overwrite groups defined in templates.</source>
        <translation>Programları birlikte gruplayabilir ve onlara bir grup adı verebilirsiniz. Program grupları, program adları yerine bazı ayarlarla kullanılabilir. Alan için tanımlanan gruplar, şablonlarda tanımlanan grupların üzerine yazılır.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2891"/>
        <source>Process Restrictions</source>
        <translation>İşlem Kısıtlamaları</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2978"/>
        <source>Set network/internet access for unlisted processes:</source>
        <translation>Listelenmemiş işlemler için ağ/internet erişimini ayarla:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3025"/>
        <source>Test Rules, Program:</source>
        <translation>Kuralları test et, Program:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3035"/>
        <source>Port:</source>
        <translation>Port:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3045"/>
        <source>IP:</source>
        <translation>IP:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3055"/>
        <source>Protocol:</source>
        <translation>Protokol:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3065"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3081"/>
        <source>Add Rule</source>
        <translation>Kural Ekle</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="561"/>
        <location filename="Forms/OptionsWindow.ui" line="3097"/>
        <location filename="Forms/OptionsWindow.ui" line="3536"/>
        <location filename="Forms/OptionsWindow.ui" line="3861"/>
        <source>Action</source>
        <translation>Eylem</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3102"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3107"/>
        <source>IP</source>
        <translation>IP</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3112"/>
        <source>Protocol</source>
        <translation>Protokol</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3134"/>
        <source>CAUTION: Windows Filtering Platform is not enabled with the driver, therefore these rules will be applied only in user mode and can not be enforced!!! This means that malicious applications may bypass them.</source>
        <translation>DİKKAT: Windows Filtreleme Platformu sürücü ile etkinleştirilmemiştir, bu nedenle bu kurallar yalnızca kullanıcı modunda uygulanacaktır ve zorlanmaz! Bu, kötü amaçlı uygulamaların bunları atlayabileceği anlamına gelir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1279"/>
        <source>Allow sandboxed programs to manage Hardware/Devices</source>
        <translation>Korumalı alan programlarının Donanım/Aygıtları yönetmesine izin ver</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1238"/>
        <source>Open access to Windows Local Security Authority</source>
        <translation>Windows Yerel Güvenlik Yetkilisine açık erişim</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2993"/>
        <location filename="Forms/OptionsWindow.ui" line="4233"/>
        <source>Network Firewall</source>
        <translation>Ağ Güvenlik Duvarı</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="238"/>
        <source>General Configuration</source>
        <translation>Genel Yapılandırma</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="211"/>
        <source>Box Type Preset:</source>
        <translation>Alan türü ön ayarı:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="179"/>
        <source>Box info</source>
        <translation>Alan bilgisi</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="142"/>
        <source>&lt;b&gt;More Box Types&lt;/b&gt; are exclusively available to &lt;u&gt;project supporters&lt;/u&gt;, the Privacy Enhanced boxes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;protect user data from illicit access&lt;/font&gt;&lt;/b&gt; by the sandboxed programs.&lt;br /&gt;If you are not yet a supporter, then please consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt;, to receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;.&lt;br /&gt;You can test the other box types by creating new sandboxes of those types, however processes in these will be auto terminated after 5 minutes.</source>
        <translation>&lt;b&gt;Daha Fazla Alan Türü&lt;/b&gt; yalnızca &lt;u&gt;proje destekçileri&lt;/u&gt; tarafından kullanılabilir, Gelişmiş Gizlilik alanları &lt;b&gt;&lt;font color=&apos;red&apos;&gt;kullanıcı verilerini korumalı alandaki programların yetkisiz erişimine karşı korur.&lt;/font&gt;&lt;/b&gt;&lt;br /&gt;Henüz destekçi değilseniz, lütfen &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;projeyi desteklemeyi&lt;/a&gt; düşünün. &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;destekçi sertifikası&lt;/a&gt;.&lt;br /&gt;Diğer alan türlerini, bu türlerden yeni korumalı alanlar oluşturarak test edebilirsiniz, ancak bu alanlardaki işlemler 5 dakika sonra otomatik olarak sonlandırılacaktır.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="621"/>
        <source>Open Windows Credentials Store (user mode)</source>
        <translation>Windows Kimlik Bilgileri Deposunu aç (Kullanıcı modu)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="661"/>
        <source>Prevent change to network and firewall parameters (user mode)</source>
        <translation>Ağ ve güvenlik duvarı parametrelerinde değişikliği engelle (Kullanıcı modu)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2792"/>
        <source>Prioritize rules based on their Specificity and Process Match Level</source>
        <translation>Özgüllüklerine ve İşlem Eşleştirme Düzeylerine göre kurallara öncelik ver</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2844"/>
        <source>Privacy Mode, block file and registry access to all locations except the generic system ones</source>
        <translation>Gizlilik Modu, genel sistem konumları dışındaki tüm konumlara yapılan dosya ve kayıt defteri erişimini engeller</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2826"/>
        <source>Access Mode</source>
        <translation>Erişim Modu</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2864"/>
        <source>When the Privacy Mode is enabled, sandboxed processes will be only able to read C:\Windows\*, C:\Program Files\*, and parts of the HKLM registry, all other locations will need explicit access to be readable and/or writable. In this mode, Rule Specificity is always enabled.</source>
        <translation>Gizlilik Modu etkinleştirildiğinde, korumalı alan işlemleri yalnızca C:\Windows\*, C:\Program Files\* ve HKLM kayıt defterinin bölümlerini okuyabilir, diğer tüm konumların okunabilir ve/veya yazılabilir olması için açık erişime ihtiyacı olacaktır. Bu modda, Kural Özgüllüğü her zaman etkindir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2778"/>
        <source>Rule Policies</source>
        <translation>Kural İlkeleri</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2799"/>
        <source>Apply File and Key Open directives only to binaries located outside the sandbox.</source>
        <translation>Dosya ve Anahtar Açma yönergelerini yalnızca korumalı alanın dışında bulunan ikili dosyalara uygula.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1436"/>
        <source>Start the sandboxed RpcSs as a SYSTEM process (not recommended)</source>
        <translation>Korumalı alanlı RpcS&apos;leri bir SİSTEM işlemi olarak başlat (Önerilmez)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1525"/>
        <source>Allow only privileged processes to access the Service Control Manager</source>
        <translation>Yalnızca ayrıcalıklı işlemlerin Hizmet Kontrol Yöneticisine erişmesine izin ver</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1498"/>
        <source>Drop critical privileges from processes running with a SYSTEM token</source>
        <translation>Bir SİSTEM belirteci ile çalışan işlemlerden kritik ayrıcalıkları düşür</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1491"/>
        <location filename="Forms/OptionsWindow.ui" line="1539"/>
        <source>(Security Critical)</source>
        <translation>(Güvenlik Açısından Kritik)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1460"/>
        <source>Protect sandboxed SYSTEM processes from unprivileged processes</source>
        <translation>Korumalı SİSTEM işlemlerini ayrıcalıksız işlemlerden koru</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1228"/>
        <source>Security Isolation through the usage of a heavily restricted process token is Sandboxie&apos;s primary means of enforcing sandbox restrictions, when this is disabled the box is operated in the application compartment mode, i.e. it’s no longer providing reliable security, just simple application compartmentalization.</source>
        <translation>Güvenlik Yalıtımı, Sandboxie&apos;nin çok kısıtlı işlem belirteci kullanımı yoluyla korumalı alan kısıtlamalarını uygulamasının birincil yoludur. Bu devre dışı bırakılırsa, alan, uygulama bölmesi modunda çalıştırılır, yani artık sağlıklı bir güvenlik sağlayamaz ve yalnızca basit uygulama bölümlemesi sağlar.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1197"/>
        <source>Disable Security Isolation (experimental)</source>
        <translation>Güvenlik Yalıtımını devre dışı bırak (Deneysel)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1255"/>
        <source>Security Isolation &amp; Filtering</source>
        <translation>Güvenlik Yalıtımı &amp; Filtreleme</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1211"/>
        <source>Disable Security Filtering (not recommended)</source>
        <translation>Güvenlik Filtrelemeyi devre dışı bırak (Önerilmez)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1218"/>
        <source>Security Filtering used by Sandboxie to enforce filesystem and registry access restrictions, as well as to restrict process access.</source>
        <translation>Güvenlik Filtreme, Sandboxie tarafından dosya sistemi ve kayıt defteri erişim kısıtlamalarını yürütmek ve aynı zamanda işlem erişimini kısıtlamak için kullanılır.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1286"/>
        <source>The below options can be used safely when you don&apos;t grant admin rights.</source>
        <translation>Yönetici hakları verilmediğinde aşağıdaki seçenekler güvenle kullanılabilir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="89"/>
        <source>Always show this sandbox in the systray list (Pinned)</source>
        <translation>Bu korumalı alanı her zaman sistem tepsisi listesinde göster (Sabitlenmiş)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2833"/>
        <source>The rule specificity is a measure to how well a given rule matches a particular path, simply put the specificity is the length of characters from the begin of the path up to and including the last matching non-wildcard substring. A rule which matches only file types like &quot;*.tmp&quot; would have the highest specificity as it would always match the entire file path.
The process match level has a higher priority than the specificity and describes how a rule applies to a given process. Rules applying by process name or group have the strongest match level, followed by the match by negation (i.e. rules applying to all processes but the given one), while the lowest match levels have global matches, i.e. rules that apply to any process.</source>
        <translation>Kural özgüllüğü, belirli bir kuralın belirli bir yolla ne kadar iyi eşleştiğinin bir ölçüsüdür. Basitçe söylemek gerekirse, özgüllük son eşleşen özel karakter olmayan alt dize dahil yolun başından sonuna kadar olan karakterlerin uzunluğudur. Yalnızca &quot;*.tmp&quot; gibi dosya türleriyle eşleşen bir kural, her zaman tüm dosya yolu ile eşleşeceği için en yüksek özgüllüğe sahip olacaktır.
İşlem eşleştirme düzeyi, özgüllükten daha yüksek bir önceliğe sahiptir ve bir kuralın belirli bir işleme nasıl uygulanacağını tanımlar. İşlem adına veya grubuna göre uygulanan kurallar en güçlü eşleştirme düzeyine sahiptir. Ardından olumsuzlama ile eşleştirme gelir, yani belirtilen işlem dışındaki tüm işlemlere uygulanan kurallara aittir; en düşük eşleştirme düzeyleri ise genel eşleştirmelere, yani herhangi bir işleme uygulanan kurallara aittir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3451"/>
        <source>Disable the use of RpcMgmtSetComTimeout by default (this may resolve compatibility issues)</source>
        <translation>Varsayılan olarak RpcMgmtSetComTimeout kullanımını devre dışı bırak (Uyumluluk sorunlarını çözebilir)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3468"/>
        <source>Triggers</source>
        <translation>Tetikleyiciler</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3531"/>
        <source>Event</source>
        <translation>Olay</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3501"/>
        <location filename="Forms/OptionsWindow.ui" line="3520"/>
        <location filename="Forms/OptionsWindow.ui" line="3658"/>
        <source>Run Command</source>
        <translation>Komut Çalıştır</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3690"/>
        <source>Start Service</source>
        <translation>Hizmet Başlat</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3665"/>
        <source>These events are executed each time a box is started</source>
        <translation>Bu olaylar, bir alan her başlatıldığında yürütülür</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3668"/>
        <source>On Box Start</source>
        <translation>Alan Başlangıcında</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3549"/>
        <location filename="Forms/OptionsWindow.ui" line="3697"/>
        <source>These commands are run UNBOXED just before the box content is deleted</source>
        <translation>Bu komutlar, alan içeriği silinmeden hemen önce ALAN DIŞINDAN çalıştırılır</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3623"/>
        <source>These commands are executed only when a box is initialized. To make them run again, the box content must be deleted.</source>
        <translation>Bu komutlar yalnızca bir alan ilk kullanıma hazırlandığında yürütülür. Tekrar çalıştırabilmeleri için alan içeriğinin silinmesi gerekir.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3626"/>
        <source>On Box Init</source>
        <translation>Alan İlk Kullanımında</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3636"/>
        <source>Here you can specify actions to be executed automatically on various box events.</source>
        <translation>Burada, çeşitli alan olaylarında otomatik olarak yürütülecek eylemleri belirleyebilirsiniz.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="668"/>
        <source>Allow to read memory of unsandboxed processes (not recommended)</source>
        <translation>Korumalı alanda olmayan işlemlerin belleğini okumaya izin ver (Önerilmez)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="716"/>
        <source>Issue message 2111 when a process access is denied</source>
        <translation>Bir işlem erişimi reddedildiğinde 2111 mesajını yayınla</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3431"/>
        <source>Allow use of nested job objects (works on Windows 8 and later)</source>
        <translation>İç içe iş nesnelerinin kullanımına izin ver (Windows 8 ve sonraki sürümlerde çalışır)</translation>
    </message>
</context>
<context>
    <name>PopUpWindow</name>
    <message>
        <location filename="Forms/PopUpWindow.ui" line="32"/>
        <source>SandboxiePlus Notifications</source>
        <translation>SandboxiePlus Bildirimleri</translation>
    </message>
</context>
<context>
    <name>ProgramsDelegate</name>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="55"/>
        <source>Group: %1</source>
        <translation>Grup: %1</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="Views/SbieView.cpp" line="1497"/>
        <source>Drive %1</source>
        <translation>Sürücü %1</translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="SandMan.cpp" line="3604"/>
        <source>OK</source>
        <translation>TAMAM</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3605"/>
        <source>Apply</source>
        <translation>Uygula</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3606"/>
        <source>Cancel</source>
        <translation>İptal</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3607"/>
        <source>&amp;Yes</source>
        <translation>&amp;Evet</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3608"/>
        <source>&amp;No</source>
        <translation>&amp;Hayır</translation>
    </message>
</context>
<context>
    <name>RecoveryWindow</name>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="164"/>
        <source>Close</source>
        <translation>Kapat</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="42"/>
        <source>Add Folder</source>
        <translation>Klasör Ekle</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="110"/>
        <source>Recover</source>
        <translation>Kurtar</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="78"/>
        <source>Refresh</source>
        <translation>Yenile</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="142"/>
        <source>Delete</source>
        <translation>Sil</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="171"/>
        <source>Show All Files</source>
        <translation>Tüm Dosyaları Göster</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="184"/>
        <source>TextLabel</source>
        <translation>Metin Etiketi</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="32"/>
        <source>SandboxiePlus - Recovery</source>
        <translation>SandboxiePlus - Kurtarma</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="68"/>
        <source>Recover target:</source>
        <translation>Hedefe kurtar:</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="61"/>
        <source>Delete Content</source>
        <translation>İçeriği Sil</translation>
    </message>
</context>
<context>
    <name>SelectBoxWindow</name>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="32"/>
        <source>SandboxiePlus select box</source>
        <translation>SandboxiePlus alan seç</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="87"/>
        <source>Select the sandbox in which to start the program, installer or document.</source>
        <translation>Programın, yükleyicinin veya belgenin başlatılacağı korumalı alanı seçin.</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="80"/>
        <source>Run As UAC Administrator</source>
        <translation>UAC Yöneticisi Olarak Çalıştır</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="47"/>
        <source>Run Sandboxed</source>
        <translation>Korumalı Alanda Çalıştır</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="73"/>
        <source>Run Outside the Sandbox</source>
        <translation>Korumalı Alanın Dışında Çalıştır</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="97"/>
        <source>Run in a new Sandbox</source>
        <translation>Yeni Korumalı Alanda Çalıştır</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="65"/>
        <source>Sandbox</source>
        <translation>Korumalı Alan</translation>
    </message>
</context>
<context>
    <name>SettingsWindow</name>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="665"/>
        <location filename="Forms/SettingsWindow.ui" line="1676"/>
        <location filename="Forms/SettingsWindow.ui" line="1786"/>
        <location filename="Forms/SettingsWindow.ui" line="1919"/>
        <source>Name</source>
        <translation>Ad</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1681"/>
        <source>Path</source>
        <translation>Yol</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1568"/>
        <source>Change Password</source>
        <translation>Parolayı değiştir</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1616"/>
        <source>Clear password when main window becomes hidden</source>
        <translation>Ana pencere gizlendiğinde parolayı temizle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="32"/>
        <source>SandboxiePlus Settings</source>
        <translation>SandboxiePlus Ayarları</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1575"/>
        <source>Password must be entered in order to make changes</source>
        <translation>Değişiklik yapmak için parola girilmelidir</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1211"/>
        <source>New Versions</source>
        <translation>Yeni sürümler</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1241"/>
        <source>Check periodically for updates of Sandboxie-Plus</source>
        <translation>Sandboxie-Plus güncellemelerini düzenli aralıklarla denetle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="65"/>
        <source>General Options</source>
        <translation>Genel Ayarlar</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="932"/>
        <source>Use Dark Theme</source>
        <translation>Koyu temayı kullan</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1794"/>
        <source>Enable</source>
        <translation>Etkinleştir</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1734"/>
        <source>Add Folder</source>
        <translation>Klasör Ekle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1623"/>
        <source>Only Administrator user accounts can make changes</source>
        <translation>Yalnızca Yönetici hesapları değişiklik yapabilir</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1602"/>
        <source>Config protection</source>
        <translation>Yapılandırma Koruması</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1689"/>
        <source>Add Program</source>
        <translation>Program Ekle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1821"/>
        <source>Sandboxie has detected the following software applications in your system. Click OK to apply configuration settings, which will improve compatibility with these applications. These configuration settings will have effect in all existing sandboxes and in any new sandboxes.</source>
        <translation>Sandboxie, sisteminizde aşağıdaki yazılım uygulamalarını belirledi. Bu uygulamalarla uyumluluğu artıracak yapılandırma ayarlarını uygulamak için Tamam&apos;a tıklayın. Bu yapılandırmalar mevcut tüm korumalı alanlarda ve tüm yeni oluşturulacak alanlarda etkili olacaktır.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1630"/>
        <source>Watch Sandboxie.ini for changes</source>
        <translation>Değişiklikler için Sandboxie.ini dosyasını izle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1778"/>
        <source>In the future, don&apos;t check software compatibility</source>
        <translation>Gelecekte, yazılım uyumluluğu denetlenmesin</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1814"/>
        <source>Disable</source>
        <translation>Devre Dışı Bırak</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1724"/>
        <source>When any of the following programs is launched outside any sandbox, Sandboxie will issue message SBIE1301.</source>
        <translation>Aşağıdaki programlardan herhangi biri korumalı alanın dışında başlatıldığında, Sandboxie SBIE1301 mesajını yayınlayacaktır.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1703"/>
        <source>Remove Program</source>
        <translation>Programı Kaldır</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="534"/>
        <source>Add &apos;Run Sandboxed&apos; to the explorer context menu</source>
        <translation>Dosya gezgini bağlam menüsüne &apos;Korumalı Alanda Çalıştır&apos; seçeneği ekle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1696"/>
        <source>Issue message 1308 when a program fails to start</source>
        <translation>Bir program başlatılamadığında 1308 mesajını yayınla</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1411"/>
        <source>Sandbox default</source>
        <translation>Korumalı Alan Varsayılanları</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1717"/>
        <source>Prevent the listed programs from starting on this system</source>
        <translation>Listelenen programların bu sistemde başlamasını önle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="195"/>
        <source>Open urls from this ui sandboxed</source>
        <translation>Bu kullanıcı arayüzündeki bağlantıları korumalı alanda aç</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1531"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/filerootpath&quot;&gt;file system root&lt;/a&gt;: </source>
        <translation>Korumalı alan &lt;a href=&quot;sbie://docs/filerootpath&quot;&gt;dosya sistemi kökü&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1488"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/ipcrootpath&quot;&gt;ipc root&lt;/a&gt;: </source>
        <translation>Korumalı alan &lt;a href=&quot;sbie://docs/ipcrootpath&quot;&gt;ipc kökü&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1391"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/keyrootpath&quot;&gt;registry root&lt;/a&gt;: </source>
        <translation>Korumalı alan &lt;a href=&quot;sbie://docs/keyrootpath&quot;&gt;kayıt kökü&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1501"/>
        <source>Portable root folder</source>
        <translation>Taşınabilir kök klasörü</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="626"/>
        <source>Start UI with Windows</source>
        <translation>Windows başlangıcında kullanıcı arayüzünü başlat</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="633"/>
        <source>Start UI when a sandboxed process is started</source>
        <translation>Korumalı alanda bir işlem başlatıldığında kullanıcı arayüzünü başlat</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="80"/>
        <source>Show file recovery window when emptying sandboxes</source>
        <translation>Korumalı alanlar boşaltılırken kurtarma penceresini göster</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1481"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="55"/>
        <source>General Config</source>
        <translation>Genel Yapılandırma</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="173"/>
        <source>Hotkey for terminating all boxed processes:</source>
        <translation>Tüm alanlardaki işlemleri sonlandırmak için kısayol tuşu:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="492"/>
        <source>Systray options</source>
        <translation>Sistem Tepsisi Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="185"/>
        <source>UI Language:</source>
        <translation>Kullanıcı arayüzü dili:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="394"/>
        <source>Shell Integration</source>
        <translation>Kabuk Entegrasyonu</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="513"/>
        <source>Run Sandboxed - Actions</source>
        <translation>Korumalı Alanda Çalıştır - Eylemler</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="499"/>
        <source>Always use DefaultBox</source>
        <translation>Her zaman varsayılan korumalı alanı kullan</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="73"/>
        <source>Count and display the disk space occupied by each sandbox</source>
        <translation>Her bir korumalı alanın kapladığı disk alanını hesapla ve görüntüle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="364"/>
        <source>This option also enables asynchronous operation when needed and suspends updates.</source>
        <translation>Bu seçenek ayrıca gerektiğinde eşzamansız çalışmayı da etkinleştirir ve güncellemeleri askıya alır.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="367"/>
        <source>Suppress pop-up notifications when in game / presentation mode</source>
        <translation>Oyun / sunum modundayken açılır bildirimleri bastır</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="577"/>
        <source>Start Sandbox Manager</source>
        <translation>Korumalı Alan Yöneticisini Başlatma</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="475"/>
        <source>Use Compact Box List</source>
        <translation>Kompakt alan listesini kullan</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="800"/>
        <source>Interface Config</source>
        <translation>Arayüz Yapılandırması</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="868"/>
        <source>Show &quot;Pizza&quot; Background in box list *</source>
        <translation>Alan listesinde &quot;Pizza&quot; arka planını göster *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="925"/>
        <source>Make Box Icons match the Border Color</source>
        <translation>Alan simgelerini kenarlık rengiyle eşleştir</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="915"/>
        <source>Use a Page Tree in the Box Options instead of Nested Tabs *</source>
        <translation>Alan seçeneklerinde iç içe sekmeler yerine sayfa ağacı kullan *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="908"/>
        <source>Interface Options</source>
        <translation>Arayüz Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="828"/>
        <source>Use large icons in box list *</source>
        <translation>Alan listesinde büyük simgeler kullan *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="970"/>
        <source>High DPI Scaling</source>
        <translation>Yüksek DPI ölçekleme</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="891"/>
        <source>Don&apos;t show icons in menus *</source>
        <translation>Menülerde simgeleri gösterme *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="980"/>
        <source>Font Scaling</source>
        <translation>Yazı tipi ölçekleme</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1042"/>
        <source>(Restart required)</source>
        <translation>(Yeniden başlatma gereklidir)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="838"/>
        <source>* a partially checked checkbox will leave the behavior to be determined by the view mode.</source>
        <translation>* kısmen işaretlenmiş bir onay kutusu, olması gereken davranışı görüntüleme modu tarafından belirlenecek şekilde bırakacaktır.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="144"/>
        <source>Show the Recovery Window as Always on Top</source>
        <translation>Kurtarma penceresini her zaman üstte göster</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1056"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="942"/>
        <source>Alternate row background in lists</source>
        <translation>Listelerde alternatif satır arka planını göster</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="94"/>
        <source>SandMan Options</source>
        <translation>SandMan Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="208"/>
        <source>Notifications</source>
        <translation>Bildirimler</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="240"/>
        <source>Add Entry</source>
        <translation>Giriş Ekle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="357"/>
        <source>Show file migration progress when copying large files into a sandbox</source>
        <translation>Büyük dosyaları bir korumalı alana kopyalarken dosya taşıma ilerlemesini göster</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="248"/>
        <source>Message ID</source>
        <translation>Mesaj NO</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="253"/>
        <source>Message Text (optional)</source>
        <translation>Mesaj Metni (isteğe bağlı)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="268"/>
        <source>SBIE Messages</source>
        <translation>SBIE Mesajları</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="287"/>
        <source>Delete Entry</source>
        <translation>Girişi Sil</translation>
    </message>
    <message>
        <source>Don&apos;t show the popup message log for all SBIE messages</source>
        <translation type="vanished">Tüm SBIE mesajları için açılan mesaj günlüğünü gösterme</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="301"/>
        <source>Notification Options</source>
        <translation>Bildirim Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="327"/>
        <source>Sandboxie may be issue &lt;a href=&quot;sbie://docs/sbiemessages&quot;&gt;SBIE Messages&lt;/a&gt; to the Message Log and shown them as Popups. Some messages are informational and notify of a common, or in some cases special, event that has occurred, other messages indicate an error condition.&lt;br /&gt;You can hide selected SBIE messages from being popped up, using the below list:</source>
        <translation>Sandboxie, Mesaj Günlüğüne &lt;a href=&quot;sbie://docs/ sbiemessages&quot;&gt;SBIE Mesajları&lt;/a&gt; yayınlayabilir ve bunları Açılır Pencereler olarak gösterebilir. Bazı mesajlar bilgilendirme amaçlıdır ve meydana gelen genel amaçlı veya bazı durumlara özel olayları bildirir, diğer mesajlar ise hata durumunlarını belirtir.&lt;br /&gt;Aşağıdaki listeyi kullanarak seçili SBIE mesajlarının açılmasını engelleyebilirsiniz:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="404"/>
        <source>Windows Shell</source>
        <translation>Windows Kabuğu</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="670"/>
        <source>Icon</source>
        <translation>Simge</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="727"/>
        <source>Move Up</source>
        <translation>Yukarı Taşı</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="746"/>
        <source>Move Down</source>
        <translation>Aşağı Taşı</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="818"/>
        <source>Use Fusion Theme</source>
        <translation>Füzyon temasını kullan</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="949"/>
        <source>Show overlay icons for boxes and processes</source>
        <translation>Alanlar ve işlemler için simge bindirmelerini göster</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="959"/>
        <source>Display Options</source>
        <translation>Görüntüleme Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1083"/>
        <source>Graphic Options</source>
        <translation>Grafik Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1337"/>
        <source>Advanced Config</source>
        <translation>Gelişmiş Yapılandırma</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1451"/>
        <source>Sandboxing features</source>
        <translation>Korumalı Alan Özellikleri</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1521"/>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation>Anonim bir belirteç (deneysel) yerine Sandboxie oturum açmayı kullan</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1560"/>
        <source>Sandboxie.ini Presets</source>
        <translation>Sandboxie.ini Ön Ayarları</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1644"/>
        <source>Program Control</source>
        <translation>Program Denetimi</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1347"/>
        <source>Sandboxie Config</source>
        <translation>Sandboxie Yapılandırması</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1198"/>
        <source>The preview channel contains the latest GitHub pre-releases.</source>
        <translation>Ön izleme kanalı, en son GitHub ön sürümlerini içerir.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1218"/>
        <source>The stable channel contains the latest stable GitHub releases.</source>
        <translation>Kararlı kanal, en son kararlı GitHub sürümlerini içerir.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1221"/>
        <source>Search in the Stable channel</source>
        <translation>Kararlı kanalda ara</translation>
    </message>
    <message>
        <source>Keeping Sandboxie up to date with the rolling releases of Windows and compatible with all web browsers is a never-ending endeavor. Please consider supporting this work with a donation.&lt;br /&gt;You can support the development with a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;PayPal donation&lt;/a&gt;, working also with credit cards.&lt;br /&gt;Or you can provide continuous support with a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Patreon subscription&lt;/a&gt;.</source>
        <translation type="vanished">Sandboxie&apos;yi Windows&apos;un devam eden sürümleriyle güncel ve tüm web tarayıcılarıyla uyumlu tutmak hiç bitmeyen bir çabadır. Lütfen bu çalışmayı bir bağışla desteklemeyi düşünün.&lt;br /&gt;Geliştirmeyi bir &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;PayPal bağışı&lt;/a&gt; ile destekleyebilirsiniz, kredi kartlarıyla da çalışır.&lt;br /&gt;Ya da bir &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Patreon aboneliği&lt;/a&gt; ile sürekli destek sağlayabilirsiniz.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1201"/>
        <source>Search in the Preview channel</source>
        <translation>Önizleme kanalında ara</translation>
    </message>
    <message>
        <source>Supporters of the Sandboxie-Plus project can receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporter certificate&lt;/a&gt;. It&apos;s like a license key but for awesome people using open source software. :-)</source>
        <translation type="vanished">Sandboxie-Plus projesinin destekçileri bir &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;destekçi sertifikası&lt;/a&gt; alabilir. Bir lisans anahtarı gibi ama açık kaynaklı yazılım kullanan harika insanlar için. :-)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1184"/>
        <source>Enter the support certificate here</source>
        <translation>Destek sertifikasını buraya girin</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1268"/>
        <source>Support Settings</source>
        <translation>Destek Ayarları</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="350"/>
        <source>Show recoverable files as notifications</source>
        <translation>Kurtarılabilir dosyaları bildirim olarak göster</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="600"/>
        <source>Show Icon in Systray:</source>
        <translation>Simgeyi sistem tepsisinde göster:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1461"/>
        <source>Use Windows Filtering Platform to restrict network access</source>
        <translation>Ağ erişimini kısıtlamak için Windows Filtreleme Platformunu kullan</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1468"/>
        <source>Hook selected Win32k system calls to enable GPU acceleration (experimental)</source>
        <translation>GPU hızlandırmayı etkinleştirmek için seçili win32k sistem çağrılarını kancala (Deneysel)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="151"/>
        <source>Run box operations asynchronously whenever possible (like content deletion)</source>
        <translation>Alan işlemlerini mümkünse eşzamansız olarak çalıştır (İçerik silme gibi)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="412"/>
        <source>Show boxes in tray list:</source>
        <translation>Alanları tepsi listesinde göster:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="527"/>
        <source>Add &apos;Run Un-Sandboxed&apos; to the context menu</source>
        <translation>Bağlam menüsüne &apos;Korumalı Alanın Dışında Çalıştır&apos; seçeneği ekle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="137"/>
        <source>Recovery Options</source>
        <translation>Kurtarma Seçenekleri</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="442"/>
        <source>Show a tray notification when automatic box operations are started</source>
        <translation>Otomatik alan işlemleri başlatıldığında bir tepsi bildirimi göster</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="435"/>
        <source>Start Menu Integration</source>
        <translation>Başlat Menüsü Entegrasyonu</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="520"/>
        <source>Scan shell folders and offer links in run menu</source>
        <translation>Kabuk klasörlerini tara ve çalıştır menüsünde kısayollar sun</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="541"/>
        <source>Integrate with Host Start Menu</source>
        <translation>Ana Bilgisayar Başlat Menüsü ile entegre et</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="810"/>
        <source>User Interface</source>
        <translation>Kullanıcı Arayüzü</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="858"/>
        <source>Use new config dialog layout *</source>
        <translation>Yeni yapılandırma diyalog düzenini kullan *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="643"/>
        <source>Run Menu</source>
        <translation>Çalıştır Menüsü</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="695"/>
        <source>Add program</source>
        <translation>Program Ekle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="753"/>
        <source>You can configure custom entries for all sandboxes run menus.</source>
        <translation>Tüm korumalı alanların çalıştırma menüsünde görünecek özel girişleri yapılandırabilirsiniz.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="775"/>
        <location filename="Forms/SettingsWindow.ui" line="1866"/>
        <source>Remove</source>
        <translation>Kaldır</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="675"/>
        <source>Command Line</source>
        <translation>Komut Satırı</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="340"/>
        <source>Disable SBIE messages popups (they will still be logged to the Messages tab)</source>
        <translation>SBIE mesajları için açılır pencereleri devre dışı bırak (bunlar yine de Mesajlar sekmesine kaydedilecektir)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1097"/>
        <source>Support &amp;&amp; Updates</source>
        <translation>Destek &amp;&amp; Güncellemeler</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1228"/>
        <source>Supporters of the Sandboxie-Plus project can receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;. It&apos;s like a license key but for awesome people using open source software. :-)</source>
        <translation>Sandboxie-Plus projesinin destekçileri bir &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;destekçi sertifikası&lt;/a&gt; alabilir. Bir lisans anahtarı gibi ama açık kaynaklı yazılım kullanan harika insanlar için. :-)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1321"/>
        <source>Keeping Sandboxie up to date with the rolling releases of Windows and compatible with all web browsers is a never-ending endeavor. You can support the development by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;directly contributing to the project&lt;/a&gt;, showing your support by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;purchasing a supporter certificate&lt;/a&gt;, becoming a patron by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;subscribing on Patreon&lt;/a&gt;, or through a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;PayPal donation&lt;/a&gt;.&lt;br /&gt;Your support plays a vital role in the advancement and maintenance of Sandboxie.</source>
        <translation>Sandboxie&apos;yi Windows&apos;un devam eden sürümleri ve tüm web tarayıcıları ile uyumlu tutmak, hiç bitmeyen bir çabadır. Projeye &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;doğrudan katkıda bulunarak&lt;/a&gt;, &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;destekçi sertifikası satın alarak&lt;/a&gt;, &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Patreon&apos;a abone olarak&lt;/a&gt; veya &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;PayPal bağışı yaparak&lt;/a&gt; projenin gelişimini destekleyebilirsiniz.&lt;br /&gt;Desteğiniz, Sandboxie&apos;nin ilerlemesi ve sürdürülmesinde hayati bir rol oynar.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1371"/>
        <source>Activate Kernel Mode Object Filtering</source>
        <translation>Kernel modu nesne filtrelemeyi etkinleştir</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1418"/>
        <source>Default sandbox:</source>
        <translation>Varsayılan korumalı alan:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1654"/>
        <source>Program Alerts</source>
        <translation>Program Uyarıları</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1710"/>
        <source>Issue message 1301 when forced processes has been disabled</source>
        <translation>Zorunlu işlemler devre dışı bırakıldığında 1301 mesajını yayınla</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1748"/>
        <source>App Templates</source>
        <translation>Uygulama Şablonları</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1758"/>
        <source>App Compatibility</source>
        <translation>Uygulama Uyumluluğu</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1834"/>
        <source>Local Templates</source>
        <translation>Yerel Şablonlar</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1885"/>
        <source>Add Template</source>
        <translation>Şablon Ekle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1905"/>
        <source>Text Filter</source>
        <translation>Metin Filtresi</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1927"/>
        <source>This list contains user created custom templates for sandbox options</source>
        <translation>Bu liste, korumalı alan seçenekleri için kullanıcı tarafından oluşturulan özel şablonları içerir</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1944"/>
        <source>Edit ini Section</source>
        <translation>Ini Düzenleme Bölümü</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1953"/>
        <source>Save</source>
        <translation>Kaydet</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1960"/>
        <source>Edit ini</source>
        <translation>Ini Düzenle</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1986"/>
        <source>Cancel</source>
        <translation>İptal</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1147"/>
        <source>Version Updates</source>
        <translation>Sürüm güncellemeleri</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1208"/>
        <source>New full versions from the selected release channel.</source>
        <translation>Seçili yayın kanalından yeni tam sürümler.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1144"/>
        <source>Hotpatches for the installed version, updates to the Templates.ini and translations.</source>
        <translation>Kurulu sürüm için hızlı yamalar, Templates.ini ve çeviri güncellemeleri.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1161"/>
        <source>This supporter certificate has expired, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>Bu destekçi sertifikasının süresi doldu, lütfen &lt;a href=&quot;sbie://update/cert&quot;&gt;yenilenmiş bir sertifika alın&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1171"/>
        <source>In the future, don&apos;t notify about certificate expiration</source>
        <translation>Gelecekte, sertifika süresinin dolmasıyla ilgili bildirimde bulunma</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1609"/>
        <source>Only Administrator user accounts can use Pause Forcing Programs command</source>
        <translation>Yalnızca Yönetici hesapları Programları Zorlamayı Duraklat komutunu kullanabilir</translation>
    </message>
</context>
<context>
    <name>SnapshotsWindow</name>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="66"/>
        <source>Name:</source>
        <translation>Ad:</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="189"/>
        <source>Remove Snapshot</source>
        <translation>Anlık Görüntüyü Kaldır</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="120"/>
        <source>Description:</source>
        <translation>Açıklama:</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="208"/>
        <source>Go to Snapshot</source>
        <translation>Anlık Görüntüye Git</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="157"/>
        <source>Take Snapshot</source>
        <translation>Anlık Görüntü Al</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="60"/>
        <source>Selected Snapshot Details</source>
        <translation>Seçili Anlık Görüntü Ayrıntıları</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="139"/>
        <source>Snapshot Actions</source>
        <translation>Anlık Görüntü Eylemleri</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="32"/>
        <source>SandboxiePlus - Snapshots</source>
        <translation>SandboxiePlus - Anlık Görüntüler</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="101"/>
        <source>When deleting a snapshot content, it will be returned to this snapshot instead of none.</source>
        <translation>Anlık görüntüsü alınmış bir alanın içeriği silinirken, alanın içeriği varsayılan anlık görüntüye döndürülecektir.</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="104"/>
        <source>Default snapshot</source>
        <translation>Varsayılan anlık görüntü</translation>
    </message>
</context>
</TS>
