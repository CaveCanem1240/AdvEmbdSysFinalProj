#include <stdint.h>
#include "LCD.h"
#include "os.h"
#include "tm4c123gh6pm.h"
#include "screen.h"
#include "font.h"

#define IMGW 128
#define IMGH 64
#define IMGSIZE IMGW*IMGH
static uint16_t Img[IMGSIZE];

uint32_t Bitmap_StX=0; // position along the horizonal axis 0 to 20
uint32_t Bitmap_StY=0; // position along the vertical axis 0 to 11

uint8_t HalfScreen = 0; //0: upper, 1: bottom

uint16_t *Map_Image;

void Bitmap_init(uint16_t *img)
{
	uint32_t i = 0;
	while(i < IMGSIZE){
			*img = 0x0000;
			img++;
			i++;
	}
	img-=IMGSIZE;
}



void act_FuncInStack(StackDataType *DispStackDatapt){
	if(DispStackDatapt->LCD_operation ==  &BSP_LCD_DrawPixel || DispStackDatapt->LCD_operation == &BitmapSetPixel){
		BitmapSetPixel(DispStackDatapt->x, DispStackDatapt->y, DispStackDatapt->Color);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_FillRect || DispStackDatapt->LCD_operation == &Bitmap_FillRect){
		Bitmap_FillRect(DispStackDatapt->x, DispStackDatapt->y, DispStackDatapt->Color, DispStackDatapt->w, DispStackDatapt->h);
	}
	else if(DispStackDatapt->LCD_operation == &Bitmap_DrawBox){
		Bitmap_DrawBox(DispStackDatapt->x, DispStackDatapt->y, DispStackDatapt->Color, DispStackDatapt->w, DispStackDatapt->h);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_FillScreen || DispStackDatapt->LCD_operation == &Bitmap_FillScreen){
		Bitmap_FillScreen(DispStackDatapt->Color);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_DrawFastVLine || DispStackDatapt->LCD_operation == &Bitmap_DrawVLine){
		Bitmap_DrawVLine(DispStackDatapt->x, DispStackDatapt->y, DispStackDatapt->h, DispStackDatapt->Color);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_DrawFastHLine || DispStackDatapt->LCD_operation == &Bitmap_DrawHLine){
		Bitmap_DrawHLine(DispStackDatapt->x, DispStackDatapt->y, DispStackDatapt->w, DispStackDatapt->Color);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_DrawBitmap || DispStackDatapt->LCD_operation == &Bitmap_DrawBitmap){
		Bitmap_DrawBitmap(DispStackDatapt->x, DispStackDatapt->y, DispStackDatapt->image, DispStackDatapt->w, DispStackDatapt->h);
	}
	else if(DispStackDatapt->LCD_operation ==  &Bitmap_DrawBitmapTP){
		Bitmap_DrawBitmapTP(DispStackDatapt->x, DispStackDatapt->y, DispStackDatapt->image, DispStackDatapt->w, DispStackDatapt->h);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_DrawCharS || DispStackDatapt->LCD_operation == &Bitmap_DrawCharS || DispStackDatapt->LCD_operation == &BSP_LCD_DrawChar){
		Bitmap_DrawCharS(DispStackDatapt->x, DispStackDatapt->y, *(DispStackDatapt->string), DispStackDatapt->Color, DispStackDatapt->size);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_DrawString || DispStackDatapt->LCD_operation == &Bitmap_DrawString){
		Bitmap_DrawString(DispStackDatapt->x, DispStackDatapt->y,  DispStackDatapt->string, DispStackDatapt->Color);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_DrawStringHighlight || DispStackDatapt->LCD_operation == &Bitmap_DrawStringHighlight){
		Bitmap_DrawStringHighlight(DispStackDatapt->x, DispStackDatapt->y, DispStackDatapt->string, DispStackDatapt->Color);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_Message || DispStackDatapt->LCD_operation == &Bitmap_MessageUDec4){
		Bitmap_MessageUDec4(DispStackDatapt->device, DispStackDatapt->y, DispStackDatapt->x, DispStackDatapt->string, DispStackDatapt->value);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_MessageUDec3 || DispStackDatapt->LCD_operation == &Bitmap_MessageUDec3){
		Bitmap_MessageUDec3(DispStackDatapt->device, DispStackDatapt->y, DispStackDatapt->x, DispStackDatapt->string, DispStackDatapt->value);
	}
	else if(DispStackDatapt->LCD_operation ==  &BSP_LCD_DrawCrosshair || DispStackDatapt->LCD_operation == &Bitmap_DrawCrosshair){
		Bitmap_DrawCrosshair(DispStackDatapt->x, DispStackDatapt->y, DispStackDatapt->Color);
	}
}


