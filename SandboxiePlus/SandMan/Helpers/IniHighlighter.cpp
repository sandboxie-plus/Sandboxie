#include "stdafx.h"
#include "IniHighlighter.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../version.h"


// Settings validation and tooltip handling
const QString CIniHighlighter::DEFAULT_SETTINGS_FILE = "SbieSettings.ini";
const QString CIniHighlighter::DEFAULT_VERSION = "0.0.0";

QVersionNumber CIniHighlighter::s_currentVersion;
QString CIniHighlighter::s_currentLanguage;
QMutex CIniHighlighter::s_languageMutex;

QHash<QString, SettingInfo> CIniHighlighter::validSettings;
QDateTime CIniHighlighter::lastFileModified;
bool CIniHighlighter::settingsLoaded = false;
QMutex CIniHighlighter::settingsMutex;

QHash<QString, QString> CIniHighlighter::tooltipCache;
QMutex CIniHighlighter::tooltipCacheMutex;

CIniHighlighter::TooltipMode CIniHighlighter::s_tooltipMode = TooltipMode::FullTooltip;
QMutex CIniHighlighter::s_tooltipModeMutex;

CIniHighlighter::KeywordGroup<CIniHighlighter::KeywordType::Context> CIniHighlighter::contextData;
CIniHighlighter::KeywordGroup<CIniHighlighter::KeywordType::Category> CIniHighlighter::categoryData;
CIniHighlighter::KeywordGroup<CIniHighlighter::KeywordType::Requirements> CIniHighlighter::requirementsData;
// Settings validation and tooltip handling

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
	QString settingsPath = QCoreApplication::applicationDirPath() + "/" + DEFAULT_SETTINGS_FILE;
	QFileInfo fileInfo(settingsPath);

	bool needToLoad = false;
	{
		QMutexLocker locker(&settingsMutex); // Lock for checking cache status
		needToLoad = !settingsLoaded || !fileInfo.exists() || fileInfo.lastModified() > lastFileModified;
	}

	if (needToLoad) {
		loadSettingsIni(settingsPath);
	}
	else {
		qDebug() << "[validSettings] Using cached settings (" << validSettings.size() << " entries)";
	}

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

