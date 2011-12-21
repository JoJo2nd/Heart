/********************************************************************
	created:	2009/10/29
	created:	29:10:2009   21:12
	filename: 	hTypeList.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HCHEART_TYPELIST_H__
#define HCHEART_TYPELIST_H__

#include "hTypes.h"

#define HEART_TYPELIST_1(T1) hTypeList<T1, hNullType>

#define HEART_TYPELIST_2(T1, T2) hTypeList<T1, HEART_TYPELIST_1(T2) >

#define HEART_TYPELIST_3(T1, T2, T3) hTypeList<T1, HEART_TYPELIST_2(T2, T3) >

#define HEART_TYPELIST_4(T1, T2, T3, T4) \
	hTypeList<T1, HEART_TYPELIST_3(T2, T3, T4) >

#define HEART_TYPELIST_5(T1, T2, T3, T4, T5) \
	hTypeList<T1, HEART_TYPELIST_4(T2, T3, T4, T5) >

#define HEART_TYPELIST_6(T1, T2, T3, T4, T5, T6) \
	hTypeList<T1, HEART_TYPELIST_5(T2, T3, T4, T5, T6) >

#define HEART_TYPELIST_7(T1, T2, T3, T4, T5, T6, T7) \
	hTypeList<T1, HEART_TYPELIST_6(T2, T3, T4, T5, T6, T7) >

#define HEART_TYPELIST_8(T1, T2, T3, T4, T5, T6, T7, T8) \
	hTypeList<T1, HEART_TYPELIST_7(T2, T3, T4, T5, T6, T7, T8) >

#define HEART_TYPELIST_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) \
	hTypeList<T1, HEART_TYPELIST_8(T2, T3, T4, T5, T6, T7, T8, T9) >

#define HEART_TYPELIST_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) \
	hTypeList<T1, HEART_TYPELIST_9(T2, T3, T4, T5, T6, T7, T8, T9, T10) >

#define HEART_TYPELIST_11(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) \
	hTypeList<T1, HEART_TYPELIST_10(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) >

#define HEART_TYPELIST_12(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) \
	hTypeList<T1, HEART_TYPELIST_11(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12) >

#define HEART_TYPELIST_13(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) \
	hTypeList<T1, HEART_TYPELIST_12(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13) >

#define HEART_TYPELIST_14(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14) \
	hTypeList<T1, HEART_TYPELIST_13(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14) >

#define HEART_TYPELIST_15(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15) \
	hTypeList<T1, HEART_TYPELIST_14(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15) >

#define HEART_TYPELIST_16(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16) \
	hTypeList<T1, HEART_TYPELIST_15(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16) >

#define HEART_TYPELIST_17(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17) \
	hTypeList<T1, HEART_TYPELIST_16(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17) >

#define HEART_TYPELIST_18(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18) \
	hTypeList<T1, HEART_TYPELIST_17(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18) >

#define HEART_TYPELIST_19(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19) \
	hTypeList<T1, HEART_TYPELIST_18(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19) >

#define HEART_TYPELIST_20(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20) \
	hTypeList<T1, HEART_TYPELIST_19(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20) >

#define HEART_TYPELIST_21(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21) \
	hTypeList<T1, HEART_TYPELIST_20(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21) >

#define HEART_TYPELIST_22(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22) \
	hTypeList<T1, HEART_TYPELIST_21(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22) >

#define HEART_TYPELIST_23(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23) \
	hTypeList<T1, HEART_TYPELIST_22(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23) >

#define HEART_TYPELIST_24(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24) \
	hTypeList<T1, HEART_TYPELIST_23(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24) >

#define HEART_TYPELIST_25(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25) \
	hTypeList<T1, HEART_TYPELIST_24(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25) >

#define HEART_TYPELIST_26(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26) \
	hTypeList<T1, HEART_TYPELIST_25(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26) >

#define HEART_TYPELIST_27(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27) \
	hTypeList<T1, HEART_TYPELIST_26(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27) >

#define HEART_TYPELIST_28(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28) \
	hTypeList<T1, HEART_TYPELIST_27(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28) >

#define HEART_TYPELIST_29(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29) \
	hTypeList<T1, HEART_TYPELIST_28(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29) >

#define HEART_TYPELIST_30(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30) \
	hTypeList<T1, HEART_TYPELIST_29(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30) >

#define HEART_TYPELIST_31(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31) \
	hTypeList<T1, HEART_TYPELIST_30(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31) >

#define HEART_TYPELIST_32(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32) \
	hTypeList<T1, HEART_TYPELIST_31(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32) >

#define HEART_TYPELIST_33(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32, T33) \
	hTypeList<T1, HEART_TYPELIST_32(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32, T33) >

#define HEART_TYPELIST_34(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32, T33, T34) \
	hTypeList<T1, HEART_TYPELIST_33(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32, T33, T34) >

#define HEART_TYPELIST_35(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35) \
	hTypeList<T1, HEART_TYPELIST_34(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35) >

#define HEART_TYPELIST_36(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36) \
	hTypeList<T1, HEART_TYPELIST_35(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36) >

#define HEART_TYPELIST_37(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37) \
	hTypeList<T1, HEART_TYPELIST_36(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37) >

#define HEART_TYPELIST_38(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38) \
	hTypeList<T1, HEART_TYPELIST_37(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38) >

#define HEART_TYPELIST_39(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39) \
	hTypeList<T1, HEART_TYPELIST_38(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39) >

#define HEART_TYPELIST_40(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40) \
	hTypeList<T1, HEART_TYPELIST_39(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40) >

#define HEART_TYPELIST_41(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41) \
	hTypeList<T1, HEART_TYPELIST_40(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41) >

#define HEART_TYPELIST_42(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42) \
	hTypeList<T1, HEART_TYPELIST_41(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42) >

#define HEART_TYPELIST_43(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42, T43) \
	hTypeList<T1, HEART_TYPELIST_42(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42, T43) >

#define HEART_TYPELIST_44(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42, T43, T44) \
	hTypeList<T1, HEART_TYPELIST_43(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42, T43, T44) >

#define HEART_TYPELIST_45(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45) \
	hTypeList<T1, HEART_TYPELIST_44(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45) >

#define HEART_TYPELIST_46(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45, T46) \
	hTypeList<T1, HEART_TYPELIST_45(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45, T46) >

#define HEART_TYPELIST_47(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45, T46, T47) \
	hTypeList<T1, HEART_TYPELIST_46(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45, T46, T47) >

#define HEART_TYPELIST_48(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45, T46, T47, T48) \
	hTypeList<T1, HEART_TYPELIST_47(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45, T46, T47, T48) >

#define HEART_TYPELIST_49(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45, T46, T47, T48, T49) \
	hTypeList<T1, HEART_TYPELIST_48(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45, T46, T47, T48, T49) >

#define HEART_TYPELIST_50(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45, T46, T47, T48, T49, T50) \
	hTypeList<T1, HEART_TYPELIST_49(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
	T41, T42, T43, T44, T45, T46, T47, T48, T49, T50) >

template< typename _T, typename _U >
struct hTypeList
{
	typedef _T Head;
	typedef _U Tail;
};

namespace hTL
{
	template <class TList> 
	struct Length; 

	template <> 
	struct Length< hNullType > 
	{ 
		enum { value = 0 }; 
	};

	template < class _T, class _U > 
	struct Length< hTypeList< _T, _U > > 
	{ 
		enum { value = 1 + Length< _U >::value }; 
	};

	template < typename TList, hUint32 index > 
	struct TypeAt; 

	template< typename _T, typename _U >
	struct TypeAt< hTypeList< _T, _U >, 0 >
	{
		typedef _T Result;
	};

	template< typename _T, typename _U, hUint32 idx >
	struct TypeAt< hTypeList< _T, _U >, idx >
	{
		typedef typename TypeAt< _U, idx - 1 >::Result Result;
	};

	template <class TList, class T> struct Erase; 

	template <class T>                         // Specialization 1 
	struct Erase< hNullType, T> 
	{ 
		typedef hNullType Result; 
	}; 

	template < class T, class Tail >             // Specialization 2 
	struct Erase< hTypeList< T, Tail >, T > 
	{
		typedef Tail Result; 
	}; 

	template < class Head, class Tail, class T > // Specialization 3 
	struct Erase< hTypeList< Head, Tail >, T > 
	{ 
		typedef hTypeList< Head, 
			typename Erase< Tail, T >::Result > Result; 
	};

	template <class TList, class T> struct EraseAll; 

	template < class T > 
	struct EraseAll< hNullType, T > 
	{ 
		typedef hNullType Result; 
	}; 

	template < class T, class Tail > 
	struct EraseAll< hTypeList< T, Tail >, T > 
	{ 
		// Go all the way down the list removing the type 
		typedef typename EraseAll< Tail, T >::Result Result; 
	}; 

	template <class Head, class Tail, class T> 
	struct EraseAll< hTypeList< Head, Tail >, T > 
	{ 
		// Go all the way down the list removing the type 
		typedef hTypeList<Head, 
			typename EraseAll<Tail, T>::Result> 
			Result; 
	}; 

	template < class TList > struct NoDuplicates; 

	template <> struct NoDuplicates< hNullType > 
	{ 
		typedef hNullType Result; 
	}; 

	template <class TList> 
	struct NoDuplicates
	{
	private:
		typedef typename TList::Head Head;
		typedef typename TList::Tail Tail;

		typedef typename NoDuplicates< Tail >::Result L1;
		typedef typename Erase< L1, Head >::Result    L2;

	public:
		typedef typename hTypeList< Head, L2 > Result;
	};
}

#endif // HCHEART_TYPELIST_H__