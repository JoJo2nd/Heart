/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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

