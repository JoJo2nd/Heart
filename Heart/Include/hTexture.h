/********************************************************************

	filename: 	hTexture.h	
	
	Copyright (c) 31:3:2012 James Moran
	
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

#ifndef hiTexture_h__
#define hiTexture_h__

class TextureBuilder;

namespace Heart
{
	class hRenderer;

namespace Cmd
{
	class FlushTextureLevel;
}

    class hTexture : public hResourceClassBase,
                     public hPtrImpl< hdTexture >
	{
	public:

		hTexture( hRenderer* prenderer ) 
			: renderer_( prenderer )
            , textureData_(NULL)
			, lockPtr_(NULL)
            , levelDescs_(NULL)
		{}
		virtual ~hTexture()
		{
            ReleaseCPUTextureData();
            hDELETE_ARRAY_SAFE(hGeneralHeap, textureData_);
		}

        struct LevelDesc
        {
            hUint32 width_;
            hUint32 height_;
            void*   mipdata_;
            hUint32 mipdataSize_;
        };

		virtual hUint32			Width( hUint32 level = 0 ) { hcAssert( level < nLevels_ ); return levelDescs_[ level ].width_; }
		virtual hUint32			Height( hUint32 level = 0 ) { hcAssert( level < nLevels_ ); return levelDescs_[ level ].height_; }
        void                    ReleaseCPUTextureData();
        hColour                 ReadPixel(hUint32 x, hUint32 y);
		//void					Lock( hUint16 level, hTextureMapInfo* info );
		//void					Unlock( hTextureMapInfo* info );
        void                    Serialise( hSerialiser* ser ) const;
        void                    Deserialise( hSerialiser* ser );

	private:

		friend class Heart::hRenderer;
        friend class ::TextureBuilder;
		friend class Cmd::FlushTextureLevel;

		void					Release();
		//void					FlushLevel( hUint32 level, hUint32 inpitch, void* data );

		hTexture( const hTexture& c );
		hTexture& operator = ( const hTexture& rhs );

        hRenderer*				renderer_;
		hTextureFormat			format_;
        hUint32                 totalDataSize_;
		hByte*					textureData_;
		hUint32					nLevels_;
		LevelDesc*				levelDescs_;
		void*					lockPtr_;
	};

    struct hTextureMapInfo
    {
        void*		        ptr_;
        hUint32		        level_;
        hUint32		        pitch_;
        hTexture*           tex_;
        hdMappedData    privateDeviceData_;
    };

    template<>
    inline void SerialiseMethod< Heart::hTexture >( Heart::hSerialiser* ser, const Heart::hTexture& tex )
    {
        tex.Serialise( ser );
    }

    template<>
    inline void DeserialiseMethod< Heart::hTexture >( Heart::hSerialiser* ser, Heart::hTexture& tex )
    {
        tex.Deserialise( ser );
    }

    template<>
    inline void SerialiseMethod< Heart::hTexture::LevelDesc >( Heart::hSerialiser* ser, const Heart::hTexture::LevelDesc& data )
    {
        SERIALISE_ELEMENT( data.width_ );
        SERIALISE_ELEMENT( data.height_ );
        SERIALISE_ELEMENT_PTR_AS_INT( data.mipdata_ );
        SERIALISE_ELEMENT( data.mipdataSize_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hTexture::LevelDesc >( Heart::hSerialiser* ser, Heart::hTexture::LevelDesc& data )
    {
        DESERIALISE_ELEMENT( data.width_ );
        DESERIALISE_ELEMENT( data.height_ );
        DESERIALISE_ELEMENT_INT_AS_PTR( data.mipdata_ );
        DESERIALISE_ELEMENT( data.mipdataSize_ );
    }

}

#endif // hiTexture_h__