// Load settings from SbieSettings.ini
void CIniHighlighter::loadSettingsIni(const QString& filePath)
{
	QMutexLocker locker(&settingsMutex); // Lock during the entire load operation

	ClearLanguageCache();

	QFile file(filePath);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		validSettings.clear();
		contextData.clear();
		categoryData.clear();
		requirementsData.clear();

		QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
		// Qt5: use setCodec
		in.setCodec("UTF-8");
#endif

		QString currentSection;
		SettingInfo currentInfo;
		bool inSection = false;
		bool inConfigSection = false;

		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();

			// Skip empty lines and comments
			if (line.isEmpty() || line.startsWith(';') || line.startsWith('#'))
				continue;

			// Check if this is a section header [SectionName]
			QRegularExpression sectionRegex(R"(^\[([^\]]+)\]\s*$)");
			QRegularExpressionMatch sectionMatch = sectionRegex.match(line);

			if (sectionMatch.hasMatch()) {
				// If we were already processing a section, save it (except for config section)
				if (inSection && !currentSection.isEmpty() && !inConfigSection) {
					currentInfo.name = currentSection;
					validSettings.insert(currentInfo.name, currentInfo);
				}

				// Start a new section
				currentSection = sectionMatch.captured(1).trimmed();
				inConfigSection = (currentSection == "___SbieSettingsConfig_");

				if (!inConfigSection) {
					currentInfo = SettingInfo(); // Reset info for new regular section
					inSection = true;
				}
				else {
					// We're entering the special config section
					inSection = false;
				}
				continue;
			}

			// If not in a section yet, skip
			if (!inSection && !inConfigSection)
				continue;

			// Process key=value pairs
			int equalsPos = line.indexOf('=');
			if (equalsPos > 0) {
				QString key = line.left(equalsPos).trimmed();
				QString value = line.mid(equalsPos + 1).trimmed();

				if (inConfigSection) {
					// Process configuration settings
					if (key.compare("_ContextConf", Qt::CaseInsensitive) == 0) {
						// Parse default context keyword mappings
						contextData.mappings = parseKeywordMappings<KeywordType::Context>(value);
					}
					else if (key.startsWith("_ContextConf_", Qt::CaseInsensitive)) {
						// Handle localized context mappings
						QString langCode = extractLanguageCode(key, "_ContextConf");
						if (!langCode.isEmpty()) {
							contextData.localizedMappings.insert(langCode, parseKeywordMappings<KeywordType::Context>(value));
						}
					}
					else if (key.compare("_CategoryConf", Qt::CaseInsensitive) == 0) {
						// Parse default category keyword mappings
						categoryData.mappings = parseKeywordMappings<KeywordType::Category>(value);
					}
					else if (key.startsWith("_CategoryConf_", Qt::CaseInsensitive)) {
						// Handle localized category mappings
						QString langCode = extractLanguageCode(key, "_CategoryConf");
						if (!langCode.isEmpty()) {
							categoryData.localizedMappings.insert(langCode, parseKeywordMappings<KeywordType::Category>(value));
						}
					}
					else if (key.compare("_RequirementsConf", Qt::CaseInsensitive) == 0) {
						// Parse default requirements keyword mappings
						requirementsData.mappings = parseKeywordMappings<KeywordType::Requirements>(value);
					}
					else if (key.startsWith("_RequirementsConf_", Qt::CaseInsensitive)) {
						// Handle localized requirements mappings
						QString langCode = extractLanguageCode(key, "_RequirementsConf");
						if (!langCode.isEmpty()) {
							requirementsData.localizedMappings.insert(langCode, parseKeywordMappings<KeywordType::Requirements>(value));
						}
					}
					else if (key.compare("_ContextStyles", Qt::CaseInsensitive) == 0) {
						// Parse context tooltip style configuration
						contextData.tooltipStyle = parseStyleConfig(value);
					}
					else if (key.compare("_CategoryStyles", Qt::CaseInsensitive) == 0) {
						// Parse category tooltip style configuration
						categoryData.tooltipStyle = parseStyleConfig(value);
					}
					else if (key.compare("_RequirementsStyles", Qt::CaseInsensitive) == 0) {
						// Parse requirements tooltip style configuration
						requirementsData.tooltipStyle = parseStyleConfig(value);
					}
				}
				else {
					// Process normal setting fields
					// Helper function to sanitize version strings
					auto sanitizeVersion = [this](const QString& s, bool defaultZero = false) {
						QString v = s.trimmed();
						v.remove(QRegularExpression("[^0-9.]"));
						QRegularExpression rx("^[0-9]+\\.[0-9]+\\.[0-9]+$"); // Exact x.y.z format
						if (rx.match(v).hasMatch())
							return v;
						return defaultZero ? QString(DEFAULT_VERSION) : QString();
						};

					// Helper function to process localized text fields
					auto processLocalizedField = [&](const QString& baseKey, const QString& prefix, 
													 QString& baseField, QMap<QString, QString>& localizedMap) {
						if (key.compare(baseKey, Qt::CaseInsensitive) == 0) {
							baseField = processPlaceholders(sanitizeHtmlInput(value));
							return true;
						}
						else if (key.startsWith(prefix, Qt::CaseInsensitive)) {
							QString langCode = extractLanguageCode(key, prefix.chopped(1)); // Remove trailing '_'
							if (!langCode.isEmpty()) {
								QString localizedContent = processPlaceholders(sanitizeHtmlInput(value));
								
								// If there's already content for this language, append with newline
								if (localizedMap.contains(langCode)) {
									localizedMap[langCode] += "\n" + localizedContent;
								}
								else {
									localizedMap.insert(langCode, localizedContent);
								}
							}
							return true;
						}
						return false;
					};

					// Process special fields
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
						// Handled by helper function
					}
					else if (processLocalizedField("Description", "Description_", currentInfo.description, currentInfo.localizedDescriptions)) {
						// Handled by helper function
					}
					else if (key.compare("Requirements", Qt::CaseInsensitive) == 0)
						currentInfo.requirements = value.toLower().trimmed();
				}
			}
		}

		// Don't forget to save the last section (if not the config section)
		if (inSection && !currentSection.isEmpty() && !inConfigSection) {
			currentInfo.name = currentSection;
			// Ensure name contains only valid characters
			currentInfo.name.remove(QRegularExpression("[^a-zA-Z0-9_.]"));
			validSettings.insert(currentInfo.name, currentInfo);
		}

		file.close();

		// Update the cache status after successful load
		lastFileModified = QFileInfo(filePath).lastModified();
		settingsLoaded = true;

		qDebug() << "[validSettings] Successfully loaded" << validSettings.size() << "settings,"
			<< contextData.mappings.size() << "context mappings,"
			<< contextData.localizedMappings.size() << "localized context mappings,"
			<< categoryData.mappings.size() << "category mappings,"
			<< categoryData.localizedMappings.size() << "localized category mappings,"
			<< requirementsData.mappings.size() << "requirements mappings, and"
			<< requirementsData.localizedMappings.size() << "localized requirements mappings from" << filePath;
	}
	else {
		// File couldn't be opened - log the error
		qWarning() << "[validSettings] Failed to load settings file:" << filePath << "Error:" << file.errorString();

		// Keep settings loaded flag false so we try again next time
		settingsLoaded = false;
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

template CIniHighlighter::KeywordMappings<CIniHighlighter::KeywordType::Context>
CIniHighlighter::parseKeywordMappings<CIniHighlighter::KeywordType::Context>(const QString& value);

template CIniHighlighter::KeywordMappings<CIniHighlighter::KeywordType::Category>
CIniHighlighter::parseKeywordMappings<CIniHighlighter::KeywordType::Category>(const QString& value);

template CIniHighlighter::KeywordMappings<CIniHighlighter::KeywordType::Requirements>
CIniHighlighter::parseKeywordMappings<CIniHighlighter::KeywordType::Requirements>(const QString& value);

CIniHighlighter::TooltipStyle CIniHighlighter::parseStyleConfig(const QString& styleConfig)
{
	TooltipStyle style;

	if (styleConfig.isEmpty()) {
		return style; // Return default (empty) style
	}

	QStringList parts = styleConfig.split('|');

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
		}
	}

	// Parse font (third part) - currently not used in TooltipStyle but could be extended
	if (parts.size() >= 3 && !parts[2].trimmed().isEmpty()) {
		// Font parsing could be added here if needed in the future
		// style.fontFamily = parts[2].trimmed();
	}

	return style;
}

