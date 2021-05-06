#ifndef HTCW_GFX_POSITIONING_HPP
#define HTCW_GFX_POSITIONING_HPP
#include "bits.hpp"
namespace gfx {
    PACK
    // represents a pointx with 16-bit integer coordinates
    template <typename T>
    struct PACKED pointx {
        using type = pointx<T>;
        using value_type = T;
        // the x coordinate
        T x;
        // the y coordinate
        T y;
        // constructs a new instance
        inline pointx() {}
        // constructs a new instance with the specified coordinates
        constexpr inline pointx(T x, T y) : x(x), y(y) {
        }
        constexpr inline explicit operator pointx<bits::signedx<value_type>>() const {
            return pointx<bits::signedx<value_type>>(bits::signedx<value_type>(x),bits::signedx<value_type>(y));
        }
        constexpr inline explicit operator pointx<bits::unsignedx<value_type>>() const {
            return pointx<bits::unsignedx<value_type>>(bits::unsignedx<value_type>(x),bits::unsignedx<value_type>(y));
        }
        constexpr inline type offset(bits::signedx<value_type> x, bits::signedx<value_type> y) const {
            return type(this->x+x,this->y+y);
        }
        constexpr inline bool operator==(const type& rhs) const { 
            return x==rhs.x && y==rhs.y;   
        }
        constexpr inline type offset(bits::signedx<value_type> x,bits::signedx<value_type> y) {
            return type(this->x+x,this->y+y);
        }
    };
    // represents a size with 16-bit integer coordinates
    template <typename T>
    struct PACKED sizex {
        using type = sizex<T>;
        using value_type = T;
        // the width
        T width;
        // the height
        T height;
        // constructs a new instance
        inline sizex() {}
        // constructs a new instance with the specified width and height
        constexpr inline sizex(T width, T height) : width(width), height(height) {
        }
        constexpr explicit operator sizex<bits::signedx<value_type>>() const {
            return sizex<bits::signedx<value_type>>(bits::signedx<value_type>(width),bits::signedx<value_type>(height));
        }
        constexpr explicit operator sizex<bits::unsignedx<value_type>>() const {
            return sizex<bits::unsignedx<value_type>>(bits::unsignedx<value_type>(width),bits::unsignedx<value_type>(height));
        }
        constexpr inline bool operator==(const type& rhs) const { 
            return width==rhs.width && height==rhs.height;   
        }
    };
    enum struct rect_orientation {
        normalized = 0,
        denormalized = 1,
        flipped_horizontal = 2 | denormalized,
        flipped_vertical = 4 | denormalized
    };
    // represents a rectangle with 16-bit integer coordinates
    template <typename T>
    struct PACKED rectx
    {
        using type =rectx<T>;
        using value_type = T;
        // the x1 coordinate
        T x1;
        // the y1 coordinate
        T y1;
        // the x2 coordinate
        T x2;
        // the y2 coordinate
        T y2;
        // constructs a new instance
        inline rectx() {}
        // constructs a new instance with the specified coordinates
        constexpr inline rectx(T x1, T y1, T x2, T y2) : x1(x1), y1(y1), x2(x2), y2(y2) {
        }
        // constructs a new instance with the specified location and size
        constexpr inline rectx(pointx<T> location, sizex<T> size) : x1(location.x), y1(location.y), x2(location.x + size.width - 1), y2(location.y + size.height - 1) {
        }
        // constructs a new instance with the specified center and distance from the center to a side. This is useful for constructing circles out of bounding rectangles.
        constexpr inline rectx(pointx<T> center, typename sizex<T>::value_type distance) : 
            x1(center.x - distance ), 
            y1(center.y - distance ), 
            x2(center.x + distance - 1), 
            y2(center.y + distance - 1) {
        }
        
