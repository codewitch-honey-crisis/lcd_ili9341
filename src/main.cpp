extern "C" { void app_main(); }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "spi_master.hpp"
#include "esp_spiffs.h"
#include "ili9341.hpp"
#include "stream.hpp"
#include "gfx_bitmap.hpp"
#include "gfx_image.hpp"
#include "gfx_drawing.hpp"
#include "gfx_color_cpp14.hpp"
#include "../fonts/Bm437_ATI_8x16.h"
using namespace espidf;
using namespace io;
using namespace gfx;

// the following is configured for the ESP-WROVER-KIT
// make sure to set the pins to your set up.
#ifdef CONFIG_IDF_TARGET_ESP32
#define LCD_HOST    HSPI_HOST
#define DMA_CHAN    2

#define PIN_NUM_MISO GPIO_NUM_25
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_CLK  GPIO_NUM_19
#define PIN_NUM_CS   GPIO_NUM_22

#define PIN_NUM_DC   GPIO_NUM_21
#define PIN_NUM_RST  GPIO_NUM_18
#define PIN_NUM_BCKL GPIO_NUM_5
#elif defined CONFIG_IDF_TARGET_ESP32S2
#define LCD_HOST    SPI2_HOST
#define DMA_CHAN    LCD_HOST

#define PIN_NUM_MISO GPIO_NUM_37
#define PIN_NUM_MOSI GPIO_NUM_35
#define PIN_NUM_CLK  GPIO_NUM_36
#define PIN_NUM_CS   GPIO_NUM_34

#define PIN_NUM_DC   GPIO_NUM_4
#define PIN_NUM_RST  GPIO_NUM_5
#define PIN_NUM_BCKL GPIO_NUM_6
#elif defined CONFIG_IDF_TARGET_ESP32C3
#define LCD_HOST    SPI2_HOST
#define DMA_CHAN    LCD_HOST

#define PIN_NUM_MISO GPIO_NUM_2
#define PIN_NUM_MOSI GPIO_NUM_7
#define PIN_NUM_CLK  GPIO_NUM_6
#define PIN_NUM_CS   GPIO_NUM_10

#define PIN_NUM_DC   GPIO_NUM_9
#define PIN_NUM_RST  GPIO_NUM_18
#define PIN_NUM_BCKL GPIO_NUM_19
#endif

// To speed up transfers, every SPI transfer sends as much data as possible. 
// This define specifies how much. More means more memory use, but less 
// overhead for setting up / finishing transfers.
#define PARALLEL_LINES 16

// configure the spi bus. Must be done before the driver
spi_master g_spi_host(nullptr,
                    LCD_HOST,
                    PIN_NUM_CLK,
                    PIN_NUM_MISO,
                    PIN_NUM_MOSI,
                    GPIO_NUM_NC,
                    GPIO_NUM_NC,
                    // This is much bigger than we need:
                    PARALLEL_LINES*320*2+8,
                    DMA_CHAN);

// we use the default, modest buffer - it makes things slower but uses less
// memory. it usually works fine at default but you can change it for performance 
// tuning. It's the final parameter: Note that it shouldn't be any bigger than 
// the DMA size
using lcd_type = ili9341<LCD_HOST,
                        PIN_NUM_CS,
                        PIN_NUM_DC,
                        PIN_NUM_RST,
                        PIN_NUM_BCKL
                        /*,PARALLEL_LINES*320*2+8*/>;

// declaring this saves us typing - we can do lcd_color::white for example:
using lcd_color = gfx::color<typename lcd_type::pixel_type>;
lcd_type lcd;

