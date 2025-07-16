#include "stdafx.h"
#include "IniHighlighter.h"
#include "../version.h"

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

	// Automatically load settings from SbieSettings.csv in the same directory as SandMan.exe
	QString settingsPath = QCoreApplication::applicationDirPath() + "/SbieSettings.csv";
	loadSettingsCsv(settingsPath);

	// Use version.h macros for default/current version
	QString versionStr = QString("%1.%2.%3").arg(VERSION_MJR).arg(VERSION_MIN).arg(VERSION_REV);
	setCurrentVersion(versionStr); // Set semantic version from macros
}

CIniHighlighter::~CIniHighlighter()
{
}

void CIniHighlighter::setCurrentVersion(const QString& version)
{
	m_currentVersion = QVersionNumber::fromString(version);
}

// Load settings from SbieSettings.csv
void CIniHighlighter::loadSettingsCsv(const QString& filePath)
{
	allowedSettings.clear();
	QFile file(filePath);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(&file);
		bool firstLine = true;
		while (!in.atEnd()) {
			QString line = in.readLine();
			if (firstLine) { firstLine = false; continue; } // skip header
			QStringList parts = line.split(';');
			if (parts.size() >= 5) { // columns: Name, AddedVersion, RemovedVersion, ReaddedVersion, RenamedVersion
				SettingInfo info;

				// Only accept [a-zA-Z0-9_.] in name
				info.name = parts[0].trimmed();
				info.name.remove(QRegularExpression("[^a-zA-Z0-9_.]"));

				// Only accept [0-9.] in version fields, default to "0.0.0" if empty
				auto sanitizeVersion = [](const QString& s, bool defaultZero = false) {
					QString v = s.trimmed();
					v.remove(QRegularExpression("[^0-9.]"));
					QRegularExpression rx("^[0-9]+\\.[0-9]+\\.[0-9]+$"); // Exact x.y.z format
					if (rx.match(v).hasMatch())
						return v;
					return defaultZero ? QString("0.0.0") : QString();
					};

				info.addedVersion = sanitizeVersion(parts[1], true);  // default "0.0.0" if empty
				info.removedVersion = sanitizeVersion(parts[2]);
				info.readdedVersion = sanitizeVersion(parts[3]);
				info.renamedVersion = sanitizeVersion(parts[4]);

				allowedSettings.insert(info.name, info);
			}
		}
		file.close();
		qDebug() << "Loaded" << allowedSettings.size() << "settings from" << filePath;
	}
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

	// 3. Highlight keys based on allowedSettings and currentVersion
	if (m_enableValidation) {
		QRegularExpression keyRegex("^([^\\s=]+)\\s*=");
		QRegularExpressionMatch keyMatch = keyRegex.match(text);
		if (keyMatch.hasMatch()) {
			QString keyName = keyMatch.captured(1);
			int start = keyMatch.capturedStart(1);
			int length = keyName.length();

			if (!allowedSettings.isEmpty()) { // Only check if list is loaded
				if (allowedSettings.contains(keyName)) {
					const SettingInfo& info = allowedSettings[keyName];
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