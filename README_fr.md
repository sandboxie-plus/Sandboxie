# Sandboxie Plus / Classic

[![Licence Plus](https://img.shields.io/badge/Plus%20license-Custom%20-blue.svg)](./LICENSE.Plus) [![Licence Classique](https://img.shields.io/github/license/Sandboxie-Plus/Sandboxie?label=Classic%20license&color=blue)](./LICENSE.Classic) [![Version GitHub](https://img.shields.io/github/release/sandboxie-plus/Sandboxie.svg)](https://github.com/sandboxie-plus/Sandboxie/releases/latest) [![Pré-version GitHub](https://img.shields.io/github/release/sandboxie-plus/Sandboxie/all.svg?label=pre-release)](https://github.com/sandboxie-plus/Sandboxie/releases) [![Statut de construction GitHub](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/main.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions) [![Statut Codespell GitHub](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml)

[![Rejoignez notre serveur Discord](https://img.shields.io/badge/Join-Our%20Discord%20Server%20for%20bugs,%20feedback%20and%20more!-blue?style=for-the-badge&logo=discord)](https://discord.gg/S4tFu6Enne)

[![English](https://img.shields.io/badge/English-black)](README.md)
[![وثيقة العربية](https://img.shields.io/badge/%D9%88%D8%AB%D9%8A%D9%82%D8%A9%20%D8%A7%D9%84%D8%B9%D8%B1%D8%A8%D9%8A%D8%A9-green)](README_ar.md) 
[![中文文档](https://img.shields.io/badge/%E4%B8%AD%E6%96%87%E6%96%87%E6%A1%A3-red)](README_zh.md) 
[![Document Français](https://img.shields.io/badge/Document%20Fran%C3%A7ais-white)](README_fr.md) 
[![Документ на русском языке](https://img.shields.io/badge/%D0%94%D0%BE%D0%BA%D1%83%D0%BC%D0%B5%D0%BD%D1%82%20%D0%BD%D0%B0%20%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%BE%D0%BC%20%D1%8F%D0%B7%D1%8B%D0%BA%D0%B5-blue)](README_ru.md) 
[![Documento en Español](https://img.shields.io/badge/Documento%20en%20Espa%C3%B1ol-yellow)](README_es.md)
                                    
|  Configuration requise  |      Notes de version     |     Guide de contribution   |      Politique de sécurité      |      Code de conduite      |
|         :---:         |          :---:         |          :---:                |          :---:            |          :---:            |
| Windows 7 ou supérieur, 32 bits ou 64 bits. |  [CHANGELOG.md](./CHANGELOG.md)  |  [CONTRIBUTING.md](./CONTRIBUTING.md)  |   [SECURITY.md](./SECURITY.md)  |  [CODE_OF_CONDUCT.md](./CODE_OF_CONDUCT.md)  |

Sandboxie est un logiciel d'isolation basé sur le sandbox pour les systèmes d'exploitation Windows NT 32 bits et 64 bits. Il crée un environnement de fonctionnement isolé de type sandbox dans lequel les applications peuvent être exécutées ou installées sans modifier de manière permanente les disques locaux et les disques mappés ou le registre Windows. Un environnement virtuel isolé permet des tests contrôlés des programmes non fiables et de la navigation sur le web.<br>

Sandboxie vous permet de créer un nombre pratiquement illimité de sandbox et de les exécuter seules ou simultanément pour isoler les programmes de l'hôte et entre eux, tout en vous permettant également d'exécuter autant de programmes que vous le souhaitez dans une seule sandbox.

**Remarque : Il s'agit d'un fork communautaire qui a eu lieu après la publication du code source de Sandboxie et non de la continuation officielle du développement précédent (voir [historique du projet](./README.md#-project-history) et [#2926](https://github.com/sandboxie-plus/Sandboxie/issues/2926)).**

## ⏬ Téléchargement

[Dernière version](https://github.com/sandboxie-plus/Sandboxie/releases/latest)

## 🚀 Fonctionnalités

Sandboxie est disponible en deux éditions, Plus et Classique. Ils partagent tous deux les mêmes composants de base, ce qui signifie qu'ils ont le même niveau de sécurité et de compatibilité.
La différence réside dans la disponibilité des fonctionnalités dans l'interface utilisateur.

Sandboxie Plus dispose d'une interface utilisateur moderne basée sur Qt, qui prend en charge toutes les nouvelles fonctionnalités ajoutées depuis que le projet est devenu open source :

  * Gestionnaire de snapshots - prend une copie de toute sandbox pour être restaurée si nécessaire
  * Mode maintenance - permet de désinstaller/installer/démarrer/arrêter le pilote et le service Sandboxie si nécessaire
  * Mode portable - vous pouvez exécuter l'installateur et choisir d'extraire tous les fichiers dans un répertoire
  * Options supplémentaires de l'interface utilisateur pour bloquer l'accès aux composants Windows tels que le spooler d'imprimante et le presse-papiers
  * Plus d'options de personnalisation pour les restrictions de démarrage/exécution et d'accès à Internet
  * Sandbox en mode confidentialité qui protège les données utilisateur contre l'accès illégitime
  * Sandbox améliorées en matière de sécurité qui restreignent la disponibilité des appels système et des points de terminaison
  * Raccourcis clavier globaux pour suspendre ou terminer tous les processus sandboxés
  * Un pare-feu réseau par sandbox qui prend en charge la plateforme de filtrage Windows (WFP)
  * La liste des sandboxes peut être recherchée avec la touche de raccourci Ctrl+F
  * Une fonction de recherche pour les paramètres globaux et les options de sandbox
  * Possibilité d'importer/exporter des sandboxes vers et depuis des fichiers 7z
  * Intégration des sandboxes dans le menu Démarrer de Windows
  * Un assistant de compatibilité du navigateur pour créer des modèles pour les navigateurs non pris en charge
  * Mode vue vintage pour reproduire l'apparence graphique de Sandboxie Control
  * Un assistant de dépannage pour aider les utilisateurs à résoudre leurs problèmes
  * Un gestionnaire d'extensions pour étendre ou ajouter des fonctionnalités via des composants supplémentaires
  * Protections des sandboxes contre l'hôte, y compris la prévention de la prise de captures d'écran
  * Un système de déclenchement pour effectuer des actions lorsque la sandbox passe par différentes étapes, telles que l'initialisation, le démarrage de la sandbox, la terminaison ou la récupération des fichiers
  * Faire en sorte qu'un processus ne soit pas sandboxé, mais que ses processus enfants le soient
  * Utilisation de la sandbox comme unité de contrôle pour forcer les programmes à utiliser automatiquement le proxy SOCKS5
  * Contrôle de la résolution DNS avec la sandbox comme granularité de contrôle
  * Limiter le nombre de processus dans la sandbox et la quantité totale d'espace mémoire qu'ils peuvent occuper, et Vous pouvez limiter le nombre total de processus sandboxés par boîte
  * Un mécanisme de création de jetons complètement différent de la version pré-open-source de Sandboxie rend les sandboxes plus indépendantes dans le système
  * Sandbox cryptée - une solution de stockage de données fiable basée sur AES.

D'autres fonctionnalités peuvent être repérées en trouvant le signe = via la touche de raccourci Ctrl+F dans le fichier [CHANGELOG.md](./CHANGELOG.md).

Sandboxie Classique a l'ancienne interface utilisateur basée sur MFC qui n'est plus développée, il lui manque donc le support natif de l'interface pour les fonctionnalités Plus. Bien que certaines des fonctionnalités manquantes puissent être configurées manuellement dans le fichier de configuration Sandboxie.ini ou même remplacées par [scripts personnalisés](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/viewforum1a2d1a2d.html?f=22), l'édition Classique n'est pas recommandée pour les utilisateurs souhaitant explorer les dernières options de sécurité.

## 📚 Documentation

Une copie GitHub de la [documentation de Sandboxie](https://sandboxie-plus.github.io/sandboxie-docs) est actuellement maintenue, bien que plus de volontaires soient nécessaires pour la mettre à jour avec les nouveaux changements. Nous recommandons également de consulter les étiquettes suivantes dans ce référentiel :

[développement futur](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A"future+development") | [demandes de fonctionnalités](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A"Feature+request") | [documentation](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Adocumentation) | [problèmes de construction](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A%22build+issue%22) | [incompatibilités](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Aincompatibility) | [problèmes connus](https://github.com/sandboxie-plus/Sandboxie/labels/Known%20issue) | [régressions](https://github.com/sandboxie-plus/Sandboxie/issues?q=is%3Aissue+is%3Aopen+label%3Aregression) | [solutions de contournement](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Aworkaround) | [aide demandée](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A%22help+wanted%22) | [plus...](https://github.com/sandboxie-plus/Sandboxie/labels?sort=count-desc)

Un archive partielle du [forum ancien de Sandboxie](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums) précédemment maintenu par Invincea est encore disponible. Si vous avez besoin de trouver quelque chose de spécifique, il est possible d'utiliser la requête de recherche suivante : site:https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/


## 🚀 Outils utiles pour Sandboxie

La fonctionnalité de Sandboxie peut être améliorée avec des outils spécialisés comme les suivants :

  * [LogApiDll](https://github.com/sandboxie-plus/LogApiDll) - ajoute une sortie détaillée au journal de trace de Sandboxie, listant les invocations des fonctions API Windows pertinentes
  * [SbieHide](https://github.com/VeroFess/SbieHide) - tente de cacher la présence de SbieDll.dll de l'application en cours de sandbox
  * [SandboxToys2](https://github.com/blap/SandboxToys2) - permet de surveiller les modifications de fichiers et de registre dans une sandbox
  * [Sbiextra](https://github.com/sandboxie-plus/sbiextra) - ajoute des restrictions supplémentaires en mode utilisateur aux processus sandboxés


## 📌 Historique du projet

|      Chronologie       |    Mainteneur    |
|        :---         |       :---       |
| 2004 - 2013         | Ronen Tzur       |
| 2013 - 2017         | Invincea Inc.    |
| 2017 - 2020         | Sophos Group plc |
| [Code open-source](https://github.com/sandboxie/sandboxie) |    Tom Brown     |
| --- | --- |
| 2020 et au-delà        | David Xanatos ([Feuille de route](https://www.wilderssecurity.com/threads/sandboxie-roadmap.445545/page-8#post-3187633))    |


## 📌 Soutien au projet / parrainage

[<img align="left" height="64" width="64" src="./.github/images/binja-love.png">](https://binary.ninja/)
Merci à [Vector 35](https://vector35.com/) pour avoir fourni une licence [Binary Ninja](https://binary.ninja/) pour aider à l'ingénierie inverse.
<br>
Binary Ninja est un désassembleur interactif multi-plateformes, un décompilateur et un outil d'analyse binaire pour les ingénieurs en rétro-ingénierie, les analystes de logiciels malveillants, les chercheurs en vulnérabilité et les développeurs de logiciels.
<br>
<br>
[<img align="left" height="64" width="64" src="./.github/images/Icons8_logo.png">](https://icons8.de/)Merci à [Icons8](https://icons8.de/) pour avoir fourni des icônes pour le projet.
<br>
<br>
<br>

## 🤝 Soutenir le projet

Si vous trouvez Sandboxie utile, n'hésitez pas à contribuer via notre [Guide de contribution](./CONTRIBUTING.md).

## 📑 Contributeurs utiles

- DavidBerdik - Mainteneur de [Sandboxie Website Archive](https://github.com/Sandboxie-Website-Archive/sandboxie-website-archive.github.io)
- Jackenmen - Mainteneur des paquets Chocolatey pour Sandboxie ([support](https://github.com/Jackenmen/choco-auto/issues?q=is%3Aissue+Sandboxie))
- vedantmgoyal9 - Mainteneur de Winget Releaser pour Sandboxie ([support](https://github.com/vedantmgoyal9/winget-releaser/issues?q=is%3Aissue+Sandboxie))
- blap - Mainteneur de l'addon [SandboxToys2](https://github.com/blap/SandboxToys2)
- diversenok - Analyse de sécurité et PoCs / Corrections de sécurité
- TechLord - Équipe IRA / Ingénierie inverse
- hg421 - Analyse de sécurité et PoCs / Revues de code
- hx1997 - Analyse de sécurité et PoC
- mpheath - Auteur de l'installateur Plus / Corrections de code / Collaborateur
- offhub - Ajouts de documentation / Corrections de code / Collaborateur
- isaak654 - Modèles / Documentation / Corrections de code / Collaborateur
- typpos - Ajouts d'interface utilisateur / Documentation / Corrections de code
- Yeyixiao - Ajouts de fonctionnalités
- Deezzir - Ajouts de fonctionnalités
- okrc - Corrections de code
- Sapour - Corrections de code
- lmou523 - Corrections de code
- sredna - Corrections de code pour l'installateur Classic
- weihongx9315 - Correction de code
- jorgectf - Workflow CodeQL
- stephtr - CI / Certification
- yfdyh000 - Support de localisation pour l'installateur Plus
- Dyras - Ajouts de modèles
- cricri-pingouin - Corrections d'interface utilisateur
- Valinwolf - Interface utilisateur / Icônes
- daveout - Interface utilisateur / Icônes
- NewKidOnTheBlock - Corrections de changelog
- Naeemh1 - Ajouts de documentation
- APMichael - Ajouts de modèles
- 1mm0rt41PC - Ajouts de documentation
- Luro223 - Ajouts de documentation
- lwcorp - Ajouts de documentation
- wilders-soccerfan - Ajouts de documentation
- LumitoLuma - Correctif Qt5 et script de construction
- SNESNya - Ajouts de documentation

## 🌏 Traducteurs

- yuhao2348732, 0x391F, nkh0472, yfdyh000, gexgd0419, Zerorigin, UnnamedOrange, DevSplash, Becods, okrc, 4rt3mi5, sepcnt - Chinois simplifié
- TragicLifeHu, Hulen, xiongsp - Chinois traditionnel
- RockyTDR - Néerlandais
- clexanis, Mmoi-Fr, hippalectryon-0, Monsieur Pissou - Français (fourni par e-mail)
- bastik-1001, APMichael - Allemand
- timinoun - Hongrois (fourni par e-mail)
- isaak654, DerivativeOfLog7 - Italien
- takahiro-itou - Japonais
- VenusGirl - Coréen
- 7zip - Polonais ([fourni séparément](https://forum.xanasoft.com/viewtopic.php?f=12&t=4&start=30))
- JNylson - Portugais et portugais brésilien
- lufog - Russe
- LumitoLuma, sebadamus - Espagnol
- 1FF, Thatagata - Suédois (fourni par e-mail ou pull request)
- xorcan, fmbxnary, offhub - Turc
- SuperMaxusa, lufog - Ukrainien
- GunGunGun - Vietnamien

Tous les traducteurs sont encouragés à consulter les [notes et conseils de localisation](https://git.io/J9G19) avant d'envoyer une traduction.
