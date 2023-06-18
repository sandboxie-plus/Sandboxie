<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_TW">
<context>
    <name>CAdvancedPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="428"/>
        <source>Advanced Sandbox options</source>
        <translation>進階沙箱選項</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="429"/>
        <source>On this page advanced sandbox options can be configured.</source>
        <translation>在此頁面上，可以設定進階沙箱選項。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="434"/>
        <source>Network Access</source>
        <translation>區域網路存取</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="442"/>
        <source>Allow network/internet access</source>
        <translation>允許區域網路/網際網路存取</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="443"/>
        <source>Block network/internet by denying access to Network devices</source>
        <translation>透過拒絕存取區域網路裝置來阻止區域網路/網際網路</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="445"/>
        <source>Block network/internet using Windows Filtering Platform</source>
        <translation>使用 Windows 篩選平台阻止區域網路/網際網路</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="450"/>
        <source>Allow access to network files and folders</source>
        <oldsource>Allow access to network files and fodlers</oldsource>
        <translation>允許存取區域網路檔案和資料夾</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="451"/>
        <location filename="Wizards/NewBoxWizard.cpp" line="467"/>
        <source>This option is not recommended for Hardened boxes</source>
        <oldsource>This option is not recomended for Hardened boxes</oldsource>
        <translation>不建議將此選項用於加固型沙箱</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="457"/>
        <source>Admin Options</source>
        <translation>管理員選項</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="461"/>
        <source>Make applications think they are running elevated</source>
        <translation>使應用程式認為其已在權限提升狀態下執行</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="466"/>
        <source>Allow MSIServer to run with a sandboxed system token</source>
        <translation>允許 MSIServer 使用沙箱化系統權杖執行</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="472"/>
        <source>Box Options</source>
        <translation>沙箱選項</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="476"/>
        <source>Use a Sandboxie login instead of an anonymous token</source>
        <translation>使用 Sandboxie 登入程序替代匿名權杖</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="482"/>
        <source>Prevent sandboxes programs installed on host from loading dll&apos;s from the sandbox</source>
        <translatorcomment>&quot;應用程式擴充&quot; is the actual translation showed in Windows for TradChinese</translatorcomment>
        <translation>防止安裝在主機上各沙箱的程式從沙箱內部載入應用程式擴充 (DLL) 檔案</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="483"/>
        <source>This feature may reduce compatibility as it also prevents box located processes from writing to host located ones and even starting them.</source>
        <oldsource>This feature may reduce compatybility as it also prevents box located processes from writing to host located once and even starting them.</oldsource>
        <translation>此功能可能降低相容性，因為這將阻止沙箱中的處理程序寫入到主機對應目標，甚至於阻止處理程序本身的啟動。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="477"/>
        <source>Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.</source>
        <translation>使用自訂 Sandboxie 權杖可以更好地將各個沙箱相互隔離，同時可以實現在工作管理員的使用者欄位中顯示處理程序所屬的沙箱。但是，某些第三方安全性解決方案可能會與自訂權杖產生相容性問題。</translation>
    </message>
</context>
<context>
    <name>CBoxTypePage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="168"/>
        <source>Create new Sandbox</source>
        <translation>建立新沙箱</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="175"/>
        <source>A sandbox isolates your host system from processes running within the box, it prevents them from making permanent changes to other programs and data in your computer. The level of isolation impacts your security as well as the compatibility with applications, hence there will be a different level of isolation depending on the selected Box Type. Sandboxie can also protect your personal data from being accessed by processes running under its supervision.</source>
        <translation>沙箱會將您的主機系統與沙箱內執行的處理程序隔離開來，以防止它們對電腦中的其他程式和資料進行永久性變更。隔離的級別會影響您的安全性以及與應用程式的相容性，因此根據所選的沙箱類型，將有不同的隔離級別。Sandboxie 還可以保護您的個人資料在其監督下不會被執行的處理程序存取。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="184"/>
        <source>Enter box name:</source>
        <translation>輸入沙箱名稱:</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="202"/>
        <source>Select box type:</source>
        <oldsource>Sellect box type:</oldsource>
        <translation>選擇沙箱類型:</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="205"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>具有資料保護功能的加固型沙箱</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="206"/>
        <source>Security Hardened Sandbox</source>
        <translation>安全性防護加固型沙箱</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="207"/>
        <source>Sandbox with Data Protection</source>
        <translation>資料保護型沙箱</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="208"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>標準隔離型沙箱 (預設)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="210"/>
        <source>Application Compartment with Data Protection</source>
        <translation>資料保護型應用程式區間</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="211"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>應用程式區間 (無隔離防護)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="229"/>
        <source>Remove after use</source>
        <translation>使用後移除</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="230"/>
        <source>After the last process in the box terminates, all data in the box will be deleted and the box itself will be removed.</source>
        <translation>當沙箱中的最後一個處理程序終止後，沙箱中的所有資料將被刪除，沙箱本身也將會刪除。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="235"/>
        <source>Configure advanced options</source>
        <translation>設定進階選項組態</translation>
    </message>
</context>
<context>
    <name>CBrowserOptionsPage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="844"/>
        <source>Create Web Browser Template</source>
        <translation>建立網頁瀏覽器範本</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="845"/>
        <source>Configure web browser template options.</source>
        <translation>設定網頁瀏覽器範本選項組態。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="855"/>
        <source>Force the Web Browser to run in this sandbox</source>
        <translation>強制網頁瀏覽器在此沙箱中執行</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="859"/>
        <source>Allow direct access to the entire Web Browser profile folder</source>
        <translation>允許直接存取整個網頁瀏覽器的設定檔資料夾</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="863"/>
        <source>Allow direct access to Web Browser&apos;s phishing database</source>
        <translation>允許直接存取網頁瀏覽器的反網路釣魚資料庫</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="867"/>
        <source>Allow direct access to Web Browser&apos;s session management</source>
        <translatorcomment>Session is &quot;工作階段&quot; in TradChinese</translatorcomment>
        <translation>允許直接存取網頁瀏覽器的工作階段管理</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="871"/>
        <source>Allow direct access to Web Browser&apos;s sync data</source>
        <translation>允許直接存取網頁瀏覽器的同步資料</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="875"/>
        <source>Allow direct access to Web Browser&apos;s preferences</source>
        <translation>允許直接存取網頁瀏覽器的偏好設定</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="879"/>
        <source>Allow direct access to Web Browser&apos;s passwords</source>
        <translation>允許直接存取網頁瀏覽器儲存的密碼</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="883"/>
        <source>Allow direct access to Web Browser&apos;s cookies</source>
        <translation>允許直接存取網頁瀏覽器儲存的 Cookies</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="887"/>
        <source>Allow direct access to Web Browser&apos;s bookmarks</source>
        <translation>允許直接存取網頁瀏覽器的書籤</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="891"/>
        <source>Allow direct access to Web Browser&apos;s bookmark and history database</source>
        <translation>允許直接存取網頁瀏覽器的書籤和歷史瀏覽資料</translation>
    </message>
</context>
<context>
    <name>CBrowserPathsPage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="517"/>
        <source>Create Web Browser Template</source>
        <translation>建立網頁瀏覽器範本</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="518"/>
        <source>Configure your Web Browser&apos;s profile directories.</source>
        <oldsource>Configure your Web Browsers profile directories.</oldsource>
        <translation>設定網頁瀏覽器的使用者資料設定檔目錄組態。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="526"/>
        <source>User profile(s) directory:</source>
        <translation>使用者設定檔目錄:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="528"/>
        <source>Show also imperfect matches</source>
        <translation>同時也顯示非完美相符的結果</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="534"/>
        <source>Browser Executable (*.exe)</source>
        <translation>瀏覽器可執行檔案 (*.exe)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="551"/>
        <source>Continue without browser profile</source>
        <translation>在沒有瀏覽器設定檔的情況下繼續後續步驟</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="712"/>
        <source>Configure your Gecko based Browsers profile directories.</source>
        <translation>設定基於 Gecko 核心的瀏覽器設定檔目錄組態。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="716"/>
        <source>Configure your Chromium based Browsers profile directories.</source>
        <translation>設定基於 Chromium 核心的瀏覽器設定檔目錄組態。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="796"/>
        <source>No suitable folders have been found.
Note: you need to run the browser unsandboxed for them to get created.
Please browse to the correct user profile directory.</source>
        <oldsource>No suitable fodlers have been found.
Note: you need to run the browser unsandboxed for them to get created.
Please browse to the correct user profile directory.</oldsource>
        <translation>沒有發現合適的目錄。
注意: 您需要在不使用沙箱的情況下執行一次瀏覽器，以便目錄被正確建立。
請瀏覽並選擇正確的使用者設定檔目錄。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="801"/>
        <source>Please choose the correct user profile directory, if it is not listed you may need to browse to it.</source>
        <translation>請選擇正確的使用者設定檔目錄 (如未列出，您可能需要手動瀏覽並選取)。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="807"/>
        <source>Please ensure the selected directory is correct, the wizard is not confident in all the presented options.</source>
        <translation>請確保所選的目錄都是正確的，精靈無法幫您確認所選選項是否設定正確。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="810"/>
        <source>Please ensure the selected directory is correct.</source>
        <translation>請確保所選的目錄正確無誤。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="815"/>
        <source>This path does not look like a valid profile directory.</source>
        <translation>此路徑不是有效的設定檔目錄。</translation>
    </message>
</context>
<context>
    <name>CBrowserTypePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="325"/>
        <source>Create Web Browser Template</source>
        <translation>建立網頁瀏覽器範本</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="326"/>
        <source>Select your Web Browsers main executable, this will allow Sandboxie to identify the browser.</source>
        <oldsource>Select your Web Browsers main executable, this will allow sandboxie to identify the browser.</oldsource>
        <translation>選擇網頁瀏覽器的主程式，這將允許 Sandboxie 辨識瀏覽器。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="336"/>
        <source>Enter browser name:</source>
        <translation>輸入瀏覽器名稱:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="347"/>
        <source>Main executable (eg. firefox.exe, chrome.exe, msedge.exe, etc...):</source>
        <oldsource>Mein executable (eg. firefox.exe, chrome.exe, msedge.exe, etc...):</oldsource>
        <translation>主程式執行檔 (例如 firefox.exe, chrome.exe, msedge.exe, 等等...):</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="351"/>
        <source>Browser executable (*.exe)</source>
        <oldsource>Browser Executable (*.exe)</oldsource>
        <translation>瀏覽器可執行檔案 (*.exe)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="451"/>
        <source>The browser appears to be Gecko based, like Mozilla Firefox and its derivatives.</source>
        <translation>此瀏覽器似乎基於 Gecko 核心，例如 Mozilla Firefox 及其衍生發行版。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="454"/>
        <source>The browser appears to be Chromium based, like Microsoft Edge or Google Chrome and its derivatives.</source>
        <translation>此瀏覽器似乎基於 Chromium 核心，例如 Microsoft Edge 或 Google Chrome 及其衍生發行版。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="457"/>
        <source>Browser could not be recognized, template cannot be created.</source>
        <translation>不能辨識瀏覽器，無法建立範本。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="504"/>
        <source>This browser name is already in use, please choose an other one.</source>
        <oldsource>This browser name is already in use, please chooe an other one.</oldsource>
        <translation>此瀏覽器名稱已被使用，請選擇其他名稱。</translation>
    </message>
</context>
<context>
    <name>CCertificatePage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="195"/>
        <source>Install your &lt;b&gt;Sandboxie-Plus&lt;/b&gt; support certificate</source>
        <translation>安裝您的 &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 贊助者憑證</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="196"/>
        <source>If you have a supporter certificate, please fill it into the field below.</source>
        <translation>如果您有贊助者憑證，請填入以下欄位。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="218"/>
        <source>Start evaluation without a certificate for a limited period of time.</source>
        <translation>在沒有贊助者憑證的情況下開始進行有限時間的試用。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="246"/>
        <source>To use &lt;b&gt;Sandboxie-Plus&lt;/b&gt; in a business setting, an appropriate &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;support certificate&lt;/a&gt; for business use is required. If you do not yet have the required certificate(s), you can get those from the &lt;a href=&quot;https://xanasoft.com/shop/&quot;&gt;xanasoft.com web shop&lt;/a&gt;.</source>
        <translation>要在商業設定環境使用 &lt;b&gt;Sandboxie-Plus&lt;/b&gt;，需要適用於商業用途的&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;贊助者憑證&lt;/a&gt;。如果您還沒有所需的憑證，可以透過&lt;a href=&quot;https://xanasoft.com/shop/&quot;&gt;xanasoft.com 網路商店&lt;/a&gt;取得憑證。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="260"/>
        <source>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; provides additional features and box types exclusively to &lt;u&gt;project supporters&lt;/u&gt;. Boxes like the Privacy Enhanced boxes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;protect user data from illicit access&lt;/font&gt;&lt;/b&gt; by the sandboxed programs. If you are not yet a supporter, then please consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt; to ensure further development of Sandboxie and to receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;.</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; 僅為&lt;u&gt;專案贊助者&lt;/u&gt;提供額外的功能和沙箱類型。例如隱私增強型沙箱，可對來自沙箱化程式非法存取使用者資料的行為&lt;b&gt;&lt;font color=&apos;red&apos;&gt;提供額外的使用者資料保護&lt;/font&gt;&lt;/b&gt;。如果您還不是贊助者，請考慮 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;贊助支援此專案&lt;/a&gt;來確保 Sandboxie 的開發工作，並以此取得&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;贊助者憑證&lt;/a&gt;。</translation>
    </message>
</context>
<context>
    <name>CCleanUpJob</name>
    <message>
        <location filename="BoxJob.h" line="36"/>
        <source>Deleting Content</source>
        <translation>正在刪除內容</translation>
    </message>
</context>
<context>
    <name>CFileBrowserWindow</name>
    <message>
        <location filename="Views/FileView.cpp" line="398"/>
        <source>%1 - Files</source>
        <translation>%1 - 檔案</translation>
    </message>
</context>
<context>
    <name>CFileView</name>
    <message>
        <location filename="Views/FileView.cpp" line="188"/>
        <source>Create Shortcut</source>
        <translation>建立捷徑</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="210"/>
        <source>Pin to Box Run Menu</source>
        <translation>固定到「在沙箱中執行」選單</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="217"/>
        <source>Recover to Any Folder</source>
        <translation>復原到任意資料夾</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="219"/>
        <source>Recover to Same Folder</source>
        <translation>復原到相同資料夾</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="223"/>
        <source>Run Recovery Checks</source>
        <translation>執行復原檢查</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="287"/>
        <source>Select Directory</source>
        <translation>選擇目錄</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="353"/>
        <source>Create Shortcut to sandbox %1</source>
        <translation>為沙箱 %1 建立捷徑</translation>
    </message>
</context>
<context>
    <name>CFilesPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="302"/>
        <source>Sandbox location and behavior</source>
        <oldsource>Sandbox location and behavioure</oldsource>
        <translation>沙箱位置和行為</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="303"/>
        <source>On this page the sandbox location and its behavior can be customized.
You can use %USER% to save each users sandbox to an own folder.</source>
        <oldsource>On this page the sandbox location and its behaviorue can be customized.
You can use %USER% to save each users sandbox to an own fodler.</oldsource>
        <translation>在此頁面上，可以自訂沙箱位置及其行為。
您可以使用 %USER% 將每個使用者的沙箱儲存到各自的資料夾中。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="308"/>
        <source>Sandboxed Files</source>
        <translation>沙箱化檔案</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="331"/>
        <source>Select Directory</source>
        <translation>選擇目錄</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="339"/>
        <source>Virtualization scheme</source>
        <translation>虛擬化方案</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="343"/>
        <source>Version 1</source>
        <translation>版本 1</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="344"/>
        <source>Version 2</source>
        <translation>版本 2</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="350"/>
        <source>Separate user folders</source>
        <translation>分離使用者資料夾</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="355"/>
        <source>Use volume serial numbers for drives</source>
        <translation>使用磁碟的磁碟區序號</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="360"/>
        <source>Auto delete content when last process terminates</source>
        <translation>當所有處理程序結束後自動刪除全部內容</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="367"/>
        <source>Enable Immediate Recovery of files from recovery locations</source>
        <translation>啟用檔案立即復原 (從設定的復原位置)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="402"/>
        <source>The selected box location is not a valid path.</source>
        <oldsource>The sellected box location is not a valid path.</oldsource>
        <translation>選取的沙箱儲存位置是無效路徑。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="407"/>
        <source>The selected box location exists and is not empty, it is recommended to pick a new or empty folder. Are you sure you want to use an existing folder?</source>
        <oldsource>The sellected box location exists and is not empty, it is recomended to pick a new or empty folder. Are you sure you want to use an existing folder?</oldsource>
        <translation>選取的沙箱儲存位置已存在且不是空白目錄，推薦選擇新資料夾或是空白資料夾。確定要使用已存在的資料夾嗎？</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="412"/>
        <source>The selected box location is not placed on a currently available drive.</source>
        <oldsource>The selected box location not placed on a currently available drive.</oldsource>
        <translation>選取的沙箱儲存位置不在目前可用的磁碟上。</translation>
    </message>
</context>
<context>
    <name>CFinishPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="489"/>
        <source>Complete your configuration</source>
        <translation>完成您的組態</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="496"/>
        <source>Almost complete, click Finish to apply all selected options and conclude the wizard.</source>
        <translation>即將就緒，按下「完成」按鈕以套用所有選取的選項，並結束此精靈。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="508"/>
        <source>Keep Sandboxie-Plus up to date.</source>
        <translation>保持 Sandboxie-Plus 是最新版本。</translation>
    </message>
</context>
<context>
    <name>CFinishTemplatePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="936"/>
        <source>Create Web Browser Template</source>
        <translation>建立網頁瀏覽器範本</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="944"/>
        <source>Almost complete, click Finish to create a new  Web Browser Template and conclude the wizard.</source>
        <translation>即將就緒，按下「完成」按鈕以建立一個新的網頁瀏覽器範本，並結束此精靈。</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="962"/>
        <source>Browser name: %1
</source>
        <translation>瀏覽器名稱: %1
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="966"/>
        <source>Browser Type: Gecko (Mozilla Firefox)
</source>
        <oldsource>Browser Type: Gecko (Mozilla firefox)
</oldsource>
        <translation>瀏覽器類型: Gecko (Mozilla Firefox)
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="967"/>
        <source>Browser Type: Chromium (Google Chrome)
</source>
        <translation>瀏覽器類型: Chromium (Google Chrome)
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
        <translation>瀏覽器可執行檔路徑: %1
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="980"/>
        <source>Browser profile path: %1
</source>
        <translation>瀏覽器設定檔路徑: %1
</translation>
    </message>
</context>
<context>
    <name>CIntroPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="117"/>
        <source>Introduction</source>
        <translation>摘要資訊</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="121"/>
        <source>Welcome to the Setup Wizard. This wizard will help you to configure your copy of &lt;b&gt;Sandboxie-Plus&lt;/b&gt;. You can start this wizard at any time from the Sandbox-&gt;Maintenance menu if you do not wish to complete it now.</source>
        <translation>歡迎來到設定精靈，本精靈將幫助您設定此 &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 安裝之組態。如果您不希望現在完成，您可以從「沙箱 -&gt; 維護」選單中隨時重新啟動此精靈。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="130"/>
        <source>Select how you would like to use Sandboxie-Plus</source>
        <translation>選擇 Sandboxie-Plus 的使用方式</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="133"/>
        <source>&amp;Personally, for private non-commercial use</source>
        <translation>個人，用於私人非商業用途(&amp;P)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="138"/>
        <source>&amp;Commercially, for business or enterprise use</source>
        <translation>商業，用於企業或商業用途(&amp;C)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="143"/>
        <source>Note: this option is persistent</source>
        <translation>注意: 此選項無法在後續的設定中重新修改</translation>
    </message>
</context>
<context>
    <name>CMonitorModel</name>
    <message>
        <location filename="Models/MonitorModel.cpp" line="147"/>
        <source>Type</source>
        <translation>類型</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="148"/>
        <source>Status</source>
        <translation>狀態</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="149"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="150"/>
        <source>Count</source>
        <translation>總計</translation>
    </message>
</context>
<context>
    <name>CMultiErrorDialog</name>
    <message>
        <location filename="Dialogs/MultiErrorDialog.cpp" line="10"/>
        <source>Sandboxie-Plus - Error</source>
        <translation>Sandboxie-Plus - 錯誤</translation>
    </message>
    <message>
        <location filename="Dialogs/MultiErrorDialog.cpp" line="19"/>
        <source>Message</source>
        <translation>訊息</translation>
    </message>
</context>
<context>
    <name>CNewBoxWindow</name>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="23"/>
        <source>Sandboxie-Plus - Create New Box</source>
        <translation>Sandboxie-Plus - 建立新沙箱</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="39"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>具有資料保護功能的加固型沙箱</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="40"/>
        <source>Security Hardened Sandbox</source>
        <translation>安全性防護加固型沙箱</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="41"/>
        <source>Sandbox with Data Protection</source>
        <translation>資料保護型沙箱</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="42"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>標準隔離型沙箱 (預設)</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="44"/>
        <source>Application Compartment with Data Protection</source>
        <translation>資料保護型應用程式區間</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="45"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>應用程式區間 (無隔離防護)</translation>
    </message>
</context>
<context>
    <name>CNewBoxWizard</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="30"/>
        <source>New Box Wizard</source>
        <translation>新增沙箱精靈</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="138"/>
        <source>The new sandbox has been created using the new &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;Virtualization Scheme Version 2&lt;/a&gt;, if you experience any unexpected issues with this box, please switch to the Virtualization Scheme to Version 1 and report the issue, the option to change this preset can be found in the Box Options in the Box Structure group.</source>
        <oldsource>The new sandbox has been created using the new &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;Virtualization Scheme Version 2&lt;/a&gt;, if you expirience any unecpected issues with this box, please switch to the Virtualization Scheme to Version 1 and report the issue, the option to change this preset can be found in the Box Options in the Box Structure groupe.</oldsource>
        <translation>新沙箱按照新的 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;虛擬化方案 V2&lt;/a&gt; 建立，如果您在使用該沙箱的時候遇到任何問題，請嘗試切換至虛擬化方案 V1 並向我們反應問題，變更此預設的選項可以在「沙箱選項」中「檔案選項」的「沙箱結構」選項組內找到。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="141"/>
        <source>Don&apos;t show this message again.</source>
        <translation>不再顯示此訊息。</translation>
    </message>
</context>
<context>
    <name>COnDeleteJob</name>
    <message>
        <location filename="BoxJob.h" line="58"/>
        <source>OnDelete: %1</source>
        <translation>刪除階段: %1</translation>
    </message>
