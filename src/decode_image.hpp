/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once
#include <stdint.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "tjpgd.hpp"

//Define the height and width of the jpeg file. Make sure this matches the actual jpeg
//dimensions.
#define IMAGE_W 336
#define IMAGE_H 256

const char *TAG = "ImageDec";

//Data that is passed from the decoder function to the infunc/outfunc functions.
typedef struct {
    FILE* fd;
    const unsigned char *inData; //Pointer to jpeg data
    uint16_t inPos;              //Current position in jpeg data
    uint16_t **outData;          //Array of IMAGE_H pointers to arrays of IMAGE_W 16-bit pixel values
    int outW;                    //Width of the resulting file
    int outH;                    //Height of the resulting file
} JpegDev;

//Input function for jpeg decoder. Just returns bytes from the inData field of the JpegDev structure.
static unsigned int infunc(tjpgd::JDEC *decoder, uint8_t *buf, unsigned int len)
{
    //Read bytes from input file
    JpegDev *jd = (JpegDev *)decoder->device;
    if (buf != NULL) {
        fseek(jd->fd,jd->inPos,SEEK_SET);
        fread(buf,1,len,jd->fd);
    }
    jd->inPos += len;
    return len;
}

//Output function. Re-encodes the RGB888 data from the decoder as big-endian RGB565 and
//stores it in the outData array of the JpegDev structure.
static int outfunc(tjpgd::JDEC *decoder, void *bitmap, tjpgd::JRECT *rect)
{
    JpegDev *jd = (JpegDev *)decoder->device;
    uint8_t *in = (uint8_t *)bitmap;
    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {
            //We need to convert the 3 bytes in `in` to a rgb565 value.
            uint16_t v = 0;
            v |= ((in[0] >> 3) << 11);
            v |= ((in[1] >> 2) << 5);
            v |= ((in[2] >> 3) << 0);
            //The LCD wants the 16-bit value in big-endian, so swap bytes
            v = (v >> 8) | (v << 8);
            jd->outData[y][x] = v;
            in += 3;
        }
    }
    return 1;
}

//Size of the work space for the jpeg decoder.
#define WORKSZ 3100

//Decode the embedded image into pixel lines that can be used with the rest of the logic.
/**
 * @brief Decode the jpeg ``image.jpg`` embedded into the program file into pixel data.
 *
 * @param pixels A pointer to a pointer for an array of rows, which themselves are an array of pixels.
 *        Effectively, you can get the pixel data by doing ``decode_image(&myPixels); pixelval=myPixels[ypos][xpos];``
 * @return - ESP_ERR_NOT_SUPPORTED if image is malformed or a progressive jpeg file
 *         - ESP_ERR_NO_MEM if out of memory
 *         - ESP_OK on succesful decode
 */
esp_err_t decode_image(uint16_t ***pixels)
{
    char *work = NULL;
    int r;
    tjpgd::JDEC decoder;
    JpegDev jd;
    *pixels = NULL;
    esp_err_t ret = ESP_OK;

    //Alocate pixel memory. Each line is an array of IMAGE_W 16-bit pixels; the `*pixels` array itself contains pointers to these lines.
    *pixels = (uint16_t**)calloc(IMAGE_H, sizeof(uint16_t *));
    if (*pixels == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for lines");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }
    for (int i = 0; i < IMAGE_H; i++) {
        (*pixels)[i] = (uint16_t*)malloc(IMAGE_W * sizeof(uint16_t));
        if ((*pixels)[i] == NULL) {
            ESP_LOGE(TAG, "Error allocating memory for line %d", i);
            ret = ESP_ERR_NO_MEM;
            goto err;
        }
    }

    //Allocate the work space for the jpeg decoder.
    work = (char*)calloc(WORKSZ, 1);
    if (work == NULL) {
        ESP_LOGE(TAG, "Cannot allocate workspace");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }

    //Populate fields of the JpegDev struct.
    jd.fd = fopen("/spiffs/image.jpg","r");
    assert(jd.fd!=NULL);
    jd.inData = NULL;
    jd.inPos = 0;
    jd.outData = *pixels;
    jd.outW = IMAGE_W;
    jd.outH = IMAGE_H;

    //Prepare and decode the jpeg.
    r = tjpgd::jd_prepare(&decoder, infunc, work, WORKSZ, (void *)&jd);
    if (r != tjpgd::JDR_OK) {
        ESP_LOGE(TAG, "Image decoder: jd_prepare failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }
    r = tjpgd::jd_decomp(&decoder, outfunc, 0);
    if (r != tjpgd::JDR_OK && r != tjpgd::JDR_FMT1) {
        ESP_LOGE(TAG, "Image decoder: jd_decode failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }

    //All done! Free the work area (as we don't need it anymore) and return victoriously.
    free(work);
    return ret;
err:
    //Something went wrong! Exit cleanly, de-allocating everything we allocated.
    if (*pixels != NULL) {
        for (int i = 0; i < IMAGE_H; i++) {
            free((*pixels)[i]);
        }
        free(*pixels);
    }
    free(work);
    return ret;
}