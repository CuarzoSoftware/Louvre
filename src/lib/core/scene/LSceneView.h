#ifndef LSCENEVIEW_H
#define LSCENEVIEW_H

#include <LCompositor.h>
#include <LRenderBuffer.h>
#include <LPainter.h>
#include <LOutput.h>
#include <LView.h>
#include <LCursor.h>

#define LSCENE_MAX_AGE 5

/**
 * @brief View for rendering other views
 *
 * An LSceneView is a unique type of view. Instead of rendering its content directly into an LOutput,
 * it possesses its own framebuffer. This capability allows for advanced blending effects, such as applying masks (check LView::setBlendFunc()).
 *
 * Unlike other with other views, child views of an LSceneView are always clipped to its boundaries.\n
 * LSceneViews can function as children of other views or LSceneViews (just like any other view), but they can also be used independently.\n
 * For example, you might create an LSceneView, populate it with arranged views, execute the render() function,
 * and then use the output as a texture for other porpuses.
 *
 * The main view of the LScene class (LScene::mainView()) is a unique LSceneView designed to render its content onto one or more LOutputs instead of using its own framebuffer.\n
 *
 * @warning Use LSceneViews judiciously. When nested within another scene, they are rendered twice: first, into to its framebuffer, and then into an LOutput framebuffer or another LSurfaceView parent.
 */
class Louvre::LSceneView : public LView
{
public:   
    /**
     * @brief Construct an LSceneView with a specified buffer size, buffer scale, and optional parent.
     *
     * @param sizeB The size of the framebuffer for the scene view.
     * @param bufferScale The scale factor applied to the framebuffer.
     * @param parent The parent view that will contain this scene view.
     */
    LSceneView(const LSize &sizeB, Float32 bufferScale, LView *parent = nullptr) noexcept :
        LView(LView::SceneType, true, parent),
        m_fb(new LRenderBuffer(sizeB))
    {
        static_cast<LRenderBuffer*>(m_fb)->setScale(bufferScale);
    }

    LCLASS_NO_COPY(LSceneView)

    /**
     * @brief Destructor for the LSceneView.
     */
    ~LSceneView() noexcept;

    /**
     * @brief Retrieve the clear color of the scene view.
     *
     * @return The clear color in RGBA format.
     */
    const LRGBAF &clearColor() const noexcept
    {
        return m_clearColor;
    }

    /**
     * @brief Set the clear color of the scene view using an LRGBAF color.
     *
     * @param color The clear color in LRGBAF format.
     */
    void setClearColor(const LRGBAF &color) noexcept
    {
        if (m_clearColor.r == color.r &&
            m_clearColor.g == color.g &&
            m_clearColor.b == color.b &&
            m_clearColor.a == color.a)
            return;

        m_clearColor = color;

        if (!repaintCalled() && mapped())
            repaint();
    }

    /**
     * @brief Apply damage to all areas of the scene view for a specific output.
     *
     * @param output The output for which to apply damage.
     */
    void damageAll(LOutput *output) noexcept;

    /**
     * @brief Add specific damage areas to the scene view for a specific output.
     *
     * @param output The output for which to add damage areas.
     * @param damage The damaged regions to be added.
     */
    void addDamage(LOutput *output, const LRegion &damage) noexcept;

    /**
     * @brief Render the scene.
     *
     * This method initiates rendering for the view, excluding specified regions if provided.
     *
     * @note The rendered content can be accessed as a texture using the texture() method.
     *
     * @param exclude Regions to be excluded from rendering.
     */
    void render(const LRegion *exclude = nullptr) noexcept;

    /**
     * @brief Retrieve the texture associated with the view.
     *
     * This method returns the texture linked to the view at a specified index.\n
     * LSceneViews always have a single texture, with the exception of the main view of an LScene. The main view may possess
     * multiple textures, depending on the current LOutput thread and hardware configuration.
     *
     * @param index The index of the texture to retrieve (default is 0).
     * @return A pointer to the texture associated with the view.
     */
    LTexture *texture(Int32 index = 0) const noexcept
    {
        return (LTexture*)m_fb->texture(index);
    }

    /**
     * @brief Set the position of the scene.
     *
     * @param pos The new position of the scene.
     */
    void setPos(const LPoint &pos) noexcept
    {
        setPos(pos.x(), pos.y());
    }

    /**
     * @brief Set the position of the scene using X and Y coordinates.
     *
     * @param x The X-coordinate of the new position.
     * @param y The Y-coordinate of the new position.
     */
    void setPos(Int32 x, Int32 y) noexcept
    {
        if (x != m_customPos.x() || y != m_customPos.y())
        {
            m_customPos.setX(x);
            m_customPos.setY(y);

            if (!isLScene())
                static_cast<LRenderBuffer*>(m_fb)->setPos(m_customPos);

            if (!repaintCalled() && mapped())
                repaint();
        }
    }

    /**
     * @brief Set the size of the scene framebuffer.
     *
     * @warning Changing the framebuffer size will destroy the current framebuffer and create a new one. Therefore, this operation should not be performed frequently.
     *
     * @note Calling this method on the main view of an LScene is a no-op. Instead, use LOutput::setMode() to control the size of the output framebuffer.
     *
     * @param size The new size of the framebuffer.
     */
    void setSizeB(const LSize &size) noexcept
    {
        if (!isLScene() && size != m_fb->sizeB())
        {
            static_cast<LRenderBuffer*>(m_fb)->setSizeB(size);

            for (LOutput *o : compositor()->outputs())
                damageAll(o);
            repaint();
        }
    }

