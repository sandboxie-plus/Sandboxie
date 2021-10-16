/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWinExtras module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "stdafx.h"
#include "IconExtreactor.h"

#include <QtWin>

//#include <iostream>

#include <shellapi.h>
#include <comdef.h>
#include <commctrl.h>
#include <objbase.h>
#include <commoncontrols.h>


/* This example demonstrates the Windows-specific image conversion
 * functions. */

/*struct PixmapEntry {
	QString name;
	QPixmap pixmap;
};

typedef QVector<PixmapEntry> PixmapEntryList;*/

/*std::wostream &operator<<(std::wostream &str, const QString &s)
{
#ifdef Q_OS_WIN
	str << reinterpret_cast<const wchar_t *>(s.utf16());
#else
	str << s.toStdWString();
#endif
	return str;
}*/

QString formatSize(const QSize &size)
{
	return QString::number(size.width()) + u'x' + QString::number(size.height());
}

// Extract icons contained in executable or DLL using the Win32 API ExtractIconEx()
PixmapEntryList extractIcons(const QString &sourceFile, bool large)
{
	const QString nativeName = QDir::toNativeSeparators(sourceFile);
	const auto *sourceFileC = reinterpret_cast<const wchar_t *>(nativeName.utf16());
	const UINT iconCount = ExtractIconEx(sourceFileC, -1, nullptr, nullptr, 0);
	if (!iconCount) {
		//std::wcerr << sourceFile << " does not appear to contain icons.\n";
		return PixmapEntryList();
	}

	QScopedArrayPointer<HICON> icons(new HICON[iconCount]);
	const UINT extractedIconCount = large ?
		ExtractIconEx(sourceFileC, 0, icons.data(), nullptr, iconCount) :
		ExtractIconEx(sourceFileC, 0, nullptr, icons.data(), iconCount);
	if (!extractedIconCount) {
		qErrnoWarning("Failed to extract icons from %s", qPrintable(sourceFile));
		return PixmapEntryList();
	}

	PixmapEntryList result;
	result.reserve(int(extractedIconCount));

	//std::wcout << sourceFile << " contains " << extractedIconCount << " icon(s).\n";

	for (UINT i = 0; i < extractedIconCount; ++i) {
		PixmapEntry entry;
		entry.pixmap = QtWin::fromHICON(icons[i]);
		if (entry.pixmap.isNull()) {
			//std::wcerr << "Error converting icons.\n";
			return PixmapEntryList();
		}
		entry.name = QString::fromLatin1("%1_%2x%3").arg(i, 3, 10, QLatin1Char('0'))
			.arg(entry.pixmap.width()).arg(entry.pixmap.height());
		result.append(entry);
	}
	return result;
}

// Helper for extracting large/jumbo icons available from Windows Vista onwards
// via SHGetImageList().
QPixmap pixmapFromShellImageList(int iImageList, const SHFILEINFO &info)
{
	QPixmap result;
	// For MinGW:
	const IID iID_IImageList = { 0x46eb5926, 0x582e, 0x4017, {0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x9, 0x50} };

	IImageList *imageList = nullptr;
	if (FAILED(SHGetImageList(iImageList, iID_IImageList, reinterpret_cast<void **>(&imageList))))
		return result;

	HICON hIcon = nullptr;
	if (SUCCEEDED(imageList->GetIcon(info.iIcon, ILD_TRANSPARENT, &hIcon))) {
		result = QtWin::fromHICON(hIcon);
		DestroyIcon(hIcon);
	}
	return result;
}

