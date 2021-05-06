#pragma once
#define HTCW_ILI9341_OVERCLOCK
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "spi_master.hpp"

#include "gfx_core.hpp"
#include "gfx_positioning.hpp"
#include "gfx_pixel.hpp"

namespace espidf {
    // the driver for an ILI9341 display
    template<spi_host_device_t HostId,
            gpio_num_t PinCS,
            gpio_num_t PinDC,
            gpio_num_t PinRst,
            gpio_num_t PinBacklight,
            size_t BufferSize=64,
            size_t MaxTransactions=7,
            TickType_t Timeout=5000/portTICK_PERIOD_MS>
    struct ili9341 {
        // the SPI host to use
        constexpr static const spi_host_device_t host_id = HostId;
        // the CS pin
        constexpr static const gpio_num_t pin_cs = PinCS;
        // the DC pin
        constexpr static const gpio_num_t pin_dc = PinDC;
        // the RST pin
        constexpr static const gpio_num_t pin_rst = PinRst;
        // the BL pin
        constexpr static const gpio_num_t pin_backlight = PinBacklight;
        // indicates the buffer size. If specified, will end up being a multiple of 2. The minimum value is 4, for efficiency
        constexpr static const size_t buffer_size = ((BufferSize<4?4:BufferSize)/2)*2;
        // the width of the display
        constexpr static const uint16_t width = 320;
        // the height of the display
        constexpr static const uint16_t height = 240;
        // the maximum number of "in the air" transactions that can be queued
        constexpr static const size_t max_transactions = (0==MaxTransactions)?1:MaxTransactions;
        // the timeout for queued sends
        constexpr static const TickType_t timeout = Timeout;
        // indicates the result of driver operations
        enum struct result {
            // the operation completed successfully
            success = 0,
            // an invalid argument was passed
            invalid_argument=1,
            // there is no more memory
            out_of_memory=2,
            // an I/O error occurred
            io_error=3,
            // the operation timed out
            timeout=4,
            // there is too much I/O pending
            io_pending=5
        };
    private:
        spi_transaction_t m_trans[max_transactions];
        uint8_t m_buffer[buffer_size];
        bool m_initialized;
        size_t m_next_transaction;
        size_t m_batch_left;
        size_t m_queued_transactions;
        spi_device m_spi;
        struct init_cmd {
            uint8_t cmd;
            uint8_t data[16];
            uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
        };
        DRAM_ATTR static const init_cmd s_init_cmds[];
        

        inline static spi_device_interface_config_t get_device_config() {
            spi_device_interface_config_t devcfg={
                .command_bits=0,
                .address_bits=0,
                .dummy_bits=0,
                .mode=0,
                .duty_cycle_pos=0,
                .cs_ena_pretrans=0,
                .cs_ena_posttrans=0,
        #ifdef HTCW_ILI9341_OVERCLOCK
                .clock_speed_hz=26*1000*1000,           //Clock out at 26 MHz
        #else
                .clock_speed_hz=10*1000*1000,           //Clock out at 10 MHz
        #endif
                .input_delay_ns = 0,
                .spics_io_num=pin_cs,               //CS pin
                .flags =0,
                .queue_size=7,                          //We want to be able to queue 7 transactions at a time
                .pre_cb=[](spi_transaction_t*t){
                    int dc=(int)t->user;
                    gpio_set_level(pin_dc, dc);
                },  //Specify pre-transfer callback to handle D/C line
                .post_cb=NULL
            };
            return devcfg;
        } 
        static result xlt_err(spi_result rr) {
            
            if(spi_result::success!=rr) {
                switch(rr) {
                    case spi_result::timeout:
                        return result::timeout;
                    case spi_result::out_of_memory:
                        return result::out_of_memory;
                    case spi_result::previous_transactions_pending:
                        return result::io_pending;
                    default:
                        return result::io_error;
                }
            }
            return result::success;
        }
        result send_transaction(spi_transaction_t* trans,bool queued,bool skip_batch_commit=false) {
            result r = initialize();
            if(result::success!=r)
                return r;
            spi_result rr;
            spi_transaction_t tmp;
            if(!queued && 0!=m_queued_transactions) {
                if(!skip_batch_commit && 0!=m_batch_left) {
                    if(m_queued_transactions>=max_transactions-1) {
                        spi_transaction_t *rtrans;
                        rr=m_spi.get_next_queued_result(&rtrans);
                        if(spi_result::success!=rr)
                            goto send_transaction_error;
                        --m_queued_transactions;
                    }
                    
                    spi_device::make_write(&tmp,m_buffer,m_batch_left*2);
                    rr=m_spi.queue_transaction(&tmp,timeout);
                    if(spi_result::success!=rr)
                        goto send_transaction_error;
                    m_batch_left=0;
                    
                    if(0!=m_queued_transactions)
                        r= queued_wait();
                } else {
                    r=queued_wait();
                }
                if(result::success!=r)
                    return r;
                
            }
            
            if(m_queued_transactions>=max_transactions-1) {
                spi_transaction_t*ptrans;
                rr=m_spi.get_next_queued_result(&ptrans);
                if(spi_result::success!=rr)
                    goto send_transaction_error;
                --m_queued_transactions;
            }
            if(queued) {
                rr = m_spi.queue_transaction(trans,timeout);
            } else {
                
                rr = m_spi.transaction(trans,true);
            }
send_transaction_error:
            if(spi_result::success!=rr) {
                switch(rr) {
                    case spi_result::timeout:
                        return result::timeout;
                    case spi_result::out_of_memory:
                        return result::out_of_memory;
                    case spi_result::previous_transactions_pending:
                        return result::io_pending;
                    default:
                        return result::io_error;
                }
            }
            if(queued)
                ++m_queued_transactions;
            return result::success;
        }
        
