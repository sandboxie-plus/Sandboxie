<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ko_KR">
<context>
    <name>CAdvancedPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="428"/>
        <source>Advanced Sandbox options</source>
        <translation>고급 샌드박스 옵션</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="429"/>
        <source>On this page advanced sandbox options can be configured.</source>
        <translation>이 페이지에서 고급 샌드박스 옵션을 구성할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="434"/>
        <source>Network Access</source>
        <translation>네트워크 액세스</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="442"/>
        <source>Allow network/internet access</source>
        <translation>네트워크/인터넷 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="443"/>
        <source>Block network/internet by denying access to Network devices</source>
        <translation>네트워크 장치에 대한 액세스를 거부하여 네트워크/인터넷 차단</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="445"/>
        <source>Block network/internet using Windows Filtering Platform</source>
        <translation>Windows 필터링 플랫폼을 사용하여 네트워크/인터넷 차단</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="450"/>
        <source>Allow access to network files and folders</source>
        <oldsource>Allow access to network files and fodlers</oldsource>
        <translation>네트워크 파일 및 폴더에 대한 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="451"/>
        <location filename="Wizards/NewBoxWizard.cpp" line="467"/>
        <source>This option is not recommended for Hardened boxes</source>
        <oldsource>This option is not recomended for Hardened boxes</oldsource>
        <translation>강화 상자에는 이 옵션을 사용하지 않는 것이 좋습니다</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="457"/>
        <source>Admin Options</source>
        <translation>관리자 옵션</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="461"/>
        <source>Make applications think they are running elevated</source>
        <translation>응용 프로그램이 권한 상승으로 실행되고 있다고 생각하게 합니다</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="466"/>
        <source>Allow MSIServer to run with a sandboxed system token</source>
        <translation>MSI 서버가 샌드박스 시스템 토큰으로 실행되도록 허용</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="472"/>
        <source>Box Options</source>
        <translation>박스 옵션</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="476"/>
        <source>Use a Sandboxie login instead of an anonymous token</source>
        <translation>익명 토큰 대신 샌드박스 로그인 사용</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="482"/>
        <source>Prevent sandboxes programs installed on host from loading dll&apos;s from the sandbox</source>
        <translation>호스트에 설치된 샌드박스 프로그램이 샌드박스에서 dll을 로드하지 못하도록 방지</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="483"/>
        <source>This feature may reduce compatibility as it also prevents box located processes from writing to host located ones and even starting them.</source>
        <oldsource>This feature may reduce compatybility as it also prevents box located processes from writing to host located once and even starting them.</oldsource>
        <translation>이 기능은 박스에 위치한 프로세스가 한 번 위치한 호스트에 기록되고 심지어 시작되는 것을 방지하기 때문에 호환성을 줄일 수 있습니다.</translation>
    </message>
    <message>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation type="vanished">익명 토큰 대신 샌드박스 로그인 사용 (실험)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="477"/>
        <source>Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.</source>
        <translation>사용자 지정 샌드박스 토큰을 사용하면 개별 샌드박스를 서로 더 잘 격리할 수 있으며, 프로세스가 속한 상자의 이름을 작업 관리자의 사용자 열에 표시합니다. 그러나 일부 타사 보안 솔루션은 사용자 지정 토큰에 문제가 있을 수 있습니다.</translation>
    </message>
</context>
<context>
    <name>CBoxTypePage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="168"/>
        <source>Create new Sandbox</source>
        <translation>새 샌드박스 만들기</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="175"/>
        <source>A sandbox isolates your host system from processes running within the box, it prevents them from making permanent changes to other programs and data in your computer. The level of isolation impacts your security as well as the compatibility with applications, hence there will be a different level of isolation depending on the selected Box Type. Sandboxie can also protect your personal data from being accessed by processes running under its supervision.</source>
        <translation>샌드박스는 사용자의 호스트 시스템을 박스 내에서 실행 중인 프로세스에서 분리하여 사용자의 컴퓨터에 있는 다른 프로그램 및 데이터를 영구적으로 변경하지 못하도록 합니다. 격리 수준은 응용프로그램과의 호환성뿐만 아니라 보안에도 영향을 미치므로 선택한 박스 유형에 따라 격리 수준이 달라집니다. Sandboxie는 또한 Sandboxie의 감독 하에 실행되는 프로세스에 의해 개인 데이터가 액세스되는 것을 방지할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="184"/>
        <source>Enter box name:</source>
        <translation>박스 이름 입력:</translation>
    </message>
    <message>
        <source>New Box</source>
        <translation type="vanished">새 박스</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="202"/>
        <source>Select box type:</source>
        <oldsource>Sellect box type:</oldsource>
        <translation>박스 유형 선택:</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="205"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>데이터 보호 기능을 갖춘 강화된 샌드박스</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="206"/>
        <source>Security Hardened Sandbox</source>
        <translation>보안 강화된 샌드박스</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="207"/>
        <source>Sandbox with Data Protection</source>
        <translation>데이터 보호 기능이 있는 샌드박스</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="208"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>표준 분리 샌드박스 (기본값)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="210"/>
        <source>Application Compartment with Data Protection</source>
        <translation>데이터 보호 기능이 있는 응용 프로그램 구획</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="211"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>응용 프로그램 구획 (격리 없음)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="229"/>
        <source>Remove after use</source>
        <translation>사용 후 제거</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="230"/>
        <source>After the last process in the box terminates, all data in the box will be deleted and the box itself will be removed.</source>
        <translation>박스의 마지막 프로세스가 종료되면 박스의 모든 데이터가 삭제되고 박스 자체가 제거됩니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="235"/>
        <source>Configure advanced options</source>
        <translation>고급 옵션 구성</translation>
    </message>
</context>
<context>
    <name>CBrowserOptionsPage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="844"/>
        <source>Create Web Browser Template</source>
        <translation>웹 브라우저 템플릿 만들기</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="845"/>
        <source>Configure web browser template options.</source>
        <translation>웹 브라우저 템플릿 옵션을 구성합니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="855"/>
        <source>Force the Web Browser to run in this sandbox</source>
        <translation>이 샌드박스에서 웹 브라우저 실행</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="859"/>
        <source>Allow direct access to the entire Web Browser profile folder</source>
        <translation>전체 웹 브라우저 프로필 폴더에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="863"/>
        <source>Allow direct access to Web Browser&apos;s phishing database</source>
        <translation>웹 브라우저의 피싱 데이터베이스에 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="867"/>
        <source>Allow direct access to Web Browser&apos;s session management</source>
        <translation>웹 브라우저의 세션 관리에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="871"/>
        <source>Allow direct access to Web Browser&apos;s sync data</source>
        <translation>웹 브라우저의 동기화 데이터에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="875"/>
        <source>Allow direct access to Web Browser&apos;s preferences</source>
        <translation>웹 브라우저 환경설정에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="879"/>
        <source>Allow direct access to Web Browser&apos;s passwords</source>
        <translation>웹 브라우저의 암호에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="883"/>
        <source>Allow direct access to Web Browser&apos;s cookies</source>
        <translation>웹 브라우저의 쿠키에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="887"/>
        <source>Allow direct access to Web Browser&apos;s bookmarks</source>
        <translation>웹 브라우저의 북마크에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="891"/>
        <source>Allow direct access to Web Browser&apos;s bookmark and history database</source>
        <translation>웹 브라우저의 북마크 및 기록 데이터베이스에 대한 직접 액세스 허용</translation>
    </message>
</context>
<context>
    <name>CBrowserPathsPage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="517"/>
        <source>Create Web Browser Template</source>
        <translation>웹 브라우저 템플릿 만들기</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="518"/>
        <source>Configure your Web Browser&apos;s profile directories.</source>
        <translation>웹 브라우저 프로필 디렉터리를 구성합니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="526"/>
        <source>User profile(s) directory:</source>
        <translation>사용자 프로필 디렉터리:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="528"/>
        <source>Show also imperfect matches</source>
        <translation>불완전한 일치 항목도 표시</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="534"/>
        <source>Browser Executable (*.exe)</source>
        <translation>브라우저 실행 파일 (*.exe)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="551"/>
        <source>Continue without browser profile</source>
        <translation>브라우저 프로필 없이 계속</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="712"/>
        <source>Configure your Gecko based Browsers profile directories.</source>
        <translation>Gecko 기반 브라우저 프로필 디렉터리를 구성합니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="716"/>
        <source>Configure your Chromium based Browsers profile directories.</source>
        <translation>Cromium 기반 브라우저 프로필 디렉터리를 구성합니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="796"/>
        <source>No suitable folders have been found.
Note: you need to run the browser unsandboxed for them to get created.
Please browse to the correct user profile directory.</source>
        <translation>적합한 사료를 찾을 수 없습니다.
참고: 브라우저를 생성하려면 샌드박스 없이 실행해야 합니다.
올바른 사용자 프로필 디렉터리를 찾아보십시오.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="801"/>
        <source>Please choose the correct user profile directory, if it is not listed you may need to browse to it.</source>
        <translation>올바른 사용자 프로필 디렉터리를 선택하십시오. 목록에 없으면 찾아봐야 할 수도 있습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="807"/>
        <source>Please ensure the selected directory is correct, the wizard is not confident in all the presented options.</source>
        <translation>선택한 디렉터리가 올바른지 확인하십시오. 마법사가 표시된 모든 옵션을 신뢰할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="810"/>
        <source>Please ensure the selected directory is correct.</source>
        <translation>선택한 디렉터리가 올바른지 확인하십시오.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="815"/>
        <source>This path does not look like a valid profile directory.</source>
        <translation>이 경로는 올바른 프로필 디렉터리처럼 보이지 않습니다.</translation>
    </message>
</context>
<context>
    <name>CBrowserTypePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="325"/>
        <source>Create Web Browser Template</source>
        <translation>웹 브라우저 템플릿 만들기</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="326"/>
        <source>Select your Web Browsers main executable, this will allow Sandboxie to identify the browser.</source>
        <oldsource>Select your Web Browsers main executable, this will allow sandboxie to identify the browser.</oldsource>
        <translation>웹 브라우저 기본 실행 파일을 선택하면 샌드박스가 브라우저를 식별할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="336"/>
        <source>Enter browser name:</source>
        <translation>브라우저 이름 입력:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="347"/>
        <source>Main executable (eg. firefox.exe, chrome.exe, msedge.exe, etc...):</source>
        <oldsource>Mein executable (eg. firefox.exe, chrome.exe, msedge.exe, etc...):</oldsource>
        <translation>기본 실행 파일 (예: firefox.exe, chrome.exe, msedge.exe 등...):</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="351"/>
        <source>Browser executable (*.exe)</source>
        <oldsource>Browser Executable (*.exe)</oldsource>
        <translation>브라우저 실행 파일 (*.exe)</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="451"/>
        <source>The browser appears to be Gecko based, like Mozilla Firefox and its derivatives.</source>
        <translation>브라우저는 Mozilla Firefox 및 그 파생 제품처럼 Gecko 기반인 것으로 보입니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="454"/>
        <source>The browser appears to be Chromium based, like Microsoft Edge or Google Chrome and its derivatives.</source>
        <translation>브라우저는 Microsoft Edge 또는 Google Chrome 및 그 파생 제품과 같이 Chrome 기반으로 보입니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="457"/>
        <source>Browser could not be recognized, template cannot be created.</source>
        <translation>브라우저를 인식할 수 없습니다. 템플릿을 만들 수 없습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="504"/>
        <source>This browser name is already in use, please choose an other one.</source>
        <translation>이 브라우저 이름은 이미 사용 중입니다. 다른 브라우저 이름을 선택하십시오.</translation>
    </message>
</context>
<context>
    <name>CCertificatePage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="195"/>
        <source>Install your &lt;b&gt;Sandboxie-Plus&lt;/b&gt; support certificate</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; 지원 인증서를 설치합니다</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="196"/>
        <source>If you have a supporter certificate, please fill it into the field below.</source>
        <translation>후원자 인증서가 있으면 아래 필드에 입력하십시오.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="218"/>
        <source>Start evaluation without a certificate for a limited period of time.</source>
        <translation>제한된 기간 동안 인증서 없이 평가를 시작합니다.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="246"/>
        <source>To use &lt;b&gt;Sandboxie-Plus&lt;/b&gt; in a business setting, an appropriate &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;support certificate&lt;/a&gt; for business use is required. If you do not yet have the required certificate(s), you can get those from the &lt;a href=&quot;https://xanasoft.com/shop/&quot;&gt;xanasoft.com web shop&lt;/a&gt;.</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt;를 비즈니스용으로 사용하려면 비즈니스 사용에 적합한 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;후원 인증서&lt;/a&gt;가 필요합니다. 필요한 인증서가 아직 없는 경우 &lt;a href=&quot;https://xanasoft.com/shop/&quot;&gt;xanasoft.com 웹 상점에서 인증서를 받을 수 있습니다&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="260"/>
        <source>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; provides additional features and box types exclusively to &lt;u&gt;project supporters&lt;/u&gt;. Boxes like the Privacy Enhanced boxes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;protect user data from illicit access&lt;/font&gt;&lt;/b&gt; by the sandboxed programs. If you are not yet a supporter, then please consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt; to ensure further development of Sandboxie and to receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;.</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt;는 &lt;u&gt;프로젝트 후원자&lt;/u&gt;에게만 추가 기능과 박스 유형을 제공합니다. 개인 정보 강화 박스와 같은 박스는 샌드박스 프로그램의 &lt;b&gt;&lt;font color=&apos;red&apos;&gt;불법 액세스으로부터 사용자 데이터를 보호&lt;/font&gt;&lt;/b&gt;합니다. 아직 후원자가 되지 않았다면 샌드박스의 추가 개발 및 후원자 인증서 발급을 위한 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;프로젝트 지원을 고려해주시기 바랍니다.&lt;/a&gt;.</translation>
    </message>
</context>
<context>
    <name>CCleanUpJob</name>
    <message>
        <location filename="BoxJob.h" line="36"/>
        <source>Deleting Content</source>
        <translation>내용 삭제 중</translation>
    </message>
</context>
<context>
    <name>CFileBrowserWindow</name>
    <message>
        <location filename="Views/FileView.cpp" line="398"/>
        <source>%1 - Files</source>
        <translation>%1 - 파일</translation>
    </message>
</context>
<context>
    <name>CFileView</name>
    <message>
        <location filename="Views/FileView.cpp" line="188"/>
        <source>Create Shortcut</source>
        <translation>바로가기 만들기</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="210"/>
        <source>Pin to Box Run Menu</source>
        <translation>박스 실행 메뉴에 고정</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="217"/>
        <source>Recover to Any Folder</source>
        <translation>임의의 폴더로 복구</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="219"/>
        <source>Recover to Same Folder</source>
        <translation>동일한 폴더로 복구</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="223"/>
        <source>Run Recovery Checks</source>
        <translation>복구 검사 실행</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="287"/>
        <source>Select Directory</source>
        <translation>디렉터리 선택</translation>
    </message>
    <message>
        <location filename="Views/FileView.cpp" line="353"/>
        <source>Create Shortcut to sandbox %1</source>
        <translation>sandbox %1로 바로가기 만들기</translation>
    </message>
</context>
<context>
    <name>CFilesPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="302"/>
        <source>Sandbox location and behavior</source>
        <oldsource>Sandbox location and behavioure</oldsource>
        <translation>샌드박스 위치 및 동작</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="303"/>
        <source>On this page the sandbox location and its behavior can be customized.
You can use %USER% to save each users sandbox to an own folder.</source>
        <oldsource>On this page the sandbox location and its behaviorue can be customized.
You can use %USER% to save each users sandbox to an own fodler.</oldsource>
        <translation>이 페이지에서 샌드박스 위치 및 해당 동작을 사용자 정의할 수 있습니다.
%USER%를 사용하여 각 사용자 샌드박스를 자신의 폴더에 저장할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="308"/>
        <source>Sandboxed Files</source>
        <translation>샌드박스 파일</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="331"/>
        <source>Select Directory</source>
        <translation>디렉터리 선택</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="339"/>
        <source>Virtualization scheme</source>
        <translation>가상화 구성표</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="343"/>
        <source>Version 1</source>
        <translation>버전 1</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="344"/>
        <source>Version 2</source>
        <translation>버전 2</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="350"/>
        <source>Separate user folders</source>
        <translation>개별 사용자 폴더</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="355"/>
        <source>Use volume serial numbers for drives</source>
        <translation>드라이브에 볼륨 일련 번호 사용</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="360"/>
        <source>Auto delete content when last process terminates</source>
        <translation>마지막 프로세스가 종료될 때 내용 자동 삭제</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="367"/>
        <source>Enable Immediate Recovery of files from recovery locations</source>
        <translation>복구 위치에서 파일 즉시 복구 사용</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="402"/>
        <source>The selected box location is not a valid path.</source>
        <oldsource>The sellected box location is not a valid path.</oldsource>
        <translation>선택한 박스 위치가 올바른 경로가 아닙니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="407"/>
        <source>The selected box location exists and is not empty, it is recommended to pick a new or empty folder. Are you sure you want to use an existing folder?</source>
        <oldsource>The sellected box location exists and is not empty, it is recomended to pick a new or empty folder. Are you sure you want to use an existing folder?</oldsource>
        <translation>선택한 박스 위치가 존재하며 비어 있지 않습니다. 새 폴더나 빈 폴더를 선택하는 것이 좋습니다. 기존 폴더를 사용하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="412"/>
        <source>The selected box location is not placed on a currently available drive.</source>
        <oldsource>The selected box location not placed on a currently available drive.</oldsource>
        <translation>선택한 박스 위치가 현재 사용 가능한 드라이브에 있지 않습니다.</translation>
    </message>
</context>
<context>
    <name>CFinishPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="489"/>
        <source>Complete your configuration</source>
        <translation>구성을 완료합니다</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="496"/>
        <source>Almost complete, click Finish to apply all selected options and conclude the wizard.</source>
        <translation>거의 완료되었습니다. 선택한 옵션을 모두 적용하고 마법사를 끝내려면 마침을 클릭하십시오.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="508"/>
        <source>Keep Sandboxie-Plus up to date.</source>
        <translation>Sandboxie-Plus를 최신 상태로 유지합니다.</translation>
    </message>
</context>
<context>
    <name>CFinishTemplatePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="936"/>
        <source>Create Web Browser Template</source>
        <translation>웹 브라우저 템플릿 만들기</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="944"/>
        <source>Almost complete, click Finish to create a new  Web Browser Template and conclude the wizard.</source>
        <translation>거의 완료되면 마침을 클릭하여 새 웹 브라우저 템플릿을 만들고 마법사를 완료합니다.</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="962"/>
        <source>Browser name: %1
</source>
        <translation>브라우저 이름: %1
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="966"/>
        <source>Browser Type: Gecko (Mozilla Firefox)
</source>
        <translation>브라우저 유형: Gecko (Mozilla Firefox)
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="967"/>
        <source>Browser Type: Chromium (Google Chrome)
</source>
        <translation>브라우저 유형: Chromium (Google Chrome)
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
        <translation>브라우저 실행 파일 경로: %1
</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="980"/>
        <source>Browser profile path: %1
</source>
        <translation>브라우저 프로필 경로: %1
</translation>
    </message>
</context>
<context>
    <name>CIntroPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="117"/>
        <source>Introduction</source>
        <translation>소개</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="121"/>
        <source>Welcome to the Setup Wizard. This wizard will help you to configure your copy of &lt;b&gt;Sandboxie-Plus&lt;/b&gt;. You can start this wizard at any time from the Sandbox-&gt;Maintenance menu if you do not wish to complete it now.</source>
        <translation>설정 마법사에 오신 것을 환영합니다. 이 마법사는 &lt;b&gt;Sandboxie-Plus&lt;/b&gt;의 사본을 구성할 수 있도록 도와줍니다. 지금 완료하지 않으려면 샌드박스-&gt;유지관리 메뉴에서 언제든지 이 마법사를 시작할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="130"/>
        <source>Select how you would like to use Sandboxie-Plus</source>
        <translation>Sandboxie-Plus 사용 방법 선택</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="133"/>
        <source>&amp;Personally, for private non-commercial use</source>
        <translation>개인적, 사적인 비상업적으로 사용(&amp;P)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="138"/>
        <source>&amp;Commercially, for business or enterprise use</source>
        <translation>상업적, 사업 또는 기업용으로 사용(&amp;C)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="143"/>
        <source>Note: this option is persistent</source>
        <translation>참고: 이 옵션은 영구적입니다</translation>
    </message>
</context>
<context>
    <name>CMonitorModel</name>
    <message>
        <location filename="Models/MonitorModel.cpp" line="147"/>
        <source>Type</source>
        <translation>유형</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="148"/>
        <source>Status</source>
        <translation>상태</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="149"/>
        <source>Value</source>
        <translation>값</translation>
    </message>
    <message>
        <location filename="Models/MonitorModel.cpp" line="150"/>
        <source>Count</source>
        <translation>갯수</translation>
    </message>
</context>
<context>
    <name>CMultiErrorDialog</name>
    <message>
        <location filename="Dialogs/MultiErrorDialog.cpp" line="10"/>
        <source>Sandboxie-Plus - Error</source>
        <translation>Sandboxie-Plus - 오류</translation>
    </message>
    <message>
        <location filename="Dialogs/MultiErrorDialog.cpp" line="19"/>
        <source>Message</source>
        <translation>메시지</translation>
    </message>
</context>
<context>
    <name>CNewBoxWindow</name>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="23"/>
        <source>Sandboxie-Plus - Create New Box</source>
        <translation>Sandboxie-Plus - 새 박스 만들기</translation>
    </message>
    <message>
        <source>New Box</source>
        <translation type="vanished">새 박스</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="39"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>데이터 보호 기능을 갖춘 강화 샌드박스</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="40"/>
        <source>Security Hardened Sandbox</source>
        <translation>보안 강화 샌드박스</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="41"/>
        <source>Sandbox with Data Protection</source>
        <translation>데이터 보호 기능이 있는 샌드박스</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="42"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>표준 분리 샌드박스 (기본값)</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="44"/>
        <source>Application Compartment with Data Protection</source>
        <translation>데이터 보호 기능이 있는 응용 프로그램 구획</translation>
    </message>
    <message>
        <location filename="Windows/NewBoxWindow.cpp" line="45"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>응용 프로그램 구획 (격리 없음)</translation>
    </message>
</context>
<context>
    <name>CNewBoxWizard</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="30"/>
        <source>New Box Wizard</source>
        <translation>새 박스 마법사</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="138"/>
        <source>The new sandbox has been created using the new &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;Virtualization Scheme Version 2&lt;/a&gt;, if you experience any unexpected issues with this box, please switch to the Virtualization Scheme to Version 1 and report the issue, the option to change this preset can be found in the Box Options in the Box Structure group.</source>
        <oldsource>The new sandbox has been created using the new &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;Virtualization Scheme Version 2&lt;/a&gt;, if you expirience any unecpected issues with this box, please switch to the Virtualization Scheme to Version 1 and report the issue, the option to change this preset can be found in the Box Options in the Box Structure groupe.</oldsource>
        <translation>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-delete-v2&quot;&gt;새 가상화 구성표 버전 2&lt;/a&gt;를 사용하여 새 샌드박스가 생성되었습니다. 이 박스에서 예기치 않은 문제가 발생하면 가상화 구성표를 버전 1로 전환하고 문제를 보고하십시오. 이 사전 설정을 변경하는 옵션은 박스 구조 그룹의 박스 옵션에서 찾을 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="141"/>
        <source>Don&apos;t show this message again.</source>
        <translation>이 메시지를 다시 표시하지 않습니다.</translation>
    </message>
</context>
<context>
    <name>COnDeleteJob</name>
    <message>
        <location filename="BoxJob.h" line="58"/>
        <source>OnDelete: %1</source>
        <translation>삭제 시: %1</translation>
    </message>
</context>
<context>
    <name>COnlineUpdater</name>
    <message>
        <location filename="OnlineUpdater.cpp" line="99"/>
        <source>Do you want to check if there is a new version of Sandboxie-Plus?</source>
        <translation>새 버전의 Sandboxie-Plus가 있는지 확인하시겠습니까?</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="100"/>
        <source>Don&apos;t show this message again.</source>
        <translation>이 메시지를 다시 표시하지 않습니다.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="209"/>
        <source>Checking for updates...</source>
        <translation>업데이트를 확인하는 중...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="248"/>
        <source>server not reachable</source>
        <translation>서버에 연결할 수 없음</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="249"/>
        <location filename="OnlineUpdater.cpp" line="251"/>
        <source>Failed to check for updates, error: %1</source>
        <translation>업데이트를 확인하지 못했습니다. 오류: %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="404"/>
        <source>&lt;p&gt;Do you want to download the installer?&lt;/p&gt;</source>
        <translation>&lt;p&gt;설치 프로그램을 다운로드하시겠습니까?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="406"/>
        <source>&lt;p&gt;Do you want to download the updates?&lt;/p&gt;</source>
        <translation>&lt;p&gt;업데이트를 다운로드하시겠습니까?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="408"/>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;update page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation>&lt;p&gt;&lt;a href=&quot;%1&quot;&gt;업데이트 페이지&lt;/a&gt;로 이동하시겠습니까??&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="416"/>
        <source>Don&apos;t show this update anymore.</source>
        <translation>이 업데이트를 더 이상 표시하지 않습니다.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="534"/>
        <source>Downloading updates...</source>
        <translation>업데이트 다운로드 중...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="560"/>
        <source>invalid parameter</source>
        <translation>잘못된 매개변수</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="561"/>
        <source>failed to download updated information</source>
        <oldsource>failed to download update informations</oldsource>
        <translation>업데이트 정보를 다운로드하지 못했습니다</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="562"/>
        <source>failed to load updated json file</source>
        <oldsource>failed to load update json file</oldsource>
        <translation>업데이트 json 파일을 불러오지 못했습니다</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="563"/>
        <source>failed to download a particular file</source>
        <translation>특정 파일을 다운로드하지 못했습니다</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="564"/>
        <source>failed to scan existing installation</source>
        <translation>기존 설치를 검색하지 못했습니다</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="565"/>
        <source>updated signature is invalid !!!</source>
        <oldsource>update signature is invalid !!!</oldsource>
        <translation>업데이트 서명이 잘못되었습니다 !!!</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="566"/>
        <source>downloaded file is corrupted</source>
        <translation>다운로드한 파일이 손상되었습니다</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="567"/>
        <source>internal error</source>
        <translation>내부 오류</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="568"/>
        <source>unknown error</source>
        <translation>알 수 없는 오류</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="592"/>
        <source>Failed to download updates from server, error %1</source>
        <translation>서버에서 업데이트를 다운로드하지 못했습니다, 오류 %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="611"/>
        <source>&lt;p&gt;Updates for Sandboxie-Plus have been downloaded.&lt;/p&gt;&lt;p&gt;Do you want to apply these updates? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Sandboxie-Plus에 대한 업데이트가 다운로드되었습니다.&lt;/p&gt;&lt;p&gt;이 업데이트를 적용하시겠습니까? 샌드박스로 실행 중인 프로그램이 있으면 프로그램이 종료됩니다.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="715"/>
        <source>Downloading installer...</source>
        <translation>설치 프로그램을 다운로드하는 중...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="757"/>
        <source>Failed to download installer from: %1</source>
        <translation>설치 관리자를 다운로드하지 못했습니다: %1</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="783"/>
        <source>&lt;p&gt;A new Sandboxie-Plus installer has been downloaded to the following location:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Do you want to begin the installation? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation>&lt;p&gt;새 Sandboxie-Plus 설치 관리자가 다음 위치에 다운로드되었습니다:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;설치를 시작하시겠습니까? 샌드박스로 실행 중인 프로그램이 있으면 종료됩니다.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="848"/>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;info page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation>&lt;p&gt; &lt;a href=&quot;%1&quot;&gt;정보 페이지로 이동하시겠습니까&lt;/a&gt;?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="856"/>
        <source>Don&apos;t show this announcement in the future.</source>
        <translation>앞으로 이 공지사항을 표시하지 않습니다.</translation>
    </message>
    <message>
        <source>&lt;p&gt;There is a new version of Sandboxie-Plus available.&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;New version:&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</source>
        <translation type="vanished">&lt;p&gt;사용할 수 있는 새로운 버전의 Sandboxie-Plus가 있습니다.&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;새 버전:&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="398"/>
        <source>&lt;p&gt;There is a new version of Sandboxie-Plus available.&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;&lt;b&gt;New version:&lt;/b&gt;&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;사용할 수 있는 새로운 버전의 Sandboxie-Plus가 있습니다.&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;&lt;b&gt;새 버전:&lt;/b&gt;&lt;/font&gt; &lt;b&gt;%1&lt;/b&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p&gt;Do you want to download the latest version?&lt;/p&gt;</source>
        <translation type="vanished">&lt;p&gt;최신 버전을 다운로드하시겠습니까?&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p&gt;Do you want to go to the &lt;a href=&quot;%1&quot;&gt;download page&lt;/a&gt;?&lt;/p&gt;</source>
        <translation type="vanished">&lt;p&gt; &lt;a href=&quot;%1&quot;&gt;다운로드 페이지로 이동하시겠습니까&lt;/a&gt;?&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Don&apos;t show this message anymore.</source>
        <translation type="vanished">이 메시지를 더 이상 표시하지 않습니다.</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="274"/>
        <source>No new updates found, your Sandboxie-Plus is up-to-date.

