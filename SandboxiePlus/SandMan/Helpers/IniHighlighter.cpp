#include "stdafx.h"
#include "IniHighlighter.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../version.h"

const QString CIniHighlighter::DEFAULT_SETTINGS_FILE = "SbieSettings.ini";
const QString CIniHighlighter::DEFAULT_VERSION = "0.0.0";

QVersionNumber CIniHighlighter::s_currentVersion;
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
QHash<QString, SettingInfo> CIniHighlighter::validSettings;
QDateTime CIniHighlighter::lastFileModified;
bool CIniHighlighter::settingsLoaded = false;
QMutex CIniHighlighter::settingsMutex;
QHash<QString, QString> CIniHighlighter::tooltipCache;
QMutex CIniHighlighter::tooltipCacheMutex;

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

void CIniHighlighter::setCurrentVersion(const QString& version)
{
	m_currentVersion = QVersionNumber::fromString(version);
}

// Load settings from SbieSettings.ini
void CIniHighlighter::loadSettingsIni(const QString& filePath)
{
	QMutexLocker locker(&settingsMutex); // Lock during the entire load operation

	// Clear tooltip cache when settings are reloaded
	{
		QMutexLocker cacheLock(&tooltipCacheMutex);
		tooltipCache.clear();
	}

	QFile file(filePath);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		validSettings.clear();
		QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
		// Qt5: use setCodec
		in.setCodec("UTF-8");
#endif

		QString currentSection;
		SettingInfo currentInfo;
		bool inSection = false;

		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();

			// Skip empty lines and comments
			if (line.isEmpty() || line.startsWith(';') || line.startsWith('#'))
				continue;

			// Check if this is a section header [SectionName]
			QRegularExpression sectionRegex(R"(^\[([^\]]+)\]\s*$)");
			QRegularExpressionMatch sectionMatch = sectionRegex.match(line);

			if (sectionMatch.hasMatch()) {
				// If we were already processing a section, save it
				if (inSection && !currentSection.isEmpty()) {
					currentInfo.name = currentSection;
					validSettings.insert(currentInfo.name, currentInfo);
				}

				// Start a new section
				currentSection = sectionMatch.captured(1).trimmed();
				currentInfo = SettingInfo(); // Reset info for new section
				inSection = true;
				continue;
			}

			// If not in a section yet, skip
			if (!inSection)
				continue;

			// Process key=value pairs
			int equalsPos = line.indexOf('=');
			if (equalsPos > 0) {
				QString key = line.left(equalsPos).trimmed();
				QString value = line.mid(equalsPos + 1).trimmed();

				// Helper function to sanitize version strings
				auto sanitizeVersion = [this](const QString& s, bool defaultZero = false) {
					QString v = s.trimmed();
					v.remove(QRegularExpression("[^0-9.]"));
					QRegularExpression rx("^[0-9]+\\.[0-9]+\\.[0-9]+$"); // Exact x.y.z format
					if (rx.match(v).hasMatch())
						return v;
					return defaultZero ? QString(DEFAULT_VERSION) : QString();
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
				else if (key.compare("Syntax", Qt::CaseInsensitive) == 0) {
					// Replace \n escape sequences with actual newlines
					value.replace("\\n", "\n");
					currentInfo.syntax = value;
				}
				else if (key.compare("Description", Qt::CaseInsensitive) == 0) {
					// Handle main description
					// Replace \n escape sequences with actual newlines
					value.replace("\\n", "\n");

					// If there's already content in the description field,
					// append this new value with a newline separator
					if (!currentInfo.description.isEmpty()) {
						currentInfo.description += "\n" + value;
					}
					else {
						currentInfo.description = value;
					}
				}
				// Handle localized descriptions (Description_XX=value)
				else if (key.startsWith("Description_", Qt::CaseInsensitive)) {
					// Modified regex to capture any language code after Description_
					QRegularExpression langRegex("^Description_(.+)$", QRegularExpression::CaseInsensitiveOption);
					QRegularExpressionMatch langMatch = langRegex.match(key);

					if (langMatch.hasMatch()) {
						// Get the language code and normalize to lowercase for consistent matching
						QString langCode = langMatch.captured(1).toLower();
						QString localizedDesc = value;

						// Replace \n escape sequences with actual newlines
						localizedDesc.replace("\\n", "\n");

						// If there's already content for this language, append with newline
						if (currentInfo.localizedDescriptions.contains(langCode)) {
							currentInfo.localizedDescriptions[langCode] += "\n" + localizedDesc;
						}
						else {
							currentInfo.localizedDescriptions.insert(langCode, localizedDesc);
						}
					}
				}
				else if (key.compare("Flags", Qt::CaseInsensitive) == 0)
					currentInfo.flags = value.toLower().trimmed();
			}
		}

		// Don't forget to save the last section
		if (inSection && !currentSection.isEmpty()) {
			currentInfo.name = currentSection;
			// Ensure name contains only valid characters
			currentInfo.name.remove(QRegularExpression("[^a-zA-Z0-9_.]"));
			validSettings.insert(currentInfo.name, currentInfo);
		}

		file.close();

		// Update the cache status after successful load
		lastFileModified = QFileInfo(filePath).lastModified();
		settingsLoaded = true;

		qDebug() << "[validSettings] Successfully loaded" << validSettings.size() << "settings from" << filePath;
	}
	else {
		// File couldn't be opened - log the error
		qWarning() << "[validSettings] Failed to load settings file:" << filePath << "Error:" << file.errorString();

		// Keep settings loaded flag false so we try again next time
		settingsLoaded = false;
	}
}

