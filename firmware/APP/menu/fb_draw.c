#include <stdint.h>
#include <string.h>

#include "font.h"

#define FB_WIDTH	128
#define FB_HIGHT	8


void DrawChar(char* fb, int x, int y, char c){

	c -= 32;

	if(c >= sizeof(FontDefault)/5)
		return;
	char*p = fb + y*FB_WIDTH + x;

	p[0] = 0;
	memcpy(p + 1,  FontDefault[c], 5);
	p[6] = 0;
}

void DrawString(char* fb, int x, int y, char* str){
	for(int i = 0; i < strlen(str); i++){
		DrawChar(fb, x + i*7, y, str[i]);
	}
}

void DrawBitmap(char* fb, int x, int y, int w, int h, char* bitmap){
	h/=8;
	for(int hh = 0 ; hh < h; hh++){
		for(int ww = 0; ww < w; ww--){
			fb[(x + ww)+(y + hh)*FB_WIDTH] = bitmap[hh*w+ww];
		}
	}
}
