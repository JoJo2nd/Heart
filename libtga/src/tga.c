
#include "libtga/tga.h"
#include <memory.h>

//#pack 0

#define LTGA_RLE_FLAG (0x0008)
#define LTGA_HFLIPBIT               (1 << 4)//set for top to bottom order
#define LTGA_VFLIPBIT               (1 << 5)//set for left to right order

#pragma pack (push, 1)

typedef struct TGAHeader
{
    tga_byte  idSize_;
    tga_byte  colourMapType_;
    tga_byte  imageType_;
    tga_int16 colourMapStart_;
    tga_int16 colourMapLen_;
    tga_byte  colourMapBits_;
    tga_int16 originX_;
    tga_int16 originY_;
    tga_int16 width_;
    tga_int16 height_;
    tga_byte  bpp_;
    tga_byte  imgDesc_;
} TGAHeader_t;

#pragma pack (pop)

typedef struct TGAImage
{
    TGAHeader_t             header_;
    size_t                  dataSize_;
    tga_byte*               data_;
    TGADataReaderFuncs_t    dataFuncs_;
    void*                   user_;
} TGAImage_t;

typedef struct PixelReaderCtx
{
    TGAReadBytes_t  reader_;
    void*           user_;
    //RLE info
} PixelReaderCtx_t;

