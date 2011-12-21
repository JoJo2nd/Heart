/********************************************************************

	filename: 	Serialiser.h	
	
	Copyright (c) 23:1:2011 James Moran
	
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

#ifndef SERIALISER_H__
#define SERIALISER_H__

class TiXmlElement;

#define DECLARE_ENUM_NAMES( x ) extern const char* enumstrings_##x []
#define BEGIN_ENUM_NAMES( en ) const char* enumstrings_##en[] = {
#define ENUM_NAME( x )  #x,
#define END_ENUM_NAMES() }

#define GET_ENUM_STRING( en, x ) enumstrings_##en[x]

inline u32 GetEnumString( const char** strings, u32 c, const char* val )
{
	for ( u32 i = 0; i < c; ++i )
	{
		if ( strcmp( strings[i], val ) == 0 )
			return i;
	}
	return c-1;
}

#define GET_ENUM_VALUE( en, v ) ((en)GetEnumString( enumstrings_##en , sizeof(enumstrings_##en)/sizeof(enumstrings_##en[0]), v ))

#define _XML_BUFFSIZE (4098)

#define XML_LINK_INT_AS_STRING( name, val, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	_itoa( val, valstr, 10 );\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_INT_AS_STRING_WITH_ID( name, val, id, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	_itoa( val, valstr, 10 );\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->SetAttribute( "id", id ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_FLOAT_AS_STRING( name, val, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	sprintf_s( valstr, _XML_BUFFSIZE, "%f", val );\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_FLOAT_AS_STRING_WITH_ID( name, val, id, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	sprintf_s( valstr, _XML_BUFFSIZE, "%f", val );\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->SetAttribute( "id", id ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_AS_STRING( name, val, parent ) \
{\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( val ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_COLOUR_AS_STRING( name, val, parent ) \
{\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	nameele->SetDoubleAttribute( "red", val.r ); \
	nameele->SetDoubleAttribute( "green", val.g ); \
	nameele->SetDoubleAttribute( "blue", val.b ); \
	nameele->SetDoubleAttribute( "alpha", val.a ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_SERIALISER_AS_ELEMENT( name, val, parent ) \
{\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	if ( !val.Serialise( nameele ) )\
	{\
		return false;\
	}\
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_SERIALISER_AS_ELEMENT_WITH_ID( name, val, id, parent ) \
{\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	nameele->SetAttribute( "id", id ); \
	if ( !val.Serialise( nameele ) )\
	{\
		return false;\
	}\
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_PSERIALISER_AS_ELEMENT( name, val, parent ) \
{\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	if ( !val->Serialise( nameele ) )\
	{\
		return false;\
	}\
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_PSERIALISER_AS_ELEMENT_WITH_ID( name, val, id, parent ) \
{\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	nameele->SetAttribute( "id", id ); \
	if ( !val->Serialise( nameele ) )\
	{\
		return false;\
	}\
	parent->LinkEndChild( nameele ); \
}


#define XML_LINK_VEC2_AS_ELEMENT( name, val, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	sprintf_s( valstr, _XML_BUFFSIZE, "%f,%f", val.x, val.y, val.z );\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_VEC2_AS_ELEMENT_WITH_ID( name, val, id, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	sprintf_s( valstr, _XML_BUFFSIZE, "%f,%f", val.x, val.y);\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->SetAttribute( "id", id ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}


#define XML_LINK_VEC3_AS_ELEMENT( name, val, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	sprintf_s( valstr, _XML_BUFFSIZE, "%f,%f,%f", val.x, val.y, val.z );\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_VEC3_AS_ELEMENT_WITH_ID( name, val, id, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	sprintf_s( valstr, _XML_BUFFSIZE, "%f,%f,%f", val.x, val.y, val.z );\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->SetAttribute( "id", id ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_VEC4_AS_ELEMENT( name, val, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	sprintf_s( valstr, _XML_BUFFSIZE, "%f,%f,%f,%f", val.x, val.y, val.z, val.w );\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_VEC4_AS_ELEMENT_WITH_ID( name, val, id, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	sprintf_s( valstr, _XML_BUFFSIZE, "%f,%f,%f,%f", val.x, val.y, val.z, val.w );\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->SetAttribute( "id", id ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_MATRIX_AS_ELEMENT( name, val, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	sprintf_s( valstr, _XML_BUFFSIZE, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", \
		val._11, val._12, val._13, val._14, \
		val._21, val._22, val._23, val._24, \
		val._31, val._32, val._33, val._34, \
		val._41, val._42, val._43, val._44);\
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_AABB_AS_ELEMENT( name, val, parent )\
{\
	char valstr[_XML_BUFFSIZE];\
		sprintf_s( valstr, _XML_BUFFSIZE, "%f,%f,%f,%f,%f,%f", \
		val.c.x, val.c.y, val.c.z, \
		val.r[0], val.r[1], val.r[2] );\
		TiXmlElement* nameele = new TiXmlElement( name ); \
		TiXmlText* textele = new TiXmlText( valstr ); \
		nameele->LinkEndChild( textele ); \
		parent->LinkEndChild( nameele ); \
}

#define XML_LINK_FACE_AS_ELEMENT_WITH_ID( name, val, id, parent ) \
{\
	char valstr[_XML_BUFFSIZE];\
	sprintf_s( valstr, _XML_BUFFSIZE, "%u,%u,%u", \
		val[id].indices_[0], val[id].indices_[1], val[id].indices_[2] ); \
	TiXmlElement* nameele = new TiXmlElement( name ); \
	TiXmlText* textele = new TiXmlText( valstr ); \
	nameele->SetAttribute( "id", id ); \
	nameele->LinkEndChild( textele ); \
	parent->LinkEndChild( nameele ); \
}

#define XML_LINK_INT( var, parent ) XML_LINK_INT_AS_STRING( #var, var, parent )
#define XML_LINK_INT_ID( var, id, parent ) XML_LINK_INT_AS_STRING_WITH_ID( #var, var[id], id, parent )
#define XML_LINK_FLOAT( var, parent ) XML_LINK_FLOAT_AS_STRING( #var, var, parent )
#define XML_LINK_FLOAT_ID( var, id, parent ) XML_LINK_FLOAT_AS_STRING_WITH_ID( #var, var[id], id, parent )
#define XML_LINK_ENUM( var, enumv, parent ) XML_LINK_AS_STRING( #var, GET_ENUM_STRING( enumv, var ), parent )
#define XML_LINK_CSTRING( var, parent ) XML_LINK_AS_STRING( #var, var, parent )
#define XML_LINK_STRING( var, parent ) XML_LINK_AS_STRING( #var, var.c_str(), parent )
#define XML_LINK_COLOUR( var, parent ) XML_LINK_COLOUR_AS_STRING( #var, var, parent )
#define XML_LINK_PTR_OBJECT( var, parent ) XML_LINK_PSERIALISER_AS_ELEMENT( #var, var, parent )
#define XML_LINK_PTR_OBJECT_ID( var, id, parent ) XML_LINK_PSERIALISER_AS_ELEMENT_WITH_ID( #var, var[id], id, parent )
#define XML_LINK_OBJECT_( var, parent ) XML_LINK_SERIALISER_AS_ELEMENT( #var, var, parent )
#define XML_LINK_OBJECT_ID( var, id, parent ) XML_LINK_SERIALISER_AS_ELEMENT_WITH_ID( #var, var[id], id, parent )
#define XML_LINK_VEC2( var, parent ) XML_LINK_VEC2_AS_ELEMENT( #var, var, parent )
#define XML_LINK_VEC2_ID( var, id, parent ) XML_LINK_VEC2_AS_ELEMENT_WITH_ID( #var, var[id], id, parent )
#define XML_LINK_VEC3( var, parent ) XML_LINK_VEC3_AS_ELEMENT( #var, var, parent )
#define XML_LINK_VEC3_ID( var, id, parent ) XML_LINK_VEC3_AS_ELEMENT_WITH_ID( #var, var[id], id, parent )
#define XML_LINK_VEC4( var, parent ) XML_LINK_VEC4_AS_ELEMENT( #var, var, parent )
#define XML_LINK_VEC4_ID( var, id, parent ) XML_LINK_VEC4_AS_ELEMENT_WITH_ID( #var, var[id], id, parent )
#define XML_LINK_MATRIX( var, parent ) XML_LINK_MATRIX_AS_ELEMENT( #var, var, parent )
#define XML_LINK_AABB( var, parent ) XML_LINK_AABB_AS_ELEMENT( #var, var, parent )
#define XML_LINK_FACE_ID( var, id, parent ) XML_LINK_FACE_AS_ELEMENT_WITH_ID( #var, var, id, parent )

#define XML_UNLINK_INT_FROM_ELEMENT( name, val, parent ) \
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( i && i->GetText() )\
	{\
		val = atoi( i->GetText() );\
	}\
	else\
		return false;\
}

#define XML_UNLINK_INT_FROM_ELEMENT_WITH_ID( name, val, parent ) \
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	int id;\
	for ( ; i; i = i->NextSiblingElement( name ) )\
	{\
		if ( i && i->GetText() && i->QueryIntAttribute( "id", &id ) == TIXML_SUCCESS )\
		{\
			val[id] = atoi( i->GetText() );\
		}\
		else\
			return false;\
	}\
}

#define XML_UNLINK_FLOAT_FROM_ELEMENT( name, val, parent ) \
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( i && i->GetText() )\
	{\
		val = atof( i->GetText() );\
	}\
	else\
		return false;\
}

#define XML_UNLINK_FLOAT_FROM_ELEMENT_WITH_ID( name, val, parent ) \
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	int id;\
	for ( ; i; i = i->NextSiblingElement( name ) )\
	{\
		if ( i && i->GetText() && i->QueryIntAttribute( "id", &id ) == TIXML_SUCCESS )\
		{\
			val[id] = atof( i->GetText() );\
		}\
		else\
			return false;\
	}\
}

#define	XML_UNLINK_AS_STRING( name, val, parent )\
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( i )\
	{\
		if ( i->GetText() )\
			val = i->GetText();\
		else\
			val = "";\
	}\
	else\
		return false;\
}

#define XML_UNLINK_COLOUR_FROM_ELEMENT( name, val, parent )\
{\
	bool ok = false;\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( i && \
		 i->QueryFloatAttribute( "red", &val.r ) == TIXML_SUCCESS &&\
		 i->QueryFloatAttribute( "green", &val.g ) == TIXML_SUCCESS &&\
		 i->QueryFloatAttribute( "blue", &val.b ) == TIXML_SUCCESS &&\
		 i->QueryFloatAttribute( "alpha", &val.a ) == TIXML_SUCCESS )\
	{\
		ok = true;\
	}\
	else\
		return false;\
}

#define XML_UNLINK_DESERIALISER_FROM_ELEMENT( name, val, parent )\
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( !val.Deserialise( i ) )\
		return false;\
}

#define XML_UNLINK_DESERIALISER_FROM_ELEMENT_WITH_ID( name, val, parent )\
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	int id;\
	for ( ; i; i = i->NextSiblingElement( name ) )\
	{\
		if ( i->QueryIntAttribute( "id", &id ) == TIXML_SUCCESS )\
			if ( !val[id].Deserialise( i ) )\
				return false;\
	}\
}

#define XML_UNLINK_PDESERIALISER_FROM_ELEMENT( name, val, parent )\
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( !val->Deserialise( i ) )\
		return false;\
}

#define XML_UNLINK_PDESERIALISER_FROM_ELEMENT_WITH_ID( name, val, parent )\
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	int id;\
	for ( ; i; i = i->NextSiblingElement( name ) )\
	{\
		if ( i->QueryIntAttribute( "id", &id ) == TIXML_SUCCESS )\
			if ( !val[id]->Deserialise( i ) )\
				return false;\
	}\
}

#define XML_UNLINK_VEC2_FROM_ELEMENT( name, val, parent ) \
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( i && i->GetText() )\
	{\
		sscanf_s( i->GetText(), " %f , %f ", &val.x, &val.y );\
	}\
	else\
		return false;\
}

#define XML_UNLINK_VEC2_FROM_ELEMENT_WITH_ID( name, val, parent ) \
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	int id;\
	for ( ; i; i = i->NextSiblingElement( name ) )\
	{\
		if ( i->QueryIntAttribute( "id", &id ) == TIXML_SUCCESS )\
		{\
			if ( i && i->GetText() )\
			{\
				sscanf_s( i->GetText(), " %f , %f ", &val[id].x, &val[id].y );\
			}\
			else\
				return false;\
		}\
		else\
			return false;\
	}\
}

#define XML_UNLINK_VEC3_FROM_ELEMENT( name, val, parent ) \
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( i && i->GetText() )\
	{\
		sscanf_s( i->GetText(), " %f , %f, %f ", &val.x, &val.y, &val.z );\
	}\
	else\
		return false;\
}

#define XML_UNLINK_VEC3_FROM_ELEMENT_WITH_ID( name, val, parent ) \
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	int id;\
	for ( ; i; i = i->NextSiblingElement( name ) )\
	{\
		if ( i->QueryIntAttribute( "id", &id ) == TIXML_SUCCESS )\
		{\
			if ( i && i->GetText() )\
			{\
				sscanf_s( i->GetText(), " %f , %f , %f ", &val[id].x, &val[id].y, &val[id].z );\
			}\
			else\
				return false;\
		}\
		else\
			return false;\
	}\
}

#define XML_UNLINK_VEC4_FROM_ELEMENT( name, val, parent ) \
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( i && i->GetText() )\
	{\
		sscanf_s( i->GetText(), " %f , %f , %f , %f ", &val.x, &val.y, &val.z, &val.w );\
	}\
	else\
		return false;\
}

#define XML_UNLINK_VEC4_FROM_ELEMENT_WITH_ID( name, val, parent ) \
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	int id;\
	for ( ; i; i = i->NextSiblingElement( name ) )\
	{\
		if ( i->QueryIntAttribute( "id", &id ) == TIXML_SUCCESS )\
		{\
			if ( i && i->GetText() )\
			{\
				sscanf_s( i->GetText(), " %f , %f , %f ", &val[id].x, &val[id].y, &val[id].z, &val[id].w );\
			}\
			else\
				return false;\
		}\
		else\
			return false;\
	}\
}

#define XML_UNLINK_MATRIX_FROM_ELEMENT(	name, val, parent )\
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( i && i->GetText() )\
	{\
		sscanf_s( i->GetText(), " %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f , %f ", \
			&val._11, &val._12, &val._13, &val._14,\
			&val._21, &val._22, &val._23, &val._24,\
			&val._31, &val._32, &val._33, &val._34,\
			&val._41, &val._42, &val._43, &val._44);\
	}\
	else\
		return false;\
}

#define XML_UNLINK_AABB_FROM_ELEMENT(name, val, parent )\
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	if ( i && i->GetText() )\
	{\
		sscanf_s( i->GetText(), " %f , %f , %f , %f , %f , %f ", \
				 &val.c.x, &val.c.y, &val.c.z,\
				 &val.r[0], &val.r[1], &val.r[2] );\
	}\
	else\
		return false;\
}

#define XML_UNLINK_ENUM_FROM_STRING( name, val, enumtype, parent ) \
{ \
	TiXmlElement* i = parent->FirstChildElement( name ); \
	if ( i && i->GetText() ) \
	{ \
		val = GET_ENUM_VALUE( enumtype, i->GetText() ); \
	}\
}

#define XML_UNLINK_FACE_FROM_ELEMENT_WITH_ID(name, val, parent )\
{\
	TiXmlElement* i = parent->FirstChildElement( name );\
	int id;\
	for ( ; i; i = i->NextSiblingElement( name ) )\
	{\
		if ( i->QueryIntAttribute( "id", &id ) == TIXML_SUCCESS )\
		{\
			if ( i && i->GetText() )\
			{\
				sscanf_s( i->GetText(), " %u , %u , %u ", \
					&val[id].indices_[0], &val[id].indices_[1], &val[id].indices_[2] );\
			}\
			else\
				return false;\
		}\
	}\
}

#define XML_UNLINK_INT( var, parent ) XML_UNLINK_INT_FROM_ELEMENT( #var, var, parent )
#define XML_UNLINK_INT_ID( var, parent ) XML_UNLINK_INT_FROM_ELEMENT_WITH_ID( #var, var, parent )
#define XML_UNLINK_FLOAT( var, parent ) XML_UNLINK_FLOAT_FROM_ELEMENT( #var, var, parent )
#define XML_UNLINK_FLOAT_ID( var, parent ) XML_UNLINK_FLOAT_FROM_ELEMENT_WITH_ID( #var, var, parent )
#define XML_UNLINK_ENUM( var, enumv, parent ) XML_UNLINK_ENUM_FROM_STRING( #var, var, enumv, parent )
#define XML_UNLINK_STRING( var, parent ) XML_UNLINK_AS_STRING( #var, var, parent )
#define XML_UNLINK_COLOUR( var, parent ) XML_UNLINK_COLOUR_FROM_ELEMENT( #var, var, parent )
#define XML_UNLINK_PTR_OBJECT( var, parent ) XML_UNLINK_PDESERIALISER_FROM_ELEMENT( #var, var, parent )
#define XML_UNLINK_PTR_OBJECT_ID( var, parent ) XML_UNLINK_PDESERIALISER_FROM_ELEMENT_WITH_ID( #var, var, parent )
#define XML_UNLINK_OBJECT_( var, parent ) XML_UNLINK_DESERIALISER_FROM_ELEMENT( #var, var, parent )
#define XML_UNLINK_OBJECT_ID( var, parent ) XML_UNLINK_DESERIALISER_FROM_ELEMENT_WITH_ID( #var, var, parent )
#define XML_UNLINK_VEC2( var, parent ) XML_UNLINK_VEC2_FROM_ELEMENT( #var, var, parent )
#define XML_UNLINK_VEC2_ID( var, parent ) XML_UNLINK_VEC2_FROM_ELEMENT_WITH_ID( #var, var, parent )
#define XML_UNLINK_VEC3( var, parent ) XML_UNLINK_VEC3_FROM_ELEMENT( #var, var, parent )
#define XML_UNLINK_VEC3_ID( var, parent ) XML_UNLINK_VEC3_FROM_ELEMENT_WITH_ID( #var, var, parent )
#define XML_UNLINK_VEC4( var, parent ) XML_UNLINK_VEC4_FROM_ELEMENT( #var, var, parent )
#define XML_UNLINK_VEC4_ID( var, parent ) XML_UNLINK_VEC4_FROM_ELEMENT_WITH_ID( #var, var, parent )
#define XML_UNLINK_MATRIX( var, parent ) XML_UNLINK_MATRIX_FROM_ELEMENT( #var, var, parent )
#define XML_UNLINK_AABB( var, parent ) XML_UNLINK_AABB_FROM_ELEMENT( #var, var, parent )
#define XML_UNLINK_FACE_ID( var, parent ) XML_UNLINK_FACE_FROM_ELEMENT_WITH_ID( #var, var, parent )


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class Serialiser
{
public:
	virtual bool	Serialise( TiXmlElement* toelement ) = 0;
	virtual bool	Deserialise( TiXmlElement* fromelement ) = 0;

};

#endif // SERIALISER_H__