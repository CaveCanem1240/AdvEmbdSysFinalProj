
#include "screenStack.h"
#define IMGW 128
#define IMGH 64
#define IMGSIZE IMGW*IMGH


void Bitmap_init(uint16_t *img);

void act_FuncInStack(StackDataType *DispStackDatapt);

void BitmapSetPixel(int16_t x, int16_t y, uint16_t Color);

//------------Bitmap_FillRect------------
// Draw a filled rectangle at the given coordinates with the given width, height, and color.
// Requires (11 + 2*w*h) bytes of transmission (assuming image fully on screen)
// Input: x     horizontal position of the top left corner of the rectangle, columns from the left edge
//        y     vertical position of the top left corner of the rectangle, rows from the top edge
//        w     horizontal width of the rectangle
//        h     vertical height of the rectangle
//        color 16-bit color, which can be produced by BSP_LCD_Color565()
// Output: none
void Bitmap_FillRect(int16_t x, int16_t y, uint16_t color, int16_t w, int16_t h);

//------------Bitmap_FillScreen------------
// Fill the screen with the given color.
// Requires 33,293 bytes of transmission
// Input: color 16-bit color, which can be produced by BSP_LCD_Color565()
// Output: none
void Bitmap_FillScreen(uint16_t color);

//------------Bitmap_DrawVLine------------
// Draw a vertical line at the given coordinates with the given height and color.
// A vertical line is parallel to the longer side of the rectangular display
// Requires (11 + 2*h) bytes of transmission (assuming image fully on screen)
// Input: x     horizontal position of the start of the line, columns from the left edge
//        y     vertical position of the start of the line, rows from the top edge
//        h     vertical height of the line
//        color 16-bit color, which can be produced by BSP_LCD_Color565()
// Output: none
void Bitmap_DrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

void Bitmap_DrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

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
void Bitmap_DrawBitmap(int16_t x, int16_t y, const uint16_t *inputImage, int16_t w, int16_t h);

void Bitmap_DrawBitmapTP(int16_t x, int16_t y, const uint16_t *inputImage, int16_t w, int16_t h);
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
void Bitmap_DrawCharS(int16_t x, int16_t y, char c, int16_t textColor, uint8_t size);

void Bitmap_DrawCharS_Background(int16_t x, int16_t y, char c, int16_t textColor, int16_t bgColor, uint8_t size);

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
uint32_t Bitmap_DrawString(int16_t x, int16_t y, char *pt, int16_t textColor);

uint32_t Bitmap_DrawString_Background(int16_t x, int16_t y, char *pt, int16_t textColor,int16_t bgColor);

uint32_t Bitmap_DrawStringHighlight(int16_t x, int16_t y, char *pt, int16_t textColor);

//-----------------------Bitmap_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Position determined by BSP_LCD_SetCursor command
// Input: n         32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Variable format 1-10 digits with no space before or after
void Bitmap_OutUDec(uint32_t n, int16_t textColor);

//-----------------------Bitmap_OutUDec3-----------------------
// Output a 32-bit number in unsigned 3-digit decimal format
// Position determined by BSP_LCD_SetCursor command
// Input: 32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Fixed format 3 digits with no space before or after
void Bitmap_OutUDec3(uint32_t n, int16_t textColor);

//-----------------------Bitmap_OutUDec4-----------------------
// Output a 32-bit number in unsigned 4-digit decimal format
// Position determined by BSP_LCD_SetCursor command
// Input: 32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Fixed format 4 digits with no space before or after
void Bitmap_OutUDec4(uint32_t n, int16_t textColor);

//-----------------------Bitmap_OutUDec5-----------------------
// Output a 32-bit number in unsigned 5-digit decimal format
// Position determined by BSP_LCD_SetCursor command
// Input: 32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Fixed format 5 digits with no space before or after
void Bitmap_OutUDec5(uint32_t n, int16_t textColor);

//-----------------------Bitmap_OutUFix2_1-----------------------
// Output a 32-bit number in unsigned 3-digit fixed point, 0.1 resolution
// numbers 0 to 999 printed as " 0.0" to "99.9"
// Position determined by BSP_LCD_SetCursor command
// Input: 32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Fixed format 4 characters with no space before or after
void Bitmap_OutUFix2_1(uint32_t n, int16_t textColor);

//-----------------------Bitmap_OutUHex2-----------------------
// Output a 32-bit number in unsigned 2-digit hexadecimal format
// numbers 0 to 255 printed as "00," to "FF,"
// Position determined by BSP_LCD_SetCursor command
// Input: 32-bit number to be transferred
//        textColor 16-bit color of the numbers
// Output: none
// Fixed format 3 characters with comma after
void Bitmap_OutUHex2(uint32_t n, int16_t textColor);

//------------Bitmap_MessageUDec4-------------------
// Divide the LCD into two logical partitions and provide
// an interface to output a string
// inputs: 	device	specifies top(0) or bottom(1)
//					line 		specifies line number (0-5)
//                  col     specifies column number (0-20)
// 					string	pointer to NULL-terminated ASCII string
//  				value		16-bit number in unsigned decimal format
// outputs: none
void Bitmap_MessageUDec4(int device, int line, int col, char *string, unsigned int value);

//------------Bitmap_MessageUDec3-------------------
// Divide the LCD into two logical partitions and provide
// an interface to output a string
// inputs: 	device	specifies top(0) or bottom(1)
//					line 		specifies line number (0-5)
//                  col     specifies column number (0-20)
// 					string	pointer to NULL-terminated ASCII string
//  				value		16-bit number in unsigned decimal format
// outputs: none
void Bitmap_MessageUDec3(int device, int line, int col, char *string, unsigned int value);

//------------Bitmap_DrawCrosshair-------------------
// Draw a crosshair at the given coordinates
// inputs: 	x				specifies the x coordinate (0 to 127)
//					y 			specifies the y coordinate (0 to 127)
//					color		specifies the color of the crosshair
// outputs: none
void Bitmap_DrawCrosshair(int16_t x, int16_t y, int16_t color);

void Bitmap_DrawBox(int16_t x, int16_t y, int16_t color, int16_t w, int16_t h);

