#pragma once
#include <private/qabstractfileengine_p.h>
#include "Archive.h"

#include "../mischelpers_global.h"

//////////////////////////////////////////////////////////////////////////////////////////
// C7zFileEngine
// 

class MISCHELPERS_EXPORT C7zFileEngine : public QAbstractFileEngine
{
protected:
    friend class C7zFileEngineHandler;
    C7zFileEngine(const QString& filename, CArchive* pArchive, QMutex* pMutex);

public:
    virtual ~C7zFileEngine();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    virtual bool open(QIODevice::OpenMode openMode);
#else
    virtual bool open(QIODevice::OpenMode openMode, std::optional<QFile::Permissions> permissions);
#endif
    virtual bool close();
    virtual bool flush();
    virtual qint64 size() const;
    virtual qint64 pos() const;
    virtual bool seek(qint64 pos);
    virtual bool isSequential() const;
    virtual bool remove();
    virtual bool mkdir(const QString& dirName, bool createParentDirectories) const;
    virtual bool rmdir(const QString& dirName, bool recurseParentDirectories) const;
    virtual bool caseSensitive() const;
    virtual bool isRelativePath() const;
    virtual FileFlags fileFlags(FileFlags type = FileInfoAll) const;
    virtual QString fileName(FileName file = DefaultName) const;
    virtual QDateTime fileTime(FileTime time) const;
    virtual void setFileName(const QString& file);
    bool atEnd() const;

    virtual qint64 read(char* data, qint64 maxlen);
    virtual qint64 readLine(char* data, qint64 maxlen);
    virtual qint64 write(const char* data, qint64 len);

    bool isOpened() const;

    QAbstractFileEngine::Iterator* beginEntryList(QDir::Filters filters, const QStringList& filterNames);

    QFile::FileError error() const;
    QString errorString() const;

    virtual bool supportsExtension(Extension extension) const;

private:
    qint64 _size;
    FileFlags _flags;
    QString _filename;
    QDateTime _datetime;
    int _index;

    QByteArray _data;
    qint64 _pos;

    CArchive* _pArchive;
    QMutex* _pMutex;
};

//////////////////////////////////////////////////////////////////////////////////////////
// C7zFileEngineHandler
// 

class MISCHELPERS_EXPORT C7zFileEngineHandler : public QObject, public QAbstractFileEngineHandler
{
public:
    C7zFileEngineHandler(const QString& Scheme, QObject* parent = NULL);
    ~C7zFileEngineHandler();

    bool Open(const QString& ArchivePath);
    void Close();

    bool IsOpen()       { return m_pArchive != NULL; }
    QString Prefix()    { return m_Scheme; }

    QAbstractFileEngine* create(const QString& filename) const;

private:
    QString         m_Scheme;
    CArchive*       m_pArchive;
    mutable QMutex  m_Mutex;
};