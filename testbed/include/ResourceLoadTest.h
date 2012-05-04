/********************************************************************

	filename: 	ResourceLoadTest.h	
	
	Copyright (c) 3:12:2011 James Moran
	
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

#ifndef RESOURCELOADTEST_H__
#define RESOURCELOADTEST_H__

#include "Heart.h"
#include "hTexture.h"

class ResourceLoadTest : public Heart::hStateBase
{
public:
    ResourceLoadTest( Heart::HeartEngine* engine ) 
        : hStateBase( "ResourceLoadTest" )
        ,engine_( engine )
    {

    }
    ~ResourceLoadTest() {}

    virtual void				PreEnter();
    virtual hUint32				Enter();
    virtual void				PostEnter() {}
    virtual hUint32				Main();		
    virtual void				MainRender();
    virtual void				PreLeave();
    virtual hUint32				Leave();

private:

    Heart::HeartEngine*						engine_;
    Heart::hResourcePackage                 resPack_;
    Heart::hTexture*                        tex1_;
    Heart::hTexture*                        tex2_;
    Heart::hFont*                           font1_;
    Heart::hWorldScriptObject*              script_;
    Heart::hSoundResource*                  stream_;
    Heart::hSoundBankResource*              soundBank_;
    Heart::hSoundSource*                    soundSource_;
    Heart::hSoundSource*                    staticSource_[11];
    Heart::hIndexBuffer*                    ib_;
    Heart::hVertexBuffer*                   vb_;
    Heart::hMaterial*                       materialResource_;
    Heart::hMaterialInstance*               material_;
    Heart::hMaterialTechnique*              tech_;
    Heart::hRenderSubmissionCtx*            rndCtx_;
    Heart::hRendererCamera                  viewport_;
};

#endif // RESOURCELOADTEST_H__
