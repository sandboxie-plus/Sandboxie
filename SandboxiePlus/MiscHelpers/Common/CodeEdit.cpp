#include "stdafx.h"
#include "CodeEdit.h"


#define TAB_SPACES "   "

CCodeEdit::CCodeEdit(QSyntaxHighlighter* pHighlighter, QWidget* pParent)
 : QWidget(pParent)
{
	m_pMainLayout = new QGridLayout(this);
	m_pMainLayout->setContentsMargins(0,0,0,0);
	setLayout(m_pMainLayout);

	m_pSourceCode = new QTextEdit();
	QFont Font = m_pSourceCode->font();
	Font.setFamily("Courier New");
	Font.setPointSize(10);
	m_pSourceCode->setFont(Font);
	m_pSourceCode->setLineWrapMode(QTextEdit::NoWrap);
	if(pHighlighter)
		pHighlighter->setDocument(m_pSourceCode->document());
	//m_pSourceCode->setTabStopWidth (QFontMetrics(Font).width(TAB_SPACES));
	m_pMainLayout->addWidget(m_pSourceCode, 0, 0);

	connect(m_pSourceCode, SIGNAL(textChanged()), this, SIGNAL(textChanged()));

	// hot keys
	m_pFind = new QAction(tr("Find"),this);
	m_pFind->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
	connect(m_pFind, SIGNAL(triggered()), this, SLOT(OnFind()));
	m_pSourceCode->addAction(m_pFind);

	m_pFindNext = new QAction(tr("FindNext"),this);
	QList<QKeySequence> Finds;
	Finds << QKeySequence(Qt::Key_F3);
	Finds << QKeySequence(Qt::SHIFT | Qt::Key_F3) << QKeySequence(Qt::CTRL | Qt::Key_F3) << QKeySequence(Qt::ALT | Qt::Key_F3);
	Finds << QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F3) << QKeySequence(Qt::SHIFT | Qt::ALT | Qt::Key_F3) << QKeySequence(Qt::SHIFT | Qt::CTRL | Qt::Key_F3);
	Finds << QKeySequence(Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::Key_F3);
	m_pFindNext->setShortcuts(Finds);
	connect(m_pFindNext, SIGNAL(triggered()), this, SLOT(OnFindNext()));
	m_pSourceCode->addAction(m_pFindNext);

	m_pGoTo = new QAction(tr("GoTo"),this);
	m_pGoTo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
	connect(m_pGoTo, SIGNAL(triggered()), this, SLOT(OnGoTo()));
	m_pSourceCode->addAction(m_pGoTo);


	/*m_pComment = new QAction(tr("Comment"),this);
	m_pComment->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
	connect(m_pComment, SIGNAL(triggered()), this, SLOT(OnComment()));
	m_pSourceCode->addAction(m_pComment);

	m_pUnComment = new QAction(tr("UnComment"),this);
	m_pUnComment->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
	connect(m_pUnComment, SIGNAL(triggered()), this, SLOT(OnUnComment()));
	m_pSourceCode->addAction(m_pUnComment);*/
}

#define ADD_HISTORY(list,entry)	\
	list.removeAll(entry);		\
	list.prepend(entry);		\
	while(list.size() > 10)		\
		list.removeLast();

void CCodeEdit::OnFind()
{
	static QStringList Finds;
	bool bOK = false;
	m_CurFind = QInputDialog::getItem (this, tr("Find"),tr("F3: Find Next\n+ Shift: Backward\n+ Ctrl: Case Sensitively\n+ Alt: Whole Words\n\nFind String:") + QString(160,' '), Finds, 0, true, &bOK);
	if (!bOK)
		return;
	ADD_HISTORY(Finds, m_CurFind);
	OnFindNext();
}

void CCodeEdit::OnFindNext()
{
	if(m_CurFind.isEmpty())
		return;

	QTextDocument::FindFlags Flags = QTextDocument::FindFlags();
	Qt::KeyboardModifiers Mods = QApplication::keyboardModifiers();
	if(Mods & Qt::ShiftModifier)
		Flags |= QTextDocument::FindBackward;
	if(Mods & Qt::ControlModifier)
		Flags |= QTextDocument::FindCaseSensitively;
	if(Mods & Qt::AltModifier)
		Flags |= QTextDocument::FindWholeWords;

	m_pSourceCode->find(m_CurFind, Flags);
}

void CCodeEdit::OnGoTo()
{
	int iLine = QInputDialog::getText(this, tr("Go to Line:"),tr(""), QLineEdit::Normal, "").toInt();
	if(!iLine)
		return;
	
	QTextCursor Cursor = m_pSourceCode->textCursor();   
	Cursor.movePosition(QTextCursor::Start);
	while(iLine-- > 1)
		Cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
	//Cursor.select(QTextCursor::LineUnderCursor);
	m_pSourceCode->setTextCursor(Cursor);
}


/*void CCodeEdit::OnComment()
{
	QTextCursor Cursor = m_pSourceCode->textCursor();
	int Start = Cursor.selectionStart();
	int End = Cursor.selectionEnd();
	QString Text = m_pSourceCode->toPlainText();
	QString Fragment = Text.mid(Start, End - Start);

	Fragment.replace(QRegExp("^"),"'");
	Fragment.replace(QRegExp("\r?\n"),"\r\n'");

	m_pSourceCode->insertPlainText(Fragment);
}

void CCodeEdit::OnUnComment()
{
	QTextCursor Cursor = m_pSourceCode->textCursor();
	int Start = Cursor.selectionStart();
	int End = Cursor.selectionEnd();
	QString Text = m_pSourceCode->toPlainText();
	QString Fragment = Text.mid(Start, End - Start);

	Fragment.replace(QRegExp("^[ \t]*'"),"");
	Fragment.replace(QRegExp("\r?\n[ \t]*'"),"\r\n");

	m_pSourceCode->insertPlainText(Fragment);
}*/