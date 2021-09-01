#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CMultiLineEdit: public QPlainTextEdit
{
	Q_OBJECT
public:
	CMultiLineEdit(QWidget *parent = 0)
	 :QPlainTextEdit(parent) {}

	void				SetLines(const QStringList& Lines){
		setPlainText(Lines.join("\r\n"));
	}
	QStringList			GetLines(){
		return toPlainText().split(QRegExp("\r?\n"));
	}
};

///////////////////////////////////////////////////
//

class MISCHELPERS_EXPORT QNumEdit: public QLineEdit
{
	Q_OBJECT

	Q_PROPERTY(int Value READ GetValue WRITE SetValue)

public:
	QNumEdit(QWidget *parent = 0) : QLineEdit(parent) {}

	virtual void		SetValue(qint64 Value) = 0;
	virtual qint64		GetValue() = 0;
};

	///////////////////////////////////////////////////
//

class MISCHELPERS_EXPORT CKbpsEdit: public QNumEdit
{
	Q_OBJECT

public:
	CKbpsEdit(qint64 Factor = 1024, QWidget *parent = 0)
	 :QNumEdit(parent) , m_Factor(Factor) {}

	void				SetValue(qint64 Value){
		if(Value <= 0)
			setText(tr("Unlimited"));
		else
			setText(QString::number(Value/m_Factor));
	}
	qint64				GetValue(){
		int Value = text().toInt();
		if(Value > 0)
			return Value*m_Factor;
		else
			return -1; // Unlimited;
	}

protected:
	qint64		m_Factor;
};

///////////////////////////////////////////////////
//

class MISCHELPERS_EXPORT CFactorEdit: public QNumEdit
{
	Q_OBJECT

public:
	CFactorEdit(double Factor = 100.0, int Prec = 2, bool bEmpty = false, QWidget *parent = 0)
	 :QNumEdit(parent), m_Factor(Factor), m_Empty(bEmpty), m_Prec(Prec) {}

	void				SetValue(qint64 Value){
		if(!Value && m_Empty)
			setText("");
		else
			setText(QString::number(Value / m_Factor, 'f', m_Prec));
	}
	qint64				GetValue(){
		return text().toDouble() * m_Factor;
	}

protected:
	double		m_Factor;
	bool		m_Empty;
	int			m_Prec;
};

///////////////////////////////////////////////////
//

class MISCHELPERS_EXPORT CSpinBoxEx: public QSpinBox
{
	Q_OBJECT
public:
	CSpinBoxEx(quint64 Unit, QWidget *parent = 0)
	 :QSpinBox(parent) {
		 m_Unit = Unit;
		 setSingleStep(Unit);
		 setMaximum(INT_MAX);
	}

protected:
	virtual int valueFromText(const QString &text) const{
		return text.toInt() * m_Unit;
	}
    virtual QString textFromValue(int val) const{
		return QString::number(val / m_Unit);
	}

	quint64 m_Unit;
};

///////////////////////////////////////////////////
//

class MISCHELPERS_EXPORT CTxtEdit: public QWidget
{
	Q_OBJECT

	Q_PROPERTY(QString Data READ GetText WRITE SetText)

public:
	CTxtEdit(QWidget *parent = 0) : QWidget(parent) {}

	virtual void		SetText(const QString& Text) = 0;
	virtual QString		GetText() = 0;
};

///////////////////////////////////////////////////
// CPathEdit

class MISCHELPERS_EXPORT CPathEdit: public CTxtEdit
{
    Q_OBJECT
public:
	CPathEdit(bool bDirs = false, QWidget *parent = 0);

	QLineEdit*			GetEdit()						{return m_pEdit;}

	void				SetText(const QString& Text)	{m_pEdit->setText(Text);}
	QString				GetText()						{return m_pEdit->text();}
	void				SetDefault(const QString& Text)	{m_pEdit->setPlaceholderText(Text);}
	void				SetWindowsPaths(bool bSet = true) {m_bWinPath = bSet;}

signals:
	void				textChanged(const QString& text);

private slots:
	void				Browse();
protected:
	QLineEdit*		m_pEdit;
	bool			m_bDirs;
	bool			m_bWinPath;
};

///////////////////////////////////////////////////
// CProxyEdit

class MISCHELPERS_EXPORT CProxyEdit: public CTxtEdit
{
	Q_OBJECT

public:
	CProxyEdit(QWidget *parent = 0);

	void				SetText(const QString& Text);
	QString				GetText()						{return m_pEdit->text();}

signals:
	void				textChanged(const QString& text);

private slots:
	void				OnType(int Index);

protected:
	QComboBox*			m_pType;
	QLineEdit*			m_pEdit;
};

///////////////////////////////////////////////////
//

class MISCHELPERS_EXPORT QComboBoxEx: public QComboBox 
{
	Q_OBJECT

	Q_PROPERTY(QVariant Data READ GetData)

public:
	QComboBoxEx(QWidget* pParent = 0) : QComboBox(pParent) {}

	QVariant				GetData() {return itemData(currentIndex());}
};

///////////////////////////////////////////////////
//

class MISCHELPERS_EXPORT QCheckBoxEx: public QCheckBox
{
	Q_OBJECT
public:
	QCheckBoxEx(const QString& Text, bool bSwap = true) : QCheckBox(Text) 
	{
		m_bSwap = bSwap;
		setTristate();
	}

