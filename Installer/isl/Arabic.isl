; *** Inno Setup version 6.5.0+ Arabic messages ***
;
; To download user-contributed translations of this file, go to:
; https://jrsoftware.org/files/istrans/
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; Translated to Arabic by: azizr12 / Qwen
; Community Contribution
; For contributions, please visit:
; https://jrsoftware.org/files/istrans/

[LangOptions]
; The following three entries are very important. Be sure to read and
; understand the '[LangOptions] section' topic in the help file.
LanguageName=العربية
LanguageID=$0401
LanguageCodePage=1256
RightToLeft=yes
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=
;DialogFontSize=9
;DialogFontBaseScaleWidth=7
;DialogFontBaseScaleHeight=15
;WelcomeFontName=Segoe UI
;WelcomeFontSize=14

[Messages]

; *** Application titles
SetupAppTitle=المثبت
SetupWindowTitle=المثبت - %1
UninstallAppTitle=إلغاء التثبيت
UninstallAppFullTitle=إلغاء تثبيت %1

; *** Misc. common
InformationTitle=معلومات
ConfirmTitle=تأكيد
ErrorTitle=خطأ

; *** SetupLdr messages
SetupLdrStartupMessage=سيقوم هذا بتثبيت %1. هل ترغب في المتابعة؟
LdrCannotCreateTemp=خطأ في إنشاء ملف مؤقت. تم إيقاف المثبت.
LdrCannotExecTemp=خطأ في تنفيذ الملف في المجلد المؤقت. تم إيقاف المثبت.
HelpTextNote=

; *** Startup error messages
LastErrorMessage=%1.%n%nخطأ %2: %3
SetupFileMissing=الملف %1 غير موجود في مجلد التثبيت. يرجى إصلاح المشكلة أو الحصول على نسخة جديدة من البرنامج.
SetupFileCorrupt=ملفات المثبت تالفة. يرجى الحصول على نسخة جديدة من البرنامج.
SetupFileCorruptOrWrongVer=ملفات المثبت تالفة، أو غير متوافقة مع هذا الإصدار من المثبت. يرجى إصلاح المشكلة أو الحصول على نسخة جديدة من البرنامج.
InvalidParameter=تم تمرير معلمة غير صالحة إلى سطر الأوامر:%n%n%1
SetupAlreadyRunning=المثبت قيد التشغيل بالفعل.
WindowsVersionNotSupported=هذا البرنامج لا يدعم إصدار ويندوز الذي يعمل على جهاز الكمبيوتر الخاص بك.
WindowsServicePackRequired=يتطلب هذا البرنامج %1 حزمة الخدمة %2 أو أحدث.
NotOnThisPlatform=هذا البرنامج لا يعمل على %1.
OnlyOnThisPlatform=يجب تشغيل هذا البرنامج على %1.
OnlyOnTheseArchitectures=يمكن تثبيت هذا البرنامج فقط على إصدارات ويندوز المصممة لمعالجات الهندسة المعمارية التالية:%n%n%1
WinVersionTooLowError=يتطلب هذا البرنامج %1 الإصدار %2 أو أحدث.
WinVersionTooHighError=لا يمكن تثبيت هذا البرنامج على %1 الإصدار %2 أو أحدث.
AdminPrivilegesRequired=يجب تسجيل الدخول كمسؤول عند تثبيت هذا البرنامج.
PowerUserPrivilegesRequired=يجب تسجيل الدخول كمسؤول أو أن تكون عضواً في مجموعة مستخدمي الطاقة (Power Users) عند تثبيت هذا البرنامج.
SetupAppRunningError=اكتشف المثبت أن %1 قيد التشغيل حالياً.%n%nيرجى إغلاق جميع مثيلات البرنامج الآن، ثم انقر فوق "موافق" للمتابعة، أو "إلغاء" للخروج.
UninstallAppRunningError=اكتشف برنامج إلغاء التثبيت أن %1 قيد التشغيل حالياً.%n%nيرجى إغلاق جميع مثيلات البرنامج الآن، ثم انقر فوق "موافق" للمتابعة، أو "إلغاء" للخروج.

