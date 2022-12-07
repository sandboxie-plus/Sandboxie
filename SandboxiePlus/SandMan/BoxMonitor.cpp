#include "stdafx.h"
#include "BoxMonitor.h"
#include "../MiscHelpers/Common/Common.h"

CBoxMonitor::CBoxMonitor() 
{ 
	m_bTerminate = false;
}

CBoxMonitor::~CBoxMonitor() 
{
	Stop();
}

void CBoxMonitor::Notify(const std::wstring& strDirectory)
{
	m_Mutex.lock();
	m_Boxes[QString::fromStdWString(strDirectory)].Changed = true;
	m_Mutex.unlock();
}

quint64 CBoxMonitor::CounDirSize(const QString& Directory, SBox* Box) 
{
	quint64 TotalSize = 0;
	if (m_bTerminate || Box->pBox.isNull())
		return TotalSize;

	QDir Dir(Directory);
	foreach(const QFileInfo & Info, Dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot))
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
		Sleep(100);

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

			if ((Box->Changed && (!Box->IsWatched || Box->LastScan == 0 || (CurTick - Box->LastScan) > MinScanInterval)) || Box->ForceUpdate) {

				qDebug() << "Rescanning:" << Key << "(" + QDateTime::currentDateTime().toString() + ")";

				quint64 ScanStart = GetCurTick();

				Box->ScanDuration = -1;

				Box->TotalSize = CounDirSize(Key, Box);

				Box->ScanDuration = GetCurTick() - ScanStart;
				Box->LastScan = GetCurTick();

				QMetaObject::invokeMethod(this, "UpdateBox", Qt::QueuedConnection,
					//Q_RETURN_ARG(int, retVal),
					Q_ARG(QString, Key)
				);

				Box->Changed = false;
				Box->ForceUpdate = false;
			}

			m_Mutex.lock();
			if (Box->pBox.isNull())
				m_Boxes.remove(Key);
			m_Mutex.unlock();
		}
	}
}

void CBoxMonitor::UpdateBox(const QString& Path)
{
	// Note: this private function runs in the main thread

	m_Mutex.lock();
	SBox Box = m_Boxes.value(Path);
	m_Mutex.unlock();

	if (Box.pBox)
		Box.pBox->SetSize(Box.TotalSize);
}

void CBoxMonitor::WatchBox(CSandBoxPlus* pBox)
{
	QMutexLocker Lock(&m_Mutex);
	if (!isRunning()) start();

	SBox& Box = m_Boxes[pBox->GetFileRoot()];
	Box.pBox = pBox;

	Box.IsWatched = true;
	AddDirectory(pBox->GetFileRoot().toStdWString().c_str(), true, FILE_NOTIFY_CHANGE_SIZE);
}

void CBoxMonitor::ScanBox(CSandBoxPlus* pBox)
{
	QMutexLocker Lock(&m_Mutex);
	if (!isRunning()) start();

	SBox& Box = m_Boxes[pBox->GetFileRoot()];
	Box.pBox = pBox;

	Box.ForceUpdate = true;
}

void CBoxMonitor::CloseBox(CSandBoxPlus* pBox)
{
	QMutexLocker Lock(&m_Mutex);
	if (!isRunning()) return;

	SBox& Box = m_Boxes[pBox->GetFileRoot()];

	if(Box.IsWatched)
		DetachDirectory(pBox->GetFileRoot().toStdWString().c_str());
	Box.IsWatched = false;

	//Box.Changed = true;
}

void CBoxMonitor::RemoveBox(CSandBoxPlus* pBox)
{
	QMutexLocker Lock(&m_Mutex);
	if (!isRunning()) return;

	auto I = m_Boxes.find(pBox->GetFileRoot());
	if (I == m_Boxes.end())
		return;

	if(I->IsWatched)
		DetachDirectory(pBox->GetFileRoot().toStdWString().c_str());
	I->pBox.clear();
}

bool CBoxMonitor::IsScanPending(const CSandBoxPlus* pBox)
{
	QMutexLocker Lock(&m_Mutex);
	if (!isRunning()) false;

	auto I = m_Boxes.find(pBox->GetFileRoot());
	if (I == m_Boxes.end())
		return false;

	return (I->Changed && !I->IsWatched) || I->ForceUpdate || I->ScanDuration == -1;
}

void CBoxMonitor::Stop()
{
	if (!isRunning()) return;

	m_bTerminate = true;

	if (!wait(10 * 1000)) {
		terminate();
		qDebug() << "Failed to stop monitor thread, terminating!!!";
	}

	QMutexLocker Lock(&m_Mutex);

	while (!m_Boxes.isEmpty()) {
		SBox Box = m_Boxes.take(m_Boxes.firstKey());
		if(Box.IsWatched && Box.pBox)
			DetachDirectory(Box.pBox->GetFileRoot().toStdWString().c_str());
	}

	m_bTerminate = false;
}
