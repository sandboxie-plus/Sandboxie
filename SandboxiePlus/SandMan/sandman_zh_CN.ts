<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>CAdvancedPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="428"/>
        <source>Advanced Sandbox options</source>
        <translation>高级沙盒选项</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="429"/>
        <source>On this page advanced sandbox options can be configured.</source>
        <translation>本页面用于配置沙盒的高级选项</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="434"/>
        <source>Network Access</source>
        <translation>网络权限</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="442"/>
        <source>Allow network/internet access</source>
        <translation>允许网络访问</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="443"/>
        <source>Block network/internet by denying access to Network devices</source>
        <translation>通过阻止访问网络设备禁用网络权限</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="445"/>
        <source>Block network/internet using Windows Filtering Platform</source>
        <translation>通过 Windows 筛选平台 (WFP) 禁用网络权限</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="450"/>
        <source>Allow access to network files and folders</source>
        <oldsource>Allow access to network files and fodlers</oldsource>
        <translation>允许访问网络文件与文件夹</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="451"/>
        <location filename="Wizards/NewBoxWizard.cpp" line="467"/>
        <source>This option is not recommended for Hardened boxes</source>
        <oldsource>This option is not recomended for Hardened boxes</oldsource>
        <translation>不推荐加固型沙盒启用该选项</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="457"/>
        <source>Admin Options</source>
        <translation>管理员选项</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="461"/>
        <source>Make applications think they are running elevated</source>
        <translation>让应用认为自身在管理员权限下运行</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="466"/>
        <source>Allow MSIServer to run with a sandboxed system token</source>
        <translation>允许 MSIServer 在沙盒内使用系统令牌运行</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="472"/>
        <source>Box Options</source>
        <translation>沙盒选项</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="476"/>
        <source>Use a Sandboxie login instead of an anonymous token</source>
        <translation>使用 Sandboxie 限权用户替代匿名令牌</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="482"/>
        <source>Prevent sandboxes programs installed on host from loading dll&apos;s from the sandbox</source>
        <translation>阻止安装在宿主上的沙盒程序从沙盒内部加载动态链接库(DLL)文件</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="483"/>
        <source>This feature may reduce compatibility as it also prevents box located processes from writing to host located ones and even starting them.</source>
        <oldsource>该功能可能对兼容性造成影响，因为它阻止了沙箱内的进程向主机进程写入数据，以及启动它们。</oldsource>
        <translation></translation>
    </message>
    <message>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation type="vanished">使用 Sandboxie 限权用户，而不是匿名令牌 (实验性)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="477"/>
        <source>Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.</source>
        <translation>使用自定义沙盒令牌可以更好地将各个沙盒相互隔离，同时可以实现在任务管理器的用户栏中显示进程所属的沙盒。但是，某些第三方安全解决方案可能会与自定义令牌产生兼容性问题</translation>
    </message>
</context>
<context>
    <name>CBoxTypePage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="168"/>
        <source>Create new Sandbox</source>
        <translation>创建新沙盒</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="175"/>
        <source>A sandbox isolates your host system from processes running within the box, it prevents them from making permanent changes to other programs and data in your computer. The level of isolation impacts your security as well as the compatibility with applications, hence there will be a different level of isolation depending on the selected Box Type. Sandboxie can also protect your personal data from being accessed by processes running under its supervision.</source>
        <translation>沙盒将主机系统与在盒内运行的进程隔离开来，可以防止它们对计算机中的其它程序和数据进行永久性的更改。隔离级别会影响您的安全性以及与应用程序的兼容性，因此根据所选的沙盒类型会有不同的隔离级别。此外沙盒还可以保护你的个人数据不被受限制下运行的进程的访问</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="184"/>
        <source>Enter box name:</source>
        <translation>输入沙盒名称：</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="202"/>
        <source>Select box type:</source>
        <oldsource>Sellect box type:</oldsource>
        <translation>选择沙盒类型：</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="205"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>带数据保护的加固型沙盒</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="206"/>
        <source>Security Hardened Sandbox</source>
        <translation>安全防护加固型沙盒</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="207"/>
        <source>Sandbox with Data Protection</source>
        <translation>带数据保护的沙盒</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="208"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>标准隔离沙盒(默认)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="210"/>
        <source>Application Compartment with Data Protection</source>
        <translation>带数据保护的应用隔间</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="211"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>应用隔间(无隔离防护)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="229"/>
        <source>Remove after use</source>
        <translation>在使用结束后删除</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="230"/>
        <source>After the last process in the box terminates, all data in the box will be deleted and the box itself will be removed.</source>
        <translation>在沙盒中所有进程结束后，沙盒中所有数据及沙盒本身将会被删除</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="235"/>
        <source>Configure advanced options</source>
        <translation>高级选项</translation>
    </message>
</context>
<context>
    <name>CBrowserOptionsPage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="844"/>
        <source>Create Web Browser Template</source>
        <translation>创建网络浏览器模板</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="845"/>
        <source>Configure web browser template options.</source>
        <translation>配置网络浏览器模板选项</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="855"/>
        <source>Force the Web Browser to run in this sandbox</source>
        <translation>强制网络浏览器在此沙盒中运行</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="859"/>
        <source>Allow direct access to the entire Web Browser profile folder</source>
        <translation>允许直接访问整个网络浏览器的配置目录</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="863"/>
        <source>Allow direct access to Web Browser&apos;s phishing database</source>
        <translation>允许直接访问网络浏览器的反钓鱼数据库</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="867"/>
        <source>Allow direct access to Web Browser&apos;s session management</source>
        <translation>允许直接访问网络浏览器的会话管理</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="871"/>
        <source>Allow direct access to Web Browser&apos;s sync data</source>
        <translation>允许直接访问网络浏览器的同步数据</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="875"/>
        <source>Allow direct access to Web Browser&apos;s preferences</source>
        <translation>允许直接访问网络浏览器的偏好设定</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="879"/>
        <source>Allow direct access to Web Browser&apos;s passwords</source>
        <translation>允许直接访问网络浏览器存储的密码</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="883"/>
        <source>Allow direct access to Web Browser&apos;s cookies</source>
        <translation>允许直接访问网络浏览器存储的 Cookies</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="887"/>
        <source>Allow direct access to Web Browser&apos;s bookmarks</source>
        <translation>允许直接访问网络浏览器收藏的书签</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="891"/>
        <source>Allow direct access to Web Browser&apos;s bookmark and history database</source>
        <translation>允许直接访问网络浏览器收藏的书签和历史浏览数据</translation>
    </message>
</context>
<context>
    <name>CBrowserPathsPage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="517"/>
        <source>Create Web Browser Template</source>
        <translation>创建网络浏览器模板</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="518"/>
        <source>Configure your Web Browser&apos;s profile directories.</source>
        <oldsource>Configure your Web Browsers profile directories.</oldsource>
        <translation>配置网络浏览器的用户资料配置文件目录</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="526"/>
        <source>User profile(s) directory:</source>
        <translation>用户资料目录：</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="528"/>
        <source>Show also imperfect matches</source>
        <translation>显示不完全匹配</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="534"/>
        <source>Browser Executable (*.exe)</source>
        <translation>浏览可执行文件 (*.exe)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="551"/>
        <source>Continue without browser profile</source>
        <translation>在没有浏览器配置文件的情况下继续后续步骤</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="712"/>
        <source>Configure your Gecko based Browsers profile directories.</source>
        <translation>配置基于 Gecko 核心的浏览器配置文件目录</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="716"/>
        <source>Configure your Chromium based Browsers profile directories.</source>
        <translation>配置基于 Chromium 核心的浏览器配置文件目录</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="796"/>
        <source>No suitable folders have been found.
Note: you need to run the browser unsandboxed for them to get created.
Please browse to the correct user profile directory.</source>
        <oldsource>No suitable fodlers have been found.
Note: you need to run the browser unsandboxed for them to get created.
Please browse to the correct user profile directory.</oldsource>
        <translation>没有发现合适的目录
注意：你需要在不使用沙盒的情况下运行一次浏览器，以便使它们被正确创建
请浏览并选择正确的用户资料配置文件目录</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="801"/>
        <source>Please choose the correct user profile directory, if it is not listed you may need to browse to it.</source>
        <translation>请选择正确的用户资料配置文件目录 (如未列出，你可能需要手动浏览并选择)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="807"/>
        <source>Please ensure the selected directory is correct, the wizard is not confident in all the presented options.</source>
        <translation>请确保所选的目录都是正确的，向导无法帮你核实所选选项是否配置正确</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="810"/>
        <source>Please ensure the selected directory is correct.</source>
        <translation>请确保所选的目录正确无误</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="815"/>
        <source>This path does not look like a valid profile directory.</source>
        <translation>此路径不是有效的配置文件目录</translation>
    </message>
</context>
<context>
    <name>CBrowserTypePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="325"/>
        <source>Create Web Browser Template</source>
        <translation>创建网络浏览器模板</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="326"/>
        <source>Select your Web Browsers main executable, this will allow Sandboxie to identify the browser.</source>
        <oldsource>Select your Web Browsers main executable, this will allow sandboxie to identify the browser.</oldsource>
        <translation>选择网络浏览器的主程序，以便使 Sandboxie 尝试识别浏览器类别</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="336"/>
        <source>Enter browser name:</source>
        <translation>输入浏览器名称：</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="347"/>
        <source>Main executable (eg. firefox.exe, chrome.exe, msedge.exe, etc...):</source>
        <oldsource>Mein executable (eg. firefox.exe, chrome.exe, msedge.exe, etc...):</oldsource>
        <translation>主程序 (例如 firefox.exe, chrome.exe, msedge.exe, 等...)：</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="351"/>
        <source>Browser executable (*.exe)</source>
        <oldsource>Browser Executable (*.exe)</oldsource>
        <translation>浏览可执行程序 (*.exe)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="451"/>
        <source>The browser appears to be Gecko based, like Mozilla Firefox and its derivatives.</source>
        <translation>该浏览器似乎是基于 Gecko 核心的，例如 Mozilla Firefox 及其衍生发行版</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="454"/>
        <source>The browser appears to be Chromium based, like Microsoft Edge or Google Chrome and its derivatives.</source>
        <translation>该浏览器似乎是基于 Chromium 核心的，例如 Microsoft Edge 或 Google Chrome 及其衍生发行版</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="457"/>
        <source>Browser could not be recognized, template cannot be created.</source>
        <translation>无法识别浏览器类型，模板将无法被创建</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="504"/>
        <source>This browser name is already in use, please choose an other one.</source>
        <oldsource>This browser name is already in use, please chooe an other one.</oldsource>
        <translation>该浏览器名称已被使用，请选择其他名称</translation>
    </message>
</context>
<context>
    <name>CCertificatePage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="195"/>
        <source>Install your &lt;b&gt;Sandboxie-Plus&lt;/b&gt; support certificate</source>
        <translation>安装你的 &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 赞助者凭据</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="196"/>
        <source>If you have a supporter certificate, please fill it into the field below.</source>
        <translation>如果你有赞助者凭据，请填入以下字段</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="218"/>
        <source>Start evaluation without a certificate for a limited period of time.</source>
        <translation>开始在没有赞助者凭据的情况下进行有限时间的试用</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="246"/>
        <source>To use &lt;b&gt;Sandboxie-Plus&lt;/b&gt; in a business setting, an appropriate &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;support certificate&lt;/a&gt; for business use is required. If you do not yet have the required certificate(s), you can get those from the &lt;a href=&quot;https://xanasoft.com/shop/&quot;&gt;xanasoft.com web shop&lt;/a&gt;.</source>
        <translation>要在商业环境中使用  &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 高级设置，需要适用于商业用途的&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;赞助者凭据&lt;/a&gt;。如果你还没有所需的凭据，可以通过&lt;a href=&quot;https://xanasoft.com/shop/&quot;&gt;xanasoft.com 网络商店&lt;/a&gt;获取凭据</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="260"/>
        <source>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; provides additional features and box types exclusively to &lt;u&gt;project supporters&lt;/u&gt;. Boxes like the Privacy Enhanced boxes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;protect user data from illicit access&lt;/font&gt;&lt;/b&gt; by the sandboxed programs. If you are not yet a supporter, then please consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt; to ensure further development of Sandboxie and to receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;.</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; 为&lt;u&gt;项目赞助者&lt;/u&gt;提供额外的沙盒类型和其它高级功能。例如“隐私增强”类型的沙盒对来自沙盒化程序非法访问用户数据的行为&lt;b&gt;&lt;font color=&apos;red&apos;&gt;提供额外的用户数据保护&lt;/font&gt;&lt;/b&gt;。如果你还不是赞助者，请考虑 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;捐赠支持此项目&lt;/a&gt;来帮助 Sandboxie 的开发工作，并以此获取&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;赞助者凭据&lt;/a&gt;</translation>
    </message>
</context>
<context>
    <name>CCleanUpJob</name>
    <message>
        <location filename="BoxJob.h" line="36"/>
        <source>Deleting Content</source>
        <translation>正在删除内容</translation>
    </message>
</context>
<context>
    <name>CFileBrowserWindow</name>
    <message>
        <location filename="Views/FileView.cpp" line="398"/>
        <source>%1 - Files</source>
        <translation>%1 - 文件</translation>
    </message>
</context>
<context>
    <name>CFileView</name>
    <message>
        <location filename="Views/FileView.cpp" line="188"/>
        <source>Create Shortcut</source>
        <translation>创建快捷方式</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="210"/>
        <source>Pin to Box Run Menu</source>
        <translation>固定到在沙盒中运行菜单</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="217"/>
        <source>Recover to Any Folder</source>
        <translation>恢复到任意文件夹</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="219"/>
        <source>Recover to Same Folder</source>
        <translation>恢复到对应的目录</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="223"/>
        <source>Run Recovery Checks</source>
        <translation>运行恢复检查</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="287"/>
        <source>Select Directory</source>
        <translation>选择目录</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="353"/>
        <source>Create Shortcut to sandbox %1</source>
        <translation>为沙盒 %1 创建快捷方式</translation>
    </message>
</context>
<context>
    <name>CFilesPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="302"/>
        <source>Sandbox location and behavior</source>
        <oldsource>Sandbox location and behavioure</oldsource>
        <translation>沙盒位置与行为</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="303"/>
        <source>On this page the sandbox location and its behavior can be customized.
You can use %USER% to save each users sandbox to an own folder.</source>
        <oldsource>On this page the sandbox location and its behaviorue can be customized.
You can use %USER% to save each users sandbox to an own fodler.</oldsource>
        <translation>本页面用于配置沙盒位置与行为
可以使用 %USER% 来将用户拥有的沙盒存储到自身的用户目录下</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="308"/>
        <source>Sandboxed Files</source>
        <translation>沙盒化文件</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="331"/>
        <source>Select Directory</source>
        <translation>选择目录</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="339"/>
        <source>Virtualization scheme</source>
        <translation>虚拟化方案</translation>
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
        <translation>区分用户文件夹</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="355"/>
        <source>Use volume serial numbers for drives</source>
        <translation>使用驱动器的卷序列号</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="360"/>
        <source>Auto delete content when last process terminates</source>
        <translation>当所有进程结束后删除所有内容</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="367"/>
        <source>Enable Immediate Recovery of files from recovery locations</source>
        <translation>启用立即恢复功能</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="402"/>
        <source>The selected box location is not a valid path.</source>
        <oldsource>The sellected box location is not a valid path.</oldsource>
        <translation>所选的沙盒存储路径无效</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="407"/>
        <source>The selected box location exists and is not empty, it is recommended to pick a new or empty folder. Are you sure you want to use an existing folder?</source>
        <oldsource>The sellected box location exists and is not empty, it is recomended to pick a new or empty folder. Are you sure you want to use an existing folder?</oldsource>
        <translation>所选的沙盒存储路径不是空的，推荐选择空文件夹或新建文件夹。确定要使用当前选择的文件夹吗？</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="412"/>
        <source>The selected box location is not placed on a currently available drive.</source>
        <oldsource>The selected box location not placed on a currently available drive.</oldsource>
        <translation>所选的沙盒存储路径所在的驱动器当前不可用</translation>
    </message>
</context>
<context>
    <name>CFinishPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="489"/>
        <source>Complete your configuration</source>
        <translation>完成你的配置</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="496"/>
        <source>Almost complete, click Finish to apply all selected options and conclude the wizard.</source>
        <translation>所有设置基本完成了，点击 &quot;完成&quot;，应用所有选定的选项，并结束此向导</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="508"/>
        <source>Keep Sandboxie-Plus up to date.</source>
        <translation>保持 Sandboxie-Plus 为最新版本</translation>
    </message>
</context>
<context>
    <name>CFinishTemplatePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="936"/>
        <source>Create Web Browser Template</source>
        <translation>创建网络浏览器模板</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="944"/>
        <source>Almost complete, click Finish to create a new  Web Browser Template and conclude the wizard.</source>
        <translation>差不多设置好了，点击“完成”按钮，创建一个新的网络浏览器模板，并结束向导</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="962"/>
        <source>Browser name: %1
</source>
        <translation>浏览器名称：%1
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="966"/>
        <source>Browser Type: Gecko (Mozilla Firefox)
</source>
        <oldsource>Browser Type: Gecko (Mozilla firefox)
</oldsource>
        <translation>浏览器类型：Gecko (Mozilla Firefox)
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="967"/>
        <source>Browser Type: Chromium (Google Chrome)
</source>
        <translation>浏览器类型：Chromium (Google Chrome)
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
        <translation>浏览器起始目录：%1
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="980"/>
        <source>Browser profile path: %1
</source>
        <translation>浏览器个人资料目录：%1
</translation>
    </message>
</context>
<context>
    <name>CIntroPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="117"/>
        <source>Introduction</source>
        <translation>摘要</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="121"/>
        <source>Welcome to the Setup Wizard. This wizard will help you to configure your copy of &lt;b&gt;Sandboxie-Plus&lt;/b&gt;. You can start this wizard at any time from the Sandbox-&gt;Maintenance menu if you do not wish to complete it now.</source>
        <translation>欢迎来到设置指南，本指南将帮助配置你的 &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 副本设置。如果你不希望现在就完成向导设置，你可以从 “沙盒 -&gt; 维护”菜单中随时重新启动此向导</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="130"/>
        <source>Select how you would like to use Sandboxie-Plus</source>
        <translation>选择 Sandboxie-Plus 的用途</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="133"/>
        <source>&amp;Personally, for private non-commercial use</source>
        <translation>个人(&amp;P)，用于私人非商业用途</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="138"/>
        <source>&amp;Commercially, for business or enterprise use</source>
        <translation>商业(&amp;C)，用于企业或商业用途</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="143"/>
        <source>Note: this option is persistent</source>
        <translation>注意：此选项无法在后续的设置中重新修改</translation>
    </message>
</context>
<context>
    <name>CMonitorModel</name>
    <message>
        <location filename="Models/MonitorModel.cpp" line="147"/>
        <source>Type</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="148"/>
        <source>Status</source>
        <translation>状态</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="149"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="150"/>
        <source>Count</source>
        <translation>总计</translation>
    </message>
</context>
<context>
    <name>CMultiErrorDialog</name>
    <message>
        <location filename="Dialogs/MultiErrorDialog.cpp" line="10"/>
        <source>Sandboxie-Plus - Error</source>
        <translation>Sandboxie-Plus - 错误</translation>
    </message>
    <message>
        <location filename="Dialogs/MultiErrorDialog.cpp" line="19"/>
        <source>Message</source>
        <translation>消息</translation>
    </message>
</context>
<context>
    <name>CNewBoxWindow</name>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="23"/>
        <source>Sandboxie-Plus - Create New Box</source>
        <translation>Sandboxie-Plus - 新建沙盒</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="39"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>带数据保护的加固型沙盒</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="40"/>
        <source>Security Hardened Sandbox</source>
        <translation>安全防护加固型沙盒</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="41"/>
        <source>Sandbox with Data Protection</source>
        <translation>带数据保护的沙盒</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="42"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>标准隔离沙盒(默认)</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="44"/>
        <source>Application Compartment with Data Protection</source>
        <translation>带数据保护的应用隔间</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="45"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>应用隔间(无隔离防护)</translation>
    </message>
</context>
<context>
    <name>CNewBoxWizard</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="30"/>
        <source>New Box Wizard</source>
        <translation>新建沙盒向导</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="138"/>
        <source>The new sandbox has been created using the new &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;Virtualization Scheme Version 2&lt;/a&gt;, if you experience any unexpected issues with this box, please switch to the Virtualization Scheme to Version 1 and report the issue, the option to change this preset can be found in the Box Options in the Box Structure group.</source>
        <oldsource>The new sandbox has been created using the new &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;Virtualization Scheme Version 2&lt;/a&gt;, if you expirience any unecpected issues with this box, please switch to the Virtualization Scheme to Version 1 and report the issue, the option to change this preset can be found in the Box Options in the Box Structure groupe.</oldsource>
        <translation>新沙盒将按照新的 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;虚拟化方案 2&lt;/a&gt;创建，如果您在使用该沙盒的时候遇到任何问题，请尝试切换至旧版本的虚拟化方案并反馈相应的问题，该选项可以在沙盒结构菜单中找到。</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="141"/>
        <source>Don&apos;t show this message again.</source>
        <translation>不再显示此消息</translation>
    </message>
</context>
<context>
    <name>COnDeleteJob</name>
    <message>
        <location filename="BoxJob.h" line="58"/>
        <source>OnDelete: %1</source>
        <translation>删除阶段: %1</translation>
    </message>
