#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(QSBIEAPI_LIB)
#  define QSBIEAPI_EXPORT Q_DECL_EXPORT
# else
#  define QSBIEAPI_EXPORT Q_DECL_IMPORT
# endif
#else
# define QSBIEAPI_EXPORT
#endif