Note: The update check is often behind the latest GitHub release to ensure that only tested updates are offered.</source>
        <translation>새 업데이트를 찾을 수 없습니다. Sandboxie-Plus가 최신 상태입니다.

참고: 테스트된 업데이트만 제공되도록 최신 GitHub 릴리스보다 업데이트 검사가 늦은 경우가 많습니다.</translation>
    </message>
    <message>
        <source>Downloading new version...</source>
        <translation type="vanished">새 버전을 다운로드하는 중...</translation>
    </message>
    <message>
        <source>Failed to download update from: %1</source>
        <translation type="vanished">업데이트를 다운로드하지 못했습니다: %1</translation>
    </message>
    <message>
        <source>&lt;p&gt;A Sandboxie-Plus update has been downloaded to the following location:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Do you want to begin the installation? If any programs are running sandboxed, they will be terminated.&lt;/p&gt;</source>
        <translation type="vanished">&lt;p&gt;A Sandboxie-Plus 업데이트가 다음 위치에 다운로드되었습니다:&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;%2&quot;&gt;%1&lt;/a&gt;&lt;/p&gt;&lt;p&gt;설치를 시작하시겠습니까? 샌드박스를 실행 중인 프로그램이 있으면 프로그램이 종료됩니다.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="964"/>
        <source>Checking for certificate...</source>
        <translation>인증서를 확인하는 중...</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="1004"/>
        <source>No certificate found on server!</source>
        <translation>서버에서 인증서를 찾을 수 없습니다!</translation>
    </message>
    <message>
        <location filename="OnlineUpdater.cpp" line="1009"/>
        <source>There is no updated certificate available.</source>
        <translation>업데이트된 인증서가 없습니다.</translation>
    </message>
