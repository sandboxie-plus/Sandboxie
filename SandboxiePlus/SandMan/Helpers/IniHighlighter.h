#pragma once

#include <QSyntaxHighlighter>
#include <QVersionNumber>

#define INI_WITH_JSON

struct HighlightRule {
	QRegularExpression pattern;
	QTextCharFormat format;
};

// Settings validation and tooltip handling
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
	QMap<QString, QString> localizedSyntax;
	QString description;
	QMap<QString, QString> localizedDescriptions;
	QString requirements;
};
// Settings validation and tooltip handling

class CIniHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
	explicit CIniHighlighter(bool bDarkMode, QTextDocument* parent = nullptr, bool enableValidation = true);
	virtual ~CIniHighlighter();

	// Settings validation and tooltip handling
	enum class TooltipMode {
		Disabled = 0,        // Qt::Unchecked - no tooltips
		BasicInfo = 1,       // Qt::PartiallyChecked - setting name + description only
		FullTooltip = 2      // Qt::Checked - complete tooltip with all info
	};

	void loadSettingsIni(const QString& filePath);
	void setCurrentVersion(const QString& version);

	static QString GetSettingTooltip(const QString& settingName);
	static bool IsSettingsLoaded() { return settingsLoaded; }
	static bool IsCommentLine(const QString& line);

	static void ClearLanguageCache();
	static void SetTooltipMode(int checkState);
	static TooltipMode GetTooltipMode();
	static QString getCurrentLanguage();
	// Settings validation and tooltip handling

protected:
    void highlightBlock(const QString &text) override;

private:
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
	QVersionNumber m_currentVersion;
	bool m_enableValidation;

	static const QString DEFAULT_SETTINGS_FILE;
	static const QString DEFAULT_VERSION;

	static QVersionNumber s_currentVersion;
	static QString s_currentLanguage;
	static QMutex s_languageMutex;
	static QHash<QString, SettingInfo> validSettings;
	static QDateTime lastFileModified;
	static bool settingsLoaded;
	static QMutex settingsMutex;
	static QHash<QString, QString> tooltipCache;
	static QMutex tooltipCacheMutex;
	static TooltipMode s_tooltipMode;
	static QMutex s_tooltipModeMutex;


	static QVersionNumber getCurrentVersion();
	static QString normalizeLanguage(const QString& language);
	static QString processPlaceholders(const QString& text, const QString& settingName = QString());
	static QString sanitizeHtmlInput(const QString& input);
	static QString GetBasicSettingTooltip(const QString& settingName);
	static bool isValidForTooltip(const QString& settingName);

	// Optimized helper functions
	static void addVersionRows(QString& tooltip, const SettingInfo& info, const QString& labelStyle);
	static void processMappingsOptimized(QString& tooltip, const SettingInfo& info,
		const QString& currentLang, const QString& labelStyle);
	static void processContentOptimized(QString& tooltip, const SettingInfo& info,
		const QString& currentLang, const QString& settingName,
		const QString& labelStyle);
	static QString processTextLineOptimized(const QString& text, const QString& settingName);
	static QString selectLocalizedContentOptimized(const QString& defaultContent,
		const QMap<QString, QString>& localizedMap,
		const QString& currentLang);

	// Helper function to extract language code from localized keys
	static QString extractLanguageCode(const QString& key, const QString& prefix);

	// Define keyword types
	enum class KeywordType {
		Context,
		Category,
		Requirements
	};

	// Common keyword info structure for keys
	template<KeywordType Type>
	struct KeywordInfo {
		QString keyword;
		QString displayName;
		QString action;
	};

	struct TooltipStyle {
		QString color = "";           // red, green, blue, etc.
		bool bold = false;
		bool italic = false;
		bool underline = false;

		QString toHtmlStyle() const {
			QStringList styles;
			if (!color.isEmpty()) {
				styles << QString("color:%1").arg(color);
			}
			if (bold) {
				styles << "font-weight:bold";
			}
			if (italic) {
				styles << "font-style:italic";
			}
			if (underline) {
				styles << "text-decoration:underline";
			}
			return styles.isEmpty() ? "" : QString("style='%1'").arg(styles.join(";"));
		}
	};

	struct TooltipThemeCache {
		bool darkMode = false;
		QString bgColor;
		QString textColor;
		QString tableStyle;
		QString labelStyle;
		bool valid = false;
	};
	static const TooltipThemeCache& getTooltipThemeCache();

	// Define type aliases for mappings
	template<KeywordType Type>
	using KeywordMappings = QList<KeywordInfo<Type>>;
	template<KeywordType Type>
	using LocalizedKeywordMappings = QMap<QString, KeywordMappings<Type>>;

	// Grouped keyword data structure
	template<KeywordType Type>
	struct KeywordGroup {
		KeywordMappings<Type> mappings;
		LocalizedKeywordMappings<Type> localizedMappings;
		TooltipStyle tooltipStyle;
		
		void clear() {
			mappings.clear();
			localizedMappings.clear();
			tooltipStyle = TooltipStyle();
		}
	};
	
	// Static instances for each keyword type
	static KeywordGroup<KeywordType::Context> contextData;
	static KeywordGroup<KeywordType::Category> categoryData;
	static KeywordGroup<KeywordType::Requirements> requirementsData;

	// Helper method to parse style configuration
	static TooltipStyle parseStyleConfig(const QString& styleConfig);

	// Helper methods for parsing mappings
	template<KeywordType Type>
	static KeywordMappings<Type> parseKeywordMappings(const QString& value);

	// Enhanced processKeywordMappings with styling support
	template<typename KeywordInfoType>
	static void processKeywordMappings(
		const QString& displayText,
		const QList<KeywordInfoType>& effectiveMappings,
		const QString& labelText,
		const QString& labelStyle,
		const QString& valuePrefix,
		const TooltipStyle& textStyle,
		QString& tooltip);
};
