  
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
#include "gfx_color_cpp14.hpp"
#include "../fonts/Bm437_Acer_VGA_8x8.h"
using namespace espidf;
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
    }
}

void scroll_text_demo() {
    using lcd_color = color<typename lcd_type::pixel_type>;
    lcd.clear(lcd.bounds());
    const font& f = Bm437_Acer_VGA_8x8_FON;
    const char* text = "Hello world!";
    srect16 text_rect = srect16(spoint16(0,0),f.measure_text((ssize16)lcd.dimensions(),text));
    while(true) {
        draw::filled_rectangle(lcd,text_rect,lcd_color::black);
        if(text_rect.x2>=320)
        {
            draw::filled_rectangle(lcd,text_rect.offset(-320,0),lcd_color::black);
        }

        text_rect=text_rect.offset(1,0);
        draw::text(lcd,text_rect,text,f,lcd_color::white);
        if(text_rect.x2>=320)
        {
            draw::text(lcd,text_rect.offset(-320,0),text,f,lcd_color::white);
        }
        if(text_rect.x1>=320) {
            text_rect=text_rect.offset(-320,0);
        }
    }

}
void app_main(void)
{
    // check to make sure SPI was initialized successfully
    if(!spi_host.initialized()) {
        printf("SPI host initialization error.\r\n");
        abort();
    }
    //scroll_text_demo();
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