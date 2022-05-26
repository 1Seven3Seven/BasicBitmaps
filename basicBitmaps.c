// Including standard headers
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// My stuff
#include "basicBitmaps.h"

// Macros
#define lerp(a, b, c) a + c *(b - a)

//==============================================================================
// Setup and saving of the bitmap and other related things
//==============================================================================

void bmHeaderInit(BITMAPHEADER *bitmapHeader, int width, int height)
{
    /*
    Initilise the bitmap header to describe a bitmap of certain width and height
    */

    unsigned int fileSize = width * height * 4;     // Total size of file, 4 bytes per pixel
    memset(bitmapHeader, 0, sizeof(*bitmapHeader)); // Zero everything

    // Writing the bitmap file header
    strcpy((char *)&bitmapHeader->identifier, "BM");
    bitmapHeader->bitmapFileSize = fileSize + 54;
    bitmapHeader->offset = 54; // The size, in bytes, of both the file header and info header

    // Writing the bitmap info header
    bitmapHeader->infoHeaderSize = 40;
    bitmapHeader->width = width;
    bitmapHeader->height = height;
    bitmapHeader->colourPlanes = (short)1;
    bitmapHeader->bitsPerPixel = (short)32;
    bitmapHeader->imageSize = fileSize;
}

unsigned char *bmCreateImageData(BITMAPHEADER *bitmapHeader)
{
    /*
    Returns a pointer to some image data created off information in the given bitmap header
    */

    unsigned char *imageData = malloc(bitmapHeader->width * bitmapHeader->height * 4);
    // Set all alpha channels to 255
    int offset;
    for (int row = 0; row < bitmapHeader->width; row++)
    {
        for (int col = 0; col < bitmapHeader->height; col++)
        {
            offset = (row * bitmapHeader->width + col) * 4 + 3;
            imageData[offset] = 255;
        }
    }
    return imageData;
}

BITMAP bmGetBitmap(int width, int height)
{
    /*
    Creates a basic struct containing the neccessary data for a bitmap
    Handles a bunch of annoying stuff and makes the rest of the functions easier to use
    */

    // Create the bitmap
    BITMAP bitmap;

    // Create the bitmap header
    BITMAPHEADER bitmapHeader;
    bmHeaderInit(&bitmapHeader, width, height);

    // Create the image data
    unsigned char *imageData = bmCreateImageData(&bitmapHeader);

    // Pass everything to the bitmap
    bitmap.bitmapHeader = bitmapHeader;
    bitmap.imageData = imageData;

    // Give
    return bitmap;
}

int bmWriteToFile(BITMAP bitmap, const char *fileName)
{
    /*
    Saves the bitmap file with the given file name
    Returns 0 on failure
    */

    FILE *fptr = fopen(fileName, "wb");
    if (fptr == NULL)
    {
        return 0;
    }
    fwrite(&bitmap.bitmapHeader, sizeof(bitmap.bitmapHeader), 1, fptr);
    fwrite(bitmap.imageData, 1, bitmap.bitmapHeader.imageSize, fptr);
    fclose(fptr);
    return 1;
}

int bmGetBitmapFromFile(BITMAP *bitmap, const char *fileName)
{
    /*
    Reads the bitmap file and places the information into the bitmap struct given
    Returns 0 on a faliure
    */

    // Checking of the file exists
    FILE *fptr = fopen(fileName, "rb");
    if (fptr == NULL)
        return 0;

    // The file exists, copy the first 54 bytes into the header and check if the identifyer is correct
    fread(&bitmap->bitmapHeader, 1, 54, fptr);

    if (*(char *)&bitmap->bitmapHeader.identifier != 'B' || *(((char *)&bitmap->bitmapHeader.identifier) + 1) != 'M')
        return 0;

    // Well, now we can copy over the image data
    bitmap->imageData = bmCreateImageData(&bitmap->bitmapHeader);
    fread(bitmap->imageData, 1, bitmap->bitmapHeader.imageSize, fptr);

    // Close the file
    fclose(fptr);
    // As everything has been done
    return 1;
}

void bmFreeBitmapImageData(BITMAP *bitmap)
{
    free(bitmap->imageData);
}

//==============================================================================
// Retrieving information
//==============================================================================

