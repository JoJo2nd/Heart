/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef BASE64TEST_H__
#define BASE64TEST_H__

class Base64 : public IUnitTest
{
    DECLARE_HEART_UNIT_TEST();
public:
    Base64( Heart::hHeartEngine* engine ) 
        : IUnitTest( engine )
        , doTest_(hTrue)
    {
    }
    ~Base64() {}

    virtual hUint32 RunUnitTest()
    {
        if (doTest_)
        {
            hUint data1[201];
            hUint data2[200];
            hUint data3[199];
            hChar encode1[268*sizeof(hUint)];
            hChar encode2[267*sizeof(hUint)];
            hChar encode3[266*sizeof(hUint)];
            hUint decode1[201];
            hUint decode2[200];
            hUint decode3[199];
            hUint needdecodesize;
            hUint needencodesize;

            hcPrintf("%s Generating Data", __FUNCTION__);

            GenRandomNumbers(data1, (hUint)hStaticArraySize(data1));
            GenRandomNumbers(data2, (hUint)hStaticArraySize(data2));
            GenRandomNumbers(data3, (hUint)hStaticArraySize(data3));

            hcPrintf("%s Encoding Data", __FUNCTION__);

            needencodesize=Heart::hBase64::EncodeCalcRequiredSize(sizeof(data1));
            hcAssert(needencodesize==sizeof(encode1));
            Heart::hBase64::Encode(data1, sizeof(data1), encode1, needencodesize);

            needencodesize=Heart::hBase64::EncodeCalcRequiredSize(sizeof(data2));
            hcAssert(needencodesize==sizeof(encode2));
            Heart::hBase64::Encode(data2, sizeof(data2), encode2, needencodesize);

            needencodesize=Heart::hBase64::EncodeCalcRequiredSize(sizeof(data3));
            hcAssert(needencodesize==sizeof(encode3));
            Heart::hBase64::Encode(data3, sizeof(data3), encode3, needencodesize);

            hcPrintf("%s Decoding Data",__FUNCTION__);

            needdecodesize=Heart::hBase64::DecodeCalcRequiredSize(encode1, sizeof(encode1));
            hcAssert(needdecodesize==sizeof(decode1));
            Heart::hBase64::Decode(encode1, sizeof(encode1), decode1, sizeof(decode1));
            hcAssert(memcmp(data1, decode1, sizeof(decode1)) == 0);

            needdecodesize=Heart::hBase64::DecodeCalcRequiredSize(encode2, sizeof(encode2));
            hcAssert(needdecodesize==sizeof(decode2));
            Heart::hBase64::Decode(encode2, sizeof(encode2), decode2, sizeof(decode2));
            hcAssert(memcmp(data2, decode2, sizeof(decode2)) == 0);

            needdecodesize=Heart::hBase64::DecodeCalcRequiredSize(encode3, sizeof(encode3));
            hcAssert(needdecodesize==sizeof(decode3));
            Heart::hBase64::Decode(encode3, sizeof(encode3), decode3, sizeof(decode3));
            hcAssert(memcmp(data3, decode3, sizeof(decode3)) == 0);

            hcPrintf("%s Test Complete",__FUNCTION__);

            doTest_=hFalse;
        }

        if (timer_.elapsedSec() > 30 || getForceExitFlag()) {
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }

        return 0;
    }

    void GenRandomNumbers(hUint* t, hUint tc) 
    {
        for ( hUint i = 0; i < tc; ++i )
        {
            t[i] = rand();
        }
    }


private:
    hBool          doTest_;
    Heart::hTimer  timer_;
};

#endif // BASE64TEST_H__