</context>
<context>
    <name>COptionsWindow</name>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="446"/>
        <location filename="Windows/OptionsWindow.cpp" line="460"/>
        <location filename="Windows/OptionsAccess.cpp" line="24"/>
        <source>Browse for File</source>
        <translation>파일 찾아보기</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="25"/>
        <source>Browse for Folder</source>
        <translation>폴더 찾아보기</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="312"/>
        <source>Closed</source>
        <translation>닫힘</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="313"/>
        <source>Closed RT</source>
        <translation>RT 닫힘</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="314"/>
        <source>Read Only</source>
        <translation>읽기 전용</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="308"/>
        <source>Normal</source>
        <translation>일반</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="309"/>
        <source>Open</source>
        <translation>열기</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="310"/>
        <source>Open for All</source>
        <translation>모두 열기</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="311"/>
        <source>No Rename</source>
        <translation>이름 바꾸기 안 함</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="315"/>
        <source>Box Only (Write Only)</source>
        <translation>박스 전용 (쓰기 전용)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="316"/>
        <source>Ignore UIPI</source>
        <translation>UIPI 무시</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="318"/>
        <location filename="Windows/OptionsAccess.cpp" line="335"/>
        <location filename="Windows/OptionsAccess.cpp" line="348"/>
        <source>Unknown</source>
        <translation>알 수 없음</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="325"/>
        <source>Regular Sandboxie behavior - allow read and also copy on write.</source>
        <translation>정기 Sandboxie 동작 - 읽기를 허용하고 쓰기 시 복사를 허용합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="326"/>
        <source>Allow write-access outside the sandbox.</source>
        <translation>샌드박스 외부에서 쓰기 액세스를 허용합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="327"/>
        <source>Allow write-access outside the sandbox, also for applications installed inside the sandbox.</source>
        <translation>샌드박스 외부에 쓰기 액세스를 허용하고, 샌드박스 내부에 설치된 응용 프로그램에도 쓰기 액세스를 허용합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="328"/>
        <source>Don&apos;t rename window classes.</source>
        <translation>창 클래스 이름을 바꾸지 않습니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="329"/>
        <source>Deny access to host location and prevent creation of sandboxed copies.</source>
        <translation>호스트 위치에 대한 액세스를 거부하고 샌드박스 복사본이 생성되지 않도록 합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="330"/>
        <source>Block access to WinRT class.</source>
        <translation>WinRT 클래스에 대한 액세스를 차단합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="331"/>
        <source>Allow read-only access only.</source>
        <translation>읽기 전용 액세스만 허용합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="332"/>
        <source>Hide host files, folders or registry keys from sandboxed processes.</source>
        <translation>샌드박스 프로세스에서 호스트 파일, 폴더 또는 레지스트리 키를 숨깁니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="333"/>
        <source>Ignore UIPI restrictions for processes.</source>
        <translation>프로세스에 대한 UIPI 제한을 무시합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="342"/>
        <source>File/Folder</source>
        <translation>파일/폴더</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="343"/>
        <source>Registry</source>
        <translation>레지스트리</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="344"/>
        <source>IPC Path</source>
        <translation>IPC 경로</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="345"/>
        <source>Wnd Class</source>
        <translation>창 클래스</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="346"/>
        <source>COM Object</source>
        <translation>COM 개체</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="353"/>
        <source>Select File</source>
        <translation>파일 선택</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="353"/>
        <source>All Files (*.*)</source>
        <translation>모든 파일 (*.*)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="365"/>
        <location filename="Windows/OptionsForce.cpp" line="242"/>
        <location filename="Windows/OptionsForce.cpp" line="252"/>
        <location filename="Windows/OptionsRecovery.cpp" line="128"/>
        <location filename="Windows/OptionsRecovery.cpp" line="139"/>
        <source>Select Directory</source>
        <translation>디렉터리 선택</translation>
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
        <translation>모든 프로그램</translation>
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
        <translation>그룹: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="537"/>
        <source>COM objects must be specified by their GUID, like: {00000000-0000-0000-0000-000000000000}</source>
        <translation>COM 개체는 다음과 같이 GUID로 지정해야 합니다: {00000000-0000-0000-0000-000000000000}</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="543"/>
        <source>RT interfaces must be specified by their name.</source>
        <translation>RT 인터페이스는 이름으로 지정해야 합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="557"/>
        <source>Opening all IPC access also opens COM access, do you still want to restrict COM to the sandbox?</source>
        <translation>모든 IPC 액세스를 열면 COM 액세스도 열립니다. 여전히 COM을 샌드박스로 제한하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="558"/>
        <source>Don&apos;t ask in future</source>
        <translation>앞으로 묻지 않음</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="567"/>
        <source>&apos;OpenWinClass=program.exe,#&apos; is not supported, use &apos;NoRenameWinClass=program.exe,*&apos; instead</source>
        <translation>&apos;OpenWinClass=program.exe,#&apos;는 지원되지 않으므로 대신 &apos;NoRenameWinClass=program.exe,*&apos;를 사용하십시오</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="611"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="605"/>
        <location filename="Windows/OptionsGeneral.cpp" line="579"/>
        <location filename="Windows/OptionsGrouping.cpp" line="234"/>
        <location filename="Windows/OptionsGrouping.cpp" line="258"/>
        <location filename="Windows/OptionsNetwork.cpp" line="533"/>
        <source>Template values can not be edited.</source>
        <translation>템플릿 값은 편집할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAccess.cpp" line="679"/>
        <source>Template values can not be removed.</source>
        <translation>템플릿 값은 제거할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="41"/>
        <source>Enable the use of win32 hooks for selected processes. Note: You need to enable win32k syscall hook support globally first.</source>
        <translation>선택한 프로세스에 대해 win32 후크를 사용할 수 있습니다. 참고: 먼저 win32k syscall 훅 지원을 전체적으로 활성화해야 합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="42"/>
        <source>Enable crash dump creation in the sandbox folder</source>
        <translation>샌드박스 폴더에서 충돌 덤프 생성 실행</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="43"/>
        <source>Always use ElevateCreateProcess fix, as sometimes applied by the Program Compatibility Assistant.</source>
        <translation>프로그램 호환성 관리자에서 적용되는 ElevateCreateProcess 수정을 항상 사용하십시오.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="45"/>
        <source>Enable special inconsistent PreferExternalManifest behaviour, as needed for some Edge fixes</source>
        <oldsource>Enable special inconsistent PreferExternalManifest behavioure, as neede for some edge fixes</oldsource>
        <translation>일부 가장자리 수정에 필요한 경우 일관성이 없는 특수 PreferExternalManifest 동작 사용</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="46"/>
        <source>Set RpcMgmtSetComTimeout usage for specific processes</source>
        <translation>특정 프로세스에 대한 RpcMgmtSetComTimeout 사용량 설정</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="47"/>
        <source>Makes a write open call to a file that won&apos;t be copied fail instead of turning it read-only.</source>
        <translation>복사되지 않을 파일에 대한 쓰기 열기 호출이 읽기 전용으로 전환되지 않도록 합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="50"/>
        <source>Make specified processes think they have admin permissions.</source>
        <oldsource>Make specified processes think thay have admin permissions.</oldsource>
        <translation>지정된 프로세스가 관리자 권한을 가지고 있다고 생각하도록 합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="51"/>
        <source>Force specified processes to wait for a debugger to attach.</source>
        <translation>지정된 프로세스가 디버거가 연결될 때까지 대기하도록 합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="53"/>
        <source>Sandbox file system root</source>
        <translation>샌드박스 파일 시스템 루트</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="54"/>
        <source>Sandbox registry root</source>
        <translation>샌드박스 레지스트리 루트</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="55"/>
        <source>Sandbox ipc root</source>
        <translation>샌드박스 ipc 루트</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="575"/>
        <source>Add special option:</source>
        <translation>특수 옵션 추가:</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="726"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="730"/>
        <source>On Start</source>
        <translation>시작 시</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="727"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="735"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="739"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="743"/>
        <source>Run Command</source>
        <translation>명령 실행</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="731"/>
        <source>Start Service</source>
        <translation>서비스 시작</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="734"/>
        <source>On Init</source>
        <translation>초기화 시</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="738"/>
        <source>On File Recovery</source>
        <translation>파일 복구 시</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="742"/>
        <source>On Delete Content</source>
        <translation>콘텐츠 삭제 시</translation>
    </message>
    <message>
        <source>On Delete</source>
        <translation type="vanished">삭제 시</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="753"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="775"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="786"/>
        <location filename="Windows/OptionsAdvanced.cpp" line="797"/>
        <source>Please enter the command line to be executed</source>
        <translation>실행할 명령줄을 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="835"/>
        <source>Please enter a program file name</source>
        <translation>프로그램 파일 이름을 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="917"/>
        <source>Deny</source>
        <translation>거부</translation>
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
        <translation>처리</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="111"/>
        <location filename="Windows/OptionsForce.cpp" line="122"/>
        <source>Folder</source>
        <translation>폴더</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="222"/>
        <location filename="Windows/OptionsForce.cpp" line="232"/>
        <source>Select Executable File</source>
        <translation>실행 파일 선택</translation>
    </message>
    <message>
        <location filename="Windows/OptionsForce.cpp" line="222"/>
        <location filename="Windows/OptionsForce.cpp" line="232"/>
        <source>Executable Files (*.exe)</source>
        <translation>실행 파일 (*.exe)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="19"/>
        <source>This option requires a valid supporter certificate</source>
        <translation>이 옵션을 사용하려면 유효한 후원자 인증서가 필요합니다</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="22"/>
        <source>Supporter exclusive option</source>
        <translation>후원자 전용 옵션</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="47"/>
        <source>Don&apos;t alter the window title</source>
        <translation>창 제목 변경 안 함</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="48"/>
        <source>Display [#] indicator only</source>
        <translation>[#] 표시기만 표시</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="49"/>
        <source>Display box name in title</source>
        <translation>제목에 박스 이름 표시</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="51"/>
        <source>Border disabled</source>
        <translation>테두리 사용 안 함</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="52"/>
        <source>Show only when title is in focus</source>
        <translation>제목에 초점이 맞춰진 경우에만 표시</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="53"/>
        <source>Always show</source>
        <translation>항상 표시</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="56"/>
        <source>Hardened Sandbox with Data Protection</source>
        <translation>데이터 보호 기능을 갖춘 강화 샌드박스</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="57"/>
        <source>Security Hardened Sandbox</source>
        <translation>보안 강화 샌드박스</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="58"/>
        <source>Sandbox with Data Protection</source>
        <translation>데이터 보호 기능을 갖춘 샌드박스</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="59"/>
        <source>Standard Isolation Sandbox (Default)</source>
        <translation>표준 분리 샌드박스 (기본값)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="61"/>
        <source>Application Compartment with Data Protection</source>
        <translation>데이터 보호 기능이 있는 응용 프로그램 구획</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="62"/>
        <source>Application Compartment (NO Isolation)</source>
        <translation>응용 프로그램 칸 (격리 없음)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="109"/>
        <source>Custom icon</source>
        <translation>사용자 지정 아이콘</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="161"/>
        <source>Version 1</source>
        <translation>버전 1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="162"/>
        <source>Version 2</source>
        <translation>버전 2</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="198"/>
        <source>Browse for Program</source>
        <translation>프로그램 찾아보기</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="258"/>
        <source>Open Box Options</source>
        <translation>박스 열기 옵션</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="259"/>
        <source>Browse Content</source>
        <translation>내용 찾아보기</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="260"/>
        <source>Start File Recovery</source>
        <translation>파일 복구 시작</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="261"/>
        <source>Show Run Dialog</source>
        <translation>실행 대화 상자 표시</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="294"/>
        <source>Indeterminate</source>
        <translation>불확실한</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="484"/>
        <location filename="Windows/OptionsGeneral.cpp" line="584"/>
        <source>Always copy</source>
        <translation>항상 복사</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="485"/>
        <location filename="Windows/OptionsGeneral.cpp" line="585"/>
        <source>Don&apos;t copy</source>
        <translation>복사 안 함</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="486"/>
        <location filename="Windows/OptionsGeneral.cpp" line="586"/>
        <source>Copy empty</source>
        <translation>빈 복사</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="712"/>
        <source>kilobytes (%1)</source>
        <translation>킬로바이트 (%1)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="818"/>
        <source>Select color</source>
        <translation>색상 선택</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="893"/>
        <source>Select Program</source>
        <translation>프로그램 선택</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="764"/>
        <source>Please enter a service identifier</source>
        <translation>서비스 식별자를 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="893"/>
        <source>Executables (*.exe *.cmd)</source>
        <translation>실행 파일 (*.exe *.cmd)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="897"/>
        <location filename="Windows/OptionsGeneral.cpp" line="913"/>
        <source>Please enter a menu title</source>
        <translation>메뉴 제목을 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGeneral.cpp" line="909"/>
        <source>Please enter a command</source>
        <translation>명령을 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGrouping.cpp" line="141"/>
        <source>Please enter a name for the new group</source>
        <translation>새 그룹의 이름을 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/OptionsGrouping.cpp" line="229"/>
        <source>Please select group first.</source>
        <translation>먼저 그룹을 선택하세요.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="37"/>
        <location filename="Windows/OptionsNetwork.cpp" line="588"/>
        <source>Any</source>
        <translation>모두</translation>
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
        <translation>액세스 허용</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="53"/>
        <source>Block using Windows Filtering Platform</source>
        <translation>Windows 필터링 플랫폼을 사용하여 차단</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="54"/>
        <source>Block by denying access to Network devices</source>
        <translation>네트워크 장치에 대한 액세스를 거부하여 차단</translation>
    </message>
    <message>
        <location filename="Windows/OptionsAdvanced.cpp" line="917"/>
        <location filename="Windows/OptionsNetwork.cpp" line="171"/>
        <location filename="Windows/OptionsNetwork.cpp" line="574"/>
        <source>Allow</source>
        <translation>허용</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="172"/>
        <source>Block (WFP)</source>
        <translation>차단 (WFP)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="173"/>
        <source>Block (NDev)</source>
        <translation>차단 (NDev)</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="278"/>
        <source>A non empty program name is required.</source>
        <translation>비어 있지 않은 프로그램 이름이 필요합니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsNetwork.cpp" line="575"/>
        <source>Block</source>
        <translation>차단</translation>
    </message>
    <message>
        <source>Exclusion</source>
        <translation type="vanished">제외</translation>
    </message>
    <message>
        <location filename="Windows/OptionsRecovery.cpp" line="150"/>
        <source>Please enter a file extension to be excluded</source>
        <translation>제외할 파일 확장자를 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="56"/>
        <source>All Categories</source>
        <translation>모든 범주</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="95"/>
        <source>Custom Templates</source>
        <translation>사용자 지정 템플릿</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="96"/>
        <source>Email Reader</source>
        <translation>이메일 리더</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="97"/>
        <source>PDF/Print</source>
        <translation>PDF/인쇄</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="98"/>
        <source>Security/Privacy</source>
        <translation>보안/개인 정보</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="99"/>
        <source>Desktop Utilities</source>
        <translation>데스크탑 유틸리티</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="100"/>
        <source>Download Managers</source>
        <translation>다운로드 관리자</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="101"/>
        <source>Miscellaneous</source>
        <translation>기타</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="102"/>
        <source>Web Browser</source>
        <translation>웹 브라우저</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="103"/>
        <source>Media Player</source>
        <translation>미디어 플레이어</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="104"/>
        <source>Torrent Client</source>
        <translation>Torrent 클라이언트</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="151"/>
        <source>This template is enabled globally. To configure it, use the global options.</source>
        <translation>이 템플릿은 전체적으로 사용 가능합니다. 구성하려면 전역 옵션을 사용하십시오.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="187"/>
        <source>Please enter the template identifier</source>
        <translation>템플릿 식별자를 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="195"/>
        <source>Error: %1</source>
        <translation>오류: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="222"/>
        <source>Do you really want to delete the selected local template(s)?</source>
        <translation>선택한 로컬 템플릿을 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsTemplates.cpp" line="228"/>
        <source>Only local templates can be removed!</source>
        <translation>로컬 템플릿만 제거할 수 있습니다!</translation>
    </message>
    <message>
        <source>Do you really want to delete the selected local template?</source>
        <translation type="vanished">선택한 로컬 템플릿을 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="181"/>
        <source>Sandboxie Plus - &apos;%1&apos; Options</source>
        <translation>Sandboxie Plus - &apos;%1&apos; 옵션</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="287"/>
        <source>File Options</source>
        <translation>파일 옵션</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="319"/>
        <source>Grouping</source>
        <translation>그룹</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="525"/>
        <source>Add %1 Template</source>
        <translation>%1 템플릿 추가</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="625"/>
        <source>Search for options</source>
        <translation>옵션 검색</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="755"/>
        <source>Box: %1</source>
        <translation>박스: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="756"/>
        <source>Template: %1</source>
        <translation>템플릿: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="757"/>
        <source>Global: %1</source>
        <translation>전역: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="758"/>
        <source>Default: %1</source>
        <translation>기본값: %1</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="935"/>
        <source>This sandbox has been deleted hence configuration can not be saved.</source>
        <translation>이 샌드박스가 삭제되어 구성을 저장할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="982"/>
        <source>Some changes haven&apos;t been saved yet, do you really want to close this options window?</source>
        <translation>일부 변경 사항이 아직 저장되지 않았습니다. 이 옵션 창을 닫으시겠습니까?</translation>
    </message>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="1005"/>
        <source>Enter program:</source>
        <translation>프로그램 입력:</translation>
    </message>
    <message>
        <source>Lingerer</source>
        <translation type="vanished">나머지</translation>
    </message>
    <message>
        <source>Leader</source>
        <translation type="vanished">대표</translation>
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
        <translation>자세한 설명은 %1을 참조하십시오.</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="57"/>
        <source>Dismiss</source>
        <translation>해제</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="58"/>
        <source>Remove this message from the list</source>
        <translation>목록에서 이 메시지 제거</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="61"/>
        <source>Hide all such messages</source>
        <translation>해당 메시지 모두 숨기기</translation>
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
        <translation>해제</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="367"/>
        <source>Remove this progress indicator from the list</source>
        <translation>목록에서 이 진행률 표시기 제거</translation>
    </message>
</context>
<context>
    <name>CPopUpPrompt</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="108"/>
        <source>Remember for this process</source>
        <translation>이 과정을 기억</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="118"/>
        <source>Yes</source>
        <translation>예</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="123"/>
        <source>No</source>
        <translation>아니오</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="128"/>
        <source>Terminate</source>
        <translation>종료</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="149"/>
        <source>Yes and add to allowed programs</source>
        <translation>예, 허용된 프로그램에 추가</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="184"/>
        <source>Requesting process terminated</source>
        <translation>요청 프로세스가 종료되었습니다</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="189"/>
        <source>Request will time out in %1 sec</source>
        <translation>요청이 %1초 후에 시간 초과됩니다</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="191"/>
        <source>Request timed out</source>
        <translation>요청이 시간 초과되었습니다</translation>
    </message>
</context>
<context>
    <name>CPopUpRecovery</name>
    <message>
        <location filename="Windows/PopUpWindow.h" line="245"/>
        <source>Recover to:</source>
        <translation>복구 대상:</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="250"/>
        <source>Browse</source>
        <translation>찾아보기</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="251"/>
        <source>Clear folder list</source>
        <translation>폴더 목록 지우기</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="259"/>
        <source>Recover</source>
        <translation>복구</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="260"/>
        <source>Recover the file to original location</source>
        <translation>파일을 원래 위치로 복구</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="263"/>
        <source>Recover &amp;&amp; Explore</source>
        <translation>복구 및 탐색</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="264"/>
        <source>Recover &amp;&amp; Open/Run</source>
        <translation>복구 및 열기/실행</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="266"/>
        <source>Open file recovery for this box</source>
        <translation>이 박스에 대한 파일 복구 열기</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="274"/>
        <source>Dismiss</source>
        <translation>해제</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="275"/>
        <source>Don&apos;t recover this file right now</source>
        <translation>이 파일을 지금 복구하지 않음</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="278"/>
        <source>Dismiss all from this box</source>
        <translation>이 박스에서 모두 삭제</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="279"/>
        <source>Disable quick recovery until the box restarts</source>
        <translation>박스를 다시 시작할 때까지 빠른 복구 사용 안 함</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.h" line="303"/>
        <source>Select Directory</source>
        <translation>디렉터리 선택</translation>
    </message>
</context>
<context>
    <name>CPopUpWindow</name>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="25"/>
        <source>Sandboxie-Plus Notifications</source>
        <translation>Sandboxie-Plus 알림</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="171"/>
        <source>Do you want to allow the print spooler to write outside the sandbox for %1 (%2)?</source>
        <translation>%1 (%2) 동안 인쇄 스풀러가 샌드박스 외부에 쓰도록 허용하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="279"/>
        <source>Do you want to allow %4 (%5) to copy a %1 large file into sandbox: %2?
File name: %3</source>
        <translation>%4 (%5)가 %1 대용량 파일을 샌드박스: %2에 복사하도록 허용하시겠습니까?
파일 이름: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="285"/>
        <source>Do you want to allow %1 (%2) access to the internet?
Full path: %3</source>
        <translation>%1 (%2)의 인터넷 액세스를 허용하시겠습니까?
전체 경로: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="344"/>
        <source>%1 is eligible for quick recovery from %2.
The file was written by: %3</source>
        <translation>%1은 %2에서 빠르게 복구할 수 있습니다.
파일 작성자: %3</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="346"/>
        <source>an UNKNOWN process.</source>
        <translation>알 수 없는 프로세스.</translation>
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
        <translation>알 수 없음</translation>
    </message>
    <message>
        <location filename="Windows/PopUpWindow.cpp" line="437"/>
        <source>Migrating a large file %1 into the sandbox %2, %3 left.
Full path: %4</source>
        <translation>큰 파일 %1을 샌드박스 %2로 마이그레이션하는 중, %3 남았습니다.
전체 경로: %4</translation>
    </message>
</context>
<context>
    <name>CRecoveryLogWnd</name>
    <message>
        <location filename="SandManRecovery.cpp" line="306"/>
        <source>Sandboxie-Plus - Recovery Log</source>
        <translation>Sandboxie-Plus - 복구 로그</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="317"/>
        <source>Time|Box Name|File Path</source>
        <translation>시간|박스 이름|파일 경로</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="319"/>
        <source>Cleanup Recovery Log</source>
        <translation>복구 로그 정리</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="329"/>
        <source>The following files were recently recovered and moved out of a sandbox.</source>
        <oldsource>the following files were recently recovered and moved out of a sandbox.</oldsource>
        <translation>다음 파일은 최근에 복구되어 샌드박스 밖으로 이동되었습니다.</translation>
    </message>
</context>
<context>
    <name>CRecoveryWindow</name>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="46"/>
        <source>%1 - File Recovery</source>
        <translation>%1 - 파일 복구</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="70"/>
        <source>File Name</source>
        <translation>파일 이름</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="71"/>
        <source>File Size</source>
        <translation>파일 크기</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="72"/>
        <source>Full Path</source>
        <translation>전체 경로</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="106"/>
        <source>Remember target selection</source>
        <translation>대상 선택 기억</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="112"/>
        <source>Delete everything, including all snapshots</source>
        <translation>모든 스냅샷을 포함한 모든 항목 삭제</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="138"/>
        <source>Original location</source>
        <translation>원래 위치</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="139"/>
        <source>Browse for location</source>
        <translation>위치 찾아보기</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="140"/>
        <source>Clear folder list</source>
        <translation>폴더 목록 지우기</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="180"/>
        <location filename="Windows/RecoveryWindow.cpp" line="201"/>
        <location filename="Windows/RecoveryWindow.cpp" line="552"/>
        <source>Select Directory</source>
        <translation>디렉터리 선택</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="248"/>
        <source>Do you really want to delete %1 selected files?</source>
        <translation>%1개의 선택한 파일을 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="276"/>
        <source>Close until all programs stop in this box</source>
        <translation>이 박스에서 모든 프로그램을 중지할 때까지 닫기</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="277"/>
        <source>Close and Disable Immediate Recovery for this box</source>
        <translation>이 박스를 닫고 즉시 복구 사용 안 함</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="297"/>
        <source>There are %1 new files available to recover.</source>
        <translation>복구할 수 있는 새 파일 %1개가 있습니다.</translation>
    </message>
    <message>
        <location filename="Windows/RecoveryWindow.cpp" line="588"/>
        <source>There are %1 files and %2 folders in the sandbox, occupying %3 of disk space.</source>
        <translation>샌드박스에는 %3의 디스크 공간을 차지하는 %1개의 파일과 %2개의 폴더가 있습니다.</translation>
    </message>
</context>
<context>
    <name>CSandBox</name>
    <message>
        <location filename="SandMan.cpp" line="3614"/>
        <source>Waiting for folder: %1</source>
        <translation>폴더 대기 중: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3615"/>
        <source>Deleting folder: %1</source>
        <translation>폴더 삭제 중: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3616"/>
        <source>Merging folders: %1 &amp;gt;&amp;gt; %2</source>
        <translation>폴더 병합: %1 &amp;gt;&amp;gt; %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3617"/>
        <source>Finishing Snapshot Merge...</source>
        <translation>스냅샷 병합을 완료하는 중...</translation>
    </message>
</context>
<context>
    <name>CSandBoxPlus</name>
    <message>
        <location filename="SbiePlusAPI.cpp" line="652"/>
        <source>Disabled</source>
        <translation>사용 안 함</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="663"/>
        <source>OPEN Root Access</source>
        <translation>루트 액세스 열기</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="665"/>
        <source>Application Compartment</source>
        <translation>응용 프로그램 구획</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="667"/>
        <source>NOT SECURE</source>
        <translation>안전하지 않음</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="669"/>
        <source>Reduced Isolation</source>
        <translation>격리 감소</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="671"/>
        <source>Enhanced Isolation</source>
        <translation>격리 강화</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="674"/>
        <source>Privacy Enhanced</source>
        <translation>개인 정보 강화</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="677"/>
        <source>API Log</source>
        <translation>API 로그</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="680"/>
        <source>No INet (with Exceptions)</source>
        <translation>INET 없음 (예외 포함)</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="682"/>
        <source>No INet</source>
        <translation>INet 없음</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="685"/>
        <source>Net Share</source>
        <translation>Net 공유</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="687"/>
        <source>No Admin</source>
        <translation>관리자 없음</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="690"/>
        <source>Auto Delete</source>
        <translation>자동 삭제</translation>
    </message>
    <message>
        <location filename="SbiePlusAPI.cpp" line="693"/>
        <source>Normal</source>
        <translation>일반</translation>
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
        <translation>열 초기화</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3504"/>
        <source>Copy Cell</source>
        <translation>셀 복사</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3505"/>
        <source>Copy Row</source>
        <translation>행 복사</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3506"/>
        <source>Copy Panel</source>
        <translation>패널 복사</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1185"/>
        <source>Time|Message</source>
        <translation>시간|메시지</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1194"/>
        <source>Sbie Messages</source>
        <translation>Sbie 메시지</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1201"/>
        <source>Trace Log</source>
        <translation>추적 로그</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="45"/>
        <source>Show/Hide</source>
        <translation>표시/숨김</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="566"/>
        <location filename="SandMan.cpp" line="568"/>
        <location filename="SandMan.cpp" line="616"/>
        <location filename="SandMan.cpp" line="618"/>
        <source>Pause Forcing Programs</source>
        <translation>프로그램 강제 일시 중지</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="468"/>
        <location filename="SandMan.cpp" line="684"/>
        <source>&amp;Sandbox</source>
        <translation>샌드박스(&amp;S)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="469"/>
        <location filename="SandMan.cpp" line="942"/>
        <location filename="SandMan.cpp" line="943"/>
        <source>Create New Box</source>
        <translation>새 박스 만들기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="470"/>
        <source>Create Box Group</source>
        <translation>박스 그룹 만들기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="474"/>
        <location filename="SandMan.cpp" line="615"/>
        <source>Terminate All Processes</source>
        <translation>모든 프로세스 종료</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="572"/>
        <source>Disable File Recovery</source>
        <translation>파일 복구 사용 안 함</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="410"/>
        <source>&amp;Maintenance</source>
        <translation>유지 관리(&amp;M)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="411"/>
        <source>Connect</source>
        <translation>연결</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="412"/>
        <source>Disconnect</source>
        <translation>연결 끊기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="414"/>
        <source>Stop All</source>
        <translation>모두 중지</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="415"/>
        <source>&amp;Advanced</source>
        <translation>고급(&amp;A)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="416"/>
        <source>Install Driver</source>
        <translation>드라이버 설치</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="417"/>
        <source>Start Driver</source>
        <translation>드라이버 시작</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="418"/>
        <source>Stop Driver</source>
        <translation>드라이버 중지</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="419"/>
        <source>Uninstall Driver</source>
        <translation>드라이버 제거</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="421"/>
        <source>Install Service</source>
        <translation>서비스 설치</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="422"/>
        <source>Start Service</source>
        <translation>서비스 시작</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="423"/>
        <source>Stop Service</source>
        <translation>서비스 중지</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="424"/>
        <source>Uninstall Service</source>
        <translation>서비스 제거</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="426"/>
        <source>Setup Wizard</source>
        <translation>설정 마법사</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="429"/>
        <source>Uninstall All</source>
        <translation>모두 제거</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="500"/>
        <location filename="SandMan.cpp" line="654"/>
        <source>Exit</source>
        <translation>종료</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="503"/>
        <location filename="SandMan.cpp" line="656"/>
        <source>&amp;View</source>
        <translation>보기(&amp;V)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="435"/>
        <source>Simple View</source>
        <translation>단순 보기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="436"/>
        <source>Advanced View</source>
        <translation>고급 보기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="441"/>
        <source>Always on Top</source>
        <translation>항상 맨 위에</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="510"/>
        <source>Show Hidden Boxes</source>
        <translation>숨겨진 박스 표시</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="512"/>
        <source>Show All Sessions</source>
        <translation>모든 세션 표시</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="529"/>
        <source>Refresh View</source>
        <translation>보기 새로 고침</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="535"/>
        <source>Clean Up</source>
        <translation>정리</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="536"/>
        <source>Cleanup Processes</source>
        <translation>프로세스 정리</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="538"/>
        <source>Cleanup Message Log</source>
        <translation>메시지 로그 정리</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="539"/>
        <source>Cleanup Trace Log</source>
        <translation>추적 로그 정리</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="540"/>
        <source>Cleanup Recovery Log</source>
        <translation>복구 로그 정리</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="542"/>
        <source>Keep terminated</source>
        <translation>종료된 상태로 유지</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="562"/>
        <source>&amp;Options</source>
        <translation>옵션(&amp;O)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="563"/>
        <location filename="SandMan.cpp" line="705"/>
        <source>Global Settings</source>
        <translation>전역 설정</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="597"/>
        <location filename="SandMan.cpp" line="715"/>
        <source>Reset all hidden messages</source>
        <translation>숨겨진 모든 메시지 재설정</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="598"/>
        <location filename="SandMan.cpp" line="716"/>
        <source>Reset all GUI options</source>
        <translation>모든 GUI 옵션 재설정</translation>
    </message>
    <message>
        <source>Edit ini file</source>
        <translation type="vanished">ini 파일 편집</translation>
    </message>
    <message>
        <source>Reload ini file</source>
        <translation type="vanished">ini 파일 다시 불러오기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="555"/>
        <source>Trace Logging</source>
        <translation>추적 로깅</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="447"/>
        <source>&amp;Help</source>
        <translation>도움말(&amp;H)</translation>
    </message>
    <message>
        <source>Support Sandboxie-Plus with Donations</source>
        <oldsource>Support Sandboxie-Plus with a Donation</oldsource>
        <translation type="vanished">Sandboxie-Plus를 기부금으로 지원</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="456"/>
        <source>Visit Support Forum</source>
        <translation>지원 포럼 방문</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="455"/>
        <source>Online Documentation</source>
        <translation>온라인 문서</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="458"/>
        <source>Check for Updates</source>
        <translation>업데이트 확인</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="460"/>
        <source>About the Qt Framework</source>
        <translation>Qt Framework 정보</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="461"/>
        <location filename="SandMan.cpp" line="3588"/>
        <source>About Sandboxie-Plus</source>
        <translation>Sandboxie-Plus 정보</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="689"/>
        <source>Create New Sandbox</source>
        <translation>새 샌드박스 만들기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="690"/>
        <source>Create New Group</source>
        <translation>새 그룹 만들기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="792"/>
        <location filename="SandMan.cpp" line="930"/>
        <location filename="SandMan.cpp" line="931"/>
        <source>Cleanup</source>
        <translation>정리</translation>
    </message>
    <message>
        <source>&lt;a href=&quot;sbie://update/package&quot; style=&quot;color: red;&quot;&gt;There is a new build of Sandboxie-Plus available&lt;/a&gt;</source>
        <translation type="vanished">&lt;a href=&quot;sbie://update/package&quot; style=&quot;color: red;&quot;&gt;사용할 수 있는 새로운 Sandboxie-Plus 빌드가 있습니다&lt;/a&gt;</translation>
    </message>
    <message>
        <source>Click to install update</source>
        <translation type="vanished">업데이트를 설치하려면 클릭하십시오</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1031"/>
        <source>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Support Sandboxie-Plus on Patreon&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Patreon에서 Sandboxie-Plus 지원&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1032"/>
        <source>Click to open web browser</source>
        <translation>웹 브라우저를 열려면 클릭하십시오</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1211"/>
        <source>Time|Box Name|File Path</source>
        <translation>시간|박스 이름|파일 경로</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="559"/>
        <location filename="SandMan.cpp" line="667"/>
        <location filename="SandMan.cpp" line="1221"/>
        <source>Recovery Log</source>
        <translation>복구 로그</translation>
    </message>
    <message>
        <source>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus release ready&lt;/a&gt;</source>
        <translation type="vanished">&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;새 Sandboxie-Plus 릴리스가 준비되었습니다&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1015"/>
        <source>Click to run installer</source>
        <translation>설치 프로그램을 실행하려면 클릭</translation>
    </message>
    <message>
        <source>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update ready&lt;/a&gt;</source>
        <translation type="vanished">&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;새 Sandboxie-Plus 업데이트가 준비되었습니다&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1019"/>
        <source>Click to apply update</source>
        <translation>업데이트를 적용하려면 클릭</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1265"/>
        <source>Do you want to close Sandboxie Manager?</source>
        <translation>샌드박스 관리자를 닫으시겠습니까?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1282"/>
        <source>Sandboxie-Plus was running in portable mode, now it has to clean up the created services. This will prompt for administrative privileges.

Do you want to do the clean up?</source>
        <translation>Sandboxie-Plus는 휴대용 모드로 실행 중이었고 이제 생성된 서비스를 정리해야 합니다. 관리자 권한을 입력하라는 메시지가 표시됩니다.

정리하시겠습니까?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1283"/>
        <location filename="SandMan.cpp" line="1696"/>
        <location filename="SandMan.cpp" line="2027"/>
        <location filename="SandMan.cpp" line="2635"/>
        <location filename="SandMan.cpp" line="3063"/>
        <location filename="SandMan.cpp" line="3079"/>
        <source>Don&apos;t show this message again.</source>
        <translation>이 메시지를 다시 표시하지 않습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1452"/>
        <source>This box provides &lt;a href=&quot;sbie://docs/security-mode&quot;&gt;enhanced security isolation&lt;/a&gt;, it is suitable to test untrusted software.</source>
        <oldsource>This box provides enhanced security isolation, it is suitable to test untrusted software.</oldsource>
        <translation>이 박스는 향상된 보안 격리 기능을 제공하며 신뢰할 수 없는 소프트웨어를 테스트하는 데 적합합니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1456"/>
        <source>This box provides standard isolation, it is suitable to run your software to enhance security.</source>
        <translation>이 박스는 표준 격리를 제공하며, 보안을 강화하기 위해 소프트웨어를 실행하는 데 적합합니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1460"/>
        <source>This box does not enforce isolation, it is intended to be used as an &lt;a href=&quot;sbie://docs/compartment-mode&quot;&gt;application compartment&lt;/a&gt; for software virtualization only.</source>
        <oldsource>This box does not enforce isolation, it is intended to be used as an application compartment for software virtualization only.</oldsource>
        <translation>이 박스는 분리를 강제하지 않으며 소프트웨어 가상화 전용 &lt;a href=&quot;sbie://docs/compartment-mode&quot;&gt;응용프로그램 구획&lt;/a&gt;으로 사용됩니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1465"/>
        <source>&lt;br /&gt;&lt;br /&gt;This box &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;prevents access to all user data&lt;/a&gt; locations, except explicitly granted in the Resource Access options.</source>
        <oldsource>

This box &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;prevents access to all user data&lt;/a&gt; locations, except explicitly granted in the Resource Access options.</oldsource>
        <translation>&lt;br /&gt;&lt;br /&gt;이 박스는 리소스 액세스 옵션에 명시적으로 부여된 경우를 제외하고 &lt;a href=&quot;sbie://docs/privacy-mode&quot;&gt;모든 사용자 데이터 위치에 대한 액세스&lt;/a&gt;를 금지합니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1553"/>
        <source>Unknown operation &apos;%1&apos; requested via command line</source>
        <translation>명령줄에서 알 수 없는 작업 &apos;%1&apos;이(가) 요청되었습니다</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="105"/>
        <source>Dismiss Update Notification</source>
        <translation>업데이트 알림 해제</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="181"/>
        <source> - Driver/Service NOT Running!</source>
        <translation> - 드라이버/서비스가 실행되고 있지 않습니다!</translation>
    </message>
    <message>
        <location filename="SandManTray.cpp" line="183"/>
        <source> - Deleting Sandbox Content</source>
        <translation> - 샌드박스 내용 삭제</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1763"/>
        <source>Executing OnBoxDelete: %1</source>
        <translation>OnBoxDelete 실행 중: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1784"/>
        <source>Auto Deleting %1 Content</source>
        <translation>%1 내용 자동 삭제</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1969"/>
        <source>Auto deleting content of %1</source>
        <translation>%1의 내용을 자동 삭제하는 중</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>%1 Directory: %2</source>
        <translation>%1 디렉터리: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>Application</source>
        <translation>응용프로그램</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2003"/>
        <source>Installation</source>
        <translation>설치</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2005"/>
        <source>Current Config: %1</source>
        <translation>현재 구성: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2771"/>
        <location filename="SandMan.cpp" line="2773"/>
        <location filename="SandMan.cpp" line="3322"/>
        <source>Sandboxie-Plus - Error</source>
        <translation>Sandboxie-Plus - 오류</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2771"/>
        <source>Failed to stop all Sandboxie components</source>
        <translation>모든 Sandboxie 구성 요소를 중지하지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2773"/>
        <source>Failed to start required Sandboxie components</source>
        <translation>필수 Sandboxie 구성 요소를 시작하지 못했습니다</translation>
    </message>
    <message>
        <source>Do you want to check if there is a new version of Sandboxie-Plus?</source>
        <translation type="vanished">새 버전의 Sandboxie-Plus가 있는지 확인하시겠습니까?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="165"/>
        <source>WARNING: Sandboxie-Plus.ini in %1 cannot be written to, settings will not be saved.</source>
        <translation>경고: %1의 Sandboxie-Plus.ini에 쓸 수 없습니다. 설정이 저장되지 않습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="783"/>
        <source>New-Box Menu</source>
        <translation>새 박스 메뉴</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="800"/>
        <source>Edit-ini Menu</source>
        <translation>ini 메뉴 편집</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="844"/>
        <source>Toolbar Items</source>
        <translation>도구 모음 항목</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="846"/>
        <source>Reset Toolbar</source>
        <translation>도구 모음 재설정</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1014"/>
        <source>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus release %1 ready&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/installer&quot; style=&quot;color: red;&quot;&gt;새 Sandboxie-Plus 릴리스 %1이(가) 준비되었습니다&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1018"/>
        <source>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update %1 ready&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/apply&quot; style=&quot;color: red;&quot;&gt;새 Sandboxie-Plus 업데이트 %1이(가) 준비되었습니다&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1022"/>
        <source>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;There is a new Sandboxie-Plus update v%1 available&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;사용 가능한 새 Sandboxie-Plus 업데이트 v%1이 있습니다&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1694"/>
        <source>Some compatibility templates (%1) are missing, probably deleted, do you want to remove them from all boxes?</source>
        <translation>일부 호환성 템플릿(%1)이(가) 없거나 삭제되었을 수 있습니다. 모든 박스에서 제거하시겠습니까?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1711"/>
        <source>Cleaned up removed templates...</source>
        <translation>제거된 템플릿을 정리했습니다...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2024"/>
        <source>Sandboxie-Plus was started in portable mode, do you want to put the Sandbox folder into its parent directory?
Yes will choose: %1
No will choose: %2</source>
        <translation>Sandboxie-Plus가 휴대용 모드로 시작되었습니다. 샌드박스 폴더를 상위 디렉터리에 넣으시겠습니까?
예 선택: %1
아니요 선택: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2100"/>
        <source>Default sandbox not found; creating: %1</source>
        <translation>기본 샌드박스를 찾을 수 없습니다. 생성: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2140"/>
        <source>   -   NOT connected</source>
        <translation>   -   연결되지 않음</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2376"/>
        <source>PID %1: </source>
        <translation>PID %1: </translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2378"/>
        <source>%1 (%2): </source>
        <translation>%1 (%2): </translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2407"/>
        <source>The selected feature set is only available to project supporters. Processes started in a box with this feature set enabled without a supporter certificate will be terminated after 5 minutes.&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Become a project supporter&lt;/a&gt;, and receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;</source>
        <translation>선택한 기능 세트는 프로젝트 후원자만 사용할 수 있습니다. 후원자 인증서 없이 이 기능 세트가 활성화된 박스에서 시작된 프로세스는 5분 후에 종료됩니다.&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;프로젝트 후원자가 되어&lt;/a&gt;, &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;후원 인증서&lt;/a&gt;를 받습니다</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="202"/>
        <source>Recovering file %1 to %2</source>
        <translation>%1 파일을 %2로 복구하는 중</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="214"/>
        <source>The file %1 already exists, do you want to overwrite it?</source>
        <translation>%1 파일이 이미 있습니다. 덮어쓰시겠습니까?</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="179"/>
        <location filename="SandManRecovery.cpp" line="215"/>
        <source>Do this for all files!</source>
        <translation>모든 파일에 대해 이 작업을 수행하십시오!</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="99"/>
        <location filename="SandManRecovery.cpp" line="159"/>
        <source>Checking file %1</source>
        <translation>%1 파일을 확인하는 중</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="107"/>
        <source>The file %1 failed a security check!

%2</source>
        <translation>%1 파일이 보안 검사에 실패했습니다!

%2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="115"/>
        <source>All files passed the checks</source>
        <translation>모든 파일이 검사를 통과했습니다</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="178"/>
        <source>The file %1 failed a security check, do you want to recover it anyway?

%2</source>
        <oldsource>The file %1 failed a security check, do you want to recover it anyways?

%2</oldsource>
        <translation>%1 파일이 보안 검사에 실패했습니다. 그래도 복구하시겠습니까?

%2</translation>
    </message>
    <message>
        <location filename="SandManRecovery.cpp" line="243"/>
        <source>Failed to recover some files: 
</source>
        <translation>일부 파일을 복구하지 못했습니다: 
</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2528"/>
        <source>Only Administrators can change the config.</source>
        <translation>관리자만 구성을 변경할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2538"/>
        <source>Please enter the configuration password.</source>
        <translation>구성 암호를 입력하십시오.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2546"/>
        <source>Login Failed: %1</source>
        <translation>로그인 실패: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2575"/>
        <source>Do you want to terminate all processes in all sandboxes?</source>
        <translation>모든 sandboxes의 모든 프로세스를 종료하시겠습니까?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2576"/>
        <source>Terminate all without asking</source>
        <translation>묻지 않고 모두 종료</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2634"/>
        <source>Sandboxie-Plus was started in portable mode and it needs to create necessary services. This will prompt for administrative privileges.</source>
        <translation>Sandboxie-Plus는 휴대용 모드로 시작되었으며 필요한 서비스를 만들어야 합니다. 관리 권한을 묻는 메시지가 나타납니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2668"/>
        <source>CAUTION: Another agent (probably SbieCtrl.exe) is already managing this Sandboxie session, please close it first and reconnect to take over.</source>
        <translation>주의: 다른 에이전트 (아마도 SbieCtrl.exe)가 이미 이 Sandboxie 세션을 관리하고 있습니다. 먼저 이 세션을 닫은 후 다시 연결하여 작업을 수행하십시오.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2800"/>
        <source>Executing maintenance operation, please wait...</source>
        <translation>유지 보수 작업을 실행하는 중입니다. 잠시 기다려 주십시오...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2980"/>
        <source>Do you also want to reset hidden message boxes (yes), or only all log messages (no)?</source>
        <translation>숨겨진 메시지 박스 (예)를 재설정하시겠습니까, 아니면 모든 로그 메시지 (아니오)만 재설정하시겠습니까?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3077"/>
        <source>The changes will be applied automatically whenever the file gets saved.</source>
        <translation>파일이 저장될 때마다 변경 내용이 자동으로 적용됩니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3078"/>
        <source>The changes will be applied automatically as soon as the editor is closed.</source>
        <translation>편집기가 닫히는 즉시 변경 내용이 자동으로 적용됩니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3257"/>
        <source>Error Status: 0x%1 (%2)</source>
        <translation>오류 상태: 0x%1(%2)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3258"/>
        <source>Unknown</source>
        <translation>알 수 없음</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3277"/>
        <source>A sandbox must be emptied before it can be deleted.</source>
        <translation>샌드박스를 삭제하려면 먼저 비워야 합니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3281"/>
        <source>Failed to copy box data files</source>
        <translation>박스 데이터 파일을 복사하지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3286"/>
        <source>Failed to remove old box data files</source>
        <translation>이전 박스 데이터 파일을 제거하지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3300"/>
        <source>Unknown Error Status: 0x%1</source>
        <translation>알 수 없는 오류 상태: 0x%1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3507"/>
        <source>Case Sensitive</source>
        <translation>대소문자 구분</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3508"/>
        <source>RegExp</source>
        <translation>정규식</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3509"/>
        <source>Highlight</source>
        <translation>강조</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3510"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3511"/>
        <source>&amp;Find ...</source>
        <translation>찾기(&amp;F)...</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3512"/>
        <source>All columns</source>
        <translation>모든 열</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3561"/>
        <source>&lt;h3&gt;About Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;Version %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2023 by DavidXanatos&lt;/p&gt;</source>
        <oldsource>&lt;h3&gt;About Sandboxie-Plus&lt;/h3&gt;&lt;p&gt;Version %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2022 by DavidXanatos&lt;/p&gt;</oldsource>
        <translation>&lt;h3&gt;Sandboxie-Plus 정보 - 한국어: VenusGirl&lt;/h3&gt;&lt;p&gt;버전 %1&lt;/p&gt;&lt;p&gt;Copyright (c) 2020-2023 by DavidXanatos&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3569"/>
        <source>This copy of Sandboxie+ is certified for: %1</source>
        <translation>Sandboxie+의 이 사본은 다음에 대해 인증되었습니다: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3571"/>
        <source>Sandboxie+ is free for personal and non-commercial use.</source>
        <translation>Sandboxie+는 개인 및 비상업적인 사용에 대해 무료입니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3574"/>
        <source>Sandboxie-Plus is an open source continuation of Sandboxie.&lt;br /&gt;Visit &lt;a href=&quot;https://sandboxie-plus.com&quot;&gt;sandboxie-plus.com&lt;/a&gt; for more information.&lt;br /&gt;&lt;br /&gt;%3&lt;br /&gt;&lt;br /&gt;Driver version: %1&lt;br /&gt;Features: %2&lt;br /&gt;&lt;br /&gt;Icons from &lt;a href=&quot;https://icons8.com&quot;&gt;icons8.com&lt;/a&gt;</source>
        <translation>Sandboxie-Plus는 Sandboxie의 오픈 소스 연속입니다.&lt;br /&gt;더 많은 정보는 &lt;a href=&quot;https://sandboxie-plus.com&quot;&gt;sandboxie-plus.com&lt;/a&gt;를 방문하세요.&lt;br /&gt;&lt;br /&gt;%3&lt;br /&gt;&lt;br /&gt;드라이버 버전: %1&lt;br /&gt;기능: %2&lt;br /&gt;&lt;br /&gt;아이콘 제공은 &lt;a href=&quot;https://icons8.com&quot;&gt;icons8.com&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3260"/>
        <source>Administrator rights are required for this operation.</source>
        <translation>이 작업을 수행하려면 관리자 권한이 필요합니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="437"/>
        <source>Vintage View (like SbieCtrl)</source>
        <translation>빈티지 보기 (SbieCtrl 처럼)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="454"/>
        <source>Contribute to Sandboxie-Plus</source>
        <translation>Sandboxie-Plus에 기여</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="471"/>
        <source>Import Box</source>
        <translation>박스 가져오기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="473"/>
        <location filename="SandMan.cpp" line="614"/>
        <source>Run Sandboxed</source>
        <translation>샌드박스에서 실행</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="575"/>
        <source>Disable Message Popup</source>
        <translation>메시지 팝업 사용 안 함</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="476"/>
        <location filename="SandMan.cpp" line="628"/>
        <source>Is Window Sandboxed?</source>
        <oldsource>Is Window Sandboxed</oldsource>
        <translation>샌드박스 창이 있습니까?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="521"/>
        <source>Show File Panel</source>
        <translation>파일 패널 표시</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="585"/>
        <location filename="SandMan.cpp" line="723"/>
        <location filename="SandMan.cpp" line="958"/>
        <location filename="SandMan.cpp" line="959"/>
        <source>Edit Sandboxie.ini</source>
        <translation>Sandboxie.ini 편집</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="588"/>
        <source>Edit Templates.ini</source>
        <translation>Templates.ini 편집</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="590"/>
        <source>Edit Sandboxie-Plus.ini</source>
        <translation>Sandboxie-Plus.ini 편집</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="594"/>
        <location filename="SandMan.cpp" line="726"/>
        <source>Reload configuration</source>
        <translation>구성 다시 불러오기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="613"/>
        <source>&amp;File</source>
        <translation>파일(&amp;F)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="629"/>
        <source>Resource Access Monitor</source>
        <translation>리소스 액세스 모니터</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="662"/>
        <source>Programs</source>
        <translation>프로그램</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="663"/>
        <source>Files and Folders</source>
        <translation>파일 및 폴더</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="691"/>
        <source>Import Sandbox</source>
        <translation>박스 가져오기</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="693"/>
        <source>Set Container Folder</source>
        <translation>컨테이너 폴더 설정</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="696"/>
        <source>Set Layout and Groups</source>
        <translation>레이아웃 및 그룹 설정</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="698"/>
        <source>Reveal Hidden Boxes</source>
        <translation>숨겨진 박스 표시</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="704"/>
        <source>&amp;Configure</source>
        <translation>구성(&amp;C)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="708"/>
        <source>Program Alerts</source>
        <translation>프로그램 경고</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="710"/>
        <source>Windows Shell Integration</source>
        <translation>Windows 셸 통합</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="712"/>
        <source>Software Compatibility</source>
        <translation>소프트웨어 호환성</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="721"/>
        <source>Lock Configuration</source>
        <translation>구성 잠금</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="740"/>
        <source>Sandbox %1</source>
        <translation>샌드박스 %1</translation>
    </message>
    <message>
        <source>&lt;a href=&quot;sbie://update/package&quot; style=&quot;color: red;&quot;&gt;There is a new build of Sandboxie-Plus ready&lt;/a&gt;</source>
        <translation type="vanished">&lt;a href=&quot;sbie://update/package&quot; style=&quot;color: red;&quot;&gt;Sandboxie-Plus의 새로운 빌드가 준비되었습니다.&lt;/a&gt;</translation>
    </message>
    <message>
        <source>&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;There is a new build of Sandboxie-Plus available&lt;/a&gt;</source>
        <translation type="vanished">&lt;a href=&quot;sbie://update/check&quot; style=&quot;color: red;&quot;&gt;사용할 수 있는 새로운 Sandboxie-Plus 빌드가 있습니다&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1023"/>
        <source>Click to download update</source>
        <translation>업데이트를 다운로드하려면 클릭</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1639"/>
        <source>No Force Process</source>
        <translation>강제 프로세스 없음</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1856"/>
        <source>Removed Shortcut: %1</source>
        <translation>바로 가기 제거: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1935"/>
        <source>Updated Shortcut to: %1</source>
        <translation>업데이트된 바로 가기: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1937"/>
        <source>Added Shortcut to: %1</source>
        <translation>바로 가기 추가: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="1988"/>
        <source>Auto removing sandbox %1</source>
        <translation>샌드박스 %1 자동 제거 중</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2004"/>
        <source>Sandboxie-Plus Version: %1 (%2)</source>
        <translation>Sandboxie-Plus 버전: %1 (%2)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2006"/>
        <source>Data Directory: %1</source>
        <translation>데이터 디렉터리: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2069"/>
        <source> for Personal use</source>
        <translation> 개인 사용</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2071"/>
        <source>   -   for Non-Commercial use ONLY</source>
        <translation>   -   비상업적 용도로만 사용</translation>
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
        <translation>%2 박스에서 시작한 프로그램 %1은 프로젝트 후원자가 독점적으로 사용할 수 있는 기능을 사용하도록 구성되었기 때문에 5분 후에 종료됩니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2340"/>
        <source>The box %1 is configured to use features exclusively available to project supporters, these presets will be ignored.</source>
        <translation>%1 박스는 프로젝트 후원자가 독점적으로 사용할 수 있는 기능을 사용하도록 구성되었으며, 이러한 사전 설정은 무시됩니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2341"/>
        <source>&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;Become a project supporter&lt;/a&gt;, and receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;</source>
        <translation>&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;프로젝트 후원자가 되어&lt;/a&gt;, &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;후원자 인증서&lt;/a&gt;를 받습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2475"/>
        <source>The evaluation period has expired!!!</source>
        <oldsource>The evaluation periode has expired!!!</oldsource>
        <translation>평가 기간이 만료되었습니다!!!</translation>
    </message>
    <message>
        <source>Select file name</source>
        <translation type="vanished">파일 이름 선택</translation>
    </message>
    <message>
        <source>7-zip Archive (*.7z)</source>
        <translation type="vanished">7-zip 압축파일 (*.7z)</translation>
    </message>
    <message>
        <source>This name is already in use, please select an alternative box name</source>
        <oldsource>This Name is already in use, please select an alternative box name</oldsource>
        <translation type="vanished">이 이름은 이미 사용 중입니다. 다른 박스 이름을 선택하십시오</translation>
    </message>
    <message>
        <source>Importing: %1</source>
        <translation type="vanished">가져오기: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2595"/>
        <source>Please enter the duration, in seconds, for disabling Forced Programs rules.</source>
        <translation>강제 프로그램 규칙을 비활성화하는 기간을 초로 입력하십시오.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2610"/>
        <source>No Recovery</source>
        <translation>복구 안 함</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2616"/>
        <source>No Messages</source>
        <translation>메시지 없음</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2672"/>
        <source>&lt;b&gt;ERROR:&lt;/b&gt; The Sandboxie-Plus Manager (SandMan.exe) does not have a valid signature (SandMan.exe.sig). Please download a trusted release from the &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get&quot;&gt;official Download page&lt;/a&gt;.</source>
        <translation>&lt;b&gt;오류:&lt;/b&gt; Sandboxie-Plus Manager(SandMan.exe)에 유효한 서명(SandMan.exe.sig)이 없습니다. &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get&quot;&gt;공식 다운로드 페이지&lt;/a&gt;에서 신뢰할 수 있는 릴리스를 다운로드하십시오.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2775"/>
        <source>Maintenance operation failed (%1)</source>
        <translation>유지 관리 작업에 실패했습니다 (%1)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2780"/>
        <source>Maintenance operation completed</source>
        <translation>유지 보수 작업이 완료되었습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2911"/>
        <source>In the Plus UI, this functionality has been integrated into the main sandbox list view.</source>
        <translation>Plus UI에서 이 기능은 기본 샌드박스 목록 보기에 통합되었습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2912"/>
        <source>Using the box/group context menu, you can move boxes and groups to other groups. You can also use drag and drop to move the items around. Alternatively, you can also use the arrow keys while holding ALT down to move items up and down within their group.&lt;br /&gt;You can create new boxes and groups from the Sandbox menu.</source>
        <translation>박스/그룹의 상황에 맞는 메뉴를 사용하여 박스와 그룹을 다른 그룹으로 이동할 수 있습니다. 끌어서 놓기를 사용하여 항목을 이동할 수도 있습니다. 또는 ALT를 누른 상태에서 화살표 키를 사용하여 그룹 내에서 항목을 위아래로 이동할 수도 있습니다.&lt;br /&gt;.샌드박스 메뉴에서 새 박스 및 그룹을 생성할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3061"/>
        <source>You are about to edit the Templates.ini, this is generally not recommended.
This file is part of Sandboxie and all change done to it will be reverted next time Sandboxie is updated.</source>
        <oldsource>You are about to edit the Templates.ini, thsi is generally not recommeded.
This file is part of Sandboxie and all changed done to it will be reverted next time Sandboxie is updated.</oldsource>
        <translation>Templates.ini를 편집하려고 하는데 일반적으로 권장되지 않습니다.
이 파일은 Sandboxie의 일부이며 다음에 Sandboxie가 업데이트될 때 변경된 모든 내용이 되돌아갑니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3136"/>
        <source>Sandboxie config has been reloaded</source>
        <translation>Sandboxie 구성을 다시 불러왔습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3261"/>
        <source>Failed to execute: %1</source>
        <translation>실행하지 못했습니다: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3262"/>
        <source>Failed to connect to the driver</source>
        <translation>드라이버에 연결하지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3263"/>
        <source>Failed to communicate with Sandboxie Service: %1</source>
        <translation>Sandboxie Service와 통신하지 못했습니다: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3264"/>
        <source>An incompatible Sandboxie %1 was found. Compatible versions: %2</source>
        <translation>호환되지 않는 Sandboxie %1이(가) 발견되었습니다. 호환 버전: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3265"/>
        <source>Can&apos;t find Sandboxie installation path.</source>
        <translation>Sandboxie 설치 경로를 찾을 수 없습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3266"/>
        <source>Failed to copy configuration from sandbox %1: %2</source>
        <translation>%1에서 구성을 복사하지 못했습니다: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3267"/>
        <source>A sandbox of the name %1 already exists</source>
        <translation>%1 이름의 샌드박스가 이미 있습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3268"/>
        <source>Failed to delete sandbox %1: %2</source>
        <translation>샌드박스 %1을(를) 삭제하지 못했습니다: %2</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3269"/>
        <source>The sandbox name can not be longer than 32 characters.</source>
        <translation>샌드박스 이름은 32자를 초과할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3270"/>
        <source>The sandbox name can not be a device name.</source>
        <translation>샌드박스 이름은 장치 이름이 될 수 없습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3271"/>
        <source>The sandbox name can contain only letters, digits and underscores which are displayed as spaces.</source>
        <translation>샌드박스 이름에는 공백으로 표시되는 문자, 숫자 및 밑줄만 포함될 수 있습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3272"/>
        <source>Failed to terminate all processes</source>
        <translation>모든 프로세스를 종료하지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3273"/>
        <source>Delete protection is enabled for the sandbox</source>
        <translation>샌드박스에 대해 삭제 보호가 활성화되었습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3274"/>
        <source>All sandbox processes must be stopped before the box content can be deleted</source>
        <translation>박스 내용을 삭제하려면 먼저 모든 샌드박스 프로세스를 중지해야 합니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3275"/>
        <source>Error deleting sandbox folder: %1</source>
        <translation>샌드박스 폴더 삭제 중 오류 발생: %1</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3278"/>
        <source>Failed to move directory &apos;%1&apos; to &apos;%2&apos;</source>
        <translation>&apos;%1&apos; 디렉터리를 &apos;%2&apos;로 이동하지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3279"/>
        <source>This Snapshot operation can not be performed while processes are still running in the box.</source>
        <translation>프로세스가 박스에서 실행 중인 동안에는 이 스냅샷 작업을 수행할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3280"/>
        <source>Failed to create directory for new snapshot</source>
        <translation>새 스냅샷에 대한 디렉터리를 생성하지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3282"/>
        <source>Snapshot not found</source>
        <translation>스냅샷을 찾을 수 없음</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3283"/>
        <source>Error merging snapshot directories &apos;%1&apos; with &apos;%2&apos;, the snapshot has not been fully merged.</source>
        <translation>&apos;%1&apos; 스냅샷 디렉터리를 &apos;%2&apos;과(와) 병합하는 동안 오류가 발생했습니다. 스냅샷이 완전히 병합되지 않았습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3284"/>
        <source>Failed to remove old snapshot directory &apos;%1&apos;</source>
        <translation>이전 스냅샷 디렉터리 &apos;%1&apos;을(를) 제거하지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3285"/>
        <source>Can&apos;t remove a snapshot that is shared by multiple later snapshots</source>
        <translation>이후 여러 스냅샷이 공유하는 스냅샷을 제거할 수 없습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3287"/>
        <source>You are not authorized to update configuration in section &apos;%1&apos;</source>
        <translation>&apos;%1&apos; 섹션의 구성을 업데이트할 수 있는 권한이 없습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3288"/>
        <source>Failed to set configuration setting %1 in section %2: %3</source>
        <translation>%2 섹션에서 구성 설정 %1을 설정하지 못했습니다: %3</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3289"/>
        <source>Can not create snapshot of an empty sandbox</source>
        <translation>빈 샌드박스의 스냅샷을 생성할 수 없습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3290"/>
        <source>A sandbox with that name already exists</source>
        <translation>같은 이름의 샌드박스가 이미 있습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3291"/>
        <source>The config password must not be longer than 64 characters</source>
        <translation>구성 암호는 64자를 초과할 수 없습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3292"/>
        <source>The operation was canceled by the user</source>
        <translation>사용자가 작업을 취소했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3294"/>
        <source>Import/Export not available, 7z.dll could not be loaded</source>
        <translation>가져오기/내보내기 기능을 사용할 수 없습니다, 7z.dll을 불러올 수 없습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3295"/>
        <source>Failed to create the box archive</source>
        <translation>박스 압축파일을 만들지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3296"/>
        <source>Failed to open the 7z archive</source>
        <translation>7z 압축파일을 열지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3297"/>
        <source>Failed to unpack the box archive</source>
        <translation>박스 압축파일의 압축을 풀지 못했습니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3298"/>
        <source>The selected 7z file is NOT a box archive</source>
        <translation>선택한 7z 파일이 박스 압축파일이 아닙니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3324"/>
        <source>Operation failed for %1 item(s).</source>
        <translation>%1 항목에 대한 작업에 실패했습니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3355"/>
        <source>Do you want to open %1 in a sandboxed (yes) or unsandboxed (no) Web browser?</source>
        <translation>샌드박스에서 (예) 또는 샌드박스가 없는 (아니오) 웹 브라우저에서 %1을 여시겠습니까?</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3356"/>
        <source>Remember choice for later.</source>
        <translation>나중을 위해 선택을 기억합니다.</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2481"/>
        <source>The supporter certificate is not valid for this build, please get an updated certificate</source>
        <translation>후원자 인증서가 이 빌드에 유효하지 않습니다. 업데이트된 인증서를 받으십시오</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2484"/>
        <source>The supporter certificate has expired%1, please get an updated certificate</source>
        <oldsource>The supporter certificate is expired %1 days ago, please get an updated certificate</oldsource>
        <translation>후원자 인증서가 %1일 전에 만료되었습니다. 업데이트된 인증서를 받으십시오</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2485"/>
        <source>, but it remains valid for the current build</source>
        <translation>, 하지만 현재 빌드에 대해서는 유효합니다</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="2487"/>
        <source>The supporter certificate will expire in %1 days, please get an updated certificate</source>
        <translation>후원자 인증서가 %1일 후에 만료됩니다. 업데이트된 인증서를 받으십시오</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="88"/>
        <source>The selected window is running as part of program %1 in sandbox %2</source>
        <translation>선택한 창이 샌드박스 %2에서 %1 프로그램의 일부로 실행되고 있습니다</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="95"/>
        <source>The selected window is not running as part of any sandboxed program.</source>
        <translation>선택한 창이 샌드박스 프로그램의 일부로 실행되고 있지 않습니다.</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="134"/>
        <source>Drag the Finder Tool over a window to select it, then release the mouse to check if the window is sandboxed.</source>
        <translation>창 위로 검색기 도구를 끌어 선택한 다음 마우스를 놓아 창이 샌드박스 상태인지 확인합니다.</translation>
    </message>
    <message>
        <location filename="SbieFindWnd.cpp" line="204"/>
        <source>Sandboxie-Plus - Window Finder</source>
        <translation>Sandboxie-Plus - 창 검색기</translation>
    </message>
    <message>
        <location filename="main.cpp" line="123"/>
        <source>Sandboxie Manager can not be run sandboxed!</source>
        <translation>Sandboxie 관리자는 샌드박스에서 실행을 할 수 없습니다!</translation>
    </message>
</context>
<context>
    <name>CSbieModel</name>
    <message>
        <location filename="Models/SbieModel.cpp" line="159"/>
        <source>Box Group</source>
        <translation>박스 그룹</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="315"/>
        <source>Empty</source>
        <translation>비어 있음</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="559"/>
        <source>Name</source>
        <translation>이름</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="560"/>
        <source>Process ID</source>
        <translation>프로세스 ID</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="561"/>
        <source>Status</source>
        <translation>상태</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="562"/>
        <source>Title</source>
        <translation>제목</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="563"/>
        <source>Info</source>
        <translation>정보</translation>
    </message>
    <message>
        <location filename="Models/SbieModel.cpp" line="567"/>
        <source>Path / Command Line</source>
        <translation>명령 줄</translation>
    </message>
</context>
<context>
    <name>CSbieProcess</name>
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
        <translation>Sbie Crypto</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="62"/>
        <source>Sbie WuauServ</source>
        <translation>Sbie WuauServ</translation>
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
        <location filename="SbieProcess.cpp" line="65"/>
        <source>MSI Installer</source>
        <translation>MSI 설치 프로그램</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="66"/>
        <source>Trusted Installer</source>
        <translation>신뢰할 수 있는 설치 프로그램</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="67"/>
        <source>Windows Update</source>
        <translation>Windows 업데이트</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="68"/>
        <source>Windows Explorer</source>
        <translation>Windows 탐색기</translation>
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
        <translation>Windows Ink Services</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="80"/>
        <source>Chromium Based</source>
        <translation>Chromium 기반</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="81"/>
        <source>Google Updater</source>
        <translation>Google 업데이터</translation>
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
        <translation>Firefox 플러그인 컨테이너</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="87"/>
        <source>Generic Web Browser</source>
        <translation>일반 웹 브라우저</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="88"/>
        <source>Generic Mail Client</source>
        <translation>일반 메일 클라이언트</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="89"/>
        <source>Thunderbird</source>
        <translation>Thunderbird</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="99"/>
        <source>Terminated</source>
        <translation>종료</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="105"/>
        <source>Forced </source>
        <translation>강제 </translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="103"/>
        <source>Running</source>
        <translation>실행</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="109"/>
        <source> Elevated</source>
        <translation> 권한 상승</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="111"/>
        <source> as System</source>
        <translation> 시스템으로</translation>
    </message>
    <message>
        <location filename="SbieProcess.cpp" line="114"/>
        <source> in session %1</source>
        <translation> 세션 %1에서</translation>
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
        <translation>새 박스 만들기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="397"/>
        <source>Remove Group</source>
        <translation>그룹 제거</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="161"/>
        <location filename="Views/SbieView.cpp" line="291"/>
        <source>Run</source>
        <translation>실행</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="166"/>
        <source>Run Program</source>
        <translation>프로그램 실행</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="167"/>
        <source>Run from Start Menu</source>
        <translation>시작 메뉴에서 실행</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="178"/>
        <source>Default Web Browser</source>
        <translation>기본 웹 브라우저</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="179"/>
        <source>Default eMail Client</source>
        <translation>기본 이메일 클라이언트</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="181"/>
        <source>Windows Explorer</source>
        <translation>Windows 탐색기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="182"/>
        <source>Registry Editor</source>
        <translation>레지스트리 편집기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="183"/>
        <source>Programs and Features</source>
        <translation>프로그램 및 기능</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="194"/>
        <source>Terminate All Programs</source>
        <translation>모든 프로그램 종료</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="102"/>
        <location filename="Views/SbieView.cpp" line="200"/>
        <location filename="Views/SbieView.cpp" line="259"/>
        <location filename="Views/SbieView.cpp" line="339"/>
        <location filename="Views/SbieView.cpp" line="378"/>
        <source>Create Shortcut</source>
        <translation>바로 가기 만들기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="202"/>
        <location filename="Views/SbieView.cpp" line="324"/>
        <source>Explore Content</source>
        <translation>내용 탐색</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="204"/>
        <location filename="Views/SbieView.cpp" line="331"/>
        <source>Snapshots Manager</source>
        <translation>스냅샷 관리자</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="205"/>
        <source>Recover Files</source>
        <translation>파일 복구</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="206"/>
        <location filename="Views/SbieView.cpp" line="323"/>
        <source>Delete Content</source>
        <translation>내용 삭제</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="213"/>
        <source>Sandbox Presets</source>
        <translation>샌드박스 사전 설정</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="215"/>
        <source>Ask for UAC Elevation</source>
        <translation>UAC 상승 요청</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="216"/>
        <source>Drop Admin Rights</source>
        <translation>관리자 권한 삭제</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="217"/>
        <source>Emulate Admin Rights</source>
        <translation>관리자 권한 에뮬레이트</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="225"/>
        <source>Block Internet Access</source>
        <translation>인터넷 액세스 차단</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="227"/>
        <source>Allow Network Shares</source>
        <translation>네트워크 공유 허용</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="208"/>
        <source>Sandbox Options</source>
        <translation>샌드박스 옵션</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="177"/>
        <source>Standard Applications</source>
        <translation>표준 응용 프로그램</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="197"/>
        <source>Browse Files</source>
        <translation>파일 찾아보기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="236"/>
        <location filename="Views/SbieView.cpp" line="329"/>
        <source>Sandbox Tools</source>
        <translation>샌드박스 도구</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="237"/>
        <source>Duplicate Box Config</source>
        <translation>중복 박스 구성</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="240"/>
        <location filename="Views/SbieView.cpp" line="342"/>
        <source>Rename Sandbox</source>
        <translation>샌드박스 이름 바꾸기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="241"/>
        <location filename="Views/SbieView.cpp" line="343"/>
        <source>Move Sandbox</source>
        <translation>샌드박스 이동</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="252"/>
        <location filename="Views/SbieView.cpp" line="354"/>
        <source>Remove Sandbox</source>
        <translation>샌드박스 제거</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="257"/>
        <location filename="Views/SbieView.cpp" line="376"/>
        <source>Terminate</source>
        <translation>종료</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="260"/>
        <source>Preset</source>
        <translation>사전 설정</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="100"/>
        <location filename="Views/SbieView.cpp" line="261"/>
        <source>Pin to Run Menu</source>
        <translation>실행할 메뉴 고정</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="263"/>
        <source>Block and Terminate</source>
        <translation>차단 및 종료</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="267"/>
        <source>Allow internet access</source>
        <translation>인터넷 액세스 허용</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="269"/>
        <source>Force into this sandbox</source>
        <translation>이 샌드박스에 강제 적용</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="271"/>
        <source>Set Linger Process</source>
        <translation>링거 프로세스 설정</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="273"/>
        <source>Set Leader Process</source>
        <translation>지시자 프로세스 설정</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="536"/>
        <source>    File root: %1
</source>
        <translation>    파일 루트: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="537"/>
        <source>    Registry root: %1
</source>
        <translation>    레지스트리 루트: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="538"/>
        <source>    IPC root: %1
</source>
        <translation>    IPC 루트: %1
</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="540"/>
        <source>Options:
    </source>
        <translation>옵션:
    </translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="748"/>
        <source>[None]</source>
        <translation>[없음]</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1059"/>
        <source>Please enter a new group name</source>
        <translation>새 그룹 이름을 입력하십시오</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="857"/>
        <source>Do you really want to remove the selected group(s)?</source>
        <translation>선택한 그룹을 제거하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="152"/>
        <location filename="Views/SbieView.cpp" line="282"/>
        <source>Create Box Group</source>
        <translation>박스 그룹 만들기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="395"/>
        <source>Rename Group</source>
        <translation>그룹 이름 바꾸기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="157"/>
        <location filename="Views/SbieView.cpp" line="287"/>
        <source>Stop Operations</source>
        <translation>작업 중지</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="186"/>
        <source>Command Prompt</source>
        <translation>명령 프롬프트</translation>
    </message>
    <message>
        <source>Boxed Tools</source>
        <translation type="vanished">박스형 도구</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="187"/>
        <source>Command Prompt (as Admin)</source>
        <translation>명령 프롬프트 (관리자)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="191"/>
        <source>Command Prompt (32-bit)</source>
        <translation>명령 프롬프트 (32비트)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="176"/>
        <source>Execute Autorun Entries</source>
        <translation>자동 실행 항목 실행</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="330"/>
        <source>Browse Content</source>
        <translation>내용 찾아보기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="196"/>
        <source>Box Content</source>
        <translation>박스 내용</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="203"/>
        <source>Open Registry</source>
        <translation>레지스트리 열기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="199"/>
        <location filename="Views/SbieView.cpp" line="338"/>
        <source>Refresh Info</source>
        <translation>정보 새로 고침</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="170"/>
        <location filename="Views/SbieView.cpp" line="302"/>
        <source>(Host) Start Menu</source>
        <translation>(호스트) 시작 메뉴</translation>
    </message>
    <message>
        <source>More Tools</source>
        <translation type="vanished">추가 도구</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="231"/>
        <source>Immediate Recovery</source>
        <translation>즉시 복구</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="233"/>
        <source>Disable Force Rules</source>
        <translation>강제 규칙 사용 안 함</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="238"/>
        <source>Export Box</source>
        <translation>박스 내보내기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="242"/>
        <location filename="Views/SbieView.cpp" line="344"/>
        <source>Move Up</source>
        <translation>위로 이동</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="247"/>
        <location filename="Views/SbieView.cpp" line="349"/>
        <source>Move Down</source>
        <translation>아래로 이동</translation>
    </message>
    <message>
        <source>Run Sandboxed</source>
        <translation type="vanished">샌드박스에서 실행</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="296"/>
        <source>Run Web Browser</source>
        <translation>웹 브라우저 실행</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="297"/>
        <source>Run eMail Reader</source>
        <translation>이메일 리더 실행</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="298"/>
        <source>Run Any Program</source>
        <translation>모든 프로그램 실행</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="299"/>
        <source>Run From Start Menu</source>
        <translation>시작 메뉴에서 실행</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="307"/>
        <source>Run Windows Explorer</source>
        <translation>Windows 탐색기 실행</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="321"/>
        <source>Terminate Programs</source>
        <translation>프로그램 종료</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="322"/>
        <source>Quick Recover</source>
        <translation>빠른 복구</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="327"/>
        <source>Sandbox Settings</source>
        <translation>샌드박스 설정</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="334"/>
        <source>Duplicate Sandbox Config</source>
        <translation>샌드박스 구성 복제</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="335"/>
        <source>Export Sandbox</source>
        <translation>샌드박스 내보내기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="396"/>
        <source>Move Group</source>
        <translation>그룹 이동</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="847"/>
        <source>Please enter a new name for the Group.</source>
        <translation>그룹의 새 이름을 입력하십시오.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="894"/>
        <source>Move entries by (negative values move up, positive values move down):</source>
        <translation>항목 이동 기준 (음수 값은 위로 이동, 양수 값은 아래로 이동):</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="942"/>
        <source>A group can not be its own parent.</source>
        <translation>그룹은 자신의 상위 그룹이 될 수 없습니다.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1031"/>
        <source>This name is already in use, please select an alternative box name</source>
        <translation>이 이름은 이미 사용 중입니다. 다른 박스 이름을 선택하십시오</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1045"/>
        <source>Importing: %1</source>
        <translation>가져오기: %1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1085"/>
        <source>The Sandbox name and Box Group name cannot use the &apos;,()&apos; symbol.</source>
        <translation>샌드박스 이름 및 박스 그룹 이름에는 &apos;,(&apos;) 기호를 사용할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1090"/>
        <source>This name is already used for a Box Group.</source>
        <translation>이 이름은 이미 박스 그룹에 사용되고 있습니다.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1095"/>
        <source>This name is already used for a Sandbox.</source>
        <translation>이 이름은 샌드박스에 이미 사용되고 있습니다.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1145"/>
        <location filename="Views/SbieView.cpp" line="1215"/>
        <location filename="Views/SbieView.cpp" line="1462"/>
        <source>Don&apos;t show this message again.</source>
        <translation>이 메시지를 다시 표시하지 않습니다.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1206"/>
        <location filename="Views/SbieView.cpp" line="1226"/>
        <location filename="Views/SbieView.cpp" line="1630"/>
        <source>This Sandbox is empty.</source>
        <translation>샌드박스가 비어 있습니다.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1233"/>
        <source>WARNING: The opened registry editor is not sandboxed, please be careful and only do changes to the pre-selected sandbox locations.</source>
        <translation>경고: 열려 있는 레지스트리 편집기는 샌드박스가 아닙니다. 미리 선택한 샌드박스 위치만 변경하십시오.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1234"/>
        <source>Don&apos;t show this warning in future</source>
        <translation>앞으로 이 경고를 표시하지 않음</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1290"/>
        <source>Please enter a new name for the duplicated Sandbox.</source>
        <translation>복제된 샌드박스의 새 이름을 입력하십시오.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1290"/>
        <source>%1 Copy</source>
        <translation>%1 복사</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1018"/>
        <location filename="Views/SbieView.cpp" line="1329"/>
        <source>Select file name</source>
        <translation>파일 이름 선택</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="153"/>
        <location filename="Views/SbieView.cpp" line="283"/>
        <source>Import Box</source>
        <translation>박스 가져오기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1018"/>
        <location filename="Views/SbieView.cpp" line="1329"/>
        <source>7-zip Archive (*.7z)</source>
        <translation>7-zip 압축파일 (*.7z)</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1337"/>
        <source>Exporting: %1</source>
        <translation>내보내기: %1</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1344"/>
        <source>Please enter a new name for the Sandbox.</source>
        <translation>샌드박스의 새 이름을 입력하십시오.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1365"/>
        <source>Do you really want to remove the selected sandbox(es)?&lt;br /&gt;&lt;br /&gt;Warning: The box content will also be deleted!</source>
        <oldsource>Do you really want to remove the selected sandbox(es)?</oldsource>
        <translation>선택한 샌드박스를 제거하시겠습니까?&lt;br /&gt;&lt;br /&gt;경고: 박스 내용도 삭제됩니다!</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1403"/>
        <source>This Sandbox is already empty.</source>
        <translation>이 샌드박스는 이미 비어 있습니다.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1413"/>
        <source>Do you want to delete the content of the selected sandbox?</source>
        <translation>선택한 샌드박스의 내용을 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1414"/>
        <location filename="Views/SbieView.cpp" line="1418"/>
        <source>Also delete all Snapshots</source>
        <translation>또한 모든 스냅샷 삭제</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1417"/>
        <source>Do you really want to delete the content of all selected sandboxes?</source>
        <translation>선택한 모든 샌드박스의 내용을 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1444"/>
        <source>Do you want to terminate all processes in the selected sandbox(es)?</source>
        <translation>선택한 샌드박스의 모든 프로세스를 종료하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1445"/>
        <location filename="Views/SbieView.cpp" line="1539"/>
        <source>Terminate without asking</source>
        <translation>묻지 않고 종료</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1460"/>
        <source>The Sandboxie Start Menu will now be displayed. Select an application from the menu, and Sandboxie will create a new shortcut icon on your real desktop, which you can use to invoke the selected application under the supervision of Sandboxie.</source>
        <oldsource>The Sandboxie Start Menu will now be displayed. Select an application from the menu, and Sandboxie will create a newshortcut icon on your real desktop, which you can use to invoke the selected application under the supervision of Sandboxie.</oldsource>
        <translation>이제 샌드박스 시작 메뉴가 표시됩니다. 메뉴에서 응용프로그램을 선택하면 Sandboxie가 실제 바탕 화면에 새 바로 가기 아이콘을 만듭니다. 이 아이콘을 사용하면 Sandboxie의 감독 하에 선택한 응용프로그램을 호출할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1514"/>
        <location filename="Views/SbieView.cpp" line="1566"/>
        <source>Create Shortcut to sandbox %1</source>
        <translation>샌드박스 %1 바로 가기 만들기</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1538"/>
        <source>Do you want to terminate %1?</source>
        <oldsource>Do you want to %1 %2?</oldsource>
        <translation>%1을 종료하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1538"/>
        <source>the selected processes</source>
        <translation>선택된 과정</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1589"/>
        <source>This box does not have Internet restrictions in place, do you want to enable them?</source>
        <translation>이 박스에는 인터넷 제한이 없습니다. 인터넷 제한을 사용하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Views/SbieView.cpp" line="1673"/>
        <source>This sandbox is disabled, do you want to enable it?</source>
        <translation>이 샌드박스를 사용할 수 없습니다. 사용하시겠습니까?</translation>
    </message>
</context>
<context>
    <name>CSelectBoxWindow</name>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="89"/>
        <source>Sandboxie-Plus - Run Sandboxed</source>
        <translation>Sandboxie-Plus - 샌드박스에서 실행</translation>
    </message>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="206"/>
        <source>Are you sure you want to run the program outside the sandbox?</source>
        <translation>샌드박스 외부에서 프로그램을 실행하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Windows/SelectBoxWindow.cpp" line="219"/>
        <source>Please select a sandbox.</source>
        <translation>샌드박스를 선택하십시오.</translation>
    </message>
</context>
<context>
    <name>CSettingsWindow</name>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="130"/>
        <source>Sandboxie Plus - Global Settings</source>
        <oldsource>Sandboxie Plus - Settings</oldsource>
        <translation>Sandboxie Plus - 전역 설정</translation>
    </message>
    <message>
        <source>Advanced Config</source>
        <translation type="vanished">고급 구성</translation>
    </message>
    <message>
        <source>Sandbox Config</source>
        <translation type="vanished">샌드박스 구성</translation>
    </message>
    <message>
        <source>Config Protection</source>
        <translation type="vanished">구성 보호</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="216"/>
        <source>Auto Detection</source>
        <translation>자동 탐지</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="217"/>
        <source>No Translation</source>
        <translation>번역 안 함</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="237"/>
        <source>Don&apos;t integrate links</source>
        <translation>링크 통합 안 함</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="238"/>
        <source>As sub group</source>
        <translation>하위 그룹으로</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="239"/>
        <source>Fully integrate</source>
        <translation>완전 통합</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="241"/>
        <source>Don&apos;t show any icon</source>
        <oldsource>Don&apos;t integrate links</oldsource>
        <translation>아무 아이콘도 표시하지 않음</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="242"/>
        <source>Show Plus icon</source>
        <translation>플러스 아이콘 표시</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="243"/>
        <source>Show Classic icon</source>
        <translation>클래식 아이콘 표시</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="245"/>
        <source>All Boxes</source>
        <translation>모든 박스</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="246"/>
        <source>Active + Pinned</source>
        <translation>활성 + 고정</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="247"/>
        <source>Pinned Only</source>
        <translation>고정만</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="249"/>
        <source>None</source>
        <translation>없음</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="250"/>
        <source>Native</source>
        <translation>원본</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="251"/>
        <source>Qt</source>
        <translation>Qt</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="253"/>
        <source>Ignore</source>
        <translation>무시</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="265"/>
        <source>%1</source>
        <oldsource>%1 %</oldsource>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="412"/>
        <source>Add %1 Template</source>
        <translation>%1 템플릿 추가</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="496"/>
        <source>Search for settings</source>
        <translation>설정 검색</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="597"/>
        <source>Please enter message</source>
        <translation>메시지를 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="719"/>
        <location filename="Windows/SettingsWindow.cpp" line="720"/>
        <location filename="Windows/SettingsWindow.cpp" line="730"/>
        <source>Run &amp;Sandboxed</source>
        <translation>샌드박스에서 실행(&amp;S)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="751"/>
        <source>Sandboxed Web Browser</source>
        <translation>샌드박스 웹 브라우저</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="254"/>
        <location filename="Windows/SettingsWindow.cpp" line="259"/>
        <source>Notify</source>
        <translation>알림</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="255"/>
        <location filename="Windows/SettingsWindow.cpp" line="260"/>
        <source>Download &amp; Notify</source>
        <translation>다운로드하고 알림</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="256"/>
        <location filename="Windows/SettingsWindow.cpp" line="261"/>
        <source>Download &amp; Install</source>
        <translation>다운로드하고 설치</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="345"/>
        <source>Browse for Program</source>
        <translation>프로그램 찾아보기</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="629"/>
        <source>Select Program</source>
        <translation>프로그램 선택</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="629"/>
        <source>Executables (*.exe *.cmd)</source>
        <translation>실행 파일 (*.exe *.cmd)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="633"/>
        <location filename="Windows/SettingsWindow.cpp" line="646"/>
        <source>Please enter a menu title</source>
        <translation>메뉴 제목을 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="642"/>
        <source>Please enter a command</source>
        <translation>명령을 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="964"/>
        <source>This supporter certificate has expired, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>이 후원자 인증서가 만료되었습니다, &lt;a href=&quot;sbie://update/cert&quot;&gt;에서 업데이트된 인증서를 받으십시오&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="967"/>
        <source>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;Plus features will be disabled in %1 days.&lt;/font&gt;</source>
        <translation>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;%1일 후에 추가 기능이 비활성화됩니다.&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="969"/>
        <source>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;For this build Plus features remain enabled.&lt;/font&gt;</source>
        <translation>&lt;br /&gt;&lt;font color=&apos;red&apos;&gt;이 빌드 Plus 기능은 계속 사용 가능합니다.&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="971"/>
        <source>&lt;br /&gt;Plus features are no longer enabled.</source>
        <translation>&lt;br /&gt;Plus 기능이 더 이상 사용되지 않습니다.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="977"/>
        <source>This supporter certificate will &lt;font color=&apos;red&apos;&gt;expire in %1 days&lt;/font&gt;, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>이 후원자 인증서는 &lt;font color=&apos;red&apos;&gt; %1일 후 &lt;/font&gt;에 만료됩니다, &lt;a href=&quot;sbie://update/cert&quot;&gt;에서 업데이트된 인증서를 받으십시오&lt;/a&gt;.</translation>
    </message>
    <message>
        <source>Live channel is distributed as revisions only</source>
        <translation type="vanished">라이브 채널은 리비전으로만 배포됩니다</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1006"/>
        <source>Supporter certificate required</source>
        <oldsource>Supproter certificate required</oldsource>
        <translation>후원자 인증서 필요</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1112"/>
        <source>Run &amp;Un-Sandboxed</source>
        <translation>샌드박스 없이 실행(&amp;U)</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1371"/>
        <source>This does not look like a certificate. Please enter the entire certificate, not just a portion of it.</source>
        <translation>인증서로 보이지 않습니다. 인증서 일부가 아닌 전체 인증서를 입력하십시오.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1390"/>
        <source>This certificate is unfortunately expired.</source>
        <translation>이 인증서는 안타깝게도 만료되었습니다.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1392"/>
        <source>This certificate is unfortunately outdated.</source>
        <translation>안타깝게도 이 인증서는 오래되었습니다.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1395"/>
        <source>Thank you for supporting the development of Sandboxie-Plus.</source>
        <translation>Sandboxie-Plus 개발을 지원해 주셔서 감사합니다.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1402"/>
        <source>This support certificate is not valid.</source>
        <translation>이 후원 인증서는 유효하지 않습니다.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1440"/>
        <location filename="Windows/SettingsWindow.cpp" line="1576"/>
        <source>Select Directory</source>
        <translation>디렉터리 선택</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1469"/>
        <source>&lt;a href=&quot;check&quot;&gt;Check Now&lt;/a&gt;</source>
        <translation>&lt;a href=&quot;check&quot;&gt;지금 확인&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1536"/>
        <source>Please enter the new configuration password.</source>
        <translation>새 구성 암호를 입력하십시오.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1540"/>
        <source>Please re-enter the new configuration password.</source>
        <translation>새 구성 암호를 다시 입력하십시오.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1545"/>
        <source>Passwords did not match, please retry.</source>
        <translation>암호가 일치하지 않습니다. 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1557"/>
        <source>Process</source>
        <translation>프로세스</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1557"/>
        <source>Folder</source>
        <translation>폴더</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1567"/>
        <source>Please enter a program file name</source>
        <translation>프로그램 파일 이름을 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1635"/>
        <source>Please enter the template identifier</source>
        <translation>템플릿 식별자를 입력하십시오</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1643"/>
        <source>Error: %1</source>
        <translation>오류: %1</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1668"/>
        <source>Do you really want to delete the selected local template(s)?</source>
        <translation>선택한 로컬 템플릿을 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="Windows/SettingsWindow.cpp" line="1824"/>
        <source>%1 (Current)</source>
        <translation>%1 (현재)</translation>
    </message>
    <message>
        <source>&lt;a href=&quot;0&quot;&gt;%1&lt;/a&gt;</source>
        <translation type="vanished">&lt;a href=&quot;0&quot;&gt;%1&lt;/a&gt;</translation>
    </message>
    <message>
        <source>&lt;a href=&quot;1&quot;&gt;%1&lt;/a&gt;</source>
        <translation type="vanished">&lt;a href=&quot;1&quot;&gt;%1&lt;/a&gt;</translation>
    </message>
    <message>
        <source>Do you want to download the version %1?</source>
        <translation type="vanished">%1 버전을 다운로드하시겠습니까?</translation>
    </message>
</context>
<context>
    <name>CSetupWizard</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="29"/>
        <source>Setup Wizard</source>
        <translation>설치 마법사</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="40"/>
        <source>The decision you make here will affect which page you get to see next.</source>
        <translation>여기서 내리는 결정은 다음에 볼 수 있는 페이지에 영향을 미칩니다.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="43"/>
        <source>This help is likely not to be of any help.</source>
        <translation>이 도움말은 도움이 되지 않을 것 같습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="47"/>
        <source>Sorry, I already gave all the help I could.</source>
        <translation>죄송합니다. 이미 제가 할 수 있는 모든 도움을주었습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="49"/>
        <source>Setup Wizard Help</source>
        <translation>설치 마법사 도움말</translation>
    </message>
</context>
<context>
    <name>CShellPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="417"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; shell integration</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; 셸 통합 구성</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="418"/>
        <source>Configure how Sandboxie-Plus should integrate with your system.</source>
        <translation>Sandboxie-Plus를 시스템과 통합하는 방법을 구성합니다.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="422"/>
        <source>Start UI with Windows</source>
        <translation>Windows와 함께 UI 시작</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="427"/>
        <source>Add &apos;Run Sandboxed&apos; to the explorer context menu</source>
        <translation>탐색기의 상황에 맞는 메뉴에 &apos;샌드박스에서 실행&apos; 추가</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="432"/>
        <source>Add desktop shortcut for starting Web browser under Sandboxie</source>
        <translation>Sandboxie에서 웹 브라우저를 시작하기 위한 바탕 화면 바로 가기 추가</translation>
    </message>
</context>
<context>
    <name>CSnapshotsWindow</name>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="24"/>
        <source>%1 - Snapshots</source>
        <translation>%1 - 스냅샷</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="38"/>
        <source>Snapshot</source>
        <translation>스냅샷</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="56"/>
        <source>Revert to empty box</source>
        <translation>빈 박스로 돌아가기</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="103"/>
        <source> (default)</source>
        <translation> (기본값)</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="173"/>
        <source>Please enter a name for the new Snapshot.</source>
        <translation>새 스냅샷의 이름을 입력하십시오.</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="173"/>
        <source>New Snapshot</source>
        <translation>새 스냅샷</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="196"/>
        <source>Do you really want to switch the active snapshot? Doing so will delete the current state!</source>
        <translation>활성 스냅샷을 전환하시겠습니까? 이렇게 하면 현재 상태가 삭제됩니다!</translation>
    </message>
    <message>
        <location filename="Windows/SnapshotsWindow.cpp" line="230"/>
        <source>Do you really want to delete the selected snapshot?</source>
        <translation>선택한 스냅샷을 삭제하시겠습니까?</translation>
    </message>
</context>
<context>
    <name>CStackView</name>
    <message>
        <location filename="Views/StackView.cpp" line="17"/>
        <source>#|Symbol</source>
        <translation>#|기호</translation>
    </message>
</context>
<context>
    <name>CSummaryPage</name>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="527"/>
        <source>Create the new Sandbox</source>
        <translation>새 샌드박스 만들기</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="535"/>
        <source>Almost complete, click Finish to create a new sandbox and conclude the wizard.</source>
        <translation>거의 완료되었습니다. 마침을 클릭하여 새 샌드박스를 만들고 마법사를 종료합니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="544"/>
        <source>Save options as new defaults</source>
        <translation>옵션을 새 기본값으로 저장</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="555"/>
        <source>Don&apos;t show the summary page in future (unless advanced options were set)</source>
        <translation>나중에 요약 페이지 표시 안 함 (고급 옵션이 설정되지 않은 경우)</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="574"/>
        <source>
This Sandbox will be saved to: %1</source>
        <translation>
이 샌드박스는 다음 위치에 저장됩니다: %1</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="577"/>
        <source>
This box&apos;s content will be DISCARDED when it&apos;s closed, and the box will be removed.</source>
        <oldsource>
This box&apos;s content will be DISCARDED when its closed, and the box will be removed.</oldsource>
        <translation>
이 박스의 내용물은 닫히면 폐기되고 박스는 제거됩니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="579"/>
        <source>
This box will DISCARD its content when its closed, its suitable only for temporary data.</source>
        <translation>
이 박스는 닫히면 내용을 삭제하고 임시 데이터에만 적합합니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="581"/>
        <source>
Processes in this box will not be able to access the internet or the local network, this ensures all accessed data to stay confidential.</source>
        <translation>
이 박스의 프로세스는 인터넷 또는 로컬 네트워크에 액세스할 수 없으므로 액세스된 모든 데이터가 기밀로 유지됩니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="583"/>
        <source>
This box will run the MSIServer (*.msi installer service) with a system token, this improves the compatibility but reduces the security isolation.</source>
        <oldsource>
This box will run the MSIServer (*.msi installer service) with a system token, this improves the compatybility but reduces the security isolation.</oldsource>
        <translation>
이 박스는 시스템 토큰으로 MSI 서버 (*.msi 설치 관리자 서비스)를 실행합니다. 이렇게 하면 호환성은 향상되지만 보안 분리는 줄어듭니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="585"/>
        <source>
Processes in this box will think they are run with administrative privileges, without actually having them, hence installers can be used even in a security hardened box.</source>
        <translation>
이 박스의 프로세스는 관리자 권한 없이 실행되므로 보안 강화 박스에서도 설치 프로그램을 사용할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="587"/>
        <source>
Processes in this box will be running with a custom process token indicating the sandbox they belong to.</source>
        <oldsource>
Processes in this box will be running with a custom process token indicating the sandbox thay belong to.</oldsource>
        <translation>
이 박스의 프로세스는 자신이 속한 샌드박스를 나타내는 사용자 지정 프로세스 토큰으로 실행됩니다.</translation>
    </message>
    <message>
        <location filename="Wizards/NewBoxWizard.cpp" line="620"/>
        <source>Failed to create new box: %1</source>
        <translation>새 상자를 만들지 못했습니다: %1</translation>
    </message>
</context>
<context>
    <name>CSupportDialog</name>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="115"/>
        <source>The installed supporter certificate &lt;b&gt;has expired %1 days ago&lt;/b&gt; and &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;must be renewed&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;</source>
        <oldsource>The installed supporter certificate &lt;b&gt;has expired %1 days ago&lt;/b&gt; and &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;must be renewed&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;</oldsource>
        <translation>설치된 후원자 인증서가 &lt;b&gt;%1일 전에 만료&lt;/b&gt; 되었으므로 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;갱신해야 합니다&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="117"/>
        <source>&lt;b&gt;You have installed Sandboxie-Plus more than %1 days ago.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</source>
        <translation>&lt;b&gt;%1일 전에 Sandboxie-Plus를 설치했습니다.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="119"/>
        <source>&lt;u&gt;Commercial use of Sandboxie past the evaluation period&lt;/u&gt;, requires a valid &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;support certificate&lt;/a&gt;.</source>
        <oldsource>&lt;u&gt;Commercial use of Sandboxie past the evaluation period&lt;/u&gt;, requires a valid &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;support certificate&lt;/a&gt;.</oldsource>
        <translation>&lt;u&gt;평가 기간이 지난 Sandboxie를 상업적으로 사용하려면 &lt;/u&gt;, 유효한 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;후원 인증서&lt;/a&gt;가 필요합니다.</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="126"/>
        <source>The installed supporter certificate is &lt;b&gt;outdated&lt;/b&gt; and it is &lt;u&gt;not valid for&lt;b&gt; this version&lt;/b&gt;&lt;/u&gt; of Sandboxie-Plus.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>설치된 후원자 인증서가 &lt;b&gt;오래되어&lt;/b&gt; &lt;u&gt;&lt;b&gt;이 버전&lt;/b&gt;의  Sandboxie-Plus에는 &lt;b&gt;유효하지 않습니다&lt;/b&gt;&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="128"/>
        <source>The installed supporter certificate is &lt;b&gt;expired&lt;/b&gt; and &lt;u&gt;should be renewed&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</source>
        <oldsource>The installed supporter certificate is &lt;b&gt;expired&lt;/b&gt; and &lt;u&gt;should to be renewed&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</oldsource>
        <translation>설치된 후원자 인증서가 &lt;b&gt;만료&lt;/b&gt;되었으므로 &lt;u&gt;갱신해야 합니다&lt;/u&gt;.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="130"/>
        <source>&lt;b&gt;You have been using Sandboxie-Plus for more than %1 days now.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</source>
        <translation>&lt;b&gt;%1일 이상 Sandboxie-Plus를 사용하고 있습니다.&lt;/b&gt;&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="133"/>
        <source>Sandboxie on ARM64 requires a valid supporter certificate for continued use.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>계속 사용하려면 ARM64의 Sandboxie에 유효한 후원자 인증서가 필요합니다.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="135"/>
        <source>Personal use of Sandboxie is free of charge on x86/x64, although some functionality is only available to project supporters.&lt;br /&gt;&lt;br /&gt;</source>
        <translation>Sandboxie의 개인 사용은 x86/x64에서 무료이지만 일부 기능은 프로젝트 후원자에게만 제공됩니다.&lt;br /&gt;&lt;br /&gt;</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="138"/>
        <source>Please continue &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;supporting the project&lt;/a&gt; by renewing your &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; and continue using the &lt;b&gt;enhanced functionality&lt;/b&gt; in new builds.</source>
        <oldsource>Please continue &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt; by renewing your &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; and continue using the &lt;b&gt;enhanced functionality&lt;/b&gt; in new builds.</oldsource>
        <translation>&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;후원자 인증서&lt;/a&gt;를 갱신하여 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-renew-cert&quot;&gt;프로젝트 지원&lt;/a&gt;을 계속해서 새 빌드에서&lt;b&gt;향상된 기능&lt;/b&gt;을 계속 사용하십시오.</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="141"/>
        <source>Sandboxie &lt;u&gt;without&lt;/u&gt; a valid supporter certificate will sometimes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;pause for a few seconds&lt;/font&gt;&lt;/b&gt;. This pause allows you to consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;purchasing a supporter certificate&lt;/a&gt; or &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;earning one by contributing&lt;/a&gt; to the project. &lt;br /&gt;&lt;br /&gt;A &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; not just removes this reminder, but also enables &lt;b&gt;exclusive enhanced functionality&lt;/b&gt; providing better security and compatibility.</source>
        <oldsource>Sandboxie &lt;u&gt;without&lt;/u&gt; a valid supporter certificate will sometimes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;pause for a few seconds&lt;/font&gt;&lt;/b&gt;, to give you time to contemplate the option of &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt;.&lt;br /&gt;&lt;br /&gt;A &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt; not just removes this reminder, but also enables &lt;b&gt;exclusive enhanced functionality&lt;/b&gt; providing better security and compatibility.</oldsource>
        <translation>유효한 후원자 인증서가 &lt;u&gt;없는&lt;/u&gt; 샌드박스는 때때로 &lt;b&gt;&lt;font color=&apos;red&apos;&gt;몇 초 동안 일시 중지&lt;/font&gt;&lt;/b&gt;됩니다. 이 일시 중지를 통해 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;후원자 인증서를 구입&lt;/a&gt;하거나 프로젝트에 기여하여 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;인증서를 획득&lt;/a&gt;할 수 있습니다.&lt;br /&gt;&lt;br /&gt;&lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;후원자 인증서&lt;/a&gt;는 이 알림을 제거할 뿐만 아니라 더 나은 보안과 호환성을 제공하는 &lt;b&gt;독점적인 향상된 기능&lt;/b&gt;을 사용할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="172"/>
        <source>Sandboxie-Plus - Support Reminder</source>
        <translation>Sandboxie-Plus - 지원 알림</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="251"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="258"/>
        <source>Quit</source>
        <translation>종료</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="259"/>
        <source>Continue</source>
        <translation>계속</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="260"/>
        <source>Get Certificate</source>
        <translation>인증서 가져오기</translation>
    </message>
    <message>
        <location filename="Windows/SupportDialog.cpp" line="261"/>
        <source>Enter Certificate</source>
        <translation>인증서 입력</translation>
    </message>
</context>
<context>
    <name>CTemplateTypePage</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="258"/>
        <source>Create new Template</source>
        <translation>새 템플릿 만들기</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="267"/>
        <source>Select template type:</source>
        <translation>템플릿 유형 선택:</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="271"/>
        <source>%1 template</source>
        <translation>%1 템플릿</translation>
    </message>
</context>
<context>
    <name>CTemplateWizard</name>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="37"/>
        <source>Compatibility Template Wizard</source>
        <oldsource>Compatybility Template Wizard</oldsource>
        <translation>호환성 템플릿 마법사</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="48"/>
        <source>Custom</source>
        <translation>사용자 지정</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="49"/>
        <source>Web Browser</source>
        <translation>웹 브라우저</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="82"/>
        <source>Force %1 to run in this sandbox</source>
        <translation>%1을(를) 이 샌드박스에서 강제 실행</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="100"/>
        <source>Allow direct access to the entire %1 profile folder</source>
        <translation>전체 %1 프로필 폴더에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="113"/>
        <location filename="Wizards/TemplateWizard.cpp" line="168"/>
        <source>Allow direct access to %1 phishing database</source>
        <translation>%1 피싱 데이터베이스에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="127"/>
        <source>Allow direct access to %1 session management</source>
        <translation>%1 세션 관리에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="136"/>
        <location filename="Wizards/TemplateWizard.cpp" line="199"/>
        <source>Allow direct access to %1 passwords</source>
        <translation>%1 암호에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="146"/>
        <location filename="Wizards/TemplateWizard.cpp" line="208"/>
        <source>Allow direct access to %1 cookies</source>
        <translation>%1 쿠키에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="155"/>
        <location filename="Wizards/TemplateWizard.cpp" line="227"/>
        <source>Allow direct access to %1 bookmark and history database</source>
        <translation>%1 북마크 및 기록 데이터베이스에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="180"/>
        <source>Allow direct access to %1 sync data</source>
        <translation>%1 동기화 데이터에 대한 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="190"/>
        <source>Allow direct access to %1 preferences</source>
        <translation>%1 기본 설정에 직접 액세스 허용</translation>
    </message>
    <message>
        <location filename="Wizards/TemplateWizard.cpp" line="217"/>
        <source>Allow direct access to %1 bookmarks</source>
        <translation>%1 북마크에 대한 직접 액세스 허용</translation>
    </message>
</context>
<context>
    <name>CTraceModel</name>
    <message>
        <location filename="Models/TraceModel.cpp" line="196"/>
        <source>Unknown</source>
        <translation>알 수 없음</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="175"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="176"/>
        <source>Process %1</source>
        <translation>프로세스 %1</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="179"/>
        <source>Thread %1</source>
        <translation>스레드 %1</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="333"/>
        <source>Process</source>
        <translation>프로세스</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="335"/>
        <source>Type</source>
        <translation>유형</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="336"/>
        <source>Status</source>
        <translation>상태</translation>
    </message>
    <message>
        <location filename="Models/TraceModel.cpp" line="337"/>
        <source>Value</source>
        <translation>값</translation>
    </message>
</context>
<context>
    <name>CTraceView</name>
    <message>
        <location filename="Views/TraceView.cpp" line="255"/>
        <source>Show as task tree</source>
        <translation>작업 트리로 표시</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="259"/>
        <source>Show NT Object Tree</source>
        <translation>NT 개체 트리 표시</translation>
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
        <translation>[모두]</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="273"/>
        <source>TID:</source>
        <translation>TID:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="281"/>
        <source>Type:</source>
        <translation>유형:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="292"/>
        <source>Status:</source>
        <translation>상태:</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="295"/>
        <source>Open</source>
        <translation>열기</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="296"/>
        <source>Closed</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="297"/>
        <source>Trace</source>
        <translation>추적</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="298"/>
        <source>Other</source>
        <translation>기타</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="306"/>
        <source>Show All Boxes</source>
        <translation>모든 박스 표시</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="310"/>
        <source>Show Stack Trace</source>
        <translation>스택 추적 표시</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="315"/>
        <source>Save to file</source>
        <translation>파일에 저장</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="329"/>
        <source>Cleanup Trace Log</source>
        <translation>추적 로그 정리</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="694"/>
        <source>Save trace log to file</source>
        <translation>파일에 추적 로그 저장</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="700"/>
        <source>Failed to open log file for writing</source>
        <translation>쓰기 위해 로그 파일을 열지 못했습니다</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="720"/>
        <source>Unknown</source>
        <translation>알 수 없음</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="624"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="Views/TraceView.cpp" line="251"/>
        <source>Monitor mode</source>
        <translation>모니터 모드</translation>
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
        <translation>추적 모니터</translation>
    </message>
</context>
<context>
    <name>CUIPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="302"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; UI</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; UI 구성</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="303"/>
        <source>Select the user interface style you prefer.</source>
        <translation>원하는 사용자 인터페이스 스타일을 선택합니다.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="307"/>
        <source>&amp;Advanced UI for experts</source>
        <translation>전문가를 위한 고급 UI(&amp;A)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="312"/>
        <source>&amp;Simple UI for beginners</source>
        <translation>초보자를 위한 간단한 UI(&amp;S)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="317"/>
        <source>&amp;Vintage SbieCtrl.exe UI</source>
        <translation>빈티지 SbieCtrl.exe UI(&amp;V)</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="342"/>
        <source>Use Bright Mode</source>
        <translation>밝은 모드 사용</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="346"/>
        <source>Use Dark Mode</source>
        <translation>어두운 모드 사용</translation>
    </message>
</context>
<context>
    <name>CWFPPage</name>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="453"/>
        <source>Configure &lt;b&gt;Sandboxie-Plus&lt;/b&gt; network filtering</source>
        <translation>&lt;b&gt;Sandboxie-Plus&lt;/b&gt; 네트워크 필터링 구성</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="454"/>
        <source>Sandboxie can use the Windows Filtering Platform (WFP) to restrict network access.</source>
        <translation>Sandboxie는 Windows 필터링 플랫폼 (WFP)를 사용하여 네트워크 액세스를 제한할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="460"/>
        <source>Using WFP allows Sandboxie to reliably enforce IP/Port based rules for network access. Unlike system level application firewalls, Sandboxie can use different rules in each box for the same application. If you already have a good and reliable application firewall and do not need per box rules, you can leave this option unchecked. Without WFP enabled, Sandboxie will still be able to reliably and entirely block processes from accessing the network. However, this can cause the process to crash, as the driver blocks the required network device endpoints. Even with WFP disabled, Sandboxie offers to set IP/Port based rules, however these will be applied in user mode only and not be enforced by the driver. Hence, without WFP enabled, an intentionally malicious process could bypass those rules, but not the entire network block.</source>
        <translation>WFP를 사용하면 Sandboxie가 네트워크 액세스에 대해 IP/포트 기반 규칙을 안정적으로 적용할 수 있습니다. 시스템 수준 응용 프로그램 방화벽과 달리 샌드박스는 동일한 응용 프로그램에 대해 각 박스에서 서로 다른 규칙을 사용할 수 있습니다. 안정적이고 양호한 응용 프로그램 방화벽이 이미 있고 박스별 규칙이 필요하지 않은 경우 이 옵션을 선택 취소한 상태로 둘 수 있습니다. WFP가 활성화되지 않은 경우에도 Sandboxie는 여전히 신뢰할 수 있고 전적으로 프로세스가 네트워크에 액세스하는 것을 차단할 수 있습니다. 그러나 드라이버가 필요한 네트워크 장치 끝점을 차단하기 때문에 프로세스가 중단될 수 있습니다. WFP가 비활성화된 상태에서도 Sandboxie는 IP/포트 기반 규칙을 설정할 수 있지만 이는 사용자 모드에서만 적용되며 드라이버에 의해 강제되지는 않습니다. 따라서 WFP를 사용하지 않으면 의도적으로 악의적인 프로세스가 이러한 규칙을 무시할 수 있지만 전체 네트워크 블록은 무시할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="Wizards/SetupWizard.cpp" line="469"/>
        <source>Enable Windows Filtering Platform (WFP) support</source>
        <translation>Windows 필터링 플랫폼 (WFP) 지원 사용</translation>
    </message>
</context>
<context>
    <name>NewBoxWindow</name>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="32"/>
        <source>SandboxiePlus new box</source>
        <translation>SandboxiePlus 새 박스</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="60"/>
        <source>Box Type Preset:</source>
        <translation>박스 유형 사전 설정:</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="91"/>
        <source>A sandbox isolates your host system from processes running within the box, it prevents them from making permanent changes to other programs and data in your computer. The level of isolation impacts your security as well as the compatibility with applications, hence there will be a different level of isolation depending on the selected Box Type. Sandboxie can also protect your personal data from being accessed by processes running under its supervision.</source>
        <translation>샌드박스는 호스트 시스템을 포장에서 실행 중인 프로세스로부터 격리하여 컴퓨터의 다른 프로그램 및 데이터를 영구적으로 변경할 수 없도록 합니다. 격리 수준은 보안뿐만 아니라 응용프로그램과의 호환성에 영향을 미치므로 선택한 박스 유형에 따라 격리 수준이 달라집니다. Sandboxie는 또한 관리 하에 실행되는 프로세스에 의해 개인 데이터에 액세스되는 것을 방지할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="101"/>
        <source>Box info</source>
        <translation>박스 정보</translation>
    </message>
    <message>
        <location filename="Forms/NewBoxWindow.ui" line="67"/>
        <source>Sandbox Name:</source>
        <translation>샌드박스 이름:</translation>
    </message>
</context>
<context>
    <name>OptionsWindow</name>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="32"/>
        <source>SandboxiePlus Options</source>
        <translation>SandboxiePlus 옵션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="52"/>
        <source>General Options</source>
        <translation>일반 옵션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="62"/>
        <source>Box Options</source>
        <translation>박스 옵션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="96"/>
        <source>Sandbox Indicator in title:</source>
        <translation>제목의 샌드박스 표시기:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="106"/>
        <source>Sandboxed window border:</source>
        <translation>샌드박스 창 테두리:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="338"/>
        <source>Use volume serial numbers for drives, like: \drive\C~1234-ABCD</source>
        <translation>드라이브에 볼륨 일련 번호 사용, 예: \drive\C~1234-ABCD</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="382"/>
        <source>The box structure can only be changed when the sandbox is empty</source>
        <translation>상자 구조는 샌드박스가 비어 있는 경우에만 변경할 수 있습니다</translation>
    </message>
    <message>
        <source>Allow sandboxed processes to open files protected by EFS</source>
        <translation type="vanished">샌드박스 프로세스가 EFS에 의해 보호되는 파일을 열 수 있도록 허용</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="352"/>
        <source>Disk/File access</source>
        <translation>디스크/파일 액세스</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="331"/>
        <source>Virtualization scheme</source>
        <translation>가상화 체계</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="481"/>
        <source>2113: Content of migrated file was discarded
2114: File was not migrated, write access to file was denied
2115: File was not migrated, file will be opened read only</source>
        <translation>2113: 마이그레이션된 파일의 내용이 삭제되었습니다
2114: 파일이 마이그레이션되지 않았습니다. 파일에 대한 쓰기 액세스가 거부되었습니다
2115: 파일이 마이그레이션되지 않았습니다. 읽기 전용으로 파일이 열립니다</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="486"/>
        <source>Issue message 2113/2114/2115 when a file is not fully migrated</source>
        <translation>파일이 완전히 마이그레이션되지 않은 경우 메시지 2113/2114/2115 발생</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="510"/>
        <source>Add Pattern</source>
        <translation>패턴 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="517"/>
        <source>Remove Pattern</source>
        <translation>패턴 제거</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="571"/>
        <source>Pattern</source>
        <translation>패턴</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="579"/>
        <source>Sandboxie does not allow writing to host files, unless permitted by the user. When a sandboxed application attempts to modify a file, the entire file must be copied into the sandbox, for large files this can take a significate amount of time. Sandboxie offers options for handling these cases, which can be configured on this page.</source>
        <translation>Sandboxie는 사용자가 허용하지 않는 한 호스트 파일에 쓰는 것을 허용하지 않습니다. 샌드박스 응용프로그램이 파일을 수정하려고 할 때 전체 파일을 샌드박스에 복사해야 합니다. 큰 파일의 경우 상당한 시간이 걸릴 수 있습니다. Sandboxie는 이 페이지에서 구성할 수 있는 이러한 사례를 처리하는 옵션을 제공합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="589"/>
        <source>Using wildcard patterns file specific behavior can be configured in the list below:</source>
        <translation>와일드카드 패턴 사용 파일별 동작은 아래 목록에서 구성할 수 있습니다:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="603"/>
        <source>When a file cannot be migrated, open it in read-only mode instead</source>
        <translation>파일을 마이그레이션할 수 없는 경우 대신 읽기 전용 모드로 엽니다</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="613"/>
        <source>Restrictions</source>
        <translation>제한 사항</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="730"/>
        <location filename="Forms/OptionsWindow.ui" line="766"/>
        <location filename="Forms/OptionsWindow.ui" line="783"/>
        <location filename="Forms/OptionsWindow.ui" line="992"/>
        <location filename="Forms/OptionsWindow.ui" line="1037"/>
        <source>Protect the system from sandboxed processes</source>
        <translation>샌드박스 프로세스로부터 시스템 보호</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="874"/>
        <source>Icon</source>
        <translation>아이콘</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="899"/>
        <source>Move Up</source>
        <translation>위로 이동</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="918"/>
        <source>Move Down</source>
        <translation>아래로 이동</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="995"/>
        <source>Elevation restrictions</source>
        <translation>권한 제한</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1080"/>
        <source>Drop rights from Administrators and Power Users groups</source>
        <translation>관리자 및 Power Users 그룹에서 권한 삭제</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="221"/>
        <source>px Width</source>
        <translation>px 너비</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1002"/>
        <source>Make applications think they are running elevated (allows to run installers safely)</source>
        <translation>응용 프로그램이 높은 수준으로 실행되고 있다고 생각하도록 함 (설치 프로그램을 안전하게 실행할 수 있음)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1094"/>
        <source>CAUTION: When running under the built in administrator, processes can not drop administrative privileges.</source>
        <translation>주의: 기본 관리자에서 실행할 때 프로세스는 관리 권한을 삭제할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="252"/>
        <source>Appearance</source>
        <translation>모양</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1016"/>
        <source>(Recommended)</source>
        <translation>(추천)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="172"/>
        <source>Show this box in the &apos;run in box&apos; selection prompt</source>
        <translation>&apos;박스안에서 실행&apos; 선택 프롬프트에 이 박스 표시</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="279"/>
        <source>File Options</source>
        <translation>파일 옵션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="324"/>
        <source>Auto delete content when last sandboxed process terminates</source>
        <translation>마지막 샌드박스 프로세스가 종료될 때 내용 자동 삭제</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="453"/>
        <source>Copy file size limit:</source>
        <translation>복사 파일 크기 제한:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="410"/>
        <source>Box Delete options</source>
        <translation>박스 삭제 옵션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="294"/>
        <source>Protect this sandbox from deletion or emptying</source>
        <translation>이 샌드박스를 삭제 또는 비우지 않도록 보호</translation>
    </message>
    <message>
        <source>Raw Disk access</source>
        <translation type="vanished">Raw 디스크 액세스</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="433"/>
        <location filename="Forms/OptionsWindow.ui" line="474"/>
        <source>File Migration</source>
        <translation>파일 마이그레이션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="301"/>
        <source>Allow elevated sandboxed applications to read the harddrive</source>
        <translation>상승된 샌드박스 응용프로그램에서 하드 드라이브 읽기 허용</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="287"/>
        <source>Warn when an application opens a harddrive handle</source>
        <translation>응용 프로그램이 하드 드라이브 핸들을 열 때 경고</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="503"/>
        <source>kilobytes</source>
        <translation>킬로바이트</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="596"/>
        <source>Issue message 2102 when a file is too large</source>
        <translation>파일이 너무 큰 경우 메시지 2102 발행</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="460"/>
        <source>Prompt user for large file migration</source>
        <translation>사용자에게 대용량 파일 마이그레이션 확인</translation>
    </message>
    <message>
        <source>Access Restrictions</source>
        <translation type="vanished">액세스 제한</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="688"/>
        <source>Allow the print spooler to print to files outside the sandbox</source>
        <translation>샌드박스 외부의 파일로 인쇄 스풀러 인쇄 허용</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="702"/>
        <source>Remove spooler restriction, printers can be installed outside the sandbox</source>
        <translation>스풀러 제한 제거, 샌드박스 외부에 프린터를 설치할 수 있음</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="641"/>
        <source>Block read access to the clipboard</source>
        <translation>클립보드에 대한 읽기 액세스 차단</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="709"/>
        <source>Open System Protected Storage</source>
        <translation>시스템 보호 저장소 열기</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="675"/>
        <source>Block access to the printer spooler</source>
        <translation>프린터 스풀러에 대한 액세스 차단</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="733"/>
        <source>Other restrictions</source>
        <translation>기타 제한</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="769"/>
        <source>Printing restrictions</source>
        <translation>인쇄 제한</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="786"/>
        <source>Network restrictions</source>
        <translation>네트워크 제한</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="695"/>
        <source>Block network files and folders, unless specifically opened.</source>
        <translation>특별히 열지 않는 한 네트워크 파일 및 폴더를 차단합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="796"/>
        <source>Run Menu</source>
        <translation>실행 메뉴</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="925"/>
        <source>You can configure custom entries for the sandbox run menu.</source>
        <translation>샌드박스 실행 메뉴에 대한 사용자 정의 항목을 구성할 수 있습니다.</translation>
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
        <translation>이름</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="879"/>
        <source>Command Line</source>
        <translation>명령 줄</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="845"/>
        <source>Add program</source>
        <translation>프로그램 추가</translation>
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
        <translation>제거</translation>
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
        <translation>유형</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1583"/>
        <source>Program Groups</source>
        <translation>프로그램 그룹</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1591"/>
        <source>Add Group</source>
        <translation>그룹 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1623"/>
        <location filename="Forms/OptionsWindow.ui" line="1977"/>
        <location filename="Forms/OptionsWindow.ui" line="2054"/>
        <location filename="Forms/OptionsWindow.ui" line="2132"/>
        <location filename="Forms/OptionsWindow.ui" line="2918"/>
        <source>Add Program</source>
        <translation>프로그램 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1734"/>
        <source>Force Folder</source>
        <translation>강제 폴더</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2257"/>
        <location filename="Forms/OptionsWindow.ui" line="2357"/>
        <location filename="Forms/OptionsWindow.ui" line="2490"/>
        <source>Path</source>
        <translation>경로</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1777"/>
        <source>Force Program</source>
        <translation>강제 프로그램</translation>
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
        <translation>템플릿 표시</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1121"/>
        <source>Security note: Elevated applications running under the supervision of Sandboxie, with an admin or system token, have more opportunities to bypass isolation and modify the system outside the sandbox.</source>
        <translation>보안 참고 사항: 관리자 또는 시스템 토큰을 사용하여 샌드박스의 감독 하에 실행되는 고급 응용 프로그램은 분리를 우회하고 샌드박스 외부에서 시스템을 수정할 수 있는 기회가 더 많습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1047"/>
        <source>Allow MSIServer to run with a sandboxed system token and apply other exceptions if required</source>
        <translation>MSI 서버가 샌드박스 시스템 토큰으로 실행되도록 허용하고 필요한 경우 다른 예외를 적용합니다</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1104"/>
        <source>Note: Msi Installer Exemptions should not be required, but if you encounter issues installing a msi package which you trust, this option may help the installation complete successfully. You can also try disabling drop admin rights.</source>
        <translation>참고: Msi 설치 관리자 면제는 필요하지 않지만 신뢰할 수 있는 msi 패키지를 설치하는 데 문제가 발생할 경우 이 옵션을 사용하면 설치가 성공적으로 완료될 수 있습니다. 삭제 관리자 권한을 비활성화할 수도 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="238"/>
        <source>General Configuration</source>
        <translation>일반 구성</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="211"/>
        <source>Box Type Preset:</source>
        <translation>박스 유형 사전 설정:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="179"/>
        <source>Box info</source>
        <translation>박스 정보</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="142"/>
        <source>&lt;b&gt;More Box Types&lt;/b&gt; are exclusively available to &lt;u&gt;project supporters&lt;/u&gt;, the Privacy Enhanced boxes &lt;b&gt;&lt;font color=&apos;red&apos;&gt;protect user data from illicit access&lt;/font&gt;&lt;/b&gt; by the sandboxed programs.&lt;br /&gt;If you are not yet a supporter, then please consider &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporting the project&lt;/a&gt;, to receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;.&lt;br /&gt;You can test the other box types by creating new sandboxes of those types, however processes in these will be auto terminated after 5 minutes.</source>
        <translation>&lt;b&gt;더 많은 박스 유형&lt;/b&gt;은  &lt;u&gt;프로젝트 후원자&lt;/u&gt; 독점적으로 사용할 수 있으며, 개인 정보 보호 강화 박스는 샌드박스 프로그램에 의한 &lt;b&gt;&lt;font color=&apos;red&apos;&gt;불법 액세스으로부터 사용자 데이터를 보호&lt;/font&gt;&lt;/b&gt;합니다.&lt;br /&gt;아직 후원자가 아닌 경우 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;후원자 인증서&lt;/a&gt;를 받을 수 있도록 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;프로젝트 지원&lt;/a&gt;을 고려해 주시기 바랍니다.&lt;br /&gt;이러한 유형의 새 샌드박스를 만들어 다른 박스 유형을 테스트할 수 있지만, 이러한 유형의 프로세스는 5분 후에 자동으로 종료됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="89"/>
        <source>Always show this sandbox in the systray list (Pinned)</source>
        <translation>시스템 트레이 목록에 항상 이 샌드박스 표시 (고정)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="621"/>
        <source>Open Windows Credentials Store (user mode)</source>
        <translation>Windows 자격 증명 저장소 열기 (사용자 모드)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="661"/>
        <source>Prevent change to network and firewall parameters (user mode)</source>
        <translation>네트워크 및 방화벽 매개 변수 (사용자 모드) 변경 방지</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="716"/>
        <source>Issue message 2111 when a process access is denied</source>
        <translation>프로세스 액세스이 거부되면 메시지 2111 발행</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1630"/>
        <source>You can group programs together and give them a group name.  Program groups can be used with some of the settings instead of program names. Groups defined for the box overwrite groups defined in templates.</source>
        <translation>프로그램을 그룹화하고 그룹 이름을 지정할 수 있습니다. 프로그램 그룹은 프로그램 이름 대신 일부 설정과 함께 사용할 수 있습니다. 박스에 대해 정의된 그룹은 템플릿에 정의된 덮어쓰기 그룹입니다.</translation>
    </message>
    <message>
        <source>Forced Programs</source>
        <translation type="vanished">강제 프로그램</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1699"/>
        <source>Programs entered here, or programs started from entered locations, will be put in this sandbox automatically, unless they are explicitly started in another sandbox.</source>
        <translation>여기에 입력된 프로그램 또는 입력된 위치에서 시작된 프로그램은 다른 샌드박스에서 명시적으로 시작하지 않는 한 이 샌드박스에 자동으로 저장됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1920"/>
        <source>Stop Behaviour</source>
        <translation>동작 중지</translation>
    </message>
    <message>
        <source>Remove Program</source>
        <translation type="vanished">프로그램 제거</translation>
    </message>
    <message>
        <source>Add Leader Program</source>
        <translation type="vanished">대표 프로그램 추가</translation>
    </message>
    <message>
        <source>Add Lingering Program</source>
        <translation type="vanished">남은 프로그램 추가</translation>
    </message>
    <message>
        <source>Lingering programs will be automatically terminated if they are still running after all other processes have been terminated.

If leader processes are defined, all others are treated as lingering processes.</source>
        <translation type="vanished">남은 프로그램은 다른 모든 프로세스가 종료된 후에도 계속 실행 중인 경우 자동으로 종료됩니다.

대표 프로세스가 정의되면 다른 모든 프로세스는 남아있는 프로세스로 취급됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2092"/>
        <source>Start Restrictions</source>
        <translation>시작 제한</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2100"/>
        <source>Issue message 1308 when a program fails to start</source>
        <translation>프로그램 시작 실패 시 메시지 1308 발생</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2151"/>
        <source>Allow only selected programs to start in this sandbox. *</source>
        <translation>이 샌드박스에서 선택한 프로그램만 시작하도록 허용합니다. *</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2158"/>
        <source>Prevent selected programs from starting in this sandbox.</source>
        <translation>선택한 프로그램이 이 샌드박스에서 시작되지 않도록 합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2165"/>
        <source>Allow all programs to start in this sandbox.</source>
        <translation>이 샌드박스에서 모든 프로그램을 시작할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2172"/>
        <source>* Note: Programs installed to this sandbox won&apos;t be able to start at all.</source>
        <translation>* 참고: 이 샌드박스에 설치된 프로그램은 시작할 수 없습니다.</translation>
    </message>
    <message>
        <source>Internet Restrictions</source>
        <translation type="vanished">인터넷 제한</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2891"/>
        <source>Process Restrictions</source>
        <translation>프로세스 제한</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2911"/>
        <source>Issue message 1307 when a program is denied internet access</source>
        <translation>프로그램이 인터넷 액세스를 거부하면 메시지 1307 발행</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2925"/>
        <source>Prompt user whether to allow an exemption from the blockade.</source>
        <translation>차단 면제를 허용할지 여부를 사용자에게 묻습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2939"/>
        <source>Note: Programs installed to this sandbox won&apos;t be able to access the internet at all.</source>
        <translation>참고: 이 샌드박스에 설치된 프로그램은 인터넷에 전혀 액세스할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2252"/>
        <location filename="Forms/OptionsWindow.ui" line="2352"/>
        <location filename="Forms/OptionsWindow.ui" line="2485"/>
        <location filename="Forms/OptionsWindow.ui" line="2605"/>
        <location filename="Forms/OptionsWindow.ui" line="2679"/>
        <location filename="Forms/OptionsWindow.ui" line="2968"/>
        <source>Access</source>
        <translation>액세스</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2978"/>
        <source>Set network/internet access for unlisted processes:</source>
        <translation>목록에 없는 프로세스에 대한 네트워크/인터넷 액세스 설정:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3025"/>
        <source>Test Rules, Program:</source>
        <translation>테스트 규칙, 프로그램:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3035"/>
        <source>Port:</source>
        <translation>포트:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3045"/>
        <source>IP:</source>
        <translation>IP:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3055"/>
        <source>Protocol:</source>
        <translation>프로토콜:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3065"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3081"/>
        <source>Add Rule</source>
        <translation>규칙 추가</translation>
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
        <translation>프로그램</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="561"/>
        <location filename="Forms/OptionsWindow.ui" line="3097"/>
        <location filename="Forms/OptionsWindow.ui" line="3536"/>
        <location filename="Forms/OptionsWindow.ui" line="3861"/>
        <source>Action</source>
        <translation>동작</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3102"/>
        <source>Port</source>
        <translation>포트</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3107"/>
        <source>IP</source>
        <translation>IP</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3112"/>
        <source>Protocol</source>
        <translation>프로토콜</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3134"/>
        <source>CAUTION: Windows Filtering Platform is not enabled with the driver, therefore these rules will be applied only in user mode and can not be enforced!!! This means that malicious applications may bypass them.</source>
        <translation>주의: Windows 필터링 플랫폼이 드라이버에서 사용할 수 없으므로 이 규칙은 사용자 모드에서만 적용되며 강제 적용할 수 없습니다!!! 즉, 악성 프로그램이 이를 무시할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2194"/>
        <source>Resource Access</source>
        <translation>리소스 액세스</translation>
    </message>
    <message>
        <source>Resource Access Rules</source>
        <translation type="vanished">리소스 액세스 규칙</translation>
    </message>
    <message>
        <source>Configure which processes can access what resources. Double click on an entry to edit it.
&apos;Open&apos; File and Key access only applies to program binaries located outside the sandbox.
You can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behaviour in the Policies tab.</source>
        <translation type="vanished">어떤 프로세스가 어떤 리소스에 액세스할 수 있는지 구성합니다. 항목을 두 번 눌러 편집합니다.
파일 및 키 &apos;열기&apos; 액세스은 샌드박스 외부에 위치한 프로그램 이진 파일에만 적용됩니다.
대신 &apos;모두 열기&apos;를 사용하여 모든 프로그램에 적용하거나 정책 탭에서 이 동작을 변경할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2277"/>
        <source>Add File/Folder</source>
        <translation>파일/폴더 추가</translation>
    </message>
    <message>
        <source>Remove User</source>
        <translation type="vanished">사용자 제거</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2571"/>
        <source>Add Wnd Class</source>
        <translation>창 클래스 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2450"/>
        <source>Add IPC Path</source>
        <translation>IPC 경로 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2384"/>
        <source>Add Reg Key</source>
        <translation>Reg 키 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2711"/>
        <source>Add COM Object</source>
        <translation>COM 개체 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3151"/>
        <source>File Recovery</source>
        <translation>파일 복구</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3230"/>
        <source>Add Folder</source>
        <translation>폴더 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3270"/>
        <source>Ignore Extension</source>
        <translation>확장자 무시</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3316"/>
        <source>Ignore Folder</source>
        <translation>폴더 무시</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3277"/>
        <source>Enable Immediate Recovery prompt to be able to recover files as soon as they are created.</source>
        <translation>파일이 생성되는 즉시 복구할 수 있도록 즉시 복구 프롬프트를 실행합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3260"/>
        <source>You can exclude folders and file types (or file extensions) from Immediate Recovery.</source>
        <translation>즉시 복구에서 폴더 및 파일 유형 (또는 파일 확장자)을 제외할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3193"/>
        <source>When the Quick Recovery function is invoked, the following folders will be checked for sandboxed content. </source>
        <translation>빠른 복구 기능이 호출되면 샌드박스 내용에 대해 다음 폴더가 확인됩니다. </translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3358"/>
        <source>Advanced Options</source>
        <translation>고급 옵션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3950"/>
        <source>Miscellaneous</source>
        <translation>기타</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2618"/>
        <source>Don&apos;t alter window class names created by sandboxed programs</source>
        <translation>샌드박스 프로그램에서 만든 창 클래스 이름 변경 안 함</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1505"/>
        <source>Do not start sandboxed services using a system token (recommended)</source>
        <translation>시스템 토큰을 사용하여 샌드박스 서비스를 시작하지 않음 (권장)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1252"/>
        <location filename="Forms/OptionsWindow.ui" line="1269"/>
        <location filename="Forms/OptionsWindow.ui" line="1354"/>
        <location filename="Forms/OptionsWindow.ui" line="1426"/>
        <location filename="Forms/OptionsWindow.ui" line="1450"/>
        <location filename="Forms/OptionsWindow.ui" line="1474"/>
        <source>Protect the sandbox integrity itself</source>
        <translation>샌드박스 무결성 자체 보호</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1498"/>
        <source>Drop critical privileges from processes running with a SYSTEM token</source>
        <translation>SYSTEM 토큰으로 실행 중인 프로세스에서 중요한 권한 삭제</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1491"/>
        <location filename="Forms/OptionsWindow.ui" line="1539"/>
        <source>(Security Critical)</source>
        <translation>(보안 중요)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1460"/>
        <source>Protect sandboxed SYSTEM processes from unprivileged processes</source>
        <translation>권한이 없는 프로세스로부터 샌드박스 SYSTEM 프로세스 보호</translation>
    </message>
    <message>
        <source>Sandbox isolation</source>
        <translation type="vanished">샌드박스 격리</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3403"/>
        <source>Force usage of custom dummy Manifest files (legacy behaviour)</source>
        <translation>사용자 지정 더미 매니페스트 파일 강제 사용 (레거시 동작)</translation>
    </message>
    <message>
        <source>Network Firewall Rules</source>
        <translation type="vanished">네트워크 방화벽 규칙</translation>
    </message>
    <message>
        <source>Resource Access Policies</source>
        <translation type="vanished">리소스 액세스 정책</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2833"/>
        <source>The rule specificity is a measure to how well a given rule matches a particular path, simply put the specificity is the length of characters from the begin of the path up to and including the last matching non-wildcard substring. A rule which matches only file types like &quot;*.tmp&quot; would have the highest specificity as it would always match the entire file path.
The process match level has a higher priority than the specificity and describes how a rule applies to a given process. Rules applying by process name or group have the strongest match level, followed by the match by negation (i.e. rules applying to all processes but the given one), while the lowest match levels have global matches, i.e. rules that apply to any process.</source>
        <translation>규칙 특수성은 지정된 규칙이 특정 경로와 얼마나 잘 일치하는지 측정하는 것으로, 단순하게 말해서 특수성은 경로 시작부터 마지막 일치하는 비 와일드카드 하위 문자열까지 포함한 문자 길이입니다. &quot;*.tmp&quot;와 같은 파일 형식만 일치하는 규칙은 항상 전체 파일 경로와 일치하므로 가장 높은 특수성을 가집니다.
프로세스 일치 수준은 특수성보다 높은 우선 순위를 가지며 규칙이 지정된 프로세스에 적용되는 방식을 설명합니다. 프로세스 이름 또는 그룹별로 적용되는 규칙은 일치 수준이 가장 강하고 부정에 의한 일치 수준 (즉, 지정된 프로세스를 제외한 모든 프로세스에 적용되는 규칙)이 그 뒤를 이으며, 가장 낮은 일치 수준에는 전역 일치, 즉 모든 프로세스에 적용되는 규칙이 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2792"/>
        <source>Prioritize rules based on their Specificity and Process Match Level</source>
        <translation>특수성 및 프로세스 일치 수준에 따라 규칙 우선 순위 지정</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2844"/>
        <source>Privacy Mode, block file and registry access to all locations except the generic system ones</source>
        <translation>개인 정보 보호 모드, 일반 시스템 위치를 제외한 모든 위치에 대한 파일 및 레지스트리 액세스 차단</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2826"/>
        <source>Access Mode</source>
        <translation>액세스 모드</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2864"/>
        <source>When the Privacy Mode is enabled, sandboxed processes will be only able to read C:\Windows\*, C:\Program Files\*, and parts of the HKLM registry, all other locations will need explicit access to be readable and/or writable. In this mode, Rule Specificity is always enabled.</source>
        <translation>개인 정보 모드가 활성화된 경우 샌드박스 프로세스는 C:만 읽을 수 있습니다. \Windows\*, C:\Program Files\* 및 HKLM 레지스트리의 일부 다른 위치에서는 읽기 및/또는 쓰기 가능하려면 명시적 액세스 권한이 필요합니다. 이 모드에서는 규칙 특정성이 항상 활성화됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2778"/>
        <source>Rule Policies</source>
        <translation>규칙 정책</translation>
    </message>
    <message>
        <source>Apply Close...=!&lt;program&gt;,... rules also to all binaries located in the sandboxed.</source>
        <translation type="vanished">닫기 적용...=!&lt;프로그램&gt;,... 또한 규칙은 샌드박스에 있는 모든 이진 파일에 적용됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2799"/>
        <source>Apply File and Key Open directives only to binaries located outside the sandbox.</source>
        <translation>샌드박스 외부에 있는 이진 파일에만 파일 및 키 열기 지시문을 적용합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1436"/>
        <source>Start the sandboxed RpcSs as a SYSTEM process (not recommended)</source>
        <translation>SYSTEM 프로세스로 샌드박스 RpcS 시작 (권장하지 않음)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1525"/>
        <source>Allow only privileged processes to access the Service Control Manager</source>
        <translation>권한 있는 프로세스만 서비스 제어 관리자에 액세스할 수 있도록 허용</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3375"/>
        <location filename="Forms/OptionsWindow.ui" line="3424"/>
        <source>Compatibility</source>
        <translation>호환성</translation>
    </message>
    <message>
        <source>Open access to COM infrastructure (not recommended)</source>
        <translation type="vanished">COM 인프라에 대한 개방형 액세스 (권장하지 않음)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1559"/>
        <source>Add sandboxed processes to job objects (recommended)</source>
        <translation>작업 개체에 샌드박스 프로세스 추가 (권장)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3410"/>
        <source>Emulate sandboxed window station for all processes</source>
        <translation>모든 프로세스에 대해 샌드박스 창 스테이션 에뮬레이트</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3431"/>
        <source>Allow use of nested job objects (works on Windows 8 and later)</source>
        <translation>중첩된 작업 개체 사용 허용 (Windows 8 이상에서 작동)</translation>
    </message>
    <message>
        <source>Isolation</source>
        <translation type="vanished">격리</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1228"/>
        <source>Security Isolation through the usage of a heavily restricted process token is Sandboxie&apos;s primary means of enforcing sandbox restrictions, when this is disabled the box is operated in the application compartment mode, i.e. it’s no longer providing reliable security, just simple application compartmentalization.</source>
        <translation>매우 제한된 프로세스 토큰의 사용을 통한 보안 격리는 샌드박스 제한을 시행하는 Sandboxie의 주요 수단이며, 이 기능이 비활성화되면 박스가 응용 프로그램 구획 모드에서 작동됩니다 (즉, 더 이상 신뢰할 수있는 보안을 제공하지 않고 간단한 응용 프로그램 구획화).</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1279"/>
        <source>Allow sandboxed programs to manage Hardware/Devices</source>
        <translation>샌드박스 프로그램에서 하드웨어/장치 관리 허용</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1197"/>
        <source>Disable Security Isolation (experimental)</source>
        <translation>보안 격리 사용 안 함 (실험적)</translation>
    </message>
    <message>
        <source>Various advanced isolation features can break compatibility with some applications. If you are using this sandbox &lt;b&gt;NOT for Security&lt;/b&gt; but for simple application portability, by changing these options you can restore compatibility by sacrificing some security.</source>
        <translation type="vanished">다양한 고급 격리 기능으로 인해 일부 응용 프로그램과의 호환성이 손상될 수 있습니다. 이 샌드박스를 &lt;b&gt;보안이 아닌&lt;/b&gt; 단순한 응용프로그램 이식용으로 사용하는 경우 이러한 옵션을 변경하여 일부 보안을 희생하여 호환성을 복원할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1204"/>
        <source>Open access to Windows Security Account Manager</source>
        <translation>Windows 보안 계정 관리자에 대한 액세스 열기</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1238"/>
        <source>Open access to Windows Local Security Authority</source>
        <translation>Windows 로컬 보안 기관에 대한 개방형 액세스 권한</translation>
    </message>
    <message>
        <source>Security</source>
        <translation type="vanished">보안</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1040"/>
        <source>Security enhancements</source>
        <translation>보안 강화</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="971"/>
        <source>Use the original token only for approved NT system calls</source>
        <translation>승인된 NT 시스템 호출에만 원본 토큰 사용</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1023"/>
        <source>Restrict driver/device access to only approved ones</source>
        <translation>드라이버/장치 액세스만 승인된 것으로 제한</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="978"/>
        <source>Enable all security enhancements (make security hardened box)</source>
        <translation>모든 보안 향상 사용 (보안 강화 박스 만들기)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="668"/>
        <source>Allow to read memory of unsandboxed processes (not recommended)</source>
        <translation>샌드박스되지 않은 프로세스의 메모리 읽기 허용(권장하지 않음)</translation>
    </message>
    <message>
        <source>COM/RPC</source>
        <translation type="vanished">COM/RPC</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3451"/>
        <source>Disable the use of RpcMgmtSetComTimeout by default (this may resolve compatibility issues)</source>
        <translation>기본적으로 RpcMgmtSetComTimeout 사용 안 함 (호환성 문제가 해결될 수 있음)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1255"/>
        <source>Security Isolation &amp; Filtering</source>
        <translation>보안 격리 및 필터링</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1211"/>
        <source>Disable Security Filtering (not recommended)</source>
        <translation>보안 필터링 사용 안 함 (권장하지 않음)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1218"/>
        <source>Security Filtering used by Sandboxie to enforce filesystem and registry access restrictions, as well as to restrict process access.</source>
        <translation>Sandboxie에서 사용하는 보안 필터링은 파일 시스템 및 레지스트리 액세스 제한을 적용하고 프로세스 액세스를 제한하는 데 사용됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1286"/>
        <source>The below options can be used safely when you don&apos;t grant admin rights.</source>
        <translation>다음 옵션은 관리자 권한을 부여하지 않을 때 안전하게 사용할 수 있습니다.</translation>
    </message>
    <message>
        <source>Access isolation</source>
        <translation type="vanished">액세스 격리</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3468"/>
        <source>Triggers</source>
        <translation>트리거</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3531"/>
        <source>Event</source>
        <translation>이벤트</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3501"/>
        <location filename="Forms/OptionsWindow.ui" line="3520"/>
        <location filename="Forms/OptionsWindow.ui" line="3658"/>
        <source>Run Command</source>
        <translation>명령 실행</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3690"/>
        <source>Start Service</source>
        <translation>서비스 시작</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3665"/>
        <source>These events are executed each time a box is started</source>
        <translation>이 이벤트는 박스가 시작될 때마다 실행됩니다</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3668"/>
        <source>On Box Start</source>
        <translation>박스 시작 시</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3549"/>
        <location filename="Forms/OptionsWindow.ui" line="3697"/>
        <source>These commands are run UNBOXED just before the box content is deleted</source>
        <translation>이 명령은 박스 내용이 삭제되기 직전에 UNBOXED로 실행됩니다</translation>
    </message>
    <message>
        <source>On Box Delete</source>
        <translation type="vanished">박스 삭제 시</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3623"/>
        <source>These commands are executed only when a box is initialized. To make them run again, the box content must be deleted.</source>
        <translation>이러한 명령은 박스가 초기화될 때만 실행됩니다. 다시 실행하려면 박스 내용을 삭제해야 합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3626"/>
        <source>On Box Init</source>
        <translation>박스 초기화 시</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3636"/>
        <source>Here you can specify actions to be executed automatically on various box events.</source>
        <translation>여기서 다양한 박스 이벤트에 대해 자동으로 실행할 동작을 지정할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3718"/>
        <source>Hide Processes</source>
        <translation>프로세스 숨기기</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3767"/>
        <location filename="Forms/OptionsWindow.ui" line="3825"/>
        <source>Add Process</source>
        <translation>프로세스 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3743"/>
        <source>Hide host processes from processes running in the sandbox.</source>
        <translation>샌드박스에서 실행 중인 프로세스에서 호스트 프로세스를 숨깁니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="259"/>
        <source>Double click action:</source>
        <translation>두 번 클릭 동작:</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="359"/>
        <source>Separate user folders</source>
        <translation>개별 사용자 폴더</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="396"/>
        <source>Box Structure</source>
        <translation>박스 구조</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="953"/>
        <source>Security Options</source>
        <translation>보안 옵션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="963"/>
        <source>Security Hardening</source>
        <translation>보안 강화</translation>
    </message>
    <message>
        <source>Various Restrictions</source>
        <translation type="vanished">다양한 제한 사항</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1153"/>
        <source>Security Isolation</source>
        <translation>보안 격리</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1174"/>
        <source>Various isolation features can break compatibility with some applications. If you are using this sandbox &lt;b&gt;NOT for Security&lt;/b&gt; but for application portability, by changing these options you can restore compatibility by sacrificing some security.</source>
        <translation>다양한 분리 기능은 일부 응용 프로그램과의 호환성을 손상시킬 수 있습니다. 이 샌드박스를 &lt;b&gt;보안이 아닌&lt;/b&gt; 응용 프로그램 이동성을 위해 사용하는 경우 이러한 옵션을 변경하여 일부 보안을 희생하여 호환성을 복원할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1272"/>
        <source>Access Isolation</source>
        <translation>액세스 격리</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1299"/>
        <location filename="Forms/OptionsWindow.ui" line="1357"/>
        <source>Image Protection</source>
        <translation>이미지 보호</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1307"/>
        <source>Issue message 1305 when a program tries to load a sandboxed dll</source>
        <translation>프로그램이 샌드박스된 dll을 로드하려고 할 때 1305 메시지 발생</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1340"/>
        <source>Prevent sandboxes programs installed on host from loading dll&apos;s from the sandbox</source>
        <translation>호스트에 설치된 샌드박스 프로그램이 샌드박스에서 dll을 로드하지 못하도록 방지</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1383"/>
        <source>Sandboxie’s resource access rules often discriminate against program binaries located inside the sandbox. OpenFilePath and OpenKeyPath work only for application binaries located on the host natively. In order to define a rule without this restriction, OpenPipePath or OpenConfPath must be used. Likewise, all Closed(File|Key|Ipc)Path directives which are defined by negation e.g. ‘ClosedFilePath=! iexplore.exe,C:Users*’ will be always closed for binaries located inside a sandbox. Both restriction policies can be disabled on the “Access policies” page.
This is done to prevent rogue processes inside the sandbox from creating a renamed copy of themselves and accessing protected resources. Another exploit vector is the injection of a library into an authorized process to get access to everything it is allowed to access. Using Host Image Protection, this can be prevented by blocking applications (installed on the host) running inside a sandbox from loading libraries from the sandbox itself.</source>
        <translation>샌드박스의 리소스 액세스 규칙은 샌드박스 내부에 있는 프로그램 이진 파일을 차별하는 경우가 많습니다. OpenFilePath 및 OpenKeyPath는 호스트에 기본적으로 있는 응용 프로그램 이진 파일에만 작동합니다. 이 제한이 없는 규칙을 정의하려면 OpenPipePath 또는 OpenConfPath를 사용해야 합니다. 마찬가지로 모두 닫힘 (파일|키|Ipc)예를 들어 ‘ClosedFilePath=! iexplore.exe,C:Users*&apos;는 샌드박스 내에 있는 이진 파일의 경우 항상 닫혀 있습니다. 두 제한 정책 모두 &quot;액세스 정책&quot; 페이지에서 사용 불가능으로 설정할 수 있습니다.
이 작업은 샌드박스 내부의 악성 프로세스가 자신의 이름이 변경된 복사본을 생성하여 보호된 리소스에 액세스하는 것을 방지하기 위해 수행됩니다. 또 다른 공격 벡터는 라이브러리가 액세스할 수 있는 모든 항목에 액세스하기 위해 승인된 프로세스에 라이브러리를 주입하는 것입니다. 호스트 이미지 보호를 사용하면 샌드박스 내부에서 실행 중인 응용 프로그램 (호스트에 설치)이 샌드박스 자체에서 라이브러리를 로드하지 못하도록 차단하여 이러한 문제를 방지할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1404"/>
        <source>Advanced Security</source>
        <oldsource>Adcanced Security</oldsource>
        <translation>고급 보안</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1412"/>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation>익명 토큰 대신 샌드박스 로그인 사용 (실험적)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1429"/>
        <source>Other isolation</source>
        <translation>기타 격리</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1453"/>
        <source>Privilege isolation</source>
        <translation>권한 분리</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1477"/>
        <source>Sandboxie token</source>
        <translation>Sandboxie 토큰</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1566"/>
        <source>Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.</source>
        <translation>사용자 정의 Sandboxie 토큰을 사용하면 개별 Sandboxie를 서로 더 잘 분리할 수 있으며, 작업 관리자의 사용자 열에 프로세스가 속한 박스의 이름이 표시됩니다. 그러나 일부 타사 보안 솔루션에는 사용자 지정 토큰에 문제가 있을 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1657"/>
        <source>Program Control</source>
        <translation>프로그램 제어</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1679"/>
        <source>Force Programs</source>
        <translation>강제 프로그램</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1791"/>
        <source>Disable forced Process and Folder for this sandbox</source>
        <translation>이 샌드박스에 대해 강제 프로세스 및 폴더 사용 안 함</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1801"/>
        <source>Breakout Programs</source>
        <translation>탈옥 프로그램</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1870"/>
        <source>Breakout Program</source>
        <translation>탈옥 프로그램</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1896"/>
        <source>Breakout Folder</source>
        <translation>탈옥 폴더</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1903"/>
        <source>Programs entered here will be allowed to break out of this sandbox when they start. It is also possible to capture them into another sandbox, for example to have your web browser always open in a dedicated box.</source>
        <oldsource>Programs entered here will be allowed to break out of this box when thay start, you can capture them into an other box. For example to have your web browser always open in a dedicated box. This feature requires a valid supporter certificate to be installed.</oldsource>
        <translation>여기에 입력된 프로그램은 시작할 때 이 박스에서 벗어날 수 있습니다. 다른 박스에 캡처할 수 있습니다. 예를 들어 웹 브라우저를 항상 전용 박스에 열도록 합니다. 이 기능을 설치하려면 올바른 후원자 인증서를 설치해야 합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1930"/>
        <source>Lingering Programs</source>
        <translation>남은 프로그램</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="1996"/>
        <source>Lingering programs will be automatically terminated if they are still running after all other processes have been terminated.</source>
        <translation>남은 프로그램은 다른 모든 프로세스가 종료된 후에도 계속 실행 중인 경우 자동으로 종료됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2009"/>
        <source>Leader Programs</source>
        <translation>대표 프로그램</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2075"/>
        <source>If leader processes are defined, all others are treated as lingering processes.</source>
        <translation>대표 프로세스가 정의되어 있는 경우 다른 모든 프로세스는 계속 진행 중인 프로세스로 간주됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2204"/>
        <source>Files</source>
        <translation>파일</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2297"/>
        <source>Configure which processes can access Files, Folders and Pipes. 
&apos;Open&apos; access only applies to program binaries located outside the sandbox, you can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behavior in the Policies tab.</source>
        <translation>파일, 폴더 및 파이프에 액세스할 수 있는 프로세스를 구성합니다. &apos;열기&apos; 액세스은 샌드박스 외부에 위치한 프로그램 이진 파일에만 적용되며, 대신 &apos;모두 열기&apos;를 사용하여 모든 프로그램에 적용하거나 정책 탭에서 이 동작을 변경할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2311"/>
        <source>Registry</source>
        <translation>레지스트리</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2404"/>
        <source>Configure which processes can access the Registry. 
&apos;Open&apos; access only applies to program binaries located outside the sandbox, you can use &apos;Open for All&apos; instead to make it apply to all programs, or change this behavior in the Policies tab.</source>
        <translation>레지스트리에 액세스할 수 있는 프로세스를 구성합니다. &apos;열기&apos; 액세스은 샌드박스 외부에 위치한 프로그램 이진 파일에만 적용되며, 대신 &apos;모두 열기&apos;를 사용하여 모든 프로그램에 적용하거나 정책 탭에서 이 동작을 변경할 수 있습니다.</translation>
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
        <translation>ALPC 포트 및 기타 프로세스 메모리 및 컨텍스트와 같은 NT IPC 개체에 액세스할 수 있는 프로세스를 구성합니다.
프로세스를 지정하려면 &apos;$:program.exe&apos;를 경로로 사용합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2525"/>
        <source>Wnd</source>
        <translation>창</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2610"/>
        <source>Wnd Class</source>
        <translation>창 클래스</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2625"/>
        <source>Configure which processes can access desktop objects like windows and alike.</source>
        <oldsource>Configure which processes can access Desktop objects like Windows and alike.</oldsource>
        <translation>Windows 등의 데스크탑 개체에 액세스스할 수 있는 프로세스를 구성합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2638"/>
        <source>COM</source>
        <translation>COM</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2684"/>
        <source>Class Id</source>
        <translation>클래스 Id</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2731"/>
        <source>Configure which processes can access COM objects.</source>
        <translation>COM 개체에 액세스할 수 있는 프로세스를 구성합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2743"/>
        <source>Don&apos;t use virtualized COM, Open access to hosts COM infrastructure (not recommended)</source>
        <translation>가상화된 COM 사용 안 함, 호스트 COM 인프라에 대한 액세스 열기 (권장하지 않음)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2751"/>
        <source>Access Policies</source>
        <translation>액세스 정책</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2785"/>
        <source>Apply Close...=!&lt;program&gt;,... rules also to all binaries located in the sandbox.</source>
        <translation>닫기 적용...=!&lt;프로그램&gt;,... 또한 샌드박스에 있는 모든 이진 파일에 대한 규칙도 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2881"/>
        <source>Network Options</source>
        <translation>네트워크 옵션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3161"/>
        <source>Quick Recovery</source>
        <translation>빠른 복구</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3240"/>
        <source>Immediate Recovery</source>
        <translation>즉시 복구</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3337"/>
        <source>Various Options</source>
        <translation>다양한 옵션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3396"/>
        <source>Apply ElevateCreateProcess Workaround (legacy behaviour)</source>
        <translation>ElevateCreateProcess 해결 방법 적용 (레거시 동작)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3458"/>
        <source>Use desktop object workaround for all processes</source>
        <translation>모든 프로세스에 대해 데스크톱 개체 해결 방법 사용</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3498"/>
        <source>This command will be run before the box content will be deleted</source>
        <translation>박스 내용이 삭제되기 전에 이 명령이 실행됩니다</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3552"/>
        <source>On File Recovery</source>
        <translation>파일 복구 시</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3581"/>
        <source>This command will be run before a file is being recovered and the file path will be passed as the first argument. If this command returns anything other than 0, the recovery will be blocked</source>
        <oldsource>This command will be run before a file is being recoverd and the file path will be passed as the first argument, if this command return something other than 0 the recovery will be blocked</oldsource>
        <translation>이 명령은 파일을 복구하기 전에 실행되며 파일 경로가 첫 번째 인수로 전달됩니다. 이 명령이 0이 아닌 다른 것을 반환하는 경우 복구가 차단됩니다</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3584"/>
        <source>Run File Checker</source>
        <translation>파일 검사 실행</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3700"/>
        <source>On Delete Content</source>
        <translation>콘텐츠 삭제 시</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3774"/>
        <source>Don&apos;t allow sandboxed processes to see processes running in other boxes</source>
        <translation>샌드박스 프로세스에서 다른 박스에서 실행 중인 프로세스 보기 허용 안 함</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3781"/>
        <source>Protect processes in this box from being accessed by specified unsandboxed host processes.</source>
        <translation>지정된 샌드박스되지 않은 호스트 프로세스가 이 박스에 액세스하지 못하도록 이 박스에 있는 프로세스를 보호합니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3792"/>
        <location filename="Forms/OptionsWindow.ui" line="3856"/>
        <source>Process</source>
        <translation>프로세스</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3832"/>
        <source>Block also read access to processes in this sandbox</source>
        <translation>이 샌드박스의 프로세스에 대한 읽기 액세스도 차단</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3882"/>
        <source>Users</source>
        <translation>사용자</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3900"/>
        <source>Restrict Resource Access monitor to administrators only</source>
        <translation>리소스 액세스 모니터를 관리자로만 제한</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3907"/>
        <source>Add User</source>
        <translation>사용자 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3937"/>
        <source>Add user accounts and user groups to the list below to limit use of the sandbox to only those accounts.  If the list is empty, the sandbox can be used by all user accounts.

Note:  Forced Programs and Force Folders settings for a sandbox do not apply to user accounts which cannot use the sandbox.</source>
        <translation>샌드박스 사용을 해당 계정으로만 제한하려면 아래 목록에 사용자 계정 및 사용자 그룹을 추가하십시오. 목록이 비어 있으면 모든 사용자 계정에서 샌드박스를 사용할 수 있습니다.

참고: 샌드박스에 대한 강제 프로그램 및 강제 폴더 설정은 샌드박스를 사용할 수 없는 사용자 계정에는 적용되지 않습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3982"/>
        <source>Add Option</source>
        <translation>옵션 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="3996"/>
        <source>Here you can configure advanced per process options to improve compatibility and/or customize sandboxing behavior.</source>
        <oldsource>Here you can configure advanced per process options to improve compatibility and/or customize sand boxing behavior.</oldsource>
        <translation>여기서 호환성 향상 및/또는 샌드박스 동작을 사용자 정의하도록 고급 프로세스별 옵션을 구성할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4007"/>
        <source>Option</source>
        <translation>옵션</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4048"/>
        <source>Tracing</source>
        <translation>추적</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4056"/>
        <source>API call trace (requires LogAPI to be installed in the Sbie directory)</source>
        <oldsource>API call trace (requirers logapi to be installed in the sbie dir)</oldsource>
        <translation>API 호출 추적 (Sbie 디렉터리에 LogAPI를 설치해야 함)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4076"/>
        <source>Pipe Trace</source>
        <translation>파이프 추적</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4096"/>
        <source>Log all SetError&apos;s to Trace log (creates a lot of output)</source>
        <translation>모든 SetError를 추적 로그에 기록 (많은 출력을 생성)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4116"/>
        <source>Log Debug Output to the Trace Log</source>
        <translation>추적 로그에 디버그 출력 기록</translation>
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
        <translation>드라이버에 표시된 모든 액세스 이벤트를 리소스 액세스 로그에 기록합니다.

이 옵션은 이벤트 마스크를 &quot;*&quot;로 설정합니다 - 모든 액세스 이벤트
&quot;*&quot; 대신
&quot;A&quot; - 허용된 액세스
&quot;D&quot; - 거부된 액세스
&quot;I&quot; - 액세스 요청 무시
를 지정하여 ini를 사용하여 로깅을 사용자 지정할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4143"/>
        <source>Ntdll syscall Trace (creates a lot of output)</source>
        <translation>Ntdll syscall 추적 (많은 출력을 생성합니다)</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4150"/>
        <source>File Trace</source>
        <translation>파일 추적</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4157"/>
        <source>Disable Resource Access Monitor</source>
        <translation>리소스 액세스 모니터 사용 안 함</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4164"/>
        <source>IPC Trace</source>
        <translation>IPC 추적</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4171"/>
        <source>GUI Trace</source>
        <translation>GUI 추적</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4185"/>
        <source>Resource Access Monitor</source>
        <translation>리소스 액세스 모니터</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4212"/>
        <source>Access Tracing</source>
        <translation>액세스 추적</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4219"/>
        <source>COM Class Trace</source>
        <translation>COM 클래스 추적</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4226"/>
        <source>Key Trace</source>
        <translation>키 추적</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="2993"/>
        <location filename="Forms/OptionsWindow.ui" line="4233"/>
        <source>Network Firewall</source>
        <translation>네트워크 방화벽</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4250"/>
        <source>Debug</source>
        <translation>디버그</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4306"/>
        <source>WARNING, these options can disable core security guarantees and break sandbox security!!!</source>
        <translation>경고, 이러한 옵션은 핵심 보안 보장을 비활성화하고 샌드박스 보안을 파괴할 수 있습니다!!!</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4316"/>
        <source>These options are intended for debugging compatibility issues, please do not use them in production use. </source>
        <translation>이러한 옵션은 호환성 문제를 디버깅하기 위한 것이므로 프로덕션에서 사용하지 마십시오. </translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4331"/>
        <source>App Templates</source>
        <translation>앱 템플릿</translation>
    </message>
    <message>
        <source>Compatibility Templates</source>
        <translation type="vanished">템플릿 호환성</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4416"/>
        <source>Filter Categories</source>
        <translation>필터 범주</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4387"/>
        <source>Text Filter</source>
        <translation>텍스트 필터</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4409"/>
        <source>Add Template</source>
        <translation>템플릿 추가</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4364"/>
        <source>This list contains a large amount of sandbox compatibility enhancing templates</source>
        <translation>이 목록에는 많은 양의 샌드박스 호환성 향상 템플릿이 포함되어 있습니다</translation>
    </message>
    <message>
        <source>Remove Template</source>
        <translation type="vanished">템플릿 제거</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4433"/>
        <source>Category</source>
        <translation>범주</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4468"/>
        <source>Template Folders</source>
        <translation>템플릿 폴더</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4488"/>
        <source>Configure the folder locations used by your other applications.

Please note that this values are currently user specific and saved globally for all boxes.</source>
        <translation>다른 응용 프로그램에서 사용하는 폴더 위치를 구성합니다.

이 값은 현재 사용자마다 다르며 모든 박스에 대해 전역으로 저장됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4017"/>
        <location filename="Forms/OptionsWindow.ui" line="4519"/>
        <source>Value</source>
        <translation>값</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4530"/>
        <source>Accessibility</source>
        <translation>접근성</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4626"/>
        <source>To compensate for the lost protection, please consult the Drop Rights settings page in the Restrictions settings group.</source>
        <translation>손실된 보호를 보상하려면 제한 설정 그룹의 삭제 권한 설정 페이지를 참조하십시오.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4550"/>
        <source>Screen Readers: JAWS, NVDA, Window-Eyes, System Access</source>
        <translation>화면 판독기: JAWS, NVDA, Window-Eyes, 시스템 액세스</translation>
    </message>
    <message>
        <source>DNS Request Logging</source>
        <oldsource>Dns Request Logging</oldsource>
        <translation type="obsolete">DNS 요청 로깅</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4341"/>
        <source>Templates</source>
        <translation>템플릿</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4590"/>
        <source>The following settings enable the use of Sandboxie in combination with accessibility software.  Please note that some measure of Sandboxie protection is necessarily lost when these settings are in effect.</source>
        <translation>다음 설정은 내게 필요한 옵션 소프트웨어와 함께 Sandboxie를 사용할 수 있도록 합니다. 이러한 설정이 적용되면 일부 Sandboxie 보호 기능이 손실됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4643"/>
        <source>Edit ini Section</source>
        <translation>이 섹션 편집</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4649"/>
        <source>Edit ini</source>
        <translation>ini 편집</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4662"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="Forms/OptionsWindow.ui" line="4685"/>
        <source>Save</source>
        <translation>저장</translation>
    </message>
</context>
<context>
    <name>PopUpWindow</name>
    <message>
        <location filename="Forms/PopUpWindow.ui" line="32"/>
        <source>SandboxiePlus Notifications</source>
        <translation>SandboxiePlus 알림</translation>
    </message>
</context>
<context>
    <name>ProgramsDelegate</name>
    <message>
        <location filename="Windows/OptionsWindow.cpp" line="55"/>
        <source>Group: %1</source>
        <translation>그룹: %1</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="Views/SbieView.cpp" line="1497"/>
        <source>Drive %1</source>
        <translation>드라이브 %1</translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="SandMan.cpp" line="3604"/>
        <source>OK</source>
        <translation>확인</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3605"/>
        <source>Apply</source>
        <translation>적용</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3606"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3607"/>
        <source>&amp;Yes</source>
        <translation>예(&amp;Y)</translation>
    </message>
    <message>
        <location filename="SandMan.cpp" line="3608"/>
        <source>&amp;No</source>
        <translation>아니오(&amp;N)</translation>
    </message>
</context>
<context>
    <name>RecoveryWindow</name>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="32"/>
        <source>SandboxiePlus - Recovery</source>
        <translation>SandboxiePlus - 복구</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="164"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="68"/>
        <source>Recover target:</source>
        <translation>복구 대상:</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="42"/>
        <source>Add Folder</source>
        <translation>폴더 추가</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="61"/>
        <source>Delete Content</source>
        <translation>내용 삭제</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="110"/>
        <source>Recover</source>
        <translation>복원</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="78"/>
        <source>Refresh</source>
        <translation>새로 고침</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="142"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="171"/>
        <source>Show All Files</source>
        <translation>모든 파일 표시</translation>
    </message>
    <message>
        <location filename="Forms/RecoveryWindow.ui" line="184"/>
        <source>TextLabel</source>
        <translation>텍스트 레이블</translation>
    </message>
</context>
<context>
    <name>SelectBoxWindow</name>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="32"/>
        <source>SandboxiePlus select box</source>
        <translation>SandboxiePlus 선택 박스</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="87"/>
        <source>Select the sandbox in which to start the program, installer or document.</source>
        <translation>프로그램, 설치 프로그램 또는 문서를 시작할 샌드박스를 선택합니다.</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="97"/>
        <source>Run in a new Sandbox</source>
        <translation>새 샌드박스에서 실행</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="65"/>
        <source>Sandbox</source>
        <translation>샌드박스</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="80"/>
        <source>Run As UAC Administrator</source>
        <translation>UAC 관리자로 실행</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="47"/>
        <source>Run Sandboxed</source>
        <translation>샌드박스에서 실행</translation>
    </message>
    <message>
        <location filename="Forms/SelectBoxWindow.ui" line="73"/>
        <source>Run Outside the Sandbox</source>
        <translation>샌드박스 밖에서 실행</translation>
    </message>
</context>
<context>
    <name>SettingsWindow</name>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="32"/>
        <source>SandboxiePlus Settings</source>
        <translation>SandboxiePlus 설정</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="55"/>
        <source>General Config</source>
        <translation>일반 구성</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="80"/>
        <source>Show file recovery window when emptying sandboxes</source>
        <oldsource>Show first recovery window when emptying sandboxes</oldsource>
        <translation>샌드박스를 비울 때 파일 복구 창 표시</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="195"/>
        <source>Open urls from this ui sandboxed</source>
        <translation>이 UI 샌드박스에서 URL 열기</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="492"/>
        <source>Systray options</source>
        <translation>시스템 트레이 옵션</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="185"/>
        <source>UI Language:</source>
        <translation>UI 언어:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="394"/>
        <source>Shell Integration</source>
        <translation>쉘 통합</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="513"/>
        <source>Run Sandboxed - Actions</source>
        <translation>샌드박스에서 실행 - 동작</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="577"/>
        <source>Start Sandbox Manager</source>
        <translation>샌드박스 관리자 시작</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="633"/>
        <source>Start UI when a sandboxed process is started</source>
        <translation>샌드박스 프로세스가 시작될 때 UI 시작</translation>
    </message>
    <message>
        <source>Show Notifications for relevant log Messages</source>
        <translation type="vanished">관련 로그 메시지에 대한 알림 표시</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="626"/>
        <source>Start UI with Windows</source>
        <translation>Windows와 함께 UI 시작</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="534"/>
        <source>Add &apos;Run Sandboxed&apos; to the explorer context menu</source>
        <translation>탐색기의 상황에 맞는 메뉴에 &apos;샌드박스에서 실행&apos; 추가</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="151"/>
        <source>Run box operations asynchronously whenever possible (like content deletion)</source>
        <translation>가능한 경우 항상 비동기적으로 박스 작업 실행 (예: 내용 삭제)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="173"/>
        <source>Hotkey for terminating all boxed processes:</source>
        <translation>박스화된 모든 프로세스를 종료하기 위한 단축키:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="412"/>
        <source>Show boxes in tray list:</source>
        <translation>트레이 목록에 박스 표시:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="499"/>
        <source>Always use DefaultBox</source>
        <translation>항상 기본 박스 사용</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="527"/>
        <source>Add &apos;Run Un-Sandboxed&apos; to the context menu</source>
        <translation>상황에 맞는 메뉴에 &apos;샌드박스 없이 실행&apos; 추가</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="442"/>
        <source>Show a tray notification when automatic box operations are started</source>
        <translation>자동 박스 작업이 시작될 때 트레이 알림 표시</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="838"/>
        <source>* a partially checked checkbox will leave the behavior to be determined by the view mode.</source>
        <translation>* 부분적으로 선택된 확인란은 보기 모드에서 확인할 동작을 남깁니다.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1337"/>
        <source>Advanced Config</source>
        <translation>고급 구성</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1371"/>
        <source>Activate Kernel Mode Object Filtering</source>
        <translation>커널 모드 개체 필터링 활성화</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1531"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/filerootpath&quot;&gt;file system root&lt;/a&gt;: </source>
        <translation>샌드박스 &lt;a href=&quot;sbie://docs/filerootpath&quot;&gt;파일 시스템 루트&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1616"/>
        <source>Clear password when main window becomes hidden</source>
        <translation>기본 창이 숨겨질 때 암호 지우기</translation>
    </message>
    <message>
        <source>Separate user folders</source>
        <translation type="vanished">개별 사용자 폴더</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1488"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/ipcrootpath&quot;&gt;ipc root&lt;/a&gt;: </source>
        <translation>샌드박스 &lt;a href=&quot;sbie://docs/ipcrootpath&quot;&gt;ipc 루트&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1411"/>
        <source>Sandbox default</source>
        <translation>샌드박스 기본값</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1602"/>
        <source>Config protection</source>
        <translation>구성 보호</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1481"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="94"/>
        <source>SandMan Options</source>
        <translation>SandMan 옵션</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="208"/>
        <source>Notifications</source>
        <translation>알림</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="240"/>
        <source>Add Entry</source>
        <translation>항목 추가</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="357"/>
        <source>Show file migration progress when copying large files into a sandbox</source>
        <translation>대용량 파일을 샌드박스에 복사할 때 파일 마이그레이션 진행률 표시</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="248"/>
        <source>Message ID</source>
        <translation>메시지 ID</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="253"/>
        <source>Message Text (optional)</source>
        <translation>메시지 텍스트 (선택사항)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="268"/>
        <source>SBIE Messages</source>
        <translation>SBIE 메시지</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="287"/>
        <source>Delete Entry</source>
        <translation>항목 삭제</translation>
    </message>
    <message>
        <source>Don&apos;t show the popup message log for all SBIE messages</source>
        <translation type="vanished">모든 SBIE 메시지에 대한 팝업 메시지 로그 표시 안 함</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="301"/>
        <source>Notification Options</source>
        <translation>알림 옵션</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="327"/>
        <source>Sandboxie may be issue &lt;a href=&quot;sbie://docs/sbiemessages&quot;&gt;SBIE Messages&lt;/a&gt; to the Message Log and shown them as Popups. Some messages are informational and notify of a common, or in some cases special, event that has occurred, other messages indicate an error condition.&lt;br /&gt;You can hide selected SBIE messages from being popped up, using the below list:</source>
        <oldsource>Sandboxie may be issue &lt;a href= &quot;sbie://docs/ sbiemessages&quot;&gt;SBIE Messages&lt;/a&gt; to the Message Log and shown them as Popups. Some messages are informational and notify of a common, or in some cases special, event that has occurred, other messages indicate an error condition.&lt;br /&gt;You can hide selected SBIE messages from being popped up, using the below list:</oldsource>
        <translation>Sandboxie는 메시지 로그에 문제 &lt;a href=&quot;sbie://docs/sbiemessages&quot;&gt;SBIE 메시지&lt;/a&gt;가 발생하여 팝업으로 표시될 수 있습니다. 일부 메시지는 정보를 제공하며 일반적으로 발생한 이벤트 또는 경우에 따라 특별한 이벤트를 알려주고 다른 메시지는 오류 상태를 나타냅니다.&lt;br /&gt;아래 목록을 사용하여 선택한 SBIE 메시지가 팝업되지 않도록 숨길 수 있습니다:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="340"/>
        <source>Disable SBIE messages popups (they will still be logged to the Messages tab)</source>
        <oldsource>Disable SBIE messages popups (SBIE will still be logged to the log tab)</oldsource>
        <translation>SBIE 메시지 팝업 사용 안 함 (메시지 탭에 계속 기록됨)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="404"/>
        <source>Windows Shell</source>
        <translation>Windows 셸</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="670"/>
        <source>Icon</source>
        <translation>아이콘</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="727"/>
        <source>Move Up</source>
        <translation>위로 이동</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="746"/>
        <source>Move Down</source>
        <translation>아래로 이동</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="949"/>
        <source>Show overlay icons for boxes and processes</source>
        <translation>박스 및 프로세스에 대한 오버레이 아이콘 표시</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="959"/>
        <source>Display Options</source>
        <translation>디스플레이 옵션</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1083"/>
        <source>Graphic Options</source>
        <translation>그래픽 옵션</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1228"/>
        <source>Supporters of the Sandboxie-Plus project can receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;supporter certificate&lt;/a&gt;. It&apos;s like a license key but for awesome people using open source software. :-)</source>
        <translation>Sandboxie-Plus 프로젝트의 후원는 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;후원자 인증서&lt;/a&gt;를 받을 수 있습니다. 이것은 라이선스 키와 비슷하지만 오픈 소스 소프트웨어를 사용하는 멋진 사람들을 위한 것입니다. :-</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1321"/>
        <source>Keeping Sandboxie up to date with the rolling releases of Windows and compatible with all web browsers is a never-ending endeavor. You can support the development by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;directly contributing to the project&lt;/a&gt;, showing your support by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;purchasing a supporter certificate&lt;/a&gt;, becoming a patron by &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;subscribing on Patreon&lt;/a&gt;, or through a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;PayPal donation&lt;/a&gt;.&lt;br /&gt;Your support plays a vital role in the advancement and maintenance of Sandboxie.</source>
        <translation>Sandboxie를 Windows의 롤링 릴리스로 최신 상태로 유지하고 모든 웹 브라우저와 호환되는 것은 끊임없는 노력입니다. &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-contribute&quot;&gt;프로젝트에 직접적으로 기여&lt;/a&gt;하거나, &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&quot;&gt;후원자 인증서를 구매&lt;/a&gt;하여 지지를 표시하거나, &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Patreon에 가입&lt;/a&gt;하여 후원자가 되거나, &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;Pay-pal을 통해 개발을 지원&lt;/a&gt;할 수 있습니다.&lt;br /&gt;여러분의 지원은 샌드박스의 발전과 유지에 중요한 역할을 합니다.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1391"/>
        <source>Sandbox &lt;a href=&quot;sbie://docs/keyrootpath&quot;&gt;registry root&lt;/a&gt;: </source>
        <translation>샌드박스 &lt;a href=&quot;sbie://docs/keyrootpath&quot;&gt;레지스트리 루트&lt;/a&gt;: </translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1451"/>
        <source>Sandboxing features</source>
        <translation>샌드박스 기능</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1560"/>
        <source>Sandboxie.ini Presets</source>
        <translation>샌드박스.ini 프리셋</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1568"/>
        <source>Change Password</source>
        <translation>암호 변경</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1575"/>
        <source>Password must be entered in order to make changes</source>
        <translation>변경하려면 암호 입력</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1623"/>
        <source>Only Administrator user accounts can make changes</source>
        <translation>관리자 사용자 계정만 변경할 수 있음</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1630"/>
        <source>Watch Sandboxie.ini for changes</source>
        <translation>Sandboxie.ini에서 변경 내용 보기</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1748"/>
        <source>App Templates</source>
        <translation>앱 템플릿</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1758"/>
        <source>App Compatibility</source>
        <translation>앱 호환성</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1609"/>
        <source>Only Administrator user accounts can use Pause Forcing Programs command</source>
        <oldsource>Only Administrator user accounts can use Pause Forced Programs Rules command</oldsource>
        <translation>관리자 사용자 계정만 프로그램 일시 중지 명령을 사용할 수 있음</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1501"/>
        <source>Portable root folder</source>
        <translation>휴대용 루트 폴더</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="350"/>
        <source>Show recoverable files as notifications</source>
        <translation>복구 가능한 파일을 알림으로 표시</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="65"/>
        <source>General Options</source>
        <translation>일반 옵션</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="600"/>
        <source>Show Icon in Systray:</source>
        <translation>시스템 트레이에 아이콘 표시:</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1461"/>
        <source>Use Windows Filtering Platform to restrict network access</source>
        <translation>Windows 필터링 플랫폼을 사용하여 네트워크 액세스 제한</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1468"/>
        <source>Hook selected Win32k system calls to enable GPU acceleration (experimental)</source>
        <translation>선택한 Win32k 시스템 호출을 후크하여 GPU 가속 (실험적) 사용</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="73"/>
        <source>Count and display the disk space occupied by each sandbox</source>
        <oldsource>Count and display the disk space ocupied by each sandbox</oldsource>
        <translation>각 샌드박스가 차지하는 디스크 공간을 계산하고 표시</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="475"/>
        <source>Use Compact Box List</source>
        <translation>압축 박스 목록 사용</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="800"/>
        <source>Interface Config</source>
        <translation>인터페이스 구성</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="925"/>
        <source>Make Box Icons match the Border Color</source>
        <translation>테두리 색과 일치하는 박스 아이콘 만들기</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="915"/>
        <source>Use a Page Tree in the Box Options instead of Nested Tabs *</source>
        <translation>박스 옵션에서 중첩 탭 대신 페이지 트리 사용 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="908"/>
        <source>Interface Options</source>
        <translation>인터페이스 옵션</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="828"/>
        <source>Use large icons in box list *</source>
        <translation>박스 목록에서 큰 아이콘 사용 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="970"/>
        <source>High DPI Scaling</source>
        <translation>높은 DPI 스케일링</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="891"/>
        <source>Don&apos;t show icons in menus *</source>
        <translation>메뉴에 아이콘 표시 안 함 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="932"/>
        <source>Use Dark Theme</source>
        <translation>어두운 테마 사용</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="980"/>
        <source>Font Scaling</source>
        <translation>글꼴 크기 조정</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1042"/>
        <source>(Restart required)</source>
        <translation>(재시작 필요)</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="144"/>
        <source>Show the Recovery Window as Always on Top</source>
        <translation>복구 창을 항상 맨 위에 표시</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="868"/>
        <source>Show &quot;Pizza&quot; Background in box list *</source>
        <oldsource>Show &quot;Pizza&quot; Background in box list*</oldsource>
        <translation>박스 목록에 &quot;피자&quot; 배경 표시 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1056"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="942"/>
        <source>Alternate row background in lists</source>
        <translation>목록의 대체 행 배경</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="818"/>
        <source>Use Fusion Theme</source>
        <translation>퓨전 테마 사용</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1521"/>
        <source>Use a Sandboxie login instead of an anonymous token (experimental)</source>
        <translation>선택한 Win32k 시스템 호출을 후크하여 GPU 가속 (실험적) 사용</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1644"/>
        <source>Program Control</source>
        <translation>프로그램 제어</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="665"/>
        <location filename="Forms/SettingsWindow.ui" line="1676"/>
        <location filename="Forms/SettingsWindow.ui" line="1786"/>
        <location filename="Forms/SettingsWindow.ui" line="1919"/>
        <source>Name</source>
        <translation>이름</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1681"/>
        <source>Path</source>
        <translation>경로</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1703"/>
        <source>Remove Program</source>
        <translation>프로그램 제거</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1689"/>
        <source>Add Program</source>
        <translation>프로그램 추가</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1724"/>
        <source>When any of the following programs is launched outside any sandbox, Sandboxie will issue message SBIE1301.</source>
        <translation>다음 프로그램 중 하나가 샌드박스 외부에서 실행되면 샌드박스는 메시지 SBIE1301를 발행합니다.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1734"/>
        <source>Add Folder</source>
        <translation>폴더 추가</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1717"/>
        <source>Prevent the listed programs from starting on this system</source>
        <translation>나열된 프로그램이 이 시스템에서 시작되지 않도록 합니다</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1696"/>
        <source>Issue message 1308 when a program fails to start</source>
        <translation>프로그램이 시작되지 않을 때 메시지 1308 발행</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="137"/>
        <source>Recovery Options</source>
        <translation>복구 옵션</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="435"/>
        <source>Start Menu Integration</source>
        <translation>시작 메뉴 통합</translation>
    </message>
    <message>
        <source>Integrate boxes with Host Start Menu</source>
        <translation type="vanished">호스트 시작 메뉴에 박스 통합</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="520"/>
        <source>Scan shell folders and offer links in run menu</source>
        <translation>실행 메뉴에서 셸 폴더 검색 및 링크 제공</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="541"/>
        <source>Integrate with Host Start Menu</source>
        <translation>호스트 시작 메뉴와 통합</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="858"/>
        <source>Use new config dialog layout *</source>
        <translation>새 구성 대화 상자 레이아웃 사용 *</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1654"/>
        <source>Program Alerts</source>
        <translation>프로그램 경고</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1710"/>
        <source>Issue message 1301 when forced processes has been disabled</source>
        <translation>강제 프로세스가 비활성화되면 메시지 1301을 발행</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1347"/>
        <source>Sandboxie Config</source>
        <oldsource>Config Protection</oldsource>
        <translation>Sandboxie Config 구성</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="364"/>
        <source>This option also enables asynchronous operation when needed and suspends updates.</source>
        <translation>또한 이 옵션은 필요할 때 비동기식 작업을 활성화하고 업데이트를 일시 중단합니다.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="367"/>
        <source>Suppress pop-up notifications when in game / presentation mode</source>
        <translation>게임/프레젠테이션 모드에서 팝업 알림 표시 안 함</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="810"/>
        <source>User Interface</source>
        <translation>사용자 인터페이스</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="643"/>
        <source>Run Menu</source>
        <translation>실행 메뉴</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="695"/>
        <source>Add program</source>
        <translation>프로그램 추가</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="753"/>
        <source>You can configure custom entries for all sandboxes run menus.</source>
        <translation>모든 샌드박스 실행 메뉴에 대한 사용자 정의 항목을 구성할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="775"/>
        <location filename="Forms/SettingsWindow.ui" line="1866"/>
        <source>Remove</source>
        <translation>제거</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="675"/>
        <source>Command Line</source>
        <translation>명령 줄</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1097"/>
        <source>Support &amp;&amp; Updates</source>
        <translation>지원 및 업데이트</translation>
    </message>
    <message>
        <source>Sandbox Config</source>
        <translation type="vanished">샌드박스 구성</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1418"/>
        <source>Default sandbox:</source>
        <translation>기본 샌드박스:</translation>
    </message>
    <message>
        <source>Compatibility</source>
        <translation type="vanished">호환성</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1778"/>
        <source>In the future, don&apos;t check software compatibility</source>
        <translation>앞으로는 소프트웨어 호환성 검사 안 함</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1794"/>
        <source>Enable</source>
        <translation>사용함</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1814"/>
        <source>Disable</source>
        <translation>사용 안 함</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1821"/>
        <source>Sandboxie has detected the following software applications in your system. Click OK to apply configuration settings, which will improve compatibility with these applications. These configuration settings will have effect in all existing sandboxes and in any new sandboxes.</source>
        <translation>Sandboxie가 시스템에서 다음 소프트웨어 응용 프로그램을 탐지했습니다. 확인을 클릭하여 구성 설정을 적용하면 해당 응용프로그램과의 호환성이 향상됩니다. 이러한 구성 설정은 모든 기존 샌드박스 및 새 샌드박스에 적용됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1834"/>
        <source>Local Templates</source>
        <translation>로컬 템플릿</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1885"/>
        <source>Add Template</source>
        <translation>템플릿 추가</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1905"/>
        <source>Text Filter</source>
        <translation>텍스트 필터</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1927"/>
        <source>This list contains user created custom templates for sandbox options</source>
        <translation>이 목록에는 샌드박스 옵션에 대해 사용자가 생성한 사용자 지정 템플릿이 포함되어 있습니다</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1944"/>
        <source>Edit ini Section</source>
        <translation>INI 섹션 편집</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1953"/>
        <source>Save</source>
        <translation>저장</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1960"/>
        <source>Edit ini</source>
        <translation>ini 편집</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1986"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <source>Support</source>
        <translation type="vanished">지원</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1147"/>
        <source>Version Updates</source>
        <translation>버전 업데이트</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1208"/>
        <source>New full versions from the selected release channel.</source>
        <translation>선택한 릴리스 채널의 새 전체 버전입니다.</translation>
    </message>
    <message>
        <source>Full Updates</source>
        <translation type="vanished">전체 업데이트</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1144"/>
        <source>Hotpatches for the installed version, updates to the Templates.ini and translations.</source>
        <translation>설치된 버전의 핫패치, Templates.ini 및 변환에 대한 업데이트입니다.</translation>
    </message>
    <message>
        <source>Version Revisions</source>
        <translation type="vanished">버전 수정사항</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1161"/>
        <source>This supporter certificate has expired, please &lt;a href=&quot;sbie://update/cert&quot;&gt;get an updated certificate&lt;/a&gt;.</source>
        <translation>이 후원자 인증서가 만료되었습니다, &lt;a href=&quot;sbie://update/cert&quot;&gt;업데이트된 인증서를 받으십시오&lt;/a&gt;.</translation>
    </message>
    <message>
        <source>The live channel provides the latest unsigned test builds.</source>
        <translation type="vanished">라이브 채널은 서명되지 않은 최신 테스트 빌드를 제공합니다.</translation>
    </message>
    <message>
        <source>Search in the Live channel</source>
        <translation type="vanished">라이브 채널에서 검색</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1198"/>
        <source>The preview channel contains the latest GitHub pre-releases.</source>
        <translation>미리보기 채널에는 최신 GitHub 사전 릴리스가 포함되어 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1211"/>
        <source>New Versions</source>
        <translation>새 버전</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1218"/>
        <source>The stable channel contains the latest stable GitHub releases.</source>
        <translation>안정적인 채널에는 최신 안정적인 GitHub 릴리스가 포함되어 있습니다.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1221"/>
        <source>Search in the Stable channel</source>
        <translation>안정적인 채널에서 검색</translation>
    </message>
    <message>
        <source>Keeping Sandboxie up to date with the rolling releases of Windows and compatible with all web browsers is a never-ending endeavor. Please consider supporting this work with a donation.&lt;br /&gt;You can support the development with a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;PayPal donation&lt;/a&gt;, working also with credit cards.&lt;br /&gt;Or you can provide continuous support with a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Patreon subscription&lt;/a&gt;.</source>
        <translation type="vanished">Sandboxie를 Windows의 롤링 릴리스로 최신 상태로 유지하고 모든 웹 브라우저와 호환되도록 하는 것은 결코 끝나지 않는 노력입니다. 기부금으로 이 일을 후원하는 것을 고려해 주세요.&lt;br /&gt; &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=donate&quot;&gt;PayPal 기부금&lt;/a&gt;으로 개발을 지원할 수 있으며, 신용카드도 사용할 수 있습니다.&lt;br /&gt;또는 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=patreon&quot;&gt;Patreon 구독을 통해 지속적인 지원을 제공할 수 있습니다.&lt;/a&gt;.</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1201"/>
        <source>Search in the Preview channel</source>
        <translation>미리 보기 채널에서 검색</translation>
    </message>
    <message>
        <source>Supporters of the Sandboxie-Plus project can receive a &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-get-cert&quot;&gt;supporter certificate&lt;/a&gt;. It&apos;s like a license key but for awesome people using open source software. :-)</source>
        <translation type="vanished">Sandboxie-Plus 프로젝트의 후원자는 &lt;a href=&quot;https://sandboxie-plus.com/go.php?to=sbie-cert&quot;&gt;후원자 인증서&lt;/a&gt;를 받습니다. 라이선스 키와 비슷하지만 오픈 소스 소프트웨어를 사용하는 멋진 사람들을 위한 것입니다. :-)</translation>
    </message>
    <message>
        <source>Download Updates automatically</source>
        <translation type="vanished">자동으로 업데이트 다운로드</translation>
    </message>
    <message>
        <source>Search in the Release channel</source>
        <translation type="vanished">릴리스 채널에서 검색</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1171"/>
        <source>In the future, don&apos;t notify about certificate expiration</source>
        <translation>이후 인증서 만료에 대해 알리지 않음</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1184"/>
        <source>Enter the support certificate here</source>
        <translation>여기에 후원 인증서를 입력하십시오</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1268"/>
        <source>Support Settings</source>
        <translation>지원 설정</translation>
    </message>
    <message>
        <source>Install updates automatically</source>
        <translation type="vanished">업데이트 자동 설치</translation>
    </message>
    <message>
        <location filename="Forms/SettingsWindow.ui" line="1241"/>
        <source>Check periodically for updates of Sandboxie-Plus</source>
        <translation>Sandboxie-Plus의 업데이트를 정기적으로 확인합니다</translation>
    </message>
</context>
<context>
    <name>SnapshotsWindow</name>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="32"/>
        <source>SandboxiePlus - Snapshots</source>
        <translation>SandboxiePlus - 스냅샷</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="60"/>
        <source>Selected Snapshot Details</source>
        <translation>선택한 스냅샷 세부 정보</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="66"/>
        <source>Name:</source>
        <translation>이름:</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="120"/>
        <source>Description:</source>
        <translation>설명:</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="101"/>
        <source>When deleting a snapshot content, it will be returned to this snapshot instead of none.</source>
        <translation>스냅샷 내용을 삭제할 때 스냅샷이 없는 대신 이 스냅샷으로 반환됩니다.</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="104"/>
        <source>Default snapshot</source>
        <translation>기본 스냅샷</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="139"/>
        <source>Snapshot Actions</source>
        <translation>스냅샷 작업</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="189"/>
        <source>Remove Snapshot</source>
        <translation>스냅샷 제거</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="208"/>
        <source>Go to Snapshot</source>
        <translation>스냅샷으로 이동</translation>
    </message>
    <message>
        <location filename="Forms/SnapshotsWindow.ui" line="157"/>
        <source>Take Snapshot</source>
        <translation>스냅샷 생성</translation>
    </message>
</context>
</TS>
