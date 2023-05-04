#pragma once
#include "../framework.h"

void InitOsVersionInfo();
extern OSVERSIONINFOW g_osvi;

extern std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> g_str_conv;

bool ListDir(std::wstring Path, std::vector<std::wstring>& Entries);
std::wstring hexStr(unsigned char* data, int len);


template <typename T>
T Trimmx(const T& String, const T& Blank)
{
	typename T::size_type Start = String.find_first_not_of(Blank);
	typename T::size_type End = String.find_last_not_of(Blank)+1;
	if(Start == std::wstring::npos)
		return T();
	return  String.substr(Start, End - Start);
}

__inline std::wstring Trimm(const std::wstring& String)	{return Trimmx(String, std::wstring(L" \r\n\t"));}
__inline std::string Trimm(const std::string& String)		{return Trimmx(String, std::string(" \r\n\t"));}

template <typename T>
std::pair<T,T> Split2x(const T& String, T Separator, bool Back)
{
	typename T::size_type Sep = Back ? String.rfind(Separator) : String.find(Separator);
	if(Sep != T::npos)
		return std::pair<T,T>(String.substr(0, Sep), String.substr(Sep+Separator.length()));
	return std::pair<T,T>(String, T());
}

__inline std::pair<std::string,std::string> Split2(const std::string& String, std::string Separator = ",", bool Back = false) {return Split2x(String, Separator, Back);}
__inline std::pair<std::wstring,std::wstring> Split2(const std::wstring& String, std::wstring Separator = L",", bool Back = false) {return Split2x(String, Separator, Back);}

template <typename T>
std::vector<T> SplitStrx(const T& String, const T& Separator, bool bKeepEmpty = true, bool bMulti = false)
{
	std::vector<T> StringList;
	typename T::size_type Pos = 0;
	for(;;)
	{
		typename T::size_type Sep = bMulti ? String.find_first_of(Separator,Pos) : String.find(Separator,Pos);
		if(Sep != T::npos)
		{
			if(bKeepEmpty || Sep-Pos > 0)
				StringList.push_back(String.substr(Pos,Sep-Pos));
			Pos = Sep+1;
		}
		else
		{
			if(bKeepEmpty || Pos < String.length())
				StringList.push_back(String.substr(Pos));
			break;
		}
	}
	return StringList;
}

__inline std::vector<std::string> SplitStr(const std::string& String, std::string Separator = ",", bool bKeepEmpty = true, bool bMulti = false) {return SplitStrx(String, Separator, bKeepEmpty, bMulti);}
__inline std::vector<std::wstring> SplitStr(const std::wstring& String, std::wstring Separator = L",", bool bKeepEmpty = true, bool bMulti = false) {return SplitStrx(String, Separator, bKeepEmpty, bMulti);}

template <typename T>
T JoinStrx(const std::vector<T>& StringList, const T& Separator)
{
	T String;
    for(typename std::vector<T>::const_iterator I = StringList.begin(); I != StringList.end(); I++)
	{
		if(!String.empty())
			String += Separator;
		String += *I;
	}
	return String;
}

__inline std::string JoinStr(const std::vector<std::string>& String, std::string Separator = ",") {return JoinStrx(String, Separator);}
__inline std::wstring JoinStr(const std::vector<std::wstring>& String, std::wstring Separator = L",") {return JoinStrx(String, Separator);}

template <typename T>
__inline bool xchicmp(const T Chr1, const T Chr2)
{
	T L = ((Chr1 >= 'A') && (Chr1 <= 'Z')) ? Chr1 + 32 : Chr1;	// L'a' - L'A' == 32
	T R = ((Chr2 >= 'A') && (Chr2 <= 'Z')) ? Chr2 + 32 : Chr2;
	return L == R;
}

template <typename T>
const T* wildstrcmp(const T* Wild, const T* Str)
{
	const T *cp = NULL, *mp = NULL;

	while ((*Str) && (*Wild != '*')) 
	{
		if (!xchicmp(*Wild, *Str) && (*Wild != '?')) 
			return NULL;
		Wild++;
		Str++;
	}

	while (*Str) 
	{
		if (*Wild == '*') 
		{
			if (!*++Wild) 
				return Str;
			mp = Wild;
			cp = Str+1;
		} 
		else if (xchicmp(*Wild, *Str) || (*Wild == '?')) 
		{
			Wild++;
			Str++;
		}
		else 
		{
			Wild = mp;
			Str = cp++;
		}
	}

	while (*Wild == '*') 
		Wild++;
	return *Wild ? NULL : Str; // rest
}