; *** Startup questions
PrivilegesRequiredOverrideTitle=تحديد وضع التثبيت
PrivilegesRequiredOverrideInstruction=تحديد وضع التثبيت
PrivilegesRequiredOverrideText1=يمكن تثبيت %1 لجميع المستخدمين (يتطلب صلاحيات المسؤول)، أو لك فقط.
PrivilegesRequiredOverrideText2=يمكن تثبيت %1 لك فقط، أو لجميع المستخدمين (يتطلب صلاحيات المسؤول).
PrivilegesRequiredOverrideAllUsers=تثبيت ل&جميع المستخدمين
PrivilegesRequiredOverrideAllUsersRecommended=تثبيت ل&جميع المستخدمين (موصى به)
PrivilegesRequiredOverrideCurrentUser=تثبيت لي &فقط
PrivilegesRequiredOverrideCurrentUserRecommended=تثبيت لي &فقط (موصى به)

; *** Misc. errors
ErrorCreatingDir=لم يتمكن المثبت من إنشاء المجلد "%1"
ErrorTooManyFilesInDir=لا يمكن إنشاء ملف في المجلد "%1" لأنه يحتوي على عدد كبير جداً من الملفات.

; *** Setup common messages
ExitSetupTitle=الخروج من المثبت
ExitSetupMessage=لم يكتمل التثبيت. إذا قمت بالخروج الآن، فلن يتم تثبيت البرنامج.%n%nيمكنك تشغيل المثبت مرة أخرى في وقت لاحق لإكمال التثبيت.%n%nهل تريد الخروج من المثبت؟
AboutSetupMenuItem=&حول المثبت...
AboutSetupTitle=حول المثبت
AboutSetupMessage=%1 %2%n%3%n%nالصفحة الرئيسية لـ %1:%n%4
AboutSetupNote=
TranslatorNote=

; *** Buttons
ButtonBack=< &السابق
ButtonNext=&التالي >
ButtonInstall=&تثبيت
ButtonOK=موافق
ButtonCancel=إلغاء
ButtonYes=&نعم
ButtonYesToAll=نعم لل&كل
ButtonNo=&لا
ButtonNoToAll=لا لل&كل
ButtonFinish=&إنهاء
ButtonBrowse=&استعراض...
ButtonWizardBrowse=ا&ستعراض...
ButtonNewFolder=&مجلد جديد

; *** "Select Language" dialog messages
SelectLanguageTitle=تحديد لغة المثبت
SelectLanguageLabel=حدد اللغة التي سيتم استخدامها أثناء التثبيت.

; *** Common wizard text
ClickNext=انقر فوق "التالي" للمتابعة، أو "إلغاء" للخروج من المثبت.
BeveledLabel=
BrowseDialogTitle=استعراض للمجلد
BrowseDialogLabel=حدد مجلداً من القائمة أدناه، ثم انقر فوق "موافق".
NewFolderName=مجلد جديد

; *** "Welcome" wizard page
WelcomeLabel1=مرحباً بك في معالج تثبيت [name]
WelcomeLabel2=سيقوم هذا بتثبيت [name/ver] على جهاز الكمبيوتر الخاص بك.%n%nيوصى بإغلاق جميع التطبيقات الأخرى قبل المتابعة.

; *** "Password" wizard page
WizardPassword=كلمة المرور
PasswordLabel1=عملية التثبيت هذه محمية بكلمة مرور.
PasswordLabel3=يرجى إدخال كلمة المرور، ثم انقر فوق "التالي" للمتابعة. كلمات المرور حساسة لحالة الأحرف.
PasswordEditLabel=&كلمة المرور:
IncorrectPassword=كلمة المرور التي أدخلتها غير صحيحة. يرجى المحاولة مرة أخرى.

; *** "License Agreement" wizard page
WizardLicense=اتفاقية الترخيص
LicenseLabel=يرجى قراءة المعلومات الهامة التالية قبل المتابعة.
LicenseLabel3=يرجى قراءة اتفاقية الترخيص التالية. يجب عليك قبول شروط هذه الاتفاقية قبل المتابعة في التثبيت.
LicenseAccepted=أوافق على &الاتفاقية
LicenseNotAccepted=لا أوافق على الا&تفاقية

; *** "Information" wizard pages
WizardInfoBefore=معلومات
InfoBeforeLabel=يرجى قراءة المعلومات الهامة التالية قبل المتابعة.
InfoBeforeClickLabel=عندما تكون مستعداً لمتابعة التثبيت، انقر فوق "التالي".
WizardInfoAfter=معلومات
InfoAfterLabel=يرجى قراءة المعلومات الهامة التالية قبل المتابعة.
InfoAfterClickLabel=عندما تكون مستعداً لمتابعة التثبيت، انقر فوق "التالي".