typedef void(*TGAPixelReader_t)(PixelReaderCtx_t* ctx, tga_byte* dst);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int TGAReadNoCompress(void* dst, size_t dstSize, TGAReadBytes_t reader, const TGAHeader_t* header, void* user);
int TGAReadRLECompress(void* dst, size_t dstSize, TGAReadBytes_t reader, const TGAHeader_t* header, void* user);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void TGAPixel32Reader(PixelReaderCtx_t* ctx, tga_byte* dst);
void TGAPixel24Reader(PixelReaderCtx_t* ctx, tga_byte* dst);
void TGAPixel16Reader(PixelReaderCtx_t* ctx, tga_byte* dst);
void TGAPixel8Reader(PixelReaderCtx_t* ctx, tga_byte* dst);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TGAImage_t* TGACreateImage(TGADataReaderFuncs_t funcs, void* user)
{
    TGAImage_t* img = (funcs.mallocFunc_)(sizeof(TGAImage_t), user);
    img->dataFuncs_ = funcs;
    img->user_ = user;
    img->dataSize_ = 0;
    img->data_ = NULL;
    memset(&img->header_, 0, sizeof(img->header_));

    return img;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TGADestroyImage(TGAImage_t* image)
{
    if (!image)
        return;

    (image->dataFuncs_.freeFunc_)(image->data_, image->user_);
    image->data_ = NULL;
    image->dataSize_ = 0;

    (image->dataFuncs_.freeFunc_)(image, image->user_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TGAGetImageInfo(TGAImage_t* image, TGAImageInfo_t* info)
{
    if (!image || !info)
        return;

    info->bbp_ = image->header_.bpp_*8;
    info->width_ = image->header_.width_;
    info->height_ = image->header_.height_;
    info->rleCompressed_ = (image->header_.imageType_ & LTGA_RLE_FLAG) == LTGA_RLE_FLAG;
    info->colourWidth_ = image->header_.bpp_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int TGASetImageInfo(TGAImage_t* image, const TGAImageInfo_t* info)
{
    //TODO:
    return LTGA_ERR_NOSUPPORT;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int TGASetImageData(tga_byte* src, size_t size)
{
    //TODO:
    return LTGA_ERR_NOSUPPORT;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

tga_byte*   LTGA_API TGAGetImageData( TGAImage_t* image )
{
    return image->data_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int TGAReadImage(TGAImage_t* image)
{
    void* user = image->user_;
    TGAHeader_t* header = &image->header_;
    tga_byte tmpPixel;
    tga_byte* cpyPixel;
    tga_byte* dstPixel;
    int i,j,c;

    //TODO: more error handling
    (image->dataFuncs_.seek_)(0, LTGA_SEEK_SET, user);

    (image->dataFuncs_.reader_)(header, sizeof(TGAHeader_t), user);
    //change bits per pixel to bytes per pixel
    header->bpp_ /= 8;

    //No colour map support...yet
    if (header->imageType_ == 1 || header->imageType_ == 9)
        return LTGA_ERR_NOSUPPORT;

    image->dataSize_ = header->bpp_*header->width_*header->height_;
    image->data_     = (image->dataFuncs_.mallocFunc_)(image->dataSize_, user);
    if (!image->data_)
        return LTGA_ERR_NOMEM;

    (image->dataFuncs_.seek_)(header->idSize_, LTGA_SEEK_CUR, user);

    if (image->header_.imageType_ & LTGA_RLE_FLAG)
    {
        //Run Length Encoded
        TGAReadRLECompress(image->data_, image->dataSize_, image->dataFuncs_.reader_, header, user);
    }
    else
    {
        //No compression
        TGAReadNoCompress(image->data_, image->dataSize_, image->dataFuncs_.reader_, header, user);
    }

    //flip image vertically
    if ((header->imgDesc_ & LTGA_VFLIPBIT) == 0)
    {
        for (i=0; i < (header->height_>>1); ++i)
        {
            cpyPixel = image->data_+(i*header->width_*header->bpp_);
            dstPixel = image->data_+((header->height_ - 1 - i)*header->width_*header->bpp_);
            for (j=0; j < header->width_*header->bpp_; ++j)
            {
                tmpPixel = *dstPixel;
                *dstPixel = *cpyPixel;
                *cpyPixel = tmpPixel;

                ++dstPixel;
                ++cpyPixel;
            }
        }
    }

    //flip image horizontally 
    if ((header->imgDesc_ & LTGA_HFLIPBIT))
    {
        for (i=0; i < header->height_; ++i)
        {
            dstPixel = image->data_+(i*header->width_*header->bpp_);
            cpyPixel = dstPixel+((header->width_-2)*header->bpp_);
            for (j=0; j < (header->width_ >> 1); ++j)
            {
                for (c=0; c < header->bpp_; ++c)
                {
                    tmpPixel = dstPixel[c];
                    dstPixel[c] = cpyPixel[c];
                    cpyPixel[c] = tmpPixel;
                }

                dstPixel += header->bpp_;
                cpyPixel -= header->bpp_;
            }
        }
    }

    return 0;
}

int TGAReadNoCompress(void* dst, size_t dstSize, TGAReadBytes_t reader, const TGAHeader_t* header, void* user)
{
    size_t  size = 0;
    tga_byte* pixel = dst;
    PixelReaderCtx_t ctx;
    TGAPixelReader_t pixelReader = NULL;

    if (header->bpp_ == 4)      pixelReader = TGAPixel32Reader;
    else if (header->bpp_ == 3) pixelReader = TGAPixel24Reader;
    else if (header->bpp_ == 2) pixelReader = TGAPixel16Reader;
    else if (header->bpp_ == 1) pixelReader = TGAPixel8Reader;

    ctx.reader_ = reader;
    ctx.user_ = user;

    while (size < dstSize)
    {
        (pixelReader)(&ctx, pixel);
        size += header->bpp_;
        pixel += header->bpp_;
    }

    return 0;
}

int TGAReadRLECompress(void* dst, size_t dstSize, TGAReadBytes_t reader, const TGAHeader_t* header, void* user)
{
    size_t  size = 0;
    tga_byte* pixel = dst;
    tga_byte tmpPixel[4];
    PixelReaderCtx_t ctx;
    TGAPixelReader_t pixelReader = NULL;
    int rleCount = 0;
    int rleCMD = 0;
    int rleRepeat = 0;
    int i = 0;

    if (header->bpp_ == 4)      pixelReader = TGAPixel32Reader;
    else if (header->bpp_ == 3) pixelReader = TGAPixel24Reader;
    else if (header->bpp_ == 2) pixelReader = TGAPixel16Reader;
    else if (header->bpp_ == 1) pixelReader = TGAPixel8Reader;

    ctx.reader_ = reader;
    ctx.user_ = user;

    while (size < dstSize)
    {
        if (rleCount == 0)
        {
            (reader)(&rleCMD, 1, user);
            rleCount = 1 + (rleCMD & 127);
            rleRepeat = 0;
        }

        if (!rleRepeat)
        {
            (pixelReader)(&ctx, tmpPixel);
             rleRepeat = rleCMD >> 7;
        }

        for( i=0; i < header->bpp_; ++i) pixel[i] = tmpPixel[i];
        size += header->bpp_;
        pixel += header->bpp_;
        --rleCount;
    }

    return 0;
}

void TGAPixel32Reader(PixelReaderCtx_t* ctx, tga_byte* dst)
{
    tga_byte pixel[4];
    (ctx->reader_)(pixel, 4, ctx->user_);
    //TGA is BGRA so BGRA ==> RGBA
    dst[0] = pixel[2];
    dst[1] = pixel[1];
    dst[2] = pixel[0];
    dst[3] = pixel[3];
}
void TGAPixel24Reader(PixelReaderCtx_t* ctx, tga_byte* dst)
{
    tga_byte pixel[4];
    (ctx->reader_)(pixel, 3, ctx->user_);
    //TGA is BGR so BGR ==> RGB
    dst[0] = pixel[2];
    dst[1] = pixel[1];
    dst[2] = pixel[0];
}
void TGAPixel16Reader(PixelReaderCtx_t* ctx, tga_byte* dst)
{
    tga_byte pixel[4];
    (ctx->reader_)(pixel, 2, ctx->user_);
    //TGA is Luminous/Greyscale+Alpha
    dst[0] = pixel[0];
    dst[1] = pixel[1];
}
void TGAPixel8Reader(PixelReaderCtx_t* ctx, tga_byte* dst)
{
    tga_byte pixel[4];
    (ctx->reader_)(pixel, 1, ctx->user_);
    //TGA is Luminous/Greyscale 
    dst[0] = pixel[0];
}
