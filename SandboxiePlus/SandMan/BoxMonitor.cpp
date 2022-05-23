#include "stdafx.h"
#include "BoxMonitor.h"
#include "../MiscHelpers/Common/Common.h"

CBoxMonitor::CBoxMonitor() 
{ 
	m_bTerminate = false;
	start();
}

CBoxMonitor::~CBoxMonitor() 
{
	m_bTerminate = true;
	if (!wait(10 * 1000))
		terminate();
}

void CBoxMonitor::Notify(const wstring& strDirectory)
{
	m_Mutex.lock();
	m_Boxes[QString::fromStdWString(strDirectory)].Changed = true;
	m_Mutex.unlock();
}

quint64 CBoxMonitor::CounDirSize(const QString& Directory, SBox* Box) 
{
	quint64 TotalSize = 0;
	if (Box->pBox.isNull() || m_bTerminate)
		return TotalSize;

	QDir Dir(Directory);
	foreach(const QFileInfo & Info, Dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot))
	{
		if (Info.isDir())
			TotalSize += CounDirSize(Info.filePath(), Box);
		else
			TotalSize += QFile(Info.filePath()).size();
	}

	return TotalSize;
}

void CBoxMonitor::run()
{
	while (!m_bTerminate) 
	{
		m_Mutex.lock();
		QList<QString> Keys = m_Boxes.keys();
		m_Mutex.unlock();

		quint64 CurTick = GetCurTick();

		foreach(const QString& Key, Keys) {

			m_Mutex.lock();
			SBox* Box = &m_Boxes[Key];
			m_Mutex.unlock();

			quint64 MinScanInterval = Box->ScanDuration * 100;
			if (MinScanInterval < 30 * 1000)
				MinScanInterval = 30 * 1000;
			if (MinScanInterval > 30 * 60 * 1000)
				MinScanInterval = 30 * 60 * 1000;

			if (Box->Changed && (Box->Closed || Box->LastScan == 0 || (CurTick - Box->LastScan) > MinScanInterval)) {

				qDebug() << "Rescanning:" << Key << "(" + QDateTime::currentDateTime().toString() + ")";

				quint64 ScanStart = GetCurTick();

				Box->TotalSize = CounDirSize(Key, Box);

				Box->ScanDuration = GetCurTick() - ScanStart;
				Box->LastScan = GetCurTick();

				QMetaObject::invokeMethod(this, "UpdateBox", Qt::QueuedConnection,
					//Q_RETURN_ARG(int, retVal),
					Q_ARG(QString, Key)
				);

				Box->Changed = false;
			}

			m_Mutex.lock();
			if (Box->pBox.isNull())
				m_Boxes.remove(Key);
			m_Mutex.unlock();
		}

		Sleep(1000);
	}
}

void CBoxMonitor::UpdateBox(const QString& Path)
{
	// Note: this functin runs in the main thread

	m_Mutex.lock();
	SBox Box = m_Boxes.value(Path);
	m_Mutex.unlock();

	if (Box.pBox)
		Box.pBox->SetSize(Box.TotalSize);
}

void CBoxMonitor::AddBox(CSandBoxPlus* pBox, bool AndWatch)
{
	QMutexLocker Lock(&m_Mutex);

	m_Boxes[pBox->GetFileRoot()].pBox = pBox;

	if (AndWatch) {
		m_Boxes[pBox->GetFileRoot()].Closed = false;
		AddDirectory(pBox->GetFileRoot().toStdWString().c_str(), true, FILE_NOTIFY_CHANGE_SIZE);
	}
	else
		m_Boxes[pBox->GetFileRoot()].Changed = true;
}

void CBoxMonitor::CloseBox(CSandBoxPlus* pBox, bool AndClear)
{
	QMutexLocker Lock(&m_Mutex);

	if(AndClear)
		m_Boxes[pBox->GetFileRoot()].pBox.clear();
	m_Boxes[pBox->GetFileRoot()].Closed = true;

	DetachDirectory(pBox->GetFileRoot().toStdWString().c_str());
}