        result send_next_command(uint8_t cmd,bool queued,bool skip_batch_commit=false) {
            spi_transaction_t* ptrans = &m_trans[m_next_transaction];
            m_next_transaction=(m_next_transaction+1)%max_transactions;
            spi_device::make_write(ptrans,&cmd,1,(void*)0);
            return send_transaction(ptrans,queued,skip_batch_commit);
        }
        result send_next_data(const uint8_t* data,size_t size,bool queued,bool skip_batch_commit=false) {
            spi_transaction_t* ptrans = &m_trans[m_next_transaction];
            m_next_transaction=(m_next_transaction+1)%max_transactions;
            spi_device::make_write(ptrans,data,size,(void*)1);
            return send_transaction(ptrans,queued,skip_batch_commit);
        }
        result batch_write_commit_impl(bool queued) {
            if(m_batch_left==0)  {
                return result::success;
            }
            // for some reason this trans struct seems to get corrupted
            // so we just rewrite the whole thing
            result r = send_next_data(m_buffer,m_batch_left*2,queued);
            if(result::success!=r)
                return r;
            m_batch_left=0;
            return result::success;
        }
        result batch_write_begin_impl(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,bool queued) {
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
            //Column Address Set
            result r=send_next_command(0x2A,queued);
            if(result::success!=r)
                return r;
            uint8_t tx_data[4];
            tx_data[0]=x1>>8;             //Start Col High
            tx_data[1]=x1&0xFF;           //Start Col Low
            tx_data[2]=x2>>8;             //End Col High
            tx_data[3]=x2&0xff;           //End Col Low
            r=send_next_data(tx_data,4,queued,true);
            if(result::success!=r)
                return r;
            //Page address set
            r=send_next_command(0x2B,queued);
            if(result::success!=r)
                return r;
            tx_data[0]=y1>>8;        //Start page high
            tx_data[1]=y1&0xff;      //start page low
            tx_data[2]=y2>>8;        //end page high
            tx_data[3]=y2&0xff;      //end page low
            r=send_next_data(tx_data,4,queued,true);
            // memory write
            return send_next_command(0x2C,queued);

        }       
        result batch_write_impl(const uint16_t* pixels,size_t count,bool queued) {
            if(!m_initialized)
                return result::io_error;
            result r;
            size_t index = m_batch_left;
            if(index==buffer_size/2) {
                 r=send_next_data(m_buffer,buffer_size,queued,true);
                if(result::success!=r) {
                    return r;
                }
                m_batch_left=0;
                index = 0;
            }
            uint16_t* p=((uint16_t*)m_buffer)+index;
            while(0<count) {    
                *p=*pixels;
                --count;
                ++m_batch_left;
                ++pixels;
                ++p;
                if(m_batch_left==(buffer_size/2)) {
                    r=send_next_data(m_buffer,buffer_size,queued,true);
                    if(result::success!=r)
                        return r;
                    p=(uint16_t*)m_buffer;
                    m_batch_left=0;
                }
            }
            return result::success;
        }
        result pixel_write_impl(uint16_t x,uint16_t y,uint16_t color,bool queued) {
            // check values
            if(x>=width || y>=height)
                return result::success;
            
            // set the address window. we're not actually batching here.
            result r=batch_write_begin_impl(x,y,x,y,queued);
            if(result::success!=r)
                return r;
            return send_next_data((uint8_t*)&color,2,queued);
        }
    public:
        // constructs a new instance of the driver
        ili9341(result* out_result = nullptr) : m_initialized(false),m_next_transaction(0),  m_batch_left(0),m_queued_transactions(0), m_spi(host_id,get_device_config())  {
            if(!m_spi.initialized()) {
                if(nullptr!=out_result) {
                    *out_result=result::io_error;
                }
                return;
            }
        }
        // indicates if the driver is initialized
        inline bool initialized() const {
            return m_initialized;
        }
        // forces initialization of the driver
        result initialize()
        {
            if(!m_initialized) {
                m_batch_left=0;
                static const TickType_t ts = 100/portTICK_RATE_MS;

                int cmd=0;
                
                //Initialize non-SPI GPIOs
                gpio_set_direction(pin_dc, GPIO_MODE_OUTPUT);
                gpio_set_direction(pin_rst, GPIO_MODE_OUTPUT);
                gpio_set_direction(pin_backlight, GPIO_MODE_OUTPUT);

                //Reset the display
                gpio_set_level(pin_rst, 0);
                vTaskDelay(ts);
                gpio_set_level(pin_rst, 1);
                vTaskDelay(ts);
                
                //Send all the commands
                while (s_init_cmds[cmd].databytes!=0xff) {
                    if(spi_result::success!= m_spi.write(&s_init_cmds[cmd].cmd,1)) {
                        return result::io_error;
                    }
                    if(spi_result::success!= m_spi.write(s_init_cmds[cmd].data,s_init_cmds[cmd].databytes&0x1F,(void*)1)) {
                        return result::io_error;
                    }
                    if (s_init_cmds[cmd].databytes&0x80) {
                        vTaskDelay(ts);
                    }
                    ++cmd;
                }

                ///Enable backlight
                gpio_set_level(pin_backlight, 0);

                m_initialized=true;
            }
            return result::success;
        }
        // writes bitmap data to the frame buffer
        result frame_write(uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2,const uint8_t* bmp_data) {
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
            if(x1>=width || y1>=height)
                return result::success;
            result r=batch_write_begin(x1,y1,x2,y2);
            if(result::success!=r)
                return r;
            return send_next_data(bmp_data,(x2-x1+1)*(y2-y1+1)*2,false,true);
        }
        // queues a frame write operation. The bitmap data must be valid for the duration of the operation (until queued_wait())
        result queued_frame_write(uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2,uint8_t* bmp_data,bool precommit=false) {
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
            if(x1>=width || y1>=height)
                return result::success;
            result r;
            if(precommit) {
                // flush any pending batches or transactions if necessary:
                r=batch_write_commit_impl(true);
                if(result::success!=r) {
                    return r;
                }
            }
            r=batch_write_begin_impl(x1,y1,x2,y2,true);
            if(result::success!=r)
                return r;
            
            r=send_next_data(bmp_data,(x2-x1+1)*(y2-y1+1)*2,true,false);
            
            //When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens
            //mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to
            //finish because we may as well spend the time calculating the draw. When that is done, we can call
            //queued_wait(), which will wait for the transfers to be don.
            // otherwise, the transactions will be queued as the old ones finish
            return r;    
        }
// fills the target rectangle of the frame buffer with a pixel
        result frame_fill(uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color) {
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
            uint16_t w = x2-x1+1;
            uint16_t h = y2-y1+1;
            result r=batch_write_begin(x1,y1,x2,y2);
            if(result::success!=r)
                return r;
            size_t pc=w*h;
            while(pc>0) {
                r=batch_write(&color,1);
                if(result::success!=r)
                    return r;
                --pc;
            }
            r=batch_write_commit();
            return r;           
        }
        // queues the fill of a target rectangle with the specified pixel
        result queued_frame_fill(uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color) {
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
            uint16_t w = x2-x1+1;
            uint16_t h = y2-y1+1;
            result r=queued_batch_write_begin(x1,y1,x2,y2);
            if(result::success!=r)
                return r;
            size_t pc=w*h;
            while(pc>0) {
                r=queued_batch_write(&color,1);
                if(result::success!=r)
                    return r;
                --pc;
            }
            r=queued_batch_write_commit();
            return r;
        }