// demonstrates how to use the "bare metal" driver calls, bypassing GFX
void raw_driver_batch_demo() {
    lcd.batch_write_begin(0,0,lcd_type::width-1,lcd_type::height-1);
    
    for(uint16_t y=0;y<lcd_type::height;++y) {
        for(uint16_t x=0;x<lcd_type::width;++x) {
            // alternate white and black
            uint16_t v=0xFFFF*((x+y)%2);
            if(lcd_type::result::success!=lcd.batch_write(&v,1)) {
                printf("write pixel failed\r\n");
                y=lcd_type::height;
                return;
            }
        }
    }
            
    lcd.batch_write_commit();
    vTaskDelay(3000/portTICK_PERIOD_MS);
    lcd.queued_batch_write_begin(0,0,lcd_type::width-1,lcd_type::height-1);
    for(uint16_t y=0;y<lcd_type::height;++y) {
        for(uint16_t x=0;x<lcd_type::width;++x) {
            uint16_t v=0xF800*((x+y)%2);
            v+=v=0x001F*(1-((x+y)%2));
            if(lcd_type::result::success!=lcd.queued_batch_write(&v,1)) {
                printf("write pixel failed\r\n");
                y=lcd_type::height;
                return;
            }           
        }
    }
    lcd.queued_batch_write_commit();
    vTaskDelay(3000/portTICK_PERIOD_MS);
    
}
void app_main(void)
{
    // check to make sure SPI was initialized successfully
    if(!g_spi_host.initialized()) {
        printf("SPI host initialization error.\r\n");
        abort();
    }
    // mount SPIFFS
    esp_err_t ret;
    esp_vfs_spiffs_conf_t conf = {};
    conf.base_path="/spiffs";
    conf.format_if_mount_failed=false;
    conf.max_files=5;
    conf.partition_label="storage";
    ret=esp_vfs_spiffs_register(&conf);
    ESP_ERROR_CHECK(ret);   
    raw_driver_batch_demo();
    
    // clear the display
    lcd.clear(lcd.bounds());
    
    // we actually don't need more than 3 bits here for the colors 
    // we are using. Storing it in 3 bits saves memory but the 
    // color depth isn't realistic for most things
    using bmp_type = bitmap<rgb_pixel<3> /*typename lcd_type::pixel_type*/>;
    using bmp_color = color<typename bmp_type::pixel_type>;
    const size16 bmp_size(64,64);
    uint8_t* bmp_buffer = (uint8_t*)malloc(bmp_type::sizeof_buffer(bmp_size));
    if(nullptr==bmp_buffer) {
        printf("out of memory\r\n");
        abort();
    }
 
    bmp_type bmp(bmp_size,bmp_buffer);
    bmp.clear(bmp.bounds());
   
    // bounding info for the face
    srect16 bounds=(srect16)bmp.bounds();
    rect16 ubounds=(rect16)bounds;

    // draw the face
    draw::filled_ellipse(bmp,bounds,bmp_color::yellow);
    
    // draw the left eye
    srect16 eye_bounds_left(spoint16(bounds.width()/5,
                            bounds.height()/5),
                            ssize16(bounds.width()/5,
                            bounds.height()/3));
    
    draw::filled_ellipse(bmp,eye_bounds_left,bmp_color::black);
    
    // draw the right eye
    srect16 eye_bounds_right(
        spoint16(
            64-eye_bounds_left.x1-eye_bounds_left.width(),
            eye_bounds_left.y1
        ),eye_bounds_left.dimensions());
    draw::filled_ellipse(bmp,eye_bounds_right,bmp_color::black);
    
    // draw the mouth
    srect16 mouth_bounds=bounds.inflate(-bounds.width()/7,
                                    -bounds.height()/8).normalize();
    // we need to clip part of the circle we'll be drawing
    srect16 mouth_clip(mouth_bounds.x1,
                    mouth_bounds.y1+mouth_bounds.height()/(float)1.6,
                    mouth_bounds.x2,
                    mouth_bounds.y2);
    draw::ellipse(bmp,mouth_bounds,bmp_color::black,&mouth_clip);

    // draw it centered horizontally 
    draw::bitmap(lcd,bounds.offset((lcd_type::width-bmp_size.width)/2,0),
                                bmp,
                                ubounds);
    
    const font& f = Bm437_ATI_8x16_FON;
    const char* text = "Have a nice day!";
    // center the text
    srect16 sr = (srect16)lcd.bounds().offset(0,bmp_size.height).crop(lcd.bounds());
    ssize16 tsiz = f.measure_text(sr.dimensions(),text);
    sr=sr.offset((sr.width()-tsiz.width)/2,0);
    // draw it
    draw::text(lcd,sr,text,f,lcd_color::antique_white);
    
    vTaskDelay(3000/portTICK_PERIOD_MS);

    // load an image
    io::file_stream fs("/spiffs/image3.jpg");
    if(!fs.caps().read) {
        printf("image file not found\r\n");
        abort();
    }
    gfx_result gr=jpeg_image::load(&fs,[](const jpeg_image::region_type& region,
                                    point16 location,
                                    void* state) {
        lcd_type* plcd = (lcd_type*)state;
        // we're not using it here, but we can do things like specify 
        // clipping here if we need it:
        return gfx_result::success==
            draw::bitmap(*plcd,
                        srect16((spoint16)location,
                        (ssize16)region.dimensions()),
                        region,region.bounds());
        // alternatively we could have just done this, 
        // which doesn't support clipping:
        // return gfx_result::success==plcd->write_frame(region.bounds(),
        //                                             region,
        //                                             location);
    },&lcd);
    if(gr!=gfx_result::success) {
        printf("image draw error %d\r\n",(int)gr);
    }
    vTaskDelay(portMAX_DELAY); 
}