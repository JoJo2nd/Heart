/********************************************************************

	filename: 	hRenderable.cpp	
	
	Copyright (c) 13:10:2012 James Moran
	
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

namespace Heart
{

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void hRenderable::SetMaterial(hMaterial* material)
{
    hcAssertMsg(material, "Material cannot be NULL");

    if (materialInstance_)
    {
        material_->DestroyMaterialInstance(materialInstance_);
        materialInstance_ = NULL;
    }

    material_ = material;
    materialInstance_ = material_->CreateMaterialInstance();
    materialKey_ = material_->GetMatKey();
}

}