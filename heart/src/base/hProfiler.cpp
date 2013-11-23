/********************************************************************

	filename: 	hProfiler.cpp	
	
	Copyright (c) 1:4:2012 James Moran
	
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
    __declspec(selectany)
	hProfilerManager* g_ProfilerManager_;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT hProfilerManager* GetProfiler()
    {
        return g_ProfilerManager_;
    }

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hProfilerManager::hProfilerManager()
        : entryCount_(0)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hProfilerManager::~hProfilerManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hProfilerManager::BeginFrame()
	{
        inclusiveTime_ = 0;
        stackTop_ = 0;
        for (hUint32 i = 0; i < entryCount_; ++i)
        {
            entries_[i].microSecInclusive_ = 0;
            entries_[i].microSecExclusive_ = 0;
        }
	}
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hProfileEntry* hProfilerManager::CreateEntry( const hChar* tag )
    {
        hcAssertMsg(entryCount_ < s_maxProfileEntries, "Run out of profile entries, increase hProfileManager::MAX_ENTRIES");
        hProfileEntry* ret = entries_+entryCount_;
        entries_[entryCount_].tag_ = tag;
        entryCount_++;
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int excluseSort(const void* lhs, const void* rhs)
    {
        return ((hProfileEntry*)lhs)->microSecInclusive_-((hProfileEntry*)lhs)->microSecExclusive_ > ((hProfileEntry*)rhs)->microSecInclusive_-((hProfileEntry*)rhs)->microSecExclusive_ ? -1 : 1;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hProfilerManager::CopyAndSortEntries( hProfileEntry* outEntries )
    {
        hMemCpy(outEntries, entries_, sizeof(hProfileEntry)*entryCount_);
        qsort(outEntries, entryCount_, sizeof(hProfileEntry), excluseSort);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hProfilerManager::EnterScope( hProfileEntry* et )
    {
        hcAssertMsg(stackTop_ < s_maxProfileEntries, "Stack overflow");
        stack_[stackTop_++] = et;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hProfilerManager::ExitScope( hProfileEntry* et, hUint32 time )
    {
        et->microSecInclusive_ += time;

        stackTop_--;
        if (stackTop_ > 0)
            stack_[stackTop_-1]->microSecExclusive_ += time;
    }
}
#if 0
#define MAX_SAMPLE  1024*1024
#ifdef HEART_64BIT
#   define hNaked 
#elif HEART_32BIT
#   define hNaked __declspec(naked)
#endif

struct entry
{
    unsigned int   eip_func;
    unsigned int   eip_caller;
    __int64        time;
    __int64        child_time;
    int            stack_depth;
    int            stack_ptr;
};

struct ProfileHeader
{
    double m_cpu_speed;
    DWORD  m_num_samples;
    char   m_module_name[512];
};

hThreadLocal int g_count = 0;
hThreadLocal entry* g_trace_ptr = 0;
hThreadLocal bool g_trace_active = false;
hThreadLocal unsigned int g_call_stack[4096];
hThreadLocal int g_stack_idx = 0;

////////////////////////////////////////////////////////////////////////////////////////////////
static void FlushProfileSamples(const char* fname)
{
    FILE* dump_file = fopen(fname,"wb");

    ProfileHeader header;
    header.m_num_samples = g_count;
    GetModuleFileName(0,header.m_module_name,512);

    header.m_cpu_speed = 1000;//IGG::g_ProfileCon.m_cpu_speed;
    // write the module filename so we can find the symbols later
    fwrite(&header,sizeof(header),1,dump_file);

    // write the samples
    fwrite(g_trace_ptr,sizeof(entry),g_count,dump_file);
    fclose(dump_file);
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
//  _pleave()
//
//  written by: Rob Wyatt
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" void hNaked _cdecl _pexit( void ) 
{
#ifdef HEART_64BIT
#else
    _asm
    {
        sub esp,4
            push eax
            push ebx
            push ecx
            push edx
            push ebp
            push edi
            push esi

            mov ecx,g_stack_idx
            dec ecx
            mov g_stack_idx,ecx
            lea eax,[g_call_stack+ecx*8]

        mov ebx,[eax+0]
        mov edi,[eax+4]
        mov [edi+4],ebx             // store the calling functions address
            mov [esp+28],ebx

            // get the new time stamp and do a 64bit subtract from the source time
            rdtsc
            sub edx,[edi+12]
        sbb eax,[edi+8]
        mov [edi+8],eax
            mov [edi+12],edx
            mov [edi+24],ecx             // call index

            cmp ecx,1
            jl  noparent

            dec ecx
            lea ebx,[g_call_stack+ecx*8]
        mov esi,[ebx+4]
        add [esi+16],eax
            adc [esi+20],edx
noparent:
        pop esi
            pop edi
            pop ebp
            pop edx
            pop ecx
            pop ebx
            pop eax
            ret
    }
#endif // HEART_64BIT
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//  _penter()
//
//  written by: Rob Wyatt
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" void hNaked _cdecl _penter( void ) 
{
#ifdef HEART_64BIT
#else
    _asm 
    {
        cmp BYTE PTR [g_trace_active],0  // if we are not active then terminate as quickly as possible
            jne  start_profile
            ret

start_profile:
        // store all the registers
        push eax
            push ebx
            push ecx
            push edx
            push ebp
            push edi
            push esi

            mov edi,g_count
            mov eax,g_trace_ptr                      // load the pointer our data
            mov esi,edi
            shl edi,5    
            lea edi,[eax+edi]                        // destination address of where to store
        inc esi
            mov g_count,esi                           // update the sample counter

            lea ebx,[esp+28]                          // get the stack address of the return address
        mov eax,[ebx]                             // get the return address
        mov [edi+0],eax                           // store it
            mov DWORD PTR [edi+4],0                   // store the stack pointer
            rdtsc                                     // 64bit time stamp in EDX:EAX
            mov [edi+8],eax                           // store low 32 bits
            mov [edi+12],edx                          // store high 32 bits
            mov DWORD PTR[edi+16],0
            mov DWORD PTR [edi+20],0                            // zero child time
            mov DWORD PTR [edi+24],0xffffffff                   // zero child time    
            mov [edi+28],esp                          // store stack pointer
            xor ebx,ebx
            mov eax,1
            cmp esi,MAX_SAMPLE                        // if we just stored the last sample the disable tracing
            cmovge eax,ebx
            mov DWORD PTR g_trace_active,eax

            // fiddle the return address
            mov eax,g_stack_idx
            lea eax,[g_call_stack+eax*8]
        inc g_stack_idx
            mov ebx,[esp+32]
        mov [eax+0],ebx
            mov [eax+4],edi
            // send the function return back to us
            mov eax,_pleave
            mov DWORD PTR [esp+32],eax

            // restore registers and return
            pop esi
            pop edi
            pop ebp
            pop edx
            pop ecx
            pop ebx
            pop eax
            ret
    }
#endif // HEART_32BIT
}
#endif