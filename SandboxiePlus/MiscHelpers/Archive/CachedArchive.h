#pragma once

#include "../mischelpers_global.h"

#include "Archive.h"

#ifdef USE_7Z

class MISCHELPERS_EXPORT CCachedArchive: public CArchive
{
public:
	CCachedArchive(const QString &ArchivePath);

	bool						Update(QMap<int, QIODevice*> *FileList, bool bDelete = true) {LogError("Cachen archives can not be updated"); return false;}
	bool						Extract(QMap<int, QIODevice*> *FileList, bool bDelete = true);

protected:

	QMap<int, QByteArray>		m_CacheMap;
};

#endif