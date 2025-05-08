# 更新日志
本项目的所有重要更改将在此文件中记录。
本项目遵循 [语义化版本控制](http://semver.org/)。


## [1.15.13 / 5.70.13] - 2025-04-??

### 新增
- 在捐赠支持页面新增了复制硬件 ID 的选项


## [1.15.12 / 5.70.12] - 2025-05-01

### 新增
- 为 Plus 界面新增印度尼西亚语翻译 [4bad4d1](https://github.com/sandboxie-plus/Sandboxie/commit/4bad4d190a90a6329d549e5a31ca8ee13b4eb9a1)（感谢 ？？？）
- 新增 WeType 输入法（IME）模板 [#4715](https://github.com/sandboxie-plus/Sandboxie/pull/4715)（感谢 bot-1450）
- 在代理配置中新增主机名支持（在 WSA_Startup 时查找主机）
- Sandboxie 现在支持绑定到适配器，而非仅绑定 IP（在 WSA_Startup 时解析 IP）
- 新增对 %ProgramFiles(x86)% 变量的支持 [#2402](https://github.com/sandboxie-plus/Sandboxie/issues/2402)
- 新增 TemplateDefaultFolders，包含所有需在增强隐私盒中创建的默认路径

### 更改
- 修正了沙盒列表中的异常排序问题 [#4615](https://github.com/sandboxie-plus/Sandboxie/issues/4615)
- 支持标签页中的名称与 HwID 信息默认隐藏，可按需切换显示 [#4714](https://github.com/sandboxie-plus/Sandboxie/pull/4714)
- 新增可选的代理中继实现（不再篡改现有套接字，而是启动中继线程）[实验性]

### 修复
- 修复了“在沙盒中运行”选择提示未显示沙盒别名的问题 [#4709](https://github.com/sandboxie-plus/Sandboxie/issues/4709)
- 修复了使用 Ctrl+F 后，沙盒分组的展开和关闭记录被打乱的问题 [#4708](https://github.com/sandboxie-plus/Sandboxie/issues/4708)
- 为注册表的 set/get 函数添加了额外的检查
- 修复了 Pool_Alloc 中的问题
- 修复了 SbieCtrl.exe 在 Process Explorer 中持续维持 3.2MB/s IO 速率的问题 [#4693](https://github.com/sandboxie-plus/Sandboxie/issues/4693)
- 修复了腾讯 TIM 模板导致无法使用拖放功能的问题 [#4688](https://github.com/sandboxie-plus/Sandboxie/issues/4688)
- 改进了 Sandboxie.ini 文件很大时响应速度变慢的问题 [#4573](https://github.com/sandboxie-plus/Sandboxie/issues/4573)
- 修复了 Msi 安装过程中无法在数据保护沙盒中创建 AppData\Romaing\Microsoft 文件夹的问题 [#4711](https://github.com/sandboxie-plus/Sandboxie/issues/4711)
- 修复了 Sandboxie 在时间加速相关的两个错误，并新增了两个时间函数的钩子 [#4721](https://github.com/sandboxie-plus/Sandboxie/pull/4721)（感谢 pwnmelife）
- 修复了协议选择的翻译字符串被保存到配置文件中的问题
- 修复了沙盒选择器图标
- 修复了切换 UI 语言时工具栏未更新的问题 [#4726](https://github.com/sandboxie-plus/Sandboxie/issues/4726)
- 修复了 PingInfoView 在 Sandboxie 中运行时报错的问题 [#4718](https://github.com/sandboxie-plus/Sandboxie/issues/4718)
- 修复了卸载注册表项未找到时，CollectProducts() 卡在循环中的问题 [#4753](https://github.com/sandboxie-plus/Sandboxie/issues/4753)
- 修复了与 SbieDll.dll 相关的崩溃问题 [#4754](https://github.com/sandboxie-plus/Sandboxie/issues/4754)
- 修复了在创建快捷方式时自动下载文件的问题 [#4663](https://github.com/sandboxie-plus/Sandboxie/issues/4663) [4750](https://github.com/sandboxie-plus/Sandboxie/pull/4750)（感谢 WZ-Tong）

### 移除
- 移除了无效的“删除内容”按钮 [#4720](https://github.com/sandboxie-plus/Sandboxie/pull/4720)（感谢 habatake）


## [1.15.11 / 5.70.11] - 2025-04-16

### 修复
- 修复了 SboxHostDll.dll 无法注入 OfficeClickToRun.exe 的问题


## [1.15.10 / 5.70.10] - 2025-04-15

### 新增
- 新增 'BindAdapterIP=192.168.100.123' 和/或 'BindAdapterIP=::ffff:c0a8:647b' 以强制沙盒程序使用指定的主机 IP
  - 注意：如果该 IP 未绑定到任一主机网络接口，连接将会失败
  - 配置可以按进程设置，例如 'BindAdapterIP=program.exe,192.168.100.123'，但仅限于 ini 文件，无法通过用户界面配置

### 更改
- 大幅改善对 Patreon 证书的处理。
- 改进了证书选项卡，序列号输入框现在总是可见，证书文本编辑框也是如此，尽管它不显示关键信息 UPDATE_KEY/SIGNATURE，而只在鼠标点击时显示完整数据
- 精简了新证书的应用流程

### 修复
- 修复了 Firefox 137.0 在 Sandboxie-Plus v1.15.9 中生成 SBIE2328 通知的问题 [#4652](https://github.com/sandboxie-plus/Sandboxie/issues/4638)[#4652](https://github.com/sandboxie-plus/Sandboxie/issues/4652)[#4640](https://github.com/sandboxie-plus/Sandboxie/issues/4640)
- 修复了 1.15.9 中缺少 SbieShellExt.dll 的问题
- 修复了 Process_GetCommandLine 导致内存泄漏的问题 [#4658](https://github.com/sandboxie-plus/Sandboxie/issues/4658)
- 修复了 Thunderbird 102.15.1 在 Sandboxie-Plus v1.15.9 下未加载所有扩展的问题 [#4653](https://github.com/sandboxie-plus/Sandboxie/issues/4653)
- 修复了在 Sandboxie 中，Invoke SetParent 将窗口更改为仅消息窗口失败的问题 [#4656](https://github.com/sandboxie-plus/Sandboxie/issues/4656)
- 修复了检索评估证书时处理 Unicode 名称的问题
- 修复了删除内容消息框是否应始终在最上方的问题 [#4673](https://github.com/sandboxie-plus/Sandboxie/issues/4673)


## [1.15.9 / 5.70.9] - 2025-03-??

### 新增
- 复制沙箱，并从 UI 中获取内容 [#4542](https://github.com/sandboxie-plus/Sandboxie/issues/4542)
- 添加新的不安全调试选项 'OpenAllSysCalls=y'
- 添加 WindowGrid 模板
- 添加新的证书类型 [DEVELOPER](https://xanasoft.com/product/sandboxie-plus-developer/)，它可以禁用用户模式组件验证，但只能以节点锁定的形式使用。
  - 注意：所有当前和未来的贡献者或 Eternal/Huge 证书持有者都有资格获得免费的开发密钥；如有需要，请通过电子邮件请求。

### 更改
- 当未为特定 IP 类型配置代理时，连接将失败
  - 此更改防止在同时运行双栈 IPv4 和 IPv6 时发生 IP 泄露，但仅配置了一种代理类型
- 保存时 ini 编辑器不再重置
- 现在可以在不重新加载驱动程序的情况下更改 DisableWinNtHook

### 修复
- 修复了框删除/移除确认对话框中与 bullet 字符相关的编码问题 [#4591](https://github.com/sandboxie-plus/Sandboxie/issues/4591)
- 更改箱的设置/类型会导致一些模板设置在箱中重复 [#4595](https://github.com/sandboxie-plus/Sandboxie/issues/4595)
- 在尝试将沙箱放置在分区根目录时添加警告 [#4582](https://github.com/sandboxie-plus/Sandboxie/issues/4582)
- 修复了卸载时的错误 [#4540](https://github.com/sandboxie-plus/Sandboxie/issues/4540)
- 修复了在 Sandboxie 中运行时 Golang Cmd.Run() 不返回的问题 [#4538](https://github.com/sandboxie-plus/Sandboxie/issues/4538)
  - 修复了可能帮助许多其他应用程序的通用句柄重复问题
- 改进了当 Sandboxie.ini 文件过大时，响应速度变差的问题 [#4573](https://github.com/sandboxie-plus/Sandboxie/issues/4573)
- 修复了在特定安装中的 Sandboxie 冻结/崩溃问题 [#4537](https://github.com/sandboxie-plus/Sandboxie/issues/4537)
  - 当 sandman 挂起并被终止时，下次启动时会询问是否禁用软件兼容性检查的更新扫描
- 修复了与钩子监视器显示有关的问题
- 修复了 ini 节区编辑的问题，eqc 不再关闭窗口
- 修复了在 Firefox 137 Nightly 及后续版本中引入的钩子问题 [#4522](https://github.com/sandboxie-plus/Sandboxie/issues/4522)
- 修复了不安全调试选项 'OriginalToken=y' 的问题
- 框组名称意外输入了换行符 [#4608](https://github.com/sandboxie-plus/Sandboxie/issues/4608)
- 修复了在新箱向导中从路径选择下拉菜单选择的路径返回“不是有效路径”错误的问题 [#4611](https://github.com/sandboxie-plus/Sandboxie/issues/4611)


## [1.15.8 / 5.70.8] - 2025-03-12

### 更改
- 更新了在线更新程序

### 修复
- 修复了 "WriteFilePath" 似乎工作不正确的问题 [#4553](https://github.com/sandboxie-plus/Sandboxie/issues/4553)
- 修复了沙盒备注，在保存时会删除空行的问题 [#4547](https://github.com/sandboxie-plus/Sandboxie/issues/4547)


## [1.15.7 / 5.70.7] - 2025-02-23

### 新增
- 服务现在也可以记录监视事件

### 更改
- 更新 Notepad++ 模板以在 Sandboxie Classic 中工作
- 更新 MiscHelpers.dll，改进查找器

### 修复
- 修复 Sandboxie Plus 安装程序中的过时条目
- 修复从“开始菜单运行”创建的固定快捷方式无法使用的问题 [#4502](https://github.com/sandboxie-plus/Sandboxie/issues/4502)
- 修复在资源低的情况下驱动程序潜在的蓝屏死机问题
- 修复 NsiAllocateAndGetTable 碰到崩溃的问题（感谢 tzf-omkey)[#4511](https://github.com/sandboxie-plus/Sandboxie/issues/4511)
- 修复当沙箱进程接收到设备更改通知时的内存泄漏
- 修复在隐私增强的沙箱中 NtQueryDirectoryFile 挂钩导致的内存泄漏问题 [#4509](https://github.com/sandboxie-plus/Sandboxie/issues/4509)


## [1.15.6 / 5.70.6] - 2025-02-03

### 新增
- 为 SBIE2224 添加了通用故障排除脚本：沙箱程序已崩溃
  - 为 Viber (Viber.exe) 提供了临时解决方案
- 添加调试选项 "DropChildProcessToken=program.exe,y"，用于调试绿色沙箱而不设置子进程令牌
- 添加缺失的钩子至 NtQueryInformationByName

### 更改
- 重新设计 CreateDesktop 处理方式，以修复浏览器及基于浏览器的应用程序的问题

### 修复
- 修复 Sandboxie Plus 数据文件夹在更新至版本 1.15.5 后未能正常工作的问题
- 修复故障排除引擎中持锁过长的问题
- 修复 Adobe Acrobat Reader 在应用程序隔离箱中崩溃的问题 [#4363](https://github.com/sandboxie-plus/Sandboxie/issues/4363)
- 修复 Viber 软件已更新，但在使用 Sandboxie 程序打开时，系统程序始终崩溃的问题 [#4298](https://github.com/sandboxie-plus/Sandboxie/issues/4298)
- 修复无法在 Sandboxie 中运行 qutebrowser 的问题 [#4419](https://github.com/sandboxie-plus/Sandboxie/issues/4419)
- 修复在版本 1.15.5 中启用 ApiTrace 时无法在沙箱中启动应用程序的问题 [#4488](https://github.com/sandboxie-plus/Sandboxie/issues/4488)
- 修复在 Windows 10 中使用 OpenWndStation=y 与调用 CreateDesktopW/A 的应用时出现的蓝屏死机问题



## [1.15.5 / 5.70.5] - 2025-01-27

### 新增
- 添加 'HookTrace=y' 以调试 API 钩取

### 更改
- 将 Sandboxie Plus 数据文件夹从 %LOCALAPPDATA%\\Sandboxie-Plus 移动到 %LOCALAPPDATA%\\Xanasoft\\Sandboxie-Plus
  - 注意：文件夹将自动移动。当降级至旧版本时，需要手动移动回去
- 重新排序沙箱上下文菜单选项 [#4444](https://github.com/sandboxie-plus/Sandboxie/issues/4444)

### 修复
- 修复以非高级视图按下以管理员身份运行时的崩溃问题
- 修复最新 Windows 11 ARM64 上所有应用程序的 WerFault.exe 错误 [#4422](https://github.com/sandboxie-plus/Sandboxie/issues/4422)
- 修复 ARM64 上阻止 x64 可执行文件运行的问题 [#4415](https://github.com/sandboxie-plus/Sandboxie/issues/4415)
- 修复进程自我终止的问题
- 修复 Windows 10 ARM64 上找不到 msedge_elf.dll 的问题
- 修复 Windows 10 ARM64 的兼容性问题 [#3600](https://github.com/sandboxie-plus/Sandboxie/issues/3600)
- 修复当 Gui_ConnectConsole 等待 SbieSvc.exe 退出时的无限循环问题 [#4462](https://github.com/sandboxie-plus/Sandboxie/pull/4462)（感谢 bot-1450)
- 修复在 RunCommand 设置中的命令路径包含空格时程序图标不显示的问题 [#4416](https://github.com/sandboxie-plus/Sandboxie/issues/4416)
- 修复模板扫描无法正常工作的问题 [#4401](https://github.com/sandboxie-plus/Sandboxie/issues/4401)
- 修复通过 mstsc.exe 连接到 RDP 服务器时加密沙箱中凭据对话框不显示的问题 [#4389](https://github.com/sandboxie-plus/Sandboxie/issues/4389)
- 修复文件夹恢复仅移动该文件夹内部文件的问题 [#4453](https://github.com/sandboxie-plus/Sandboxie/issues/4453)
- 修复查询文件属性失败导致程序异常的问题 [#4457](https://github.com/sandboxie-plus/Sandboxie/issues/4457)(感谢 lmou523)
- 修复 DNS 过滤中未定义域名的 IP 导致应用程序崩溃的问题 [#4475](https://github.com/sandboxie-plus/Sandboxie/issues/4475)
- 修复从“运行程序”菜单启动时文件未写入的问题 [#4398](https://github.com/sandboxie-plus/Sandboxie/issues/4398)


## [1.15.4 / 5.70.4] - 2024-12-19

### 新增
- 为 Joplin 添加模板 [#4402](https://github.com/sandboxie-plus/Sandboxie/pull/4402)（感谢 offhub)

### 修复
- 修复 INI 编辑器的暗模式
- 修复删除沙箱内容的问题 [#4407](https://github.com/sandboxie-plus/Sandboxie/pull/4407)（感谢 bot-1450)
- 修复从沙箱选择窗口中“无沙箱运行”不再工作的的问题 [#4403](https://github.com/sandboxie-plus/Sandboxie/issues/4403)
- 修复在启用 `ProtectHostImages=y` 的情况下，Firefox 标签页崩溃的问题 [#4394](https://github.com/sandboxie-plus/Sandboxie/issues/4394)
  - 仅考虑默认安装位置；在这些位置之外安装的基于 Firefox 的浏览器可能仍会崩溃
  - 为防止此问题，请手动为自定义安装路径在配置中添加 `DontCopy=<CustomInstallPath>`
- 修复当从 AlertFolder 打开 DLL 时使用 x64dbg 出现的蓝屏死机 "SYSTEM_SERVICE_EXCEPTION (3b)" 问题 [#4421](https://github.com/sandboxie-plus/Sandboxie/issues/4421)
- 修复当终止所有沙箱程序时出现的蓝屏死机 "CRITICAL_PROCESS_DIED" 问题 [#1316](https://github.com/sandboxie-plus/Sandboxie/issues/1316)
  - 注意：我们现在逐个终止被沙箱的进程，而不是使用作业对象，除非设置了 "TerminateJobObject=y"
- 修复 INI 编辑器在添加高亮功能后字体选择无法工作的的问题 [#4429](https://github.com/sandboxie-plus/Sandboxie/issues/4429)
- 修复与 'LogMessageEvents=y' 相关的蓝屏死机问题



## [1.15.3 / 5.70.3] - 2024-11-28

### 新增
- 改进 INI 编辑器，现在支持搜索（快捷键：Ctrl+F)
- 添加 SBIE1321 以记录所有强制进程事件，可以通过 "NotifyForceProcessEnabled=y" 启用，改进 [#4113](https://github.com/sandboxie-plus/Sandboxie/issues/4113)
- 添加 Sandboxie Plus UI 的自定义字体配置 [#4397](https://github.com/sandboxie-plus/Sandboxie/pull/4397)（感谢 habatake)

### 更改
- 改进支持通知
- 改进卸载 Sandboxie Plus 残余文件的问题 [#4374](https://github.com/sandboxie-plus/Sandboxie/pull/4374)

### 修复
- 修复 ARM64 平台上的 SSL 问题，导致更新程序和证书检索失败



## [1.15.2 / 5.70.2] - 2024-11-18

### 新增
- 添加 "NetworkAdapterMAC=0,AA-BB-CC-DD-EE-FF" 以设置每个沙箱的 MAC 地址（感谢 Yeyixiao)
- 添加 "DiskSerialNumber=DeviceName,1234-ABCD" 以设置单个沙箱的磁盘序列号（感谢 Yeyixiao)
- 添加在全局设置的编辑框中隐藏证书的功能（灵感来自 Yeyixiao)
- 添加同时在多个沙箱中打开程序的功能 [#4231](https://github.com/sandboxie-plus/Sandboxie/issues/4231)
- 在沙箱设置中添加 "描述（Description）" 字段 [#4243](https://github.com/sandboxie-plus/Sandboxie/issues/4243)
- 添加 "NotifyMsiInstaller=y"，默认启用以在沙箱中运行没有推荐排除项的 MSI 安装程序时显示消息 SBIE2194 [#4330](https://github.com/sandboxie-plus/Sandboxie/issues/4330)
  - SBIE2194： MSI 安装程序需要在 INI 中设置 'MsiInstallerExemptions=y' 选项才能正确工作，然而此选项会削弱隔离。
- 添加隐藏已安装程序的选项 [#4139](https://github.com/sandboxie-plus/Sandboxie/issues/4139)
- 添加隐藏托盘图标的选项 [#4075](https://github.com/sandboxie-plus/Sandboxie/issues/4075)
- 添加改进的跟踪日志过滤功能 [#4338](https://github.com/sandboxie-plus/Sandboxie/issues/4338)
- 添加 SbieMessages 的事件日志监控 [#4113](https://github.com/sandboxie-plus/Sandboxie/issues/4113)
  - 将 'LogMessageEvents=y' 添加到全局设置，以将所有 Sbie 事件记录到系统事件日志中

### 更改
- 验证与 Windows build 27749 的兼容性并更新 DynData
- 通过拖放运行时，现将应用程序的父文件夹用作工作目录 [#4073](https://github.com/sandboxie-plus/Sandboxie/issues/4073)
- 更新至 Qt 5.15.16 和 OpenSSL 3.4.0 [#4370](https://github.com/sandboxie-plus/Sandboxie/pull/4370)（感谢 offhub)

### 修复
- 修复 Sandboxie Plus 安装或更新期间 .tmp 文件签名的问题 [#2643](https://github.com/sandboxie-plus/Sandboxie/issues/2643) [#4343](https://github.com/sandboxie-plus/Sandboxie/issues/4343)
- 修复 DLL 卸载问题
- 修复 "Files Resource Access" 授予对排除文件夹的访问问题 [#4007](https://github.com/sandboxie-plus/Sandboxie/issues/4007)
- 修复当设置 "ForceDisableAdminOnly=y" 时，“无沙箱运行”从运行菜单中消失的问题 [#4233](https://github.com/sandboxie-plus/Sandboxie/issues/4233)
- 修复重命名文件或文件夹时在空操作条件下的死锁问题 [#4304](https://github.com/sandboxie-plus/Sandboxie/issues/4304)
- 修复移动沙箱文件或文件夹的问题 [#4329](https://github.com/sandboxie-plus/Sandboxie/issues/4329)
- 修复 Sandboxie 导致 Firefox Nightly 崩溃的问题 [#4183](https://github.com/sandboxie-plus/Sandboxie/issues/4183)
- “从快速预览中沙箱运行”应只有一个选项 [#4339](https://github.com/sandboxie-plus/Sandboxie/issues/4339)

## [1.15.1 / 5.70.1] - 2024-10-29

### 更改
- 验证了与 Windows 27729 构建的兼容性并更新了 DynData
- 更新了 Templates.ini，以授权访问多媒体类调度服务 [#4312](https://github.com/sandboxie-plus/Sandboxie/pull/4312)（感谢 offhub)
- 更新至 Inno Setup 6.3.3 [#4020](https://github.com/sandboxie-plus/Sandboxie/issues/4020)

### 修复
- 修复了 Sandboxie 加密在红框中无法启动的问题
- 修复了使用 explorer.exe 时与突破进程相关的问题

## [1.15.0 / 5.70.0] - 2024-10-19

### 新增
- 新增用户代理机制以支持特定用户的操作
- 新增对 EFS 的支持，使用用户代理 [#1980](https://github.com/sandboxie-plus/Sandboxie/issues/1980)
  - 启用此功能，请在沙箱配置中添加 'EnableEFS=y'（需要高级支持者证书)
- 新增突破文档功能 [#2741](https://github.com/sandboxie-plus/Sandboxie/issues/2741)
  - 使用类似 'BreakoutDocument=C:\path\*.txt' 的语法来指定路径和扩展名
  - 安全警告：不要使用以通配符结束的路径，如 'BreakoutDocument=C:\path\*'，因为这样会允许在沙箱外执行恶意脚本！
- 新增将箱文件夹 ACL 设置为仅允许创建用户访问的机制 'LockBoxToUser=y'
- 新增在沙箱文件上保持原始 ACL 的选项 'UseOriginalACLs=y'
- 新增选项 'OpenWPADEndpoint=y' [#4292](https://github.com/sandboxie-plus/Sandboxie/issues/4292)

### 更改
- 改善了 SandboxieCrypto 启动性能
- 改善了沙箱中 RPCSS 的启动性能
- 更新至 Qt 5.15.15 和 OpenSSL 3.3.2 [#4223](https://github.com/sandboxie-plus/Sandboxie/pull/4223)（感谢 offhub)
- 设置了 UI 控件的标签顺序和伙伴 [#4300](https://github.com/sandboxie-plus/Sandboxie/pull/4300)（感谢 gexgd0419)

### 修复
- 修复了 ImDiskApp 卸载键总是写入注册表的问题 [#4282](https://github.com/sandboxie-plus/Sandboxie/issues/4282)
- 通过添加新的 ACL 处理修复了安全问题 ID-24 [CVE-2024-49360](https://github.com/sandboxie-plus/Sandboxie/security/advisories/GHSA-4chj-3c28-gvmp)
  - 注意：在多用户系统上设置 'LockBoxToUser=y'

## [1.14.10 / 5.69.10] - 2024-10-03

### 新增
- 新增直接导入加密归档文件的功能 [#4255](https://github.com/sandboxie-plus/Sandboxie/issues/4255)

### 更改
- 当 SbieSvc.exe 工作进程崩溃时，现在可以自动重启

### 修复
- 修复了沙箱路径条目组合框的问题
- 修复了 GetRawInputDeviceInfoW() 代理引起的缓冲区溢出问题 [#4267](https://github.com/sandboxie-plus/Sandboxie/issues/4267)（感谢 marti4d)

## [1.14.9 / 5.69.9] - 2024-09-19

### 新增
- 新增备用默认沙箱路径至箱向导：
  - \\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%
  - \\??\\%SystemDrive%\\Sandbox\\%SANDBOX%
  - \\??\\%SystemDrive%\\Users\\%USER%\\Sandbox\\%SANDBOX%
- 新增沙箱导入对话框

### 更改
- 全局设置中的沙箱根选择现在为组合框

### 修复
- 修复了导出的加密归档文件无法被 Sandboxie 解压的问题 [#4229](https://github.com/sandboxie-plus/Sandboxie/issues/4229)

## [1.14.8 / 5.69.8] - 2024-09-09

### 更改
- 允许用户使用 .zip 文件导入/导出箱 [#4200](https://github.com/sandboxie-plus/Sandboxie/pull/4200)

### 修复
- 修复了在 1.14.7 中引入的支持者证书问题

## [1.14.7 / 5.69.7] - 2024-09-05

### 新增
- 新增 "RandomRegUID" (bool)，将 Windows 产品 ID 设置为随机值
- 新增 "HideDiskSerialNumber" (bool)，在应用程序尝试获取磁盘序列号时返回随机值
- 新增在支持设置页面获取免费的 10 天评估证书的选项
  - 评估证书与 HwID 绑定，每个 HwID 最多可请求 3 个证书
- 新增 "TerminateWhenExit" (bool，在 Sandboxie-Plus.ini 中) 在 SandMan 退出时终止所有进程 [#4171](https://github.com/sandboxie-plus/Sandboxie/issues/4171)
- 新增询问箱导入位置的问答框 [#4169](https://github.com/sandboxie-plus/Sandboxie/issues/4169)
- 新增用于配置 DropConHostIntegrity 的 UI 选项
- 新增 "HideNetworkAdapterMAC" (bool)，在应用程序尝试获取网络适配器 MAC 地址时返回随机值
- 新增共享模板选择到新箱向导的高级选项中的共享模板功能 [#4199](https://github.com/sandboxie-plus/Sandboxie/issues/4199)
  - 可用的共享模板数量增加到 10 个
  - 若要更新列表中显示的名称，只需调整每个模板中的 "Tmpl.Title" 设置

### 修复
- 修复和改善 HideDiskSerialNumber 选项导致应用程序崩溃的问题 [#4185](https://github.com/sandboxie-plus/Sandboxie/issues/4185)
- 修复了加密代理密码格式不正确的问题 [#4197](https://github.com/sandboxie-plus/Sandboxie/issues/4197)
- 修复了 NtQueryDirectoryObject（不应返回 "STATUS_MORE_ENTRIES")，因为这是一种简单的沙箱检测 [#4201](https://github.com/sandboxie-plus/Sandboxie/issues/4201)

## [1.14.6 / 5.69.6] - 2024-07-30

### 新增
- 新增沙箱别名 [#4112](https://github.com/sandboxie-plus/Sandboxie/issues/4112)

### 修复
- 修复 Windows 7 下的 CustomLCID 问题 [#4117](https://github.com/sandboxie-plus/Sandboxie/issues/4117)
- 修复了 1.14.0 中与非高级证书相关的设置窗口问题
- 修复了 API_PROCESS_EXEMPTION_CONTROL 的问题

## [1.14.5 / 5.69.5] - 2024-07-23

### 新增
- 新增 HwID 显示
- 新增语言欺骗 "CustomLCID=1033" [#4024](https://github.com/sandboxie-plus/Sandboxie/pull/4024)（感谢 Yeyixiao)
- 新增选项以始终以管理员身份运行 SandMan UI [#4090](https://github.com/sandboxie-plus/Sandboxie/issues/4090)
- 新增代理排除功能 [#4036](https://github.com/sandboxie-plus/Sandboxie/issues/4036)
- 新增 "ForceChildren=Program.exe" [#4070](https://github.com/sandboxie-plus/Sandboxie/issues/4070)
- 为 "ForceRestartAll" 和 "UseCreateToken" 在选项窗口中新增 UI 选项
- 新增一个可选的上下文菜单选项，以快速强制文件夹/文件
  - 注意：您也可以使用 "SandMan.exe /add_force program_path" 来做到这一点

### 更改
- 证书格式现在可以接受显式有效期天数的指定，以便无缝证书续订

### 修复
- 修复了打开沙箱设置时多次出现支持者证书弹窗的问题 [#4074](https://github.com/sandboxie-plus/Sandboxie/issues/4074)
- 修复了 HwID 绑定序列密钥失败的问题，当没有获得 HwID 时
- 修复了 "UseChangeSpeed=y" 的问题
- 修复了 "HideFirmwareInfo=y" 实现的错误
  - 更改注册路径至键 "HKCU\\System\\SbieCustom"，值："SMBiosTable"
  - 新增 UI 选项
- 修复了加密沙箱中的 schannel 错误 SEC_E_SECPKG_NOT_FOUND [#4081](https://github.com/sandboxie-plus/Sandboxie/issues/4081)
- 修复了 "沙箱名称过长" 的错误，导致 Sbie2327 中出现异常 [#4064](https://github.com/sandboxie-plus/Sandboxie/issues/4064)
- 修复了作业对象的内存限制，现在可以超过 4GB [#4096](https://github.com/sandboxie-plus/Sandboxie/issues/4096)

## [1.14.4 / 5.69.4] - 2024-07-13

### 更改
- 改善了残留物的删除 [#4050](https://github.com/sandboxie-plus/Sandboxie/pull/4050)

### 修复
- 修复 "开始限制" 选项卡布局破损的问题 [#4045](https://github.com/sandboxie-plus/Sandboxie/issues/4045)
- 修复了管理员无法更改沙箱配置的问题 [#4057](https://github.com/sandboxie-plus/Sandboxie/issues/4057) [#4068](https://github.com/sandboxie-plus/Sandboxie/issues/4068)

## [1.14.3 / 5.69.3] - 2024-07-01

### 更改
- 更新至 Qt 5.15.14 和 OpenSSL 3.3.1 [#3994](https://github.com/sandboxie-plus/Sandboxie/pull/3994)（感谢 offhub)

### 修复
- 修复了使用 MSFT 账户时，沙箱中无法以管理员身份启动应用程序的问题 [#4022](https://github.com/sandboxie-plus/Sandboxie/issues/4022)
- 修复了 Sbie 1.14.1 和 1.14.2 中的 Firefox 问题 [#4012](https://github.com/sandboxie-plus/Sandboxie/issues/4012)
  - 回滚了在 1.14.1 中添加的驱动程序验证器修复
- 修复了 CustomChromiumFlags 和 --single-argument 的问题 [#4033](https://github.com/sandboxie-plus/Sandboxie/issues/4033)
- 修复了在启用 UseCreateToken/SandboxieAllGroup 时，Sandboxie 程序在关闭以管理员身份运行的程序后未终止的问题 [#4030](https://github.com/sandboxie-plus/Sandboxie/issues/4030)

## [1.14.2 / 5.69.2] - 2024-06-19

### 新增
- 新增 SbieIni 选项以修改受密码保护的配置 [#3903](https://github.com/sandboxie-plus/Sandboxie/issues/3903)
  - 用法：set|append|insert|delete [/passwd:********] <section> <setting> <value>
  - 注意：使用 /passwd 而不带密码将使 SbieIni 在控制台提示输入密码，这样可以隐藏密码以防被捕获
- 新增 "PromptForInternetAccess" 选项的复选框至新箱向导
- 新增选项 "HideNonSystemProcesses"，以隐藏沙箱中不属于沙箱的进程
- 新增选项 "HideSbieProcesses"，以隐藏 Sandboxie 工作进程（SbieSvc、SandboxieRpcSs 等)
- 新增选项 "HideFirmwareInfo"
  - 当设置后，尝试获取固件信息的程序将从 HKEY_CURRENT_USER\\SOFTWARE\\SandboxieHide\\FalseFirmwareValue 中获取虚假数据
- 新增模板 "BlockAccessWMI"，以防止沙箱进程通过 WMI 访问系统信息
- 新增模板 "BlockLocalConnect"，以防止沙箱进程向本地主机发送网络数据包以突破沙箱
- 新增新选项 "AllowCoverTaskbar" [#3975](https://github.com/sandboxie-plus/Sandboxie/issues/3975)
- 新增 RPC 端口消息过滤机制，以阻止驱动程序通过 RDP 进行不安全的调用 [#3930](https://github.com/sandboxie-plus/Sandboxie/issues/3930)
  - 用法："RpcPortFilter=Port,ID,Label"，标签是可选的
- 新增 "Job Object" 选项页面，以收集与作业对象相关的所有选项

### 更改
- 扩展 "Temp Template" 以允许删除本地模板部分

### 修复
- 修复了新引入的实验性 "UseCreateToken=y" 机制的安全问题
- 修复了在使用 MSFT 在线账户时 "UseCreateToken=y" 的问题
- 修复了导出沙箱功能未包含隐藏文件的问题 [#3980](https://github.com/sandboxie-plus/Sandboxie/issues/3980)（感谢 L4cache)
- 修复了 Chrome 打印机停止工作的问题 [#3926](https://github.com/sandboxie-plus/Sandboxie/issues/3926)
  - Sandboxie 将在基于 Chromium 的浏览器命令行中添加 CustomChromiumFlags=--disable-features=PrintCompositorLPAC
  - 注意：当前 Less Privileged App Container (LPAC) 与 Sandboxie 不兼容
- 修复了访问相对符号链接时问题，目标以点开头 [#3981](https://github.com/sandboxie-plus/Sandboxie/issues/3981)
- 修复了通过双击系统托盘上下文窗口打开沙箱属性窗口的问题 [#3861](https://github.com/sandboxie-plus/Sandboxie/issues/3861)
- 修复了自版本 1.12.9 后强制程序的启动延迟问题 [#3868](https://github.com/sandboxie-plus/Sandboxie/issues/3868)
  - 此问题是在 1.13.0 中引入的，可能影响了其他用例导致各种问题
- 修复了杂项选项列表的问题
- 改善了与沙箱运行的 Steam 的兼容性
- 修复了与 FakeAdminRights 的兼容性问题 [#3989](https://github.com/sandboxie-plus/Sandboxie/pull/3989)（感谢 offhub)

## [1.14.1 / 5.69.1] - 2024-06-06

### 新增
- 新增将 "Sandboxie\All Sandboxes" SID 添加至使用 SandboxieLogon 的令牌中 [#3191](https://github.com/sandboxie-plus/Sandboxie/issues/3191)
  - 要使用此功能，必须启用 "SandboxieAllGroup=y"
  - 注意： 这从根本上改变了 Sbie 用于令牌创建的机制，新的机制可以通过 "UseCreateToken=y" 单独启用
- 新增 "EditAdminOnly=y" 现在可以按箱配置
- 新增封闭箱窗口的 UI 选项至新箱向导中
- 在选择箱窗口中新增 UI 选项以启动未被沙箱化的进程，但强制子进程沙箱化
- 新增选项 "AlertBeforeStart"
  - 当设置时，在使用 "Start.exe" 启动新程序进入沙箱时，会弹出提示，并检查启动 "Start.exe" 的程序是否为 Sandboxie 组件，如果不是，则弹出警告
- 新增 SetupWizard 中的 "EditAdminOnly" 选项。

### 更改
- 将高级新箱向导拆分为两个页面以便扩展
- 重新组织了箱选项。

### 修复
- 修复了代理身份验证设置的问题
- 修复了 SbieSvc 中的内存泄漏
- 修复了 WFP 选项应用不一致的问题 [#3900](https://github.com/sandboxie-plus/Sandboxie/issues/3900)
- 修复了缓冲区哈希函数中的资源泄漏
- 修复了启用 BlockInterferenceControl 时 DLL 名称损坏的问题 [#3945](https://github.com/sandboxie-plus/Sandboxie/issues/3945)
- 修复了驱动程序验证器的问题

## [1.14.0 / 5.69.0] - 2024-05-17

### 新增
- 新增限制沙箱化进程内存及单个沙箱中的进程数量的选项，通过作业对象（感谢 Yeyixiao)
  - 使用 "TotalMemoryLimit"（字节数)设置沙箱的总体内存限制，使用 "ProcessMemoryLimit"（字节数)限制单个进程的内存
  - 使用 "ProcessNumberLimit"（数量)设置进程数量限制
- 新增调整沙箱内进程逻辑速度的能力，包括减少固定延迟和修改单人游戏速度（感谢 Yeyixiao)
  - 注意：您可以设置 "UseChangeSpeed=y" 来配置以下选项："AddTickSpeed"、"AddSleepSpeed"、"AddTimerSpeed"、"LowTickSpeed"、"LowSleepSpeed" 和 "LowTimerSpeed"（仅整数值)
  - 注意：这些选项使用乘法而不是加减；"Add" 系列由乘法配置，而 "Low" 系列由除法配置
  - 注意：当设置 "AddSleepSpeed=0" 时，所有睡眠函数调用将被跳过。例如，您可以绕过潜藏恶意软件中的固定延迟代码，从而减少分析时间而不影响基本操作，这对病毒分析师非常有用
- 新增 /fcp /force_children 命令行选项至 Start.exe；这允许启动一个未被沙箱化的程序，但使其所有子进程都被沙箱化
- 新增强制沙箱化进程使用预定义的 SOCKS5 代理的能力
- 新增拦截 DNS 查询的能力，以便可以记录和/或重定向
- 新增支持基于 RFC1928 的 SOCKS5 代理身份验证（感谢 Deezzir)
- 新增 SOCKS5 代理的测试对话框 UI（感谢 Deezzir)
- 新增自动移除以 "Template_Temp_" 开头的模板引用的能力

### 更改
- 验证了与 Windows 26217 构建的兼容性并更新了 DynData

### 修复
- 修复了早期批量的 Large Supporter 证书的问题

## [1.13.7 / 5.68.7] - 2024-05-01

### 新增
- 在 Sandboxie Plus 关于对话框中增加 SbieDll.dll 和 SbieSvc.exe 的文件版本信息

### 修改
- 改进 SandMan 中关于 DropAdminRights 的复选框 [#3851](https://github.com/sandboxie-plus/Sandboxie/pull/3851) （感谢 offhub)

### 修复
- 修复了文件的符号链接 [#3852](https://github.com/sandboxie-plus/Sandboxie/issues/3852)
- 修复了启动代理选项的问题 [#3844](https://github.com/sandboxie-plus/Sandboxie/pull/3844)（感谢 offhub)
- 修复了在 1.13.5 中引入的 Delete V2 问题



## [1.13.6 / 5.68.6] - 2024-04-21

### 新增
- 增加了 "BlockInterferenceControl=y" 选项，以防止沙箱化进程强制窗口位于最上层并移动鼠标指针（感谢 Yeyixiao)
  - 注意：此选项可能会在游戏中引起问题，因此不推荐用于游戏沙箱
- 增加对硬链接的支持 [#3826](https://github.com/sandboxie-plus/Sandboxie/issues/3826)
- 增加从驱动程序终止卡住的沙箱化进程的机制
- 增加可编辑的触发器列表 [#3742](https://github.com/sandboxie-plus/Sandboxie/issues/3742)
- 增加了可选的屏幕截图保护扩展到用户界面 [#3739](https://github.com/sandboxie-plus/Sandboxie/issues/3739)
- 增加了一个按钮来编辑本地/自定义模板 [#3738](https://github.com/sandboxie-plus/Sandboxie/issues/3738)
- 增加 "Run Sandboxed" 窗口的可调整大小 [#3697](https://github.com/sandboxie-plus/Sandboxie/issues/3697)
- 增加 Notepad++ 模板 [#3836](https://github.com/sandboxie-plus/Sandboxie/pull/3836)（感谢 offhub)

### 修改
- 改进了 Avast 模板 [#3824](https://github.com/sandboxie-plus/Sandboxie/pull/3824)（感谢 offhub)
- 重命名了一些实验性选项，并在用户界面中将其标记为实验性
  - "IsBlockCapture=y" -> "BlockScreenCapture=y"
  - "IsProtectScreen=>" -> "CoverBoxedWindows=y"

### 修复
- 修复了更改 BlockDNS 或 BlockPorts 选项时无效的应用按钮 [#3807](https://github.com/sandboxie-plus/Sandboxie/issues/3807)
- 修复了新 Qt 导致的故障排除向导中断问题 [#3810](https://github.com/sandboxie-plus/Sandboxie/discussions/3810)
- 修复了设置对话框未显示正确的 RAM 驱动器字母
- 修复由于最新 Qt 构建缺少 SSL 支持而导致的更新程序故障 [#3810](https://github.com/sandboxie-plus/Sandboxie/discussions/3810)
- 修复启用 "DropAdminRights/FakeAdminRights" 后将 "BlockInterferePower 和 ForceProtectionOnMount" 添加到 INI 的问题 [#3825](https://github.com/sandboxie-plus/Sandboxie/issues/3825)
- 修复 KeePass 由于 "BlockScreenCapture=y" 导致的 "Out of Memory" 崩溃问题 [#3768](https://github.com/sandboxie-plus/Sandboxie/issues/3768)
- 修复在 Windows 7 中使用 IsBlockCapture=y 的 Sandboxie 1.13.4 无法正常工作的问题 [#3769](https://github.com/sandboxie-plus/Sandboxie/issues/3769)
- 修复 explorer.exe 中的 "FakeAdminRights=y" 问题 [#3638](https://github.com/sandboxie-plus/Sandboxie/issues/3638)
- 修复强制文件夹警告通知 [#3569](https://github.com/sandboxie-plus/Sandboxie/issues/3569)



## [1.13.5 / 5.68.5] - 2024-04-10

### 新增
- 增加了为每个沙箱将所有进程设置到某个核心的能力 [#3276](https://github.com/sandboxie-plus/Sandboxie/issues/3276)
  - 在 Sandboxie.ini 中设置 "CpuAffinityMask=0x00000001"，其中 0x00000001 是指示要使用核心的位掩码
  - 仅支持 0-31 核心，对 32 及以上的核心在使用此选项时将始终禁用
- 增加了 Samba 和 DNS 端口阻止的复选框
- 增加 Weasel 模板 [#3806](https://github.com/sandboxie-plus/Sandboxie/pull/3806)（感谢 xWTF)

### 修改
- 在测试签名模式下，Sandboxie 将默认尝试过时的偏移量
- 更新为 Qt 5.15.13，并应用最新的安全补丁 [#3694](https://github.com/sandboxie-plus/Sandboxie/pull/3694)（感谢 LumitoLuma)
- 将网络限制从一般限制移动到网络页面上的单独标签
- 改进证书检索用户界面消息
- 改进 MPC-BE 模板 [#3798](https://github.com/sandboxie-plus/Sandboxie/pull/3798)（感谢 offhub)

### 修复
- 修复了虚拟化方案版本 2 导致极慢的文件删除速度 [#3650](https://github.com/sandboxie-plus/Sandboxie/issues/3650)

### 删除
- 删除了 Templates.ini 中过时的推荐 [#3802](https://github.com/sandboxie-plus/Sandboxie/pull/3802)



## [1.13.4 / 5.68.4] - 2024-03-25

### 新增
- 增加防止沙箱化进程访问沙箱外窗口图像的选项 [#1985](https://github.com/sandboxie-plus/Sandboxie/issues/1985)（感谢 Yeyixiao)
  - 可以通过 "IsBlockCapture=y" 启用
  - 请参见 SandMan 用户界面中的沙箱选项 "Prevent sandboxed processes from using public methods to capture window images"
- 增加 "LingerExemptWnds=n" 以使滞留进程监视机制不再豁免带窗口的滞留进程
- 增加选项 'SharedTemplate' 到 Box Wizard [#3737](https://github.com/sandboxie-plus/Sandboxie/pull/3737)（感谢 offhub)
- 增加强制启用加密沙箱保护的选项 [#3736](https://github.com/sandboxie-plus/Sandboxie/pull/3736)（感谢 Yeyixiao)
- 增加一个菜单和按钮/图标来挂起所有进程 [#3741] (https://github.com/sandboxie-plus/Sandboxie/issues/3741)

### 修改
- 选项 "LingerLeniency=n" 现在也禁用新启动的滞留进程的 5 秒宽限期 [#1892](https://github.com/sandboxie-plus/Sandboxie/issues/1892)

### 修复
- 修复与开始菜单文件夹相关的符号链接问题



## [1.13.3 / 5.68.3] - 2024-03-16

### 新增
- 增加证书使用指南链接到支持页面

### 修复
- 修复 "IsProtectScreen=y" 的问题 [3656](https://github.com/sandboxie-plus/Sandboxie/pull/3656#discussion_r1518549704)
- 修复热键问题并将默认的 'suspend all' 热键更改为 Shift+Alt+Pause
- 修复当使用全局热键时挂起状态未更新的问题
- 修复在某些情况下新 ** 模式失败的问题



## [1.13.2 / 5.68.2] - 2024-03-07

### 新增
- 增加菜单项以管理员身份重新启动 SandMan [#3581](https://github.com/sandboxie-plus/Sandboxie/issues/3581)（感谢 Yeyixiao)
- 增加阻止沙箱化进程截图的选项（感谢 Yeyixiao)
  - 可以通过 "IsProtectScreen=y" 启用
  - 请参见 SandMan 用户界面中的沙箱选项 "Prevent processes from capturing window images from sandboxed windows"
- 增加防止沙箱化进程干预电源操作的选项 [#3640](https://github.com/sandboxie-plus/Sandboxie/issues/3640)（感谢 Yeyixiao)
  - 可以通过 "BlockInterferePower=y" 启用
  - 请参见 SandMan 用户界面中的沙箱选项 "Prevent sandboxed processes from interfering with power operations"
- 增加新的模式机制，使用 `**` 作为占位符表示不包含 `\` 的任意路径元素 [1ff2867](https://github.com/sandboxie-plus/Sandboxie/commit/1ff28679b0557a642d29f1651a90a8f7fec77171)

### 修改
- 重新设计了 SandMan 中挂起所有进程的选项（在 1.13.1 中引入) [#3582](https://github.com/sandboxie-plus/Sandboxie/issues/3582#issuecomment-1969628215)

### 修复
- 修复隐私模式、NormalFilePath 和符号链接问题 [#3660](https://github.com/sandboxie-plus/Sandboxie/issues/3660)
- 修复在应用空间沙箱中访问回收站的问题 [#3665](https://github.com/sandboxie-plus/Sandboxie/issues/3665)



## [1.13.1 / 5.68.1] - 2024-02-27

### 新增
- 增加在 SandMan 中挂起所有进程的选项 [#3582](https://github.com/sandboxie-plus/Sandboxie/issues/3582)（感谢 Yeyixiao)
- 增加 "On Terminate" 触发器 [#3584](https://github.com/sandboxie-plus/Sandboxie/issues/3584)（感谢 Yeyixiao)

### 修改
- 更改了 DynData 格式以添加标志
- 恢复了没有驱动器字母的卷的新沙箱目录结构 [#3632](https://github.com/sandboxie-plus/Sandboxie/issues/3632)
  - GUID 使用可以通过 "UseVolumeGuidWhenNoLetter=y" 重新启用

### 修复
- 增加缺失的 API 跟踪复选框
- 修复与 Windows ARM64 Insider 构建 26052 及更高版本的兼容性问题
- 修复符号链接问题 [#3537](https://github.com/sandboxie-plus/Sandboxie/issues/3537#issuecomment-1954496527)
- 修复在应用空间沙箱中文件重定向问题 [#3637](https://github.com/sandboxie-plus/Sandboxie/issues/3637)
- 修复与隔离模式兼容性回退的问题
- 修复缺失的最大密码长度检查 [#3639](https://github.com/sandboxie-plus/Sandboxie/issues/3639)
- 修复在没有驱动器字母的卷中从沙箱启动可执行文件的问题，兼容 Windows 1803 及更早版本 [#3627](https://github.com/sandboxie-plus/Sandboxie/issues/3627)

### 删除
- 删除 UseNewSymlinkResolver 设置，因为新的机制始终使用



## [1.13.0 / 5.68.0] - 2024-02-10

### 新增
- 增加高级 API 跟踪功能

### 修改
- 重新设计 SCM Hooking 以改善 Windows 10 的兼容性
- 重新设计与未文档化的 Windows 内核对象的偏移量处理
  - 现在可以独立于驱动程序更新所需的偏移量
  - DynData blob 经过数字签名，当在测试签名模式下时，签名将被忽略
  - 当 Sandboxie 遇到尚不支持的内核构建时，将禁用基于令牌的隔离以防止系统不稳定
  - 此安全机制在参与 Windows Insider 程序的系统上禁用
  - 对于 Insider 程序中的系统，将尝试最新已知的偏移量
- 重新设计低级代码注入机制以增加对 Windows Insider 构建 26040 及更高版本的兼容性
- 为核心 Sandboxie 二进制文件启用 CET Shadow Stack 兼容标志

### 修复
- 修复与 Windows Insider 构建 26040 及更高版本的兼容性问题

### 删除
- 清理代码，删除过时的 VC 6.0 解决方案



## [1.12.9 / 5.67.9] - 2024-02-06

### 修复
- 修复了上一个版本中引入的符号链接解析器问题 [#3481](https://github.com/sandboxie-plus/Sandboxie/issues/3481)



## [1.12.8 / 5.67.8] - 2024-01-31

### 新增
- 在 Plus 用户界面中增加日语语言支持 [#3573](https://github.com/sandboxie-plus/Sandboxie/pull/3573)

### 修复
- 修复警告问题：`SBIE2321 Cannot manage device map: C0000034 / 11` [#2996](https://github.com/sandboxie-plus/Sandboxie/issues/2996)
- 修复在模拟管理员沙箱中无法运行 explorer.exe 的问题 [#3516](https://github.com/sandboxie-plus/Sandboxie/issues/3516)
- 修复启用追踪日志时与 WFP 相关的潜在 BSoD 问题 [#2471](https://github.com/sandboxie-plus/Sandboxie/issues/2471)
- 修复在运行菜单中固定程序未使用相同工作目录的问题 [#3555](https://github.com/sandboxie-plus/Sandboxie/issues/3555)
- 修复 UseNewSymlinkResolver 导致应用程序同时创建链接和目标文件夹的问题 [#3481](https://github.com/sandboxie-plus/Sandboxie/issues/3481)
- 修复 Plus 用户界面通知窗口在显示模态对话框时卡住的问题 [#3586](https://github.com/sandboxie-plus/Sandboxie/issues/3586)



## [1.12.7 / 5.67.7] - 2024-01-10

### 修复
- 修复 "Duplicate Box Config" 选项无法保留原沙箱配置顺序的问题 [#3523](https://github.com/sandboxie-plus/Sandboxie/issues/3523)
- 修复在启用 "Configure Advanced Options" 时 "Save options as new defaults" 选项未出现的问题 [#3522](https://github.com/sandboxie-plus/Sandboxie/issues/3522)
- 修复在启用数据保护的 Windows 11 沙箱中运行 Windows 资源管理器的问题 [#3517](https://github.com/sandboxie-plus/Sandboxie/issues/3517)
  - 隐私箱的默认模板现在设置为 `NormalFilePath=%ProgramData%\Microsoft\*`
- 修复在安全加固沙箱中，使用 MB_DEFAULT_DESKTOP_ONLY 或 MB_SERVICE_NOTIFICATION 的消息框无法正确显示标题和文本的问题 [#3529](https://github.com/sandboxie-plus/Sandboxie/issues/3529)
- 修复在安全加固沙箱中 npm 无法运行的问题 [#3505](https://github.com/sandboxie-plus/Sandboxie/issues/3505)
- 修复在 1.12.6 中引入的符号链接和打开路径问题 [#3537](https://github.com/sandboxie-plus/Sandboxie/issues/3537)
  - 当遇到没有关联驱动器字母的卷时，Sandboxie 使用 `\drive\{guid}` 代替 `\drive\[letter]`
  - 注意：如果随后该卷被分配了驱动器字母，在 `\drive\{guid}` 下的数据将被忽略！
- 修复在右键单击使用 Sandboxie Plus 创建的快捷方式时 "Run Unsandboxed" 命令的问题 [#3528](https://github.com/sandboxie-plus/Sandboxie/issues/3528)
  - 注意：为了使修复生效，需重新应用 Shell 集成
- 修复 `Error Status: 0x0000065b (Function failed during execution)` [#3504](https://github.com/sandboxie-plus/Sandboxie/issues/3504)
- 修复隐私增强沙箱因错误代码 SBIE2204 失败的问题 [#3542](https://github.com/sandboxie-plus/Sandboxie/issues/3542)
- 修复 OpenFilePath 目录未在其父文件夹中列举的问题 [#3519](https://github.com/sandboxie-plus/Sandboxie/issues/3519)



## [1.12.6 / 5.67.6] - 2024-01-02

### 修改
- 改进了工具栏自定义菜单的行为

### 修复
- 修复 1.12.4 中引入的与 Start.exe 在加密箱中无法运行的问题 [#3514](https://github.com/sandboxie-plus/Sandboxie/issues/3514)
- 修复加密私有箱中启动进程时出现 "目录名无效" 的问题 [#3475](https://github.com/sandboxie-plus/Sandboxie/issues/3475)
- 修复在沙箱内部创建的符号链接无法正常工作的问题 [#3181](https://github.com/sandboxie-plus/Sandboxie/issues/3181)
- 修复对多个文件夹或一个驱动器字母与一个文件夹挂载的驱动器的问题
- 修复在使用 "SysCallLockDown=y" 的 Windows 11 上 explorer.exe 的问题 [#3516](https://github.com/sandboxie-plus/Sandboxie/issues/3516)
- 修复 SandMan 未显示位于 ImDisk 卷上的进程图标的问题



## [1.12.5 / 5.67.5] - 2023-12-19

### 修复
- 修复了影响基于 Chromium 的浏览器的错误，涉及 "--type=crashpad-handler" 实例 [#post-3177139](https://www.wilderssecurity.com/threads/sandboxie-plus-1-12-5.453131/#post-3177139)



## [1.12.4 / 5.67.4] - 2023-12-18

### 新增
- 在 Sbie 消息选项卡中增加日期和时间的显示 [#3429](https://github.com/sandboxie-plus/Sandboxie/issues/3429)

### 修改
- 在没有活跃的支持者证书的情况下，稳定通道将不再提供自动更新
  - 自动更新程序仍会通知有关新稳定版本的信息，并指导用户手动下载最新的安装程序
- SandMan 用户界面中的清理按钮现在即使在未连接到 Sandboxie 核心组件时也可用
- 箱创建向导现在允许基于任何其他箱类型创建加密沙箱

### 修复
- 修复在 ImDisk 卷中运行沙箱化进程的问题 [#3472](https://github.com/sandboxie-plus/Sandboxie/discussions/3472)
- 修复样本 634d066fd4f9a8b201a3ddf346e880be 在 Windows 7 x64 上无法被终止的问题 [#3482](https://github.com/sandboxie-plus/Sandboxie/issues/3482)
- 修复 UseNewSymlinkResolver 导致应用程序同时创建链接和目标文件夹的问题 [#3481](https://github.com/sandboxie-plus/Sandboxie/issues/3481)
- 修复沙箱重命名导致组层次结构破坏的问题 [#3430](https://github.com/sandboxie-plus/Sandboxie/issues/3430)
- 修复配有红色框的加密沙箱阻止访问其根目录的问题 [#3475](https://github.com/sandboxie-plus/Sandboxie/issues/3475)
- 修复在 1.12.0 中引入的 SandMan 崩溃问题 [#3492](https://github.com/sandboxie-plus/Sandboxie/issues/3492)



## [1.12.3 / 5.67.3] - 2023-12-02

### 新增
- 增加提供对机密箱有用的排除项的模板

### 修复
- 修复安全问题 ID-23，SeManageVolumePrivilege 现在被阻止，因为它允许读取 MFT 数据（感谢 Diversenok)
- 修复在强制进程进入机密箱时的程序启动问题 [#3173](https://github.com/sandboxie-plus/Sandboxie/issues/3173)



## [1.12.2 / 5.67.2] - 2023-11-28

### 新增
- 在导出箱时增加选项对话框 [#3409](https://github.com/sandboxie-plus/Sandboxie/issues/3409)

### 修改
- 将进程信息检索移动到 SbieSvc，解决当 SandMan 不以管理员身份运行时某些信息在隔离箱类型中不可用的问题
- 将进程挂起/恢复移动到 SbieSvc [#3156](https://github.com/sandboxie-plus/Sandboxie/issues/3156)

### 修复
- 修复使用 AutoDelete 选项时 Microsoft Edge 的问题 [#post-3173507](https://www.wilderssecurity.com/threads/sandboxie-plus-v1-12-1-pre-release.452939/#post-3173507)
- 修复与 Acrobat Reader 的警告问题： `SBIE2205 Service not implemented: CredEnumerateA` [#issuecomment-1826280016](https://github.com/sandboxie-plus/Sandboxie/issues/3441#issuecomment-1826280016)
- 修复在沙箱化 Microsoft 365 应用（如 Outlook、Word)时打开任何网页链接时的 UNEXPECTED_KERNEL_MODE_TRAP BSoD [#3427](https://github.com/sandboxie-plus/Sandboxie/issues/3427)
- 修复强制进程警告消息的问题
- 修复在线更新器未尊重可携式模式的问题 [#3406](https://github.com/sandboxie-plus/Sandboxie/issues/3406)
- 修复在加密箱中无法使用快照功能的问题 [#3439](https://github.com/sandboxie-plus/Sandboxie/issues/3439)



## [1.12.1 / 5.67.1] - 2023-11-23

### 修改
- 改进打开网址对话框 [#3401](https://github.com/sandboxie-plus/Sandboxie/issues/3401)
- 改进挂起进程检测

### 修复
- 修复使 SandMan 始终在最上层的热键问题
- 修复 ThreadSuspendCount 在已终止线程上失败的问题 [25054d0](https://github.com/sandboxie-plus/Sandboxie/commit/25054d0b2bd34c12471eefdf6ba9550e50ef02e5) [#3375](https://github.com/sandboxie-plus/Sandboxie/issues/3375)
- 修复消息文本 [#3408](https://github.com/sandboxie-plus/Sandboxie/issues/3408)
- 修复 Firefox 120.0 的警告问题： `SBIE2205 Service not implemented: CredWriteA` [#3441](https://github.com/sandboxie-plus/Sandboxie/issues/3441)
- 修复 Firefox 120.0 的警告问题： `SBIE2205 Service not implemented: CredReadA` [#3453](https://github.com/sandboxie-plus/Sandboxie/issues/3453)
- 修复过时的 Chromium 路径问题 [#3436](https://github.com/sandboxie-plus/Sandboxie/pull/3436)



## [1.12.0 / 5.67.0] - 2023-10-25

### 新增
- 增加自动将 USB 驱动器设置为强制文件夹的机制（需要支持者证书)
- 增加父母控制的故障排除脚本 [#3318](https://github.com/sandboxie-plus/Sandboxie/issues/3318)
- 开始对故障排除脚本的德语翻译 ...\SandboxiePlus\SandMan\Troubleshooting\lang_de.json [#3293](https://github.com/sandboxie-plus/Sandboxie/issues/3293)
- 增加 "get_cert SBIEX-XXXXX-XXXXX-XXXXX-XXXXX" 命令到 UpdUtil.exe，允许通过序列号使用命令行接收证书
- 增加撤销泄露或退款证书的机制
- 增加新的全局热键以将 SandMan 置于最上面 ALT+Break [#3320](https://github.com/sandboxie-plus/Sandboxie/issues/3320)
- 增加排除特定箱子不被 'Terminate all processes' 的选项 [#3108](https://github.com/sandboxie-plus/Sandboxie/issues/3108)
  - 注意：按下恐慌按钮热键三次，间隔少于 1 秒，即可终止所有进程而不例外
- 增加可自定义的全局热键，切换 "挂起强制程序" [#2441](https://github.com/sandboxie-plus/Sandboxie/issues/2441)
- 增加警告以防止广泛的 "强制文件夹" 设置 [#650](https://github.com/sandboxie-plus/Sandboxie/issues/650)
- 增加 CheckExpectFile 函数到 Sandboxie Plus [#768](https://github.com/sandboxie-plus/Sandboxie/issues/768)

### 修改
- 改进挂起进程处理 [#3375](https://github.com/sandboxie-plus/Sandboxie/issues/3375)
- 改进问题报告处理
- 更新提醒计划

### 修复
- 修复自动更新器未提供版本更新的问题
- 修复新符号链接处理代码的问题 [#3340](https://github.com/sandboxie-plus/Sandboxie/issues/3340)
- 修复在非服务状态下 Scm_StartServiceCtrlDispatcherX 行为不当的问题 [#1246](https://github.com/sandboxie-plus/Sandboxie/issues/1246) [#3297](https://github.com/sandboxie-plus/Sandboxie/issues/3297)
- 修复使用 mklink 创建到 OpenPipePath 的符号链接原始文件夹的配置问题 [#3207](https://github.com/sandboxie-plus/Sandboxie/issues/3207)

### 删除
- 删除过时的 /nosbiectrl 开关 [#3391](https://github.com/sandboxie-plus/Sandboxie/issues/3391)



## [1.11.4 / 5.66.4] - 2023-10-06

### 修改
- 改进 "Lock All Encrypted Boxes" 选项的行为 [#3350](https://github.com/sandboxie-plus/Sandboxie/issues/3350)
- 修复拼写错误：‘NoRestartOnPAC’ 选项变为 ‘NoRestartOnPCA’ [0e175ee](https://github.com/sandboxie-plus/Sandboxie/commit/0e175eedfde50198a3fa7bdef3a91fdbad5f5226)

### 修复
- 修复卸载受保护图像的问题 [#3347](https://github.com/sandboxie-plus/Sandboxie/issues/3347)
- 修复在 Sbie 消息选项卡中高亮无法关闭的问题 [#3338](https://github.com/sandboxie-plus/Sandboxie/issues/3338)
- 修复沙箱中的应用程序未通过 "开始菜单" 运行的问题 [#3334](https://github.com/sandboxie-plus/Sandboxie/issues/3334)
- 修复 NtQueryDirectoryObject 问题 [#3310](https://github.com/sandboxie-plus/Sandboxie/issues/3310)
- 修复一些 GUI 函数在 FileRootPath 值以 Volume{GUID} 开头时使用错误箱路径的问题 [#3345](https://github.com/sandboxie-plus/Sandboxie/issues/3345)
- 修复将程序保留在内存中时无法运行的问题 [#3349](https://github.com/sandboxie-plus/Sandboxie/issues/3349)
- 修复进程限制切换工作不正常的问题 [#3355](https://github.com/sandboxie-plus/Sandboxie/issues/3355)



## [1.11.3 / 5.66.3] - 2023-09-27

### 新增
- 在沙箱选项窗口中增加容器头备份/恢复选项 [#post-3165085](https://www.wilderssecurity.com/threads/sandboxie-plus-v1-11-3.452487/#post-3165085)

### 修改
- 更新至 7-Zip 23.01 [4ee1464](https://github.com/sandboxie-plus/Sandboxie/commit/4ee146430f70c91917fbcbfb77909b5b0b84a78c)

### 修复
- 修复更改加密箱密码或导出加密箱时显示不正确的文本 [#3296](https://github.com/sandboxie-plus/Sandboxie/pull/3296)
- 修复图像文件未被创建为稀疏文件的问题

## [1.11.2 / 5.66.2] - 2023-09-09

### 新增
- 增加个人证书过期时的更新警告
- 增加通过点击关闭按钮终止 UI 的选项 [#3253](https://github.com/sandboxie-plus/Sandboxie/issues/3253)
- 增加一键恢复被最小化到系统托盘的 Sandboxie 窗口的选项 [#3251](https://github.com/sandboxie-plus/Sandboxie/issues/3251)
- 增加通过点击最小化按钮将 Sandboxie 最小化到系统托盘的选项 [#3252](https://github.com/sandboxie-plus/Sandboxie/issues/3252)
- 增加独立的桌面集成设置 [#3246](https://github.com/sandboxie-plus/Sandboxie/issues/3246)
- 增加 AdGuard 浏览器助手的模板预设 [#3283](https://github.com/sandboxie-plus/Sandboxie/pull/3283)

### 更改
- 将托盘选项移至独立标签页
- 用更好的压缩图像替换了一些图形（感谢 idealths) [1af82bb](https://github.com/sandboxie-plus/Sandboxie/commit/1af82bb39643e7470dd4839bf3cebe8a4acf9d06)
- 将 /mount_protected 命令行开关的检查提前到 /mount 命令行开关之前 [#3289](https://github.com/sandboxie-plus/Sandboxie/pull/3289)

### 修复
- 修复了订阅证书识别问题
- 修复了关于窗口中 Logo 被截断的问题 [#3249](https://github.com/sandboxie-plus/Sandboxie/issues/3249)
- 修复了使用 RAM 驱动器时文件恢复的问题 [d82b62e](https://github.com/sandboxie-plus/Sandboxie/commit/d82b62ee78d865e21005b9b81dfa9dac9f524b90)



## [1.11.1 / 5.66.1] - 2023-08-31

### 新增
- 增加 'RamDiskLetter=R:\' 选项，允许将 RAM 驱动器根目录挂载到驱动器字母上 [938e0a8](https://github.com/sandboxie-plus/Sandboxie/commit/938e0a8c8d88e3780ece674c6702654d0b4e6ddc)

### 更改
- 将新选项布局更改为非复古视图的默认设置（可以在设置中更改回去) [94c3f5e](https://github.com/sandboxie-plus/Sandboxie/commit/94c3f5e35bf9e7c993557f2c9d4e6e5129e9d1df)

### 修复
- 修复了重新创建 RAM 沙箱连接的问题 [2542351](https://github.com/sandboxie-plus/Sandboxie/commit/254235136fa8b74ad147f03b646d4015208c14be)
- 修复了设置向导中 Sandboxie logo 缩放的问题 [#3227](https://github.com/sandboxie-plus/Sandboxie/issues/3227)
- 修复了在创建箱子向导中截断文本的问题 [#3226](https://github.com/sandboxie-plus/Sandboxie/issues/3226)
- 修复了与 ImBox.exe 的 Windows 7 兼容性问题 [1f0b2b7](https://github.com/sandboxie-plus/Sandboxie/commit/1f0b2b71ba47436252fd55eece2c3624085b46dc)
- 修复了 InjectDll/InjectDll64 设置未正确添加到沙箱配置中的错误 [#3254](https://github.com/sandboxie-plus/Sandboxie/pull/3254)
- 修复了与 'UseNewSymlinkResolver=y' 相关的问题 [01a2670](https://github.com/sandboxie-plus/Sandboxie/commit/01a2670b9cc6f00bdc81b990e0c75c2688fd54fd)
- 修复了 Vintage View 模式下 SandMan 崩溃的问题 [#3264](https://github.com/sandboxie-plus/Sandboxie/issues/3264)
- 修复了固定多个指向同一文件的快捷方式的问题 [#3259](https://github.com/sandboxie-plus/Sandboxie/issues/3259)



## [1.11.0 / 5.66.0] - 2023-08-25

### 新增
- 增加 ImDisk 驱动程序，支持在 RAM 驱动器中创建沙箱
- 增加了加密沙箱支持；这会创建不泄露数据到宿主 PC 的机密箱
  - 通过 ImDisk 驱动程序和新的 ImBox 组件，采用来自 [DiskCryptor](https://diskcryptor.org/) 的加密实现，沙箱根文件夹存储在加密的容器文件中
  - 使用 SbieDrv 防止不属于沙箱的进程访问加密沙箱的根文件夹
  - 使用 'ConfidentialBox=y' 选项，可以阻止宿主进程读取沙箱进程
- 在关于对话框中增加证书信息
- 增强证书样式的灵活性
- 增加商业客户通过序列号检索硬件绑定证书的选项
- 增加使用升级序列号升级现有证书的选项

### 更改
- 改善在线更新器代码
- 在箱子创建向导中将下拉列表替换为单选按钮 [#1381](https://github.com/sandboxie-plus/Sandboxie/issues/1381)

### 修复
- 修复在沙箱中创建的符号链接无法正常工作的错误 [#3181](https://github.com/sandboxie-plus/Sandboxie/issues/3181)
- 修复了 Plus UI 弹出窗口中文字截断的问题 [#3195](https://github.com/sandboxie-plus/Sandboxie/issues/3195)



## [1.10.5 / 5.65.5] - 2023-08-12

### 更改
- 子菜单在鼠标离开时不会关闭（感谢 typpos) [#2963](https://github.com/sandboxie-plus/Sandboxie/issues/2963)

### 修复
- 修复商业证书的验证问题



## [1.10.4 / 5.65.4] - 2023-08-11

### 修复
- 修复了 Sandboxie-Plus-x64-v1.10.3 启动时崩溃的问题 [#3174](https://github.com/sandboxie-plus/Sandboxie/issues/3174)
- 修复了所有向导中的未翻译按钮问题 [#3133](https://github.com/sandboxie-plus/Sandboxie/issues/3133)



## [1.10.3 / 5.65.3] - 2023-08-06

### 新增
- 在运行菜单中增加对 URL 快捷方式文件的支持 [#3151](https://github.com/sandboxie-plus/Sandboxie/issues/3151)
- 增加了在特殊情况下 NtQueryObject 锁定的解决方法；要启用，请使用 'UseDriverObjLookup=y'
- 添加项管理器：在版本列中加入维护者信息提示 [#3167](https://github.com/sandboxie-plus/Sandboxie/issues/3167)
- 增加打开网站的机制用于附件 [#3166](https://github.com/sandboxie-plus/Sandboxie/issues/3166)

### 更改
- 增强商业证书处理，增加使用计数和机器绑定选项

### 修复
- 修复了固定快捷方式的问题
- 修复了进程暂停/恢复上下文菜单的问题 [#3156](https://github.com/sandboxie-plus/Sandboxie/issues/3156)
- 修复与 Qt 基础安装程序相关的问题 [#2493](https://github.com/sandboxie-plus/Sandboxie/issues/2493) [#3153](https://github.com/sandboxie-plus/Sandboxie/issues/3153)



## [1.10.2 / 5.65.2] - 2023-07-31

### 新增
- 重新增加了暂停沙箱进程的选项 [#3126](https://github.com/sandboxie-plus/Sandboxie/issues/3126)

### 更改
- 更改了附加数据的格式 [#3135](https://github.com/sandboxie-plus/Sandboxie/issues/3135)
  - 从版本 1.10.0 和 1.10.1 来的所有用户都需要在附加项管理器中重新安装组件
- "OpenClipboard=n" 现在也拒绝写入剪贴板 [#1367](https://github.com/sandboxie-plus/Sandboxie/issues/1367)

### 修复
- 修复跨重命名目录的问题
- 修复自动滚动不工作的情况 [#393](https://github.com/sandboxie-plus/Sandboxie/issues/393)
- 修复与箱子删除保护有关的 UI 问题 [#3104](https://github.com/sandboxie-plus/Sandboxie/issues/3104)
- 修复链接参数处理中的问题 [#2969](https://github.com/sandboxie-plus/Sandboxie/issues/2969)
- 修复在 1.10.1 中引入的 IPC 问题 [#3132](https://github.com/sandboxie-plus/Sandboxie/issues/3132) [#3134](https://github.com/sandboxie-plus/Sandboxie/issues/3134)
- 修复固定运行条目图标的问题
- 修复 UGlobalHotkey 库与 Qt6 不兼容的问题

### 删除
- 删除对 LogAPI 库的硬编码支持
  - 请使用附加项管理器和 DLL 注入设置



## [1.10.1 / 5.65.1] - 2023-07-24

### 新增
- 增加 UI 选项以选择自定义文本编辑器打开 Sandboxie.ini [#3116](https://github.com/sandboxie-plus/Sandboxie/issues/3116)
- 增加对箱子删除和内容删除的独立保护 [#3104](https://github.com/sandboxie-plus/Sandboxie/issues/3104)
- 在跟踪日志选项卡中增加“自动滚动”功能 [#393](https://github.com/sandboxie-plus/Sandboxie/issues/393)

### 更改
- 重新设计 NT 对象句柄处理
- "OpenClipboard=n" 现在也在用户模式下实现，从而使其适用于隔离箱
- 更改 Delete V2 方案以使用 FilePaths.dat 中的驱动器字母（与 NT 路径的向后兼容性保留) [#3053](https://github.com/sandboxie-plus/Sandboxie/issues/3053)
- 改善快照合并的稳健性 [#3017](https://github.com/sandboxie-plus/Sandboxie/issues/3017)

### 修复
- 修复“禁用安全隔离”导致游戏停止播放音频的问题 [#2893](https://github.com/sandboxie-plus/Sandboxie/issues/2893)
- 修复 NtQueryDirectoryObject 未实现的问题 [#2734](https://github.com/sandboxie-plus/Sandboxie/issues/2734)
- 修复运行菜单条目的工作目录问题
- 修复沙箱命名空间中的全局符号链接不正确的问题 [#3112](https://github.com/sandboxie-plus/Sandboxie/issues/3112)
- 修复点击“显示堆栈跟踪”时出现的 'Addon already installed!' 错误 [#3114](https://github.com/sandboxie-plus/Sandboxie/issues/3114)
- 修复 BoxNameTitle=process.exe,- 在切换其他选项时被去除的问题 [#3106](https://github.com/sandboxie-plus/Sandboxie/issues/3106)
- 修复异步分配的 PCA 任务未被正确检测的问题 [#1919](https://github.com/sandboxie-plus/Sandboxie/issues/1919)
- 修复与第一个 Windows 10 版本不兼容的问题 [#3117](https://github.com/sandboxie-plus/Sandboxie/issues/3117)
- 修复移除沙箱仅在应用程序运行于沙箱时删除沙箱内容的问题 [#3118](https://github.com/sandboxie-plus/Sandboxie/issues/3118)
- 修复脚本引擎未正确终止引起的崩溃问题 [#3120](https://github.com/sandboxie-plus/Sandboxie/issues/3120)
- 修复 ImDisk 在 Sandboxie 监督下导致 SBIE2337 和有时 BSoD 的问题 [#1092](https://github.com/sandboxie-plus/Sandboxie/issues/1092)
- 修复快照合并重复目录连接的问题 [#3016](https://github.com/sandboxie-plus/Sandboxie/issues/3016)
- 修复使用 Delete V2 重命名功能时与快照相关的问题
- 修复在使用网络共享时与 Delete V2 相关的问题
- 修复在使用 "UseVolumeSerialNumbers=y" 时访问驱动器根的问题
- 修复移除快照在使用 Delete V2 时复活已删除文件的问题 [#3015](https://github.com/sandboxie-plus/Sandboxie/issues/3015)

### 删除
- 删除对 Internet Download Manager 的模板支持 [#991](https://github.com/sandboxie-plus/Sandboxie/issues/991#issuecomment-1646582375)



## [1.10.0 / 5.65.0] - 2023-07-12

### 新增
- 增加沙箱脚本引擎，使 SandMan 更加灵活
- 增加可脚本化的故障排除向导 [#1875](https://github.com/sandboxie-plus/Sandboxie/issues/1875)
- 增加附加项管理器，帮助安装附加和第三方组件，现有附加项：
  - [ImDisk Toolkit](https://sourceforge.net/projects/imdisk-toolkit/) - 用于创建 RAM 驱动器和其他虚拟驱动器
  - [V4 Script Debugger](https://github.com/DavidXanatos/NeoScriptTools) - 用于调试故障排除脚本
  - [Microsoft Debug Help Library](https://learn.microsoft.com/en-us/windows/win32/debug/debug-help-library) - 用于 1.9.6 中引入的堆栈跟踪功能
  - [signcheck.exe](https://learn.microsoft.com/en-us/sysinternals/downloads/sigcheck) - 用于在恢复文件之前在 VirusTotal 上扫描文件
  - [SbieHide.dll](https://github.com/VeroFess/SbieHide) - 一个第三方 DLL，用于隐藏 SbieDll.dll
  - [LogAPI.dll](https://bsa.isoftware.nl/) - 一种用于 Buster Sandbox Analyzer 的 API 日志记录库
- 增加设置更新间隔的选项，时间设定为 1、7、14 或 30 天
- 在 SbieCtrl.exe 中增加 `What's new in Sandboxie Plus` 对话框，以突出 Plus UI 的新功能
  - 注意：该对话框在安装 Sandboxie Classic 后显示
- 增加 "fixdacls" 命令到 KmdUtil.exe，用于修复 Sandboxie 文件夹上损坏的 DACL 条目，解决 SbieDll.dll 加载失败的问题
- 增加隐藏 Sandboxie 自身进程的功能 [#3054](https://github.com/sandboxie-plus/Sandboxie/issues/3054)
- 增加在 Plus UI 中缓存 Sandboxie 消息的功能 [#2920](https://github.com/sandboxie-plus/Sandboxie/issues/2920)
- 增加直接从 SBIE 消息弹出窗口调用故障排除向导的按钮

### 更改
- 安装向导现在有一个专门的更新配置页面
  - 注意：对所有未启用更新的用户只会显示此页面一次
- 将支持页面拆分为 Sandboxie Support 和 Sandboxie Updater 标签
- 当 troubleshooting.7z 文件可用时，将使用脚本引擎来匹配兼容性模板
  - 注意：这允许通过使用 AppCompatibility.js 脚本来更好地细分模板选择
- 重新设计底层代码注入机制，以提高灵活性和调试能力
  - 注意：主注入转向代码现在用 C 而不是汇编语言编写，并能够正确报告 SbieDll.dll 加载错误为 SBIE2181
- 改善会话代理启动的灵活性
- 改善 SBIEMSG 帮助处理，链接现在包含消息详细信息，以便指向更精确的文档（如果可用)
- 更新证书验证代码

### 修复
- 修复 Sandboxie Classic 安装程序中的卸载问题 [d1863ff](https://github.com/sandboxie-plus/Sandboxie/commit/d1863ffadfe105c695de71c9e841c2fd568116fe)
- 引入 Chrome 在 Windows 11 的 KB5027231 更新后无法启动的解决方法 [#3040](https://github.com/sandboxie-plus/Sandboxie/issues/3040)
- 改善调试版的 procmon/堆栈跟踪兼容性
- 修复非标准命令行的问题
- 修复在线更新器以每天而不是每周间隔进行检查的问题

### 删除
- 清理了重复代码（感谢 lmou523) [#3067](https://github.com/sandboxie-plus/Sandboxie/pull/3067)



## [1.9.8 / 5.64.8] - 2023-06-21

### 更改
- 改善 Sandboxie Classic 安装程序中的卸载过程（感谢 sredna) [#3044](https://github.com/sandboxie-plus/Sandboxie/pull/3044)

### 修复
- 修复当路径是通过符号链接创建时出现的链接错误（感谢 lmou523) [#3038](https://github.com/sandboxie-plus/Sandboxie/pull/3038)
- 修复文件覆盖对话框后恢复窗口的焦点 [#3048](https://github.com/sandboxie-plus/Sandboxie/issues/3048)
- 修复了由于异常代码 c0000005 导致的 Sandboxie Plus 崩溃问题 [#3024](https://github.com/sandboxie-plus/Sandboxie/issues/3024)



## [1.9.7 / 5.64.7] - 2023-06-09

### 新增
- 增加可配置的工具栏项目（感谢 typpos) [#2961](https://github.com/sandboxie-plus/Sandboxie/pull/2961)
- 增加 Plus UI 的新图标（感谢 typpos) [#3010](https://github.com/sandboxie-plus/Sandboxie/pull/3010) [#3011](https://github.com/sandboxie-plus/Sandboxie/pull/3011)

### 更改
- 对 BlockSoftwareUpdaters 模板进行小幅更新（感谢 APMichael) [#2991](https://github.com/sandboxie-plus/Sandboxie/pull/2991)

### 修复
- 修复在 Plus UI 中使用 “复制面板” 时出现的空行问题 [#2995](https://github.com/sandboxie-plus/Sandboxie/issues/2995)
- 修复 Plus 安装程序在更新当前已安装版本时未遵循 IniPath 值的问题（感谢 offhub) [#2994](https://github.com/sandboxie-plus/Sandboxie/issues/2994)
- 修复崩溃转储的时间戳
- 修复 Firefox 114 在启用了启动限制的安全强化箱中崩溃的问题（感谢 offhub) [#3019](https://github.com/sandboxie-plus/Sandboxie/issues/3019)
- 修复当没有 GUI 代理时的错误（感谢 lmou523) [#3020](https://github.com/sandboxie-plus/Sandboxie/pull/3020)

## [1.9.6 / 5.64.6] - 2023-05-30

### 新增
- 为所有跟踪消息添加完整堆栈跟踪
  - 激活 Trace Log 选项卡还默认开启 Keep Terminated 功能 [#2988](https://github.com/sandboxie-plus/Sandboxie/issues/2988#issuecomment-1567787776)
- 新增以非管理员身份启动默认记事本编辑器的选项 [#2987](https://github.com/sandboxie-plus/Sandboxie/issues/2987)

### 修复
- 修复了 SbieDrv 驱动程序中的令牌操作问题
- 修复“重置所有 GUI 选项”未按预期重置所有 GUI 部分的问题 [#2967](https://github.com/sandboxie-plus/Sandboxie/issues/2967)
- 修复 Box Type 预设下的 sbie:// 链接 [#2959](https://github.com/sandboxie-plus/Sandboxie/issues/2959#issuecomment-1565264161)
- 修复“重置所有 GUI 选项”导致所有沙箱名称消失 [#2972](https://github.com/sandboxie-plus/Sandboxie/issues/2972)
- 修复游戏无法通过“从开始菜单运行”正常启动的问题 [#2969](https://github.com/sandboxie-plus/Sandboxie/issues/2969)
- 修复 Microsoft Excel 数据网格中的拖放问题 [9455e96](https://github.com/sandboxie-plus/Sandboxie/commit/9455e96a699cbc665f791e191f2a13bb40783ab0)
  - 可在 [#856](https://github.com/sandboxie-plus/Sandboxie/issues/856) 中提供其他拖放场景的反馈
- 修复回归：DLL 加载问题（未找到入口点) [#2980](https://github.com/sandboxie-plus/Sandboxie/issues/2980)
- 修复在虚拟化方案 v2 激活时 Sandboxie 未标记已删除文件或注册表项的问题（1.9.4/1.9.5) [#2984](https://github.com/sandboxie-plus/Sandboxie/issues/2984)
- 修复 SandMan：Sandboxie.ini 的时间戳 [#2985](https://github.com/sandboxie-plus/Sandboxie/issues/2985)
- 修复多次按下“显示 NT 对象树”按钮后崩溃（Plus UI) [#2943](https://github.com/sandboxie-plus/Sandboxie/issues/2943)

## [1.9.5 / 5.64.5] - 2023-05-26

### 新增
- 新增将 SBIE 消息 ID 转换为可点击链接 [#2953](https://github.com/sandboxie-plus/Sandboxie/issues/2953)
- 新增相同日志消息数量的限制 [#2856](https://github.com/sandboxie-plus/Sandboxie/issues/2856)

### 修复
- 修复 sbie:// 链接无法正常工作的问题 [#2959](https://github.com/sandboxie-plus/Sandboxie/issues/2959)
- 修复 Sbie 消息、Trace Log、Recovery Log 选项卡不应隐藏某些列的问题 [#2940](https://github.com/sandboxie-plus/Sandboxie/issues/2940)
- 修复 RegisterDeviceNotificationW 函数 DBT_DEVTYP_DEVICEINTERFACE [#2129](https://github.com/sandboxie-plus/Sandboxie/discussions/2129) [#1975](https://github.com/sandboxie-plus/Sandboxie/issues/1975)
- 修复可能在 1.9.4 中引入的 BSoD 问题（由 7starsseeker 报告) [#2958](https://github.com/sandboxie-plus/Sandboxie/issues/2958)

## [1.9.4 / 5.64.4] - 2023-05-24

### 新增
- 为 Plus UI 添加更多文档链接
- 新增托盘菜单选项以关闭待处理的更新通知
- 新增将文件固定/收藏到托盘 [#2913](https://github.com/sandboxie-plus/Sandboxie/issues/2913)

### 变更
- 改善隐私增强的盒子类型的兼容性模板（感谢 offhub) [#2899](https://github.com/sandboxie-plus/Sandboxie/pull/2899)
- 改善设置中的支持页面和提醒 [#2896](https://github.com/sandboxie-plus/Sandboxie/issues/2896)
- 改善签名错误消息 [#2931](https://github.com/sandboxie-plus/Sandboxie/issues/2931)
- 更改在存在例外时不显示“No Inet” [#2919](https://github.com/sandboxie-plus/Sandboxie/issues/2919)

### 修复
- 修复 ARM64 构建中的 Qt6 问题
- 修复无驱动器字母的盒子挂载时的 V2 删除问题
- 修复高 DPI 缩放下的图标覆盖问题
- 修复多项选择中的行为问题（感谢 okrc) [#2903](https://github.com/sandboxie-plus/Sandboxie/pull/2903)
- 修复默认盒子分组的问题（感谢 okrc) [#2910](https://github.com/sandboxie-plus/Sandboxie/pull/2910)
- 修复沙箱重命名的问题 [#2912](https://github.com/sandboxie-plus/Sandboxie/issues/2912)
- 修复打开系统保护存储的复选框未选中的问题 [#2866](https://github.com/sandboxie-plus/Sandboxie/issues/2866)
- 修复防火墙规则缺乏色彩对比使文本难以阅读的问题 [#2900](https://github.com/sandboxie-plus/Sandboxie/issues/2900)
- 修复RecoverFolder显示 GUID 而不是文件夹名称的问题 [#2918](https://github.com/sandboxie-plus/Sandboxie/issues/2918)
- 修复操作停止时沙盘图标覆盖在系统托盘中卡住的问题 [#2869](https://github.com/sandboxie-plus/Sandboxie/issues/2869)
- 修复文件面板不允许永久调整列大小的问题 [#2930](https://github.com/sandboxie-plus/Sandboxie/issues/2930)
- 修复重命名具有沙箱运行条目的盒子可能会破坏这些条目的问题 [#2921](https://github.com/sandboxie-plus/Sandboxie/issues/2921)
- 修复设置向导后 WFP 未启用及其他问题 [#2915](https://github.com/sandboxie-plus/Sandboxie/issues/2915)
- 修复沙箱视图中的名称列永远不应被隐藏 [#2933](https://github.com/sandboxie-plus/Sandboxie/issues/2933)
- 修复文件面板未共享沙箱视图列中的右键选项的问题 [#2934](https://github.com/sandboxie-plus/Sandboxie/issues/2934)
- 修复 NetworkEnableWFP 和 EnableObjectFiltering 设置复选框的问题 [#2935](https://github.com/sandboxie-plus/Sandboxie/issues/2935)
- 修复未引用的服务路径 [#537](https://github.com/sandboxie-plus/Sandboxie/issues/537)
- 修复在沙箱中运行 Microsoft Word 的插入图表失败 [#2863](https://github.com/sandboxie-plus/Sandboxie/issues/2863)
- 修复切换沙箱类型预设配置时发生错误的问题 [#2941](https://github.com/sandboxie-plus/Sandboxie/issues/2941)
- 修复无法打开 Microsoft Excel 的问题（感谢 lmou523) [#2890](https://github.com/sandboxie-plus/Sandboxie/issues/2890)

## [1.9.3 / 5.64.3] - 2023-05-08

### 新增
- 新增全局设置以禁用覆盖图标
- 新增指定 Sandboxie.ini 位置到注册表的功能
  - 打开 "HKLM\SYSTEM\CurrentControlSet\Services\SbieDrv" 并设置 "IniPath" [#2837](https://github.com/sandboxie-plus/Sandboxie/issues/2837)
  - 使用 REG_SZ 字符串作为路径，如 "\\??\C:\my_path\my_sandboxie.ini"

### 变更
- 改善 DLL 注入失败处理
- 更新波兰语翻译（感谢用户 7zip)

### 修复
- 修复命令行问题 [#2858](https://github.com/sandboxie-plus/Sandboxie/issues/2858)
- 修复恢复窗口中总是显示在最上面的问题 [#2885](https://github.com/sandboxie-plus/Sandboxie/issues/2885)

## [1.9.2 / 5.64.2] - 2023-04-24

### 新增
- 新增盒子预设菜单选项以切换 'DisableForceRules=y'（感谢 offhub) [#2851](https://github.com/sandboxie-plus/Sandboxie/pull/2851)

### 修复
- 修复 SBIE1305 消息的问题
- 修复韩语翻译的编码问题 [#2833](https://github.com/sandboxie-plus/Sandboxie/issues/2833)

## [1.9.1 / 5.64.1] - 2023-04-23

### 新增
- 新增禁用给定沙箱所有强制规则的选项 [#2797](https://github.com/sandboxie-plus/Sandboxie/issues/2797)

### 变更
- 重新设计盒子分组的配置存储

### 修复
- 修复经典安装程序中的系统检查 [#2812](https://github.com/sandboxie-plus/Sandboxie/pull/2812)
- 修复 Normal[File/Key]Path 的内存损坏问题 [#2588](https://github.com/sandboxie-plus/Sandboxie/issues/2588)
- 修复快照管理器窗口中的 DPI 缩放问题 [#782](https://github.com/sandboxie-plus/Sandboxie/issues/782)
- 修复 arm64 DialogProc 钩子的问题 [#2838](https://github.com/sandboxie-plus/Sandboxie/issues/2838)
- 修复与 7-Zip 文件系统实现的 Qt6 兼容性问题 [#2681](https://github.com/sandboxie-plus/Sandboxie/issues/2681)
- 修复使用 Qt6 的 TLS 支持问题 [#2682](https://github.com/sandboxie-plus/Sandboxie/issues/2682)
- 新增 Spotify 的模板预设 [#2673](https://github.com/sandboxie-plus/Sandboxie/issues/2673)
- 新增查找器的缺失本地化 [#2845](https://github.com/sandboxie-plus/Sandboxie/issues/2845)

## [1.9.0 / 5.64.0] - 2023-04-17

### 新增
- 新增本地模板编辑器到全局设置窗口
- 新增隐藏的消息列表
- 新增防止位于沙箱外的进程加载封装的 DLL 的选项
  - 要启用此功能，请使用 "ProtectHostImages=y"
- 新增选项以阻止沙箱内部程序，但不阻止沙箱外部程序
- 新增 SbieLogon 选项到新盒子向导 [#2823](https://github.com/sandboxie-plus/Sandboxie/issues/2823)
- 新增一些 UI 调试选项 [#2816](https://github.com/sandboxie-plus/Sandboxie/issues/2816)

### 变更
- 运行菜单条目现在可以拥有自定义图标
- 初始化编辑页面将在用户更改内容时自动切换到编辑模式
- 改善全局设置窗口更改处理
- 重新组织全局选项

### 修复
- 修复 SandMan UI 中的组重命名问题 [#2804](https://github.com/sandboxie-plus/Sandboxie/issues/2804)

## [1.8.4 / 5.63.4] - 2023-04-07

### 新增
- 新增安装程序图标 [#2795](https://github.com/sandboxie-plus/Sandboxie/issues/2795)
- 新增进程列表中的令牌类型指示器

### 修复
- 修复 Microsoft Edge 112.x 的兼容性问题
- 修复更新程序问题 [#2790](https://github.com/sandboxie-plus/Sandboxie/issues/2790)
- 修复新盒子向导中的盒子命名问题，改为使盒子名称固定 [#2792](https://github.com/sandboxie-plus/Sandboxie/issues/2792)
- 修复 Firefox/Thunderbird 的问题 [#2799](https://github.com/sandboxie-plus/Sandboxie/issues/2799)

## [1.8.3 / 5.63.3] - 2023-04-05

### 修复
- 修复 WFP 支持和驱动程序验证器的问题

## [1.8.2a / 5.63.2] - 2023-04-02

### 变更
- 在隔离模式下禁用基于令牌的解决方法

### 修复
- 修复在媒体移除时触发新 SBIE2307 消息的问题
- 从禁用的状态中排除某些旧令牌黑客（针对 Firefox)
- 修复长期存在的与隔离类型盒子有关的 ping 问题 [#1608](https://github.com/sandboxie-plus/Sandboxie/issues/1608)

## [1.8.2 / 5.63.2] - 2023-04-01

### 新增
- 重新设计 CreateAppContainerToken 钩子以返回受限令牌，用于解决该问题 [#2762](https://github.com/sandboxie-plus/Sandboxie/issues/2762)
  - 注意：此行为可以通过 'FakeAppContainerToken=program.exe,n' 禁用
- 在应用隔离模式下启用应用容器兼容性
  - 注意：这应该改善 Microsoft Edge 的兼容性
- 新增网页浏览器兼容性模板向导 [#2761](https://github.com/sandboxie-plus/Sandboxie/issues/2761)
- 新增动态检测基于 Chromium 和 Firefox 的浏览器的机制
  - 注意：新机制可以通过 'DynamicImageDetection=program.exe,n' 禁用

### 变更
- 将 'DropAppContainerTokens=program.exe,n' 重命名为 'DropAppContainerToken=program.exe,n'
- 'DropAppContainerToken=program.exe,y' 现在可以在应用隔离盒中使用，尽管从安全角度来看不建议使用
- 现在默认启用用于 Chrome、Firefox 和 Acrobat 的桌面安全解决方法，您可以通过 "UseSbieDeskHack=n" 禁用
  - 注意：这应该允许 Electron 应用无需 'SpecialImage=chrome,program.exe' 即可运行
- 禁用旧令牌黑客，因为这些似乎在新应用容器令牌下不再需要
  - 注意：若存在与 Microsoft Edge、Chrome、Firefox 或 Acrobat 相关的问题，可以通过 'DeprecatedTokenHacks=y' 重新启用
- 更新到 Inno Setup 6.2.2，并附带新的安装程序图标

### 修复
- 修复全局设置 INI 部分编辑的问题
- 修复 'UseRegDeleteV2=y' 的问题 [#2756](https://github.com/sandboxie-plus/Sandboxie/issues/2756)
- 自动运行路径现在支持任何长度 [#2769](https://github.com/sandboxie-plus/Sandboxie/pull/2769)（感谢 Sapour)
- 恢复窗口：未选择任何文件/文件夹时不再显示删除确认对话框 [#2771](https://github.com/sandboxie-plus/Sandboxie/pull/2771)（感谢 Sapour)
- 修复 WeChat 崩溃问题 [#2772](https://github.com/sandboxie-plus/Sandboxie/pull/2772)

### 删除
- 删除对 Maxthon 4 的过时解决方法

## [1.8.1 / 5.63.1] - 2023-03-12

### 新增
- 添加证书保护 [#2722](https://github.com/sandboxie-plus/Sandboxie/issues/2722)

### 修复
- 修复了跟踪日志中查找器搜索的问题
- 修复了 NT 名称空间虚拟化的问题 [#2701](https://github.com/sandboxie-plus/Sandboxie/issues/2701)
- 修复了“以未沙盒模式运行”的问题 [#2710](https://github.com/sandboxie-plus/Sandboxie/issues/2710)
- 修复了新盒向导的问题 [#2729](https://github.com/sandboxie-plus/Sandboxie/issues/2729)
- 修复了突破进程和用户限制沙箱的问题 [#2732](https://github.com/sandboxie-plus/Sandboxie/issues/2732)
- 修复了一些 UI 问题 [#2733](https://github.com/sandboxie-plus/Sandboxie/issues/2733)
- 修复了无用的帮助按钮 [#2748](https://github.com/sandboxie-plus/Sandboxie/issues/2748)

## [1.8.0 / 5.63.0] - 2023-02-27

### 新增
- 运行菜单现在支持文件夹，可以通过在 UI 的名称列中输入 foldername1\foldername2\entryname 来使用
- 添加了待处理更新的托盘指示器
- 为 CreateDirectoryObject(Ex) 和 OpenDirectoryObject 添加了虚拟化（提高了安全性，防止命名占用)
  - 注意：可以通过设置 'NtNamespaceIsolation=n' 来禁用此功能

### 更改
- 'OpenProtectedStorage=y' 已被模板替代
- 将所有内置访问规则移动到一组默认模板
- 将 WinInetCache 控制移动到模板 OpenWinInetCache；'CloseWinInetCache=y' 现在已过时
- 添加了 CreateAppContainerToken 的钩子，这也应该改善与其他应用程序的兼容性 [#1926](https://github.com/sandboxie-plus/Sandboxie/issues/1926)
  - 注意：不再需要 Template_Edge_Fix
- 替换了一些图标
- 将“支持”全局设置页面移至“高级配置”页面之上并更名为“支持与更新”
- 当将文件拖放到 SandMan UI 中以运行时，当前选中的盒子将在盒子选择对话框中预先选择
- 改进了访问规则处理 [#2633](https://github.com/sandboxie-plus/Sandboxie/discussions/2633)
- SbieCtrl 现在在检查更新时使用新的更新格式
- 在规则的具体性中将主要匹配项优先于辅助匹配项

### 修复
- 为隔离类型盒子添加了 AppContainer 支持
- 修复安全问题 ID-22，NtCreateSectionEx 未被驱动程序过滤
- 修复了没有系统令牌启动服务的问题
- 修复了与新文件迁移设置相关的问题 [#2700](https://github.com/sandboxie-plus/Sandboxie/issues/2700)
- 修复了 ARM64 上的外壳集成问题 [#2685](https://github.com/sandboxie-plus/Sandboxie/issues/2685)
- 修复了与驱动程序验证器的新问题 [#2708](https://github.com/sandboxie-plus/Sandboxie/issues/2708)

## [1.7.2 / 5.62.2] - 2023-02-05

### 更改
- 再次重新组织盒子选项，旧盒子布局再次为默认
- SBIE2227 指出不支持 8.3 名称的卷现在默认禁用
  - 注意：可以使用 "EnableVerboseChecks=y" 重新启用此检查

### 修复
- 修复了删除 v1/v2 显示未遵循全局预设的问题
- 避免在拖动时阻止 Windows 资源管理器 [#2660](https://github.com/sandboxie-plus/Sandboxie/pull/2660)
- 修复了 QtSingleApp 的问题 [#2659](https://github.com/sandboxie-plus/Sandboxie/issues/2659)
- 修复了更新程序有时未能创建临时目录的问题 [#2615](https://github.com/sandboxie-plus/Sandboxie/issues/2615)
- 修复了快照移除的问题 [#2663](https://github.com/sandboxie-plus/Sandboxie/issues/2663)
- 修复了符号链接的问题 [#2606](https://github.com/sandboxie-plus/Sandboxie/issues/2606)
- 修复了 AppContainer 隔离的问题，应用程序容器令牌现在默认被丢弃
  - 注意：可以通过 'DropAppContainerTokens=program.exe,n' 禁用此行为

## [1.7.1 / 5.62.1] - 2023-01-30

### 新增
- 添加在盒子选择对话框中创建新沙箱的选项
- 添加沙箱创建向导（不适用于复古视图模式)
- 添加打开所有 COM 类的能力 [#2448](https://github.com/sandboxie-plus/Sandboxie/issues/2448)
  - 注意：使用 OpenClsid={00000000-0000-0000-0000-000000000000} 打开所有
- SandMan UI 现在指示沙箱进程是否具有提升（管理员)或系统令牌
- DropAdminRights 现在可以按进程配置 [#2293](https://github.com/sandboxie-plus/Sandboxie/issues/2293)
- 添加自我删除盒子 [#1936](https://github.com/sandboxie-plus/Sandboxie/issues/1936)
- 添加了盒子选择对话框的 Ctrl+F 搜索过滤器
- 添加了编辑 Templates.ini 和 Sandboxie-Plus.ini 的菜单选项

### 更改
- 重构驱动中的网络阻止代码
- 盒子选项现在在适当时显示展开的路径
- 使新盒子选项布局为默认（可在外观设置中更改)

### 修复
- 修复了 BlockNetworkFiles=y 与 RestrictDevices=y 不一起工作的情况 [#2629](https://github.com/sandboxie-plus/Sandboxie/issues/2629)
- 修复了因 1.7.0 引入的 SandMan 崩溃问题
- 修复了跟踪日志过滤器区分大小写的问题
- 修复了与 Delete V2 相关的性能问题
- 修复了 NtQueryDirectoryFile 数据对齐的问题 [#2443](https://github.com/sandboxie-plus/Sandboxie/issues/2443)
- 修复了 Microsoft Edge 111 开发版的问题 [#2631](https://github.com/sandboxie-plus/Sandboxie/issues/2631)
- 修复了 mio 套接字的问题 [#2617](https://github.com/sandboxie-plus/Sandboxie/issues/2617)
- 修复了从选项/设置窗口创建的运行菜单条目的问题 [#2610](https://github.com/sandboxie-plus/Sandboxie/issues/2610)
- 修复了使用快照时开始菜单的问题 [#2589](https://github.com/sandboxie-plus/Sandboxie/issues/2589)

## [1.7.0 / 5.62.0] - 2023-01-27

### 新增
- 添加了 OnFileRecovery 触发器，允许在恢复文件之前进行检查 [#2202](https://github.com/sandboxie-plus/Sandboxie/issues/2202)
- 向沙箱选项添加了更多预设
  - 注意：这些只能在盒子为空时更改
- 添加了带有附加设置的新文件迁移选项页面
- 添加了 SBIE2113/SBIE2114/SBIE2115 消息，以指示因预设未迁移的文件
- 添加了 'NoParallelLoading=y' 以禁用 DLL 的并行加载，以调试加载程序问题

### 更改
- 将 SeparateUserFolders 复选框从全局设置移动到每个盒子选项

### 修复
- 解决了 SbieDll.dll 与影子堆栈的不兼容性，并启用了 /CETCOMPAT 对 SbieDll.dll [#2559](https://github.com/sandboxie-plus/Sandboxie/issues/2559)
- 添加缺失的注册表钩子以改善与新应用程序的兼容性
- 修复了隐私模式盒子中注册表条目的权限问题

## [1.6.7 / 5.61.7] - 2023-01-24

### 新增
- 向 Sandboxie Classic 添加了应用支持者证书的选项

### 更改
- 有限时间证书现在延长了 1 个月的有效期，以改善续订体验

### 修复
- 修复了希伯来语语言（经典 UI)的问题 [#2608](https://github.com/sandboxie-plus/Sandboxie/issues/2608)
- 修复了开始菜单集成和快照的问题 [#2589](https://github.com/sandboxie-plus/Sandboxie/issues/2589)

## [1.6.6 / 5.61.6] - 2023-01-16

### 更改
- 重新设计跟踪日志的检索以显著提高性能
- 改进了列表/树查找器
- 改进了跟踪日志
- 为隔离盒子启用了丢弃管理员权限选项
  - 注意：未沙盒模式下启动的程序无法丢弃权限，但沙盒内的 UAC 提示和提升尝试会被阻止

### 修复
- 修复了驱动程序中的潜在 BSoD 问题
- 修复了跟踪日志中的崩溃 [#2599](https://github.com/sandboxie-plus/Sandboxie/issues/2599)
- 修复了托盘问题 [#2600](https://github.com/sandboxie-plus/Sandboxie/pull/2600) (okrc)
- 修复了快捷方式的问题 [#2601](https://github.com/sandboxie-plus/Sandboxie/pull/2601) (okrc)

## [1.6.5 / 5.61.5] - 2023-01-10

### 新增
- 添加用户模式系统调用跟踪，现在系统调用可以在隔离盒子以及所有 Win32k 系统调用中记录
- 向跟踪日志添加 nt 对象视图

### 更改
- 重新设计跟踪日志模型，现在可以在不到一秒的时间内加载超过一百万条条目
- 当缺少适当的 DLL 时，ApiLog 支持现在在跟踪 UI 中隐藏

### 修复
- 修复了 Microsoft Edge 在安全加固盒中的问题 [#2571](https://github.com/sandboxie-plus/Sandboxie/issues/2571)
- 在 ARM64 系统上打开了 OpenIpcPath=\\{BEC19D6F-D7B2-41A8-860C-8787BB964F2D}
- 修复了与 Windows 11 21H2 ARM64 不兼容的问题 [#2431](https://github.com/sandboxie-plus/Sandboxie/issues/2431)

## [1.6.4 / 5.61.4] - 2022-12-31

### 新增
- 添加选项以禁用启动时沙箱清理 [#2553](https://github.com/sandboxie-plus/Sandboxie/issues/2553)
- 在帮助菜单中添加了适用于 Plus 和 Classic UI 的贡献指南 [#2551](https://github.com/sandboxie-plus/Sandboxie/pull/2551)

### 修复
- 修复了 SandMan 启动时自动删除沙箱的问题 [#2555](https://github.com/sandboxie-plus/Sandboxie/issues/2555)
- 修复了 Windows 11 上的上下文菜单问题，当 SandMan 尚未运行时 [#2284](https://github.com/sandboxie-plus/Sandboxie/issues/2284)
- 修复了关于 cmbDefault 的显示问题 [#2560](https://github.com/sandboxie-plus/Sandboxie/pull/2560) (okrc)
- 修复了色彩不准确的问题 [#2570](https://github.com/sandboxie-plus/Sandboxie/pull/2570) (okrc)
- 修复了第一次启动时 Templates.ini 加载的问题 [#2574](https://github.com/sandboxie-plus/Sandboxie/issues/2574)
- 修复了 Cyberpunk 2077 在沙盒中无法加载插件的问题
- 修复了游戏中的性能问题
- 修复了在 Windows 11 ARM64 Build 22621.819 中观察到的 FFS hooking 问题

## [1.6.3 / 5.61.3] - 2022-12-21

### 更改
- 改进了网络位置感知模板

## [1.6.2b / 5.61.2] - 2022-12-21

### 修复
- 修复了当“版本更新”下拉菜单设置为“忽略”时更新标签的问题
- 修复了 32 位 Windows 10 系统上的 WoW64 thunking 问题 [#2546](https://github.com/sandboxie-plus/Sandboxie/issues/2546)
- 修复了沙箱中自动删除选项未工作的情况 [#2531](https://github.com/sandboxie-plus/Sandboxie/issues/2531)
  - 注意：现在每次重新启动后都会进行清理
- 修复了 Windows 11 下网络位置感知的问题 [#2530](https://github.com/sandboxie-plus/Sandboxie/issues/2530)
- 修复了恢复窗口的问题 [#2458](https://github.com/sandboxie-plus/Sandboxie/issues/2458)

## [1.6.2a / 5.61.2] - 2022-12-19

### 修复
- 修复了在 1.6.1a 中引入的复古 UI 崩溃问题

## [1.6.2 / 5.61.2] - 2022-12-19

### 新增
- 添加腾讯 TIM 的模板 [#2516](https://github.com/sandboxie-plus/Sandboxie/pull/2516) (感谢 TooYoungTooSimp)

## [1.6.1b / 5.61.1] - 2022-12-16

### 新增
- 添加游戏/演示模式 [#2534](https://github.com/sandboxie-plus/Sandboxie/issues/2534)
- 添加选择自定义盒子图标的选项

### 修复
- 修复支持设置页面未正确显示版本更新的问题

## [1.6.1a / 5.61.1] - 2022-12-07

### 新增
- 在沙箱菜单中新增“运行沙箱”命令
- 现可用其他沙箱替换 DefaultBox [#2445](https://github.com/sandboxie-plus/Sandboxie/issues/2445)

### 修复
- 改进了对 [#2495](https://github.com/sandboxie-plus/Sandboxie/issues/2495) 的修复



## [1.6.1 / 5.61.1] - 2022-12-04

### 新增
- 新增全局选项，以便将运行菜单条目引入所有沙箱

### 更改
- 重新组织了命令提示符条目 [#2451](https://github.com/sandboxie-plus/Sandboxie/issues/2451)
- “实时”更新频道现在是“预览”频道的一部分，版本更新现可禁用

### 修复
- 修复了支持设置下拉菜单重复出现的问题 [#2502](https://github.com/sandboxie-plus/Sandboxie/pull/2502) (okrc)
- 将翻译文件夹标记为非推荐使用，将在安装时被删除 [#2500](https://github.com/sandboxie-plus/Sandboxie/pull/2500) (lufog)
- 修复了未计算隐藏项目磁盘使用的问题 [#2503](https://github.com/sandboxie-plus/Sandboxie/pull/2503) (okrc)
- 修复了可按进程设置的布尔设置问题 [#2495](https://github.com/sandboxie-plus/Sandboxie/issues/2495)
- 修复了未计算隐藏项目磁盘使用的问题 [#2503](https://github.com/sandboxie-plus/Sandboxie/pull/2503)



## [1.6.0 / 5.61.0] - 2022-11-26

### 新增
- 增加导入/导出沙箱为 .7z 文件的功能
- 新增更新机制，允许增量更新
  - 用于“实时”更新频道中的夜间测试构建
  - 用于安装后更新 Templates.ini 和翻译文件 [#1105](https://github.com/sandboxie-plus/Sandboxie/issues/1105)

### 更改
- 重构了沙箱上下文菜单
- SandMan 的翻译文件现在已压缩并位于 translations.7z 中

### 修复
- 修复了回收图标未及时更新的问题 [#2457](https://github.com/sandboxie-plus/Sandboxie/issues/2457) (感谢 okrc)
- 修复了“索引超出范围”的问题 [#2470](https://github.com/sandboxie-plus/Sandboxie/pull/2470) (感谢 okrc)
- 修复了安装后启动 SandMan 的问题 [#2284](https://github.com/sandboxie-plus/Sandboxie/issues/2284)
- 修复了 my_version.h 的编码问题 [#2475](https://github.com/sandboxie-plus/Sandboxie/issues/2475)
- 修复了空组闪烁序列的问题 [#2486](https://github.com/sandboxie-plus/Sandboxie/pull/2486) (感谢 okrc)



## [1.5.3 / 5.60.3] - 2022-11-08

### 新增
- 设置为自动删除的沙箱现在标记为小红色回收符号
- 在 Plus UI 上新增越南语 [#2438](https://github.com/sandboxie-plus/Sandboxie/pull/2438)

### 修复
- 修复了沙箱选项的问题 [#2400](https://github.com/sandboxie-plus/Sandboxie/issues/2400)
- 修复了 Smart App Control 的问题 [#2341](https://github.com/sandboxie-plus/Sandboxie/issues/2341)
- 修复了使用隐私沙箱时快照的问题 [#2427](https://github.com/sandboxie-plus/Sandboxie/issues/2427)
- 修复了 m_pColorslider 更改未应用的问题 [#2433](https://github.com/sandboxie-plus/Sandboxie/pull/2433) (感谢 okrc)
- 修复了当文件面板打开时切换快照的问题
- 修复了选择空沙箱时文件面板的问题 [#2419](https://github.com/sandboxie-plus/Sandboxie/issues/2419)
- 修复了菜单图标未禁用的问题 [#2406](https://github.com/sandboxie-plus/Sandboxie/issues/2406)
- 修复了与 Microsoft Edge 的兼容性问题 [#2312](https://github.com/sandboxie-plus/Sandboxie/issues/2312)
- 修复了 UsePrivacyMode=y 与 Windows 7 的兼容性问题 [#2423](https://github.com/sandboxie-plus/Sandboxie/issues/2423)
- 修复了 Sandboxie Plus 卸载时的一个小问题 [#2421](https://github.com/sandboxie-plus/Sandboxie/issues/2421)
- 修复了驱动初始化失败引起的 BSoD 问题（在 1.5.1 中引入) [#2431](https://github.com/sandboxie-plus/Sandboxie/issues/2431)
- 修复了即使真实路径不存在也会列出虚假路径的问题 [#2403](https://github.com/sandboxie-plus/Sandboxie/issues/2403)
- 修复了 Firefox 106.x 请求对插件可执行文件的写入访问问题 [#2391](https://github.com/sandboxie-plus/Sandboxie/issues/2391) [#2411](https://github.com/sandboxie-plus/Sandboxie/issues/2411)



## [1.5.2 / 5.60.2] - 2022-10-28

### 更改
- 实用组现在会自动清理
- 改进了残余物的删除 [#2409](https://github.com/sandboxie-plus/Sandboxie/pull/2409)

### 修复
- 修复了在虚拟驱动器上的 SBIE2227 问题
- 修复了与 1.5.0 引入的本机 32 位 Windows 的问题 [#2401](https://github.com/sandboxie-plus/Sandboxie/issues/2401)



## [1.5.1 / 5.60.1] - 2022-10-26

### 新增
- 当沙箱位于不支持 8.3 名称的卷上时，新增 SBIE2227 警告
- 为腾讯 QQ 新增模板 [#2367](https://github.com/sandboxie-plus/Sandboxie/issues/2367)

### 修复
- 修复了使用自定义路径重命名沙箱时的问题 [#2368](https://github.com/sandboxie-plus/Sandboxie/issues/2368)
- 正确修复了 Firefox 106 的问题 [46e9979](https://github.com/sandboxie-plus/Sandboxie/commit/46e99799e2b753b85224dc921ec141a05025acfd)
- 修复了替代 UI 模式的问题 [#2380](https://github.com/sandboxie-plus/Sandboxie/issues/2380)
- 修复了与突围进程的命令行损坏的问题 [#2377](https://github.com/sandboxie-plus/Sandboxie/issues/2377)
- 修复了与隐私增强沙箱类型的问题 [#2342](https://github.com/sandboxie-plus/Sandboxie/issues/2342)
- 修复了沙箱对象目录初始化的问题 [#2342](https://github.com/sandboxie-plus/Sandboxie/issues/2342)
- Sandboxie 不再留下永久的目录对象
- 修复了安全问题 ID-21 AlpcConnectPortEx 未被驱动筛选 [#2396](https://github.com/sandboxie-plus/Sandboxie/issues/2396)
- 修复了程序控制选项的问题 [#2395](https://github.com/sandboxie-plus/Sandboxie/issues/2395)



## [1.5.0 / 5.60.0] - 2022-10-19

### 新增
- 支持 Windows ARM64 [#1321](https://github.com/sandboxie-plus/Sandboxie/issues/1321) [#645](https://github.com/sandboxie-plus/Sandboxie/issues/645)
  - 移植 SbieDrv 为 ARM64
  - 移植低级注入机制为 ARM64/ARM64EC
  - 移植系统调用钩子为 ARM64/ARM64EC
  - 移植 SbieDll.dll 为 ARM64/ARM64EC
  - 注意：ARM32 在 ARM64 上未实现，将终止并显示消息 SBIE2338
  - 注意：当 Sandboxie 正在运行时，它会全局禁用对 x86 进程的 CHPE 二进制文件的使用 - 这是强制进程功能所需的。这可以通过将全局选项 "DisableCHPE=n" 添加到 Sandboxie.ini 来禁用，这将终止在沙箱外启动的 x86 进程，并显示消息 SBIE2338，而不是强制执行

### 更改
- 重新设计了 API 兼容性检查
- 突围进程现在对所有用户可用

### 修复
- 修复了在 x86 应用程序中的 Win32 钩子问题
- 编辑模板时避免窗口重叠 [#2339](https://github.com/sandboxie-plus/Sandboxie/pull/2339) (感谢 okrc)
- 修复了 OpenWinClass UI 设置的错误写入 [#2347](https://github.com/sandboxie-plus/Sandboxie/pull/2347) (感谢 okrc)
- 修复了本地模板的问题 [#2338](https://github.com/sandboxie-plus/Sandboxie/pull/2338) (感谢 okrc)
- 修复了与 Edge WebView2 的兼容性问题 [#2350](https://github.com/sandboxie-plus/Sandboxie/issues/2350)
- 为 Firefox 106 内容进程沙箱问题添加临时解决方法
- 修复了重命名沙箱的问题 [#2358](https://github.com/sandboxie-plus/Sandboxie/pull/2358) (感谢 okrc)



## [1.4.2 / 5.59.2] - 2022-10-10

### 新增
- 为资源访问模式新增工具提示 [#2300](https://github.com/sandboxie-plus/Sandboxie/issues/2300)
- 新增控制 ApplyElevateCreateProcessFix 的 UI 选项 [#2302](https://github.com/sandboxie-plus/Sandboxie/issues/2302)
- 如果某个进程需要 'ApplyElevateCreateProcessFix=y'，新增消息 2226

### 更改
- 将限制选项卡从新安全页面移回常规页面

### 修复
- 修复了突围进程页面中的按钮标题错误
- 修复了保存沙箱恢复选项的问题
- 修复了 SandMan 的显示问题 [#2306](https://github.com/sandboxie-plus/Sandboxie/pull/2306) (感谢 okrc)
- 修复了主题未自动更改的问题 [#2307](https://github.com/sandboxie-plus/Sandboxie/pull/2307)
- 修复了在 Windows 关机时保存 SandMan 窗口状态的问题
- 修复了其他小问题 [#2301](https://github.com/sandboxie-plus/Sandboxie/issues/2301)
- 修复了 106.x 中引入的与 Microsoft Edge 的问题 [#2325](https://github.com/sandboxie-plus/Sandboxie/issues/2325)
- 修复了 Vivaldi hooking 问题，UseVivaldiWorkaround 现在不再需要 [#1783](https://github.com/sandboxie-plus/Sandboxie/issues/1783)
- 修复了高级选项页面上杂项选项卡的问题 [#2315](https://github.com/sandboxie-plus/Sandboxie/issues/2315#issuecomment-1272544086)



## [1.4.1 / 5.59.1] - 2022-10-05

### 新增
- 新增对 Windows 11 的深色标题栏支持 [#2299](https://github.com/sandboxie-plus/Sandboxie/pull/2299)

### 更改
- 在 Sbie 5.28 及更高版本中，WinInetCache 是开启的，这会破坏 IE 的源视图，因此现在可以通过 'CloseWinInetCache=y' 禁用

### 修复
- 修复了 WarnProcess 和 WarnFolder 在某些配置下无法正常工作的的问题



## [1.4.0 / 5.59.0] - 2022-09-30

### 新增
- 新增从开始菜单集成运行的功能 [#1836](https://github.com/sandboxie-plus/Sandboxie/issues/1836)
- 新增开始菜单枚举 [#1570](https://github.com/sandboxie-plus/Sandboxie/issues/1570)
- 新增突围进程的 UI [#1904](https://github.com/sandboxie-plus/Sandboxie/issues/1904)
- 新增每个沙箱自定义双击操作的选项
- 新增高级选项卡中的杂项选项卡，允许配置特定进程和其他高级选项
- 在沙箱选项对话框中新增 "SeparateUserFolders=y" 和 "SandboxieLogon=y"
- 在选项页面的部分标签中新增图标

### 更改
- 为 Qt 6.3.1 做准备
- 重新构建常规设置页面
- 重新构建沙箱选项页面，新增一个标签并将一些高级选项移至那里

### 修复
- 修复了 Plus UI 中菜单栏的问题 [#2280](https://github.com/sandboxie-plus/Sandboxie/pull/2280) (感谢 okrc)



## [1.3.5 / 5.58.5] - 2022-09-26

### 新增
- 为 Windows 11 Shell 菜单新增本地化 [#2229](https://github.com/sandboxie-plus/Sandboxie/issues/2229)

### 更改
- 改进了 Plus UI 中恢复窗口的行为 [#2266](https://github.com/sandboxie-plus/Sandboxie/issues/2266)

### 修复
- 修复了 Sandboxie-Plus.ini 中的过期数据问题 [#2248](https://github.com/sandboxie-plus/Sandboxie/pull/2248) (感谢 okrc)
- 修复了虚假清单的问题 [#2252](https://github.com/sandboxie-plus/Sandboxie/issues/2252)
- 修复了 XYplorer 的问题 [#2230](https://github.com/sandboxie-plus/Sandboxie/issues/2230)
- 修复了 Plus UI 中的崩溃问题 [e9e21c2](https://github.com/sandboxie-plus/Sandboxie/commit/e9e21c215ed87cf1d5aa999407ab2f99d5da9e4a)
- 修复了当 NoIcons=1 时 m_pCleanUpButton 显示为空的问题 [#2273](https://github.com/sandboxie-plus/Sandboxie/pull/2273) (感谢 okrc)

## [1.3.4 / 5.58.4] - 2022-09-19

### 添加
- 在 Plus UI 中添加了 NoRenameWinClass
- 将 Windows.UI.* 添加到硬编码的已知类列表中，以解决 WinUI 应用程序的问题 [#2109](https://github.com/sandboxie-plus/Sandboxie/issues/2109)

### 更改
- NoRenameWinClass 现在支持通配符

### 修复
- 修复了启动时未检测到默认箱的问题 [#2195](https://github.com/sandboxie-plus/Sandboxie/issues/2195)
- 修复了移动沙盒菜单的问题 [#2225](https://github.com/sandboxie-plus/Sandboxie/issues/2225)（感谢 okrc)
- 修复了 Sandboxie-Plus.ini 中过期数据的问题 [#2234](https://github.com/sandboxie-plus/Sandboxie/pull/2234)（感谢 okrc)
- 修复了自动启动问题 [#2219](https://github.com/sandboxie-plus/Sandboxie/issues/2219)
- 修复了防火墙 UI 问题，所有程序条目缺少 * 前缀 [#2247](https://github.com/sandboxie-plus/Sandboxie/issues/2247)
- 修复了 BlockPorts 模板中缺少 * 前缀的问题 [4420ba4](https://github.com/sandboxie-plus/Sandboxie/commit/4420ba4448a797b7369917058c34e8a78c2ec9fc)
- 修复了与各种 Electron 应用程序相关的问题 [#2217](https://github.com/sandboxie-plus/Sandboxie/issues/2217) [#2235](https://github.com/sandboxie-plus/Sandboxie/issues/2235) [#2201](https://github.com/sandboxie-plus/Sandboxie/issues/2201) [#2166](https://github.com/sandboxie-plus/Sandboxie/issues/2166)
  - 现在默认行为为 UseElectronWorkaround=n

### 删除
- 删除了过时的 VPNTunnel 模板



## [1.3.3 / 5.58.3] - 2022-09-12

### 添加
- 当 LogFile 注册设置应用作为解决方法时，添加了 domain\user 表示法 [#2207](https://github.com/sandboxie-plus/Sandboxie/issues/2207)
  - 用法：在 "HKLM\SYSTEM\CurrentControlSet\Services\SbieSvc" 中添加 REG_SZ "LogFile"，值为 "3;[path]\Sandboxie.log"
- 添加了阻止主机进程访问沙盒进程的选项 [#2132](https://github.com/sandboxie-plus/Sandboxie/issues/2132)
  - 用法：DenyHostAccess=Program.exe,y
  - 注意：默认情况下，该保护仅适用于写访问，也就是说，未沙盒进程仍然可以获得只读访问权限
  - 若要防止主机进程获得读取权限，还必须设置 ConfidentialBox=y，这需要支持者证书
- 为 ReHIPS 添加了兼容性模板
- 添加了在隐私箱类型中创建所有默认文件夹的功能 [#2218](https://github.com/sandboxie-plus/Sandboxie/issues/2218)

### 更改
- 改进了非管理员用户的 SandMan 设置行为 [#2123](https://github.com/sandboxie-plus/Sandboxie/issues/2123)

### 修复
- 修复了通过拖放移动组时的问题
- 批准了更多所需的系统调用 [#2190](https://github.com/sandboxie-plus/Sandboxie/issues/2190)
- 修复了删除箱内容时文件面板视图打开时的问题
- 修复了配置保护的问题 [#2206](https://github.com/sandboxie-plus/Sandboxie/issues/2206)
- 修复了默认箱的问题 [#2195](https://github.com/sandboxie-plus/Sandboxie/issues/2195)
- 修复了用于进程终止的键盘删除快捷键的问题

### 删除
- 删除了过时的 Online Armor 模板



## [1.3.2 / 5.58.2] - 2022-08-30

### 添加
- 在箱选项对话框的子标签中添加了图标
- 恢复和消息弹出菜单选项现在不再跨 UI 重启保持
- 添加了新的箱颜色，白色箱表示它并不是真正的沙盒，并在用户指定 OpenFilePath=* 或类似时显示

### 更改
- Sandboxie 在强制进程暂时禁用时不再发出消息 1301
  - 可以通过 "NotifyForceProcessDisabled=y" 重新启用该消息
- 重新设计了 Plus UI 中的 "Open COM" 复选框机制
  - 现在它使用模板，并且在 OpenIpcPath=* 设置时，也可以保持 COM 关闭

### 修复
- 修复了与 Proxifier 的兼容性问题 [#2163](https://github.com/sandboxie-plus/Sandboxie/issues/2163)
- 修复了韩语翻译中的编码问题 [#2173](https://github.com/sandboxie-plus/Sandboxie/pull/2173)
- 修复了可用更新消息的问题



## [1.3.1 / 5.58.1] - 2022-08-20

### 添加
- 添加了独立于暗黑主题切换融合主题的能力
- 添加了从支持页下载更新的能力
- 将缺失的系统调用添加到增强版箱类型 [88bc06a](https://github.com/sandboxie-plus/Sandboxie/commit/88bc06a0c7368a81c80a77d7a89ddc73455abb25) [b775264](https://github.com/sandboxie-plus/Sandboxie/commit/b775264a4824e49b554f1b776c377170e5f90797) [04b2377](https://github.com/sandboxie-plus/Sandboxie/commit/04b23770f53597c12eda9122c774ed5165129147)（感谢 Mr.X)
- 在 Plus UI 设置和箱选项对话框中添加了搜索框 [#2134](https://github.com/sandboxie-plus/Sandboxie/issues/2134)
- 将韩语翻译添加到 Plus UI [#2133](https://github.com/sandboxie-plus/Sandboxie/pull/2133)（感谢 VenusGirl)
- 将沙盒管理器（SandMan)托盘菜单添加了分组功能 [#2148](https://github.com/sandboxie-plus/Sandboxie/issues/2148)

### 更改
- 改进了信息标签
- 复古模式的外观现在更加复古
- 使用 SandMan 命令 "Options -> Reload INI file" 重新加载配置时，现在会更新已批准的系统调用列表
- 使规则特异性更加具体，现在具有更少通配符的规则将覆盖更多通配符的规则
  - 注意：尾部通配符是单独评估的

### 修复
- 修复了显示沙盒配置的问题 [#2111](https://github.com/sandboxie-plus/Sandboxie/issues/2111)
- 修复了在切换视图时出现的闪烁问题 [#2050](https://github.com/sandboxie-plus/Sandboxie/issues/2050)
- 修复了 Plus UI 中各种复选框的不一致问题 [ef4ac1b](https://github.com/sandboxie-plus/Sandboxie/commit/ef4ac1b6b34d505e46515e9aabb98411a9b1751e) [06c89e3](https://github.com/sandboxie-plus/Sandboxie/commit/06c89e3f45036f593fed7a0d0d59d54313e8ca77)
- 修复了证书验证问题 [238cb44](https://github.com/sandboxie-plus/Sandboxie/commit/238cb44969923479148e210814ab91d2428ec4b0)
- 修复了 "UseRuleSpecificity" 设置的问题 [#2124](https://github.com/sandboxie-plus/Sandboxie/issues/2124) [file.c#L965-L966](https://github.com/sandboxie-plus/Sandboxie/blob/ff759692a222cf7c492cb4d5cfd76c79fbde1c2b/Sandboxie/core/drv/file.c#L965-L966)



## [1.3.0 / 5.58.0] - 2022-08-09

### 添加
- 添加了 ntoskrnl/ntdll 的钩子配置
  - 可以使用 "DisableWinNtHook=..." 禁用单个 ntdll 钩子
- 添加了一种新的安全增强箱模式；添加 "UseSecurityMode=y" 来启用
  - 启用此设置时，组合了 "SysCallLockDown=y"，限制使用 NT 系统调用，以及 "DropAdminRights=y" 和 "RestrictDevices=y"
  - 仅在全局部分配置为 "ApproveWinNtSysCall=..."/"ApproveWin32SysCall=..." 的调用将使用原始令牌执行
  - 所有未获得批准的 NT 系统调用将使用沙盒令牌执行，这在某些场景中可能导致兼容性问题
  - 可能还需要允许额外的系统调用，这必须在 [GlobalSettings] 中完成，且必须重新启动驱动程序
  - 注意：使用先前版本创建的安全增强箱现在将在 UI 中显示为正常
  - 安全增强图标现在被重新用于新的超级极安全增强箱模式
  - 注意：新的增强安全功能需要支持者证书
- 为 "强制进程" 标签添加浏览选项

### 更改
- 将 "DeviceSecurity" 模板替换为专用设置 "RestrictDevices=y"
  - 注意：根据需要，可以添加更多 "NormalFilePath=..." 条目以打开特定设备
- 规则特异性现在更加具体，精确规则现在会覆盖以通配符结尾的规则



## [1.2.8b / 5.57.7] - 2022-08-08

### 修复
- 修复了在 Windows 11 上上下文菜单设置的问题
- 修复了复古模式中的列问题 [#2103](https://github.com/sandboxie-plus/Sandboxie/issues/2103)



## [1.2.8 / 5.57.7] - 2022-08-05

### 修复
- 修复了 SandMan 中缺失的卸载程序



## [1.2.7 / 5.57.7] - 2022-07-31

### 添加
- 在所有列表中添加了交替行颜色的选项 [#2073](https://github.com/sandboxie-plus/Sandboxie/issues/2073)

### 更改
- SandboxieLogon 默认情况下现在禁用，因为它与第三方恶意软件工具不兼容 [#2025](https://github.com/sandboxie-plus/Sandboxie/issues/2025)
- 访问视图列表现在会在文件和键路径末尾添加尾部 "*"，与驱动程序方式相同 [2039](https://github.com/sandboxie-plus/Sandboxie/issues/2039)
- Shell 集成的设置现在由 SandMan 完成，而不是安装程序
- 卸载程序现在可以删除沙盒文件夹 [#1235](https://github.com/sandboxie-plus/Sandboxie/pull/1235)



## [1.2.6 / 5.57.6] - 2022-07-25

### 更改
- 重新设计了全局选项的保存

### 修复
- 修复了删除内容选项的问题 [#2043](https://github.com/sandboxie-plus/Sandboxie/issues/2043)
- 修复了箱首选项的问题 [#2046](https://github.com/sandboxie-plus/Sandboxie/issues/2046)
- 修复了 Delete V2 注册表的问题



## [1.2.5 / 5.57.5] - 2022-07-22

### 更改
- 改进了一些图标

### 修复
- 修复了证书验证问题



## [1.2.4 / 5.57.4] - 2022-07-21

### 添加
- 在恢复窗口中添加了删除按钮 [#2024](https://github.com/sandboxie-plus/Sandboxie/issues/2024)

### 更改
- 改进了树形选择显示

### 修复
- 修复了文件面板的问题
- 修复了一些键绑定的问题 [#2030](https://github.com/sandboxie-plus/Sandboxie/issues/2030)
- 修复了终止 SbieSvc 时的 RemoveSidName 问题
- 修复了新钩子机制的问题
- 修复了 1.2.0 中引入的与 Win32k 钩子相关的蓝屏问题 [#2035](https://github.com/sandboxie-plus/Sandboxie/issues/2035)
- 修复了 Element 1.11 和 Electron 解决方法的问题 [#2023](https://github.com/sandboxie-plus/Sandboxie/issues/2023)



## [1.2.3 / 5.57.3] - 2022-07-13

### 修复
- 修复了与新菜单代码相关的问题

### 更改
- 重新设计了框架绘制



## [1.2.2 / 5.57.2] - 2022-07-13

### 修复
- 修复了框架绘制相关的问题
- 修复了上一个版本中引入的托盘和箱菜单的问题
- 从树形列表中移除了焦点矩形

### 更改
- 重构了菜单创建代码

## [1.2.1 / 5.57.1] - 2022-07-11

### 新增
- 增加了 Plus UI 的瑞典语翻译（感谢 pb1)
- 增加了复古视图模式，使 SandMan UI 看起来像 SbieCtrl
- 增加了替代的托盘菜单机制
- 增加了根据边框颜色自动生成沙箱图标的功能

### 更改
- 更改了盒子组图标为专用图标
- “浏览内容”现在作为主窗口的侧边面板可用
- 动画时钟图标叠加

### 修复
- 修复了 Windows 7 上的 DPI 问题
- 修复了软件兼容性选项卡的问题
- 修复了在构建 1.1.1 中引入的 OpenKeyPath 问题 [#2006](https://github.com/sandboxie-plus/Sandboxie/issues/2006)


## [1.2.0 / 5.57.0] - 2022-06-28

### 新增
- 重新设计了 “SandboxieLogon=y”; 默认启用，因为每个沙箱现在都有自己的 SID
  - 注意：这强制执行沙箱之间的严格隔离

### 更改
- 重新构建了钩子管理，已卸载的 DLL 现在可以正确取消钩住 [#1243](https://github.com/sandboxie-plus/Sandboxie/issues/1243)
- 盒子排序现在存储在 Sandboxie-Plus.ini 中
- 改进了 DPI 缩放行为


## [1.1.3 / 5.56.3] - 2022-06-20

### 新增
- 增加了组优先排序 [#1922](https://github.com/sandboxie-plus/Sandboxie/issues/1922)

### 更改
- 更新了经典 UI 的瑞典语翻译（感谢 pb1)
- 恢复了 Plus UI 的土耳其语翻译 [#1419](https://github.com/sandboxie-plus/Sandboxie/issues/1419)（感谢 fmbxnary)

### 修复
- 修复了删除时恢复窗口的问题 [#1948](https://github.com/sandboxie-plus/Sandboxie/issues/1948)
- 修复了路径列的双击问题 [#1951](https://github.com/sandboxie-plus/Sandboxie/issues/1951)
- “AllowBoxedJobs=n” 恢复为默认行为，因为已报告问题 [#1954](https://github.com/sandboxie-plus/Sandboxie/issues/1954)
- 修复了互联网阻止的问题 [#1955](https://github.com/sandboxie-plus/Sandboxie/issues/1955)
- 修复了 Plus UI 中的分组问题 [#1950](https://github.com/sandboxie-plus/Sandboxie/issues/1950)
- 修复了在 Windows 11 中使用 Win32k 钩子时 CredentialUIBroker.exe 的问题 [#1839](https://github.com/sandboxie-plus/Sandboxie/issues/1839)
- 修复了 Delete V2 的问题 [#1939](https://github.com/sandboxie-plus/Sandboxie/issues/1939)


## [1.1.2 / 5.56.2] - 2022-06-14

### 新增
- 将缺失的文件恢复日志添加到 SandMan [#425](https://github.com/sandboxie-plus/Sandboxie/issues/425)
- 当所有文件恢复完毕时，立即恢复窗口将自动关闭 [#1498](https://github.com/sandboxie-plus/Sandboxie/issues/1498)
- SandMan 的立即恢复窗口默认为总在最上方，类似于 SbieCtrl；可通过 “Options/RecoveryOnTop=n” 禁用 [#1465](https://github.com/sandboxie-plus/Sandboxie/issues/1465)
- 增加了从预设子菜单切换立即恢复的选项 [#1653](https://github.com/sandboxie-plus/Sandboxie/issues/1653)
- 增加了全局禁用文件恢复和消息弹出窗口的选项
- 增加了每个盒子的刷新选项 [#1945](https://github.com/sandboxie-plus/Sandboxie/issues/1945)

### 更改
- Chrome、Firefox 和 Acrobat 的桌面安全解决方法现在可以通过 “UseSbieDeskHack=y” 启用用于所有进程
- 改进了双击行为 [#1935](https://github.com/sandboxie-plus/Sandboxie/issues/1935)
- 文件恢复时刷新盒子大小信息

### 修复
- 修复了在 1.1.1 中引入的不必要的 Sandboxie 配置重载问题 [#1938](https://github.com/sandboxie-plus/Sandboxie/issues/1938)
- 修复了恢复窗口焦点的问题 [#1374](https://github.com/sandboxie-plus/Sandboxie/issues/1374)
- 修复了在 1.1.1 中引入的桌面对象问题 [#1934](https://github.com/sandboxie-plus/Sandboxie/issues/1934)
- 修复了使用 GPO 预设时 Edge 启动提升的问题 [#1913](https://github.com/sandboxie-plus/Sandboxie/issues/1913)


## [1.1.1 / 5.56.1] - 2022-06-07

### 新增
- 兼容性模板现在可以从设置窗口查看 [#1891](https://github.com/sandboxie-plus/Sandboxie/issues/1891)
- 刷新命令现在绑定到 F5 [#1885](https://github.com/sandboxie-plus/Sandboxie/issues/1885)
- 增加了更多首次启动向导选项
- 增加了一个选项以在任何给定盒子打开时永久禁用立即恢复 [#1478](https://github.com/sandboxie-plus/Sandboxie/issues/1478)
- 双击路径列现在会在 Windows Explorer 中打开盒子根目录 [#1924](https://github.com/sandboxie-plus/Sandboxie/issues/1924)

### 更改
- 更改了移动盒子的行为 [#1879](https://github.com/sandboxie-plus/Sandboxie/issues/1879)
- 改进了 PreferExternalManifest 选项的实现
- Win32k 钩子现在默认仅用于 Edge 和 Chromium 应用程序，因为它们会导致其他软件出现问题 [#1902](https://github.com/sandboxie-plus/Sandboxie/issues/1902) [#1912](https://github.com/sandboxie-plus/Sandboxie/issues/1912) [#1897](https://github.com/sandboxie-plus/Sandboxie/issues/1897)
- “AllowBoxedJobs=y” 现在为默认行为

### 修复
- 修复了 Windows 11 上与 KB5014019 相关的 Edge 问题
- 修复了在使用 “SeparateUserFolders=y” 时与新 Delete V2 机制相关的问题 [#1885](https://github.com/sandboxie-plus/Sandboxie/issues/1885)
- 修复了凭证问题 [#1770](https://github.com/sandboxie-plus/Sandboxie/pull/1770)
- 修复了强制进程优先级 [#1883](https://github.com/sandboxie-plus/Sandboxie/issues/1883)
- 修复了与新 Delete V2 机制相关的问题
- 修复了在较旧版本的 Windows 上的 Windows 11 菜单的问题 [#1877](https://github.com/sandboxie-plus/Sandboxie/issues/1877)
- 刷新现在在没有 WatchBoxSize 选项的情况下也能正常工作 [#1885](https://github.com/sandboxie-plus/Sandboxie/issues/1885)
- 修复了 WatchBoxSize=true 导致的崩溃问题 [#1885](https://github.com/sandboxie-plus/Sandboxie/issues/1885)
- 修复了恢复文件夹路径的问题 [#1840](https://github.com/sandboxie-plus/Sandboxie/issues/1840)
- 修复了影响 Acrobat Reader 的 Sbie 桌面和 wndStation 问题 [#1863](https://github.com/sandboxie-plus/Sandboxie/issues/1863)
- 修复了盒子分组问题 [#1921](https://github.com/sandboxie-plus/Sandboxie/issues/1921) [#1920](https://github.com/sandboxie-plus/Sandboxie/issues/1920]
- 修复了更改语言时的问题 [#1914](https://github.com/sandboxie-plus/Sandboxie/issues/1914)
- 修复了 BreakoutFolder 的问题 [#1908](https://github.com/sandboxie-plus/Sandboxie/issues/1908)
- 修复了 x86 异常处理的 SbieDll.dll 问题
- 修复了影响 Visual Studio 的应用程序特定的注册表 hive (RegLoadAppKey) 问题 [#1576](https://github.com/sandboxie-plus/Sandboxie/issues/1576) [#1452](https://github.com/sandboxie-plus/Sandboxie/issues/1452]


## [1.1.0 / 5.56.0] - 2022-05-24

### 新增
- 增加了对 NtRenameKey 的支持（这需要 UseRegDeleteV2=y) [#205](https://github.com/sandboxie-plus/Sandboxie/issues/205)
- 增加了盒子大小信息 [#1780](https://github.com/sandboxie-plus/Sandboxie/issues/1780)

### 更改
- 重新构建了 Sandboxie 用于标记主机文件为已删除的机制
  - 在新的行为下，会在盒子根目录中创建数据文件（FilePaths.dat)，而不是虚拟文件
  - 可以通过 “UseFileDeleteV2=y” 启用，对注册表同样适用 “UseRegDeleteV2=y”，这将创建一个注册表文件（RegPaths.dat)
- 重新构建了 TlsNameBuffer 机制，使其更灵活，减少错误
- 显著减少了 SandMan.exe 的 CPU 使用率

### 修复
- 修复了文件夹重命名问题（这需要 UseFileDeleteV2=y) [#71](https://github.com/sandboxie-plus/Sandboxie/issues/71)
- 修复了进程访问的问题 [#1603](https://github.com/sandboxie-plus/Sandboxie/issues/1603)
- 修复了翻译问题 [#1864](https://github.com/sandboxie-plus/Sandboxie/issues/1864)
- 修复了盒子选择窗口的 UI 问题 [#1867](https://github.com/sandboxie-plus/Sandboxie/issues/1867)
- 修复了切换语言时的 UI 问题 [#1871](https://github.com/sandboxie-plus/Sandboxie/issues/1871)


## [1.0.22 / 5.55.22] - 2022-05-15

### 新增
- 增加了 SandMan.exe 的自动更新下载和静默安装选项 [#917](https://github.com/sandboxie-plus/Sandboxie/issues/917)
- 跟踪监视模式现在可以保存到文件 [#1851](https://github.com/sandboxie-plus/Sandboxie/issues/1851)
- 跟踪日志现在显示 IPC 对象类型信息
- 增加了对 Windows 11 上下文菜单的支持

### 修复
- 修复了 SandMan 崩溃问题 [#1846](https://github.com/sandboxie-plus/Sandboxie/issues/1846)
- 修复了 Windows Server 2022 构建 20348的问题
- 修复了翻译切换问题 [#1852](https://github.com/sandboxie-plus/Sandboxie/issues/1852)


## [1.0.21 / 5.55.21] - 2022-05-10

### 新增
- 增加了 “FuncSkipHook=FunctionName” 选项，以选择性禁用某些函数钩子

### 更改
- 改进了支持证书条目框
- 更改语言不再需要在 Plus UI 上重启
- 修复了使用 SbieCtrl 更改设置时的高 CPU 负载问题

### 修复
- 修复了使用 MinGW 工具链编译的 Firefox/Chromium 浏览器的问题 [#538](https://github.com/sandboxie-plus/Sandboxie/issues/538)
- 修复了 Plus UI 上的文件夹恢复问题 [#1840](https://github.com/sandboxie-plus/Sandboxie/issues/1840) [#1380](https://github.com/sandboxie-plus/Sandboxie/issues/1380)


## [1.0.20 / 5.55.20] - 2022-05-02

### 修复
- 修复了在上一版本中引入的 Firefox 视频播放问题 [#1831](https://github.com/sandboxie-plus/Sandboxie/issues/1831)
- 修复了与驱动程序相关的蓝屏死机问题 [#1811](https://github.com/sandboxie-plus/Sandboxie/issues/1811)
- 修复了编辑启动限制条目的问题
- 修复了网络选项卡的问题 [#1825](https://github.com/sandboxie-plus/Sandboxie/issues/1825)
- 修复了如果 SandMan 以管理员身份运行的便携模式问题 [#1764](https://github.com/sandboxie-plus/Sandboxie/issues/1764)


## [1.0.19 / 5.55.19] - 2022-04-21

### 新增
- 增加了对组的拖放支持 [#1775](https://github.com/sandboxie-plus/Sandboxie/issues/1775)
- 增加了对所有条目类型的删除键支持 [#1779](https://github.com/sandboxie-plus/Sandboxie/issues/1779)
- 当尝试在打开了 COM 的盒子中运行 explorer.exe 时给予警告 [#1716](https://github.com/sandboxie-plus/Sandboxie/issues/1716)

### 修复
- 修复了 SandMan UI 中的崩溃问题 [#1772](https://github.com/sandboxie-plus/Sandboxie/issues/1772)
- 修复了在启用 EnableObjectFiltering 的情况下某些安装程序的问题 [#1795](https://github.com/sandboxie-plus/Sandboxie/issues/1795)
- 修复了允许 NtCreateSymbolicLinkObject 在沙箱中安全使用
- 增加了对 Vivaldi 钩子问题的解决方法 [#1783](https://github.com/sandboxie-plus/Sandboxie/issues/1783)
  - 注意：该修复是临时的，可以通过 UseVivaldiWorkaround=n 禁用
- 修复了与快照相关的注册表问题 [#1782](https://github.com/sandboxie-plus/Sandboxie/issues/1782)
- 修复了盒子分组问题 [#1778](https://github.com/sandboxie-plus/Sandboxie/issues/1778) [#1777](https://github.com/sandboxie-plus/Sandboxie/issues/1777) [#1776](https://github.com/sandboxie-plus/Sandboxie/issues/1776)
- 修复了与盒子分组相关的进一步问题 [#1698](https://github.com/sandboxie-plus/Sandboxie/issues/1698) [#1697](https://github.com/sandboxie-plus/Sandboxie/issues/1697)
- 修复了快照 UI 的问题 [#1696](https://github.com/sandboxie-plus/Sandboxie/issues/1696) [#1695](https://github.com/sandboxie-plus/Sandboxie/issues/1695)
- 修复了恢复对话框焦点的问题 [#1374](https://github.com/sandboxie-plus/Sandboxie/issues/1374)


## [1.0.18 / 5.55.18] - 2022-04-13

### 新增
- 将小型浏览器添加到 BlockSoftwareUpdaters 模板中（由 APMichael 提供) [#1784](https://github.com/sandboxie-plus/Sandboxie/pull/1784)

### 更改
- 默认情况下，失败的内存读取尝试对未沙箱化的进程不再导致消息 2111
  - 注意：仍然可以通过设置中的 “NotifyProcessAccessDenied=y” 来启用该消息
- 重新排序了 BlockSoftwareUpdaters 模板（由 APMichael 提供) [#1785](https://github.com/sandboxie-plus/Sandboxie/pull/1785)

### 修复
- 修复了在隔室模式中管道模拟的问题
- 修复了近期构建中引入的盒子清理问题
- 修复了丢失的跟踪日志清理命令 [#1773](https://github.com/sandboxie-plus/Sandboxie/issues/1773)
- 修复了无法取消固定已钉到运行菜单的程序的问题 [#1694](https://github.com/sandboxie-plus/Sandboxie/issues/1694)


## [1.0.17 / 5.55.17] - 2022-04-02

### 新增
- 增加了复选框以轻松访问未沙箱化进程的内存（旧 Sbie 行为，不推荐)

### 更改
- 改进了 OpenProcess/OpenThread 日志记录

### 修复
- 修复了新监控模式中的崩溃问题
- 修复了资源访问条目解析的问题


## [1.0.16 / 5.55.16] - 2022-04-01

### 新增
- 修复的安全问题 ID-20：无法再读取未沙箱化进程的内存，可能会出现例外
  - 可以使用 ReadIpcPath=$:program.exe 允许读取未沙箱化进程或其他盒子的进程的访问权限
- 将 “监控模式” 添加到资源访问跟踪中，类似于旧 SbieCtrl.exe 的监视视图

### 更改
- EnableObjectFiltering 现在默认设置为启用，并替换了 Sbie 的旧进程/线程句柄过滤器
- `$:` 语法现在接受通配符 `$:*`，不再有特殊通配符

### 修复
- 修复了 NtGetNextProcess 被完全禁用而非正确过滤的问题
- 修复了在沙箱中创建新进程时重新构建的图像名称解析问题
- 修复了与 HideOtherBoxes=y 相关的回归问题 [#1743](https://github.com/sandboxie-plus/Sandboxie/issues/1743) [#1666](https://github.com/sandboxie-plus/Sandboxie/issues/1666)


## [1.0.15 / 5.55.15] - 2022-03-24

### 修复
- 修复了最近构建中引起的内存损坏，导致 Chrome 有时崩溃
- 修复的安全问题 ID-18：NtCreateSymbolicLinkObject 未被过滤（感谢 Diversenok)


## [1.0.14 / 5.55.14] - 2022-03-23

### 新增
- 增加通知以警告默认更新检查器落后于 GitHub 上的最新版本，以确保只有无缺陷的构建被提供为更新 [#1682](https://github.com/sandboxie-plus/Sandboxie/issues/1682)
- 将主要浏览器添加到 BlockSoftwareUpdaters 模板中（由 Dyras 提供) [#1630](https://github.com/sandboxie-plus/Sandboxie/pull/1630)
- 当 Sandboxie-Plus.ini 不可写时给予警告 [#1681](https://github.com/sandboxie-plus/Sandboxie/issues/1681)
- 增加了对关键区域的清理（由 chunyou128 提供) [#1686](https://github.com/sandboxie-plus/Sandboxie/pull/1686)

### 更改
- 改进了突破进程的命令行处理 [#1655](https://github.com/sandboxie-plus/Sandboxie/issues/1655)
- 禁用 SBIE2193 通知（由 isaak654 提供) [#1690](https://github.com/sandboxie-plus/Sandboxie/pull/1690)
- 改进了错误信息 6004 [#1719](https://github.com/sandboxie-plus/Sandboxie/issues/1719)

### 修复
- 修复了新托盘列表的暗黑模式问题
- 修复了当 Sandboxie-Plus.ini 不可写时未显示警告的问题 [#1681](https://github.com/sandboxie-plus/Sandboxie/issues/1681)
- 修复了软件兼容性复选框的问题（感谢 MitchCapper) [#1678](https://github.com/sandboxie-plus/Sandboxie/issues/1678)
- 修复了在盒子关闭时事件并不总是执行的问题 [#1658](https://github.com/sandboxie-plus/Sandboxie/issues/1658)
- 修复了 key_merge.c 中的内存泄漏
- 修复了隐私模式下枚举注册表键的问题
- 修复了在 1.0.13 中引入的设置问题 [#1684](https://github.com/sandboxie-plus/Sandboxie/issues/1684)
- 修复了解析防火墙端口选项时的崩溃问题
- 修复的安全问题 ID-19：在某些情况下，沙箱进程可能会获得对未沙箱化线程的写权限句柄 [#1714](https://github.com/sandboxie-plus/Sandboxie/issues/1714)


## [1.0.13 / 5.55.13] - 2022-03-08

### 修复
- 修复的安全问题 ID-17：硬链接创建未正确过滤（感谢 Diversenok)
- 修复了检查证书条目的问题


## [1.0.12 / 5.55.12] - 2022-03-02

### 新增
- 当 SandMan.exe 崩溃时增加了迷你转储创建功能

### 更改
- 禁用新沙箱中的 Chrome 和 Firefox 钓鱼条目（由 isaak654 提供) [#1616](https://github.com/sandboxie-plus/Sandboxie/pull/1616)
- 更新了 BlockSoftwareUpdaters 模板中的 Mozilla 路径（由 isaak654 提供) [#1623](https://github.com/sandboxie-plus/Sandboxie/pull/1623)
- 将 “暂停强制程序规则” 命令重命名为 “暂停强制程序”（仅限 Plus)
- 重新构建托盘图标生成，现在使用叠加图标，增加了忙碌叠加

### 修复
- 修复了在隐私模式下访问网络驱动器的问题 [#1617](https://github.com/sandboxie-plus/Sandboxie/issues/1617)
- 修复了在隔室模式下 ping 的问题 [#1608](https://github.com/sandboxie-plus/Sandboxie/issues/1608)
- 修复了在盒子中创建和关闭大量进程时 SandMan UI 冻结的问题 [#1607](https://github.com/sandboxie-plus/Sandboxie/issues/1607)
- 修复了在运行菜单中编辑命令行条目未被识别的问题 [#1648](https://github.com/sandboxie-plus/Sandboxie/issues/1648)
- 修复了驱动程序中的蓝屏问题（感谢 Diversenok)
- 修复了与 Windows 11 Insider 构建 22563.1 的不兼容问题 [#1654](https://github.com/sandboxie-plus/Sandboxie/issues/1654)

## [1.0.11 / 5.55.11] - 2022-02-13

### 新增
- 添加了在盒子内容自动删除时的可选托盘通知
- 添加了 Free Download Manager 模板
- 当打开未沙盒的 regedit 时添加警告 [#1606](https://github.com/sandboxie-plus/Sandboxie/issues/1606)
- 添加了在官方 Qt 5.15.2 中缺少的语言文件（由 DevSplash 提供) [#1605](https://github.com/sandboxie-plus/Sandboxie/pull/1605)

### 更改
- 上一个版本引入的异步盒子操作现在默认禁用
- 将系统托盘选项从常规选项卡移动到外壳集成选项卡
- 移除了 "AlwaysUseWin32kHooks"，现在这些 Win32 钩子始终启用
  - 注意：您可以使用 "UseWin32kHooks=program.exe,n" 来为选定程序禁用它们
- 将 Listary 模板更新到 v6（由 isaak654 提供) [#1610](https://github.com/sandboxie-plus/Sandboxie/pull/1610)

### 修复
- 修复了与 SECUROM 的兼容性问题 [#1597](https://github.com/sandboxie-plus/Sandboxie/issues/1597)
- 修复了模态性问题 [#1615](https://github.com/sandboxie-plus/Sandboxie/issues/1615)
- 修复了 Templates.ini 中 OpenWinClass 的特殊形式 [d6d9588](https://github.com/sandboxie-plus/Sandboxie/commit/d6d95889a91d31dd55dd2b2d136d8f80c9a8ea71)

## [1.0.10 / 5.55.10] - 2022-02-06

### 新增
- 添加了仅在托盘中显示正在运行的盒子的选项 [#1186](https://github.com/sandboxie-plus/Sandboxie/issues/1186)
  - 额外选项仅显示固定的盒子，在盒子选项中可以将盒子设置为始终显示在托盘列表中（固定)
- 添加了重置 GUI 的选项菜单命令 [#1589](https://github.com/sandboxie-plus/Sandboxie/issues/1589)
- 添加了“以未沙盒方式运行”上下文菜单选项
- 添加了新的触发器 "OnBoxDelete"，允许在盒子内容删除之前指定一个在未沙盒状态下运行的命令
  - 注意：这可以用作 "DeleteCommand" 的替代 [#591](https://github.com/sandboxie-plus/Sandboxie/issues/591)
- 选定的盒子操作（删除)不再显示进度对话框 [#1061](https://github.com/sandboxie-plus/Sandboxie/issues/1061)
  - 如果正在运行操作的盒子显示闪烁的沙漏图标，可以使用上下文菜单取消该操作

### 更改
- 现在可以使用 "HideHostProcess=program.exe" 来隐藏 Sandboxie 服务 [#1336](https://github.com/sandboxie-plus/Sandboxie/issues/1336)
- 更新程序阻止现在使用名为 BlockSoftwareUpdaters 的模板
- 强化 "StartProgram=..."，使 "StartCommand=..." 成为过时
  - 要获得与 "StartCommand=..." 相同的功能，请使用 "StartProgram=%SbieHome%\Start.exe ..."
- 将 "Auto Start" 常规选项卡与 "Auto Exec" 高级选项卡合并为一个通用的 "Triggers" 高级选项卡

### 修复
- 修复了新突破进程功能的一些问题并增强了安全性（感谢 Diversenok)
- 修复了重新打开已打开窗口的问题 [#1584](https://github.com/sandboxie-plus/Sandboxie/issues/1584)
- 修复了桌面访问问题 [#1588](https://github.com/sandboxie-plus/Sandboxie/issues/1588)
- 修复了命令行调用处理的问题 [#1133](https://github.com/sandboxie-plus/Sandboxie/issues/1133)
- 修复了在切换总是在最上面属性时主窗口状态的 UI 问题 [#1169](https://github.com/sandboxie-plus/Sandboxie/issues/1169)
- 修复了托盘列表中盒子上下文菜单的问题 [1106](https://github.com/sandboxie-plus/Sandboxie/issues/1106)
- 修复了 "AutoExec=..."
- 修复了取消盒子删除操作不工作的问题 [#1061](https://github.com/sandboxie-plus/Sandboxie/issues/1061)
- 修复了 DPI 缩放和颜色选择器对话框的问题 [#803](https://github.com/sandboxie-plus/Sandboxie/issues/803)

### 移除
- 移除了 "UseRpcMgmtSetComTimeout=AppXDeploymentClient.dll,y"，这是用来与 Free Download Manager 一起使用的，因为它导致了其他问题
  - 如果您将 Free Download Manager 与在沙盒中的设置 "RpcMgmtSetComTimeout=n" 一起使用，您必须手动将该行添加到您的 Sandboxie.ini 中

## [1.0.9 / 5.55.9] - 2022-01-31

### 新增
- SandMan 现在会在访问选项被修改时，使所有被包装的进程实时更新其路径设置
- 添加了新的维护菜单选项“卸载所有”，用于在便携模式下快速移除所有组件
- 添加了版本号到 Sandboxie Classic 的标题栏
- 添加了返回到空盒状态的选项，同时保留所有快照
- Sandboxie-Plus.ini 现在可以放置在 C:\ProgramData\Sandboxie-Plus\ 文件夹中并优先使用（用于商业用途)
- 添加对 AF_UNIX 在 Windows 上的支持，以解决与 OpenJDK17 及后续版本相关的问题 [#1009](https://github.com/sandboxie-plus/Sandboxie/issues/1009) [#1520](https://github.com/sandboxie-plus/Sandboxie/issues/1520) [#1521](https://github.com/sandboxie-plus/Sandboxie/issues/1521)

### 更改
- 重新设计了突破机制，使其基于服务并不允许父进程访问被突破的子进程
- 启用沙盒进程的目录连接创建 [#1375](https://github.com/sandboxie-plus/Sandboxie/issues/1375)
- 在盒子创建时恢复 "AutoRecover=y" [#1554](https://github.com/sandboxie-plus/Sandboxie/discussions/1554)
- 改进了快照支持 [#1220](https://github.com/sandboxie-plus/Sandboxie/issues/1220)
- 将 "Disable Forced Programs" 命令重命名为 "Pause Forced Programs Rules"（仅限 Plus)

### 修复
- 修复了 "EnableObjectFiltering=y" 下 BreakoutProcess 无法工作的情况
- 修复了安全问题 ID-16：在未沙盒启动 *COMSRV* 时，返回的进程句柄具有完全访问权限
- 修复了进度对话框的问题 [#1562](https://github.com/sandboxie-plus/Sandboxie/issues/1562)
- 修复了在 Sandboxie 中处理目录连接的问题 [#1396](https://github.com/sandboxie-plus/Sandboxie/issues/1396)
- 修复了 File_NtCloseImpl 中的句柄泄漏
- 修复了在上一个版本中引入的最大化窗口的边框问题 [#1561](https://github.com/sandboxie-plus/Sandboxie/issues/1561)
- 修复了一些索引溢出的问题（感谢 7eRoM) [#1571](https://github.com/sandboxie-plus/Sandboxie/pull/1571]
- 修复了与 sysnative 目录相关的问题 [#1403](https://github.com/sandboxie-plus/Sandboxie/issues/1403)
- 修复了在通过上下文菜单运行沙盒时启动 SandMan 的问题 [#1579](https://github.com/sandboxie-plus/Sandboxie/issues/1579)
- 修复了在创建主窗口时的暗色模式闪烁问题 [#1231](https://github.com/sandboxie-plus/Sandboxie/issues/1231#issuecomment-1024469681)
- 修复了与快照错误处理相关的问题 [#350](https://github.com/sandboxie-plus/Sandboxie/issues/350)
- 修复了“总是在最上面”选项的问题（仅限 Plus)

## [1.0.8 / 5.55.8] - 2022-01-18

### 新增
- 在 Plus UI 中添加了葡萄牙语（葡萄牙)支持（由 JNylson、isaak654、mpheath 提供) [#1497](https://github.com/sandboxie-plus/Sandboxie/pull/1497)
- 添加了 "BreakoutProcess=program.exe"，通过此选项可以从盒内启动未沙盒的应用程序 [#1500](https://github.com/sandboxie-plus/Sandboxie/issues/1500)
  - 程序图像必须位于沙盒外部才能正常工作
  - 如果另一个沙盒配置了 "ForceProcess=program.exe"，它将捕获该进程
  - 使用案例：设置一个强制运行 Web 浏览器的盒子，当另一个盒子打开网站时，这将在专用浏览器盒中进行
  - 注意："BreakoutFolder=some\path" 也是可用的
- 为 Classic 安装程序添加了静默卸载开关 `/remove /S`（由 sredna 提供) [#1532](https://github.com/sandboxie-plus/Sandboxie/pull/1532)

### 更改
- 文件名 "sandman_pt" 改为 "sandman_pt_BR"（巴西葡萄牙语) [#1497](https://github.com/sandboxie-plus/Sandboxie/pull/1497)
- 文件名 "sandman_ua" 改为 "sandman_uk"（乌克兰语) [#1527](https://github.com/sandboxie-plus/Sandboxie/issues/1527]
  - 注意：鼓励翻译者在创建新 Pull Request 前遵循 [本地化说明和提示](https://github.com/sandboxie-plus/Sandboxie/discussions/1123#discussioncomment-1203489)
- 更新了 Firefox 更新阻止器（由 isaak654 发现) [#1545](https://github.com/sandboxie-plus/Sandboxie/issues/1545#issuecomment-1013807831)

### 修复
- 修复了打开所有文件访问 OpenFilePath=* 的问题 [#971](https://github.com/sandboxie-plus/Sandboxie/issues/971)
- 修复了打开网络共享的相关问题 [#1529](https://github.com/sandboxie-plus/Sandboxie/issues/1529)
- 修复了 Classic 安装程序的潜在升级问题（由 isaak654 提供) [130c43a](https://github.com/sandboxie-plus/Sandboxie/commit/130c43a62c9778b734fa625bf4f46b12d0701719)
- 修复了 Classic 安装程序的小问题（由 sredna 提供) [#1533](https://github.com/sandboxie-plus/Sandboxie/pull/1533)
- 修复了 Ldr_FixImagePath_2 的问题 [#1507](https://github.com/sandboxie-plus/Sandboxie/issues/1507)
- 使用 SandMan UI 和 UI 关闭时，使用 "Run Sandboxed" 将保持关闭
- 修复了 Util_GetProcessPidByName 的问题，这应该解决驱动程序有时在启动时未能启动的问题 [#1451](https://github.com/sandboxie-plus/Sandboxie/issues/1451)
- SandMan 现在在启动被包装的进程时像 SbieCtrl 一样在后台运行 [post506](https://forum.xanasoft.com/threads/direct-start-firefox-in-box.173/post-506)
- 修复了全屏时持久盒边框未显示任务栏的问题 [post474](https://forum.xanasoft.com/threads/taskbar-problems-when-using-sandboxie-in-windows-11.224/post-474)
- 修复了盒边框未跨多个显示器的问题 [#1512](https://github.com/sandboxie-plus/Sandboxie/issues/1512)
- 修复了使用 DPI 缩放时边框的问题 [#1506](https://github.com/sandboxie-plus/Sandboxie/issues/1506)
- 修复了 Qt 的 DPI 问题 [#1368](https://github.com/sandboxie-plus/Sandboxie/issues/1368)
- 修复了在暗色模式下窗口创建时的亮闪问题 [#1231](https://github.com/sandboxie-plus/Sandboxie/issues/1231)
- 修复了 PortableRootDir 设置的相关问题 [#1509](https://github.com/sandboxie-plus/Sandboxie/issues/1509)
- 修复了在驱动未连接时设置窗口崩溃的问题
- 修复了 Finder Tool 的 DPI 问题 [#912](https://github.com/sandboxie-plus/Sandboxie/issues/912)
- 修复了 reused 进程 ID 的另一个问题 [#1547](https://github.com/sandboxie-plus/Sandboxie/issues/1547)
- 修复了与 SeAccessCheckByType 相关的问题 [#1548](https://github.com/sandboxie-plus/Sandboxie/issues/1548)

## [1.0.7 / 5.55.7] - 2022-01-06

### 新增
- 添加了实验性选项 "CreateToken=y"，以创建一个新的令牌而不复用现有的
- 添加了选项 "DisableRTBlacklist=y"，允许禁用硬编码的运行时类黑名单
- 添加了新的模板 "DeviceSecurity"，以锁定对系统上设备驱动程序的访问
  - 注意：此模板需要 RuleSpecificity 可用才能正常工作
- 添加了在 Plus UI 中设置自定义 INI 编辑器的选项 [#1475](https://github.com/sandboxie-plus/Sandboxie/issues/1475)
- 添加了选项 "LingerLeniency=n" 以解决问题 [#997](https://github.com/sandboxie-plus/Sandboxie/issues/997)

### 更改
- 重新设计了驱动中的系统调用调用代码
  - Win32k 钩子现在与 HVCI 兼容 [#1483](https://github.com/sandboxie-plus/Sandboxie/issues/1483)

### 修复
- 修复了驱动中的内存泄漏（conf_user.c)
- 修复了在打开路径中重命名文件的问题，该问题在 1.0.6 中引入
- 修复了导致 Chromium 浏览器无法正确关闭的问题 [#1496](https://github.com/sandboxie-plus/Sandboxie/issues/1496)
- 修复了与 Start.exe 相关的问题 [#1517](https://github.com/sandboxie-plus/Sandboxie/issues/1517) [#1516](https://github.com/sandboxie-plus/Sandboxie/issues/1516)
- 修复了 SandMan 中 reused 进程 ID 的问题
- 修复了 KmdUtil 有时未正常终止驱动的情况 [#1493](https://github.com/sandboxie-plus/Sandboxie/issues/1493)

### 移除
- 移除了 OpenToken，因为它仅是 UnrestrictedToken=y 和 UnfilteredToken=y 同时设置的简写

## [1.0.6 / 5.55.6] - 2021-12-31

### 新增
- 将“打开方式”替换为 Sandboxie 对话框，以适应 Windows 10 [#1138](https://github.com/sandboxie-plus/Sandboxie/issues/1138)
- 添加了在应用程序隔离模式下运行 Win32 商店应用程序的能力（需要在 Windows 11 中打开 COM)
  - 注意：这并不意味着 UWP 商店应用程序，仅指可以通过商店部署的常规 Win32 应用程序
- 添加了新的调试选项 "UnstrippedToken=y" 和 "KeepUserGroup=y"
- 在恢复窗口中添加了双击恢复文件和文件夹的功能 [#1466](https://github.com/sandboxie-plus/Sandboxie/issues/1466)
- 在 Plus UI 中添加了乌克兰语支持（由 SuperMaxusa 提供) [#1488](https://github.com/sandboxie-plus/Sandboxie/pull/1488)

### 更改
- "UseSbieWndStation=y" 现在是默认行为 [#1442](https://github.com/sandboxie-plus/Sandboxie/issues/1442)
- 由于不兼容（蓝屏死机)，在启用 HVCI 时禁用了 Win32k 钩子 [#1483](https://github.com/sandboxie-plus/Sandboxie/issues/1483)

### 修复
- 修复了隐私模式下盒子初始化的问题 [#1469](https://github.com/sandboxie-plus/Sandboxie/issues/1469)
- 修复了近期版本中引入的快捷方式创建问题 [#1471](https://github.com/sandboxie-plus/Sandboxie/issues/1471)
- 修复了隐私增强型盒子和规则特异性中的各种问题
- 修复了与 SeAccessCheckByType 等相关的问题
- 修复了在 32 位 Windows 上的 Win32k 钩子相关问题 [#1479](https://github.com/sandboxie-plus/Sandboxie/issues/1479)

### 移除
- 从 32 位版本中移除了过时的 SkyNet 根套件检测

## [1.0.5 / 5.55.5] - 2021-12-25

### 新增
- 沙盒顶层异常处理程序用于创建崩溃转储
  - 可以通过将 "EnableMiniDump=process.exe,y" 或 "EnableMiniDump=y" 进行每个进程或全局启用
  - 转储标志可以使用十六进制设置为 "MiniDumpFlags=0xAABBCCDD"
  - 可以使用 "MiniDumpFlags=Extended" 设置预选标志集以获取详细转储
  - 注意：使用 EnableMiniDump 选项创建的转储文件位于：`C:\Sandbox\%USER%\%SANDBOX%`
- 添加对 Osiris 和 Slimjet 浏览器的模板支持（由 Dyras 提供) [#1454](https://github.com/sandboxie-plus/Sandboxie/pull/1454)

### 更改
- 改进了 SbieDll 初始化
- 将 Name_Buffer_Depth 的大小加倍 [#1342](https://github.com/sandboxie-plus/Sandboxie/issues/1342)
- 改进了模板视图中的文本过滤器 [#1456](https://github.com/sandboxie-plus/Sandboxie/issues/1456)

### 修复
- 修复了强制进程显示的问题 [#1447](https://github.com/sandboxie-plus/Sandboxie/issues/1447)
- 修复了与 GetClassName 相关的崩溃问题 [#1448](https://github.com/sandboxie-plus/Sandboxie/issues/1448)
- 修复了轻微的 UI 问题 [#1382](https://github.com/sandboxie-plus/Sandboxie/issues/1382)
- 修复了 UI 语言预设问题 [#1348](https://github.com/sandboxie-plus/Sandboxie/issues/1348)
- 修复了 SandMan UI 中的分组问题 [#1358](https://github.com/sandboxie-plus/Sandboxie/issues/1358)
- 修复了 EnableWin32kHooks 相关的问题 [#1458](https://github.com/sandboxie-plus/Sandboxie/issues/1458)

### 安装程序重新发布，修复如下：
- 修复了启动 Office 应用程序时的回归问题 [#1468](https://github.com/sandboxie-plus/Sandboxie/issues/1468)

## [1.0.4 / 5.55.4] - 2021-12-20

### 新增
- Hook Win32 系统调用的机制现也适用于在 WoW64 下运行的 32 位应用程序
- 添加了对 Win32k 钩子机制的自定义，因为默认情况下仅安装 GdiDdDDI* 钩子
  - 通过指定 "EnableWin32Hook=..." 强制安装其他钩子
  - 或通过 "DisableWin32Hook=..." 禁用默认钩子的安装
  - 请注意，一些 Win32k 钩子可能会导致蓝屏死机或不定义的行为（！)
  - 明显有问题的 Win32k 钩子已被列入黑名单，可以通过 "IgnoreWin32HookBlacklist=y" 绕过
- 添加了调试选项 "AdjustBoxedSystem=n"，以禁用使用系统令牌运行服务的 ACL 调整
- 添加了 "NoUACProxy=y" 选项及相应模板，以禁用 UAC 代理
  - 注意：以隔离模式配置盒子的会默认启用此模板
- 添加了 UI 选项以更改默认 RpcMgmtSetComTimeout 预设
- 在 Plus 安装程序中添加更多条目（当前翻译在 [Languages.iss](https://github.com/sandboxie-plus/Sandboxie/blob/master/Installer/Languages.iss) 文件中需要更新)

### 更改
- "EnableWin32kHooks=y" 现在默认启用，因为在 1.0.3 中没有报告问题
  - 注意：目前仅应用 GdiDdDDI* 钩子，这对于 Chromium 硬件加速所需
- 清理了底层钩子代码
- 当存在 "RunServicesAsSystem=y" 或 "MsiInstallerExemptions=y" 时，"RunRpcssAsSystem=y" 现在会自动应用于应用程序隔离模式下的沙箱

### 修复
- 修复了在请求的打开服务未运行时的 RPC 处理问题 [#1443](https://github.com/sandboxie-plus/Sandboxie/issues/1443)
- 修复了在 32 位应用程序中使用 NdrClientCall2 的钩子问题
- 修复了在使用 SandMan 时运行沙盒的起始目录问题 [#1436](https://github.com/sandboxie-plus/Sandboxie/issues/1436)
- 修复了从网络共享位置恢复的相关问题 [#1435](https://github.com/sandboxie-plus/Sandboxie/issues/1435)

## [1.0.3 / 5.55.3] - 2021-12-12

### 新增
- 添加了在 Windows 10 及更高版本上 Hook Win32k 系统调用的机制，这应该解决 Chromium 硬件加速的问题
  - 注意：此机制尚不适用于在 WoW64 下运行的 32 位应用程序
  - 要启用它，请在全局 INI 部分添加 "EnableWin32kHooks=y"，该功能是高度实验性的（！)
  - 钩子将自动应用于 Chromium GPU 进程
  - 要便为所选盒子的所有进程强制使用 Win32k 钩子，请添加 "AlwaysUseWin32kHooks=program.exe,y" [#1261](https://github.com/sandboxie-plus/Sandboxie/issues/1261) [#1395](https://github.com/sandboxie-plus/Sandboxie/issues/1395)

### 修复
- 修复了 GetVersionExW 中的错误，导致 "OverrideOsBuild=..." 无法工作 [#605](https://github.com/sandboxie-plus/Sandboxie/issues/605) [#1426](https://github.com/sandboxie-plus/Sandboxie/issues/1426)
- 修复了 INI 文件中使用某些 UTF-8 字符的问题
- 修复了与虚拟网络编辑器相关的隔离问题 [#1102](https://github.com/sandboxie-plus/Sandboxie/issues/1102)

## [1.0.2 / 5.55.2] - 2021-12-08

### 修复
- 修复了重载时恢复窗口未刷新计数的问题 [#1402](https://github.com/sandboxie-plus/Sandboxie/issues/1402)
- 修复了 1.0.0 中引入的打印问题 [#1397](https://github.com/sandboxie-plus/Sandboxie/issues/1397)
- 修复了与 CreateProcess 函数相关的问题 [#1408](https://github.com/sandboxie-plus/Sandboxie/issues/1408]

## [1.0.1 / 5.55.1] - 2021-12-06

### 新增
- 向大多数主要盒子选项列表中添加了复选框
- 添加了 SumatraPDF 模板（由 Dyras 提供) [#1391](https://github.com/sandboxie-plus/Sandboxie/pull/1391)

### 更改
- 回滚了 "OpenClsid=..." 处理的更改
- 使盒子选项中的所有主要列表可编辑

### 修复
- 修复了 1.0.0 中引入的只读路径问题
- 修复了 1.0.0 版本引入的蓝屏死机问题 [#1389](https://github.com/sandboxie-plus/Sandboxie/issues/1389)
- 修复了在沙盒中运行 Chromium 浏览器时出现的多个 BITS 通知（由 isaak654 提供) [ca320ec](https://github.com/sandboxie-plus/Sandboxie/commit/ca320ecc17180ff09a67bdefc524b30cf3540c08) [#1081](https://github.com/sandboxie-plus/Sandboxie/issues/1081)
- 修复了 Run Menu 条目中可执行文件的选择（由 isaak654 提供) [#1379](https://github.com/sandboxie-plus/Sandboxie/issues/1379)

### 移除
- 移除了 Virtual Desktop Manager 模板（由 isaak654 提供) [d775807](https://github.com/sandboxie-plus/Sandboxie/commit/d7758071f6930539c4e1f236297b4cfa332346ad) [#1326](https://github.com/sandboxie-plus/Sandboxie/discussions/1326)

## [1.0.0 / 5.55.0] - 2021-11-17

### 新增
- 新增了隐私增强模式，使用 "UsePrivacyMode=y" 的沙箱将不允许读取包含用户数据的路径
  - 除了通用的 Windows 系统路径外，所有位置需要显式打开以获取读取和/或写入权限
  - 使用 "NormalFilePath=..."、"NormalKeyPath=..."、"NormalIpcPath=..." 可使位置可读且受沙箱保护
- 新增了新的应用程序隔离模式，通过在沙箱配置中添加 "NoSecurityIsolation=y" 启用
  - 在此模式下，安全性被兼容性取代，不应对不受信任的应用程序使用
  - 注意：在此模式下，文件和注册表过滤仍然存在，因此进程在没有管理权限的情况下运行
  - 这是相对安全的，通过设置 "NoSecurityFiltering=y" 可以禁用所有过滤
- 新增实验性使用 ObRegisterCallbacks 以过滤对象创建和重复
  - 此过滤与常规 SbieDrv 的基于系统调用的过滤无关，因此也适用于应用程序隔离
  - 启用后，运行在隔离中的应用程序将无法操作运行在沙箱外部的进程
  - 注意：此功能提高了非隔离应用程序沙箱的安全性
  - 要启用此功能，请在全局部分设置 "EnableObjectFiltering=y" 并重新加载驱动程序
  - 当全局激活时，可以通过 "DisableObjectFilter=y" 禁用特定沙箱的过滤
- 新增 "DontOpenForBoxed=n" 选项，禁用对打开文件和打开密钥指令中被隔离进程的歧视
  - 此行为并不会改善安全性，但可能令人困扰，且应用程序隔离将始终禁用此选项
- 新增设置以完全开放对 COM 基础架构的访问

### 更改
- 重新设计资源访问路径匹配机制，以便选择性地对更具体的规则应用优先于较不具体的规则
  - 例如 "OpenFilePath=C:\User\Me\AppData\Firefox" 优先于 "WriteFilePath=C:\User\Me\"
  - 要启用此新行为，请在你的 Sandboxie.ini 中添加 "UseRuleSpecificity=y"，此行为在隐私增强模式下始终启用
  - 新增 "NormalFilePath=..." 以恢复特定路径上的默认 Sandboxie 行为
  - 新增 "OpenConfPath=..."，类似于 "OpenPipePath=..."，是一个适用于位于沙箱内的可执行文件的 "OpenKeyPath=..." 变体
- 删除了在创建过程中复制箱子的选项，取而代之的是箱子上下文菜单提供复制选项
- 重新设计了箱子创建对话框以提供新的箱子类型

### 修复
- 修复了 Sandboxie Plus 卸载过程中出现的 SBIE1401 通知 (by mpheath) [68fa37d](https://github.com/sandboxie-plus/Sandboxie/commit/68fa37d45be2be3565917d0de097709b7aa009e0)
- 修复了处理 FLT_FILE_NAME_INFORMATION 时的内存泄露 (by Therzok) [#1371](https://github.com/sandboxie-plus/Sandboxie/pull/1371)

## [0.9.8d / 5.53.3] - 2021-11-01

### 新增
- 如果用户希望在安装后启动 SandMan.exe，新增复选框 [#1318](https://github.com/sandboxie-plus/Sandboxie/issues/1318)
- 新增 Windows 10 虚拟桌面管理器的模板 [#1326](https://github.com/sandboxie-plus/Sandboxie/discussions/1326)

### 更改
- "OpenClsid=..." 不再仅限于 CLSCTX_LOCAL_SERVER 执行上下文
  - 这允许在 COM 辅助服务中运行带有 CLSCTX_INPROC_SERVER 标志的对象
- 在跟踪视图中，现在可以一次选择多种类型
- 一些 Plus UI 项目已被设置为可翻译 (by gexgd0419) [#1320](https://github.com/sandboxie-plus/Sandboxie/pull/1320)
- 更改默认的 "终止所有被隔离进程" 键为 Shift+Pause (by isaak654) [#1337](https://github.com/sandboxie-plus/Sandboxie/issues/1337)

### 修复
- 修复了 SbieCtrl 与新的 INI 处理机制相关的 INI 写入问题 [#1331](https://github.com/sandboxie-plus/Sandboxie/issues/1331)
- 修复了跟踪日志过滤的问题
- 修复了 Plus 安装程序中的德国语言空格问题 (by mpheath) [#1333](https://github.com/sandboxie-plus/Sandboxie/issues/1333)
- 恢复了 Waterfox 钓鱼模板条目并进行了适当修复 (by APMichael) [#1334](https://github.com/sandboxie-plus/Sandboxie/issues/1334)

## [0.9.8c / 5.53.2] - 2021-10-24

### 新增
- 新增显式行以在卸载时删除空的 Shell 注册表项 (by mpheath) [3f661a8](https://github.com/sandboxie-plus/Sandboxie/commit/3f661a8d49137b6d2c3e00757952c71b0df11e4d)

### 修复
- 修复了模板部分在编辑器中不显示的问题 [#1287](https://github.com/sandboxie-plus/Sandboxie/issues/1287)
- 修复了先前构建中自动删除箱子内容损坏的问题 [#1296](https://github.com/sandboxie-plus/Sandboxie/issues/1296) [#1324](https://github.com/sandboxie-plus/Sandboxie/issues/1324)
- 修复了在 "浏览内容" 窗口中的崩溃问题 [#1313](https://github.com/sandboxie-plus/Sandboxie/issues/1313)
- 修复了图标解析的问题 [#1310](https://github.com/sandboxie-plus/Sandboxie/issues/1310)
- 修复了状态栏中无效的 "无网络" 状态 [#1312](https://github.com/sandboxie-plus/Sandboxie/issues/1312)
- 修复了 Windows 资源管理器搜索框无法工作的问题 (by isaak654) [#1002](https://github.com/sandboxie-plus/Sandboxie/issues/1002)
- 修复了 Waterfox 钓鱼模板 (by Dyras) [#1309](https://github.com/sandboxie-plus/Sandboxie/pull/1309)
- 修复了 Plus 安装程序中的中文翻译文件问题 (by mpheath) [#1317](https://github.com/sandboxie-plus/Sandboxie/issues/1317)
- 修复了 Plus 安装程序上的自动运行注册表项路径 (by mpheath) [abd2d44](https://github.com/sandboxie-plus/Sandboxie/commit/abd2d44cd6f305da956ad70c7481cb1256efff24)
- 修复了 SbieSvc.exe 中的内存损坏

## [0.9.8b / 5.53.1] - 2021-10-19

### 新增
- 新增在 Plus UI 中将跟踪日志保存到文件的能力
- 在 Plus UI 中新增法语 (by clexanis) [#1155](https://github.com/sandboxie-plus/Sandboxie/issues/1155)

### 更改
- 网络流量跟踪现在正确记录到驱动程序日志，而不是内核调试日志
- Plus 安装程序将在安装后自动启动 SandMan.exe，以修复任务栏图标问题 [#post-3040211](https://www.wilderssecurity.com/threads/sandboxie-plus-0-9-7-test-build.440906/page-4#post-3040211)
- 经典安装程序在更新时将显示许可协议 [#1187](https://github.com/sandboxie-plus/Sandboxie/issues/1187)

### 修复
- 修复了模板部分在编辑器中不显示的问题 [#1287](https://github.com/sandboxie-plus/Sandboxie/issues/1287)
- 修复了造成某些应用程序在任务栏中显示两个按钮组的应用程序 ID 问题 [#1101](https://github.com/sandboxie-plus/Sandboxie/issues/1101)
- 修复了 Plus UI 上最大 INI 值长度的问题 [#1293](https://github.com/sandboxie-plus/Sandboxie/issues/1293)
- 修复了最近引入的处理空 Sandboxie.ini 的问题 [#1292](https://github.com/sandboxie-plus/Sandboxie/issues/1292)
- 修复了 "SpecialImages" 模板的问题 (by Coverlin) [#1288](https://github.com/sandboxie-plus/Sandboxie/issues/1288) [#1289](https://github.com/sandboxie-plus/Sandboxie/issues/1289)
- 修复了箱子清空时的问题 [#1296](https://github.com/sandboxie-plus/Sandboxie/issues/1296)
- 修复了某些语言的问题 [#1304](https://github.com/sandboxie-plus/Sandboxie/issues/1304)
- 修复了挂载目录的问题 [#1302](https://github.com/sandboxie-plus/Sandboxie/issues/1302)
- 补充了 Qt 库的缺失翻译 [#1305](https://github.com/sandboxie-plus/Sandboxie/issues/1305)
- 修复了与 Windows 兼容性助手相关的问题 [#1265](https://github.com/sandboxie-plus/Sandboxie/issues/1265)
- 修复了特定进程图像设置的问题 [#1307](https://github.com/sandboxie-plus/Sandboxie/issues/1307)

## [0.9.8 / 5.53.0] - 2021-10-15

### 新增
- 新增调试开关以禁用 Sbie 控制台重定向 "NoSandboxieConsole=y"
  - 注意：这之前是 "NoSandboxieDesktop=y" 的一部分
- 在日志中新增 Sbie+ 版本 [#1277](https://github.com/sandboxie-plus/Sandboxie/issues/1277)
- 为 Plus 安装程序新增卸载清理额外文件的功能 (by mpheath) [#1235](https://github.com/sandboxie-plus/Sandboxie/pull/1235)
- 为 Plus 安装程序新增设置语言的功能 (by mpheath) [#1241](https://github.com/sandboxie-plus/Sandboxie/issues/1241)
- 新增与 SbieMsg.dll 的事件日志消息，用于 Plus 安装程序 (by mpheath)
- 现在保存分组展开状态
- 在跟踪标签中新增附加过滤器
- 在 Templates.ini 中新增一个新部分 [DefaultTemplates]，其中包含始终应用的强制模板 [0c9ecb0](https://github.com/sandboxie-plus/Sandboxie/commit/0c9ecb084286821c0db7436c41ef99e3b9daca76#diff-965721e9c3f2350b16f4acb47d3fb75654976f0dbb4da3c507d0eaff16a4f5f2)

### 更改
- 重新设计并扩展 RPC 日志
- 重新引入 "UseRpcMgmtSetComTimeout=some.dll,n" 设置，当未指定 "RpcPortBinding" 条目时使用
  - 这允许独立于超时设置启用/禁用箱外 RPC 绑定
- 现在可以明确在每个图像名称基础上设置 "BoxNameTitle" 值 [#1190](https://github.com/sandboxie-plus/Sandboxie/issues/1190)

### 修复
- 修复了无法从被隔离的 Windows 资源管理器删除只读文件的问题 [#1237](https://github.com/sandboxie-plus/Sandboxie/issues/1237)
- 修复了 Plus UI 中错误的恢复目标 [#1274](https://github.com/sandboxie-plus/Sandboxie/issues/1274)
- 修复了在 0.9.7a 中引入的 SBIE2101 问题 [#1279](https://github.com/sandboxie-plus/Sandboxie/issues/1279)
- 修复了在箱子选择窗中排序的问题 [#1269](https://github.com/sandboxie-plus/Sandboxie/issues/1269)
- 修复了托盘刷新问题 [#1250](https://github.com/sandboxie-plus/Sandboxie/issues/1250)
- 修复了托盘活动显示 [#1221](https://github.com/sandboxie-plus/Sandboxie/issues/1221)
- 修复了恢复窗口在任务栏中不显示的问题 [#1195](https://github.com/sandboxie-plus/Sandboxie/issues/1195)
- 修复了暗主题预设不实时更新的问题 [#1270](https://github.com/sandboxie-plus/Sandboxie/issues/1270)
- 修复了 Microsoft Edge 关于 "FakeAdminRights=y" 的投诉问题 [#1271](https://github.com/sandboxie-plus/Sandboxie/issues/1271)
- 修复了在全局部分使用本地模板的问题 [#1212](https://github.com/sandboxie-plus/Sandboxie/issues/1212)
- 修复了 MinGW 中的 git.exe 冻结问题 [#1238](https://github.com/sandboxie-plus/Sandboxie/issues/1238)
- 修复了在黑暗模式下搜索高亮的问题

### 删除
- 删除了对跟踪日志进行排序的功能，因为这消耗了过多的CPU

## [0.9.7e / 5.52.5] - 2021-10-09

### 更改
- 再次重新设计设置处理，现在驱动程序在枚举时保持顺序，但为了良好的性能，平行保持一个哈希映射以进行快速精确查找

## [0.9.7d / 5.52.4] - 2021-10-06

### 修复
- 修复了 SbieCtrl 的另一个 INI 问题

## [0.9.7c / 5.52.3] - 2021-10-05

### 修复
- 修复了另一个与 SbieApi_EnumBoxesEx 相关的处理错误

## [0.9.7b / 5.52.2] - 2021-10-04

### 修复
- 修复了加载非 Unicode Sandboxie.ini 的问题，该问题是在先前的构建中引入的

## [0.9.7 / 5.52.1] - 2021-10-02

### 新增
- 在进程状态列中新增强制进程指示器 [#1174](https://github.com/sandboxie-plus/Sandboxie/issues/1174)
- 新增 "SbieTrace=y" 选项，以跟踪 Sandboxie 进程与 Sandboxie 核心组件之间的交互
- 在初始化空沙箱时，设置 MSI 调试键以生成 MSI 安装程序服务的调试输出
- 新增 "DisableComProxy=y" 以禁用通过服务的 COM 代理
- 新增 "ProcessLimit=..." 允许限制沙箱中进程的最大数量 [#1230](https://github.com/sandboxie-plus/Sandboxie/issues/1230)
- 新增缺失的 IPC 日志记录

### 更改
- 重新设计 SbieSvc INI 服务器以允许设置缓存并大幅提高性能
  - 现在的 Sandboxie.ini 中的注释被保留，以及所有条目的顺序
- 启用配置部分列表替换，采用哈希映射以提高配置性能
- 改进了 Plus 安装程序的进度和状态消息 (by mpheath) [#1168](https://github.com/sandboxie-plus/Sandboxie/pull/1168)
- 重新设计了 RpcSs 启动机制，沙箱中的 RpcSs 和 DcomLaunch 现在可以作为系统运行，使用 "RunRpcssAsSystem=y"
  - 注意：从安全角度考虑，通常不推荐这样做，但在某些场景中可能需要兼容性
- 重新设计 WTSQueryUserToken 处理以在所有场景中正常工作
- 重新设计配置值列表以使用哈希表以提高性能

### 修复
- 修复了 Windows 7 中 Plus 升级安装的问题 (by mpheath) [#1194](https://github.com/sandboxie-plus/Sandboxie/pull/1194)
- 修复了在每个箱子启动时执行而不是仅在初始化期间执行的自定义 autoexec 命令
- 修复了设计问题，限制每个沙箱的最大进程数为 511
- 修复了残留进程监视机制中的句柄泄漏
- 修复了打开设备路径（如 "\\??\\FltMgr")时的问题
- 修复了驱动程序签名的显式 FileDigestAlgorithm 选项的构建问题 (by isaak654) [#1210](https://github.com/sandboxie-plus/Sandboxie/pull/1210)
- 修复了资源访问日志有时损坏的问题
- 修复了 Microsoft Office Click-to-Run 的问题 [#428](https://github.com/sandboxie-plus/Sandboxie/issues/428) [#882](https://github.com/sandboxie-plus/Sandboxie/issues/882)

### 删除
- 删除了对 Microsoft EMET（增强缓解体验工具包)的支持，因为它在 2018 年已停止支持
- 删除了对 Messenger Plus! Live 的支持，因为 MSN Messenger 在 2013 年已停止支持
- 禁用 Plus UI 上的土耳其语，因为不活跃 (by isaak654) [#1215](https://github.com/sandboxie-plus/Sandboxie/pull/1215)

## [0.9.6 / 5.51.6] - 2021-09-12

### 新增
- 新增重命名组的能力 [#1152](https://github.com/sandboxie-plus/Sandboxie/issues/1152)
- 新增定义沙箱自定义顺序的能力，可以使用移动上下文菜单或按住 Alt + 箭头键移动
- 将恢复窗口恢复列表新增 [#988](https://github.com/sandboxie-plus/Sandboxie/issues/988)
- 新增恢复窗口的查找器

### 更改
- 更新模板中的 BlockPort 规则，以符合新的 NetworkAccess 格式（by isaak654)[#1162](https://github.com/sandboxie-plus/Sandboxie/pull/1162)
- 立即恢复行为的默认设置现在是显示恢复窗口，而不是使用通知窗口 [#988](https://github.com/sandboxie-plus/Sandboxie/issues/988)
- 新的运行对话框现在需要双击 [#1171](https://github.com/sandboxie-plus/Sandboxie/issues/1171)
- 重新设计了恢复窗口

### 修复
- 修复了创建组菜单时的问题 [#1151](https://github.com/sandboxie-plus/Sandboxie/issues/1151)
- 修复了重命名时导致箱子失去组关联的问题
- 修复了与 Thunderbird 91+ 相关的问题 [#1156](https://github.com/sandboxie-plus/Sandboxie/issues/1156)
- 修复了文件处置处理的问题 [#1161](https://github.com/sandboxie-plus/Sandboxie/issues/1161)
- 修复了 Windows 11 22449.1000 的问题 [#1164](https://github.com/sandboxie-plus/Sandboxie/issues/1164)
- 修复了 SRWare Iron 模板 (by Dyras) [#1146](https://github.com/sandboxie-plus/Sandboxie/pull/1146)
- 修复了经典 UI 中标签定位的问题 (by isaak654) [#1088](https://github.com/sandboxie-plus/Sandboxie/issues/1088)
- 修复了仅将星号设置为路径时发生的旧问题 [#971](https://github.com/sandboxie-plus/Sandboxie/issues/971)

## [0.9.5 / 5.51.5] - 2021-08-30

### 新增
- 新增在会话 0 中运行沙箱的选项 [session 0](https://techcommunity.microsoft.com/t5/ask-the-performance-team/application-compatibility-session-0-isolation/ba-p/372361)
  - 注意：此时进程具有系统令牌，因此建议启用 "DropAdminRights=y"
- 如果 UI 以管理员权限运行，现在可以终止其他会话中的被隔离进程
- 新增 "StartSystemBox=" 选项以在 Sbie 启动/系统启动时在会话 0 中自动运行一个箱子
  - 注意：箱子启动是通过发出 Start.exe /box:[name] auto_run 完成的
- 新增 Start.exe 自动运行命令以启动所有被沙箱化的自动启动位置
- 新增 Start.exe /keep_alive 命令行开关，该开关保持进程在箱中运行，直到它正常终止
- 新增 "StartCommand="，该命令可通过 Start.exe 在箱子启动时执行复杂命令
- 新增菜单选项以启动 regedit 并加载箱子的注册表项
- 在 Plus UI 中新增系统托盘选项以显示经典图标 [#963](https://github.com/sandboxie-plus/Sandboxie/issues/963#issuecomment-903933535)

### 更改
- 将命令提示符图标和字符串从 "Terminal" 更改为 "Command Prompt" [#1135](https://github.com/sandboxie-plus/Sandboxie/issues/1135)
- 重新设计箱子菜单布局

### 修复
- 修复了与 Windows Server 2022（构建 20348)兼容性的问题 [#1143](https://github.com/sandboxie-plus/Sandboxie/issues/1143)
- 修复了创建快捷方式时的问题 [#1134](https://github.com/sandboxie-plus/Sandboxie/issues/1134)

### 安装程序于 2021-08-31 重新发布，修复了以下问题：
- 修复了 Plus 升级后 KmdUtil 警告 1061 (by mpheath) [#968](https://github.com/sandboxie-plus/Sandboxie/issues/968) [#1139](https://github.com/sandboxie-plus/Sandboxie/issues/1139)

## [0.9.4 / 5.51.4] - 2021-08-22

### 新增
- 新增日志子菜单中的清除命令 [#391](https://github.com/sandboxie-plus/Sandboxie/issues/391)
- 新增禁用进程终止提示的选项 [#514](https://github.com/sandboxie-plus/Sandboxie/issues/514)
- 新增 "Options/InstantRecovery" 设置到 Sandboxie-Plus.ini，以使用恢复窗口替代通知弹出窗口 [#988](https://github.com/sandboxie-plus/Sandboxie/issues/988)
- 新增重命名非空沙箱的能力 [#1100](https://github.com/sandboxie-plus/Sandboxie/issues/1100)
- 新增移除非空沙箱的能力
- 新增 SandMan UI 的文件浏览窗口以覆盖 SbieCtrl 的文件查看功能 [#578](https://github.com/sandboxie-plus/Sandboxie/issues/578)

### 更改
- Sbie UI 中的通用错误现在以十六进制显示状态代码，并在可用时提供字符串描述

### 修复
- 修复了 "del" 快捷键终止进程不总是有效的问题
- 修复了组显示问题 [#1094](https://github.com/sandboxie-plus/Sandboxie/issues/1094)
- 修复了在已在沙箱中使用 "沙箱运行" 的文件时出现的问题 [#1099](https://github.com/sandboxie-plus/Sandboxie/issues/1099)

## [0.9.3 / 5.51.3] - 2021-08-08

> 阅读开发者关于新 [WFP 功能](https://github.com/sandboxie-plus/Sandboxie/releases/tag/0.9.3) 的说明

### 添加
- 可以与 Sandboxie 链接一起使用“无沙箱运行”选项 [#614](https://github.com/sandboxie-plus/Sandboxie/issues/614)

### 修复
- 修复了经典版本中的“在沙箱外运行”问题 [#614](https://github.com/sandboxie-plus/Sandboxie/issues/614#issuecomment-894710466)
- 修复了打开模板时未加载编辑标签的问题 [#1054](https://github.com/sandboxie-plus/Sandboxie/issues/1054#issuecomment-893001316)
- 修复了“探索沙箱”问题 [#972](https://github.com/sandboxie-plus/Sandboxie/issues/972)
- 修复了沙箱流程的起始目录问题 [#1071](https://github.com/sandboxie-plus/Sandboxie/issues/1071)
- 修复了语言自动检测问题 [#1018](https://github.com/sandboxie-plus/Sandboxie/issues/1018)
- 修复了多个同名文件的问题，通过始终显示扩展名 [#1041](https://github.com/sandboxie-plus/Sandboxie/issues/1041)
- 修复了 SandMan UI 中的多个程序分组问题 [#1054](https://github.com/sandboxie-plus/Sandboxie/issues/1054)
- 修复了“无磁盘”错误 [#966](https://github.com/sandboxie-plus/Sandboxie/issues/966)
- 修复了使用 qMake 的 32 位版本的问题，-O2 选项导致 QSbieAPI.dll 崩溃 [#995](https://github.com/sandboxie-plus/Sandboxie/issues/995)
- 修复了最近版本引入的 UserSettings 问题 [#1054](https://github.com/sandboxie-plus/Sandboxie/issues/1054)

## [0.9.2 / 5.51.2] - 2021-08-07（预发布)

### 添加
- 添加了重新配置驱动程序的功能，允许在不重新加载/重启的情况下启用/禁用 WFP 和其他功能

### 更改
- 重新组织并改进了设置窗口
- 改进了托盘图标，沙子颜色现在更黄

### 修复
- 修复了 5.51.0 中引入的进程启动处理问题 [#1063](https://github.com/sandboxie-plus/Sandboxie/issues/1063)
- 修复了 5.51.0 中引入的快速恢复问题
- 修复了与 Intel 11 代和 AMD Ryzen 5XXX CPU 的 CET 硬件强制栈保护不兼容的问题 [#1067](https://github.com/sandboxie-plus/Sandboxie/issues/1067) [#1012](https://github.com/sandboxie-plus/Sandboxie/issues/1012)

### 删除
- 从驱动程序中注释掉所有与 Windows XP 特定的支持代码

## [0.9.1 / 5.51.1] - 2021-07-31（预发布)

### 添加
- 添加托盘图标以指示与驱动程序的连接中断
- 添加自定义托盘图标的选项
- 添加 “DllSkipHook=some.dll” 选项以禁用对选定 DLL 的钩子安装
- 添加对 Plus 安装程序的本地化支持（由 yfdyh000 和 mpheath 提供) [#923](https://github.com/sandboxie-plus/Sandboxie/pull/923)

### 更改
- 重新设计了 NtClose 处理以提高性能和可扩展性
- 改进了托盘框菜单和列表

### 修复
- 修复了与伪管理员和某些 NSIS 安装程序的问题 [#1052](https://github.com/sandboxie-plus/Sandboxie/issues/1052)
- 修复了 FileDispositionInformation 行为的问题，导致虚假的文件删除处理
- 修复了检查 WFP 状态的问题
- 修复了在启动时 WFP 无法初始化的问题
- 修复了启动后托盘沙箱选项不可用的问题
- 修复了框选项中访问更改标志未正确设置的问题 [#1065](https://github.com/sandboxie-plus/Sandboxie/issues/1065)

## [0.9.0 / 5.51.0] - 2021-07-29（预发布)

### 添加
- 添加对 Windows 过滤平台（WFP)的支持，以替代基于设备的网络阻止方案
  - 要启用此支持，请将 'NetworkEnableWFP=y' 添加到全局部分并重启或重新加载驱动程序
  - 要对特定沙箱使用 WFP，请添加 'AllowNetworkAccess=n'
  - 您可以通过使用 'AllowNetworkAccess=program.exe,y' 允许某些进程
  - 您还可以通过将 'AllowNetworkAccess=n' 添加到全局部分来全局启用此策略
  - 在这种情况下，您可以通过将 'AllowNetworkAccess=y' 添加到特定盒子来免除整个沙箱
  - 您可以通过使用 'AllowNetworkAccess=program.exe,n' 阻止某些进程
  - 注意：WFP 不如旧方法绝对，使用 WFP 将仅过滤 TCP/UDP 通信
  - 受限制的沙箱进程仍将能够使用系统服务解析域名，但它们将无法直接发送或接收数据包
  - WFP 的优点是可以通过限制通信到指定地址或选定端口的方式实施过滤规则，使用 "NetworkAccess=..."
- 在未设置 "NetworkEnableWFP=y" 的情况下，添加完全功能的基于规则的用户模式数据包过滤器
  - 该机制替代了旧的 "BlockPort=..." 功能
  - 注意：此过滤器仅适用于外部连接/流量，内入流量需要 WFP 模式或第三方防火墙
  - 与旧的用户模式机制类似，恶意应用程序可以通过解除钩住某些函数来绕过它，因此建议在需要可靠隔离时使用内核模式的 WFP 机制
- 添加新的跟踪选项 "NetFwTrace=*" 来跟踪防火墙组件的操作
  - 请注意，驱动程序仅跟踪内核调试输出，使用 DbgView.exe 进行记录
- 现在可以使用 API_QUERY_PROCESS_INFO 获取沙箱线程的模拟令牌
  - 注意：此功能由 TaskExplorer 使用，以允许检查沙箱内部令牌
  - 注意：进程必须具有管理权限才能能够使用此 API
- 添加一个 UI 选项以切换 "MsiInstallerExemptions=y"
  - 以防未来的 Windows 版本打破系统无模式中的某些东西
- 向驱动程序添加 ObRegisterCallbacks 的示例代码
- 添加新的调试选项 "DisableFileFilter=y" 和 "DisableKeyFilter=y"，允许禁用文件和注册表过滤
  - 注意：这些选项仅用于测试，并禁用沙箱隔离的核心部分
- 向 SandMan.exe 添加了一些命令行选项

### 更改
- 大大提高了跟踪日志的性能，但现在无法同时记录到 SandMan 和 SbieCtrl
- 重新设计了进程创建代码以使用 PsSetCreateProcessNotifyRoutineEx，并改进了进程终止

### 修复
- 为 ConnectEx 函数添加了缺失的钩子

## [0.8.9 / 5.50.9] - 2021-07-28 热修复 2

### 修复
- 修复了注册会话领导者的问题

## [0.8.9 / 5.50.9] - 2021-07-28 热修复 1

### 修复
- 修复了 Windows 7 的问题

## [0.8.9 / 5.50.9] - 2021-07-27

### 更改
- 更新了一些图标
- 更新 GitHub 构建动作以使用 Qt 5.15.2
- 改进了“完整”托盘图标，使其与“空”图标更易区分
- 更改代码完整性验证策略 [#1003](https://github.com/sandboxie-plus/Sandboxie/issues/1003)
  - 代码签名不再需要更改配置，若要保护预设，请使用现有的 "EditAdminOnly=y"

### 修复
- 修复了最近版本中引入的无系统模式 MSI 的问题
- 修复了 MSI 安装程序在 Windows 11 上无法创建操作服务器机制的问题
- 修复了 MSI 安装程序在 Windows 11 上的系统无模式下无法工作的情况
- 修复了 Inno Setup 脚本在 Sandboxie Plus 卸载时无法删除 shell 集成键的问题（由 mpheath 提供) [#1037](https://github.com/sandboxie-plus/Sandboxie/pull/1037)

## [0.8.8 / 5.50.8] - 2021-07-13

### 更改
- MSIServer 不再需要以系统身份运行；这完成了在沙箱中默认不使用系统令牌的转移
  - 安全增强选项 "MsiInstallerExemptions=n" 现在是默认行为

### 修复
- 修复了“探索沙箱”命令的问题 [#972](https://github.com/sandboxie-plus/Sandboxie/issues/972)
- 回滚了从使用 NtQueryKey 到 NtQueryObject 的切换，因为这似乎会破坏 Windows 10 版本 1803 及更早版本 [#984](https://github.com/sandboxie-plus/Sandboxie/issues/984)
  - 此更改是为了修复 [#951](https://github.com/sandboxie-plus/Sandboxie/issues/951)
  - 要使用 NtQueryObject，可以将选项 "UseObjectNameForKeys=y" 添加到 Sandboxie.ini

## [0.8.7b / 5.50.7] - 2021-07-11

### 修复
- 修复了之前版本中引入的自动删除激活的盒子的问题 [#986](https://github.com/sandboxie-plus/Sandboxie/issues/986)

## [0.8.7 / 5.50.7] - 2021-07-10

### 添加
- 添加总是自动选择 DefaultBox 的选项 [#959](https://github.com/sandboxie-plus/Sandboxie/issues/959)
  - 当启用此选项时，通过按住 CTRL 可以弹出正常的盒子选择对话框
- 添加在“在盒子中运行”对话框中隐藏沙箱的选项
  - 有助于避免列出不安全的兼容性测试盒子
- 向系统托盘添加盒子选项 [#439](https://github.com/sandboxie-plus/Sandboxie/issues/439) [#272](https://github.com/sandboxie-plus/Sandboxie/issues/272)

### 更改
- 将默认的“终止所有沙箱进程”快捷键从 Ctrl+Pause 更改为 Ctrl+Alt+Pause [#974](https://github.com/sandboxie-plus/Sandboxie/issues/974)
- Start.exe 不再链接未使用的 MFC 代码，将文件大小从 2.5 MB 降低到 250 KB 以下
- 更新了主 SandMan 和托盘图标 [#963](https://github.com/sandboxie-plus/Sandboxie/issues/963)
- 改进了盒子树状视图

### 修复
- 向 KmdUtil.exe 添加了额外延迟和重试，以减轻卸载驱动程序时的问题 [#968](https://github.com/sandboxie-plus/Sandboxie/issues/968)
- 修复了安装后 SbieCtrl 未正确启动的问题 [#969](https://github.com/sandboxie-plus/Sandboxie/issues/969)
- 修复了“探索沙箱” shell 选项的问题 [#972](https://github.com/sandboxie-plus/Sandboxie/issues/972)
- 修复了以提升权限运行 SandMan 时的问题 [#932](https://github.com/sandboxie-plus/Sandboxie/issues/932)
- 修复了新盒子选择对话框显示已禁用盒子的问题
- 修复了更新盒子活动状态的问题

### 删除
- 移除 Online Armor 的支持，因为此产品自 2016 年以来已被弃用

## [0.8.6 / 5.50.6] - 2021-07-07

### 添加
- 添加 LibreWolf 模板（由 Dyras 提供) [#929](https://github.com/sandboxie-plus/Sandboxie/pull/929)

### 修复
- 修复了 0.8.5 中引入的性能bug

## [0.8.5 / 5.50.5] - 2021-07-06

### 添加
- 添加全局热键以终止所有沙箱过程（默认：Ctrl+Pause)
- “在沙箱中运行”对话框现在可以由 SandMan UI 处理
- 添加“允许沙箱作业=y”，允许沙箱进程在 Windows 8 及更高版本上使用嵌套作业
  - 注意：这允许 Chrome 和其他程序使用作业系统以实现额外的隔离
- 将 Librewolf.exe 添加到 Firefox 衍生程序列表中 [#927](https://github.com/sandboxie-plus/Sandboxie/issues/927)
- 添加以沙箱方式运行 regedit 的菜单命令
- 向 SandMan UI 添加用于更新和新闻的新支持设置选项卡
- 向 Sbie 服务和 UI 添加代码完整性验证
- 为 Vivaldi Notes 添加模板（由 isaak654 提供) [#948](https://github.com/sandboxie-plus/Sandboxie/issues/948)

### 更改
- 将驱动程序使用的进程列表替换为速度更快的哈希映射实现
  - 注意：此更改提供了几乎静态的系统调用速度为 1.2µs，无论运行的进程数量
  - 旧列表，运行 100 个程序需要 4.5µs；运行 200 个程序需要 12µs；运行 300 个程序需要 18µs 每个系统调用
  - 注意：某些放缓也影响未沙箱的应用程序，因为驱动程序处理某些回调的方式
- 将驱动程序使用的每个进程线程列表替换为速度更快的哈希映射实现
- 替换配置部分列表为哈希映射以改善配置性能，并将行限制增加至 100000
  - 此功能尚未在生产版本中启用
- 只在连接时检查默认盒子存在与否
- 便携式目录对话框现在显示目录 [#924](https://github.com/sandboxie-plus/Sandboxie/issues/924)
- 当终止时，沙箱进程现在会首先尝试终止作业对象
- 驱动程序现在可以默认结束问题进程而无需服务的帮助
- 盒子删除例程现在最多重试 10 次，见 [#954](https://github.com/sandboxie-plus/Sandboxie/issues/954)
- 将服务使用的进程列表替换为速度更快的哈希映射实现
- 将服务使用的每个进程线程列表替换为速度更快的哈希映射实现

### 修复
- 修复了 SetServiceStatus 中的初始化错误（由 flamencist 提供) [#921](https://github.com/sandboxie-plus/Sandboxie/issues/921)
- 修复了传统 UI 设置中的按钮位置（由 isaak654 提供) [#914](https://github.com/sandboxie-plus/Sandboxie/issues/914)
- 修复了 SandMan UI 中缺少密码长度检查的问题 [#925](https://github.com/sandboxie-plus/Sandboxie/issues/925)
- 修复了通过名称打开作业对象的问题
- 修复了重新打开作业对象句柄时缺少权限检查的问题（感谢 Diversenok)
- 修复了某些 Chromium 90+ 钩子影响派生浏览器中 PDF 显示的问题 [#930](https://github.com/sandboxie-plus/Sandboxie/issues/930) [#817](https://github.com/sandboxie-plus/Sandboxie/issues/817)
- 修复了与 SbieSvc 通信所用的 LPC 端口重新连接的问题
- 修复了设置问题 [#957](https://github.com/sandboxie-plus/Sandboxie/issues/957)
- 修复了资源访问 COM 设置中的小型 UI 问题 [#958](https://github.com/sandboxie-plus/Sandboxie/issues/958)
- 修复了 NtQueryKey 使用 NtQueryObject 的问题 [#951](https://github.com/sandboxie-plus/Sandboxie/issues/951)
- 修复了在路径解析失败时的 key.c 崩溃
- 针对最上层模式问题添加了解决方法 [#873](https://github.com/sandboxie-plus/Sandboxie/issues/873)
  - 通知窗口不仅在 5 秒内保持最上层状态
- 修复了 5.49.5 中引入的目录删除问题
- 修复了创建盒子副本时的问题

### 删除
- 删除 "BlockPassword=n" 的切换，因为它似乎没有效果 [#938](https://github.com/sandboxie-plus/Sandboxie/issues/938)
  - 建议使用 "OpenSamEndpoint=y" 以允许 Windows 10 中的密码更改

## [0.8.2 / 5.50.2] - 2021-06-15

### 更改
- 按浏览器拆分反钓鱼规则（由 isaak654 提供) [#910](https://github.com/sandboxie-plus/Sandboxie/pull/910)

### 修复
- 正确修复了与驱动程序验证和用户句柄有关的问题 [#906](https://github.com/sandboxie-plus/Sandboxie/issues/906)
- 修复了 0.8.0 中引入的 CreateWindow 函数问题
- 修复了过时的 BoxDisplayOrder 条目保持的问题 [#900](https://github.com/sandboxie-plus/Sandboxie/issues/900)

## [0.8.1 / 5.50.1] - 2021-06-14

### 修复
- 修复了与驱动程序验证和用户句柄有关的问题
- 修复了 FLT_FILE_NAME_INFORMATION 对象的驱动程序内存泄漏
- 修复了 5.50.0 中引入的剪贴板问题 [#899](https://github.com/sandboxie-plus/Sandboxie/issues/899)
- 修复了 Windows 7 32 位上 DcomLaunch 的问题 [#898](https://github.com/sandboxie-plus/Sandboxie/issues/898)

## [0.8.0 / 5.50.0] - 2021-06-13

### 添加
- 通常，Sandboxie 会对位于沙箱中的未排除图像应用 “Close...=!<program>,...” 指令
  - 添加 'AlwaysCloseForBoxed=n' 以禁用此行为，因为这可能并不总是想要的，并且它不会提供额外的安全性
- 向 SandMan UI 添加进程映像信息
- 在 Plus UI 中本地化模板类别 [#727](https://github.com/sandboxie-plus/Sandboxie/issues/727)
- 添加 "DisableResourceMonitor=y" 以禁用对选定盒子的资源访问监视
- 添加选项以仅显示选定沙箱的跟踪条目 [#886](https://github.com/sandboxie-plus/Sandboxie/issues/886)
- 添加 "UseVolumeSerialNumbers=y"，允许驱动器字母在 \drive\ 沙箱位置后跟卷序列号
  - 这有助于避免在使用相同字母的多个便携式驱动器上文件混合
  - 注意：此选项与经典 UI 的恢复功能不兼容，仅 SandMan UI 完全兼容
- 添加 "ForceRestart=PicoTorrent.exe" 到 PicoTorrent 模板以修复兼容性问题 [#720](https://github.com/sandboxie-plus/Sandboxie/issues/720)
- 添加对 RPC 模板的本地化支持（由 isaak654 提供) [#736](https://github.com/sandboxie-plus/Sandboxie/issues/736)

### 更改
- 便携式清理消息现在具有是/否/取消选项 [#874](https://github.com/sandboxie-plus/Sandboxie/issues/874)
- 合并了 Proc_CreateProcessInternalW 和 Proc_CreateProcessInternalW_RS5 以删除重复代码
- ElevateCreateProcess 修复有时会被程序兼容性助手应用，现在默认不再模拟 [#858](https://github.com/sandboxie-plus/Sandboxie/issues/858)
  - 使用 'ApplyElevateCreateProcessFix=y' 或 'ApplyElevateCreateProcessFix=program.exe,y' 启用它
- 仅当跟踪日志没有条目并且记录被停止时才禁用跟踪

### 修复
- 修复了新的全局钩子仿真机制和 WoW64 进程中的 APC 问题 [#780](https://github.com/sandboxie-plus/Sandboxie/issues/780) [#779](https://github.com/sandboxie-plus/Sandboxie/issues/779)
- 修复了与 BlockPort 选项有关的 IPv6 问题
- 修复了指定 "OpenWinClass=*" 时与 CheatEngine 有关的问题 [#786](https://github.com/sandboxie-plus/Sandboxie/issues/786)
- 修复了 SbieDrv 中的内存损坏 [#838](https://github.com/sandboxie-plus/Sandboxie/issues/838)
- 修复了 CreateProcess 调用中的进程提升崩溃问题 [#858](https://github.com/sandboxie-plus/Sandboxie/issues/858)
- 修复了在内置管理员帐户下运行时的进程提升问题 [#3](https://github.com/sandboxie-plus/Sandboxie/issues/3)
- 修复了模板预览重置未保存条目的盒子选项窗口的问题 [#621](https://github.com/sandboxie-plus/Sandboxie/issues/621)

## [0.7.5 / 5.49.8] - 2021-06-05

### 添加
- 现在可以通过 "OpenClipboard=n" 禁用对沙箱的剪贴板访问 [#794](https://github.com/sandboxie-plus/Sandboxie/issues/794)

### 更改
- 现在由于与 Unity 游戏的兼容性，OpenBluetooth 模板默认启用 [#799](https://github.com/sandboxie-plus/Sandboxie/issues/799)
- 现在可以根据每个进程设置 "PreferExternalManifest=program.exe,y"

### 修复
- 修复了与最新 VS2019 更新的编译问题
- 修复了与 Vivaldi 浏览器的相关问题 [#821](https://github.com/sandboxie-plus/Sandboxie/issues/821)
- 修复了 Plus UI 中一些盒子选项的相关问题 [#879](https://github.com/sandboxie-plus/Sandboxie/issues/879)
- 修复了与基于 Chromium 的浏览器中的硬件加速有关的一些问题 [#795](https://github.com/sandboxie-plus/Sandboxie/issues/795)
- “停止所有”命令现在优先发出 “KmdUtil scandll”，以解决 SbieDll.dll 使用时的问题
- 针对 Electron 应用程序的解决方法，通过在 GPU 渲染进程上强制附加命令行参数 [#547](https://github.com/sandboxie-plus/Sandboxie/issues/547) [#310](https://github.com/sandboxie-plus/Sandboxie/issues/310) [#215](https://github.com/sandboxie-plus/Sandboxie/issues/215)
- 修复了软件兼容性选项卡无法始终正确显示模板名称的问题 [#774](https://github.com/sandboxie-plus/Sandboxie/issues/774)

## [0.7.4 / 5.49.7] - 2021-04-11

### 添加
- 向 Plus UI 添加禁用文件迁移提示的选项，使用 PromptForFileMigration=n [#643](https://github.com/sandboxie-plus/Sandboxie/issues/643)
- 为各种安全隔离功能添加 UI 选项
- 向 Plus UI 添加设置模板值的缺失功能
- 为 Popcorn-Time、Clementine Music Player、Strawberry Music Player、32 位 MPC-HC 添加模板（由 Dyras 提供) [#726](https://github.com/sandboxie-plus/Sandboxie/pull/726) [#737](https://github.com/sandboxie-plus/Sandboxie/pull/737)

### 更改
- 将 AutoRecover 和 Favourites 的默认设置对齐到 Plus 版本（感谢 isaak654) [#747](https://github.com/sandboxie-plus/Sandboxie/pull/747)
- 电子邮件客户端和浏览器的列表现在集中于 Dll_GetImageType
- 将 Templates.ini 中的 localstore.rdf 引用替换为 xulstore.json（由 isaak654 提供) [#751](https://github.com/sandboxie-plus/Sandboxie/pull/751)

### 修复
- 修复了与记录互联网阻塞相关的轻微问题
- 修复了定位于网络共享上的文件恢复问题 [#711](https://github.com/sandboxie-plus/Sandboxie/issues/711)
- 修复了与 CallTrace 相关的 UI 问题 [#769](https://github.com/sandboxie-plus/Sandboxie/issues/769)
- 修复了沙箱快捷方式创建时接收重复扩展名的问题 [#770](https://github.com/sandboxie-plus/Sandboxie/issues/770)
- 修复了传统 UI 中标签错位的问题（感谢 isaak654) [#759](https://github.com/sandboxie-plus/Sandboxie/pull/759)
- 修复了 SbieCtrl 中的分隔线问题（感谢 isaak654) [#761](https://github.com/sandboxie-plus/Sandboxie/pull/761)
- 修复了 The Bat! 模板中损坏的路径（由 isaak654 提供) [#756](https://github.com/sandboxie-plus/Sandboxie/pull/756)
- 修复了媒体播放器试图在盒子内写入不必要的媒体文件的问题（由 Dyras 提供) [#743](https://github.com/sandboxie-plus/Sandboxie/pull/743) [#536](https://github.com/sandboxie-plus/Sandboxie/issues/536)

## [0.7.3 / 5.49.5] - 2021-03-27

### 新增
- 新增 "UseSbieWndStation=y" 以在选定的进程中模拟 CreateDesktop，而不仅限于 Firefox 和 Chrome [#635](https://github.com/sandboxie-plus/Sandboxie/issues/635)
- 新增选项以降低控制台主机进程的完整性，现在可以使用 "DropConHostIntegrity=y" [#678](https://github.com/sandboxie-plus/Sandboxie/issues/678)
- 新增选项以轻松添加本地模板
- 新增新的 torrent 客户端和媒体播放器模板（由 Dyras 提供） [#719](https://github.com/sandboxie-plus/Sandboxie/pull/719)

### 更改
- 重新设计窗口钩子机制以提高性能 [#697](https://github.com/sandboxie-plus/Sandboxie/issues/697) [#519](https://github.com/sandboxie-plus/Sandboxie/issues/519) [#662](https://github.com/sandboxie-plus/Sandboxie/issues/662) [#69](https://github.com/sandboxie-plus/Sandboxie/issues/69) [#109](https://github.com/sandboxie-plus/Sandboxie/issues/109) [#193](https://github.com/sandboxie-plus/Sandboxie/issues/193)
  - 解决了文件保存对话框打开时间超过 30 秒的问题
  - 此修复大幅提高了沙箱化进程的 Win32 GUI 性能
- 重新设计 RPC 解析器，使其可通过 ini 文件配置
  - 以下选项现在已弃用：
    - "UseRpcMgmtSetComTimeout=some.dll,n"，请使用 "RpcPortBinding=some.dll,*,TimeOut=y"
    - "OpenUPnP=y", "OpenBluetooth=y", "OpenSmartCard=n"，请使用新的 RPC 模板
  - 请查看 Templates.ini 以获取使用示例

### 修复
- 修复了特定进程的钩子被应用于给定沙箱中的所有进程的问题
- 修复了在 SandMan UI 中消息和模板有时未正确显示的问题
- 修复了兼容性设置未正确应用的问题
- 修复了在 0.7.1 中引入的自动删除问题 [#637](https://github.com/sandboxie-plus/Sandboxie/issues/637)
- 修复了 NtSetInformationFile, FileDispositionInformation 导致 Opera 安装失败的问题
- 修复了在 0.7.2 构建中引入的 MacType 问题 [#676](https://github.com/sandboxie-plus/Sandboxie/issues/676)
- 修复了当禁用窗口重命名选项时，全球沙箱化窗口钩子不工作的情况
- 修复了保存本地模板的问题
- 修复了使用 runas 启动在 Sandboxie 监督之外创建的进程时的问题 [#688](https://github.com/sandboxie-plus/Sandboxie/issues/688)
  - 由于 runas 功能默认不可访问，这不构成安全问题
  - 要启用 runas 功能，请在 Sandboxie.ini 中添加 "OpenIpcPath=\RPC Control\SECLOGON"
  - 请注意，这样做可能会导致其他未知问题
- 修复了与 Windows 10 32 位 Insider Preview Build 21337 的驱动程序兼容性问题
- 修复了 Windows 7 的驱动程序签名问题


## [0.7.2 / 5.49.0] - 2021-03-04

### 新增
- 新增修改报告的 Windows 版本的选项 "OverrideOsBuild=7601"，适用于 Windows 7 SP1 [#605](https://github.com/sandboxie-plus/Sandboxie/issues/605)
- 追踪日志现在可以像树一样结构化，以进程作为根项，线程作为分支

### 更改
- SandboxieCrypto 现在始终迁移 CatRoot2 文件，以防止真实文件被锁定
- 大幅提高了追踪日志性能
- MSI 服务器现在可以在 "FakeAdminRights=y" 和 "DropAdminRights=y" 选项下运行 [#600](https://github.com/sandboxie-plus/Sandboxie/issues/600)
  - MSI 服务器的特殊服务豁免可以通过 "MsiInstallerExemptions=n" 禁用
- 更改 SCM 访问检查行为；非提权用户现在可以使用用户令牌启动服务
  - 现在仅在使用系统令牌启动服务时需要提升
- 重新设计追踪日志机制，使其更详细
- 重新设计 RPC 机制，使其更加灵活

### 修复
- 修复了在 5.48.0 中引入的一些安装程序的问题 [#595](https://github.com/sandboxie-plus/Sandboxie/issues/595)
- 修复了 Plus UI 中的 "将用户添加到沙箱" 问题 [#597](https://github.com/sandboxie-plus/Sandboxie/issues/597)
- 修复安全问题 ID-15：HostInjectDll 机制允许本地特权升级（感谢 hg421）
- 经典 UI 不再允许使用无效或保留的设备名称创建沙箱 [#649](https://github.com/sandboxie-plus/Sandboxie/issues/649)


## [0.7.1 / 5.48.5] - 2021-02-21

### 新增
- 增强了 RpcMgmtSetComTimeout 处理，使用 "UseRpcMgmtSetComTimeout=some.dll,n"
  - 该选项允许为每个 DLL 指定是否应使用 RpcMgmtSetComTimeout
  - 此设置优先于硬编码的和每个进程的预设
  - "UseRpcMgmtSetComTimeout=some.dll" 和 "UseRpcMgmtSetComTimeout=some.dll,y" 是等效的
- 新增 "FakeAdminRights=y" 选项，使进程在特定沙箱中认为它们拥有管理员权限
  - 推荐与 "DropAdminRights=y" 结合使用，以提高安全性
  - 使用 "FakeAdminRights=y" 和 "DropAdminRights=y" 后，安装程序应仍然正常工作
- 为 SSDP API（简单服务发现协议）添加了 RPC 支持，可以使用 "OpenUPnP=y" 启用

### 更改
- SbieCrypto 不再触发消息 1313
- 更改枚举进程 API；每个沙箱现在可以枚举超过 511 个进程（无限制）
- 重新组织了沙箱设置
- 使 COM 跟踪更加详细
- 现在 "RpcMgmtSetComTimeout=y" 再次成为默认行为，似乎整体上引起的问题较少

### 修复
- 修复了在 DevCMApi 过滤存在时的 Webcam 访问问题
- 修复了针对 'AppXDeploymentClient.dll' 的免费下载管理器的问题，因此 RpcMgmtSetComTimeout=y 将默认用于此 [#573](https://github.com/sandboxie-plus/Sandboxie/issues/573)
- 修复了未能阻止所有 WinRM 文件的问题，使用 "BlockWinRM=n" 可禁用此文件阻止
- 修复了 Sandboxie Classic 在沙箱设置 -> 外观中保存任何选项时崩溃的问题（由 typpos 提供） [#586](https://github.com/sandboxie-plus/Sandboxie/issues/586)


## [0.7.0 / 5.48.0] - 2021-02-14

### 新增
- 新增托盘图标的沙箱化指示器，工具提示中现在包含 [#]（如果启用）
- 追踪日志缓冲区现在可以使用 "TraceBufferPages=2560" 进行调整
  - 该值表示将使用的 4K 大页面的数量；此处总计 10 MB
- 新功能用于列表查找器

### 更改
- 改进了 RPC 调试
- 改进了与 RpcMgmtSetComTimeout 相关的 IPC 处理；"RpcMgmtSetComTimeout=n" 现在是默认行为
  - 对特定调用 DLL 的必需例外已被硬编码
- LogAPI 库现在使用 Sandboxie 的跟踪设施来记录事件，而不是使用其自己的管道服务器

### 修复
- 修复安全问题 ID-11：提升的沙箱化进程可以访问卷/磁盘进行读取（感谢 hg421）
  - 该保护选项可以通过使用 "AllowRawDiskRead=y" 禁用
- 修复了与 GoogleUpdate.exe 相关的 SetCurrentProcessExplicitAppUserModelID 的崩溃问题
- 修复了资源监视器按时间戳排序的问题
- 修复了无效的 Opera 书签路径（由 isaak654 提供） [#542](https://github.com/sandboxie-plus/Sandboxie/pull/542)
- 修复安全问题 ID-12：驱动程序中的竞争条件允许获取对进程的提升权限句柄（感谢 typpos） [#549](https://github.com/sandboxie-plus/Sandboxie/pull/549)
- 修复安全问题 ID-13："\\RPC Control\\samss lpc" 现在已被驱动程序过滤（感谢 hg421） [#553](https://github.com/sandboxie-plus/Sandboxie/issues/553）
  - 这允许提升的进程更改密码、删除用户等；要禁用过滤，请使用 "OpenSamEndpoint=y"
- 修复安全问题 ID-14："\\Device\\DeviceApi\\CMApi" 现在已被驱动程序过滤（感谢 hg421） [#552](https://github.com/sandboxie-plus/Sandboxie/issues/552）
  - 这允许提升的过程更改硬件配置；要禁用过滤，请使用 "OpenDevCMApi=y"


## [0.6.7 / 5.47.1] - 2021-02-01

### 新增
- 新增 UI 语言自动检测

### 修复
- 修复了 Brave.exe 现在被正确识别为基于 Chrome 的，而不是基于 Firefox 的
- 修复了在 0.6.5 中引入的与近期 Edge 构建有关的问题
  - 0.6.5 行为可以针对具体进程使用 "RpcMgmtSetComTimeout=POPPeeper.exe,n" 设置
- 修复了分组问题 [#445](https://github.com/sandboxie-plus/Sandboxie/issues/445)
- 修复了从托盘还原主窗口的状态 [#288](https://github.com/sandboxie-plus/Sandboxie/issues/288)


## [0.6.5 / 5.47.0] - 2021-01-31

### 新增
- 新增对 Waterfox.exe、Palemoon.exe 和 Basilisk.exe Firefox 分支及 Brave.exe 的检测 [#468](https://github.com/sandboxie-plus/Sandboxie/issues/468)
- 新增蓝牙 API 支持，IPC 端口可以使用 "OpenBluetooth=y" 打开 [#319](https://github.com/sandboxie-plus/Sandboxie/issues/319）
  - 这应该解决许多 Unity 游戏在启动时长时间挂起的问题
- 新增增强的 RPC/IPC 接口跟踪
- 当 SandMan UI 找不到 DefaultBox 时，将重新创建它
- "Disable Forced Programs" 时间现在被保存并重新加载

### 更改
- 降低 SandMan 的 CPU 使用率
- Sandboxie.ini 和 Templates.ini 现在可以使用 UTF-8 编码 [#461](https://github.com/sandboxie-plus/Sandboxie/issues/461) [#197](https://github.com/sandboxie-plus/Sandboxie/issues/197)
  - 此功能是实验性的，未带 UTF-8 签名的文件也应被识别
  - "ByteOrderMark=yes" 已过时，Sandboxie.ini 现在始终带有 BOM/签名
- 旧版语言文件现在可以使用 UTF-8 编码
- 重新设计文件迁移行为，移除硬编码列表，支持使用模板 [#441](https://github.com/sandboxie-plus/Sandboxie/issues/441）
  - 现在可以使用 "CopyAlways="、"DontCopy=" 和 "CopyEmpty=" 这些语法与 "OpenFilePath=" 相同
  - "CopyBlockDenyWrite=program.exe,y" 使对无法复制的文件的写入打开调用失败，而不是将其变为只读
- 移除了硬编码的 SkipHook 列表以支持模板

### 修复
- 修复了 Sbie 驱动程序中的旧内存池泄漏 [#444](https://github.com/sandboxie-plus/Sandboxie/issues/444)
- 修复了访问限制 UI 中的项目选择问题
- 修复了 SbieCtrl.exe 中的更新程序崩溃问题 [#450](https://github.com/sandboxie-plus/Sandboxie/issues/450)
- 修复了在 Sbie 5.33.1 中引入的 RPC 调用问题
- 修复了最近失效的 "终止所有" 命令
- 修复了 SandMan UI 中的一些小问题
- 修复了与 Windows 7 和 8 之间的 IPC 问题，导致进程终止
- 修复了 "恢复到" 功能


## [0.6.0 / 5.46.5] - 2021-01-25

### 新增
- 新增确认提示以终止所有命令
- 新增窗口标题到沙箱化进程信息 [#360](https://github.com/sandboxie-plus/Sandboxie/issues/360)
- 新增基于 WinSpy 的沙箱化窗口查找器 [#351](https://github.com/sandboxie-plus/Sandboxie/issues/351)
- 新增查看禁用沙箱的选项，并双击沙箱以启用它

### 更改
- "重置列" 现在根据内容调整大小，并且现在可以本地化 [#426](https://github.com/sandboxie-plus/Sandboxie/issues/426)
- 模态窗口现在相对于父窗口居中 [#417](https://github.com/sandboxie-plus/Sandboxie/issues/417)
- 改进了新沙箱窗口 [#417](https://github.com/sandboxie-plus/Sandboxie/issues/417)

### 修复
- 修复了窗口模态性的问题 [#409](https://github.com/sandboxie-plus/Sandboxie/issues/409)
- 修复了将主窗口设置为始终在最上层时的问题 [#417](https://github.com/sandboxie-plus/Sandboxie/issues/417)
- 修复了与 Windows 10 Insider build 21286 的驱动程序问题
- 修复了快照对话框的问题 [#416](https://github.com/sandboxie-plus/Sandboxie/issues/416)
- 修复了写入路径时已存在于快照中但不在外部的情况 [#415](https://github.com/sandboxie-plus/Sandboxie/issues/415)


## [0.5.5 / 5.46.4] - 2021-01-17

### 新增
- 新增 "SandboxService=..." 以强制在沙箱中启动选定的服务
- 新增 Plus UI 中的模板清理功能
- 新增互联网提示，现在也允许永久性互联网访问
- 在 SandMan UI 中新增沙箱根文件夹的浏览按钮 [#382](https://github.com/sandboxie-plus/Sandboxie/issues/382)
- 新增 Windows Explorer 信息消息
- 新增选项以始终将 SandMan UI 保持在最上层 [#352](https://github.com/sandboxie-plus/Sandboxie/issues/352)
- 允许将文件拖放到 SandMan.exe 以沙箱运行 [#355](https://github.com/sandboxie-plus/Sandboxie/issues/355)
- 在启动沙箱应用程序时启动 SandMan UI [#367](https://github.com/sandboxie-plus/Sandboxie/issues/367）
- 恢复窗口现在可以列出所有文件
- 恢复窗口新增文件计数器
- 当指定 "NoAddProcessToJob=y" 时，Chrome 和相关浏览器现在可以完全使用作业系统
  - 注意："NoAddProcessToJob=y" 降低了沙箱的隔离性，但受影响的功能大多由 UIPI 覆盖
- 新增优化的 Sbie 视图默认列宽
- 新增对 Yandex 和 Ungoogled Chromium 浏览器的模板支持（由 isaak654 提供）

### 更改
- 更新了多个浏览器修复的模板（感谢 isaak654）
- 当尝试对空沙箱进行快照时，将显示适当的错误消息 [#381](https://github.com/sandboxie-plus/Sandboxie/issues/381)
- 恢复窗口的新布局
- Sbie 视图排序现在不区分大小写

### 修复
- 修复了主窗口隐藏时子窗口关闭终止应用程序的问题 [#349](https://github.com/sandboxie-plus/Sandboxie/issues/349）
- 修复了与非模态窗口相关的问题 [#349](https://github.com/sandboxie-plus/Sandboxie/issues/349）
- 修复了绑定到移动模式的驱动程序连接问题
- 修复了快照窗口中的小问题
- 修复了尝试创建已存在沙箱时缺少错误消息的问题 [#359](https://github.com/sandboxie-plus/Sandboxie/issues/359）
- 修复了在沙箱已经被删除时仍允许保存设置的问题 [#359](https://github.com/sandboxie-plus/Sandboxie/issues/359]
- 修复了深色模式中禁用项的问题 [#359](https://github.com/sandboxie-plus/Sandboxie/issues/359）
- 修复了按 Esc 关闭某些对话框的问题 [#359](https://github.com/sandboxie-plus/Sandboxie/issues/359）
- 修复了许多窗口的制表符停止问题


## [0.5.4d / 5.46.3] - 2021-01-11

### 更改
- 改进了访问跟踪，移除了冗余条目
- OpenIpcPath=\BaseNamedObjects\[CoreUI]-* 现在在驱动程序中硬编码，无需模板条目
- WindowsFontCache 现在默认打开
- 在驱动程序中重构了一些 IPC 代码

### 修复
- 修复安全问题 ID-10：注册表隔离可能被规避，自 Windows 10 创造者更新以来一直存在
- 修复了 SandMan UI 中创建时间未能正确更新的问题

## [0.5.4c / 5.46.2] - 2021-01-10

### 新增
- 增加 "CallTrace=*" 以记录所有系统调用到访问日志

### 更改
- 改进 IPC 日志记录代码
- 改进 MSG_2101 日志记录

### 修复
- 修复了 IPC 跟踪中的更多问题
- 修复了 Chrome 和其衍生产品上的 SBIE2101 问题



## [0.5.4b / 5.46.1] - 2021-01-08

### 新增
- 增加 "RunServiceAsSystem=..." 允许特定命名服务以系统身份运行

### 更改
- 重新构建了一些与 SCM 访问相关的代码

### 修复
- 修复了 SbieSvc.exe 中的崩溃问题，此问题在上一个版本中引入
- 修复了 SandMan UI 更新检查的问题
- 修复了安全问题 ID-9：一个沙箱进程在启用 DropAdminRights 的情况下仍然可以作为系统启动

### 移除
- 移除了 "ProtectRpcSs=y"，由于与新隔离默认设置的不兼容



## [0.5.4 / 5.46.0] - 2021-01-06

### 新增
- 修复了安全问题 ID-4：Sandboxie 现在从沙箱系统令牌中剥离特别有问题的权限
  - 有了这些，进程可能试图绕过沙箱隔离（感谢 Diversenok）
  - 可以通过 "StripSystemPrivileges=n" 启用旧的行为（绝对不推荐）
- 增加新的隔离选项 "ClosePrintSpooler=y" 和 "OpenSmartCard=n"
  - 这些资源默认是开放的，但为了增强安全性，建议将其关闭
- 修复了安全问题 ID-5：增加打印机后台处理程序过滤器，以防止打印机在沙箱之外被设置
  - 可以通过 "OpenPrintSpooler=y" 禁用该过滤器
- 在恢复已存在的文件时增加覆盖提示
- 将 "StartProgram=", "StartService=" 和 "AutoExec=" 选项添加到 SandMan UI
- 增加了更多兼容性模板（感谢 isaak654） [#294](https://github.com/sandboxie-plus/Sandboxie/pull/294)

### 更改
- 更改了模拟 SCM 行为，沙箱服务默认不再以沙箱系统身份启动
  - 使用 "RunServicesAsSystem=y" 启用旧的行为
  - 注意：具有系统令牌的沙箱服务仍然受限于沙箱
  - 然而，如果最初不授予它们系统令牌，则可以消除潜在的利用向量
  - 注意：此选项与 "ProtectRpcSs=y" 不兼容，并且优先级更高！
- 重新调整动态 IPC 端口处理
- 改进资源监视器状态字符串

### 修复
- 修复了安全问题 ID-6：进程可能在沙箱之外生成进程（感谢 Diversenok）
- 修复了安全问题 ID-7：动态 IPC 端口处理中的 bug 允许绕过 IPC 隔离
- 修复了 IPC 跟踪问题
- 修复了安全问题 ID-8：CVE-2019-13502 "\RPC Control\LSARPC_ENDPOINT" 现在由驱动程序过滤（感谢 Diversenok）
  - 这允许更改某些系统选项，禁用过滤的使用 "OpenLsaEndpoint=y"
- 修复了 SBIE2303 中的钩子问题，涉及 Chrome、Edge 和可能的其他浏览器 [#68](https://github.com/sandboxie-plus/Sandboxie/issues/68) [#166](https://github.com/sandboxie-plus/Sandboxie/issues/166)
- 修复了执行快照操作时对运行进程的检查失败的问题
- 修复了一些框选项复选框未正确初始化的问题
- 修复了 SandMan 未连接到驱动程序时某些不可用选项未正确禁用的问题
- 修复了 MSI 安装程序无法在 Windows 20H2 上创建 "C:\Config.msi" 文件夹的问题 [#219](https://github.com/sandboxie-plus/Sandboxie/issues/219)
- 增加了对通用列表命令的缺失本地化支持
- 修复了在运行沙箱化的 Windows 资源管理器时 "iconcache_*" 的问题
- 修复了更多与组相关的问题



## [0.5.3b / 5.45.2] - 2021-01-02

### 新增
- 增加可移植盒子根文件夹选项的设置
- 将进程名称添加到资源日志
- 在 SandMan UI 的进程视图中添加命令行列

### 修复
- 修复了与组处理相关的一些问题 [#262](https://github.com/sandboxie-plus/Sandboxie/issues/262)
- 修复了在 64 位系统上运行 32 位程序时 GetRawInputDeviceInfo 的问题
- 修复了在 "资源访问" 选项卡中按下应用时，最后编辑的值并不总是生效的问题
- 修复了在资源访问监视器中合并条目时的问题



## [0.5.3a / 5.45.2] - 2020-12-29

### 新增
- 在 SandMan UI 中添加提示以选择链接是否在沙箱浏览器或非沙箱浏览器中打开 [#273](https://github.com/sandboxie-plus/Sandboxie/issues/273)
- 增加更多恢复选项
- 增加 "ClosedClsid=" 以阻止在兼容性问题时使用 COM 对象
- 增加 "ClsidTrace=*" 选项以跟踪 COM 使用
- 增加 "ClosedRT=" 选项以阻止访问有问题的 Windows RT 接口
- 增加选项以在 SandMan UI 中为任何选定的进程创建链接
- 增加选项以重置所有隐藏消息
- 增加更多进程预设 "强制程序" 和 "允许互联网访问"
- 在 Sandboxie.ini 中增加 "SpecialImage=chrome,some_electron_app.exe" 选项，有效的镜像类型 "chrome"、"firefox"
  - 通过此选项，您可以为这些浏览器的新冷僻分支启用特殊的硬编码解决方案
- 添加德语翻译（感谢 bastik-1001）到 SandMan UI
- 添加俄语翻译（感谢 lufog）到 SandMan UI
- 添加葡萄牙语翻译（感谢 JNylson）到 SandMan UI

### 更改
- 更改文档和更新 URL 为新的 sandboxie-plus.com 域名
- 大幅改进设置脚本（感谢 mpheath）
- "OpenClsid=" 和 "ClosedClsid=" 现在支持指定程序或组名称
- 默认情况下，当以便携模式启动时，沙箱文件夹将位于 Sandboxie 实例的父目录中

### 修复
- 新的 SandMan UI 中分组菜单无法完全使用 [#277](https://github.com/sandboxie-plus/Sandboxie/issues/277)
- 修复了在 SandMan UI 中无法设置快速恢复的问题
- 修复了在 SandMan UI 中加载进程图标时导致的资源泄漏问题
- 修复了与 OpenToken 调试选项相关的问题
- 修复了 Chrome 在访问导致调用 "FindAppUriHandlersAsync" 的网站时崩溃的问题 [#198](https://github.com/sandboxie-plus/Sandboxie/issues/198)
- 修复了以便携模式启动时连接到驱动程序的问题
- 修复了创建新盒子时缺失模板设置的问题

### 移除
- 移除了过时的 "OpenDefaultClsid=n"，改为使用 "ClosedClsid=" 及相应的值
- 移除了挂起/恢复菜单项，轮询该状态会浪费大量 CPU 循环；请使用 TaskExplorer 进行该功能



## [0.5.2a / 5.45.1] - 2020-12-23

### 修复
- 修复了 SandMan UI 中的翻译支持
- 修复了沙箱 Windows 资源管理器的问题 [#289](https://github.com/sandboxie-plus/Sandboxie/issues/289)
- 修复了简体中文本地化问题



## [0.5.2 / 5.45.1] - 2020-12-23

### 新增
- 在 SandMan UI 中增加高级新盒子创建对话框
- 增加显示/隐藏托盘上下文菜单项
- 在文件恢复对话框中增加刷新按钮
- 增加从 {install-dir}/Icons/{icon}.png 加载图标的机制，以进行 UI 自定义
- 在 SandMan UI 中增加托盘指示器以显示禁用强制程序状态
- 为 SandMan UI 中的盒子选项添加程序名称建议
- 在选项窗口中增加列宽度保存功能

### 更改
- 重新组织高级盒子选项
- 更改图标（感谢 Valinwolf 选择新的图标） [#235](https://github.com/sandboxie-plus/Sandboxie/issues/235)
- 更新 Templates.ini（感谢 isaak654） [#256](https://github.com/sandboxie-plus/Sandboxie/pull/256） [#258](https://github.com/sandboxie-plus/Sandboxie/pull/258）
- 增加在 SandMan UI 中禁用强制进程时间的最大值

### 修复
- 修复了使用 Windows 10 "核心隔离" 时在 5.45.0 中引入的 BSoD 问题 [#221](https://github.com/sandboxie-plus/Sandboxie/issues/221)
- 修复了与残留/领导进程相关的轻微问题
- 修复了 SandMan UI 中的菜单问题
- 修复了 SandMan UI 中的停止行为页面问题
- 修复了 Plus 安装程序未显示 KmdUtil 窗口的问题
- 修复了 SandMan UI 在 Windows 关闭时保存 UI 设置的问题
- 修复了 Plus 安装程序自动运行的问题 [#247](https://github.com/sandboxie-plus/Sandboxie/issues/247)
- 修复了旧版安装程序未能移除所有文件的问题
- 修复了与 Windows 20H1 及更高版本的驱动程序兼容性问题 [#228](https://github.com/sandboxie-plus/Sandboxie/issues/228）
  - 这解决了 "停止等待"，LINE Messenger 挂起等问题...
- 修复了在 5.45.0 中引入的 SbieCtrl.exe 快速恢复问题 [#224](https://github.com/sandboxie-plus/Sandboxie/issues/224）
- 修复了高级隐藏进程设置未能保存的问题
- 修复了 UI 中的一些拼写错误（感谢 isaak654） [#252](https://github.com/sandboxie-plus/Sandboxie/pull/252） [#253](https://github.com/sandboxie-plus/Sandboxie/pull/253） [#254](https://github.com/sandboxie-plus/Sandboxie/pull/254）
- 修复了在将沙箱化进程放入作业对象时 GetRawInputDeviceInfo 失败的问题 [#176](https://github.com/sandboxie-plus/Sandboxie/issues/176） [#233](https://github.com/sandboxie-plus/Sandboxie/issues/233）
  - 此修复解决了 CP2077 和其他游戏无法获得键盘输入的问题（感谢 Rostok）
- 修复了 ClipCursor 失败不再扩展消息日志
- 修复了在 SandMan UI 中添加恢复文件夹的问题
- 修复了在使用非默认 Sbie 安装位置时 Office 2019 模板出现的问题
- 修复了设置沙箱文件夹最后访问属性的问题 [#218](https://github.com/sandboxie-plus/Sandboxie/issues/218）
- 修复了进程启动信号的问题



## [0.5.1 / 5.45.0] - 2020-12-12

### 新增
- 增加简单视图模式

### 更改
- 更新 SandMan UI 以使用 Qt 5.15.1

### 修复
- 修复了进度对话框崩溃问题
- 修复了更新检查器的进度对话框取消按钮无法工作的问题
- 修复了删除沙箱内容时与 NtQueryDirectoryFile 相关的问题
- 修复了通知窗口中的暗黑主题问题
- 修复了禁用强制程序托盘菜单的问题



## [0.5.0 / 5.45.0] - 2020-12-06

### 新增
- 增加新的通知窗口
- 增加用户交互控制机制，当使用新的 SandMan UI 时
  - 当文件超过复制限制时，不再失败，系统会提示用户是否复制该文件
  - 当互联网访问被阻止时，用户现在可以实时豁免该限制
- 增加缺失的文件恢复和自动/快速恢复功能 [#188](https://github.com/sandboxie-plus/Sandboxie/issues/188） [#178](https://github.com/sandboxie-plus/Sandboxie/issues/178）
- 增加静默的 MSG_1399 沙箱进程启动通知，以跟踪短暂的沙箱进程
- 增加防止全局进程启动的能力，Sandboxie 现在不仅可以警告，还可以阻止警告列表中的进程
  - 设置 "StartRunAlertDenied=y" 以启用进程阻止
- 进程启动警报/阻止机制现在也可以处理文件夹，使用 "AlertFolder=..."
- 增加合并快照的能力 [#151](https://github.com/sandboxie-plus/Sandboxie/issues/151）
- 在新 UI 中为沙箱上下文菜单添加图标
- 在沙箱选项窗口中增加更多高级选项
- 增加文件迁移进度指示器
- 增加更多运行命令和每个沙箱的自定义运行命令
  - 用户现在可以指定程序，以便在沙箱的运行菜单中可用
  - 还可以从预设菜单中将进程固定到该列表
- 增加更多 Windows 10 特定模板预设
- 增加创建桌面快捷方式到沙箱化项目的能力
- 为盒子选项标签增加图标
- 增加盒子分组
- 增加新的调试选项 "DebugTrace=y" 以记录调试输出到跟踪日志
- 在新 SandMan UI 中增加检查更新功能
- 在传统 SbieCtrl UI 中增加检查更新功能

### 已更改
- 可以通过指定 "CopyLimitKb=-1" 禁用文件迁移限制 [#526](https://github.com/sandboxie-plus/Sandboxie/issues/526)
- 改进和重构了消息记录机制，内存使用减少了 2 倍
- 终止的盒装进程现在会列出几秒钟
- 重新设计了新 UI 的沙箱删除机制
- 重新构建了沙箱选项窗口
- 现在可以使用最新的 ntdll.lib 编译 SbieDLL.dll（感谢 Team-IRA 的 TechLord 的帮助）
- 改进了自动驱动程序自我修复

### 修复
- 修复了迁移大于 4GB 的文件的问题
- 修复了允许恶意应用绕过互联网封锁的问题
- 修复了从非沙箱进程记录消息时的问题，向 API_LOG_MESSAGE_ARGS 添加了 process_id 参数
- 修复了本地化问题
- 修复了在 Legacy UI SbieCtrl.exe 中使用文件恢复的问题，当 "SeparateUserFolders=n" 被设置时
- 当程序因限制而被阻止启动时，不再发出多余的消息
- 修复了 UI 未正确显示异步错误的问题
- 修复了快照操作失败时的问题
- 修复了新 UI 中 IpcPath 和 WinClass 的一些特殊情况
- 修复了通过 WHQL 兼容性测试的驱动程序问题
- 修复了 Classic 安装程序的问题

## [0.4.5 / 5.44.1] - 2020-11-16

### 新增
- 在 SandMan UI 的托盘菜单中添加了 "终止所有进程" 和 "禁用强制程序" 命令
- 程序启动限制设置现在可以在白名单和黑名单之间切换
  - 可以从上下文菜单终止并将程序列入黑名单
- 添加了额外的进程上下文菜单选项，现在可以从菜单中设置持久进程和领头进程
- 添加了查看任何给定盒子模板预设的选项
- 在模板视图中添加了文本过滤器
- 添加了新的兼容性模板：
  - Windows 10 核心 UI 组件：OpenIpcPath=\BaseNamedObjects\[CoreUI]-* 解决中文输入和表情符号的问题 [#120](https://github.com/sandboxie-plus/Sandboxie/issues/120) [#88](https://github.com/sandboxie-plus/Sandboxie/issues/88)
  - Firefox Quantum，访问 Windows 的 FontCachePort 以与 Windows 7 兼容
- 添加了实验性调试选项 "OriginalToken=y"，允许沙箱进程保留其原始的未受限制的令牌
  - 此选项与 "OpenToken=y" 可比，仅用于测试和调试，因为它会破坏大多数安全措施（！）
- 添加了调试选项 "NoSandboxieDesktop=y"，禁用桌面代理机制
  - 注意：没有未受限制的令牌，使用此选项的应用程序将无法启动
- 添加了调试选项 "NoSysCallHooks=y"，禁用驱动程序的系统调用处理
  - 注意：没有未受限制的令牌，使用此选项的应用程序将无法启动
- 添加了记录详尽访问轨迹到资源监视器的能力
  - 使用 INI 选项 "FileTrace=*", "PipeTrace=*", "KeyTrace=*", "IpcTrace=*", "GuiTrace=*" 记录所有事件
  - 用 "*" 替换只记录： "A" - 允许，"D" - 拒绝，或 "I" - 忽略事件
- 添加了将调试输出字符串记录到资源监视器的能力
  - 使用 INI 选项 DebugTrace=y 进行启用

### 已更改
- AppUserModelID 字符串不再包含 Sandboxie 版本字符串
- 默认情况下，Sbie 的应用程序清单黑客已被禁用，因为它会导致 Windows 10 上的版本检查问题
  - 要启用旧行为，请向全局或特定盒子 INI 部分添加 "PreferExternalManifest=y"
- 资源日志机制现在可以处理多个字符串，以减少字符串拷贝操作

### 修复
- 修复了禁用某些限制设置失败的问题
- 修复了从预设菜单禁用互联网封锁有时失败的问题
- SandMan UI 中的兼容性列表现在显示正确的模板名称
- 修复了驱动程序中使用已释放内存的问题
- 将 swprintf 替换为 snwprintf，以防止 SbieDll.dll 中潜在的缓冲区溢出
- 修复了在 SandMan UI 中资源日志和 API 日志的性能问题

## [0.4.4 / 5.44.0] - 2020-11-03

### 新增
- 添加了 SbieLdr（实验性）

### 已更改
- 将代码注入机制从 SbieSvc 移动到 SbieDll
- 将函数钩子机制从 SbieDrv 移动到 SbieDll
- 引入了一种无驱动的新方法来解析 wow64 ntdll 基址

### 已移除
- 移除了对 Windows Vista x64 的支持

## [0.4.3 / 5.43.7] - 2020-11-03

### 新增
- 在 SandMan UI 添加了禁用强制程序菜单命令

### 修复
- 修复了因早期 Driver Verifier 修复引入的文件重命名错误 [#174](https://github.com/sandboxie-plus/Sandboxie/issues/174) [#153](https://github.com/sandboxie-plus/Sandboxie/issues/153)
- 修复了保存访问列表的问题
- 修复了 SandMan UI 中程序组解析的问题
- 修复了互联网访问限制选项的问题 [#177](https://github.com/sandboxie-plus/Sandboxie/issues/177) [#185](https://github.com/sandboxie-plus/Sandboxie/issues/185)
- 修复了在直接驱动器上删除沙箱的问题 [#139](https://github.com/sandboxie-plus/Sandboxie/issues/139)

## [0.4.2 / 5.43.6] - 2020-10-10

### 新增
- 添加了 "探索盒子" 右键菜单选项

### 修复
- 修复了 SbieSvc 和其他组件中的线程句柄泄漏 [#144](https://github.com/sandboxie-plus/Sandboxie/issues/144)
- msedge.exe 现在被归类为 Chromium 派生物
- 修复了与 Chrome 自身沙箱的 Chrome 86+ 兼容性错误 [#149](https://github.com/sandboxie-plus/Sandboxie/issues/149)

## [0.4.1 / 5.43.5] - 2020-09-12

### 新增
- 在 SandMan UI 中添加核心版本兼容性检查
- 向 SbiePlus 添加了外壳集成选项

### 已更改
- SbieCtrl 不再在首次启动时自动显示教程
- 在钩住跳板时，原始函数的迁移部分不再被 noped，这会导致 Unity 游戏出现问题

### 修复
- 修复了深色模式下垂直标签的颜色问题
- 修复了添加新强制文件夹时路径分隔符错误的问题
- 修复了在 5.43 中引入的目录列表错误
- 修复了在未连接到驱动程序时设置窗口的问题
- 修复了以管理员身份启动 SandMan UI 时的问题
- 修复了 SandMan UI 中自动内容删除未工作的情况

## [0.4.0 / 5.43] - 2020-09-05

### 新增
- 向 Plus 版本添加了正式的自定义安装程序
- 向 Sbie 核心添加了沙箱快照功能
  - 文件系统以增量方式保存，快照相互叠加
  - 目前每个快照都会获得盒子注册表的完整副本
  - 每个快照可以有多个子快照
- 向资源监视器添加了访问状态
- 添加了更改边框宽度的设置 [#113](https://github.com/sandboxie-plus/Sandboxie/issues/113)
- 向 SandMan 添加了快照管理器 UI
- 添加了用于验证 Yubikey 或可比 2FA 设备的模板
- 添加了程序警报的 UI
- 向 UI 添加了软件兼容性选项

### 已更改
- SandMan UI 现在自行处理沙箱内容的删除
- 不再新增冗余的资源访问作为新事件

### 修复
- 修复了从延迟加载库钩住函数时的问题
- 修复了钩住已钩住函数时的问题
- 修复了新盒子设置编辑器的问题

### 已移除
- 移除了针对过时的反恶意软件产品的钩子机制中的废弃临时解决方案

## [0.3.5 / 5.42.1] - 2020-07-19

### 新增
- 新增设置窗口
- 新增翻译支持
- 新增黑暗主题
- 新增自动启动选项
- 新增沙盒选项
- 新增调试选项 "NoAddProcessToJob=y"

### 更改
- 改进空沙盒托盘图标
- 改进消息解析
- 更新主页链接

### 修复
- 修复 SandMan.exe 在重命名沙盒时的 INI 问题
- 修复上一个版本引入的 INI 自动重新加载 bug
- 修复钩住延迟加载的库时出现的问题


## [0.3 / 5.42] - 2020-07-04

### 新增
- 现在可以使用 API_QUERY_PROCESS_INFO 获取沙盒进程的原始进程令牌
  - 注意：此功能由 TaskExplorer 使用，以便检查沙盒内部令牌
- 新增选项 "KeepTokenIntegrity=y"，使 Sbie 令牌保持其初始完整性级别（调试选项）
  - 注意：如果您不清楚其安全隐患，请勿使用调试选项（！）
- 日志消息中新增进程 ID，对调试非常有用
- 资源日志中新增查找器
- 新增选项 "HideHostProcess=program.exe"，以隐藏未沙盒的主机进程
  - 注意：Sbie 默认会隐藏其他沙盒的进程，现在可以通过 "HideOtherBoxes=n" 来控制该行为
- 沙盒的 RpcSs 和 DcomLaunch 现在可以在选项 "ProtectRpcSs=y" 下作为系统运行，但这会导致沙盒的 Windows 资源管理器等出现问题
- 现在可以使用 "OpenDefaultClsid=n" 禁用内置的 Clsid 白名单
- 现在可以使用 del 键终止进程，并需要确认
- 新增沙盒窗口边框显示到 SandMan.exe
- 新增 Sbie 日志消息的通知
- 新增沙盒预设子菜单，以快速更改一些设置
  - 启用/禁用 API 日志记录；LogAPI DLL 现已与 Sandboxie Plus 一起分发
  - 移除管理员权限
  - 阻止/允许互联网访问
  - 阻止/允许访问网络文件
- 在沙盒状态列中新增更多信息
- 在 SbieModel 中新增路径列
- 在 SbieView 中新增信息工具提示

### 更改
- 重新设计 ApiLog，新增 PID 和 PID 过滤器
- 对更改的自动配置重新加载现在延迟 500 毫秒，以防止在增量更改时多次重新加载
- 沙盒名称现在将 "_" 替换为 " "，从而启用由单独单词组成的名称

### 修复
- 已在便携模式中新增缺失的 PreferExternalManifest 初始化
- 修复了安全问题 ID-2：修复沙盒系统进程的权限问题
  - 注意：可以使用 "ExposeBoxedSystem=y" 恢复旧行为（调试选项）
- 修复了安全问题 ID-3：修复沙盒服务的 SCM 访问检查缺失（感谢 Diversenok）
  - 注意：要禁用访问检查，请使用 "UnrestrictedSCM=y"（调试选项）
- 修复了服务服务器中的缺失初始化，导致沙盒程序在查询服务状态时崩溃
- 修复了许多在启用驱动程序验证器时导致 SbieDrv.sys 出现蓝屏的问题 [#57](https://github.com/sandboxie-plus/Sandboxie/issues/57)
  - GetThreadTokenOwnerPid 和 File_Api_Rename 中的 0xF6
  - File_PreOperation 中 FltGetFileNameInformation 的缺失非可选参数
  - Key_StoreValue 和 Key_PreDataInject 中的 0xE3


## [0.2.2 / 5.41.2] - 2020-06-19

### 新增
- 新增选项 "SeparateUserFolders=n"，以不再将用户配置文件文件单独存储在沙盒中
- 新增 "SandboxieLogon=y"，使进程在 "Sandboxie" 用户的 SID 下运行，而非匿名用户
  - 注意：必须启用全局选项 "AllowSandboxieLogon=y"，并首先手动创建 "Sandboxie" 用户帐户，重载驱动程序，否则进程启动将失败
- 改进了驱动程序中关于进程创建错误的调试

### 修复
- 修复了驱动程序重载后日志消息丢失的问题
- 修复了 MSI 安装程序问题，请参见 Proc_CreateProcessInternalW_RS5


## [0.2.1 / 5.41.1] - 2020-06-18

### 新增
- 为不同类型新增不同的沙盒图标
  - 红色 LogAPI/BSA 启用
  - 还有更多即将推出 :D
- 为异步操作添加了进度窗口
- 新增 DPI 适应性 [#56](https://github.com/sandboxie-plus/Sandboxie/issues/56)
- 驱动程序文件现在被混淆，以避免误报
- 为 Sandboxie.ini 新增额外的调试选项 OpenToken=y，结合了 UnrestrictedToken=y 和 UnfilteredToken=y
  - 注意：使用这些选项会削弱沙盒隔离，它们用于调试，并可用于以后的更好应用程序虚拟化

### 更改
- 在处理 InjectDll 时，SbieDll.dll 现在如果输入路径以反斜杠开头，将在 SbieHome 文件夹中查找 DLL
  - 即 "InjectDll=\LogAPI\i386\logapi32v.dll" 或 "InjectDll64=\LogAPI\amd64\logapi64v.dll"

### 修复
- IniWatcher 在便携模式下无法正常工作
- 服务路径修复导致其他服务出现问题
- 针对 MSI 安装程序问题的解决方法


## [0.2 / 5.41.0] - 2020-06-08

### 新增
- IniWatcher，.ini 文件现在在每次更改时自动重新加载
- 在沙盒菜单中新增维护菜单，允许安装/卸载和启动/停止 Sandboxie 驱动程序、服务
- SandMan.exe 现在打包了 Sbie 文件，当没有安装 Sbie 时，作为便携式安装使用
- 新增清理日志的选项

### 更改
- Sbie 驱动程序现在首先检查主路径中的配置文件 Sandboxie.ini，然后再检查 SystemRoot

### 修复
- 修复安全问题 ID-1：沙盒进程可能在未沙盒的进程上获取写入句柄（感谢 Diversenok）
  - 这允许将代码注入未沙盒的进程
- 修复了路径中包含空格时沙盒服务未启动的问题
- NtQueryInformationProcess 现在返回沙盒进程的正确沙盒路径


## [0.1 / 5.40.2] - 2020-06-01

### 新增
- 创建了一个基于 Qt 的新用户界面，名为 SandMan（Sandboxie 管理器）
- 资源监视器现在显示 PID
- 新增使用更新的 BSA LogAPI 库的基本 API 调用日志

### 更改
- 重新设计资源监视器，以支持多个事件消费者
- 重新设计日志，以支持多个事件消费者


## [5.40.1] - 2020-04-10

### 新增
- 为资源访问监视器新增 "其他" 类型
- 在已记录资源中添加对 StartService 的调用

### 修复
- 修复了与 Windows 1903 相关的 "无法访问 Windows Installer 服务" 问题