int bmGetWidth(BITMAP bitmap)
{
    return bitmap.bitmapHeader.width;
}

int bmGetHeight(BITMAP bitmap)
{
    return bitmap.bitmapHeader.height;
}

unsigned int bmGetBitmapFileSize(BITMAP bitmap)
{
    return bitmap.bitmapHeader.bitmapFileSize;
}

unsigned int bmGetImageSize(BITMAP bitmap)
{
    return bitmap.bitmapHeader.imageSize;
}

double bmGetImageCenterX(BITMAP bitmap)
{
    return ((double)bitmap.bitmapHeader.width - 1.0) / 2;
}

double bmGetImageCenterY(BITMAP bitmap)
{
    return ((double)bitmap.bitmapHeader.height - 1.0) / 2;
}

//==============================================================================
// Drawing to the bitmap
//==============================================================================

void bmFillImageData(BITMAP bitmap, COLOUR colour)
{
    /*
    Fills the image data with the given colour
    */

    int offset;
    for (int row = 0; row < bitmap.bitmapHeader.height; row++)
    {
        for (int col = 0; col < bitmap.bitmapHeader.width; col++)
        {
            offset = (row * bitmap.bitmapHeader.width + col) * 4;
            bitmap.imageData[offset + 0] = colour.blue;
            bitmap.imageData[offset + 1] = colour.green;
            bitmap.imageData[offset + 2] = colour.red;
            bitmap.imageData[offset + 3] = 255;
        }
    }
}

void bmDrawRectangle(BITMAP bitmap, COLOUR colour, int left, int right, int bottom, int top, char flags)
{
    /*
    Fills a rectangle in the bitmap with the given colour
    Ignores any area outside of the bitmap
    */

    // Constraining to the bitmap size
    if (left < 0)
        left = 0;
    if (right > bitmap.bitmapHeader.width)
        right = bitmap.bitmapHeader.width;
    if (top > bitmap.bitmapHeader.height)
        top = bitmap.bitmapHeader.height;
    if (bottom < 0)
        bottom = 0;

    // Writing to the bitmap
    int offset;
    if (!flags) // No flags
    {
        for (int row = bottom; row < top; row++)
        {
            for (int col = left; col < right; col++)
            {
                offset = (row * bitmap.bitmapHeader.width + col) * 4;
                bitmap.imageData[offset + 0] = colour.blue;
                bitmap.imageData[offset + 1] = colour.green;
                bitmap.imageData[offset + 2] = colour.red;
            }
        }
    }
    else if (flags & BM_BLEND_RGB_ADD) // Add the rgb values
    {
        for (int row = bottom; row < top; row++)
        {
            for (int col = left; col < right; col++)
            {
                offset = (row * bitmap.bitmapHeader.width + col) * 4;

                // Blue
                if (bitmap.imageData[offset + 0] + colour.blue > 255)
                    bitmap.imageData[offset + 0] = 255;
                else
                    bitmap.imageData[offset + 0] += colour.blue;

                // Green
                if (bitmap.imageData[offset + 1] + colour.green > 255)
                    bitmap.imageData[offset + 1] = 255;
                else
                    bitmap.imageData[offset + 1] += colour.green;

                // Red
                if (bitmap.imageData[offset + 2] + colour.red > 255)
                    bitmap.imageData[offset + 2] = 255;
                else
                    bitmap.imageData[offset + 2] += colour.red;
            }
        }
    }
    else if (flags & BM_BLEND_RGB_SUB) // Subtract the rgb values
    {
        for (int row = bottom; row < top; row++)
        {
            for (int col = left; col < right; col++)
            {
                offset = (row * bitmap.bitmapHeader.width + col) * 4;

                // Blue
                if (bitmap.imageData[offset + 0] - colour.blue < 0)
                    bitmap.imageData[offset + 0] = 0;
                else
                    bitmap.imageData[offset + 0] -= colour.blue;

                // Green
                if (bitmap.imageData[offset + 1] - colour.green < 0)
                    bitmap.imageData[offset + 1] = 0;
                else
                    bitmap.imageData[offset + 1] -= colour.green;

                // Red
                if (bitmap.imageData[offset + 2] - colour.red < 0)
                    bitmap.imageData[offset + 2] = 0;
                else
                    bitmap.imageData[offset + 2] -= colour.red;
            }
        }
    }
}

