#ifndef LNINEPATCHVIEW_H
#define LNINEPATCHVIEW_H

#include <LTextureView.h>
#include <LEdge.h>
#include <LBitset.h>

/**
 * @brief Nine-Patch Texture View
 *
 * The LNinePatchView class is useful for implementing buttons, containers, borders, etc. in an efficient way. It consists of 9 sub LTextureView s arranged as shown below:
 *
 * <center><img height="512px" src="https://lh3.googleusercontent.com/pw/AP1GczOl2R_1xInk2iUzNxSpwmVISEqAwcNP9chbXLMf4_0EwyZyzxeLSR0Blmq-_QWnnAFT8NdRkGOmLAQuZy1tK6-aIJkPPh_4SaQX63yw4-4GUgDF_HE=w2400"></center>
 *
 * The portion of the texture, position and size of each subview is automatically calculated from the provided center() (E) and the dimensions set with setSize().
 * 
 * For more information about the nine-patch technique, check out [this link](https://en.wikipedia.org/wiki/9-slice_scaling).
 *
 * Subviews can be accessed directly via subViews() or getSubView(), for example, to toggle the visibility of a specific one, change its color, etc.
 *
 * @warning If used as borders (with an invisible center), remember to hide the central view, otherwise, the scene will unnecessarily paint invisible pixels.
 */
class Louvre::LNinePatchView : public LView
{
public:

    /**
     * @brief Creates an empty nine-patch view.
     */
    LNinePatchView(LView *parent = nullptr) noexcept : LView(NinePatchType, false, parent)
    {
        init(nullptr, 1, {0});
    }

    /**
     * @brief Creates a nine-patch view with a specified texture.
     *
     * @param texture The texture, or `nullptr` (unmapped).
     * @param bufferScale The scaling factor of the texture.
     * @param center The subrect of the texture corresponding to the central view in surface coordinates.
     *               If extended beyond the texture bounds, it will be clipped.
     * @param parent The parent view, or `nullptr` if there is no parent. Defaults to `nullptr`.
     */
    LNinePatchView(LTexture *texture, Float32 bufferScale, const LRectF &center, LView *parent = nullptr) noexcept :
        LView(NinePatchType, false, parent)
    {
        init(texture, bufferScale, center);
    }

    LCLASS_NO_COPY(LNinePatchView)

    /**
     * @brief Destructor for LNinePatchView.
     */
    ~LNinePatchView() noexcept { notifyDestruction(); };

    /**
     * @brief Sets the position of the view.
     *
     * @param x The x-coordinate in surface coordinates.
     * @param y The y-coordinate in surface coordinates.
     */
    void setPos(Int32 x, Int32 y) noexcept;

    /**
     * @brief Sets the position of the view.
     *
     * @param pos The position in surface coordinates.
     */
    void setPos(const LPoint &pos) noexcept
    {
        setPos(pos.x(), pos.y());
    }

    /**
     * @brief Sets the size of the view.
     *
     * @note The size is constrained by minSize()
     *
     * @param width The width in surface coordinates.
     * @param height The height in surface coordinates.
     */
    void setSize(Int32 width, Int32 height) noexcept;

    /**
     * @brief Sets the size of the view.
     *
     * @note The size is constrained by minSize()
     *
     * @param size The size in surface coordinates.
     */
    void setSize(const LSize &size) noexcept
    {
        setSize(size.w(), size.h());
    }

    /**
     * @brief Returns the minimum size of the view.
     *
     * The minimum size is equal to the texture's size in surface coordinates (the buffer size divided by the buffer scale) minus the center rect size.
     *
     * If no texture is set the minimum size is (0, 0).
     */
    const LSize &minSize() const noexcept
    {
        return m_minSize;
    }

    /**
     * @brief Returns the current nine-patch texture.
     *
     * @see setTexture()
     *
     * @return A pointer to the texture, or `nullptr` if no texture is set.
     */
    LTexture *texture() const noexcept
    {
        return m_texture;
    }

    /**
     * @brief Sets the nine-patch texture.
     *
     * @param texture The texture, or `nullptr` to unset.
     * @param bufferScale The scaling factor of the texture.
     * @param center The subrect of the texture corresponding to the central view in surface coordinates.
     *               If extended beyond the texture bounds, it will be clipped.
     */
    void setTexture(LTexture *texture, Float32 bufferScale, const LRectF &center) noexcept;

    /**
     * @brief Returns the texture subrect for the central view in surface coordinates.
     *
     * @see setTexture()
     *
     * If no texture is set (0, 0, 0, 0) is returned.
     */
    const LRectF &center() const noexcept
    {
        return m_center;
    }

    /**
     * @brief Retrieves one of the nine sub texture views.
     * 
     * Example:
     * 
     * @code
     * ninePatchView->getSubView(LEdgeTop | LEdgeLeft) // Returns the top-left sub-view
     * @endcode
     * 
     * @param edge The edge or corner to retrieve. Passing @ref LEdgeNone (or a value that is not a corner or edge) returns the central view.
     * @return A pointer to the requested LTextureView.
     */
    LTextureView *getSubView(LBitset<LEdge> edge) noexcept;

    /**
     * @brief Array of the 9 sub-views.
     *
     * The order of the views is L, T, R, B, TL, TR, BR, BL, and Center.
     *
     * If a specific view needs to be retrieved, consider using getSubView() instead.
     * @return A reference to the vector of sub-views.
     */
    std::array<LTextureView, 9> &subViews() noexcept
    {
        return m_subViews;
    }

    bool nativeMapped() const noexcept override;
    const LPoint &nativePos() const noexcept override;
    const LSize &nativeSize() const noexcept override;
    Float32 bufferScale() const noexcept override;
    void enteredOutput(LOutput *output) noexcept override;
    void leftOutput(LOutput *output) noexcept override;
    const std::vector<LOutput*> &outputs() const noexcept override;
    void requestNextFrame(LOutput *output) noexcept override;
    const LRegion *damage() const noexcept override;
    const LRegion *translucentRegion() const noexcept override;
    const LRegion *opaqueRegion() const noexcept override;
    const LRegion *inputRegion() const noexcept override;
    void paintEvent(const PaintEventParams &params) noexcept override;
protected:
    void init(LTexture *texture, Float32 bufferScale, const LRectF &center) noexcept;
    void updateSubViews() noexcept;
    LPoint m_nativePos;
    LSize m_nativeSize;
    LSize m_minSize;
    std::vector<LOutput*> m_outputs;
    std::unique_ptr<LRegion> m_inputRegion;
    LRectF m_center;
    LWeak<LTexture> m_texture;
    std::array<LTextureView, 9> m_subViews;
};

#endif // LNINEPATCHVIEW_H
