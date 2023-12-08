#include "stdafx.h"
#include "IniObject.h"
#include "../SandMan.h"
#include "../Views/SbieView.h"

QString CIniObject::getName() const {
    if (!m_pIni) return ""; // no m_pIni means plus config 
    return m_pIni->GetName();
}

QVariantMap CIniObject::getIniSection(const QVariantMap& Options) {
    if (!m_pIni) return QVariantMap();

    QMultiMap<QString, QVariant> Map;
    auto Section = m_pIni->GetIniSection(0,m_WithTemplate || Options["withTemplate"].toBool());
    for (auto I = Section.begin(); I != Section.end(); ++I)
        Map.insert(I->first, I->second);

    QVariantMap Data;
    foreach(const QString & Key, Map.uniqueKeys())
        Data[Key] = Map.values(Key);
    return Data;
}

bool CIniObject::setIniValue(const QString& Setting, const QVariant& Value) {
    if (!m_pIni) return theConf->SetValue(Setting, Value);
    return !m_pIni->SetText(Setting, Value.toString()).IsError();
}

bool CIniObject::insertIniValue(const QString& Setting, const QString& Value) {
    if (!m_pIni) return false;
    return !m_pIni->InsertText(Setting, Value).IsError();
}

bool CIniObject::appendIniValue(const QString& Setting, const QString& Value) {
    if (!m_pIni) return false;
    return !m_pIni->AppendText(Setting, Value).IsError();
}

bool CIniObject::delIniValue(const QString& Setting, const QString& Value) {
    if (!m_pIni) { theConf->DelValue(Setting); return true; }
    return !m_pIni->DelValue(Setting, Value).IsError();
}

QStringList CIniObject::getIniValues(const QString& Setting, const QVariantMap& Options) {
    if (!m_pIni) return QStringList();
    return m_pIni->GetTextList(Setting, m_WithTemplate || Options["withTemplate"].toBool(), Options["expand"].toBool(), Options["withGlobal"].toBool());
}

QVariant CIniObject::getIniValue(const QString& Setting, const QVariantMap& Options) {
    if (!m_pIni) return theConf->GetValue(Setting);
    if(Options["type"] == "bool")
        return m_pIni->GetBool(Setting, Options["default"].toBool(), Options["withGlobal"].toBool(), m_WithTemplate || Options["withTemplate"].toBool());
    QString Text = m_pIni->GetText(Setting, Options["default"].toString(), Options["withGlobal"].toBool(), !Options["expand"].toBool(), m_WithTemplate || Options["withTemplate"].toBool());
    if (Options["type"] == "number")
        return Text.toLongLong();
    return Text;
}

bool CIniObject::removeSection() {
    SB_STATUS Status = m_pIni->RemoveSection();
    return !Status.IsError();
}


void CSTmplObject::ApplyChanges(bool bApply) {
    CWizardEngine* pEngine = qobject_cast<CWizardEngine*>(parent());
    pEngine->SetApplyShadow(m_OriginalName, bApply);
}