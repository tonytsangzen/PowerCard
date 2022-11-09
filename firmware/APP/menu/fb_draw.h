#ifndef __FB_DRAW_H__
#define __FB_DRAW_H__

void DrawChar(char* fb, int x, int y, char c);

void DrawString(char* fb, int x, int y, char* str);

void DrawBitmap(char* fb, int x, int y, int w, int h, char* bitmap);


#endif
