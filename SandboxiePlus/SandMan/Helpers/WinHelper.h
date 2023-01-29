#pragma once

QVariantMap ResolveShortcut(const QString& LinkPath);

QPixmap LoadWindowsIcon(const QString& Path, quint32 Index);

bool PickWindowsIcon(QWidget* pParent, QString& Path, quint32& Index);