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
    LSceneView(const LSize &sizeB, Int32 bufferScale, LView *parent = nullptr);

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
    const LRGBAF &clearColor() const;

    /**
     * @brief Set the clear color of the scene view using individual RGBA components.
     *
     * @param r The red color component (0.0 to 1.0).
     * @param g The green color component (0.0 to 1.0).
     * @param b The blue color component (0.0 to 1.0).
     * @param a The alpha value (0.0 to 1.0).
     */
    void setClearColor(Float32 r, Float32 g, Float32 b, Float32 a);

    /**
     * @brief Set the clear color of the scene view using an LRGBAF color.
     *
     * @param color The clear color in LRGBAF format.
     */
    void setClearColor(const LRGBAF &color);

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
     * @brief Check if the view is the main view of an LScene.
     *
     * @return True if it is the maint view of an LScene, false otherwise.
     */
    bool isLScene() const;

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
    void setPos(const LPoint &pos);

    /**
     * @brief Set the position of the scene using X and Y coordinates.
     *
     * @param x The X-coordinate of the new position.
     * @param y The Y-coordinate of the new position.
     */
    void setPos(Int32 x, Int32 y);

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
    void setScale(Int32 scale);

    virtual bool nativeMapped() const override;
    virtual const LPoint &nativePos() const override;
    virtual const LSize &nativeSize() const override;
    virtual Int32 bufferScale() const override;
    virtual void enteredOutput(LOutput *output) override;
    virtual void leftOutput(LOutput *output) override;
    virtual const std::list<LOutput*> &outputs() const override;
    virtual bool isRenderable() const override;
    virtual void requestNextFrame(LOutput *output) override;
    virtual const LRegion *damage() const override;
    virtual const LRegion *translucentRegion() const override;
    virtual const LRegion *opaqueRegion() const override;
    virtual const LRegion *inputRegion() const override;
    virtual void paintRect(LPainter *p,
                           Int32 srcX, Int32 srcY,
                           Int32 srcW, Int32 srcH,
                           Int32 dstX, Int32 dstY,
                           Int32 dstW, Int32 dstH,
                           Float32 scale,
                           Float32 alpha) override;

LPRIVATE_IMP(LSceneView)
    /// @cond OMIT
    friend class LScene;
    LSceneView(LFramebuffer *framebuffer = nullptr, LView *parent = nullptr);
    /// @endcond
};

#endif // LSCENEVIEW_H
