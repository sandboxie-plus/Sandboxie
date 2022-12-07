#include "stdafx.h"
#include "SplitFile.h"


CSplitFile::CSplitFile(const QString &name, quint64 size)
{
	m_PartSize = size;
	m_FileName = name;

	m_Position = 0;
	m_Index = -1;
}

CSplitFile::~CSplitFile()
{
	QIODevice::close();
}

QString CSplitFile::GetPartName(int Index)
{
	QString Ext = QString::number(Index+1);
	while(Ext.length() < 3)
		Ext.prepend("0");
	return m_FileName + "." + Ext;
}

bool CSplitFile::open(OpenMode flags)
{
	if(flags & ReadOnly)
	{
		for(int i = 0;; i++)
		{
			QString PartName = GetPartName(i);
			if(!QFile::exists(PartName))
				break;

			QFile* pFile = new QFile(PartName, this);
			pFile->open(flags);
			m_FileParts.append(pFile);
		}
	}

	m_Index = 0; // we are open for business
	return QIODevice::open(flags);
}

void CSplitFile::close()
{
	m_Position = 0;
	m_Index = -1;
	foreach(QFile* pPart, m_FileParts)
	{
		pPart->close();
		delete pPart;
	}
	m_FileParts.clear();
	QIODevice::close();
}

qint64 CSplitFile::size() const
{
	quint64 uSize = 0;
	foreach(QFile* pPart, m_FileParts)
		uSize += pPart->size();
	return uSize;
}

qint64 CSplitFile::pos() const
{
	return m_Position;
}

bool CSplitFile::seek(qint64 offset)
{
	if(m_Index == -1)
		return false;

	qint64 iToGo = offset - m_Position;
	int i = m_Index;

	while(iToGo)
	{
		if(iToGo > 0)
		{
			qint64 tmp = m_FileParts[i]->pos();
			if(tmp + iToGo < (qint64)m_PartSize)
			{
				m_FileParts[i]->seek(tmp + iToGo);
				//iToGo = 0;
				break;
			}
			else
			{
				if(++i == m_FileParts.count())
				{
					QFile* pFile = new QFile(GetPartName(m_Index), this);
					pFile->open(openMode());
					m_FileParts.append(pFile);
				}
				iToGo -= tmp;
				m_FileParts[i]->seek(0);
			}
		}
		else if(iToGo < 0)
		{
			qint64 tmp = m_FileParts[i]->pos();
			if(tmp -iToGo >= 0)
			{
				m_FileParts[i]->seek(tmp + iToGo);
				//iToGo = 0;
				break;
			}
			else
			{
				if(--i == -1)
					return false;
				iToGo += tmp;
				m_FileParts[i]->seek(m_FileParts[i]->size()-1);
			}
		}
	}

	m_Index = i;
	m_Position = offset;
	return true;
}
	
qint64 CSplitFile::readData(char *data, qint64 maxlen)
{
	if(m_Index == -1)
		return -1;

	quint64 read = 0;
	while(maxlen - read)
	{
		if(m_Index >= m_FileParts.count())
			break;

		quint64 uToGo = maxlen - read;
		quint64 uAvail = m_FileParts[m_Index]->size() - m_FileParts[m_Index]->pos();
		if(uToGo > uAvail)
			uToGo = uAvail;

		quint64 Tmp = m_FileParts[m_Index]->read(data + read, uToGo);
		if(Tmp == -1)
			return -1;
		read += Tmp;
		m_Position += Tmp;
		
		if(m_FileParts[m_Index]->pos() >= m_FileParts[m_Index]->size())
			m_Index++;
	}
	return read;
}

qint64 CSplitFile::writeData(const char *data, qint64 len)
{
	if(m_Index == -1)
		return -1;

	quint64 written = 0;
	while(len - written)
	{
		if(m_Index >= m_FileParts.count())
		{
			QFile* pFile = new QFile(GetPartName(m_Index), this);
			pFile->open(openMode());
			m_FileParts.append(pFile);
		}

		quint64 uToGo = len - written;
		quint64 uAvail = m_PartSize - m_FileParts[m_Index]->pos();
		if(uToGo > uAvail)
			uToGo = uAvail;

		quint64 Tmp = m_FileParts[m_Index]->write(data + written, uToGo);
		if(Tmp == -1)
			return -1;
		written += Tmp;
		m_Position += Tmp;

		if(m_FileParts[m_Index]->pos() >= (qint64)m_PartSize)
			m_Index++;
	}
	return written;
}
