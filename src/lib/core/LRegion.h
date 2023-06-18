#ifndef LREGION_H
#define LREGION_H

#include <LNamespaces.h>
#include <LRect.h>
#include <vector>

using namespace std;

/*!
 * @brief Group of non overlapping rectangles
 *
 * The LRegion class offers an efficient way to create sets of rectangles without overlapping their geometries.\n
 * It has methods for performing additions, subtractions, intersections, etc of rectangles.\n
 * It is used by the library to calculate the damage of surfaces, define their opaque, translucent, and input regions.\n
 * Internally, it uses the algorithm and methods of the [Pixman](http://www.pixman.org/) library.
 */
class Louvre::LRegion
{
public:

    /// Constructor
    LRegion();

    /// Destructor
    ~LRegion();

    /// Copy constructor
    LRegion(const LRegion &);

    /// The assignment operator
    LRegion &operator=( const LRegion &);

    /// Deletes all rects
    void clear();

    /// Adds a rect (union)
    void addRect(const LRect &rect);
    void addRect(Int32 x, Int32 y, Int32 w, Int32 h);

    /// Adds a region (union)
    void addRegion(const LRegion &region);

    /// Subtracts a rect
    void subtractRect(const LRect &rect);

    /// Subtracts a region
    void subtractRegion(const LRegion &region);

    /// Intersects a region
    void intersectRegion(const LRegion &region);

    /// Multiplies the components of each rectangle by the factor
    void multiply(Float32 factor);

    /// @returns true if the region containts the point
    bool containsPoint(const LPoint &point) const;

    /// Traslates each rect by the offset
    void offset(const LPoint &offset);

    /// Relaces the current region by **regionToCopy**
    void copy(const LRegion &regionToCopy);

    /// Inverts the region contained within rect
    void inverse(const LRect &rect);

    /// @returns true if the region has zero rects
    bool empty() const;

    /// Clips the region to the area given by the rect
    void clip(const LRect &rect);

    /// List of rects that make up the region
    const vector<LRect> &rects() const;

    /// List of rects that make up the region
    LBox *rects(Int32 *n) const;
private:
    mutable bool m_changed = false;
    mutable vector<LRect>m_rects;
    mutable void *m_region = nullptr;
};

#endif // LREGION_H
