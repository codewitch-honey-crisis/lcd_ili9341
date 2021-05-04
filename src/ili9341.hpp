#pragma once
#define CONFIG_LCD_OVERCLOCK
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "spi_master.hpp"

#include "gfx_core.hpp"
#include "gfx_positioning.hpp"
#include "gfx_pixel.hpp"

namespace espidf {
    template<spi_host_device_t HostId,gpio_num_t PinCS,gpio_num_t PinDC,gpio_num_t PinRst,gpio_num_t PinBacklight,size_t BufferSize=64>
    struct ili9341 {
        constexpr static const spi_host_device_t host_id = HostId;
        constexpr static const gpio_num_t pin_cs = PinCS;
        constexpr static const gpio_num_t pin_dc = PinDC;
        constexpr static const gpio_num_t pin_rst = PinRst;
        constexpr static const gpio_num_t pin_backlight = PinBacklight;
        // indicates the buffer size. If specified, will end up being a multiple of 2. The minimum value is 4, for efficiency
        constexpr static const size_t buffer_size = ((BufferSize<4?4:BufferSize)/2)*2;
        constexpr static const uint16_t width = 320;
        constexpr static const uint16_t height = 240;
        enum struct result {
            success = 0,
            invalid_argument=1,
            out_of_memory=2,
            io_error=3
        };
    private:
        spi_transaction_t m_trans[6] = {
            // #0 Even transfers are commands
            {
                .flags=SPI_TRANS_USE_TXDATA,                 ///< Bitwise OR of SPI_TRANS_* flags
                .cmd=0,                   /**< Command data, of which the length is set in the ``command_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "command" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF 3.0.</b>
                                *
                                *  Example: write 0x0123 and command_bits=12 to send command 0x12, 0x3_ (in previous version, you may have to write 0x3_12).
                                */
                .addr=0,                  /**< Address data, of which the length is set in the ``address_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "address" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF3.0.</b>
                                *
                                *  Example: write 0x123400 and address_bits=24 to send address of 0x12, 0x34, 0x00 (in previous version, you may have to write 0x12340000).
                                */
                .length=8,                  ///< Total data length, in bits
                .rxlength=0,                ///< Total data length received, should be not greater than ``length`` in full-duplex mode (0 defaults this to the value of ``length``).
                .user=(void*)0,                     ///< User-defined variable. Can be used to store eg transaction ID.
                //union {
                //    const void *tx_buffer;      ///< Pointer to transmit buffer, or NULL for no MOSI phase
                //    uint8_t tx_data[4];         ///< If SPI_TRANS_USE_TXDATA is set, data set here is sent directly from this variable.
                //}
                .tx_data = {0x2A,0,0,0}, // column address set
                .rx_buffer=NULL
            },
            // #1 Odd transfers are data
            {
                .flags=SPI_TRANS_USE_TXDATA,                 ///< Bitwise OR of SPI_TRANS_* flags
                .cmd=0,                   /**< Command data, of which the length is set in the ``command_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "command" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF 3.0.</b>
                                *
                                *  Example: write 0x0123 and command_bits=12 to send command 0x12, 0x3_ (in previous version, you may have to write 0x3_12).
                                */
                .addr=0,                  /**< Address data, of which the length is set in the ``address_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "address" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF3.0.</b>
                                *
                                *  Example: write 0x123400 and address_bits=24 to send address of 0x12, 0x34, 0x00 (in previous version, you may have to write 0x12340000).
                                */
                .length=8*4,                  ///< Total data length, in bits
                .rxlength=0,                ///< Total data length received, should be not greater than ``length`` in full-duplex mode (0 defaults this to the value of ``length``).
                .user=(void*)1,                     ///< User-defined variable. Can be used to store eg transaction ID.
                //union {
                //    const void *tx_buffer;      ///< Pointer to transmit buffer, or NULL for no MOSI phase
                //    uint8_t tx_data[4];         ///< If SPI_TRANS_USE_TXDATA is set, data set here is sent directly from this variable.
                //}
                .tx_data = {0,0,0,0}, // column start and end address high and low
                .rx_buffer=NULL
            },
            // #2 Even transfers are commands
            {
                .flags=SPI_TRANS_USE_TXDATA,                 ///< Bitwise OR of SPI_TRANS_* flags
                .cmd=0,                   /**< Command data, of which the length is set in the ``command_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "command" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF 3.0.</b>
                                *
                                *  Example: write 0x0123 and command_bits=12 to send command 0x12, 0x3_ (in previous version, you may have to write 0x3_12).
                                */
                .addr=0,                  /**< Address data, of which the length is set in the ``address_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "address" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF3.0.</b>
                                *
                                *  Example: write 0x123400 and address_bits=24 to send address of 0x12, 0x34, 0x00 (in previous version, you may have to write 0x12340000).
                                */
                .length=8,                  ///< Total data length, in bits
                .rxlength=0,                ///< Total data length received, should be not greater than ``length`` in full-duplex mode (0 defaults this to the value of ``length``).
                .user=(void*)0,                     ///< User-defined variable. Can be used to store eg transaction ID.
                //union {
                //    const void *tx_buffer;      ///< Pointer to transmit buffer, or NULL for no MOSI phase
                //    uint8_t tx_data[4];         ///< If SPI_TRANS_USE_TXDATA is set, data set here is sent directly from this variable.
                //}
                .tx_data = {0x2B,0,0,0}, // "page"/row address set
                .rx_buffer=NULL
            },
            // #3 Odd transfers are data
            {
                .flags=SPI_TRANS_USE_TXDATA,                 ///< Bitwise OR of SPI_TRANS_* flags
                .cmd=0,                   /**< Command data, of which the length is set in the ``command_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "command" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF 3.0.</b>
                                *
                                *  Example: write 0x0123 and command_bits=12 to send command 0x12, 0x3_ (in previous version, you may have to write 0x3_12).
                                */
                .addr=0,                  /**< Address data, of which the length is set in the ``address_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "address" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF3.0.</b>
                                *
                                *  Example: write 0x123400 and address_bits=24 to send address of 0x12, 0x34, 0x00 (in previous version, you may have to write 0x12340000).
                                */
                .length=8*4,                  ///< Total data length, in bits
                .rxlength=0,                ///< Total data length received, should be not greater than ``length`` in full-duplex mode (0 defaults this to the value of ``length``).
                .user=(void*)1,                     ///< User-defined variable. Can be used to store eg transaction ID.
                //union {
                //    const void *tx_buffer;      ///< Pointer to transmit buffer, or NULL for no MOSI phase
                //    uint8_t tx_data[4];         ///< If SPI_TRANS_USE_TXDATA is set, data set here is sent directly from this variable.
                //}
                .tx_data = {0,0,0,0}, // "page"/row start and end address high and low
                .rx_buffer=NULL
            },
            // #4 (Write) Even transfers are commands
            {
                .flags=SPI_TRANS_USE_TXDATA,                 ///< Bitwise OR of SPI_TRANS_* flags
                .cmd=0,                   /**< Command data, of which the length is set in the ``command_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "command" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF 3.0.</b>
                                *
                                *  Example: write 0x0123 and command_bits=12 to send command 0x12, 0x3_ (in previous version, you may have to write 0x3_12).
                                */
                .addr=0,                  /**< Address data, of which the length is set in the ``address_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "address" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF3.0.</b>
                                *
                                *  Example: write 0x123400 and address_bits=24 to send address of 0x12, 0x34, 0x00 (in previous version, you may have to write 0x12340000).
                                */
                .length=8,                  ///< Total data length, in bits
                .rxlength=0,                ///< Total data length received, should be not greater than ``length`` in full-duplex mode (0 defaults this to the value of ``length``).
                .user=(void*)0,                     ///< User-defined variable. Can be used to store eg transaction ID.
                //union {
                //    const void *tx_buffer;      ///< Pointer to transmit buffer, or NULL for no MOSI phase
                //    uint8_t tx_data[4];         ///< If SPI_TRANS_USE_TXDATA is set, data set here is sent directly from this variable.
                //}
                .tx_data = {0x2C,0,0,0}, // memory write
                .rx_buffer=NULL
            },
            // #5 (Write) Odd transfers are data
            {
                .flags=0,                 ///< Bitwise OR of SPI_TRANS_* flags
                .cmd=0,                   /**< Command data, of which the length is set in the ``command_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "command" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF 3.0.</b>
                                *
                                *  Example: write 0x0123 and command_bits=12 to send command 0x12, 0x3_ (in previous version, you may have to write 0x3_12).
                                */
                .addr=0,                  /**< Address data, of which the length is set in the ``address_bits`` of spi_device_interface_config_t.
                                *
                                *  <b>NOTE: this field, used to be "address" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF3.0.</b>
                                *
                                *  Example: write 0x123400 and address_bits=24 to send address of 0x12, 0x34, 0x00 (in previous version, you may have to write 0x12340000).
                                */
                .length=0,                  /// set to Total data length, in bits 
                .rxlength=0,                ///< Total data length received, should be not greater than ``length`` in full-duplex mode (0 defaults this to the value of ``length``).
                .user=(void*)1,                     ///< User-defined variable. Can be used to store eg transaction ID.
                //union {
                //    const void *tx_buffer;      ///< Pointer to transmit buffer, or NULL for no MOSI phase
                //    uint8_t tx_data[4];         ///< If SPI_TRANS_USE_TXDATA is set, data set here is sent directly from this variable.
                //}
                .tx_buffer=nullptr, // set to the frame buffer data
                .rx_buffer=NULL
            }
        };
        uint8_t m_buffer[buffer_size];
        bool m_initialized;
        size_t m_batch_left;
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
        #ifdef CONFIG_LCD_OVERCLOCK
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
        
