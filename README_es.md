# Sandboxie Plus / Classic

[![Licencia Plus](https://img.shields.io/badge/Plus%20license-Custom%20-blue.svg)](./LICENSE.Plus) [![Licencia Classic](https://img.shields.io/github/license/Sandboxie-Plus/Sandboxie?label=Classic%20license&color=blue)](./LICENSE.Classic) [![Versión de GitHub](https://img.shields.io/github/release/sandboxie-plus/Sandboxie.svg)](https://github.com/sandboxie-plus/Sandboxie/releases/latest) [![Pre-lanzamiento de GitHub](https://img.shields.io/github/release/sandboxie-plus/Sandboxie/all.svg?label=pre-release)](https://github.com/sandboxie-plus/Sandboxie/releases) [![Estado de compilación de GitHub](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/main.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions) [![Estado de Codespell de GitHub](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml/badge.svg)](https://github.com/sandboxie-plus/Sandboxie/actions/workflows/codespell.yml)

[![Únase a nuestro servidor de Discord](https://img.shields.io/badge/Join-Our%20Discord%20Server%20for%20bugs,%20feedback%20and%20more!-blue?style=for-the-badge&logo=discord)](https://discord.gg/S4tFu6Enne)

[![English](https://img.shields.io/badge/English-black)](README.md)
[![وثيقة العربية](https://img.shields.io/badge/%D9%88%D8%AB%D9%8A%D9%82%D8%A9%20%D8%A7%D9%84%D8%B9%D8%B1%D8%A8%D9%8A%D8%A9-green)](README_ar.md) 
[![中文文档](https://img.shields.io/badge/%E4%B8%AD%E6%96%87%E6%96%87%E6%A1%A3-red)](README_zh.md) 
[![Document Français](https://img.shields.io/badge/Document%20Fran%C3%A7ais-white)](README_fr.md) 
[![Документ на русском языке](https://img.shields.io/badge/%D0%94%D0%BE%D0%BA%D1%83%D0%BC%D0%B5%D0%BD%D1%82%20%D0%BD%D0%B0%20%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%BE%D0%BC%20%D1%8F%D0%B7%D1%8B%D0%BA%D0%B5-blue)](README_ru.md) 
[![Documento en Español](https://img.shields.io/badge/Documento%20en%20Espa%C3%B1ol-yellow)](README_es.md)
                                        
|  Requisitos del sistema  |      Notas de la versión     |     Guía de contribución   |      Política de seguridad      |      Código de conducta      |
|         :---:         |          :---:         |          :---:                |          :---:            |          :---:            |
| Windows 7 o superior, 32 bits o 64 bits. |  [CHANGELOG.md](./CHANGELOG.md)  |  [CONTRIBUTING.md](./CONTRIBUTING.md)  |   [SECURITY.md](./SECURITY.md)  |  [CODE_OF_CONDUCT.md](./CODE_OF_CONDUCT.md)  |

Sandboxie es un software de aislamiento basado en sandbox para sistemas operativos basados en Windows NT de 32 bits y 64 bits. Crea un entorno operativo aislado similar a un sandbox en el que las aplicaciones se pueden ejecutar o instalar sin modificar permanentemente los discos locales y asignados o el registro de Windows. Un entorno virtual aislado permite pruebas controladas de programas no confiables y navegación web.<br>

Sandboxie te permite crear sandboxes virtualmente ilimitados y ejecutarlos solos o simultáneamente para aislar programas del host y entre sí, mientras también te permite ejecutar tantos programas simultáneamente en una sola caja como desees.

**Nota: Este es un fork comunitario que tuvo lugar después de la publicación del código fuente de Sandboxie y no es la continuación oficial del desarrollo anterior (ver [historia del proyecto](./README.md#-project-history) y [#2926](https://github.com/sandboxie-plus/Sandboxie/issues/2926)).**

## ⏬ Descargar

[Última versión](https://github.com/sandboxie-plus/Sandboxie/releases/latest)

## 🚀 Características

Sandboxie está disponible en dos ediciones, Plus y Classic. Ambas comparten los mismos componentes principales, lo que significa que tienen el mismo nivel de seguridad y compatibilidad.
Lo que es diferente es la disponibilidad de características en la interfaz de usuario.

Sandboxie Plus tiene una interfaz de usuario moderna basada en Qt, que admite todas las nuevas características que se han agregado desde que el proyecto se hizo de código abierto:

  * Administrador de instantáneas: toma una copia de cualquier caja para ser restaurada cuando sea necesario
  * Modo de mantenimiento: permite desinstalar/instalar/iniciar/detener el controlador y el servicio de Sandboxie cuando sea necesario
  * Modo portátil: puedes ejecutar el instalador y elegir extraer todos los archivos a un directorio
  * Opciones adicionales de la interfaz de usuario para bloquear el acceso a componentes de Windows como el spooler de impresión y el portapapeles
  * Más opciones de personalización para las restricciones de inicio/ejecución y acceso a Internet
  * Sandboxes en modo de privacidad que protegen los datos del usuario de accesos ilegítimos
  * Sandboxes mejoradas en seguridad que restringen la disponibilidad de llamadas de sistema y puntos finales
  * Teclas de acceso rápido globales para suspender o terminar todos los procesos en el sandbox
  * Un firewall de red por sandbox que admite la Plataforma de Filtrado de Windows (WFP)
  * La lista de sandboxes se puede buscar con la tecla de acceso rápido Ctrl+F
  * Una función de búsqueda para Configuraciones Globales y Opciones de Sandbox
  * Capacidad para importar/exportar sandboxes a y desde archivos 7z
  * Integración de sandboxes en el menú de inicio de Windows
  * Un asistente de compatibilidad de navegador para crear plantillas para navegadores no compatibles
  * Modo de vista vintage para reproducir la apariencia gráfica de Sandboxie Control
  * Un asistente de solución de problemas para ayudar a los usuarios con sus problemas
  * Un gestor de complementos para extender o añadir funcionalidad a través de componentes adicionales
  * Protecciones de sandboxes contra el host, incluyendo la prevención de tomar capturas de pantalla
  * Un sistema de desencadenantes para realizar acciones, cuando un sandbox pasa por diferentes etapas, como la inicialización, el inicio de la caja, la terminación o la recuperación de archivos
  * Hacer que un proceso no esté sandboxeado, pero que sus procesos secundarios estén sandboxeados
  * Uso del sandbox como una unidad de control para forzar a los programas a usar automáticamente el proxy SOCKS5
  * Control de resolución de DNS con el sandbox como granularidad de control
  * Limitar el número de procesos en el sandbox y la cantidad total de espacio de memoria que pueden ocupar, y puedes limitar el número total de procesos sandboxeados por caja
  * Un mecanismo de creación de tokens completamente diferente a la versión pre-open-source de Sandboxie hace que los sandboxes sean más independientes en el sistema
  * Sandbox encriptado: una solución de almacenamiento de datos confiable basada en AES.

Se pueden encontrar más características buscando el signo = mediante la tecla de acceso rápido Ctrl+F en el archivo [CHANGELOG.md](./CHANGELOG.md).

Sandboxie Classic tiene la antigua interfaz de usuario basada en MFC que ya no se desarrolla, por lo que carece de soporte de interfaz nativa para las características de Plus. Aunque algunas de las características faltantes se pueden configurar manualmente en el archivo de configuración Sandboxie.ini o incluso reemplazarlas con [scripts personalizados](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/viewforum1a2d1a2d.html?f=22), la edición Classic no se recomienda para usuarios que desean explorar las últimas opciones de seguridad.

## 📚 Documentación

Actualmente se mantiene una copia de GitHub de la [documentación de Sandboxie](https://sandboxie-plus.github.io/sandboxie-docs), aunque se necesitan más voluntarios para mantenerla actualizada con los nuevos cambios. También recomendamos consultar las siguientes etiquetas en este repositorio:

[desarrollo futuro](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A"future+development") | [solicitudes de características](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A"Feature+request") | [documentación](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Adocumentation) | [problemas de compilación](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A%22build+issue%22) | [incompatibilidades](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Aincompatibility) | [problemas conocidos](https://github.com/sandboxie-plus/Sandboxie/labels/Known%20issue) | [regresiones](https://github.com/sandboxie-plus/Sandboxie/issues?q=is%3Aissue+is%3Aopen+label%3Aregression) | [soluciones alternativas](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3Aworkaround) | [ayuda requerida](https://github.com/sandboxie-plus/Sandboxie/issues?q=label%3A%22help+wanted%22) | [más...](https://github.com/sandboxie-plus/Sandboxie/labels?sort=count-desc)

Todavía está disponible un archivo parcial del [antiguo foro de Sandboxie](https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums) que anteriormente era mantenido por Invincea. Si necesitas encontrar algo específico, es posible utilizar la siguiente consulta de búsqueda: site:https://sandboxie-website-archive.github.io/www.sandboxie.com/old-forums/


## 🚀 Herramientas útiles para Sandboxie

La funcionalidad de Sandboxie se puede mejorar con herramientas especializadas como las siguientes:

  * [LogApiDll](https://github.com/sandboxie-plus/LogApiDll) - agrega una salida detallada al registro de seguimiento de Sandboxie, listando invocaciones de funciones relevantes de la API de Windows
  * [SbieHide](https://github.com/VeroFess/SbieHide) - intenta ocultar la presencia de SbieDll.dll de la aplicación que se está ejecutando en el sandbox
  * [SandboxToys2](https://github.com/blap/SandboxToys2) - permite monitorear cambios en archivos y registros en un sandbox
  * [Sbiextra](https://github.com/sandboxie-plus/sbiextra) - agrega restricciones adicionales en modo usuario a los procesos en el sandbox


## 📌 Historia del proyecto

|      Cronología       |    Mantenedor    |
|        :---         |       :---       |
| 2004 - 2013         | Ronen Tzur       |
| 2013 - 2017         | Invincea Inc.    |
| 2017 - 2020         | Sophos Group plc |
| [Código abierto](https://github.com/sandboxie/sandboxie) |    Tom Brown     |
| --- | --- |
| Desde 2020        | David Xanatos ([Hoja de ruta](https://www.wilderssecurity.com/threads/sandboxie-roadmap.445545/page-8#post-3187633))    |


## 📌 Apoyo al proyecto / patrocinio

[<img align="left" height="64" width="64" src="./.github/images/binja-love.png">](https://binary.ninja/)
Gracias a [Vector 35](https://vector35.com/) por proporcionar una licencia [Binary Ninja](https://binary.ninja/) para ayudar con la ingeniería inversa.
<br>
Binary Ninja es un desensamblador interactivo multiplataforma, descompilador y herramienta de análisis binario para ingenieros inversos, analistas de malware, investigadores de vulnerabilidades y desarrolladores de software.
<br>
<br>
[<img align="left" height="64" width="64" src="./.github/images/Icons8_logo.png">](https://icons8.de/)Gracias a [Icons8](https://icons8.de/) por proporcionar iconos para el proyecto.
<br>
<br>
<br>

## 🤝 Apoya el proyecto

Si encuentras Sandboxie útil, siéntete libre de contribuir a través de nuestra [Guía de contribución](./CONTRIBUTING.md).

## 📑 Contribuyentes útiles

- DavidBerdik - Mantenedor del [Archivo del sitio web de Sandboxie](https://github.com/Sandboxie-Website-Archive/sandboxie-website-archive.github.io)
- Jackenmen - Mantenedor de paquetes Chocolatey para Sandboxie ([soporte](https://github.com/Jackenmen/choco-auto/issues?q=is%3Aissue+Sandboxie))
- vedantmgoyal9 - Mantenedor de Winget Releaser para Sandboxie ([soporte](https://github.com/vedantmgoyal9/winget-releaser/issues?q=is%3Aissue+Sandboxie))
- blap - Mantenedor del complemento [SandboxToys2](https://github.com/blap/SandboxToys2)
- diversenok - Análisis de seguridad y PoCs / Correcciones de seguridad
- TechLord - Equipo IRA / Ingeniería inversa
- hg421 - Análisis de seguridad y PoCs / Revisiones de código
- hx1997 - Análisis de seguridad y PoC
- mpheath - Autor del instalador Plus / Correcciones de código / Colaborador
- offhub - Adiciones a la documentación / Correcciones de código / Colaborador
- isaak654 - Plantillas / Documentación / Correcciones de código / Colaborador
- typpos - Adiciones a la interfaz de usuario / Documentación / Correcciones de código
- Yeyixiao - Adiciones de características
- Deezzir - Adiciones de características
- okrc - Correcciones de código
- Sapour - Correcciones de código
- lmou523 - Correcciones de código
- sredna - Correcciones de código para el instalador Classic
- weihongx9315 - Corrección de código
- jorgectf - Flujo de trabajo de CodeQL
- stephtr - CI / Certificación
- yfdyh000 - Soporte de localización para el instalador Plus
- Dyras - Adiciones de plantillas
- cricri-pingouin - Correcciones de la interfaz de usuario
- Valinwolf - Interfaz de usuario / Iconos
- daveout - Interfaz de usuario / Iconos
- NewKidOnTheBlock - Correcciones del changelog
- Naeemh1 - Adiciones a la documentación
- APMichael - Adiciones de plantillas
- 1mm0rt41PC - Adiciones a la documentación
- Luro223 - Adiciones a la documentación
- lwcorp - Adiciones a la documentación
- wilders-soccerfan - Adiciones a la documentación
- LumitoLuma - Parche y script de construcción de Qt5
- SNESNya - Adiciones a la documentación

## 🌏 Traductores

- yuhao2348732, 0x391F, nkh0472, yfdyh000, gexgd0419, Zerorigin, UnnamedOrange, DevSplash, Becods, okrc, 4rt3mi5, sepcnt - Chino simplificado
- TragicLifeHu, Hulen, xiongsp - Chino tradicional
- RockyTDR - Neerlandés
- clexanis, Mmoi-Fr, hippalectryon-0, Monsieur Pissou - Francés (proporcionado por correo electrónico)
- bastik-1001, APMichael - Alemán
- timinoun - Húngaro (proporcionado por correo electrónico)
- isaak654, DerivativeOfLog7 - Italiano
- takahiro-itou - Japonés
- VenusGirl - Coreano
- 7zip - Polaco ([proporcionado por separado](https://forum.xanasoft.com/viewtopic.php?f=12&t=4&start=30))
- JNylson - Portugués y portugués brasileño
- lufog - Ruso
- LumitoLuma, sebadamus - Español
- 1FF, Thatagata - Sueco (proporcionado por correo electrónico o solicitud de extracción)
- xorcan, fmbxnary, offhub - Turco
- SuperMaxusa, lufog - Ucraniano
- GunGunGun - Vietnamita

Se anima a todos los traductores a revisar las [notas y consejos de localización](https://git.io/J9G19) antes de enviar una traducción.