void BitmapSetPixel(int16_t x, int16_t y, uint16_t Color){
//	uint16_t *Map_Image;
	uint16_t offset;
	Map_Image = &Img[0];
	if(HalfScreen==0 && y<64){
		offset = x + 128*(63-y);
		*(Map_Image+offset) = Color;
	}
	else if(HalfScreen==1 && y>63){
		offset = x + 128*(127-y);
		*(Map_Image+offset) = Color;
	}
}
//------------Bitmap_FillRect------------
// Draw a filled rectangle at the given coordinates with the given width, height, and color.
// Requires (11 + 2*w*h) bytes of transmission (assuming image fully on screen)
// Input: x     horizontal position of the top left corner of the rectangle, columns from the left edge
//        y     vertical position of the top left corner of the rectangle, rows from the top edge
//        w     horizontal width of the rectangle
//        h     vertical height of the rectangle
//        color 16-bit color, which can be produced by BSP_LCD_Color565()
// Output: none
void Bitmap_FillRect(int16_t x, int16_t y, uint16_t color, int16_t w, int16_t h) {
//	uint16_t *Map_Image;
	int32_t offset;
	Map_Image = &Img[0];
	int16_t startx,starty,endx,endy;

	y = y + h - 1;//
	startx = x;
	starty = y;
	endx = x + w - 1;
	endy = y - h + 1;

	if(startx > 127) return;
	if(endx < 0) return;
	if(w > 128 || h > 128) return;
	if(HalfScreen==0){
		if(starty < 0) return;
		if(endy > 63) return;
		if(endx > 127){
			endx = 127;
		}
		if(endy < 0){
			endy = 0;
		}
		if(startx < 0){
			startx = 0;
		}
		if(starty > 63){
			starty = 63;
		}
		uint8_t pixX,pixY;
		for(pixY=endy; pixY<=starty; pixY++){
			for(pixX=startx; pixX<=endx; pixX++){
				offset = pixX + 128*(63-pixY);
				*(Map_Image+offset) = color;
			}
		}
	}
	else{
		if(starty < 64) return;
		if(endy > 127) return;
		if(endx > 127){
			endx = 127;
		}
		if(endy < 64){
			endy = 64;
		}
		if(startx < 0){
			startx = 0;
		}
		if(starty > 127){
			starty = 127;
		}
		uint8_t pixX,pixY;
		for(pixY=endy; pixY<=starty; pixY++){
			for(pixX=startx; pixX<=endx; pixX++){
				offset = pixX + 128*(127-pixY);
				*(Map_Image+offset) = color;
			}
		}
	}
}

//------------Bitmap_FillScreen------------
// Fill the screen with the given color.
// Requires 33,293 bytes of transmission
// Input: color 16-bit color, which can be produced by BSP_LCD_Color565()
// Output: none
void Bitmap_FillScreen(uint16_t color) {
  Bitmap_FillRect(0, 0, color, 128, 128);  // original
//  screen is actually 129 by 129 pixels, x 0 to 128, y goes from 0 to 128
}

//------------Bitmap_DrawVLine------------
// Draw a vertical line at the given coordinates with the given height and color.
// A vertical line is parallel to the longer side of the rectangular display
// Requires (11 + 2*h) bytes of transmission (assuming image fully on screen)
// Input: x     horizontal position of the start of the line, columns from the left edge
//        y     vertical position of the start of the line, rows from the top edge
//        h     vertical height of the line
//        color 16-bit color, which can be produced by BSP_LCD_Color565()
// Output: none
void Bitmap_DrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	Bitmap_FillRect(x, y, color, 1, h);
}

void Bitmap_DrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
	Bitmap_FillRect(x, y, color, w, 1);
}