; *** "User Information" wizard page
WizardUserInfo=معلومات المستخدم
UserInfoDesc=يرجى إدخال معلوماتك.
UserInfoName=&اسم المستخدم:
UserInfoOrg=&المؤسسة:
UserInfoSerial=&الرقم التسلسلي:
UserInfoNameRequired=يجب إدخال اسم.

; *** "Select Destination Location" wizard page
WizardSelectDir=تحديد موقع الوجهة
SelectDirDesc=أين يجب تثبيت [name]؟
SelectDirLabel3=سيقوم المثبت بتثبيت [name] في المجلد التالي.
SelectDirBrowseLabel=للمتابعة، انقر فوق "التالي". إذا كنت ترغب في اختيار مجلد مختلف، انقر فوق "استعراض".
DiskSpaceGBLabel=يلزم وجود [gb] جيجابايت على الأقل من مساحة القرص الحرة.
DiskSpaceMBLabel=يلزم وجود [mb] ميجابايت على الأقل من مساحة القرص الحرة.
CannotInstallToNetworkDrive=لا يمكن للمثبت التثبيت على محرك أقراص شبكة.
CannotInstallToUNCPath=لا يمكن للمثبت التثبيت على مسار UNC.
InvalidPath=يجب إدخال مسار كامل يحتوي على حرف محرك أقراص، على سبيل المثال:%n%nC:\App%n%nأو مسار UNC بالشكل:%n%n\\server\share
InvalidDrive=محرك الأقراص أو مشاركة UNC المحددة غير موجودة أو لا يمكن الوصول إليها. يرجى تحديد محرك أقراص آخر.
DiskSpaceWarningTitle=مساحة القرص غير كافية
DiskSpaceWarning=يحتاج المثبت إلى %1 كيلوبايت على الأقل من مساحة القرص الحرة للتثبيت، لكن محرك الأقراص المحدد يحتوي على %2 كيلوبايت فقط متاحة.%n%nهل ترغب في المتابعة على أي حال؟
DirNameTooLong=اسم المجلد أو المسار طويل جداً.
InvalidDirName=اسم المجلد غير صالح.
BadDirName32=لا يمكن أن تحتوي أسماء المجلدات على أي من الأحرف التالية:%n%n%1
DirExistsTitle=المجلد موجود بالفعل
DirExists=المجلد التالي:%n%n%1%n%nموجود بالفعل. هل ترغب في التثبيت فيه على أي حال؟
DirDoesntExistTitle=المجلد غير موجود
DirDoesntExist=المجلد التالي:%n%n%1%n%nغير موجود. هل ترغب في إنشائه؟

; *** "Select Components" wizard page
WizardSelectComponents=تحديد المكونات
SelectComponentsDesc=ما هي المكونات التي يجب تثبيتها؟
SelectComponentsLabel2=حدد المكونات التي ترغب في تثبيتها؛ وألغِ تحديد المكونات التي لا ترغب في تثبيتها. انقر فوق "التالي" عندما تكون مستعداً للمتابعة.
FullInstallation=تثبيت كامل
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=تثبيت مدمج
CustomInstallation=تثبيت مخصص
NoUninstallWarningTitle=مكونات موجودة
NoUninstallWarning=اكتشف المثبت أن المكونات التالية مثبتة بالفعل على جهاز الكمبيوتر الخاص بك:%n%n%1%n%nإلغاء تحديد هذه المكونات لن يؤدي إلى إزالتها.%n%nهل ترغب في المتابعة على أي حال؟
ComponentSize1=%1 كيلوبايت
ComponentSize2=%1 ميجابايت
ComponentsDiskSpaceGBLabel=يحتاج التحديد الحالي إلى [gb] جيجابايت على الأقل من مساحة القرص.
ComponentsDiskSpaceMBLabel=يحتاج التحديد الحالي إلى [mb] ميجابايت على الأقل من مساحة القرص.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=تحديد المهام الإضافية
SelectTasksDesc=ما هي المهام الإضافية التي يجب تنفيذها؟
SelectTasksLabel2=حدد المهام الإضافية التي ترغب في أن يقوم المثبت بتنفيذها أثناء تثبيت [name]، ثم انقر فوق "التالي".

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=تحديد مجلد قائمة ابدأ
SelectStartMenuFolderDesc=أين يجب أن يضع المثبت اختصارات البرنامج؟
SelectStartMenuFolderLabel3=سيقوم المثبت بإنشاء اختصارات البرنامج في مجلد قائمة ابدأ التالي.
SelectStartMenuFolderBrowseLabel=للمتابعة، انقر فوق "التالي". إذا كنت ترغب في اختيار مجلد مختلف، انقر فوق "استعراض".
MustEnterGroupName=يجب إدخال اسم مجلد.
GroupNameTooLong=اسم المجلد أو المسار طويل جداً.
InvalidGroupName=اسم المجلد غير صالح.
BadGroupName=لا يمكن أن يحتوي اسم المجلد على أي من الأحرف التالية:%n%n%1
NoProgramGroupCheck2=&عدم إنشاء مجلد في قائمة ابدأ