        // indicates the leftmost position
        constexpr inline T left() const {
            return (x1 <= x2) ? x1 : x2;
        }
        // indicates the rightmost position
        constexpr inline T right() const {
            return (x1 > x2) ? x1 : x2;
        }
        // indicates the topmost position
        constexpr inline T top() const {
            return (y1 <= y2) ? y1 : y2;
        }
        // indicates the bottommost position
        constexpr inline T bottom() const {
            return (y1 > y2) ? y1 : y2;
        }
        // indicates the width
        constexpr inline T width() const {
            return x2>=x1?x2-x1+1:x1-x2+1;
            
        }
        constexpr inline pointx<T> point1() const {
            return pointx<T>(x1,y1);
        }
        constexpr inline pointx<T> point2() const {
            return pointx<T>(x2,y2);
        }
        // indicates the height
        constexpr inline T height() const {
            return y2>=y1?y2-y1+1:y1-y2+1;
        }
        
        constexpr inline pointx<T> top_left() const {
            return pointx<T>(left(),top());
        }
        constexpr inline pointx<T> top_right() const {
            return pointx<T>(right(),top());
        }
        constexpr inline pointx<T> bottom_left() const {
            return pointx<T>(left(),bottom());
        }
        constexpr inline pointx<T> bottom_right() const {
            return pointx<T>(right(),bottom());
        }
        // indicates the location
        constexpr inline pointx<T> location() const {
            return top_left();
        }
        // indicates the size
        constexpr inline sizex<T> dimensions() const
        {
            return sizex<T>(width(),height());
        }
        // indicates the delta used to move from x1 to x2 by 1 step
        constexpr inline value_type delta_x() {
            return x2>x1?-1:1;
        }
        // indicates the delta used to move from y1 to y2 by 1 step
        constexpr inline value_type delta_y() {
            return y2>y1?-1:1;
        }
        // indicates whether or not the specified pointx intersects with the rectangle
        constexpr inline bool intersects(pointx<T> pointx) const {
            return pointx.x>=left() && 
                    pointx.x<=right() &&
                    pointx.y>=top() &&
                    pointx.y<=bottom();
        }
        // indicates whether or not the specified rectangle intersects with this rectangle
        constexpr bool intersects(const rectx<T>& rect) const {
            return rect.intersects(top_left()) || 
                rect.intersects(bottom_right()) ||
                intersects(rect.top_left()) || 
                intersects(rect.bottom_right());
        }
        // increases or decreases the x and y bounds by the specified amounts. The rectangle is anchored on the center, and the effective width and height increases or decreases by twice the value of x or y.
        constexpr rectx<T> inflate(typename bits::signedx<T> x,typename bits::signedx<T> y) const {
            switch((int)orientation()) {
                case (int)rect_orientation::flipped_horizontal:
                    return rectx<T>(x1-x,y1-y,x2+x,y2+y);
                case (int)rect_orientation::flipped_vertical:
                    return rectx<T>(x1-x,y1+y,x2+x,y2-y);
                case (int)((int)rect_orientation::flipped_vertical|(int)rect_orientation::flipped_horizontal):
                    return rectx<T>(x1+x,y1+y,x2-x,y2-y);
                
            }
            return rectx<T>(x1-x,y1-y,x2+x,y2+y);
        }
        // offsets the rectangle by the specified amounts.
        constexpr inline rectx<T> offset(typename bits::signedx<T> x,typename bits::signedx<T> y) const {
            return rectx<T>(x1+x,y1+y,x2+x,y2+y);
        }
        // normalizes a rectangle, such that x1<=x2 and y1<=y2
        constexpr inline rectx<T> normalize() const {
            return rectx<T>(location(),dimensions());
        }
        // indicates whether or not the rectangle is flipped along the horizontal or vertical axes.
        constexpr rect_orientation orientation() const {
            int result = (int)rect_orientation::normalized;
            if(x1>x2)
                result |= (int)rect_orientation::flipped_horizontal;
            if(y1>y2)
                result |= (int)rect_orientation::flipped_vertical;
            return (rect_orientation)result;
        }
        // crops a copy of the rectangle by bounds
        constexpr rectx<T> crop(const rectx<T>& bounds) const {
            if(x1<=x2) {
                if(y1<=y2) {
                    return rectx<T>(
                            x1<bounds.left()?bounds.left():x1,
                            y1<bounds.top()?bounds.top():y1,
                            x2>bounds.right()?bounds.right():x2,
                            y2>bounds.bottom()?bounds.bottom():y2
                        );            
                } else {
                    return rectx<T>(
                            x1<bounds.left()?bounds.left():x1,
                            y2<bounds.top()?bounds.top():y2,
                            x2>bounds.right()?bounds.right():x2,
                            y1>bounds.bottom()?bounds.bottom():y1
                        );            
                }
            } else {
                if(y1<=y2) {
                    return rectx<T>(
                            x2<bounds.left()?bounds.left():x2,
                            y1<bounds.top()?bounds.top():y1,
                            x1>bounds.right()?bounds.right():x1,
                            y2>bounds.bottom()?bounds.bottom():y2
                        );            
                } else {
                    return rectx<T>(
                            x2<bounds.left()?bounds.left():x2,
                            y2<bounds.top()?bounds.top():y2,
                            x1>bounds.right()?bounds.right():x1,
                            y1>bounds.bottom()?bounds.bottom():y1
                        );            
                }
            }
        }
        // indicates if this rectangle entirely contains the other rectangle
        constexpr bool contains(const rectx<T>& other) const {
            return intersects(other.point1()) &&
                intersects(other.point2());
        }
        constexpr inline rectx<T> flip_horizontal() const {
            return rectx<T>(x2,y1,x1,y2);
        }
        constexpr inline rectx<T> flip_vertical() const {
            return rectx<T>(x1,y2,x2,y1);
        }
        constexpr inline rectx<T> flip_all() const {
            return rectx<T>(x2,y2,x1,y1);
        }
        // splits a rectangle by another rectangle, returning between 0 and 4 rectangles as a result
        constexpr size_t split(rectx<T>& split_rect,size_t out_count, rectx<T>* out_rects) const {
            if(0==out_count || !intersects(split_rect)) return 0;
            size_t result = 0;
            if(split_rect.top()>top()) {
                *(out_rects++)=rectx<T>(left(),top(),right(),split_rect.top()-1);
                ++result;
            }
            if(result==out_count) return result;
            if(split_rect.left()>left()) {
                *(out_rects++)=rectx<T>(left(),split_rect.top(),split_rect.left()-1,split_rect.bottom());
                ++result;
            }
            if(result==out_count) return result;
            if(split_rect.right()>right()) {
                *(out_rects++)=rectx<T>(split_rect.right(),split_rect.top(),right(),split_rect.bottom());
                ++result;
            }
            if(result==out_count) return result;
            if(split_rect.bottom()<bottom()) {
                *(out_rects++)=rectx<T>(left(),split_rect.bottom()+1,right(),bottom());
                ++result;
            }
            return result;
        }
        explicit operator rectx<bits::signedx<value_type>>() const {
            using t = bits::signedx<value_type>;
            return rectx<bits::signedx<value_type>>(t(x1),t(y1),t(x2),t(y2));
        }
        explicit operator rectx<bits::unsignedx<value_type>>() const {
            using t = bits::unsignedx<value_type>;
            return rectx<bits::unsignedx<value_type>>(t(x1),t(y1),t(x2),t(y2));
        }
        constexpr inline bool operator==(const type& rhs) const { 
            return x1==rhs.x1 && y1==rhs.y1 &&
                x2==rhs.x2 && y2==rhs.y2;
        }
    };
    RESTORE_PACK
    
    using spoint16 = pointx<int16_t>;
    using ssize16 = sizex<int16_t>;
    using srect16 = rectx<int16_t>;

    using point16 = pointx<uint16_t>;
    using size16 = sizex<uint16_t>;
    using rect16 = rectx<uint16_t>;
}
#endif
