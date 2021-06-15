#ifndef COLLAPSIBLEGROUPBOX_H
#define COLLAPSIBLEGROUPBOX_H

#include <QGroupBox>
#include <QMap>
#include <QMargins>
#include <QPair>
#include <QToolButton>

class QResizeEvent;
class CollapseExpandButton;
class QSpacerItem;

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CCollapsibleGroupBox : public QGroupBox
{
public:
  explicit CCollapsibleGroupBox(QWidget *parent = nullptr);

protected:
  void resizeEvent(QResizeEvent *);

private:
  void resizeCollapseButton();
  void collapseLayout(QLayout *layout);
  void collapseSpacer(QSpacerItem *spacer);
  void expandLayout(QLayout *layout);
  void expandSpacer(QSpacerItem *spacer);

  QToolButton *m_clExpButton;
  QMap<const void *, QMargins> m_layoutMargins;
  QMap<const void *, QPair<QSize, QSizePolicy>> m_spacerSizes;

private slots:
  void onScreenChanged();
  void onVisibilityChanged();

};

#endif // COLLAPSIBLEGROUPBOX_H