	void SetState(int State)
	{
		if(m_bSwap)
		{
			switch(State)
			{
			case 0:	State = Qt::PartiallyChecked; break;
			case 1: State = Qt::Checked; break;
			case 2:	State = Qt::Unchecked; break;
			}
		}
		setCheckState((Qt::CheckState)State);
	}

	int GetState()
	{
		int State = checkState();
		if(m_bSwap)
		{
			switch(State)
			{
			case Qt::PartiallyChecked:	State = 0; break;
			case Qt::Checked:			State = 1; break;
			case Qt::Unchecked:			State = 2; break;
			}
		}
		return State;
	}

protected:
	bool		m_bSwap;
};

///////////////////////////////////////////////////
//

class MISCHELPERS_EXPORT QSecretCheckBox: public QCheckBoxEx
{
	Q_OBJECT
public:
	QSecretCheckBox(const QString& Text) : QCheckBoxEx(Text, false) 
	{
		setTristate(false);
	}

protected:
	void mouseDoubleClickEvent (QMouseEvent * e)
	{
		if(QApplication::keyboardModifiers() & Qt::ControlModifier)
			setTristate();
	}
};

///////////////////////////////////////////////////
// QSpinBoxEx

class MISCHELPERS_EXPORT QSpinBoxEx: public QSpinBox
{
public:
	QSpinBoxEx(QWidget* parent, const QString& Default, const QString& Suffix = "", bool exponential = false) : QSpinBox(parent)
	{
		m_default = 1;

		setRange(0, INT_MAX);
		setSuffix(Suffix);
		setSpecialValueText(Default + Suffix);
		m_exponential = exponential;
	}

	void stepBy(int steps)
	{
		if(m_exponential)
		{
			int Value = value();
			if(!Value)
			{
				Value = m_default;
				if(Value <= 0)
					Value = 1;
			}

			if(steps == 1)
				setValue(Value*2);
			else if(steps == -1)
				setValue(Value/2);
			else
				QSpinBox::stepBy(steps);
		}
		else
			QSpinBox::stepBy(steps);
	}

	void setDefault(int value) {m_default = value;}

protected:
	int	m_default;
	bool m_exponential;
};


///////////////////////////////////////////////////
// CMenuAction & CActionWidget

class MISCHELPERS_EXPORT QMenu_: public QMenu { public: void initStyleOption_(QStyleOptionMenuItem *option, const QAction *action) const {QMenu::initStyleOption(option, action);} };


class MISCHELPERS_EXPORT CActionWidget: public QWidget
{
public:
	CActionWidget(QWidget* parent = 0) : QWidget(parent) {}

	void paintEvent(QPaintEvent* e)
	{
		QPainter p(this);

		QMenu_ Menu;
		QAction* action = new QAction(this);

		QStyleOptionMenuItem opt;
        Menu.initStyleOption_(&opt, action);
		opt.menuRect = opt.rect = QRect(0, 0, 22, 22); // with this icons and text are drown properly
        style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);
	}
};

class MISCHELPERS_EXPORT CMenuAction: public QWidgetAction 
{
public:
	CMenuAction (QWidget* pControll, const QString& Title, QString IconFile = "") : QWidgetAction (pControll->parent())
	{
		QWidget* pWidget = new CActionWidget(pControll->parentWidget());
        QHBoxLayout* pLayout = new QHBoxLayout();
		pLayout->setMargin(0);

		if(!IconFile.isEmpty())
		{
			QImage Image(IconFile);
			QLabel* pIcon = new QLabel();
			pIcon->setToolTip(Title);
			pIcon->setPixmap(QPixmap::fromImage(Image).scaled(16,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
			pLayout->addWidget(pIcon);

			pLayout->insertSpacing(0, 3);
			pLayout->insertSpacing(2, 6);
		}
		else
		{
			pLayout->addWidget(new QLabel(Title));

			pLayout->insertSpacing(0, 32);
		}

        pLayout->addWidget(pControll);

        pWidget->setLayout(pLayout);
        setDefaultWidget(pWidget);
    }
};

///////////////////////////////////////////////////
// QCheckComboBox

class MISCHELPERS_EXPORT QCheckComboBox : public QComboBox
{
	Q_OBJECT
public:
	QCheckComboBox()
	{
		view()->viewport()->installEventFilter(this);
	}

	virtual bool eventFilter(QObject *watched, QEvent *e)
	{
		switch (e->type())
		{
		case QEvent::MouseButtonPress:
			break;
		case QEvent::MouseButtonRelease:
			QMouseEvent *m = static_cast<QMouseEvent *>(e);

			// is check box rect pressed
			if (m->localPos().x() < this->height()) 
			{
				QStandardItemModel *model_ = qobject_cast<QStandardItemModel *>(model());
				Qt::CheckState state = model_->data(view()->currentIndex(), Qt::CheckStateRole).value<Qt::CheckState>();

				if (state != Qt::Checked)
					state = Qt::Checked;
				else
					state = Qt::Unchecked;

				model_->setData(view()->currentIndex(), state, Qt::CheckStateRole);

				emit checkStateChanged(view()->currentIndex().row(), state);
				return true;
			}

			/*if (isVisible() && view()->rect().contains(m->pos()) && view()->currentIndex().isValid()
				&& (view()->currentIndex().flags() & Qt::ItemIsEnabled)
				&& (view()->currentIndex().flags() & Qt::ItemIsSelectable)) {
				this->hidePopup();
				this->setCurrentIndex(view()->currentIndex().row());
				return true;
			}*/

			break;
		}
		return false;
	}

signals:
	void checkStateChanged(int Index, Qt::CheckState state);
};