//------------Bitmap_DrawBitmap------------
// Displays a 16-bit color BMP image.  A bitmap file that is created
// by a PC image processing program has a header and may be padded
// with dummy columns so the data have four byte alignment.  This
// function assumes that all of that has been stripped out, and the
// array image[] has one 16-bit halfword for each pixel to be
// displayed on the screen (encoded in reverse order, which is
// standard for bitmap files).  An array can be created in this
// format from a 24-bit-per-pixel .bmp file using the associated
// converter program.
// (x,y) is the screen location of the lower left corner of BMP image
// Requires (11 + 2*w*h) bytes of transmission (assuming image fully on screen)
// Input: x     horizontal position of the bottom left corner of the image, columns from the left edge
//        y     vertical position of the bottom left corner of the image, rows from the top edge
//        image pointer to a 16-bit color BMP image
//        w     number of pixels wide
//        h     number of pixels tall
// Output: none
// Must be less than or equal to 128 pixels wide by 128 pixels high
void Bitmap_DrawBitmap(int16_t x, int16_t y, const uint16_t *inputImage, int16_t w, int16_t h){
//	uint16_t *Map_Image;
	int32_t offset,inputoffset;
	Map_Image = &Img[0];
	int16_t startx,starty,endx,endy;

	startx = x;
	starty = y;
	endx = x + w - 1;
	endy = y - h + 1;

	if(startx > 127) return;
	if(endx < 0) return;
	if(w > 128 || h > 128) return;
	if(HalfScreen==0){
		if(starty < 0) return;
		if(endy > 63) return;
		if(endx > 127){
			endx = 127;
		}
		if(endy < 0){
			endy = 0;
		}
		if(startx < 0){
			startx = 0;
		}
		if(starty > 63){
			starty = 63;
		}
		uint8_t pixX,pixY;
		for(pixY=endy; pixY<=starty; pixY++){
			for(pixX=startx; pixX<=endx; pixX++){
				offset = pixX + 128*(63-pixY);
				inputoffset = pixX-x + w*(y-pixY);
				*(Map_Image+offset) = *(inputImage+inputoffset);
			}
		}
	}
	else{
		if(starty < 64) return;
		if(endy > 127) return;
		if(endx > 127){
			endx = 127;
		}
		if(endy < 64){
			endy = 64;
		}
		if(startx < 0){
			startx = 0;
		}
		if(starty > 127){
			starty = 127;
		}
		uint8_t pixX,pixY;
		for(pixY=endy; pixY<=starty; pixY++){
			for(pixX=startx; pixX<=endx; pixX++){
				offset = pixX + 128*(127-pixY);
				inputoffset = pixX-x + w*(y-pixY);
				*(Map_Image+offset) = *(inputImage+inputoffset);
			}
		}
	}
}

void Bitmap_DrawBitmapTP(int16_t x, int16_t y, const uint16_t *inputImage, int16_t w, int16_t h){
//	uint16_t *Map_Image;
	int32_t offset,inputoffset;
	Map_Image = &Img[0];
	int16_t startx,starty,endx,endy;

	startx = x;
	starty = y;
	endx = x + w - 1;
	endy = y - h + 1;

	if(startx > 127) return;
	if(endx < 0) return;
	if(w > 128 || h > 128) return;
	if(HalfScreen==0){
		if(starty < 0) return;
		if(endy > 63) return;
		if(endx > 127){
			endx = 127;
		}
		if(endy < 0){
			endy = 0;
		}
		if(startx < 0){
			startx = 0;
		}
		if(starty > 63){
			starty = 63;
		}
		uint8_t pixX,pixY;
		for(pixY=endy; pixY<=starty; pixY++){
			for(pixX=startx; pixX<=endx; pixX++){
				offset = pixX + 128*(63-pixY);
				inputoffset = pixX-x + w*(y-pixY);
				if(*(inputImage+inputoffset)!=0xFFFF){
					*(Map_Image+offset) = *(inputImage+inputoffset);
				}
			}
		}
	}
	else{
		if(starty < 64) return;
		if(endy > 127) return;
		if(endx > 127){
			endx = 127;
		}
		if(endy < 64){
			endy = 64;
		}
		if(startx < 0){
			startx = 0;
		}
		if(starty > 127){
			starty = 127;
		}
		uint8_t pixX,pixY;
		for(pixY=endy; pixY<=starty; pixY++){
			for(pixX=startx; pixX<=endx; pixX++){
				offset = pixX + 128*(127-pixY);
				inputoffset = pixX-x + w*(y-pixY);
				if(*(inputImage+inputoffset)!=0xFFFF){
					*(Map_Image+offset) = *(inputImage+inputoffset);
				}
			}
		}
	}
}