        // begins a batch write for the given target coordinates
        result batch_write_begin(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2) {
            return batch_write_begin_impl(x1,y1,x2,y2,false);
        }
        // queues the beginning of a batch write for the target coordinates
        result queued_batch_write_begin(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2) {
            return batch_write_begin_impl(x1,y1,x2,y2,true);
        }
        // does a batch write. the batch operation must have been started with
        // batch_write_begin() or queued_batch_write_begin()
        result batch_write(const uint16_t* pixels,size_t count) {
            return batch_write_impl(pixels,count,false);
        }
        // does a queued batch write. the batch operation must have been started with
        // batch_write_begin() or queued_batch_write_begin()
        result queued_batch_write(const uint16_t* pixels,size_t count) {
            return batch_write_impl(pixels,count,true);
        }
        // commits any pending batch
        result batch_write_commit() {
            return batch_write_commit_impl(false);
        }
        // queues the commit of any pending batch
        result queued_batch_write_commit() {
            return batch_write_commit_impl(true);
        }
        // writes a pixel at the specified coordinates
        inline result pixel_write(uint16_t x,uint16_t y,uint16_t color) {
            return pixel_write_impl(x,y,color,false);
        }
        // queues a write of a pixel at the specified coordinates
        inline result queued_pixel_write(uint16_t x,uint16_t y,uint16_t color) {
            return pixel_write_impl(x,y,true);
        }
        // waits for all pending queued operations
        result queued_wait()
        {
            spi_transaction_t *rtrans;
            //Wait for all of the transactions to be done and get back the results.
            for (;m_queued_transactions>0; --m_queued_transactions) {
                if(spi_result::success!=m_spi.get_next_queued_result(&rtrans))
                    return result::io_error;
                //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
            }
            return result::success;
        }
        
