/********************************************************************

	filename: 	WorldObjectBuilder.cpp	
	
	Copyright (c) 29:3:2012 James Moran
	
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

#include "WorldObjectBuilder.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

WorldObjectBuilder::WorldObjectBuilder(const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo)
    : gdResourceBuilderBase(resBuilderInfo)
    , data_(NULL)
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

WorldObjectBuilder::~WorldObjectBuilder()
{

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WorldObjectBuilder::BuildResource()
{
    totalsize_ = 0;
    root_.xml_ = new Heart::hXMLDocument();
    ParseFile(root_, GetInputFile());

    data_ = new hByte[totalsize_+1];
    char* end = rapidxml::print((char*)data_, *root_.xml_, 0);
    *end = 0;

    GetOutputFile()->Write(data_, (hUint32)end - (hUint32)data_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WorldObjectBuilder::CleanUpFromBuild()
{
    delete data_;
    data_ = NULL;
    for (hUint32 i = 0; i < docs_.size(); ++i)
    {
        docs_[i].data_ = NULL;
        delete docs_[i].xml_;
        docs_[i].xml_ = NULL;
    }

    root_.data_ = NULL;
    delete root_.xml_;
    root_.xml_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void WorldObjectBuilder::ParseFile( XMLDoc& doc, GameData::gdFileHandle* file )
{
    hUint32 size = (hUint32)file->GetFileSize();
    totalsize_ += size;
    file->Seek(0);
    doc.data_ = new hByte[size+1];
    file->Read(doc.data_, size);
    doc.data_[size] = 0;

    // 0xEF, 0xBB, 0xBF
    if ( doc.data_[0] == 0xEF && 
         doc.data_[1] == 0xBB &&
         doc.data_[2] == 0xBF )
    {
        doc.xml_->parse<rapidxml::parse_default>((char*)doc.data_+3);
    }
    else
    {
        doc.xml_->parse<rapidxml::parse_default>((char*)doc.data_);
    }

    Heart::hXMLGetter includes = Heart::hXMLGetter(doc.xml_).FirstChild("objectscript").FirstChild("include");
    for (; includes.ToNode(); includes = includes.NextSibling())
    {
        docs_.resize(docs_.size()+1);
        XMLDoc d;
        d.xml_ = new Heart::hXMLDocument;
        GameData::gdFileHandle* incfile = OpenFile(includes.GetAttribute("input")->value());
        ParseFile(d, incfile);
        docs_[docs_.size()-1] = d;
        Heart::hXMLGetter children = Heart::hXMLGetter(docs_[docs_.size()-1].xml_).FirstChild(NULL);
        for (; children.ToNode(); children = children.NextSibling())
        {
            children.ToNode()->parent()->remove_node(children.ToNode());
            includes.ToNode()->parent()->append_node(children.ToNode());
        }
        includes.ToNode()->parent()->remove_node(includes.ToNode());
        incfile->Close();
    }
}
