/********************************************************************
	created:	2010/08/23
	created:	23:8:2010   21:27
	filename: 	NumericalLimits.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef NUMERICALLIMITS_H__
#define NUMERICALLIMITS_H__

#include "hTypes.h"
#include <float.h>

namespace Heart
{
namespace Limits
{
	const hFloat FLOAT_MAX = FLT_MAX;
	const hFloat FLOAT_MIN = FLT_MIN;
	const hFloat FLOAT_EPSILON = FLT_EPSILON;
}
}

#endif // NUMERICALLIMITS_H__