void bmDrawCircle(BITMAP bitmap, COLOUR colour, int x, int y, int radius, char flags)
{
    /*
    Fills a circle in the bitmap with the given colour
    Ignores any area outside of the bitmap
    */

    // To prevent going over the whole bitmap, find a rectagle constraining the circle
    int left = x - radius, right = x + radius;
    int bottom = y - radius, top = y + radius;

    // Constraining to the bitmap size
    if (left < 0)
        left = 0;
    if (right > bitmap.bitmapHeader.width)
        right = bitmap.bitmapHeader.width;
    if (top > bitmap.bitmapHeader.height)
        top = bitmap.bitmapHeader.height;
    if (bottom < 0)
        bottom = 0;

    // Writing to the bitmap
    int offset, xDifference, yDifference;
    int radiousSquared = radius * radius;
    if (!flags)
    {
        for (int row = bottom; row < top; row++)
        {
            for (int col = left; col < right; col++)
            {
                // Circle equation x^2 + y^2 = r^2
                xDifference = x - col;
                xDifference *= xDifference;
                yDifference = y - row;
                yDifference *= yDifference;

                if (xDifference + yDifference < radiousSquared)
                {
                    offset = (row * bitmap.bitmapHeader.width + col) * 4;
                    bitmap.imageData[offset + 0] = colour.blue;
                    bitmap.imageData[offset + 1] = colour.green;
                    bitmap.imageData[offset + 2] = colour.red;
                }
            }
        }
    }
    else if (flags & BM_BLEND_RGB_ADD)
    {
        for (int row = bottom; row < top; row++)
        {
            for (int col = left; col < right; col++)
            {
                // Circle equation x^2 + y^2 = r^2
                xDifference = x - col;
                xDifference *= xDifference;
                yDifference = y - row;
                yDifference *= yDifference;

                if (xDifference + yDifference < radiousSquared)
                {
                    offset = (row * bitmap.bitmapHeader.width + col) * 4;

                    // Blue
                    if (bitmap.imageData[offset + 0] + colour.blue > 255)
                        bitmap.imageData[offset + 0] = 255;
                    else
                        bitmap.imageData[offset + 0] += colour.blue;

                    // Green
                    if (bitmap.imageData[offset + 1] + colour.green > 255)
                        bitmap.imageData[offset + 1] = 255;
                    else
                        bitmap.imageData[offset + 1] += colour.green;

                    // Red
                    if (bitmap.imageData[offset + 2] + colour.red > 255)
                        bitmap.imageData[offset + 2] = 255;
                    else
                        bitmap.imageData[offset + 2] += colour.red;
                }
            }
        }
    }
    else if (flags & BM_BLEND_RGB_SUB)
    {
        for (int row = bottom; row < top; row++)
        {
            for (int col = left; col < right; col++)
            {
                // Circle equation x^2 + y^2 = r^2
                xDifference = x - col;
                xDifference *= xDifference;
                yDifference = y - row;
                yDifference *= yDifference;

                if (xDifference + yDifference < radiousSquared)
                {
                    offset = (row * bitmap.bitmapHeader.width + col) * 4;

                    // Blue
                    if (bitmap.imageData[offset + 0] - colour.blue < 0)
                        bitmap.imageData[offset + 0] = 0;
                    else
                        bitmap.imageData[offset + 0] -= colour.blue;

                    // Green
                    if (bitmap.imageData[offset + 1] - colour.green < 0)
                        bitmap.imageData[offset + 1] = 0;
                    else
                        bitmap.imageData[offset + 1] -= colour.green;

                    // Red
                    if (bitmap.imageData[offset + 2] - colour.red < 0)
                        bitmap.imageData[offset + 2] = 0;
                    else
                        bitmap.imageData[offset + 2] -= colour.red;
                }
            }
        }
    }
}

