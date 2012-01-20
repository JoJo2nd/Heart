/********************************************************************

	filename: 	gdFile.h	
	
	Copyright (c) 8:11:2011 James Moran
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.
	
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	
	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.
	
	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.
	
	3. This notice may not be removed or altered from any source
	distribution.

*********************************************************************/

#ifndef GDFILEHANDLE_H__
#define GDFILEHANDLE_H__

namespace GameData
{
    class gdDLL_EXPORT gdFileTimestamp
    {
    public:
        gdFileTimestamp();
        ~gdFileTimestamp();

        gdTime_t     GetTimestamp() const;
        void         SetTimestamp( gdTime_t val );
        void         SetPath( const gdChar* val );
        const hChar* GetPath() const;
        gdBool       IsValid() const;

        gdBool      operator == ( const gdFileTimestamp& rhs ) const ;
        gdBool      operator != ( const gdFileTimestamp& rhs ) const;

        gdDEFINE_SAVE_VERSION( 0 )

    private:

        friend class boost::serialization::access;
        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            switch ( version )
            {
            case 0:
                arc & BOOST_SERIALIZATION_NVP( path_ );
                arc & BOOST_SERIALIZATION_NVP( lastModTime_ );
            default:
                break;
            }
        }


        gdString    path_;
        gdTime_t    lastModTime_;
    };

    class gdDLL_EXPORT gdFileHandle : public Heart::hISerialiseStream
    {
    public:
        gdFileHandle();
        ~gdFileHandle();

        void                    Open( const gdChar* filename, gdBool write = false );
        gdBool                  IsValid() const { return file_ != NULL; }
        void                    Close();
        const gdFileTimestamp&  GetFileTimestamp() const;
        gdUint64                GetFileSize() const;
        const gdByte*           GetWrittenDataMD5Digest() const;
        const hChar*            GetPath() const;

        // hISerialiseStream - Methods for serialisation
        gdUint32                Read( void* pBuffer, gdUint32 size );
        gdUint32                Write( const void* pBuffer, gdUint32 size );
        gdUint32                Seek( gdUint64 offset );
        gdUint64                Tell();
        void                    SetEnableMD5Gen( gdBool val ) { md5Enabled_ = val; }

    private:
        
        FILE*                               file_;
        gdFileTimestamp                     timestamp_;  
        gdUint64                            fileSize_;
        cyMD5_CTX                           writtenDataMD5_;
        gdByte                              md5Digest_[16];
        gdBool                              md5Enabled_;
    };
}

#endif // GDFILEHANDLE_H__