</context>
<context>
    <name>COnlineUpdater</name>
    <message>
        <location filename="OnlineUpdater.cpp" line="99"/>
        <source>Do you want to check if there is a new version of Sandboxie-Plus?</source>
        <translation>您是否想检查 Sandboxie-Plus 的新版本？</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="100"/>
        <source>Don&apos;t show this message again.</source>
        <translation>不再显示此消息</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="209"/>
        <source>Checking for updates...</source>
        <translation>正在检查更新...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="248"/>
        <source>server not reachable</source>
        <translation>无法连接到服务器</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="249"/>
        <location filename="OnlineUpdater.cpp" line="251"/>
        <source>Failed to check for updates, error: %1</source>
        <translation>检查更新失败，错误：%1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="404"/>
        <source>&lt;p&gt;Do you want to download the installer?&lt;/p&gt;</source>
        <translation>&lt;p&gt;是否下载此安装程序？&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="406"/>
        <source>&lt;p&gt;Do you want to download the updates?&lt;/p&gt;</source>
        <translation>&lt;p&gt;是否下载此更新包？&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="408"/>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;update page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation>&lt;p&gt;是否转跳到&lt;a href=&quot;%1&quot;&gt;更新页面&lt;/a&gt;？&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="416"/>
        <source>Don&apos;t show this update anymore.</source>
        <translation>不再显示此次更新</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="534"/>
        <source>Downloading updates...</source>
        <translation>正在下载更新...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="560"/>
        <source>invalid parameter</source>
        <translation>无效参数</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="561"/>
        <source>failed to download updated information</source>
        <oldsource>failed to download update informations</oldsource>
        <translation>无法获取更新信息</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="562"/>
        <source>failed to load updated json file</source>
        <oldsource>failed to load update json file</oldsource>
        <translation>加载更新 Json 文件失败</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="563"/>
        <source>failed to download a particular file</source>
        <translation>未能下载特定文件</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="564"/>
        <source>failed to scan existing installation</source>
        <translation>未能扫描现有的安装</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="565"/>
        <source>updated signature is invalid !!!</source>
        <oldsource>update signature is invalid !!!</oldsource>
        <translation>更新包签名无效 !!!</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="566"/>
        <source>downloaded file is corrupted</source>
        <translation>下载的文件已损坏</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="567"/>
        <source>internal error</source>
        <translation>内部错误</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="568"/>
        <source>unknown error</source>
        <translation>未知错误</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="592"/>
        <source>Failed to download updates from server, error %1</source>
        <translation>从服务器下载更新失败，错误 %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="611"/>
        <source>&lt;p&gt;Updates for Sandboxie-Plus have been downloaded.&lt;/p&gt;&lt;p&gt;Do you want to apply these updates? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Sandboxie-Plus 的更新已下载。&lt;/p&gt;&lt;p&gt;是否要安装更新？本操作需要终止所有沙盒中运行的程序。&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="715"/>
        <source>Downloading installer...</source>
        <translation>正在下载安装程序...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="757"/>
        <source>Failed to download installer from: %1</source>
        <translation>从 %1 下载安装程序失败</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="783"/>
        <source>&lt;p&gt;A new Sandboxie-Plus installer has been downloaded to the following location:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Do you want to begin the installation? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation>&lt;p&gt;一个新的 Sandboxie-Plus 安装程序已被下载到以下位置：&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;是否安装？本操作需要终止所有沙盒中运行的程序。&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="848"/>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;info page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation>&lt;p&gt;您是否要前往&lt; &quot;%1&quot;&gt;信息页&lt;/a&gt;？&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="856"/>
        <source>Don&apos;t show this announcement in the future.</source>
        <translation>不再显示此公告</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="398"/>
        <source>&lt;p&gt;There is a new version of Sandboxie-Plus available.&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;&lt;b&gt;New version:&lt;/b&gt;&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;Sandboxie-Plus 存在可用的新版本，&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;&lt;b&gt;新版本: &lt;/b&gt;&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="274"/>
        <source>No new updates found, your Sandboxie-Plus is up-to-date.

Note: The update check is often behind the latest GitHub release to ensure that only tested updates are offered.</source>
        <translation>当前没有可用的更新, Sandboxie Plus 已是最新版本

注意: 更新检查通常落后于 GitHub Release 的版本，以确保只提供经过测试的更新</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="964"/>
        <source>Checking for certificate...</source>
        <translation>检索凭据...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="1004"/>
        <source>No certificate found on server!</source>
        <translation>未在服务器检索到凭据！</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="1009"/>
        <source>There is no updated certificate available.</source>
        <translation>目前没有可用的凭据更新</translation>
    </message>
