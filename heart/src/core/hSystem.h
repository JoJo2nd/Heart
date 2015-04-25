/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef hoWindow_h__
#define hoWindow_h__

#include "base/hTypes.h"
#include "core/hDeviceSystemWindow.h"

namespace Heart
{

    class hSystem : public hdSystemWindow
    {
    public:
    	
	    hSystem() 
	    {
	    }

	    hBool	Create(const HeartConfig& config);
	    void	Update();
	    void	Destroy();

    private:

    };

}


#endif // hoWindow_h__
