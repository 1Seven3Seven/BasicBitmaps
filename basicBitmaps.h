#ifndef BASICBITMAPS_H
#define BASICBITMAPS_H

#define BM_BLEND_RGB_ADD 1
#define BM_BLEND_RGB_SUB 2

#pragma pack(1) // To prevent c from adding padding to the structure below
typedef struct  // Contains all the necessary information for a bitmap header
{
    // The bitmap file header, 14 bytes total
    unsigned short identifier;   // 2 Bytes: The header field used to identify the BMP file, should always be "BM"
    unsigned int bitmapFileSize; // 4 Bytes: The size of the file in bytes
    unsigned short reserved1;    // 2 Bytes: Reserved section, should be 0
    unsigned short reserved2;    // 2 Bytes: Reserved section, should be 0
    unsigned int offset;         // 4 Bytes: The offset to the start of the bitmap image data, the pixel array, should be 54
    // The bitmap info header, 40 bytes total
    unsigned int infoHeaderSize;      // 4 Bytes: The size of this header, should be 54
    int width;                        // 4 Bytes: The width of the pixel array, signed integer
    int height;                       // 4 Bytes: The height of the pixel array, signed interger
    unsigned short colourPlanes;      // 2 Bytes: The number of colour planes, must be 1 according to wikipedia
    unsigned short bitsPerPixel;      // 2 Bytes: The number of bits per pixel, I dont think I really needed to explain that
    unsigned int compressionMethod;   // 4 Bytes: The compression method used, 0 for no compression
    unsigned int imageSize;           // 4 Bytes: The size of the raw bitmap data, a dummy 0 can be used if a 0 is used for compression, my functions expect a proper value though
    int horizontalResolution;         // 4 Bytes: The horizontal resolution of the image, pixel per metre, signed integer
    int verticalResolution;           // 4 Bytes: The vertical resolution of the image, pixel per metre, signed integer
    unsigned int colourPaletteNumber; // 4 Bytes: The number of colours in the colour palatte, use 0 as we wont use a colour palette
    unsigned int importantColours;    // 4 Bytes: The number of important colours used, 0 when every colour is important, generally ignored
    // Total size 54 bytes
} BITMAPHEADER;

#pragma pack() // Return padding to normal

typedef struct // A struct to contain all the information relating to a bitmap
{
    BITMAPHEADER bitmapHeader;
    unsigned char *imageData;
} BITMAP;

typedef struct // I have no idea what this is used for
{
    unsigned char red, green, blue; // Hmmmm, incomprehensible...
} COLOUR;

// Setup and saving of a bitmap and other related things
void bmHeaderInit(BITMAPHEADER *bitmapHeader, int width, int height);
unsigned char *bmCreateImageData(BITMAPHEADER *bitmapHeader);

BITMAP bmGetBitmap(int width, int height);
int bmWriteToFile(BITMAP bitmap, const char *fileName);
int bmGetBitmapFromFile(BITMAP *bitmap, const char *fileName);

void bmFreeBitmapImageData(BITMAP *bitmap);

// Retrieving information
int bmGetWidth(BITMAP bitmap);
int bmGetHeight(BITMAP bitmap);
unsigned int bmGetBitmapFileSize(BITMAP bitmap);
unsigned int bmGetImageSize(BITMAP bitmap);
double bmGetImageCenterX(BITMAP bitmap);
double bmGetImageCenterY(BITMAP bitmap);

// Drawing things to the bitmap
void bmFillImageData(BITMAP bitmap, COLOUR colour);
void bmDrawRectangle(BITMAP bitmap, COLOUR colour, int left, int right, int bottom, int top, char flags);
void bmDrawCircle(BITMAP bitmap, COLOUR colour, int x, int y, int radius, char flags);
void bmDrawLine(BITMAP bitmap, COLOUR colour, int startX, int startY, int endX, int endY, char flags);
void bmSetColorAt(BITMAP bitmap, COLOUR colour, int x, int y, char flags);

// More interesting things to do with the bitmaps
void bmRotateImage(BITMAP bitmap, double xCenter, double yCenter, double angle);

// Miscellaneous
COLOUR bmGetColour(unsigned char red, unsigned char green, unsigned char blue);

// By Seven

#endif