; *** "Ready to Install" wizard page
WizardReady=جاهز للتثبيت
ReadyLabel1=المثبت جاهز الآن لبدء تثبيت [name] على جهاز الكمبيوتر الخاص بك.
ReadyLabel2a=انقر فوق "تثبيت" لمتابعة التثبيت، أو انقر فوق "السابق" إذا كنت ترغب في مراجعة أو تغيير أي إعدادات.
ReadyLabel2b=انقر فوق "تثبيت" لمتابعة التثبيت.
ReadyMemoUserInfo=معلومات المستخدم:
ReadyMemoDir=موقع الوجهة:
ReadyMemoType=نوع التثبيت:
ReadyMemoComponents=المكونات المحددة:
ReadyMemoGroup=مجلد قائمة ابدأ:
ReadyMemoTasks=المهام الإضافية:

; *** TDownloadWizardPage wizard page and DownloadTemporaryFile
DownloadingLabel2=جاري تنزيل الملفات...
ButtonStopDownload=&إيقاف التنزيل
StopDownload=هل أنت متأكد من أنك تريد إيقاف التنزيل؟
ErrorDownloadAborted=تم إيقاف التنزيل
ErrorDownloadFailed=فشل التنزيل: %1 %2
ErrorDownloadSizeFailed=فشل في الحصول على الحجم: %1 %2
ErrorProgress=تقدم غير صالح: %1 من %2
ErrorFileSize=حجم ملف غير صالح: متوقع %1، تم العثور على %2

; *** TExtractionWizardPage wizard page and ExtractArchive
ExtractingLabel=جاري استخراج الملفات...
ButtonStopExtraction=&إيقاف الاستخراج
StopExtraction=هل أنت متأكد من أنك تريد إيقاف الاستخراج؟
ErrorExtractionAborted=تم إيقاف الاستخراج
ErrorExtractionFailed=فشل الاستخراج: %1

; *** Archive extraction failure details
ArchiveIncorrectPassword=كلمة المرور غير صحيحة
ArchiveIsCorrupted=ملف الأرشيف تالف
ArchiveUnsupportedFormat=تنسيق ملف الأرشيف غير مدعوم

; *** "Preparing to Install" wizard page
WizardPreparing=جاري التحضير للتثبيت
PreparingDesc=يقوم المثبت بالتحضير لتثبيت [name] على جهاز الكمبيوتر الخاص بك.
PreviousInstallNotCompleted=لم يكتمل تثبيت/إلغاء تثبيت البرنامج السابق. يجب عليك إعادة تشغيل جهاز الكمبيوتر الخاص بك لإكمال ذلك التثبيت.%n%nبعد إعادة تشغيل جهاز الكمبيوتر الخاص بك، قم بتشغيل المثبت مرة أخرى لإكمال تثبيت [name].
CannotContinue=لا يمكن للمثبت المتابعة. يرجى النقر فوق "إلغاء" للخروج.
ApplicationsFound=تستخدم التطبيقات التالية ملفات يحتاج المثبت إلى تحديثها. يوصى بالسماح للمثبت بإغلاق هذه التطبيقات تلقائياً.
ApplicationsFound2=تستخدم التطبيقات التالية ملفات يحتاج المثبت إلى تحديثها. يوصى بالسماح للمثبت بإغلاق هذه التطبيقات تلقائياً. بعد الانتهاء من التثبيت، سيحاول المثبت إعادة تشغيل هذه التطبيقات.
CloseApplications=إغلاق التطبيقات &تلقائياً
DontCloseApplications=&عدم إغلاق التطبيقات
ErrorCloseApplications=لم يتمكن المثبت من إغلاق جميع التطبيقات تلقائياً. يوصى بإغلاق جميع التطبيقات التي تستخدم الملفات التي يحتاج المثبت إلى تحديثها قبل المتابعة.
PrepareToInstallNeedsRestart=يجب على المثبت إعادة تشغيل جهاز الكمبيوتر الخاص بك. بعد إعادة تشغيل جهاز الكمبيوتر الخاص بك، قم بتشغيل المثبت مرة أخرى لإكمال تثبيت [name].%n%nهل ترغب في إعادة التشغيل الآن؟

