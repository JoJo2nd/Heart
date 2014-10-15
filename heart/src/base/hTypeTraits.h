/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#if (_MSC_VER >= 1500) || defined (__clang__) || defined (__GNUC__)
#   include <type_traits>
   
#   define hIs_array                   std::is_array                  
#   define hIs_class                   std::is_class                  
#   define hIs_enum                    std::is_enum                   
#   define hIs_floating_point          std::is_floating_point         
#   define hIs_function                std::is_function               
#   define hIs_integral                std::is_integral       
#   define hIs_signed                  std::is_signed
#   define hIs_lvalue_reference        std::is_lvalue_reference       
#   define hIs_member_function_pointer std::is_member_function_pointer
#   define hIs_member_object_pointer   std::is_member_object_pointer  
#   define hIs_pointer                 std::is_pointer                
#   define hIs_union                   std::is_union                  
#   define hIs_void                    std::is_void                   
#   define hIs_reference               std::is_reference              
#   define hIs_const                   std::is_const                  
#   define hIs_pod                     std::is_pod                    
#   define hAlignment_of               std::alignment_of              
#   define hIs_same                    std::is_same
#   define hRemove_pointer             std::remove_pointer
#   define hRemove_const               std::remove_const
#   define hRemove_extent              std::remove_extent
    
#else
#   error ("Unsupported compiler, however using boost::type_traits is a possilbe option")
#endif