QString CIniHighlighter::GetSettingTooltip(const QString& settingName)
{
	// Early safety check - if settings aren't loaded yet, don't try to access them
	if (!settingsLoaded || settingName.isEmpty())
		return QString();

	// Check tooltip cache first (fast path)
	{
		QMutexLocker cacheLock(&tooltipCacheMutex);
		QHash<QString, QString>::const_iterator it = tooltipCache.find(settingName);
		if (it != tooltipCache.end())
			return it.value();
	}

	// Generate tooltip content (requires settings lock)
	QString tooltip;
	{
		QMutexLocker locker(&settingsMutex); // Thread-safe access to shared data

		// Double-check after acquiring the lock
		if (!settingsLoaded || validSettings.isEmpty() || !validSettings.contains(settingName))
			return QString();

		const SettingInfo& info = validSettings[settingName];

		// Define the table style once
		const QString tableStyle = "style='border:none; white-space:nowrap;'";

		// Define the cell style once to avoid repetition
		const QString labelStyle = "style='text-align:left; padding-right:8px;'";
		const QString valuePrefix = ": ";

		// Create a table structure for aligned label-value pairs
		tooltip = "<table " + tableStyle + ">";

		// Add setting name row with merged cells - make the name bold and centered
		tooltip += "<tr><td colspan='2' style='text-align:left; font-weight:bold;'>" + settingName + "</td></tr>";

		// Add all available fields as rows with aligned labels and values
		if (!info.addedVersion.isEmpty() && info.addedVersion != DEFAULT_VERSION) {
			tooltip += "<tr><td " + labelStyle + ">" + tr("Added in version") + "</td>";
			tooltip += "<td>" + valuePrefix + info.addedVersion + "</td></tr>";
		}

		if (!info.removedVersion.isEmpty()) {
			tooltip += "<tr><td " + labelStyle + ">" + tr("Removed in version") + "</td>";
			tooltip += "<td>" + valuePrefix + info.removedVersion + "</td></tr>";
		}

		if (!info.readdedVersion.isEmpty()) {
			tooltip += "<tr><td " + labelStyle + ">" + tr("Re-added in version") + "</td>";
			tooltip += "<td>" + valuePrefix + info.readdedVersion + "</td></tr>";
		}

		if (!info.renamedVersion.isEmpty()) {
			tooltip += "<tr><td " + labelStyle + ">" + tr("Renamed in version") + "</td>";
			tooltip += "<td>" + valuePrefix + info.renamedVersion + "</td></tr>";
		}

		if (!info.supersededBy.isEmpty()) {
			tooltip += "<tr><td " + labelStyle + ">" + tr("Superseded by") + "</td>";
			tooltip += "<td>" + valuePrefix + info.supersededBy + "</td></tr>";
		}

		if (!info.category.isEmpty()) {
			QString categoryDisplay = info.category;

			// Check which category types are present
			bool hasAdvanced = categoryDisplay.contains('a');
			bool hasDebug= categoryDisplay.contains('d');
			bool hasFile = categoryDisplay.contains('f');
			bool hasGeneral = categoryDisplay.contains('g');
			bool hasNetwork = categoryDisplay.contains('n');
			bool hasProgramControl = categoryDisplay.contains('p');
			bool hasResourceAccess = categoryDisplay.contains('r');
			bool hasSecurity = categoryDisplay.contains('s');
			bool hasTemplate = categoryDisplay.contains('t');
			bool hasVarious = categoryDisplay.contains('v');

			// Build the appropriate category type string
			QStringList categoryLabels;
			if (hasAdvanced)
				categoryLabels.append(tr("Advanced"));
			if (hasDebug)
				categoryLabels.append(tr("Debug"));
			if (hasFile)
				categoryLabels.append(tr("File"));
			if (hasGeneral)
				categoryLabels.append(tr("General"));
			if (hasNetwork)
				categoryLabels.append(tr("Network"));
			if (hasProgramControl)
				categoryLabels.append(tr("Program Control"));
			if (hasResourceAccess)
				categoryLabels.append(tr("Resource Access"));
			if (hasSecurity)
				categoryLabels.append(tr("Security"));
			if (hasTemplate)
				categoryLabels.append(tr("Template"));
			if (hasVarious)
				categoryLabels.append(tr("Various"));

			QString categoryType = categoryLabels.join(" + ");

			// Only display descriptive categories if we recognized some flags
			if (!categoryType.isEmpty()) {
				tooltip += "<tr><td " + labelStyle + ">" + tr("Category") + "</td>";
				tooltip += "<td>" + valuePrefix + categoryType + "</td></tr>";
			}
			// Fall back to raw value if no recognized categories
			else {
				tooltip += "<tr><td " + labelStyle + ">" + tr("Category") + "</td>";
				tooltip += "<td>" + valuePrefix + categoryDisplay + "</td></tr>";
			}
		}

		if (!info.context.isEmpty()) {
			QString contextDisplay = info.context;

			// Check which context types are present
			bool hasGlobal = contextDisplay.contains('g');
			bool hasTemplate = contextDisplay.contains('t');
			bool hasSandbox = contextDisplay.contains('s');
			bool hasUser = contextDisplay.contains('u');

			// Skip showing context if all types are present
			if (!(hasGlobal && hasTemplate && hasSandbox && hasUser)) {
				// Build the appropriate context type string
				QStringList typeLabels;
				if (hasGlobal)
					typeLabels.append(tr("Global"));
				if (hasTemplate)
					typeLabels.append(tr("Template"));
				if (hasSandbox)
					typeLabels.append(tr("Per Sandbox"));
				if (hasUser)
					typeLabels.append(tr("User Settings"));

				QString contextType = typeLabels.join(" + ");

				// Only display the tooltip row if we have recognized context types
				if (!contextType.isEmpty()) {
					tooltip += "<tr><td " + labelStyle + ">" + tr("Context") + "</td>";
					tooltip += "<td>" + valuePrefix + "<i>" + contextType + "</i></td></tr>";
				}
			}
		}

		if (!info.syntax.isEmpty()) {
			// Handle the syntax to preserve its own line breaks
			QStringList syntaxLines = info.syntax.split('\n');
			tooltip += "<tr><td " + labelStyle + " style='vertical-align:top;'>" + tr("Syntax") + "</td>";
			tooltip += "<td>" + valuePrefix + syntaxLines[0];

			// Add additional syntax lines with proper indentation
			for (int i = 1; i < syntaxLines.size(); i++) {
				tooltip += "<br>&nbsp;&nbsp;" + syntaxLines[i];
			}
			tooltip += "</td></tr>";
		}

		// Get the current UI language from configuration
		QString currentLang = theConf->GetString("Options/UiLanguage");

		// Handle special case "native" (use English)
		if (currentLang.compare("native", Qt::CaseInsensitive) == 0) {
			currentLang = "en";
		}

		// If no language is set, determine from system locale
		if (currentLang.isEmpty()) {
			currentLang = QLocale::system().name();
		}

		currentLang = currentLang.toLower();

		// Choose description based on language
		QString description;

		// First try exact match
		if (info.localizedDescriptions.contains(currentLang)) {
			description = info.localizedDescriptions[currentLang];
		}
		// Try with just the language part (if it's a locale with country code like "en_US")
		else if (currentLang.contains('_') &&
			info.localizedDescriptions.contains(currentLang.left(currentLang.indexOf('_')))) {
			description = info.localizedDescriptions[currentLang.left(currentLang.indexOf('_'))];
		}
		// Fall back to default description
		else {
			description = info.description;
		}

		if (!description.isEmpty()) {
			// Handle the description to preserve its own line breaks
			QStringList descLines = description.split('\n');
			tooltip += "<tr><td " + labelStyle + " style='vertical-align:top;'>" + tr("Description") + "</td>";
			tooltip += "<td>" + valuePrefix + descLines[0];

			// Add additional description lines with proper indentation
			for (int i = 1; i < descLines.size(); i++) {
				tooltip += "<br>&nbsp;&nbsp;" + descLines[i];
			}
			tooltip += "</td></tr>";
		}

		if (!info.flags.isEmpty()) {
			// Special handling: parse all certificate requirements
			bool hasStandard = info.flags.contains("s");
			bool hasAdvanced = info.flags.contains("e");
			bool hasInsider = info.flags.contains("i");

			// Handle certificate requirements (standard and advanced)
			if (hasStandard || hasAdvanced) {
				QStringList certRequirements;

				if (hasStandard)
					certRequirements.append(tr("Supporter Certificate"));

				if (hasAdvanced)
					certRequirements.append(tr("Advanced Encryption Pack"));

				QString certMessage = certRequirements.size() > 1
					? tr("Requires: %1").arg(certRequirements.join(tr(" + ")))
					: (hasAdvanced ? tr("Requires an Advanced Encryption Pack") : tr("Requires a Supporter Certificate"));

				tooltip += "<tr><td " + labelStyle + " style='color:red;'>" + tr("Certificate") + "</td>";
				tooltip += "<td style='color:red;'>" + valuePrefix + certMessage + "</td></tr>";
			}

			// Handle insider build requirement (i) on a separate line
			if (hasInsider) {
				tooltip += "<tr><td " + labelStyle + " style='color:red;'>" + tr("Build") + "</td>";
				tooltip += "<td style='color:red;'>" + valuePrefix + tr("Requires an Insider Build") + "</td></tr>";
			}
		}

		tooltip += "</table>";
	}

	// Store the generated tooltip in cache
	if (!tooltip.isEmpty()) {
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