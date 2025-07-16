#pragma once

#include <QSyntaxHighlighter>
#include <QVersionNumber>

#define INI_WITH_JSON

struct HighlightRule {
	QRegularExpression pattern;
	QTextCharFormat format;
};

struct SettingInfo {
	QString name;
	QString addedVersion;
	QString removedVersion;
	QString readdedVersion;
	QString renamedVersion;
};

class CIniHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
	explicit CIniHighlighter(bool bDarkMode, QTextDocument* parent = nullptr, bool enableValidation = true);
	virtual ~CIniHighlighter();

	// Load allowed settings from a CSV file
	void loadSettingsCsv(const QString& filePath);

	// Set the current semantic version for highlighting
	void setCurrentVersion(const QString& version);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightRule> highlightRules;

    QTextCharFormat sectionFormat;
    QTextCharFormat keyFormat;
    QTextCharFormat valueFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat equalsFormat; // New format for '=' character
    QTextCharFormat valuePrefixFormat; // Format for text before first comma in value
    QTextCharFormat firstCommaFormat;  // Format for the first comma in value
	QTextCharFormat futureKeyFormat;   // Format for future keys
	QTextCharFormat removedKeyFormat;  // Format for removed keys
	QTextCharFormat renamedKeyFormat;  // Format for renamed keys
	QTextCharFormat unknownKeyFormat;  // Format for unknown keys

#ifdef INI_WITH_JSON
    // JSON-specific formats
    QTextCharFormat jsonKeyFormat;
    QTextCharFormat jsonStringFormat;
    QTextCharFormat jsonNumberFormat;
    QTextCharFormat jsonBoolNullFormat;
    QTextCharFormat jsonBracesFormat;
    QTextCharFormat jsonColonFormat;
    QTextCharFormat jsonCommaFormat;

    // JSON highlighting rules
    QVector<HighlightRule> jsonHighlightRules;
#endif

	// Map of allowed settings loaded from CSV file with version info
	QMap<QString, SettingInfo> allowedSettings;

	// Current semantic version
	QVersionNumber m_currentVersion;

	bool m_enableValidation;
};
