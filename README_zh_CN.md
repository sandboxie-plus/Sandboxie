# Sandboxie Plus / Classic

<p align='center'>
<a href='./README.md'>EN</a> | 中文
</p>

[![Plus license](https://img.shields.io/badge/Plus%20license-Custom%20-blue.svg)](./LICENSE.Plus) [![Classic license](https://img.shields.io/github/license/Sandboxie-Plus/Sandboxie?label=Classic%20license&color=blue)](./LICENSE.Classic) [![GitHub Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie.svg)](https://github.com/sandboxie-plus/Sandboxie/releases/latest) [![GitHub Pre-Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie/all.svg?label=pre-release)](https://github.com/sandboxie-plus/Sandboxie/releases) [![GitHub Build Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/main.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions) [![GitHub Codespell Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml) [![WinGet Build Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/winget.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/winget.yml) [![Gurubase](https://img.shields.io/badge/Gurubase-Ask%20Sandboxie%20Guru-006BFF)](https://gurubase.io/g/sandboxie)

[![Roadmap](https://img.shields.io/badge/Roadmap-Link%20-blue?style=for-the-badge)](https://www.wilderssecurity.com/threads/updated-sandboxie-plus-roadmap.456886/) [![Join our Discord Server](https://img.shields.io/badge/Join-Our%20Discord%20Server%20for%20bugs,%20feedback%20and%20more!-blue?style=for-the-badge&logo=discord)](https://discord.gg/S4tFu6Enne)

|      系统要求      |      发布说明      |     贡献指南      |      安全政策      |      行为准则       |
|        :---:       |          :---:     |          :---:    |          :---:      |          :---:       |
| Windows 7 或更高版本 (64位) | [CHANGELOG_zh_CN.md](./CHANGELOG_zh_CN.md) | [CONTRIBUTING.md](./CONTRIBUTING.md) | [SECURITY.md](./SECURITY.md) | [CODE_OF_CONDUCT.md](./CODE_OF_CONDUCT.md) |

Sandboxie 是一种基于沙盒的隔离软件，适用于 Windows NT 操作系统。它创建了一个类似沙盒的隔离操作环境，在该环境中，可以运行或安装应用程序，而不对本地和映射驱动器及 Windows 注册表进行永久性修改。隔离的虚拟环境可以控制不受信任程序的测试和网络浏览。<br>

Sandboxie 允许您创建几乎无限的沙盒，并单独或同时运行它们，以隔离程序与主机及彼此的访问，同时还允许您在单个沙盒中同时运行尽可能多的程序。

**注意：这是一个社区分支，发生在 Sandboxie 源代码发布后，并不是之前开发的官方延续（请参见 [项目历史](#project-history) 和 [#2926](https://github.com/sandboxie-plus/Sandboxie/issues/2926)）。**

## ⏬ 下载

[最新版本](https://github.com/sandboxie-plus/Sandboxie/releases/latest)

## ✨ 更新日志

<a href='./CHANGELOG_zh_CN.md'>中文</a>

## 🚀 功能

Sandboxie 提供两个版本，Plus 和 Classic。它们共享相同的核心组件，这意味着它们具有相同的安全性和兼容性水平。
不同之处在于用户界面中功能的可用性。

Sandboxie Plus 拥有基于现代 Qt 的用户界面，支持自项目开源以来添加的所有新功能：

  * 快照管理器 - 复制任意沙盒的状态副本以便在需要时恢复
  * 维护菜单 - 支持在需要时卸载/安装/启动/停止 Sandboxie 驱动程序和服务
  * 便携模式 - 运行安装程序并选择将所有文件解压到指定目录
  * 额外的界面选项，可阻止访问 Windows 组件如打印机后台处理程序和剪贴板
  * 针对“开始/运行”和互联网访问限制提供更多自定义选项
  * 隐私模式沙盒，保护用户数据免受非法访问
  * 安全增强沙盒，限制系统调用和端点的可用性
  * 全局热键，用于挂起或终止所有沙盒内进程
  * 每个沙盒的网络防火墙，支持 Windows 过滤平台（WFP）
  * 沙盒列表支持使用快捷键 Ctrl+F 搜索
  * 支持搜索全局设置和沙盒选项
  * 支持将沙盒导入/导出为 7z 文件
  * 沙盒集成到 Windows 开始菜单
  * 一个浏览器兼容性向导，用于为不支持的浏览器创建模板
  * 经典视图模式，重现 Sandboxie Control 的图形界面
  * 故障排除向导，帮助用户解决常见问题
  * 插件管理器，通过附加组件扩展或新增功能
  * 保护沙盒免受主机影响，包括防止截图
  * 触发系统，当沙盒经历初始化、启动、终止或文件恢复等不同阶段时执行操作
  * 允许某进程不受沙盒限制，但其子进程受沙盒保护
  * 将沙盒作为一种控制单元，强制程序自动使用 SOCKS5 代理
  * DNS 解析控制，具有沙盒粒度的控制能力
  * 限制沙盒中的进程数量及其占用的总内存空间，并且可以限制每个沙盒中的沙盒化进程总数
  * 完全不同于 Sandboxie 开源前版本的令牌创建机制，使沙盒在系统中更具独立性
  * 加密沙盒 - 基于 AES 的可靠数据存储解决方案
  * 防止沙盒程序以常规方式生成不必要的唯一标识符
  * 内置 INI 编辑器，可通过可视提示和工具提示帮助用户理解已配置或想要添加的设置
  * 支持配置外部文本编辑器，而不仅限于系统默认编辑器
  * 支持控制边框的 Alpha 透明度
  * 自定义 UAC 对话框，可模拟权限、授予权限或取消提权尝试
  * 现代化图标，并可在部分位置使用旧式图标
  * 可以更改用户界面字体
  * 沙盒或分组可以使用自定义颜色或图标

更多功能可以通过在 [CHANGELOG_zh_CN.md](./CHANGELOG_zh_CN.md) 文件中使用快捷键 Ctrl+F 查找符号 `=` 来发现。

Sandboxie Classic 采用了不再开发的基于 MFC 的旧版界面，因此它不支持 Plus 功能的原生界面。尽管部分缺失的功能可以通过手动编辑 Sandboxie.ini 配置文件或使用[自定义脚本](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/viewforum1a2d1a2d.html?f=22)来实现替代，但不建议希望体验最新安全选项的用户使用 Classic 版本。

## 📚 文档

目前维护着一份 [Sandboxie 文档](https://sandboxie-plus.github.io/sandboxie-docs) 的 GitHub 副本，但仍需要更多志愿者来跟进新变化并保持其更新。建议同时关注以下标签以追踪当前的问题：[Labels · sandboxie-plus/Sandboxie](https://github.com/sandboxie-plus/Sandboxie/labels)。

之前由 Invincea 维护的 [旧 Sandboxie 论坛](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums) 的部分存档仍然可用。如果需要查找特定内容，可以使用以下搜索查询：`site:https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/`


## 🚀 Sandboxie 的实用工具

Sandboxie 的功能可以通过以下专业工具进行增强：

  * [LogApiDll](https://github.com/sandboxie-plus/LogApiDll) - 为 Sandboxie 的跟踪日志添加详细输出，列出相关 Windows API 函数的调用情况
  * [SbieHide](https://github.com/VeroFess/SbieHide) - 尝试隐藏 SbieDll.dll 在被沙盒化应用中的存在
  * [SandboxToys2](https://github.com/blap/SandboxToys2) - 允许监控沙盒内的文件和注册表更改
  * [Sbiextra](https://github.com/sandboxie-plus/sbiextra) - 对沙盒进程添加额外的用户模式限制
  * [WrapLocale](https://github.com/UserUnknownFactor/WrapLocale) - 提供比原生 LangId 功能更灵活的区域设置伪装选项

<a id="project-history"></a>
## 📌 项目历史

|      时间线       |    维护者    |
|        :---       |       :---   |
| 2004 - 2013       | Ronen Tzur   |
| 2013 - 2017       | Invincea Inc. |
| 2017 - 2020       | Sophos Group plc |
| 2020年4月8日 - [开源代码](https://community.sophos.com/sandboxie/f/forum/119641/important-sandboxie-open-source-code-is-available-for-download) | Sophos Ltd. |
| 2020年4月9日起 - 项目分支 | David Xanatos |

想查找旧版 Sandboxie？请查看[版本历史](https://github.com/sandboxie-plus/sandboxie-old)。

请查看当前 [路线图](https://www.wilderssecurity.com/threads/updated-sandboxie-plus-roadmap.456886/)。

## 📌 项目支持/赞助

[<img align="left" height="64" width="64" src="./.github/images/binja-love.png">](https://binary.ninja/)
感谢 [Vector 35](https://vector35.com/) 提供的 [Binary Ninja](https://binary.ninja/) 许可证，帮助进行逆向工程。
<br>
Binary Ninja 是一个多平台交互式反汇编、反编译和二进制分析工具，专为逆向工程师、恶意软件分析师、漏洞研究人员和软件开发人员设计。<br>
<br>
[<img align="left" height="64" width="64" src="./.github/images/Icons8_logo.png">](https://icons8.de/)感谢 [Icons8](https://icons8.de/) 为项目提供图标。
<br>
<br>
<br>

## 🤝 支持该项目

如果您发现 Sandboxie 有用，请随时通过我们的 [贡献指南](./CONTRIBUTING.md) 进行贡献。

## 📑 感谢贡献者

- DavidBerdik - [Sandboxie 网站存档](https://github.com/Sandboxie-Website-Archive/sandboxie-website-archive.github.io) 维护者
- Jackenmen - Sandboxie 的 Chocolatey 软件包维护者（[支持](https://github.com/Jackenmen/choco-auto/issues?q=is%3Aissue+Sandboxie)）
- vedantmgoyal9 - Sandboxie 的 Winget Releaser 维护者（[支持](https://github.com/vedantmgoyal9/winget-releaser/issues?q=is%3Aissue+Sandboxie)）
- blap - [SandboxToys2](https://github.com/blap/SandboxToys2) 插件维护者
- diversenok - 安全分析与 PoC / 安全修复
- TechLord - Team-IRA / 逆向工程
- hg421 - 安全分析与 PoC / 代码评审
- hx1997 - 安全分析与 PoC
- mpheath - Plus 安装程序作者 / 代码修复 / 合作者
- offhub - 文档补充 / 代码修复 / Qt5 补丁与构建脚本 / 合作者
- LumitoLuma - Qt5 补丁与构建脚本
- QZLin - [sandboxie-docs](https://sandboxie-plus.github.io/sandboxie-docs/) 主题作者
- isaak654 - 模板 / 文档 / 代码修复 / 合作者
- typpos - 界面新增功能 / 文档 / 代码修复
- Yeyixiao - 功能新增
- Deezzir - 功能新增
- wzxjohn - 代码修复 / 文档补充
- okrc - 代码修复
- Sapour - 代码修复
- lmou523 - 代码修复
- sredna - Classic 安装程序的代码修复
- weihongx9315 - 代码修复
- marti4d - 代码修复
- jorgectf - CodeQL 工作流
- stephtr - CI / 认证
- yfdyh000 - Plus 安装程序的本地化支持
- Dyras - 模板补充
- cricri-pingouin - 界面修复
- Valinwolf - 界面 / 图标
- daveout - 界面 / 图标
- kokofixcomputers - [Discord](https://discord.gg/S4tFu6Enne) 频道支持成员
- NewKidOnTheBlock - 更新日志修正
- Naeemh1 - 文档补充
- APMichael - 模板补充
- 1mm0rt41PC - 文档补充
- Luro223 - 文档补充
- lwcorp - 文档补充
- wilders-soccerfan - 文档补充
- LepordCat - 文档补充
- stdedos - 文档补充
- habatake - 界面新增功能 / 代码修复
- Polyester6719 - 文档补充

## 🌏 翻译人员

- czoins - 阿拉伯语
- yuhao2348732、0x391F、nkh0472、yfdyh000、gexgd0419、Zerorigin、UnnamedOrange、DevSplash、Becods、okrc、4rt3mi5、sepcnt、fzxx、Vstory、GT-Stardust、habatake - 简体中文
- TragicLifeHu、Hulen、xiongsp、habatake - 繁体中文
- RockyTDR - 荷兰语
- clexanis、Mmoi-Fr、hippalectryon-0、Monsieur Pissou - 法语（通过电子邮件提供）
- bastik-1001、APMichael - 德语
- timinoun - 匈牙利语（通过电子邮件提供）
- isaak654、DerivativeOfLog7 - 意大利语
- takahiro-itou、lllIIIlll - 日语
- VenusGirl - 韩语
- divinity76 - 书面挪威语
- 7zip、AndrzejRafalowski - 波兰语（[单独提供](https://forum.xanasoft.com/threads/polish-translation.4/page-2)）
- JNylson - 葡萄牙语及巴西葡萄牙语
- lufog、marat2509 - 俄语
- LumitoLuma、sebadamus - 西班牙语
- 1FF、Thatagata - 瑞典语（通过电子邮件或拉取请求提供）
- xorcan、fmbxnary、offhub - 土耳其语
- SuperMaxusa、lufog、Nazar1ky - 乌克兰语
- GunGunGun - 越南语

所有译者在提交翻译前，建议先查看[本地化说明和提示](https://git.io/J9G19)。

## 📚 文档翻译人员

- Vstory、GT-Stardust、wzxjohn、SOLEADO20、habatake - 简体中文

所有文档译者在提交翻译前，建议先查看[多语言翻译贡献指南](https://github.com/sandboxie-plus/sandboxie-docs/issues/175#issuecomment-2840258519)。
