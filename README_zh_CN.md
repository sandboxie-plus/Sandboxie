# Sandboxie Plus / Classic

<p align='center'>
<a href='./README.md'>EN</a> | 中文
</p>

[![Plus license](https://img.shields.io/badge/Plus%20license-Custom%20-blue.svg)](./LICENSE.Plus) [![Classic license](https://img.shields.io/github/license/Sandboxie-Plus/Sandboxie?label=Classic%20license&color=blue)](./LICENSE.Classic) [![GitHub Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie.svg)](https://github.com/sandboxie-plus/Sandboxie/releases/latest) [![GitHub Pre-Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie/all.svg?label=pre-release)](https://github.com/sandboxie-plus/Sandboxie/releases) [![GitHub Build Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/main.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions) [![GitHub Codespell Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml) [![WinGet Build Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/winget.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/winget.yml) [![Gurubase](https://img.shields.io/badge/Gurubase-Ask%20Sandboxie%20Guru-006BFF)](https://gurubase.io/g/sandboxie)

[![Roadmap](https://img.shields.io/badge/Roadmap-Link%20-blue?style=for-the-badge)](https://www.wilderssecurity.com/threads/sandboxie-roadmap.445545/page-8#post-3187633) [![Join our Discord Server](https://img.shields.io/badge/Join-Our%20Discord%20Server%20for%20bugs,%20feedback%20and%20more!-blue?style=for-the-badge&logo=discord)](https://discord.gg/S4tFu6Enne)

|      系统要求      |      发布说明      |     贡献指南      |      安全政策      |      行为准则       |
|        :---:       |          :---:     |          :---:    |          :---:      |          :---:       |
| Windows 7 或更高版本，32位或 64位。 | [CHANGELOG_zh_CN](./CHANGELOG_zh_CN) | [CONTRIBUTING.md](./CONTRIBUTING.md) | [SECURITY.md](./SECURITY.md) | [CODE_OF_CONDUCT.md](./CODE_OF_CONDUCT.md) |

Sandboxie 是一种基于沙盒的隔离软件，适用于 32 位和 64 位 Windows NT 操作系统。它创建了一个类似沙盒的隔离操作环境，在该环境中，可以运行或安装应用程序，而不对本地和映射驱动器及 Windows 注册表进行永久性修改。隔离的虚拟环境可以控制不受信任程序的测试和网络浏览。<br>

Sandboxie 允许您创建几乎无限的沙盒，并单独或同时运行它们，以隔离程序与主机及彼此的访问，同时还允许您在单个沙盒中同时运行尽可能多的程序。

**注意：这是一个社区分支，发生在 Sandboxie 源代码发布后，并不是之前开发的官方延续（请参见 [项目历史](./README.md#-project-history) 和 [#2926](https://github.com/sandboxie-plus/Sandboxie/issues/2926)）。**

## ⏬ 下载

[最新版本](https://github.com/sandboxie-plus/Sandboxie/releases/latest)

## ✨ 更新日志

<a href='./CHANGELOG.md'>EN</a> | <a href='./CHANGELOG_zh_CN.md'>中文</a>

## 🚀 功能

Sandboxie 提供两个版本，Plus 和 Classic。它们共享相同的核心组件，这意味着它们具有相同的安全性和兼容性。不同之处在于用户界面中功能的可用性。

Sandboxie Plus 具有现代的基于 Qt 的用户界面，支持自项目开源以来添加的所有新功能：

  * 快照管理器 - 对任何沙盒进行复制，以便在需要时恢复
  * 维护菜单 - 允许在需要时卸载/安装/启动/停止 Sandboxie 驱动程序和服务
  * 便携模式 - 您可以运行安装程序并选择将所有文件提取到一个目录
  * 额外的用户界面选项，以阻止对 Windows 组件如打印后台处理程序和剪贴板的访问
  * 对启动/运行和互联网访问限制的更多自定义选项
  * 隐私模式沙盒，保护用户数据不被非法访问
  * 安全增强沙盒，限制系统调用和端点的可用性
  * 全局热键，可暂停或终止所有沙盒进程
  * 每个沙盒的网络防火墙，支持 Windows 过滤平台（WFP）
  * 可以使用快捷键 Ctrl+F 搜索沙盒列表
  * 全球设置和沙盒选项的搜索功能
  * 能够将沙盒导入/导出为 7z 文件
  * 将沙盒集成到 Windows 开始菜单
  * 浏览器兼容性向导，创建不支持浏览器的模板
  * 老式视图模式，以重现 Sandboxie 控制的图形外观
  * 故障排除向导，以帮助用户解决问题
  * 附加组件管理器，通过额外组件扩展或添加功能
  * 沙盒对主机的保护，包括防止截屏
  * 触发器系统，用于在沙盒经历不同阶段（如初始化、沙盒启动、终止或文件恢复）时执行操作
  * 使一个进程不在沙盒中，但其子进程在沙盒中
  * 将沙盒作为控制单位，强制程序自动使用 SOCKS5 代理
  * DNS 解析控制，沙盒化为控制粒度
  * 限制沙盒内进程的数量及它们可以占用的总内存空间，您可以限制每个沙盒的总沙盒化进程数量
  * 与 Sandboxie 的预开源版本完全不同的令牌创建机制，使沙盒在系统中更加独立
  * 加密沙盒 - 基于 AES 的可靠数据存储解决方案
  * 防止沙盒化程序以正常方式生成不必要的唯一标识符

更多功能可以通过在 [CHANGELOG.md](./CHANGELOG.md) 文件中使用快捷键 Ctrl+F 找到标记 `=`。

Sandboxie Classic 具有旧版不再开发的基于 MFC 的用户界面，因此不支持 Plus 功能的原生界面支持。虽然一些缺失的功能可以通过 Sandboxie.ini 配置文件手动配置，甚至可以通过 [自定义脚本](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/viewforum1a2d1a2d.html?f=22) 进行替换，但不推荐希望探索最新安全选项的用户使用 Classic 版本。

## 📚 文档

目前维护的 [Sandboxie 文档](https://sandboxie-plus.github.io/sandboxie-docs) 的 GitHub 副本，虽然需要更多志愿者来保持与新变化的更新。建议您也检查以下标签以跟踪当前问题：[Labels · sandboxie-plus/Sandboxie](https://github.com/sandboxie-plus/Sandboxie/labels)。

之前由 Invincea 维护的 [旧 Sandboxie 论坛](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums) 的部分存档仍然可用。如果您需要找到某些特定内容，可以使用以下搜索查询：`site:https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/`。

## 🚀 对 Sandboxie 的有用工具

Sandboxie 的功能可以通过以下专用工具进行增强：

  * [LogApiDll](https://github.com/sandboxie-plus/LogApiDll) - 向 Sandboxie 的跟踪日志添加详细输出，列出相关 Windows API 函数的调用
  * [SbieHide](https://github.com/VeroFess/SbieHide) - 尝试隐藏 SbieDll.dll 在被沙盒化的应用程序中的存在
  * [SandboxToys2](https://github.com/blap/SandboxToys2) - 允许在沙盒中监控文件和注册表变化
  * [Sbiextra](https://github.com/sandboxie-plus/sbiextra) - 为沙盒化进程添加额外的用户模式限制

## 📌 项目历史

|      时间线       |    维护者    |
|        :---       |       :---   |
| 2004 - 2013       | Ronen Tzur   |
| 2013 - 2017       | Invincea Inc. |
| 2017 - 2020       | Sophos Group plc |
| 2020年4月8日 - [开源代码](https://community.sophos.com/sandboxie/f/forum/119641/important-sandboxie-open-source-code-is-available-for-download) | Sophos Ltd. |
| 2020年4月9日 - 项目分支 | David Xanatos |

请查看当前 [路线图](https://www.wilderssecurity.com/threads/sandboxie-roadmap.445545/page-8#post-3187633)。

## 📌 项目支持/赞助

[<img align="left" height="64" width="64" src="https://github.com/sandboxie-plus/Sandboxie/raw/master/.github/images/binja-love.png">](https://binary.ninja/)
感谢 [Vector 35](https://vector35.com/) 提供的 [Binary Ninja](https://binary.ninja/) 许可证，帮助进行逆向工程。
<br>
Binary Ninja 是一个多平台交互式反汇编、反编译和二进制分析工具，专为逆向工程师、恶意软件分析师、漏洞研究人员和软件开发人员设计。<br>
<br>
[<img align="left" height="64" width="64" src="https://github.com/sandboxie-plus/Sandboxie/raw/master/.github/images/Icons8_logo.png">](https://icons8.de/)感谢 [Icons8](https://icons8.de/) 为项目提供图标。
<br>
<br>
<br>

## 🤝 支持该项目

如果您发现 Sandboxie 有用，请随时通过我们的 [贡献指南](./CONTRIBUTING.md) 进行贡献。

## 📑 感谢贡献者

- DavidBerdik - [Sandboxie 网站档案](https://github.com/Sandboxie-Website-Archive/sandboxie-website-archive.github.io) 维护者
- Jackenmen - Sandboxie 的 Chocolatey 包维护者 ([支持](https://github.com/Jackenmen/choco-auto/issues?q=is%3Aissue+Sandboxie))
- vedantmgoyal9 - Sandboxie 的 Winget 发布者维护者 ([支持](https://github.com/vedantmgoyal9/winget-releaser/issues?q=is%3Aissue+Sandboxie))
- blap - [SandboxToys2](https://github.com/blap/SandboxToys2) 附加组件维护者
- diversenok - 安全分析与 POC / 安全修复
- TechLord - Team-IRA / 逆向工程
- hg421 - 安全分析与 POC / 代码审查
- hx1997 - 安全分析与 POC
- mpheath - Plus 安装程序作者 / 代码修复 / 合作
- offhub - 文档补充 / 代码修复 / Qt5 补丁和构建脚本 / 合作
- LumitoLuma - Qt5 补丁和构建脚本
- QZLin - [sandboxie-docs](https://sandboxie-plus.github.io/sandboxie-docs/) 主题作者
- isaak654 - 模板 / 文档 / 代码修复 / 合作
- typpos - 用户界面补充 / 文档 / 代码修复
- Yeyixiao - 功能补充
- Deezzir - 功能补充
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
- cricri-pingouin - 用户界面修复
- Valinwolf - 用户界面 / 图标
- daveout - 用户界面 / 图标
- kokofixcomputers - [Discord](https://discord.gg/S4tFu6Enne) 频道的支持成员
- NewKidOnTheBlock - 更新日志修复
- Naeemh1 - 文档补充
- APMichael - 模板补充
- 1mm0rt41PC - 文档补充
- Luro223 - 文档补充
- lwcorp - 文档补充
- wilders-soccerfan - 文档补充
- LepordCat - 文档补充
- stdedos - 文档补充

## 🌏 翻译人员

- czoins - 阿拉伯语
- yuhao2348732, 0x391F, nkh0472, yfdyh000, gexgd0419, Zerorigin, UnnamedOrange, DevSplash, Becods, okrc, 4rt3mi5, sepcnt, 风之暇想 - 简体中文
- TragicLifeHu, Hulen, xiongsp - 繁体中文
- RockyTDR - 荷兰语
- clexanis, Mmoi-Fr, hippalectryon-0, Monsieur Pissou - 法语（通过电子邮件提供）
- bastik-1001, APMichael - 德语
- timinoun - 匈牙利语（通过电子邮件提供）
- isaak654, DerivativeOfLog7 - 意大利语
- takahiro-itou - 日语
- VenusGirl - 韩语
- 7zip - 波兰语（[单独提供](https://forum.xanasoft.com/threads/polish-translation.4/page-2)）
- JNylson - 葡萄牙语和巴西葡萄牙语
- lufog, marat2509 - 俄语
- LumitoLuma, sebadamus - 西班牙语
- 1FF, Thatagata - 瑞典语（通过电子邮件或拉取请求提供）
- xorcan, fmbxnary, offhub - 土耳其语
- SuperMaxusa, lufog - 乌克兰语
- GunGunGun - 越南语

所有翻译人员都被鼓励在发送翻译之前查看 [本地化说明和提示](https://git.io/J9G19)。
