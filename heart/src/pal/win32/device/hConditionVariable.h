/********************************************************************

    filename:   hConditionVariable.h  
    
    Copyright (c) 19:10:2013 James Moran
    
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

#ifndef HCONDITIONVARIABLE_H__
#define HCONDITIONVARIABLE_H__

namespace Heart
{
    class hdW32ConditionVariable
    {
    public:
        hdW32ConditionVariable() {
            InitializeConditionVariable(&var_);
        }
        ~hdW32ConditionVariable() {

        }
        void wait(hdW32Mutex* mtx) {
            SleepConditionVariableCS(&var_, &mtx->mutex_, INFINITE);
        }
        /* Wake a single thread waiting on CV */
        void signal() {
            WakeConditionVariable(&var_);
        }
        /* Wake ALL threads waiting on CV */
        void broadcast() {
            WakeAllConditionVariable(&var_);
        }
    private:
        hdW32ConditionVariable(const hdW32ConditionVariable&);
        hdW32ConditionVariable& operator = (const hdW32ConditionVariable&);

        CONDITION_VARIABLE var_;
    };
}

#endif // HCONDITIONVARIABLE_H__