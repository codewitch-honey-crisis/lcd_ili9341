#ifndef HTCW_GFX_BITMAP
#define HTCW_GFX_BITMAP
#include "gfx_core.hpp"
#include "gfx_pixel.hpp"
#include "gfx_positioning.hpp"
namespace gfx {
    // represents an in-memory bitmap
    template<typename PixelType>
    class bitmap final {
        const size16 m_dimensions;
        uint8_t* const m_begin;
    public:
        // the type of the bitmap, itself
        using type = bitmap<PixelType>;
        // the type of the pixel used for the bitmap
        using pixel_type = PixelType;
        using caps = gfx::gfx_caps<true,false,false,false,false,false,false,false,false,false,false>;
        
        // constructs a new bitmap with the specified size and buffer
        bitmap(size16 dimensions,void* buffer) : m_dimensions(dimensions),m_begin((uint8_t*)buffer) {}
        // constructs a new bitmap with the specified width, height and buffer
        bitmap(uint16_t width,uint16_t height,void* buffer) : m_dimensions(width,height),m_begin((uint8_t*)buffer) {}
        gfx_result point(point16 location,pixel_type* out_pixel) const {
            if(nullptr==out_pixel)
                return gfx_result::invalid_argument;
            if(location.x>=dimensions().width||location.y>=dimensions().height) {
                *out_pixel = pixel_type();
                return gfx_result::success;
            }
            const size_t bit_depth = pixel_type::bit_depth;
            const size_t offs = (location.y*dimensions().width+location.x)*bit_depth;
            const size_t offs_bits = offs % 8;
            uint8_t tmp[pixel_type::packed_size+(((int)pixel_type::pad_right_bits)<=offs_bits)];
            tmp[pixel_type::packed_size]=0;
            memcpy(tmp,begin()+offs/8,sizeof(tmp));
            if(0<offs_bits)
                bits::shift_left(tmp,0,sizeof(tmp)*8,offs_bits);
            pixel_type result;
            typename pixel_type::int_type r = 0;
            memcpy(&r,tmp,pixel_type::packed_size);
            r=helpers::order_guard(r);
            r&=pixel_type::mask;
            result.native_value=r;
            *out_pixel=result;
            return gfx_result::success;
        }
        gfx_result point(point16 location,pixel_type rhs) {
            // clip
            if(location.x>=dimensions().width||location.y>=dimensions().height)
                return gfx_result::success;
            const size_t bit_depth = pixel_type::bit_depth;
            const size_t offs = (location.y*dimensions().width+location.x)*bit_depth;
            const size_t offs_bits = offs % 8;
            // now set the pixel
            uint8_t tmp[pixel_type::packed_size+(((int)pixel_type::pad_right_bits)<=offs_bits)];
            *((typename pixel_type::int_type*)tmp)=rhs.value();
            bits::shift_right(tmp,0,pixel_type::bit_depth+offs_bits,offs_bits);
            bits::set_bits(offs_bits,pixel_type::bit_depth,begin()+offs/8,tmp);
            return gfx_result::success;
        }
        pixel_type point(point16 location) const {
            pixel_type result;
            point(location,&result);
            return result;
        }
        // indicates the dimensions of the bitmap
        inline size16 dimensions() const {
            return m_dimensions;
        }
        // provides a bounding rectangle for the bitmap, starting at 0,0
        inline rect16 bounds() const {
            return rect16(point16(0,0),dimensions());
        }
        // indicates the size of the bitmap, in pixels
        inline size_t size_pixels() const {
            return m_dimensions.height*m_dimensions.width;
        }
        // indicates the size of the bitmap in bytes
        inline size_t size_bytes() const {
            return (size_pixels()*(PixelType::bit_depth/8.0)+.99999);
        }
        // indicates the beginning of the bitmap buffer
        inline uint8_t* begin() const {
            return m_begin;
        }
        // indicates just past the end of the bitmap buffer
        inline uint8_t* end() const {
            return begin()+size_bytes();
        }
        template<typename Destination>
        gfx_result convert(Destination& out) const {
            size16 dim = bounds().crop(out.bounds()).dimensions();
            point16 pt;
            for(pt.y = 0;pt.y<dim.width;++pt.y) {
                for(pt.x=0;pt.x<dim.height;++pt.x) {
                    pixel_type px;
                    point(pt,&px); // don't bother err checking
                    typename Destination::pixel_type dpx;
                    if(!px.convert(&dpx))
                        return gfx_result::not_supported;
                    gfx_result r = out.point(pt,dpx);
                    if(r!=gfx_result::success) {
                        return r;
                    }
                }
            }
            return gfx_result::success;
        }
        // clears a region of the bitmap
        gfx_result clear(const rect16& dst) {
            if(!dst.intersects(bounds())) return gfx_result::success;
            rect16 dstr = dst.crop(bounds());
            size_t dy = 0, dye=dstr.height();
            if(pixel_type::byte_aligned) {
                const size_t line_len = dstr.width()*pixel_type::packed_size;
                while(dy<dye) {
                   uint8_t* pdst = begin()+(((dstr.top()+dy)*dimensions().width+dstr.left())*pixel_type::packed_size); 
                   memset(pdst,0,line_len);
                   ++dy;
                }
            } else {
                // unaligned version
                const size_t line_len_bits = dstr.width()*pixel_type::bit_depth;
                while(dy<dye) {
                    const size_t offs = (((dstr.top()+dy)*dimensions().width+dstr.left())*pixel_type::bit_depth);
                    const size_t offs_bytes = offs / 8;
                    const size_t offs_bits = offs % 8;

                    uint8_t* pdst = begin()+offs_bytes;
                    bits::set_bits(pdst,offs_bits,line_len_bits,false);
                    ++dy;
                }
            }
            return gfx_result::success;
        }
        // fills a region of the bitmap with the specified pixel
        gfx_result fill(const rect16& dst,const pixel_type& pixel) {
            if(!dst.intersects(bounds())) return gfx_result::success;
            typename pixel_type::int_type be_val = pixel.value();
            rect16 dstr = dst.crop(bounds());
            size_t dy = 0, dye=dstr.height();
            if(pixel_type::byte_aligned) {
                const size_t line_len = dstr.width()*pixel_type::packed_size;
                while(dy<dye) {
                   uint8_t* pdst = begin()+(((dstr.top()+dy)*dimensions().width+dstr.left())*pixel_type::packed_size); 
                   for(size_t x = 0;x<line_len;x+=pixel_type::packed_size) {
                       
                       memcpy(pdst,&be_val,pixel_type::packed_size);
                       pdst+=pixel_type::packed_size;
                   }
                   ++dy;
                }
            } else if(pixel_type::bit_depth!=1) {
                // unaligned version
                uint8_t buf[pixel_type::packed_size+1];
                // set this to 9 to ensure no match on first iteration:
                size_t old_offs_bits=9;
                while(dy<dye) {
                    size_t dx = dstr.left();
                    while(dx<=dstr.right()) {
                        const size_t offs = (((dstr.top()+dy)*dimensions().width+dx)*pixel_type::bit_depth); 
                        const size_t offs_bits = offs % 8;
                        if(offs_bits!=old_offs_bits) {
                            memcpy(buf,&be_val,pixel_type::packed_size);
                            buf[pixel_type::packed_size]=0;
                            if(offs_bits!=0)
                                bits::shift_right(buf,0,sizeof(buf)*8,offs_bits);
                        }
                        const size_t offs_bytes = offs/8;
                        bits::set_bits(offs_bits,pixel_type::bit_depth,begin()+offs_bytes,buf);
                        ++dx;
                        old_offs_bits=offs_bits;
                    }
                    ++dy;
                }
            } else { // monochrome specialization
                const size_t line_len_bits = dstr.width()*pixel_type::bit_depth;
                bool set = 0!=pixel.native_value;
                while(dy<dye) {
                    const size_t offs = (dstr.top()+dy)*dimensions().width+dstr.left()*pixel_type::bit_depth;
                    uint8_t* pdst = begin()+(offs/8);
                    bits::set_bits(pdst,offs%8,line_len_bits,set);
                    ++dy;
                }
            } 
            return gfx_result::success;
        }
        // blts a portion of this bitmap to the destination at the specified location.
        // out of bounds regions are cropped
        gfx_result blt_to(const rect16& src,type& dst,point16 location) {
            if(!src.intersects(bounds())) return gfx_result::success;
            rect16 srcr = src.crop(bounds());
            rect16 dstr= rect16(location,srcr.dimensions()).crop(dst.bounds());
            srcr=rect16(srcr.location(),dstr.dimensions());
            size_t dy=0,dye=dstr.height();
            size_t dxe = dstr.width();
            if(pixel_type::byte_aligned) {
               const size_t line_len = dstr.width()*pixel_type::packed_size;
               while(dy<dye) {
                   uint8_t* psrc = begin()+(((srcr.top()+dy)*dimensions().width+srcr.left())*pixel_type::packed_size);
                   uint8_t* pdst = dst.begin()+(((dstr.top()+dy)*dst.dimensions().width+dstr.left())*pixel_type::packed_size);
                   memcpy(pdst,psrc,line_len);
                   ++dy;
               }
            } else {
                // unaligned version
                const size_t line_len_bits = dstr.width()*pixel_type::bit_depth;
                const size_t line_block_pels = line_len_bits>(128*8)?(128*8)/pixel_type::bit_depth:dstr.width();
                const size_t line_block_bits = line_block_pels*pixel_type::bit_depth;
                const size_t buf_size = line_block_bits/8.0+1.99999;
                uint8_t buf[buf_size];
                buf[buf_size-1]=0;
                while(dy<dye) {
                    size_t dx=0;
                    while(dx<dxe) {
                        const size_t src_offs = ((srcr.top()+dy)*dimensions().width+srcr.left()+dx)*pixel_type::bit_depth;
                        const auto dpx = (dstr.top()+dy)*dst.dimensions().width+dstr.left()+dx;
                        const size_t dst_offs =dpx*pixel_type::bit_depth;
                        const size_t src_offs_bits = src_offs % 8;
                        const size_t dst_offs_bits = dst_offs % 8;
                        uint8_t* psrc = begin()+(src_offs/8);
                        uint8_t* pdst = dst.begin()+(dst_offs/8);
                        const size_t block_pels = (line_block_pels+dx>dstr.width())?dstr.width()-dx:line_block_pels;
                        if(0==block_pels) 
                            break;
                        const size_t block_bytes = (block_pels*pixel_type::bit_depth)/8.0+.99999;
                        const size_t block_bits = block_pels*pixel_type::bit_depth;
                        const int shift = dst_offs_bits-src_offs_bits;
                        // TODO: Make this more efficient:
                        memcpy(buf,psrc,block_bytes+(dpx+block_pels<=dst.size_pixels()));//(line_block_pels+dpx<(dst.size_pixels()-1)));
                        if(0<shift) {
                            bits::shift_right(buf,0,(sizeof(buf))*8,shift);
                        } else if(0>shift) {
                            bits::shift_left(buf,0,(sizeof(buf))*8,-shift);
                        }
                        bits::set_bits(dst_offs_bits,block_bits, pdst,buf);
                        dx+=block_pels;    
                    }
                    ++dy;
                }
        
            }
            
            // TODO:
            // clear any bits we couldn't copy 
            // on account of being out of bounds?
            return gfx_result::success;    
        }
        // computes the minimum required size for a bitmap buffer, in bytes
        inline static size_t sizeof_buffer(size16 size) {
            return (size.width*size.height*pixel_type::bit_depth+7)/8;
        }
        // computes the minimum required size for a bitmap buffer, in bytes
        inline static size_t sizeof_buffer(uint16_t width,uint16_t height) {
            return sizeof_buffer(size16(width,height));
        }
        // this check is already guaranteed by asserts in the pixel itself
        // this is more so it won't compile unless PixelType is actually a pixel
        static_assert(PixelType::channels>0,"The type is not a pixel or the pixel is invalid");
    };
}

#endif