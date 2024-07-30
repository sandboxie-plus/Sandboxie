# Sandboxie Plus / Classic

[![English Version Readme](https://img.shields.io/badge/English%20Version%20Readme-blue)](./README.md) [![Plus license](https://img.shields.io/badge/Plus%20license-Custom%20-blue.svg)](./LICENSE.Plus) [![Classic license](https://img.shields.io/github/license/Sandboxie-Plus/Sandboxie?label=Classic%20license&color=blue)](./LICENSE.Classic) [![GitHub Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie.svg)](https://github.com/sandboxie-plus/Sandboxie/releases/latest) [![GitHub Pre-Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie/all.svg?label=pre-release)](https://github.com/sandboxie-plus/Sandboxie/releases) [![GitHub Build Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/main.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions) [![GitHub Codespell Status](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml)

[![加入我们的 Discord 服务器](https://img.shields.io/badge/加入-我们的%20Discord%20服务器，报告错误、提供反馈等！-blue?style=for-the-badge&logo=discord)](https://discord.gg/S4tFu6Enne)

[![English](https://img.shields.io/badge/English-black)](README.md)
[![وثيقة العربية](https://img.shields.io/badge/%D9%88%D8%AB%D9%8A%D9%82%D8%A9%20%D8%A7%D9%84%D8%B9%D8%B1%D8%A8%D9%8A%D8%A9-green)](README_ar.md) 
[![中文文档](https://img.shields.io/badge/%E4%B8%AD%E6%96%87%E6%96%87%E6%A1%A3-red)](README_zh.md) 
[![Document Français](https://img.shields.io/badge/Document%20Fran%C3%A7ais-white)](README_fr.md) 
[![Документ на русском языке](https://img.shields.io/badge/%D0%94%D0%BE%D0%BA%D1%83%D0%BC%D0%B5%D0%BD%D1%82%20%D0%BD%D0%B0%20%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%BE%D0%BC%20%D1%8F%D0%B7%D1%8B%D0%BA%D0%B5-blue)](README_ru.md) 
[![Documento en Español](https://img.shields.io/badge/Documento%20en%20Espa%C3%B1ol-yellow)](README_es.md)

|  系统要求  |      发布说明     |     贡献指南   |      安全策略      |      行为准则      |
|         :---:         |          :---:         |          :---:                |          :---:            |          :---:            |
| Windows 7 或更高版本, 32 位或 64 位. |  [CHANGELOG.md](./CHANGELOG.md)  |  [CONTRIBUTING.md](./CONTRIBUTING.md)  |   [SECURITY.md](./SECURITY.md)  |  [CODE_OF_CONDUCT.md](./CODE_OF_CONDUCT.md)  |

Sandboxie 是一款基于沙盒的隔离软件,适用于 32 位和 64 位 Windows NT 操作系统。它创建了一个类似沙盒的隔离操作环境,可以在其中运行或安装应用程序,而无需永久修改本地驱动器、映射驱动器或 Windows 注册表。隔离的虚拟环境允许对不受信任的程序和网上冲浪进行受控测试。<br>

Sandboxie 允许您创建几乎无限数量的沙盒,并单独或同时运行它们以将程序与主机和彼此隔离,同时还允许您在单个沙盒中同时运行任意多个程序。

**注意:这是在 Sandboxie 源代码发布后进行的社区分支,而不是之前开发的官方延续(参见[项目历史](./README.md#-project-history)和[#2926](https://github.com/sandboxie-plus/Sandboxie/issues/2926))。**

## ⏬ 下载

[最新发布](https://github.com/sandboxie-plus/Sandboxie/releases/latest)

## 🚀 特性

Sandboxie 有两个版本:Plus 和 Classic。它们共享相同的核心组件,这意味着它们具有相同的安全性和兼容性级别。
不同之处在于用户界面中可用的功能。

Sandboxie Plus 有一个现代的基于 Qt 的 UI,支持自项目开源以来添加的所有新功能:

  * 快照管理器 - 对任何沙盒进行复制,以便在需要时恢复
  * 维护模式 - 允许在需要时卸载/安装/启动/停止 Sandboxie 驱动程序和服务
  * 便携模式 - 您可以运行安装程序并选择将所有文件提取到一个目录
  * 额外的 UI 选项,用于阻止访问 Windows 组件,如打印机后台程序和剪贴板
  * 更多的开始/运行和互联网访问限制的自定义选项
  * 保护用户数据免受非法访问的隐私模式沙盒
  * 限制系统调用和端点可用性的安全增强型沙盒
  * 用于暂停或终止所有沙盒进程的全局热键
  * 每个沙盒的网络防火墙,支持 Windows 筛选平台(WFP)
  * 可以使用快捷键 Ctrl+F 搜索沙盒列表
  * 全局设置和沙盒选项的搜索功能
  * 能够将沙盒导入/导出到 7z 文件
  * 将沙盒集成到 Windows 开始菜单中
  * 浏览器兼容性向导,用于为不受支持的浏览器创建模板
  * 复古视图模式,重现 Sandboxie Control 的图形外观
  * 故障排除向导,帮助用户解决问题
  * 插件管理器,通过附加组件扩展或添加功能
  * 保护沙盒免受主机攻击,包括防止截图
  * 触发系统,当沙盒经历不同阶段(如初始化、沙盒启动、终止或文件恢复)时执行操作
  * 使进程不受沙盒限制,但其子进程受沙盒限制
  * 将沙盒作为控制单元,强制程序自动使用 SOCKS5 代理
  * 以沙盒为控制粒度的 DNS 解析控制
  * 限制沙盒中的进程数量和它们可以占用的总内存空间,您可以限制每个沙盒的沙盒进程总数
  * 与 Sandboxie 开源前版本完全不同的令牌创建机制,使沙盒在系统中更加独立
  * 加密沙盒 - 基于 AES 的可靠数据存储解决方案

更多功能可以通过在 [CHANGELOG.md](./CHANGELOG.md) 文件中使用快捷键 Ctrl+F 查找 `=` 符号来发现。

Sandboxie Classic 具有旧的不再开发的基于 MFC 的 UI,因此缺乏对 Plus 功能的原生界面支持。尽管一些缺失的功能可以在 Sandboxie.ini 配置文件中手动配置,甚至可以用[自定义脚本](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/viewforum1a2d1a2d.html?f=22)替换,但不建议想要探索最新安全选项的用户使用 Classic 版本。

## 📚 文档

目前正在维护 [Sandboxie 文档](https://sandboxie-plus.github.io/sandboxie-docs)的 GitHub 副本,尽管需要更多志愿者来保持其与新变更的更新。我们建议也查看此存储库中的以下标签:

[未来开发](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A"future+development") | [功能请求](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A"Feature+request") | [文档](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Adocumentation) | [构建问题](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A%22build+issue%22) | [不兼容性](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Aincompatibility) | [已知问题](https://github.com/sandboxie-plus/Sandboxie/labels/Known%20issue) | [回归](https://github.com/sandboxie-plus/Sandboxie/issues?q=is%3Aissue+is%3Aopen+label%3Aregression) | [解决方法](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Aworkaround) | [需要帮助](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A%22help+wanted%22) | [更多...](https://github.com/sandboxie-plus/Sandboxie/labels?sort=count-desc)

先前由 Invincea 维护的[旧 Sandboxie 论坛](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums)的部分存档仍然可用。如果您需要查找特定内容,可以使用以下搜索查询:`site:https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/`

## 🚀 Sandboxie 的实用工具

Sandboxie 的功能可以通过以下专门工具来增强:

  * [LogApiDll](https://github.com/sandboxie-plus/LogApiDll) - 为 Sandboxie 的跟踪日志添加详细输出,列出相关 Windows API 函数的调用
  * [SbieHide](https://github.com/VeroFess/SbieHide) - 尝试隐藏 SbieDll.dll 的存在,使被沙盒化的应用程序无法察觉
  * [SandboxToys2](https://github.com/blap/SandboxToys2) - 允许监控沙盒中的文件和注册表更改
  * [Sbiextra](https://github.com/sandboxie-plus/sbiextra) - 为沙盒进程添加额外的用户模式限制

## 📌 项目历史

|      时间线       |    维护者    |
|        :---         |       :---       |
| 2004 - 2013         | Ronen Tzur       |
| 2013 - 2017         | Invincea Inc.    |
| 2017 - 2020         | Sophos Group plc |
| [开源代码](https://github.com/sandboxie/sandboxie) |    Tom Brown     |
| --- | --- |
| 2020 至今        | David Xanatos ([路线图](https://www.wilderssecurity.com/threads/sandboxie-roadmap.445545/page-8#post-3187633))    |

## 📌 项目支持/赞助

[<img align="left" height="64" width="64" src="./.github/images/binja-love.png">](https://binary.ninja/)
感谢 [Vector 35](https://vector35.com/) 提供 [Binary Ninja](https://binary.ninja/) 许可证以帮助逆向工程。
<br>
Binary Ninja 是一个多平台交互式反汇编器、反编译器和二进制分析工具,适用于逆向工程师、恶意软件分析师、漏洞研究人员和软件开发人员。<br>
<br>
[<img align="left" height="64" width="64" src="./.github/images/Icons8_logo.png">](https://icons8.de/)感谢 [Icons8](https://icons8.de/) 为项目提供图标。
<br>
<br>
<br>

## 🤝 支持项目

如果您发现 Sandboxie 有用,请随时通过我们的[贡献指南](./CONTRIBUTING.md)做出贡献。


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
- SNESNya - 文档补充

## 🌏 翻译人员

- yuhao2348732, 0x391F, nkh0472, yfdyh000, gexgd0419, Zerorigin, UnnamedOrange, DevSplash, Becods, okrc, 4rt3mi5, sepcnt - 简体中文
- TragicLifeHu, Hulen, xiongsp - 繁体中文
- RockyTDR - 荷兰语
- clexanis, Mmoi-Fr, hippalectryon-0, Monsieur Pissou - 法语（通过电邮提供）
- bastik-1001, APMichael - 德语
- timinoun - 匈牙利语（通过电邮提供）
- isaak654, DerivativeOfLog7 - 意大利语
- takahiro-itou - 日语
- VenusGirl - 韩语
- 7zip - 波兰语（[单独提供](https://forum.xanasoft.com/viewtopic.php?f=12&t=4&start=30)）
- JNylson - 葡萄牙语和巴西葡萄牙语
- lufog - 俄语
- LumitoLuma, sebadamus - 西班牙语
- 1FF, Thatagata - 瑞典语（通过电邮或PR提供）
- xorcan, fmbxnary, offhub - 土耳其语
- SuperMaxusa, lufog - 乌克兰语
- GunGunGun - 越南语

翻译人员在翻以前，必须阅读 [本地化说明和提示](https://git.io/J9G19) 后再提交翻译。
