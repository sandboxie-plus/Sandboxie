#include "uexception.h"

UException::UException(const QString& message) throw()
    : Message(message.toLocal8Bit())
{
}

UException::~UException() throw () {
}

const char* UException::what() const throw () {
    return Message.data();
}
