#include "led_matrix.h"

#define PIN_WS2812B  D1  
#define NUM_PIXELS   64  

uint32_t  RGB(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

LedMatrix::LedMatrix(){
  pPixels = new Adafruit_NeoPixel(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);
  pPixels->begin();
}

void LedMatrix::clear(){
  pPixels->clear(); 
  pPixels->show();
}
	
void LedMatrix::setPixel(int x, int y, unsigned int color){
  pPixels->setPixelColor(y*8+x, color); // it only takes effect if pixels.show() is called
  pPixels->show();
}

void LedMatrix::preparePixel(int x, int y, unsigned int color){
  pPixels->setPixelColor(y*8+x, color); // it only takes effect if pixels.show() is called
}

void LedMatrix::show(){
  pPixels->show();
}

void LedMatrix::fillColor(unsigned int color){
  pPixels->fill(color);
  pPixels->show();
}
