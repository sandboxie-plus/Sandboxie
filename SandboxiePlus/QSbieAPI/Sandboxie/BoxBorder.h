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

#include "../qsbieapi_global.h"

class CSbieAPI;

class QSBIEAPI_EXPORT CBoxBorder: public QObject
{
	Q_OBJECT
public:
	CBoxBorder(CSbieAPI* pApi, QObject* parent = 0);
	virtual ~CBoxBorder();

	void		ThreadFunc();
	void		TimerProc();

protected:

	CSbieAPI*	m_Api;

private:
	struct SBoxBorder* m;

	void		GetActiveWindowRect(struct HWND__* hWnd, struct tagRECT* rect);
	bool		IsMounseOnTitle(struct HWND__* hWnd, struct tagRECT* rect, const struct tagRECT* Desktop);
	bool		CheckMousePointer();
};