    public:
        ili9341(result* out_result = nullptr) : m_initialized(false), m_batch_left(0), m_spi(host_id,get_device_config())  {
            if(!m_spi.initialized()) {
                if(nullptr!=out_result) {
                    *out_result=result::io_error;
                }
                return;
            }
        }
        inline bool initialize() const {
            return m_initialized;
        }
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
        result queue_frame_write(uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2,uint8_t* bmp_data) {
            result r = initialize();
            if(result::success!=r)
                return r;
            r=batch_write_commit();
            if(result::success!=r)
                return r;
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
            
            //Column Address Set
            if(spi_result::success!=m_spi.queue_transaction(&m_trans[0]))
                return result::io_error;
            m_trans[1].tx_data[0]=x1>>8;             //Start Col High
            m_trans[1].tx_data[1]=x1&0xFF;           //Start Col Low
            m_trans[1].tx_data[2]=x2>>8;             //End Col High
            m_trans[1].tx_data[3]=x2&0xff;           //End Col Low
            if(spi_result::success!=m_spi.queue_transaction(&m_trans[1]))
                return result::io_error;
            //Page address set
            if(spi_result::success!=m_spi.queue_transaction(&m_trans[2]))
                return result::io_error;
            m_trans[3].tx_data[0]=y1>>8;        //Start page high
            m_trans[3].tx_data[1]=y1&0xff;      //start page low
            m_trans[3].tx_data[2]=y2>>8;        //end page high
            m_trans[3].tx_data[3]=y2&0xff;      //end page low
            if(spi_result::success!=m_spi.queue_transaction(&m_trans[3]))
                return result::io_error;
            //memory write
            if(spi_result::success!=m_spi.queue_transaction(&m_trans[4]))
                return result::io_error;
            m_trans[5].flags=0;
            m_trans[5].tx_buffer=bmp_data;
            m_trans[5].length=(x2-x1+1)*(y2-y1+1)*16;
            if(spi_result::success!=m_spi.queue_transaction(&m_trans[5]))
                return result::io_error;
            
            
            //When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens
            //mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to
            //finish because we may as well spend the time calculating the next line. When that is done, we can call
            //send_line_finish, which will wait for the transfers to be done and check their status.
            return result::success;
        }

