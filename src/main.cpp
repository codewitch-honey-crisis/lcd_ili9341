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
#include "pretty_effect.hpp"
#include "gfx_drawing.hpp"
#include "gfx_image.hpp"
#include "stream.hpp"
#include "gfx_color_cpp14.hpp"
#include "../fonts/Bm437_Acer_VGA_8x8.h"
#include "../fonts/Bm437_ACM_VGA_9x16.h"
#include "../fonts/Bm437_ATI_9x16.h"
using namespace espidf;
using namespace io;
using namespace gfx;

// the following is configured for the ESP-WROVER-KIT
// make sure to set the pins to your set up.
#ifdef CONFIG_IDF_TARGET_ESP32
#if defined(ESP_WROVER_KIT)
#define LCD_ILI9341
#define PARALLEL_LINES 16
#define LCD_HOST    HSPI_HOST
#define DMA_CHAN    2
#define PIN_NUM_MISO GPIO_NUM_25
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_CLK  GPIO_NUM_19
#define PIN_NUM_CS   GPIO_NUM_22

#define PIN_NUM_DC   GPIO_NUM_21
#define PIN_NUM_RST  GPIO_NUM_18
#define PIN_NUM_BCKL GPIO_NUM_5
#elif defined(ESP32_TTGO)
#define LCD_ST7789
#define LCD_WIDTH 240
#define LCD_HEIGHT 135
#define PARALLEL_LINES 16
#define LCD_HOST    HSPI_HOST
#define DMA_CHAN    2
#define PIN_NUM_MISO GPIO_NUM_NC
#define PIN_NUM_MOSI GPIO_NUM_19
#define PIN_NUM_CLK  GPIO_NUM_18
#define PIN_NUM_CS   GPIO_NUM_5

#define PIN_NUM_DC   GPIO_NUM_16
#define PIN_NUM_RST  GPIO_NUM_NC
#define PIN_NUM_BCKL GPIO_NUM_4
#else
#define LCD_ILI9341
#define PARALLEL_LINES 16
#define LCD_HOST    HSPI_HOST
#define DMA_CHAN    2
#define PIN_NUM_MISO GPIO_NUM_25
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_CLK  GPIO_NUM_19
#define PIN_NUM_CS   GPIO_NUM_22

#define PIN_NUM_DC   GPIO_NUM_21
#define PIN_NUM_RST  GPIO_NUM_18
#define PIN_NUM_BCKL GPIO_NUM_5
#endif

#elif defined CONFIG_IDF_TARGET_ESP32S2
#define LCD_ILI9341
#define PARALLEL_LINES 16
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
#define LCD_ILI9341
#define PARALLEL_LINES 16
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

