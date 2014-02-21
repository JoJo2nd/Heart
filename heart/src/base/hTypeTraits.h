/********************************************************************

    filename:   hTypeTraits.h  
    
    Copyright (c) 9:1:2013 James Moran
    
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

#if (_MSC_VER >= 1500)
#   include <type_traits>
   
#define hIs_array                   std::is_array                  
#define hIs_class                   std::is_class                  
#define hIs_enum                    std::is_enum                   
#define hIs_floating_point          std::is_floating_point         
#define hIs_function                std::is_function               
#define hIs_integral                std::is_integral       
#define hIs_signed                  std::is_signed
#define hIs_lvalue_reference        std::is_lvalue_reference       
#define hIs_member_function_pointer std::is_member_function_pointer
#define hIs_member_object_pointer   std::is_member_object_pointer  
#define hIs_pointer                 std::is_pointer                
#define hIs_union                   std::is_union                  
#define hIs_void                    std::is_void                   
#define hIs_reference               std::is_reference              
#define hIs_const                   std::is_const                  
#define hIs_pod                     std::is_pod                    
#define hAlignment_of               std::alignment_of              
#define hIs_same                    std::is_same
#define hRemove_pointer             std::remove_pointer
#define hRemove_const               std::remove_const
#define hRemove_extent              std::remove_extent

#else
#   pragma error ("Unsupported compiler, however using boost::type_traits is a possilbe option")
#endif