void CIniHighlighter::addVersionRows(QString& tooltip, const SettingInfo& info, const QString& labelStyle)
{
	static const QString valuePrefix = QStringLiteral(": ");

	// Use static const QStringLiteral for frequently used strings
	static const QString addedVersionLabel = tr("Added in version");
	static const QString removedVersionLabel = tr("Removed in version");
	static const QString readdedVersionLabel = tr("Re-added in version");
	static const QString renamedVersionLabel = tr("Renamed in version");
	static const QString supersededByLabel = tr("Superseded by");

	// Optimized version row addition with early returns
	if (!info.addedVersion.isEmpty() && info.addedVersion != DEFAULT_VERSION) {
		tooltip += QStringLiteral("<tr><td ") % labelStyle % QStringLiteral(">")
			% addedVersionLabel % QStringLiteral("</td><td>")
			% valuePrefix % info.addedVersion % QStringLiteral("</td></tr>");
	}

	if (!info.removedVersion.isEmpty()) {
		tooltip += QStringLiteral("<tr><td ") % labelStyle % QStringLiteral(">")
			% removedVersionLabel % QStringLiteral("</td><td>")
			% valuePrefix % info.removedVersion % QStringLiteral("</td></tr>");
	}

	if (!info.readdedVersion.isEmpty()) {
		tooltip += QStringLiteral("<tr><td ") % labelStyle % QStringLiteral(">")
			% readdedVersionLabel % QStringLiteral("</td><td>")
			% valuePrefix % info.readdedVersion % QStringLiteral("</td></tr>");
	}

	if (!info.renamedVersion.isEmpty()) {
		tooltip += QStringLiteral("<tr><td ") % labelStyle % QStringLiteral(">")
			% renamedVersionLabel % QStringLiteral("</td><td>")
			% valuePrefix % info.renamedVersion % QStringLiteral("</td></tr>");
	}

	if (!info.supersededBy.isEmpty()) {
		tooltip += QStringLiteral("<tr><td ") % labelStyle % QStringLiteral(">")
			% supersededByLabel % QStringLiteral("</td><td>")
			% valuePrefix % info.supersededBy % QStringLiteral("</td></tr>");
	}
}

