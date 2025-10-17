#include "stdafx.h"
#include "IniHighlighter.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../version.h"


// Settings validation, tooltip handling and auto completion
const QString CIniHighlighter::DEFAULT_SETTINGS_FILE = "SbieSettings";
const QString CIniHighlighter::DEFAULT_VERSION = "0.0.0";

QVersionNumber CIniHighlighter::s_currentVersion;
QString CIniHighlighter::s_currentLanguage;
QMutex CIniHighlighter::s_languageMutex;

QHash<QString, SettingInfo> CIniHighlighter::validSettings;
QDateTime CIniHighlighter::lastFileModified;
QDateTime CIniHighlighter::lastUserFileModified;
bool CIniHighlighter::settingsLoaded = false;
bool CIniHighlighter::userIniLoaded = false;
QMutex CIniHighlighter::settingsMutex;
QMutex CIniHighlighter::userSettingsMutex;
QString CIniHighlighter::s_masterVersion;

QHash<QString, QString> CIniHighlighter::tooltipCache;
QMutex CIniHighlighter::tooltipCacheMutex;
CIniHighlighter::TooltipMode CIniHighlighter::s_tooltipMode = TooltipMode::BasicInfo;
QMutex CIniHighlighter::s_tooltipModeMutex;

QString CIniHighlighter::s_tooltipBgColorDark, CIniHighlighter::s_tooltipBgColorLight;
QString CIniHighlighter::s_tooltipTextColorDark, CIniHighlighter::s_tooltipTextColorLight;

CIniHighlighter::KeywordGroup<CIniHighlighter::KeywordType::Context> CIniHighlighter::contextData;
CIniHighlighter::KeywordGroup<CIniHighlighter::KeywordType::Category> CIniHighlighter::categoryData;
CIniHighlighter::KeywordGroup<CIniHighlighter::KeywordType::Requirements> CIniHighlighter::requirementsData;

QHash<QString, QString> CIniHighlighter::hideConfRules;
QHash<QString, QString> CIniHighlighter::hideConfExclusions;
QMutex CIniHighlighter::hideConfMutex;

int CIniHighlighter::s_maxSettingNameLength = 64;
bool CIniHighlighter::s_maxSettingNameLengthValid = false;
int CIniHighlighter::s_minSettingNameLength = 1;
bool CIniHighlighter::s_minSettingNameLengthValid = false;

bool CIniHighlighter::settingsDirty = true;
bool CIniHighlighter::userSettingsDirty = true;

namespace {
	// HTML fragments
    namespace HtmlTags {
        static const QString HTML_START = QStringLiteral("<html><body>");
        static const QString HTML_END = QStringLiteral("</body><html>");
        static const QString TR_TD_START = QStringLiteral("<tr><td ");
        static const QString TD_TR_END = QStringLiteral("</td></tr>");
        static const QString TD_TAG = QStringLiteral("<td>");
        static const QString TD_START = QStringLiteral("<td ");
        static const QString TD_END = QStringLiteral("</td>");
        static const QString TR_TAG = QStringLiteral("<tr>");
        static const QString TR_END = QStringLiteral("</tr>");
        static const QString TAG_CLOSE = QStringLiteral(">");
        static const QString TABLE_START = QStringLiteral("<table ");
        static const QString TABLE_END = QStringLiteral("</table>");
        static const QString VALUE_PREFIX = QStringLiteral(": ");
        static const QString BR_NBSP = QStringLiteral("<br>&nbsp;&nbsp;");
        static const QString TABLE_HEADER_START = QStringLiteral("<tr><td colspan='2' ");
		static const QString SPAN_COLOR_START = QStringLiteral("<span style='color:");
		static const QString SPAN_COLOR_CLOSE = QStringLiteral(";'>");
		static const QString SPAN_END = QStringLiteral("</span>");
		static const QString SPAN_FONT_CONSOLAS = QStringLiteral("<span style='font-family: Consolas, monospace;'>");
    }

	// HTML attributes and styles
	namespace HtmlAttribs {
		static const QString TABLE_STYLE = QStringLiteral("style='border:none; white-space:nowrap; background-color:%1; color:%2;'");
		static const QString LABEL_STYLE = QStringLiteral("style='text-align:left; padding-right:8px; color:%1;'");
		static const QString STYLE_BOLD = QStringLiteral("font-weight:bold;");
		static const QString STYLE_ITALIC = QStringLiteral("font-style:italic;");
		static const QString STYLE_UNDERLINE = QStringLiteral("text-decoration:underline;");
		static const QString STYLE_ALIGNMENT = QStringLiteral("text-align:%1;");
		static const QString COLOR_VAR = QStringLiteral("color:%1");
		static const QString STYLE_VAR = QStringLiteral("style='%1'");
		static const QString STYLE_START = QStringLiteral("style='");
		static const QString STYLE_TOP = QStringLiteral(" style='vertical-align:top;'");
		static const QString SEPARATOR = QStringLiteral(";");
	}
    
    // Common text replacements
    namespace TextReplacements {
		static const QString LABEL_JOINER = QStringLiteral(" + ");
        static const QString ESCAPE_N = QStringLiteral("\\n");
        static const QString ESCAPE_T = QStringLiteral("\\t");
        static const QString ESCAPE_R = QStringLiteral("\\r");
        static const QString ESCAPE_S = QStringLiteral("\\s");
        static const QString ESCAPE_BACKSLASH = QStringLiteral("\\\\");
        static const QString NEWLINE = QStringLiteral("\n");
        static const QString TAB = QStringLiteral("\t");
        static const QString TAB_NBSP = QStringLiteral("&nbsp;&nbsp;&nbsp;&nbsp;");
        static const QString CARRIAGE_RETURN = QStringLiteral("\r");
        static const QString SPACE = QStringLiteral("&nbsp;");
        static const QString BACKSLASH = QStringLiteral("\\");
    }
    
    // Compiled regex patterns for better performance
    namespace CompiledRegex {
        static const QRegularExpression SECTION_REGEX(R"(^\[([^\]]+)\]\s*$)");
        static const QRegularExpression VERSION_REGEX("^[0-9]+\\.[0-9]+\\.[0-9]+$");
        static const QRegularExpression KEY_REGEX(R"(^([^\s=]+)\s*=)");
        static const QRegularExpression COMMENT_REGEX(R"(^\s*[;#].*)");
        static const QRegularExpression CLEAN_NAME_REGEX("[^a-zA-Z0-9_.]");
        static const QRegularExpression SETTING_NAME_REGEX("^[a-zA-Z0-9_.]+$");
    }
}
// End Settings validation, tooltip handling and auto completion

