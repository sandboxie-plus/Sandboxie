#pragma once

#include "../mischelpers_global.h"

struct PixmapEntry {
	QString name;
	QPixmap pixmap;
};

typedef QVector<PixmapEntry> PixmapEntryList;

MISCHELPERS_EXPORT PixmapEntryList extractIcons(const QString &sourceFile, bool large);
MISCHELPERS_EXPORT PixmapEntryList extractShellIcons(const QString &sourceFile, bool addOverlays);