</context>
<context>
    <name>COptionsWindow</name>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="181"/>
        <source>Sandboxie Plus - &apos;%1&apos; Options</source>
        <translation>Sandboxie Plus - &apos;%1&apos; 选项</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="41"/>
        <source>Enable the use of win32 hooks for selected processes. Note: You need to enable win32k syscall hook support globally first.</source>
        <translation>对选定的进程启用 Win32 钩子(注意：需要先启用全局范围的 Win32k 系统调用钩子支持)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="42"/>
        <source>Enable crash dump creation in the sandbox folder</source>
        <translation>启用在沙盒目录下创建崩溃转储文件</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="43"/>
        <source>Always use ElevateCreateProcess fix, as sometimes applied by the Program Compatibility Assistant.</source>
        <translation>始终应用 ElevateCreateProcess 修复，偶尔会被程序兼容性助手(PCA)调用</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="45"/>
        <source>Enable special inconsistent PreferExternalManifest behaviour, as needed for some Edge fixes</source>
        <oldsource>Enable special inconsistent PreferExternalManifest behavioure, as neede for some edge fixes</oldsource>
        <translation>启用不一致的特殊 PreferExternalManifest 行为支持，修复 Microsoft Edge 存在的某些问题可能需要打开此选项</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="46"/>
        <source>Set RpcMgmtSetComTimeout usage for specific processes</source>
        <translation>为特定进程设置 RpcMgmtSetComTimeout 选项</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="47"/>
        <source>Makes a write open call to a file that won&apos;t be copied fail instead of turning it read-only.</source>
        <translation>使得一个禁止被复制文件的写入句柄调用失败，而不是将其变成只读</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="50"/>
        <source>Make specified processes think they have admin permissions.</source>
        <oldsource>Make specified processes think thay have admin permissions.</oldsource>
        <translation>让特定进程认为它们具有管理员权限</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="51"/>
        <source>Force specified processes to wait for a debugger to attach.</source>
        <translation>强制指定的进程等待调试器附加</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="53"/>
        <source>Sandbox file system root</source>
        <translation>沙盒文件系统根目录</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="54"/>
        <source>Sandbox registry root</source>
        <translation>沙盒注册表根目录</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="55"/>
        <source>Sandbox ipc root</source>
        <translation>沙盒 IPC 根目录</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="575"/>
        <source>Add special option:</source>
        <translation>添加特殊选项:</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="726"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="730"/>
        <source>On Start</source>
        <translation>沙盒启动阶段</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="727"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="735"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="739"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="743"/>
        <source>Run Command</source>
        <translation>执行命令</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="731"/>
        <source>Start Service</source>
        <translation>启动服务</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="734"/>
        <source>On Init</source>
        <translation>沙盒初始阶段</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="738"/>
        <source>On File Recovery</source>
        <translation>文件恢复阶段</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="742"/>
        <source>On Delete Content</source>
        <translation>内容删除阶段</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="753"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="775"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="786"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="797"/>
        <source>Please enter the command line to be executed</source>
        <translation>请输入需要执行的命令行</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="917"/>
        <source>Deny</source>
        <translation>拒绝(禁止)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="1022"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="19"/>
        <source>This option requires a valid supporter certificate</source>
        <translation>此选项需要一份有效的赞助者凭据</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="22"/>
        <source>Supporter exclusive option</source>
        <translation>赞助者专属选项</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="47"/>
        <source>Don&apos;t alter the window title</source>
        <translation>不改变窗口标题</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="48"/>
        <source>Display [#] indicator only</source>
        <translation>只显示 [#] 标记</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="49"/>
        <source>Display box name in title</source>
        <translation>标题内显示沙盒名称</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="51"/>
        <source>Border disabled</source>
        <translation>禁用边框</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="52"/>
        <source>Show only when title is in focus</source>
        <translation>仅在标题栏获取焦点时显示</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="53"/>
        <source>Always show</source>
        <translation>始终显示</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="56"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>带数据保护的加固型沙盒</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="57"/>
        <source>Security Hardened Sandbox</source>
        <translation>安全防护加固型沙盒</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="58"/>
        <source>Sandbox with Data Protection</source>
        <translation>带数据保护的沙盒</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="59"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>标准隔离沙盒(默认)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="61"/>
        <source>Application Compartment with Data Protection</source>
        <translation>带数据保护的应用隔间</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="62"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>应用隔间(无隔离防护)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="109"/>
        <source>Custom icon</source>
        <translation>自定义图标</translation>
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
        <translation>浏览程序</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="258"/>
        <source>Open Box Options</source>
        <translation>打开沙盒选项</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="259"/>
        <source>Browse Content</source>
        <translation>浏览内容</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="260"/>
        <source>Start File Recovery</source>
        <translation>开始恢复文件</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="261"/>
        <source>Show Run Dialog</source>
        <translation>显示运行对话框</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="294"/>
        <source>Indeterminate</source>
        <translation>不确定</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="484"/>
        <location filename="Windows/OptionsGeneral.cpp" line="584"/>
        <source>Always copy</source>
        <translation>始终复制</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="485"/>
        <location filename="Windows/OptionsGeneral.cpp" line="585"/>
        <source>Don&apos;t copy</source>
        <translation>不要复制</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="486"/>
        <location filename="Windows/OptionsGeneral.cpp" line="586"/>
        <source>Copy empty</source>
        <translation>复制空的副本</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="446"/>
        <location filename="Windows/OptionsWindow.cpp" line="460"/>
        <location filename="Windows/OptionsAccess.cpp" line="24"/>
        <source>Browse for File</source>
        <translation>浏览文件</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="25"/>
        <source>Browse for Folder</source>
        <translation>浏览文件夹</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="287"/>
        <source>File Options</source>
        <translation>文件选项</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="319"/>
        <source>Grouping</source>
        <translation>分组</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="525"/>
        <source>Add %1 Template</source>
        <translation>添加 %1 模板</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="625"/>
        <source>Search for options</source>
        <translation>搜索选项</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="755"/>
        <source>Box: %1</source>
        <translation>沙盒: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="756"/>
        <source>Template: %1</source>
        <translation>模板: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="757"/>
        <source>Global: %1</source>
        <translation>全局: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="758"/>
        <source>Default: %1</source>
        <translation>默认: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="935"/>
        <source>This sandbox has been deleted hence configuration can not be saved.</source>
        <translation>该沙盒已被删除，因此配置无法保存</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="982"/>
        <source>Some changes haven&apos;t been saved yet, do you really want to close this options window?</source>
        <translation>部分变更未保存，确定要关闭这个选项窗口吗？</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="712"/>
        <source>kilobytes (%1)</source>
        <translation>KB (%1)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="818"/>
        <source>Select color</source>
        <translation>选择颜色</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="893"/>
        <source>Select Program</source>
        <translation>选择程序</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="764"/>
        <source>Please enter a service identifier</source>
        <translation>请输入一个服务标识符</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="893"/>
        <source>Executables (*.exe *.cmd)</source>
        <translation>可执行文件 (*.exe *.cmd)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="897"/>
        <location filename="Windows/OptionsGeneral.cpp" line="913"/>
        <source>Please enter a menu title</source>
        <translation>请输入一个菜单标题</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="909"/>
        <source>Please enter a command</source>
        <translation>请输入一则命令</translation>
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
        <translation>组: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGrouping.cpp" line="141"/>
        <source>Please enter a name for the new group</source>
        <translation>请输入新组的名称</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="1005"/>
        <source>Enter program:</source>
        <translation>请输入程序：</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGrouping.cpp" line="229"/>
        <source>Please select group first.</source>
        <translation>请先选择组</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="111"/>
        <location filename="Windows/OptionsForce.cpp" line="122"/>
        <source>Process</source>
        <translation>进程</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="111"/>
        <location filename="Windows/OptionsForce.cpp" line="122"/>
        <source>Folder</source>
        <translation>文件夹</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="222"/>
        <location filename="Windows/OptionsForce.cpp" line="232"/>
        <source>Select Executable File</source>
        <translation>选择可执行文件</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="222"/>
        <location filename="Windows/OptionsForce.cpp" line="232"/>
        <source>Executable Files (*.exe)</source>
        <translation>可执行文件 (*.exe)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="365"/>
        <location filename="Windows/OptionsForce.cpp" line="242"/>
        <location filename="Windows/OptionsForce.cpp" line="252"/>
        <location filename="Windows/OptionsRecovery.cpp" line="128"/>
        <location filename="Windows/OptionsRecovery.cpp" line="139"/>
        <source>Select Directory</source>
        <translation>选择目录</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="312"/>
        <source>Closed</source>
        <translation>封禁</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="313"/>
        <source>Closed RT</source>
        <translation>封禁 RT</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="314"/>
        <source>Read Only</source>
        <translation>只读</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="308"/>
        <source>Normal</source>
        <translation>标准</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="309"/>
        <source>Open</source>
        <translation>开放</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="310"/>
        <source>Open for All</source>
        <translation>完全开放</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="311"/>
        <source>No Rename</source>
        <translation>禁止重命名</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="315"/>
        <source>Box Only (Write Only)</source>
        <translation>仅沙盒内 (只写)</translation>
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
        <translation>常规沙盒行为 - 允许读取及写时复制</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="326"/>
        <source>Allow write-access outside the sandbox.</source>
        <translation>允许透写到沙盒外(仅当执行写操作的程序位于沙盒外时)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="327"/>
        <source>Allow write-access outside the sandbox, also for applications installed inside the sandbox.</source>
        <translation>允许透写到沙盒外(无论执行写操作的程序是否位于沙盒内)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="328"/>
        <source>Don&apos;t rename window classes.</source>
        <translation>禁止重命名 Windows 窗口类</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="329"/>
        <source>Deny access to host location and prevent creation of sandboxed copies.</source>
        <translation>拒绝对主机位置的访问，防止在沙盒内创建相应的副本</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="330"/>
        <source>Block access to WinRT class.</source>
        <translation>阻止对 WinRT 类的访问</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="331"/>
        <source>Allow read-only access only.</source>
        <translation>只允许只读访问</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="332"/>
        <source>Hide host files, folders or registry keys from sandboxed processes.</source>
        <translation>对沙盒内的进程隐藏主机文件、目录或注册表键值</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="333"/>
        <source>Ignore UIPI restrictions for processes.</source>
        <translation>忽略对进程的 UIPI 限制</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="342"/>
        <source>File/Folder</source>
        <translation>文件/文件夹</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="343"/>
        <source>Registry</source>
        <translation>注册表</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="344"/>
        <source>IPC Path</source>
        <translation>IPC 路径</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="345"/>
        <source>Wnd Class</source>
        <translation>窗口类</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="346"/>
        <source>COM Object</source>
        <translation>COM 组件</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="353"/>
        <source>Select File</source>
        <translation>选择文件</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="353"/>
        <source>All Files (*.*)</source>
        <translation>所有文件 (*.*)</translation>
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
        <translation>所有程序</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="537"/>
        <source>COM objects must be specified by their GUID, like: {00000000-0000-0000-0000-000000000000}</source>
        <translation>COM 对象必须用其 GUID 来指定，例如：{00000000-0000-0000-0000-000000000000}</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="543"/>
        <source>RT interfaces must be specified by their name.</source>
        <translation>RT 接口必须用其名称来指定</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="557"/>
        <source>Opening all IPC access also opens COM access, do you still want to restrict COM to the sandbox?</source>
        <translation>开放 IPC 访问权限的同时也将开放 COM 的访问权限，你是否想继续在沙盒内限制 COM 接口的访问权限?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="558"/>
        <source>Don&apos;t ask in future</source>
        <translation>此后不再询问</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="567"/>
        <source>&apos;OpenWinClass=program.exe,#&apos; is not supported, use &apos;NoRenameWinClass=program.exe,*&apos; instead</source>
        <translation>不支持 &apos;OpenWinClass=program.exe,#&apos; 配置格式，请使用 &apos;NoRenameWinClass=program.exe,*&apos; 替换</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="611"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="605"/>
        <location filename="Windows/OptionsGeneral.cpp" line="579"/>
        <location filename="Windows/OptionsGrouping.cpp" line="234"/>
        <location filename="Windows/OptionsGrouping.cpp" line="258"/>
        <location filename="Windows/OptionsNetwork.cpp" line="533"/>
        <source>Template values can not be edited.</source>
        <translation>模板值不能被编辑</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="679"/>
        <source>Template values can not be removed.</source>
        <translation>模板值无法被移除</translation>
    </message>
    <message>
        <location filename="Windows/OptionsRecovery.cpp" line="150"/>
        <source>Please enter a file extension to be excluded</source>
        <translation>请输入一个要排除的文件扩展名</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="835"/>
        <source>Please enter a program file name</source>
        <translation>请输入一个程序文件名称</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="56"/>
        <source>All Categories</source>
        <translation>所有类别</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="95"/>
        <source>Custom Templates</source>
        <translation>自定义模板</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="96"/>
        <source>Email Reader</source>
        <translation>电子邮件客户端</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="97"/>
        <source>PDF/Print</source>
        <translation>PDF/打印</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="98"/>
        <source>Security/Privacy</source>
        <translation>安全和隐私</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="99"/>
        <source>Desktop Utilities</source>
        <translation>桌面工具</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="100"/>
        <source>Download Managers</source>
        <translation>下载程序</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="101"/>
        <source>Miscellaneous</source>
        <translation>杂项</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="102"/>
        <source>Web Browser</source>
        <translation>网络浏览器</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="103"/>
        <source>Media Player</source>
        <translation>多媒体播放器</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="104"/>
        <source>Torrent Client</source>
        <translation>BT 种子(Torrent)客户端</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="151"/>
        <source>This template is enabled globally. To configure it, use the global options.</source>
        <translation>此模板已全局启用，如需配置，请前往全局选项</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="187"/>
        <source>Please enter the template identifier</source>
        <translation>请输入模板标识符</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="195"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="222"/>
        <source>Do you really want to delete the selected local template(s)?</source>
        <translation>你真的想删除选定的本地模板吗？</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="228"/>
        <source>Only local templates can be removed!</source>
        <translation>仅可删除本地模板！</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="37"/>
        <location filename="Windows/OptionsNetwork.cpp" line="588"/>
        <source>Any</source>
        <translation>任意</translation>
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
        <translation>允许访问</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="53"/>
        <source>Block using Windows Filtering Platform</source>
        <translation>阻止访问 - 使用 Windows 筛选平台 (WFP)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="54"/>
        <source>Block by denying access to Network devices</source>
        <translation>阻止访问 - 通过禁止访问网络设备</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="917"/>
        <location filename="Windows/OptionsNetwork.cpp" line="171"/>
        <location filename="Windows/OptionsNetwork.cpp" line="574"/>
        <source>Allow</source>
        <translation>允许</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="172"/>
        <source>Block (WFP)</source>
        <translation>阻止 (WFP)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="173"/>
        <source>Block (NDev)</source>
        <translation>阻止 (网络设备)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="278"/>
        <source>A non empty program name is required.</source>
        <translation>程序名不得为空</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="575"/>
        <source>Block</source>
        <translation>阻止</translation>
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
        <translation>访问 %1 以查阅详细说明</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="57"/>
        <source>Dismiss</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="58"/>
        <source>Remove this message from the list</source>
        <translation>在列表中移除此消息</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="61"/>
        <source>Hide all such messages</source>
        <translation>隐藏所有类似的消息</translation>
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
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="367"/>
        <source>Remove this progress indicator from the list</source>
        <translation>移除列表中的该进程标识符</translation>
    </message>
</context>
<context>
    <name>CPopUpPrompt</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="108"/>
        <source>Remember for this process</source>
        <translation>记住对此进程的选择</translation>
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
        <translation>终止</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="149"/>
        <source>Yes and add to allowed programs</source>
        <translation>是且添加到允许的程序</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="184"/>
        <source>Requesting process terminated</source>
        <translation>请求的进程已终止</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="189"/>
        <source>Request will time out in %1 sec</source>
        <translation>请求将在 %1 秒后超时</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="191"/>
        <source>Request timed out</source>
        <translation>请求超时</translation>
    </message>
</context>
<context>
    <name>CPopUpRecovery</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="245"/>
        <source>Recover to:</source>
        <translation>恢复到：</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="250"/>
        <source>Browse</source>
        <translation>浏览</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="251"/>
        <source>Clear folder list</source>
        <translation>清除文件夹列表</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="259"/>
        <source>Recover</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="260"/>
        <source>Recover the file to original location</source>
        <translation>恢复文件到原路径</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="263"/>
        <source>Recover &amp;&amp; Explore</source>
        <translation>恢复并浏览</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="264"/>
        <source>Recover &amp;&amp; Open/Run</source>
        <translation>恢复并打开/运行</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="266"/>
        <source>Open file recovery for this box</source>
        <translation>针对此沙盒打开文件恢复</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="274"/>
        <source>Dismiss</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="275"/>
        <source>Don&apos;t recover this file right now</source>
        <translation>目前暂不恢复此文件</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="278"/>
        <source>Dismiss all from this box</source>
        <translation>对此沙盒忽略全部</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="279"/>
        <source>Disable quick recovery until the box restarts</source>
        <translation>在沙盒重启前禁用快速恢复</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="303"/>
        <source>Select Directory</source>
        <translation>选择目录</translation>
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
        <translation>要允许 %1 (%2) 利用打印处理服务在沙盒外写入吗？</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="279"/>
        <source>Do you want to allow %4 (%5) to copy a %1 large file into sandbox: %2?
File name: %3</source>
        <translation>要允许 %4 (%5) 复制大文件 %1 到 %2 沙盒吗？
文件名：%3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="285"/>
        <source>Do you want to allow %1 (%2) access to the internet?
Full path: %3</source>
        <translation>要允许 %1 (%2) 访问网络吗？
完整路径：%3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="344"/>
        <source>%1 is eligible for quick recovery from %2.
The file was written by: %3</source>
        <translation>%1 可以从 %2 快速恢复
文件写入者：%3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="346"/>
        <source>an UNKNOWN process.</source>
        <translation>未知进程</translation>
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
        <translation>迁移一个大文件 %1 到沙盒 %2，剩余 %3
完整路径：%4</translation>
    </message>
</context>
<context>
    <name>CRecoveryLogWnd</name>
    <message>
        <location filename="SandManRecovery.cpp" line="306"/>
        <source>Sandboxie-Plus - Recovery Log</source>
        <translation>Sandboxie-Plus - 恢复日志</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="317"/>
        <source>Time|Box Name|File Path</source>
        <translation>时间|沙盒名称|文件路径</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="319"/>
        <source>Cleanup Recovery Log</source>
        <translation>清理恢复日志</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="329"/>
        <source>The following files were recently recovered and moved out of a sandbox.</source>
        <oldsource>the following files were recently recovered and moved out of a sandbox.</oldsource>
        <translation>以下是最近被恢复并移出沙盒的文件</translation>
    </message>
</context>
<context>
    <name>CRecoveryWindow</name>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="46"/>
        <source>%1 - File Recovery</source>
        <translation>%1 - 文件恢复</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="70"/>
        <source>File Name</source>
        <translation>文件名称</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="71"/>
        <source>File Size</source>
        <translation>文件大小</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="72"/>
        <source>Full Path</source>
        <translation>完整路径</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="106"/>
        <source>Remember target selection</source>
        <translation>记住对此目标的选择</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="112"/>
        <source>Delete everything, including all snapshots</source>
        <translation>删除所有内容，包括所有快照</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="138"/>
        <source>Original location</source>
        <translation>原始位置</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="139"/>
        <source>Browse for location</source>
        <translation>浏览位置</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="140"/>
        <source>Clear folder list</source>
        <translation>清除文件夹列表</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="180"/>
        <location filename="Windows/RecoveryWindow.cpp" line="201"/>
        <location filename="Windows/RecoveryWindow.cpp" line="552"/>
        <source>Select Directory</source>
        <translation>选择目录</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="248"/>
        <source>Do you really want to delete %1 selected files?</source>
        <translation>是否删除 %1 选中的文件？</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="276"/>
        <source>Close until all programs stop in this box</source>
        <translation>关闭，在沙盒内全部程序停止后再显示</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="277"/>
        <source>Close and Disable Immediate Recovery for this box</source>
        <translation>关闭并禁用此沙盒的立即恢复功能</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="297"/>
        <source>There are %1 new files available to recover.</source>
        <translation>有 %1 个新文件可供恢复</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="588"/>
        <source>There are %1 files and %2 folders in the sandbox, occupying %3 of disk space.</source>
        <oldsource>There are %1 files and %2 folders in the sandbox, occupying %3 bytes of disk space.</oldsource>
        <translation>此沙盒中共有 %1 个文件和 %2 个文件夹，占用了 %3 磁盘空间</translation>
    </message>
</context>
<context>
    <name>CSandBox</name>
    <message>
        <location filename="SandMan.cpp" line="3614"/>
        <source>Waiting for folder: %1</source>
        <translation>正在等待文件夹: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3615"/>
        <source>Deleting folder: %1</source>
        <translation>正在删除文件夹: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3616"/>
        <source>Merging folders: %1 &amp;gt;&amp;gt; %2</source>
        <translation>正在合并文件夹: %1 &amp;gt;&amp;gt; %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3617"/>
        <source>Finishing Snapshot Merge...</source>
        <translation>正在完成快照合并...</translation>
    </message>
</context>
<context>
    <name>CSandBoxPlus</name>
    <message>
        <location filename="SbiePlusAPI.cpp" line="652"/>
        <source>Disabled</source>
        <translation>禁用</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="663"/>
        <source>OPEN Root Access</source>
        <translation>开放 Root 根权限</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="665"/>
        <source>Application Compartment</source>
        <translation>应用隔间</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="667"/>
        <source>NOT SECURE</source>
        <translation>不安全</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="669"/>
        <source>Reduced Isolation</source>
        <translation>削弱隔离</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="671"/>
        <source>Enhanced Isolation</source>
        <translation>加强隔离</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="674"/>
        <source>Privacy Enhanced</source>
        <translation>隐私增强</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="677"/>
        <source>API Log</source>
        <translation>API 日志</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="680"/>
        <source>No INet (with Exceptions)</source>
        <translation>无 INet (允许例外)</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="682"/>
        <source>No INet</source>
        <translation>无网络</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="685"/>
        <source>Net Share</source>
        <translation>网络共享</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="687"/>
        <source>No Admin</source>
        <translation>无管理员</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="690"/>
        <source>Auto Delete</source>
        <translation>自动删除</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="693"/>
        <source>Normal</source>
        <translation>标准</translation>
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
        <location filename="SandMan.cpp" line="1015"/>
        <source>Click to run installer</source>
        <translation>点击以运行安装程序</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1019"/>
        <source>Click to apply update</source>
        <translation>点击以应用更新</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1022"/>
        <source>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update v%1 available&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;存在一个新的 Sandboxie-Plus 更新 v%1 可用&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1639"/>
        <source>No Force Process</source>
        <translation>没有必沙程序</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3503"/>
        <source>Reset Columns</source>
        <translation>重置列</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3504"/>
        <source>Copy Cell</source>
        <translation>复制此格</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3505"/>
        <source>Copy Row</source>
        <translation>复制此行</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3506"/>
        <source>Copy Panel</source>
        <translation>复制此表</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1185"/>
        <source>Time|Message</source>
        <translation>时间|消息</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1194"/>
        <source>Sbie Messages</source>
        <translation>沙盒消息</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1201"/>
        <source>Trace Log</source>
        <translation>跟踪日志</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="45"/>
        <source>Show/Hide</source>
        <translation>显示/隐藏</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="468"/>
        <location filename="SandMan.cpp" line="684"/>
        <source>&amp;Sandbox</source>
        <translation>沙盒(&amp;S)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="469"/>
        <location filename="SandMan.cpp" line="942"/>
        <location filename="SandMan.cpp" line="943"/>
        <source>Create New Box</source>
        <translation>新建沙盒</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="470"/>
        <source>Create Box Group</source>
        <translation>新建沙盒组</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="474"/>
        <location filename="SandMan.cpp" line="615"/>
        <source>Terminate All Processes</source>
        <translation>终止所有进程</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="572"/>
        <source>Disable File Recovery</source>
        <translation>禁用文件恢复</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="575"/>
        <source>Disable Message Popup</source>
        <oldsource>Disable Message PopUp</oldsource>
        <translation>禁用消息弹窗</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="410"/>
        <source>&amp;Maintenance</source>
        <translation>维护(&amp;M)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="411"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="412"/>
        <source>Disconnect</source>
        <translation>断开</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="414"/>
        <source>Stop All</source>
        <translation>停止所有</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="415"/>
        <source>&amp;Advanced</source>
        <translation>高级(&amp;A)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="416"/>
        <source>Install Driver</source>
        <translation>安装驱动</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="417"/>
        <source>Start Driver</source>
        <translation>启动驱动</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="418"/>
        <source>Stop Driver</source>
        <translation>停止驱动</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="419"/>
        <source>Uninstall Driver</source>
        <translation>卸载驱动</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="421"/>
        <source>Install Service</source>
        <translation>安装服务</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="422"/>
        <source>Start Service</source>
        <translation>启动服务</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="423"/>
        <source>Stop Service</source>
        <translation>停止服务</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="424"/>
        <source>Uninstall Service</source>
        <translation>卸载服务</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="426"/>
        <source>Setup Wizard</source>
        <translation>设置指南</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="429"/>
        <source>Uninstall All</source>
        <translation>全部卸载</translation>
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
        <translation>视图(&amp;V)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="435"/>
        <source>Simple View</source>
        <translation>简易视图</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="436"/>
        <source>Advanced View</source>
        <translation>高级视图</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="441"/>
        <source>Always on Top</source>
        <translation>窗口置顶</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="510"/>
        <source>Show Hidden Boxes</source>
        <translation>显示隐藏沙盒</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="512"/>
        <source>Show All Sessions</source>
        <translation>显示所有会话的进程</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="529"/>
        <source>Refresh View</source>
        <translation>刷新视图</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="535"/>
        <source>Clean Up</source>
        <translation>清理</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="536"/>
        <source>Cleanup Processes</source>
        <translation>清理所有记录</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="538"/>
        <source>Cleanup Message Log</source>
        <translation>清理消息日志</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="539"/>
        <source>Cleanup Trace Log</source>
        <translation>清理跟踪日志</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="540"/>
        <source>Cleanup Recovery Log</source>
        <translation>清理恢复日志</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="542"/>
        <source>Keep terminated</source>
        <translation>保留终止的进程</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="562"/>
        <source>&amp;Options</source>
        <translation>选项(&amp;O)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="563"/>
        <location filename="SandMan.cpp" line="705"/>
        <source>Global Settings</source>
        <translation>全局设置</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="597"/>
        <location filename="SandMan.cpp" line="715"/>
        <source>Reset all hidden messages</source>
        <translation>重置所有已隐藏消息</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="598"/>
        <location filename="SandMan.cpp" line="716"/>
        <source>Reset all GUI options</source>
        <translation>重置所有图形界面设置选项</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="555"/>
        <source>Trace Logging</source>
        <translation>启用跟踪日志</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="437"/>
        <source>Vintage View (like SbieCtrl)</source>
        <translation>经典视图(类似 SbieCtrl)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="447"/>
        <source>&amp;Help</source>
        <translation>帮助(&amp;H)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="456"/>
        <source>Visit Support Forum</source>
        <translation>访问用户支持论坛</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="455"/>
        <source>Online Documentation</source>
        <translation>在线文档</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="454"/>
        <source>Contribute to Sandboxie-Plus</source>
        <translation>为 Sandboxie-Plus 贡献力量</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="458"/>
        <source>Check for Updates</source>
        <translation>检查更新</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="460"/>
        <source>About the Qt Framework</source>
        <translation>关于 Qt 框架</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="461"/>
        <location filename="SandMan.cpp" line="3588"/>
        <source>About Sandboxie-Plus</source>
        <translation>关于 Sandboxie-Plus</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="471"/>
        <source>Import Box</source>
        <translation>导入沙盒</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="473"/>
        <location filename="SandMan.cpp" line="614"/>
        <source>Run Sandboxed</source>
        <translation>运行沙盒</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="476"/>
        <location filename="SandMan.cpp" line="628"/>
        <source>Is Window Sandboxed?</source>
        <oldsource>Is Window Sandboxed</oldsource>
        <translation>检查窗口是否沙盒化</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="521"/>
        <source>Show File Panel</source>
        <translation>显示文件面板</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="585"/>
        <location filename="SandMan.cpp" line="723"/>
        <location filename="SandMan.cpp" line="958"/>
        <location filename="SandMan.cpp" line="959"/>
        <source>Edit Sandboxie.ini</source>
        <translation>编辑 Sandboxie 配置文件</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="588"/>
        <source>Edit Templates.ini</source>
        <translation>编辑模板配置文件</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="590"/>
        <source>Edit Sandboxie-Plus.ini</source>
        <translation>编辑 Sandboxie-Plus 配置文件</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="594"/>
        <location filename="SandMan.cpp" line="726"/>
        <source>Reload configuration</source>
        <translation>重新加载配置文件</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="613"/>
        <source>&amp;File</source>
        <translation>文件(&amp;F)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="629"/>
        <source>Resource Access Monitor</source>
        <translation>资源访问监控</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="662"/>
        <source>Programs</source>
        <translation>程序</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="663"/>
        <source>Files and Folders</source>
        <translation>文件和目录</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="689"/>
        <source>Create New Sandbox</source>
        <translation>新建沙盒</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="690"/>
        <source>Create New Group</source>
        <translation>新建沙盒组</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="691"/>
        <source>Import Sandbox</source>
        <translation>导入沙盒</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="693"/>
        <source>Set Container Folder</source>
        <translation>设置沙盒容器目录</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="696"/>
        <source>Set Layout and Groups</source>
        <translation>设置布局和组</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="698"/>
        <source>Reveal Hidden Boxes</source>
        <translation>显示隐藏的沙盒</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="704"/>
        <source>&amp;Configure</source>
        <translation>配置(&amp;C)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="708"/>
        <source>Program Alerts</source>
        <translation>程序警报</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="710"/>
        <source>Windows Shell Integration</source>
        <translation>Windows Shell 界面集成</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="712"/>
        <source>Software Compatibility</source>
        <translation>软件兼容</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="721"/>
        <source>Lock Configuration</source>
        <translation>锁定配置</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="740"/>
        <source>Sandbox %1</source>
        <translation>沙盒 %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="783"/>
        <source>New-Box Menu</source>
        <translation type="unfinished">新建沙盒菜单</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="792"/>
        <location filename="SandMan.cpp" line="930"/>
        <location filename="SandMan.cpp" line="931"/>
        <source>Cleanup</source>
        <translation>清理</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="800"/>
        <source>Edit-ini Menu</source>
        <translation>编辑配置菜单</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="844"/>
        <source>Toolbar Items</source>
        <translation>工具栏项目</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="846"/>
        <source>Reset Toolbar</source>
        <translation>重置工具栏</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1031"/>
        <source>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Support Sandboxie-Plus on Patreon&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;在 Patreon 上捐赠 Sandboxie-Plus&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1032"/>
        <source>Click to open web browser</source>
        <translation>在浏览器打开捐赠页面</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1211"/>
        <source>Time|Box Name|File Path</source>
        <translation>时间|沙盒名称|文件路径</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="559"/>
        <location filename="SandMan.cpp" line="667"/>
        <location filename="SandMan.cpp" line="1221"/>
        <source>Recovery Log</source>
        <translation>恢复日志</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1023"/>
        <source>Click to download update</source>
        <translation>点击下载更新</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1265"/>
        <source>Do you want to close Sandboxie Manager?</source>
        <translation>确定要关闭 Sandboxie 管理器？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1282"/>
        <source>Sandboxie-Plus was running in portable mode, now it has to clean up the created services. This will prompt for administrative privileges.

Do you want to do the clean up?</source>
        <translation>Sandboxie-Plus 正运行于便携模式，现在将清理所创建的服务，这将寻求管理员权限

是否确认清理？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1283"/>
        <location filename="SandMan.cpp" line="1696"/>
        <location filename="SandMan.cpp" line="2027"/>
        <location filename="SandMan.cpp" line="2635"/>
        <location filename="SandMan.cpp" line="3063"/>
        <location filename="SandMan.cpp" line="3079"/>
        <source>Don&apos;t show this message again.</source>
        <translation>不再显示此消息</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1452"/>
        <source>This box provides &lt;a href=&quot;sbie://docs/security-mode&quot;&gt;enhanced security isolation&lt;/a&gt;, it is suitable to test untrusted software.</source>
        <oldsource>This box provides enhanced security isolation, it is suitable to test untrusted software.</oldsource>
        <translation>此类沙盒提供增强的安全隔离，它适用于测试不受信任的软件</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1456"/>
        <source>This box provides standard isolation, it is suitable to run your software to enhance security.</source>
        <translation>此类沙盒提供标准的隔离，它适用于以安全的方式来运行你的软件</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1460"/>
        <source>This box does not enforce isolation, it is intended to be used as an &lt;a href=&quot;sbie://docs/compartment-mode&quot;&gt;application compartment&lt;/a&gt; for software virtualization only.</source>
        <oldsource>This box does not enforce isolation, it is intended to be used as an application compartment for software virtualization only.</oldsource>
        <translation>此类沙盒不执行隔离，它的目的是将一个应用程序虚拟化</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1465"/>
        <source>&lt;br /&gt;&lt;br /&gt;This box &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;prevents access to all user data&lt;/a&gt; locations, except explicitly granted in the Resource Access options.</source>
        <oldsource>

This box &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;prevents access to all user data&lt;/a&gt; locations, except explicitly granted in the Resource Access options.</oldsource>
        <translation>此类沙盒将限制沙盒内程序对沙盒外数据的访问，除非在资源访问选项中明确授权</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1553"/>
        <source>Unknown operation &apos;%1&apos; requested via command line</source>
        <translation>来自命令行的未知操作请求 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="105"/>
        <source>Dismiss Update Notification</source>
        <translation>忽略更新通知</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="181"/>
        <source> - Driver/Service NOT Running!</source>
        <translation> - 驱动程序/服务未运行!</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="183"/>
        <source> - Deleting Sandbox Content</source>
        <translation> - 正在删除沙盒内容</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1763"/>
        <source>Executing OnBoxDelete: %1</source>
        <translation>在删除沙盒时执行: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1784"/>
        <source>Auto Deleting %1 Content</source>
        <translation>自动删除 %1 的内容</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1856"/>
        <source>Removed Shortcut: %1</source>
        <translation>移除快捷方式: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1935"/>
        <source>Updated Shortcut to: %1</source>
        <translation>更新快捷方式到: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1937"/>
        <source>Added Shortcut to: %1</source>
        <translation>添加快捷方式: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1969"/>
        <source>Auto deleting content of %1</source>
        <translation>自动删除 %1 的内容</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1988"/>
        <source>Auto removing sandbox %1</source>
        <translation>自动删除沙盒 %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>%1 Directory: %2</source>
        <translation>%1 目录: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>Application</source>
        <translation>应用程序</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>Installation</source>
        <translation>安装</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2771"/>
        <location filename="SandMan.cpp" line="2773"/>
        <location filename="SandMan.cpp" line="3322"/>
        <source>Sandboxie-Plus - Error</source>
        <translation>Sandboxie-Plus - 错误</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2771"/>
        <source>Failed to stop all Sandboxie components</source>
        <translation>停止全部的 Sandboxie 组件失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2773"/>
        <source>Failed to start required Sandboxie components</source>
        <translation>启动所需的 Sandboxie 组件失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3136"/>
        <source>Sandboxie config has been reloaded</source>
        <translation>已重载沙盒配置文件</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2481"/>
        <source>The supporter certificate is not valid for this build, please get an updated certificate</source>
        <translation>此赞助者凭据对该版本沙盒无效，请获取可用的新凭据</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2484"/>
        <source>The supporter certificate has expired%1, please get an updated certificate</source>
        <translation>此赞助者凭据已过期%1，请获取可用的新凭据</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2485"/>
        <source>, but it remains valid for the current build</source>
        <translation>，但它对当前构建的沙盒版本仍然有效</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2487"/>
        <source>The supporter certificate will expire in %1 days, please get an updated certificate</source>
        <translation>此赞助者凭据将在 %1 天后过期，请获取可用的新凭据</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2407"/>
        <source>The selected feature set is only available to project supporters. Processes started in a box with this feature set enabled without a supporter certificate will be terminated after 5 minutes.&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Become a project supporter&lt;/a&gt;, and receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;</source>
        <translation>选定的特性只对项目赞助者可用。如果没有赞助者凭据，在启用该特性的沙盒里启动的进程，将在 5 分钟后被终止。&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;成为项目赞助者&lt;/a&gt;，以获得&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;赞助者凭据&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3257"/>
        <source>Error Status: 0x%1 (%2)</source>
        <translation>错误状态: 0x%1 (%2)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3258"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3286"/>
        <source>Failed to remove old box data files</source>
        <translation>无法删除旧沙盒中的数据文件</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3292"/>
        <source>The operation was canceled by the user</source>
        <translation>该操作已被用户取消</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3300"/>
        <source>Unknown Error Status: 0x%1</source>
        <translation>未知错误状态: 0x%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3507"/>
        <source>Case Sensitive</source>
        <translation>区分大小写</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3508"/>
        <source>RegExp</source>
        <translation>正则表达式</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3509"/>
        <source>Highlight</source>
        <translation>高亮显示</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3510"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3511"/>
        <source>&amp;Find ...</source>
        <translation>查找(&amp;F)...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3512"/>
        <source>All columns</source>
        <translation>所有列</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3561"/>
        <source>&lt;h3&gt;About Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;Version %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2023 by DavidXanatos&lt;/p&gt;</source>
        <oldsource>&lt;h3&gt;About Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;Version %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2022 by DavidXanatos&lt;/p&gt;</oldsource>
        <translation>&lt;h3&gt;关于 Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;版本 %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2023 by DavidXanatos&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3569"/>
        <source>This copy of Sandboxie+ is certified for: %1</source>
        <translation>此 Sandboxie+ 副本已授权给: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3571"/>
        <source>Sandboxie+ is free for personal and non-commercial use.</source>
        <translation>Sandboxie+ 可免费用于个人和非商业用途</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3574"/>
        <source>Sandboxie-Plus is an open source continuation of Sandboxie.&lt;br /&gt;Visit &lt;a href=&quot;https://sandboxie-plus.com&quot;&gt;sandboxie-plus.com&lt;/a&gt; for more information.&lt;br /&gt;&lt;br /&gt;%3&lt;br /&gt;&lt;br /&gt;Driver version: %1&lt;br /&gt;Features: %2&lt;br /&gt;&lt;br /&gt;Icons from &lt;a href=&quot;https://icons8.com&quot;&gt;icons8.com&lt;/a&gt;</source>
        <translation>Sandboxie-Plus 是知名程序 Sandboxie 自开源以来的一个延续&lt;br /&gt;访问 &lt;a href=&quot;https://sandboxie-plus.com&quot;&gt;sandboxie-plus.com&lt;/a&gt; 来了解更多信息&lt;br /&gt;&lt;br /&gt;%3&lt;br /&gt;&lt;br /&gt;驱动版本: %1&lt;br /&gt;特性: %2&lt;br /&gt;&lt;br /&gt;图标来源: &lt;a href=&quot;https://icons8.com&quot;&gt;icons8.com&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2100"/>
        <source>Default sandbox not found; creating: %1</source>
        <translation>未找到默认沙盒，正在创建：%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="165"/>
        <source>WARNING: Sandboxie-Plus.ini in %1 cannot be written to, settings will not be saved.</source>
        <translation>警告: %1 中的 Sandboxie-Plus.ini 不能被写入，设置将不会被保存</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="566"/>
        <location filename="SandMan.cpp" line="568"/>
        <location filename="SandMan.cpp" line="616"/>
        <location filename="SandMan.cpp" line="618"/>
        <source>Pause Forcing Programs</source>
        <translation>停用必沙程序规则</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1014"/>
        <source>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus release %1 ready&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;存在一个新的 Sandboxie-Plus 版本 %1 已准备就绪&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1018"/>
        <source>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update %1 ready&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;存在一个新的 Sandboxie-Plus 更新 %1 已准备就绪&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1694"/>
        <source>Some compatibility templates (%1) are missing, probably deleted, do you want to remove them from all boxes?</source>
        <translation>部分兼容性模板(%1)丢失，可能已被删除，是否要在所有沙盒中移除？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1711"/>
        <source>Cleaned up removed templates...</source>
        <translation>已清理缺失的模板...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2024"/>
        <source>Sandboxie-Plus was started in portable mode, do you want to put the Sandbox folder into its parent directory?
Yes will choose: %1
No will choose: %2</source>
        <translation>Sandboxie-Plus 运行于便携模式，是否要将沙盒目录放到上一层目录中？
“是”将选择目录: %1
“否”将选择目录: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2140"/>
        <source>   -   NOT connected</source>
        <translation>   -   未连接</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2338"/>
        <source>The program %1 started in box %2 will be terminated in 5 minutes because the box was configured to use features exclusively available to project supporters.</source>
        <translation>在沙盒 %2 中启动的程序 %1 将在 5 分钟之后自动终止，因为此沙盒被配置为使用项目赞助者的特供功能</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2340"/>
        <source>The box %1 is configured to use features exclusively available to project supporters, these presets will be ignored.</source>
        <translation>沙盒 %1 被配置为使用项目赞助者专有的沙盒类型，这些预设选项将被忽略</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2341"/>
        <source>&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Become a project supporter&lt;/a&gt;, and receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;</source>
        <translation>&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;成为项目赞助者&lt;/a&gt;，以获得&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;赞助者凭据&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2376"/>
        <source>PID %1: </source>
        <translation>进程 PID %1: </translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2378"/>
        <source>%1 (%2): </source>
        <translation>%1 (%2): </translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="202"/>
        <source>Recovering file %1 to %2</source>
        <translation>恢复文件 %1 到 %2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="214"/>
        <source>The file %1 already exists, do you want to overwrite it?</source>
        <translation>文件 %1 已存在，要覆盖它吗？</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="179"/>
        <location filename="SandManRecovery.cpp" line="215"/>
        <source>Do this for all files!</source>
        <translation>对所有文件都执行此操作！</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="99"/>
        <location filename="SandManRecovery.cpp" line="159"/>
        <source>Checking file %1</source>
        <translation>正在检查文件 %1</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="107"/>
        <source>The file %1 failed a security check!

%2</source>
        <oldsource>The file %1 failed a security check!

%2</oldsource>
        <translation>文件 %1 未通过安全检查!

%2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="115"/>
        <source>All files passed the checks</source>
        <translation>所有文件都通过了检查</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="178"/>
        <source>The file %1 failed a security check, do you want to recover it anyway?

%2</source>
        <oldsource>The file %1 failed a security check, do you want to recover it anyway?

%2</oldsource>
        <translation>文件 %1 未通过安全检查，您要忽略警告并恢复吗？

%2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="243"/>
        <source>Failed to recover some files: 
</source>
        <translation>部分文件恢复失败: 
</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2528"/>
        <source>Only Administrators can change the config.</source>
        <translation>仅管理员可更改该配置</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2538"/>
        <source>Please enter the configuration password.</source>
        <translation>请输入配置保护密码</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2546"/>
        <source>Login Failed: %1</source>
        <translation>登录失败：%1</translation>
    </message>
    <message>
        <source>Select file name</source>
        <translation type="vanished">选择文件名</translation>
    </message>
    <message>
        <source>7-zip Archive (*.7z)</source>
        <translation type="vanished">7-zip 压缩包 (*.7z)</translation>
    </message>
    <message>
        <source>This name is already in use, please select an alternative box name</source>
        <oldsource>This Name is already in use, please select an alternative box name</oldsource>
        <translation type="vanished">名称已占用，请选择其他沙盒名</translation>
    </message>
    <message>
        <source>Importing: %1</source>
        <translation type="vanished">正在导入：%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2575"/>
        <source>Do you want to terminate all processes in all sandboxes?</source>
        <translation>确定要终止所有沙盒中的所有进程吗？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2576"/>
        <source>Terminate all without asking</source>
        <translation>终止所有且不再询问</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2610"/>
        <source>No Recovery</source>
        <translation>没有恢复文件</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2616"/>
        <source>No Messages</source>
        <translation>没有消息</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2634"/>
        <source>Sandboxie-Plus was started in portable mode and it needs to create necessary services. This will prompt for administrative privileges.</source>
        <translation>Sandboxie-Plus 正以便携模式启动，需要创建所需的服务，这将会寻求管理员权限</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2668"/>
        <source>CAUTION: Another agent (probably SbieCtrl.exe) is already managing this Sandboxie session, please close it first and reconnect to take over.</source>
        <translation>警告：另一代理程序 (可能是 SbieCtrl.exe) 已接管当前 Sandboxie 会话，请将其关闭，然后尝试重新连接以接管控制</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2672"/>
        <source>&lt;b&gt;ERROR:&lt;/b&gt; The Sandboxie-Plus Manager (SandMan.exe) does not have a valid signature (SandMan.exe.sig). Please download a trusted release from the &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get&quot;&gt;official Download page&lt;/a&gt;.</source>
        <translation>&lt;b&gt;错误：&lt;/b&gt;Sandboxie Plus管理器（SandMan.exe）没有有效的签名（SandMan.exe.sig）。请从&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get&quot;&gt;官方下载&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2780"/>
        <source>Maintenance operation completed</source>
        <translation>维护作业完成</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2800"/>
        <source>Executing maintenance operation, please wait...</source>
        <translation>正在执行操作维护，请稍候...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2911"/>
        <source>In the Plus UI, this functionality has been integrated into the main sandbox list view.</source>
        <translation>在 Plus 视图，此功能已被整合到主沙盒列表中</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2912"/>
        <source>Using the box/group context menu, you can move boxes and groups to other groups. You can also use drag and drop to move the items around. Alternatively, you can also use the arrow keys while holding ALT down to move items up and down within their group.&lt;br /&gt;You can create new boxes and groups from the Sandbox menu.</source>
        <translation>使用“沙盒/组”右键菜单，你可以将沙盒在沙盒组之间移动
同时，你也可以通过 Alt + 方向键或鼠标拖动来整理列表
另外，你可以通过右键菜单来新建“沙盒/组”</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2980"/>
        <source>Do you also want to reset hidden message boxes (yes), or only all log messages (no)?</source>
        <translation>请确认是否要重置已隐藏的消息框(选“是”)，或者仅重置所有日志消息(选“否”)？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3061"/>
        <source>You are about to edit the Templates.ini, this is generally not recommended.
This file is part of Sandboxie and all change done to it will be reverted next time Sandboxie is updated.</source>
        <oldsource>You are about to edit the Templates.ini, thsi is generally not recommeded.
This file is part of Sandboxie and all changed done to it will be reverted next time Sandboxie is updated.</oldsource>
        <translation>您正准备编辑模板配置文件, 但通常不推荐这么做
因为该文件是 Sandboxie 的一部分并且所有的更改会在下次更新时被重置</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3077"/>
        <source>The changes will be applied automatically whenever the file gets saved.</source>
        <translation>每次该文件被保存时，更改将自动应用</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3078"/>
        <source>The changes will be applied automatically as soon as the editor is closed.</source>
        <translation>编辑器被关闭后，更改将很快自动应用</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3260"/>
        <source>Administrator rights are required for this operation.</source>
        <translation>此操作需要管理员权限</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3261"/>
        <source>Failed to execute: %1</source>
        <translation>执行失败：%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3262"/>
        <source>Failed to connect to the driver</source>
        <translation>连接驱动程序失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3263"/>
        <source>Failed to communicate with Sandboxie Service: %1</source>
        <translation>无法与 Sandboxie 服务通信：%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3264"/>
        <source>An incompatible Sandboxie %1 was found. Compatible versions: %2</source>
        <translation>发现不兼容的 Sandboxie %1，其它兼容的版本：%2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3265"/>
        <source>Can&apos;t find Sandboxie installation path.</source>
        <translation>无法找到 Sandboxie 的安装路径</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3266"/>
        <source>Failed to copy configuration from sandbox %1: %2</source>
        <translation>复制沙盒配置 %1: %2 失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3267"/>
        <source>A sandbox of the name %1 already exists</source>
        <translation>名为 %1 的沙盒已存在</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3268"/>
        <source>Failed to delete sandbox %1: %2</source>
        <translation>删除沙盒 %1: %2 失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3269"/>
        <source>The sandbox name can not be longer than 32 characters.</source>
        <translation>沙盒名称不能超过 32 个字符</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3270"/>
        <source>The sandbox name can not be a device name.</source>
        <translation>沙盒名称不能为设备名称</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3271"/>
        <source>The sandbox name can contain only letters, digits and underscores which are displayed as spaces.</source>
        <translation>沙盒名称只能包含字母、数字和下划线(显示为空格)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3272"/>
        <source>Failed to terminate all processes</source>
        <translation>终止所有进程失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3273"/>
        <source>Delete protection is enabled for the sandbox</source>
        <translation>该沙盒已启用删除保护</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3274"/>
        <source>All sandbox processes must be stopped before the box content can be deleted</source>
        <translation>在删除沙盒内容之前，必须先停止沙盒内的所有进程</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3275"/>
        <source>Error deleting sandbox folder: %1</source>
        <translation>删除沙盒文件夹出错：%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3277"/>
        <source>A sandbox must be emptied before it can be deleted.</source>
        <translation>沙盒被删除前必须清空</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3278"/>
        <source>Failed to move directory &apos;%1&apos; to &apos;%2&apos;</source>
        <translation>移动目录 &apos;%1&apos; 到 &apos;%2&apos; 失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3279"/>
        <source>This Snapshot operation can not be performed while processes are still running in the box.</source>
        <translation>因有进程正在沙盒中运行，此快照操作无法完成</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3280"/>
        <source>Failed to create directory for new snapshot</source>
        <translation>创建新快照的目录失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2004"/>
        <source>Sandboxie-Plus Version: %1 (%2)</source>
        <translation>Sandboxie-Plus 版本: %1 (%2)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2005"/>
        <source>Current Config: %1</source>
        <translation>当前配置: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2006"/>
        <source>Data Directory: %1</source>
        <translation>数据存放目录: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2069"/>
        <source> for Personal use</source>
        <translation>个人用户</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2071"/>
        <source>   -   for Non-Commercial use ONLY</source>
        <translation>   -   仅用于非商业用途</translation>
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
        <oldsource>The evaluation periode has expired!!!</oldsource>
        <translation>已超过评估期限！！！</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2595"/>
        <source>Please enter the duration, in seconds, for disabling Forced Programs rules.</source>
        <translation>请输入「停用必沙程序规则」的持续时间 (单位: 秒)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2775"/>
        <source>Maintenance operation failed (%1)</source>
        <translation>维护作业执行失败 (%1)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3281"/>
        <source>Failed to copy box data files</source>
        <translation>复制沙盒数据文件失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3282"/>
        <source>Snapshot not found</source>
        <translation>没有找到快照</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3283"/>
        <source>Error merging snapshot directories &apos;%1&apos; with &apos;%2&apos;, the snapshot has not been fully merged.</source>
        <translation>合并快照目录 &apos;%1&apos; 和 &apos;%2&apos; 出错，快照没有被完全合并</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3284"/>
        <source>Failed to remove old snapshot directory &apos;%1&apos;</source>
        <translation>移除旧快照的目录 &apos;%1&apos; 失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3285"/>
        <source>Can&apos;t remove a snapshot that is shared by multiple later snapshots</source>
        <translation>无法移除被多个后续快照所共享的快照</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3287"/>
        <source>You are not authorized to update configuration in section &apos;%1&apos;</source>
        <translation>您未被授权在 &apos;%1&apos; 更新配置</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3288"/>
        <source>Failed to set configuration setting %1 in section %2: %3</source>
        <translation>在 %2: %3 中设定配置设置 %1 失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3289"/>
        <source>Can not create snapshot of an empty sandbox</source>
        <translation>无法为空的沙盒创建快照</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3290"/>
        <source>A sandbox with that name already exists</source>
        <translation>已存在同名沙盒</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3291"/>
        <source>The config password must not be longer than 64 characters</source>
        <translation>配置保护密码长度不能超过 64 个字符</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3294"/>
        <source>Import/Export not available, 7z.dll could not be loaded</source>
        <translation>导入/导出不可用，无法加载 7z.dll</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3295"/>
        <source>Failed to create the box archive</source>
        <translation>无法创建沙盒存档</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3296"/>
        <source>Failed to open the 7z archive</source>
        <translation>无法打开7z存档</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3297"/>
        <source>Failed to unpack the box archive</source>
        <translation>无法解压沙盒存档</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3298"/>
        <source>The selected 7z file is NOT a box archive</source>
        <translation>所选的 7z 文件不是沙盒存档</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3324"/>
        <source>Operation failed for %1 item(s).</source>
        <translation>%1 项操作失败</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3355"/>
        <source>Do you want to open %1 in a sandboxed (yes) or unsandboxed (no) Web browser?</source>
        <translation>是否在沙盒中的浏览器打开链接 %1 ？</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3356"/>
        <source>Remember choice for later.</source>
        <translation>记住选择供之后使用</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="88"/>
        <source>The selected window is running as part of program %1 in sandbox %2</source>
        <translation>选择的窗口正作为程序 %1 的一部分，并运行在沙盒 %2 中</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="95"/>
        <source>The selected window is not running as part of any sandboxed program.</source>
        <translation>选择的窗口并未作为任何沙盒化程序的一部分而运行</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="134"/>
        <source>Drag the Finder Tool over a window to select it, then release the mouse to check if the window is sandboxed.</source>
        <translation>拖拽准星到被选窗口上，松开鼠标检查窗口是否来自沙盒化的程序</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="204"/>
        <source>Sandboxie-Plus - Window Finder</source>
        <translation>Sandboxie-Plus - 窗口探查器</translation>
    </message>
    <message>
        <location filename="main.cpp" line="123"/>
        <source>Sandboxie Manager can not be run sandboxed!</source>
        <translation>Sandboxie 管理器不能在沙盒中运行！</translation>
    </message>
</context>
<context>
    <name>CSbieModel</name>
    <message>
        <location filename="Models/SbieModel.cpp" line="159"/>
        <source>Box Group</source>
        <translation>沙盒组</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="315"/>
        <source>Empty</source>
        <translation>空</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="559"/>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="560"/>
        <source>Process ID</source>
        <translation>进程 ID</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="561"/>
        <source>Status</source>
        <translation>状态</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="562"/>
        <source>Title</source>
        <translation>标题</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="563"/>
        <source>Info</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="567"/>
        <source>Path / Command Line</source>
        <translation>路径 / 命令行</translation>
    </message>
</context>
<context>
    <name>CSbieProcess</name>
    <message>
        <location filename="SbieProcess.cpp" line="59"/>
        <source>Sbie RpcSs</source>
        <translation>Sbie RPC 子系统</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="60"/>
        <source>Sbie DcomLaunch</source>
        <translation>Sbie DCOM 服务启动器</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="61"/>
        <source>Sbie Crypto</source>
        <translation>Sbie 密码学服务</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="62"/>
        <source>Sbie WuauServ</source>
        <translation>Sbie 微软自动更新服务</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="63"/>
        <source>Sbie BITS</source>
        <translation>Sbie 后台智能传输服务</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="64"/>
        <source>Sbie Svc</source>
        <translation>沙盒软件服务</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="65"/>
        <source>MSI Installer</source>
        <oldsource>Msi Installer</oldsource>
        <translation>MSI 安装程序</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="66"/>
        <source>Trusted Installer</source>
        <translation>可信安装程序</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="67"/>
        <source>Windows Update</source>
        <translation>Windows 更新</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="68"/>
        <source>Windows Explorer</source>
        <translation>Windows 资源管理器</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="69"/>
        <source>Internet Explorer</source>
        <translation>Internet Explorer</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="70"/>
        <source>Firefox</source>
        <oldsource>FireFox</oldsource>
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
        <oldsource>WinAmp</oldsource>
        <translation>Winamp</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="73"/>
        <source>KMPlayer</source>
        <oldsource>KM Player</oldsource>
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
        <translation>服务模型注册表</translation>
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
        <translation>Windows 快捷方式服务</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="80"/>
        <source>Chromium Based</source>
        <translation>基于 Chromium 的浏览器</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="81"/>
        <source>Google Updater</source>
        <translation>Google 更新程序</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="82"/>
        <source>Acrobat Reader</source>
        <translation>Acrobat Reader</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="83"/>
        <source>MS Outlook</source>
        <translation>微软 Outlook</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="84"/>
        <source>MS Excel</source>
        <translation>微软 Excel</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="85"/>
        <source>Flash Player</source>
        <translation>Flash Player</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="86"/>
        <source>Firefox Plugin Container</source>
        <oldsource>FireFox Plugin Container</oldsource>
        <translation>Firefox 插件容器</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="87"/>
        <source>Generic Web Browser</source>
        <translation>常规网络浏览器</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="88"/>
        <source>Generic Mail Client</source>
        <translation>常规电子邮件客户端</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="89"/>
        <source>Thunderbird</source>
        <translation>Thunderbird</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="99"/>
        <source>Terminated</source>
        <translation>已终止</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="105"/>
        <source>Forced </source>
        <translation>必沙 </translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="103"/>
        <source>Running</source>
        <translation>正在运行</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="109"/>
        <source> Elevated</source>
        <translation> 管理员</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="111"/>
        <source> as System</source>
        <translation> 系统权限</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="114"/>
        <source> in session %1</source>
        <translation> 位于会话 %1</translation>
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
        <translation>新建沙盒</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="397"/>
        <source>Remove Group</source>
        <translation>移除组</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="161"/>
        <location filename="Views/SbieView.cpp" line="291"/>
        <source>Run</source>
        <translation>运行</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="166"/>
        <source>Run Program</source>
        <translation>运行程序(Run)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="167"/>
        <source>Run from Start Menu</source>
        <translation>从开始菜单运行</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="178"/>
        <source>Default Web Browser</source>
        <translation>默认浏览器</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="179"/>
        <source>Default eMail Client</source>
        <translation>默认电子邮件客户端</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="181"/>
        <source>Windows Explorer</source>
        <translation>Windows 文件资源管理器</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="182"/>
        <source>Registry Editor</source>
        <translation>注册表编辑器</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="183"/>
        <source>Programs and Features</source>
        <translation>程序和功能</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="194"/>
        <source>Terminate All Programs</source>
        <translation>终止所有程序</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="102"/>
        <location filename="Views/SbieView.cpp" line="200"/>
        <location filename="Views/SbieView.cpp" line="259"/>
        <location filename="Views/SbieView.cpp" line="339"/>
        <location filename="Views/SbieView.cpp" line="378"/>
        <source>Create Shortcut</source>
        <translation>创建快捷方式</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="202"/>
        <location filename="Views/SbieView.cpp" line="324"/>
        <source>Explore Content</source>
        <translation>浏览内容</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="204"/>
        <location filename="Views/SbieView.cpp" line="331"/>
        <source>Snapshots Manager</source>
        <translation>快照管理器</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="205"/>
        <source>Recover Files</source>
        <translation>文件恢复</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="206"/>
        <location filename="Views/SbieView.cpp" line="323"/>
        <source>Delete Content</source>
        <translation>删除内容</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="213"/>
        <source>Sandbox Presets</source>
        <translation>预置配置</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="215"/>
        <source>Ask for UAC Elevation</source>
        <translation>询问 UAC 提权</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="216"/>
        <source>Drop Admin Rights</source>
        <translation>撤销管理员权限</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="217"/>
        <source>Emulate Admin Rights</source>
        <translation>模拟管理员权限</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="225"/>
        <source>Block Internet Access</source>
        <translation>拦截网络访问</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="227"/>
        <source>Allow Network Shares</source>
        <translation>允许网络共享</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="208"/>
        <source>Sandbox Options</source>
        <translation>沙盒选项</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="177"/>
        <source>Standard Applications</source>
        <translation>标准应用程序</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="197"/>
        <source>Browse Files</source>
        <translation>浏览文件</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="236"/>
        <location filename="Views/SbieView.cpp" line="329"/>
        <source>Sandbox Tools</source>
        <translation>沙盒工具</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="237"/>
        <source>Duplicate Box Config</source>
        <translation>复制配置</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="240"/>
        <location filename="Views/SbieView.cpp" line="342"/>
        <source>Rename Sandbox</source>
        <translation>重命名沙盒</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="241"/>
        <location filename="Views/SbieView.cpp" line="343"/>
        <source>Move Sandbox</source>
        <translation>移动沙盒</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="252"/>
        <location filename="Views/SbieView.cpp" line="354"/>
        <source>Remove Sandbox</source>
        <translation>移除沙盒</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="257"/>
        <location filename="Views/SbieView.cpp" line="376"/>
        <source>Terminate</source>
        <translation>终止</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="260"/>
        <source>Preset</source>
        <translation>预设</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="100"/>
        <location filename="Views/SbieView.cpp" line="261"/>
        <source>Pin to Run Menu</source>
        <translation>固定到运行菜单</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="263"/>
        <source>Block and Terminate</source>
        <translation>阻止并终止</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="267"/>
        <source>Allow internet access</source>
        <translation>允许网络访问</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="269"/>
        <source>Force into this sandbox</source>
        <translation>强制入此沙盒</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="271"/>
        <source>Set Linger Process</source>
        <translation>设置驻留进程</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="273"/>
        <source>Set Leader Process</source>
        <translation>设置引导进程</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="536"/>
        <source>    File root: %1
</source>
        <translation>    文件根目录: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="537"/>
        <source>    Registry root: %1
</source>
        <translation>    注册表根: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="538"/>
        <source>    IPC root: %1
</source>
        <translation>    IPC 根: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="540"/>
        <source>Options:
    </source>
        <translation>选项:
    </translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="748"/>
        <source>[None]</source>
        <translation>[无]</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1059"/>
        <source>Please enter a new group name</source>
        <translation>请输入新的组名</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="857"/>
        <source>Do you really want to remove the selected group(s)?</source>
        <translation>确定要移除选中的组吗？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="152"/>
        <location filename="Views/SbieView.cpp" line="282"/>
        <source>Create Box Group</source>
        <translation>新建沙盒组</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="395"/>
        <source>Rename Group</source>
        <translation>重命名组</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="157"/>
        <location filename="Views/SbieView.cpp" line="287"/>
        <source>Stop Operations</source>
        <translation>停止作业</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="186"/>
        <source>Command Prompt</source>
        <translation>命令提示符</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="187"/>
        <source>Command Prompt (as Admin)</source>
        <translation>命令提示符 (管理员)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="191"/>
        <source>Command Prompt (32-bit)</source>
        <translation>命令提示符 (32 位)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="176"/>
        <source>Execute Autorun Entries</source>
        <translation>执行自动运行项目</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="330"/>
        <source>Browse Content</source>
        <translation>浏览内容</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="196"/>
        <source>Box Content</source>
        <translation>沙盒内容</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="203"/>
        <source>Open Registry</source>
        <translation>打开注册表</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="199"/>
        <location filename="Views/SbieView.cpp" line="338"/>
        <source>Refresh Info</source>
        <translation>刷新信息</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="153"/>
        <location filename="Views/SbieView.cpp" line="283"/>
        <source>Import Box</source>
        <translation>导入沙盒</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="170"/>
        <location filename="Views/SbieView.cpp" line="302"/>
        <source>(Host) Start Menu</source>
        <translation>开始菜单(宿主)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="231"/>
        <source>Immediate Recovery</source>
        <translation>即时恢复</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="233"/>
        <source>Disable Force Rules</source>
        <translation>禁用“强制规则”</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="238"/>
        <source>Export Box</source>
        <translation>导出沙盒</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="242"/>
        <location filename="Views/SbieView.cpp" line="344"/>
        <source>Move Up</source>
        <translation>上移</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="247"/>
        <location filename="Views/SbieView.cpp" line="349"/>
        <source>Move Down</source>
        <translation>下移</translation>
    </message>
    <message>
        <source>Run Sandboxed</source>
        <translation type="vanished">运行</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="296"/>
        <source>Run Web Browser</source>
        <translation>默认浏览器</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="297"/>
        <source>Run eMail Reader</source>
        <translation>默认电子邮件客户端</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="298"/>
        <source>Run Any Program</source>
        <translation>运行程序(Run)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="299"/>
        <source>Run From Start Menu</source>
        <translation>从开始菜单运行</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="307"/>
        <source>Run Windows Explorer</source>
        <translation>Windows 资源管理器</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="321"/>
        <source>Terminate Programs</source>
        <translation>终止程序</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="322"/>
        <source>Quick Recover</source>
        <translation>快速恢复</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="327"/>
        <source>Sandbox Settings</source>
        <translation>沙盒配置</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="334"/>
        <source>Duplicate Sandbox Config</source>
        <translation>复制配置</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="335"/>
        <source>Export Sandbox</source>
        <translation>导出沙盒</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="396"/>
        <source>Move Group</source>
        <translation>移动组</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="847"/>
        <source>Please enter a new name for the Group.</source>
        <translation>为此组指定新的名称</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="894"/>
        <source>Move entries by (negative values move up, positive values move down):</source>
        <translation>将项目移动的距离(负数向上移动，正数向下移动):</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="942"/>
        <source>A group can not be its own parent.</source>
        <translation>组不能是自己的父级</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1031"/>
        <source>This name is already in use, please select an alternative box name</source>
        <translation>名称已占用，请选择其他沙盒名</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1045"/>
        <source>Importing: %1</source>
        <translation>正在导入：%1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1085"/>
        <source>The Sandbox name and Box Group name cannot use the &apos;,()&apos; symbol.</source>
        <translation>⌈沙盒/组⌋名称不能使用 &apos;,()&apos; 等符号</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1090"/>
        <source>This name is already used for a Box Group.</source>
        <translation>名称已被用于现有的其它沙盒组</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1095"/>
        <source>This name is already used for a Sandbox.</source>
        <translation>名称已被用于现有的其它沙盒</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1145"/>
        <location filename="Views/SbieView.cpp" line="1215"/>
        <location filename="Views/SbieView.cpp" line="1462"/>
        <source>Don&apos;t show this message again.</source>
        <translation>不再显示此消息</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1206"/>
        <location filename="Views/SbieView.cpp" line="1226"/>
        <location filename="Views/SbieView.cpp" line="1630"/>
        <source>This Sandbox is empty.</source>
        <translation>此沙盒是空的</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1233"/>
        <source>WARNING: The opened registry editor is not sandboxed, please be careful and only do changes to the pre-selected sandbox locations.</source>
        <translation>警告：打开的注册表编辑器未沙盒化，请审慎且仅对预先选定的沙盒节点进行修改</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1234"/>
        <source>Don&apos;t show this warning in future</source>
        <translation>不再显示此警告</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1290"/>
        <source>Please enter a new name for the duplicated Sandbox.</source>
        <translation>请为此复制的沙盒输入一个新名称</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1290"/>
        <source>%1 Copy</source>
        <translatorcomment>沙盒名称只能包含字母、数字和下划线，不应对此处的文本进行翻译！</translatorcomment>
        <translation>%1 Copy</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1018"/>
        <location filename="Views/SbieView.cpp" line="1329"/>
        <source>Select file name</source>
        <translation>选择文件名</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1018"/>
        <location filename="Views/SbieView.cpp" line="1329"/>
        <source>7-zip Archive (*.7z)</source>
        <translation>7-zip 压缩包 (*.7z)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1337"/>
        <source>Exporting: %1</source>
        <translation>正在导出：%1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1344"/>
        <source>Please enter a new name for the Sandbox.</source>
        <translation>请为该沙盒输入新名称</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1365"/>
        <source>Do you really want to remove the selected sandbox(es)?&lt;br /&gt;&lt;br /&gt;Warning: The box content will also be deleted!</source>
        <translation>确定要删除选中的沙盒？&lt;br /&gt;&lt;br /&gt;警告：沙盒内的内容也将被删除！</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1403"/>
        <source>This Sandbox is already empty.</source>
        <translation>此沙盒已清空</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1413"/>
        <source>Do you want to delete the content of the selected sandbox?</source>
        <translation>确定要删除选中沙盒的内容吗？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1414"/>
        <location filename="Views/SbieView.cpp" line="1418"/>
        <source>Also delete all Snapshots</source>
        <translation>同时删除所有快照</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1417"/>
        <source>Do you really want to delete the content of all selected sandboxes?</source>
        <translation>你真的想删除所有选定的沙盒的内容吗？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1444"/>
        <source>Do you want to terminate all processes in the selected sandbox(es)?</source>
        <translation>确定要终止所选沙盒中的所有进程吗？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1445"/>
        <location filename="Views/SbieView.cpp" line="1539"/>
        <source>Terminate without asking</source>
        <translation>终止且不再询问</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1460"/>
        <source>The Sandboxie Start Menu will now be displayed. Select an application from the menu, and Sandboxie will create a new shortcut icon on your real desktop, which you can use to invoke the selected application under the supervision of Sandboxie.</source>
        <oldsource>The Sandboxie Start Menu will now be displayed. Select an application from the menu, and Sandboxie will create a newshortcut icon on your real desktop, which you can use to invoke the selected application under the supervision of Sandboxie.</oldsource>
        <translation>现在将显示 Sandboxie 开始菜单。从菜单中选择一个应用程序，Sandboxie 将在真实桌面上创建一个新的快捷方式图标，你可以用它来调用所选受 Sandboxie 监督的应用程序。</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1514"/>
        <location filename="Views/SbieView.cpp" line="1566"/>
        <source>Create Shortcut to sandbox %1</source>
        <translation>为沙盒 %1 创建快捷方式</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1538"/>
        <source>Do you want to terminate %1?</source>
        <oldsource>Do you want to %1 %2?</oldsource>
        <translation>确定要终止 %1？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1538"/>
        <source>the selected processes</source>
        <translation>选中的进程</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1589"/>
        <source>This box does not have Internet restrictions in place, do you want to enable them?</source>
        <translation>此沙盒无互联网限制，确定启用吗？</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1673"/>
        <source>This sandbox is disabled, do you want to enable it?</source>
        <translation>此沙盒已禁用，确定启用吗？</translation>
    </message>
</context>
<context>
    <name>CSelectBoxWindow</name>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="89"/>
        <source>Sandboxie-Plus - Run Sandboxed</source>
        <translation>Sandboxie-Plus - 在沙盒内运行</translation>
    </message>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="206"/>
        <source>Are you sure you want to run the program outside the sandbox?</source>
        <translation>确定要在沙盒外运行程序吗？</translation>
    </message>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="219"/>
        <source>Please select a sandbox.</source>
        <translation>请选择一个沙盒</translation>
    </message>
</context>
<context>
    <name>CSettingsWindow</name>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="130"/>
        <source>Sandboxie Plus - Global Settings</source>
        <oldsource>Sandboxie Plus - Settings</oldsource>
        <translation>Sandboxie Plus - 全局设置</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="216"/>
        <source>Auto Detection</source>
        <translation>自动检测</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="217"/>
        <source>No Translation</source>
        <translation>保持默认</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="237"/>
        <source>Don&apos;t integrate links</source>
        <translation>不整合</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="238"/>
        <source>As sub group</source>
        <translation>作为子组</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="239"/>
        <source>Fully integrate</source>
        <translation>全面整合</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="241"/>
        <source>Don&apos;t show any icon</source>
        <oldsource>Don&apos;t integrate links</oldsource>
        <translation>不显示</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="242"/>
        <source>Show Plus icon</source>
        <translation>Plus 版</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="243"/>
        <source>Show Classic icon</source>
        <translation>经典版</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="245"/>
        <source>All Boxes</source>
        <translation>所有沙盒</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="246"/>
        <source>Active + Pinned</source>
        <translation>激活或已固定的沙盒</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="247"/>
        <source>Pinned Only</source>
        <translation>仅已固定的沙盒</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="249"/>
        <source>None</source>
        <translation>无</translation>
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
        <location filename="Windows/SettingsWindow.cpp" line="265"/>
        <source>%1</source>
        <oldsource>%1 %</oldsource>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="496"/>
        <source>Search for settings</source>
        <translation>搜索设置</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="719"/>
        <location filename="Windows/SettingsWindow.cpp" line="720"/>
        <location filename="Windows/SettingsWindow.cpp" line="730"/>
        <source>Run &amp;Sandboxed</source>
        <translation>在沙盒中运行(&amp;S)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="751"/>
        <source>Sandboxed Web Browser</source>
        <translation>浏览器(沙盒)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="254"/>
        <location filename="Windows/SettingsWindow.cpp" line="259"/>
        <source>Notify</source>
        <translation>通知</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="253"/>
        <source>Ignore</source>
        <translation>忽略</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="255"/>
        <location filename="Windows/SettingsWindow.cpp" line="260"/>
        <source>Download &amp; Notify</source>
        <translation>下载并通知</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="256"/>
        <location filename="Windows/SettingsWindow.cpp" line="261"/>
        <source>Download &amp; Install</source>
        <translation>下载并安装</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="345"/>
        <source>Browse for Program</source>
        <translation>浏览程序</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="412"/>
        <source>Add %1 Template</source>
        <translation>添加 %1 模板</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="597"/>
        <source>Please enter message</source>
        <translation>请输入信息</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="629"/>
        <source>Select Program</source>
        <translation>选择程序</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="629"/>
        <source>Executables (*.exe *.cmd)</source>
        <translation>可执行文件 (*.exe *.cmd)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="633"/>
        <location filename="Windows/SettingsWindow.cpp" line="646"/>
        <source>Please enter a menu title</source>
        <translation>请输入一个菜单标题</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="642"/>
        <source>Please enter a command</source>
        <translation>请输入一则命令</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="964"/>
        <source>This supporter certificate has expired, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>此赞助者凭据已过期，请&lt;a href=&quot;sbie://update/cert&quot;&gt;获取新凭据&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="967"/>
        <source>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Plus features will be disabled in %1 days.&lt;/font&gt;</source>
        <translation>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Plus 附加的高级功能将在 %1 天后被禁用&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="969"/>
        <source>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;For this build Plus features remain enabled.&lt;/font&gt;</source>
        <translation>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;在此版本中，Plus 附加的高级功能仍是可用的&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="971"/>
        <source>&lt;br /&gt;Plus features are no longer enabled.</source>
        <translation>&lt;br /&gt;Plus 附加的高级功能已不再可用</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="977"/>
        <source>This supporter certificate will &lt;font color=&apos;red&apos;&gt;expire in %1 days&lt;/font&gt;, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>此赞助者凭据将&lt;font color=&apos;red&apos;&gt;在 %1 天后过期&lt;/font&gt;，请&lt;a href=&quot;sbie://update/cert&quot;&gt;获取新凭据&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1006"/>
        <source>Supporter certificate required</source>
        <oldsource>Supproter certificate required</oldsource>
        <translation>需要赞助者凭据</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1112"/>
        <source>Run &amp;Un-Sandboxed</source>
        <translation>在沙盒外运行(&amp;U)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1371"/>
        <source>This does not look like a certificate. Please enter the entire certificate, not just a portion of it.</source>
        <translation>这看起来不像是一份凭据。请输入完整的凭据，而不仅仅是其中的一部分</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1390"/>
        <source>This certificate is unfortunately expired.</source>
        <translation>非常抱歉，此凭据已过期</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1392"/>
        <source>This certificate is unfortunately outdated.</source>
        <translation>非常抱歉，此凭据已过时</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1395"/>
        <source>Thank you for supporting the development of Sandboxie-Plus.</source>
        <translation>感谢您对 Sandboxie-Plus 开发工作的支持</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1402"/>
        <source>This support certificate is not valid.</source>
        <translation>此赞助者凭据无效</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1440"/>
        <location filename="Windows/SettingsWindow.cpp" line="1576"/>
        <source>Select Directory</source>
        <translation>选择目录</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1469"/>
        <source>&lt;a href=&quot;check&quot;&gt;Check Now&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;check&quot;&gt;立即检查&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1536"/>
        <source>Please enter the new configuration password.</source>
        <translation>请输入新的配置保护密码</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1540"/>
        <source>Please re-enter the new configuration password.</source>
        <translation>请再次输入新的配置保护密码</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1545"/>
        <source>Passwords did not match, please retry.</source>
        <translation>输入的密码不一致，请重新输入</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1557"/>
        <source>Process</source>
        <translation>进程</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1557"/>
        <source>Folder</source>
        <translation>文件夹</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1567"/>
        <source>Please enter a program file name</source>
        <translation>请输入一个程序文件名</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1635"/>
        <source>Please enter the template identifier</source>
        <translation>请输入模板标识符</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1643"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1668"/>
        <source>Do you really want to delete the selected local template(s)?</source>
        <translation>你真的想删除选定的本地模板吗？</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1824"/>
        <source>%1 (Current)</source>
        <translation>%1 (当前)</translation>
    </message>
</context>
<context>
    <name>CSetupWizard</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="29"/>
        <source>Setup Wizard</source>
        <translation>设置指南</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="40"/>
        <source>The decision you make here will affect which page you get to see next.</source>
        <translation>你在这里做出的决定将影响你接下来看到的页面内容</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="43"/>
        <source>This help is likely not to be of any help.</source>
        <translation>此类帮助很可能没用提供任何实质性帮助</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="47"/>
        <source>Sorry, I already gave all the help I could.</source>
        <translation>对不起，我已经提供了所有能给的帮助</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="49"/>
        <source>Setup Wizard Help</source>
        <translation>设置向导帮助</translation>
    </message>
</context>
<context>
    <name>CShellPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="417"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; shell integration</source>
        <translation>设置 &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 系统集成选项</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="418"/>
        <source>Configure how Sandboxie-Plus should integrate with your system.</source>
        <translation>配置 Sandboxie-Plus 应如何与你的系统整合</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="422"/>
        <source>Start UI with Windows</source>
        <translation>随系统启动沙盒管理器</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="427"/>
        <source>Add &apos;Run Sandboxed&apos; to the explorer context menu</source>
        <translation>在资源管理器中添加“在沙盒中运行”右键菜单</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="432"/>
        <source>Add desktop shortcut for starting Web browser under Sandboxie</source>
        <translation>添加沙盒化的网络浏览器快捷方式到桌面</translation>
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
        <translation>恢复到空沙盒</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="103"/>
        <source> (default)</source>
        <translation> (默认)</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="173"/>
        <source>Please enter a name for the new Snapshot.</source>
        <translation>请输入新快照的名称</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="173"/>
        <source>New Snapshot</source>
        <translation>新快照</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="196"/>
        <source>Do you really want to switch the active snapshot? Doing so will delete the current state!</source>
        <translation>确定要切换正在使用的快照？这将删除当前的状态！</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="230"/>
        <source>Do you really want to delete the selected snapshot?</source>
        <translation>确定要删除选中的快照？</translation>
    </message>
</context>
<context>
    <name>CStackView</name>
    <message>
        <location filename="Views/StackView.cpp" line="17"/>
        <source>#|Symbol</source>
        <translation type="unfinished">#|符号</translation>
    </message>
</context>
<context>
    <name>CSummaryPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="527"/>
        <source>Create the new Sandbox</source>
        <translation>创建新沙盒</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="535"/>
        <source>Almost complete, click Finish to create a new sandbox and conclude the wizard.</source>
        <translation>即将就绪, 点击完成按钮结束沙盒创建向导</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="544"/>
        <source>Save options as new defaults</source>
        <translation>保存选项为新的默认配置</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="555"/>
        <source>Don&apos;t show the summary page in future (unless advanced options were set)</source>
        <translation>以后不再显示总结页面 (除非启用高级选项)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="574"/>
        <source>
This Sandbox will be saved to: %1</source>
        <translation>
该沙盒将保存到: %1</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="577"/>
        <source>
This box&apos;s content will be DISCARDED when it&apos;s closed, and the box will be removed.</source>
        <oldsource>
This box&apos;s content will be DISCARDED when its closed, and the box will be removed.</oldsource>
        <translation>
该沙盒中的内容将在所有程序结束后被删除，同时沙盒本身将被移除</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="579"/>
        <source>
This box will DISCARD its content when its closed, its suitable only for temporary data.</source>
        <translation>
该沙盒中的内容将在所有程序结束后被删除，仅适合临时数据</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="581"/>
        <source>
Processes in this box will not be able to access the internet or the local network, this ensures all accessed data to stay confidential.</source>
        <translation>
该沙盒中所有进程将无法访问网络和本地连接，以确保所有可访问的数据不被泄露</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="583"/>
        <source>
This box will run the MSIServer (*.msi installer service) with a system token, this improves the compatibility but reduces the security isolation.</source>
        <oldsource>
This box will run the MSIServer (*.msi installer service) with a system token, this improves the compatybility but reduces the security isolation.</oldsource>
        <translation>
该沙盒允许 MSIServer (*.msi 安装服务) 在沙盒内使用系统令牌运行，这将改善兼容性但会影响安全隔离效果</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="585"/>
        <source>
Processes in this box will think they are run with administrative privileges, without actually having them, hence installers can be used even in a security hardened box.</source>
        <translation>
该沙盒中所有进程将认为其运行在管理员模式下，即使实际上并没有该权限，这有助于在安全加固型沙盒中运行安装程序</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="587"/>
        <source>
Processes in this box will be running with a custom process token indicating the sandbox they belong to.</source>
        <oldsource>
Processes in this box will be running with a custom process token indicating the sandbox thay belong to.</oldsource>
        <translation>
该沙箱中的进程将会以沙箱专属的自定义进程凭据运行</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="620"/>
        <source>Failed to create new box: %1</source>
        <translation>无法创建新沙盒: %1</translation>
    </message>
</context>
<context>
    <name>CSupportDialog</name>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="115"/>
        <source>The installed supporter certificate &lt;b&gt;has expired %1 days ago&lt;/b&gt; and &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;must be renewed&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;</source>
        <oldsource>The installed supporter certificate &lt;b&gt;has expired %1 days ago&lt;/b&gt; and &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;must be renewed&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;</oldsource>
        <translation>已安装的赞助者凭据&lt;b&gt;已经过期 %1 天了&lt;/b&gt;，&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;必须续期&lt;/a&gt;。&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="117"/>
        <source>&lt;b&gt;You have installed Sandboxie-Plus more than %1 days ago.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</source>
        <translation>&lt;b&gt;您已安装 Sandboxie-Plus 超过 %1 天了。&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="119"/>
        <source>&lt;u&gt;Commercial use of Sandboxie past the evaluation period&lt;/u&gt;, requires a valid &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;support certificate&lt;/a&gt;.</source>
        <oldsource>&lt;u&gt;Commercial use of Sandboxie past the evaluation period&lt;/u&gt;, requires a valid &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;support certificate&lt;/a&gt;.</oldsource>
        <translation>&lt;u&gt;商业使用 Sandboxie 的评估期已过&lt;/u&gt;，需要一份有效的&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;赞助者凭据&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="126"/>
        <source>The installed supporter certificate is &lt;b&gt;outdated&lt;/b&gt; and it is &lt;u&gt;not valid for&lt;b&gt; this version&lt;/b&gt;&lt;/u&gt; of Sandboxie-Plus.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>安装的赞助者凭据&lt;b&gt;已过期&lt;/b&gt;，并且&lt;u&gt;不再适用于&lt;b&gt;此版本&lt;/b&gt;&lt;/u&gt;的 Sandboxie-Plus。&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="128"/>
        <source>The installed supporter certificate is &lt;b&gt;expired&lt;/b&gt; and &lt;u&gt;should be renewed&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</source>
        <oldsource>The installed supporter certificate is &lt;b&gt;expired&lt;/b&gt; and &lt;u&gt;should to be renewed&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</oldsource>
        <translation>安装的赞助者凭据&lt;b&gt;已过期&lt;/b&gt;，&lt;u&gt;应当更新&lt;/u&gt;。&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="130"/>
        <source>&lt;b&gt;You have been using Sandboxie-Plus for more than %1 days now.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</source>
        <translation>&lt;b&gt;已经使用 Sandboxie-Plus 超过 %1 天了。&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="133"/>
        <source>Sandboxie on ARM64 requires a valid supporter certificate for continued use.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Sandboxie 的 ARM64 支持需要一份有效的赞助者凭据。&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="135"/>
        <source>Personal use of Sandboxie is free of charge on x86/x64, although some functionality is only available to project supporters.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>个人用途的 Sandboxie 在 x86/x64 平台上是免费的，尽管有些功能只对项目赞助者开放。&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="138"/>
        <source>Please continue &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;supporting the project&lt;/a&gt; by renewing your &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; and continue using the &lt;b&gt;enhanced functionality&lt;/b&gt; in new builds.</source>
        <oldsource>Please continue &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt; by renewing your &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; and continue using the &lt;b&gt;enhanced functionality&lt;/b&gt; in new builds.</oldsource>
        <translation>请考虑继续 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;捐赠支持该项目&lt;/a&gt;，以续签 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;赞助者凭据&lt;/a&gt;，从而继续使用新构建版本中的增强功能</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="141"/>
        <source>Sandboxie &lt;u&gt;without&lt;/u&gt; a valid supporter certificate will sometimes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;pause for a few seconds&lt;/font&gt;&lt;/b&gt;. This pause allows you to consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;purchasing a supporter certificate&lt;/a&gt; or &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;earning one by contributing&lt;/a&gt; to the project. &lt;br /&gt;&lt;br /&gt;A &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; not just removes this reminder, but also enables &lt;b&gt;exclusive enhanced functionality&lt;/b&gt; providing better security and compatibility.</source>
        <oldsource>Sandboxie &lt;u&gt;without&lt;/u&gt; a valid supporter certificate will sometimes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;pause for a few seconds&lt;/font&gt;&lt;/b&gt;, to give you time to contemplate the option of &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;A &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; not just removes this reminder, but also enables &lt;b&gt;exclusive enhanced functionality&lt;/b&gt; providing better security and compatibility.</oldsource>
        <translation>Sandboxie &lt;u&gt;在没有&lt;/u&gt;有效的赞助者凭据时有时会&lt;b&gt;&lt;font color=&apos;red&apos;&gt;弹窗提醒&lt;/font&gt;&lt;/b&gt;，让您考虑是否&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;捐赠支持此项目&lt;/a&gt;(但不会中断不需要赞助着凭据的沙盒内的程序)，&lt;br /&gt;&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;赞助者凭据&lt;/a&gt;不仅可以消除这种提醒，还可以 &lt;b&gt;提供特殊的增强功能&lt;b&gt;，实现更好的安全性和兼容性</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="172"/>
        <source>Sandboxie-Plus - Support Reminder</source>
        <translation>Sandboxie-Plus - 捐赠提醒</translation>
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
        <translation>继续</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="260"/>
        <source>Get Certificate</source>
        <translation>获取凭据</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="261"/>
        <source>Enter Certificate</source>
        <translation>输入凭据</translation>
    </message>
</context>
<context>
    <name>CTemplateTypePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="258"/>
        <source>Create new Template</source>
        <translation>创建新的模板</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="267"/>
        <source>Select template type:</source>
        <translation>选择模板类型：</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="271"/>
        <source>%1 template</source>
        <translation>%1 模板</translation>
    </message>
</context>
<context>
    <name>CTemplateWizard</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="37"/>
        <source>Compatibility Template Wizard</source>
        <oldsource>Compatybility Template Wizard</oldsource>
        <translation>兼容性模板向导</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="48"/>
        <source>Custom</source>
        <translation>自定义</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="49"/>
        <source>Web Browser</source>
        <translation>网络浏览器</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="82"/>
        <source>Force %1 to run in this sandbox</source>
        <translation>强制 %1 在此沙盒内运行</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="100"/>
        <source>Allow direct access to the entire %1 profile folder</source>
        <translation>允许直接访问整个 %1 的配置目录</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="113"/>
        <location filename="Wizards/TemplateWizard.cpp" line="168"/>
        <source>Allow direct access to %1 phishing database</source>
        <translation>允许直接访问 %1 的反钓鱼数据库</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="127"/>
        <source>Allow direct access to %1 session management</source>
        <translation>允许直接访问 %1 的会话管理</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="136"/>
        <location filename="Wizards/TemplateWizard.cpp" line="199"/>
        <source>Allow direct access to %1 passwords</source>
        <translation>允许直接访问 %1 存储的密码</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="146"/>
        <location filename="Wizards/TemplateWizard.cpp" line="208"/>
        <source>Allow direct access to %1 cookies</source>
        <translation>允许直接访问 %1 存储的 Cookies</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="155"/>
        <location filename="Wizards/TemplateWizard.cpp" line="227"/>
        <source>Allow direct access to %1 bookmark and history database</source>
        <translation>允许直接访问 %1 收藏的书签和历史浏览数据</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="180"/>
        <source>Allow direct access to %1 sync data</source>
        <translation>允许直接访问 %1 的同步数据</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="190"/>
        <source>Allow direct access to %1 preferences</source>
        <translation>允许直接访问 %1 的偏好设定</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="217"/>
        <source>Allow direct access to %1 bookmarks</source>
        <translation>允许直接访问 %1 收藏的书签</translation>
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
        <translation>进程 %1</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="179"/>
        <source>Thread %1</source>
        <translation>线程 %1</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="333"/>
        <source>Process</source>
        <translation>进程</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="335"/>
        <source>Type</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="336"/>
        <source>Status</source>
        <translation>状态</translation>
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
        <location filename="Views/TraceView.cpp" line="251"/>
        <source>Monitor mode</source>
        <translation>监控模式</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="255"/>
        <source>Show as task tree</source>
        <translation>显示为任务树</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="259"/>
        <source>Show NT Object Tree</source>
        <translation>展示 NT 对象树</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="266"/>
        <source>PID:</source>
        <translation>进程 PID：</translation>
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
        <translation>线程 TID：</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="281"/>
        <source>Type:</source>
        <translation>类型：</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="292"/>
        <source>Status:</source>
        <translation>状态：</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="295"/>
        <source>Open</source>
        <translation>开放</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="296"/>
        <source>Closed</source>
        <translation>封禁</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="297"/>
        <source>Trace</source>
        <translation>跟踪</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="298"/>
        <source>Other</source>
        <translation>其它</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="306"/>
        <source>Show All Boxes</source>
        <translation>显示所有沙盒</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="310"/>
        <source>Show Stack Trace</source>
        <translation>显示堆栈跟踪</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="315"/>
        <source>Save to file</source>
        <translation>保存到文件</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="329"/>
        <source>Cleanup Trace Log</source>
        <translation>清理跟踪日志</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="694"/>
        <source>Save trace log to file</source>
        <translation>保存跟踪日志到文件</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="700"/>
        <source>Failed to open log file for writing</source>
        <translation>无法打开日志文件进行写入</translation>
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
        <translation>Sandboxie-Plus - 跟踪监测</translation>
    </message>
</context>
<context>
    <name>CUIPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="302"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; UI</source>
        <translation>配置 &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 用户界面</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="303"/>
        <source>Select the user interface style you prefer.</source>
        <translation>选择您喜欢的用户界面风格</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="307"/>
        <source>&amp;Advanced UI for experts</source>
        <translation>适合专家的高级视图(&amp;A)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="312"/>
        <source>&amp;Simple UI for beginners</source>
        <translation>适合新手的简易视图(&amp;S)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="317"/>
        <source>&amp;Vintage SbieCtrl.exe UI</source>
        <translation>SbieCtrl 经典视图(&amp;V)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="342"/>
        <source>Use Bright Mode</source>
        <translation>使用浅色主题</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="346"/>
        <source>Use Dark Mode</source>
        <translation>使用深色主题</translation>
    </message>
</context>
<context>
    <name>CWFPPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="453"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; network filtering</source>
        <translation>配置 &lt;b&gt;Sandboxie-Plus&lt;/b&gt; 的网络过滤功能</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="454"/>
        <source>Sandboxie can use the Windows Filtering Platform (WFP) to restrict network access.</source>
        <translation>沙盒可以使用 Windows 筛选平台 (WFP) 来限制网络访问</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="460"/>
        <source>Using WFP allows Sandboxie to reliably enforce IP/Port based rules for network access. Unlike system level application firewalls, Sandboxie can use different rules in each box for the same application. If you already have a good and reliable application firewall and do not need per box rules, you can leave this option unchecked. Without WFP enabled, Sandboxie will still be able to reliably and entirely block processes from accessing the network. However, this can cause the process to crash, as the driver blocks the required network device endpoints. Even with WFP disabled, Sandboxie offers to set IP/Port based rules, however these will be applied in user mode only and not be enforced by the driver. Hence, without WFP enabled, an intentionally malicious process could bypass those rules, but not the entire network block.</source>
        <translation>启用 WFP 使 Sandboxie 能够可靠地执行基于 IP/端口 的网络访问规则
与系统层级的应用防火墙不同，Sandboxie 可以针对同一应用在不同的沙盒内设置不同的规则
如果你已有一个更友好、更可靠的应用防火墙，并且不需要针对同一应用在不同沙盒设置不同的规则，则可不勾选此选项
如果不启用 WFP，Sandboxie 仍然能够可靠地完全阻止进程访问网络
然而，这可能会导致进程崩溃，因为驱动程序会阻止程序访问请求的网络设备端点
即使禁用 WFP，Sandboxie 也将提供基于 IP/端口 的规则过滤功能，但此时规则只能在用户态下应用，而无法被驱动程序强制执行
因此，如果不启用 WFP，某些恶意程序可能可以绕过这些规则，但不能绕过整个网络区块</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="469"/>
        <source>Enable Windows Filtering Platform (WFP) support</source>
        <translation>启用 Windows 筛选平台 (WFP) 功能支持</translation>
    </message>
</context>
<context>
    <name>NewBoxWindow</name>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="32"/>
        <source>SandboxiePlus new box</source>
        <translation>SandboxiePlus 新建沙盒</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="60"/>
        <source>Box Type Preset:</source>
        <translation>沙盒类型预设配置：</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="91"/>
        <source>A sandbox isolates your host system from processes running within the box, it prevents them from making permanent changes to other programs and data in your computer. The level of isolation impacts your security as well as the compatibility with applications, hence there will be a different level of isolation depending on the selected Box Type. Sandboxie can also protect your personal data from being accessed by processes running under its supervision.</source>
        <translation>沙盒将主机系统与在盒内运行的进程隔离开来，可以防止它们对计算机中的其它程序和数据进行永久性的改变，根据所选的沙盒类型，会有不同的隔离程度，隔离的程度影响到主机的安全性以及盒内应用程序的兼容性，此外沙盒还可以保护你的个人数据不被受监督下运行的进程的访问</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="101"/>
        <source>Box info</source>
        <translation>沙盒信息</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="67"/>
        <source>Sandbox Name:</source>
        <translation>沙盒名称：</translation>
    </message>
</context>
<context>
    <name>OptionsWindow</name>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="32"/>
        <source>SandboxiePlus Options</source>
        <translation>SandboxiePlus 选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="52"/>
        <source>General Options</source>
        <translation>常规选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="62"/>
        <source>Box Options</source>
        <translation>沙盒选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="252"/>
        <source>Appearance</source>
        <translation>外观</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="221"/>
        <source>px Width</source>
        <translation>宽度(像素)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="96"/>
        <source>Sandbox Indicator in title:</source>
        <translation>标题栏中的沙盒标识：</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="106"/>
        <source>Sandboxed window border:</source>
        <translation>沙盒内窗口边框：</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="730"/>
        <location filename="Forms/OptionsWindow.ui" line="766"/>
        <location filename="Forms/OptionsWindow.ui" line="783"/>
        <location filename="Forms/OptionsWindow.ui" line="992"/>
        <location filename="Forms/OptionsWindow.ui" line="1037"/>
        <source>Protect the system from sandboxed processes</source>
        <translation>保护系统免受沙盒内进程的影响</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="995"/>
        <source>Elevation restrictions</source>
        <translation>提权限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="695"/>
        <source>Block network files and folders, unless specifically opened.</source>
        <translation>拦截对网络文件和文件夹的访问，除非专门开放访问权限</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1002"/>
        <source>Make applications think they are running elevated (allows to run installers safely)</source>
        <translation>使应用程序认为自己已被提权运行(允许安全地运行安装程序)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="786"/>
        <source>Network restrictions</source>
        <translation>网络限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1080"/>
        <source>Drop rights from Administrators and Power Users groups</source>
        <translation>撤销管理员和 Power Users 用户组的权限</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1016"/>
        <source>(Recommended)</source>
        <translation>(推荐)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="279"/>
        <source>File Options</source>
        <translation>文件选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="324"/>
        <source>Auto delete content when last sandboxed process terminates</source>
        <translation>最后一个沙盒内的进程终止后自动删除内容</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="453"/>
        <source>Copy file size limit:</source>
        <translation>复制文件大小限制：</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="410"/>
        <source>Box Delete options</source>
        <translation>沙盒删除选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="294"/>
        <source>Protect this sandbox from deletion or emptying</source>
        <translation>保护此沙盒免受删除或清空</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="433"/>
        <location filename="Forms/OptionsWindow.ui" line="474"/>
        <source>File Migration</source>
        <translation>文件迁移</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="301"/>
        <source>Allow elevated sandboxed applications to read the harddrive</source>
        <translation>允许提权的沙盒内程序读取硬盘</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="287"/>
        <source>Warn when an application opens a harddrive handle</source>
        <translation>有程序打开硬盘句柄时警示</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="503"/>
        <source>kilobytes</source>
        <translation>KB</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="596"/>
        <source>Issue message 2102 when a file is too large</source>
        <translation>文件太大时，提示问题代码 2102</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="460"/>
        <source>Prompt user for large file migration</source>
        <translation>询问用户是否迁移大文件</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="702"/>
        <source>Remove spooler restriction, printers can be installed outside the sandbox</source>
        <translation>解除打印限制，可在沙盒外安装打印机</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="769"/>
        <source>Printing restrictions</source>
        <translation>打印限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="709"/>
        <source>Open System Protected Storage</source>
        <translation>开放“系统保护的存储”权限</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="688"/>
        <source>Allow the print spooler to print to files outside the sandbox</source>
        <translation>允许打印服务在沙盒外打印文件</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1094"/>
        <source>CAUTION: When running under the built in administrator, processes can not drop administrative privileges.</source>
        <translation>警告：在内置的管理员用户下运行时，不能撤销进程的管理员权限</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="675"/>
        <source>Block access to the printer spooler</source>
        <translation>阻止访问打印服务</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="733"/>
        <source>Other restrictions</source>
        <translation>其它限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="641"/>
        <source>Block read access to the clipboard</source>
        <translation>阻止访问系统剪贴板</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="172"/>
        <source>Show this box in the &apos;run in box&apos; selection prompt</source>
        <translation>在“在沙盒中运行”对话框中显示此沙盒</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1121"/>
        <source>Security note: Elevated applications running under the supervision of Sandboxie, with an admin or system token, have more opportunities to bypass isolation and modify the system outside the sandbox.</source>
        <translation>安全提示：在沙盒监管下运行的程序，若具有管理员或系统权限令牌，将有更多机会绕过沙盒的隔离，并修改沙盒外部的系统</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1047"/>
        <source>Allow MSIServer to run with a sandboxed system token and apply other exceptions if required</source>
        <translation>允许 MSIServer 在沙盒内使用系统令牌运行，并在必要时给予其它限制权限的豁免</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1104"/>
        <source>Note: Msi Installer Exemptions should not be required, but if you encounter issues installing a msi package which you trust, this option may help the installation complete successfully. You can also try disabling drop admin rights.</source>
        <translation>注意：MSI 安装程序的权限豁免不是必须的，但是如果在安装受信任的程序包时遇到问题，此选项可能会有助于成功完成安装，此外也可以尝试关闭「撤销管理员权限」选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="796"/>
        <source>Run Menu</source>
        <translation>运行菜单</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="925"/>
        <source>You can configure custom entries for the sandbox run menu.</source>
        <translation>可以在此处为沙盒列表的「运行」菜单配置自定义命令</translation>
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
        <translation>名称</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="879"/>
        <source>Command Line</source>
        <translation>命令行</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="845"/>
        <source>Add program</source>
        <translation>添加程序</translation>
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
        <translation>类型</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1583"/>
        <source>Program Groups</source>
        <translation>程序组</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1591"/>
        <source>Add Group</source>
        <translation>添加组</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1623"/>
        <location filename="Forms/OptionsWindow.ui" line="1977"/>
        <location filename="Forms/OptionsWindow.ui" line="2054"/>
        <location filename="Forms/OptionsWindow.ui" line="2132"/>
        <location filename="Forms/OptionsWindow.ui" line="2918"/>
        <source>Add Program</source>
        <translation>添加程序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1734"/>
        <source>Force Folder</source>
        <translation>必沙目录</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2257"/>
        <location filename="Forms/OptionsWindow.ui" line="2357"/>
        <location filename="Forms/OptionsWindow.ui" line="2490"/>
        <source>Path</source>
        <translation>路径</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1777"/>
        <source>Force Program</source>
        <translation>必沙程序</translation>
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
        <translation>显示模板</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="238"/>
        <source>General Configuration</source>
        <translation>常规配置</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="211"/>
        <source>Box Type Preset:</source>
        <translation>沙盒类型预设配置：</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="179"/>
        <source>Box info</source>
        <translation>沙盒信息</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="142"/>
        <source>&lt;b&gt;More Box Types&lt;/b&gt; are exclusively available to &lt;u&gt;project supporters&lt;/u&gt;, the Privacy Enhanced boxes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;protect user data from illicit access&lt;/font&gt;&lt;/b&gt; by the sandboxed programs.&lt;br /&gt;If you are not yet a supporter, then please consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt;, to receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;.&lt;br /&gt;You can test the other box types by creating new sandboxes of those types, however processes in these will be auto terminated after 5 minutes.</source>
        <translation>&lt;b&gt;更多沙盒类型&lt;/b&gt;仅&lt;u&gt;项目赞助者&lt;/u&gt;可用，隐私增强沙盒&lt;b&gt;&lt;font color=&apos;red&apos;&gt;保护用户数据免受沙盒化的程序非法访问&lt;/font&gt;&lt;/b&gt;&lt;br /&gt;如果你还不是赞助者，请考虑&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;捐赠此项目&lt;/a&gt;，来获得&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;赞助者凭据&lt;/a&gt;&lt;br /&gt;当然你也可以直接新建一个这些类型的沙盒进行测试，不过沙盒中运行的程序将在 5 分钟之后自动终止</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="89"/>
        <source>Always show this sandbox in the systray list (Pinned)</source>
        <translation>固定住此沙盒，以便总是在系统托盘列表显示</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="621"/>
        <source>Open Windows Credentials Store (user mode)</source>
        <translation>开放 Windows 凭据存储访问权限 (用户态)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="661"/>
        <source>Prevent change to network and firewall parameters (user mode)</source>
        <translation>拦截对网络及防火墙参数的更改 (用户态)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1630"/>
        <source>You can group programs together and give them a group name.  Program groups can be used with some of the settings instead of program names. Groups defined for the box overwrite groups defined in templates.</source>
        <translation>可以在此处将应用程序分组并给它们分配一个组名，程序组可用于代替程序名被用于某些设置，在此处定义的沙盒程序组将覆盖模板中定义的程序组</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1699"/>
        <source>Programs entered here, or programs started from entered locations, will be put in this sandbox automatically, unless they are explicitly started in another sandbox.</source>
        <translation>此处指定的程序或者指定位置中的程序，将自动进入此沙盒，除非已明确在其它沙盒中启动它</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1920"/>
        <source>Stop Behaviour</source>
        <translation>停止行为</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2092"/>
        <source>Start Restrictions</source>
        <translation>启动限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2100"/>
        <source>Issue message 1308 when a program fails to start</source>
        <translation>程序启动失败时，提示问题代码 1308</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2151"/>
        <source>Allow only selected programs to start in this sandbox. *</source>
        <translation>仅允许所选程序在此沙盒中启动 *</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2158"/>
        <source>Prevent selected programs from starting in this sandbox.</source>
        <translation>阻止所选的程序在此沙盒中启动</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2165"/>
        <source>Allow all programs to start in this sandbox.</source>
        <translation>允许所有程序在此沙盒中启动</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2172"/>
        <source>* Note: Programs installed to this sandbox won&apos;t be able to start at all.</source>
        <translation>* 注意：安装在此沙盒里的程序将完全无法启动</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2891"/>
        <source>Process Restrictions</source>
        <translation>程序限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2911"/>
        <source>Issue message 1307 when a program is denied internet access</source>
        <translation>程序被拒绝访问网络时，提示问题代码 1307</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2939"/>
        <source>Note: Programs installed to this sandbox won&apos;t be able to access the internet at all.</source>
        <translation>注意：安装在此沙盒中的程序将完全无法访问网络</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2925"/>
        <source>Prompt user whether to allow an exemption from the blockade.</source>
        <translation>询问用户是否允许例外</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2194"/>
        <source>Resource Access</source>
        <translation>资源访问</translation>
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
        <translation>程序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2252"/>
        <location filename="Forms/OptionsWindow.ui" line="2352"/>
        <location filename="Forms/OptionsWindow.ui" line="2485"/>
        <location filename="Forms/OptionsWindow.ui" line="2605"/>
        <location filename="Forms/OptionsWindow.ui" line="2679"/>
        <location filename="Forms/OptionsWindow.ui" line="2968"/>
        <source>Access</source>
        <translation>访问</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2384"/>
        <source>Add Reg Key</source>
        <translation>添加注册表键值</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2277"/>
        <source>Add File/Folder</source>
        <translation>添加文件/文件夹</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2571"/>
        <source>Add Wnd Class</source>
        <translation>添加窗口类</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2711"/>
        <source>Add COM Object</source>
        <translation>添加 COM 对象</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2450"/>
        <source>Add IPC Path</source>
        <translation>添加 IPC 路径</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3151"/>
        <source>File Recovery</source>
        <translation>文件恢复</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3230"/>
        <source>Add Folder</source>
        <translation>添加文件夹</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3270"/>
        <source>Ignore Extension</source>
        <translation>忽略扩展名</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3316"/>
        <source>Ignore Folder</source>
        <translation>忽略文件夹</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3277"/>
        <source>Enable Immediate Recovery prompt to be able to recover files as soon as they are created.</source>
        <translation>启用快速恢复提示，以便快速恢复创建的文件</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3260"/>
        <source>You can exclude folders and file types (or file extensions) from Immediate Recovery.</source>
        <translation>可以在此处从快速恢复中排除特定目录和文件类型(扩展名)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3193"/>
        <source>When the Quick Recovery function is invoked, the following folders will be checked for sandboxed content. </source>
        <translation>当快速恢复功能被调用时，检查沙盒内的下列文件夹 </translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3358"/>
        <source>Advanced Options</source>
        <translation>高级选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3950"/>
        <source>Miscellaneous</source>
        <translation>杂项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1505"/>
        <source>Do not start sandboxed services using a system token (recommended)</source>
        <translation>不使用系统令牌启动沙盒化的服务 (推荐)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2618"/>
        <source>Don&apos;t alter window class names created by sandboxed programs</source>
        <translation>不要改变由沙盒内程序创建的窗口类名</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1252"/>
        <location filename="Forms/OptionsWindow.ui" line="1269"/>
        <location filename="Forms/OptionsWindow.ui" line="1354"/>
        <location filename="Forms/OptionsWindow.ui" line="1426"/>
        <location filename="Forms/OptionsWindow.ui" line="1450"/>
        <location filename="Forms/OptionsWindow.ui" line="1474"/>
        <source>Protect the sandbox integrity itself</source>
        <translation>沙盒完整性保护</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3375"/>
        <location filename="Forms/OptionsWindow.ui" line="3424"/>
        <source>Compatibility</source>
        <translation>兼容性</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1559"/>
        <source>Add sandboxed processes to job objects (recommended)</source>
        <translation>添加沙盒化进程到作业对象 (推荐)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3403"/>
        <source>Force usage of custom dummy Manifest files (legacy behaviour)</source>
        <translation>强制使用自定义虚拟 Manifest 文件 (传统行为)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1525"/>
        <source>Allow only privileged processes to access the Service Control Manager</source>
        <translation>仅允许特权进程访问“服务控制管理器”</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3410"/>
        <source>Emulate sandboxed window station for all processes</source>
        <translation>为所有进程模拟沙盒化的窗口状况</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1204"/>
        <source>Open access to Windows Security Account Manager</source>
        <oldsource>Open access to windows Security Account Manager</oldsource>
        <translation>开放 Windows 安全帐户管理器 (SAM) 的访问权限</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3718"/>
        <source>Hide Processes</source>
        <translation>隐藏进程</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3767"/>
        <location filename="Forms/OptionsWindow.ui" line="3825"/>
        <source>Add Process</source>
        <translation>添加进程</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3743"/>
        <source>Hide host processes from processes running in the sandbox.</source>
        <translation>对沙盒内运行的进程隐藏宿主的进程</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3774"/>
        <source>Don&apos;t allow sandboxed processes to see processes running in other boxes</source>
        <translation>不允许沙盒内的进程查看其它沙盒里运行的进程</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3882"/>
        <source>Users</source>
        <translation>用户</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3900"/>
        <source>Restrict Resource Access monitor to administrators only</source>
        <translation>仅允许管理员访问“资源访问监视器”</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3907"/>
        <source>Add User</source>
        <translation>添加用户</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3937"/>
        <source>Add user accounts and user groups to the list below to limit use of the sandbox to only those accounts.  If the list is empty, the sandbox can be used by all user accounts.

Note:  Forced Programs and Force Folders settings for a sandbox do not apply to user accounts which cannot use the sandbox.</source>
        <translation>添加用户和用户组到下方列表来仅限这些系统用户使用沙盒，如果列表为空，则所有系统用户均可使用沙盒

注意：沙盒的必沙程序及文件夹设置不适用于不能运行沙盒的系统用户</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4048"/>
        <source>Tracing</source>
        <translation>跟踪</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4219"/>
        <source>COM Class Trace</source>
        <translation>COM 类跟踪</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4164"/>
        <source>IPC Trace</source>
        <translation>IPC 跟踪</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4226"/>
        <source>Key Trace</source>
        <translation>键值跟踪</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4171"/>
        <source>GUI Trace</source>
        <translation>GUI 跟踪</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1040"/>
        <source>Security enhancements</source>
        <translation>安全增强</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="971"/>
        <source>Use the original token only for approved NT system calls</source>
        <translation>只在经过批准的 NT 系统调用中使用原始令牌</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1023"/>
        <source>Restrict driver/device access to only approved ones</source>
        <translation>将对“驱动程序/设备”的访问权限制在已知的终结点列表内</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="978"/>
        <source>Enable all security enhancements (make security hardened box)</source>
        <translation>启用所有安全增强功能(安全防护加固型沙盒选项)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="259"/>
        <source>Double click action:</source>
        <translation>双击动作：</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="359"/>
        <source>Separate user folders</source>
        <translation>隔离不同用户的文件夹</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="396"/>
        <source>Box Structure</source>
        <translation>沙盒结构</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="874"/>
        <source>Icon</source>
        <translation>图标</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="899"/>
        <source>Move Up</source>
        <translation>上移</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="918"/>
        <source>Move Down</source>
        <translation>下移</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="953"/>
        <source>Security Options</source>
        <translation>安全选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="963"/>
        <source>Security Hardening</source>
        <translation>安全加固</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1153"/>
        <source>Security Isolation</source>
        <translation>安全隔离</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1174"/>
        <source>Various isolation features can break compatibility with some applications. If you are using this sandbox &lt;b&gt;NOT for Security&lt;/b&gt; but for application portability, by changing these options you can restore compatibility by sacrificing some security.</source>
        <translation>注意：各种隔离功能会破坏与某些应用程序的兼容性&lt;br /&gt;如果使用此沙盒&lt;b&gt;不是为了安全性&lt;/b&gt;，而是为了应用程序的可移植性，可通过改变这些选项，以便通过牺牲部分安全性来恢复兼容性</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1272"/>
        <source>Access Isolation</source>
        <translation>访问隔离</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1299"/>
        <location filename="Forms/OptionsWindow.ui" line="1357"/>
        <source>Image Protection</source>
        <translation>映像保护</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1307"/>
        <source>Issue message 1305 when a program tries to load a sandboxed dll</source>
        <translation>当一个程序试图加载一个沙盒内部的动态链接库(DLL)文件时，提示问题代码 1305</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1340"/>
        <source>Prevent sandboxes programs installed on host from loading dll&apos;s from the sandbox</source>
        <translation>防止安装在宿主上的沙盒程序从沙盒内部加载动态链接库(DLL)文件</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1383"/>
        <source>Sandboxie’s resource access rules often discriminate against program binaries located inside the sandbox. OpenFilePath and OpenKeyPath work only for application binaries located on the host natively. In order to define a rule without this restriction, OpenPipePath or OpenConfPath must be used. Likewise, all Closed(File|Key|Ipc)Path directives which are defined by negation e.g. ‘ClosedFilePath=! iexplore.exe,C:Users*’ will be always closed for binaries located inside a sandbox. Both restriction policies can be disabled on the “Access policies” page.
This is done to prevent rogue processes inside the sandbox from creating a renamed copy of themselves and accessing protected resources. Another exploit vector is the injection of a library into an authorized process to get access to everything it is allowed to access. Using Host Image Protection, this can be prevented by blocking applications (installed on the host) running inside a sandbox from loading libraries from the sandbox itself.</source>
        <translation>Sandboxie 的资源访问规则通常对位于沙盒内的二进制程序具有歧视性

一般情况下，OpenFilePath 和 OpenKeyPath 只对宿主机上的原生程序（安装在宿主上的）有效
为了定义没有此类限制的规则，则必须使用 OpenPipePath 和 OpenConfPath

同样的，通过否定来定义所有的 Closed(File|Key|Ipc)Path 指令
例如：&apos;ClosedFilePath=! iexplore.exe,C:Users*&apos;将限制沙盒内的程序访问相应资源

这两种限制策略都可以通过“访问策略”页面来禁用

这样做是为了防止沙盒内的流氓进程创建自己的重命名副本并访问受保护的资源

另一个漏洞载体是将一个动态链接库注入到一个被授权进程中，以获得对被授权进程所允许访问的一切资源的访问权
使用主机映像保护，可以通过阻止在沙箱内运行的应用程序（安装在宿主上的）加载来自沙盒的动态链接库来防止此类现象</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1404"/>
        <source>Advanced Security</source>
        <oldsource>Adcanced Security</oldsource>
        <translation>安全性(高级)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1412"/>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation>使用 Sandboxie 限权用户，而不是匿名令牌 (实验性)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1429"/>
        <source>Other isolation</source>
        <translation>其它隔离</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1453"/>
        <source>Privilege isolation</source>
        <translation>特权隔离</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1477"/>
        <source>Sandboxie token</source>
        <translation>沙盒令牌</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1566"/>
        <source>Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.</source>
        <translation>使用自定义沙盒令牌可以更好地将各个沙盒相互隔离，同时可以实现在任务管理器的用户栏中显示进程所属的沙盒
但是，某些第三方安全解决方案可能会与自定义令牌产生兼容性问题</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1657"/>
        <source>Program Control</source>
        <translation>程序控制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1679"/>
        <source>Force Programs</source>
        <translation>必沙程序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1791"/>
        <source>Disable forced Process and Folder for this sandbox</source>
        <translation>禁用此沙盒的“强制进程/目录 规则”</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1801"/>
        <source>Breakout Programs</source>
        <translation>分离程序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1870"/>
        <source>Breakout Program</source>
        <translation>分离程序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1896"/>
        <source>Breakout Folder</source>
        <translation>分离目录</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1903"/>
        <source>Programs entered here will be allowed to break out of this sandbox when they start. It is also possible to capture them into another sandbox, for example to have your web browser always open in a dedicated box.</source>
        <oldsource>Programs entered here will be allowed to break out of this box when thay start, you can capture them into an other box. For example to have your web browser always open in a dedicated box. This feature requires a valid supporter certificate to be installed.</oldsource>
        <translation>此处设置的程序在启动时将被允许脱离这个沙盒，利用此选项可以将程序捕获到另一个沙盒里
例如，让网络浏览器总是在一个专门的沙盒里打开</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1930"/>
        <source>Lingering Programs</source>
        <translation>驻留程序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1996"/>
        <source>Lingering programs will be automatically terminated if they are still running after all other processes have been terminated.</source>
        <translation>其它所有程序被终止后，仍在运行的驻留程序将自动终止</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2009"/>
        <source>Leader Programs</source>
        <translation>引导进程</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2075"/>
        <source>If leader processes are defined, all others are treated as lingering processes.</source>
        <translation>如果定义了引导进程，其它进程将被视作驻留进程</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2204"/>
        <source>Files</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2297"/>
        <source>Configure which processes can access Files, Folders and Pipes. 
&apos;Open&apos; access only applies to program binaries located outside the sandbox, you can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behavior in the Policies tab.</source>
        <translation>配置哪些进程可以访问文件、文件夹和管道
“开放”访问权限只适用于原先已位于沙盒之外的程序二进制文件
你可以使用“完全开放”来对所有程序开放所有权限，或者在策略标签中改变这一行为</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2311"/>
        <source>Registry</source>
        <translation>注册表</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2404"/>
        <source>Configure which processes can access the Registry. 
&apos;Open&apos; access only applies to program binaries located outside the sandbox, you can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behavior in the Policies tab.</source>
        <translation>配置哪些进程可以读写注册表
“开放”访问权限只适用于原先已位于沙盒之外的程序二进制文件
你可以使用“完全开放”来对所有程序开放所有权限，或者在策略标签中改变这一行为</translation>
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
        <translation>配置哪些进程可以访问 NT IPC 对象，如 ALPC 端口及其他进程的内存和相关运行状态环境
如需指定一个进程，使用“$:program.exe”作为路径值(不含双引号)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2525"/>
        <source>Wnd</source>
        <translation>窗口</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2610"/>
        <source>Wnd Class</source>
        <translation>窗口类</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2625"/>
        <source>Configure which processes can access desktop objects like windows and alike.</source>
        <oldsource>Configure which processes can access Desktop objects like Windows and alike.</oldsource>
        <translation>配置哪些进程可以访问桌面对象，如 Windows 或其它类似对象</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2638"/>
        <source>COM</source>
        <translation>COM</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2684"/>
        <source>Class Id</source>
        <translation>类 Id</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2731"/>
        <source>Configure which processes can access COM objects.</source>
        <translation>配置哪些进程可以访问 COM 对象</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2743"/>
        <source>Don&apos;t use virtualized COM, Open access to hosts COM infrastructure (not recommended)</source>
        <translation>不虚拟化 COM 对象，而是开放主机的 COM 基础结构 (不推荐)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2751"/>
        <source>Access Policies</source>
        <translation>权限策略</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2785"/>
        <source>Apply Close...=!&lt;program&gt;,... rules also to all binaries located in the sandbox.</source>
        <translation>将 Close...=!&lt;program&gt;,... 规则，应用到位于沙盒内的所有相关二进制文件</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2881"/>
        <source>Network Options</source>
        <translation>网络选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2978"/>
        <source>Set network/internet access for unlisted processes:</source>
        <translation>不在列表中的程序的网络访问权限：</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3025"/>
        <source>Test Rules, Program:</source>
        <translation>测试规则或程序:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3035"/>
        <source>Port:</source>
        <translation>端口:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3045"/>
        <source>IP:</source>
        <translation>IP:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3055"/>
        <source>Protocol:</source>
        <translation>协议:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3065"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3081"/>
        <source>Add Rule</source>
        <translation>添加规则</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="561"/>
        <location filename="Forms/OptionsWindow.ui" line="3097"/>
        <location filename="Forms/OptionsWindow.ui" line="3536"/>
        <location filename="Forms/OptionsWindow.ui" line="3861"/>
        <source>Action</source>
        <translation>动作</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3102"/>
        <source>Port</source>
        <translation>端口</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3107"/>
        <source>IP</source>
        <translation>IP</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3112"/>
        <source>Protocol</source>
        <translation>协议</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3134"/>
        <source>CAUTION: Windows Filtering Platform is not enabled with the driver, therefore these rules will be applied only in user mode and can not be enforced!!! This means that malicious applications may bypass them.</source>
        <translation>警告：未在此驱动程序启用 Windows 筛选平台，因此以下规则只能在用户模式下生效，无法被强制执行！！！恶意程序可能会绕过这些规则的限制</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2833"/>
        <source>The rule specificity is a measure to how well a given rule matches a particular path, simply put the specificity is the length of characters from the begin of the path up to and including the last matching non-wildcard substring. A rule which matches only file types like &quot;*.tmp&quot; would have the highest specificity as it would always match the entire file path.
The process match level has a higher priority than the specificity and describes how a rule applies to a given process. Rules applying by process name or group have the strongest match level, followed by the match by negation (i.e. rules applying to all processes but the given one), while the lowest match levels have global matches, i.e. rules that apply to any process.</source>
        <translation>规则的特异度是衡量一个给定规则对特定路径的匹配程度，简单地说，特异度是指从路径的最开始到最后一个匹配的非通配符子串之间的字符长度，一个只匹配 “*.tmp” 这样的文件类型的规则将具有最高的特异性，因为它总是匹配整个文件路径
进程匹配级别的优先级高于特异度，它描述了一条规则如何适用于一个给定的进程，按进程名称或程序组应用的规则具有最高的匹配级别，其次是否定匹配模式(即适用于匹配除给定进程以外的所有进程的规则)，而匹配级别最低的是全局匹配，即适用于任何进程的规则</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2792"/>
        <source>Prioritize rules based on their Specificity and Process Match Level</source>
        <translation>基于规则的特异度和进程匹配级别对规则进行优先级排序</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2844"/>
        <source>Privacy Mode, block file and registry access to all locations except the generic system ones</source>
        <translation>隐私模式，阻止对通用系统目录之外的所有文件位置和注册表节点的访问</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2826"/>
        <source>Access Mode</source>
        <translation>访问权限模式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2864"/>
        <source>When the Privacy Mode is enabled, sandboxed processes will be only able to read C:\Windows\*, C:\Program Files\*, and parts of the HKLM registry, all other locations will need explicit access to be readable and/or writable. In this mode, Rule Specificity is always enabled.</source>
        <translation>当启用隐私模式时，沙盒进程将只能读取 C:\Windows\* 、 C:\Program Files\* 和注册表 HKLM 节点下的部分内容，除此之外的所有其它位置都需要明确的访问授权才能被读取或写入，在此模式下，专有规则将总是被应用</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2778"/>
        <source>Rule Policies</source>
        <translation>规则策略</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2799"/>
        <source>Apply File and Key Open directives only to binaries located outside the sandbox.</source>
        <translation>只对位于沙盒之外的二进制文件应用文件和密钥权限开放指令</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1436"/>
        <source>Start the sandboxed RpcSs as a SYSTEM process (not recommended)</source>
        <translation>以系统进程启动沙盒服务 RpcSs (不推荐)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3431"/>
        <source>Allow use of nested job objects (works on Windows 8 and later)</source>
        <oldsource>Allow use of nested job objects (experimental, works on Windows 8 and later)</oldsource>
        <translation>允许使用嵌套作业对象(job object) (仅适用于 Windows 8 及更高版本)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1498"/>
        <source>Drop critical privileges from processes running with a SYSTEM token</source>
        <translation>撤销以系统令牌运行中的程序的关键特权</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1491"/>
        <location filename="Forms/OptionsWindow.ui" line="1539"/>
        <source>(Security Critical)</source>
        <translation>(安全关键)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1460"/>
        <source>Protect sandboxed SYSTEM processes from unprivileged processes</source>
        <translation>保护沙盒中的系统进程免受非特权进程的影响</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1228"/>
        <source>Security Isolation through the usage of a heavily restricted process token is Sandboxie&apos;s primary means of enforcing sandbox restrictions, when this is disabled the box is operated in the application compartment mode, i.e. it’s no longer providing reliable security, just simple application compartmentalization.</source>
        <translation>通过严格限制进程令牌的使用来进行安全隔离是 Sandboxie 执行沙盒化限制的主要手段，当它被禁用时，沙盒将在应用隔间模式下运行，此时将不再提供可靠的安全限制，只是简单进行应用分隔</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1279"/>
        <source>Allow sandboxed programs to manage Hardware/Devices</source>
        <translation>允许沙盒内程序管理硬件设备</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1238"/>
        <source>Open access to Windows Local Security Authority</source>
        <translation>开放 Windows 本地安全验证 (LSA) 的访问权限</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="668"/>
        <source>Allow to read memory of unsandboxed processes (not recommended)</source>
        <translation>允许读取非沙盒进程的内存 (不推荐)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="716"/>
        <source>Issue message 2111 when a process access is denied</source>
        <translation>进程被拒绝访问非沙盒进程内存时，提示问题代码 2111</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3451"/>
        <source>Disable the use of RpcMgmtSetComTimeout by default (this may resolve compatibility issues)</source>
        <translation>默认禁用 RpcMgmtSetComTimeout (或许可以解决兼容性问题)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1197"/>
        <source>Disable Security Isolation (experimental)</source>
        <translation>禁用安全隔离 (实验性)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1255"/>
        <source>Security Isolation &amp; Filtering</source>
        <translation>安全隔离 &amp; 筛查</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1211"/>
        <source>Disable Security Filtering (not recommended)</source>
        <translation>禁用安全筛查功能 (不推荐)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1218"/>
        <source>Security Filtering used by Sandboxie to enforce filesystem and registry access restrictions, as well as to restrict process access.</source>
        <translation>安全筛查被 Sandboxie 用来强制执行文件系统和注册表访问限制，以及限制进程访问</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1286"/>
        <source>The below options can be used safely when you don&apos;t grant admin rights.</source>
        <translation>以下选项可以在你未授予管理员权限时安全的使用</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3468"/>
        <source>Triggers</source>
        <translation>触发器</translation>
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
        <translation>执行命令</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3690"/>
        <source>Start Service</source>
        <translation>启动服务</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3665"/>
        <source>These events are executed each time a box is started</source>
        <translation>这些事件当沙盒每次启动时都会被执行</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3668"/>
        <source>On Box Start</source>
        <translation>沙盒启动阶段</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3549"/>
        <location filename="Forms/OptionsWindow.ui" line="3697"/>
        <source>These commands are run UNBOXED just before the box content is deleted</source>
        <translation>这些命令将在删除沙盒的内容之前，以非沙盒化的方式被执行</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3623"/>
        <source>These commands are executed only when a box is initialized. To make them run again, the box content must be deleted.</source>
        <translation>这些命令只在沙盒被初始化时执行，要使它们再次运行，必须删除沙盒内容</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3626"/>
        <source>On Box Init</source>
        <translation>沙盒初始阶段</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3636"/>
        <source>Here you can specify actions to be executed automatically on various box events.</source>
        <translation>在此处可以配置各种沙盒事件中自动执行特定的动作</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4056"/>
        <source>API call trace (requires LogAPI to be installed in the Sbie directory)</source>
        <oldsource>API call trace (requirers logapi to be installed in the sbie dir)</oldsource>
        <translation>API 调用跟踪 (需要安装 LogAPI 模块到沙盒目录)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4096"/>
        <source>Log all SetError&apos;s to Trace log (creates a lot of output)</source>
        <translation>记录所有 SetError 到跟踪日志 (将产生大量输出)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4150"/>
        <source>File Trace</source>
        <translation>文件跟踪</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4076"/>
        <source>Pipe Trace</source>
        <translation>管道跟踪</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4212"/>
        <source>Access Tracing</source>
        <translation>访问跟踪</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4116"/>
        <source>Log Debug Output to the Trace Log</source>
        <translation>调试日志输出到跟踪日志</translation>
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
        <translation>将驱动程序看到的所有访问事件记录到资源访问日志

这些选项将事件掩码设定为 &quot;*&quot; - 所有访问事件
另外可以通过编辑配置文本来详细自定义日志行为
&quot;A&quot; - 允许的访问
&quot;D&quot; - 拒绝的访问
&quot;I&quot; - 忽略访问请求
来代替 &quot;*&quot;</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4143"/>
        <source>Ntdll syscall Trace (creates a lot of output)</source>
        <translation>Ntdll 系统调用跟踪 (将产生大量输出)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4157"/>
        <source>Disable Resource Access Monitor</source>
        <translation>禁用资源访问监控器</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4185"/>
        <source>Resource Access Monitor</source>
        <translation>资源访问监控</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2993"/>
        <location filename="Forms/OptionsWindow.ui" line="4233"/>
        <source>Network Firewall</source>
        <translation>网络防火墙</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4250"/>
        <source>Debug</source>
        <translation>调试</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4306"/>
        <source>WARNING, these options can disable core security guarantees and break sandbox security!!!</source>
        <translation>警告，这些选项可使核心安全保障失效并且破坏沙盒安全！</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4316"/>
        <source>These options are intended for debugging compatibility issues, please do not use them in production use. </source>
        <translation>这些选项是为调试兼容性问题提供的，日常使用者勿碰 </translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4331"/>
        <source>App Templates</source>
        <translation>应用模板</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4416"/>
        <source>Filter Categories</source>
        <translation>类别筛选</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4387"/>
        <source>Text Filter</source>
        <translation>文本筛选</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4409"/>
        <source>Add Template</source>
        <translation>添加模板</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4364"/>
        <source>This list contains a large amount of sandbox compatibility enhancing templates</source>
        <translation>此列表含有大量的沙盒兼容性增强模板</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4433"/>
        <source>Category</source>
        <translation>类别</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4468"/>
        <source>Template Folders</source>
        <translation>目录模板</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4488"/>
        <source>Configure the folder locations used by your other applications.

Please note that this values are currently user specific and saved globally for all boxes.</source>
        <translation>配置你的其它应用程序所使用的文件夹位置

请注意，这些值对当前用户的所有沙盒保存</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4017"/>
        <location filename="Forms/OptionsWindow.ui" line="4519"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4530"/>
        <source>Accessibility</source>
        <translation>无障碍功能</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4626"/>
        <source>To compensate for the lost protection, please consult the Drop Rights settings page in the Restrictions settings group.</source>
        <translation>要弥补失去的保护，请参考“限制”设置组中的降低权限部分</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4550"/>
        <source>Screen Readers: JAWS, NVDA, Window-Eyes, System Access</source>
        <translation>屏幕阅读器：JAWS、NVDA、Window-Eyes、系统无障碍接口</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="338"/>
        <source>Use volume serial numbers for drives, like: \drive\C~1234-ABCD</source>
        <translation>使用驱动器的卷系列号，例如：\drive\C~1234-ABCD</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="382"/>
        <source>The box structure can only be changed when the sandbox is empty</source>
        <translation>只有在沙盒为空时，才能更改沙盒结构</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="352"/>
        <source>Disk/File access</source>
        <translation>“磁盘/文件”访问权限</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="331"/>
        <source>Virtualization scheme</source>
        <translation>虚拟化方案</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="481"/>
        <source>2113: Content of migrated file was discarded
2114: File was not migrated, write access to file was denied
2115: File was not migrated, file will be opened read only</source>
        <translation>2113：待迁移文件的内容被遗弃了
2114：文件没有被迁移，文件的写入访问被拒绝
2115：文件没有被迁移，文件将以只读方式打开</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="486"/>
        <source>Issue message 2113/2114/2115 when a file is not fully migrated</source>
        <translation>当一个文件没有被完全迁移时，提示问题代码：2113/2114/2115</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="510"/>
        <source>Add Pattern</source>
        <translation>添加“模式”</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="517"/>
        <source>Remove Pattern</source>
        <translation>移除“模式”</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="571"/>
        <source>Pattern</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="579"/>
        <source>Sandboxie does not allow writing to host files, unless permitted by the user. When a sandboxed application attempts to modify a file, the entire file must be copied into the sandbox, for large files this can take a significate amount of time. Sandboxie offers options for handling these cases, which can be configured on this page.</source>
        <translation>Sandboxie 不被允许对主机文件进行写入，除非得到用户的允许
当沙盒化的应用程序试图修改一个文件时，整个文件必须被复制到沙盒中
对于大文件来说，这可能需要相当长的时间
Sandboxie 提供了针对这些情况的处理选项，可以在此页面进行配置</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="589"/>
        <source>Using wildcard patterns file specific behavior can be configured in the list below:</source>
        <translation>使用“通配符模式”，具体的文件行为可以在下面的列表中进行配置：</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="603"/>
        <source>When a file cannot be migrated, open it in read-only mode instead</source>
        <translation>当一个文件不能被迁移时，尝试以只读模式打开它</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="613"/>
        <source>Restrictions</source>
        <translation>限制选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3161"/>
        <source>Quick Recovery</source>
        <translation>快速恢复</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3240"/>
        <source>Immediate Recovery</source>
        <translation>即时恢复</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3337"/>
        <source>Various Options</source>
        <translation>其它杂项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3396"/>
        <source>Apply ElevateCreateProcess Workaround (legacy behaviour)</source>
        <translation>应用 ElevateCreateProcess 解决方案 (传统行为)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3458"/>
        <source>Use desktop object workaround for all processes</source>
        <translation>对所有进程应用桌面对象解决方案</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3498"/>
        <source>This command will be run before the box content will be deleted</source>
        <translation>该命令将在删除沙盒内容之前运行</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3552"/>
        <source>On File Recovery</source>
        <translation>文件恢复阶段</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3581"/>
        <source>This command will be run before a file is being recovered and the file path will be passed as the first argument. If this command returns anything other than 0, the recovery will be blocked</source>
        <oldsource>This command will be run before a file is being recoverd and the file path will be passed as the first argument, if this command return something other than 0 the recovery will be blocked</oldsource>
        <translation>该命令将在文件恢复前运行，文件路径将作为最先被传递的参数，如果该命令的返回值不为 0，恢复动作将被终止</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3584"/>
        <source>Run File Checker</source>
        <translation>运行文件检查</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3700"/>
        <source>On Delete Content</source>
        <translation>内容删除阶段</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3781"/>
        <source>Protect processes in this box from being accessed by specified unsandboxed host processes.</source>
        <translation>保护此沙盒内的进程不被指定的非沙盒的主机进程访问</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3792"/>
        <location filename="Forms/OptionsWindow.ui" line="3856"/>
        <source>Process</source>
        <translation>进程</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3832"/>
        <source>Block also read access to processes in this sandbox</source>
        <translation>阻止对位于该沙盒中的进程的读取访问</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3982"/>
        <source>Add Option</source>
        <translation>添加选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3996"/>
        <source>Here you can configure advanced per process options to improve compatibility and/or customize sandboxing behavior.</source>
        <oldsource>Here you can configure advanced per process options to improve compatibility and/or customize sand boxing behavior.</oldsource>
        <translation>在此处可以配置各个进程的高级选项，以提高兼容性或自定义沙盒的某些行为</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4007"/>
        <source>Option</source>
        <translation>选项</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4341"/>
        <source>Templates</source>
        <translation>模板</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4590"/>
        <source>The following settings enable the use of Sandboxie in combination with accessibility software.  Please note that some measure of Sandboxie protection is necessarily lost when these settings are in effect.</source>
        <translation>以下设置允许 Sandboxie 与辅助功能软件结合，请注意当这些设置生效时，会使 Sandboxie 的部分保护措施失效</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4643"/>
        <source>Edit ini Section</source>
        <translation>配置文本</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4649"/>
        <source>Edit ini</source>
        <translation>编辑配置</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4662"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4685"/>
        <source>Save</source>
        <translation>保存</translation>
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
        <translation>组: %1</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="Views/SbieView.cpp" line="1497"/>
        <source>Drive %1</source>
        <translation>磁盘 %1</translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="SandMan.cpp" line="3604"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3605"/>
        <source>Apply</source>
        <translation>应用</translation>
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
        <translation>SandboxiePlus - 恢复</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="164"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="68"/>
        <source>Recover target:</source>
        <translation>恢复目标位置：</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="61"/>
        <source>Delete Content</source>
        <translation>删除内容</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="42"/>
        <source>Add Folder</source>
        <translation>添加文件夹</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="110"/>
        <source>Recover</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="78"/>
        <source>Refresh</source>
        <translation>刷新</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="142"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="171"/>
        <source>Show All Files</source>
        <translation>显示所有文件</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="184"/>
        <source>TextLabel</source>
        <translation>文本标签</translation>
    </message>
</context>
<context>
    <name>SelectBoxWindow</name>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="32"/>
        <source>SandboxiePlus select box</source>
        <translation>SandboxiePlus 选择沙盒</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="87"/>
        <source>Select the sandbox in which to start the program, installer or document.</source>
        <translation>选择要用于运行程序、安装程序或打开文件的沙盒</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="97"/>
        <source>Run in a new Sandbox</source>
        <translation>在新沙盒中运行</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="65"/>
        <source>Sandbox</source>
        <translation>沙盒</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="80"/>
        <source>Run As UAC Administrator</source>
        <translation>以 UAC 管理员权限运行</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="47"/>
        <source>Run Sandboxed</source>
        <translation>在此沙盒内运行</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="73"/>
        <source>Run Outside the Sandbox</source>
        <translation>在沙盒外运行</translation>
    </message>
</context>
<context>
    <name>SettingsWindow</name>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="32"/>
        <source>SandboxiePlus Settings</source>
        <translation>SandboxiePlus 设置</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="55"/>
        <source>General Config</source>
        <oldsource>General Options</oldsource>
        <translation>常规选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="195"/>
        <source>Open urls from this ui sandboxed</source>
        <translation>总是在沙盒中打开设置页面的链接</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="151"/>
        <source>Run box operations asynchronously whenever possible (like content deletion)</source>
        <translation>尽可能以异步方式执行沙盒的各类操作 (如内容删除)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="65"/>
        <source>General Options</source>
        <translation>常规选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="412"/>
        <source>Show boxes in tray list:</source>
        <translation>沙盒列表显示：</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="527"/>
        <source>Add &apos;Run Un-Sandboxed&apos; to the context menu</source>
        <translation>添加“在沙盒外运行”到右键菜单</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="442"/>
        <source>Show a tray notification when automatic box operations are started</source>
        <translation>当沙盒自动化作业事件开始执行时，弹出托盘通知</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1371"/>
        <source>Activate Kernel Mode Object Filtering</source>
        <translation>激活内核模式的对象过滤器</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1468"/>
        <source>Hook selected Win32k system calls to enable GPU acceleration (experimental)</source>
        <translation>Hook 选定的 Win32k 系统调用钩子以启用 GPU 加速 (实验性)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="137"/>
        <source>Recovery Options</source>
        <translation>恢复选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="94"/>
        <source>SandMan Options</source>
        <translation>SandMan 选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="208"/>
        <source>Notifications</source>
        <translation>消息通知</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="240"/>
        <source>Add Entry</source>
        <translation>添加条目</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="357"/>
        <source>Show file migration progress when copying large files into a sandbox</source>
        <translation>将大文件复制到沙盒内部时显示文件迁移进度</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="248"/>
        <source>Message ID</source>
        <translation>消息 ID</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="253"/>
        <source>Message Text (optional)</source>
        <translation>信息文本 (可选)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="268"/>
        <source>SBIE Messages</source>
        <translation>SBIE 消息</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="287"/>
        <source>Delete Entry</source>
        <translation>删除条目</translation>
    </message>
    <message>
        <source>Don&apos;t show the popup message log for all SBIE messages</source>
        <translation type="vanished">不显示“SBIE 消息”的所有弹出式信息记录</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="301"/>
        <source>Notification Options</source>
        <translation>通知选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="327"/>
        <source>Sandboxie may be issue &lt;a href=&quot;sbie://docs/sbiemessages&quot;&gt;SBIE Messages&lt;/a&gt; to the Message Log and shown them as Popups. Some messages are informational and notify of a common, or in some cases special, event that has occurred, other messages indicate an error condition.&lt;br /&gt;You can hide selected SBIE messages from being popped up, using the below list:</source>
        <oldsource>Sandboxie may be issue &lt;a href= &quot;sbie://docs/ sbiemessages&quot;&gt;SBIE Messages&lt;/a&gt; to the Message Log and shown them as Popups. Some messages are informational and notify of a common, or in some cases special, event that has occurred, other messages indicate an error condition.&lt;br /&gt;You can hide selected SBIE messages from being popped up, using the below list:</oldsource>
        <translation>Sandboxie 可能会将 &lt;a href= &quot;sbie://docs/ sbiemessages&quot;&gt;SBIE 消息&lt;/a&gt;记录到信息日志中，并以弹出窗口的形式通知&lt;br /&gt;有些消息仅仅是信息性的，通知一个普通的或某些特殊的事件发生，其它消息表明一个错误状况&lt;br /&gt;你可以使用此列表来隐藏所设定的“SBIE 消息”，使其不被弹出：</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="340"/>
        <source>Disable SBIE messages popups (they will still be logged to the Messages tab)</source>
        <oldsource>Disable SBIE messages popups (SBIE will still be logged to the log tab)</oldsource>
        <translation>禁用 SBIE 消息通知 (SBIE 仍然会被记录到消息日志中)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="404"/>
        <source>Windows Shell</source>
        <translation>Windows 窗口管理器</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="435"/>
        <source>Start Menu Integration</source>
        <translation>开始菜单集成</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="520"/>
        <source>Scan shell folders and offer links in run menu</source>
        <translation>扫描系统 Shell 目录并在开始菜单中集成快捷方式</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="541"/>
        <source>Integrate with Host Start Menu</source>
        <translation>与主机开始菜单整合：</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="670"/>
        <source>Icon</source>
        <translation>图标</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="727"/>
        <source>Move Up</source>
        <translation>上移</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="746"/>
        <source>Move Down</source>
        <translation>下移</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="810"/>
        <source>User Interface</source>
        <translation>用户界面</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="858"/>
        <source>Use new config dialog layout *</source>
        <translation>使用新的配置对话框视图 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="949"/>
        <source>Show overlay icons for boxes and processes</source>
        <translation>为沙盒与进程显示覆盖图标</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1228"/>
        <source>Supporters of the Sandboxie-Plus project can receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;. It&apos;s like a license key but for awesome people using open source software. :-)</source>
        <translation>Sandboxie Plus项目的支持者可以收到&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;支持者证书&lt;/a&gt;。这与许可证密钥类似，但适用于使用开源软件的优秀用户。:-)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1321"/>
        <source>Keeping Sandboxie up to date with the rolling releases of Windows and compatible with all web browsers is a never-ending endeavor. You can support the development by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;directly contributing to the project&lt;/a&gt;, showing your support by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;purchasing a supporter certificate&lt;/a&gt;, becoming a patron by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;subscribing on Patreon&lt;/a&gt;, or through a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;PayPal donation&lt;/a&gt;.&lt;br /&gt;Your support plays a vital role in the advancement and maintenance of Sandboxie.</source>
        <translation>让Sandboxie跟上Windows的滚动发布并与所有网络浏览器兼容是一项永无止境的努力。您可以通过以下方式进行支持：&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;直接为项目贡献&lt;/a&gt;，通过&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;购买支持者证书&lt;/a&gt;，通过&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;在Patreon上订阅&lt;/a&gt;，或通过&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;PayPal捐赠&lt;/a&gt;。&lt;br/&gt;您的支持对Sandboxie的发展和维护起着至关重要的作用。</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1834"/>
        <source>Local Templates</source>
        <translation>本地模板</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1885"/>
        <source>Add Template</source>
        <translation>添加模板</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1905"/>
        <source>Text Filter</source>
        <translation>筛选文本</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1927"/>
        <source>This list contains user created custom templates for sandbox options</source>
        <translation>该列表包含用户为沙盒选项创建的自定义模板</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="643"/>
        <source>Run Menu</source>
        <translation>运行菜单</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="695"/>
        <source>Add program</source>
        <translation>添加程序</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="753"/>
        <source>You can configure custom entries for all sandboxes run menus.</source>
        <translation>你可以为所有沙盒配置自定义运行菜单条目</translation>
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
        <translation>命令行</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1097"/>
        <source>Support &amp;&amp; Updates</source>
        <translatorcomment>强迫症对齐，未想到合适的翻译之前，暂不打算跟进此处的翻译</translatorcomment>
        <translation>捐赠支持</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1418"/>
        <source>Default sandbox:</source>
        <translation>默认沙盒：</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1654"/>
        <source>Program Alerts</source>
        <translation>程序警报</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1710"/>
        <source>Issue message 1301 when forced processes has been disabled</source>
        <translation>当必沙进程被禁止时，提示问题代码 1301</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1944"/>
        <source>Edit ini Section</source>
        <translation>配置文本</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1953"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1960"/>
        <source>Edit ini</source>
        <translation>编辑配置</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1986"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1147"/>
        <source>Version Updates</source>
        <translation>待更新版本：</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1208"/>
        <source>New full versions from the selected release channel.</source>
        <translation>来自选定发布通道的新的完整版本</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1144"/>
        <source>Hotpatches for the installed version, updates to the Templates.ini and translations.</source>
        <translation>针对已安装版本的 Templates.ini 模板和翻译的热更新补丁</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="959"/>
        <source>Display Options</source>
        <translation>显示选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1083"/>
        <source>Graphic Options</source>
        <translation>图形选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1198"/>
        <source>The preview channel contains the latest GitHub pre-releases.</source>
        <translation>预览版通道包含最新的 GitHub 预发布版本</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1211"/>
        <source>New Versions</source>
        <translation>新版本：</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1218"/>
        <source>The stable channel contains the latest stable GitHub releases.</source>
        <translation>稳定版通道包含最新的 GitHub 稳定版本</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1221"/>
        <source>Search in the Stable channel</source>
        <translation>稳定版通道</translation>
    </message>
    <message>
        <source>Keeping Sandboxie up to date with the rolling releases of Windows and compatible with all web browsers is a never-ending endeavor. Please consider supporting this work with a donation.&lt;br /&gt;You can support the development with a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;PayPal donation&lt;/a&gt;, working also with credit cards.&lt;br /&gt;Or you can provide continuous support with a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Patreon subscription&lt;/a&gt;.</source>
        <translatorcomment>感觉用“事业”有点过于夸张，此处用“努力”感觉更贴合</translatorcomment>
        <translation type="vanished">使 Sandboxie 与 Windows 的滚动更新保持同步，并和主流浏览器保持兼容性，这是一项永无止境的努力，请考虑捐赠以支持这项工作&lt;br /&gt;您可以通过 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;PayPal 捐赠&lt;/a&gt; (支持使用信用卡付款)来支持项目的开发&lt;br /&gt;您也可以通过 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Patreon 订阅&lt;/a&gt; 来提供持续的捐赠支持</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1201"/>
        <source>Search in the Preview channel</source>
        <translation>预览版通道</translation>
    </message>
    <message>
        <source>Supporters of the Sandboxie-Plus project can receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporter certificate&lt;/a&gt;. It&apos;s like a license key but for awesome people using open source software. :-)</source>
        <translation type="vanished">Sandboxie-Plus 项目的赞助者将收到 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;赞助者凭据&lt;/a&gt;，这类似于许可密钥，是为拥抱开源软件的优秀人士准备的 :-)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1161"/>
        <source>This supporter certificate has expired, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>此赞助者凭据已过期，请&lt;a href=&quot;sbie://update/cert&quot;&gt;获取新凭据&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1241"/>
        <source>Check periodically for updates of Sandboxie-Plus</source>
        <translation>定期检查有无 Sandboxie-Plus 更新</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1171"/>
        <source>In the future, don&apos;t notify about certificate expiration</source>
        <translation>不再通知凭据过期的情况</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="626"/>
        <source>Start UI with Windows</source>
        <translation>随系统启动沙盒管理器</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="534"/>
        <source>Add &apos;Run Sandboxed&apos; to the explorer context menu</source>
        <translation>在资源管理器中添加“在沙盒中运行”右键菜单</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="633"/>
        <source>Start UI when a sandboxed process is started</source>
        <translation>随沙盒化应用启动沙盒管理器</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="80"/>
        <source>Show file recovery window when emptying sandboxes</source>
        <oldsource>Show first recovery window when emptying sandboxes</oldsource>
        <translation>在清空沙盒时显示恢复窗口</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1602"/>
        <source>Config protection</source>
        <translation>保护配置</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1488"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/ipcrootpath&quot;&gt;ipc root&lt;/a&gt;: </source>
        <translation>沙盒 &lt;a href=&quot;sbie://docs/ipcrootpath&quot;&gt;IPC&#x3000;根目录&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1391"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/keyrootpath&quot;&gt;registry root&lt;/a&gt;: </source>
        <translation>沙盒 &lt;a href=&quot;sbie://docs/keyrootpath&quot;&gt;注册表根目录&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1616"/>
        <source>Clear password when main window becomes hidden</source>
        <translation>主窗口隐藏时清除密码</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1609"/>
        <source>Only Administrator user accounts can use Pause Forcing Programs command</source>
        <oldsource>Only Administrator user accounts can use Pause Forced Programs Rules command</oldsource>
        <translation>仅管理员用户可「停用必沙程序规则」</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1531"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/filerootpath&quot;&gt;file system root&lt;/a&gt;: </source>
        <translation>沙盒 &lt;a href=&quot;sbie://docs/filerootpath&quot;&gt;文件系统根目录&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="173"/>
        <source>Hotkey for terminating all boxed processes:</source>
        <translation>终止所有沙盒内进程的快捷键:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="492"/>
        <source>Systray options</source>
        <translation>任务栏托盘区域选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="350"/>
        <source>Show recoverable files as notifications</source>
        <translation>将可恢复的文件以通知形式显示</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="185"/>
        <source>UI Language:</source>
        <translation>界面语言:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="600"/>
        <source>Show Icon in Systray:</source>
        <translation>托盘图标显示：</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="394"/>
        <source>Shell Integration</source>
        <translation>系统集成</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="513"/>
        <source>Run Sandboxed - Actions</source>
        <translation>“在沙盒中运行”选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="499"/>
        <source>Always use DefaultBox</source>
        <translation>总是使用 DefaultBox 沙盒</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="577"/>
        <source>Start Sandbox Manager</source>
        <translation>沙盒管理器启动选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1337"/>
        <source>Advanced Config</source>
        <translation>高级选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1461"/>
        <source>Use Windows Filtering Platform to restrict network access</source>
        <translation>使用 Windows 筛选平台 (WFP) 限制网络访问</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1451"/>
        <source>Sandboxing features</source>
        <translation>沙盒功能</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1347"/>
        <source>Sandboxie Config</source>
        <oldsource>Config Protection</oldsource>
        <translation>保护配置</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1623"/>
        <source>Only Administrator user accounts can make changes</source>
        <translation>仅管理员用户可更改</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1575"/>
        <source>Password must be entered in order to make changes</source>
        <translation>更改必须输入密码</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1568"/>
        <source>Change Password</source>
        <translation>更改密码</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1184"/>
        <source>Enter the support certificate here</source>
        <translation>在此输入赞助者凭据</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1268"/>
        <source>Support Settings</source>
        <translation>支持设置</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1501"/>
        <source>Portable root folder</source>
        <translation>便携化根目录</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1481"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1411"/>
        <source>Sandbox default</source>
        <translation>沙盒预设</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1630"/>
        <source>Watch Sandboxie.ini for changes</source>
        <translation>监控 Sandboxie.ini 变更</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="73"/>
        <source>Count and display the disk space occupied by each sandbox</source>
        <oldsource>Count and display the disk space ocupied by each sandbox</oldsource>
        <translation>统计并显示每个沙盒的磁盘空间占用情况</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="475"/>
        <source>Use Compact Box List</source>
        <translation>使用紧凑的沙盒列表</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="800"/>
        <source>Interface Config</source>
        <translation>界面设置</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="868"/>
        <source>Show &quot;Pizza&quot; Background in box list *</source>
        <oldsource>Show &quot;Pizza&quot; Background in box list*</oldsource>
        <translation>在沙盒列表中显示“披萨”背景 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="925"/>
        <source>Make Box Icons match the Border Color</source>
        <translation>保持沙盒内的图标与边框颜色一致</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="915"/>
        <source>Use a Page Tree in the Box Options instead of Nested Tabs *</source>
        <translation>在沙盒选项中使用页面树，而不是嵌套标签 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="908"/>
        <source>Interface Options</source>
        <translation>界面选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="828"/>
        <source>Use large icons in box list *</source>
        <translation>在沙盒列表中使用大图标 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="970"/>
        <source>High DPI Scaling</source>
        <translation>高 DPI 缩放</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="891"/>
        <source>Don&apos;t show icons in menus *</source>
        <translation>不在“工具栏/菜单列表”中显示图标 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="932"/>
        <source>Use Dark Theme</source>
        <translation>使用深色主题</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="980"/>
        <source>Font Scaling</source>
        <translation>字体缩放</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1042"/>
        <source>(Restart required)</source>
        <translation>(需要重启沙盒)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="838"/>
        <source>* a partially checked checkbox will leave the behavior to be determined by the view mode.</source>
        <translation>* 标复选框的显示效果取决于具体的视图模式</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="144"/>
        <source>Show the Recovery Window as Always on Top</source>
        <translation>始终置顶恢复文件窗口</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1056"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="942"/>
        <source>Alternate row background in lists</source>
        <translation>在沙盒列表中使用奇偶(交替)行背景色</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="818"/>
        <source>Use Fusion Theme</source>
        <translation>使用 Fusion 风格主题</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1521"/>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation>使用 Sandboxie 限权用户，而不是匿名令牌 (实验性)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="665"/>
        <location filename="Forms/SettingsWindow.ui" line="1676"/>
        <location filename="Forms/SettingsWindow.ui" line="1786"/>
        <location filename="Forms/SettingsWindow.ui" line="1919"/>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="364"/>
        <source>This option also enables asynchronous operation when needed and suspends updates.</source>
        <translation>在暂缓更新或其它需要的情况使用异步操作</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="367"/>
        <source>Suppress pop-up notifications when in game / presentation mode</source>
        <translation>在“游戏/演示”模式下，禁止弹出通知</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1681"/>
        <source>Path</source>
        <translation>路径</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1703"/>
        <source>Remove Program</source>
        <translation>删除程序</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1689"/>
        <source>Add Program</source>
        <translation>添加程序</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1724"/>
        <source>When any of the following programs is launched outside any sandbox, Sandboxie will issue message SBIE1301.</source>
        <translation>下列程序在沙盒之外启动时，Sandboxie 将提示 SBIE1301 警告</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1734"/>
        <source>Add Folder</source>
        <translation>添加文件夹</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1717"/>
        <source>Prevent the listed programs from starting on this system</source>
        <translation>阻止下列程序在此系统中启动</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1560"/>
        <source>Sandboxie.ini Presets</source>
        <translation>Sandboxie.ini 预设选项</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1644"/>
        <source>Program Control</source>
        <translation>程序控制</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1696"/>
        <source>Issue message 1308 when a program fails to start</source>
        <translation>程序启动失败时，提示问题代码 1308</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1748"/>
        <source>App Templates</source>
        <translation>应用模板</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1758"/>
        <source>App Compatibility</source>
        <translation>软件兼容性</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1778"/>
        <source>In the future, don&apos;t check software compatibility</source>
        <translation>之后不再检查软件兼容性</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1794"/>
        <source>Enable</source>
        <translation>启用</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1814"/>
        <source>Disable</source>
        <translation>禁用</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1821"/>
        <source>Sandboxie has detected the following software applications in your system. Click OK to apply configuration settings, which will improve compatibility with these applications. These configuration settings will have effect in all existing sandboxes and in any new sandboxes.</source>
        <translation>沙盒已检测到系统中安装了以下软件，点击“确定”应用配置，将改进与这些软件的兼容性，这些配置将作用于所有沙盒，包括现存和未来新增的沙盒</translation>
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
        <translation>所选快照详情</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="66"/>
        <source>Name:</source>
        <translation>名称：</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="120"/>
        <source>Description:</source>
        <translation>说明：</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="101"/>
        <source>When deleting a snapshot content, it will be returned to this snapshot instead of none.</source>
        <translation>当删除一个快照时，它将被回退到此快照创建时的状态，而不是直接清空</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="104"/>
        <source>Default snapshot</source>
        <translation>默认快照</translation>
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
        <translation>进入快照</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="157"/>
        <source>Take Snapshot</source>
        <translation>抓取快照</translation>
    </message>
</context>
</TS>
