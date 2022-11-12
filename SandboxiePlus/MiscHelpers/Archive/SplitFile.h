#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CSplitFile : public QIODevice
{
	Q_OBJECT
public:
	CSplitFile(const QString &name, quint64 size = -1);
	~CSplitFile();

	virtual bool open(OpenMode flags);
	virtual void close();
	virtual qint64 size() const;
    virtual qint64 pos() const;
    virtual bool seek(qint64 offset);
	virtual bool atEnd() const			{return pos() >= size();}

protected:
	virtual qint64	readData(char *data, qint64 maxlen);
    virtual qint64	writeData(const char *data, qint64 len);

	QString				GetPartName(int Index);

	quint64				m_PartSize;
	QString				m_FileName;

	QVector<QFile*>		m_FileParts;

	quint64				m_Position;
	int					m_Index;
};