; *** "Installing" wizard page
WizardInstalling=جاري التثبيت
InstallingLabel=يرجى الانتظار بينما يقوم المثبت بتثبيت [name] على جهاز الكمبيوتر الخاص بك.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=إكمال معالج تثبيت [name]
FinishedLabelNoIcons=أكمل المثبت تثبيت [name] على جهاز الكمبيوتر الخاص بك.
FinishedLabel=أكمل المثبت تثبيت [name] على جهاز الكمبيوتر الخاص بك. يمكن تشغيل التطبيق عن طريق تحديد الاختصارات المثبتة.
ClickFinish=انقر فوق "إنهاء" للخروج من المثبت.
FinishedRestartLabel=لإكمال تثبيت [name]، يجب على المثبت إعادة تشغيل جهاز الكمبيوتر الخاص بك. هل ترغب في إعادة التشغيل الآن؟
FinishedRestartMessage=لإكمال تثبيت [name]، يجب على المثبت إعادة تشغيل جهاز الكمبيوتر الخاص بك.%n%nهل ترغب في إعادة التشغيل الآن؟
ShowReadmeCheck=نعم، أريد عرض ملف اقرأني (README)
YesRadio=&نعم، أعد تشغيل الكمبيوتر الآن
NoRadio=&لا، سأعيد تشغيل الكمبيوتر لاحقاً
; used for example as 'Run MyProg.exe'
RunEntryExec=تشغيل %1
; used for example as 'View Readme.txt'
RunEntryShellExec=عرض %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=يحتاج المثبت إلى القرص التالي
SelectDiskLabel2=يرجى إدخال القرص %1 والنقر فوق "موافق".%n%nإذا كانت الملفات الموجودة على هذا القرص في مجلد غير المجلد المعروض أدناه، فقم بإدخال المسار الصحيح أو انقر فوق "استعراض".
PathLabel=&المسار:
FileNotInDir2=لم يتم العثور على الملف "%1" في المسار "%2". يرجى إدخال القرص الصحيح أو تحديد مجلد آخر.
SelectDirectoryLabel=يرجى تحديد موقع القرص التالي.

; *** Installation phase messages
SetupAborted=لم يكتمل المثبت.%n%nيرجى إصلاح المشكلة ثم تشغيل المثبت مرة أخرى.
AbortRetryIgnoreSelectAction=تحديد الإجراء
AbortRetryIgnoreRetry=&إعادة المحاولة
AbortRetryIgnoreIgnore=&تجاهل الخطأ والمتابعة
AbortRetryIgnoreCancel=إلغاء عملية التثبيت
RetryCancelSelectAction=تحديد الإجراء
RetryCancelRetry=&إعادة المحاولة
RetryCancelCancel=إلغاء

; *** Installation status messages
StatusClosingApplications=جاري إغلاق التطبيقات...
StatusCreateDirs=جاري إنشاء المجلدات...
StatusExtractFiles=جاري استخراج الملفات...
StatusDownloadFiles=جاري تنزيل الملفات...
StatusCreateIcons=جاري إنشاء الاختصارات...
StatusCreateIniEntries=جاري إنشاء إدخالات INI...
StatusCreateRegistryEntries=جاري إنشاء إدخالات السجل (Registry)...
StatusRegisterFiles=جاري تسجيل الملفات...
StatusSavingUninstall=جاري حفظ معلومات إلغاء التثبيت...
StatusRunProgram=جاري إنهاء التثبيت...
StatusRestartingApplications=جاري إعادة تشغيل التطبيقات...
StatusRollback=جاري التراجع عن التغييرات...

