#include "stdafx.h"
#include "Common.h"

#ifdef USE_OPENSSL
#include <openssl/rand.h>
#endif

#ifndef WIN32 // vswprintf
#include <stdio.h>
#include <stdarg.h>


int vswprintf_l(wchar_t * _String, size_t _Count, const wchar_t * _Format, va_list _Ap)
{
	wchar_t _Format_l[1025];
	ASSERT(wcslen(_Format) < 1024);
	wcscpy(_Format_l, _Format);

	for(int i=0; i<wcslen(_Format_l); i++)
	{
		if(_Format_l[i] == L'%')
		{
			switch(_Format_l[i+1])
			{
				case L's':	_Format_l[i+1] = 'S'; break;
				case L'S':	_Format_l[i+1] = 's'; break;
			}
		}
	}

	return vswprintf(_String, _Count, _Format_l, _Ap);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Time Functions
// 

time_t GetTime()
{
	QDateTime dateTime = QDateTime::currentDateTime();
	time_t time = dateTime.toSecsSinceEpoch(); // returns time in seconds (since 1970-01-01T00:00:00) in UTC !
	return time;
}

struct SCurTick
{
	SCurTick()	{Timer.start();}
	quint64 Get(){return Timer.elapsed();}
	QElapsedTimer Timer;
}	g_CurTick;

quint64 GetCurTick()
{
	return g_CurTick.Get();
}

QString UnEscape(QString Text)
{
	QString Value;
	bool bEsc = false;
	for(int i = 0; i < Text.size(); i++)
	{
		QChar Char = Text.at(i);
		if(bEsc)
		{
			switch(Char.unicode())
			{
				case L'\\':	Value += L'\\';	break;
				case L'\'':	Value += L'\'';	break;
				case L'\"':	Value += L'\"';	break;
				case L'a':	Value += L'\a';	break;
				case L'b':	Value += L'\b';	break;
				case L'f':	Value += L'\f';	break;
				case L'n':	Value += L'\n';	break;
				case L'r':	Value += L'\r';	break;
				case L't':	Value += L'\t';	break;
				case L'v':	Value += L'\v';	break;
				default:	Value += Char.unicode();break;
			}
			bEsc = false;
		}
		else if(Char == L'\\')
			bEsc = true;
		else
			Value += Char;
	}	
	return Value;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Other Functions
// 

quint64 GetRand64()
{
	quint64 Rand64;
#ifdef USE_OPENSSL
	int Ret = RAND_bytes((byte*)&Rand64, sizeof(quint64));
	ASSERT(Ret == 1); // An error occurs if the PRNG has not been seeded with enough randomness to ensure an unpredictable byte sequence.
#else
	//CryptoPP::AutoSeededRandomPool rng;
	//rng.GenerateBlock((byte*)&Rand64, sizeof(quint64));

	Rand64 = QRandomGenerator::system()->generate64();
#endif
	return Rand64;
}

QString GetRand64Str(bool forUrl)
{
	quint64 Rand64 = GetRand64();
	QString sRand64 = QByteArray((char*)&Rand64,sizeof(quint64)).toBase64();
	if(forUrl)
		sRand64.replace("+","-").replace("/","_");
	return sRand64.replace("=","");
}


int	GetRandomInt(int iMin, int iMax)
{
	return QRandomGenerator::system()->bounded(iMin, iMax);
}

StrPair Split2(const QString& String, QString Separator, bool Back)
{
	int Sep = Back ? String.lastIndexOf(Separator) : String.indexOf(Separator);
	if(Sep != -1)
		return qMakePair(String.left(Sep).trimmed(), String.mid(Sep+Separator.length()).trimmed());
	return qMakePair(String.trimmed(), QString());
}

QStringList SplitStr(const QString& String, QString Separator)
{
	QStringList List = String.split(Separator);
	for(int i=0; i < List.count(); i++)
	{
		List[i] = List[i].trimmed();
		if(List[i].isEmpty())
			List.removeAt(i--);
	}
	return List;
}

TArguments GetArguments(const QString& Arguments, QChar Separator, QChar Assigner, QString* First, bool bLowerKeys, bool bReadEsc)
{
	TArguments ArgumentList;

	bool bReadValue = false;
	QString Name;
	QString Value;
	QChar Prime = L'\0';
	bool bEsc = false;
	for(int i = 0; i < Arguments.size(); i++)
	{
		QChar Char = Arguments.at(i);
		
		if(Prime != L'\0') // inside a string
		{
			if(bEsc) // ESC sequence handling
			{
				switch(Char.unicode())
				{
					case L'\\':	Value += L'\\';	break;
					case L'\'':	Value += L'\'';	break;
					case L'\"':	Value += L'\"';	break;
					case L'a':	Value += L'\a';	break;
					case L'b':	Value += L'\b';	break;
					case L'f':	Value += L'\f';	break;
					case L'n':	Value += L'\n';	break;
					case L'r':	Value += L'\r';	break;
					case L't':	Value += L'\t';	break;
					case L'v':	Value += L'\v';	break;
					default:	Value += L'?';	break;
				}
				bEsc = false;
			}
			else if(bReadEsc && Char == L'\\')
				bEsc = true;
			else if(Char == Prime) // end of the string
				Prime = L'\0';
			else
			{
				if(bReadValue)
					Value += Char;
				else
					Name += Char;
			}
			continue;
		}
		else if(Char == L'"' || Char == L'\'') // begin of a string
		{
			Prime = Char;
			continue;
		}

		if(/*Char == L' ' ||*/ Char == L'\t')
			continue;

		if(!bReadValue) // reading argument name, or value for default argument
		{
			if(Char == Separator)
			{
				if(First) {*First = Name; First = NULL;}
				else ArgumentList.insertMulti("",Name.trimmed());
				Name.clear();
			}
			else if(Char == Assigner)
				bReadValue = true;
			else
				Name += Char;
		}
		else
		{
			if(Char == Separator)
			{
				if (bLowerKeys) Name = Name.toLower();
				ArgumentList.insertMulti(Name.trimmed(),Value.trimmed());
				//if(First) {*First = Name; First = NULL;}
				Name.clear();
				Value.clear();
				bReadValue = false;
			}
			else
				Value += Char;
		}
	}

	if(!Name.isEmpty())
	{
		if(bReadValue)
		{
			if (bLowerKeys) Name = Name.toLower();
			ArgumentList.insertMulti(Name.trimmed(),Value.trimmed());
			//if (First) { *First = Name; }
		}
		else
		{
			if (First) { *First = Name; }
			else ArgumentList.insertMulti("", Name.trimmed());
		}
	}

	return ArgumentList;
}

QString FormatSize(quint64 Size, int Precision)
{
	double Div;
	if(Size > (quint64)(Div = 1.0*1024*1024*1024*1024*1024*1024))
		return QString::number(double(Size)/Div, 'f', Precision) + " EB";
	if(Size > (quint64)(Div = 1.0*1024*1024*1024*1024*1024))
		return QString::number(double(Size)/Div, 'f', Precision) + " PB";
	if(Size > (quint64)(Div = 1.0*1024*1024*1024*1024))
		return QString::number(double(Size)/Div, 'f', Precision) + " TB";
	if(Size > (quint64)(Div = 1.0*1024*1024*1024))
		return QString::number(double(Size)/Div, 'f', Precision) + " GB";
	if(Size > (quint64)(Div = 1.0*1024*1024))
		return QString::number(double(Size)/Div, 'f', Precision) + " MB";
	if(Size > (quint64)(Div = 1.0*1024))
		return QString::number(double(Size)/Div, 'f', Precision) + " KB";
	return QString::number(double(Size)) + "B";
}

QString FormatRate(quint64 Size, int Precision)
{
	return FormatSize(Size, Precision) + "/s";
}

QString FormatUnit(quint64 Size, int Precision)
{
	double Div;
	if(Size > (quint64)(Div = 1.0*1000*1000*1000*1000*1000*1000))
		return QString::number(double(Size)/Div, 'f', Precision) + " E";
	if(Size > (quint64)(Div = 1.0*1000*1000*1000*1000*1000))
		return QString::number(double(Size)/Div, 'f', Precision) + " P";
	if(Size > (quint64)(Div = 1.0*1000*1000*1000*1000))
		return QString::number(double(Size)/Div, 'f', Precision) + " T";
	if(Size > (quint64)(Div = 1.0*1000*1000*1000))
		return QString::number(double(Size)/Div, 'f', Precision) + " G";
	if(Size > (quint64)(Div = 1.0*1000*1000))
		return QString::number(double(Size)/Div, 'f', Precision) + " M";
	if(Size > (quint64)(Div = 1.0*1000))
		return QString::number(double(Size)/Div, 'f', Precision) + " K";
	return QString::number(double(Size));
}


//QString FormatTime(quint64 Time, bool ms)
//{
//	int milliseconds = 0;
//	if (ms) {
//		milliseconds = Time % 1000;
//		Time /= 1000;
//	}
//	int seconds = Time % 60;
//	Time /= 60;
//	int minutes = Time % 60;
//	Time /= 60;
//	int hours = Time % 24;
//	int days = Time / 24;
//	if(ms && (minutes == 0) && (hours == 0) && (days == 0))
//		return QString().sprintf("%02d.%04d", seconds, milliseconds);
//	if((hours == 0) && (days == 0))
//		return QString().sprintf("%02d:%02d", minutes, seconds);
//	if (days == 0)
//		return QString().sprintf("%02d:%02d:%02d", hours, minutes, seconds);
//	return QString().sprintf("%dd%02d:%02d:%02d", days, hours, minutes, seconds);
//}

QString	FormatNumber(quint64 Number)
{
	QString String = QString::number(Number);
	for (int i = String.length() - 3; i > 0; i -= 3)
		String.insert(i, QString::fromWCharArray(L"\u202F")); // L"\u2009"
	return String;
}

QString	FormatAddress(quint64 Address, int length)
{
	return "0x" + QString::number(Address, 16).rightJustified(length, '0');
}

bool ReadFromDevice(QIODevice* dev, char* data, int len, int timeout)
{
	while (dev->bytesAvailable() < len) {
		if (!dev->waitForReadyRead(timeout))
			return false;
	}
	return dev->read(data, len) == len;
}

my_hsv rgb2hsv(my_rgb in)
{
    my_hsv      out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}

my_rgb hsv2rgb(my_hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    my_rgb      out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}

uint8_t clamp(float v) //define a function to bound and round the input float value to 0-255
{
    if (v < 0)
        return 0;
    if (v > 255)
        return 255;
    return (uint8_t)v;
}

// http://beesbuzz.biz/code/16-hsv-color-transforms
QRgb change_hsv_c(QRgb rgb, float fHue, float fSat, float fVal)
{
	float in_r = qRed(rgb);
	float in_g = qGreen(rgb);
	float in_b = qBlue(rgb);

    const float cosA = fSat*cos(fHue*3.14159265f/180); //convert degrees to radians
    const float sinA = fSat*sin(fHue*3.14159265f/180); //convert degrees to radians

    //helpers for faster calc //first 2 could actually be precomputed
    const float aThird = 1.0f/3.0f;
    const float rootThird = sqrtf(aThird);
    const float oneMinusCosA = (1.0f - cosA);
    const float aThirdOfOneMinusCosA = aThird * oneMinusCosA;
    const float rootThirdTimesSinA =  rootThird * sinA;
    const float plus = aThirdOfOneMinusCosA +rootThirdTimesSinA;
    const float minus = aThirdOfOneMinusCosA -rootThirdTimesSinA;

    //calculate the rotation matrix
    float matrix[3][3] = {
        {   cosA + oneMinusCosA / 3.0f  , minus                         , plus                          },
        {   plus                        , cosA + aThirdOfOneMinusCosA   , minus                         },
        {   minus                       , plus                          , cosA + aThirdOfOneMinusCosA   }
    };

    //Use the rotation matrix to convert the RGB directly
    float out_r = clamp((in_r*matrix[0][0] + in_g*matrix[0][1] + in_b*matrix[0][2])*fVal);
    float out_g = clamp((in_r*matrix[1][0] + in_g*matrix[1][1] + in_b*matrix[1][2])*fVal);
    float out_b = clamp((in_r*matrix[2][0] + in_g*matrix[2][1] + in_b*matrix[2][2])*fVal);
    return qRgb(out_r, out_g, out_b);
}

void GrayScale (QImage& Image)
{
	if (Image.depth () == 32)
	{
		uchar* r = (Image.bits ());
		uchar* g = (Image.bits () + 1);
		uchar* b = (Image.bits () + 2);

		uchar* end = (Image.bits() + Image.sizeInBytes());
		while (r != end)
		{
			*r = *g = *b = (((*r + *g) >> 1) + *b) >> 1; // (r + b + g) / 3

			r += 4;
			g += 4;
			b += 4;
		}
	}
	else
	{
		for (int i = 0; i < Image.colorCount (); i++)
		{
			uint r = qRed (Image.color (i));
			uint g = qGreen (Image.color (i));
			uint b = qBlue (Image.color (i));

			uint gray = (((r + g) >> 1) + b) >> 1;

			Image.setColor (i, qRgba (gray, gray, gray, qAlpha (Image.color (i))));
		}
	}
}

QIcon MakeNormalAndGrayIcon(QIcon Icon)
{
	QImage Image = Icon.pixmap(Icon.availableSizes().first()).toImage();
	Icon.addPixmap(QPixmap::fromImage(Image), QIcon::Normal);
	GrayScale(Image);
	Icon.addPixmap(QPixmap::fromImage(Image), QIcon::Disabled);
	return Icon;
}

QIcon MakeActionIcon(const QString& IconFile)
{
	QImage Image(IconFile);
	QIcon Icon;
	Icon.addPixmap(QPixmap::fromImage(Image), QIcon::Normal);
	GrayScale(Image);
	Icon.addPixmap(QPixmap::fromImage(Image), QIcon::Disabled);
	return Icon;
}

QAction* MakeAction(QToolBar* pParent, const QString& IconFile, const QString& Text)
{
	QAction* pAction = new QAction(Text, pParent);
	pAction->setIcon(MakeActionIcon(IconFile));
	pParent->addAction(pAction);
	return pAction;
}

QMenu* MakeMenu(QMenu* pParent, const QString& Text, const QString& IconFile)
{
	if(!IconFile.isEmpty())
	{
		QImage Image(IconFile);
		QIcon Icon;
		Icon.addPixmap(QPixmap::fromImage(Image), QIcon::Normal);
		GrayScale(Image);
		Icon.addPixmap(QPixmap::fromImage(Image), QIcon::Disabled);
		return pParent->addMenu(Icon, Text);
	}
	return pParent->addMenu(Text);
}

QAction* MakeAction(QMenu* pParent, const QString& Text, const QString& IconFile)
{
	QAction* pAction = new QAction(Text, pParent);
	if(!IconFile.isEmpty())
	{
		QImage Image(IconFile);
		QIcon Icon;
		Icon.addPixmap(QPixmap::fromImage(Image), QIcon::Normal);
		GrayScale(Image);
		Icon.addPixmap(QPixmap::fromImage(Image), QIcon::Disabled);
		pAction->setIcon(Icon);
	}
	pParent->addAction(pAction);
	return pAction;
}

QAction* MakeAction(QActionGroup* pGroup, QMenu* pParent, const QString& Text, const QVariant& Data)
{
	QAction* pAction = new QAction(Text, pParent);
	pAction->setCheckable(true);
	pAction->setData(Data);
	pAction->setActionGroup(pGroup);
	pParent->addAction(pAction);
	return pAction;
}

void SetPaleteTexture(QPalette& palette, QPalette::ColorRole role, const QImage& image)
{
	for (int i = 0; i < QPalette::NColorGroups; ++i) {
		QBrush brush(image);
		brush.setColor(palette.brush(QPalette::ColorGroup(i), role).color());
		palette.setBrush(QPalette::ColorGroup(i), role, brush);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// 
// 

#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>

bool InitConsole(bool bCreateIfNeeded)
{
	if (AttachConsole(ATTACH_PARENT_PROCESS) == FALSE)
	{
		if (!bCreateIfNeeded)
			return false;
		AllocConsole();
	}
	freopen("CONOUT$", "w", stdout);
	printf("\r\n");
	return true;
}
#endif

//
// avoid flashing a bright white window when in dark mode
//

void SafeShow(QWidget* pWidget) {
	static bool Lock = false;
	pWidget->setProperty("windowOpacity", 0.0);
	if (Lock == false) {
		Lock = true;
		pWidget->show();
		QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
		Lock = false;
	} else
		pWidget->show();
	pWidget->setProperty("windowOpacity", 1.0);
}

