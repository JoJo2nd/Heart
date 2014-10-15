/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef DRIVEFILESYSTEM_H__
#define DRIVEFILESYSTEM_H__

#include "core/hIFileSystem.h"

namespace Heart
{
    class hDriveFileSystem : public hIFileSystem
    {
    public:
        hDriveFileSystem() 
        {
        }
        ~hDriveFileSystem()
        {
        }

        hIFile*	OpenFile( const hChar* filename, hFileMode mode ) const;
        void	CloseFile( hIFile* pFile ) const;
        void	EnumerateFiles( const hChar* path, hEnumerateFilesCallback fn ) const;
        void    createDirectory(const hChar* path);

    private:
    };
}

#endif // DRIVEFILESYSTEM_H__