// Extract icons that would be  displayed by the Explorer (shell)
PixmapEntryList extractShellIcons(const QString &sourceFile, bool addOverlays)
{
	enum { // Shell image list ids
		sHIL_EXTRALARGE = 0x2, // 48x48 or user-defined
		sHIL_JUMBO = 0x4 // 256x256 (Vista or later)
	};

	struct FlagEntry {
		const char *name;
		unsigned flags;
	};

	const FlagEntry modeEntries[] =
	{
		{"",     0},
		{"open", SHGFI_OPENICON},
		{"sel",  SHGFI_SELECTED},
	};
	const FlagEntry standardSizeEntries[] =
	{
		{"s",  SHGFI_SMALLICON},
		{"l",  SHGFI_LARGEICON},
		{"sh", SHGFI_SHELLICONSIZE},
	};

	const QString nativeName = QDir::toNativeSeparators(sourceFile);
	const auto *sourceFileC = reinterpret_cast<const wchar_t *>(nativeName.utf16());

	SHFILEINFO info;
	unsigned int baseFlags = SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_ICONLOCATION;
	if (addOverlays)
		baseFlags |= SHGFI_ADDOVERLAYS | SHGFI_OVERLAYINDEX;
	if (!QFileInfo(sourceFile).isDir())
		baseFlags |= SHGFI_USEFILEATTRIBUTES;

	PixmapEntryList result;
	for (auto modeEntry : modeEntries) {
		const unsigned modeFlags = baseFlags | modeEntry.flags;
		QString modePrefix = QLatin1String("_shell_");
		if (modeEntry.name[0])
			modePrefix += QLatin1String(modeEntry.name) + u'_';
		for (auto standardSizeEntry : standardSizeEntries) {
			const unsigned flags = modeFlags | standardSizeEntry.flags;
			const QString prefix = modePrefix + QLatin1String(standardSizeEntry.name)
				+ u'_';
			ZeroMemory(&info, sizeof(SHFILEINFO));
			const HRESULT hr = SHGetFileInfo(sourceFileC, 0, &info, sizeof(SHFILEINFO), flags);
			if (FAILED(hr)) {
				_com_error error(hr);
				//std::wcerr << "SHGetFileInfo() failed for \"" << nativeName << "\", "
				//	<< std::hex << std::showbase << flags << std::dec << std::noshowbase
				//	<< " (" << prefix << "): " << error.ErrorMessage() << '\n';
				continue;
			}

			if (info.hIcon) {
				PixmapEntry entry;
				entry.pixmap = QtWin::fromHICON(info.hIcon);
				DestroyIcon(info.hIcon);
				if (entry.pixmap.isNull()) {
					//std::wcerr << "Error converting icons.\n";
					return PixmapEntryList();
				}
				entry.name = prefix + formatSize(entry.pixmap.size());

				const int iconIndex = info.iIcon & 0xFFFFFF;
				const int overlayIconIndex = info.iIcon >> 24;

				//std::wcout << "Obtained icon #" << iconIndex;
				//if (addOverlays)
				//	std::wcout << " (overlay #" << overlayIconIndex << ')';
				//if (info.szDisplayName[0])
				//	std::wcout << " from " << QString::fromWCharArray(info.szDisplayName);
				//std::wcout << " (" << entry.pixmap.width() << 'x'
				//	<< entry.pixmap.height() << ") for " << std::hex << std::showbase << flags
				//	<< std::dec << std::noshowbase << '\n';

				result.append(entry);
			}
		} // for standardSizeEntryCount
		// Windows Vista onwards: extract large/jumbo icons
		if (info.hIcon) {
			const QPixmap extraLarge = pixmapFromShellImageList(sHIL_EXTRALARGE, info);
			if (!extraLarge.isNull()) {
				PixmapEntry entry;
				entry.pixmap = extraLarge;
				entry.name = modePrefix + QLatin1String("xl_") + formatSize(extraLarge.size());
				result.append(entry);
			}
			const QPixmap jumbo = pixmapFromShellImageList(sHIL_JUMBO, info);
			if (!jumbo.isNull()) {
				PixmapEntry entry;
				entry.pixmap = jumbo;
				entry.name = modePrefix + QLatin1String("jumbo_") + formatSize(extraLarge.size());
				result.append(entry);
			}
		}
	} // for modes
	return result;
}

/*
const char description[] =
"\nExtracts Windows icons from executables, DLL or icon files and writes them\n"
"out as numbered .png-files.\n"
"When passing the --shell option, the icons displayed by Explorer are extracted.\n";


#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("Icon Extractor"));
	QCoreApplication::setOrganizationName(QStringLiteral("QtProject"));
	QCoreApplication::setApplicationVersion(QLatin1String(QT_VERSION_STR));
	QCommandLineParser parser;
	parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);
	parser.setApplicationDescription(QLatin1String(description));
	parser.addHelpOption();
	parser.addVersionOption();
	const QCommandLineOption largeIconOption(QStringLiteral("large"), QStringLiteral("Extract large icons"));
	parser.addOption(largeIconOption);
	const QCommandLineOption shellIconOption(QStringLiteral("shell"), QStringLiteral("Extract shell icons using SHGetFileInfo()"));
	parser.addOption(shellIconOption);
	const QCommandLineOption shellOverlayOption(QStringLiteral("overlay"), QStringLiteral("Extract shell overlay icons"));
	parser.addOption(shellOverlayOption);
	parser.addPositionalArgument(QStringLiteral("file"), QStringLiteral("The file to open."));
	parser.addPositionalArgument(QStringLiteral("image_file_folder"), QStringLiteral("The folder to store the images."));
	parser.process(app);
	const QStringList &positionalArguments = parser.positionalArguments();
	if (positionalArguments.isEmpty())
		parser.showHelp(0);

	QString imageFileRoot = positionalArguments.size() > 1 ? positionalArguments.at(1) : QDir::currentPath();
	const QFileInfo imageFileRootInfo(imageFileRoot);
	if (!imageFileRootInfo.isDir()) {
		std::wcerr << imageFileRoot << " is not a directory.\n";
		return 1;
	}
	const QString &sourceFile = positionalArguments.constFirst();
	imageFileRoot = imageFileRootInfo.absoluteFilePath() + u'/' + QFileInfo(sourceFile).baseName();

	const PixmapEntryList pixmaps = parser.isSet(shellIconOption)
		? extractShellIcons(sourceFile, parser.isSet(shellOverlayOption))
		: extractIcons(sourceFile, parser.isSet(largeIconOption));

	for (const auto &entry : pixmaps) {
		const QString fileName = imageFileRoot + entry.name + QLatin1String(".png");
		if (!entry.pixmap.save(fileName)) {
			std::wcerr << "Error writing image file " << fileName << ".\n";
			return 1;
		}
		std::wcout << "Wrote " << QDir::toNativeSeparators(fileName) << ".\n";
	}
	return 0;
}*/