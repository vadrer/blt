/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#define BUILD_BLT_TCL_PROCS 1
#include <bltInt.h>

/* !BEGIN!: Do not edit below this line. */

BltTclIntProcs bltTclIntProcs = {
    TCL_STUB_MAGIC,
    NULL,
    NULL, /* 0 */
    Blt_GetArrayFromObj, /* 1 */
    Blt_NewArrayObj, /* 2 */
    Blt_RegisterArrayObj, /* 3 */
    Blt_IsArrayObj, /* 4 */
    Blt_Assert, /* 5 */
    Blt_DBuffer_VarAppend, /* 6 */
    Blt_DBuffer_Format, /* 7 */
    Blt_DBuffer_Init, /* 8 */
    Blt_DBuffer_Free, /* 9 */
    Blt_DBuffer_Extend, /* 10 */
    Blt_DBuffer_AppendData, /* 11 */
    Blt_DBuffer_Concat, /* 12 */
    Blt_DBuffer_Resize, /* 13 */
    Blt_DBuffer_SetLength, /* 14 */
    Blt_DBuffer_Create, /* 15 */
    Blt_DBuffer_Destroy, /* 16 */
    Blt_DBuffer_LoadFile, /* 17 */
    Blt_DBuffer_SaveFile, /* 18 */
    Blt_DBuffer_AppendByte, /* 19 */
    Blt_DBuffer_AppendShort, /* 20 */
    Blt_DBuffer_AppendInt, /* 21 */
    Blt_DBuffer_ByteArrayObj, /* 22 */
    Blt_DBuffer_StringObj, /* 23 */
    Blt_DBuffer_String, /* 24 */
    Blt_DBuffer_Base64Decode, /* 25 */
    Blt_DBuffer_Base64EncodeToObj, /* 26 */
    Blt_DBuffer_AppendBase85, /* 27 */
    Blt_DBuffer_AppendBase64, /* 28 */
    Blt_InitCmd, /* 29 */
    Blt_InitCmds, /* 30 */
    Blt_GetVariableNamespace, /* 31 */
    Blt_GetCommandNamespace, /* 32 */
    Blt_EnterNamespace, /* 33 */
    Blt_LeaveNamespace, /* 34 */
    Blt_ParseObjectName, /* 35 */
    Blt_MakeQualifiedName, /* 36 */
    Blt_CommandExists, /* 37 */
    Blt_GetOpFromObj, /* 38 */
    Blt_CreateSpline, /* 39 */
    Blt_EvaluateSpline, /* 40 */
    Blt_FreeSpline, /* 41 */
    Blt_CreateParametricCubicSpline, /* 42 */
    Blt_EvaluateParametricCubicSpline, /* 43 */
    Blt_FreeParametricCubicSpline, /* 44 */
    Blt_CreateCatromSpline, /* 45 */
    Blt_EvaluateCatromSpline, /* 46 */
    Blt_FreeCatromSpline, /* 47 */
    Blt_ComputeNaturalSpline, /* 48 */
    Blt_ComputeQuadraticSpline, /* 49 */
    Blt_ComputeNaturalParametricSpline, /* 50 */
    Blt_ComputeCatromParametricSpline, /* 51 */
    Blt_ParseSwitches, /* 52 */
    Blt_FreeSwitches, /* 53 */
    Blt_SwitchChanged, /* 54 */
    Blt_SwitchInfo, /* 55 */
    Blt_SwitchValue, /* 56 */
    Blt_Malloc, /* 57 */
    Blt_Realloc, /* 58 */
    Blt_Free, /* 59 */
    Blt_Calloc, /* 60 */
    Blt_Strdup, /* 61 */
    Blt_MallocAbortOnError, /* 62 */
    Blt_CallocAbortOnError, /* 63 */
    Blt_ReallocAbortOnError, /* 64 */
    Blt_StrdupAbortOnError, /* 65 */
    Blt_DictionaryCompare, /* 66 */
    Blt_GetUid, /* 67 */
    Blt_FreeUid, /* 68 */
    Blt_FindUid, /* 69 */
    Blt_CreatePipeline, /* 70 */
    Blt_InitHexTable, /* 71 */
    Blt_DStringAppendElements, /* 72 */
    Blt_LoadLibrary, /* 73 */
    Blt_Panic, /* 74 */
    Blt_Warn, /* 75 */
    Blt_GetSideFromObj, /* 76 */
    Blt_NameOfSide, /* 77 */
    Blt_OpenFile, /* 78 */
    Blt_ExprDoubleFromObj, /* 79 */
    Blt_ExprIntFromObj, /* 80 */
    Blt_Itoa, /* 81 */
    Blt_Ltoa, /* 82 */
    Blt_Utoa, /* 83 */
    Blt_Dtoa, /* 84 */
    Blt_Base64_Decode, /* 85 */
    Blt_Base64_DecodeToBuffer, /* 86 */
    Blt_Base64_DecodeToObj, /* 87 */
    Blt_Base64_EncodeToObj, /* 88 */
    Blt_Base64_MaxBufferLength, /* 89 */
    Blt_Base64_Encode, /* 90 */
    Blt_Base85_MaxBufferLength, /* 91 */
    Blt_Base85_Encode, /* 92 */
    Blt_Base16_Encode, /* 93 */
    Blt_IsBase64, /* 94 */
    Blt_GetDoubleFromString, /* 95 */
    Blt_GetDoubleFromObj, /* 96 */
    Blt_GetTimeFromObj, /* 97 */
    Blt_GetTime, /* 98 */
    Blt_GetDateFromObj, /* 99 */
    Blt_GetDate, /* 100 */
    Blt_GetPositionFromObj, /* 101 */
    Blt_GetCountFromObj, /* 102 */
    Blt_SimplifyLine, /* 103 */
    Blt_GetLong, /* 104 */
    Blt_GetLongFromObj, /* 105 */
    Blt_FormatString, /* 106 */
    Blt_LowerCase, /* 107 */
    Blt_GetPlatformId, /* 108 */
    Blt_LastError, /* 109 */
    Blt_NaN, /* 110 */
    Blt_AlmostEquals, /* 111 */
    Blt_SecondsToDate, /* 112 */
    Blt_DateToSeconds, /* 113 */
    Blt_GetCachedVar, /* 114 */
    Blt_FreeCachedVars, /* 115 */
};

/* !END!: Do not edit above this line. */
