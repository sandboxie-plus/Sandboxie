#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(MISCHELPERS_LIB)
#  define MISCHELPERS_EXPORT Q_DECL_EXPORT
# else
#  define MISCHELPERS_EXPORT Q_DECL_IMPORT
# endif
#else
# define MISCHELPERS_EXPORT
#endif
