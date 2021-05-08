extern "C" { void app_main(); }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "i2c_master.hpp"
#include "esp_spiffs.h"
#include "ssd1306.hpp"
#include "gfx_drawing.hpp"
#include "gfx_image.hpp"
#include "gfx_drawing.hpp"
#include "stream.hpp"
#include "gfx_color_cpp14.hpp"
#include "../fonts/Bm437_Acer_VGA_8x8.h"
#include "../fonts/Bm437_ACM_VGA_9x16.h"
#include "../fonts/Bm437_ATI_9x16.h"
using namespace espidf;
using namespace io;
using namespace gfx;

// ensure the following is configured for your setup
#define LCD_PORT I2C_NUM_0
#define PIN_SDA GPIO_NUM_21
#define PIN_SCL GPIO_NUM_22
#define PIN_RST GPIO_NUM_NC
#define I2C_FREQ 400000
#define LCD_WIDTH 128
#define LCD_HEIGHT 64
i2c_master i2c(nullptr,LCD_PORT,PIN_SDA,PIN_SCL,true,true,I2C_FREQ);

#if defined(LCD_VDC_5)
#define LCD_VDC_3_3 false
#else
#define LCD_VDC_3_3 true
#endif

// we use the default, modest buffer - it makes things slower but uses less
// memory. it usually works fine at default but you can change it for performance 
// tuning. It's the final parameter: Note that it shouldn't be any bigger than 
// the DMA size
using lcd_type = ssd1306_i2c<LCD_WIDTH,LCD_HEIGHT,LCD_PORT,0x3C,LCD_VDC_3_3,PIN_RST>;

lcd_type lcd;

using lcd_color = color<typename lcd_type::pixel_type>;
// produced by request
void scroll_text_demo() {
    lcd.clear(lcd.bounds());
    const font& f = Bm437_ATI_9x16_FON;
    const char* text = "copyright (C) 2021\r\nby honey the codewitch";
    ssize16 text_size = f.measure_text((ssize16)lcd.dimensions(),text);
    srect16 text_rect = srect16(spoint16((lcd.dimensions().width-text_size.width)/2,(lcd.dimensions().height-text_size.height)/2),text_size);
    int16_t text_start = text_rect.x1;
    bool first=true;
    while(true) {
        draw::filled_rectangle(lcd,text_rect,lcd_color::black);
        if(text_rect.x2>=320) {
            draw::filled_rectangle(lcd,text_rect.offset(-lcd.dimensions().width,0),lcd_color::black);
        }

        text_rect=text_rect.offset(2,0);
        draw::text(lcd,text_rect,text,f,lcd_color::old_lace,lcd_color::black,false);
        if(text_rect.x2>=lcd.dimensions().width){
            draw::text(lcd,text_rect.offset(-lcd.dimensions().width,0),text,f,lcd_color::old_lace,lcd_color::black,false);
        }
        if(text_rect.x1>=lcd.dimensions().width) {
            text_rect=text_rect.offset(-lcd.dimensions().width,0);
            first=false;
        }
        
        if(!first && text_rect.x1>=text_start)
            break;
        vTaskDelay(1);
    }
}
void lines_demo() {
    draw::filled_rectangle(lcd,(srect16)lcd.bounds(),lcd_color::white);
    const font& f = Bm437_ATI_9x16_FON;
    const char* text = "ESP32 GFX Demo";
    srect16 text_rect = srect16(spoint16(0,0),f.measure_text((ssize16)lcd.dimensions(),text));
    text_rect=text_rect.offset((lcd.dimensions().width-text_rect.width())/2,(lcd.dimensions().height-text_rect.height())/2);
    draw::text(lcd,text_rect,text,f,lcd_color::dark_blue);

    for(int i = 1;i<100;++i) {
        draw::line(lcd,srect16(0,i*(lcd.dimensions().height/100.0),i*(lcd.dimensions().width/100.0),lcd.dimensions().height-1),lcd_color::light_blue);
        draw::line(lcd,srect16(lcd.dimensions().width-i*(lcd.dimensions().width/100.0)-1,0,lcd.dimensions().width-1,lcd.dimensions().height-i*(lcd.dimensions().height/100.0)-1),lcd_color::hot_pink);
        draw::line(lcd,srect16(0,lcd.dimensions().height-i*(lcd.dimensions().height/100.0),i*(lcd.dimensions().width/100.0)-1,0),lcd_color::pale_green);
        draw::line(lcd,srect16(lcd.dimensions().width-1,i*(lcd.dimensions().height/100.0),lcd.dimensions().width-i*(lcd.dimensions().width/100.0)-1,lcd.dimensions().height-1),lcd_color::yellow);
        vTaskDelay(1);
    }
    //vTaskDelay(3000/portTICK_PERIOD_MS);
}

void app_main(void)
{
    // check to make sure SPI was initialized successfully
    if(!i2c.initialized()) {
        printf("I2C host initialization error.\r\n");
        vTaskDelay(portMAX_DELAY);
    }
    lcd_type::result r= lcd.initialize();
    if(lcd_type::result::success!=r) {
        printf("display initialization error.\r\n");
        vTaskDelay(portMAX_DELAY);
    }
    while(true) {
        lines_demo();
        scroll_text_demo();
    }
}