; *** Misc. errors
ErrorInternal2=خطأ داخلي: %1
ErrorFunctionFailedNoCode=فشل %1
ErrorFunctionFailed=فشل %1؛ الرمز %2
ErrorFunctionFailedWithMessage=فشل %1؛ الرمز %2.%n%3
ErrorExecutingProgram=خطأ في تنفيذ الملف:%n%1

; *** Registry errors
ErrorRegOpenKey=خطأ في فتح مفتاح السجل:%n%1\%2
ErrorRegCreateKey=خطأ في إنشاء مفتاح السجل:%n%1\%2
ErrorRegWriteKey=خطأ في الكتابة في مفتاح السجل:%n%1\%2

; *** INI errors
ErrorIniEntry=خطأ في إنشاء إدخال INI في الملف "%1".

; *** File copying errors
FileAbortRetryIgnoreSkipNotRecommended=&تخطي هذا الملف (غير موصى به)
FileAbortRetryIgnoreIgnoreNotRecommended=&تجاهل الخطأ والمتابعة (غير موصى به)
SourceIsCorrupted=ملف المصدر تالف
SourceDoesntExist=ملف المصدر "%1" غير موجود
SourceVerificationFailed=فشل التحقق من ملف المصدر: %1
VerificationSignatureDoesntExist=ملف التوقيع "%1" غير موجود
VerificationSignatureInvalid=ملف التوقيع "%1" غير صالح
VerificationKeyNotFound=يستخدم ملف التوقيع "%1" مفتاحاً غير معروف
VerificationFileNameIncorrect=اسم الملف غير صحيح
VerificationFileTagIncorrect=علامة الملف غير صحيحة
VerificationFileSizeIncorrect=حجم الملف غير صحيح
VerificationFileHashIncorrect=تجزئة الملف غير صحيحة
ExistingFileReadOnly2=لا يمكن استبدال الملف الموجود لأنه للقراءة فقط.
ExistingFileReadOnlyRetry=&إزالة خاصية القراءة فقط وإعادة المحاولة
ExistingFileReadOnlyKeepExisting=&الاحتفاظ بالملف الموجود
ErrorReadingExistingDest=حدث خطأ أثناء محاولة قراءة الملف الموجود:
FileExistsSelectAction=تحديد الإجراء
FileExists2=الملف موجود بالفعل.
FileExistsOverwriteExisting=&الكتابة فوق الملف الموجود
FileExistsKeepExisting=&الاحتفاظ بالملف الموجود
FileExistsOverwriteOrKeepAll=&قم بذلك لجميع التعارضات اللاحقة
ExistingFileNewerSelectAction=تحديد الإجراء
ExistingFileNewer2=الملف الموجود أحدث من الملف الذي يحاول المثبت تثبيته.
ExistingFileNewerOverwriteExisting=&الكتابة فوق الملف الموجود
ExistingFileNewerKeepExisting=&الاحتفاظ بالملف الموجود (موصى به)
ExistingFileNewerOverwriteOrKeepAll=&قم بذلك لجميع التعارضات اللاحقة
ErrorChangingAttr=حدث خطأ أثناء محاولة تغيير سمات الملف الموجود:
ErrorCreatingTemp=حدث خطأ أثناء محاولة إنشاء ملف في مجلد الوجهة:
ErrorReadingSource=حدث خطأ أثناء محاولة قراءة ملف المصدر:
ErrorCopying=حدث خطأ أثناء محاولة نسخ الملف:
ErrorDownloading=حدث خطأ أثناء محاولة تنزيل الملف:
ErrorExtracting=حدث خطأ أثناء محاولة استخراج ملف الأرشيف:
ErrorReplacingExistingFile=حدث خطأ أثناء محاولة استبدال الملف الموجود:
ErrorRestartReplace=فشل RestartReplace:
ErrorRenamingTemp=حدث خطأ أثناء محاولة إعادة تسمية ملف في مجلد الوجهة:
ErrorRegisterServer=تعذر تسجيل DLL/OCX: %1
ErrorRegSvr32Failed=فشل RegSvr32 برمز الخروج %1
ErrorRegisterTypeLib=تعذر تسجيل مكتبة الأنواع: %1