        // GFX bindings
 public:
        // indicates the type, itself
        using type = ili9341<HostId,PinCS,PinDC,PinRst,PinBacklight,BufferSize,MaxTransactions,Timeout>;
        // indicates the pixel type
        using pixel_type = gfx::rgb_pixel<16>;
        // indicates the capabilities of the driver
        using caps = gfx::gfx_caps<false,true,true,true,false,true,false,true,false,true,false>;
 
 private:
        gfx::gfx_result xlt_err(result r) {
            switch(r) {
                case result::io_error:
                    return gfx::gfx_result::device_error;
                case result::out_of_memory:
                    return gfx::gfx_result::out_of_memory;
                case result::success:
                    return gfx::gfx_result::success;
                default:
                    return gfx::gfx_result::invalid_argument;
            }
        }
        template<typename Source>
        gfx::gfx_result write_frame_impl(const gfx::rect16& src_rect,const Source& src,gfx::point16 location,bool async) {
            result r;
            gfx::rect16 srcr = src_rect.normalize().crop(src.bounds());
            gfx::rect16 dstr(location,src_rect.dimensions());
            dstr=dstr.crop(bounds());
            if(srcr.width()>dstr.width()) {
                srcr.x2=srcr.x1+dstr.width()-1;
            }
            if(srcr.height()>dstr.height()) {
                srcr.y2=srcr.y1+dstr.height()-1;
            }
            if(gfx::helpers::is_same<pixel_type,typename Source::pixel_type>::value && Source::caps::blt) {
                // direct blt
                if(src.bounds().width()==srcr.width() && srcr.x1==0) {
                    if(!async)
                        r=frame_write(dstr.x1,dstr.y1,dstr.x2,dstr.y2,src.begin()+(srcr.y1*src.dimensions().width*2));
                    else
                        r=queued_frame_write(dstr.x1,dstr.y1,dstr.x2,dstr.y2,src.begin()+(srcr.y1*src.dimensions().width*2));
                    if(result::success!=r) {
                        return xlt_err(r);
                    }
                    return gfx::gfx_result::success;
                }
                // line by line blt
                uint16_t yy=0;
                uint16_t hh=srcr.height();
                uint16_t ww = src.dimensions().width;
                while(yy<hh) {
                    if(!async)
                        r = frame_write(dstr.x1,dstr.y1+yy,dstr.x2,dstr.x2+yy,src.begin()+(ww*(srcr.y1+yy)+srcr.x1));
                    else
                        r = queued_frame_write(dstr.x1,dstr.y1+yy,dstr.x2,dstr.x2+yy,src.begin()+(ww*(srcr.y1+yy)+srcr.x1));
                    if(result::success!=r) {
                        return xlt_err(r);
                    }
                    ++yy;
                }
                return gfx::gfx_result::success;
            }
            uint16_t w = dstr.dimensions().width;
            uint16_t h = dstr.dimensions().height;
            if(!async)
                r=batch_write_begin(dstr.x1,dstr.y1,dstr.x2,dstr.y2);
            else
                r=queued_batch_write_begin(dstr.x1,dstr.y1,dstr.x2,dstr.y2);
            if(result::success!=r) {
                return xlt_err(r);
            }
            for(uint16_t y=0;y<h;++y) {
                for(uint16_t x=0;x<w;++x) {
                    typename Source::pixel_type pp;
                    gfx::gfx_result rr=src.point(gfx::point16(x+srcr.x1,y+srcr.y1), &pp);
                    if(rr!=gfx::gfx_result::success)
                        return rr;
                    pixel_type p;
                    if(!pp.convert(&p)) {
                        return gfx::gfx_result::invalid_format;
                    }
                    uint16_t pv = p.value();
                    if(!async)
                        r = batch_write(&pv,1);
                    else
                        r = queued_batch_write(&pv,1);
                    if(result::success!=r) {
                        return xlt_err(r);
                    }
                }
            }
            if(!async)
                r=batch_write_commit();
            else
                r=queued_batch_write_commit();
            if(result::success!=r) {
                return xlt_err(r);
            }
            return gfx::gfx_result::success;
        }
 public:
        // retrieves the dimensions of the screen
        constexpr inline gfx::size16 dimensions() const {
            return gfx::size16(width,height);
        }
        // retrieves the bounds of the screen
        constexpr inline gfx::rect16 bounds() const {
            return gfx::rect16(gfx::point16(0,0),dimensions());
        }
        // sets a point to the specified pixel
        gfx::gfx_result point(gfx::point16 location,pixel_type pixel) {
            result r = pixel_write(location.x,location.y,pixel.value());
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // asynchronously sets a point to the specified pixel
        gfx::gfx_result point_async(gfx::point16 location,pixel_type pixel) {
            result r = queued_pixel_write(location.x,location.y,pixel.value());
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // fills the specified rectangle with the specified pixel
        gfx::gfx_result fill(const gfx::rect16& rect,pixel_type color) {
            result r=frame_fill(rect.x1,rect.y1,rect.x2,rect.y2,color.value());
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // asynchronously fills the specified rectangle with the specified pixel
        gfx::gfx_result fill_async(const gfx::rect16& rect,pixel_type color) {
            result r=queued_frame_fill(rect.x1,rect.y1,rect.x2,rect.y2,color.value());
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // clears the specified rectangle
        inline gfx::gfx_result clear(const gfx::rect16& rect) {
            pixel_type p;
            return fill(rect,p);
        }
        // asynchronously clears the specified rectangle
        inline gfx::gfx_result clear_async(const gfx::rect16& rect) {
            pixel_type p;
            return fill_async(rect,p);
        }
        // begins a batch operation for the specified rectangle
        gfx::gfx_result begin_batch(const gfx::rect16& rect) {
            if(result::success!=batch_write_begin(rect.x1,rect.y1,rect.x2,rect.y2))
                return gfx::gfx_result::device_error;
            return gfx::gfx_result::success;
        }
        // asynchronously begins a batch operation for the specified rectangle
        gfx::gfx_result begin_batch_async(const gfx::rect16& rect) {
            result r =queued_batch_write_begin(rect.x1,rect.y1,rect.x2,rect.y2);
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // writes a pixel to a pending batch
        gfx::gfx_result write_batch(pixel_type color) {
            uint16_t p = color.value();
            result r = this->batch_write(&p,1);
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // asynchronously writes a pixel to a pending batch
        gfx::gfx_result write_batch_async(pixel_type color) {
            uint16_t p = color.value();
            result r = this->queued_batch_write(&p,1);
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // commits a pending batch
        gfx::gfx_result commit_batch() {
            result r=batch_write_commit();
            if(result::success!=r)
                return xlt_err(r);
                
            return gfx::gfx_result::success;
        }
        // asynchronously commits a pending batch
        gfx::gfx_result commit_batch_async() {
            result r=queued_batch_write_commit();
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // writes source data to a frame
        template<typename Source>
        inline gfx::gfx_result write_frame(const gfx::rect16& src_rect,const Source& src,gfx::point16 location) {
            return write_frame_impl(src_rect,src,location,false);
        }
        // asynchronously writes source data to a frame
        template<typename Source>
        inline gfx::gfx_result write_frame_async(const gfx::rect16& src_rect,const Source& src,gfx::point16 location) {
            return write_frame_impl(src_rect,src,location,true);
        }
        // waits for all pending asynchronous operations to complete
        gfx::gfx_result wait_async() {
            result r = batch_write_commit_impl(m_queued_transactions!=0);
            if(result::success!=r)
                return xlt_err(r);
            r=queued_wait();
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
    };
    template<spi_host_device_t HostId,gpio_num_t PinCS,gpio_num_t PinDC,gpio_num_t PinRst,gpio_num_t PinBacklight,size_t BufferSize,size_t MaxTransactions,TickType_t Timeout>
    const typename ili9341<HostId,PinCS,PinDC,PinRst,PinBacklight,BufferSize,MaxTransactions,Timeout>::init_cmd ili9341<HostId,PinCS,PinDC,PinRst,PinBacklight,BufferSize,MaxTransactions,Timeout>::s_init_cmds[]={
            /* Power contorl B, power control = 0, DC_ENA = 1 */
            {0xCF, {0x00, 0x83, 0X30}, 3},
            /* Power on sequence control,
            * cp1 keeps 1 frame, 1st frame enable
            * vcl = 0, ddvdh=3, vgh=1, vgl=2
            * DDVDH_ENH=1
            */
            {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
            /* Driver timing control A,
            * non-overlap=default +1
            * EQ=default - 1, CR=default
            * pre-charge=default - 1
            */
            {0xE8, {0x85, 0x01, 0x79}, 3},
            /* Power control A, Vcore=1.6V, DDVDH=5.6V */
            {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
            /* Pump ratio control, DDVDH=2xVCl */
            {0xF7, {0x20}, 1},
            /* Driver timing control, all=0 unit */
            {0xEA, {0x00, 0x00}, 2},
            /* Power control 1, GVDD=4.75V */
            {0xC0, {0x26}, 1},
            /* Power control 2, DDVDH=VCl*2, VGH=VCl*7, VGL=-VCl*3 */
            {0xC1, {0x11}, 1},
            /* VCOM control 1, VCOMH=4.025V, VCOML=-0.950V */
            {0xC5, {0x35, 0x3E}, 2},
            /* VCOM control 2, VCOMH=VMH-2, VCOML=VML-2 */
            {0xC7, {0xBE}, 1},
            /* Memory access contorl, MX=MY=0, MV=1, ML=0, BGR=1, MH=0 */
            {0x36, {0x28}, 1},
            /* Pixel format, 16bits/pixel for RGB/MCU interface */
            {0x3A, {0x55}, 1},
            /* Frame rate control, f=fosc, 70Hz fps */
            {0xB1, {0x00, 0x1B}, 2},
            /* Enable 3G, disabled */
            {0xF2, {0x08}, 1},
            /* Gamma set, curve 1 */
            {0x26, {0x01}, 1},
            /* Positive gamma correction */
            {0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
            /* Negative gamma correction */
            {0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
            /* Column address set, SC=0, EC=0xEF */
            {0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
            /* Page address set, SP=0, EP=0x013F */
            {0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
            /* Memory write */
            {0x2C, {0}, 0},
            /* Entry mode set, Low vol detect disabled, normal display */
            {0xB7, {0x07}, 1},
            /* Display function control */
            {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
            /* Sleep out */
            {0x11, {0}, 0x80},
            /* Display on */
            {0x29, {0}, 0x80},
            {0, {0}, 0xff},
        };
}