#ifndef LSCENEVIEW_H
#define LSCENEVIEW_H

#include <LView.h>

/**
 * @brief View for rendering other views
 *
 * An LSceneView is a unique type of view. Instead of rendering its content directly into an LOutput,
 * it possesses its own framebuffer. This capability allows for advanced blending effects, such as applying masks (check LView::setBlendFunc()).
 *
 * Unlike other Views, child views of an LSceneView are always clipped to the boundaries of the LSceneView rectangle.
 * LSceneViews can function as children of other views or LSceneViews (just like any other view), but they can also operate independently.
 * For example, you might create an LSceneView, populate it with arranged views, execute the render() function,
 * and then use that output as a texture for other porpuses.
 *
 * The main view of the LScene class (LScene::mainView()) is a unique LSceneView designed to render its content onto one or more LOutputs instead of using its own framebuffer.\n
 *
 * @warning Use LSceneViews judiciously. When nested within another scene, they are rendered twice: first, the scene renders itself to its framebuffer, and then its parent scene renders that to itself as well or into an LOutput framebuffer.
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
    LSceneView(const LSize &sizeB, Float32 bufferScale, LView *parent = nullptr);

    /// @cond OMIT
    LSceneView(const LSceneView&) = delete;
    LSceneView& operator= (const LSceneView&) = delete;
    /// @endcond

    /**
     * @brief Destructor for the LSceneView.
     */
    ~LSceneView();

    /**
     * @brief Retrieve the clear color of the scene view.
     *
     * @return The clear color in RGBA format.
     */
    inline const LRGBAF &clearColor() const noexcept
    {
        return m_clearColor;
    }

    /**
     * @brief Set the clear color of the scene view using an LRGBAF color.
     *
     * @param color The clear color in LRGBAF format.
     */
    inline void setClearColor(const LRGBAF &color) noexcept
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
    void damageAll(LOutput *output);

    /**
     * @brief Add specific damage areas to the scene view for a specific output.
     *
     * @param output The output for which to add damage areas.
     * @param damage The damaged regions to be added.
     */
    void addDamage(LOutput *output, const LRegion &damage);

    /**
     * @brief Render the scene.
     *
     * This method initiates rendering for the view, excluding specified regions if provided.
     *
     * @note The rendered content can be accessed as a texture using the texture() method.
     *
     * @param exclude Regions to be excluded from rendering.
     */
    virtual void render(const LRegion *exclude = nullptr);

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
    virtual LTexture *texture(Int32 index = 0) const;

    /**
     * @brief Set the position of the scene.
     *
     * @param pos The new position of the scene.
     */
    inline void setPos(const LPoint &pos) noexcept
    {
        setPos(pos.x(), pos.y());
    }

    /**
     * @brief Set the position of the scene using X and Y coordinates.
     *
     * @param x The X-coordinate of the new position.
     * @param y The Y-coordinate of the new position.
     */
    void setPos(Int32 x, Int32 y) noexcept;

    /**
     * @brief Set the size of the scene framebuffer.
     *
     * @warning Changing the framebuffer size will destroy the current framebuffer and create a new one. Therefore, this operation should not be performed frequently.
     *
     * @note Calling this method on the main view of an LScene is a no-op. Instead, use LOutput::setMode() to control the size of the output framebuffer.
     *
     * @param size The new size of the framebuffer.
     */
    void setSizeB(const LSize &size);

    /**
     * @brief Set the scale factor for the scene framebuffer.
     *
     * @param scale The new scale factor to be applied.
     */
    void setScale(Float32 scale);

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

/// @cond OMIT
protected:
    class ThreadData : public LObject
    {
    public:
        std::list<LRegion*>prevDamageList;
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
    LSceneView(LFramebuffer *framebuffer = nullptr, LView *parent = nullptr);

    void calcNewDamage(LView *view);
    void drawOpaqueDamage(LView *view);
    void drawTranslucentDamage(LView *view);
    void parentClipping(LView *parent, LRegion *region);
    void drawBackground(bool addToOpaqueSum);
    void clearTmpVariables(ThreadData &ctd);
    void damageAll(ThreadData &ctd);
    void checkRectChange(ThreadData &ctd);

/// @endcond
};

#endif // LSCENEVIEW_H
