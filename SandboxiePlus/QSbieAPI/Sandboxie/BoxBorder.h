/*
 *
 * Copyright (c) 2020-2022, David Xanatos
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <qobject.h>
#include <QList>
#include <QString>
#include <QStringList>

#include "../qsbieapi_global.h"

class CSbieAPI;
struct HFONT__;

struct SBoxBorderFont
{
	QString family = "Segoe UI", nativeFamily, style, fullName, error;
	int weight = 700, writingSystem = 0;
	bool italic = false, underline = false, strikeout = false;
	double pointSize = 0; // Zero keeps the legacy BorderColor size reference.
};

struct SBoxBorderFontFace
{
	QString family, style, fullName;
	int weight = 400;
	bool italic = false;
};

struct SBoxBorderLabelPosition
{
	QString placement = "top";
	int percent = 50;
	QString text = "stacked";
	QString movement = "adaptive-any";

	static SBoxBorderLabelPosition Parse(const QString& value)
	{
		SBoxBorderLabelPosition position;
		QStringList parts = value.split(",");
		if (parts.size() < 2 || parts.size() > 4)
			return position;

		QString placement = parts[0].trimmed().toLower();
		QString text = parts.size() >= 3 ? parts[2].trimmed().toLower() : "stacked";
		QString movement = parts.size() >= 4 ? parts[3].trimmed().toLower() : "adaptive-any";
		bool valid = false;
		int percent = parts[1].trimmed().toInt(&valid);
		if ((placement != "top" && placement != "bottom" && placement != "left" && placement != "right") ||
			(text != "stacked" && text != "rotated" && text != "mirrored" && text != "stacked-compact") ||
			(movement != "fixed" && movement != "adaptive" && movement != "adaptive-opposite" && movement != "adaptive-any") ||
			!valid || percent < 0 || percent > 100)
			return position;

		position.placement = placement;
		position.percent = percent;
		position.text = text;
		position.movement = movement;
		return position;
	}
};

class QSBIEAPI_EXPORT CBoxBorder: public QObject
{
	Q_OBJECT
public:
	CBoxBorder(CSbieAPI* pApi, QObject* parent = 0);
	virtual ~CBoxBorder();

	static SBoxBorderFont ParseLabelFont(const QString& value);
	static QString SerializeLabelFont(const SBoxBorderFont& font);
	static QList<SBoxBorderFontFace> EnumLabelFontFaces(const QString& family);
	// Caller owns the returned font and must deselect it before DeleteObject.
	static HFONT__* CreateLabelFont(const SBoxBorderFont& font, int legacyReference, int dpi,
		double* realizedPoints = NULL, QString* error = NULL);

	void		ThreadFunc();
	void		TimerProc();
	struct SBoxBorder* GetBorderData() { return m; }

protected:

	CSbieAPI*	m_Api;

private:
	struct SBoxBorder* m;

	void		GetActiveWindowRect(struct HWND__* hWnd, struct tagRECT* rect);
	bool		IsMouseOnTitle(struct HWND__* hWnd, struct tagRECT* rect, const struct tagRECT* Desktop);
	bool		CheckMousePointer();

	void		DrawAllSandboxedBorders();
	bool		CheckGlobalAllBordersMode();
};

