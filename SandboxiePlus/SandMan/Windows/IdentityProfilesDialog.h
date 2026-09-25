#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <functional>

#include "../Helpers/IdentityProfile.h"

class CIdentityProfileEditDialog : public QDialog
{
	Q_OBJECT
public:
	CIdentityProfileEditDialog(CIdentityProfileStore& Store, const CIdentityProfile& Profile, QWidget* parent = Q_NULLPTR);

	const CIdentityProfile& GetProfile() const { return m_Profile; }

	// Programmatic access used by the dialog smoke tests.
	void SetName(const QString& Name) { m_pName->setText(Name); }
	void AddVolume(const QString& Device, const QString& Serial);
	void RemoveSelectedVolume();
	void RegenerateSerials();
	bool Save();
	QString GetLastError() const { return m_LastError; }
	QStringList GetVolumesText() const;

private slots:
	void OnAdd();
	void OnRemove();
	void OnRegenerate();
	void OnSave();

private:
	bool CollectProfile(CIdentityProfile& Profile, QString* pError);

	CIdentityProfileStore& m_Store;
	CIdentityProfile m_Profile;
	QString m_LastError;

	QLineEdit* m_pName;
	QTableWidget* m_pVolumes;
	QLabel* m_pInfo;
};

class CIdentityProfilesDialog : public QDialog
{
	Q_OBJECT
public:
	typedef std::function<QStringList(const QString& ProfileId)> UsersFunc;

	CIdentityProfilesDialog(CIdentityProfileStore& Store, UsersFunc Users, QWidget* parent = Q_NULLPTR);

	void Reload();
	QString GetSelectedId() const;
	void SelectId(const QString& Id);
	int GetProfileCount() const { return m_pList->topLevelItemCount(); }
	QString GetLastError() const { return m_LastError; }

	// Non-interactive variants used by the smoke tests.
	bool CloneSelected(const QString& NewName);
	bool RegenerateSelected();
	bool DeleteSelected(bool Force);
	bool ImportFile(const QString& Path, QString* pNewId = nullptr);
	bool ExportSelected(const QString& Path);

private slots:
	void OnNew();
	void OnEdit();
	void OnClone();
	void OnRegenerate();
	void OnImport();
	void OnExport();
	void OnDelete();
	void OnSelectionChanged();

private:
	void Report(const QString& Text);

	CIdentityProfileStore& m_Store;
	UsersFunc m_Users;
	QString m_LastError;

	QTreeWidget* m_pList;
	QLabel* m_pInfo;
	QPushButton* m_pEdit;
	QPushButton* m_pClone;
	QPushButton* m_pRegenerate;
	QPushButton* m_pExport;
	QPushButton* m_pDelete;
};