void bmDrawLine(BITMAP bitmap, COLOUR colour, int startX, int startY, int endX, int endY, char flags)
{
    /*
    Draws a line in the bitmap with the given colour
    Ignores any area outside of the bitmap
    */

    // Constraining to the bitmap size
    if (startX < 0)
        startX = 0;
    if (endX > bitmap.bitmapHeader.width)
        endX = bitmap.bitmapHeader.width;
    if (startY < 0)
        startY = 0;
    if (endY > bitmap.bitmapHeader.height)
        endY = bitmap.bitmapHeader.height;

    // Finding number of points to draw
    int numPoints;
    int dx = abs(startX - endX), dy = abs(startY - endY);
    if (dx > dy)
        numPoints = dx;
    else
        numPoints = dy;
    numPoints++;

    // Writing to bitmap
    int offset, row, col;
    if (!flags) // No flags
    {
        for (int i = 0; i < numPoints; i++)
        {
            col = (int)lerp(startX, endX, (double)i / numPoints);
            row = (int)lerp(startY, endY, (double)i / numPoints);

            offset = (row * bitmap.bitmapHeader.width + col) * 4;

            bitmap.imageData[offset + 0] = colour.blue;
            bitmap.imageData[offset + 1] = colour.green;
            bitmap.imageData[offset + 2] = colour.red;
        }
    }
    if (flags & BM_BLEND_RGB_ADD)
    {
        for (int i = 0; i < numPoints; i++)
        {
            col = (int)lerp(startX, endX, (double)i / numPoints);
            row = (int)lerp(startY, endY, (double)i / numPoints);

            offset = (row * bitmap.bitmapHeader.width + col) * 4;

            // Blue
            if (bitmap.imageData[offset + 0] + colour.blue > 255)
                bitmap.imageData[offset + 0] = 255;
            else
                bitmap.imageData[offset + 0] += colour.blue;

            // Green
            if (bitmap.imageData[offset + 1] + colour.green > 255)
                bitmap.imageData[offset + 1] = 255;
            else
                bitmap.imageData[offset + 1] += colour.green;

            // Red
            if (bitmap.imageData[offset + 2] + colour.red > 255)
                bitmap.imageData[offset + 2] = 255;
            else
                bitmap.imageData[offset + 2] += colour.red;
        }
    }
    else if (flags & BM_BLEND_RGB_SUB)
    {
        for (int i = 0; i < numPoints; i++)
        {
            col = (int)lerp(startX, endX, (double)i / numPoints);
            row = (int)lerp(startY, endY, (double)i / numPoints);

            offset = (row * bitmap.bitmapHeader.width + col) * 4;

            // Blue
            if (bitmap.imageData[offset + 0] - colour.blue < 0)
                bitmap.imageData[offset + 0] = 0;
            else
                bitmap.imageData[offset + 0] -= colour.blue;

            // Green
            if (bitmap.imageData[offset + 1] - colour.green < 0)
                bitmap.imageData[offset + 1] = 0;
            else
                bitmap.imageData[offset + 1] -= colour.green;

            // Red
            if (bitmap.imageData[offset + 2] - colour.red < 0)
                bitmap.imageData[offset + 2] = 0;
            else
                bitmap.imageData[offset + 2] -= colour.red;
        }
    }
}

void bmSetColorAt(BITMAP bitmap, COLOUR colour, int x, int y, char flags)
{
    /*
    Draws a line in the bitmap with the given colour
    Ignores any area outside of the bitmap
    */

    // Constraining to the bitmap size
    if (x < 0 || x > bitmap.bitmapHeader.width)
        return;
    if (y < 0 || y > bitmap.bitmapHeader.height)
        return;

    // Writing to bitmap
    int offset = (y * bitmap.bitmapHeader.width + x) * 4;
    if (!flags)
    {
        bitmap.imageData[offset + 0] = colour.blue;
        bitmap.imageData[offset + 1] = colour.green;
        bitmap.imageData[offset + 2] = colour.red;
    }
    else if (flags & BM_BLEND_RGB_ADD)
    {
        // Blue
        if (bitmap.imageData[offset + 0] + colour.blue > 255)
            bitmap.imageData[offset + 0] = 255;
        else
            bitmap.imageData[offset + 0] += colour.blue;

        // Green
        if (bitmap.imageData[offset + 1] + colour.green > 255)
            bitmap.imageData[offset + 1] = 255;
        else
            bitmap.imageData[offset + 1] += colour.green;

        // Red
        if (bitmap.imageData[offset + 2] + colour.red > 255)
            bitmap.imageData[offset + 2] = 255;
        else
            bitmap.imageData[offset + 2] += colour.red;
    }
    else if (flags & BM_BLEND_RGB_SUB)
    {
        // Blue
        if (bitmap.imageData[offset + 0] - colour.blue < 0)
            bitmap.imageData[offset + 0] = 0;
        else
            bitmap.imageData[offset + 0] -= colour.blue;

        // Green
        if (bitmap.imageData[offset + 1] - colour.green < 0)
            bitmap.imageData[offset + 1] = 0;
        else
            bitmap.imageData[offset + 1] -= colour.green;

        // Red
        if (bitmap.imageData[offset + 2] - colour.red < 0)
            bitmap.imageData[offset + 2] = 0;
        else
            bitmap.imageData[offset + 2] -= colour.red;
    }
}

