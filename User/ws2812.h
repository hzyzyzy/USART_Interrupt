/*
 * ws2812.h
 *
 *  Created on: 2024年7月7日
 *      Author: hp
 */

#ifndef USER_WS2812_H_
#define USER_WS2812_H_

#include <ch32v00x.h>



#define PIXEL_NUM 9



//硬件spi模拟ws2811时序（用spi的8位数据模拟ws281x的一位数据）

//  _____

// |     |___|   11110000  high level

//  ___

// |   |_____|   11000000  low level



#define WS_HIGH 0XF0

#define WS_LOW  0XC0



void ws281x_init(void);

void ws281x_closeAll(void);

void ws281x_rainbowCycle(uint8_t wait);

uint32_t ws281x_color(uint8_t red, uint8_t green, uint8_t blue);

void ws281x_setPixelColor(uint16_t n ,uint32_t GRBcolor);

void ws281x_show(void);



void ws281x_theaterChase(uint32_t c, uint8_t wait);

void ws281x_colorWipe(uint32_t c, uint8_t wait);

void ws281x_rainbow(uint8_t wait);

void ws281x_theaterChaseRainbow(uint8_t wait);


#endif /* USER_WS2812_H_ */
