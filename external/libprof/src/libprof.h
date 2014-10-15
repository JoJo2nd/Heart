/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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