        result wait_queued_frame_write()
        {
            spi_transaction_t *rtrans;
            //Wait for all 6 transactions to be done and get back the results.
            for (int x=0; x<6; x++) {
                if(spi_result::success!=m_spi.get_next_queued_result(&rtrans))
                    return result::io_error;
                //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
            }
            return result::success;
        }
        result batch_write_begin(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2) {
            result r = initialize();
            if(result::success!=r)
                return r;
            r=batch_write_commit();
            if(result::success!=r)
                return r;
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
            if(spi_result::success!=m_spi.transaction(&m_trans[0],true))
                return result::io_error;
            m_trans[1].tx_data[0]=x1>>8;             //Start Col High
            m_trans[1].tx_data[1]=x1&0xFF;           //Start Col Low
            m_trans[1].tx_data[2]=x2>>8;             //End Col High
            m_trans[1].tx_data[3]=x2&0xff;           //End Col Low
            if(spi_result::success!=m_spi.transaction(&m_trans[1],true))
                return result::io_error;
            //Page address set
            if(spi_result::success!=m_spi.transaction(&m_trans[2],true))
                return result::io_error;
            m_trans[3].tx_data[0]=y1>>8;        //Start page high
            m_trans[3].tx_data[1]=y1&0xff;      //start page low
            m_trans[3].tx_data[2]=y2>>8;        //end page high
            m_trans[3].tx_data[3]=y2&0xff;      //end page low
            if(spi_result::success!=m_spi.transaction(&m_trans[3],true))
                return result::io_error;
             //memory write
            if(spi_result::success!=m_spi.transaction(&m_trans[4],true))
                return result::io_error;
            spi_device::make_write(&m_trans[5],m_buffer,buffer_size,(void*)1);
            return result::success;
        }
        result batch_write(uint16_t* pixels,size_t count) {
            if(!m_initialized)
                return result::io_error;
            size_t index = m_batch_left;
            if(index==buffer_size/2) {
                //printf("batch write sending %d bytes\r\n",(int)m_batch_left*2);
                /*if(spi_result::success!=m_spi.transaction(&m_trans[5],true)) {
                    return result::io_error;
                }*/
                if(spi_result::success!=m_spi.write(m_buffer,buffer_size,(void*)1)) {
                    return result::io_error;
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
                    //printf("batch write sending %d bytes\r\n",(int)m_batch_left*2);
                    /*if(spi_result::success!=m_spi.transaction(&m_trans[5],true)) {
                        return result::io_error;
                    }*/
                    if(spi_result::success!=m_spi.write(m_buffer,buffer_size,(void*)1)) {
                        return result::io_error;
                    }
                    p=(uint16_t*)m_buffer;
                    m_batch_left=0;
                }
            }
            return result::success;
        }
        result batch_write_commit() {
            if(m_batch_left==0) 
                return result::success;
            // for some reason this trans struct seems to get corrupted
            // so we just rewrite the whole thing
            //spi_device::make_write(&m_trans[5],m_buffer,m_batch_left*2);
            //printf("batch write commit sending %d bytes\r\n",(int)m_batch_left*2);
            /*if(spi_result::success!=m_spi.transaction(&m_trans[5],true)) {
                return result::io_error;
            }*/
            if(spi_result::success!=m_spi.write(m_buffer,m_batch_left*2,(void*)1)) {
                return result::io_error;
            }
        
            m_batch_left=0;
            return result::success;
        }
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
            if(result::success!=batch_write_begin(x1,y1,x2,y2))
                return result::io_error;
            m_trans[5].flags=0;
            m_trans[5].tx_buffer=bmp_data;
            m_trans[5].length=(x2-x1+1)*(y2-y1+1)*16;
            if(spi_result::success!=m_spi.transaction(&m_trans[5],true))
                return result::io_error;
            m_batch_left=0;
            