//------------Bitmap_DrawCharS------------
// Simple character draw function.  This is the same function from
// Adafruit_GFX.c but adapted for this processor.  However, each call
// to BSP_LCD_DrawPixel() calls setAddrWindow(), which needs to send
// many extra data and commands.  If the background color is the same
// as the text color, no background will be printed, and text can be
// drawn right over existing images without covering them with a box.
// Requires (11 + 2*size*size)*6*8 bytes of transmission (image fully on screen; textcolor != bgColor)
// Input: x         horizontal position of the top left corner of the character, columns from the left edge
//        y         vertical position of the top left corner of the character, rows from the top edge
//        c         character to be printed
//        textColor 16-bit color of the character
//        bgColor   16-bit color of the background
//        size      number of pixels per character pixel (e.g. size==2 prints each pixel of font as 2x2 square)
// Output: none
void Bitmap_DrawCharS(int16_t x, int16_t y, char c, int16_t textColor, uint8_t size){
  uint8_t line; // vertical column of pixels of character in font
  int32_t i, j;
  if((x >= 128)            || // Clip right
     (y >= 128)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

  for (i=0; i<6; i++ ) {
    if (i == 5)
      line = 0x0;
    else
      line = Font[(c*5)+i];
    for (j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          BitmapSetPixel(x+i, y+j, textColor);
        else {  // big size
          Bitmap_FillRect(x+(i*size), y+(j*size), textColor, size, size);
        }
      }
      line >>= 1;
    }
  }
}


void Bitmap_DrawCharS_Background(int16_t x, int16_t y, char c, int16_t textColor, int16_t bgColor, uint8_t size){
  uint8_t line; // vertical column of pixels of character in font
  int32_t i, j;
  if((x >= 128)            || // Clip right
     (y >= 128)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

  for (i=0; i<6; i++ ) {
    if (i == 5)
      line = 0x0;
    else
      line = Font[(c*5)+i];
    for (j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          BitmapSetPixel(x+i, y+j, textColor);
        else {  // big size
          Bitmap_FillRect(x+(i*size), y+(j*size), textColor, size, size);
        }
      } else if (bgColor != textColor) {
        if (size == 1) // default size
          BitmapSetPixel(x+i, y+j, bgColor);
        else {  // big size
          Bitmap_FillRect(x+i*size, y+j*size, bgColor, size, size);
        }
      }
      line >>= 1;
    }
  }
}
//------------Bitmap_DrawString------------
// String draw function.
// 13 rows (0 to 12) and 21 characters (0 to 20)
// Requires (11 + size*size*6*8) bytes of transmission for each character
// Input: x         columns from the left edge (0 to 20)
//        y         rows from the top edge (0 to 12)
//        pt        pointer to a null terminated string to be printed
//        textColor 16-bit color of the characters
// bgColor is Black and size is 1
// Output: number of characters printed
uint32_t Bitmap_DrawString(int16_t x, int16_t y, char *pt, int16_t textColor){
  uint32_t count = 0;
  if(y>12) return 0;
  while(*pt){
    Bitmap_DrawCharS(x*6, y*10, *pt, textColor, 1);
    pt++;
    x = x+1;
    if(x>20) return count;  // number of characters printed
    count++;
  }
  return count;  // number of characters printed
}
uint32_t Bitmap_DrawString_Background(int16_t x, int16_t y, char *pt, int16_t textColor,int16_t bgColor){
  uint32_t count = 0;
  if(y>12) return 0;
  while(*pt){
    Bitmap_DrawCharS_Background(x*6, y*10, *pt, textColor, bgColor, 1);
    pt++;
    x = x+1;
    if(x>20) return count;  // number of characters printed
    count++;
  }
  return count;  // number of characters printed
}
uint32_t Bitmap_DrawStringHighlight(int16_t x, int16_t y, char *pt, int16_t textColor){
  uint32_t count = 0;
  if(y>12) return 0;
  while(*pt){
    Bitmap_DrawCharS_Background(x*6, y*10, *pt, textColor, 0xFFFF, 1);
    pt++;
    x = x+1;
    if(x>20) return count;  // number of characters printed
    count++;
  }
  return count;  // number of characters printed
}
//-----------------------Bitmap_fillmessage-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
char Bitmap_Message[12];
uint32_t Bitmap_Messageindex;

