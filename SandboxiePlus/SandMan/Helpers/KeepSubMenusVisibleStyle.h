#pragma once

#include <QProxyStyle>

// Keep submenus open when mouse leaves
class KeepSubMenusVisibleStyle : public QProxyStyle {
public:
	KeepSubMenusVisibleStyle(QStyle* style = 0) : QProxyStyle(style) {}

	int styleHint(StyleHint styleHint, const QStyleOption* opt = nullptr,
		const QWidget* widget = nullptr, QStyleHintReturn* returnData = nullptr) const
	{
		if (styleHint == SH_Menu_SubMenuDontStartSloppyOnLeave) return 1;
		return QProxyStyle::styleHint(styleHint, opt, widget, returnData);
	}
};
