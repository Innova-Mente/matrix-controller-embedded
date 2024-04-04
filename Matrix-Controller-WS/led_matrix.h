#ifndef __LED_MATRIX__
#define __LED_MATRIX__

#include <Adafruit_NeoPixel.h>

uint32_t  RGB(uint8_t r, uint8_t g, uint8_t b);

class LedMatrix {

  Adafruit_NeoPixel* pPixels;
  
public:

  LedMatrix();
	void clear();
	void setPixel(int x, int y, unsigned int color);
	void preparePixel(int x, int y, unsigned int color);
  void show();
	void fillColor(unsigned int color);

};

#endif