void static Bitmap_fillmessage(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    Bitmap_fillmessage(n/10);
    n = n%10;
  }
  Bitmap_Message[Bitmap_Messageindex] = (n+'0'); /* n is between 0 and 9 */
  if(Bitmap_Messageindex<11)Bitmap_Messageindex++;
}
void static Bitmap_fillmessage3(uint32_t n){
  if(n>999)n=999;
  if(n>=100){  // 100 to 999
    Bitmap_Messageindex = 0;
  } else if(n>=10){  // 10 to 99
    Bitmap_Message[0] = ' ';
    Bitmap_Messageindex = 1;
  }else{ //
    Bitmap_Message[0] = ' '; /* n is between 1 and 9 */
    Bitmap_Message[1] = ' ';
    Bitmap_Messageindex = 2;
  }
  Bitmap_fillmessage(n);
}
void static Bitmap_fillmessage4(uint32_t n){
  if(n>9999)n=9999;
  if(n>=1000){  // 1000 to 9999
    Bitmap_Messageindex = 0;
  } else if(n>=100){  // 100 to 999
    Bitmap_Message[0] = ' ';
    Bitmap_Messageindex = 1;
  }else if(n>=10){ //
    Bitmap_Message[0] = ' '; /* n is between 10 and 99 */
    Bitmap_Message[1] = ' ';
    Bitmap_Messageindex = 2;
  }else{
    Bitmap_Message[0] = ' '; /* n is between 0 and 9 */
    Bitmap_Message[1] = ' ';
    Bitmap_Message[2] = ' ';
    Bitmap_Messageindex = 3;
  }
  Bitmap_fillmessage(n);
}
void static Bitmap_fillmessage5(uint32_t n){
  if(n>99999)n=99999;
  if(n>=10000){  // 10000 to 99999
    Bitmap_Messageindex = 0;
  } else if(n>=1000){  // 1000 to 9999
    Bitmap_Message[0] = ' ';
    Bitmap_Messageindex = 1;
  }else if(n>=100){  // 100 to 999
    Bitmap_Message[0] = ' ';
    Bitmap_Message[1] = ' ';
    Bitmap_Messageindex = 2;
  }else if(n>=10){ //
    Bitmap_Message[0] = ' '; /* n is between 10 and 99 */
    Bitmap_Message[1] = ' ';
    Bitmap_Message[2] = ' ';
    Bitmap_Messageindex = 3;
  }else{
    Bitmap_Message[0] = ' '; /* n is between 0 and 9 */
    Bitmap_Message[1] = ' ';
    Bitmap_Message[2] = ' ';
    Bitmap_Message[3] = ' ';
    Bitmap_Messageindex = 4;
  }
  Bitmap_fillmessage(n);
}
void static Bitmap_fillmessage2_1(uint32_t n){
  if(n>999)n=999;
  if(n>=100){  // 100 to 999
    Bitmap_Message[0] = (n/100+'0'); /* tens digit */
    n = n%100; //the rest
  }else { // 0 to 99
    Bitmap_Message[0] = ' '; /* n is between 0.0 and 9.9 */
  }
  Bitmap_Message[1] = (n/10+'0'); /* ones digit */
  n = n%10; //the rest
  Bitmap_Message[2] = '.';
  Bitmap_Message[3] = (n+'0'); /* tenths digit */
  Bitmap_Message[4] = 0;
}
void static Bitmap_fillmessage2_Hex(uint32_t n){ char digit;
  if(n>255){
    Bitmap_Message[0] = '*';
    Bitmap_Message[1] = '*';
  }else{
    digit = n/16;
    if(digit<10){
      digit = digit+'0';
    }else{
      digit = digit+'A'-10;
    }
    Bitmap_Message[0] = digit; /* 16's digit */
    digit = n%16;
    if(digit<10){
      digit = digit+'0';
    }else{
      digit = digit+'A'-10;
    }
    Bitmap_Message[1] = digit; /* ones digit */
  }
  Bitmap_Message[2] = ',';
  Bitmap_Message[3] = 0;
}

//********Bitmap_SetCursor*****************
// Move the cursor to the desired X- and Y-position.  The
// next character of the next unsigned decimal will be
// printed here.  X=0 is the leftmost column.  Y=0 is the top
// row.
// inputs: newX  new X-position of the cursor (0<=newX<=20)
//         newY  new Y-position of the cursor (0<=newY<=12)
// outputs: none
void Bitmap_SetCursor(uint32_t newX, uint32_t newY){
  if((newX > 20) || (newY > 12)){       // bad input
    return;                             // do nothing
  }
  Bitmap_StX = newX;
  Bitmap_StY = newY;
}


