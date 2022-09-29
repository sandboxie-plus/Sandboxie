#pragma once

#include "../mischelpers_global.h"

#ifndef WIN32
MISCHELPERS_EXPORT int vswprintf_l(wchar_t * _String, size_t _Count, const wchar_t * _Format, va_list _Ap);
#endif

MISCHELPERS_EXPORT time_t GetTime();
MISCHELPERS_EXPORT quint64 GetCurTick();


MISCHELPERS_EXPORT quint64 GetRand64();
MISCHELPERS_EXPORT QString GetRand64Str(bool forUrl = true);

MISCHELPERS_EXPORT int	GetRandomInt(int iMin, int iMax);

MISCHELPERS_EXPORT typedef QPair<QString,QString> StrPair;
MISCHELPERS_EXPORT StrPair Split2(const QString& String, QString Separator = "=", bool Back = false);
MISCHELPERS_EXPORT QStringList SplitStr(const QString& String, QString Separator);

typedef MISCHELPERS_EXPORT QMultiMap<QString,QString> TArguments;
TArguments MISCHELPERS_EXPORT GetArguments(const QString& Arguments, QChar Separator = L';', QChar Assigner = L'=', QString* First = NULL, bool bLowerKeys = false, bool bReadEsc = false);

MISCHELPERS_EXPORT QString UnEscape(QString Text);

MISCHELPERS_EXPORT QString FormatSize(quint64 Size, int Precision = 2);
__inline QString FormatSizeEx(quint64 Size, bool bEx) { return bEx && (Size == 0) ? QString() : FormatSize(Size); }
MISCHELPERS_EXPORT QString FormatRate(quint64 Size, int Precision = 2);
__inline QString FormatRateEx(quint64 Size, bool bEx) { return bEx && (Size == 0) ? QString() : FormatRate(Size); }
MISCHELPERS_EXPORT QString FormatUnit(quint64 Size, int Precision = 0);
//MISCHELPERS_EXPORT QString	FormatTime(quint64 Time, bool ms = false);
MISCHELPERS_EXPORT QString	FormatNumber(quint64 Number);
__inline QString FormatNumberEx(quint64 Number, bool bEx) { return bEx && (Number == 0) ? QString() : FormatNumber(Number); }
MISCHELPERS_EXPORT QString	FormatAddress(quint64 Address, int length = 16);


inline bool operator < (const QHostAddress &key1, const QHostAddress &key2)
{
	// Note: toIPv6Address also works for IPv4 addresses
	Q_IPV6ADDR ip1 = key1.toIPv6Address();
	Q_IPV6ADDR ip2 = key2.toIPv6Address();
    return memcmp(&ip1, &ip2, sizeof(Q_IPV6ADDR)) < 0;
}

template <typename T>
QVariantList toVariantList( const QList<T> &list )
{
    QVariantList newList;
    foreach( const T &item, list )
        newList << item;

    return newList;
}

template <typename T>
QList<T> reversed( const QList<T> & in ) {
    QList<T> result;
    result.reserve( in.size() ); // reserve is new in Qt 4.7
    std::reverse_copy( in.begin(), in.end(), std::back_inserter( result ) );
    return result;
}

template <class T>
class CScoped
{
public:
	CScoped(T* Val = NULL)			{m_Val = Val;}
	~CScoped()						{delete m_Val;}

	CScoped<T>& operator=(const CScoped<T>& Scoped)	{ASSERT(0); return *this;} // copying is explicitly forbidden
	CScoped<T>& operator=(T* Val)	{ASSERT(!m_Val); m_Val = Val; return *this;}

	inline T* Val() const			{return m_Val;}
	inline T* &Val()				{return m_Val;}
	inline T* Detache()				{T* Val = m_Val; m_Val = NULL; return Val;}
    inline T* operator->() const	{return m_Val;}
    inline T& operator*() const     {return *m_Val;}
    inline operator T*() const		{return m_Val;}

private:
	T*	m_Val;
};

MISCHELPERS_EXPORT bool ReadFromDevice(QIODevice* dev, char* data, int len, int timeout = 5000);

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} my_rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} my_hsv;

my_hsv MISCHELPERS_EXPORT rgb2hsv(my_rgb in);
my_rgb MISCHELPERS_EXPORT hsv2rgb(my_hsv in);

QRgb MISCHELPERS_EXPORT change_hsv_c(QRgb rgb, float fHue, float fSat, float fVal);
MISCHELPERS_EXPORT void GrayScale (QImage& Image);

MISCHELPERS_EXPORT QIcon MakeNormalAndGrayIcon(QIcon Icon);
MISCHELPERS_EXPORT QIcon MakeActionIcon(const QString& IconFile);
MISCHELPERS_EXPORT QAction* MakeAction(QToolBar* pParent, const QString& IconFile, const QString& Text = "");
MISCHELPERS_EXPORT QMenu* MakeMenu(QMenu* pParent, const QString& Text, const QString& IconFile = "");
MISCHELPERS_EXPORT QAction* MakeAction(QMenu* pParent, const QString& Text, const QString& IconFile = "");
MISCHELPERS_EXPORT QAction* MakeAction(QActionGroup* pGroup, QMenu* pParent, const QString& Text, const QVariant& Data);
MISCHELPERS_EXPORT void SetPaleteTexture(QPalette& palette, QPalette::ColorRole role, const QImage& image);

#ifdef WIN32
MISCHELPERS_EXPORT bool InitConsole(bool bCreateIfNeeded = true);
#endif

MISCHELPERS_EXPORT void SafeShow(QWidget* pWidget);
MISCHELPERS_EXPORT int SafeExec(QDialog* pDialog);

template <typename T>
QSet<T> ListToSet(const QList<T>& qList) { return QSet<QString>(qList.begin(), qList.end()); }

template <typename T>
QList<T> SetToList(const QSet<T>& qSet) { return QList<QString>(qSet.begin(), qSet.end()); }