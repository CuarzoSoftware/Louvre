#include <private/LXCursorPrivate.h>

#include <LTexture.h>

using namespace Louvre;

LXCursor::LXCursor()
{
    m_imp = new LXCursorPrivate();
}

LXCursor::~LXCursor()
{
    delete imp()->texture;
    delete m_imp;
}

LTexture *LXCursor::texture() const
{
    return imp()->texture;
}

const LSize &LXCursor::sizeB() const
{
    return imp()->texture->sizeB();
}

const LPoint &LXCursor::hotspotB() const
{
    return imp()->hotspotB;
}
