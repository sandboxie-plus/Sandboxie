#pragma once

#include "Helpers/ReadDirectoryChanges.h"
#include "SbiePlusAPI.h"

class CBoxMonitor : public QThread, public CReadDirectoryChanges
{
	Q_OBJECT
public:
	CBoxMonitor();
	~CBoxMonitor();

	virtual void Notify(const std::wstring& strDirectory);

	virtual void run();

	void WatchBox(CSandBoxPlus* pBox);
	void ScanBox(CSandBoxPlus* pBox);
	void CloseBox(CSandBoxPlus* pBox);
	void RemoveBox(CSandBoxPlus* pBox);

	bool IsScanPending(const CSandBoxPlus* pBox);

	void Stop();

private slots:
	void UpdateBox(const QString& Path);

protected:

	struct SBox
	{
		SBox() {
			ForceUpdate = false;
			Changed = false;
			IsWatched = false;
			LastScan = 0;
			ScanDuration = 0;
			TotalSize = 0;
		}

		QPointer<CSandBoxPlus> pBox;
		bool ForceUpdate;
		bool Changed;
		bool IsWatched;
		quint64 LastScan;
		quint64 ScanDuration;

		quint64 TotalSize;
	};

	quint64 CounDirSize(const QString& Dir, SBox* Box);

	QMutex m_Mutex;
	QMap<QString, SBox> m_Boxes;
	bool m_bTerminate;
};