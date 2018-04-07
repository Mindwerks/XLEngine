#pragma once

#include <assert.h>
#include "../CommonTypes.h"

typedef int32_t fixed28_4;
typedef int32_t fixed16_16;

class Fixed16_16Math
{
public:
	static inline fixed16_16 FloatToFixed(float value);
	static inline fixed16_16 IntToFixed(int32_t value);
	static inline float FixedToFloat(fixed16_16 value);
};

inline fixed16_16 Fixed16_16Math::FloatToFixed(float value)
{
	return (fixed16_16)( value*65536.0f );
}

inline fixed16_16 Fixed16_16Math::IntToFixed(int32_t value)
{
	return (fixed16_16)( value*65536 );
}

inline float Fixed16_16Math::FixedToFloat(fixed16_16 value)
{
	return (float)value/65536.0f;
}

class Fixed28_4_Math
{
public:
	static inline fixed28_4 FloatToFixed(float value);
	static inline fixed28_4 IntToFixed(int32_t value);
	static inline float FixedToFloat(fixed28_4 value);

	static inline fixed28_4 Mul(fixed28_4 A, fixed28_4 B);
	static inline int32_t Ceil(fixed28_4 value);
	static inline void FloorDivMod(int32_t Numerator, int32_t Denominator, int32_t& Floor, int32_t& Mod);
};

inline fixed28_4 Fixed28_4_Math::FloatToFixed(float value)
{
	return (fixed28_4)( value*16.0f );
}

inline fixed28_4 Fixed28_4_Math::IntToFixed(int32_t value)
{
	return (fixed28_4)( value*16 );
}

inline float Fixed28_4_Math::FixedToFloat(fixed28_4 value)
{
	return (float)value/16.0f;
}

inline fixed28_4 Fixed28_4_Math::Mul(fixed28_4 A, fixed28_4 B)
{
	return (A*B)>>4;
}

inline int32_t Fixed28_4_Math::Ceil(fixed28_4 value)
{
	int32_t retValue;
	int32_t Numerator = value - 1 + 16;
	if ( Numerator >= 0 )
	{
		retValue = Numerator>>4;
	}
	else	//deal with negative numbers.
	{
		retValue  = -( -Numerator/16 );
		retValue -= ( (-Numerator)%16 ) ? 1 : 0;
	}
	return retValue;
}

inline void Fixed28_4_Math::FloorDivMod(int32_t Numerator, int32_t Denominator, int32_t& Floor, int32_t& Mod)
{
	assert( Denominator > 0 );
	if ( Numerator >= 0 )
	{
		//positive case.
		Floor = Numerator / Denominator;
		Mod   = Numerator % Denominator;
	}
	else
	{
		//handle negative numerator.
		Floor = -( (-Numerator) / Denominator );
		Mod   =    (-Numerator) % Denominator;
		if ( Mod )
		{
			//there is a remainder.
			Floor--;
			Mod = Denominator - Mod;
		}
	}
}
