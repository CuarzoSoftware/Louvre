#include <private/LOutputManagerPrivate.h>
#include <private/LCompositorPrivate.h>

#include <LOutput.h>

using namespace Louvre;

LOutputManager::LOutputManager(Params *params)
{
    m_imp = new LOutputManagerPrivate();
    m_imp->compositor = params->compositor;
}

LOutputManager::~LOutputManager()
{
    delete m_imp;
}

LCompositor *LOutputManager::compositor() const
{
    return m_imp->compositor;
}

const list<LOutput *> *LOutputManager::outputs() const
{
    return m_imp->compositor->imp()->graphicBackend->getConnectedOutputs(m_imp->compositor);
}

