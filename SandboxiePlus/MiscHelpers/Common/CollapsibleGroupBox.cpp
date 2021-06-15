#include "stdafx.h"
#include "collapsiblegroupbox.h"
#include <QApplication>
#include <QGuiApplication>
#include <QLayout>
#include <QResizeEvent>
#include <QScreen>
#include <QStyle>
#include <QTimer>
#include <QWindow>
#include <cassert>
#include <cmath>

inline
QWindow *findWindowForWidget(const QWidget *widget)
{
  for (;;) {
    QWindow *wh = widget->window()->windowHandle();
    if (wh != nullptr)
      return wh;

    widget = qobject_cast<const QWidget *>(widget->parent());
    if (widget == nullptr)
      return nullptr;
  }
}

inline
QScreen * findScreenForWidget(const QWidget *widget)
{
  for (;;) {
    QWindow *wh = widget->window()->windowHandle();
    if (wh != nullptr) {
      QScreen *scr = wh->screen();
      if (scr != nullptr)
        return scr;
    }

    widget = qobject_cast<const QWidget *>(widget->parent());
    if (widget == nullptr)
      return nullptr;
  }
}

CCollapsibleGroupBox::CCollapsibleGroupBox(QWidget *parent) :
  QGroupBox(parent)
{
  m_clExpButton = new QToolButton(this);
  m_clExpButton->setCheckable(true);

  connect(m_clExpButton, &QToolButton::clicked, this, &CCollapsibleGroupBox::onVisibilityChanged);

  QTimer::singleShot(0, this, [this] {
    auto wh = findWindowForWidget(this);
    if (wh != nullptr)
      connect(wh, &QWindow::screenChanged, this, &CCollapsibleGroupBox::onScreenChanged);
  });

  QTimer::singleShot(0, this, &CCollapsibleGroupBox::resizeCollapseButton);
}

void CCollapsibleGroupBox::collapseLayout(QLayout *lay)
{
  assert(!m_layoutMargins.contains(lay));

  const int cnt = lay->count();
  for (int idx = 0; idx < cnt; idx++) {
    auto lit = lay->itemAt(idx);

    if (lit->widget()) {
      auto w = lit->widget();
      if (w != m_clExpButton)
        w->setVisible(false);
    }
    else if (lit->spacerItem())
      collapseSpacer(lit->spacerItem());
    else if (lit->layout())
      collapseLayout(lit->layout());
  }

  m_layoutMargins[lay] = lay->contentsMargins();
  lay->setContentsMargins(0, 0, 0, 0);
}

void CCollapsibleGroupBox::collapseSpacer(QSpacerItem *spacer)
{
  assert(!m_spacerSizes.contains(spacer));

  m_spacerSizes[spacer] = {spacer->sizeHint(), spacer->sizePolicy()};
  spacer->changeSize(0, 0);
}

void CCollapsibleGroupBox::expandLayout(QLayout *lay)
{
  assert(m_layoutMargins.contains(lay));

  const int cnt = lay->count();
  for (int idx = 0; idx < cnt; idx++) {
    auto lit = lay->itemAt(idx);

    if (lit->widget())
      lit->widget()->setVisible(true);
    else if (lit->spacerItem())
      expandSpacer(lit->spacerItem());
    else if (lit->layout())
      expandLayout(lit->layout());
  }

  lay->setContentsMargins(m_layoutMargins[lay]);
}

void CCollapsibleGroupBox::expandSpacer(QSpacerItem *spacer)
{
  assert(m_spacerSizes.contains(spacer));

  const auto &sz = m_spacerSizes[spacer].first;
  const auto &pol = m_spacerSizes[spacer].second;

  spacer->changeSize(sz.width(), sz.height(), pol.horizontalPolicy(), pol.verticalPolicy());
}

void CCollapsibleGroupBox::onScreenChanged()
{
  resizeCollapseButton();
}

void CCollapsibleGroupBox::onVisibilityChanged()
{
  assert(this->layout() != nullptr);

  if(m_clExpButton->isChecked())
  {
    m_layoutMargins.clear();
    m_spacerSizes.clear();

    collapseLayout(this->layout());
  }
  else
  {
    expandLayout(this->layout());
  }
}

void CCollapsibleGroupBox::resizeCollapseButton()
{
  const QScreen *scr = findScreenForWidget(this);

  if (scr == nullptr)
    return;

  const auto &size = this->size();

#ifdef Q_OS_WIN
  qreal baseSize = 15.0;
  int yOffset = 5;
#else
  qreal baseSize = 22.0;
  int yOffset = 0;
#endif

  if (scr == nullptr)
    return;

  if (QString::compare(QApplication::style()->objectName(), "fusion") == 0)
    baseSize = 15.0;

  const qreal dpi = scr->logicalDotsPerInchX();
  const qreal btnSize = floor((baseSize * dpi / 96.0) + 0.5);

  m_clExpButton->setGeometry(size.width() - btnSize, yOffset, btnSize, btnSize);
}

void CCollapsibleGroupBox::resizeEvent(QResizeEvent *)
{
  resizeCollapseButton();
}