            return result::success;
        }
        
        result write_pixel(uint16_t x,uint16_t y,uint16_t color) {
            result r = initialize();
            if(result::success!=r)
                return r;
            r=batch_write_commit();
            if(result::success!=r)
                return r;
            // check values
            if(x>=width || y>=height)
                return result::success;
            static const bool use_polling=true;
            //Column Address Set
            if(spi_result::success!=m_spi.transaction(&m_trans[0],use_polling))
                return result::io_error;
            taskYIELD();
            m_trans[1].tx_data[2]=m_trans[1].tx_data[0]=x>>8;             //Start Col High
            m_trans[1].tx_data[3]=m_trans[1].tx_data[1]=x&0xFF;           //Start Col Low
            if(spi_result::success!=m_spi.transaction(&m_trans[1],use_polling))
                return result::io_error;
            //Page address set
            if(spi_result::success!=m_spi.transaction(&m_trans[2],use_polling))
                return result::io_error;
            m_trans[3].tx_data[2]=m_trans[3].tx_data[0]=y>>8;        //Start page high
            m_trans[3].tx_data[3]=m_trans[3].tx_data[1]=y&0xff;      //start page low
            if(spi_result::success!=m_spi.transaction(&m_trans[3],use_polling))
                return result::io_error;
            //memory write
            if(spi_result::success!=m_spi.transaction(&m_trans[4],use_polling))
                return result::io_error;
            spi_device::make_write(&m_trans[5],(uint8_t*)&color,2,(void*)1);
            if(spi_result::success!=m_spi.transaction(&m_trans[5],use_polling))
                return result::io_error;
            
            //if(spi_result::success!=m_spi.write((uint8_t*)&color,2,(void*)1))
            //    return result::io_error;
            
            return result::success;
        }
      