</context>
<context>
    <name>COnlineUpdater</name>
    <message>
        <location filename="OnlineUpdater.cpp" line="99"/>
        <source>Do you want to check if there is a new version of Sandboxie-Plus?</source>
        <translation>您想要檢查 Sandboxie-Plus 是否存在新版本嗎？</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="100"/>
        <source>Don&apos;t show this message again.</source>
        <translation>不再顯示此訊息。</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="209"/>
        <source>Checking for updates...</source>
        <translation>檢查更新中...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="248"/>
        <source>server not reachable</source>
        <translation>伺服器無法存取</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="249"/>
        <location filename="OnlineUpdater.cpp" line="251"/>
        <source>Failed to check for updates, error: %1</source>
        <translation>檢查更新失敗，錯誤: %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="404"/>
        <source>&lt;p&gt;Do you want to download the installer?&lt;/p&gt;</source>
        <translation>&lt;p&gt;是否下載此安裝程式？&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="406"/>
        <source>&lt;p&gt;Do you want to download the updates?&lt;/p&gt;</source>
        <translation>&lt;p&gt;是否下載此更新？&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="408"/>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;update page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation>&lt;p&gt;是否前往&lt;a href=&quot;%1&quot;&gt;更新頁面&lt;/a&gt;？&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="416"/>
        <source>Don&apos;t show this update anymore.</source>
        <translation>不再顯示此次更新。</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="534"/>
        <source>Downloading updates...</source>
        <translation>正在下載更新...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="560"/>
        <source>invalid parameter</source>
        <translation>無效參數</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="561"/>
        <source>failed to download updated information</source>
        <oldsource>failed to download update informations</oldsource>
        <translation>下載更新資訊失敗</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="562"/>
        <source>failed to load updated json file</source>
        <oldsource>failed to load update json file</oldsource>
        <translation>載入已更新的 Json 檔案失敗</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="563"/>
        <source>failed to download a particular file</source>
        <translation>下載特定檔案失敗</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="564"/>
        <source>failed to scan existing installation</source>
        <translation>掃描現有的安裝失敗</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="565"/>
        <source>updated signature is invalid !!!</source>
        <oldsource>update signature is invalid !!!</oldsource>
        <translation>無效的更新檔簽章！！！</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="566"/>
        <source>downloaded file is corrupted</source>
        <translation>下載的檔案已損毀</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="567"/>
        <source>internal error</source>
        <translation>內部錯誤</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="568"/>
        <source>unknown error</source>
        <translation>未知錯誤</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="592"/>
        <source>Failed to download updates from server, error %1</source>
        <translation>從伺服器下載更新失敗，錯誤 %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="611"/>
        <source>&lt;p&gt;Updates for Sandboxie-Plus have been downloaded.&lt;/p&gt;&lt;p&gt;Do you want to apply these updates? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Sandboxie-Plus 的更新已下載。&lt;/p&gt;&lt;p&gt;是否要套用這些更新？如果任何程式正在沙箱化執行，都將被終止。&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="715"/>
        <source>Downloading installer...</source>
        <translation>正在下載安裝程式...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="757"/>
        <source>Failed to download installer from: %1</source>
        <translation>下載安裝程式失敗，來源: %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="783"/>
        <source>&lt;p&gt;A new Sandboxie-Plus installer has been downloaded to the following location:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Do you want to begin the installation? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation>&lt;p&gt;一個新的 Sandboxie-Plus 安裝程式已被下載到以下位置:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;是否要開始安裝？如果任何程式正在沙箱化執行，都將被終止。&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="848"/>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;info page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation>&lt;p&gt;您是否想要前往 &lt;a href=&quot;%1&quot;&gt;資訊頁面&lt;/a&gt;？&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="856"/>
        <source>Don&apos;t show this announcement in the future.</source>
        <translation>此後不再顯示此公告。</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="398"/>
        <source>&lt;p&gt;There is a new version of Sandboxie-Plus available.&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;&lt;b&gt;New version:&lt;/b&gt;&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;有新版本 Sandboxie-Plus 可用，&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;新版本:&lt;/font&gt;&lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="274"/>
        <source>No new updates found, your Sandboxie-Plus is up-to-date.

Note: The update check is often behind the latest GitHub release to ensure that only tested updates are offered.</source>
        <translation>沒有發現更新，您的 Sandboxie-Plus 已為最新版本。

注意: 更新檢查通常落後於最新的 GitHub 版本，以確保僅提供經過測試的更新。</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="964"/>
        <source>Checking for certificate...</source>
        <translation>檢查憑證中...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="1004"/>
        <source>No certificate found on server!</source>
        <translation>未在伺服器中尋找到憑證！</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="1009"/>
        <source>There is no updated certificate available.</source>
        <translation>目前沒有可用的憑證更新。</translation>
    </message>