// configure the spi bus. Must be done before the driver
spi_master spi_host(nullptr,
                    LCD_HOST,
                    PIN_NUM_CLK,
                    PIN_NUM_MISO,
                    PIN_NUM_MOSI,
                    GPIO_NUM_NC,
                    GPIO_NUM_NC,
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
                        PIN_NUM_BCKL>;

lcd_type lcd;

//Simple routine to generate some patterns and send them to the LCD. Don't expect anything too
//impressive. Because the SPI driver handles transactions in the background, we can calculate the next line
//while the previous one is being sent.
static void display_pretty_colors()
{
    uint16_t *lines[2];
    //Allocate memory for the pixel buffers
    for (int i=0; i<2; i++) {
        lines[i]=(uint16_t*)heap_caps_malloc(320*PARALLEL_LINES*sizeof(uint16_t), MALLOC_CAP_DMA);
        assert(lines[i]!=NULL);
    }
    int frame=0;
    //Indexes of the line currently being sent to the LCD and the line we're calculating.
    int sending_line=-1;
    int calc_line=0;

    while(true) {
        ++frame;
        for (int y=0; y<240; y+=PARALLEL_LINES) {
            //Calculate a line.
            pretty_effect_calc_lines(lines[calc_line], y, frame, PARALLEL_LINES);
            //Finish up the sending process of the previous line, if any
            //if (sending_line!=-1) lcd.queued_wait();//send_line_finish(spi);
            //Swap sending_line and calc_line
            sending_line=calc_line;
            calc_line=(calc_line==1)?0:1;
            //Send the line we currently calculated.
            // queued_frame_write works better the larger the transfer size. Here ours is pretty big
            lcd.queued_frame_write(0,y,lcd_type::width-1,y+PARALLEL_LINES-1,(uint8_t*)lines[sending_line]);
            //The line set is queued up for sending now; the actual sending happens in the
            //background. We can go on to calculate the next line set as long as we do not
            //touch line[sending_line]; the SPI sending process is still reading from that.
        }
        if(0==frame%50) {
            int pid = (frame/50)%3;
            file_stream fs((0==pid)?"/spiffs/image.jpg":(1==pid)?"/spiffs/image2.jpg":"/spiffs/image3.jpg");
            gfx::jpeg_image::load(&fs,[](const typename gfx::jpeg_image::region_type& region,gfx::point16 location,void* state) {
                uint16_t** out = (uint16_t**)state;
                // to go as fast as possible, we access the bmp
                // as raw memory
                uint8_t *in = region.begin();
                gfx::rect16 r = region.bounds().offset(location.x,location.y);
                gfx::point16 pt;
                for (pt.y = r.y1; pt.y <= r.y2; ++pt.y) {
                    for (pt.x = r.x1; pt.x <= r.x2; ++pt.x) {
                        //We need to convert the 3 bytes in `in` to a rgb565 value.
                        // we could use pixel<>.convert<> and it's almost as efficient
                        // but it's actually more lines of code because we have to
                        // convert to and from raw values
                        // so we may as well just keep it raw
                        
                        uint16_t v = 0;
                        v |= ((in[0] >> 3) <<  11);
                        v |= ((in[1] >> 2) << 5);
                        v |= ((in[2] >> 3) );
                        //The LCD wants the 16-bit value in big-endian, so swap bytes
                        v=gfx::helpers::order_guard(v);
                        out[pt.y][pt.x] = v;
                        in+=3;
                    }
                }
                return true;

            },pixels);
        }
    }
}

using lcd_color = color<typename lcd_type::pixel_type>;
// produced by request
void scroll_text_demo() {
    lcd.clear(lcd.bounds());
    const font& f = Bm437_ATI_9x16_FON;
    const char* text = "Hello world!";
    srect16 text_rect = srect16(spoint16(0,0),f.measure_text((ssize16)lcd.dimensions(),text));
    while(true) {
        draw::filled_rectangle(lcd,text_rect,lcd_color::black);
        if(text_rect.x2>=320)
        {
            draw::filled_rectangle(lcd,text_rect.offset(-320,0),lcd_color::black);
        }

        text_rect=text_rect.offset(1,0);
        draw::text(lcd,text_rect,text,f,lcd_color::white,lcd_color::black,false);
       // draw::line(lcd,text_rect.inflate(-(text_rect.width()/2),-(text_rect.height()/2)),lcd_color::white);
        if(text_rect.x2>=320)
        {
            draw::text(lcd,text_rect.offset(-320,0),text,f,lcd_color::white,lcd_color::black,false);
        //    draw::line(lcd,text_rect.offset(-320,0).inflate(-(text_rect.width()/2),-(text_rect.height()/2)),lcd_color::white);
        }
        if(text_rect.x1>=320) {
            text_rect=text_rect.offset(-320,0);
        }
        
        //vTaskDelay(portMAX_DELAY);
    }
}
void lines_demo() {
    lcd.fill(lcd.bounds(),lcd_color::white);
    const font& f = Bm437_ATI_9x16_FON;
    const char* text = "ESP32 GFX Demo";
    srect16 text_rect = srect16(spoint16(0,0),f.measure_text((ssize16)lcd.dimensions(),text));
    text_rect=text_rect.offset((lcd_type::width-text_rect.width())/2,(lcd_type::height-text_rect.height())/2);
    draw::text(lcd,text_rect,text,f,lcd_color::dark_blue);

    for(int i = 1;i<100;++i) {
        draw::line(lcd,srect16(0,i*(lcd_type::height/100.0),i*(lcd_type::width/100.0),lcd_type::height-1),lcd_color::light_blue);
        draw::line(lcd,srect16(lcd_type::width-i*(lcd_type::width/100.0)-1,0,lcd_type::width-1,lcd_type::height-i*(lcd_type::height/100.0)-1),lcd_color::hot_pink);
        draw::line(lcd,srect16(0,lcd_type::height-i*(lcd_type::height/100.0),i*(lcd_type::width/100.0)-1,0),lcd_color::pale_green);
        draw::line(lcd,srect16(lcd_type::width-1,i*(lcd_type::height/100.0),lcd_type::width-i*(lcd_type::width/100.0)-1,lcd_type::height-1),lcd_color::yellow);
        
    }
    //vTaskDelay(3000/portTICK_PERIOD_MS);
}
void app_main(void)
{
    // check to make sure SPI was initialized successfully
    if(!spi_host.initialized()) {
        printf("SPI host initialization error.\r\n");
        abort();
    }
    lines_demo();
    // mount SPIFFS
    esp_err_t ret;
    esp_vfs_spiffs_conf_t conf = {};
    conf.base_path="/spiffs";
    conf.format_if_mount_failed=false;
    conf.max_files=5;
    conf.partition_label="storage";
    ret=esp_vfs_spiffs_register(&conf);
    ESP_ERROR_CHECK(ret);   
    gfx_result rr;
    rr=pretty_effect_init();
    if(gfx_result::success!=rr) {
        printf("Error loading demo: %d\r\n",(int)rr);
    }
    display_pretty_colors();
}