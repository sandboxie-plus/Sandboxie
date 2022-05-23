#pragma once

#include "Helpers/ReadDirectoryChanges.h"
#include "SbiePlusAPI.h"

class CBoxMonitor : public QThread, public CReadDirectoryChanges
{
	Q_OBJECT
public:
	CBoxMonitor();
	~CBoxMonitor();

	virtual void Notify(const wstring& strDirectory);

	virtual void run();

	void AddBox(CSandBoxPlus* pBox, bool AndWatch = false);
	void CloseBox(CSandBoxPlus* pBox, bool AndClear = false);

private slots:
	void UpdateBox(const QString& Path);

protected:

	struct SBox
	{
		SBox() {
			Changed = false;
			Closed = false;
			LastScan = 0;
			ScanDuration = 0;
			TotalSize = 0;
		}

		QPointer<CSandBoxPlus> pBox;
		bool Changed;
		bool Closed;
		quint64 LastScan;
		quint64 ScanDuration;

		quint64 TotalSize;
	};

	quint64 CounDirSize(const QString& Dir, SBox* Box);

	QMutex m_Mutex;
	QMap<QString, SBox> m_Boxes;
	bool m_bTerminate;
};