//-----------------------Bitmap_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Position determined by BSP_LCD_SetCursor command
// Input: n         32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Variable format 1-10 digits with no space before or after
void Bitmap_OutUDec(uint32_t n, int16_t textColor){
  Bitmap_Messageindex = 0;
  Bitmap_fillmessage(n);
  Bitmap_Message[Bitmap_Messageindex] = 0; // terminate
  Bitmap_DrawString(Bitmap_StX,Bitmap_StY, Bitmap_Message,textColor);
  Bitmap_StX = Bitmap_StX+Bitmap_Messageindex;
  if(Bitmap_StX>20){
    Bitmap_StX = 20;
    Bitmap_DrawCharS(Bitmap_StX*6,Bitmap_StY*10,'*',LCD_RED, 1);
  }
}


//-----------------------Bitmap_OutUDec3-----------------------
// Output a 32-bit number in unsigned 3-digit decimal format
// Position determined by BSP_LCD_SetCursor command
// Input: 32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Fixed format 3 digits with no space before or after
void Bitmap_OutUDec3(uint32_t n, int16_t textColor){
  Bitmap_Messageindex = 0;
  Bitmap_fillmessage3(n);
  Bitmap_Message[Bitmap_Messageindex] = 0; // terminate
  Bitmap_DrawString(Bitmap_StX,Bitmap_StY, Bitmap_Message,textColor);
  Bitmap_StX = Bitmap_StX+Bitmap_Messageindex;
  if(Bitmap_StX>20){
    Bitmap_StX = 20;
    Bitmap_DrawCharS(Bitmap_StX*6,Bitmap_StY*10,'*',LCD_RED, 1);
  }
}

//-----------------------Bitmap_OutUDec4-----------------------
// Output a 32-bit number in unsigned 4-digit decimal format
// Position determined by BSP_LCD_SetCursor command
// Input: 32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Fixed format 4 digits with no space before or after
void Bitmap_OutUDec4(uint32_t n, int16_t textColor){
  Bitmap_Messageindex = 0;
  Bitmap_fillmessage4(n);
  Bitmap_Message[Bitmap_Messageindex] = 0; // terminate
  Bitmap_DrawString(Bitmap_StX,Bitmap_StY, Bitmap_Message,textColor);
  Bitmap_StX = Bitmap_StX+Bitmap_Messageindex;
  if(Bitmap_StX>20){
    Bitmap_StX = 20;
    Bitmap_DrawCharS(Bitmap_StX*6,Bitmap_StY*10,'*',LCD_RED, 1);
  }
}

//-----------------------Bitmap_OutUDec5-----------------------
// Output a 32-bit number in unsigned 5-digit decimal format
// Position determined by BSP_LCD_SetCursor command
// Input: 32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Fixed format 5 digits with no space before or after
void Bitmap_OutUDec5(uint32_t n, int16_t textColor){
  Bitmap_Messageindex = 0;
  Bitmap_fillmessage5(n);
  Bitmap_Message[Bitmap_Messageindex] = 0; // terminate
  Bitmap_DrawString(Bitmap_StX,Bitmap_StY, Bitmap_Message,textColor);
  Bitmap_StX = Bitmap_StX+Bitmap_Messageindex;
  if(Bitmap_StX>20){
    Bitmap_StX = 20;
    Bitmap_DrawCharS(Bitmap_StX*6,Bitmap_StY*10,'*',LCD_RED, 1);
  }
}

//-----------------------Bitmap_OutUFix2_1-----------------------
// Output a 32-bit number in unsigned 3-digit fixed point, 0.1 resolution
// numbers 0 to 999 printed as " 0.0" to "99.9"
// Position determined by BSP_LCD_SetCursor command
// Input: 32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Fixed format 4 characters with no space before or after
void Bitmap_OutUFix2_1(uint32_t n, int16_t textColor){
  Bitmap_fillmessage2_1(n);
  Bitmap_DrawString(Bitmap_StX,Bitmap_StY, Bitmap_Message,textColor);
  Bitmap_StX = Bitmap_StX+4;
  if(Bitmap_StX>20){
    Bitmap_StX = 20;
    Bitmap_DrawCharS(Bitmap_StX*6,Bitmap_StY*10,'*',LCD_RED, 1);
  }
}

