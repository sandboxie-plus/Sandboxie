#pragma once

QVariantMap ResolveShortcut(const QString& LinkPath);

QPixmap LoadWindowsIcon(const QString& Path, quint32 Index);

bool PickWindowsIcon(QWidget* pParent, QString& Path, quint32& Index);

void ProtectWindow(void* hWnd, unsigned long affinity = 0x00000011); // Default to WDA_EXCLUDEFROMCAPTURE (0x00000011)

QString GetProductVersion(const QString& filePath);

bool KillProcessByWnd(const QString& WndName);

bool CheckInternet();

QVariantList EnumNICs();