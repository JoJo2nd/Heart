/*
* Copyright (c) 2007, Insomniac Games
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the <organization> nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY INSOMNIAC GAMES ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL INSOMNIAC GAMES BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
////////////////////////////////////////////////////////////////////////////////////////////////
//
//  main.cpp
//
//  written by: Rob Wyatt
//
//  Tool to decode and display the results of the IGSYS function level profiler.
//
////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <algorithm>
#include "Dbghelp.h"    // for PDB symbol access

#define build_64bit (1)

BOOL CALLBACK EnumSymProc( 
    PSYMBOL_INFO pSymInfo,   
    ULONG SymbolSize,      
    PVOID UserContext)
{
    UNREFERENCED_PARAMETER(UserContext);

    printf("0x%p %4u %s\n", 
        pSymInfo->Address, SymbolSize, pSymInfo->Name);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
struct ProfileSample
{
    uintptr_t   eip_func;
    uintptr_t   eip_caller;
    __int64        time;
    __int64        child_time;
    int            stack_depth;
    int            stack_ptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////
struct ProfileHeader
{
    double      m_cpu_speed;
    uintptr_t   m_base_address;
    DWORD       m_num_samples;
    char        m_module_name[512];
    ProfileSample m_samples[0];
};

////////////////////////////////////////////////////////////////////////////////////////////////
struct ProfileSort
{
    uintptr_t           m_eip;
    uintptr_t           m_count;
    unsigned __int64    m_cycles;
    unsigned __int64    m_min;
    unsigned __int64    m_max;
};

////////////////////////////////////////////////////////////////////////////////////////////////
enum SortKey
{
    SORT_TOTAL_TIME,
    SORT_AVG_TIME,
    SORT_MIN_TIME,
    SORT_MAX_TIME,
    SORT_TOTAL_CALLS
};

////////////////////////////////////////////////////////////////////////////////////////////////
static inline float CyclesToMillis(unsigned __int64 cycles,double cpu_speed)
{
    return (float)((1000.0 * (double)cycles) / cpu_speed);
}

////////////////////////////////////////////////////////////////////////////////////////////////
static inline bool ProfileSortTotal( const ProfileSort* elem1, const ProfileSort* elem2 )
{
    return elem1->m_cycles > elem2->m_cycles;
}

////////////////////////////////////////////////////////////////////////////////////////////////
static inline bool ProfileSortAvg( const ProfileSort* elem1, const ProfileSort* elem2 )
{
    return (elem1->m_cycles/elem1->m_count) > (elem2->m_cycles/elem2->m_count);
}

////////////////////////////////////////////////////////////////////////////////////////////////
static inline bool ProfileSortMin( const ProfileSort* elem1, const ProfileSort* elem2 )
{
    return elem1->m_min > elem2->m_min;
}

////////////////////////////////////////////////////////////////////////////////////////////////
static inline bool ProfileSortMax( const ProfileSort* elem1, const ProfileSort* elem2 )
{
    return elem1->m_max > elem2->m_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////
static inline bool ProfileSortCalls( const ProfileSort* elem1, const ProfileSort* elem2 )
{
    return elem1->m_count > elem2->m_count;
}

////////////////////////////////////////////////////////////////////////////////////////////////
DWORD64 InitSymbols(char* exe,char* dir=0)
{
    SymSetOptions(SYMOPT_DEFERRED_LOADS|SYMOPT_FAIL_CRITICAL_ERRORS|SYMOPT_ALLOW_ABSOLUTE_SYMBOLS|SYMOPT_UNDNAME|SYMOPT_DEBUG|SYMOPT_LOAD_LINES|SYMOPT_INCLUDE_32BIT_MODULES);
    if (SymInitialize(GetCurrentProcess(),dir,true)==0)
        return false;

    DWORD64 baseaddress=SymLoadModuleEx(GetCurrentProcess(),0,exe,0,0,0,0,0);
    if (baseaddress==0)
    {
        return false;
    }

    IMAGEHLP_MODULE64 module={0};
    module.SizeOfStruct=sizeof(module);
    SymGetModuleInfo64(GetCurrentProcess(), baseaddress, &module);

    //SymEnumSymbols(GetCurrentProcess(), baseaddress, "*", EnumSymProc, NULL);

    return baseaddress;
}

////////////////////////////////////////////////////////////////////////////////////////////////
DWORD GetAddressOfFunction(const char* func)
{
    char buffer[2048];
    SYMBOL_INFO* info = (SYMBOL_INFO*)buffer;
    if (!SymFromName(GetCurrentProcess(),(char*)func,info))
    {
        return 0;
    }
    return (DWORD)(info->Address);
}


////////////////////////////////////////////////////////////////////////////////////////////////
void GetSymbolInfoFromAddress(DWORD64 adr, char* symbol, DWORD symbol_len, char* filename, DWORD filename_len,DWORD& line)
{
    // get the filename info
    char buff[2048];
    IMAGEHLP_LINE64 l;
    ZeroMemory(&l,sizeof(l));
    l.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    l.FileName = buff;
    l.Address = adr;
    DWORD d;
    buff[0] = 0;
    if (SymGetLineFromAddr64(GetCurrentProcess(),adr,&d,&l)==0)
    {
        line = l.LineNumber;
        if (filename && filename_len)
        {
            *filename = 0;
        }
    }
    else
    {
        if (filename && filename_len)
        {  
            if (buff[0])
            {
                strncpy(filename,l.FileName,filename_len);
                filename[filename_len-1] = 0;
            }
            else
            {
                filename[0] = 0;
            }
        }
    }

    // get the symbol info
    DWORD64 disp;
    ULONG64 buffer[(sizeof(PIMAGEHLP_SYMBOL64) +
        MAX_SYM_NAME*sizeof(TCHAR) +
        sizeof(ULONG64) - 1) /
        sizeof(ULONG64)];
    SYMBOL_INFO* pSymbol = (SYMBOL_INFO*)buffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;


    if (SymFromAddr(GetCurrentProcess(),adr,&disp,pSymbol)==0)
    {
        if (symbol && symbol_len)
        {
            sprintf(symbol,"0x%x",(DWORD)adr);
        }
    }
    else
    {
        char disp_symbol[MAX_SYM_NAME+16];
        //sprintf(disp_symbol,"%s + 0x%x",pSymbol->Name,disp);
        sprintf(disp_symbol,"%s",pSymbol->Name);

        if (symbol_len && symbol)
        {
            // copy the symbol name
            strncpy(symbol,disp_symbol,symbol_len);
            symbol[symbol_len-1] = 0;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
ProfileHeader* LoadSampleFile(const char* file)
{
    FILE* f = fopen(file,"rb");
    if (f==0)
        return 0;
    fseek(f,0,SEEK_END);
    int len = ftell(f);
    fseek(f,0,SEEK_SET);

    BYTE* data = new BYTE[len];
    fread(data,len,1,f);
    fclose(f);

    return (ProfileHeader*)data;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void FixupSampleFile(ProfileHeader* header, DWORD64 new_base_address)
{
    DWORD64 old_base_address=header->m_base_address;
    for (DWORD i=0, n=header->m_num_samples; i<n; ++i) {
        header->m_samples[i].eip_caller-=old_base_address;
        header->m_samples[i].eip_caller+=new_base_address;
        header->m_samples[i].eip_func-=old_base_address;
        header->m_samples[i].eip_func+=new_base_address;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////
void DumpInOrder(ProfileHeader* header)
{
    for (DWORD i=0;i<header->m_num_samples;i++)
    {
        char eip_symbol[2048];
        char eip_filename[2048];
        DWORD eip_line;
        char call_symbol[2048];
        char call_filename[2048];
        DWORD call_line;


        GetSymbolInfoFromAddress(header->m_samples[i].eip_func-0x05,eip_symbol,2048,eip_filename,2048,eip_line);
        if (eip_filename[0])
            printf("%s [%s:Line %d]\n",eip_symbol,eip_filename,eip_line);
        else
            printf("%s\n",eip_symbol);

        GetSymbolInfoFromAddress(header->m_samples[i].eip_caller,call_symbol,2048,call_filename,2048,call_line);
        if (call_filename[0])
            printf("Called from '%s' [%s:Line %d]\n",call_symbol,call_filename,call_line);
        else
            printf("Called from '%s'\n",call_symbol);

        printf("Stack pointer 0x%x\n",header->m_samples[i].stack_ptr);
        if (header->m_samples[i].stack_depth>=0)
        {
            // only display execution time if the entry was closed      
            printf("Local Execution time: %0.4fms [%I64d cycles]\n",CyclesToMillis(header->m_samples[i].time-header->m_samples[i].child_time,header->m_cpu_speed),header->m_samples[i].time-header->m_samples[i].child_time);
            printf("Total Execution time: %0.4fms [%I64d cycles]\n",CyclesToMillis(header->m_samples[i].time,header->m_cpu_speed),header->m_samples[i].time,header->m_cpu_speed);
        }
        else
        {
            printf("Function did not return\n");
        }
        printf("\n");
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////
void DumpCallGraph(ProfileHeader* header)
{
    for (DWORD i=0;i<header->m_num_samples;i++)
    {
        char eip_symbol[2048];
        char eip_filename[2048];
        DWORD eip_line;

        for (int s=0;s<header->m_samples[i].stack_depth;s++)
            printf(" ");

        char time_str[256];
        if (header->m_samples[i].stack_depth>=0)
        {
            unsigned __int64 ctime = header->m_samples[i].time-header->m_samples[i].child_time;
            unsigned __int64 ttime = header->m_samples[i].time;
            sprintf_s(time_str, sizeof(time_str), "%0.4fms [%I64d cycles] (%0.4fms [%I64d cycles])",
                CyclesToMillis(ctime,header->m_cpu_speed),header->m_samples[i].time-header->m_samples[i].child_time,
                CyclesToMillis(ttime,header->m_cpu_speed),header->m_samples[i].time);
        }
        else
            sprintf_s(time_str, sizeof(time_str),"Did not complete");

        GetSymbolInfoFromAddress(header->m_samples[i].eip_func-0x05,eip_symbol,2048,eip_filename,2048,eip_line);
        DWORD stack_bytes = header->m_samples[0].stack_ptr-header->m_samples[i].stack_ptr;
        printf("%s() [stack offset %d bytes] - %s\n",eip_symbol,stack_bytes,time_str);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////
void SortbyTime(ProfileHeader* header,SortKey k)
{
    std::map<uintptr_t,ProfileSort> func_map;
    std::map<uintptr_t,ProfileSort>::iterator i;

    for (DWORD t=0;t<header->m_num_samples;t++)
    {
        // we only want to time functions that completed
        if (header->m_samples[t].stack_depth<0)
            continue;

        i=func_map.find(header->m_samples[t].eip_func);
        if (i==func_map.end())
        {
            ProfileSort s;
            s.m_count = 1;
            s.m_cycles = header->m_samples[t].time-header->m_samples[t].child_time;
            s.m_min = s.m_cycles;
            s.m_max = s.m_cycles;
            s.m_eip = header->m_samples[t].eip_func;
            func_map.insert( std::pair<uintptr_t,ProfileSort>(header->m_samples[t].eip_func,s) );
        }
        else
        {
            unsigned __int64 ltime = header->m_samples[t].time-header->m_samples[t].child_time;
            (*i).second.m_count++;
            (*i).second.m_cycles += ltime;
            if (ltime>(*i).second.m_max)
                (*i).second.m_max = ltime;
            if (ltime<(*i).second.m_min)
                (*i).second.m_min = ltime;
        }    
    }

    // now make a vector of pointers which get sorted
    std::vector<ProfileSort*> func_sort;
    std::vector<ProfileSort*>::iterator i_p;
    for (i=func_map.begin();i!=func_map.end();++i)
    {
        func_sort.push_back( &((*i).second) );
    }

    if (k==SORT_TOTAL_TIME)
        std::sort(func_sort.begin(),func_sort.end(),ProfileSortTotal);
    else if (k==SORT_AVG_TIME)
        std::sort(func_sort.begin(),func_sort.end(),ProfileSortAvg);
    else if (k==SORT_MIN_TIME)
        std::sort(func_sort.begin(),func_sort.end(),ProfileSortMin);
    else if (k==SORT_MAX_TIME)
        std::sort(func_sort.begin(),func_sort.end(),ProfileSortMax);
    else if (k==SORT_TOTAL_CALLS)
        std::sort(func_sort.begin(),func_sort.end(),ProfileSortCalls);

    // now print the results
    for (i_p=func_sort.begin();i_p!=func_sort.end();++i_p)
    {
        ProfileSort* entry = (*i_p);

        char eip_symbol[2048];
        char eip_filename[2048];
        DWORD eip_line;

        GetSymbolInfoFromAddress(entry->m_eip,eip_symbol,2048,eip_filename,2048,eip_line);

        double ttime = CyclesToMillis(entry->m_cycles,header->m_cpu_speed);
        double mintime = CyclesToMillis(entry->m_min,header->m_cpu_speed);
        double maxtime = CyclesToMillis(entry->m_max,header->m_cpu_speed);
        if (eip_line==0)
        {
            printf("%s()\n",eip_symbol);
        }
        else
        {
            if (eip_filename[0])
                printf("%s() - [%s:Line %d]\n",eip_symbol,eip_filename,eip_line);
            else
                printf("%s()\n",eip_symbol);
        }
        printf("Total time       = %0.4fms [%I64d cycles], In %d calls\n",ttime,entry->m_cycles,entry->m_count);
        printf("Average per call = %0.4fms [%I64d cycles]\n",ttime/entry->m_count,entry->m_cycles/entry->m_count);
        printf("Min time         = %0.4fms [%I64d cycles]\n",mintime,entry->m_min);
        printf("Max time         = %0.4fms [%I64d cycles]\n",maxtime,entry->m_max);
        printf("\n");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolInfo(ProfileHeader* header, const char* symbol)
{
    // get the address of the specified symbol
    DWORD adr = GetAddressOfFunction(symbol);
    if (adr==0)
    {
        printf("Symbol %s not found\n");
        return;
    }

    std::vector<uintptr_t> called_from;
    DWORD count=0;
    unsigned __int64 total_time = 0;
    for (DWORD t=0;t<header->m_num_samples;t++)
    {
        if (header->m_samples[t].eip_func-5 == adr)
        {
            // we have a hit on the function
            unsigned __int64 ttime = header->m_samples[t].time-header->m_samples[t].child_time;
            count++;
            called_from.push_back(header->m_samples[t].eip_caller);
            total_time += ttime;;
        }
    }

    char eip_symbol[2048];
    char eip_filename[2048];
    DWORD eip_line;
    printf("%s() is called %d times\n",symbol,count);
    GetSymbolInfoFromAddress(adr,eip_symbol,2048,eip_filename,2048,eip_line);

    if (count>0)
    {
        printf("From the following functions:\n");
        for (DWORD i=0;i<count;i++)
        {
            GetSymbolInfoFromAddress(called_from[i],eip_symbol,2048,eip_filename,2048,eip_line);
            if (eip_filename[0])
                printf("   %s [%s:line %d]\n",eip_symbol,eip_filename,eip_line);
            else
                printf("   %s\n",eip_symbol);
        }

        printf("Total time consumed by %d calls to '%s': %.4fms [%I64d cycles]\n",count,symbol,CyclesToMillis(total_time,header->m_cpu_speed),total_time);
        printf("Average Time: %.4fms [%I64d cycles]\n",CyclesToMillis(total_time,header->m_cpu_speed)/count,total_time/count);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
void DumpInfo(ProfileHeader* header)
{
    printf("Captured %d samples from '%s'\n",header->m_num_samples, header->m_module_name);
    printf("CPU Speed = %.0f Hz\n",header->m_cpu_speed);
    printf("\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    if (argc<2)
    {
        printf("Usage: igprofiler <sample_data_file> [options]\n");
        return -1;
    }

    char* sample_file = argv[1];

    ProfileHeader* header = LoadSampleFile(sample_file);  
    if (header==0)
    {
        printf("Cannot load sample data file '%s'\n",sample_file);
        return -1;
    }

    DumpInfo(header);

    DWORD64 base_address=InitSymbols(header->m_module_name,0);

    FixupSampleFile(header, base_address);

    if (argc>2)
    {
        for (int arg = 2; arg < argc; )
        {
            if (stricmp(argv[arg],"-callgraph")==0)
            {
                DumpCallGraph(header);
            }
            else if (stricmp(argv[arg],"-total_time")==0)
            {
                SortbyTime(header,SORT_TOTAL_TIME);
            }
            else if (stricmp(argv[arg],"-avg_time")==0)
            {
                SortbyTime(header,SORT_AVG_TIME);
            }
            else if (stricmp(argv[arg],"-min_time")==0)
            {
                SortbyTime(header,SORT_MIN_TIME);
            }
            else if (stricmp(argv[arg],"-max_time")==0)
            {
                SortbyTime(header,SORT_MAX_TIME);
            }
            else if (stricmp(argv[arg],"-calls")==0)
            {
                SortbyTime(header,SORT_TOTAL_CALLS);
            }
            else if (stricmp(argv[arg],"-query")==0)
            {
                if (arg<argc)
                {
                    arg++;
                    SymbolInfo(header,argv[arg]);
                }
            }

            arg++;
        }
    }
    else
    {
        // just dump the file in text form
        DumpInOrder(header);
    }
}

