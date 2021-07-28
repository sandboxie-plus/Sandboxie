#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QStringList>
#include <QKeyEvent>

#include "uexception.h"
#include "uglobal.h"

class UGLOBALHOTKEY_EXPORT UKeySequence : public QObject
{
    Q_OBJECT
public:
    explicit UKeySequence(QObject *parent = 0);
    explicit UKeySequence(const QString& str, QObject *parent = 0);
    void FromString(const QString& str);
    QString ToString();
    void AddKey(int key);
    void AddKey(const QString& key);
    void AddModifiers(Qt::KeyboardModifiers mod);
    void AddKey(const QKeyEvent* event);
    inline size_t Size() const {
        return Keys.size();
    }
    inline int operator [] (size_t n) const {
        if ((int)n > Keys.size()) {
            throw UException("Out of bounds");
        }
        return Keys[n];
    }
    QVector<int> GetSimpleKeys() const;
    QVector<int> GetModifiers() const;
private:
    QVector<int> Keys;
};

