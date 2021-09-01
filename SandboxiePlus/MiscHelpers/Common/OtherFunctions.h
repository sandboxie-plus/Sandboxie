#pragma once

#include "../mischelpers_global.h"

/////////////////////////
// HeapSort

template <class T>
void HeapSortAux(T &refArray, quint32 first, quint32 last, void (*HeapMov)(T &, quint32, quint32), bool (*HeapCmp)(T &, quint32, quint32))
{
	quint32 r;
	for (r = first; !(r & 0x80000000) && (r<<1) < last;)
	{
		quint32 r2 = (r<<1)+1;
		if (r2 != last)
		{
			if(HeapCmp(refArray, r2, r2+1))
				r2++;
		}
		if(HeapCmp(refArray, r, r2))
		{
			HeapMov(refArray, r2, r);
			r = r2;
		}
		else
			break;
	}
}

template <class T>
void HeapSort(T &refArray, void (*HeapMov)(T &, quint32, quint32), bool (*HeapCmp)(T &, quint32, quint32))
{
	int n = refArray.size();
	if (n > 0)
	{
		int r;
		for (r = n/2; r--;)
			HeapSortAux(refArray, r, n-1, HeapMov, HeapCmp);
		for (r = n; --r;)
		{
			HeapMov(refArray, r, 0);
			HeapSortAux(refArray, 0, r-1, HeapMov, HeapCmp);
		}
	}
}

/////////////////////////
// MergeSort

template<typename I> void MergeSortAux(I& lst, I& res, quint32 begin, quint32 middle, quint32 end, void (*Cpy)(I& T, I& S, quint32 t, quint32 s), bool (*Cmp)(I& A, quint32 l, quint32 r))
{
	quint32 a = begin;
	quint32 b = middle;
	quint32 r = 0;

	while (a < middle && b < end)
	{
		if (Cmp(lst, a, b)) 
			Cpy(res, lst, r++, a++);
		else
			Cpy(res, lst, r++, b++);
	}

	while (a < middle) 
		Cpy(res, lst, r++, a++);
	while (b < end) 
		Cpy(res, lst, r++, b++);

	a = begin;
	r = 0;
	while (a < end) 
		Cpy(lst, res, a++, r++);
}

template<typename I> void MergeSort(I& lst, I& res, int begin, int end, void (*Cpy)(I& T, I& S, quint32 t, quint32 s), bool (*Cmp)(I& A, quint32 l, quint32 r))
{
	quint32 s = end-begin;
	if (s > 1)
	{
		quint32 middle = begin+s/2;
		MergeSort(lst, res, begin, middle, Cpy, Cmp);
		MergeSort(lst, res, middle, end, Cpy, Cmp);
		MergeSortAux(lst, res, begin, middle, end, Cpy, Cmp);
	}
}

template<typename I> void MergeSort(I& lst, void (*Cpy)(I& T, I& S, quint32 t, quint32 s), bool (*Cmp)(I& A, quint32 l, quint32 r))
{
	I res = lst; // temporary space
	MergeSort(lst, res, 0, lst.size(), Cpy, Cmp);
}

/////////////////////////
// Reverse

template <class T>
void Reverse(T* Data, size_t Size)
{
	for(size_t i=0; i < Size/2; i++)
	{
		T Temp = Data[i];
		Data[i] = Data[Size - 1 - i];
		Data[Size - 1 - i] = Temp;
	}
}

//inline QString	tr(const char* s)							{return QObject::tr(s);}

//////////////////////////////////////////////////////////////////////////////////////////
// File system functions
// 



MISCHELPERS_EXPORT QString		ReadFileAsString(const QString& filename);
MISCHELPERS_EXPORT bool			WriteStringToFile(const QString& filename, const QString& content);
MISCHELPERS_EXPORT bool			CreateDir(const QString& path);
MISCHELPERS_EXPORT bool			DeleteDir(const QString& path, bool bEmpty = false);
MISCHELPERS_EXPORT bool			CopyDir(const QString& srcDirPath, const QString& destDirPath, bool bMove = false);
MISCHELPERS_EXPORT QStringList	ListDir(const QString& srcDirPath);
MISCHELPERS_EXPORT bool			SafeRemove(const QString& path);

MISCHELPERS_EXPORT QString GetRelativeSharedPath(const QString& fullPath, const QStringList& shared, QString& rootPath);

MISCHELPERS_EXPORT QString NameOfFile(const QString& FileName);

#ifdef _DEBUG
MISCHELPERS_EXPORT void GlobalTest();
#endif