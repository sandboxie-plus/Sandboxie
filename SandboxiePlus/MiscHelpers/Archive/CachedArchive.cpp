#include "stdafx.h"
#include "CachedArchive.h"

#ifdef USE_7Z

CCachedArchive::CCachedArchive(const QString &ArchivePath)
 : CArchive(ArchivePath)
{
}

bool CCachedArchive::Extract(QMap<int, QIODevice*> *FileList, bool bDelete)
{
	QMap<int, QIODevice*> Files;
	foreach(int ArcIndex, FileList->keys())
	{
		if(!m_CacheMap.contains(ArcIndex))
			Files.insert(ArcIndex, new QBuffer(&m_CacheMap[ArcIndex]));
	}

	if(!Files.isEmpty())
	{
		if(!CArchive::Extract(&Files))
			return false;
	}

	foreach(int ArcIndex, FileList->keys())
	{
		QIODevice* pIO = FileList->value(ArcIndex);
		pIO->open(QIODevice::WriteOnly);
		pIO->write(m_CacheMap.value(ArcIndex));
		pIO->close();
		delete pIO;
	}
	return true;
}

#endif