//-----------------------Bitmap_OutUHex2-----------------------
// Output a 32-bit number in unsigned 2-digit hexadecimal format
// numbers 0 to 255 printed as "00," to "FF,"
// Position determined by BSP_LCD_SetCursor command
// Input: 32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Fixed format 3 characters with comma after
void Bitmap_OutUHex2(uint32_t n, int16_t textColor){
  Bitmap_fillmessage2_Hex(n);
  Bitmap_DrawString(Bitmap_StX,Bitmap_StY, Bitmap_Message,textColor);
  Bitmap_StX = Bitmap_StX+3;
  if(Bitmap_StX>20){
    Bitmap_StX = 20;
    Bitmap_DrawCharS(Bitmap_StX*6,Bitmap_StY*10,'*',LCD_RED, 1);
  }
}
//------------Bitmap_MessageUDec4-------------------
// Divide the LCD into two logical partitions and provide
// an interface to output a string
// inputs: 	device	specifies top(0) or bottom(1)
//					line 		specifies line number (0-5)
//                  col     specifies column number (0-20)
// 					string	pointer to NULL-terminated ASCII string
//  				value		16-bit number in unsigned decimal format
// outputs: none
void Bitmap_MessageUDec4(int device, int line, int col, char *string, unsigned int value){
	uint16_t StringVPosition, DecimalHPosition;
	StringVPosition = device * 7 + line;
	DecimalHPosition = col + Bitmap_DrawString(col, StringVPosition, string, LCD_WHITE);
	Bitmap_SetCursor(DecimalHPosition, StringVPosition);
	Bitmap_OutUDec4(value, LCD_WHITE);
}

//------------Bitmap_MessageUDec3-------------------
// Divide the LCD into two logical partitions and provide
// an interface to output a string
// inputs: 	device	specifies top(0) or bottom(1)
//					line 		specifies line number (0-5)
//                  col     specifies column number (0-20)
// 					string	pointer to NULL-terminated ASCII string
//  				value		16-bit number in unsigned decimal format
// outputs: none
void Bitmap_MessageUDec3(int device, int line, int col, char *string, unsigned int value){
	uint16_t StringVPosition, DecimalHPosition;
	StringVPosition = device * 7 + line;
	DecimalHPosition = col + Bitmap_DrawString(col, StringVPosition, string, LCD_WHITE);
	Bitmap_SetCursor(DecimalHPosition, StringVPosition);
	Bitmap_OutUDec3(value, LCD_WHITE);
}



//------------Bitmap_DrawCrosshair-------------------
// Draw a crosshair at the given coordinates
// inputs: 	x				specifies the x coordinate (0 to 127)
//					y 			specifies the y coordinate (0 to 127)
//					color		specifies the color of the crosshair
// outputs: none
void Bitmap_DrawCrosshair(int16_t x, int16_t y, int16_t color) {
  // Your Code Here
	int16_t halfCrossLength_x = 5;
	int16_t halfCrossLength_y = 5;
	int16_t CrossLength_x,CrossLength_y;
	int16_t start_x,start_y;
	
	start_x = x-halfCrossLength_x;
	start_y = y-halfCrossLength_y;
	CrossLength_x = 2*halfCrossLength_x+1;
	CrossLength_y = 2*halfCrossLength_y+1;
	
	
	if(start_x < 0){
		start_x=0;
		CrossLength_x = x + halfCrossLength_x;
	}
	if(x + halfCrossLength_x > 127){
		CrossLength_x = 128-start_x;
	}
	if(start_y < 0){
		start_y=0;
		CrossLength_y = y + halfCrossLength_y;
	}
	if(y + halfCrossLength_y > 117){
		CrossLength_y = 118-start_y;
	}
	Bitmap_DrawHLine(start_x,y      ,CrossLength_x,color);
	Bitmap_DrawVLine(x,      start_y,CrossLength_y,color);
}

//------------Bitmap_DrawBox------------------
// Draw a box at the given coordinates
// outputs: none
void Bitmap_DrawBox(int16_t x, int16_t y, int16_t color, int16_t w, int16_t h) {
	Bitmap_DrawHLine(x,    y,    w, color);
	Bitmap_DrawHLine(x,    y+h-1,w, color);
	Bitmap_DrawVLine(x,    y,    h, color);
	Bitmap_DrawVLine(x+w-1,y,    h, color);
}


