/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef HCONDITIONVARIABLE_H__
#define HCONDITIONVARIABLE_H__

namespace Heart {
    class hConditionVariable
    {
    public:
        hConditionVariable() {
            InitializeConditionVariable(&var_);
        }
        ~hConditionVariable() {

        }
        hBool tryWait(hMutex* mtx) {
            return SleepConditionVariableCS(&var_, &mtx->mutex_, 0) != 0;
        }
        hBool wait(hMutex* mtx) {
            return SleepConditionVariableCS(&var_, &mtx->mutex_, INFINITE) != 0;
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
        hConditionVariable(const hConditionVariable&);
        hConditionVariable& operator = (const hConditionVariable&);

        CONDITION_VARIABLE var_;
    };
}

#endif // HCONDITIONVARIABLE_H__