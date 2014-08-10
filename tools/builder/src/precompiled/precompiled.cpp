/********************************************************************

	filename: 	precompiled.cpp	
	
	Copyright (c) 9:11:2012 James Moran
	
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

#include "precompiled.h"
#include "common/type_database.h"

#if 0
void escapeRegex(std::string &regex)
{
    boost::replace_all(regex, "\\", "\\\\");
    boost::replace_all(regex, "^", "\\^");
    boost::replace_all(regex, ".", "\\.");
    boost::replace_all(regex, "$", "\\$");
    boost::replace_all(regex, "|", "\\|");
    boost::replace_all(regex, "(", "\\(");
    boost::replace_all(regex, ")", "\\)");
    boost::replace_all(regex, "[", "\\[");
    boost::replace_all(regex, "]", "\\]");
    boost::replace_all(regex, "*", "\\*");
    boost::replace_all(regex, "+", "\\+");
    boost::replace_all(regex, "?", "\\?");
    boost::replace_all(regex, "/", "\\/");
}

bool wildcardMatch(const std::string &text, std::string wildcardPattern, bool caseSensitive /*= true*/)
{
    // Escape all regex special chars
    escapeRegex(wildcardPattern);

    // Convert chars '*?' back to their regex equivalents
    boost::replace_all(wildcardPattern, "\\?", ".");
    boost::replace_all(wildcardPattern, "\\*", ".*");

    boost::regex pattern(wildcardPattern, caseSensitive ? boost::regex::normal : boost::regex::icase);

    return regex_match(text, pattern);
}
#endif

//////////////////////////////////////////////////////////////////////////
//register types
DEFINE_AND_REGISTER_TYPE(Heart::proto, ServiceRegister);
DEFINE_AND_REGISTER_TYPE(Heart::proto, ServiceUnregister);
DEFINE_AND_REGISTER_TYPE(Heart::proto, ConsoleLogUpdate);
//package.proto
DEFINE_AND_REGISTER_TYPE(Heart::proto, PackageEntry);
DEFINE_AND_REGISTER_TYPE(Heart::proto, PackageHeader);
//resource_common.proto
DEFINE_AND_REGISTER_TYPE(Heart::proto, MessageContainer);
DEFINE_AND_REGISTER_TYPE(Heart::proto, Colour);
DEFINE_AND_REGISTER_TYPE(Heart::proto, ResourceSection);
DEFINE_AND_REGISTER_TYPE(Heart::proto, ResourceHeader);
//resource_material_fx.proto
DEFINE_AND_REGISTER_TYPE(Heart::proto, SamplerState);
DEFINE_AND_REGISTER_TYPE(Heart::proto, BlendState);
DEFINE_AND_REGISTER_TYPE(Heart::proto, DepthStencilState);
DEFINE_AND_REGISTER_TYPE(Heart::proto, RasterizerState);
DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialPass);
DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialTechnique);
DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialGroup);
DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialParameter);
DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialResource);
DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialSampler);
//resource_mesh.proto
DEFINE_AND_REGISTER_TYPE(Heart::proto, VertexStream);
DEFINE_AND_REGISTER_TYPE(Heart::proto, Renderable);
DEFINE_AND_REGISTER_TYPE(Heart::proto, Mesh);
//resource_shader.proto
DEFINE_AND_REGISTER_TYPE(Heart::proto, ShaderIncludeSource);
DEFINE_AND_REGISTER_TYPE(Heart::proto, ShaderInputStream);
DEFINE_AND_REGISTER_TYPE(Heart::proto, ShaderResource);
DEFINE_AND_REGISTER_TYPE(Heart::proto, ShaderResourceContainer);
//resource_texture.proto
DEFINE_AND_REGISTER_TYPE(Heart::proto, TextureMip);
DEFINE_AND_REGISTER_TYPE(Heart::proto, TextureResource);

