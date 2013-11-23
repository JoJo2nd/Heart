/********************************************************************

    filename:   libprof.h  
    
    Copyright (c) 7:11:2013 James Moran
    
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
#pragma once

#ifndef LIBPROF_H__
#define LIBPROF_H__


#ifdef __cplusplus
extern "C" {
#endif

#define lpf_api         _cdecl

void lpf_api lpf_begin_thread_profile(const char* samplesname);
void lpf_api lpf_end_thread_profile(int discardresults);
int lpf_api lpf_is_thread_profiling(void);


#ifdef __cplusplus
};
#endif

#endif // LIBPROF_H__