</context>
<context>
    <name>COptionsWindow</name>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="446"/>
        <location filename="Windows/OptionsWindow.cpp" line="460"/>
        <location filename="Windows/OptionsAccess.cpp" line="24"/>
        <source>Browse for File</source>
        <translation>瀏覽檔案</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="25"/>
        <source>Browse for Folder</source>
        <translation>瀏覽資料夾</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="312"/>
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="313"/>
        <source>Closed RT</source>
        <translation>封閉 RT</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="314"/>
        <source>Read Only</source>
        <translation>唯讀</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="308"/>
        <source>Normal</source>
        <translation>標準</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="309"/>
        <source>Open</source>
        <translation>開放</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="310"/>
        <source>Open for All</source>
        <translation>完全開放</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="311"/>
        <source>No Rename</source>
        <translation>沒有重新命名</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="315"/>
        <source>Box Only (Write Only)</source>
        <translation>僅沙箱內 (唯寫)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="316"/>
        <source>Ignore UIPI</source>
        <translation>忽略 UIPI</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="318"/>
        <location filename="Windows/OptionsAccess.cpp" line="335"/>
        <location filename="Windows/OptionsAccess.cpp" line="348"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="325"/>
        <source>Regular Sandboxie behavior - allow read and also copy on write.</source>
        <translation>常規 Sandboxie 行為 - 允許讀取及寫入時複製。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="326"/>
        <source>Allow write-access outside the sandbox.</source>
        <translation>允許沙箱外的寫入。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="327"/>
        <source>Allow write-access outside the sandbox, also for applications installed inside the sandbox.</source>
        <translation>允許沙箱外的寫入 (同時適用於安裝在沙箱內的應用程式)。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="328"/>
        <source>Don&apos;t rename window classes.</source>
        <translation>不重新命名視窗類別。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="329"/>
        <source>Deny access to host location and prevent creation of sandboxed copies.</source>
        <translation>拒絕對主機位置的存取，防止在沙箱內建立相應的複本。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="330"/>
        <source>Block access to WinRT class.</source>
        <translation>阻止對 WinRT 類別的存取。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="331"/>
        <source>Allow read-only access only.</source>
        <translation>只允許唯讀存取。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="332"/>
        <source>Hide host files, folders or registry keys from sandboxed processes.</source>
        <translation>對沙箱內的處理程序隱藏主機檔案、資料夾或登錄機碼。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="333"/>
        <source>Ignore UIPI restrictions for processes.</source>
        <translation>忽略對處理程序的 UIPI 限制。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="342"/>
        <source>File/Folder</source>
        <translation>檔案和資料夾</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="343"/>
        <source>Registry</source>
        <translation>登錄</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="344"/>
        <source>IPC Path</source>
        <translation>IPC 路徑</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="345"/>
        <source>Wnd Class</source>
        <translation>Wnd 元件</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="346"/>
        <source>COM Object</source>
        <translation>COM 物件</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="353"/>
        <source>Select File</source>
        <translation>選擇檔案</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="353"/>
        <source>All Files (*.*)</source>
        <translation>所有檔案 (*.*)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="365"/>
        <location filename="Windows/OptionsForce.cpp" line="242"/>
        <location filename="Windows/OptionsForce.cpp" line="252"/>
        <location filename="Windows/OptionsRecovery.cpp" line="128"/>
        <location filename="Windows/OptionsRecovery.cpp" line="139"/>
        <source>Select Directory</source>
        <translation>選擇目錄</translation>
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
        <translation>所有程式</translation>
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
        <translation>群組: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="537"/>
        <source>COM objects must be specified by their GUID, like: {00000000-0000-0000-0000-000000000000}</source>
        <translation>COM 物件必須被它們的 GUID 所指定，例如: {00000000-0000-0000-0000-000000000000}</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="543"/>
        <source>RT interfaces must be specified by their name.</source>
        <translation>RT 介面必須用其名稱來指定。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="557"/>
        <source>Opening all IPC access also opens COM access, do you still want to restrict COM to the sandbox?</source>
        <translation>開放 IPC 存取權限的同時也將開放 COM 的存取權限，您是否想繼續在沙箱內限制 COM 介面的存取權限？</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="558"/>
        <source>Don&apos;t ask in future</source>
        <translation>此後不再詢問</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="567"/>
        <source>&apos;OpenWinClass=program.exe,#&apos; is not supported, use &apos;NoRenameWinClass=program.exe,*&apos; instead</source>
        <translation>不支援 &apos;OpenWinClass=program.exe,#&apos;，請改為使用 &apos;NoRenameWinClass=program.exe,*&apos;</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="611"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="605"/>
        <location filename="Windows/OptionsGeneral.cpp" line="579"/>
        <location filename="Windows/OptionsGrouping.cpp" line="234"/>
        <location filename="Windows/OptionsGrouping.cpp" line="258"/>
        <location filename="Windows/OptionsNetwork.cpp" line="533"/>
        <source>Template values can not be edited.</source>
        <translation>範本值無法編輯。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="679"/>
        <source>Template values can not be removed.</source>
        <translation>範本值無法刪除。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="41"/>
        <source>Enable the use of win32 hooks for selected processes. Note: You need to enable win32k syscall hook support globally first.</source>
        <translation>對選取的處理程序啟用 Win32 勾點 (注意: 需要先啟用全域範圍的 Win32k Syscall 勾點支援)。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="42"/>
        <source>Enable crash dump creation in the sandbox folder</source>
        <translation>啟用沙箱資料夾內損毀傾印檔案之建立</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="43"/>
        <source>Always use ElevateCreateProcess fix, as sometimes applied by the Program Compatibility Assistant.</source>
        <translation>始終使用 ElevateCreateProcess 修復，因偶爾會被程式相容性疑難排解員套用。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="45"/>
        <source>Enable special inconsistent PreferExternalManifest behaviour, as needed for some Edge fixes</source>
        <oldsource>Enable special inconsistent PreferExternalManifest behavioure, as neede for some edge fixes</oldsource>
        <translation>啟用特殊的差異化 PreferExternalManifest 行為 (修復 Microsoft Edge 存在的某些問題需要開啟此選項)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="46"/>
        <source>Set RpcMgmtSetComTimeout usage for specific processes</source>
        <translation>為特定處理程序設定 RpcMgmtSetComTimeout 選項</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="47"/>
        <source>Makes a write open call to a file that won&apos;t be copied fail instead of turning it read-only.</source>
        <translatorcomment>Hard to understand, But anyway there&apos;s &quot;Makes | a write open call | to a file that won&apos;t be copied | fail&quot;</translatorcomment>
        <translation>使對一個「沒有被複製的」檔案進行的｢寫入控制代碼」呼叫失敗，而不是將檔案本身變成唯讀。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="50"/>
        <source>Make specified processes think they have admin permissions.</source>
        <oldsource>Make specified processes think thay have admin permissions.</oldsource>
        <translation>讓指定的處理程序認為它們具有管理員權限。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="51"/>
        <source>Force specified processes to wait for a debugger to attach.</source>
        <translation>強制要求被指定的處理程序等待偵錯工具的附加。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="53"/>
        <source>Sandbox file system root</source>
        <translation>沙箱檔案系統根目錄</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="54"/>
        <source>Sandbox registry root</source>
        <translation>沙箱登錄根目錄</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="55"/>
        <source>Sandbox ipc root</source>
        <translation>沙箱 IPC 根目錄</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="575"/>
        <source>Add special option:</source>
        <translation>加入特殊選項:</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="726"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="730"/>
        <source>On Start</source>
        <translation>啟動階段</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="727"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="735"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="739"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="743"/>
        <source>Run Command</source>
        <translation>執行命令</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="731"/>
        <source>Start Service</source>
        <translation>啟動服務</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="734"/>
        <source>On Init</source>
        <translation>初始化階段</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="738"/>
        <source>On File Recovery</source>
        <translation>檔案復原階段</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="742"/>
        <source>On Delete Content</source>
        <translation>內容刪除階段</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="753"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="775"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="786"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="797"/>
        <source>Please enter the command line to be executed</source>
        <translation>請輸入將要執行的命令列</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="835"/>
        <source>Please enter a program file name</source>
        <translation>請輸入一個程式檔案名稱</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="917"/>
        <source>Deny</source>
        <translation>拒絕 (停用)</translation>
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
        <translation>處理程序</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="111"/>
        <location filename="Windows/OptionsForce.cpp" line="122"/>
        <source>Folder</source>
        <translation>資料夾</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="222"/>
        <location filename="Windows/OptionsForce.cpp" line="232"/>
        <source>Select Executable File</source>
        <translation>選擇可執行檔</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="222"/>
        <location filename="Windows/OptionsForce.cpp" line="232"/>
        <source>Executable Files (*.exe)</source>
        <translation>可執行檔 (*.exe)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="19"/>
        <source>This option requires a valid supporter certificate</source>
        <translation>此選項需要一份有效的贊助者憑證</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="22"/>
        <source>Supporter exclusive option</source>
        <translation>贊助者專屬選項</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="47"/>
        <source>Don&apos;t alter the window title</source>
        <translation>不改變視窗標題</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="48"/>
        <source>Display [#] indicator only</source>
        <translation>只顯示 [#] 標記</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="49"/>
        <source>Display box name in title</source>
        <translation>標題內顯示沙箱名稱</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="51"/>
        <source>Border disabled</source>
        <translation>停用邊框</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="52"/>
        <source>Show only when title is in focus</source>
        <translation>僅在標題處在焦點時顯示</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="53"/>
        <source>Always show</source>
        <translation>總是顯示</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="56"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>具有資料保護功能的加固型沙箱</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="57"/>
        <source>Security Hardened Sandbox</source>
        <translation>安全性防護加固型沙箱</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="58"/>
        <source>Sandbox with Data Protection</source>
        <translation>資料保護型沙箱</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="59"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>標準隔離型沙箱 (預設)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="61"/>
        <source>Application Compartment with Data Protection</source>
        <translation>資料保護型應用程式區間</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="62"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>應用程式區間 (無隔離防護)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="109"/>
        <source>Custom icon</source>
        <translation>自訂圖示</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="161"/>
        <source>Version 1</source>
        <translation>版本 1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="162"/>
        <source>Version 2</source>
        <translation>版本 2</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="198"/>
        <source>Browse for Program</source>
        <translation>瀏覽程式</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="258"/>
        <source>Open Box Options</source>
        <translation>開啟沙箱選項</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="259"/>
        <source>Browse Content</source>
        <translation>瀏覽內容</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="260"/>
        <source>Start File Recovery</source>
        <translation>開始復原檔案</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="261"/>
        <source>Show Run Dialog</source>
        <translation>顯示執行對話方塊</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="294"/>
        <source>Indeterminate</source>
        <translation>不確定</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="484"/>
        <location filename="Windows/OptionsGeneral.cpp" line="584"/>
        <source>Always copy</source>
        <translation>永遠複製</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="485"/>
        <location filename="Windows/OptionsGeneral.cpp" line="585"/>
        <source>Don&apos;t copy</source>
        <translation>不要複製</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="486"/>
        <location filename="Windows/OptionsGeneral.cpp" line="586"/>
        <source>Copy empty</source>
        <translation>複製空內容</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="712"/>
        <source>kilobytes (%1)</source>
        <translation>KB (%1)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="818"/>
        <source>Select color</source>
        <translation>選擇顏色</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="893"/>
        <source>Select Program</source>
        <translation>選擇程式</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="764"/>
        <source>Please enter a service identifier</source>
        <translation>請輸入服務識別字元</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="893"/>
        <source>Executables (*.exe *.cmd)</source>
        <translation>可執行檔案 (*.exe *.cmd)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="897"/>
        <location filename="Windows/OptionsGeneral.cpp" line="913"/>
        <source>Please enter a menu title</source>
        <translation>請輸入一個選單標題</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="909"/>
        <source>Please enter a command</source>
        <translation>請輸入一則命令</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGrouping.cpp" line="141"/>
        <source>Please enter a name for the new group</source>
        <translation>請輸入新群組的名稱</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGrouping.cpp" line="229"/>
        <source>Please select group first.</source>
        <translation>請先選取群組。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="37"/>
        <location filename="Windows/OptionsNetwork.cpp" line="588"/>
        <source>Any</source>
        <translation>任何</translation>
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
        <translation>允許存取</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="53"/>
        <source>Block using Windows Filtering Platform</source>
        <translation>阻止存取 - 使用 Windows 篩選平台 (WFP)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="54"/>
        <source>Block by denying access to Network devices</source>
        <translation>阻止存取 - 透過拒絕對區域網路裝置的存取</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="917"/>
        <location filename="Windows/OptionsNetwork.cpp" line="171"/>
        <location filename="Windows/OptionsNetwork.cpp" line="574"/>
        <source>Allow</source>
        <translation>允許</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="172"/>
        <source>Block (WFP)</source>
        <translation>阻止 (WFP)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="173"/>
        <source>Block (NDev)</source>
        <translation>阻止 (區域網路裝置)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="278"/>
        <source>A non empty program name is required.</source>
        <translation>程式名稱不得為空。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="575"/>
        <source>Block</source>
        <translation>阻止</translation>
    </message>
    <message>
        <location filename="Windows/OptionsRecovery.cpp" line="150"/>
        <source>Please enter a file extension to be excluded</source>
        <translation>請輸入要排除的副檔名</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="56"/>
        <source>All Categories</source>
        <translation>所有類別</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="95"/>
        <source>Custom Templates</source>
        <translation>自訂範本</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="96"/>
        <source>Email Reader</source>
        <translation>電子郵件閱讀器</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="97"/>
        <source>PDF/Print</source>
        <translation>PDF/列印</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="98"/>
        <source>Security/Privacy</source>
        <translation>安全性/隱私</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="99"/>
        <source>Desktop Utilities</source>
        <translation>桌面實用工具</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="100"/>
        <source>Download Managers</source>
        <translation>下載管理員</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="101"/>
        <source>Miscellaneous</source>
        <translation>雜項</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="102"/>
        <source>Web Browser</source>
        <translation>網頁瀏覽器</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="103"/>
        <source>Media Player</source>
        <translation>多媒體播放器</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="104"/>
        <source>Torrent Client</source>
        <translation>BT種子 (Torrent) 用戶端</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="151"/>
        <source>This template is enabled globally. To configure it, use the global options.</source>
        <translation>此範本已全域啟用。如需設定組態，請前往全域選項。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="187"/>
        <source>Please enter the template identifier</source>
        <translation>請輸入範本識別碼</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="195"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="222"/>
        <source>Do you really want to delete the selected local template(s)?</source>
        <translation>是否刪除選取的本地範本？</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="228"/>
        <source>Only local templates can be removed!</source>
        <translation>僅可刪除本地範本！</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="181"/>
        <source>Sandboxie Plus - &apos;%1&apos; Options</source>
        <translation>Sandboxie Plus - &apos;%1&apos; 選項</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="287"/>
        <source>File Options</source>
        <translation>檔案選項</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="319"/>
        <source>Grouping</source>
        <translation>分組</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="525"/>
        <source>Add %1 Template</source>
        <translation>加入 %1 範本</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="625"/>
        <source>Search for options</source>
        <translation>搜尋選項</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="755"/>
        <source>Box: %1</source>
        <translation>沙箱: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="756"/>
        <source>Template: %1</source>
        <translation>範本: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="757"/>
        <source>Global: %1</source>
        <translation>全域: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="758"/>
        <source>Default: %1</source>
        <translation>預設: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="935"/>
        <source>This sandbox has been deleted hence configuration can not be saved.</source>
        <translation>此沙箱已被刪除，因此組態無法儲存。</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="982"/>
        <source>Some changes haven&apos;t been saved yet, do you really want to close this options window?</source>
        <translation>部分變更未儲存，確定關閉這個選項視窗嗎？</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="1005"/>
        <source>Enter program:</source>
        <translation>請輸入程式:</translation>
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
        <translation>造訪 %1 以取得詳細說明。</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="57"/>
        <source>Dismiss</source>
        <translation>關閉</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="58"/>
        <source>Remove this message from the list</source>
        <translation>從清單中刪除此訊息</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="61"/>
        <source>Hide all such messages</source>
        <translation>隱藏所有類似訊息</translation>
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
        <translation>關閉</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="367"/>
        <source>Remove this progress indicator from the list</source>
        <translation>在清單中刪除此處理程序標記</translation>
    </message>
</context>
<context>
    <name>CPopUpPrompt</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="108"/>
        <source>Remember for this process</source>
        <translation>記住對此處理程序的選擇</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="118"/>
        <source>Yes</source>
        <translation>是</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="123"/>
        <source>No</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="128"/>
        <source>Terminate</source>
        <translation>終止</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="149"/>
        <source>Yes and add to allowed programs</source>
        <translation>確定並新增到允許的程式中</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="184"/>
        <source>Requesting process terminated</source>
        <translation>請求的處理程序已終止</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="189"/>
        <source>Request will time out in %1 sec</source>
        <translation>請求將在 %1 秒後逾時</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="191"/>
        <source>Request timed out</source>
        <translation>請求逾時</translation>
    </message>
</context>
<context>
    <name>CPopUpRecovery</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="245"/>
        <source>Recover to:</source>
        <translation>復原至:</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="250"/>
        <source>Browse</source>
        <translation>瀏覽</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="251"/>
        <source>Clear folder list</source>
        <translation>清除資料夾清單</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="259"/>
        <source>Recover</source>
        <translation>復原</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="260"/>
        <source>Recover the file to original location</source>
        <translation>復原檔案到原始路徑</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="263"/>
        <source>Recover &amp;&amp; Explore</source>
        <translation>復原 &amp;&amp; 瀏覽</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="264"/>
        <source>Recover &amp;&amp; Open/Run</source>
        <translation>復原 &amp;&amp; 開啟/執行</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="266"/>
        <source>Open file recovery for this box</source>
        <translation>為此沙箱開啟檔案復原</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="274"/>
        <source>Dismiss</source>
        <translation>關閉</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="275"/>
        <source>Don&apos;t recover this file right now</source>
        <translation>目前暫不復原此檔案</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="278"/>
        <source>Dismiss all from this box</source>
        <translation>對此沙箱全部忽略</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="279"/>
        <source>Disable quick recovery until the box restarts</source>
        <translation>在沙箱重新啟動前停用快速復原</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="303"/>
        <source>Select Directory</source>
        <translation>選擇目錄</translation>
    </message>
</context>
<context>
    <name>CPopUpWindow</name>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="25"/>
        <source>Sandboxie-Plus Notifications</source>
        <translation>Sandboxie-Plus 通知</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="171"/>
        <source>Do you want to allow the print spooler to write outside the sandbox for %1 (%2)?</source>
        <translation>您確定允許 %1 (%2) 使用列印服務在沙箱外寫入嗎？</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="279"/>
        <source>Do you want to allow %4 (%5) to copy a %1 large file into sandbox: %2?
File name: %3</source>
        <translation>您確定允許 %4 (%5) 複製大型檔案 %1 至沙箱: %2？
檔案名稱: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="285"/>
        <source>Do you want to allow %1 (%2) access to the internet?
Full path: %3</source>
        <translation>您確定允許 %1 (%2) 存取網路嗎？
完整路徑: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="344"/>
        <source>%1 is eligible for quick recovery from %2.
The file was written by: %3</source>
        <translation>%1 可以從 %2 快速復原。
檔案寫入自: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="346"/>
        <source>an UNKNOWN process.</source>
        <translation>未知處理程序。</translation>
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
        <translation>未知</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="437"/>
        <source>Migrating a large file %1 into the sandbox %2, %3 left.
Full path: %4</source>
        <translation>遷移一個大型檔案 %1 到沙箱 %2，剩餘 %3。
完整路徑: %4</translation>
    </message>
</context>
<context>
    <name>CRecoveryLogWnd</name>
    <message>
        <location filename="SandManRecovery.cpp" line="306"/>
        <source>Sandboxie-Plus - Recovery Log</source>
        <translation>Sandboxie-Plus - 復原日誌</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="317"/>
        <source>Time|Box Name|File Path</source>
        <translation>時間|沙箱名稱|檔案路徑</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="319"/>
        <source>Cleanup Recovery Log</source>
        <translation>清理復原日誌</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="329"/>
        <source>The following files were recently recovered and moved out of a sandbox.</source>
        <oldsource>the following files were recently recovered and moved out of a sandbox.</oldsource>
        <translation>以下是最近被復原並移出沙箱的檔案。</translation>
    </message>
</context>
<context>
    <name>CRecoveryWindow</name>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="46"/>
        <source>%1 - File Recovery</source>
        <translation>%1 - 檔案復原</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="70"/>
        <source>File Name</source>
        <translation>檔案名稱</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="71"/>
        <source>File Size</source>
        <translation>檔案大小</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="72"/>
        <source>Full Path</source>
        <translation>完整路徑</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="106"/>
        <source>Remember target selection</source>
        <translation>記住對此目的的選擇</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="112"/>
        <source>Delete everything, including all snapshots</source>
        <translation>刪除所有內容，包括所有快照</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="138"/>
        <source>Original location</source>
        <translation>原始位置</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="139"/>
        <source>Browse for location</source>
        <translation>瀏覽位置</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="140"/>
        <source>Clear folder list</source>
        <translation>清除資料夾清單</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="180"/>
        <location filename="Windows/RecoveryWindow.cpp" line="201"/>
        <location filename="Windows/RecoveryWindow.cpp" line="552"/>
        <source>Select Directory</source>
        <translation>選擇目錄</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="248"/>
        <source>Do you really want to delete %1 selected files?</source>
        <translation>是否刪除 %1 選取的檔案？</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="276"/>
        <source>Close until all programs stop in this box</source>
        <translation>關閉，在沙箱內全部程式停止後再顯示</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="277"/>
        <source>Close and Disable Immediate Recovery for this box</source>
        <translation>關閉並停用此沙箱的快速復原</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="297"/>
        <source>There are %1 new files available to recover.</source>
        <translation>有 %1 個新檔案可供復原。</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="588"/>
        <source>There are %1 files and %2 folders in the sandbox, occupying %3 of disk space.</source>
        <translation>此沙箱中共有 %1 個檔案和 %2 個資料夾，占用了 %3 磁碟空間。</translation>
    </message>
</context>
<context>
    <name>CSandBox</name>
    <message>
        <location filename="SandMan.cpp" line="3614"/>
        <source>Waiting for folder: %1</source>
        <translation>正在等待資料夾: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3615"/>
        <source>Deleting folder: %1</source>
        <translation>正在刪除資料夾: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3616"/>
        <source>Merging folders: %1 &amp;gt;&amp;gt; %2</source>
        <translation>正在合併資料夾: %1 &amp;gt;&amp;gt; %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3617"/>
        <source>Finishing Snapshot Merge...</source>
        <translation>正在完成快照合併...</translation>
    </message>
</context>
<context>
    <name>CSandBoxPlus</name>
    <message>
        <location filename="SbiePlusAPI.cpp" line="652"/>
        <source>Disabled</source>
        <translation>停用</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="663"/>
        <source>OPEN Root Access</source>
        <translation>開放 Root 存取權限</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="665"/>
        <source>Application Compartment</source>
        <translation>應用程式區間</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="667"/>
        <source>NOT SECURE</source>
        <translation>不安全</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="669"/>
        <source>Reduced Isolation</source>
        <translation>弱化隔離</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="671"/>
        <source>Enhanced Isolation</source>
        <translation>增強隔離</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="674"/>
        <source>Privacy Enhanced</source>
        <translation>隱私增強</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="677"/>
        <source>API Log</source>
        <translation>API 日誌</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="680"/>
        <source>No INet (with Exceptions)</source>
        <translation>無網際網路 (允許例外)</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="682"/>
        <source>No INet</source>
        <translation>無網際網路</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="685"/>
        <source>Net Share</source>
        <translation>區域網路共享</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="687"/>
        <source>No Admin</source>
        <translation>無管理員</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="690"/>
        <source>Auto Delete</source>
        <translation>自動刪除</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="693"/>
        <source>Normal</source>
        <translation>標準</translation>
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
        <translation>重設欄</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3504"/>
        <source>Copy Cell</source>
        <translation>複製單元格</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3505"/>
        <source>Copy Row</source>
        <translation>複製列</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3506"/>
        <source>Copy Panel</source>
        <translation>複製表格</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1185"/>
        <source>Time|Message</source>
        <translation>時間|訊息</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1194"/>
        <source>Sbie Messages</source>
        <translation>SBIE 訊息</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1201"/>
        <source>Trace Log</source>
        <translation>追蹤日誌</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="45"/>
        <source>Show/Hide</source>
        <translation>顯示/隱藏</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="566"/>
        <location filename="SandMan.cpp" line="568"/>
        <location filename="SandMan.cpp" line="616"/>
        <location filename="SandMan.cpp" line="618"/>
        <source>Pause Forcing Programs</source>
        <translation>暫停強制沙箱程式</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="468"/>
        <location filename="SandMan.cpp" line="684"/>
        <source>&amp;Sandbox</source>
        <translation>沙箱(&amp;S)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="469"/>
        <location filename="SandMan.cpp" line="942"/>
        <location filename="SandMan.cpp" line="943"/>
        <source>Create New Box</source>
        <translation>建立新沙箱</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="470"/>
        <source>Create Box Group</source>
        <translation>建立沙箱群組</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="474"/>
        <location filename="SandMan.cpp" line="615"/>
        <source>Terminate All Processes</source>
        <translation>終止所有處理程序</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="572"/>
        <source>Disable File Recovery</source>
        <translation>停用檔案復原</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="410"/>
        <source>&amp;Maintenance</source>
        <translation>維護(&amp;M)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="411"/>
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="412"/>
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="414"/>
        <source>Stop All</source>
        <translation>全部停止</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="415"/>
        <source>&amp;Advanced</source>
        <translation>進階(&amp;A)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="416"/>
        <source>Install Driver</source>
        <translation>安裝驅動程式</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="417"/>
        <source>Start Driver</source>
        <translation>啟動驅動程式</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="418"/>
        <source>Stop Driver</source>
        <translation>停止驅動程式</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="419"/>
        <source>Uninstall Driver</source>
        <translation>解除安裝驅動程式</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="421"/>
        <source>Install Service</source>
        <translation>安裝服務</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="422"/>
        <source>Start Service</source>
        <translation>啟動服務</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="423"/>
        <source>Stop Service</source>
        <translation>停止服務</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="424"/>
        <source>Uninstall Service</source>
        <translation>解除安裝服務</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="426"/>
        <source>Setup Wizard</source>
        <translation>設定精靈</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="429"/>
        <source>Uninstall All</source>
        <translation>全部解除安裝</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="500"/>
        <location filename="SandMan.cpp" line="654"/>
        <source>Exit</source>
        <translation>退出</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="503"/>
        <location filename="SandMan.cpp" line="656"/>
        <source>&amp;View</source>
        <translation>檢視(&amp;V)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="435"/>
        <source>Simple View</source>
        <translation>簡易檢視</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="436"/>
        <source>Advanced View</source>
        <translation>進階檢視</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="441"/>
        <source>Always on Top</source>
        <translation>保持視窗置頂</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="510"/>
        <source>Show Hidden Boxes</source>
        <translation>顯示隱藏沙箱</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="512"/>
        <source>Show All Sessions</source>
        <translation>顯示所有工作階段</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="529"/>
        <source>Refresh View</source>
        <translation>重新整理檢視</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="535"/>
        <source>Clean Up</source>
        <translation>清理</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="536"/>
        <source>Cleanup Processes</source>
        <translation>清理處理程序</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="538"/>
        <source>Cleanup Message Log</source>
        <translation>清理訊息日誌</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="539"/>
        <source>Cleanup Trace Log</source>
        <translation>清理追蹤日誌</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="540"/>
        <source>Cleanup Recovery Log</source>
        <translation>清理復原日誌</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="542"/>
        <source>Keep terminated</source>
        <translation>保持終止</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="562"/>
        <source>&amp;Options</source>
        <translation>選項(&amp;O)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="563"/>
        <location filename="SandMan.cpp" line="705"/>
        <source>Global Settings</source>
        <translation>全域設定</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="597"/>
        <location filename="SandMan.cpp" line="715"/>
        <source>Reset all hidden messages</source>
        <translation>重設所有已隱藏訊息</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="598"/>
        <location filename="SandMan.cpp" line="716"/>
        <source>Reset all GUI options</source>
        <translation>重設所有 GUI 設定選項</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="555"/>
        <source>Trace Logging</source>
        <translation>啟用追蹤日誌</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="447"/>
        <source>&amp;Help</source>
        <translation>說明(&amp;H)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="456"/>
        <source>Visit Support Forum</source>
        <translation>造訪支援論壇</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="455"/>
        <source>Online Documentation</source>
        <translation>線上文件</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="458"/>
        <source>Check for Updates</source>
        <translation>檢查更新</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="460"/>
        <source>About the Qt Framework</source>
        <translation>關於 Qt 框架</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="461"/>
        <location filename="SandMan.cpp" line="3588"/>
        <source>About Sandboxie-Plus</source>
        <translation>關於 Sandboxie-Plus</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="689"/>
        <source>Create New Sandbox</source>
        <translation>建立新沙箱</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="690"/>
        <source>Create New Group</source>
        <translation>建立新群組</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="792"/>
        <location filename="SandMan.cpp" line="930"/>
        <location filename="SandMan.cpp" line="931"/>
        <source>Cleanup</source>
        <translation>清理</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1014"/>
        <source>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus release %1 ready&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;有一個新的 Sandboxie-Plus 版本 %1 準備就緒&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1018"/>
        <source>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update %1 ready&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;有一個 Sandboxie-Plus 更新 v%1 準備就緒&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1022"/>
        <source>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update v%1 available&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;有一個 Sandboxie-Plus 更新 v%1 可用&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1031"/>
        <source>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Support Sandboxie-Plus on Patreon&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;在 Patreon 上支援 Sandboxie-Plus&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1032"/>
        <source>Click to open web browser</source>
        <translation>按一下以開啟網頁瀏覽器</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1211"/>
        <source>Time|Box Name|File Path</source>
        <translation>時間|沙箱名稱|檔案路徑</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="559"/>
        <location filename="SandMan.cpp" line="667"/>
        <location filename="SandMan.cpp" line="1221"/>
        <source>Recovery Log</source>
        <translation>復原日誌</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1015"/>
        <source>Click to run installer</source>
        <translation>按一下以執行安裝程式</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1019"/>
        <source>Click to apply update</source>
        <translation>按一下以套用更新</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1265"/>
        <source>Do you want to close Sandboxie Manager?</source>
        <translation>您確定要關閉 Sandboxie 管理員？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1282"/>
        <source>Sandboxie-Plus was running in portable mode, now it has to clean up the created services. This will prompt for administrative privileges.

Do you want to do the clean up?</source>
        <translation>Sandboxie-Plus 正執行於便攜模式，現在將清理所建立的服務。這將需要管理員權限。

是否確認清理？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1283"/>
        <location filename="SandMan.cpp" line="1696"/>
        <location filename="SandMan.cpp" line="2027"/>
        <location filename="SandMan.cpp" line="2635"/>
        <location filename="SandMan.cpp" line="3063"/>
        <location filename="SandMan.cpp" line="3079"/>
        <source>Don&apos;t show this message again.</source>
        <translation>不再顯示此訊息。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1452"/>
        <source>This box provides &lt;a href=&quot;sbie://docs/security-mode&quot;&gt;enhanced security isolation&lt;/a&gt;, it is suitable to test untrusted software.</source>
        <oldsource>This box provides enhanced security isolation, it is suitable to test untrusted software.</oldsource>
        <translation>此類沙箱提供了&lt;a href=&quot;sbie://docs/security-mode&quot;&gt;強化安全性隔離&lt;/a&gt;，它適用於測試不受信任的軟體。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1456"/>
        <source>This box provides standard isolation, it is suitable to run your software to enhance security.</source>
        <translation>此類沙箱提供了標準的隔離，它適用於以增強的安全性來執行您的軟體。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1460"/>
        <source>This box does not enforce isolation, it is intended to be used as an &lt;a href=&quot;sbie://docs/compartment-mode&quot;&gt;application compartment&lt;/a&gt; for software virtualization only.</source>
        <oldsource>This box does not enforce isolation, it is intended to be used as an application compartment for software virtualization only.</oldsource>
        <translation>此類沙箱不執行隔離，它只是用來作為軟體虛擬化的&lt;a href=&quot;sbie://docs/compartment-mode&quot;&gt;應用程式區間&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1465"/>
        <source>&lt;br /&gt;&lt;br /&gt;This box &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;prevents access to all user data&lt;/a&gt; locations, except explicitly granted in the Resource Access options.</source>
        <oldsource>

This box &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;prevents access to all user data&lt;/a&gt; locations, except explicitly granted in the Resource Access options.</oldsource>
        <translation>&lt;br /&gt;&lt;br /&gt;此類沙箱會&lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;阻止存取所有使用者資料&lt;/a&gt;的位置，除非在資源存取選項中明確授權。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1553"/>
        <source>Unknown operation &apos;%1&apos; requested via command line</source>
        <translation>透過命令列請求的未知操作 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="105"/>
        <source>Dismiss Update Notification</source>
        <translation>忽略更新通知</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="181"/>
        <source> - Driver/Service NOT Running!</source>
        <translation> - 驅動程式/服務尚未執行！</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="183"/>
        <source> - Deleting Sandbox Content</source>
        <translation> - 刪除沙箱內容</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1763"/>
        <source>Executing OnBoxDelete: %1</source>
        <translation>在刪除沙箱時執行: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1784"/>
        <source>Auto Deleting %1 Content</source>
        <translation>自動刪除 %1 的內容</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1969"/>
        <source>Auto deleting content of %1</source>
        <translation>自動刪除 %1 的內容</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>%1 Directory: %2</source>
        <translation>%1 目錄: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>Application</source>
        <translation>應用程式</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>Installation</source>
        <translation>安裝</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2005"/>
        <source>Current Config: %1</source>
        <translation>目前組態: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2771"/>
        <location filename="SandMan.cpp" line="2773"/>
        <location filename="SandMan.cpp" line="3322"/>
        <source>Sandboxie-Plus - Error</source>
        <translation>Sandboxie-Plus - 錯誤</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2771"/>
        <source>Failed to stop all Sandboxie components</source>
        <translation>停止所有 Sandboxie 元件失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2773"/>
        <source>Failed to start required Sandboxie components</source>
        <translation>啟動所需 Sandboxie 元件失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="165"/>
        <source>WARNING: Sandboxie-Plus.ini in %1 cannot be written to, settings will not be saved.</source>
        <translation>警告: 無法寫入 %1 中的 Sandboxie-Plus.ini，設定將不會被儲存。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1694"/>
        <source>Some compatibility templates (%1) are missing, probably deleted, do you want to remove them from all boxes?</source>
        <translation>部分相容性範本 (%1) 已遺失，可能是已被刪除，是否將其從所有沙箱中移除？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1711"/>
        <source>Cleaned up removed templates...</source>
        <translation>清理已刪除的範本...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2024"/>
        <source>Sandboxie-Plus was started in portable mode, do you want to put the Sandbox folder into its parent directory?
Yes will choose: %1
No will choose: %2</source>
        <translation>Sandboxie-Plus 已於便攜模式中啟動，是否將沙箱資料夾放入其上級目錄？
「是」將選擇: %1
「否」將選擇: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2100"/>
        <source>Default sandbox not found; creating: %1</source>
        <translation>未找到預設沙箱; 建立中: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2140"/>
        <source>   -   NOT connected</source>
        <translation>   -   未連線</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2376"/>
        <source>PID %1: </source>
        <translation>處理程序 PID %1: </translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2378"/>
        <source>%1 (%2): </source>
        <translation>%1 (%2): </translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2407"/>
        <source>The selected feature set is only available to project supporters. Processes started in a box with this feature set enabled without a supporter certificate will be terminated after 5 minutes.&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Become a project supporter&lt;/a&gt;, and receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;</source>
        <translation>選取的功能只對專案贊助者可用。如果沒有贊助者憑證，在啟用此功能的沙箱內啟動的處理程序，將在 5 分鐘後自動終止。&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;成為專案贊助者&lt;/a&gt;，以取得&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;贊助者憑證&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="202"/>
        <source>Recovering file %1 to %2</source>
        <translation>復原檔案 %1 至 %2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="214"/>
        <source>The file %1 already exists, do you want to overwrite it?</source>
        <translation>檔案 %1 已存在，是否要將其覆蓋？</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="179"/>
        <location filename="SandManRecovery.cpp" line="215"/>
        <source>Do this for all files!</source>
        <translation>為所有檔案執行此操作！</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="99"/>
        <location filename="SandManRecovery.cpp" line="159"/>
        <source>Checking file %1</source>
        <translation>正在檢查檔案 %1</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="107"/>
        <source>The file %1 failed a security check!

%2</source>
        <translation>檔案 %1 未通過安全性檢查！

%2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="115"/>
        <source>All files passed the checks</source>
        <translation>所有檔案已通過檢查</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="178"/>
        <source>The file %1 failed a security check, do you want to recover it anyway?

%2</source>
        <oldsource>The file %1 failed a security check, do you want to recover it anyways?

%2</oldsource>
        <translation>檔案 %1 未通過安全性檢查，您要忽略警告並復原嗎？

%2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="243"/>
        <source>Failed to recover some files: 
</source>
        <translation>部分檔案復原失敗: 
</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2528"/>
        <source>Only Administrators can change the config.</source>
        <translation>僅管理員可變更此組態。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2538"/>
        <source>Please enter the configuration password.</source>
        <translation>請輸入組態的密碼。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2546"/>
        <source>Login Failed: %1</source>
        <translation>登入失敗: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2575"/>
        <source>Do you want to terminate all processes in all sandboxes?</source>
        <translation>確定要終止所有沙箱中的所有處理程序嗎？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2576"/>
        <source>Terminate all without asking</source>
        <translation>終止全部並不再詢問</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2634"/>
        <source>Sandboxie-Plus was started in portable mode and it needs to create necessary services. This will prompt for administrative privileges.</source>
        <translation>Sandboxie-Plus 已於便攜模式中啟動，需建立必要的服務。這將需要管理員權限。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2668"/>
        <source>CAUTION: Another agent (probably SbieCtrl.exe) is already managing this Sandboxie session, please close it first and reconnect to take over.</source>
        <translation>注意: 另一個代理 (可能是 SbieCtrl.exe) 已經在管理這個 Sandboxie 工作階段，請先關閉其他代理並重新連線進行接管控制。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2800"/>
        <source>Executing maintenance operation, please wait...</source>
        <translation>正在執行維護作業，請稍候...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2980"/>
        <source>Do you also want to reset hidden message boxes (yes), or only all log messages (no)?</source>
        <translation>請確認是否要重設已隱藏的訊息框 (選「是」)，或者僅重設所有日誌訊息 (選「否」)？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3077"/>
        <source>The changes will be applied automatically whenever the file gets saved.</source>
        <translation>每當檔案儲存後更改將自動套用。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3078"/>
        <source>The changes will be applied automatically as soon as the editor is closed.</source>
        <translation>變更將在編輯器關閉後自動提交。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3257"/>
        <source>Error Status: 0x%1 (%2)</source>
        <translation>錯誤程式碼: 0x%1 (%2)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3258"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3277"/>
        <source>A sandbox must be emptied before it can be deleted.</source>
        <translation>刪除沙箱前必須先清空。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3281"/>
        <source>Failed to copy box data files</source>
        <translation>複製沙箱資料檔案失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3286"/>
        <source>Failed to remove old box data files</source>
        <translation>移除舊沙箱資料檔案失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3300"/>
        <source>Unknown Error Status: 0x%1</source>
        <translation>未知錯誤狀態: 0x%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3507"/>
        <source>Case Sensitive</source>
        <translation>區分大小寫</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3508"/>
        <source>RegExp</source>
        <translation>正規表示式</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3509"/>
        <source>Highlight</source>
        <translation>醒目提示</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3510"/>
        <source>Close</source>
        <translation>關閉</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3511"/>
        <source>&amp;Find ...</source>
        <translation>尋找(&amp;F)...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3512"/>
        <source>All columns</source>
        <translation>所有欄</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3561"/>
        <source>&lt;h3&gt;About Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;Version %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2023 by DavidXanatos&lt;/p&gt;</source>
        <oldsource>&lt;h3&gt;About Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;Version %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2022 by DavidXanatos&lt;/p&gt;</oldsource>
        <translation>&lt;h3&gt;關於 Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;版本 %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2023 by DavidXanatos&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3569"/>
        <source>This copy of Sandboxie+ is certified for: %1</source>
        <translation>此 Sandboxie+ 複本已授權給: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3571"/>
        <source>Sandboxie+ is free for personal and non-commercial use.</source>
        <translation>Sandboxie+ 可免費用於個人和非商業用途。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3574"/>
        <source>Sandboxie-Plus is an open source continuation of Sandboxie.&lt;br /&gt;Visit &lt;a href=&quot;https://sandboxie-plus.com&quot;&gt;sandboxie-plus.com&lt;/a&gt; for more information.&lt;br /&gt;&lt;br /&gt;%3&lt;br /&gt;&lt;br /&gt;Driver version: %1&lt;br /&gt;Features: %2&lt;br /&gt;&lt;br /&gt;Icons from &lt;a href=&quot;https://icons8.com&quot;&gt;icons8.com&lt;/a&gt;</source>
        <translation>Sandboxie-Plus 是著名程式 Sandboxie 自開源以來的一個延續。&lt;br /&gt;拜訪 &lt;a href=&quot;https://sandboxie-plus.com&quot;&gt;sandboxie-plus.com&lt;/a&gt; 來了解更多資訊。&lt;br /&gt;&lt;br /&gt;%3&lt;br /&gt;&lt;br /&gt;驅動版本: %1&lt;br /&gt;功能: %2&lt;br /&gt;&lt;br /&gt;圖示來源 &lt;a href=&quot;https://icons8.com&quot;&gt;icons8.com&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3260"/>
        <source>Administrator rights are required for this operation.</source>
        <translation>此操作需要管理員權限。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="437"/>
        <source>Vintage View (like SbieCtrl)</source>
        <translation>經典檢視 (類似 SbieCtrl)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="454"/>
        <source>Contribute to Sandboxie-Plus</source>
        <translation>為 Sandboxie 做出貢獻</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="471"/>
        <source>Import Box</source>
        <translation>匯入沙箱</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="473"/>
        <location filename="SandMan.cpp" line="614"/>
        <source>Run Sandboxed</source>
        <translation>在沙箱中執行</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="575"/>
        <source>Disable Message Popup</source>
        <translation>停用訊息快顯視窗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="476"/>
        <location filename="SandMan.cpp" line="628"/>
        <source>Is Window Sandboxed?</source>
        <oldsource>Is Window Sandboxed</oldsource>
        <translation>檢查視窗是否沙箱化？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="521"/>
        <source>Show File Panel</source>
        <translation>顯示檔案面板</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="585"/>
        <location filename="SandMan.cpp" line="723"/>
        <location filename="SandMan.cpp" line="958"/>
        <location filename="SandMan.cpp" line="959"/>
        <source>Edit Sandboxie.ini</source>
        <translation>編輯 Sandboxie.ini</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="588"/>
        <source>Edit Templates.ini</source>
        <translation>編輯 Templates.ini</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="590"/>
        <source>Edit Sandboxie-Plus.ini</source>
        <translation>編輯 Sandboxie-Plus.ini</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="594"/>
        <location filename="SandMan.cpp" line="726"/>
        <source>Reload configuration</source>
        <translation>重新載入組態</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="613"/>
        <source>&amp;File</source>
        <translation>檔案(&amp;F)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="629"/>
        <source>Resource Access Monitor</source>
        <translation>資源存取監控</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="662"/>
        <source>Programs</source>
        <translation>程式</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="663"/>
        <source>Files and Folders</source>
        <translation>檔案和資料夾</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="691"/>
        <source>Import Sandbox</source>
        <translation>匯入沙箱</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="693"/>
        <source>Set Container Folder</source>
        <translation>設定沙箱容器資料夾</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="696"/>
        <source>Set Layout and Groups</source>
        <translation>設定外觀和群組</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="698"/>
        <source>Reveal Hidden Boxes</source>
        <translation>顯示隱藏的沙箱</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="704"/>
        <source>&amp;Configure</source>
        <translation>組態(&amp;C)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="708"/>
        <source>Program Alerts</source>
        <translation>程式警報</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="710"/>
        <source>Windows Shell Integration</source>
        <translation>Windows 殼層整合</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="712"/>
        <source>Software Compatibility</source>
        <translation>軟體相容性</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="721"/>
        <source>Lock Configuration</source>
        <translation>鎖定組態</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="740"/>
        <source>Sandbox %1</source>
        <translation>沙箱 %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="783"/>
        <source>New-Box Menu</source>
        <translation>新-沙箱選單</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="800"/>
        <source>Edit-ini Menu</source>
        <translation>編輯-ini 選單</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="844"/>
        <source>Toolbar Items</source>
        <translation>工具列項目</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="846"/>
        <source>Reset Toolbar</source>
        <translation>重設工具列</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1023"/>
        <source>Click to download update</source>
        <translation>按一下以下載更新</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1639"/>
        <source>No Force Process</source>
        <translation>沒有強制沙箱程序</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1856"/>
        <source>Removed Shortcut: %1</source>
        <translation>已移除捷徑: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1935"/>
        <source>Updated Shortcut to: %1</source>
        <translation>已更新捷徑至: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1937"/>
        <source>Added Shortcut to: %1</source>
        <translation>加入捷徑至: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1988"/>
        <source>Auto removing sandbox %1</source>
        <translation>自動刪除沙箱 %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2004"/>
        <source>Sandboxie-Plus Version: %1 (%2)</source>
        <translation>Sandboxie-Plus 版本: %1 (%2)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2006"/>
        <source>Data Directory: %1</source>
        <translation>資料目錄: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2069"/>
        <source> for Personal use</source>
        <translation> 個人使用者</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2071"/>
        <source>   -   for Non-Commercial use ONLY</source>
        <translation>   -   僅用於非商業用途</translation>
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
        <translation>在沙箱 %2 中啟動的程式 %1 將在 5 分鐘之後自動終止，因為此沙箱被設定為使用專案贊助者的專有功能。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2340"/>
        <source>The box %1 is configured to use features exclusively available to project supporters, these presets will be ignored.</source>
        <translation>沙箱 %1 被設定為使用專案贊助者專有的沙箱類型，這些預設選項將被忽略。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2341"/>
        <source>&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Become a project supporter&lt;/a&gt;, and receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;</source>
        <translation>&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;成為專案贊助者&lt;/a&gt;，以取得&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;贊助者憑證&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2475"/>
        <source>The evaluation period has expired!!!</source>
        <oldsource>The evaluation periode has expired!!!</oldsource>
        <translation>評估期已過！！！</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2595"/>
        <source>Please enter the duration, in seconds, for disabling Forced Programs rules.</source>
        <translation>請輸入「停用強制沙箱程式規則」的持續時間 (單位：秒)。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2610"/>
        <source>No Recovery</source>
        <translation>沒有復原檔案</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2616"/>
        <source>No Messages</source>
        <translation>沒有訊息</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2672"/>
        <source>&lt;b&gt;ERROR:&lt;/b&gt; The Sandboxie-Plus Manager (SandMan.exe) does not have a valid signature (SandMan.exe.sig). Please download a trusted release from the &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get&quot;&gt;official Download page&lt;/a&gt;.</source>
        <translation>&lt;b&gt;錯誤:&lt;/b&gt; Sandboxie-Plus 管理員 (SandMan.exe) 沒有有效的數位簽章 (SandMan.exe.sig)。請從&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get&quot;&gt;官方下載頁面&lt;/a&gt;下載可信賴的版本。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2775"/>
        <source>Maintenance operation failed (%1)</source>
        <translation>維護作業執行失敗 (%1)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2780"/>
        <source>Maintenance operation completed</source>
        <translation>維護作業完成</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2911"/>
        <source>In the Plus UI, this functionality has been integrated into the main sandbox list view.</source>
        <translation>在 Plus UI 中，此功能已被整合到主沙箱清單檢視中。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2912"/>
        <source>Using the box/group context menu, you can move boxes and groups to other groups. You can also use drag and drop to move the items around. Alternatively, you can also use the arrow keys while holding ALT down to move items up and down within their group.&lt;br /&gt;You can create new boxes and groups from the Sandbox menu.</source>
        <translation>使用「沙箱/群組」右鍵選單，您可以將沙箱在沙箱群組之間移動。同時，您也可以透過 Alt + 方向鍵或滑鼠拖曳來整理清單。&lt;br /&gt;另外，您可以透過右鍵選單來新增「沙箱/群組」。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3061"/>
        <source>You are about to edit the Templates.ini, this is generally not recommended.
This file is part of Sandboxie and all change done to it will be reverted next time Sandboxie is updated.</source>
        <oldsource>You are about to edit the Templates.ini, thsi is generally not recommeded.
This file is part of Sandboxie and all changed done to it will be reverted next time Sandboxie is updated.</oldsource>
        <translation>您正準備編輯範本設定檔 Templates.ini，但通常不推薦這麼做。
因為該檔案是 Sandboxie 的一部分並且所有的變更會在下次 Sandboxie 更新時被還原。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3136"/>
        <source>Sandboxie config has been reloaded</source>
        <translation>已重新載入 Sandboxie 組態</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3261"/>
        <source>Failed to execute: %1</source>
        <translation>執行失敗: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3262"/>
        <source>Failed to connect to the driver</source>
        <translation>連線驅動程式失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3263"/>
        <source>Failed to communicate with Sandboxie Service: %1</source>
        <translation>無法與 Sandboxie 服務建立聯絡: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3264"/>
        <source>An incompatible Sandboxie %1 was found. Compatible versions: %2</source>
        <translation>已發現不相容的 Sandboxie %1。相容版本為: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3265"/>
        <source>Can&apos;t find Sandboxie installation path.</source>
        <translation>無法找到 Sandboxie 安裝路徑。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3266"/>
        <source>Failed to copy configuration from sandbox %1: %2</source>
        <translation>複製沙箱組態 %1: %2 失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3267"/>
        <source>A sandbox of the name %1 already exists</source>
        <translation>沙箱名稱 %1 已存在</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3268"/>
        <source>Failed to delete sandbox %1: %2</source>
        <translation>刪除沙箱 %1: %2 失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3269"/>
        <source>The sandbox name can not be longer than 32 characters.</source>
        <translation>沙箱名稱不能超過 32 個字元。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3270"/>
        <source>The sandbox name can not be a device name.</source>
        <translation>沙箱名稱不能為裝置名稱。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3271"/>
        <source>The sandbox name can contain only letters, digits and underscores which are displayed as spaces.</source>
        <translation>沙箱名稱不能為空白，只能包含字母、數字和下劃線。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3272"/>
        <source>Failed to terminate all processes</source>
        <translation>終止所有處理程序失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3273"/>
        <source>Delete protection is enabled for the sandbox</source>
        <translation>沙箱的刪除保護已被啟用</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3274"/>
        <source>All sandbox processes must be stopped before the box content can be deleted</source>
        <translation>在刪除沙箱內容之前，必須先停止沙箱內的所有處理程序</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3275"/>
        <source>Error deleting sandbox folder: %1</source>
        <translation>刪除沙箱資料夾錯誤: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3278"/>
        <source>Failed to move directory &apos;%1&apos; to &apos;%2&apos;</source>
        <translation>移動目錄 &apos;%1&apos; 到 &apos;%2&apos; 失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3279"/>
        <source>This Snapshot operation can not be performed while processes are still running in the box.</source>
        <translation>因處理程序正在沙箱中執行，此快照操作無法完成。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3280"/>
        <source>Failed to create directory for new snapshot</source>
        <translation>為新快照建立目錄失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3282"/>
        <source>Snapshot not found</source>
        <translation>未發現快照</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3283"/>
        <source>Error merging snapshot directories &apos;%1&apos; with &apos;%2&apos;, the snapshot has not been fully merged.</source>
        <translation>合併快照目錄 &apos;%1&apos; 和 &apos;%2&apos; 錯誤，快照沒有被完全合併。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3284"/>
        <source>Failed to remove old snapshot directory &apos;%1&apos;</source>
        <translation>移除舊快照的目錄 &apos;%1&apos; 失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3285"/>
        <source>Can&apos;t remove a snapshot that is shared by multiple later snapshots</source>
        <translation>無法刪除由多個後續快照共享的快照</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3287"/>
        <source>You are not authorized to update configuration in section &apos;%1&apos;</source>
        <translation>您未被授權在 &apos;%1&apos; 更新組態</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3288"/>
        <source>Failed to set configuration setting %1 in section %2: %3</source>
        <translation>在 %2: %3 中設定組態選項 %1 失敗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3289"/>
        <source>Can not create snapshot of an empty sandbox</source>
        <translation>無法為空的沙箱建立快照</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3290"/>
        <source>A sandbox with that name already exists</source>
        <translation>已存在同名沙箱</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3291"/>
        <source>The config password must not be longer than 64 characters</source>
        <translation>組態密碼不得超過 64 個字元</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3292"/>
        <source>The operation was canceled by the user</source>
        <translation>此操作已被使用者取消</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3294"/>
        <source>Import/Export not available, 7z.dll could not be loaded</source>
        <translation>匯入/匯出無法使用，無法載入 7z.dll</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3295"/>
        <source>Failed to create the box archive</source>
        <translation>無法建立沙箱封存檔案</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3296"/>
        <source>Failed to open the 7z archive</source>
        <translation>無法開啟 7z 封存檔案</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3297"/>
        <source>Failed to unpack the box archive</source>
        <translation>無法解壓縮沙箱封存檔案</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3298"/>
        <source>The selected 7z file is NOT a box archive</source>
        <translation>所選的 7z 檔案不是沙箱封存檔案</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3324"/>
        <source>Operation failed for %1 item(s).</source>
        <translation>%1 項操作失敗。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3355"/>
        <source>Do you want to open %1 in a sandboxed (yes) or unsandboxed (no) Web browser?</source>
        <translation>是否在沙箱化網頁瀏覽器開啟連結 %1 ？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3356"/>
        <source>Remember choice for later.</source>
        <translation>記住選擇供之後使用。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2481"/>
        <source>The supporter certificate is not valid for this build, please get an updated certificate</source>
        <translation>此贊助者憑證對此版本沙箱無效，請取得更新的憑證</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2484"/>
        <source>The supporter certificate has expired%1, please get an updated certificate</source>
        <oldsource>The supporter certificate is expired %1 days ago, please get an updated certificate</oldsource>
        <translation>此贊助者憑證已逾期%1，請取得更新的憑證</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2485"/>
        <source>, but it remains valid for the current build</source>
        <translation>，但它對目前組建的沙箱版本仍然有效</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2487"/>
        <source>The supporter certificate will expire in %1 days, please get an updated certificate</source>
        <translation>此贊助者憑證將在 %1 天後逾期，請取得更新的憑證</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="88"/>
        <source>The selected window is running as part of program %1 in sandbox %2</source>
        <translation>選擇的視窗正作為程式 %1 一部分執行在沙箱 %2 中</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="95"/>
        <source>The selected window is not running as part of any sandboxed program.</source>
        <translation>選擇的視窗並非作為任何沙箱化程式的一部分而執行。</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="134"/>
        <source>Drag the Finder Tool over a window to select it, then release the mouse to check if the window is sandboxed.</source>
        <translation>拖曳搜尋小工具 (左方) 到要選取的視窗上，放開滑鼠檢查視窗是否來自沙箱化的程式。</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="204"/>
        <source>Sandboxie-Plus - Window Finder</source>
        <translation>Sandboxie-Plus - 視窗尋找工具</translation>
    </message>
    <message>
        <location filename="main.cpp" line="123"/>
        <source>Sandboxie Manager can not be run sandboxed!</source>
        <translation>Sandboxie 管理員不能在沙箱中執行！</translation>
    </message>
</context>
<context>
    <name>CSbieModel</name>
    <message>
        <location filename="Models/SbieModel.cpp" line="159"/>
        <source>Box Group</source>
        <translation>沙箱群組</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="315"/>
        <source>Empty</source>
        <translation>空白</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="559"/>
        <source>Name</source>
        <translation>名稱</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="560"/>
        <source>Process ID</source>
        <translation>處理程序 ID</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="561"/>
        <source>Status</source>
        <translation>狀態</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="562"/>
        <source>Title</source>
        <translation>標題</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="563"/>
        <source>Info</source>
        <translation>資訊</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="567"/>
        <source>Path / Command Line</source>
        <translation>路徑 / 命令列</translation>
    </message>
</context>
<context>
    <name>CSbieProcess</name>
    <message>
        <location filename="SbieProcess.cpp" line="59"/>
        <source>Sbie RpcSs</source>
        <translation>沙箱 RPC 子系統</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="60"/>
        <source>Sbie DcomLaunch</source>
        <translation>沙箱 DCOM 服務啟動器</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="61"/>
        <source>Sbie Crypto</source>
        <translation>沙箱密碼學服務</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="62"/>
        <source>Sbie WuauServ</source>
        <translation>沙箱 WuauServ</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="63"/>
        <source>Sbie BITS</source>
        <translation>沙箱背景智慧傳輸服務 (BITS)</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="64"/>
        <source>Sbie Svc</source>
        <translation>沙箱軟體服務</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="65"/>
        <source>MSI Installer</source>
        <translation>MSI 安裝程式</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="66"/>
        <source>Trusted Installer</source>
        <translation>可信安裝程式</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="67"/>
        <source>Windows Update</source>
        <translation>Windows 更新</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="68"/>
        <source>Windows Explorer</source>
        <translation>Windows 檔案總管</translation>
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
        <translation>Windows Media Player</translation>
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
        <translation>Windows Live Mail</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="75"/>
        <source>Service Model Reg</source>
        <translation>服務模型登錄</translation>
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
        <translation>Windows 捷徑服務</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="80"/>
        <source>Chromium Based</source>
        <translation>基於 Chromium 的瀏覽器</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="81"/>
        <source>Google Updater</source>
        <translation>Google 更新程式</translation>
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
        <translation>Firefox 外掛程式容器</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="87"/>
        <source>Generic Web Browser</source>
        <translation>一般網頁瀏覽器</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="88"/>
        <source>Generic Mail Client</source>
        <translation>一般電子郵件用戶端</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="89"/>
        <source>Thunderbird</source>
        <translation>Thunderbird</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="99"/>
        <source>Terminated</source>
        <translation>已終止</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="105"/>
        <source>Forced </source>
        <translation>強制沙箱 </translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="103"/>
        <source>Running</source>
        <translation>運作中</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="109"/>
        <source> Elevated</source>
        <translation> 已提升權限</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="111"/>
        <source> as System</source>
        <translation> 系統權限</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="114"/>
        <source> in session %1</source>
        <translation> 在工作階段 %1</translation>
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
        <translation>建立新沙箱</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="397"/>
        <source>Remove Group</source>
        <translation>刪除群組</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="161"/>
        <location filename="Views/SbieView.cpp" line="291"/>
        <source>Run</source>
        <translation>執行</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="166"/>
        <source>Run Program</source>
        <translation>執行程式</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="167"/>
        <source>Run from Start Menu</source>
        <translation>從開始選單執行</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="178"/>
        <source>Default Web Browser</source>
        <translation>預設網頁瀏覽器</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="179"/>
        <source>Default eMail Client</source>
        <translation>預設電子郵件用戶端</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="181"/>
        <source>Windows Explorer</source>
        <translation>Windows 檔案總管</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="182"/>
        <source>Registry Editor</source>
        <translation>登錄編輯程式</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="183"/>
        <source>Programs and Features</source>
        <translation>程式和功能</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="194"/>
        <source>Terminate All Programs</source>
        <translation>終止所有程式</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="102"/>
        <location filename="Views/SbieView.cpp" line="200"/>
        <location filename="Views/SbieView.cpp" line="259"/>
        <location filename="Views/SbieView.cpp" line="339"/>
        <location filename="Views/SbieView.cpp" line="378"/>
        <source>Create Shortcut</source>
        <translation>建立捷徑</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="202"/>
        <location filename="Views/SbieView.cpp" line="324"/>
        <source>Explore Content</source>
        <translation>瀏覽內容</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="204"/>
        <location filename="Views/SbieView.cpp" line="331"/>
        <source>Snapshots Manager</source>
        <translation>快照管理</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="205"/>
        <source>Recover Files</source>
        <translation>復原檔案</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="206"/>
        <location filename="Views/SbieView.cpp" line="323"/>
        <source>Delete Content</source>
        <translation>刪除內容</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="213"/>
        <source>Sandbox Presets</source>
        <translation>沙箱預設</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="215"/>
        <source>Ask for UAC Elevation</source>
        <translation>詢問 UAC 權限提升</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="216"/>
        <source>Drop Admin Rights</source>
        <translation>廢棄管理員許可</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="217"/>
        <source>Emulate Admin Rights</source>
        <translation>模擬管理員許可</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="225"/>
        <source>Block Internet Access</source>
        <translation>阻止網際網路存取</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="227"/>
        <source>Allow Network Shares</source>
        <translation>允許區域網路共用</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="208"/>
        <source>Sandbox Options</source>
        <translation>沙箱選項</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="177"/>
        <source>Standard Applications</source>
        <translation>標準應用程式</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="197"/>
        <source>Browse Files</source>
        <translation>瀏覽檔案</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="236"/>
        <location filename="Views/SbieView.cpp" line="329"/>
        <source>Sandbox Tools</source>
        <translation>沙箱工具</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="237"/>
        <source>Duplicate Box Config</source>
        <translation>複製沙箱組態</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="240"/>
        <location filename="Views/SbieView.cpp" line="342"/>
        <source>Rename Sandbox</source>
        <translation>重新命名沙箱</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="241"/>
        <location filename="Views/SbieView.cpp" line="343"/>
        <source>Move Sandbox</source>
        <translation>移動沙箱</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="252"/>
        <location filename="Views/SbieView.cpp" line="354"/>
        <source>Remove Sandbox</source>
        <translation>刪除沙箱</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="257"/>
        <location filename="Views/SbieView.cpp" line="376"/>
        <source>Terminate</source>
        <translation>終止</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="260"/>
        <source>Preset</source>
        <translation>預設</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="100"/>
        <location filename="Views/SbieView.cpp" line="261"/>
        <source>Pin to Run Menu</source>
        <translation>固定到執行選單</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="263"/>
        <source>Block and Terminate</source>
        <translation>阻止並終止</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="267"/>
        <source>Allow internet access</source>
        <translation>允許網際網路存取</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="269"/>
        <source>Force into this sandbox</source>
        <translation>強制加入此沙箱</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="271"/>
        <source>Set Linger Process</source>
        <translation>設定駐留處理程序</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="273"/>
        <source>Set Leader Process</source>
        <translation>設定引導處理程序</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="536"/>
        <source>    File root: %1
</source>
        <translation>    檔案根目錄: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="537"/>
        <source>    Registry root: %1
</source>
        <translation>    登錄根目錄: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="538"/>
        <source>    IPC root: %1
</source>
        <translation>    IPC 根目錄: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="540"/>
        <source>Options:
    </source>
        <translation>選項:
    </translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="748"/>
        <source>[None]</source>
        <translation>[無]</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1059"/>
        <source>Please enter a new group name</source>
        <translation>請輸入新的群組名稱</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="857"/>
        <source>Do you really want to remove the selected group(s)?</source>
        <translation>確定要刪除所選群組嗎？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="152"/>
        <location filename="Views/SbieView.cpp" line="282"/>
        <source>Create Box Group</source>
        <translation>建立沙箱群組</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="395"/>
        <source>Rename Group</source>
        <translation>重新命名群組</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="157"/>
        <location filename="Views/SbieView.cpp" line="287"/>
        <source>Stop Operations</source>
        <translation>停止作業</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="186"/>
        <source>Command Prompt</source>
        <translation>命令提示字元</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="187"/>
        <source>Command Prompt (as Admin)</source>
        <translation>命令提示字元 (管理員)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="191"/>
        <source>Command Prompt (32-bit)</source>
        <translation>命令提示字元 (32 位元)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="176"/>
        <source>Execute Autorun Entries</source>
        <translation>執行「自動執行」條目</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="330"/>
        <source>Browse Content</source>
        <translation>瀏覽內容</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="196"/>
        <source>Box Content</source>
        <translation>沙箱內容</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="203"/>
        <source>Open Registry</source>
        <translation>開啟登錄</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="199"/>
        <location filename="Views/SbieView.cpp" line="338"/>
        <source>Refresh Info</source>
        <translation>重新整理資訊</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="153"/>
        <location filename="Views/SbieView.cpp" line="283"/>
        <source>Import Box</source>
        <translation>匯入沙箱</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="170"/>
        <location filename="Views/SbieView.cpp" line="302"/>
        <source>(Host) Start Menu</source>
        <translation>開始選單 (主機)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="231"/>
        <source>Immediate Recovery</source>
        <translation>即時復原</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="233"/>
        <source>Disable Force Rules</source>
        <translation>停用強制規則</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="238"/>
        <source>Export Box</source>
        <translation>匯出沙箱</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="242"/>
        <location filename="Views/SbieView.cpp" line="344"/>
        <source>Move Up</source>
        <translation>向上移</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="247"/>
        <location filename="Views/SbieView.cpp" line="349"/>
        <source>Move Down</source>
        <translation>向下移</translation>
    </message>
    <message>
        <source>Run Sandboxed</source>
        <translation type="vanished">在沙箱中執行</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="296"/>
        <source>Run Web Browser</source>
        <translation>執行網頁瀏覽器</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="297"/>
        <source>Run eMail Reader</source>
        <translation>執行電子郵件閱讀器</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="298"/>
        <source>Run Any Program</source>
        <translation>執行任意程式 (Run)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="299"/>
        <source>Run From Start Menu</source>
        <translation>從開始選單執行</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="307"/>
        <source>Run Windows Explorer</source>
        <translation>執行 Windows 檔案總管</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="321"/>
        <source>Terminate Programs</source>
        <translation>終止程式</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="322"/>
        <source>Quick Recover</source>
        <translation>快速復原</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="327"/>
        <source>Sandbox Settings</source>
        <translation>沙箱設定</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="334"/>
        <source>Duplicate Sandbox Config</source>
        <translation>複製沙箱組態</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="335"/>
        <source>Export Sandbox</source>
        <translation>匯出沙箱</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="396"/>
        <source>Move Group</source>
        <translation>移動群組</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="847"/>
        <source>Please enter a new name for the Group.</source>
        <translation>請為群組輸入新名稱。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="894"/>
        <source>Move entries by (negative values move up, positive values move down):</source>
        <translation>移動條目 (負值向上移動，正值向下移動):</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="942"/>
        <source>A group can not be its own parent.</source>
        <translation>群組不能作為其本身的上級群組。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1031"/>
        <source>This name is already in use, please select an alternative box name</source>
        <translation>名稱已被使用，請選擇其他沙箱名稱</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1045"/>
        <source>Importing: %1</source>
        <translation>正在匯入: %1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1085"/>
        <source>The Sandbox name and Box Group name cannot use the &apos;,()&apos; symbol.</source>
        <translation>沙箱名稱和沙箱群組名稱不能使用 &apos;,()&apos; 符號。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1090"/>
        <source>This name is already used for a Box Group.</source>
        <translation>名稱已被用於現有的其它沙箱群組。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1095"/>
        <source>This name is already used for a Sandbox.</source>
        <translation>名稱已被用於現有的其它沙箱。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1145"/>
        <location filename="Views/SbieView.cpp" line="1215"/>
        <location filename="Views/SbieView.cpp" line="1462"/>
        <source>Don&apos;t show this message again.</source>
        <translation>不再顯示此訊息。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1206"/>
        <location filename="Views/SbieView.cpp" line="1226"/>
        <location filename="Views/SbieView.cpp" line="1630"/>
        <source>This Sandbox is empty.</source>
        <translation>此沙箱為空。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1233"/>
        <source>WARNING: The opened registry editor is not sandboxed, please be careful and only do changes to the pre-selected sandbox locations.</source>
        <translation>警告: 開啟的登錄編輯程式未沙箱化，請審慎且僅對預先選取的沙箱位置進行修改。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1234"/>
        <source>Don&apos;t show this warning in future</source>
        <translation>之後不再顯示此警告</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1290"/>
        <source>Please enter a new name for the duplicated Sandbox.</source>
        <translation>請為複製的沙箱輸入一個新名稱。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1290"/>
        <source>%1 Copy</source>
        <translatorcomment>沙箱名稱只能包含字母、數字和下劃線，不應對此處的文字進行翻譯！</translatorcomment>
        <translation>%1 Copy</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1018"/>
        <location filename="Views/SbieView.cpp" line="1329"/>
        <source>Select file name</source>
        <translation>選擇檔名</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1018"/>
        <location filename="Views/SbieView.cpp" line="1329"/>
        <source>7-zip Archive (*.7z)</source>
        <translation>7-zip 封存檔案 (*.7z)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1337"/>
        <source>Exporting: %1</source>
        <translation>正在匯出: %1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1344"/>
        <source>Please enter a new name for the Sandbox.</source>
        <translation>請為沙箱輸入新名稱。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1365"/>
        <source>Do you really want to remove the selected sandbox(es)?&lt;br /&gt;&lt;br /&gt;Warning: The box content will also be deleted!</source>
        <oldsource>Do you really want to remove the selected sandbox(es)?</oldsource>
        <translation>確定要刪除選取的沙箱？&lt;br /&gt;&lt;br /&gt;警告: 沙箱內的內容也將一併刪除！</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1403"/>
        <source>This Sandbox is already empty.</source>
        <translation>此沙箱為空。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1413"/>
        <source>Do you want to delete the content of the selected sandbox?</source>
        <translation>確定要刪除所選沙箱的內容嗎？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1414"/>
        <location filename="Views/SbieView.cpp" line="1418"/>
        <source>Also delete all Snapshots</source>
        <translation>同時刪除所有快照</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1417"/>
        <source>Do you really want to delete the content of all selected sandboxes?</source>
        <translation>確定要刪除所選沙箱的內容嗎？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1444"/>
        <source>Do you want to terminate all processes in the selected sandbox(es)?</source>
        <translation>確定要終止選定沙箱中的所有處理程序嗎？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1445"/>
        <location filename="Views/SbieView.cpp" line="1539"/>
        <source>Terminate without asking</source>
        <translation>終止且不再詢問</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1460"/>
        <source>The Sandboxie Start Menu will now be displayed. Select an application from the menu, and Sandboxie will create a new shortcut icon on your real desktop, which you can use to invoke the selected application under the supervision of Sandboxie.</source>
        <oldsource>The Sandboxie Start Menu will now be displayed. Select an application from the menu, and Sandboxie will create a newshortcut icon on your real desktop, which you can use to invoke the selected application under the supervision of Sandboxie.</oldsource>
        <translation>現在將顯示 Sandboxie 開始選單。從選單中選擇一個應用程式，Sandboxie 將在真實桌面上建立一個新的捷徑圖示，您可以用它來呼叫所選受 Sandboxie 監督的應用程式。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1514"/>
        <location filename="Views/SbieView.cpp" line="1566"/>
        <source>Create Shortcut to sandbox %1</source>
        <translation>為沙箱 %1 建立捷徑</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1538"/>
        <source>Do you want to terminate %1?</source>
        <oldsource>Do you want to %1 %2?</oldsource>
        <translation>確定要終止 %1 嗎？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1538"/>
        <source>the selected processes</source>
        <translation>選取的處理程序</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1589"/>
        <source>This box does not have Internet restrictions in place, do you want to enable them?</source>
        <translation>此沙箱無網際網路限制，確定要啟用嗎？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1673"/>
        <source>This sandbox is disabled, do you want to enable it?</source>
        <translation>此沙箱已停用，是否啟用？</translation>
    </message>
</context>
<context>
    <name>CSelectBoxWindow</name>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="89"/>
        <source>Sandboxie-Plus - Run Sandboxed</source>
        <translation>Sandboxie-Plus - 在沙箱內執行</translation>
    </message>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="206"/>
        <source>Are you sure you want to run the program outside the sandbox?</source>
        <translation>您確定要在沙箱外執行程式嗎？</translation>
    </message>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="219"/>
        <source>Please select a sandbox.</source>
        <translation>請選擇一個沙箱。</translation>
    </message>
</context>
<context>
    <name>CSettingsWindow</name>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="130"/>
        <source>Sandboxie Plus - Global Settings</source>
        <oldsource>Sandboxie Plus - Settings</oldsource>
        <translation>Sandboxie Plus - 全域設定</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="216"/>
        <source>Auto Detection</source>
        <translation>自動偵測</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="217"/>
        <source>No Translation</source>
        <translation>無翻譯</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="237"/>
        <source>Don&apos;t integrate links</source>
        <translation>不整合連結</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="238"/>
        <source>As sub group</source>
        <translation>作為次級群組</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="239"/>
        <source>Fully integrate</source>
        <translation>完全整合</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="241"/>
        <source>Don&apos;t show any icon</source>
        <oldsource>Don&apos;t integrate links</oldsource>
        <translation>不顯示任何圖示</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="242"/>
        <source>Show Plus icon</source>
        <translation>顯示 Plus 版圖示</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="243"/>
        <source>Show Classic icon</source>
        <translation>顯示經典版圖示</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="245"/>
        <source>All Boxes</source>
        <translation>所有沙箱</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="246"/>
        <source>Active + Pinned</source>
        <translation>啟用中或已固定的沙箱</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="247"/>
        <source>Pinned Only</source>
        <translation>僅已固定的沙箱</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="249"/>
        <source>None</source>
        <translation>無</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="250"/>
        <source>Native</source>
        <translation>原生</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="251"/>
        <source>Qt</source>
        <translation>Qt</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="253"/>
        <source>Ignore</source>
        <translation>忽略</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="265"/>
        <source>%1</source>
        <oldsource>%1 %</oldsource>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="496"/>
        <source>Search for settings</source>
        <translation>搜尋設定</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="719"/>
        <location filename="Windows/SettingsWindow.cpp" line="720"/>
        <location filename="Windows/SettingsWindow.cpp" line="730"/>
        <source>Run &amp;Sandboxed</source>
        <translation>在沙箱中執行(&amp;S)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="751"/>
        <source>Sandboxed Web Browser</source>
        <translation>沙箱化網頁瀏覽器</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="254"/>
        <location filename="Windows/SettingsWindow.cpp" line="259"/>
        <source>Notify</source>
        <translation>通知</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="255"/>
        <location filename="Windows/SettingsWindow.cpp" line="260"/>
        <source>Download &amp; Notify</source>
        <translation>下載並通知</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="256"/>
        <location filename="Windows/SettingsWindow.cpp" line="261"/>
        <source>Download &amp; Install</source>
        <translation>下載並安裝</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="345"/>
        <source>Browse for Program</source>
        <translation>瀏覽程式</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="412"/>
        <source>Add %1 Template</source>
        <translation>加入 %1 範本</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="597"/>
        <source>Please enter message</source>
        <translation>請輸入訊息</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="629"/>
        <source>Select Program</source>
        <translation>選擇程式</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="629"/>
        <source>Executables (*.exe *.cmd)</source>
        <translation>可執行檔案 (*.exe *.cmd)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="633"/>
        <location filename="Windows/SettingsWindow.cpp" line="646"/>
        <source>Please enter a menu title</source>
        <translation>請輸入一個選單標題</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="642"/>
        <source>Please enter a command</source>
        <translation>請輸入一則命令</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="964"/>
        <source>This supporter certificate has expired, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>此贊助者憑證已逾期，請&lt;a href=&quot;sbie://update/cert&quot;&gt;取得新憑證&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="967"/>
        <source>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Plus features will be disabled in %1 days.&lt;/font&gt;</source>
        <translation>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Plus 附加的進階功能將在 %1 天後被停用。&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="969"/>
        <source>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;For this build Plus features remain enabled.&lt;/font&gt;</source>
        <translation>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;在此版本中，Plus 附加的進階功能仍是可用的。&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="971"/>
        <source>&lt;br /&gt;Plus features are no longer enabled.</source>
        <translation>&lt;br /&gt;Plus 附加的進階功能已不再可用。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="977"/>
        <source>This supporter certificate will &lt;font color=&apos;red&apos;&gt;expire in %1 days&lt;/font&gt;, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>此贊助者憑證將&lt;font color=&apos;red&apos;&gt;在 %1 天後逾期&lt;/font&gt;，請&lt;a href=&quot;sbie://update/cert&quot;&gt;取得新憑證&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1006"/>
        <source>Supporter certificate required</source>
        <oldsource>Supproter certificate required</oldsource>
        <translation>需要贊助者憑證</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1112"/>
        <source>Run &amp;Un-Sandboxed</source>
        <translation>在沙箱外執行(&amp;U)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1371"/>
        <source>This does not look like a certificate. Please enter the entire certificate, not just a portion of it.</source>
        <translation>這看起來不像是一份憑證。請輸入完整的憑證，而不僅僅是其中的一部分。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1390"/>
        <source>This certificate is unfortunately expired.</source>
        <translation>很不幸此憑證已逾期。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1392"/>
        <source>This certificate is unfortunately outdated.</source>
        <translation>很不幸此憑證已廢止。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1395"/>
        <source>Thank you for supporting the development of Sandboxie-Plus.</source>
        <translation>感謝您對 Sandboxie-Plus 開發工作的支持。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1402"/>
        <source>This support certificate is not valid.</source>
        <translation>此贊助者憑證無效。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1440"/>
        <location filename="Windows/SettingsWindow.cpp" line="1576"/>
        <source>Select Directory</source>
        <translation>選擇目錄</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1469"/>
        <source>&lt;a href=&quot;check&quot;&gt;Check Now&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;check&quot;&gt;立即檢查&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1536"/>
        <source>Please enter the new configuration password.</source>
        <translation>請輸入新組態密碼。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1540"/>
        <source>Please re-enter the new configuration password.</source>
        <translation>請再次輸入新組態密碼。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1545"/>
        <source>Passwords did not match, please retry.</source>
        <translation>密碼不匹配，請重新輸入。</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1557"/>
        <source>Process</source>
        <translation>處理程序</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1557"/>
        <source>Folder</source>
        <translation>資料夾</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1567"/>
        <source>Please enter a program file name</source>
        <translation>請輸入一個程式檔案名稱</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1635"/>
        <source>Please enter the template identifier</source>
        <translation>請輸入範本識別碼</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1643"/>
        <source>Error: %1</source>
        <translation>錯誤: %1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1668"/>
        <source>Do you really want to delete the selected local template(s)?</source>
        <translation>要刪除所選取的本地範本嗎？</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1824"/>
        <source>%1 (Current)</source>
        <translation>%1 (目前)</translation>
    </message>
</context>
<context>
    <name>CSetupWizard</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="29"/>
        <source>Setup Wizard</source>
        <translation>設定精靈</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="40"/>
        <source>The decision you make here will affect which page you get to see next.</source>
        <translation>您在這裡做出的決定將影響您接下來看到的頁面內容。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="43"/>
        <source>This help is likely not to be of any help.</source>
        <translation>此類幫助很可能沒用提供任何實質性幫助。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="47"/>
        <source>Sorry, I already gave all the help I could.</source>
        <translation>對不起，我已經提供了所有能給的協助。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="49"/>
        <source>Setup Wizard Help</source>
        <translation>設定精靈幫助</translation>
    </message>
</context>
<context>
    <name>CShellPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="417"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; shell integration</source>
        <translation>設定 &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 命令整合組態</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="418"/>
        <source>Configure how Sandboxie-Plus should integrate with your system.</source>
        <translation>設定 Sandboxie-Plus 應如何與您的系統整合。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="422"/>
        <source>Start UI with Windows</source>
        <translation>Windows 啟動時開啟使用者介面</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="427"/>
        <source>Add &apos;Run Sandboxed&apos; to the explorer context menu</source>
        <translation>在檔案總管右鍵新增「在沙箱中執行」</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="432"/>
        <source>Add desktop shortcut for starting Web browser under Sandboxie</source>
        <translation>加入在 Sandboxie 中開啟網頁瀏覽器的捷徑到桌面</translation>
    </message>
</context>
<context>
    <name>CSnapshotsWindow</name>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="24"/>
        <source>%1 - Snapshots</source>
        <translation>%1 - 快照</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="38"/>
        <source>Snapshot</source>
        <translation>快照</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="56"/>
        <source>Revert to empty box</source>
        <translation>復原為空白沙箱</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="103"/>
        <source> (default)</source>
        <translation> (預設)</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="173"/>
        <source>Please enter a name for the new Snapshot.</source>
        <translation>請輸入新快照名稱。</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="173"/>
        <source>New Snapshot</source>
        <translation>新快照</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="196"/>
        <source>Do you really want to switch the active snapshot? Doing so will delete the current state!</source>
        <translation>確定要切換正在使用的快照？這將刪除目前的狀態！</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="230"/>
        <source>Do you really want to delete the selected snapshot?</source>
        <translation>確定要刪除所選快照？</translation>
    </message>
</context>
<context>
    <name>CStackView</name>
    <message>
        <location filename="Views/StackView.cpp" line="17"/>
        <source>#|Symbol</source>
        <translation>#|符號</translation>
    </message>
</context>
<context>
    <name>CSummaryPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="527"/>
        <source>Create the new Sandbox</source>
        <translation>建立新沙箱</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="535"/>
        <source>Almost complete, click Finish to create a new sandbox and conclude the wizard.</source>
        <translation>即將就緒，按下「完成」按鈕以建立新沙箱並結束精靈。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="544"/>
        <source>Save options as new defaults</source>
        <translation>儲存選項為新的預設設定</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="555"/>
        <source>Don&apos;t show the summary page in future (unless advanced options were set)</source>
        <translation>以後不再顯示總結頁面 (除非啟用進階選項)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="574"/>
        <source>
This Sandbox will be saved to: %1</source>
        <translation>
該沙箱將儲存至: %1</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="577"/>
        <source>
This box&apos;s content will be DISCARDED when it&apos;s closed, and the box will be removed.</source>
        <oldsource>
This box&apos;s content will be DISCARDED when its closed, and the box will be removed.</oldsource>
        <translation>
該沙箱中的內容將在所有程式結束後被刪除，同時沙箱本身將被移除。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="579"/>
        <source>
This box will DISCARD its content when its closed, its suitable only for temporary data.</source>
        <translation>
該沙箱中的內容將在所有程式結束後被刪除，僅適合暫存的臨時資料。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="581"/>
        <source>
Processes in this box will not be able to access the internet or the local network, this ensures all accessed data to stay confidential.</source>
        <translation>
該沙箱中所有處理程序無法存取網際網路和區域網路，以確保所有可存取的資料不被洩露。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="583"/>
        <source>
This box will run the MSIServer (*.msi installer service) with a system token, this improves the compatibility but reduces the security isolation.</source>
        <oldsource>
This box will run the MSIServer (*.msi installer service) with a system token, this improves the compatybility but reduces the security isolation.</oldsource>
        <translation>
該沙箱允許 MSIServer (*.msi 安裝程式服務) 在沙箱內使用系統權杖執行，這將改善相容性但會影響安全性隔離效果。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="585"/>
        <source>
Processes in this box will think they are run with administrative privileges, without actually having them, hence installers can be used even in a security hardened box.</source>
        <translation>
該沙箱中所有處理程序將認為它們執行在系統管理員模式下，即使實際上並沒有該權限，這有助於在安全性防護加固型沙箱中執行安裝程式。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="587"/>
        <source>
Processes in this box will be running with a custom process token indicating the sandbox they belong to.</source>
        <oldsource>
Processes in this box will be running with a custom process token indicating the sandbox thay belong to.</oldsource>
        <translation>
在此沙箱內的處理程序將以自訂處理程序權杖執行，以表明其沙箱歸屬。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="620"/>
        <source>Failed to create new box: %1</source>
        <translation>無法建立新沙箱: %1</translation>
    </message>
</context>
<context>
    <name>CSupportDialog</name>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="115"/>
        <source>The installed supporter certificate &lt;b&gt;has expired %1 days ago&lt;/b&gt; and &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;must be renewed&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;</source>
        <oldsource>The installed supporter certificate &lt;b&gt;has expired %1 days ago&lt;/b&gt; and &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;must be renewed&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;</oldsource>
        <translation>安裝的贊助者憑證&lt;b&gt;已於 %1 天前過期&lt;/b&gt;，並且&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;必須續期&lt;/a&gt;。&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="117"/>
        <source>&lt;b&gt;You have installed Sandboxie-Plus more than %1 days ago.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</source>
        <translation>&lt;b&gt;您已安裝 Sandboxie-Plus 超過 %1 天了。&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="119"/>
        <source>&lt;u&gt;Commercial use of Sandboxie past the evaluation period&lt;/u&gt;, requires a valid &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;support certificate&lt;/a&gt;.</source>
        <oldsource>&lt;u&gt;Commercial use of Sandboxie past the evaluation period&lt;/u&gt;, requires a valid &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;support certificate&lt;/a&gt;.</oldsource>
        <translation>&lt;u&gt;評估期過後使用 Sandboxie 進行商業用途&lt;/u&gt;，需要一個有效的&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;贊助者憑證&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="126"/>
        <source>The installed supporter certificate is &lt;b&gt;outdated&lt;/b&gt; and it is &lt;u&gt;not valid for&lt;b&gt; this version&lt;/b&gt;&lt;/u&gt; of Sandboxie-Plus.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>安裝的贊助者憑證&lt;b&gt;已逾期&lt;/b&gt;，並且&lt;u&gt;不再適用於&lt;b&gt;此版本&lt;/b&gt;&lt;/u&gt;的 Sandboxie-Plus。&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="128"/>
        <source>The installed supporter certificate is &lt;b&gt;expired&lt;/b&gt; and &lt;u&gt;should be renewed&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</source>
        <oldsource>The installed supporter certificate is &lt;b&gt;expired&lt;/b&gt; and &lt;u&gt;should to be renewed&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</oldsource>
        <translation>安裝的贊助者憑證&lt;b&gt;已逾期&lt;/b&gt;，&lt;u&gt;需要進行續期&lt;/u&gt;。&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="130"/>
        <source>&lt;b&gt;You have been using Sandboxie-Plus for more than %1 days now.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</source>
        <translation>&lt;b&gt;您已經使用 Sandboxie-Plus 超過 %1 天了。&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="133"/>
        <source>Sandboxie on ARM64 requires a valid supporter certificate for continued use.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Sandboxie 的 ARM64 支援需要一份有效的贊助者憑證。&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="135"/>
        <source>Personal use of Sandboxie is free of charge on x86/x64, although some functionality is only available to project supporters.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>個人用途的 Sandboxie 在 x86/x64 平台上是免費的，儘管有些功能只對專案贊助者開放。&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="138"/>
        <source>Please continue &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;supporting the project&lt;/a&gt; by renewing your &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; and continue using the &lt;b&gt;enhanced functionality&lt;/b&gt; in new builds.</source>
        <oldsource>Please continue &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt; by renewing your &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; and continue using the &lt;b&gt;enhanced functionality&lt;/b&gt; in new builds.</oldsource>
        <translation>請繼續透過更新您的&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;贊助者憑證&lt;/a&gt;來&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;支援專案&lt;/a&gt;，並繼續使用新版本中的&lt;b&gt;增強功能&lt;/b&gt;。</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="141"/>
        <source>Sandboxie &lt;u&gt;without&lt;/u&gt; a valid supporter certificate will sometimes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;pause for a few seconds&lt;/font&gt;&lt;/b&gt;. This pause allows you to consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;purchasing a supporter certificate&lt;/a&gt; or &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;earning one by contributing&lt;/a&gt; to the project. &lt;br /&gt;&lt;br /&gt;A &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; not just removes this reminder, but also enables &lt;b&gt;exclusive enhanced functionality&lt;/b&gt; providing better security and compatibility.</source>
        <oldsource>Sandboxie &lt;u&gt;without&lt;/u&gt; a valid supporter certificate will sometimes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;pause for a few seconds&lt;/font&gt;&lt;/b&gt;, to give you time to contemplate the option of &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;A &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; not just removes this reminder, but also enables &lt;b&gt;exclusive enhanced functionality&lt;/b&gt; providing better security and compatibility.</oldsource>
        <translation>Sandboxie &lt;u&gt;在沒有&lt;/u&gt;有效的贊助者憑證時有時會&lt;b&gt;&lt;font color=&apos;red&apos;&gt;快顯視窗提醒&lt;/font&gt;&lt;/b&gt;，讓您考慮是否&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;贊助支援此專案&lt;/a&gt;或&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;透過參與專案來取得一個&lt;/a&gt; (但不會中斷不需要贊助者憑證的沙箱內的程式)，。&lt;br /&gt;&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;贊助者憑證&lt;/a&gt;不僅移除了這個提醒，還可以&lt;b&gt;提供特殊的增強功能&lt;b&gt;，提供更好的安全性和相容性。</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="172"/>
        <source>Sandboxie-Plus - Support Reminder</source>
        <translation>Sandboxie-Plus - 贊助提醒</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="251"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="258"/>
        <source>Quit</source>
        <translation>退出</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="259"/>
        <source>Continue</source>
        <translation>繼續</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="260"/>
        <source>Get Certificate</source>
        <translation>取得憑證</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="261"/>
        <source>Enter Certificate</source>
        <translation>輸入憑證</translation>
    </message>
</context>
<context>
    <name>CTemplateTypePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="258"/>
        <source>Create new Template</source>
        <translation>建立新的範本</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="267"/>
        <source>Select template type:</source>
        <translation>選擇範本類型:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="271"/>
        <source>%1 template</source>
        <translation>%1 範本</translation>
    </message>
</context>
<context>
    <name>CTemplateWizard</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="37"/>
        <source>Compatibility Template Wizard</source>
        <oldsource>Compatybility Template Wizard</oldsource>
        <translation>相容性範本精靈</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="48"/>
        <source>Custom</source>
        <translation>自訂</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="49"/>
        <source>Web Browser</source>
        <translation>網頁瀏覽器</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="82"/>
        <source>Force %1 to run in this sandbox</source>
        <translation>強制 %1 在此沙箱內執行</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="100"/>
        <source>Allow direct access to the entire %1 profile folder</source>
        <translation>允許直接存取整個 %1 的設定檔資料夾</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="113"/>
        <location filename="Wizards/TemplateWizard.cpp" line="168"/>
        <source>Allow direct access to %1 phishing database</source>
        <translation>允許直接存取 %1 的反網路釣魚資料庫</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="127"/>
        <source>Allow direct access to %1 session management</source>
        <translation>允許直接存取 %1 的工作階段管理</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="136"/>
        <location filename="Wizards/TemplateWizard.cpp" line="199"/>
        <source>Allow direct access to %1 passwords</source>
        <translation>允許直接存取 %1 儲存的密碼</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="146"/>
        <location filename="Wizards/TemplateWizard.cpp" line="208"/>
        <source>Allow direct access to %1 cookies</source>
        <translation>允許直接存取 %1 儲存的 Cookies</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="155"/>
        <location filename="Wizards/TemplateWizard.cpp" line="227"/>
        <source>Allow direct access to %1 bookmark and history database</source>
        <translation>允許直接存取 %1 的書籤和歷史瀏覽資料</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="180"/>
        <source>Allow direct access to %1 sync data</source>
        <translation>允許直接存取 %1 的同步資料</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="190"/>
        <source>Allow direct access to %1 preferences</source>
        <translation>允許直接存取 %1 的偏好設定</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="217"/>
        <source>Allow direct access to %1 bookmarks</source>
        <translation>允許直接存取 %1 的書籤</translation>
    </message>
</context>
<context>
    <name>CTraceModel</name>
    <message>
        <location filename="Models/TraceModel.cpp" line="196"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="175"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="176"/>
        <source>Process %1</source>
        <translation>處理程序 %1</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="179"/>
        <source>Thread %1</source>
        <translation>執行緒 %1</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="333"/>
        <source>Process</source>
        <translation>處理程序</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="335"/>
        <source>Type</source>
        <translation>類型</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="336"/>
        <source>Status</source>
        <translation>狀態</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="337"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
</context>
<context>
    <name>CTraceView</name>
    <message>
        <location filename="Views/TraceView.cpp" line="255"/>
        <source>Show as task tree</source>
        <translation>顯示為工作樹狀目錄</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="259"/>
        <source>Show NT Object Tree</source>
        <translation>顯示 NT 物件樹狀結構</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="266"/>
        <source>PID:</source>
        <translation>處理程序 ID:</translation>
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
        <translation>[所有]</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="273"/>
        <source>TID:</source>
        <translation>執行緒 ID:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="281"/>
        <source>Type:</source>
        <translation>類型:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="292"/>
        <source>Status:</source>
        <translation>狀態:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="295"/>
        <source>Open</source>
        <translation>開放</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="296"/>
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="297"/>
        <source>Trace</source>
        <translation>追蹤</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="298"/>
        <source>Other</source>
        <translation>其他</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="306"/>
        <source>Show All Boxes</source>
        <translation>顯示所有沙箱</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="310"/>
        <source>Show Stack Trace</source>
        <translation>顯示堆疊追蹤</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="315"/>
        <source>Save to file</source>
        <translation>儲存到檔案</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="329"/>
        <source>Cleanup Trace Log</source>
        <translation>清理追蹤日誌</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="694"/>
        <source>Save trace log to file</source>
        <translation>儲存追蹤日誌到檔案</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="700"/>
        <source>Failed to open log file for writing</source>
        <translation>無法開啟日誌檔案進行寫入</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="720"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="624"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="251"/>
        <source>Monitor mode</source>
        <translation>監視模式</translation>
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
        <translation>Sandboxie-Plus - 追蹤監測</translation>
    </message>
</context>
<context>
    <name>CUIPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="302"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; UI</source>
        <translation>設定 &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 使用者介面組態</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="303"/>
        <source>Select the user interface style you prefer.</source>
        <translation>選擇您喜歡的使用者介面風格。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="307"/>
        <source>&amp;Advanced UI for experts</source>
        <translation>適合專家的進階使用者介面(&amp;A)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="312"/>
        <source>&amp;Simple UI for beginners</source>
        <translation>適合新手的簡易使用者介面(&amp;S)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="317"/>
        <source>&amp;Vintage SbieCtrl.exe UI</source>
        <translation>SbieCtrl 經典使用者介面(&amp;V)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="342"/>
        <source>Use Bright Mode</source>
        <translation>使用淺色主題</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="346"/>
        <source>Use Dark Mode</source>
        <translation>使用深色主題</translation>
    </message>
</context>
<context>
    <name>CWFPPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="453"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; network filtering</source>
        <translation>設定 &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 的區域網路過濾組態</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="454"/>
        <source>Sandboxie can use the Windows Filtering Platform (WFP) to restrict network access.</source>
        <translation>Sandboxie 可以使用 Windows 篩選平台 (WFP) 來限制區域網路存取。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="460"/>
        <source>Using WFP allows Sandboxie to reliably enforce IP/Port based rules for network access. Unlike system level application firewalls, Sandboxie can use different rules in each box for the same application. If you already have a good and reliable application firewall and do not need per box rules, you can leave this option unchecked. Without WFP enabled, Sandboxie will still be able to reliably and entirely block processes from accessing the network. However, this can cause the process to crash, as the driver blocks the required network device endpoints. Even with WFP disabled, Sandboxie offers to set IP/Port based rules, however these will be applied in user mode only and not be enforced by the driver. Hence, without WFP enabled, an intentionally malicious process could bypass those rules, but not the entire network block.</source>
        <translation>啟用 WFP 使 Sandboxie 能夠可靠地執行基於 IP/連接埠 的區域網路存取規則。與系統層級的應用程式防火牆不同，Sandboxie 可以針對同一個應用程式在不同的沙箱內設定不同的規則。如果您已有一個更友好、更可靠的應用程式防火牆，並且不需要針對同一應用程式在不同沙箱設定不同的規則，則可以不勾選此選項。如果不啟用 WFP，Sandboxie 仍然能夠有效完全阻止處理程序存取網路。然而，這可能會導致處理程序當機，因為驅動程式會阻止程式存取請求的區域網路裝置端點。即使停用 WFP，Sandboxie 也將提供基於 IP/連接埠 的規則過濾功能，但此時規則只能套用於使用者模式，而無法被驅動程式強制執行。因此，如果不啟用 WFP，某些惡意程式可能可以繞過這些規則，但不能繞過整個區域網路區塊。</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="469"/>
        <source>Enable Windows Filtering Platform (WFP) support</source>
        <translation>啟用 Windows 篩選平台 (WFP) 功能支援</translation>
    </message>
</context>
<context>
    <name>NewBoxWindow</name>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="32"/>
        <source>SandboxiePlus new box</source>
        <translation>SandboxiePlus 新沙箱</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="60"/>
        <source>Box Type Preset:</source>
        <translation>沙箱類型預設設定:</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="91"/>
        <source>A sandbox isolates your host system from processes running within the box, it prevents them from making permanent changes to other programs and data in your computer. The level of isolation impacts your security as well as the compatibility with applications, hence there will be a different level of isolation depending on the selected Box Type. Sandboxie can also protect your personal data from being accessed by processes running under its supervision.</source>
        <translation>沙箱會將您的主機系統與沙箱內執行的處理程序隔離開來，防止它們對電腦中的其他程式和資料進行永久變更。隔離的級別會影響您的安全性以及與應用程式的相容性，因此根據所選的沙箱類型，將有不同的隔離級別。Sandboxie 還可以保護您的個人資料，在其監督下不會被執行的處理程序所存取。</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="101"/>
        <source>Box info</source>
        <translation>沙箱資訊</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="67"/>
        <source>Sandbox Name:</source>
        <translation>沙箱名稱:</translation>
    </message>
</context>
<context>
    <name>OptionsWindow</name>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="32"/>
        <source>SandboxiePlus Options</source>
        <translation>SandboxiePlus 選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="52"/>
        <source>General Options</source>
        <translation>一般選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="62"/>
        <source>Box Options</source>
        <translation>沙箱選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="96"/>
        <source>Sandbox Indicator in title:</source>
        <translation>在標題顯示沙箱標記:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="106"/>
        <source>Sandboxed window border:</source>
        <translation>沙箱化視窗邊框:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="259"/>
        <source>Double click action:</source>
        <translation>按兩下動作:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="359"/>
        <source>Separate user folders</source>
        <translation>分離使用者資料夾</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="396"/>
        <source>Box Structure</source>
        <translation>沙箱結構</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="953"/>
        <source>Security Options</source>
        <translation>安全性選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="963"/>
        <source>Security Hardening</source>
        <translation>安全性強化</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="730"/>
        <location filename="Forms/OptionsWindow.ui" line="766"/>
        <location filename="Forms/OptionsWindow.ui" line="783"/>
        <location filename="Forms/OptionsWindow.ui" line="992"/>
        <location filename="Forms/OptionsWindow.ui" line="1037"/>
        <source>Protect the system from sandboxed processes</source>
        <translation>保護系統免受來自沙箱化處理程序的存取</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="995"/>
        <source>Elevation restrictions</source>
        <translation>權限提升限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1080"/>
        <source>Drop rights from Administrators and Power Users groups</source>
        <translation>廢棄來自管理員和 Power Users 使用者組的許可</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="221"/>
        <source>px Width</source>
        <translation>寬度像素</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1002"/>
        <source>Make applications think they are running elevated (allows to run installers safely)</source>
        <translation>使應用程式認為其已在權限提升狀態下執行 (允許安全地執行安裝程式)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1094"/>
        <source>CAUTION: When running under the built in administrator, processes can not drop administrative privileges.</source>
        <translation>警告: 在內建的管理員帳戶下執行時，無法解除處理程序的管理員權限。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="252"/>
        <source>Appearance</source>
        <translation>外觀</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1016"/>
        <source>(Recommended)</source>
        <translation>(推薦)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="172"/>
        <source>Show this box in the &apos;run in box&apos; selection prompt</source>
        <translation>在右鍵選單選擇「在沙箱中執行」後顯示此對話方塊</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="279"/>
        <source>File Options</source>
        <translation>檔案選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="324"/>
        <source>Auto delete content when last sandboxed process terminates</source>
        <translation>當最後的沙箱化的處理程序終止後自動刪除內容</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="453"/>
        <source>Copy file size limit:</source>
        <translation>複製檔案大小限制:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="410"/>
        <source>Box Delete options</source>
        <translation>沙箱刪除選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="294"/>
        <source>Protect this sandbox from deletion or emptying</source>
        <translation>保護此沙箱以防止被刪除或清空</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="433"/>
        <location filename="Forms/OptionsWindow.ui" line="474"/>
        <source>File Migration</source>
        <translation>檔案遷移</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="301"/>
        <source>Allow elevated sandboxed applications to read the harddrive</source>
        <translation>允許權限提升的沙箱化應用程式存取磁碟</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="287"/>
        <source>Warn when an application opens a harddrive handle</source>
        <translation>當應用程式開啟磁碟機控制代碼時發出警告</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="503"/>
        <source>kilobytes</source>
        <translation>KB</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="596"/>
        <source>Issue message 2102 when a file is too large</source>
        <translation>當檔案過大時提示錯誤代碼 2102</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="460"/>
        <source>Prompt user for large file migration</source>
        <translation>詢問使用者是否遷移大型檔案</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="688"/>
        <source>Allow the print spooler to print to files outside the sandbox</source>
        <translation>允許列印服務在沙箱外列印檔案</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="702"/>
        <source>Remove spooler restriction, printers can be installed outside the sandbox</source>
        <translation>移除列印限制，印表機可安裝至沙箱外</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="641"/>
        <source>Block read access to the clipboard</source>
        <translation>阻止存取剪貼簿</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="709"/>
        <source>Open System Protected Storage</source>
        <translation>開放系統防護儲存空間</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="675"/>
        <source>Block access to the printer spooler</source>
        <translation>阻止存取列印假離線序列</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="733"/>
        <source>Other restrictions</source>
        <translation>其它限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="769"/>
        <source>Printing restrictions</source>
        <translation>列印限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="786"/>
        <source>Network restrictions</source>
        <translation>區域網路限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="695"/>
        <source>Block network files and folders, unless specifically opened.</source>
        <translation>阻止區域網路檔案和資料夾的存取，除非額外開啟。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="796"/>
        <source>Run Menu</source>
        <translation>執行選單</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="925"/>
        <source>You can configure custom entries for the sandbox run menu.</source>
        <translation>您可為「在沙箱中執行」選單設定自訂條目組態。</translation>
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
        <translation>名稱</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="879"/>
        <source>Command Line</source>
        <translation>命令列</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="845"/>
        <source>Add program</source>
        <translation>加入程式</translation>
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
        <translation>移除</translation>
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
        <translation>類型</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1583"/>
        <source>Program Groups</source>
        <translation>程式群組</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1591"/>
        <source>Add Group</source>
        <translation>加入群組</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1623"/>
        <location filename="Forms/OptionsWindow.ui" line="1977"/>
        <location filename="Forms/OptionsWindow.ui" line="2054"/>
        <location filename="Forms/OptionsWindow.ui" line="2132"/>
        <location filename="Forms/OptionsWindow.ui" line="2918"/>
        <source>Add Program</source>
        <translation>加入程式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1734"/>
        <source>Force Folder</source>
        <translation>強制執行資料夾</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2257"/>
        <location filename="Forms/OptionsWindow.ui" line="2357"/>
        <location filename="Forms/OptionsWindow.ui" line="2490"/>
        <source>Path</source>
        <translation>路徑</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1777"/>
        <source>Force Program</source>
        <translation>強制執行程式</translation>
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
        <translation>顯示範本</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1121"/>
        <source>Security note: Elevated applications running under the supervision of Sandboxie, with an admin or system token, have more opportunities to bypass isolation and modify the system outside the sandbox.</source>
        <translation>安全性提示: 在 Sandboxie 監管下執行的程式，若具有提升的管理員或系統權限權杖，將有更多機會繞過隔離，並修改沙箱外部的系統。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1047"/>
        <source>Allow MSIServer to run with a sandboxed system token and apply other exceptions if required</source>
        <translation>允許 MSIServer 在沙箱內使用系統權杖執行，並在必要時給予其他限制方面的豁免</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1104"/>
        <source>Note: Msi Installer Exemptions should not be required, but if you encounter issues installing a msi package which you trust, this option may help the installation complete successfully. You can also try disabling drop admin rights.</source>
        <translation>注意: MSI 安裝程式豁免不是必須的。但是如果您在安裝您信任的 MSI 安裝檔時出現了問題，此選項可能會有助於成功完成安裝。您也可以嘗試關閉「廢棄管理員許可」選項。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="238"/>
        <source>General Configuration</source>
        <translation>一般組態</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="211"/>
        <source>Box Type Preset:</source>
        <translation>沙箱類型預設設定:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="179"/>
        <source>Box info</source>
        <translation>沙箱資訊</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="142"/>
        <source>&lt;b&gt;More Box Types&lt;/b&gt; are exclusively available to &lt;u&gt;project supporters&lt;/u&gt;, the Privacy Enhanced boxes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;protect user data from illicit access&lt;/font&gt;&lt;/b&gt; by the sandboxed programs.&lt;br /&gt;If you are not yet a supporter, then please consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt;, to receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;.&lt;br /&gt;You can test the other box types by creating new sandboxes of those types, however processes in these will be auto terminated after 5 minutes.</source>
        <translation>&lt;b&gt;更多沙箱類型&lt;/b&gt;僅&lt;u&gt;專案贊助者&lt;/u&gt;可用，隱私增強型沙箱&lt;b&gt;&lt;font color=&apos;red&apos;&gt;保護使用者資料免受沙箱化的程式非法存取&lt;/font&gt;&lt;/b&gt;。&lt;br /&gt;如果您還不是贊助者，請考慮&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;贊助此專案&lt;/a&gt;，來取得&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;贊助者憑證&lt;/a&gt;。&lt;br /&gt;當然您也可以新增一個這些類型的沙箱進行測試，不過沙箱中執行的程式將在 5 分鐘之後自動終止。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="89"/>
        <source>Always show this sandbox in the systray list (Pinned)</source>
        <translation>固定此沙箱，以便總是在系統匣清單中顯示</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="621"/>
        <source>Open Windows Credentials Store (user mode)</source>
        <translation>開放 Windows 憑證儲存存取權限 (使用者模式)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="661"/>
        <source>Prevent change to network and firewall parameters (user mode)</source>
        <translation>防止對區域網路及防火牆參數的變更 (使用者模式)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1630"/>
        <source>You can group programs together and give them a group name.  Program groups can be used with some of the settings instead of program names. Groups defined for the box overwrite groups defined in templates.</source>
        <translation>您可將程式分組並且給它們一個群組名稱。程式群組可以代替程式名稱被用於某些設定。在沙箱中定義的程式群組將覆蓋範本中定義的程式群組。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1699"/>
        <source>Programs entered here, or programs started from entered locations, will be put in this sandbox automatically, unless they are explicitly started in another sandbox.</source>
        <translation>此處輸入的程式，或指定位置啟動的程式，將自動加入此沙箱，除非它們被確定已在其他沙箱啟動。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1920"/>
        <source>Stop Behaviour</source>
        <translation>停止行為</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2092"/>
        <source>Start Restrictions</source>
        <translation>啟動限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2100"/>
        <source>Issue message 1308 when a program fails to start</source>
        <translation>當程式啟動失敗時提示錯誤代碼 1308</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2151"/>
        <source>Allow only selected programs to start in this sandbox. *</source>
        <translation>僅允許被選取的程式在此沙箱中啟動。 *</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2158"/>
        <source>Prevent selected programs from starting in this sandbox.</source>
        <translation>防止所選程式在此沙箱中啟動。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2165"/>
        <source>Allow all programs to start in this sandbox.</source>
        <translation>允許所有程式在此沙箱中啟動。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2172"/>
        <source>* Note: Programs installed to this sandbox won&apos;t be able to start at all.</source>
        <translation>* 注意: 安裝至此沙箱內的程式將完全無法啟動。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2891"/>
        <source>Process Restrictions</source>
        <translation>處理程序限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2911"/>
        <source>Issue message 1307 when a program is denied internet access</source>
        <translation>當程式被拒絕存取網路時提示錯誤代碼 1307</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2925"/>
        <source>Prompt user whether to allow an exemption from the blockade.</source>
        <translation>詢問使用者是否允許封鎖豁免。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2939"/>
        <source>Note: Programs installed to this sandbox won&apos;t be able to access the internet at all.</source>
        <translation>注意: 安裝在此沙箱中的程式將完全無法存取網路。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2252"/>
        <location filename="Forms/OptionsWindow.ui" line="2352"/>
        <location filename="Forms/OptionsWindow.ui" line="2485"/>
        <location filename="Forms/OptionsWindow.ui" line="2605"/>
        <location filename="Forms/OptionsWindow.ui" line="2679"/>
        <location filename="Forms/OptionsWindow.ui" line="2968"/>
        <source>Access</source>
        <translation>存取</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="338"/>
        <source>Use volume serial numbers for drives, like: \drive\C~1234-ABCD</source>
        <translation>使用磁碟的磁碟區序號，例如：\drive\C~1234-ABCD</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="382"/>
        <source>The box structure can only be changed when the sandbox is empty</source>
        <translation>只有在沙箱為空時，才能變更沙箱結構</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="352"/>
        <source>Disk/File access</source>
        <translation>「磁碟/檔案」存取權限</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="331"/>
        <source>Virtualization scheme</source>
        <translation>虛擬化方案</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="481"/>
        <source>2113: Content of migrated file was discarded
2114: File was not migrated, write access to file was denied
2115: File was not migrated, file will be opened read only</source>
        <translation>2113：待遷移檔案的內容被遺棄了
2114：檔案沒有被遷移，檔案的寫入存取被拒絕
2115：檔案沒有被遷移，檔案將以唯讀方式開啟</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="486"/>
        <source>Issue message 2113/2114/2115 when a file is not fully migrated</source>
        <translation>當一個檔案沒有被完全遷移時，提示問題代碼：2113/2114/2115</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="510"/>
        <source>Add Pattern</source>
        <translation>加入范式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="517"/>
        <source>Remove Pattern</source>
        <translation>移除范式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="571"/>
        <source>Pattern</source>
        <translation>范式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="579"/>
        <source>Sandboxie does not allow writing to host files, unless permitted by the user. When a sandboxed application attempts to modify a file, the entire file must be copied into the sandbox, for large files this can take a significate amount of time. Sandboxie offers options for handling these cases, which can be configured on this page.</source>
        <translation>Sandboxie 不被允許對主機檔案進行寫入，除非得到使用者的允許。當沙箱化的應用程式試圖修改一個檔案時，整個檔案必須被複製到沙箱中。對於大型檔案來說，這可能需要相當長的時間。Sandboxie 提供了針對這些情況的處理選項，可以在此頁面進行設定。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="589"/>
        <source>Using wildcard patterns file specific behavior can be configured in the list below:</source>
        <translation>使用萬用字元范式，具體的檔案行為可以在下面的清單中進行設定:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="603"/>
        <source>When a file cannot be migrated, open it in read-only mode instead</source>
        <translation>當一個檔案不能被遷移時，嘗試以唯讀模式開啟它</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="874"/>
        <source>Icon</source>
        <translation>圖示</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="899"/>
        <source>Move Up</source>
        <translation>向上移</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="918"/>
        <source>Move Down</source>
        <translation>向下移</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1153"/>
        <source>Security Isolation</source>
        <translation>安全性隔離</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1174"/>
        <source>Various isolation features can break compatibility with some applications. If you are using this sandbox &lt;b&gt;NOT for Security&lt;/b&gt; but for application portability, by changing these options you can restore compatibility by sacrificing some security.</source>
        <translation>注意: 各種隔離功能會破壞與某些應用程式的相容性&lt;br /&gt;如果使用此沙箱&lt;b&gt;不是為了安全性&lt;/b&gt;，而是為了應用程式的可移植性，可以透過變更這些選項，犧牲部分安全性來復原相容性。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1272"/>
        <source>Access Isolation</source>
        <translation>存取隔離</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1299"/>
        <location filename="Forms/OptionsWindow.ui" line="1357"/>
        <source>Image Protection</source>
        <translation>映像保護</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1307"/>
        <source>Issue message 1305 when a program tries to load a sandboxed dll</source>
        <translation>當一個程式試圖載入一個沙箱內部的應用程式擴充 (DLL) 檔案時，提示問題代碼 1305</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1340"/>
        <source>Prevent sandboxes programs installed on host from loading dll&apos;s from the sandbox</source>
        <translation>防止安裝在主機上的沙箱程式從沙箱內部載入應用程式擴充 (DLL) 檔案</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1383"/>
        <source>Sandboxie’s resource access rules often discriminate against program binaries located inside the sandbox. OpenFilePath and OpenKeyPath work only for application binaries located on the host natively. In order to define a rule without this restriction, OpenPipePath or OpenConfPath must be used. Likewise, all Closed(File|Key|Ipc)Path directives which are defined by negation e.g. ‘ClosedFilePath=! iexplore.exe,C:Users*’ will be always closed for binaries located inside a sandbox. Both restriction policies can be disabled on the “Access policies” page.
This is done to prevent rogue processes inside the sandbox from creating a renamed copy of themselves and accessing protected resources. Another exploit vector is the injection of a library into an authorized process to get access to everything it is allowed to access. Using Host Image Protection, this can be prevented by blocking applications (installed on the host) running inside a sandbox from loading libraries from the sandbox itself.</source>
        <translation>Sandboxie 的資源存取規則通常對位於沙箱內的二進位程式具有歧視性。OpenFilePath 和 OpenKeyPath 只對主機上的原生程式 (安裝在主機上的) 有效。為了定義沒有此類限制的規則，則必須使用 OpenPipePath 和 OpenConfPath。同樣的，透過否定來定義所有的 Closed(File|Key|Ipc)Path 指令例如：&apos;ClosedFilePath=! iexplore.exe,C:Users*&apos; 將限制沙箱內的程式存取相應資源。這兩種限制原則都可以透過「存取原則」頁面來停用。
這樣做是為了防止沙箱內的流氓處理程序建立自己的重新命名複本並存取受保護的資源。另一個漏洞載體是將一個動態連結程式庫注入到一個被授權處理程序中，以取得對被授權處理程序所允許存取的一切資源的存取權。使用主機映像保護，可以透過阻止在沙箱內執行的應用程式 (安裝在主機上的) 載入來自沙箱的動態連結程式庫來防止此類現象。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1404"/>
        <source>Advanced Security</source>
        <oldsource>Adcanced Security</oldsource>
        <translation>進階安全性</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1412"/>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation>使用 Sandboxie 限權使用者，而不是匿名權杖 (實驗性)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1429"/>
        <source>Other isolation</source>
        <translation>其他隔離</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1453"/>
        <source>Privilege isolation</source>
        <translation>權限隔離</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1566"/>
        <source>Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.</source>
        <translation>使用自訂 Sandboxie 權杖可以更好地將各個沙箱相互隔離，同時可以實現在工作管理員的使用者欄位中顯示處理程序所屬的沙箱。但是，某些第三方安全性解決方案可能會與自訂權杖產生相容性問題。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1657"/>
        <source>Program Control</source>
        <translation>應用程式控制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1679"/>
        <source>Force Programs</source>
        <translation>強制沙箱程式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1791"/>
        <source>Disable forced Process and Folder for this sandbox</source>
        <translation>停用此沙箱的「強制處理程序/資料夾」規則</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1801"/>
        <source>Breakout Programs</source>
        <translation>分離程式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1870"/>
        <source>Breakout Program</source>
        <translation>分離程式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1896"/>
        <source>Breakout Folder</source>
        <translation>分離資料夾</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1903"/>
        <source>Programs entered here will be allowed to break out of this sandbox when they start. It is also possible to capture them into another sandbox, for example to have your web browser always open in a dedicated box.</source>
        <oldsource>Programs entered here will be allowed to break out of this box when thay start, you can capture them into an other box. For example to have your web browser always open in a dedicated box. This feature requires a valid supporter certificate to be installed.</oldsource>
        <translation>在此處設定的程式，在啟動時將被允許脫離這個沙箱，您可以把它們擷取到另一個沙箱中。例如，讓網頁瀏覽器總是在一個專門的沙箱內開啟。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1930"/>
        <source>Lingering Programs</source>
        <translation>駐留程式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1996"/>
        <source>Lingering programs will be automatically terminated if they are still running after all other processes have been terminated.</source>
        <translation>其他所有程式得到終止後，仍在執行的駐留程式將自動終止。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2009"/>
        <source>Leader Programs</source>
        <translation>引導程式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2075"/>
        <source>If leader processes are defined, all others are treated as lingering processes.</source>
        <translation>如果定義了引導處理程序，其他處理程序將被視作駐留處理程序。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2204"/>
        <source>Files</source>
        <translation>檔案</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2297"/>
        <source>Configure which processes can access Files, Folders and Pipes. 
&apos;Open&apos; access only applies to program binaries located outside the sandbox, you can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behavior in the Policies tab.</source>
        <translation>設定哪些處理程序可以存取檔案、資料夾和管道。
「開放」存取權限只適用於原先已位於沙箱之外的程式二進位檔，您可以使用「完全開放」來對所有程式開放所有權限，或者在「原則」標籤中改變這一行為。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2311"/>
        <source>Registry</source>
        <translation>登錄</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2404"/>
        <source>Configure which processes can access the Registry. 
&apos;Open&apos; access only applies to program binaries located outside the sandbox, you can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behavior in the Policies tab.</source>
        <translation>設定哪些處理程序可以存取檔案、資料夾和管道。
「開放」存取權限只適用於原先已位於沙箱之外的程式二進位檔，您可以使用「完全開放」來對所有程式開放所有權限，或者在「原則」標籤中改變這一行為。</translation>
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
        <translation>設定哪些處理程序可以存取 NT IPC 物件，如 ALPC 連接埠及其他處理程序的記憶體和相關執行狀態環境。
如需指定一個處理程序，使用「$:program.exe」作為路徑值。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2525"/>
        <source>Wnd</source>
        <translation>視窗</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2610"/>
        <source>Wnd Class</source>
        <translation>Wnd 元件</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2625"/>
        <source>Configure which processes can access desktop objects like windows and alike.</source>
        <translation>設定哪些處理程序可以存取桌面物件，如 Windows 或其它類似物件。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2638"/>
        <source>COM</source>
        <translation>COM</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2684"/>
        <source>Class Id</source>
        <translation>類別識別碼</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2731"/>
        <source>Configure which processes can access COM objects.</source>
        <translation>設定哪些處理程序可以存取 COM 物件。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2743"/>
        <source>Don&apos;t use virtualized COM, Open access to hosts COM infrastructure (not recommended)</source>
        <translation>不使用虛擬化 COM，而是開放主機 COM 基礎結構的存取 (不推薦)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2751"/>
        <source>Access Policies</source>
        <translation>存取原則</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2881"/>
        <source>Network Options</source>
        <translation>區域網路選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2978"/>
        <source>Set network/internet access for unlisted processes:</source>
        <translation>為未列出的處理程序設定區域網路/網際網路存取權限:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3025"/>
        <source>Test Rules, Program:</source>
        <translation>測試規則、程式:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3035"/>
        <source>Port:</source>
        <translation>連接埠:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3045"/>
        <source>IP:</source>
        <translation>IP:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3055"/>
        <source>Protocol:</source>
        <translation>協定:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3065"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3081"/>
        <source>Add Rule</source>
        <translation>加入規則</translation>
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
        <translation>程式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="561"/>
        <location filename="Forms/OptionsWindow.ui" line="3097"/>
        <location filename="Forms/OptionsWindow.ui" line="3536"/>
        <location filename="Forms/OptionsWindow.ui" line="3861"/>
        <source>Action</source>
        <translation>動作</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3102"/>
        <source>Port</source>
        <translation>連接埠</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3107"/>
        <source>IP</source>
        <translation>IP</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3112"/>
        <source>Protocol</source>
        <translation>協定</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3134"/>
        <source>CAUTION: Windows Filtering Platform is not enabled with the driver, therefore these rules will be applied only in user mode and can not be enforced!!! This means that malicious applications may bypass them.</source>
        <translation>警告: 未在驅動程式中啟動 Windows 篩選平台，因此以下規則只能在使用者模式下生效，無法被強制執行！！！惡意程式可能會繞過這些規則的限制。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2194"/>
        <source>Resource Access</source>
        <translation>資源存取</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2277"/>
        <source>Add File/Folder</source>
        <translation>加入檔案/資料夾</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2571"/>
        <source>Add Wnd Class</source>
        <translation>加入視窗類別</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2450"/>
        <source>Add IPC Path</source>
        <translation>加入 IPC 路徑</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="971"/>
        <source>Use the original token only for approved NT system calls</source>
        <translation>只在經過批准的 NT 系統呼叫中使用原始權杖</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="978"/>
        <source>Enable all security enhancements (make security hardened box)</source>
        <translation>啟用所有安全性強化 (安全性防護加固型沙箱選項)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1023"/>
        <source>Restrict driver/device access to only approved ones</source>
        <translation>將對「驅動程式/裝置」的存取限制到已批准條目</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1040"/>
        <source>Security enhancements</source>
        <translation>安全性增強措施</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="716"/>
        <source>Issue message 2111 when a process access is denied</source>
        <translation>處理程序被拒絕存取非沙箱處理程序記憶體時提示問題代碼 2111</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1477"/>
        <source>Sandboxie token</source>
        <translation>Sandboxie 權杖</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2384"/>
        <source>Add Reg Key</source>
        <translation>加入登錄機碼</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2711"/>
        <source>Add COM Object</source>
        <translation>加入 COM 物件</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2785"/>
        <source>Apply Close...=!&lt;program&gt;,... rules also to all binaries located in the sandbox.</source>
        <translation>將 Close...=!&lt;program&gt;,... 規則，套用到位於沙箱內的所有相關二進位檔。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3151"/>
        <source>File Recovery</source>
        <translation>檔案復原</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3161"/>
        <source>Quick Recovery</source>
        <translation>快速復原</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3230"/>
        <source>Add Folder</source>
        <translation>加入資料夾</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3240"/>
        <source>Immediate Recovery</source>
        <translation>即時復原</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3270"/>
        <source>Ignore Extension</source>
        <translation>忽略副檔名</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3316"/>
        <source>Ignore Folder</source>
        <translation>忽略資料夾</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3277"/>
        <source>Enable Immediate Recovery prompt to be able to recover files as soon as they are created.</source>
        <translation>啟用快速復原提示，以便快速復原建立的檔案。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3260"/>
        <source>You can exclude folders and file types (or file extensions) from Immediate Recovery.</source>
        <translation>您可以從快速復原中排除特定目錄和檔案類型 (副檔名)。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3193"/>
        <source>When the Quick Recovery function is invoked, the following folders will be checked for sandboxed content. </source>
        <translation>當快速復原功能被執行時，下列資料夾將為沙箱化內容被檢查。 </translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3358"/>
        <source>Advanced Options</source>
        <translation>進階選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3950"/>
        <source>Miscellaneous</source>
        <translation>雜項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2618"/>
        <source>Don&apos;t alter window class names created by sandboxed programs</source>
        <translation>不要改變由沙箱化程式建立的視窗類別名稱</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1505"/>
        <source>Do not start sandboxed services using a system token (recommended)</source>
        <translation>不啟動使用系統權杖的沙箱化服務 (建議)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1252"/>
        <location filename="Forms/OptionsWindow.ui" line="1269"/>
        <location filename="Forms/OptionsWindow.ui" line="1354"/>
        <location filename="Forms/OptionsWindow.ui" line="1426"/>
        <location filename="Forms/OptionsWindow.ui" line="1450"/>
        <location filename="Forms/OptionsWindow.ui" line="1474"/>
        <source>Protect the sandbox integrity itself</source>
        <translation>保護沙箱本身的完整性</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1498"/>
        <source>Drop critical privileges from processes running with a SYSTEM token</source>
        <translation>廢棄以系統權杖執行中的程式的關鍵特權</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1491"/>
        <location filename="Forms/OptionsWindow.ui" line="1539"/>
        <source>(Security Critical)</source>
        <translation>(安全性關鍵)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1460"/>
        <source>Protect sandboxed SYSTEM processes from unprivileged processes</source>
        <translation>保護沙箱中的系統處理程序免受非特權處理程序的影響</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3403"/>
        <source>Force usage of custom dummy Manifest files (legacy behaviour)</source>
        <translation>強制使用自訂虛擬 Manifest 檔案 (遺留行為)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2833"/>
        <source>The rule specificity is a measure to how well a given rule matches a particular path, simply put the specificity is the length of characters from the begin of the path up to and including the last matching non-wildcard substring. A rule which matches only file types like &quot;*.tmp&quot; would have the highest specificity as it would always match the entire file path.
The process match level has a higher priority than the specificity and describes how a rule applies to a given process. Rules applying by process name or group have the strongest match level, followed by the match by negation (i.e. rules applying to all processes but the given one), while the lowest match levels have global matches, i.e. rules that apply to any process.</source>
        <translation>規則的明確性是衡量一個給定規則對特定路徑的相符程度，簡單地說，明確性是指從路徑的開始到最後一個相符的非萬用字元子串之間的字元長度。一個只相符「*.tmp」這樣的檔案類型規則將具有最高的明確性，因為它總是符合整個檔案路徑。
處理程序相符級別的優先順序高於明確性，它描述了一條規則如何適用於一個給定的處理程序。按處理程序名稱或組應用的規則具有最高的相符級別，其次是否定式相符 (例如: 適用於相符除給定處理程序以外的所有處理程序的規則)，而最低的相符級別是全域符合，即適用於任何處理程序的規則。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2792"/>
        <source>Prioritize rules based on their Specificity and Process Match Level</source>
        <translation>基於規則的明確性和處理程序相符級別，對規則進行優先順序排序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2844"/>
        <source>Privacy Mode, block file and registry access to all locations except the generic system ones</source>
        <translation>隱私模式，阻止對通用系統目錄之外的所有檔案位置和登錄的存取</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2826"/>
        <source>Access Mode</source>
        <translation>存取權限模式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2864"/>
        <source>When the Privacy Mode is enabled, sandboxed processes will be only able to read C:\Windows\*, C:\Program Files\*, and parts of the HKLM registry, all other locations will need explicit access to be readable and/or writable. In this mode, Rule Specificity is always enabled.</source>
        <translation>當啟用隱私模式時，沙箱化處理程序將只能讀取 C:\Windows\*、C:\Program Files\* 和登錄 HKLM 的部分內容，除此之外的所有其它位置都需要明確的存取授權才能被讀取或寫入。在此模式下，明確性規則將總是被啟用。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2778"/>
        <source>Rule Policies</source>
        <translation>規則原則</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2799"/>
        <source>Apply File and Key Open directives only to binaries located outside the sandbox.</source>
        <translation>套用檔案和金鑰開放指令權限 (僅對位於沙箱之外的二進位檔)。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1436"/>
        <source>Start the sandboxed RpcSs as a SYSTEM process (not recommended)</source>
        <translation>以系統處理程序啟動沙箱化服務 RpcSs (不推薦)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1525"/>
        <source>Allow only privileged processes to access the Service Control Manager</source>
        <translation>僅允許已有特權的處理程序存取服務控制管理員</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3375"/>
        <location filename="Forms/OptionsWindow.ui" line="3424"/>
        <source>Compatibility</source>
        <translation>相容性</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1559"/>
        <source>Add sandboxed processes to job objects (recommended)</source>
        <translation>加入沙箱化處理程序至作業物件 (推薦)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3410"/>
        <source>Emulate sandboxed window station for all processes</source>
        <translation>為所有處理程序模擬沙箱化視窗站台</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1228"/>
        <source>Security Isolation through the usage of a heavily restricted process token is Sandboxie&apos;s primary means of enforcing sandbox restrictions, when this is disabled the box is operated in the application compartment mode, i.e. it’s no longer providing reliable security, just simple application compartmentalization.</source>
        <translation>透過嚴格限制處理程序權杖的使用來進行安全性隔離是 Sandboxie 執行沙箱化限制的主要手段，當它被停用時，沙箱將在應用程式區間模式下執行，此時將不再提供可靠的安全性限制，只是簡單進行應用程式隔離。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1279"/>
        <source>Allow sandboxed programs to manage Hardware/Devices</source>
        <translation>允許沙箱內程式管理硬體/裝置</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1197"/>
        <source>Disable Security Isolation (experimental)</source>
        <translation>停用安全性隔離 (實驗性)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1204"/>
        <source>Open access to Windows Security Account Manager</source>
        <translation>開放 Windows 安全性帳戶管理員 (SAM) 的存取權限</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1238"/>
        <source>Open access to Windows Local Security Authority</source>
        <translation>開放 Windows 本地安全性認證 (LSA) 的存取權限</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="668"/>
        <source>Allow to read memory of unsandboxed processes (not recommended)</source>
        <translation>允許讀取非沙箱處理程序的記憶體 (不推薦)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3451"/>
        <source>Disable the use of RpcMgmtSetComTimeout by default (this may resolve compatibility issues)</source>
        <translation>預設情況下停用 RpcMgmtSetComTimeout (這可能會解決相容性問題)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1255"/>
        <source>Security Isolation &amp; Filtering</source>
        <translation>安全性隔離/篩選</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1211"/>
        <source>Disable Security Filtering (not recommended)</source>
        <translation>停用安全性篩選 (不推薦)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1218"/>
        <source>Security Filtering used by Sandboxie to enforce filesystem and registry access restrictions, as well as to restrict process access.</source>
        <translation>安全性篩選被 Sandboxie 用來強制執行檔案系統和登錄存取限制，以及限制處理程序存取。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1286"/>
        <source>The below options can be used safely when you don&apos;t grant admin rights.</source>
        <translation>以下選項可以在您未授予管理員許可時安全的使用。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3468"/>
        <source>Triggers</source>
        <translation>觸發器</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3531"/>
        <source>Event</source>
        <translation>事件</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3501"/>
        <location filename="Forms/OptionsWindow.ui" line="3520"/>
        <location filename="Forms/OptionsWindow.ui" line="3658"/>
        <source>Run Command</source>
        <translation>執行命令</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3690"/>
        <source>Start Service</source>
        <translation>啟動服務</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3665"/>
        <source>These events are executed each time a box is started</source>
        <translation>這些事件當沙箱每次啟動時都會被執行</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3668"/>
        <source>On Box Start</source>
        <translation>沙箱啟動階段</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3549"/>
        <location filename="Forms/OptionsWindow.ui" line="3697"/>
        <source>These commands are run UNBOXED just before the box content is deleted</source>
        <translation>這些命令將在刪除沙箱的內容之前，以非沙箱化的方式被執行</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3431"/>
        <source>Allow use of nested job objects (works on Windows 8 and later)</source>
        <translation>允許使用嵌套作業物件 (job object) (適用於 Windows 8 及更高版本)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3623"/>
        <source>These commands are executed only when a box is initialized. To make them run again, the box content must be deleted.</source>
        <translation>這些命令只在沙箱被初始化時執行。要使它們再次執行，必須刪除沙箱內容。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3626"/>
        <source>On Box Init</source>
        <translation>沙箱初始化階段</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3636"/>
        <source>Here you can specify actions to be executed automatically on various box events.</source>
        <translation>在這裡，您可以設定各種沙箱事件中自動執行特定的動作。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3718"/>
        <source>Hide Processes</source>
        <translation>隱藏處理程序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3767"/>
        <location filename="Forms/OptionsWindow.ui" line="3825"/>
        <source>Add Process</source>
        <translation>加入處理程序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3743"/>
        <source>Hide host processes from processes running in the sandbox.</source>
        <translation>面向沙箱內執行的處理程序隱藏的主機處理程序。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="613"/>
        <source>Restrictions</source>
        <translation>限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3337"/>
        <source>Various Options</source>
        <translation>多種選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3396"/>
        <source>Apply ElevateCreateProcess Workaround (legacy behaviour)</source>
        <translation>套用 ElevateCreateProcess 因應措施 (遺留行為)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3458"/>
        <source>Use desktop object workaround for all processes</source>
        <translation>對所有處理程序使用桌面物件因應措施</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3498"/>
        <source>This command will be run before the box content will be deleted</source>
        <translation>該命令將在刪除沙箱內容之前執行</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3552"/>
        <source>On File Recovery</source>
        <translation>檔案復原階段</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3581"/>
        <source>This command will be run before a file is being recovered and the file path will be passed as the first argument. If this command returns anything other than 0, the recovery will be blocked</source>
        <oldsource>This command will be run before a file is being recoverd and the file path will be passed as the first argument, if this command return something other than 0 the recovery will be blocked</oldsource>
        <translation>該命令將在檔案復原前執行，檔案路徑將作為第一個參數被傳遞，如果該命令的返回值不是 0，則復原將被阻止</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3584"/>
        <source>Run File Checker</source>
        <translation>執行檔案檢查程式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3700"/>
        <source>On Delete Content</source>
        <translation>內容刪除階段</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3774"/>
        <source>Don&apos;t allow sandboxed processes to see processes running in other boxes</source>
        <translation>不允許沙箱內的處理程序檢視其他沙箱內執行的處理程序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3781"/>
        <source>Protect processes in this box from being accessed by specified unsandboxed host processes.</source>
        <translation>保護此沙箱內的處理程序不被指定的沙箱外主機處理程序存取。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3792"/>
        <location filename="Forms/OptionsWindow.ui" line="3856"/>
        <source>Process</source>
        <translation>處理程序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3832"/>
        <source>Block also read access to processes in this sandbox</source>
        <translation>阻止對位於該沙箱中的處理程序的讀取</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3882"/>
        <source>Users</source>
        <translation>使用者</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3900"/>
        <source>Restrict Resource Access monitor to administrators only</source>
        <translation>僅允許管理員存取資源存取監控</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3907"/>
        <source>Add User</source>
        <translation>加入使用者</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3937"/>
        <source>Add user accounts and user groups to the list below to limit use of the sandbox to only those accounts.  If the list is empty, the sandbox can be used by all user accounts.

Note:  Forced Programs and Force Folders settings for a sandbox do not apply to user accounts which cannot use the sandbox.</source>
        <translation>加入使用者帳戶和使用者群組到下方清單以限制僅允許這些帳戶可以使用沙箱。如果清單內容為空，所有帳戶均可使用沙箱。

注意: 沙箱的強制沙箱程式及資料夾設定不適用於不能使用沙箱的帳戶。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3982"/>
        <source>Add Option</source>
        <translation>加入選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3996"/>
        <source>Here you can configure advanced per process options to improve compatibility and/or customize sandboxing behavior.</source>
        <oldsource>Here you can configure advanced per process options to improve compatibility and/or customize sand boxing behavior.</oldsource>
        <translation>在此處可以設定各個處理程序的進階選項，以提高相容性或自訂沙箱的某些行為。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4007"/>
        <source>Option</source>
        <translation>選項</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4048"/>
        <source>Tracing</source>
        <translation>追蹤</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4056"/>
        <source>API call trace (requires LogAPI to be installed in the Sbie directory)</source>
        <translation>API 呼叫追蹤 (需要在沙箱資料夾中安裝 LogAPI)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4076"/>
        <source>Pipe Trace</source>
        <translation>Pipe 追蹤</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4096"/>
        <source>Log all SetError&apos;s to Trace log (creates a lot of output)</source>
        <translation>記錄所有 SetError 至追蹤日誌 (產生大量輸出)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4116"/>
        <source>Log Debug Output to the Trace Log</source>
        <translation>紀錄偵錯輸出至追蹤日誌</translation>
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
        <translation>將驅動程式看到的所有存取事件記錄到資源存取日誌。

這些選項將事件遮罩設定為 &quot;*&quot; - 所有存取事件
您可透過 ini 來詳細自訂日誌行為
&quot;A&quot; - 允許的存取
&quot;D&quot; - 拒絕的存取
&quot;I&quot; - 忽略存取請求
來代替 &quot;*&quot;。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4143"/>
        <source>Ntdll syscall Trace (creates a lot of output)</source>
        <translation>Ntdll 系統呼叫追蹤 (將產生大量輸出)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4150"/>
        <source>File Trace</source>
        <translation>檔案追蹤</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4157"/>
        <source>Disable Resource Access Monitor</source>
        <translation>停用資源存取監控</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4164"/>
        <source>IPC Trace</source>
        <translation>IPC 追蹤</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4171"/>
        <source>GUI Trace</source>
        <translation>GUI 追蹤</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4185"/>
        <source>Resource Access Monitor</source>
        <translation>資源存取監控</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4212"/>
        <source>Access Tracing</source>
        <translation>存取追蹤</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4219"/>
        <source>COM Class Trace</source>
        <translation>COM 類別追蹤</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4226"/>
        <source>Key Trace</source>
        <translation>機碼追蹤</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4626"/>
        <source>To compensate for the lost protection, please consult the Drop Rights settings page in the Restrictions settings group.</source>
        <translation>為了彌補失去的保護，請參考「限制」設定組中的「廢棄許可」部分。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2993"/>
        <location filename="Forms/OptionsWindow.ui" line="4233"/>
        <source>Network Firewall</source>
        <translation>區域網路防火牆</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4250"/>
        <source>Debug</source>
        <translation>偵錯</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4306"/>
        <source>WARNING, these options can disable core security guarantees and break sandbox security!!!</source>
        <translation>警告，這些選項可使核心安全性保障失效並且破壞沙箱安全性！</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4316"/>
        <source>These options are intended for debugging compatibility issues, please do not use them in production use. </source>
        <translation>這些選項是為偵錯相容性問題設計的，請勿用於生產力用途。 </translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4331"/>
        <source>App Templates</source>
        <translation>軟體範本</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4416"/>
        <source>Filter Categories</source>
        <translation>篩選類別</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4387"/>
        <source>Text Filter</source>
        <translation>篩選文字</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4409"/>
        <source>Add Template</source>
        <translation>加入範本</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4364"/>
        <source>This list contains a large amount of sandbox compatibility enhancing templates</source>
        <translation>此清單含有大量的相容性增強範本</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4433"/>
        <source>Category</source>
        <translation>類別</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4468"/>
        <source>Template Folders</source>
        <translation>範本資料夾</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4488"/>
        <source>Configure the folder locations used by your other applications.

Please note that this values are currently user specific and saved globally for all boxes.</source>
        <translation>設定您的其他應用程式所使用的資料夾位置。

請注意，這些值為目前使用者針對所有沙箱儲存。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4017"/>
        <location filename="Forms/OptionsWindow.ui" line="4519"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4341"/>
        <source>Templates</source>
        <translation>範本</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4530"/>
        <source>Accessibility</source>
        <translation>協助工具</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4550"/>
        <source>Screen Readers: JAWS, NVDA, Window-Eyes, System Access</source>
        <translation>螢幕閱讀器: JAWS、NVDA、Window-Eyes、系統協助工具</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4590"/>
        <source>The following settings enable the use of Sandboxie in combination with accessibility software.  Please note that some measure of Sandboxie protection is necessarily lost when these settings are in effect.</source>
        <translation>以下設定允許 Sandboxie 與協助工具軟體結合。請注意當這些設定生效時，必然會失去部分 Sandboxie 保護措施。</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4643"/>
        <source>Edit ini Section</source>
        <translation>編輯 ini 區段</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4649"/>
        <source>Edit ini</source>
        <translation>編輯 ini</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4662"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4685"/>
        <source>Save</source>
        <translation>儲存</translation>
    </message>
</context>
<context>
    <name>PopUpWindow</name>
    <message>
        <location filename="Forms/PopUpWindow.ui" line="32"/>
        <source>SandboxiePlus Notifications</source>
        <translation>SandboxiePlus 通知</translation>
    </message>
</context>
<context>
    <name>ProgramsDelegate</name>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="55"/>
        <source>Group: %1</source>
        <translation>群組: %1</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="Views/SbieView.cpp" line="1497"/>
        <source>Drive %1</source>
        <translation>磁碟 %1</translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="SandMan.cpp" line="3604"/>
        <source>OK</source>
        <translation>確定</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3605"/>
        <source>Apply</source>
        <translation>套用</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3606"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3607"/>
        <source>&amp;Yes</source>
        <translation>是(&amp;Y)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3608"/>
        <source>&amp;No</source>
        <translation>否(&amp;N)</translation>
    </message>
</context>
<context>
    <name>RecoveryWindow</name>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="32"/>
        <source>SandboxiePlus - Recovery</source>
        <translation>SandboxiePlus - 復原</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="164"/>
        <source>Close</source>
        <translation>關閉</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="68"/>
        <source>Recover target:</source>
        <translation>復原目標位置:</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="42"/>
        <source>Add Folder</source>
        <translation>加入資料夾</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="61"/>
        <source>Delete Content</source>
        <translation>刪除內容</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="110"/>
        <source>Recover</source>
        <translation>復原</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="78"/>
        <source>Refresh</source>
        <translation>重新整理</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="142"/>
        <source>Delete</source>
        <translation>刪除</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="171"/>
        <source>Show All Files</source>
        <translation>顯示所有檔案</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="184"/>
        <source>TextLabel</source>
        <translation>文字標籤</translation>
    </message>
</context>
<context>
    <name>SelectBoxWindow</name>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="32"/>
        <source>SandboxiePlus select box</source>
        <translation>SandboxiePlus 選取沙箱</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="87"/>
        <source>Select the sandbox in which to start the program, installer or document.</source>
        <translation>選擇要在沙箱啟動的程式、安裝程式或文件。</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="97"/>
        <source>Run in a new Sandbox</source>
        <translation>在新沙箱中執行</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="65"/>
        <source>Sandbox</source>
        <translation>沙箱</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="80"/>
        <source>Run As UAC Administrator</source>
        <translation>以系統管理員權限執行</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="47"/>
        <source>Run Sandboxed</source>
        <translation>在沙箱中執行</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="73"/>
        <source>Run Outside the Sandbox</source>
        <translation>在沙箱外執行</translation>
    </message>
</context>
<context>
    <name>SettingsWindow</name>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="32"/>
        <source>SandboxiePlus Settings</source>
        <translation>SandboxiePlus 設定</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="55"/>
        <source>General Config</source>
        <translation>一般組態</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="80"/>
        <source>Show file recovery window when emptying sandboxes</source>
        <oldsource>Show first recovery window when emptying sandboxes</oldsource>
        <translation>在清空沙箱時顯示復原視窗</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="195"/>
        <source>Open urls from this ui sandboxed</source>
        <translation>將此使用者介面上的連結在沙箱中開啟</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="492"/>
        <source>Systray options</source>
        <translation>系統匣選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="185"/>
        <source>UI Language:</source>
        <translation>使用者介面語言:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="394"/>
        <source>Shell Integration</source>
        <translation>系統殼層整合</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="513"/>
        <source>Run Sandboxed - Actions</source>
        <translation>在沙箱中執行 - 選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="577"/>
        <source>Start Sandbox Manager</source>
        <translation>沙箱管理員啟動選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="633"/>
        <source>Start UI when a sandboxed process is started</source>
        <translation>當有沙箱化處理程序啟動時啟動使用者介面</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="626"/>
        <source>Start UI with Windows</source>
        <translation>Windows 啟動時開啟使用者介面</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="534"/>
        <source>Add &apos;Run Sandboxed&apos; to the explorer context menu</source>
        <translation>在檔案總管右鍵新增「在沙箱中執行」</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="151"/>
        <source>Run box operations asynchronously whenever possible (like content deletion)</source>
        <translation>盡可能以異步方式執行沙箱的各類操作 (如內容刪除)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="173"/>
        <source>Hotkey for terminating all boxed processes:</source>
        <translation>用於終止所有沙箱處理程式的快速鍵:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="412"/>
        <source>Show boxes in tray list:</source>
        <translation>顯示沙箱清單:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="499"/>
        <source>Always use DefaultBox</source>
        <translation>總是使用 DefaultBox</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="527"/>
        <source>Add &apos;Run Un-Sandboxed&apos; to the context menu</source>
        <translation>加入「在沙箱外執行」到右鍵選單</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="442"/>
        <source>Show a tray notification when automatic box operations are started</source>
        <translation>當沙箱自動化作業事件開始執行時，跳出系統匣通知</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="838"/>
        <source>* a partially checked checkbox will leave the behavior to be determined by the view mode.</source>
        <translation>* 部分核取的項目核取方塊之行為取決於其檢視模式。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1337"/>
        <source>Advanced Config</source>
        <translation>進階組態</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1371"/>
        <source>Activate Kernel Mode Object Filtering</source>
        <translation>啟動核心模式物件篩選</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1531"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/filerootpath&quot;&gt;file system root&lt;/a&gt;: </source>
        <translation>沙箱 &lt;a href=&quot;sbie://docs/filerootpath&quot;&gt;檔案系統根目錄&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1616"/>
        <source>Clear password when main window becomes hidden</source>
        <translation>主視窗隱藏時清除密碼</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1488"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/ipcrootpath&quot;&gt;ipc root&lt;/a&gt;: </source>
        <translation>沙箱 &lt;a href=&quot;sbie://docs/ipcrootpath&quot;&gt;IPC 根目錄&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1411"/>
        <source>Sandbox default</source>
        <translation>沙箱預設</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1602"/>
        <source>Config protection</source>
        <translation>組態保護</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1481"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="94"/>
        <source>SandMan Options</source>
        <translation>SandMan 選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="208"/>
        <source>Notifications</source>
        <translation>通知</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="240"/>
        <source>Add Entry</source>
        <translation>加入條目</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="357"/>
        <source>Show file migration progress when copying large files into a sandbox</source>
        <translation>將大型檔案複製到沙箱內部時顯示檔案遷移進度</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="248"/>
        <source>Message ID</source>
        <translation>訊息 ID</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="253"/>
        <source>Message Text (optional)</source>
        <translation>訊息文字 (可選)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="268"/>
        <source>SBIE Messages</source>
        <translation>SBIE 訊息</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="287"/>
        <source>Delete Entry</source>
        <translation>刪除條目</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="301"/>
        <source>Notification Options</source>
        <translation>通知選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="327"/>
        <source>Sandboxie may be issue &lt;a href=&quot;sbie://docs/sbiemessages&quot;&gt;SBIE Messages&lt;/a&gt; to the Message Log and shown them as Popups. Some messages are informational and notify of a common, or in some cases special, event that has occurred, other messages indicate an error condition.&lt;br /&gt;You can hide selected SBIE messages from being popped up, using the below list:</source>
        <oldsource>Sandboxie may be issue &lt;a href= &quot;sbie://docs/ sbiemessages&quot;&gt;SBIE Messages&lt;/a&gt; to the Message Log and shown them as Popups. Some messages are informational and notify of a common, or in some cases special, event that has occurred, other messages indicate an error condition.&lt;br /&gt;You can hide selected SBIE messages from being popped up, using the below list:</oldsource>
        <translation>Sandboxie 可能會將 &lt;a href=&quot;sbie://docs/sbiemessages&quot;&gt;SBIE 訊息&lt;/a&gt;記錄到訊息日誌中，並以跳出視窗的形式通知。有些訊息僅僅是資訊性的，通知一個普通的或某些特殊的事件發生，其它訊息表明一個錯誤狀況。&lt;br /&gt;您可以使用此清單來隱藏所設定的「SBIE 訊息」，使其不會跳出:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="340"/>
        <source>Disable SBIE messages popups (they will still be logged to the Messages tab)</source>
        <oldsource>Disable SBIE messages popups (SBIE will still be logged to the log tab)</oldsource>
        <translation>停用 SBIE 訊息通知 (SBIE 仍然會被記錄到訊息標籤中)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="404"/>
        <source>Windows Shell</source>
        <translation>Windows 殼層</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="670"/>
        <source>Icon</source>
        <translation>圖示</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="727"/>
        <source>Move Up</source>
        <translation>向上移</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="746"/>
        <source>Move Down</source>
        <translation>向下移</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="949"/>
        <source>Show overlay icons for boxes and processes</source>
        <translation>為沙箱與處理程序顯示覆蓋圖示</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="959"/>
        <source>Display Options</source>
        <translation>顯示選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1083"/>
        <source>Graphic Options</source>
        <translation>圖形選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1228"/>
        <source>Supporters of the Sandboxie-Plus project can receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;. It&apos;s like a license key but for awesome people using open source software. :-)</source>
        <translation>Sandboxie-Plus 專案的贊助者將收到&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;贊助者憑證&lt;/a&gt;。這類似於授權碼，但是是為擁抱開放原始碼軟體的優秀人士準備的。:-)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1321"/>
        <source>Keeping Sandboxie up to date with the rolling releases of Windows and compatible with all web browsers is a never-ending endeavor. You can support the development by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;directly contributing to the project&lt;/a&gt;, showing your support by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;purchasing a supporter certificate&lt;/a&gt;, becoming a patron by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;subscribing on Patreon&lt;/a&gt;, or through a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;PayPal donation&lt;/a&gt;.&lt;br /&gt;Your support plays a vital role in the advancement and maintenance of Sandboxie.</source>
        <translation>使 Sandboxie 與 Windows 的持續性更新相同步，並和主流網頁瀏覽器保持相容性，是一項永無止境的努力。您可以透過&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;直接參與專案&lt;/a&gt;來支援開發、透過&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;購買贊助者憑證&lt;/a&gt;來表達您的支持、透過&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;在 Patreon 上訂閱&lt;/a&gt;成為贊助者、或者透過 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;PayPal 捐款&lt;/a&gt;。&lt;br /&gt;您的支持對 Sandboxie 的進步和維護至關重要。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1391"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/keyrootpath&quot;&gt;registry root&lt;/a&gt;: </source>
        <translation>沙箱 &lt;a href=&quot;sbie://docs/keyrootpath&quot;&gt;登錄根目錄&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1451"/>
        <source>Sandboxing features</source>
        <translation>沙箱功能</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1560"/>
        <source>Sandboxie.ini Presets</source>
        <translation>Sandboxie.ini 預設選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1568"/>
        <source>Change Password</source>
        <translation>變更密碼</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1575"/>
        <source>Password must be entered in order to make changes</source>
        <translation>必須輸入密碼以進行變更</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1623"/>
        <source>Only Administrator user accounts can make changes</source>
        <translation>僅限管理員帳戶進行變更</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1630"/>
        <source>Watch Sandboxie.ini for changes</source>
        <translation>追蹤 Sandboxie.ini 變更</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1748"/>
        <source>App Templates</source>
        <translation>軟體範本</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1758"/>
        <source>App Compatibility</source>
        <translation>軟體相容性</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1609"/>
        <source>Only Administrator user accounts can use Pause Forcing Programs command</source>
        <oldsource>Only Administrator user accounts can use Pause Forced Programs Rules command</oldsource>
        <translation>僅管理員帳戶可使用「暫停強制沙箱程式」命令</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1501"/>
        <source>Portable root folder</source>
        <translation>便攜化根目錄</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="350"/>
        <source>Show recoverable files as notifications</source>
        <translation>將可復原的檔案以通知形式顯示</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="65"/>
        <source>General Options</source>
        <translation>一般選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="600"/>
        <source>Show Icon in Systray:</source>
        <translation>在系統匣中顯示圖示:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1461"/>
        <source>Use Windows Filtering Platform to restrict network access</source>
        <translation>使用 Windows 篩選平台 (WFP) 限制區域網路存取</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1468"/>
        <source>Hook selected Win32k system calls to enable GPU acceleration (experimental)</source>
        <translation>為選取的 Win32k 系統呼叫進行勾點以啟用 GPU 加速 (實驗性)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="73"/>
        <source>Count and display the disk space occupied by each sandbox</source>
        <oldsource>Count and display the disk space ocupied by each sandbox</oldsource>
        <translation>統計並顯示每個沙箱的磁碟空間佔用情況</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="475"/>
        <source>Use Compact Box List</source>
        <translation>使用緊湊的沙箱清單</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="800"/>
        <source>Interface Config</source>
        <translation>介面組態</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="925"/>
        <source>Make Box Icons match the Border Color</source>
        <translation>保持沙箱內的圖示與邊框顏色一致</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="915"/>
        <source>Use a Page Tree in the Box Options instead of Nested Tabs *</source>
        <translation>在沙箱選項中使用樹狀頁面，而不是巢式標籤 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="908"/>
        <source>Interface Options</source>
        <translation>介面選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="828"/>
        <source>Use large icons in box list *</source>
        <translation>在沙箱清單中使用大圖示 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="970"/>
        <source>High DPI Scaling</source>
        <translation>高 DPI 縮放</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="891"/>
        <source>Don&apos;t show icons in menus *</source>
        <translation>不要在選單中顯示圖示 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="932"/>
        <source>Use Dark Theme</source>
        <translation>使用深色主題</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="980"/>
        <source>Font Scaling</source>
        <translation>字型縮放</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1042"/>
        <source>(Restart required)</source>
        <translation>(需要重新啟動)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="144"/>
        <source>Show the Recovery Window as Always on Top</source>
        <translation>最上層顯示復原檔案視窗</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="868"/>
        <source>Show &quot;Pizza&quot; Background in box list *</source>
        <oldsource>Show &quot;Pizza&quot; Background in box list*</oldsource>
        <translation>在沙箱清單中顯示「披薩」背景 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1056"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="942"/>
        <source>Alternate row background in lists</source>
        <translation>在沙箱清單中使用替代背景</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="818"/>
        <source>Use Fusion Theme</source>
        <translation>使用 Fusion 風格主題</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1521"/>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation>使用 Sandboxie 登入程序替代匿名權杖 (實驗性)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="665"/>
        <location filename="Forms/SettingsWindow.ui" line="1676"/>
        <location filename="Forms/SettingsWindow.ui" line="1786"/>
        <location filename="Forms/SettingsWindow.ui" line="1919"/>
        <source>Name</source>
        <translation>名稱</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1681"/>
        <source>Path</source>
        <translation>路徑</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1703"/>
        <source>Remove Program</source>
        <translation>移除程式</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1689"/>
        <source>Add Program</source>
        <translation>加入程式</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1724"/>
        <source>When any of the following programs is launched outside any sandbox, Sandboxie will issue message SBIE1301.</source>
        <translation>當下列程式在任意沙箱之外啟動時，Sandboxie 將提示錯誤代碼 SBIE1301。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1734"/>
        <source>Add Folder</source>
        <translation>加入資料夾</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1717"/>
        <source>Prevent the listed programs from starting on this system</source>
        <translation>阻止下列程式在此系統中啟動</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1696"/>
        <source>Issue message 1308 when a program fails to start</source>
        <translation>當程式啟動失敗時提示錯誤代碼 1308</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="137"/>
        <source>Recovery Options</source>
        <translation>復原選項</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="435"/>
        <source>Start Menu Integration</source>
        <translation>開始選單整合</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="520"/>
        <source>Scan shell folders and offer links in run menu</source>
        <translation>掃描系統 Shell 資料夾並在執行選單中整合捷徑</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="541"/>
        <source>Integrate with Host Start Menu</source>
        <translation>與主機的開始功能表整合</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="858"/>
        <source>Use new config dialog layout *</source>
        <translation>使用新的組態對話框佈局 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1644"/>
        <source>Program Control</source>
        <translation>應用程式控制</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1654"/>
        <source>Program Alerts</source>
        <translation>程式警報</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1710"/>
        <source>Issue message 1301 when forced processes has been disabled</source>
        <translation>當強制沙箱處理程序被停用時，提示問題代碼 1301</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1347"/>
        <source>Sandboxie Config</source>
        <oldsource>Config Protection</oldsource>
        <translation>Sandboxie 組態</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="364"/>
        <source>This option also enables asynchronous operation when needed and suspends updates.</source>
        <translation>在暫緩更新或其它需要的情況使用異步操作。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="367"/>
        <source>Suppress pop-up notifications when in game / presentation mode</source>
        <translation>在「遊戲 / 簡報」模式下，停用跳出通知</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="810"/>
        <source>User Interface</source>
        <translation>使用者介面</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="643"/>
        <source>Run Menu</source>
        <translation>執行選單</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="695"/>
        <source>Add program</source>
        <translation>加入程式</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="753"/>
        <source>You can configure custom entries for all sandboxes run menus.</source>
        <translation>您可以為所有「在沙箱內執行」選單設定自訂條目組態。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="775"/>
        <location filename="Forms/SettingsWindow.ui" line="1866"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="675"/>
        <source>Command Line</source>
        <translation>命令列</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1097"/>
        <source>Support &amp;&amp; Updates</source>
        <translation>支援 &amp;&amp; 更新</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1418"/>
        <source>Default sandbox:</source>
        <translation>預設沙箱:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1778"/>
        <source>In the future, don&apos;t check software compatibility</source>
        <translation>以後不再檢查軟體相容性</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1794"/>
        <source>Enable</source>
        <translation>啟用</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1814"/>
        <source>Disable</source>
        <translation>停用</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1821"/>
        <source>Sandboxie has detected the following software applications in your system. Click OK to apply configuration settings, which will improve compatibility with these applications. These configuration settings will have effect in all existing sandboxes and in any new sandboxes.</source>
        <translation>Sandboxie 偵測到您的系統中安裝了以下軟體。按下「確定」套用設定，將改進與這些軟體的相容性。這些設定作用於所有沙箱，包括現存的和未來新增的沙箱。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1834"/>
        <source>Local Templates</source>
        <translation>本地範本</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1885"/>
        <source>Add Template</source>
        <translation>加入範本</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1905"/>
        <source>Text Filter</source>
        <translation>篩選文字</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1927"/>
        <source>This list contains user created custom templates for sandbox options</source>
        <translation>該清單包含使用者為沙箱選項建立的自訂範本</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1944"/>
        <source>Edit ini Section</source>
        <translation>編輯 ini 區段</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1953"/>
        <source>Save</source>
        <translation>儲存</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1960"/>
        <source>Edit ini</source>
        <translation>編輯 ini</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1986"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1147"/>
        <source>Version Updates</source>
        <translation>目前版本的更新</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1208"/>
        <source>New full versions from the selected release channel.</source>
        <translation>新的來自所選取發佈通道的完整版本。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1144"/>
        <source>Hotpatches for the installed version, updates to the Templates.ini and translations.</source>
        <translation>更新已安裝版本的 Templates.ini 範本和翻譯的修補程式。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1161"/>
        <source>This supporter certificate has expired, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>此贊助者憑證已逾期，請&lt;a href=&quot;sbie://update/cert&quot;&gt;取得新憑證&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1198"/>
        <source>The preview channel contains the latest GitHub pre-releases.</source>
        <translation>預覽通道包含最新的 GitHub 預先發佈版本。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1211"/>
        <source>New Versions</source>
        <translation>新版本</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1218"/>
        <source>The stable channel contains the latest stable GitHub releases.</source>
        <translation>穩定通道包含最新的 GitHub 穩定版本。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1221"/>
        <source>Search in the Stable channel</source>
        <translation>在穩定通道中搜尋</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1201"/>
        <source>Search in the Preview channel</source>
        <translation>在預覽通道中搜尋</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1171"/>
        <source>In the future, don&apos;t notify about certificate expiration</source>
        <translation>不再通知憑證逾期的情況</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1184"/>
        <source>Enter the support certificate here</source>
        <translation>在此輸入贊助者憑證</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1268"/>
        <source>Support Settings</source>
        <translation>贊助設定</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1241"/>
        <source>Check periodically for updates of Sandboxie-Plus</source>
        <translation>定期檢查 Sandboxie-Plus 更新</translation>
    </message>
</context>
<context>
    <name>SnapshotsWindow</name>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="32"/>
        <source>SandboxiePlus - Snapshots</source>
        <translation>SandboxiePlus - 快照</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="60"/>
        <source>Selected Snapshot Details</source>
        <translation>所選快照內容</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="66"/>
        <source>Name:</source>
        <translation>名稱:</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="120"/>
        <source>Description:</source>
        <translation>說明:</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="101"/>
        <source>When deleting a snapshot content, it will be returned to this snapshot instead of none.</source>
        <translation>刪除快照內容時，將退回至此快照時的狀態，而不是無動作。</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="104"/>
        <source>Default snapshot</source>
        <translation>預設快照</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="139"/>
        <source>Snapshot Actions</source>
        <translation>快照操作</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="189"/>
        <source>Remove Snapshot</source>
        <translation>移除快照</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="208"/>
        <source>Go to Snapshot</source>
        <translation>進入快照</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="157"/>
        <source>Take Snapshot</source>
        <translation>擷取快照</translation>
    </message>
</context>
</TS>
