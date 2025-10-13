#ifndef CZ_LLOG_H
#define CZ_LLOG_H

#include <CZ/Core/CZLogger.h>

#define LLog LouvreLogGet()

const CZ::CZLogger &LouvreLogGet() noexcept;

#endif // CZ_LLOG_H