        result frame_fill(uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color) {
            result r = initialize();
            if(result::success!=r)
                return r;
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

            if(result::success!=batch_write_begin(x1,y1,x2,y2))
                return result::io_error;
            taskYIELD();
            size_t pc=w*h;
            while(pc>0) {
                if(result::success!=batch_write(&color,1))
                    return result::io_error;
                --pc;
            }
            if(result::success!=batch_write_commit()) {
                return result::io_error;
            }
           
            return result::success;
           
        }

        // GFX bindings
 public:
        using type = ili9341<HostId,PinCS,PinDC,PinRst,PinBacklight,BufferSize>;
        using pixel_type = gfx::rgb_pixel<16>;
        using caps = gfx::gfx_caps<false,true,true,false,true,false,true,false,true,false>;
 
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
 public:
        constexpr inline gfx::size16 dimensions() const {
            return gfx::size16(width,height);
        }
        constexpr inline gfx::rect16 bounds() const {
            return gfx::rect16(gfx::point16(0,0),dimensions());
        }
        gfx::gfx_result point(gfx::point16 location,pixel_type pixel) {
            result r = write_pixel(location.x,location.y,pixel.value());
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        gfx::gfx_result fill(const gfx::rect16& rect,pixel_type color) {
            result r=frame_fill(rect.x1,rect.y1,rect.x2,rect.y2,color.value());
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        inline gfx::gfx_result clear(const gfx::rect16& rect) {
            // make a black pixel
            gfx::gsc_pixel<1> p(0);
            return fill(rect,p.convert<pixel_type>());
        }
        gfx::gfx_result begin_batch(const gfx::rect16& rect) {
            if(result::success!=batch_write_begin(rect.x1,rect.y1,rect.x2,rect.y2))
                return gfx::gfx_result::device_error;
            return gfx::gfx_result::success;
        }
        gfx::gfx_result commit_batch() {
            if(result::success!=batch_write_commit())
                return gfx::gfx_result::device_error;
            return gfx::gfx_result::success;
        }
        gfx::gfx_result batch_write(pixel_type color) {
            uint16_t p = color.value();
            result r = this->batch_write(&p,1);
            if(result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        template<typename Source>
        gfx::gfx_result frame_write(const gfx::rect16& src_rect,const Source& src,gfx::point16 location) {
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
                    result r=frame_write(dstr.x1,dstr.y1,dstr.x2,dstr.y2,src.begin()+(srcr.y1*src.dimensions().width*2));
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
                    result r = frame_write(dstr.x1,dstr.y1+yy,dstr.x2,dstr.x2+yy,src.begin()+(ww*(srcr.y1+yy)+srcr.x1));
                    if(result::success!=r) {
                        return xlt_err(r);
                    }
                    ++yy;
                }
                return gfx::gfx_result::success;
            }
            uint16_t w = dstr.dimensions().width;
            uint16_t h = dstr.dimensions().height;
            result r=batch_write_begin(dstr.x1,dstr.y1,dstr.x2,dstr.y2);
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
                    r = batch_write(&pv,1);
                    if(result::success!=r) {
                        return xlt_err(r);
                    }
                }
            }
            r=batch_write_commit();
            if(result::success!=r) {
                return xlt_err(r);
            }
            return gfx::gfx_result::success;
        }
    };
    template<spi_host_device_t HostId,gpio_num_t PinCS,gpio_num_t PinDC,gpio_num_t PinRst,gpio_num_t PinBacklight,size_t BufferSize>
    const typename ili9341<HostId,PinCS,PinDC,PinRst,PinBacklight,BufferSize>::init_cmd ili9341<HostId,PinCS,PinDC,PinRst,PinBacklight,BufferSize>::s_init_cmds[]={
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