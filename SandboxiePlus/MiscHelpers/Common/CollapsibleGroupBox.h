#ifndef COLLAPSIBLEGROUPBOX_H
#define COLLAPSIBLEGROUPBOX_H

#include <QGroupBox>
#include <QMap>
#include <QMargins>
#include <QPair>

class QResizeEvent;
class CollapseExpandButton;
class QSpacerItem;

class CollapsibleGroupBox : public QGroupBox
{
public:
  explicit CollapsibleGroupBox(QWidget *parent = nullptr);

protected:
  void resizeEvent(QResizeEvent *);

private:
  void resizeCollapseButton();
  void collapseLayout(QLayout *layout);
  void collapseSpacer(QSpacerItem *spacer);
  void expandLayout(QLayout *layout);
  void expandSpacer(QSpacerItem *spacer);

  CollapseExpandButton *m_clExpButton;
  QMap<const void *, QMargins> m_layoutMargins;
  QMap<const void *, QPair<QSize, QSizePolicy>> m_spacerSizes;

private slots:
  void onScreenChanged();
  void onVisibilityChanged();

};

#endif // COLLAPSIBLEGROUPBOX_H