    /**
     * @brief Set the scale factor for the scene framebuffer.
     *
     * @param scale The new scale factor to be applied.
     */
    void setScale(Float32 scale) noexcept
    {
        if (!isLScene() && bufferScale() != scale)
        {
            static_cast<LRenderBuffer*>(m_fb)->setScale(scale);
            for (LOutput *o : compositor()->outputs())
                damageAll(o);
            repaint();
        }
    }

    virtual bool nativeMapped() const noexcept override;
    virtual const LPoint &nativePos() const noexcept override;
    virtual const LSize &nativeSize() const noexcept override;
    virtual Float32 bufferScale() const noexcept override;
    virtual void enteredOutput(LOutput *output) noexcept override;
    virtual void leftOutput(LOutput *output) noexcept override;
    virtual const std::vector<LOutput*> &outputs() const noexcept override;
    virtual void requestNextFrame(LOutput *output) noexcept override;
    virtual const LRegion *damage() const noexcept override;
    virtual const LRegion *translucentRegion() const noexcept override;
    virtual const LRegion *opaqueRegion() const noexcept override;
    virtual const LRegion *inputRegion() const noexcept override;
    virtual void paintEvent(const PaintEventParams &params) noexcept override;

protected:
    class ThreadData : public LObject
    {
    public:
        LRegion damageRing[LSCENE_MAX_AGE];
        Int32 damageRingIndex { 0 };
        LRegion newDamage;
        LRegion manuallyAddedDamage;
        LRegion prevExternalExclude;
        LRegion opaqueSum;
        LRegion translucentSum;
        LRect prevRect;
        LPainter *p { nullptr };
        LOutput *o { nullptr };
        LBox *boxes { nullptr };
        Int32 n, w, h;
        LTransform transform;
        bool oversampling = false;
        bool fractionalScale = false;
    };

    std::unordered_map<std::thread::id, ThreadData> m_sceneThreadsMap;
    LWeak<ThreadData> m_currentThreadData;
    LFramebuffer *m_fb { nullptr };
    LRGBAF m_clearColor {0.f, 0.f, 0.f, 0.f};
    LPoint m_customPos;
    std::vector<LOutput*> m_outputs;
    PaintEventParams m_paintParams;

private:
    friend class LScene;
    friend class LView;
    LSceneView(LFramebuffer *framebuffer = nullptr, LView *parent = nullptr) noexcept :
        LView(LView::SceneType, true, parent),
        m_fb(framebuffer)
    {}

    void calcNewDamage(LView *view) noexcept;
    void drawOpaqueDamage(LView *view) noexcept;
    void drawTranslucentDamage(LView *view) noexcept;

    void parentClipping(LView *parent, LRegion *region) noexcept
    {
        if (!parent)
            return;

        region->clip(parent->pos(), parent->size());

        if (parent->parentClippingEnabled())
            parentClipping(parent->parent(), region);
    }

    void drawBackground(bool addToOpaqueSum) noexcept
    {
        auto &ctd {* m_currentThreadData.get() };
        LRegion backgroundDamage;
        pixman_region32_subtract(&backgroundDamage.m_region,
                                 &ctd.newDamage.m_region,
                                 &ctd.opaqueSum.m_region);
        ctd.p->setColor({.r = m_clearColor.r, .g = m_clearColor.g, .b = m_clearColor.b});
        ctd.p->setAlpha(m_clearColor.a);
        ctd.p->enableAutoBlendFunc(true);
        ctd.p->setColorFactor(1.f, 1.f, 1.f, 1.f);
        ctd.p->bindColorMode();
        ctd.p->drawRegion(backgroundDamage);

        if (addToOpaqueSum)
            ctd.opaqueSum.addRegion(backgroundDamage);
    }

    void clearTmpVariables(ThreadData &ctd) noexcept
    {
        ctd.newDamage.clear();
        ctd.opaqueSum.clear();
    }

    void damageAll(ThreadData &ctd) noexcept
    {
        ctd.newDamage.clear();
        ctd.newDamage.addRect(m_fb->rect());
    }

    void checkRectChange(ThreadData &ctd) noexcept
    {
        bool needsDamage { false };

        if (ctd.prevRect.size() != m_fb->rect().size())
        {
            ctd.prevRect.setSize(m_fb->rect().size());
            needsDamage = true;
        }

        if (ctd.o)
        {
            ctd.newDamage.addRegion(cursor()->damage(ctd.o));
            if ((ctd.o->fractionalOversamplingEnabled() != ctd.oversampling && ctd.o->usingFractionalScale()) || ctd.o->usingFractionalScale() != ctd.fractionalScale)
            {
                ctd.fractionalScale = ctd.o->usingFractionalScale();
                ctd.oversampling = ctd.o->fractionalOversamplingEnabled();
                needsDamage = true;
            }

            if (ctd.o->needsFullRepaint())
                needsDamage = true;

            if (ctd.transform != ctd.o->transform())
            {
                ctd.transform = ctd.o->transform();
                needsDamage = true;
            }
        }

        if (needsDamage)
            damageAll(ctd);
    }
};

#endif // LSCENEVIEW_H
