/********************************************************************
	created:	2008/08/10
	created:	10:8:2008   23:16
	filename: 	Texture.h
	author:		James Moran
	
	purpose:	
*********************************************************************/

#ifndef hiTexture_h__
#define hiTexture_h__

#include "hTypes.h"
#include "hTextureBase.h"
#include "hRendererConstants.h"

class HeartEngine;

namespace Heart
{
	class hRenderer;

namespace Cmd
{
	class FlushTextureLevel;
}

    class hTexture : public pimpl< hdTexture >, 
                     public hResourceClassBase
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
			delete levelDescs_;
		}

        struct LevelDesc
        {
            hUint32 width_;
            hUint32 height_;
            void*   mipdata_;
        };

		virtual hUint32			Width( hUint32 level = 0 ) { hcAssert( level < nLevels_ ); return levelDescs_[ level ].width_; }
		virtual hUint32			Height( hUint32 level = 0 ) { hcAssert( level < nLevels_ ); return levelDescs_[ level ].height_; }
		//void					Lock( hUint16 level, hTextureMapInfo* info );
		//void					Unlock( hTextureMapInfo* info );
        void                    Serialise( hSerialiser* ser ) const;
        void                    Deserialise( hSerialiser* ser );

	private:

		friend class Heart::hRenderer;
		friend class TextureBuilder;
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
    }

    template<>
    inline void DeserialiseMethod< Heart::hTexture::LevelDesc >( Heart::hSerialiser* ser, Heart::hTexture::LevelDesc& data )
    {
        DESERIALISE_ELEMENT( data.width_ );
        DESERIALISE_ELEMENT( data.height_ );
        DESERIALISE_ELEMENT_INT_AS_PTR( data.mipdata_ );
    }

}

#endif // hiTexture_h__