# Sandboxie Plus / Classic



[![English Version Readme](https://img.shields.io/badge/English%20Version%20Readme-blue)](./README.md) [![Plus license](https://img.shields.io/badge/Plus%20license-Custom%20-blue.svg)](./LICENSE.Plus) [![Classic license](https://img.shields.io/github/license/Sandboxie-Plus/Sandboxie?label=Classic%20license&color=blue)](./LICENSE.Classic) [![GitHub Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie.svg)](https://github.com/sandboxie-plus/Sandboxie/releases/latest) [![GitHub Pre-Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie/all.svg?label=pre-release)](https://github.com/sandboxie-plus/Sandboxie/releases) [![GitHub Build Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/main.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions) [![GitHub Codespell Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml)

[![Join our Discord Server](https://img.shields.io/badge/Join-Our%20Discord%20Server%20for%20bugs,%20feedback%20and%20more!-blue?style=for-the-badge&logo=discord)](https://discord.gg/S4tFu6Enne)

|  系统要求  |      发布说明      |     贡献指南     |    安全政策    |    行为规范    |
|:----------:|:------------------:|:----------------:|:--------------:|:--------------:|
| Windows 7 或更高版本，32位或64位。 | [CHANGELOG.md](./CHANGELOG.md) | [CONTRIBUTING.md](./CONTRIBUTING.md) | [SECURITY.md](./SECURITY.md) | [CODE_OF_CONDUCT.md](./CODE_OF_CONDUCT.md) |

Sandboxie 是一款基于沙箱的隔离软件，适用于32位和64位的Windows NT系统。它创建了一个类似沙箱的隔离环境，在这个环境中可以运行或安装应用程序，而不会永久性地修改本地和映射驱动器或Windows注册表。隔离的虚拟环境允许对不可信的程序和网络浏览进行受控测试。

Sandboxie 允许您创建几乎无限的沙箱，并单独或同时运行它们，以隔离程序与主机及彼此，同时也允许您在单个沙箱中同时运行尽可能多的程序。

**注意：这是一个社区分支项目，发生在Sandboxie源代码发布之后，不是之前开发的官方延续（参见[项目历史](./README.md#-project-history)和[#2926](https://github.com/sandboxie-plus/Sandboxie/issues/2926)）。**

## ⏬ 下载

[最新版本](https://github.com/sandboxie-plus/Sandboxie/releases/latest)

## 🚀 功能

Sandboxie 提供两种版本，Plus 和 Classic。它们共享相同的核心组件，这意味着它们具有相同的安全性和兼容性。不同之处在于用户界面中功能的可用性。

Sandboxie Plus 拥有现代化的 Qt 界面，支持自项目开源以来添加的所有新功能：

- 快照管理器 - 复制任何沙箱，以便在需要时恢复
- 维护模式 - 在需要时可以卸载/安装/启动/停止 Sandboxie 驱动程序和服务
- 便携模式 - 您可以运行安装程序并选择将所有文件提取到一个目录
- 额外的用户界面选项来阻止访问 Windows 组件，如打印机缓存和剪贴板
- 更强的自定义选项，用于限制启动/运行和互联网访问
- 保护用户数据免受非法访问的隐私模式沙箱
- 限制系统调用和端点访问的安全增强沙箱
- 全局快捷键以挂起或终止所有沙箱进程
- 每个沙箱的网络防火墙，支持 Windows 过滤平台 (WFP)
- 可以使用快捷键 Ctrl+F 搜索沙箱列表
- 搜索全局设置和沙箱选项的功能
- 能够将沙箱导入/导出为 7z 文件
- 将沙箱集成到 Windows 开始菜单
- 浏览器兼容性向导，用于为不受支持的浏览器创建模板
- 复古视图模式以重现 Sandboxie 控制的图形外观
- 故障排除向导，以帮助用户解决问题
- 附加组件管理器，用于通过额外组件扩展或添加功能
- 防止对主机的沙箱保护，包括阻止截图
- 触发系统以在沙箱经历不同阶段时执行操作，如初始化、沙箱启动、终止或文件恢复
- 使进程不被沙箱化，但其子进程被沙箱化
- 作为控制单元的沙箱化，强制程序自动使用 SOCKS5 代理
- 使用沙箱化作为控制粒度的 DNS 解析控制
- 限制沙箱中进程的数量以及它们可以占用的总内存空间，并且您可以限制每个沙箱的沙箱化进程总数
- 完全不同于 Sandboxie 开源前版本的令牌创建机制，使沙箱在系统中更为独立
- 加密沙箱 - 基于 AES 的可靠数据存储解决方案

更多功能可以通过在[CHANGELOG.md](./CHANGELOG.md)文件中按快捷键 Ctrl+F 查找 `=` 标志来发现。

Sandboxie Classic 拥有旧的、不再开发的基于 MFC 的用户界面，因此不支持 Plus 版本的原生界面功能。尽管一些缺失的功能可以在 Sandboxie.ini 配置文件中手动配置，甚至可以通过[自定义脚本](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/viewforum1a2d1a2d.html?f=22)进行替代，但 Classic 版本不建议那些想要探索最新安全选项的用户使用。

## 📚 文档

GitHub 上维护了 [Sandboxie 文档](https://sandboxie-plus.github.io/sandboxie-docs) 的副本，但需要更多志愿者来保持更新以反映新变化。我们建议您还可以查看本仓库中的以下标签：

[未来开发](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A"future+development") | [功能请求](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A"Feature+request") | [文档](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Adocumentation) | [构建问题](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A%22build+issue%22) | [不兼容](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Aincompatibility) | [已知问题](https://github.com/sandboxie-plus/Sandboxie/labels/Known%20issue) | [回归问题](https://github.com/sandboxie-plus/Sandboxie/issues?q=is%3Aissue+is%3Aopen+label%3Aregression) | [解决方法](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Aworkaround) | [求助](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A%22help+wanted%22) | [更多...](https://github.com/sandboxie-plus/Sandboxie/labels?sort=count-desc)

之前由 Invincea 维护的 [旧 Sandboxie 论坛](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums) 的部分存档仍然可用。如果您需要查找特定内容，可以使用以下搜索查询：`site:https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/`

## 🚀 有用的 Sandboxie 工具

Sandboxie 的功能可以通过以下专业工具进行增强：

- [LogApiDll](https://github.com/sandboxie-plus/LogApiDll) - 添加详细的输出到 Sandboxie 的跟踪日志，列出相关 Windows API 函数的调用
- [SbieHide](https://github.com/VeroFess/SbieHide) - 尝试隐藏 SbieDll.dll 在被沙箱化的应用程序中的存在
- [SandboxToys2](https://github.com/blap/SandboxToys2) - 允许监视沙箱中的文件和注册表更改
- [Sbiextra](https://github.com/sandboxie-plus/sbiextra) - 为沙箱化进程添加额外的用户模式限制

## 📌 项目历史

|      时间线      |    维护者    |
|:---------------:|:------------:|
| 2004 - 2013     | Ronen Tzur   |
| 2013 - 2017     | Invincea Inc.|
| 2017 - 2020     | Sophos Group plc |
| [开源代码](https://github.com/sandboxie/sandboxie) |    Tom Brown  |
| ---             | ---          |
| 2020 至今        | David Xanatos ([路线图](https://www.wilderssecurity.com/threads/sandboxie-roadmap.445545/page-8#post-3187633)) |

## 📌 项目支持 / 赞助

[<img align="left" height="64" width="64" src="./.github/images/binja-love.png">](https://binary.ninja/)
感谢 [Vector 35](https://vector35.com/) 提供 [Binary Ninja](https://binary.ninja/) 许可证，帮助逆向工程。
<br>
Binary Ninja 是一款多平台交互式反汇编、反编译和二进制分析工具，适用于逆向工程师、恶意软件分析师、漏洞研究人员和软件开发人员。
<br>
[<img align="left" height="64" width="64" src="./.github/images/Icons8_logo.png">](https://icons8.de/)感谢 [Icons8](https://icons8.de/) 为项目提供图标。
<br>
<br>
<br>

## 🤝 支持项目

如果您发现 Sandboxie 有用，请通过我们的 [贡献指南](./CONTRIBUTING.md) 进行贡献。

## 📑 感谢贡献者

- DavidBerdik - [Sandboxie 网站档案](https://github.com/Sandboxie-Website-Archive/sandboxie-website-archive.github.io) 的维护者
- Jackenmen - Sandboxie 的 Chocolatey 包维护者 ([支持](https://github.com/Jackenmen/choco-auto/issues?q=is%3Aissue+Sandboxie))
- vedantmgoyal9 - Sandboxie 的 Winget Releaser 维护者 ([支持](https://github.com/vedantmgoyal9/winget-releaser/issues?q=is%3Aissue+Sandboxie))
- blap - [SandboxToys2](https://github.com/blap/SandboxToys2) 插件的维护者
- diversenok - 安全分析和 PoCs / 安全修复
- TechLord - Team-IRA / 逆向工程
- hg421 - 安全分析和 PoCs / 代码审查
- hx1997 - 安全分析和 PoC
- mpheath - Plus 安装程序的作者 / 代码修复 / 合作者
- offhub - 文档补充 / 代码修复 / 合作者
- isaak654 - 模板 / 文档 / 代码修复 / 合作者
- typpos - UI 增强 / 文档 / 代码修复
- Yeyixiao - 功能增强
- Deezzir - 功能增强
- okrc - 代码修复
- Sapour - 代码修复
- lmou523 - 代码修复
- sredna - Classic 安装程序的代码修复
- weihongx9315 - 代码修复
- jorgectf - CodeQL 工作流
- stephtr - CI / 认证
- yfdyh000 - Plus 安装程序的本地化支持
- Dyras - 模板补充
- cricri-pingouin - UI 修复
- Valinwolf - UI / 图标
- daveout - UI / 图标
- NewKidOnTheBlock - 更新日志修复
- Naeemh1 - 文档补充
- APMichael - 模板补充
- 1mm0rt41PC - 文档补充
- Luro223 - 文档补充
- lwcorp - 文档补充
- wilders-soccerfan - 文档补充
- LumitoLuma - Qt5 补丁和构建脚本

## 🌏 翻译人员

- yuhao2348732, 0x391F, nkh0472, yfdyh000, gexgd0419, Zerorigin, UnnamedOrange, DevSplash, Becods, okrc, 4rt3mi5, sepcnt - 简体中文
- TragicLifeHu, Hulen, xiongsp - 繁体中文
- RockyTDR - 荷兰语
- clexanis, Mmoi-Fr, hippalectryon-0, Monsieur Pissou - 法语（通过电子邮件提供）
- bastik-1001, APMichael - 德语
- timinoun - 匈牙利语（通过电子邮件提供）
- isaak654, DerivativeOfLog7 - 意大利语
- takahiro-itou - 日语
- VenusGirl - 韩语
- 7zip - 波兰语（[单独提供](https://forum.xanasoft.com/viewtopic.php?f=12&t=4&start=30)）
- JNylson - 葡萄牙语和巴西葡萄牙语
- lufog - 俄语
- LumitoLuma, sebadamus - 西班牙语
- 1FF, Thatagata - 瑞典语（通过电子邮件或拉取请求提供）
- xorcan, fmbxnary, offhub - 土耳其语
- SuperMaxusa, lufog - 乌克兰语
- GunGunGun - 越南语

所有翻译人员都鼓励查看 [本地化说明和提示](https://git.io/J9G19) 后再提交翻译。
