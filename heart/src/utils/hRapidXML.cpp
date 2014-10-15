/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "utils/hRapidXML.h"
#include <exception>

namespace rapidxml
{
    //! When exceptions are disabled by defining RAPIDXML_NO_EXCEPTIONS, 
    //! this function is called to notify user about the error.
    //! It must be defined by the user.
    //! <br><br>
    //! This function cannot return. If it does, the results are undefined.
    //! <br><br>
    //! A very simple definition might look like that:
    //! <pre>
    //! void %rapidxml::%parse_error_handler(const char *what, void *where)
    //! {
    //!     std::cout << "Parse error: " << what << "\n";
    //!     std::abort();
    //! }
    //! </pre>
    //! \param what Human readable description of the error.
    //! \param where Pointer to character data where error was detected.
    void HEART_API parse_error_handler(const char *what, void *where)
    {
        hcPrintf( "XML Parse Error: %s", what );
        throw std::exception();
    }
}
namespace Heart
{
     void* HEART_API hXML_alloc_func(size_t size )
    {
        return hMalloc( size );
    }
     void HEART_API hXML_free_func(void* ptr )
    {
        hFree( ptr );
    }
}