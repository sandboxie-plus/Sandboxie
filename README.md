# Sandboxie
[![GitHub Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie.svg)](https://github.com/sandboxie-plus/Sandboxie/releases/latest) [![GitHub Pre-Release](https://img.shields.io/github/release/sandboxie-plus/Sandboxie/all.svg?label=pre-release)](https://github.com/sandboxie-plus/Sandboxie/releases)

<img align="left" src="https://user-images.githubusercontent.com/12372772/123517080-8ab20a00-d69f-11eb-8e82-2e18cf6e0303.png">
Sandboxie is a sandbox-based isolation software for 32-bit and 64-bit Windows NT-based operating systems. It creates a sandbox-like isolated operating environment in which applications can be run or installed without permanently modifying local & mapped drives or the windows registry. An isolated virtual environment allows controlled testing of untrusted programs and web surfing.<br><br>

|  System requirements  |      Release notes     |
|         :---:         |          :---:         |
| Windows 7 or higher, 32-bit or 64-bit. |  [CHANGELOG.md](https://github.com/sandboxie-plus/Sandboxie/blob/master/CHANGELOG.md)

## Download
[![GitHub All Releases](https://img.shields.io/github/downloads/sandboxie-plus/Sandboxie/total?style=for-the-badge)](https://github.com/sandboxie-plus/Sandboxie/releases/latest)

## Features
Sandboxie is available in two flavors, Plus and Classic. They both share the same core components, this means they have the same level of security and compatibility.
What's different is the availability of features in the user interface.

Plus build has a modern Qt based UI, which supports all new features that have been added since the project went open source:

  * Snapshot Manager - this feature takes a copy of any box in order to be restored when needed
  * Maintenance mode - it allows to uninstall/install/start/stop Sandboxie driver and service when needed
  * Portable mode - you can run the installer and choose to extract all files to a directory
  * Additional UI options to block access for Windows components like printer spooler and clipboard
  * More customization options for Start/Run and Internet access restrictions
  * Global hotkey to terminate all boxed processes
  * A new firewall per sandbox which supports Windows Filtering Platform (WFP)

Classic build has the old no longer developed MFC based UI, hence it lacks native interface support for Plus features. However, most of them can be configured manually in the Sandboxie.ini configuration file, and some valuable [contributions](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/viewforum1a2d1a2d.html?f=22) can still be applied nowadays.

If you need to check all new features at once, a good way to highlight them quickly is to find the equal sign in the [CHANGELOG.md](https://github.com/sandboxie-plus/Sandboxie/blob/master/CHANGELOG.md) file.

## Documentation
A GitHub copy of the [Sandboxie documentation](https://sandboxie-plus.github.io/sandboxie-docs) is currently maintained, although more volunteers are needed to keep it updated with the new changes. We recommend to check also the following labels in this repository:

[future development](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A"future+development") | [documentation](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Adocumentation) | [build issues](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A%22build+issue%22) | [known issues](https://github.com/sandboxie-plus/Sandboxie/labels/Known%20issue) | [regressions](https://github.com/sandboxie-plus/Sandboxie/issues?q=is%3Aissue+is%3Aopen+label%3Aregression) | [workaround](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Aworkaround) | [help wanted](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A%22help+wanted%22) | [more...](https://github.com/sandboxie-plus/Sandboxie/labels?sort=count-desc)

A partial archive of the [old Sandboxie forum](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums) that was previously maintained by Invincea is still available. If you need to find something there, please use the corresponding [repository search](https://github.com/Sandboxie-Website-Archive/sandboxie-website-archive.github.io).

## Project maintenance
2004 - 2013 Ronen Tzur

2013 - 2017 Invincea Inc.

2017 - 2020 Sophos Group plc

Open Source release

2020 onwards David Xanatos

## Helpful Contributors
- DavidBerdik - Maintainer of [Sandboxie Website Archive](https://github.com/Sandboxie-Website-Archive/sandboxie-website-archive.github.io)
- diversenok - Security analysis & PoC's
- stephtr - CI / Certification
- TechLord - Team-IRA / Reversing
- hg421 - Security analysis 
- typpos - UI suggestions / Documentation / Code reviews
- isaak654 - UI fixes / Templates / Documentation
- Dyras - Templates additions
- cricri-pingouin - UI fixes
- Valinwolf - UI / Icons
- daveout - UI / Icons
- mpheath - Author of Plus installer based on Inno Setup
- yfdyh000 - Localization support for Plus installer
- NewKidOnTheBlock - Changelog fixes

## Translators
- yuhao2348732, nkh0472, yfdyh000, gexgd0419 - Chinese Simplified
- TragicLifeHu, Hulen - Chinese Traditional
- RockyTDR - Dutch
- clexanis - French
- bastik-1001 - German
- isaak654 - Italian
- 7zip from [Xanasoft topic](https://forum.xanasoft.com/viewtopic.php?f=12&t=4&start=10) - Polish
- JNylson - Portuguese
- lufog - Russian
- sebadamus - Spanish
- xorcan - Turkish (disabled for inactivity, a new translator is needed)
- SuperMaxusa - Ukrainian

More volunteers are welcome here! ;)

## Support the project
If you like the tool, please consider supporting it on Patreon: https://www.patreon.com/DavidXanatos

Or you can donate directly via the PayPal button on my website: https://xanasoft.com
