/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "OGGSoundBankBuilder.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

OGGSoundBankBuilder::OGGSoundBankBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo )
    : gdResourceBuilderBase( resBuilderInfo )
    , xmlsize_(0)
    , xmlDocStr_(NULL)
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

OGGSoundBankBuilder::~OGGSoundBankBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void OGGSoundBankBuilder::BuildResource()
{
    struct SoundBank
    {
        std::string name_;
        std::string input_;
        hByte* data_;
        hUint32 datasize_;
    };
    Heart::hSoundBankResource soundBank;
    Heart::hXMLDocument inputxml;
    std::vector< SoundBank > banks;
    soundBank.sourceDataSize_ = 0;
    soundBank.namePoolSize_ = 0;

    xmlsize_ = (hUint32)GetInputFile()->GetFileSize();
    xmlDocStr_ = (hChar*)hMalloc(xmlsize_+1);
    GetInputFile()->Read( xmlDocStr_, xmlsize_ );
    xmlDocStr_[xmlsize_] = 0;

    try
    {
        inputxml.parse<rapidxml::parse_default>( xmlDocStr_ );
    }
    catch (...)
    {
    	ThrowFatalError("Failed to parse sound bank xml");
    }

    soundBank.namePoolSize_ = 0;

    for ( Heart::hXMLGetter sound = Heart::hXMLGetter(&inputxml).FirstChild("sound"); sound.ToNode(); sound = sound.NextSibling() )
    {
        SoundBank sbank;
        rapidxml::xml_attribute<>* sname = sound.GetAttribute("name");
        rapidxml::xml_attribute<>* sfile = sound.GetAttribute("file");
        if ( !sname || !sfile )
        {
            ThrowFatalError("Couldn't find name or file for sound bank sound");
        }
        soundBank.namePoolSize_ += sname->value_size()+1;

        sbank.name_ = sname->value();
        sbank.input_ = sfile->value();
        banks.push_back( sbank );
    }

    hUint32 banksize = banks.size();
    for ( hUint32 i = 0; i < banks.size(); ++i )
    {
        GameData::gdFileHandle* file = OpenFile( banks[i].input_.c_str() );
        if ( !file->IsValid() )
        {
            ThrowFatalError( "Couldn't open sound file %s", banks[i].input_.c_str() );
        }
        banks[i].datasize_ = (hUint32)file->GetFileSize();
        banks[i].data_ = new hGeneralHeap, hByte[banks[i].datasize_];
        file->Read( banks[i].data_, banks[i].datasize_ );

        soundBank.sourceDataSize_ += banks[i].datasize_;

        CloseFile( file );
    }

    soundBank.namePool_ = new hChar[soundBank.namePoolSize_];
    soundBank.sourcesData_ = new hByte[soundBank.sourceDataSize_];
    soundBank.sourceCount_ = banksize;
    soundBank.sources_ = new Heart::hStaticSoundSource[soundBank.sourceCount_];

    hChar* namestr = soundBank.namePool_;
    hByte* source = soundBank.sourcesData_;

    for ( hUint32 i = 0; i < banks.size(); ++i )
    {
        soundBank.sources_[i].nameLen_ = banks[i].name_.length();
        soundBank.sources_[i].name_ = (hChar*)((hUint32)namestr - (hUint32)soundBank.namePool_);
        soundBank.sources_[i].oggDataSize_ = banks[i].datasize_;
        soundBank.sources_[i].oggData_ = (hByte*)((hUint32)source - (hUint32)soundBank.sourcesData_);
        
        Heart::hStrCopy( namestr, banks[i].name_.length()+1, banks[i].name_.c_str() );
        namestr += banks[i].name_.length()+1;
        Heart::hMemCpy( source, banks[i].data_, banks[i].datasize_ );
        source += banks[i].datasize_;
    }

    GetSerilaiseObject()->Serialise( GetOutputFile(), soundBank );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void OGGSoundBankBuilder::CleanUpFromBuild()
{
    hFree(xmlDocStr_);
}
