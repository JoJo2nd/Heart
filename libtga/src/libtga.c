
#include "libtga.h"
#include <stdlib.h>

//#pack 0

typedef struct TGAHeader
{
    tga_byte idSize_;
    tga_byte colourMapType_;
    tga_byte imageType_;
    tga_int16 originX_;
    tga_int16 originY_;
    tga_int16 width_;
    tga_int16 height_;
    tga_byte  bpp_;
    tga_byte  imgDesc_;
} TGAHeader_t;

typedef TGAImage
{
    TGAHeader_t header_;
    tga_byte*   data_;
} TGAImage_t;

//pack off