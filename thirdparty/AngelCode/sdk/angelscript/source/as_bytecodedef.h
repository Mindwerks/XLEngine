/*
   AngelCode Scripting Library
   Copyright (c) 2003-2007 Andreas Jonsson

   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/


//
// as_bytecodedef.h
//
// Byte code definitions
//


#ifndef AS_BYTECODEDEF_H
#define AS_BYTECODEDEF_H

#include "as_config.h"

BEGIN_AS_NAMESPACE

//---------------------------------------------
// Byte code instructions

enum bcInstr
{
	// Unsorted
	BC_POP			= 0,	// Decrease stack size
	BC_PUSH			= 1,	// Increase stack size
	BC_PshC4		= 2,	// Push constant on stack
	BC_PshV4		= 3,	// Push value in variable on stack
	BC_PSF			= 4,	// Push stack frame
	BC_SWAP4		= 5,	// Swap top two dwords
	BC_NOT			= 6,    // Boolean not operator for a variable
	BC_PshG4		= 7,	// Push value in global variable on stack
	BC_LdGRdR4	    = 8,    // Same as LDG, RDR4
	BC_CALL			= 9,	// Call function
	BC_RET			= 10,	// Return from function
	BC_JMP			= 11,

	// Conditional jumps
	BC_JZ			= 12,
	BC_JNZ			= 13,
	BC_JS			= 14,	// Same as TS+JNZ or TNS+JZ
	BC_JNS			= 15,	// Same as TNS+JNZ or TS+JZ
	BC_JP			= 16,	// Same as TP+JNZ or TNP+JZ
	BC_JNP			= 17,	// Same as TNP+JNZ or TP+JZ

	// Test value
	BC_TZ			= 18,	// Test if zero
	BC_TNZ			= 19,	// Test if not zero
	BC_TS			= 20,	// Test if signaled (less than zero)
	BC_TNS			= 21,	// Test if not signaled (zero or greater)
	BC_TP			= 22,	// Test if positive (greater than zero)
	BC_TNP			= 23,	// Test if not positive (zero or less)

	// Negate value
	BC_NEGi			= 24,
	BC_NEGf			= 25,
	BC_NEGd			= 26,

	// Increment value pointed to by address in register
	BC_INCi16		= 27,
	BC_INCi8    	= 28,
	BC_DECi16   	= 29,
	BC_DECi8    	= 30, 
	BC_INCi			= 31,
	BC_DECi			= 32,
	BC_INCf			= 33,
	BC_DECf			= 34,
	BC_INCd     	= 35,
	BC_DECd     	= 36,

	// Increment variable
	BC_IncVi		= 37,
	BC_DecVi		= 38,

	// Bitwise operations
	BC_BNOT			= 39,
	BC_BAND			= 40,
	BC_BOR			= 41,
	BC_BXOR			= 42,
	BC_BSLL			= 43,
	BC_BSRL			= 44,
	BC_BSRA			= 45,

	// Unsorted
	BC_COPY			= 46,	// Do byte-for-byte copy of object
	BC_SET8			= 47,	// Push QWORD on stack
	BC_RDS8			= 48,	// Read value from address on stack onto the top of the stack
	BC_SWAP8		= 49,

	// Comparisons
	BC_CMPd     	= 50,
	BC_CMPu			= 51,
	BC_CMPf			= 52,
	BC_CMPi			= 53,

	// Comparisons with constant value
	BC_CMPIi		= 54,
	BC_CMPIf		= 55,
	BC_CMPIu        = 56,

	BC_JMPP     	= 57,	// Jump with offset in variable
	BC_PopRPtr    	= 58,	// Pop address from stack into register
	BC_PshRPtr    	= 59,	// Push address from register on stack
	BC_STR      	= 60,	// Push string address and length on stack
	BC_CALLSYS  	= 61,
	BC_CALLBND  	= 62,
	BC_SUSPEND  	= 63,
	BC_ALLOC    	= 64,
	BC_FREE     	= 65,
	BC_LOADOBJ		= 66,
	BC_STOREOBJ  	= 67,
	BC_GETOBJ    	= 68,
	BC_REFCPY    	= 69,
	BC_CHKREF    	= 70,
	BC_GETOBJREF 	= 71,
	BC_GETREF    	= 72,
	BC_SWAP48    	= 73,
	BC_SWAP84    	= 74,
	BC_OBJTYPE   	= 75,
	BC_TYPEID    	= 76,
	BC_SetV4		= 77,	// Initialize the variable with a DWORD
	BC_SetV8		= 78,	// Initialize the variable with a QWORD
	BC_ADDSi		= 79,	// Add arg to value on stack
	BC_CpyVtoV4		= 80,	// Copy value from one variable to another
	BC_CpyVtoV8		= 81,	
	BC_CpyVtoR4     = 82,	// Copy value from variable into register
	BC_CpyVtoR8		= 83,	// Copy value from variable into register
	BC_CpyVtoG4     = 84,   // Write the value of a variable to a global variable (LDG, WRTV4)
	BC_CpyRtoV4     = 85,   // Copy the value from the register to the variable
	BC_CpyRtoV8     = 86,
	BC_CpyGtoV4     = 87,   // Copy the value of the global variable to a local variable (LDG, RDR4)
	BC_WRTV1        = 88,	// Copy value from variable to address held in register
	BC_WRTV2        = 89,
	BC_WRTV4        = 90,
	BC_WRTV8        = 91,
	BC_RDR1         = 92,	// Read value from address in register and store in variable
	BC_RDR2         = 93,
	BC_RDR4         = 94,	
	BC_RDR8         = 95,
	BC_LDG          = 96,	// Load the register with the address of the global attribute
	BC_LDV          = 97,	// Load the register with the address of the variable
	BC_PGA          = 98,
	BC_RDS4         = 99,	// Read value from address on stack onto the top of the stack
	BC_VAR          = 100,	// Push the variable offset on the stack

	// Type conversions
	BC_iTOf			= 101,
	BC_fTOi			= 102,
	BC_uTOf			= 103,
	BC_fTOu			= 104,
	BC_sbTOi		= 105,	// Signed byte
	BC_swTOi		= 106,	// Signed word
	BC_ubTOi		= 107,	// Unsigned byte
	BC_uwTOi		= 108,	// Unsigned word
	BC_dTOi     	= 109,
	BC_dTOu     	= 110,
	BC_dTOf     	= 111,
	BC_iTOd     	= 112,
	BC_uTOd     	= 113,
	BC_fTOd     	= 114,

	// Math operations
	BC_ADDi			= 115,
	BC_SUBi			= 116,
	BC_MULi			= 117,
	BC_DIVi			= 118,
	BC_MODi			= 119,
	BC_ADDf			= 120,
	BC_SUBf			= 121,
	BC_MULf			= 122,
	BC_DIVf			= 123,
	BC_MODf			= 124,
	BC_ADDd     	= 125,
	BC_SUBd     	= 126,
	BC_MULd     	= 127,
	BC_DIVd     	= 128,
	BC_MODd     	= 129,

	// Math operations with constant value
	BC_ADDIi        = 130,
	BC_SUBIi        = 131,
	BC_MULIi        = 132,
	BC_ADDIf        = 133,
	BC_SUBIf        = 134,
	BC_MULIf        = 135,

	BC_SetG4		= 136,	// Initialize the global variable with a DWORD
	BC_ChkRefS      = 137,  // Verify that the reference to the handle on the stack is not null
	BC_ChkNullV     = 138,  // Verify that the variable is not a null handle
	BC_CALLINTF  	= 139,	// Call interface method 

	BC_iTOb         = 140,
	BC_iTOw         = 141,
	BC_SetV1        = 142,
	BC_SetV2        = 143,
	BC_Cast         = 144,	// Cast handle type to another handle type

	BC_i64TOi       = 145,
	BC_uTOi64       = 146,
	BC_iTOi64       = 147,
	BC_fTOi64       = 148,
	BC_dTOi64       = 149,
	BC_fTOu64       = 150,
	BC_dTOu64       = 151,
	BC_i64TOf       = 152,
	BC_u64TOf       = 153,
	BC_i64TOd       = 154,
	BC_u64TOd       = 155,
	BC_NEGi64       = 156,
	BC_INCi64       = 157,
	BC_DECi64       = 158,
	BC_BNOT64       = 159,

	BC_ADDi64   	= 160,
	BC_SUBi64   	= 161,
	BC_MULi64   	= 162,
	BC_DIVi64		= 163,
	BC_MODi64		= 164,
	BC_BAND64		= 165,
	BC_BOR64		= 166,
	BC_BXOR64		= 167,
	BC_BSLL64		= 168,
	BC_BSRL64		= 169,
	BC_BSRA64		= 170,
	BC_CMPi64       = 171,
	BC_CMPu64       = 172,
	
	BC_ChkNullS     = 173,
	BC_ClrHi        = 174,

	BC_MAXBYTECODE  = 175,

	// Temporary tokens, can't be output to the final program
	BC_PSP			= 246,
	BC_LINE			= 248,
	BC_LABEL		= 255
};

#ifdef AS_64BIT_PTR
	#define BC_RDSPTR BC_RDS8
#else
	#define BC_RDSPTR BC_RDS4
#endif


//------------------------------------------------------------
// Relocation Table
extern asDWORD relocTable[BC_MAXBYTECODE];

//------------------------------------------------------------
// Instruction sizes
const int BCTYPE_INFO         = 0;
const int BCTYPE_NO_ARG       = 1;
const int BCTYPE_W_ARG        = 2;
const int BCTYPE_wW_ARG       = 3;
const int BCTYPE_DW_ARG       = 4;
const int BCTYPE_rW_DW_ARG    = 5;
const int BCTYPE_QW_ARG       = 6;
const int BCTYPE_DW_DW_ARG    = 7;
const int BCTYPE_wW_rW_rW_ARG = 8;
const int BCTYPE_wW_QW_ARG    = 9;
const int BCTYPE_wW_rW_ARG    = 10;
const int BCTYPE_rW_ARG       = 11;
const int BCTYPE_wW_DW_ARG    = 12;
const int BCTYPE_wW_rW_DW_ARG = 13;
const int BCTYPE_rW_rW_ARG    = 14;
const int BCTYPE_W_rW_ARG     = 15;
const int BCTYPE_wW_W_ARG     = 16;
const int BCTYPE_W_DW_ARG     = 17;
const int BCTYPE_QW_DW_ARG    = 18;

#ifndef AS_64BIT_PTR
	#define BCTYPE_PTR_ARG    BCTYPE_DW_ARG
	#define BCTYPE_PTR_DW_ARG BCTYPE_DW_DW_ARG
#else
	#define BCTYPE_PTR_ARG    BCTYPE_QW_ARG
	#define BCTYPE_PTR_DW_ARG BCTYPE_QW_DW_ARG
#endif

const int BCT_POP       = BCTYPE_W_ARG;       
const int BCT_PUSH      = BCTYPE_W_ARG;
const int BCT_PshC4     = BCTYPE_DW_ARG;
const int BCT_PshV4     = BCTYPE_rW_ARG;
const int BCT_PSF       = BCTYPE_rW_ARG;
const int BCT_SWAP4     = BCTYPE_NO_ARG;
const int BCT_NOT       = BCTYPE_rW_ARG;
const int BCT_PshG4     = BCTYPE_W_ARG;
const int BCT_LdGRdR4   = BCTYPE_wW_W_ARG;
const int BCT_CALL      = BCTYPE_DW_ARG;
const int BCT_RET       = BCTYPE_W_ARG;
const int BCT_JMP       = BCTYPE_DW_ARG;

const int BCT_JZ        = BCTYPE_DW_ARG;
const int BCT_JNZ       = BCTYPE_DW_ARG;
const int BCT_JS        = BCTYPE_DW_ARG;
const int BCT_JNS       = BCTYPE_DW_ARG;
const int BCT_JP        = BCTYPE_DW_ARG;
const int BCT_JNP       = BCTYPE_DW_ARG;

const int BCT_TZ        = BCTYPE_NO_ARG;
const int BCT_TNZ       = BCTYPE_NO_ARG;
const int BCT_TS        = BCTYPE_NO_ARG;
const int BCT_TNS       = BCTYPE_NO_ARG;
const int BCT_TP        = BCTYPE_NO_ARG;
const int BCT_TNP       = BCTYPE_NO_ARG;

const int BCT_NEGi      = BCTYPE_rW_ARG;
const int BCT_NEGf      = BCTYPE_rW_ARG;
const int BCT_NEGd      = BCTYPE_rW_ARG;

const int BCT_INCi16    = BCTYPE_NO_ARG;
const int BCT_INCi8     = BCTYPE_NO_ARG;
const int BCT_DECi16    = BCTYPE_NO_ARG;
const int BCT_DECi8     = BCTYPE_NO_ARG;
const int BCT_INCi      = BCTYPE_NO_ARG;
const int BCT_DECi      = BCTYPE_NO_ARG;
const int BCT_INCf      = BCTYPE_NO_ARG;
const int BCT_DECf      = BCTYPE_NO_ARG;
const int BCT_INCd      = BCTYPE_NO_ARG;
const int BCT_DECd      = BCTYPE_NO_ARG;

const int BCT_IncVi     = BCTYPE_rW_ARG;
const int BCT_DecVi     = BCTYPE_rW_ARG;

const int BCT_BNOT      = BCTYPE_rW_ARG;
const int BCT_BAND      = BCTYPE_wW_rW_rW_ARG;
const int BCT_BOR       = BCTYPE_wW_rW_rW_ARG;
const int BCT_BXOR      = BCTYPE_wW_rW_rW_ARG;
const int BCT_BSLL      = BCTYPE_wW_rW_rW_ARG;
const int BCT_BSRL      = BCTYPE_wW_rW_rW_ARG;
const int BCT_BSRA      = BCTYPE_wW_rW_rW_ARG;

const int BCT_COPY      = BCTYPE_W_ARG;
const int BCT_SET8      = BCTYPE_QW_ARG;
const int BCT_RDS8      = BCTYPE_NO_ARG;
const int BCT_SWAP8     = BCTYPE_NO_ARG;

const int BCT_CMPd      = BCTYPE_rW_rW_ARG;
const int BCT_CMPu      = BCTYPE_rW_rW_ARG;
const int BCT_CMPf      = BCTYPE_rW_rW_ARG;
const int BCT_CMPi      = BCTYPE_rW_rW_ARG;
const int BCT_CMPIi     = BCTYPE_rW_DW_ARG;
const int BCT_CMPIf     = BCTYPE_rW_DW_ARG;
const int BCT_CMPIu     = BCTYPE_rW_DW_ARG;

const int BCT_JMPP      = BCTYPE_rW_ARG;
const int BCT_PopRPtr   = BCTYPE_NO_ARG;
const int BCT_PshRPtr   = BCTYPE_NO_ARG;
const int BCT_STR       = BCTYPE_W_ARG;
const int BCT_CALLSYS   = BCTYPE_DW_ARG;
const int BCT_CALLBND   = BCTYPE_DW_ARG;
const int BCT_SUSPEND   = BCTYPE_NO_ARG;
const int BCT_ALLOC     = BCTYPE_PTR_DW_ARG;
const int BCT_FREE      = BCTYPE_PTR_ARG;
const int BCT_LOADOBJ   = BCTYPE_rW_ARG;
const int BCT_STOREOBJ  = BCTYPE_wW_ARG;
const int BCT_GETOBJ    = BCTYPE_W_ARG;
const int BCT_REFCPY    = BCTYPE_PTR_ARG;
const int BCT_CHKREF    = BCTYPE_NO_ARG;
const int BCT_GETOBJREF = BCTYPE_W_ARG;
const int BCT_GETREF    = BCTYPE_W_ARG;
const int BCT_SWAP48    = BCTYPE_NO_ARG;
const int BCT_SWAP84    = BCTYPE_NO_ARG;
const int BCT_OBJTYPE   = BCTYPE_PTR_ARG;
const int BCT_TYPEID    = BCTYPE_DW_ARG;
const int BCT_SetV4     = BCTYPE_wW_DW_ARG;
const int BCT_SetV8     = BCTYPE_wW_QW_ARG;
const int BCT_ADDSi     = BCTYPE_DW_ARG;
const int BCT_CpyVtoV4  = BCTYPE_wW_rW_ARG;
const int BCT_CpyVtoV8  = BCTYPE_wW_rW_ARG;
const int BCT_CpyVtoR4  = BCTYPE_rW_ARG;
const int BCT_CpyVtoR8  = BCTYPE_rW_ARG;
const int BCT_CpyVtoG4  = BCTYPE_W_rW_ARG;
const int BCT_CpyRtoV4  = BCTYPE_wW_ARG;
const int BCT_CpyRtoV8  = BCTYPE_wW_ARG;
const int BCT_CpyGtoV4  = BCTYPE_wW_W_ARG;
const int BCT_WRTV1     = BCTYPE_rW_ARG;
const int BCT_WRTV2     = BCTYPE_rW_ARG;
const int BCT_WRTV4     = BCTYPE_rW_ARG;
const int BCT_WRTV8     = BCTYPE_rW_ARG;
const int BCT_RDR1      = BCTYPE_wW_ARG;
const int BCT_RDR2      = BCTYPE_wW_ARG;
const int BCT_RDR4      = BCTYPE_wW_ARG;
const int BCT_RDR8      = BCTYPE_wW_ARG;
const int BCT_LDG       = BCTYPE_W_ARG;
const int BCT_LDV       = BCTYPE_rW_ARG;
const int BCT_PGA       = BCTYPE_W_ARG;
const int BCT_RDS4      = BCTYPE_NO_ARG;
const int BCT_VAR       = BCTYPE_rW_ARG;

const int BCT_iTOf      = BCTYPE_rW_ARG;
const int BCT_fTOi      = BCTYPE_rW_ARG;
const int BCT_uTOf      = BCTYPE_rW_ARG;
const int BCT_fTOu      = BCTYPE_rW_ARG;
const int BCT_sbTOi     = BCTYPE_rW_ARG;
const int BCT_swTOi     = BCTYPE_rW_ARG;
const int BCT_ubTOi     = BCTYPE_rW_ARG;
const int BCT_uwTOi     = BCTYPE_rW_ARG;
const int BCT_dTOi      = BCTYPE_wW_rW_ARG;
const int BCT_dTOu      = BCTYPE_wW_rW_ARG;
const int BCT_dTOf      = BCTYPE_wW_rW_ARG;
const int BCT_iTOd      = BCTYPE_wW_rW_ARG;
const int BCT_uTOd      = BCTYPE_wW_rW_ARG;
const int BCT_fTOd      = BCTYPE_wW_rW_ARG;

const int BCT_ADDi      = BCTYPE_wW_rW_rW_ARG;
const int BCT_SUBi      = BCTYPE_wW_rW_rW_ARG;
const int BCT_MULi      = BCTYPE_wW_rW_rW_ARG;
const int BCT_DIVi      = BCTYPE_wW_rW_rW_ARG;
const int BCT_MODi      = BCTYPE_wW_rW_rW_ARG;
const int BCT_ADDf      = BCTYPE_wW_rW_rW_ARG;
const int BCT_SUBf      = BCTYPE_wW_rW_rW_ARG;
const int BCT_MULf      = BCTYPE_wW_rW_rW_ARG;
const int BCT_DIVf      = BCTYPE_wW_rW_rW_ARG;
const int BCT_MODf      = BCTYPE_wW_rW_rW_ARG;
const int BCT_ADDd      = BCTYPE_wW_rW_rW_ARG;
const int BCT_SUBd      = BCTYPE_wW_rW_rW_ARG;
const int BCT_MULd      = BCTYPE_wW_rW_rW_ARG;
const int BCT_DIVd      = BCTYPE_wW_rW_rW_ARG;
const int BCT_MODd      = BCTYPE_wW_rW_rW_ARG;

const int BCT_ADDIi     = BCTYPE_wW_rW_DW_ARG;
const int BCT_SUBIi     = BCTYPE_wW_rW_DW_ARG;
const int BCT_MULIi     = BCTYPE_wW_rW_DW_ARG;
const int BCT_ADDIf     = BCTYPE_wW_rW_DW_ARG;
const int BCT_SUBIf     = BCTYPE_wW_rW_DW_ARG;
const int BCT_MULIf     = BCTYPE_wW_rW_DW_ARG;

const int BCT_SetG4     = BCTYPE_W_DW_ARG;
const int BCT_ChkRefS   = BCTYPE_NO_ARG;
const int BCT_ChkNullV  = BCTYPE_rW_ARG;
const int BCT_CALLINTF  = BCTYPE_DW_ARG;

const int BCT_iTOb      = BCTYPE_rW_ARG;
const int BCT_iTOw      = BCTYPE_rW_ARG;
const int BCT_SetV1     = BCTYPE_wW_DW_ARG;
const int BCT_SetV2     = BCTYPE_wW_DW_ARG;
const int BCT_Cast      = BCTYPE_DW_ARG;

const int BCT_i64TOi    = BCTYPE_wW_rW_ARG;
const int BCT_uTOi64    = BCTYPE_wW_rW_ARG;
const int BCT_iTOi64    = BCTYPE_wW_rW_ARG;
const int BCT_fTOi64    = BCTYPE_wW_rW_ARG;
const int BCT_dTOi64    = BCTYPE_rW_ARG;
const int BCT_fTOu64    = BCTYPE_wW_rW_ARG;
const int BCT_dTOu64    = BCTYPE_rW_ARG;
const int BCT_i64TOf    = BCTYPE_wW_rW_ARG;
const int BCT_u64TOf    = BCTYPE_wW_rW_ARG;
const int BCT_i64TOd    = BCTYPE_rW_ARG;
const int BCT_u64TOd    = BCTYPE_rW_ARG;
const int BCT_NEGi64    = BCTYPE_rW_ARG;
const int BCT_INCi64    = BCTYPE_NO_ARG;
const int BCT_DECi64    = BCTYPE_NO_ARG;
const int BCT_BNOT64    = BCTYPE_rW_ARG;

const int BCT_ADDi64    = BCTYPE_wW_rW_rW_ARG;
const int BCT_SUBi64    = BCTYPE_wW_rW_rW_ARG;
const int BCT_MULi64    = BCTYPE_wW_rW_rW_ARG;
const int BCT_DIVi64    = BCTYPE_wW_rW_rW_ARG;
const int BCT_MODi64    = BCTYPE_wW_rW_rW_ARG;
const int BCT_BAND64    = BCTYPE_wW_rW_rW_ARG;
const int BCT_BOR64     = BCTYPE_wW_rW_rW_ARG;
const int BCT_BXOR64    = BCTYPE_wW_rW_rW_ARG;
const int BCT_BSLL64    = BCTYPE_wW_rW_rW_ARG;
const int BCT_BSRL64    = BCTYPE_wW_rW_rW_ARG;
const int BCT_BSRA64    = BCTYPE_wW_rW_rW_ARG;
const int BCT_CMPi64    = BCTYPE_rW_rW_ARG;
const int BCT_CMPu64    = BCTYPE_rW_rW_ARG;
const int BCT_ChkNullS  = BCTYPE_W_ARG;
const int BCT_ClrHi     = BCTYPE_NO_ARG;


// Temporary
const int BCT_PSP       = BCTYPE_W_ARG;
#ifndef BUILD_WITHOUT_LINE_CUES
	const int BCT_LINE  = BCTYPE_NO_ARG;
#else
	const int BCT_LINE  = BCTYPE_INFO;
#endif

const int bcTypes[256] =
{
	BCT_POP,
	BCT_PUSH,
	BCT_PshC4,
	BCT_PshV4,
	BCT_PSF,
	BCT_SWAP4,
	BCT_NOT,
	BCT_PshG4,
	BCT_LdGRdR4,
	BCT_CALL,
	BCT_RET,
	BCT_JMP,
	BCT_JZ,
	BCT_JNZ,
	BCT_JS,
	BCT_JNS,
	BCT_JP,
	BCT_JNP,
	BCT_TZ,
	BCT_TNZ,
	BCT_TS,
	BCT_TNS,
	BCT_TP,
	BCT_TNP,
	BCT_NEGi,
	BCT_NEGf,
	BCT_NEGd,
	BCT_INCi16,
	BCT_INCi8,
	BCT_DECi16,
	BCT_DECi8,
	BCT_INCi,
	BCT_DECi,
	BCT_INCf,
	BCT_DECf,
	BCT_INCd,
	BCT_DECd,
	BCT_IncVi,
	BCT_DecVi,
	BCT_BNOT,
	BCT_BAND,
	BCT_BOR,
	BCT_BXOR,
	BCT_BSLL,
	BCT_BSRL,
	BCT_BSRA,
	BCT_COPY,
	BCT_SET8,
	BCT_RDS8,
	BCT_SWAP8,
	BCT_CMPd,
	BCT_CMPu,
	BCT_CMPf,
	BCT_CMPi,
	BCT_CMPIi,
	BCT_CMPIf,
	BCT_CMPIu,
	BCT_JMPP,
	BCT_PopRPtr,
	BCT_PshRPtr,
	BCT_STR,
	BCT_CALLSYS,
	BCT_CALLBND,
	BCT_SUSPEND,
	BCT_ALLOC,
	BCT_FREE,
	BCT_LOADOBJ,
	BCT_STOREOBJ,
	BCT_GETOBJ,
	BCT_REFCPY,
	BCT_CHKREF,
	BCT_GETOBJREF,
	BCT_GETREF, 
	BCT_SWAP48,
	BCT_SWAP84,
	BCT_OBJTYPE,
	BCT_TYPEID,
	BCT_SetV4,
	BCT_SetV8,
	BCT_ADDSi,
	BCT_CpyVtoV4,
	BCT_CpyVtoV8,
	BCT_CpyVtoR4,
	BCT_CpyVtoR8,
	BCT_CpyVtoG4,
	BCT_CpyRtoV4,
	BCT_CpyRtoV8,
	BCT_CpyGtoV4, 
	BCT_WRTV1,
	BCT_WRTV2,
	BCT_WRTV4,
	BCT_WRTV8,
	BCT_RDR1,
	BCT_RDR2,
	BCT_RDR4,
	BCT_RDR8,
	BCT_LDG,
	BCT_LDV,
	BCT_PGA,
	BCT_RDS4,
	BCT_VAR,
	BCT_iTOf,
	BCT_fTOi,
	BCT_uTOf,
	BCT_fTOu,
	BCT_sbTOi,
	BCT_swTOi,
	BCT_ubTOi,
	BCT_uwTOi,
	BCT_dTOi,
	BCT_dTOu,
	BCT_dTOf,
	BCT_iTOd,
	BCT_uTOd,
	BCT_fTOd,
	BCT_ADDi,
	BCT_SUBi,
	BCT_MULi,
	BCT_DIVi,
	BCT_MODi,
	BCT_ADDf,
	BCT_SUBf,
	BCT_MULf,
	BCT_DIVf,
	BCT_MODf,
	BCT_ADDd,
	BCT_SUBd,
	BCT_MULd,
	BCT_DIVd,
	BCT_MODd,
	BCT_ADDIi,
	BCT_SUBIi,
	BCT_MULIi,
	BCT_ADDIf,
	BCT_SUBIf,
	BCT_MULIf,
	BCT_SetG4,
	BCT_ChkRefS,
	BCT_ChkNullV,
	BCT_CALLINTF,
	BCT_iTOb,
	BCT_iTOw,
	BCT_SetV1,
	BCT_SetV2,
	BCT_Cast,
	BCT_i64TOi,
	BCT_uTOi64,
	BCT_iTOi64,
	BCT_fTOi64,
	BCT_dTOi64,
	BCT_fTOu64,
	BCT_dTOu64,
	BCT_i64TOf,
	BCT_u64TOf,
	BCT_i64TOd,
	BCT_u64TOd,
	BCT_NEGi64,
	BCT_INCi64,
	BCT_DECi64,
	BCT_BNOT64,
	BCT_ADDi64,
	BCT_SUBi64,
	BCT_MULi64,
	BCT_DIVi64,
	BCT_MODi64,
	BCT_BAND64,
	BCT_BOR64,
	BCT_BXOR64,
	BCT_BSLL64,
	BCT_BSRL64,
	BCT_BSRA64,
	BCT_CMPi64,
	BCT_CMPu64,
	BCT_ChkNullS,
	BCT_ClrHi,
	0,0,0,0, // 175-179
	0,0,0,0,0,0,0,0,0,0, // 180-189
	0,0,0,0,0,0,0,0,0,0, // 190-199
	0,0,0,0,0,0,0,0,0,0, // 200-209
	0,0,0,0,0,0,0,0,0,0, // 210-219
	0,0,0,0,0,0,0,0,0,0, // 220-229
	0,0,0,0,0,0,0,0,0,0, // 230-239
	0,0,0,0,0,0,       // 240-245
	BCT_PSP, 
	0,  
	BCT_LINE,  
	0, 
	0,  // 250
	0,  // 251
	0,  // 252
	0,  // 253
	0,  // 254
	0,	// BC_LABEL     
};

const int bcStackInc[256] =
{
	0xFFFF,		// BC_POP
	0xFFFF,		// BC_PUSH
	1,			// BC_PshC4
	1,			// BC_PshV4
	PTR_SIZE,	// BC_PSF
	0,			// BC_SWAP4
	0,			// BC_NOT
	1,			// BC_PshG4
	0,			// BC_LdGRdR4
	0xFFFF,		// BC_CALL
	0xFFFF,		// BC_RET
	0,			// BC_JMP
	0,			// BC_JZ
	0,			// BC_JNZ
	0,			// BC_JS
	0,			// BC_JNS
	0,			// BC_JP
	0,			// BC_JNP
	0,			// BC_TZ
	0,			// BC_TNZ
	0,			// BC_TS
	0,			// BC_TNS
	0,			// BC_TP
	0,			// BC_TNP
	0,			// BC_NEGi
	0,			// BC_NEGf
	0,			// BC_NEGd
	0,			// BC_INCi16
	0,			// BC_INCi8
	0,			// BC_DECi16
	0,			// BC_DECi8
	0,			// BC_INCi
	0,			// BC_DECi
	0,			// BC_INCf
	0,			// BC_DECf
	0,			// BC_INCd
	0,			// BC_DECd
	0,			// BC_IncVi
	0,			// BC_DecVi
	0,			// BC_BNOT
	0,			// BC_BAND
	0,			// BC_BOR
	0,			// BC_BXOR
	0,			// BC_BSLL
	0,			// BC_BSRL
	0,			// BC_BSRA
	-1,			// BC_COPY
	2,			// BC_SET8
	2-PTR_SIZE,	// BC_RDS8
	0,			// BC_SWAP8
	0,			// BC_CMPd
	0,			// BC_CMPu
	0,			// BC_CMPf
	0,			// BC_CMPi
	0,			// BC_CMPIi
	0,			// BC_CMPIf
	0,			// BC_CMPIu
	0,			// BC_JMPP
	-PTR_SIZE,	// BC_PopRPtr
	PTR_SIZE,	// BC_PshRPtr
	1+PTR_SIZE,	// BC_STR
	0xFFFF,		// BC_CALLSYS
	0xFFFF,		// BC_CALLBND
	0,			// BC_SUSPEND
	0xFFFF,		// BC_ALLOC
	-PTR_SIZE,	// BC_FREE
	0,			// BC_LOADOBJ
	0,			// BC_STOREOBJ
	0,			// BC_GETOBJ
	-PTR_SIZE,	// BC_REFCPY
	0,			// BC_CHKREF
	0,			// BC_GETOBJREF
	0,			// BC_GETREF
	0,			// BC_SWAP48
	0,			// BC_SWAP84
	PTR_SIZE,	// BC_OBJTYPE
	1,			// BC_TYPEID
	0,			// BC_SetV4
	0,			// BC_SetV8
	0,			// BC_ADDSi
	0,			// BC_CpyVtoV4
	0,			// BC_CpyVtoV8
	0,			// BC_CpyVtoR4
	0,			// BC_CpyVtoR8
	0,			// BC_CpyVtoG4
	0,			// BC_CpyRtoV4
	0,			// BC_CpyRtoV8
	0,			// BC_CpyGtoV4
	0,			// BC_WRTV1
	0,			// BC_WRTV2
	0,			// BC_WRTV4
	0,			// BC_WRTV8
	0,			// BC_RDR1
	0,			// BC_RDR2
	0,			// BC_RDR4
	0,			// BC_RDR8
	0,			// BC_LDG
	0,			// BC_LDV
	PTR_SIZE,	// BC_PGA
	1-PTR_SIZE,	// BC_RDS4
	PTR_SIZE,	// BC_VAR
	0,			// BC_iTOf	
	0,			// BC_fTOi	
	0,			// BC_uTOf	
	0,			// BC_fTOu	
	0,			// BC_sbTOi	
	0,			// BC_swTOi	
	0,			// BC_ubTOi	
	0,			// BC_uwTOi	
	0,			// BC_dTOi 
	0,			// BC_dTOu 
	0,			// BC_dTOf 
	0,			// BC_iTOd 
	0,			// BC_iTOd
	0,			// BC_fTOd 
	0,			// BC_ADDi
	0,			// BC_SUBi
	0,			// BC_MULi
	0,			// BC_DIVi
	0,			// BC_MODi
	0,			// BC_ADDf
	0,			// BC_SUBf
	0,			// BC_MULf
	0,			// BC_DIVf
	0,			// BC_MODf
	0,			// BC_ADDd
	0,			// BC_SUBd
	0,			// BC_MULd
	0,			// BC_DIVd
	0,			// BC_MODd
	0,			// BC_ADDIi
	0,			// BC_SUBIi
	0,			// BC_MULIi
	0,			// BC_ADDIf
	0,			// BC_SUBIf
	0,			// BC_MULIf
	0,			// BC_SetG4
	0,			// BC_ChkRefS
	0,			// BC_ChkNullV
	0xFFFF,		// BC_CALLINTF
	0,			// BC_iTOb
	0,			// BC_iTOw
	0,			// BC_SetV1
	0,			// BC_SetV2
	0,			// BC_Cast
	0,			// BC_i64TOi
	0,			// BC_uTOi64
	0,			// BC_iTOi64
	0,			// BC_fTOi64
	0,			// BC_dTOi64
	0,			// BC_fTOu64
	0,			// BC_dTOu64
	0,			// BC_i64TOf
	0,			// BC_u64TOf
	0,			// BC_i64TOd
	0,			// BC_u64TOd
	0,          // BC_NEGi64
	0,          // BC_INCi64
	0,          // BC_DECi64
	0,          // BC_BNOT64
	0,			// BC_ADDi64
	0,			// BC_SUBi64
	0,			// BC_MULi64
	0,			// BC_DIVi64
	0,			// BC_MODi64
	0,			// BC_BAND64
	0,			// BC_BOR64
	0,			// BC_BXOR64
	0,			// BC_BSLL64
	0,			// BC_BSRL64
	0,			// BC_BSRA64
	0,			// BC_CMPi64
	0,			// BC_CMPu64
	0,			// BC_ChkNullS
	0,			// BC_ClrHi
	0,0,0,0,0, // 175-179
	0,0,0,0,0,0,0,0,0,0, // 180-189
	0,0,0,0,0,0,0,0,0,0, // 190-199
	0,0,0,0,0,0,0,0,0,0, // 200-209
	0,0,0,0,0,0,0,0,0,0, // 210-219
	0,0,0,0,0,0,0,0,0,0, // 220-229
	0,0,0,0,0,0,0,0,0,0, // 230-239
	0,0,0,0,0,0,       // 240-245
	1,		// BC_PSP
	0,		
	0xFFFF, // BC_LINE
	0,		// 249
	0, // 250
	0, // 251
	0, // 252
	0, // 253
	0, // 254
	0xFFFF,	// BC_LABEL
};

struct sByteCodeName
{
	char *name;
};

#ifdef AS_DEBUG
const sByteCodeName bcName[256] =
{
	{"POP"},
	{"PUSH"},
	{"PshC4"},
	{"PshV4"},
	{"PSF"},
	{"SWAP4"},
	{"NOT"},
	{"PshG4"},
	{"LdGRdR4"},
	{"CALL"},
	{"RET"},
	{"JMP"},
	{"JZ"},
	{"JNZ"},
	{"JS"},
	{"JNS"},
	{"JP"},
	{"JNP"},
	{"TZ"},
	{"TNZ"},
	{"TS"},
	{"TNS"},
	{"TP"},
	{"TNP"},
	{"NEGi"},
	{"NEGf"},
	{"NEGd"},
	{"INCi16"},
	{"INCi8"},
	{"DECi16"},
	{"DECi8"},
	{"INCi"},
	{"DECi"},
	{"INCf"},
	{"DECf"},
	{"INCd"},
	{"DECd"},
	{"IncVi"},
	{"DecVi"},
	{"BNOT"},
	{"BAND"},
	{"BOR"},
	{"BXOR"},
	{"BSLL"},
	{"BSRL"},
	{"BSRA"},
	{"COPY"},
	{"SET8"},
	{"RDS8"},
	{"SWAP8"},
	{"CMPd"},
	{"CMPu"},
	{"CMPf"},
	{"CMPi"},
	{"CMPIi"},
	{"CMPIf"},
	{"CMPIu"},
	{"JMPP"},
	{"PopRPtr"},
	{"PshRPtr"},
	{"STR"},
	{"CALLSYS"},
	{"CALLBND"},
	{"SUSPEND"},
	{"ALLOC"},
	{"FREE"},
	{"LOADOBJ"},
	{"STOREOBJ"},
	{"GETOBJ"},
	{"REFCPY"},
	{"CHKREF"},
	{"GETOBJREF"},
	{"GETREF"}, 
	{"SWAP48"},
	{"SWAP84"},
	{"OBJTYPE"},
	{"TYPEID"},
	{"SetV4"},
	{"SetV8"},
	{"ADDSi"},
	{"CpyVtoV4"},
	{"CpyVtoV8"},
	{"CpyVtoR4"},
	{"CpyVtoR8"},
	{"CpyVtoG4"},
	{"CpyRtoV4"},
	{"CpyRtoV8"},
	{"CpyGtoV4"},
	{"WRTV1"},
	{"WRTV2"},
	{"WRTV4"},
	{"WRTV8"},
	{"RDR1"},
	{"RDR2"},
	{"RDR4"},
	{"RDR8"},
	{"LDG"},
	{"LDV"},
	{"PGA"},
	{"RDS4"},
	{"VAR"},
	{"iTOf"},
	{"fTOi"},
	{"uTOf"},
	{"fTOu"},
	{"sbTOi"},
	{"swTOi"},
	{"ubTOi"},
	{"uwTOi"},
	{"dTOi"},
	{"dTOu"}, 
	{"dTOf"},
	{"iTOd"},
	{"uTOd"},
	{"fTOd"},
	{"ADDi"},
	{"SUBi"},
	{"MULi"},
	{"DIVi"},
	{"MODi"},
	{"ADDf"},
	{"SUBf"},
	{"MULf"},
	{"DIVf"},
	{"MODf"},
	{"ADDd"},
	{"SUBd"},
	{"MULd"},
	{"DIVd"},
	{"MODd"},
	{"ADDIi"},
	{"SUBIi"},
	{"MULIi"},
	{"ADDIf"},
	{"SUBIf"},
	{"MULIf"},
	{"SetG4"},
	{"ChkRefS"},
	{"ChkNullV"},
	{"CALLINTF"},
	{"iTOb"},
	{"iTOw"},
	{"SetV1"},
	{"SetV2"},
	{"Cast"},
	{"i64TOi"},
	{"uTOi64"},
	{"iTOi64"},
	{"fTOi64"},
	{"dTOi64"},
	{"fTOu64"},
	{"dTOu64"},
	{"i64TOf"},
	{"u64TOf"},
	{"i64TOd"},
	{"u64TOd"},
	{"NEGi64"},
	{"INCi64"},
	{"DECi64"},
	{"BNOT64"},
	{"ADDi64"},
	{"SUBi64"},
	{"MULi64"},
	{"DIVi64"},
	{"MODi64"},
	{"BAND64"},
	{"BOR64"},
	{"BXOR64"},
	{"BSLL64"},
	{"BSRL64"},
	{"BSRA64"},
	{"CMPi64"},
	{"CMPu64"},
	{"ChkNullS"},
	{"ClrHi"},
	{0},{0},{0},{0},{0}, // 175-179
	{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}, // 180-189
	{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}, // 190-199
	{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}, // 200-209
	{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}, // 210-219
	{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}, // 220-229
	{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}, // 230-239
	{0},{0},{0},{0},{0},{0},	     // 240-245
	{"PSP"},
	{0},
	{"LINE"},
	{0}, 
	{0},
	{0},
	{0},
	{0},
	{0},
	{"LABEL"}
};
#endif

END_AS_NAMESPACE

#endif
