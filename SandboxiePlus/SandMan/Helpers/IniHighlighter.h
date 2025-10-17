#pragma once

#include <QSyntaxHighlighter>
#include <QVersionNumber>
#include <QCompleter>
#include <QStringListModel>

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
	QMap<QString, QString> localizedSyntax;
	QString description;
	QMap<QString, QString> localizedDescriptions;
	QString requirements;
};

class CIniHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
	explicit CIniHighlighter(bool bDarkMode, QTextDocument* parent = nullptr, bool enableValidation = true);
	virtual ~CIniHighlighter();

	// Settings validation, tooltip handling and auto completion
	enum class TooltipMode {
		Disabled = 0,        // Qt::Unchecked - no tooltips
		BasicInfo = 1,       // Qt::PartiallyChecked - setting name + description only
		FullTooltip = 2      // Qt::Checked - complete tooltip with all info
	};

	void loadSettingsIni(const QString& filePath);
	void applyUserIniOverrides(const QString& masterVersion, const QString& userIniPath);
	void setCurrentVersion(const QString& version);

	static QString BuildPopupTooltip(const QString& settingName, bool basic);
	static QString BuildTooltipCore(const QString& settingName, bool includeMappings, bool includeContent, int reserveSize = 2048, bool preferDescriptionFirst = true);
	static QString GetBasicSettingTooltip(const QString& settingName);
	static QString GetSettingTooltip(const QString& settingName);
	static QString GetSettingTooltipForPopup(const QString& settingName);
	static bool IsSettingsLoaded() { return settingsLoaded; }
	static bool IsCommentLine(const QString& line);

	static void ClearLanguageCache();
	static void ClearHideConfCache();
	static void ClearThemeCache();
	static void SetTooltipMode(int checkState);
	static TooltipMode GetTooltipMode();
	static QString getCurrentLanguage();

	// Autocompletion support
	static QStringList GetCompletionCandidates();
	static QString FindCaseCorrectedKey(const QString& wrongKey);
	static bool IsValidKey(const QString& keyName);
	static bool IsKeyHiddenFromPopup(const QString& keyName);

	static QHash<QString, QString> hideConfRules; // setting -> action
	static QHash<QString, QString> hideConfExclusions; // setting -> action
	static QMutex hideConfMutex;

	static bool IsKeyHiddenFromContext(const QString& keyName, char context);
	static bool IsValidTooltipContext(const QString& hoveredText);

	static int getMaxSettingNameLengthOrDefault();
	static int getMinSettingNameLengthOrDefault();

	static void MarkSettingsDirty();
	static void MarkUserSettingsDirty();

	static QString s_tooltipBgColorDark, s_tooltipBgColorLight;
	static QString s_tooltipTextColorDark, s_tooltipTextColorLight;

    // End Settings validation, tooltip handling and auto completion

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

	// Settings validation, tooltip handling and auto completion
	QVersionNumber m_currentVersion;
	bool m_enableValidation;

	static const QString DEFAULT_SETTINGS_FILE;
	static const QString DEFAULT_VERSION;

	static QVersionNumber s_currentVersion;
	static QString s_currentLanguage;
	static QMutex s_languageMutex;
	static QHash<QString, SettingInfo> validSettings;
	static QDateTime lastFileModified;
	static QDateTime lastUserFileModified;
	static bool settingsLoaded;
	static bool userIniLoaded;
	static QMutex settingsMutex;
	static QMutex userSettingsMutex;
	static QString s_masterVersion;
	static QHash<QString, QString> tooltipCache;
	static QMutex tooltipCacheMutex;
	static TooltipMode s_tooltipMode;
	static QMutex s_tooltipModeMutex;

	static QVersionNumber getCurrentVersion();
	static QString normalizeLanguage(const QString& language);
	static QString sanitizeHtmlInput(const QString& input);
	static bool isValidForTooltip(const QString& settingName);
	
	static QString sanitizeVersion(const QString& versionString, bool useDefaultOnInvalid = false);
	static QString mergeHtmlStyles(const QString& baseStyle, const QString& additionalStyle);
	static QTextCharFormat determineKeyFormat(const SettingInfo& info, const QVersionNumber& currentVersion,
		const QTextCharFormat& keyFormat, const QTextCharFormat& futureKeyFormat, 
		const QTextCharFormat& removedKeyFormat, const QTextCharFormat& renamedKeyFormat,
		const QTextCharFormat& unknownKeyFormat);
	
	// Generic iterator helper template
	template<typename ContainerType, typename FunctionType>
	static void processContainer(const ContainerType& container, FunctionType&& func);
	
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

	static QString extractLanguageCode(const QString& key, const QString& prefix);

	enum class KeywordType {
		Context,
		Category,
		Requirements
	};

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
		QString alignment;           // "left", "center", "right", or empty

		QString toHtmlStyle() const;
	};

    struct TooltipCellStyles {
        TooltipStyle left;
        TooltipStyle right;
    };

    static TooltipCellStyles parseStyleConfig(const QString& styleConfig);

    struct TooltipThemeCache {
        bool darkMode = false;
        QString bgColor;
        QString textColor;
        QString tableStyle;
        QString labelStyle;
        bool valid = false;
    };
    static const TooltipThemeCache& getTooltipThemeCache();

    template<KeywordType Type>
    using KeywordMappings = QList<KeywordInfo<Type>>;
    template<KeywordType Type>
    using LocalizedKeywordMappings = QMap<QString, KeywordMappings<Type>>;

    template<KeywordType Type>
    struct KeywordGroup {
        KeywordMappings<Type> mappings;
        LocalizedKeywordMappings<Type> localizedMappings;
        TooltipCellStyles tooltipStyle;
        
        void clear() {
            mappings.clear();
            localizedMappings.clear();
            tooltipStyle = TooltipCellStyles();
        }
    };
    
    static KeywordGroup<KeywordType::Context> contextData;
    static KeywordGroup<KeywordType::Category> categoryData;
    static KeywordGroup<KeywordType::Requirements> requirementsData;

    static QMap<QString, TooltipCellStyles> genericStyles;
    static TooltipCellStyles getGenericStyles(const QString& rowType);

	template<KeywordType Type>
	static KeywordMappings<Type> parseKeywordMappings(const QString& value);

	template<KeywordType Type>
	static KeywordMappings<Type> getEffectiveMappingsWithActionFallback(
		const KeywordGroup<Type>& data, const QString& currentLang);

	template<typename KeywordInfoType>
	static void processKeywordMappings(
		const QString& displayText,
		const QList<KeywordInfoType>& effectiveMappings,
		const QString& labelText,
		const QString& labelStyle,
		const QString& valuePrefix,
		const TooltipCellStyles& cellStyles,
		QString& tooltip);

    template<KeywordType Type>
    static bool processConfigKeyword(const QString& key, const QString& value, 
                                   const QString& baseKey, 
                                   KeywordGroup<Type>& keywordGroup);

	// Unified tooltip row generation
	static void appendGenericTooltipRow(QString& tooltip, const QString& label, const QString& value,
		const QString& labelStyle, const QString& valuePrefix = ": ",
		const TooltipCellStyles& cellStyles = TooltipCellStyles{});
	
	static void appendMultiLineTooltipRow(QString& tooltip, const QString& label, 
		const QStringList& lines, const QString& labelStyle, const QString& valuePrefix,
		const QString& settingName, bool applySpecialFormatting = false);

    static void appendTableRowForContent(QString& tooltip, const QString& label, const QString& content, const QString& labelStyle, const QString& valuePrefix, const QString& settingName, bool isSyntax);

    static QString getOrSetTooltipCache(const QString& cacheKey, const std::function<QString()>& generator);

	static void parseHideConfRules(const QString& value, QHash<QString, QString>& rules);
	static bool matchesWildcard(const QString& pattern, const QString& text);
	static QString convertWildcardToRegex(const QString& wildcard);

	static int s_maxSettingNameLength;
	static bool s_maxSettingNameLengthValid;
	static int s_minSettingNameLength;
	static bool s_minSettingNameLengthValid;

	void reloadSettingsIniIfNeeded(const QString& userIniPath, const QFileInfo& userFileInfo);
	void reloadUserIniIfNeeded(const QString& userIniPath, const QFileInfo& userFileInfo);

	static bool settingsDirty;
	static bool userSettingsDirty;
	// End Settings validation, tooltip handling and auto completion
};
