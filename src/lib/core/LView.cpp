#include <private/LViewPrivate.h>
#include <private/LScenePrivate.h>
#include <LOutput.h>

using namespace Louvre;

LView::LView(UInt32 type, LView *parent)
{
    m_imp = new LViewPrivate();
    imp()->type = type;
    setParent(parent);
}

LView::~LView()
{
    setParent(nullptr);
    delete m_imp;
}

LScene *LView::scene() const
{
    if (imp()->scene)
        return imp()->scene;

    if (parent())
        return parent()->scene();

    return nullptr;
}

UInt32 LView::type() const
{
    return imp()->type;
}

void LView::repaint()
{
    for (LOutput *o : outputs())
        o->repaint();
}

LView *LView::parent() const
{
    return imp()->parent;
}

void LView::setParent(LView *view)
{
    if (parent() == view || view == this)
        return;

    if (parent())
        parent()->imp()->children.erase(imp()->parentLink);

    if (view)
    {
        view->imp()->children.push_back(this);
        imp()->parentLink = std::prev(view->imp()->children.end());
    }
    else
    {
        LScene *s = scene();

        if (s)
        {
            for (auto &pair : imp()->outputsMap)
            {
                LScene::LScenePrivate::OutputData &data = s->imp()->outputsMap[pair.first];
                data.newDamageC.addRect(pair.second.previousRectC);
            }
        }
    }

    imp()->parent = view;
}

void LView::insertAfter(LView *prev)
{
    if (!parent() || prev == this)
        return;

    if (!prev)
    {
        parent()->imp()->children.erase(imp()->parentLink);
        parent()->imp()->children.push_front(this);
        imp()->parentLink = parent()->imp()->children.begin();

        for (auto &pair : imp()->outputsMap)
            pair.second.changedOrder = true;

        repaint();
    }
    else
    {
        if (!prev->parent() || prev->parent() != parent())
            return;

        if (prev == parent()->children().back())
        {
            parent()->imp()->children.erase(imp()->parentLink);
            parent()->imp()->children.push_back(this);
            imp()->parentLink = std::prev(parent()->imp()->children.end());
        }
        else
        {
            parent()->imp()->children.erase(imp()->parentLink);
            imp()->parentLink = parent()->imp()->children.insert(std::next(prev->imp()->parentLink), this);
        }

        for (auto &pair : imp()->outputsMap)
            pair.second.changedOrder = true;

        repaint();
    }
}

std::list<Louvre::LView *> &LView::children() const
{
    return imp()->children;
}

bool LView::clippingEnabled() const
{
    return imp()->clippingEnabled;
}

void LView::enableClipping(bool enabled)
{
    imp()->clippingEnabled = enabled;
}

bool LView::inputEnabled() const
{
    return imp()->inputEnabled;
}

void LView::enableInput(bool enabled)
{
    imp()->inputEnabled = enabled;
}

bool LView::scalingEnabled() const
{
    return imp()->scalingEnabled;
}

void LView::enableScaling(bool enabled)
{
    imp()->scalingEnabled = enabled;
}

void LView::setScaledSizeC(const LSize &size)
{
    imp()->scaledSizeC = size;
}

const LSize &LView::scaledSizeC() const
{
    return imp()->scaledSizeC;
}

bool LView::mapped() const
{
    return imp()->interface.mapped(this);
}

bool LView::visible() const
{
    return imp()->visible;
}

void LView::setVisible(bool visible)
{
    imp()->visible = visible;
}

Float32 LView::opacity() const
{
    if (parent())
        return parent()->opacity() * imp()->opacity;

    return imp()->opacity;
}

void LView::setOpacity(Float32 opacity)
{
    imp()->opacity = opacity;
}
