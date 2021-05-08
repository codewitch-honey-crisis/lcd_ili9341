// NOT COMPLETE _ I'm working on this


// NOTICE OF USE:

/* Portions of this code derived from Adafruit's SSD1306 library. 
https://github.com/adafruit/Adafruit_SSD1306

The license that ships with their library is as follows:

Software License Agreement (BSD License)

Copyright (c) 2012, Adafruit Industries
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holders nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "i2c_master.hpp"
#include "gfx_core.hpp"
#include "gfx_positioning.hpp"
#include "gfx_pixel.hpp"
// TODO: Remove these or change them to consts
#define SSD1306_MEMORYMODE 0x20          ///< See datasheet
#define SSD1306_COLUMNADDR 0x21          ///< See datasheet
#define SSD1306_PAGEADDR 0x22            ///< See datasheet
#define SSD1306_SETCONTRAST 0x81         ///< See datasheet
#define SSD1306_CHARGEPUMP 0x8D          ///< See datasheet
#define SSD1306_SEGREMAP 0xA0            ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON 0xA5        ///< Not currently used
#define SSD1306_NORMALDISPLAY 0xA6       ///< See datasheet
#define SSD1306_INVERTDISPLAY 0xA7       ///< See datasheet
#define SSD1306_SETMULTIPLEX 0xA8        ///< See datasheet
#define SSD1306_DISPLAYOFF 0xAE          ///< See datasheet
#define SSD1306_DISPLAYON 0xAF           ///< See datasheet
#define SSD1306_COMSCANINC 0xC0          ///< Not currently used
#define SSD1306_COMSCANDEC 0xC8          ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET 0xD3    ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5  ///< See datasheet
#define SSD1306_SETPRECHARGE 0xD9        ///< See datasheet
#define SSD1306_SETCOMPINS 0xDA          ///< See datasheet
#define SSD1306_SETVCOMDETECT 0xDB       ///< See datasheet

#define SSD1306_SETLOWCOLUMN 0x00  ///< Not currently used
#define SSD1306_SETHIGHCOLUMN 0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE 0x40  ///< See datasheet

#define SSD1306_EXTERNALVCC 0x01  ///< External display voltage source
#define SSD1306_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A  ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL 0x2E                    ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL 0x2F                      ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3             ///< Set scroll range

namespace espidf {
    template<uint16_t Width,
            uint16_t Height,
            i2c_port_t I2CPort=I2C_NUM_0,
            uint8_t Address=0x3C,
            bool Vdc3_3=true,
            gpio_num_t PinRst=GPIO_NUM_NC,
            bool ResetBeforeInit=false,
            TickType_t Timeout=5000/portTICK_PERIOD_MS>
    struct ssd1306_i2c final {
        enum struct result {
            success = 0,
            invalid_argument = 1,
            io_error = 2,
            out_of_memory = 3,
            timeout = 4,
            not_supported=5,
            i2c_not_initalized=6
        };
        constexpr static const uint16_t width=Width;
        constexpr static const uint16_t height=Height;
        constexpr static const i2c_port_t i2c_port = I2CPort;
        constexpr static const uint8_t address = Address;
        constexpr static const bool vdc_3_3 = Vdc3_3;
        constexpr static const gpio_num_t pin_rst = PinRst;
        constexpr static const bool reset_before_init = ResetBeforeInit;
        constexpr static const TickType_t timeout = Timeout;
private:
        bool m_initialized;
        struct {
            uint16_t x1;
            uint16_t y1;
            uint16_t x2;
            uint16_t y2;
            size_t left;
        } m_batch;
        uint8_t m_contrast;
        uint8_t m_frame_buffer[width*height/8];
        result command_impl(uint8_t cmd) {
            i2c_master_command mc;
            i2c_result ir = mc.start();
            if(i2c_result::success!=ir) {
                return result::io_error;
            }
            ir= mc.begin_write(address);
            if(ir!=i2c_result::success) {
                return result::io_error;
            }
            ir = mc.write(0);
            if(ir!=i2c_result::success) {
                return result::io_error;
            }
            ir = mc.write(cmd);
//printf("cmd: 0x%02X\r\n",(int)cmd);
            if(ir!=i2c_result::success) {
                return result::io_error;
            }
            ir=mc.stop();
            if(ir!=i2c_result::success) {
                return result::io_error;
            }
            ir=i2c_master::execute(i2c_port,mc,timeout);
            if(ir!=i2c_result::success) {
                return (i2c_result::bus_timeout==ir)?result::timeout:result::io_error;
            }   
            return result::success;
        }
        result start_trans(i2c_master_command& mc,uint8_t payload) {
            i2c_result ir = mc.start();
            if(i2c_result::success!=ir) {
                return result::io_error;
            }
            ir= mc.begin_write(address);
            if(ir!=i2c_result::success) {
                return result::io_error;
            }
            ir = mc.write(payload);
            if(ir!=i2c_result::success) {
                return result::io_error;
            }
            return result::success;
        }
        result commands_impl(const uint8_t *cmds, size_t count) {
            i2c_master_command mc;
            i2c_result ir;
            result r=start_trans(mc,0);
            if(result::success!=r) {
                return r;
            }
            int buf_count = 1;
            while (count--) {
                if (buf_count >= i2c_master::buffer_length) {
                    mc.stop();
                    ir=i2c_master::execute(i2c_port,mc,timeout);
                    if(ir!=i2c_result::success) {
                        return (i2c_result::bus_timeout==ir)?result::timeout:result::io_error;
                    }       
                    
                    mc = i2c_master_command();
                    r=start_trans(mc,0);
                    if(result::success!=r) {
                        return r;
                    }
                    buf_count = 1;
                }

                ir = mc.write(*cmds,true);
                if(i2c_result::success!=ir) {
                    return result::io_error;
                }
                ++cmds;
                ++buf_count;
            }
            mc.stop();

            ir=i2c_master::execute(i2c_port,mc);

            if(ir!=i2c_result::success) {
                return (i2c_result::bus_timeout==ir)?result::timeout:result::io_error;
            }
            return result::success;;
        }
        result reset_impl() {
            if(GPIO_NUM_NC!=pin_rst) {
                if(ESP_OK!=gpio_set_level(pin_rst,1)) {
                    return result::io_error;
                }
                vTaskDelay(1/portTICK_PERIOD_MS);
                if(ESP_OK!=gpio_set_level(pin_rst, 0)) {
                    return result::io_error;
                }
                vTaskDelay(10/portTICK_PERIOD_MS);
                if(ESP_OK!=gpio_set_level(pin_rst,1)) {
                    return result::io_error;
                }
                return result::success;
            }
            return result::not_supported;
        }
        result initialize_impl() {
            result r;
            if(reset_before_init) {
                r=reset();
                if(result::success!=r && result::not_supported!=r) {
                    return r;
                }
            }
            // Init sequence
            static const uint8_t init1[] = {SSD1306_DISPLAYOFF,         // 0xAE
                                                    SSD1306_SETDISPLAYCLOCKDIV, // 0xD5
                                                    0x80, // the suggested ratio 0x80
                                                    SSD1306_SETMULTIPLEX}; // 0xA8
            r=commands_impl(init1, sizeof(init1));

            if(result::success!=r) {
                return r;
            }
            r=command_impl(height - 1);
            if(result::success!=r) {
                return r;
            }

            static const uint8_t init2[] = {SSD1306_SETDISPLAYOFFSET, // 0xD3
                                                    0x0,                      // no offset
                                                    SSD1306_SETSTARTLINE | 0x0, // line #0
                                                    SSD1306_CHARGEPUMP};        // 0x8D
            r=commands_impl(init2, sizeof(init2));
            if(result::success!=r) {
                return r;
            }
            r=command_impl(vdc_3_3 ? 0x10 : 0x14);
            if(result::success!=r) {
                return r;
            }
            static const uint8_t init3[] = {SSD1306_MEMORYMODE, // 0x20
                                                    0x00, // 0x0 act like ks0108, but we want mode 1?
                                                    SSD1306_SEGREMAP | 0x1,
                                                    SSD1306_COMSCANDEC};
            r=commands_impl(init3, sizeof(init3));
            if(result::success!=r) {
                return r;
            }
            uint8_t com_pins = 0x02;
            m_contrast = 0x8F;
            if ((width == 128) && (height == 32)) {
                com_pins = 0x02;
                m_contrast = 0x8F;
            } else if ((width == 128) && (height == 64)) {
                com_pins = 0x12;
                m_contrast = vdc_3_3 ? 0x9F:0xCF;
            } else if ((width == 96) && (height == 16)) {
                com_pins = 0x2; // ada x12
                m_contrast = vdc_3_3 ? 0x10:0xAF;
            } else {
                return result::not_supported;
            }

            r=command_impl(SSD1306_SETCOMPINS);
            if(result::success!=r) {
                return r;
            }
            r=command_impl(com_pins);
            if(result::success!=r) {
                return r;
            }
            r=command_impl(SSD1306_SETCONTRAST);
            if(result::success!=r) {
                return r;
            }
            r=command_impl(m_contrast);
            if(result::success!=r) {
                return r;
            }
            r=command_impl(SSD1306_SETPRECHARGE); // 0xd9
            if(result::success!=r) {
                return r;
            }
            r=command_impl(vdc_3_3 ? 0x22:0xF1);
            if(result::success!=r) {
                return r;
            }
            static const uint8_t init5[] = {
                SSD1306_SETVCOMDETECT, // 0xDB
                0x40,
                SSD1306_DISPLAYALLON_RESUME, // 0xA4
                SSD1306_NORMALDISPLAY,       // 0xA6
                SSD1306_DEACTIVATE_SCROLL,
                SSD1306_DISPLAYON}; // Main screen turn on
            return commands_impl(init5, sizeof(init5));
        }
        static bool normalize_values(uint16_t& x1,uint16_t& y1,uint16_t& x2,uint16_t& y2,bool check_bounds=true) {
            // normalize values
            uint16_t tmp;
            if(x1>x2) {
                tmp=x1;
                x1=x2;
                x2=tmp;
            }
            if(y1>y2) {
                tmp=y1;
                y1=y2;
                y2=tmp;
            }
            if(check_bounds) {
                if(x1>=width||y1>=height)
                    return false;
                if(x2>=width)
                    x2=width;
                if(y2>width)
                    y2=width;
            }
            return true;
        }
        static result xlt_err(i2c_result ir) {
            switch(ir) {
                case i2c_result::bus_timeout:
                    return result::timeout;
                case i2c_result::invalid_argument:
                    return result::invalid_argument;
                case i2c_result::invalid_handle:
                case i2c_result::driver_installation_error:
                case i2c_result::invalid_state:
                case i2c_result::io_error:
                    return result::io_error;
                case i2c_result::success:
                    return result::success;
                default:
                    return result::not_supported;
            }
        }
        result write_bytes(const uint8_t* data,size_t size) {
            if(0==size) return result::success;
            i2c_master_command mc;
            int out = i2c_master::buffer_length;
            bool first = true;
            while(0<size) {
                if(out>=i2c_master::buffer_length) {
                    if(!first) {
                        ir=mc.stop();
                        if(i2c_result::success!=ir)
                            return xlt_err(ir);
                    }
                    first =false;
                    r = start_trans(mc,0x40);
                    if(result::success!=r)
                        return r;
                    out = 0;
                }
                int s = size>i2c_master::buffer_length?i2c_master::buffer_length:size;
                i2c_result ir=mc.write(data,s);
                if(i2c_result::success!=ir) {
                    return xlt_err(ir);
                }
                size-=s;
                out+=s;
                
            }
            return result::success;
        }
        result display_update(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2) {
            if(!normalize_values(x1,y1,x2,y2))
                return;
            result r = initialize();
            if(result::success!=r)
                return r;
            // fix these to divisibles of 8
            y1=(y1/8)*8;
            y2+=(8-(y2%8));
            uint8_t dlist1[] = {
                SSD1306_PAGEADDR,
                y1/8,                   // Page start address
                y2/8,                   // Page end (not really, but works here)
                SSD1306_COLUMNADDR, x1}; // Column start address
            result sr=commands_impl(dlist1, sizeof(dlist1));
            if(result::success!=sr) {
                return sr;
            }
            sr=command_impl(x2); // Column end address
            if(result::success!=sr) {
                return sr;
            }
            i2c_master_command mc;
            result sr = start_trans(mc,0x40);
            int w = x2-x1+1;
            int h = y2-y1+1;
            int b1=y1/8,b2=y2/8;
            int b = b1;
            while(b<=b2) {
                uint8_t* p = m_frame_buffer+(b*width)+x1;
                r=write_bytes(p,w);
                if(result::success!=r)
                    return r;
            }
            return result::success;
        }
        void buffer_fill(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2, bool color) {
            if(!normalize_values(x1,y1,x2,y2))
                return;
            
            int y=y1;
            int m=y%8;
            if(0!=m) {
                const uint8_t set_mask = uint8_t(0xFF<<(8-m));
                const uint8_t value_mask = uint8_t(~set_mask)*color;
                uint8_t* p = m_frame_buffer+y/8+x1;
                for(int x=x1;x<=x2;++x) {
                    *p&=set_mask;
                    *p|=value_mask;
                }
                y=y1+(8-m);
            }
                
            while(y+8<=y2) {
                // we can do a faster fill for this part
                const int w = x2-x1+1;    
                uint8_t* p = m_frame_buffer+y/8+x1;
                memset(p,0xFF*color,w);
                y+=8;
            }
            m=y%8;
            if(0!=m) {
                const uint8_t set_mask = uint8_t(0xFF<<(8-m));
                const uint8_t value_mask = uint8_t(~set_mask)*color;
                uint8_t* p = m_frame_buffer+y/8+x1;
                for(int x=x1;x<=x2;++x) {
                    *p&=set_mask;
                    *p|=value_mask;
                }
            }
        }
public:
        ssd1306_i2c() : m_initialized(false)  {
            m_batch = {0};
        }
        bool initialized() const {
            return m_initialized;
        }
        result initialize() {
            if(!m_initialized) {
                return initialize_impl();
            }
            return result::success;
        }
        // resets the display. The reset pin must be available and connected
        result reset() {
            if(!initialized()) {
                return initialize_impl();
            } 

            return reset_impl();
        }
        inline result frame_fill(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,bool color) {
            buffer_fill(x1,y1,x2,y2,color);
            return display_update(x1,y1,x2,y2);
        }
        result batch_begin(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2) {
            normalize_values(x1,y1,x2,y2,false);
            uint16_t w=x2-x1+1;
            uint16_t h=y2-y1+1;
            size_t w*h/8;
            m_batch=
        }
        // GFX Bindings
        using type = ssd1306_i2c<Width,Height,I2CPort,Address,Vdc3_3,PinRst,ResetBeforeInit,Timeout>;
        using pixel_type = gfx::gsc_pixel<1>;
        using caps = gfx::gfx_caps<false,false,true,true,false,true,false,true,false,true,false>;
        private:
        gfx::gfx_result xlt_err(result r) {
            switch(r) {
                case result::io_error:
                case result::i2c_not_initalized:
                    return gfx::gfx_result::device_error;
                case result::out_of_memory:
                    return gfx::gfx_result::out_of_memory;
                case result::success:
                    return gfx::gfx_result::success;
                case result::not_supported:
                    return gfx::gfx_result::not_supported;
                case result::invalid_argument:
                    return gfx::gfx_result::invalid_argument;
                default:
                    return gfx::gfx_result::unknown_error;
            }
        }

 public:
        
        // sets a point to the specified pixel
        gfx::gfx_result point(gfx::point16 location,pixel_type color) {
            result r = frame_fill(location.x,location.y,location.x,location.y,color.native_value!=0);
            if(result::success!=r)
                return xlt_err(r);
            return result::success;
        }
        inline gfx::size16 dimensions() {return gfx::size16(width,height);}
        inline gfx::rect16 bounds() { return dimensions().bounds(); }
        gfx::gfx_result fill(const gfx::rect16& rect,pixel_type color) {
            result r = frame_fill(rect.x1,rect.y1,rect.x2,rect.y2,color.native_value!=0);
            if(result::success!=r)
                return xlt_err(r);
            return result::success;
        }
        
        // clears the specified rectangle
        inline gfx::gfx_result clear(const gfx::rect16& rect) {
            pixel_type p;
            return fill(rect,p);
        }
        // begins a batch operation for the specified rectangle
        gfx::gfx_result begin_batch(const gfx::rect16& rect) {
            return gfx::gfx_result::not_supported;
        }
        // writes a pixel to a pending batch
        gfx::gfx_result write_batch(pixel_type color) {
            return gfx::gfx_result::not_supported;
        }
        // commits a pending batch
        gfx::gfx_result commit_batch() {
            return gfx::gfx_result::not_supported;
        }
        // copies source data to a frame
        template<typename Source>
        inline gfx::gfx_result copy_from(const gfx::rect16& src_rect,const Source& src,gfx::point16 location) {
            return gfx::gfx_result::not_supported;
        }
        
    };
}