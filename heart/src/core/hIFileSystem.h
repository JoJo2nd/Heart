/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef hcFileManager_h__
#define hcFileManager_h__

#include "base/hTypes.h"
#include "base/hFunctor.h"

namespace Heart
{
    class hIFile;

    enum hFileMode
    {
        FILEMODE_READ,
        FILEMODE_WRITE,

        FILEMODE_MAX
    };

    struct hFileInfo
    {
        const hChar*	path_;
        const hChar*	name_;
        hByte			directory_;
    };

    hFUNCTOR_TYPEDEF(hBool(*)(const hFileInfo*), hEnumerateFilesCallback);

    class  hIFileSystem
    {
    public:
        virtual ~hIFileSystem() {}

        virtual hIFile*	OpenFile( const hChar* filename, hFileMode mode ) const = 0;
        virtual void	CloseFile( hIFile* pFile ) const = 0;
        virtual void	EnumerateFiles( const hChar* path, hEnumerateFilesCallback fn ) const = 0;
        virtual void    createDirectory(const hChar* path) = 0;
        virtual hBool   WriteableSystem() const { return hTrue; }

    };
}

#endif // hcFileManager_h__