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
	QString supersededBy;
	QString category;
	QString context;
	QString syntax;
	QString description;
	QMap<QString, QString> localizedDescriptions;
	QString flags;
};

class CIniHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
	explicit CIniHighlighter(bool bDarkMode, QTextDocument* parent = nullptr, bool enableValidation = true);
	virtual ~CIniHighlighter();

	// Load valid settings from a INI file
	void loadSettingsIni(const QString& filePath);

	// Set the current semantic version for highlighting
	void setCurrentVersion(const QString& version);

	// Get tooltip text for a setting based on its version information
	static QString GetSettingTooltip(const QString& settingName);

	static bool IsSettingsLoaded() { return settingsLoaded; }

	static bool IsCommentLine(const QString& line);

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

	// Settings validation and tooltip handling
	static const QString DEFAULT_SETTINGS_FILE;
	static const QString DEFAULT_VERSION;

	static QVersionNumber s_currentVersion;
	static QVersionNumber getCurrentVersion();
	static QHash<QString, SettingInfo> validSettings;
	static QDateTime lastFileModified;
	static bool settingsLoaded;
	static QMutex settingsMutex;
	static QHash<QString, QString> tooltipCache;
	static QMutex tooltipCacheMutex;

	QVersionNumber m_currentVersion;

	bool m_enableValidation;
};
