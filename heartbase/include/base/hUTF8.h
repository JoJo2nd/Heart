/********************************************************************

	filename: 	hUTF8.h	
	
	Copyright (c) 5:9:2010 James Moran
	
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
namespace hUTF8
{
	static const hUint32 MASKBITS   = 0x3F;
	static const hUint32 MASKBYTE   = 0x80;
	static const hUint32 MASK2BYTES = 0xC0;
	static const hUint32 MASK3BYTES = 0xE0;
	static const hUint32 MASK4BYTES = 0xF0;
	static const hUint32 MASK5BYTES = 0xF8;
	static const hUint32 MASK6BYTES = 0xFC;

	typedef hUint16   Unicode;

    HEARTBASE_SLIBEXPORT
	hUint32 EncodeFromUnicode( Unicode ucIn, hChar* utf8Out );
	HEARTBASE_SLIBEXPORT
    hUint32 HEART_API DecodeToUnicode( const hChar* uft8In, Unicode& ucOut );

}
}