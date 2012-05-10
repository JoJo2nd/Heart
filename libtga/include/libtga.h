
typedef unsigned char   tga_byte;
typedef short           tga_int16;

typedef size_t(*TGAReadBytes_t)(void* dst, size_t read, void* user);
typedef size_t(*TGASeekBytes_t)(void* dst, size_t read, void* user);
typedef void*(*TGAmalloc_t)(size_t size);
typedef void*(*TGAfree_t)(void* ptr);

typedef TGADataReaderFuncs
{
    TGAReadBytes_t  reader_;
    TGASeekBytes_t  seek_;
    TGAmalloc_t     mallocFunc_;
    TGAfree_t       freeFunc_;
    void*           user_;
} TGADataReaderFuncs_t;

typedef TGAImageInfo
{
    tga_int16   width_;
    tga_int16   height_;
    tga_byte    bbp_;
    int         rleCompressed_;
} TGAImageInfo_t;

typedef TGAImage TGAImage_t;

TGAImage_t*         TGACreateImage(TGADataReaderFuncs* funcs);
void                TGADestroyImage(TGAImage* image);
TGAImageInfo_t*     TGAGetImageInfo(TGAImage* image);