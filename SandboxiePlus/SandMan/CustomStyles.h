#pragma once
#include <QProxyStyle>
#include "SandMan.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Implements properly oriented side tabs
//

class CustomTabStyle : public QProxyStyle {
public:
	CustomTabStyle(QStyle* style = 0) : QProxyStyle(style) {}

	QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const {
		QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
		if (type == QStyle::CT_TabBarTab) {
            const QTabWidget* pTabWidget = qobject_cast<const QTabWidget*>(widget->parentWidget());
            if (pTabWidget && pTabWidget->tabPosition() == QTabWidget::West) {
                s.transpose();
                QString baseName = baseStyle()->metaObject()->className();
                if (baseName == "QFusionStyle")
                    s.setHeight(s.height() * 125 / 100);
                else
                    s.setHeight(s.height() * 15 / 10);
                s.setWidth(s.width() * 11 / 10); // for the the icon
            }
		}
		return s;
	}

	void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
		if (element == CE_TabBarTabLabel) {
            const QTabWidget* pTabWidget = qobject_cast<const QTabWidget*>(widget->parentWidget());
            if (pTabWidget && pTabWidget->tabPosition() == QTabWidget::West) {
                if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
                    QStyleOptionTab opt(*tab);
                    opt.shape = QTabBar::RoundedNorth;
                    //opt.iconSize = QSize(32, 32);
                    opt.iconSize = QSize(24, 24);
                    QProxyStyle::drawControl(element, &opt, painter, widget);
                    return;
                }
            }
		}
		QProxyStyle::drawControl(element, option, painter, widget);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Keeps submenus visible when the mouse leaves them
//

class KeepSubMenusVisibleStyle : public QProxyStyle {
public:
	KeepSubMenusVisibleStyle(QStyle* style = 0) : QProxyStyle(style) {}