CIniHighlighter::CIniHighlighter(bool bDarkMode, QTextDocument* parent, bool enableValidation)
	: QSyntaxHighlighter(parent), m_enableValidation(enableValidation)
{
    // Define colors for light and dark mode
    QColor blue = bDarkMode ? QColor("#87CEFA") : QColor("#0000FF"); // Lighter blue for dark mode
    QColor green = bDarkMode ? QColor("#90EE90") : QColor("#008000"); // Lighter green for dark mode
    QColor darkRed = bDarkMode ? QColor("#FF6347") : QColor("#800000"); // Lighter red for dark mode
    QColor red = bDarkMode ? QColor("#FF4500") : QColor("#FF0000"); // Brighter red for dark mode
    QColor black = bDarkMode ? QColor("#DCDCDC") : QColor("#000000"); // Light gray for dark mode
    QColor brown = bDarkMode ? QColor("#F4A460") : QColor("#A52A2A"); // Light brown for dark mode
    QColor purple = bDarkMode ? QColor("#DA70D6") : QColor("#800080"); // Brighter purple for dark mode
    QColor gray = bDarkMode ? QColor("#A9A9A9") : QColor("#808080"); // Lighter gray for dark mode

    HighlightRule rule;

    // Section headers: [Section]
    sectionFormat.setForeground(blue);
    sectionFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^\\s*\\[.*\\]\\s*$");
    rule.format = sectionFormat;
    highlightRules.append(rule);

    // Comments: ; comment or # comment
    commentFormat.setForeground(green);
    rule.pattern = QRegularExpression("^\\s*[;#].*");
    rule.format = commentFormat;
    highlightRules.append(rule);

    // Keys: key=
    keyFormat.setForeground(darkRed);
    rule.pattern = QRegularExpression("^[\\w\\.]+(?=\\s*=)");
    rule.format = keyFormat;
    highlightRules.append(rule);

    // Equals sign: =
    equalsFormat.setForeground(red);
    rule.pattern = QRegularExpression("=");
    rule.format = equalsFormat;
    highlightRules.append(rule);

    // Values: =value
    valueFormat.setForeground(black);
    rule.pattern = QRegularExpression("(?<=\\=).*");
    rule.format = valueFormat;
    highlightRules.append(rule);

    // Initialize formats for value prefix and first comma
    valuePrefixFormat.setForeground(blue);
    firstCommaFormat.setForeground(red);
	
	// Future key format
	futureKeyFormat.setForeground(QColor("darkCyan"));
	futureKeyFormat.setBackground(QColor("white"));

	// Removed key format
	removedKeyFormat.setForeground(QColor("white"));
	removedKeyFormat.setBackground(QColor("black"));
	removedKeyFormat.setFontStrikeOut(true);

	// Renamed key format
	renamedKeyFormat.setForeground(QColor("black"));
	renamedKeyFormat.setBackground(QColor("yellow"));
	renamedKeyFormat.setFontItalic(true);
	
	// Unknown key format
	unknownKeyFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
	unknownKeyFormat.setUnderlineColor(red);

#ifdef INI_WITH_JSON
    // Initialize JSON formats
    jsonKeyFormat.setForeground(brown);
    jsonStringFormat.setForeground(black);
    jsonNumberFormat.setForeground(blue);
    jsonBoolNullFormat.setForeground(purple);
    jsonBracesFormat.setForeground(gray);
    jsonColonFormat.setForeground(red);
    jsonCommaFormat.setForeground(red);

    // 1. JSON Colon: Match colons not preceded by backslash
    HighlightRule jsonRule;
    jsonRule.pattern = QRegularExpression(R"((?<!\\):)");
    jsonRule.format = jsonColonFormat;
    jsonHighlightRules.append(jsonRule);

    // 2. JSON Comma: Match commas not preceded by backslash
    jsonRule.pattern = QRegularExpression(R"((?<!\\),)");
    jsonRule.format = jsonCommaFormat;
    jsonHighlightRules.append(jsonRule);

    // 3. JSON Keys: "key":
    jsonRule.pattern = QRegularExpression(R"("(?:(?:\\.)|[^"\\])*"(?=\s*:))");
    jsonRule.format = jsonKeyFormat;
    jsonHighlightRules.append(jsonRule);

    // 4. JSON Strings: "value" (excluding keys)
    jsonRule.pattern = QRegularExpression(R"("(?:(?:\\.)|[^"\\])*"(?!\s*:))");
    jsonRule.format = jsonStringFormat;
    jsonHighlightRules.append(jsonRule);

    // 5. JSON Numbers: 123, 45.67
    jsonRule.pattern = QRegularExpression(R"(\b-?\d+(\.\d+)?\b)");
    jsonRule.format = jsonNumberFormat;
    jsonHighlightRules.append(jsonRule);

    // 6. JSON Booleans and Null: true, false, null
    jsonRule.pattern = QRegularExpression(R"(\b(true|false|null)\b)", QRegularExpression::CaseInsensitiveOption);
    jsonRule.format = jsonBoolNullFormat;
    jsonHighlightRules.append(jsonRule);

    // 7. JSON Braces and Brackets: { }, [ ]
    jsonRule.pattern = QRegularExpression(R"([\{\}\[\]])");
    jsonRule.format = jsonBracesFormat;
    jsonHighlightRules.append(jsonRule);
#endif

	// Check if we need to load the settings file - with mutex protection
	QString settingsPath = QCoreApplication::applicationDirPath() % "/" % DEFAULT_SETTINGS_FILE % ".ini";
	QFileInfo fileInfo(settingsPath);
	reloadSettingsIniIfNeeded(settingsPath, fileInfo);

	// Check if we need to load the settings file - with mutex protection
	QString userIniPath = QCoreApplication::applicationDirPath() % "/" % DEFAULT_SETTINGS_FILE % ".user.ini";
	QFileInfo userFileInfo(userIniPath);
	reloadUserIniIfNeeded(userIniPath, userFileInfo);

	// Use cached version instead of creating a new one each time
	m_currentVersion = getCurrentVersion();
}

CIniHighlighter::~CIniHighlighter()
{
}

QVersionNumber CIniHighlighter::getCurrentVersion()
{
	if (s_currentVersion.isNull()) {
		QMutexLocker locker(&settingsMutex);
		if (s_currentVersion.isNull()) {
			QString versionStr = QString("%1.%2.%3").arg(VERSION_MJR).arg(VERSION_MIN).arg(VERSION_REV);
			s_currentVersion = QVersionNumber::fromString(versionStr);
		}
	}
	return s_currentVersion;
}

QString CIniHighlighter::normalizeLanguage(const QString& language)
{
	// Handle special case "native" (use default values)
	if (language.compare("native", Qt::CaseInsensitive) == 0) {
		return "";
	}

	// If no language is set, determine from system locale
	QString workingLang = language.isEmpty() ? QLocale::system().name() : language;

	// For English variants, return empty to use default values
	return workingLang.startsWith("en", Qt::CaseInsensitive) ? "" : workingLang.toLower();
}

QString CIniHighlighter::getCurrentLanguage()
{
	// Get the current UI language from configuration
	QString configLanguage = theConf->GetString("Options/UiLanguage");

	// Normalize the language using the helper function
	QString normalizedLanguage = normalizeLanguage(configLanguage);

	// Check if language changed and update cache - similar to needToLoad pattern
	bool needToClearCache = false;
	{
		QMutexLocker locker(&s_languageMutex);
		needToClearCache = s_currentLanguage != normalizedLanguage;
		if (needToClearCache) {
			s_currentLanguage = normalizedLanguage;
		}
	}

	if (needToClearCache) {
		ClearLanguageCache();
	}

	return s_currentLanguage;
}

QString CIniHighlighter::sanitizeVersion(const QString& versionString, bool useDefaultOnInvalid)
{
	QString cleaned = versionString.trimmed();
	cleaned.remove(QRegularExpression("[^0-9.]"));
	
	if (CompiledRegex::VERSION_REGEX.match(cleaned).hasMatch()) {
		return cleaned;
	}
	
	return useDefaultOnInvalid ? DEFAULT_VERSION : QString();
}

QString CIniHighlighter::mergeHtmlStyles(const QString& baseStyle, const QString& additionalStyle)
{
	if (additionalStyle.isEmpty()) {
		return baseStyle;
	}
	
	if (baseStyle.contains(HtmlAttribs::STYLE_START)) {
		QString result = baseStyle;
		// Remove the closing quote and add the additional style
		return result.replace(HtmlAttribs::STYLE_START, additionalStyle.mid(0, additionalStyle.length() - 1) % HtmlAttribs::SEPARATOR);
	} else {
		return baseStyle % " " % additionalStyle;
	}
}

void CIniHighlighter::setCurrentVersion(const QString& version)
{
	m_currentVersion = QVersionNumber::fromString(version);
}

QString CIniHighlighter::extractLanguageCode(const QString& key, const QString& prefix)
{
	QRegularExpression langRegex(QString("^%1_(.+)$").arg(prefix), QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch langMatch = langRegex.match(key);

	if (langMatch.hasMatch()) {
		return langMatch.captured(1).toLower();
	}
	return QString(); // Return empty string if no match
}

template<CIniHighlighter::KeywordType Type>
bool CIniHighlighter::processConfigKeyword(const QString& key, const QString& value,
	const QString& baseKey,
	KeywordGroup<Type>& keywordGroup)
{
	if (key.compare(baseKey, Qt::CaseInsensitive) == 0) {
		// Parse default keyword mappings
		keywordGroup.mappings = parseKeywordMappings<Type>(value);
		return true;
	}
	else if (key.startsWith(baseKey % "_", Qt::CaseInsensitive)) {
		// Handle localized mappings
		QString langCode = extractLanguageCode(key, baseKey);
		if (!langCode.isEmpty()) {
			keywordGroup.localizedMappings.insert(langCode, parseKeywordMappings<Type>(value));
		}
		return true;
	}
	else {
		// Create a copy to avoid modifying the original baseKey
		QString styleKey = baseKey;
		styleKey.replace("Conf", "Styles");
		if (key.compare(styleKey, Qt::CaseInsensitive) == 0) {
			// Parse tooltip style configuration
			keywordGroup.tooltipStyle = parseStyleConfig(value);
			return true;
		}
	}

	return false;
}

static void UpdateCheckboxesOnAllTopLevels(const QStringList& objectNames, std::function<void(QCheckBox*)> updater)
{
	QWidgetList topLevels = QApplication::topLevelWidgets();
	for (QWidget* widget : topLevels) {
		for (const QString& name : objectNames) {
			QCheckBox* box = widget->findChild<QCheckBox*>(name);
			if (box) {
				updater(box);
			}
		}
	}
}

// Helper to deduplicate keyword mappings from a merged string
template<typename KeywordInfoType>
QList<KeywordInfoType> deduplicateKeywordMappings(const QList<KeywordInfoType>& mappings) {
	QSet<QString> seen;
	QList<KeywordInfoType> result;
	for (const auto& info : mappings) {
		if (!seen.contains(info.keyword)) {
			result.append(info);
			seen.insert(info.keyword);
		}
	}
	return result;
}

// Utility function to parse INI files and yield (section, key, value) tuples
static void parseIniFile(QTextStream& in, std::function<void(const QString&, const QString&, const QString&)> handler) {
	QString currentSection;
	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if (line.isEmpty() || line.startsWith(';') || line.startsWith('#'))
			continue;
		QRegularExpressionMatch sectionMatch = CompiledRegex::SECTION_REGEX.match(line);
		if (sectionMatch.hasMatch()) {
			currentSection = sectionMatch.captured(1).trimmed();
			continue;
		}
		int eq = line.indexOf('=');
		if (eq <= 0)
			continue;
		QString key = line.left(eq).trimmed();
		QString value = line.mid(eq + 1).trimmed();
		handler(currentSection, key, value);
	}
}

static void ParseTooltipColorOverride(const QString& key, const QString& value)
{
	if (key.compare("_TooltipBgColor", Qt::CaseInsensitive) == 0) {
		auto parts = value.split(',');
		if (parts.size() == 2) {
			CIniHighlighter::s_tooltipBgColorDark = parts[0].trimmed();
			CIniHighlighter::s_tooltipBgColorLight = parts[1].trimmed();
		}
	}
	else if (key.compare("_TooltipTextColor", Qt::CaseInsensitive) == 0) {
		auto parts = value.split(',');
		if (parts.size() == 2) {
			CIniHighlighter::s_tooltipTextColorDark = parts[0].trimmed();
			CIniHighlighter::s_tooltipTextColorLight = parts[1].trimmed();
		}
	}
}

// Load settings from SbieSettings.ini
void CIniHighlighter::loadSettingsIni(const QString& filePath)
{
    QMutexLocker locker(&settingsMutex);

	// Invalidate cached max length on reload
	s_maxSettingNameLengthValid = false;

    QFile file(filePath);
    settingsLoaded = file.open(QIODevice::ReadOnly | QIODevice::Text);
    
    if (!settingsLoaded) {
        qWarning() << "[validSettings] Failed to load settings file:" << filePath << "Error:" << file.errorString();
        m_enableValidation = false; // Disable validation if loading fails
    }

    // Update UI based on whether settings loaded successfully
	UpdateCheckboxesOnAllTopLevels({
	QStringLiteral("chkValidateIniKeys"),
	QStringLiteral("chkEnableTooltips"),
	QStringLiteral("chkEnableAutoCompletion")
		}, [this](QCheckBox* box) {
			const QString& objName = box->objectName();
			if (settingsLoaded) {
				box->setEnabled(true);
			}
			else {
				box->setEnabled(false);
				if (objName == QLatin1String("chkValidateIniKeys")) {
					box->setText(tr("Validate (Failed)"));
				}
			}
		});

    if (!settingsLoaded) {
        return;
    }

    validSettings.clear();
    validSettings.reserve(1000);
    contextData.clear();
    categoryData.clear();
    requirementsData.clear();
    genericStyles.clear();
	s_masterVersion.clear();
	ClearLanguageCache();
	ClearHideConfCache();

    // Accumulate config lines for deduplication
    QMap<QString, QStringList> configLines;
    QMap<QString, QMap<QString, QStringList>> localizedConfigLines;

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif

    QString currentSection;
    currentSection.reserve(64);
    SettingInfo currentInfo;
    bool inSection = false;
    bool inConfigSection = false;

    // Helper lambda for config accumulation
    auto accumulateConfig = [](const QString& key, const QString& value, const QString& baseKey,
                              QMap<QString, QStringList>& configLines,
                              QMap<QString, QMap<QString, QStringList>>& localizedConfigLines,
                              std::function<QString(const QString&, const QString&)> extractLanguageCodeFn) {
        if (key.startsWith(baseKey, Qt::CaseInsensitive)) {
            if (key == baseKey) {
                configLines[baseKey].append(value);
            } else {
                QString langCode = extractLanguageCodeFn(key, baseKey);
                if (!langCode.isEmpty())
                    localizedConfigLines[baseKey][langCode].append(value);
            }
            return true;
        }
        return false;
    };

	parseIniFile(in, [&](const QString& section, const QString& key, const QString& value) {
		// Section header logic
		if (section != currentSection) {
			// Save previous section if needed
			if (inSection && !currentSection.isEmpty() && !inConfigSection) {
				currentInfo.name = currentSection;
				validSettings.insert(currentInfo.name, std::move(currentInfo));
			}
			currentSection = section;
			inConfigSection = (currentSection == "___SbieSettingsConfig_");
			inSection = !inConfigSection;
			if (inSection)
				currentInfo = SettingInfo();
		}

		// Skip lines outside any section
		if (!inSection && !inConfigSection)
			return;

		if (inConfigSection) {
			ParseTooltipColorOverride(key, value);

			if (key.compare("___Version", Qt::CaseInsensitive) == 0) {
				s_masterVersion = value;
			}
			if (accumulateConfig(key, value, "_ContextConf", configLines, localizedConfigLines, extractLanguageCode) ||
				accumulateConfig(key, value, "_CategoryConf", configLines, localizedConfigLines, extractLanguageCode) ||
				accumulateConfig(key, value, "_RequirementsConf", configLines, localizedConfigLines, extractLanguageCode)) {
				return;
			}
			if (key.compare("_HideConf", Qt::CaseInsensitive) == 0) {
				parseHideConfRules(value, hideConfRules);
				return;
			}
			if (key.compare("_HideConfExclusions", Qt::CaseInsensitive) == 0) {
				parseHideConfRules(value, hideConfExclusions);
				return;
			}
			processConfigKeyword(key, value, "_ContextConf", contextData);
			processConfigKeyword(key, value, "_CategoryConf", categoryData);
			processConfigKeyword(key, value, "_RequirementsConf", requirementsData);
			if (key.endsWith("Styles", Qt::CaseInsensitive)) {
				QString styleKey = key;
				if (styleKey.startsWith("_")) styleKey = styleKey.mid(1);
				genericStyles[styleKey] = parseStyleConfig(value);
			}
			return;
		}

		// Helper: process localized field
		auto processLocalizedField = [&](const QString& baseKey, const QString& prefix,
			QString& baseField, QMap<QString, QString>& localizedMap) {
				if (key.compare(baseKey, Qt::CaseInsensitive) == 0) {
					baseField = processTextLineOptimized(sanitizeHtmlInput(value), QString());
					return true;
				}
				if (key.startsWith(prefix, Qt::CaseInsensitive)) {
					QString langCode = extractLanguageCode(key, prefix.chopped(1));
					if (!langCode.isEmpty()) {
						QString localizedContent = processTextLineOptimized(sanitizeHtmlInput(value), QString());
						if (localizedMap.contains(langCode))
							localizedMap[langCode] += "\n" % localizedContent;
						else
							localizedMap.insert(langCode, localizedContent);
					}
					return true;
				}
				return false;
			};

		// Process known fields using the static sanitizeVersion method
		if (key.compare("AddedVersion", Qt::CaseInsensitive) == 0)
			currentInfo.addedVersion = sanitizeVersion(value, true);
		else if (key.compare("RemovedVersion", Qt::CaseInsensitive) == 0)
			currentInfo.removedVersion = sanitizeVersion(value);
		else if (key.compare("ReaddedVersion", Qt::CaseInsensitive) == 0)
			currentInfo.readdedVersion = sanitizeVersion(value);
		else if (key.compare("RenamedVersion", Qt::CaseInsensitive) == 0)
			currentInfo.renamedVersion = sanitizeVersion(value);
		else if (key.compare("SupersededBy", Qt::CaseInsensitive) == 0)
			currentInfo.supersededBy = value;
		else if (key.compare("Category", Qt::CaseInsensitive) == 0)
			currentInfo.category = value;
		else if (key.compare("Context", Qt::CaseInsensitive) == 0)
			currentInfo.context = value;
		else if (processLocalizedField("Syntax", "Syntax_", currentInfo.syntax, currentInfo.localizedSyntax)) {
		}
		else if (processLocalizedField("Description", "Description_", currentInfo.description, currentInfo.localizedDescriptions)) {
		}
		else if (key.compare("Requirements", Qt::CaseInsensitive) == 0)
			currentInfo.requirements = value/*.toLower()*/.trimmed();
		});

	// Save last section if needed
	if (inSection && !currentSection.isEmpty() && !inConfigSection) {
		currentInfo.name = currentSection;
		currentInfo.name.remove(CompiledRegex::CLEAN_NAME_REGEX);
		validSettings.insert(currentInfo.name, std::move(currentInfo));
	}

	file.close();

    // Deduplicate and set config mappings
    contextData.mappings = deduplicateKeywordMappings(parseKeywordMappings<CIniHighlighter::KeywordType::Context>(configLines["_ContextConf"].join(";")));
    for (auto it = localizedConfigLines["_ContextConf"].begin(); it != localizedConfigLines["_ContextConf"].end(); ++it) {
        contextData.localizedMappings[it.key()] = deduplicateKeywordMappings(parseKeywordMappings<CIniHighlighter::KeywordType::Context>(it.value().join(";")));
    }
    categoryData.mappings = deduplicateKeywordMappings(parseKeywordMappings<CIniHighlighter::KeywordType::Category>(configLines["_CategoryConf"].join(";")));
    for (auto it = localizedConfigLines["_CategoryConf"].begin(); it != localizedConfigLines["_CategoryConf"].end(); ++it) {
        categoryData.localizedMappings[it.key()] = deduplicateKeywordMappings(parseKeywordMappings<CIniHighlighter::KeywordType::Category>(it.value().join(";")));
    }
    requirementsData.mappings = deduplicateKeywordMappings(parseKeywordMappings<CIniHighlighter::KeywordType::Requirements>(configLines["_RequirementsConf"].join(";")));
    for (auto it = localizedConfigLines["_RequirementsConf"].begin(); it != localizedConfigLines["_RequirementsConf"].end(); ++it) {
        requirementsData.localizedMappings[it.key()] = deduplicateKeywordMappings(parseKeywordMappings<CIniHighlighter::KeywordType::Requirements>(it.value().join(";")));
    }

	QString userIniPath = QCoreApplication::applicationDirPath() % "/" % DEFAULT_SETTINGS_FILE % ".user.ini";
	applyUserIniOverrides(s_masterVersion, userIniPath);

    // Update cache status
    lastFileModified = QFileInfo(filePath).lastModified();

	qDebug() << "[validSettings] Successfully loaded"
		<< validSettings.size() << "settings,"
		<< contextData.mappings.size() << "context mappings,"
		<< contextData.localizedMappings.size() << "localized context mappings,"
		<< categoryData.mappings.size() << "category mappings,"
		<< categoryData.localizedMappings.size() << "localized category mappings,"
		<< requirementsData.mappings.size() << "requirements mappings,"
		<< requirementsData.localizedMappings.size() << "localized requirements mappings,"
		<< hideConfRules.size() << "hideConf rules,"
		<< hideConfExclusions.size() << "hideConf exclusions,"
		<< genericStyles.size() << "generic styles,"
		<< filePath;
}

void CIniHighlighter::applyUserIniOverrides(const QString& masterVersion, const QString& userIniPath)
{
	QFile userFile(userIniPath);
	if (!userFile.exists() || !userFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream userIn(&userFile);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
	userIn.setCodec("UTF-8");
#endif
	QString userVersion;
	QList<QPair<QString, QString>> userLines;
	parseIniFile(userIn, [&](const QString& section, const QString& key, const QString& value) {
		if (section != "___SbieSettingsConfig_")
			return;
		if (key.compare("___Version", Qt::CaseInsensitive) == 0) {
			userVersion = value;
		}
		userLines.append(qMakePair(key, value));
		});
	userFile.close();

	// Only process user ini if version matches
	if (!masterVersion.isEmpty() && !userVersion.isEmpty() && s_masterVersion == userVersion) {
		ClearHideConfCache();
		for (const auto& pair : userLines) {
			const QString& key = pair.first;
			const QString& value = pair.second;
			ParseTooltipColorOverride(key, value);
			if (key.endsWith("Styles", Qt::CaseInsensitive)) {
				QString styleKey = key;
				if (styleKey.startsWith("_")) styleKey = styleKey.mid(1);
				genericStyles[styleKey] = parseStyleConfig(value);
			}
			else if (key.compare("_HideConf", Qt::CaseInsensitive) == 0) {
				parseHideConfRules(value, hideConfRules);
			}
			else if (key.compare("_HideConfExclusions", Qt::CaseInsensitive) == 0) {
				parseHideConfRules(value, hideConfExclusions);
			}
		}
		qDebug() << "[validSettings] Successfully loaded user overrides from"
			<< userIniPath
			<< "with" << userLines.size() << "entries";
	}
}

template<CIniHighlighter::KeywordType Type>
CIniHighlighter::KeywordMappings<Type> CIniHighlighter::parseKeywordMappings(const QString& value)
{
	KeywordMappings<Type> mappings;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QStringList mappingStrings = value.split(';', Qt::SkipEmptyParts);
#else
	QStringList mappingStrings = value.split(';', QString::SkipEmptyParts);
#endif

	for (const QString& mapping : mappingStrings) {
		QStringList parts = mapping.split('|');
		if (parts.size() >= 2) {
			KeywordInfo<Type> info;
			info.keyword = parts[0].trimmed();
			info.displayName = parts[1].trimmed();
			if (parts.size() >= 3) {
				info.action = parts[2].trimmed();
			}
			mappings.append(info);
		}
	}

	return mappings;
}

QMap<QString, CIniHighlighter::TooltipCellStyles> CIniHighlighter::genericStyles;

CIniHighlighter::TooltipCellStyles CIniHighlighter::getGenericStyles(const QString& rowType)
{
    QString key = rowType % "Styles";
    if (genericStyles.contains(key)) {
        return genericStyles.value(key);
    }
    return TooltipCellStyles();
}

CIniHighlighter::TooltipCellStyles CIniHighlighter::parseStyleConfig(const QString& styleConfig)
{
    TooltipCellStyles cellStyles;

    if (styleConfig.isEmpty()) {
        return cellStyles; // Return default (empty) styles
    }

    // Split into left and right style definitions
    QStringList cellParts = styleConfig.split(';');
    QString leftStyleStr = cellParts.value(0).trimmed();
    QString rightStyleStr = cellParts.size() > 1 ? cellParts.value(1).trimmed() : leftStyleStr;

    auto parseSingleStyle = [](const QString& styleStr) -> TooltipStyle {
        TooltipStyle style;
        if (styleStr.isEmpty()) {
            return style;
        }
        QStringList parts = styleStr.split('|');
        // Parse color (first part)
        if (parts.size() >= 1 && !parts[0].trimmed().isEmpty()) {
            style.color = parts[0].trimmed().toLower();
        }
        // Parse format requirements (second part)
        if (parts.size() >= 2 && !parts[1].trimmed().isEmpty()) {
            QString formatStr = parts[1].trimmed().toLower();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QStringList formats = formatStr.split(',', Qt::SkipEmptyParts);
#else
            QStringList formats = formatStr.split(',', QString::SkipEmptyParts);
#endif
            for (const QString& format : formats) {
                QString fmt = format.trimmed();
                if (fmt == "bold") {
                    style.bold = true;
                }
                else if (fmt == "italic") {
                    style.italic = true;
                }
                else if (fmt == "underline") {
                    style.underline = true;
                }
				else if (fmt == "center" || fmt == "left" || fmt == "right") {
					style.alignment = fmt;
				}
            }
        }
        // Parse font (third part) - currently not used in TooltipStyle but could be extended
        // if (parts.size() >= 3 && !parts[2].trimmed().isEmpty()) {
        //     style.fontFamily = parts[2].trimmed();
        // }
        return style;
    };

    cellStyles.left = parseSingleStyle(leftStyleStr);
    cellStyles.right = parseSingleStyle(rightStyleStr);
    return cellStyles;
}

void CIniHighlighter::addVersionRows(QString& tooltip, const SettingInfo& info, const QString& labelStyle)
{
    const QString addedVersionLabel = tr("Added in version");
    const QString removedVersionLabel = tr("Removed in version");
    const QString readdedVersionLabel = tr("Re-added in version");
    const QString renamedVersionLabel = tr("Renamed in version");
    const QString supersededByLabel = tr("Superseded by");

    struct RowInfo { QString label; QString value; QString styleKey; };
    QList<RowInfo> rows = {
        {addedVersionLabel, info.addedVersion, "AddedVersion"},
        {removedVersionLabel, info.removedVersion, "RemovedVersion"},
        {readdedVersionLabel, info.readdedVersion, "ReAddedVersion"},
        {renamedVersionLabel, info.renamedVersion, "RenamedVersion"},
        {supersededByLabel, info.supersededBy, "SupersededBy"}
    };
    
    // Use the generic processContainer template to iterate
    processContainer(rows, [&](const RowInfo& row) {
        if (!row.value.isEmpty() && (row.styleKey != "AddedVersion" || row.value != DEFAULT_VERSION)) {
            TooltipCellStyles styles = getGenericStyles(row.styleKey);
            appendGenericTooltipRow(tooltip, row.label, row.value, labelStyle, HtmlTags::VALUE_PREFIX, styles);
        }
    });
}

void CIniHighlighter::appendGenericTooltipRow(QString& tooltip, const QString& label, const QString& value,
	const QString& labelStyle, const QString& valuePrefix, const TooltipCellStyles& cellStyles)
{
	if (value.isEmpty()) return;
	
	QString styledLabelStyle = mergeHtmlStyles(labelStyle, cellStyles.left.toHtmlStyle());

	QString valueCellTag = cellStyles.right.toHtmlStyle().isEmpty() ? 
		HtmlTags::TD_TAG : 
		HtmlTags::TD_START % cellStyles.right.toHtmlStyle() % HtmlTags::TAG_CLOSE;
	
	tooltip += HtmlTags::TR_TD_START % styledLabelStyle % HtmlTags::TAG_CLOSE
		% label % HtmlTags::TD_END
		% valueCellTag % valuePrefix % value % HtmlTags::TD_END % HtmlTags::TR_END;
}

void CIniHighlighter::appendMultiLineTooltipRow(QString& tooltip, const QString& label, 
	const QStringList& lines, const QString& labelStyle, const QString& valuePrefix,
	const QString& settingName, bool applySpecialFormatting)
{
	if (lines.isEmpty()) return;
	
	tooltip += HtmlTags::TR_TD_START % labelStyle % HtmlAttribs::STYLE_TOP 
		% HtmlTags::TAG_CLOSE % label % HtmlTags::TD_END % HtmlTags::TD_TAG;
	
	for (int i = 0; i < lines.size(); ++i) {
		QString processedLine = processTextLineOptimized(lines[i], settingName);

		// Syntax highlighting for special characters
		if (applySpecialFormatting) { // Syntax highlighting
			//// Bracket/parenthesis color definitions (customize as needed)
			//const QString colorSquare = QStringLiteral("#2196F3");   // blue for [ ]
			//const QString colorPipe = QStringLiteral("#2196F3");   // blue for |
			//const QString colorParen = QStringLiteral("#43A047");   // green for ( )
			//const QString colorCurly = QStringLiteral("#E65100");   // orange for { }
			//const QString colorAngle = QStringLiteral("#9C27B0");   // purple for < >
			//const QString colorComma = QStringLiteral("#D32F2F");   // red for ,
			//const QString colorSemi = QStringLiteral("#D32F2F");   // red for ;
			//const QString colorEq = QStringLiteral("#1976D2");   // deep blue for =

			//// Step 1: Replace with placeholders
			//processedLine.replace(QStringLiteral("["), QStringLiteral("__BRACKET_L__"));
			//processedLine.replace(QStringLiteral("]"), QStringLiteral("__BRACKET_R__"));
			//processedLine.replace(QStringLiteral("("), QStringLiteral("__PAREN_L__"));
			//processedLine.replace(QStringLiteral(")"), QStringLiteral("__PAREN_R__"));
			//processedLine.replace(QStringLiteral("{"), QStringLiteral("__CURLY_L__"));
			//processedLine.replace(QStringLiteral("}"), QStringLiteral("__CURLY_R__"));
			//processedLine.replace(QStringLiteral("|"), QStringLiteral("__PIPE__"));
			//processedLine.replace(QStringLiteral("&lt;"), QStringLiteral("__ANGLE_L__"));
			//processedLine.replace(QStringLiteral("&gt;"), QStringLiteral("__ANGLE_R__"));
			//processedLine.replace(QStringLiteral(","), QStringLiteral("__COMMA__"));
			//processedLine.replace(QStringLiteral("="), QStringLiteral("__EQUAL__"));
			//// Replace only semicolons not part of an HTML entity
			//processedLine.replace(QRegularExpression(R"((?<!&[a-zA-Z0-9]{1,10});)"), QStringLiteral("__SEMICOLON__"));

			//// Step 2: Replace placeholders with HTML
			//processedLine.replace(QStringLiteral("__BRACKET_L__"), QStringLiteral("<span style='color:%1;'>[</span>").arg(colorSquare));
			//processedLine.replace(QStringLiteral("__BRACKET_R__"), QStringLiteral("<span style='color:%1;'>]</span>").arg(colorSquare));
			//processedLine.replace(QStringLiteral("__PAREN_L__"), QStringLiteral("<span style='color:%1;'>(</span>").arg(colorParen));
			//processedLine.replace(QStringLiteral("__PAREN_R__"), QStringLiteral("<span style='color:%1;'>)</span>").arg(colorParen));
			//processedLine.replace(QStringLiteral("__CURLY_L__"), QStringLiteral("<span style='color:%1;'>{</span>").arg(colorCurly));
			//processedLine.replace(QStringLiteral("__CURLY_R__"), QStringLiteral("<span style='color:%1;'>}</span>").arg(colorCurly));
			//processedLine.replace(QStringLiteral("__PIPE__"), QStringLiteral("<span style='color:%1;'>|</span>").arg(colorPipe));
			//processedLine.replace(QStringLiteral("__ANGLE_L__"), QStringLiteral("<span style='color:%1;'>&lt;</span>").arg(colorAngle));
			//processedLine.replace(QStringLiteral("__ANGLE_R__"), QStringLiteral("<span style='color:%1;'>&gt;</span>").arg(colorAngle));
			//processedLine.replace(QStringLiteral("__COMMA__"), QStringLiteral("<span style='color:%1;'>,</span>").arg(colorComma));
			//processedLine.replace(QStringLiteral("__SEMICOLON__"), QStringLiteral("<span style='color:%1;'>;</span>").arg(colorSemi));
			//processedLine.replace(QStringLiteral("__EQUAL__"), QStringLiteral("<span style='color:%1;'>=</span>").arg(colorEq));

			processedLine = HtmlTags::SPAN_FONT_CONSOLAS % processedLine % HtmlTags::SPAN_END;
		}
		
		if (i == 0) {
			tooltip += valuePrefix % processedLine;
		} else {
			tooltip += HtmlTags::BR_NBSP % processedLine;
		}
	}
	
	tooltip += HtmlTags::TD_END % HtmlTags::TR_END;
}

// Helper to generate a table row for syntax or description
void CIniHighlighter::appendTableRowForContent(QString& tooltip, const QString& label, const QString& content, const QString& labelStyle, const QString& valuePrefix, const QString& settingName, bool isSyntax)
{
    if (content.isEmpty())
        return;
        
    QStringList lines = content.split('\n');
    // Use genericStyles for label and value cell if defined
    QString rowType = isSyntax ? "Syntax" : "Description";
    TooltipCellStyles styles = getGenericStyles(rowType);
    
    QString styledLabelStyle = mergeHtmlStyles(labelStyle, styles.left.toHtmlStyle());
    
    // Use the unified multi-line tooltip row method
    appendMultiLineTooltipRow(tooltip, label, lines, styledLabelStyle, valuePrefix, settingName, isSyntax);
}

QString CIniHighlighter::processTextLineOptimized(const QString& text, const QString& settingName)
{
	QString processed = text;

	// Process escape sequences and placeholders
	if (processed.contains(QLatin1Char('\\'))){
		// First handle the \s{n} pattern (replace with n &nbsp;)
		static const QRegularExpression spaceCountRegex(
			QStringLiteral(R"(\\s\{(\d+)\})"),
			QRegularExpression::CaseInsensitiveOption
		);

		int pos = 0;
		QRegularExpressionMatchIterator it = spaceCountRegex.globalMatch(processed);
		QString result;
		result.reserve(processed.length() + 100); // Pre-allocate to avoid reallocations

		while (it.hasNext()) {
			QRegularExpressionMatch match = it.next();
			// Append text before the match
			result += processed.mid(pos, match.capturedStart() - pos);
			pos = match.capturedEnd();

			int count = match.captured(1).toInt();
			//QString spaces(count, QChar(0xA0)); // Fill with &nbsp;
			QString spaces = TextReplacements::SPACE.repeated(count); // Fill with &nbsp;
			result += spaces;
		}
		// Append remaining text after last match
		result += processed.mid(pos);
		processed = result;

		// Replace literal backslash escape with a placeholder
		processed.replace(TextReplacements::ESCAPE_BACKSLASH, QStringLiteral("__ESCAPE_BACKSLASH__"));

		// Replace other escape sequences
		processed.replace(TextReplacements::ESCAPE_N, TextReplacements::NEWLINE)
			.replace(TextReplacements::ESCAPE_T, TextReplacements::TAB_NBSP)
			.replace(TextReplacements::ESCAPE_R, TextReplacements::CARRIAGE_RETURN)
			.replace(TextReplacements::ESCAPE_S, TextReplacements::SPACE);

		// Restore literal backslash escape
		processed.replace(QStringLiteral("__ESCAPE_BACKSLASH__"), TextReplacements::ESCAPE_BACKSLASH);
	}

	// Process other placeholders
	if (processed.contains(QLatin1Char('['))) {
		processed.replace(QStringLiteral("[br]"), TextReplacements::NEWLINE)
			.replace(QStringLiteral("[sbie]"), QStringLiteral("Sandboxie-Plus"))
			.replace(QStringLiteral("[bY]"), QStringLiteral("([b]Y[/b]|n)"))
			.replace(QStringLiteral("[bN]"), QStringLiteral("(y|[b]N[/b])"));

		if (processed.contains(QStringLiteral("[version]"))) {
			static const QString versionStr = QStringLiteral("%1.%2.%3").arg(VERSION_MJR).arg(VERSION_MIN).arg(VERSION_REV);
			processed.replace(QStringLiteral("[version]"), versionStr);
		}

		// Convert square bracket styling tags to HTML
		processed.replace(QStringLiteral("[b]"), QStringLiteral("<b>"))
			.replace(QStringLiteral("[/b]"), QStringLiteral("</b>"))
			.replace(QStringLiteral("[i]"), QStringLiteral("<i>"))
			.replace(QStringLiteral("[/i]"), QStringLiteral("</i>"))
			.replace(QStringLiteral("[u]"), QStringLiteral("<u>"))
			.replace(QStringLiteral("[/u]"), QStringLiteral("</u>"))
			.replace(QStringLiteral("[code]"), QStringLiteral("<code style='font-family: Consolas, monospace; background-color: #ececec; color: #222222; padding: 1px 3px;'>"))
			.replace(QStringLiteral("[/code]"), QStringLiteral("</code>"));
	}

	if (!settingName.isEmpty() && processed.contains(QStringLiteral("[sn]"))) {
		processed.replace(QStringLiteral("[sn]"), settingName);
	}

	// Dynamic color tags: [color<Color>]text[/color] -> <span style='color:<Color>;'>text</span>
	// Accept safe color tokens (named colors, #hex, rgb()/rgba()/hsl()/hsla())
	if (processed.contains(QStringLiteral("[color"))) {
		static const QRegularExpression colorBlockRegex(
			R"(\[color([A-Za-z0-9#(),.%\s]{1,32})\](.*?)\[/color\])",
			QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
		static const QRegularExpression validColorTokenRegex(
			R"(^(?:[A-Za-z]{1,32}|#[0-9A-Fa-f]{3}(?:[0-9A-Fa-f]{3})?|rgb\(\s*\d{1,3}\s*,\s*\d{1,3}\s*,\s*\d{1,3}\s*\)|rgba\(\s*\d{1,3}\s*,\s*\d{1,3}\s*,\s*\d{1,3}\s*,\s*(?:0|1|0?\.\d+)\s*\)|hsl\(\s*\d{1,3}\s*,\s*\d{1,3}%\s*,\s*\d{1,3}%\s*\)|hsla\(\s*\d{1,3}\s*,\s*\d{1,3}%\s*,\s*\d{1,3}%\s*,\s*(?:0|1|0?\.\d+)\s*\))$)");

		QString result;
		result.reserve(processed.size() + 64);
		int lastIndex = 0;

		auto it = colorBlockRegex.globalMatch(processed);
		while (it.hasNext()) {
			QRegularExpressionMatch m = it.next();
			const int start = m.capturedStart();
			const int end = m.capturedEnd();
			const QString colorToken = m.captured(1).trimmed();
			const QString innerText = m.captured(2);

			// Append text before match (replace midRef with mid for Qt6 compatibility)
			result += processed.mid(lastIndex, start - lastIndex);

			// Validate color token strictly
			if (validColorTokenRegex.match(colorToken).hasMatch()) {
				result += HtmlTags::SPAN_COLOR_START % colorToken % HtmlTags::SPAN_COLOR_CLOSE % innerText % HtmlTags::SPAN_END;
			}
			else {
				// Fallback: keep original source
				result += processed.mid(start, end - start);
			}

			lastIndex = end;
		}
		// Append remaining part
		result += processed.mid(lastIndex);
		processed = std::move(result);
	}

	return processed;
}

QString CIniHighlighter::selectLocalizedContentOptimized(const QString& defaultContent,
	const QMap<QString, QString>& localizedMap,
	const QString& currentLang)
{
	if (currentLang.isEmpty() || localizedMap.isEmpty()) {
		return defaultContent;
	}

	// Use constFind for better performance
	auto it = localizedMap.constFind(currentLang);
	if (it != localizedMap.constEnd()) {
		return it.value();
	}

	// Try with language base only if underscore is present
	if (currentLang.contains(QLatin1Char('_'))) {
		const int underscorePos = currentLang.indexOf(QLatin1Char('_'));
		const QString langBase = currentLang.left(underscorePos);

		it = localizedMap.constFind(langBase);
		if (it != localizedMap.constEnd()) {
			return it.value();
		}
	}

	return defaultContent;
}

template<CIniHighlighter::KeywordType Type>
CIniHighlighter::KeywordMappings<Type> CIniHighlighter::getEffectiveMappingsWithActionFallback(
	const KeywordGroup<Type>& data, const QString& currentLang)
{
	auto defaultMappings = data.mappings;

	// If no language is set or no localized mappings exist, use default
	if (currentLang.isEmpty() || data.localizedMappings.isEmpty()) {
		return defaultMappings;
	}

	// Try to find localized mappings for current language
	auto localizedMappings = [&]() -> KeywordMappings<Type> {
		// First try exact match for language-specific mappings
		if (data.localizedMappings.contains(currentLang)) {
			return data.localizedMappings.value(currentLang);
		}

		// Try with just the language part (if it's a locale with country code)
		if (currentLang.contains('_')) {
			QString langBase = currentLang.left(currentLang.indexOf('_'));
			if (data.localizedMappings.contains(langBase)) {
				return data.localizedMappings.value(langBase);
			}
		}

		// No localized version found, return empty
		return KeywordMappings<Type>{};
	}();

	// If no localized mappings found, use default
	if (localizedMappings.isEmpty()) {
		return defaultMappings;
	}

	// Create a merged mapping that prioritizes default actions but uses localized display names
	auto mergedMappings = defaultMappings; // Start with default mappings as base

	// Create lookup maps for efficient comparison
	QHash<QString, QString> defaultActions;
	QHash<QString, QString> defaultDisplayNames;
	for (const auto& item : defaultMappings) {
		defaultActions.insert(item.keyword, item.action);
		defaultDisplayNames.insert(item.keyword, item.displayName);
	}

	QHash<QString, QString> localizedActions;
	QHash<QString, QString> localizedDisplayNames;
	for (const auto& item : localizedMappings) {
		localizedActions.insert(item.keyword, item.action);
		localizedDisplayNames.insert(item.keyword, item.displayName);
	}

	// Check if localized version has missing keywords or different actions
	bool useDefaultActions = false;

	// Check 1: Compare sizes - if localized has fewer keywords, use default actions
	if (localizedMappings.size() < defaultMappings.size()) {
		useDefaultActions = true;
	}

	// Check 2: Look for action differences in common keywords
	if (!useDefaultActions) {
		for (const auto& defaultItem : defaultMappings) {
			if (localizedActions.contains(defaultItem.keyword)) {
				// If actions differ between default and localized, use default actions
				if (localizedActions.value(defaultItem.keyword) != defaultItem.action) {
					useDefaultActions = true;
					break;
				}
			}
		}
	}

	// Check 3: Look for missing keywords in localized version
	if (!useDefaultActions) {
		for (const auto& defaultItem : defaultMappings) {
			if (!localizedActions.contains(defaultItem.keyword)) {
				useDefaultActions = true;
				break;
			}
		}
	}

	if (useDefaultActions) {
		// Use default actions but try to use localized display names where available
		for (auto& item : mergedMappings) {
			if (localizedDisplayNames.contains(item.keyword)) {
				item.displayName = localizedDisplayNames.value(item.keyword);
			}
			// Keep default action (item.action remains unchanged)
		}
	}
	else {
		// Localized version is complete and consistent, use it as-is
		mergedMappings = localizedMappings;
	}

	return mergedMappings;
}

template<typename ContainerType, typename FunctionType>
void CIniHighlighter::processContainer(const ContainerType& container, FunctionType&& func)
{
	for (const auto& item : container) {
		func(item);
	}
}

void CIniHighlighter::highlightBlock(const QString &text)
{
    // First, reset all formatting
    setFormat(0, text.length(), QTextCharFormat());

    // 1. Check if the entire line is a comment - use compiled regex
    QRegularExpressionMatch commentMatch = CompiledRegex::COMMENT_REGEX.match(text);
    if (commentMatch.hasMatch()) {
        setFormat(0, text.length(), commentFormat);
        return; // Skip other rules
    }

    // 2. Apply INI highlighting rules (section, key, equals, value)
    for (const HighlightRule &rule : qAsConst(highlightRules)) {
        // Skip the comment rule as it's already handled
        if (rule.format.foreground() == commentFormat.foreground())
            continue;

        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            int start = match.capturedStart();
            int length = match.capturedLength();
            setFormat(start, length, rule.format);
        }
    }

	// 3. Highlight keys based on validSettings and currentVersion
	if (m_enableValidation) {
		QRegularExpressionMatch keyMatch = CompiledRegex::KEY_REGEX.match(text);
		if (keyMatch.hasMatch()) {
			QString keyName = keyMatch.captured(1);
			int start = keyMatch.capturedStart(1);
			int length = keyName.length();

			if (validSettings.isEmpty() || !validSettings.contains(keyName)) {
				setFormat(start, length, unknownKeyFormat); // underline unknown keys
			}
			else {
				const SettingInfo& info = validSettings[keyName];
				QTextCharFormat formatToUse = determineKeyFormat(info, m_currentVersion,
					keyFormat, futureKeyFormat, removedKeyFormat, renamedKeyFormat, unknownKeyFormat);
				setFormat(start, length, formatToUse);
			}
		}
	}

	// 4. Process the value part for value prefixes and first comma
    // Find the position of '=' to identify the start of the value
    int equalsIndex = text.indexOf('=');
    if (equalsIndex != -1) {
        // Start position of the value (after '=')
        int valueStart = equalsIndex + 1;
        QString valueText = text.mid(valueStart).trimmed();

        // Iterate through the value to find the first comma outside of {}
        int braceLevel = 0;
        int firstRelevantCommaIndex = -1;
        for (int i = 0; i < valueText.length(); ++i) {
            QChar currentChar = valueText[i];
            if (currentChar == '{') {
                braceLevel++;
            } else if (currentChar == '}') {
                if (braceLevel > 0)
                    braceLevel--;
            } else if (currentChar == ',' && braceLevel == 0) {
                firstRelevantCommaIndex = i;
                break;
            }
        }

        if (firstRelevantCommaIndex != -1) {
            // Position of the first comma relative to the entire line
            int commaPos = valueStart + firstRelevantCommaIndex;

            // Highlight text before the first comma in blue
            if (firstRelevantCommaIndex > 0) {
                setFormat(valueStart, firstRelevantCommaIndex, valuePrefixFormat);
            }

            // Highlight the first comma in red
            setFormat(commaPos, 1, firstCommaFormat);
        }

#ifdef INI_WITH_JSON
        bool inString = false;
        int stringStart = -1;

        for (int i = firstRelevantCommaIndex != -1 ? firstRelevantCommaIndex + 1 : 0; i < valueText.length(); ++i) {
            QChar currentChar = valueText[i];

            if (currentChar == '\"') {
                // Check if the quote is escaped
                bool escaped = false;
                int backslashCount = 0;
                int j = i - 1;
                while (j >= 0 && valueText[j] == '\\') {
                    backslashCount++;
                    j--;
                }
                if (backslashCount % 2 == 1)
                    escaped = true;

                if (!escaped) {
                    if (!inString) {
                        inString = true;
                        stringStart = valueStart + i;
                    }
                   	else {
                        inString = false;
                        // Apply string formatting from stringStart to current position
                        int length = (valueStart + i + 1) - stringStart;
                        setFormat(stringStart, length, jsonStringFormat);
                    }
                }
            }

            // Apply colon and comma formatting only if not inside a string
            if (!inString) {
                if (currentChar == ':') {
                    setFormat(valueStart + i, 1, jsonColonFormat);
                }
                else if (currentChar == ',') {
                    setFormat(valueStart + i, 1, jsonCommaFormat);
                }
            }
        }

        // If still inside a string (unclosed), format till end
        if (inString && stringStart != -1) {
            int length = text.length() - stringStart;
            setFormat(stringStart, length, jsonStringFormat);
        }

        // 4. Apply JSON Key Formatting within JSON Substrings
        // Find all JSON substrings and apply key formatting
        int current = 0;
        while (current < valueText.length()) {
            int startBrace = valueText.indexOf('{', current);
            if (startBrace == -1)
                break;
            int braceCounter = 1;
            int endBrace = startBrace + 1;
            while (endBrace < valueText.length() && braceCounter > 0) {
                if (valueText[endBrace] == '{')
                    braceCounter++;
                else if (valueText[endBrace] == '}')
                    braceCounter--;
                endBrace++;
            }
            if (braceCounter == 0) {
                // Found a JSON substring from startBrace to endBrace-1
                QString jsonString = valueText.mid(startBrace, endBrace - startBrace);
                QRegularExpression keyRegex(R"("(?:(?:\\.)|[^"\\])*"(?=\s*:))");
                QRegularExpressionMatchIterator keyMatches = keyRegex.globalMatch(jsonString);
                while (keyMatches.hasNext()) {
                    QRegularExpressionMatch keyMatch = keyMatches.next();
                    int keyStart = valueStart + startBrace + keyMatch.capturedStart();
                    int keyLength = keyMatch.capturedLength();
                    setFormat(keyStart, keyLength, jsonKeyFormat);
                }
                current = endBrace;
            }
            else {
                break;
            }
        }
#endif
    }
}

template<typename KeywordInfoType>
void CIniHighlighter::processKeywordMappings(
    const QString& displayText,
    const QList<KeywordInfoType>& effectiveMappings,
    const QString& labelText,
    const QString& labelStyle,
    const QString& valuePrefix,
    const TooltipCellStyles& cellStyles,
    QString& tooltip)
{
	QStringList typeLabels = getVisibleLabelsWithActionHiding(displayText, effectiveMappings);

    // Only display if we have labels to show
    if (!typeLabels.isEmpty()) {
        QString typeText = typeLabels.join(" + ");

        // Apply styling to label if specified
		QString styledLabelStyle = mergeHtmlStyles(labelStyle, cellStyles.left.toHtmlStyle());

        tooltip += HtmlTags::TR_TD_START % styledLabelStyle % HtmlTags::TAG_CLOSE % labelText % HtmlTags::TD_END;

        // Apply styling to value text
        QString valueStyleStr = cellStyles.right.toHtmlStyle();
        if (!valueStyleStr.isEmpty()) {
            tooltip += HtmlTags::TD_START % valueStyleStr % HtmlTags::TAG_CLOSE % valuePrefix % typeText % HtmlTags::TD_TR_END;
        }
        else {
            tooltip += HtmlTags::TD_TAG % valuePrefix % typeText % HtmlTags::TD_TR_END;
        }
    }
}

template<typename KeywordInfoType>
QStringList getVisibleLabelsWithActionHiding(
	const QString& displayText,
	const QList<KeywordInfoType>& effectiveMappings)
{
	QStringList typeLabels;
	QList<QPair<QString, QString>> matchedKeywords;
	QHash<QString, QString> keywordActions;

	for (const KeywordInfoType& keywordInfo : effectiveMappings) {
		if (displayText.contains(keywordInfo.keyword)) {
			matchedKeywords.append(qMakePair(keywordInfo.keyword, keywordInfo.displayName));
			if (!keywordInfo.action.isEmpty()) {
				keywordActions.insert(keywordInfo.keyword, keywordInfo.action);
			}
		}
	}

	for (const auto& pair : matchedKeywords) {
		const QString& keyword = pair.first;
		const QString& displayName = pair.second;
		bool isHidden = false;

		if (keywordActions.contains(keyword) && keywordActions[keyword].contains(keyword)) {
			isHidden = true;
		}
		if (!isHidden) {
			for (auto it = keywordActions.constBegin(); it != keywordActions.constEnd(); ++it) {
				if (it.key() == keyword)
					continue;
				for (const QChar& c : it.value()) {
					if (QString(c) == keyword) {
						isHidden = true;
						break;
					}
				}
				if (isHidden)
					break;
			}
		}
		if (!isHidden && !displayName.isEmpty()) {
			typeLabels.append(displayName);
		}
	}
	return typeLabels;
}

void CIniHighlighter::processMappingsOptimized(QString& tooltip, const SettingInfo& info,
	const QString& currentLang, const QString& labelStyle)
{
	const QString categoryLabel = tr("Category");
	const QString contextLabel = tr("Context");
	const QString requirementsLabel = tr("Requirements");

	// Process category mappings with action fallback
	if (!info.category.isEmpty()) {
		auto effectiveMappings = getEffectiveMappingsWithActionFallback(categoryData, currentLang);
		processKeywordMappings<KeywordInfo<KeywordType::Category>>(
			info.category, effectiveMappings, categoryLabel,
			labelStyle, HtmlTags::VALUE_PREFIX, categoryData.tooltipStyle, tooltip);
	}

	// Process context mappings with action fallback
	if (!info.context.isEmpty()) {
		auto effectiveMappings = getEffectiveMappingsWithActionFallback(contextData, currentLang);
		processKeywordMappings<KeywordInfo<KeywordType::Context>>(
			info.context, effectiveMappings, contextLabel,
			labelStyle, HtmlTags::VALUE_PREFIX, contextData.tooltipStyle, tooltip);
	}

	// Process requirements mappings with action fallback
	if (!info.requirements.isEmpty()) {
		// 1. Extract <...> substrings as literal labels
		QString req = info.requirements;
		QStringList literalLabels;
		static const QRegularExpression angleBracketRegex(R"(<([^>]+)>)");
		int offset = 0;
		QRegularExpressionMatch match;
		while ((match = angleBracketRegex.match(req, offset)).hasMatch()) {
			literalLabels << match.captured(1);
			// Remove the matched substring from req
			req.remove(match.capturedStart(), match.capturedLength());
			offset = match.capturedStart(); // Continue from where we removed
		}
		req = req.trimmed();

		// 2. Map the remaining flags to labels as before
		auto effectiveMappings = getEffectiveMappingsWithActionFallback(requirementsData, currentLang);
		QStringList typeLabels = getVisibleLabelsWithActionHiding<KeywordInfo<KeywordType::Requirements>>(req, effectiveMappings);
		

		// 3. Append literal labels
		typeLabels.append(literalLabels);

		// 4. Only display if we have labels to show
		if (!typeLabels.isEmpty()) {
			QString typeText = typeLabels.join(TextReplacements::LABEL_JOINER);

			QString styledLabelStyle = mergeHtmlStyles(labelStyle, requirementsData.tooltipStyle.left.toHtmlStyle());

			tooltip += HtmlTags::TR_TD_START % styledLabelStyle % HtmlTags::TAG_CLOSE % requirementsLabel % HtmlTags::TD_END;

			QString valueStyleStr = requirementsData.tooltipStyle.right.toHtmlStyle();
			if (!valueStyleStr.isEmpty()) {
				tooltip += HtmlTags::TD_START % valueStyleStr % HtmlTags::TAG_CLOSE % HtmlTags::VALUE_PREFIX % typeText % HtmlTags::TD_END % HtmlTags::TR_END;
			}
			else {
				tooltip += HtmlTags::TD_TAG % HtmlTags::VALUE_PREFIX % typeText % HtmlTags::TD_END % HtmlTags::TR_END;
			}
		}
	}
}

QTextCharFormat CIniHighlighter::determineKeyFormat(const SettingInfo& info, const QVersionNumber& currentVersion,
	const QTextCharFormat& keyFormat, const QTextCharFormat& futureKeyFormat, 
	const QTextCharFormat& removedKeyFormat, const QTextCharFormat& renamedKeyFormat,
	const QTextCharFormat& unknownKeyFormat)
{
	// Helper function to compare QVersionNumber objects (returns -1, 0, or 1)
	auto compareVersions = [](const QVersionNumber& v1, const QVersionNumber& v2) -> int {
		if (v1 < v2) return -1;
		if (v1 > v2) return 1;
		return 0;
	};

	// Define event structure with priority (higher priority = more recent action)
	struct VersionEvent {
		QVersionNumber version;
		QString type;
		int priority;
		QTextCharFormat format;
	};

	// Collect all version events with their respective formats and priorities
	QList<VersionEvent> events;
	
	if (!info.addedVersion.isEmpty()) {
		QVersionNumber addedVer = QVersionNumber::fromString(info.addedVersion);
		if (!addedVer.isNull()) {
			events.append({addedVer, "added", 0, keyFormat});
		}
	}
	
	if (!info.removedVersion.isEmpty()) {
		QVersionNumber removedVer = QVersionNumber::fromString(info.removedVersion);
		if (!removedVer.isNull()) {
			events.append({removedVer, "removed", 1, removedKeyFormat});
		}
	}
	
	if (!info.readdedVersion.isEmpty()) {
		QVersionNumber readdedVer = QVersionNumber::fromString(info.readdedVersion);
		if (!readdedVer.isNull()) {
			events.append({readdedVer, "readded", 2, keyFormat});
		}
	}
	
	if (!info.renamedVersion.isEmpty()) {
		QVersionNumber renamedVer = QVersionNumber::fromString(info.renamedVersion);
		if (!renamedVer.isNull()) {
			events.append({renamedVer, "renamed", 3, renamedKeyFormat});
		}
	}

	// Filter events that occurred at or before currentVersion
	QList<VersionEvent> pastEvents;
	for (const VersionEvent& event : events) {
		if (compareVersions(event.version, currentVersion) <= 0) {
			pastEvents.append(event);
		}
	}

	// Find the latest event (highest version, then highest priority for same version)
	if (!pastEvents.isEmpty()) {
		VersionEvent latestEvent = pastEvents[0];
		for (int i = 1; i < pastEvents.size(); ++i) {
			const VersionEvent& event = pastEvents[i];
			int versionCompare = compareVersions(event.version, latestEvent.version);
			
			// Use this event if it's from a later version, or same version but higher priority
			if (versionCompare > 0 || (versionCompare == 0 && event.priority > latestEvent.priority)) {
				latestEvent = event;
			}
		}

		// Return format based on the latest event
		return latestEvent.format;
	}

	// Handle keys not yet introduced/reintroduced (future features)
	if (!info.addedVersion.isEmpty()) {
		QVersionNumber addedVer = QVersionNumber::fromString(info.addedVersion);
		if (!addedVer.isNull() && compareVersions(currentVersion, addedVer) < 0) {
			return futureKeyFormat;
		}
	}
	
	if (!info.readdedVersion.isEmpty()) {
		QVersionNumber readdedVer = QVersionNumber::fromString(info.readdedVersion);
		if (!readdedVer.isNull() && compareVersions(currentVersion, readdedVer) < 0) {
			return futureKeyFormat;
		}
	}

	// If no events apply, this is likely an unknown/undocumented key
	return unknownKeyFormat;
}

void CIniHighlighter::processContentOptimized(QString& tooltip, const SettingInfo& info,
	const QString& currentLang, const QString& settingName,
	const QString& labelStyle)
{
	const QString syntaxLabel = tr("Syntax");
	const QString descriptionLabel = tr("Description");

	// Process syntax
	QString syntax = selectLocalizedContentOptimized(info.syntax, info.localizedSyntax, currentLang);
	appendTableRowForContent(tooltip, syntaxLabel, syntax, labelStyle, HtmlTags::VALUE_PREFIX, settingName, true);

	// Process description
	QString description = selectLocalizedContentOptimized(info.description, info.localizedDescriptions, currentLang);
	appendTableRowForContent(tooltip, descriptionLabel, description, labelStyle, HtmlTags::VALUE_PREFIX, settingName, false);
}

void CIniHighlighter::SetTooltipMode(int checkState)
{
	QMutexLocker locker(&s_tooltipModeMutex);

	switch (checkState) {
	case Qt::Unchecked:
		s_tooltipMode = TooltipMode::Disabled;
		break;
	case Qt::PartiallyChecked:
		s_tooltipMode = TooltipMode::BasicInfo;
		break;
	case Qt::Checked:
	default:
		s_tooltipMode = TooltipMode::FullTooltip;
		break;
	}
}

void CIniHighlighter::ClearLanguageCache()
{
	// Clear the tooltip cache since it contains language-specific content
	{
		QMutexLocker cacheLock(&tooltipCacheMutex);
		tooltipCache.clear();
	}
}

void CIniHighlighter::ClearHideConfCache()
{
	// Clear hide configuration cache
	{
		QMutexLocker hideConfLock(&hideConfMutex);
		hideConfRules.clear();
		hideConfExclusions.clear();
	}
}

QString CIniHighlighter::sanitizeHtmlInput(const QString& input)
{
	QString sanitized = input;

	// 1. Escape ALL < and > characters for security - no exceptions
	sanitized.replace("<", "&lt;");
	sanitized.replace(">", "&gt;");

	// 2. That's it! No restoration of any HTML tags.
	// All HTML formatting will come from square bracket tags processed later
	// in processTextLineOptimized()

	return sanitized;
}

bool CIniHighlighter::IsCommentLine(const QString& line)
{
	// Skip lines that start with # or ; (after optional whitespace)
	QString trimmed = line.trimmed();
	return trimmed.startsWith('#') || trimmed.startsWith(';');
}

CIniHighlighter::TooltipMode CIniHighlighter::GetTooltipMode()
{
	QMutexLocker locker(&s_tooltipModeMutex);
	return s_tooltipMode;
}

bool CIniHighlighter::isValidForTooltip(const QString& settingName)
{
	// Add input validation with compiled regex for better performance
	if (settingName.isEmpty() || settingName.length() > getMaxSettingNameLengthOrDefault()) {
		return false;
	}
	
	// Validate setting name format
	if (!CompiledRegex::SETTING_NAME_REGEX.match(settingName).hasMatch()) {
		return false;
	}
	
	return settingsLoaded && !validSettings.isEmpty() && validSettings.contains(settingName);
}

QString CIniHighlighter::GetBasicSettingTooltip(const QString& settingName)
{
	TooltipMode mode = GetTooltipMode();
	if (mode == TooltipMode::Disabled) {
		return QString();
	}
	if (mode == TooltipMode::FullTooltip) {
		return GetSettingTooltip(settingName);
	}
	if (!isValidForTooltip(settingName))
		return QString();

	QString cacheKey = settingName % QStringLiteral("_basic");
	// basic tooltip: only version rows + description (or syntax if description missing) with special-case logic
	return getOrSetTooltipCache(cacheKey, [=]() {
		return BuildTooltipCore(settingName, /*includeMappings=*/false, /*includeContent=*/false, /*reserveSize=*/1024, /*preferDescriptionFirst=*/true);
		});
}

QString CIniHighlighter::GetSettingTooltip(const QString& settingName)
{
	TooltipMode mode = GetTooltipMode();

	// When hover tooltips are disabled we still want the completion popup
	// to be able to show helpful content. Use the popup-specific generator
	// as a fallback in that case.
	if (mode == TooltipMode::Disabled) {
		return GetSettingTooltipForPopup(settingName);
	}

	if (mode == TooltipMode::BasicInfo) {
		return GetBasicSettingTooltip(settingName);
	}

	if (!isValidForTooltip(settingName))
		return QString();

	// full tooltip: include mappings + full content
	return getOrSetTooltipCache(settingName, [=]() {
		return BuildTooltipCore(settingName, /*includeMappings=*/true, /*includeContent=*/true, /*reserveSize=*/2048, /*preferDescriptionFirst=*/true);
		});
}

// Helper to build popup tooltip content for Basic or Full popup modes.
QString CIniHighlighter::BuildPopupTooltip(const QString& settingName, bool basic)
{
	// Popup basic == same semantics as hover basic (selective version rows + description/syntax)
	// Popup full == full tooltip (mappings + content)
	if (!isValidForTooltip(settingName))
		return QString();

	if (basic) {
		return BuildTooltipCore(settingName, /*includeMappings=*/false, /*includeContent=*/false, /*reserveSize=*/1024, /*preferDescriptionFirst=*/true);
	}
	else {
		return BuildTooltipCore(settingName, /*includeMappings=*/true, /*includeContent=*/true, /*reserveSize=*/2048, /*preferDescriptionFirst=*/true);
	}
}

QString CIniHighlighter::GetSettingTooltipForPopup(const QString& settingName)
{
	// Determine configured modes
	int iniMode = theConf->GetInt("Options/EnableIniTooltips", static_cast<int>(GetTooltipMode()));
	int popupMode = theConf->GetInt("Options/EnablePopupTooltips", iniMode);

	// Disabled
	if (popupMode == Qt::Unchecked)
		return QString();

	// Always return nothing for invalid/unknown keys or when settings not loaded.
	if (!isValidForTooltip(settingName))
		return QString();

	// Basic popup tooltip
	if (popupMode == Qt::PartiallyChecked) {
		QString cacheKey = QStringLiteral("popup_basic_") + settingName;
		return getOrSetTooltipCache(cacheKey, [=]() {
			return BuildPopupTooltip(settingName, /*basic=*/true);
			});
	}

	// Full popup tooltip
	QString cacheKey = QStringLiteral("popup_") + settingName;
	return getOrSetTooltipCache(cacheKey, [=]() {
		return BuildPopupTooltip(settingName, /*basic=*/false);
		});
}

QString CIniHighlighter::BuildTooltipCore(const QString& settingName, bool includeMappings, bool includeContent, int reserveSize, bool preferDescriptionFirst)
{
	// Common guard
	if (!isValidForTooltip(settingName))
		return QString();

	const TooltipThemeCache& themeCache = getTooltipThemeCache();
	QString tooltip;
	tooltip.reserve(reserveSize);

	QMutexLocker locker(&settingsMutex);
	if (!isValidForTooltip(settingName))
		return QString();

	const SettingInfo& info = validSettings[settingName];
	const QString currentLang = getCurrentLanguage();

	// Header
	tooltip = HtmlTags::HTML_START % HtmlTags::TABLE_START % themeCache.tableStyle % HtmlTags::TAG_CLOSE;
	TooltipCellStyles headerStyles = getGenericStyles("Header");
	QString headerStyleStr = headerStyles.left.toHtmlStyle();
	tooltip += HtmlTags::TABLE_HEADER_START % headerStyleStr % HtmlTags::TAG_CLOSE
		% settingName % HtmlTags::TD_END % HtmlTags::TR_END;

	// Basic (no includeContent) special-case: selective version rows and content fallback
	if (!includeContent) {
		// Helper lambda to append a version row
		auto appendVersionRow = [&](const QString& label, const QString& value, const QString& styleKey) {
			if (!value.isEmpty()) {
				TooltipCellStyles styles = getGenericStyles(styleKey);
				appendGenericTooltipRow(tooltip, label, value, themeCache.labelStyle, HtmlTags::VALUE_PREFIX, styles);
			}
			};

		static const QString renamedVersionLabel = tr("Renamed in version");
		static const QString removedVersionLabel = tr("Removed in version");
		static const QString supersededByLabel = tr("Superseded by");

		// Preserve original GetBasicSettingTooltip behavior:
		// If renamedVersion present => show renamed + superseded
		// else if removedVersion present and (readdedVersion empty OR removed > readded) => show removed + superseded
		// else => show superseded + description (fallback to syntax)
		if (!info.renamedVersion.isEmpty()) {
			appendVersionRow(renamedVersionLabel, info.renamedVersion, "RenamedVersion");
			appendVersionRow(supersededByLabel, info.supersededBy, "SupersededBy");
		}
		else if (!info.removedVersion.isEmpty() &&
			(info.readdedVersion.isEmpty() || QVersionNumber::fromString(info.removedVersion) > QVersionNumber::fromString(info.readdedVersion))) {
			appendVersionRow(removedVersionLabel, info.removedVersion, "RemovedVersion");
			appendVersionRow(supersededByLabel, info.supersededBy, "SupersededBy");
		}
		else {
			// show superseded then description or syntax
			appendVersionRow(supersededByLabel, info.supersededBy, "SupersededBy");
			QString description = selectLocalizedContentOptimized(info.description, info.localizedDescriptions, currentLang);
			if (!description.isEmpty()) {
				appendTableRowForContent(tooltip, tr("Description"), description, themeCache.labelStyle, HtmlTags::VALUE_PREFIX, settingName, false);
			}
			else {
				QString syntax = selectLocalizedContentOptimized(info.syntax, info.localizedSyntax, currentLang);
				if (!syntax.isEmpty()) {
					appendTableRowForContent(tooltip, tr("Syntax"), syntax, themeCache.labelStyle, HtmlTags::VALUE_PREFIX, settingName, true);
				}
			}
		}
	}
	else {
		// Full tooltip: include all version rows + mappings + content (existing behavior)
		addVersionRows(tooltip, info, themeCache.labelStyle);
		if (includeMappings)
			processMappingsOptimized(tooltip, info, currentLang, themeCache.labelStyle);
		processContentOptimized(tooltip, info, currentLang, settingName, themeCache.labelStyle);
	}

	tooltip += HtmlTags::TABLE_END % HtmlTags::HTML_END;
	return tooltip;
}

// Helper for tooltip cache get-or-set pattern
QString CIniHighlighter::getOrSetTooltipCache(const QString& cacheKey, const std::function<QString()>& generator)
{
    // Use early return to minimize lock time
    {
        QMutexLocker cacheLock(&tooltipCacheMutex);
        auto it = tooltipCache.constFind(cacheKey);
        if (it != tooltipCache.constEnd())
            return it.value();
    }
    
    // Generate tooltip outside of lock to reduce contention
    QString tooltip = generator();
    
    if (!tooltip.isEmpty()) {
        QMutexLocker cacheLock(&tooltipCacheMutex);
        
        // Check cache size and implement LRU eviction if needed
        static const int MAX_CACHE_SIZE = 500;
        if (tooltipCache.size() >= MAX_CACHE_SIZE) {
            // Simple eviction: remove first quarter of entries
            auto it = tooltipCache.begin();
            int removeCount = MAX_CACHE_SIZE / 4;
            for (int i = 0; i < removeCount && it != tooltipCache.end(); ++i) {
                it = tooltipCache.erase(it);
            }
        }
        
        tooltipCache.insert(cacheKey, tooltip);
    }
    
    return tooltip;
}

const CIniHighlighter::TooltipThemeCache& CIniHighlighter::getTooltipThemeCache()
{
	// Cache theme colors to avoid repeated configuration access
	static TooltipThemeCache themeCache;

	if (!themeCache.valid) {
		bool bDark;
		int iDark = theConf->GetInt("Options/UseDarkTheme", 2);
		if (iDark == 2) {
			QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
			bDark = (settings.value("AppsUseLightTheme").toInt() == 0);
		}
		else
			bDark = (iDark == 1);

		themeCache.darkMode = bDark;
		themeCache.bgColor = themeCache.darkMode
			? (!s_tooltipBgColorDark.isEmpty() ? s_tooltipBgColorDark : QStringLiteral("#2b2b2b"))
			: (!s_tooltipBgColorLight.isEmpty() ? s_tooltipBgColorLight : QStringLiteral("#ffffff"));
		themeCache.textColor = themeCache.darkMode
			? (!s_tooltipTextColorDark.isEmpty() ? s_tooltipTextColorDark : QStringLiteral("#e0e0e0"))
			: (!s_tooltipTextColorLight.isEmpty() ? s_tooltipTextColorLight : QStringLiteral("#000000"));

		// Pre-build styles to avoid repeated string operations
		themeCache.tableStyle = HtmlAttribs::TABLE_STYLE
			.arg(themeCache.bgColor, themeCache.textColor);
		themeCache.labelStyle = HtmlAttribs::LABEL_STYLE
			.arg(themeCache.textColor);
		themeCache.valid = true;
	}

	return themeCache;
}

void CIniHighlighter::ClearThemeCache()
{
	// Access the static variable inside getTooltipThemeCache()
	// by calling the function and using a const_cast to modify it.
	auto& cache = const_cast<TooltipThemeCache&>(getTooltipThemeCache());
	cache.valid = false;
}

QStringList CIniHighlighter::GetCompletionCandidates()
{
	QMutexLocker locker(&settingsMutex);
	
	if (!settingsLoaded || validSettings.isEmpty()) {
		return QStringList();
	}
	
	QStringList candidates;
	candidates.reserve(validSettings.size());
	
	for (auto it = validSettings.constBegin(); it != validSettings.constEnd(); ++it) {
		candidates.append(it.key());
	}
	
	candidates.sort(Qt::CaseInsensitive);
	return candidates;
}

QString CIniHighlighter::FindCaseCorrectedKey(const QString& wrongKey)
{
	QMutexLocker locker(&settingsMutex);
	
	if (!settingsLoaded || validSettings.isEmpty() || wrongKey.isEmpty()) {
		return QString();
	}
	
	// Fast exact match check first
	if (validSettings.contains(wrongKey)) {
		return QString(); // Key is already correct
	}
	
	// Case-insensitive search for correction
	for (auto it = validSettings.constBegin(); it != validSettings.constEnd(); ++it) {
		if (it.key().compare(wrongKey, Qt::CaseInsensitive) == 0) {
			return it.key(); // Found case-corrected version
		}
	}
	
	return QString(); // No match found
}

bool CIniHighlighter::IsValidKey(const QString& keyName)
{
	QMutexLocker locker(&settingsMutex);
	return settingsLoaded && !validSettings.isEmpty() && validSettings.contains(keyName);
}

bool CIniHighlighter::IsKeyHiddenFromPopup(const QString& keyName)
{
	// First check the hide configuration
	if (IsKeyHiddenFromContext(keyName, 'p')) {
		return true;
	}

	// Then check the existing version-based logic
	QMutexLocker locker(&settingsMutex);
	if (!settingsLoaded || validSettings.isEmpty())
		return false;
	auto it = validSettings.constFind(keyName);
	if (it == validSettings.constEnd())
		return false;

	const SettingInfo& info = it.value();
	const QVersionNumber currentVersion = getCurrentVersion();

	// Check for removed
	if (!info.removedVersion.isEmpty()) {
		QVersionNumber removedVer = QVersionNumber::fromString(info.removedVersion);
		if (!removedVer.isNull() && currentVersion >= removedVer) {
			// If readded, only hide if not readded yet
			if (!info.readdedVersion.isEmpty()) {
				QVersionNumber readdedVer = QVersionNumber::fromString(info.readdedVersion);
				if (!readdedVer.isNull() && currentVersion < readdedVer)
					return true;
				if (!readdedVer.isNull() && currentVersion >= readdedVer)
					return false;
			}
			return true;
		}
	}
	// Check for renamed
	if (!info.renamedVersion.isEmpty()) {
		QVersionNumber renamedVer = QVersionNumber::fromString(info.renamedVersion);
		if (!renamedVer.isNull() && currentVersion >= renamedVer)
			return true;
	}
	return false;
}

void CIniHighlighter::parseHideConfRules(const QString& value, QHash<QString, QString>& rules)
{
	if (value.isEmpty())
		return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QStringList ruleStrings = value.split(';', Qt::SkipEmptyParts);
#else
	QStringList ruleStrings = value.split(';', QString::SkipEmptyParts);
#endif

	for (const QString& rule : ruleStrings) {
		QStringList parts = rule.split('|');
		if (parts.size() >= 1) {
			QString setting = parts[0].trimmed();
			QString action = parts.size() >= 2 ? parts[1].trimmed() : QString();

			if (!setting.isEmpty()) {
				rules.insert(setting, action);
			}
		}
	}
}

bool CIniHighlighter::matchesWildcard(const QString& pattern, const QString& text)
{
	if (pattern == text) {
		return true;
	}

	if (!pattern.contains('*')) {
		return pattern.compare(text, Qt::CaseInsensitive) == 0;
	}

	QString regexPattern = convertWildcardToRegex(pattern);
	QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);
	return regex.match(text).hasMatch();
}

QString CIniHighlighter::convertWildcardToRegex(const QString& wildcard)
{
	QString regex = QRegularExpression::escape(wildcard);
	regex.replace("\\*", ".*");
	regex.replace("\\?", ".");
	return "^" % regex % "$";
}

bool CIniHighlighter::IsKeyHiddenFromContext(const QString& keyName, char context)
{
	QMutexLocker locker(&hideConfMutex);

	if (keyName.isEmpty()) {
		return false;
	}

	// Check if key should be hidden
	bool shouldHide = false;
	QString hideAction;

	// Check hide rules
	for (auto it = hideConfRules.constBegin(); it != hideConfRules.constEnd(); ++it) {
		if (matchesWildcard(it.key(), keyName)) {
			shouldHide = true;
			hideAction = it.value();
			break;
		}
	}

	if (!shouldHide) {
		return false;
	}

	// Check exclusions - find ALL matching exclusions and check if any applies to this context
	bool hasApplicableExclusion = false;
	for (auto it = hideConfExclusions.constBegin(); it != hideConfExclusions.constEnd(); ++it) {
		if (matchesWildcard(it.key(), keyName)) {
			QString exclusionAction = it.value();
			if (exclusionAction.isEmpty()) {
				hasApplicableExclusion = true;
				break;
			}
			else if (exclusionAction.contains(context)) {
				hasApplicableExclusion = true;
				break;
			}
		}
	}

	if (hasApplicableExclusion) {
		return false;
	}

	// Check if the original hide action applies to this context
	if (hideAction.isEmpty()) {
		return true; // Hide from all contexts
	}

	bool result = hideAction.contains(context);
	return result;
}

QString CIniHighlighter::TooltipStyle::toHtmlStyle() const {
	QStringList styles;
	if (!color.isEmpty()) {
		styles << HtmlAttribs::COLOR_VAR.arg(color);
	}
	if (bold) {
		styles << HtmlAttribs::STYLE_BOLD;
	}
	if (italic) {
		styles << HtmlAttribs::STYLE_ITALIC;
	}
	if (underline) {
		styles << HtmlAttribs::STYLE_UNDERLINE;
	}
	if (!alignment.isEmpty()) {
		styles << HtmlAttribs::STYLE_ALIGNMENT.arg(alignment);
	}

	QStringList cleanStyles;
	for (const QString& s : styles) {
		QString trimmed = s.trimmed();
		if (!trimmed.isEmpty()) {
			// Ensure each style ends with a single semicolon
			if (!trimmed.endsWith(';'))
				trimmed += ';';
			cleanStyles << trimmed;
		}
	}
	return cleanStyles.isEmpty() ? "" : HtmlAttribs::STYLE_VAR.arg(cleanStyles.join(""));
}

bool CIniHighlighter::IsValidTooltipContext(const QString& hoveredText)
{
	if (hoveredText.isEmpty() || hoveredText.length() > getMaxSettingNameLengthOrDefault())
		return false;

	// Check for exact match at start of line
	if (isValidForTooltip(hoveredText))
		return true;

	// Check for SettingName= (no trailing content), must be at start of line
	if (hoveredText.endsWith('=') && hoveredText.length() > 1) {
		QString name = hoveredText.left(hoveredText.length() - 1);
		if (isValidForTooltip(name))
			return true;
	}

	// If there is any non-empty prefix before the setting name, do not show tooltip
	// (e.g. ",SettingName", ".SettingName", " xSettingName")
	// This is already handled by the above logic, as isValidForTooltip will only match valid keys
	// at the start of the string, and not with any prefix.

	return false;
}

int CIniHighlighter::getMaxSettingNameLengthOrDefault()
{
	if (s_maxSettingNameLengthValid) {
		return s_maxSettingNameLength;
	}

	int maxLength = 0;

	for (const auto& key : validSettings.keys()) {
		int currentLength = key.length();
		if (currentLength > maxLength) {
			//qDebug() << "Key:" << key << "Length:" << currentLength;
			maxLength = currentLength;
		}
	}

	s_maxSettingNameLength = (maxLength > 0) ? maxLength : 32;
	s_maxSettingNameLengthValid = true;

	return s_maxSettingNameLength;
}

int CIniHighlighter::getMinSettingNameLengthOrDefault()
{
	if (s_minSettingNameLengthValid) {
		return s_minSettingNameLength;
	}

	int minLength = INT_MAX;

	for (const auto& key : validSettings.keys()) {
		int currentLength = key.length();
		if (currentLength < minLength) {
			qDebug() << "[validSettings] Found shorter key:" << key << "Length:" << currentLength;
			minLength = currentLength;
		}
	}

	// sensible fallback
	s_minSettingNameLength = (minLength != INT_MAX && minLength > 0) ? minLength : 1;
	s_minSettingNameLengthValid = true;

	return s_minSettingNameLength;
}

void CIniHighlighter::reloadSettingsIniIfNeeded(const QString& settingsPath, const QFileInfo& fileInfo)
{
	if (!settingsDirty) {
		qDebug() << "[validSettings] Skipping reload, settingsDirty is false";
		return;
	}

	bool needToLoad = false;
	{
		QMutexLocker locker(&settingsMutex);
		needToLoad = !settingsLoaded
			|| !fileInfo.exists()
			|| fileInfo.lastModified() > lastFileModified;
	}

	if (needToLoad) {
		loadSettingsIni(settingsPath);
	}
	else {
		qDebug() << "[validSettings] Using cached settings (" << validSettings.size() << " entries)";
	}

	settingsDirty = false;
}

void CIniHighlighter::reloadUserIniIfNeeded(const QString& userIniPath, const QFileInfo& userFileInfo)
{
	if (!userSettingsDirty) {
		qDebug() << "[validSettings] Skipping user INI reload, userSettingsDirty is false";
		return;
	}

	bool needToLoadUserIni = false;
	{
		QMutexLocker locker(&userSettingsMutex); // Lock for checking cache status
		needToLoadUserIni = !userIniLoaded
			|| !userFileInfo.exists()
			|| userFileInfo.lastModified() > lastUserFileModified
			|| !settingsLoaded;
	}

	if (needToLoadUserIni) {
		applyUserIniOverrides(s_masterVersion, userIniPath);
		lastUserFileModified = userFileInfo.lastModified();
		userIniLoaded = true;
	}
	else {
		qDebug() << "[validSettings] Using cached user overrides";
	}

	userSettingsDirty = false;
}

void CIniHighlighter::MarkSettingsDirty()
{
	settingsDirty = true;
}

void CIniHighlighter::MarkUserSettingsDirty()
{
	userSettingsDirty = true;
}