; *** Uninstall display name markings
; used for example as 'My Program (32-bit)'
UninstallDisplayNameMark=%1 (%2)
; used for example as 'My Program (32-bit, All users)'
UninstallDisplayNameMarks=%1 (%2، %3)
UninstallDisplayNameMark32Bit=32-بت
UninstallDisplayNameMark64Bit=64-بت
UninstallDisplayNameMarkAllUsers=جميع المستخدمين
UninstallDisplayNameMarkCurrentUser=المستخدم الحالي

; *** Post-installation errors
ErrorOpeningReadme=حدث خطأ أثناء محاولة فتح ملف اقرأني (README).
ErrorRestartingComputer=لم يتمكن المثبت من إعادة تشغيل الكمبيوتر. يرجى القيام بذلك يدوياً.

; *** Uninstaller messages
UninstallNotFound=الملف "%1" غير موجود. لا يمكن إلغاء التثبيت.
UninstallOpenError=لا يمكن فتح الملف "%1". لا يمكن إلغاء التثبيت.
UninstallUnsupportedVer=ملف سجل إلغاء التثبيت "%1" بتنسيق لا يمكن التعرف عليه بواسطة هذا الإصدار من برنامج إلغاء التثبيت. لا يمكن إلغاء التثبيت.
UninstallUnknownEntry=تمت مواجهة إدخال غير معروف (%1) في ملف سجل إلغاء التثبيت
ConfirmUninstall=هل أنت متأكد من رغبتك في إلغاء تثبيت %1 وجميع مكوناته بالكامل؟
UninstallOnlyOnWin64=يمكن إلغاء تثبيت هذا البرنامج المثبت على ويندوز 64-بت فقط.
OnlyAdminCanUninstall=يمكن إلغاء تثبيت هذا البرنامج المثبت بواسطة مستخدم ذي صلاحيات مسؤول فقط.
UninstallStatusLabel=يرجى الانتظار بينما يقوم برنامج إلغاء التثبيت بإزالة %1 من جهاز الكمبيوتر الخاص بك.
UninstalledAll=تم إلغاء تثبيت %1 بنجاح من جهاز الكمبيوتر الخاص بك.
UninstalledMost=اكتمل إلغاء تثبيت %1.%n%nتعذر إلغاء تثبيت بعض المكونات. يمكن إزالتها يدوياً.
UninstalledAndNeedsRestart=يجب إعادة تشغيل جهاز الكمبيوتر الخاص بك لإكمال إلغاء تثبيت %1.%n%nهل ترغب في إعادة التشغيل الآن؟
UninstallDataCorrupted=الملف "%1" تالف. لا يمكن إلغاء التثبيت.

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=حذف الملف المشترك؟
ConfirmDeleteSharedFile2=يشير النظام إلى أن الملف المشترك التالي لا تستخدمه أي برامج أخرى. هل ترغب في أن يقوم برنامج إلغاء التثبيت بحذف هذا الملف المشترك؟%n%nإذا كانت هناك برامج لا تزال تستخدم هذا الملف، فقد لا تعمل هذه البرامج بشكل صحيح بعد حذفه. إذا لم تكن متأكداً، اختر "لا". لن يؤدي الاحتفاظ بالملف على نظامك إلى أي ضرر.
SharedFileNameLabel=اسم الملف:
SharedFileLocationLabel=الموقع:
WizardUninstalling=حالة إلغاء التثبيت
StatusUninstalling=جاري إلغاء تثبيت %1...

; *** Shutdown block reasons
ShutdownBlockReasonInstallingApp=جاري تثبيت %1.
ShutdownBlockReasonUninstallingApp=جاري إلغاء تثبيت %1.

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]
NameAndVersion=%1 الإصدار %2
AdditionalIcons=اختصارات إضافية:
CreateDesktopIcon=إنشاء أيقونة على &سطح المكتب
CreateQuickLaunchIcon=إنشاء أيقونة تشغيل &سريع
ProgramOnTheWeb=%1 على الويب
UninstallProgram=إلغاء تثبيت %1
LaunchProgram=تشغيل %1
AssocFileExtension=&ربط %1 بامتداد الملف %2
AssocingFileExtension=جاري ربط %1 بامتداد الملف %2...
AutoStartProgramGroupDescription=بدء التشغيل:
AutoStartProgram=بدء تشغيل تلقائي لـ %1
AddonHostProgramNotFound=لم يتم العثور على %1 في المجلد المحدد.%n%nهل ترغب في المتابعة على أي حال؟