	int styleHint(StyleHint styleHint, const QStyleOption* opt = nullptr,
		const QWidget* widget = nullptr, QStyleHintReturn* returnData = nullptr) const override
	{
		if (styleHint == SH_Menu_SubMenuDontStartSloppyOnLeave) 
			return 1;
		return QProxyStyle::styleHint(styleHint, opt, widget, returnData);
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Gives checkable menu entries their check mark back when they also carry an icon
//

class CheckableMenuItemStyle : public QProxyStyle
{
public:
	CheckableMenuItemStyle(QStyle* pStyle = 0) : QProxyStyle(pStyle) {}

	virtual void drawControl(ControlElement element, const QStyleOption* pOption, QPainter* pPainter, const QWidget* pWidget) const
	{
		//
		// A checkable menu entry that also has an icon silently loses its check
		// mark, and the same QAction on the toolbar shows its state perfectly.
		// That is not a theme bug so much as a choice Qt makes for us: given an
		// icon, the style skips the check box entirely and instead paints a
		// sunken panel behind the icon to mean "on". On a flat theme that panel
		// is all but invisible, so a toggle looks identical in both states - and
		// an entry whose state you cannot read is worse than one with no icon.
		//
		// Dropping the icon for this one painting puts the entry back on the
		// plain-text path, which draws a real check box. The QAction keeps its
		// icon, so toolbars and anywhere else it appears are untouched, and the
		// same action can be reused in both places without a second copy of it.
		//
		// Deliberately applied to every checkable entry rather than only the
		// checked ones: an empty check box is how someone learns the entry is a
		// toggle at all, and an entry that swapped between an icon and a box as
		// it toggled would be worse than either.
		//
		if (element == CE_MenuItem) {
			if (const QStyleOptionMenuItem* pMenuItem = qstyleoption_cast<const QStyleOptionMenuItem*>(pOption)) {
				if (pMenuItem->checkType != QStyleOptionMenuItem::NotCheckable && !pMenuItem->icon.isNull()) {
					QStyleOptionMenuItem Copy(*pMenuItem);
					Copy.icon = QIcon();
					// The size is left alone on purpose: sizeFromContents still sees
					// the icon, so the column keeps its width and the check box is
					// not cramped into a narrower one than its neighbours.
					QProxyStyle::drawControl(element, &Copy, pPainter, pWidget);
					return;
				}
			}
		}
		QProxyStyle::drawControl(element, pOption, pPainter, pWidget);
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Implements nice flat buttons
//

class MyButtonStyle : public QProxyStyle
{
public:
    MyButtonStyle(QStyle* pStyle) : QProxyStyle(pStyle) {}

    virtual QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
	    if (type == CE_PushButton && widget->property("leftButton").toBool())
        {
            if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option))
            {
                if (!button->icon.isNull()) {
                    s.setWidth(s.width() + 20 + button->iconSize.width());
                }
            }
	    }
	    return s;
    }

    virtual void drawControl(ControlElement element, const QStyleOption* opt, QPainter* p, const QWidget* widget) const 
    {
        if (element == CE_PushButtonLabel && widget->property("leftButton").toBool())
        {
            if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(opt))
            {
                QRect textRect = button->rect;
                uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
                if (!proxy()->styleHint(SH_UnderlineShortcut, button, widget))
                    tf |= Qt::TextHideMnemonic;

                if (!button->icon.isNull()) {
                    QRect iconRect;
                    QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                    if (mode == QIcon::Normal && button->state & State_HasFocus)
                        mode = QIcon::Active;
                    QIcon::State state = QIcon::Off;
                    if (button->state & State_On)
                        state = QIcon::On;

                    QPixmap pixmap = button->icon.pixmap(widget ? widget->window()->windowHandle() : 0, button->iconSize, mode, state);

                    int pixmapWidth = pixmap.width() / pixmap.devicePixelRatio();
                    int pixmapHeight = pixmap.height() / pixmap.devicePixelRatio();
                    int labelWidth = pixmapWidth;
                    int labelHeight = pixmapHeight;
                    int iconSpacing = 4;//### 4 is currently hardcoded in QPushButton::sizeHint()
                    int textWidth = button->fontMetrics.boundingRect(opt->rect, tf, button->text).width();
                    if (!button->text.isEmpty())
                        labelWidth += (textWidth + iconSpacing);

                    textRect.setLeft(textRect.left() + 20 + button->iconSize.width());

                    /*************************************************************/
                    // Make the icon rectangle always be 10px in from the left edge
                    /*************************************************************/
                    iconRect = QRect(10,
                        textRect.y() + (textRect.height() - labelHeight) / 2,
                        pixmapWidth, pixmapHeight);

                    iconRect = visualRect(button->direction, textRect, iconRect);

                    /***********************************/
                    // Always horizontal align the text
                    /***********************************/
                    tf |= Qt::AlignLeft;


                    if (button->state & (State_On | State_Sunken))
                        iconRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, widget),
                            proxy()->pixelMetric(PM_ButtonShiftVertical, opt, widget));
                    p->drawPixmap(iconRect, pixmap);
                }
                else {
                    tf |= Qt::AlignHCenter;
                }
                if (button->state & (State_On | State_Sunken))
                    textRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, widget),
                        proxy()->pixelMetric(PM_ButtonShiftVertical, opt, widget));

                if (button->features & QStyleOptionButton::HasMenu) {
                    int indicatorSize = proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget);
                    if (button->direction == Qt::LeftToRight)
                        textRect = textRect.adjusted(0, 0, -indicatorSize, 0);
                    else
                        textRect = textRect.adjusted(indicatorSize, 0, 0, 0);
                }
                proxy()->drawItemText(p, textRect, tf, button->palette, (button->state & State_Enabled),
                    button->text, QPalette::ButtonText);
            }
            return;
        }
        

        // For all other controls, draw the default
        QProxyStyle::drawControl(element, opt, p, widget);
    }
};
