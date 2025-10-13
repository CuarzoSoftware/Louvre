#include <CZ/Louvre/LLog.h>

using namespace CZ;

const CZ::CZLogger &LouvreLogGet() noexcept
{
    static CZLogger logger { "Louvre", "CZ_LOUVRE_LOG_LEVEL" };
    return logger;
}
