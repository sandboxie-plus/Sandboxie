#pragma once

#include "ArchiveHelper.h"

#ifdef USE_7Z

class CArchiveIO: public IOutStream, public IInStream, public CMyUnknownImp
{
	Z7_COM_UNKNOWN_IMP_2(IOutStream, IInStream)
public:

	CArchiveIO(QIODevice* pFile, QIODevice::OpenMode Mode, bool bDelete = true)
	{
		Q_ASSERT(pFile);
		m_pFile = pFile;
		if(Mode != QIODevice::NotOpen)
			m_pFile->open(Mode);
		m_bDelete = bDelete;
	}
	~CArchiveIO()
	{
		m_pFile->close();
		if(m_bDelete)
			delete m_pFile;
	}
	
	bool Open(QIODevice::OpenMode Mode) {
		return m_pFile->open(Mode);
	}

	Z7_COM7F_IMF(Read(void *data, UInt32 size, UInt32 *processedSize))
	{
		quint64 read = m_pFile->read((char*)data, size);
		if(read == -1)
			return E_FAIL;
		if(processedSize)
			*processedSize = read;
		return S_OK;
	}

	Z7_COM7F_IMF(Write(const void *data, UInt32 size, UInt32 *processedSize))
	{
		quint64 written = m_pFile->write((const char*)data, size);
		if(written == -1)
			return E_FAIL;
		if(processedSize)
			*processedSize = written;
		return S_OK;
	}

	Z7_COM7F_IMF(Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition))
	{
		switch(seekOrigin)
		{
			case 0:								break; // FILE_BEGIN
			case 1:	offset += m_pFile->pos();	break; // FILE_CURRENT
			case 2: offset += m_pFile->size();	break; // FILE_END
			default:	Q_ASSERT(0);			return E_FAIL;
		}

		if(!m_pFile->seek(offset))
			return E_FAIL;
		if(newPosition)
			*newPosition = offset;
		return S_OK;
	}

	Z7_COM7F_IMF(SetSize(UInt64 newSize))
	{
		return S_OK;
	}

	Z7_COM7F_IMF(GetSize(UInt64 *size))
	{
		*size = m_pFile->size();
		return S_OK;
	}

protected:
	QIODevice*		m_pFile;
	bool			m_bDelete;
};

#endif