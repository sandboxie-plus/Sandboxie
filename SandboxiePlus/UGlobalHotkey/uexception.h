#pragma once

#include <exception>
#include <QString>
#include <QByteArray>
#include "uglobal.h"

class UGLOBALHOTKEY_EXPORT UException : public std::exception
{
public:
    UException(const QString& message) throw();
    const char* what() const throw();
    ~UException() throw ();
private:
    QByteArray Message;
};
