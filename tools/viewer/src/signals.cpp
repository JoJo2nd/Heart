/********************************************************************

    filename:   signals.cpp  
    
    Copyright (c) 28:12:2012 James Moran
    
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

boost::signals2::signal< void (const hChar*) > evt_consoleOutputSignal;
boost::signals2::signal< void (const hChar*) > evt_consoleInputSignal;
boost::signals2::signal< void () > evt_mainWindowCreate;
boost::signals2::signal< void (wxWindow*, const wxString&, const wxAuiPaneInfo&) > evt_registerAuiPane;