//==============================================================================
// More interesting things to do with the bitmaps
//==============================================================================

void bmRotateImage(BITMAP bitmap, double xCenter, double yCenter, double angle)
{
    /*
    Rotates the given bitmap image around the given coordinates by the given angle
    Please note that any part of the image that will be outside of the bitmaps bounds will be cut off
    */

    // Copy the image data
    unsigned char *imageCopy = malloc(bitmap.bitmapHeader.imageSize);
    memcpy(imageCopy, bitmap.imageData, bitmap.bitmapHeader.imageSize);

    // Clear the image data
    bmFillImageData(bitmap, bmGetColour(0, 0, 0));

    // Find the sin and cos of the angle
    double sinAngle, cosAngle;
    // If The angle is a nice value then
    if (angle == 0)
    {
        sinAngle = 0;
        cosAngle = 1;
    }
    else if (angle == M_PI_2)
    {
        sinAngle = 1;
        cosAngle = 0;
    }
    else if (angle == M_PI)
    {
        sinAngle = 0;
        cosAngle = -1;
    }
    else if (angle == (double)3 / 2 * M_PI)
    {
        sinAngle = -1;
        cosAngle = 0;
    }
    else // If not a nice value
    {
        sinAngle = sin(angle);
        cosAngle = cos(angle);
    }

    // Necessary variables
    int preRotationCol, preRotationRow;
    int rotatedOffset, preRotationOffset;
    // For every pixel
    for (int rotatedRow = 0; rotatedRow < bitmap.bitmapHeader.height; rotatedRow++)
    {
        for (int rotatedCol = 0; rotatedCol < bitmap.bitmapHeader.width; rotatedCol++)
        {
            // Rotate the current coords with respect to the center
            preRotationCol = cosAngle * (rotatedCol - xCenter) - sinAngle * (rotatedRow - yCenter) + xCenter;
            preRotationRow = sinAngle * (rotatedCol - xCenter) + cosAngle * (rotatedRow - yCenter) + yCenter;

            // Check if within bounds
            if (0 <= preRotationCol && preRotationCol < bitmap.bitmapHeader.width)
            {
                if (0 <= preRotationRow && preRotationRow < bitmap.bitmapHeader.width)
                {
                    // Calculate offsets
                    preRotationOffset = (preRotationRow * bitmap.bitmapHeader.width + preRotationCol) * 4;
                    rotatedOffset = (rotatedRow * bitmap.bitmapHeader.width + rotatedCol) * 4;

                    // Copy data
                    bitmap.imageData[rotatedOffset + 0] = imageCopy[preRotationOffset + 0];
                    bitmap.imageData[rotatedOffset + 1] = imageCopy[preRotationOffset + 1];
                    bitmap.imageData[rotatedOffset + 2] = imageCopy[preRotationOffset + 2];
                }
            }
        }
    }
}

//==============================================================================
// Miscellaneous
//==============================================================================

COLOUR bmGetColour(unsigned char red, unsigned char green, unsigned char blue)
{
    /*
    Simply returns a colour object that has the given values for red, green and blue
    This is to make some parts of the code easier to read
        Rather than pass in the rgb values, you now can just pass a colour object
    */

    COLOUR colour;
    colour.red = red;
    colour.green = green;
    colour.blue = blue;

    return colour;
}

// By Seven