QString CIniHighlighter::processTextLineOptimized(const QString& text, const QString& settingName)
{
	QString processed = text;

	// Process escape sequences and placeholders
	if (Q_UNLIKELY(processed.contains(QLatin1Char('\\')))) {
		processed.replace(QStringLiteral("\\n"), QStringLiteral("\n"))
			.replace(QStringLiteral("\\t"), QStringLiteral("\t"))
			.replace(QStringLiteral("\\r"), QStringLiteral("\r"))
			.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
	}

	if (Q_UNLIKELY(!settingName.isEmpty() && processed.contains(QStringLiteral("[sn]")))) {
		processed.replace(QStringLiteral("[sn]"), settingName);
	}

	// Process other placeholders
	if (Q_UNLIKELY(processed.contains(QLatin1Char('[')))) {
		processed.replace(QStringLiteral("[br]"), QStringLiteral("\n"))
			.replace(QStringLiteral("[sbie]"), QStringLiteral("Sandboxie-Plus"));

		if (Q_UNLIKELY(processed.contains(QStringLiteral("[version]")))) {
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
			.replace(QStringLiteral("[code]"), QStringLiteral("<code style='font-family: Consolas, monospace; background-color: #f0f0f0; padding: 1px 3px;'>"))
			.replace(QStringLiteral("[/code]"), QStringLiteral("</code>"));
	}

	return processed;
}

QString CIniHighlighter::selectLocalizedContentOptimized(const QString& defaultContent,
	const QMap<QString, QString>& localizedMap,
	const QString& currentLang)
{
	if (Q_LIKELY(currentLang.isEmpty() || localizedMap.isEmpty())) {
		return defaultContent;
	}

	// Use constFind for better performance
	auto it = localizedMap.constFind(currentLang);
	if (Q_LIKELY(it != localizedMap.constEnd())) {
		return it.value();
	}

	// Try with language base only if underscore is present
	if (Q_UNLIKELY(currentLang.contains(QLatin1Char('_')))) {
		const int underscorePos = currentLang.indexOf(QLatin1Char('_'));
		const QString langBase = currentLang.left(underscorePos);

		it = localizedMap.constFind(langBase);
		if (it != localizedMap.constEnd()) {
			return it.value();
		}
	}

	return defaultContent;
}

void CIniHighlighter::processMappingsOptimized(QString& tooltip, const SettingInfo& info,
	const QString& currentLang, const QString& labelStyle)
{
	static const QString valuePrefix = QStringLiteral(": ");

	// Cache translated strings to avoid repeated tr() calls
	static const QString categoryLabel = tr("Category");
	static const QString contextLabel = tr("Context");
	static const QString requirementsLabel = tr("Requirements");

	// Enhanced helper function to get effective mappings with action fallback logic
	auto getEffectiveMappingsWithActionFallback = [](const auto& data, const QString& currentLang) {
		auto defaultMappings = data.mappings;

		// If no language is set or no localized mappings exist, use default
		if (currentLang.isEmpty() || data.localizedMappings.isEmpty()) {
			return defaultMappings;
		}

		// Try to find localized mappings for current language
		auto localizedMappings = [&]() -> decltype(defaultMappings) {
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
			return decltype(defaultMappings){};
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
		QStringList localizedKeywords;
		for (const auto& item : localizedMappings) {
			localizedActions.insert(item.keyword, item.action);
			localizedDisplayNames.insert(item.keyword, item.displayName);
			localizedKeywords.append(item.keyword);
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
		};

	// Process category mappings with action fallback
	if (!info.category.isEmpty()) {
		auto effectiveMappings = getEffectiveMappingsWithActionFallback(categoryData, currentLang);
		processKeywordMappings<KeywordInfo<KeywordType::Category>>(
			info.category, effectiveMappings, categoryLabel,
			labelStyle, valuePrefix, categoryData.tooltipStyle, tooltip);
	}

	// Process context mappings with action fallback
	if (!info.context.isEmpty()) {
		auto effectiveMappings = getEffectiveMappingsWithActionFallback(contextData, currentLang);
		processKeywordMappings<KeywordInfo<KeywordType::Context>>(
			info.context, effectiveMappings, contextLabel,
			labelStyle, valuePrefix, contextData.tooltipStyle, tooltip);
	}

	// Process requirements mappings with action fallback
	if (!info.requirements.isEmpty()) {
		auto effectiveMappings = getEffectiveMappingsWithActionFallback(requirementsData, currentLang);
		processKeywordMappings<KeywordInfo<KeywordType::Requirements>>(
			info.requirements, effectiveMappings, requirementsLabel,
			labelStyle, valuePrefix, requirementsData.tooltipStyle, tooltip);
	}
}

void CIniHighlighter::processContentOptimized(QString& tooltip, const SettingInfo& info,
	const QString& currentLang, const QString& settingName,
	const QString& labelStyle)
{
	static const QString valuePrefix = QStringLiteral(": ");

	// Cache translated strings
	static const QString syntaxLabel = tr("Syntax");
	static const QString descriptionLabel = tr("Description");

	// Process syntax
	QString syntax = selectLocalizedContentOptimized(info.syntax, info.localizedSyntax, currentLang);
	if (!syntax.isEmpty()) {
		QStringList syntaxLines = syntax.split('\n');
		tooltip += QStringLiteral("<tr><td ") % labelStyle % QStringLiteral(" style='vertical-align:top;'>")
			% syntaxLabel % QStringLiteral("</td>");

		if (!syntaxLines.isEmpty()) {
			QString processedLine = processTextLineOptimized(syntaxLines[0], settingName);

			// Apply styling to square brackets only (for syntax highlighting)
			processedLine.replace(QStringLiteral("["), QStringLiteral("<span style='color:#2196F3;'>[</span>"));
			processedLine.replace(QStringLiteral("]"), QStringLiteral("<span style='color:#2196F3;'>]</span>"));

			tooltip += QStringLiteral("<td>") % valuePrefix % QStringLiteral("<span style='font-family: Consolas, monospace;'>")
				% processedLine % QStringLiteral("</span>");

			// Add additional syntax lines with proper indentation
			for (int i = 1; i < syntaxLines.size(); i++) {
				processedLine = processTextLineOptimized(syntaxLines[i], settingName);
				processedLine.replace(QStringLiteral("["), QStringLiteral("<span style='color:#2196F3;'>[</span>"));
				processedLine.replace(QStringLiteral("]"), QStringLiteral("<span style='color:#2196F3;'>]</span>"));
				tooltip += QStringLiteral("<br>&nbsp;&nbsp;<span style='font-family: Consolas, monospace;'>")
					% processedLine % QStringLiteral("</span>");
			}
			tooltip += QStringLiteral("</td></tr>");
		}
	}

	// Process description
	QString description = selectLocalizedContentOptimized(info.description, info.localizedDescriptions, currentLang);
	if (!description.isEmpty()) {
		QStringList descLines = description.split('\n');
		tooltip += QStringLiteral("<tr><td ") % labelStyle % QStringLiteral(" style='vertical-align:top;'>")
			% descriptionLabel % QStringLiteral("</td>");

		if (!descLines.isEmpty()) {
			QString processedLine = processTextLineOptimized(descLines[0], settingName);
			tooltip += QStringLiteral("<td>") % valuePrefix % processedLine;

			for (int i = 1; i < descLines.size(); i++) {
				processedLine = processTextLineOptimized(descLines[i], settingName);
				tooltip += QStringLiteral("<br>&nbsp;&nbsp;") % processedLine;
			}
			tooltip += QStringLiteral("</td></tr>");
		}
	}
}

QString CIniHighlighter::processPlaceholders(const QString& text, const QString& settingName)
{
	// This function is now redundant - use processTextLineOptimized instead
	return processTextLineOptimized(text, settingName);
}

QString CIniHighlighter::sanitizeHtmlInput(const QString& input)
{
	QString sanitized = input;

	// 1. Escape ALL < and > characters for security - no exceptions
	sanitized.replace("<", "&lt;");
	sanitized.replace(">", "&gt;");

	// 2. That's it! No restoration of any HTML tags.
	// All HTML formatting will come from square bracket tags processed later
	// in processPlaceholders() and processTextLineOptimized()

	return sanitized;
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

CIniHighlighter::TooltipMode CIniHighlighter::GetTooltipMode()
{
	QMutexLocker locker(&s_tooltipModeMutex);
	return s_tooltipMode;
}

const CIniHighlighter::TooltipThemeCache& CIniHighlighter::getTooltipThemeCache()
{
	// Cache theme colors to avoid repeated configuration access
	static thread_local TooltipThemeCache themeCache;

	if (Q_UNLIKELY(!themeCache.valid)) {
		themeCache.darkMode = theConf->GetBool("Options/DarkTheme", false);
		themeCache.bgColor = themeCache.darkMode ? QStringLiteral("#2b2b2b") : QStringLiteral("#ffffff");
		themeCache.textColor = themeCache.darkMode ? QStringLiteral("#e0e0e0") : QStringLiteral("#000000");

		// Pre-build styles to avoid repeated string operations
		themeCache.tableStyle = QStringLiteral("style='border:none; white-space:nowrap; background-color:%1; color:%2;'")
			.arg(themeCache.bgColor, themeCache.textColor);
		themeCache.labelStyle = QStringLiteral("style='text-align:left; padding-right:8px; color:%1;'")
			.arg(themeCache.textColor);
		themeCache.valid = true;
	}

	return themeCache;
}

void CIniHighlighter::ClearLanguageCache()
{
	// Clear the tooltip cache since it contains language-specific content
	{
		QMutexLocker cacheLock(&tooltipCacheMutex);
		tooltipCache.clear();
	}
}

bool CIniHighlighter::isValidForTooltip(const QString& settingName)
{
	return settingsLoaded && !settingName.isEmpty() && !validSettings.isEmpty() && validSettings.contains(settingName);
}

QString CIniHighlighter::GetBasicSettingTooltip(const QString& settingName)
{
	// Early safety check using consolidated function
	if (Q_UNLIKELY(!isValidForTooltip(settingName)))
		return QString();

	// Check tooltip cache first (fast path)
	QString cacheKey = settingName % QStringLiteral("_basic");
	{
		QMutexLocker cacheLock(&tooltipCacheMutex);
		auto it = tooltipCache.constFind(cacheKey);
		if (Q_LIKELY(it != tooltipCache.constEnd()))
			return it.value();
	}

	// Get cached theme data
	const TooltipThemeCache& themeCache = getTooltipThemeCache();

	// Generate basic tooltip content with settings lock
	QString tooltip;
	{
		QMutexLocker locker(&settingsMutex);

		// Double-check after acquiring the lock
		if (Q_UNLIKELY(!settingsLoaded || validSettings.isEmpty() || !validSettings.contains(settingName)))
			return QString();

		const SettingInfo& info = validSettings[settingName];
		const QString currentLang = getCurrentLanguage();

		// Build basic tooltip using cached theme styles
		tooltip = QStringLiteral("<table ") % themeCache.tableStyle % QStringLiteral(">");

		// Add setting name row
		tooltip += QStringLiteral("<tr><td colspan='2' style='text-align:left; font-weight:bold;'>")
			% settingName % QStringLiteral("</td></tr>");

		// Try to add description first
		QString description = selectLocalizedContentOptimized(info.description, info.localizedDescriptions, currentLang);
		if (!description.isEmpty()) {
			QStringList descLines = description.split('\n');
			tooltip += QStringLiteral("<tr><td ") % themeCache.labelStyle % QStringLiteral(" style='vertical-align:top;'>")
				% tr("Description") % QStringLiteral("</td>");

			if (!descLines.isEmpty()) {
				QString processedLine = processTextLineOptimized(descLines[0], settingName);
				tooltip += QStringLiteral("<td>: ") % processedLine;

				for (int i = 1; i < descLines.size(); i++) {
					processedLine = processTextLineOptimized(descLines[i], settingName);
					tooltip += QStringLiteral("<br>&nbsp;&nbsp;") % processedLine;
				}
				tooltip += QStringLiteral("</td></tr>");
			}
		}
		else {
			// If description is missing, show syntax instead
			QString syntax = selectLocalizedContentOptimized(info.syntax, info.localizedSyntax, currentLang);
			if (!syntax.isEmpty()) {
				QStringList syntaxLines = syntax.split('\n');
				tooltip += QStringLiteral("<tr><td ") % themeCache.labelStyle % QStringLiteral(" style='vertical-align:top;'>")
					% tr("Syntax") % QStringLiteral("</td>");

				if (!syntaxLines.isEmpty()) {
					QString processedLine = processTextLineOptimized(syntaxLines[0], settingName);

					// Apply styling to square brackets
					processedLine.replace(QStringLiteral("["), QStringLiteral("<span style='color:#2196F3;'>[</span>"));
					processedLine.replace(QStringLiteral("]"), QStringLiteral("<span style='color:#2196F3;'>]</span>"));

					tooltip += QStringLiteral("<td>: <span style='font-family: Consolas, monospace;'>")
						% processedLine % QStringLiteral("</span>");

					// Add additional syntax lines with proper indentation
					for (int i = 1; i < syntaxLines.size(); i++) {
						processedLine = processTextLineOptimized(syntaxLines[i], settingName);
						processedLine.replace(QStringLiteral("["), QStringLiteral("<span style='color:#2196F3;'>[</span>"));
						processedLine.replace(QStringLiteral("]"), QStringLiteral("<span style='color:#2196F3;'>]</span>"));
						tooltip += QStringLiteral("<br>&nbsp;&nbsp;<span style='font-family: Consolas, monospace;'>")
							% processedLine % QStringLiteral("</span>");
					}
					tooltip += QStringLiteral("</td></tr>");
				}
			}
		}

		tooltip += QStringLiteral("</table>");
	}

	// Store in cache
	if (Q_LIKELY(!tooltip.isEmpty())) {
		QMutexLocker cacheLock(&tooltipCacheMutex);
		tooltipCache.insert(cacheKey, tooltip);
	}

	return tooltip;
}

QString CIniHighlighter::GetSettingTooltip(const QString& settingName)
{
	// Check tooltip mode first
	TooltipMode mode = GetTooltipMode();

	// Return empty string if tooltips are disabled
	if (mode == TooltipMode::Disabled) {
		return QString();
	}

	// Return basic tooltip if in basic mode
	if (mode == TooltipMode::BasicInfo) {
		return GetBasicSettingTooltip(settingName);
	}

	// Early safety checks
	if (Q_UNLIKELY(!settingsLoaded || settingName.isEmpty()))
		return QString();

	// Check tooltip cache first (fast path)
	{
		QMutexLocker cacheLock(&tooltipCacheMutex);
		auto it = tooltipCache.constFind(settingName);
		if (Q_LIKELY(it != tooltipCache.constEnd()))
			return it.value();
	}

	// Get cached theme data
	const TooltipThemeCache& themeCache = getTooltipThemeCache();

	// Pre-allocate QString with estimated size to reduce memory reallocations
	QString tooltip;
	tooltip.reserve(2048);

	// Generate tooltip content with settings lock
	{
		QMutexLocker locker(&settingsMutex);

		// Double-check after acquiring the lock
		if (Q_UNLIKELY(!settingsLoaded || validSettings.isEmpty() || !validSettings.contains(settingName)))
			return QString();

		const SettingInfo& info = validSettings[settingName];

		const QString currentLang = getCurrentLanguage();

		// Use QStringBuilder for efficient string concatenation
		tooltip = QStringLiteral("<table ") % themeCache.tableStyle % QStringLiteral(">");

		// Add setting name row with merged cells
		tooltip += QStringLiteral("<tr><td colspan='2' style='text-align:left; font-weight:bold;'>")
			% settingName % QStringLiteral("</td></tr>");

		// Use fresh language
		addVersionRows(tooltip, info, themeCache.labelStyle);
		processMappingsOptimized(tooltip, info, currentLang, themeCache.labelStyle);
		processContentOptimized(tooltip, info, currentLang, settingName, themeCache.labelStyle);

		tooltip += QStringLiteral("</table>");
	}

	// Store in cache
	if (Q_LIKELY(!tooltip.isEmpty())) {
		QMutexLocker cacheLock(&tooltipCacheMutex);
		tooltipCache.insert(settingName, tooltip);
	}

	return tooltip;
}

bool CIniHighlighter::IsCommentLine(const QString& line)
{
	// Skip lines that start with # or ; (after optional whitespace)
	QString trimmed = line.trimmed();
	return trimmed.startsWith('#') || trimmed.startsWith(';');
}

void CIniHighlighter::highlightBlock(const QString &text)
{
    // First, reset all formatting
    setFormat(0, text.length(), QTextCharFormat());

    // 1. Check if the entire line is a comment
    QRegularExpression commentRegex(R"(^\s*[;#].*)");
    QRegularExpressionMatch commentMatch = commentRegex.match(text);
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
		QRegularExpression keyRegex("^([^\\s=]+)\\s*=");
		QRegularExpressionMatch keyMatch = keyRegex.match(text);
		if (keyMatch.hasMatch()) {
			QString keyName = keyMatch.captured(1);
			int start = keyMatch.capturedStart(1);
			int length = keyName.length();

			if (!validSettings.isEmpty()) { // Only check if list is loaded
				if (validSettings.contains(keyName)) {
					const SettingInfo& info = validSettings[keyName];
					QVersionNumber current = m_currentVersion;
					QVersionNumber renamed = QVersionNumber::fromString(info.renamedVersion);
					QVersionNumber readded = QVersionNumber::fromString(info.readdedVersion);
					QVersionNumber removed = QVersionNumber::fromString(info.removedVersion);
					QVersionNumber added = QVersionNumber::fromString(info.addedVersion);

					if (!info.renamedVersion.isEmpty() && current >= renamed) {
						setFormat(start, length, renamedKeyFormat);
					}
					else if (!info.readdedVersion.isEmpty() && current >= readded) {
						setFormat(start, length, keyFormat);
					}
					else if (!info.removedVersion.isEmpty() && current >= removed) {
						setFormat(start, length, removedKeyFormat);
					}
					else if (current >= added) {
						setFormat(start, length, keyFormat);
					}
					else if (current < added) {
						setFormat(start, length, futureKeyFormat);
					}
					else {
						setFormat(start, length, unknownKeyFormat);
					}
				}
				else {
					setFormat(start, length, unknownKeyFormat); // underline unknown keys
				}
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
    const CIniHighlighter::TooltipStyle& textStyle,
    QString& tooltip)
{
    QStringList typeLabels;

    // Create a temporary collection of all matched keywords with their info
    QList<QPair<QString, QString>> matchedKeywords; // keyword, displayName
    QHash<QString, QString> keywordActions; // keyword -> action

    // Collect all matched keywords and their actions
    for (const KeywordInfoType& keywordInfo : effectiveMappings) {
        if (displayText.contains(keywordInfo.keyword)) {
            matchedKeywords.append(qMakePair(keywordInfo.keyword, keywordInfo.displayName));
            if (!keywordInfo.action.isEmpty()) {
                keywordActions.insert(keywordInfo.keyword, keywordInfo.action);
            }
        }
    }

    // Process each matched keyword to determine if it should be shown
    for (const auto& pair : matchedKeywords) {
        const QString& keyword = pair.first;
        const QString& displayName = pair.second;
        bool isHidden = false;

        // Check if this keyword should be hidden (by itself or others)

        // 1. Check if it hides itself (its action contains itself)
        if (keywordActions.contains(keyword) &&
            keywordActions[keyword].contains(keyword)) {
            isHidden = true;
        }

        // 2. Check if any other keyword's action hides this one
        if (!isHidden) {
            for (auto it = keywordActions.constBegin(); it != keywordActions.constEnd(); ++it) {
                // Skip checking against itself
                if (it.key() == keyword)
                    continue;

                // If any character in another keyword's action matches this keyword, hide it
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

        // Add to display labels if not hidden
        if (!isHidden && !displayName.isEmpty()) {
            typeLabels.append(displayName);
        }
    }

    // Only display if we have labels to show
    if (!typeLabels.isEmpty()) {
        QString typeText = typeLabels.join(" + ");

        // Apply styling to label if specified
        QString styledLabelStyle = labelStyle;
        QString labelStyling = textStyle.toHtmlStyle();
        if (!labelStyling.isEmpty()) {
            // Merge with existing labelStyle
            if (labelStyle.contains("style='")) {
                styledLabelStyle = labelStyle;
                styledLabelStyle.replace("style='", labelStyling.mid(0, labelStyling.length() - 1) + ";");
            }
            else {
                styledLabelStyle += " " + labelStyling;
            }
        }

        tooltip += QStringLiteral("<tr><td ") + styledLabelStyle + QStringLiteral(">") + labelText + QStringLiteral("</td>");

        // Apply styling to value text
        QString valueStyleStr = textStyle.toHtmlStyle();
        if (!valueStyleStr.isEmpty()) {
            tooltip += QStringLiteral("<td ") + valueStyleStr + QStringLiteral(">") + valuePrefix + typeText + QStringLiteral("</td></tr>");
        }
        else {
            tooltip += QStringLiteral("<td>") + valuePrefix + typeText + QStringLiteral("</td></tr>");
        }
    }
}

template void CIniHighlighter::processKeywordMappings<CIniHighlighter::KeywordInfo<CIniHighlighter::KeywordType::Context>>(
	const QString&, const QList<CIniHighlighter::KeywordInfo<CIniHighlighter::KeywordType::Context>>&,
	const QString&, const QString&, const QString&,
	const CIniHighlighter::TooltipStyle&, QString&);

template void CIniHighlighter::processKeywordMappings<CIniHighlighter::KeywordInfo<CIniHighlighter::KeywordType::Category>>(
	const QString&, const QList<CIniHighlighter::KeywordInfo<CIniHighlighter::KeywordType::Category>>&,
	const QString&, const QString&, const QString&,
	const CIniHighlighter::TooltipStyle&, QString&);

template void CIniHighlighter::processKeywordMappings<CIniHighlighter::KeywordInfo<CIniHighlighter::KeywordType::Requirements>>(
	const QString&, const QList<CIniHighlighter::KeywordInfo<CIniHighlighter::KeywordType::Requirements>>&,
	const QString&, const QString&, const QString&,
	const CIniHighlighter::TooltipStyle&, QString&);
