
/*
 * bltDate.c --
 *
 * A date parser for the BLT toolkit.  Used to automatically convert 
 * dates in datatables to seconds.  May include fractional seconds.
 *
 *	Copyright 1993-2004 George A Howlett.
 *
 *	Permission is hereby granted, free of charge, to any person obtaining
 *	a copy of this software and associated documentation files (the
 *	"Software"), to deal in the Software without restriction, including
 *	without limitation the rights to use, copy, modify, merge, publish,
 *	distribute, sublicense, and/or sell copies of the Software, and to
 *	permit persons to whom the Software is furnished to do so, subject to
 *	the following conditions:
 *
 *	The above copyright notice and this permission notice shall be
 *	included in all copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *	LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define _BSD_SOURCE 1
#define BUILD_BLT_TCL_PROCS 1
#include "bltInt.h"
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_CTYPE_H
#  include <ctype.h>
#endif /* HAVE_CTYPE_H */
#ifdef HAVE_TIME_H
#  include <time.h>
#endif /* HAVE_TIME_H */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <stdint.h>
#include "bltAlloc.h"
#include "bltString.h"
#include "bltSwitch.h"
#include "bltChain.h"
#include "bltInitCmd.h"
#include "bltOp.h"
#include <bltMath.h>

#define HRS2MINS(x)         ((int) (60 * x))

#if defined(MAC_TCL) && !defined(TCL_MAC_USE_MSL_EPOCH)
#   define EPOCH           1904
#   define START_OF_TIME   1904
#   define END_OF_TIME     2039
#else
#   define EPOCH           1970
#   define START_OF_TIME   1902
#   define END_OF_TIME     2037
#endif

#define MER_AM		0
#define MER_PM		1
#define MER_24		2

#define PARSE_DATE		(1<<0)
#define PARSE_TIME		(1<<1)
#define PARSE_TZ		(1<<2)

static const char *tokenNames[] = {
    "end",
    "month", "wday", "yday", "day", "year", "week",
    "hours", "seconds", "minutes", "ampm", 
    "tz_std", "tz_dst", 
    "/", "-", ",", ":", "+", ".",
    "number", "iso6", "iso7", "iso8", 
    "unknown"
};

typedef enum TokenSerialNumbers {
    T_END,
    T_MONTH, T_WDAY, T_YDAY, T_MDAY, T_YEAR, T_WEEK,
    T_HOUR, T_SECOND, T_MINUTE, T_AMPM,
    T_STD, T_DST,
    T_SLASH, T_DASH, T_COMMA, T_COLON, T_PLUS, T_DOT,
    T_NUMBER, T_ISO6, T_ISO7, T_ISO8, T_UNKNOWN,
} TokenNumber;

typedef struct _Token Token;

struct _Token {
    Blt_ChainLink link;			/* If non-NULL, pointer this entry
					 * in the list of tokens. */
    Blt_Chain chain;			/* Pointer to list of tokens. */
    const char *ident;			/* String representing this token. */
    int length;				/* # of bytes in string. */
    TokenNumber id;			/* Serial Id of token. */
    uint64_t lvalue;			/* Numeric value of token. */
    float frac;				/* Fraction of seconds. */
};

typedef struct {
    Tcl_Interp *interp;			/* Interpreter associated with the
					 * parser. */
    unsigned int flags;			/* Flags: see below. */
    time_t year;			/* Year 0-9999. */
    time_t mon;				/* Month 1-12. */
    time_t wday;			/* Day of week. 1-7. */
    time_t yday;			/* Day of the year. 1-366. */
    time_t mday;			/* Day of the month. 1-31. */
    time_t week;			/* Ordinal week. 1-53. */
    time_t hour;			/* Hour 0-23. */
    time_t min;				/* Minute 0-59 */
    time_t sec;				/* Second. 0-60. */
    float frac;				/* Fractional seconds. */
    time_t tzoffset;			/* Timezone offset. */
    int isdst;				/* DST or STD. */
    int ampm;				/* Meridian. */
    char *nextCharPtr;			/* Points to the next character
					 * to parse.*/
    Blt_Chain tokens;			/* List of parsed tokens. */
    Token *currentPtr;			/* Current token being processed. */
    char buffer[BUFSIZ];		/* Buffer holding parsed string. */
    Token *headPtr, *tailPtr;
} DateParser;

typedef struct {
    const char *name;			/* Name of identifier. */
    TokenNumber id;			/* Serial Id # of identifier. */
    int value;				/* Value associated with identifier. */
} IdentTable;

static const char *monthTable[] = {
    "january",  "february",  "march",      "april",    "may",       "june", 
    "july",     "august",    "september",  "october",  "november",  "december"
};
static int numMonths = sizeof(monthTable) / sizeof(const char *);

static int  numDaysMonth[13] = {
    31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31
};

static const char *wkDayTable[] = {
    "sunday",    "monday",  "tuesday", "wednesday", "thursday",  "friday",  
    "saturday"
};

static int numWkDays = sizeof(wkDayTable) / sizeof(const char *);

static const char *meridianTable[] = {
    "am",  "pm"
};
static int numMeridians = sizeof(meridianTable) / sizeof(const char *);

/*
 * The timezone table.  (Note: This table was modified to not use any floating
 * point constants to work around an SGI compiler bug).
 */
static IdentTable tzTable[] = {
    { "gmt",  T_STD, HRS2MINS(0) },	/* Greenwich Mean */
    { "ut",   T_STD, HRS2MINS(0) },	/* Universal (Coordinated) */
    { "utc",  T_STD, HRS2MINS(0) },
    { "uct",  T_STD, HRS2MINS(0) },	/* Universal Coordinated Time */
    { "wet",  T_STD, HRS2MINS(0) },	/* Western European */
    { "bst",  T_DST, HRS2MINS(0) },	/* British Summer */
    { "wat",  T_STD, HRS2MINS(1) },	/* West Africa */
    { "at",   T_STD, HRS2MINS(2) },	/* Azores */
#if     0
    /* For completeness.  BST is also British Summer, and GST is also Guam
     * Standard. */
    { "bst",  T_STD, HRS2MINS(3) },	/* Brazil Standard */
    { "gst",  T_STD, HRS2MINS(3) },	/* Greenland Standard */
#endif
    { "nft",  T_STD, HRS2MINS(7/2) },	/* Newfoundland */
    { "nst",  T_STD, HRS2MINS(7/2) },	/* Newfoundland Standard */
    { "ndt",  T_DST, HRS2MINS(7/2) },	/* Newfoundland Daylight */
    { "ast",  T_STD, HRS2MINS(4) },	/* Atlantic Standard */
    { "adt",  T_DST, HRS2MINS(4) },	/* Atlantic Daylight */
    { "est",  T_STD, HRS2MINS(5) },	/* Eastern Standard */
    { "edt",  T_DST, HRS2MINS(5) },	/* Eastern Daylight */
    { "cst",  T_STD, HRS2MINS(6) },	/* Central Standard */
    { "cdt",  T_DST, HRS2MINS(6) },	/* Central Daylight */
    { "mst",  T_STD, HRS2MINS(7) },	/* Mountain Standard */
    { "mdt",  T_DST, HRS2MINS(7) },	/* Mountain Daylight */
    { "pst",  T_STD, HRS2MINS(8) },	/* Pacific Standard */
    { "pdt",  T_DST, HRS2MINS(8) },	/* Pacific Daylight */
    { "yst",  T_STD, HRS2MINS(9) },	/* Yukon Standard */
    { "ydt",  T_DST, HRS2MINS(9) },	/* Yukon Daylight */
    { "hst",  T_STD, HRS2MINS(10) },	/* Hawaii Standard */
    { "hdt",  T_DST, HRS2MINS(10) },	/* Hawaii Daylight */
    { "cat",  T_STD, HRS2MINS(10) },	/* Central Alaska */
    { "ahst", T_STD, HRS2MINS(10) },	/* Alaska-Hawaii Standard */
    { "nt",   T_STD, HRS2MINS(11) },	/* Nome */
    { "idlw", T_STD, HRS2MINS(12) },	/* International Date Line West */
    { "cet",  T_STD, -HRS2MINS(1) },	/* Central European */
    { "cest", T_DST, -HRS2MINS(1) },	/* Central European Summer */
    { "met",  T_STD, -HRS2MINS(1) },	/* Middle European */
    { "mewt", T_STD, -HRS2MINS(1) },	/* Middle European Winter */
    { "mest", T_DST, -HRS2MINS(1) },	/* Middle European Summer */
    { "swt",  T_STD, -HRS2MINS(1) },	/* Swedish Winter */
    { "sst",  T_DST, -HRS2MINS(1) },	/* Swedish Summer */
    { "fwt",  T_STD, -HRS2MINS(1) },	/* French Winter */
    { "fst",  T_DST, -HRS2MINS(1) },	/* French Summer */
    { "eet",  T_STD, -HRS2MINS(2) },	/* Eastern Europe, USSR Zone 1 */
    { "bt",   T_STD, -HRS2MINS(3) },	/* Baghdad, USSR Zone 2 */
    { "it",   T_STD, -HRS2MINS(7/2) },	/* Iran */
    { "zp4",  T_STD, -HRS2MINS(4) },	/* USSR Zone 3 */
    { "zp5",  T_STD, -HRS2MINS(5) },	/* USSR Zone 4 */
    { "ist",  T_STD, -HRS2MINS(11/2) },    /* Indian Standard */
    { "zp6",  T_STD, -HRS2MINS(6) },	/* USSR Zone 5 */
#if     0
    /* For completeness.  NST is also Newfoundland Stanard, nad SST is also
     * Swedish Summer. */
    { "nst",  T_STD, -HRS2MINS(13/2) },    /* North Sumatra */
    { "sst",  T_STD, -HRS2MINS( 7) },      /* south Sumatra, USSR Zone 6 */
#endif  /* 0 */ 
    { "wast", T_STD, -HRS2MINS( 7) },      /* West Australian Standard */
    { "wadt", T_DST, -HRS2MINS( 7) },      /* West Australian Daylight */
    { "jt",   T_STD, -HRS2MINS(15/2) },    /* Java (3pm in Cronusland!) */
    { "cct",  T_STD, -HRS2MINS( 8) },      /* China Coast, USSR Zone 7 */
    { "jst",  T_STD, -HRS2MINS( 9) },      /* Japan Standard, USSR Zone 8 */
    { "jdt",  T_DST, -HRS2MINS( 9) },      /* Japan Daylight */
    { "kst",  T_STD, -HRS2MINS( 9) },      /* Korea Standard */
    { "kdt",  T_DST, -HRS2MINS( 9) },      /* Korea Daylight */
    { "cast", T_STD, -HRS2MINS(19/2) },    /* Central Australian Standard */
    { "cadt", T_DST, -HRS2MINS(19/2) },    /* Central Australian Daylight */
    { "east", T_STD, -HRS2MINS(10) },      /* Eastern Australian Standard */
    { "eadt", T_DST, -HRS2MINS(10) },      /* Eastern Australian Daylight */
    { "gst",  T_STD, -HRS2MINS(10) },      /* Guam Standard, USSR Zone 9 */
    { "nzt",  T_STD, -HRS2MINS(12) },      /* New Zealand */
    { "nzst", T_STD, -HRS2MINS(12) },      /* New Zealand Standard */
    { "nzdt", T_DST, -HRS2MINS(12) },      /* New Zealand Daylight */
    { "idle", T_STD, -HRS2MINS(12) },      /* International Date Line East */
    /* ADDED BY Marco Nijdam */
    { "dst",  T_DST, HRS2MINS( 0) },       /* DST on (hour is ignored) */
    /* End ADDED */
};

static int numTimezones = sizeof(tzTable) / sizeof(IdentTable);

/*
 * Military timezone table.
 */
static IdentTable milTzTable[] = {
    { "a",  T_STD,  HRS2MINS(1)   },
    { "b",  T_STD,  HRS2MINS(2)   },
    { "c",  T_STD,  HRS2MINS(3)   },
    { "d",  T_STD,  HRS2MINS(4)   },
    { "e",  T_STD,  HRS2MINS(5)   },
    { "f",  T_STD,  HRS2MINS(6)   },
    { "g",  T_STD,  HRS2MINS(7)   },
    { "h",  T_STD,  HRS2MINS(8)   },
    { "i",  T_STD,  HRS2MINS(9)   },
    { "k",  T_STD,  HRS2MINS(10)  },
    { "l",  T_STD,  HRS2MINS(11)  },
    { "m",  T_STD,  HRS2MINS(12)  },
    { "n",  T_STD,  HRS2MINS(-1)  },
    { "o",  T_STD,  HRS2MINS(-2)  },
    { "p",  T_STD,  HRS2MINS(-3)  },
    { "q",  T_STD,  HRS2MINS(-4)  },
    { "r",  T_STD,  HRS2MINS(-5)  },
    { "s",  T_STD,  HRS2MINS(-6)  },
    { "t",  T_STD,  HRS2MINS(-7)  },
    { "u",  T_STD,  HRS2MINS(-8)  },
    { "v",  T_STD,  HRS2MINS(-9)  },
    { "w",  T_STD,  HRS2MINS(-10) },
    { "x",  T_STD,  HRS2MINS(-11) },
    { "y",  T_STD,  HRS2MINS(-12) },
    { "z",  T_STD,  HRS2MINS(0)   },
};

static int numMilitaryTimezones = sizeof(milTzTable) / sizeof(IdentTable);

typedef struct {
    int numIds;
    TokenNumber ids[6];
} Pattern;

static Pattern datePatterns[] = {
    { 2, {T_ISO7} },			/* yyyyddd (2012100) */
    { 2, {T_ISO8} },			/* yyyymmdd (20120131) */
    { 2, {T_YEAR} },			/* yyyy (2012) */
    { 3, {T_MONTH, T_YEAR} },		/* mon yyyy (Jan 2012) */
    { 3, {T_YEAR, T_MONTH} },		/* yyyy mon (2012 Jan) */
    { 3, {T_YEAR, T_WEEK} },		/* yyyywww (2012W01)*/
    { 4, {T_MDAY, T_MONTH, T_YEAR} },	/* dd mon yyyy (31 Jan 2012) */
    { 4, {T_MONTH, T_MDAY, T_YEAR} },	/* mon dd yyyy (Jan 31 2012) */
    { 4, {T_MONTH, T_SLASH, T_MDAY} },	/* mon/dd (12/23) */
    { 4, {T_YEAR, T_DASH, T_WEEK} },	/* yyyy-www (2012-W01) */
    { 4, {T_YEAR, T_DASH, T_YDAY} },	/* yyyy-ddd (2012-100) */
    { 4, {T_YEAR, T_MONTH, T_MDAY} },	/* yyyymmdd (20120131) */
    { 4, {T_YEAR, T_WEEK, T_WDAY} },	/* yyyywwwd (2012W017) */
    { 5, {T_MONTH, T_MDAY, T_COMMA, T_YEAR} },  /* (Jan 31, 2012) */
    { 6, {T_MDAY, T_DOT, T_MONTH, T_DOT, T_YEAR} },  /* (31.Jan.2012) */
    { 6, {T_MDAY, T_DASH, T_MONTH, T_DASH, T_YEAR} }, /* (31-Jan-2012) */
    { 6, {T_MONTH, T_DOT, T_MDAY, T_DOT, T_YEAR} },  /* (Jan.31.2012) */
    { 6, {T_MONTH, T_DASH, T_MDAY, T_DASH, T_YEAR} }, /* (Jan-31-2012) */
    { 6, {T_MONTH, T_SLASH, T_MDAY, T_SLASH, T_YEAR} }, /* (Jan/31/2012) */
    { 6, {T_MDAY, T_SLASH, T_MONTH, T_SLASH, T_YEAR} }, /* (31/Jan/2012) */
    { 6, {T_YEAR, T_DOT, T_MONTH, T_DOT, T_MDAY} },  /* (2012.Jan.31) */
    { 6, {T_YEAR, T_DASH, T_MONTH, T_DASH, T_MDAY} }, /* (2012-Jan-31) */
    { 6, {T_YEAR, T_DASH, T_WEEK, T_DASH, T_WDAY} }, /* (2012-W50-7) */
    { 1, {}}
};

static int numDatePatterns = sizeof(datePatterns) / sizeof(Pattern);

typedef struct {
    Tcl_Obj *tzObjPtr;
} FormatSwitches;

static Blt_SwitchSpec formatSwitches[] = 
{
    {BLT_SWITCH_OBJ, "-timezone", "",  (char *)NULL,
	Blt_Offset(FormatSwitches, tzObjPtr), 0},
    {BLT_SWITCH_END}
};

typedef struct {
    Tcl_Obj *fmtObjPtr;
} ScanSwitches;

static Blt_SwitchSpec scanSwitches[] = 
{
    {BLT_SWITCH_OBJ, "-format", "",  (char *)NULL,
	Blt_Offset(ScanSwitches, fmtObjPtr), 0},
    {BLT_SWITCH_END}
};

static int
InitParser(Tcl_Interp *interp, DateParser *datePtr, const char *string) 
{
    const char *p;
    char *q;

    memset(datePtr, 0, sizeof(DateParser));
    datePtr->interp = interp;
    for (p = string, q = datePtr->buffer; *p != '\0'; p++, q++) {
	if (isalpha(*p)) {
	    *q = tolower(*p);
	} else {
	    *q = *p;
	}
    }
    *q = '\0';
    datePtr->nextCharPtr = datePtr->buffer;
    datePtr->tokens = Blt_Chain_Create();
    return TCL_OK;
}

static void
ParseError(DateParser *datePtr, const char *fmt, ...)
{
    char string[BUFSIZ+4];
    int length;
    va_list args;

    va_start(args, fmt);
    length = vsnprintf(string, BUFSIZ, fmt, args);
    if (length > BUFSIZ) {
	strcat(string, "...");
    }
    Tcl_AppendResult(datePtr->interp, string, (char *)NULL);
    va_end(args);
}

static void
ParseWarning(DateParser *datePtr, const char *fmt, ...)
{
    char string[BUFSIZ+4];
    int length;
    va_list args;

    va_start(args, fmt);
    length = vsnprintf(string, BUFSIZ, fmt, args);
    if (length > BUFSIZ) {
	strcat(string, "...");
    }
    Tcl_ResetResult(datePtr->interp);
    Tcl_AppendResult(datePtr->interp, string, (char *)NULL);
    va_end(args);
}

static void
FreeParser(DateParser *datePtr) 
{
    Blt_Chain_Destroy(datePtr->tokens);
}

static TokenNumber
GetId(Token *tokenPtr) 
{
    if (tokenPtr == NULL) {
	return T_END;
    }
    return tokenPtr->id;
}

static void
DeleteToken(Token *tokenPtr) 
{
    if (tokenPtr->link != NULL) {
	Blt_Chain_DeleteLink(tokenPtr->chain, tokenPtr->link);
    }
}

static TokenNumber
ParseNumber(DateParser *datePtr, char *string)
{
    char *p;
    long lvalue;
    int length, result;
    Token *tokenPtr;
    Tcl_Obj *objPtr;

    p = string;
    tokenPtr = datePtr->currentPtr;
    while (isdigit(*p)) {
	p++;
    }
    length = p - string;
    objPtr = Tcl_NewStringObj(string, length);
    Tcl_IncrRefCount(objPtr);
    result = Blt_GetLongFromObj(datePtr->interp, objPtr, &lvalue);
    Tcl_DecrRefCount(objPtr);
    if (result != TCL_OK) {
	ParseError(datePtr, "error parsing \"%*s\" as number", length, string);
	return T_UNKNOWN;
    }
    tokenPtr->lvalue = lvalue;
    tokenPtr->ident = string;
    tokenPtr->length = p - string;
    return tokenPtr->id = T_NUMBER;
}

static TokenNumber
ParseString(DateParser *datePtr, int length, char *string)
{
    char c;
    int i;
    Token *tokenPtr;

    tokenPtr = datePtr->currentPtr;
    c = string[0];
    /* Month. weekday names (may be abbreviated). */
    if (length >= 3) {
	int i;

	/* Test of months. Allow abbreviations greater than 2 characters. */
	for (i = 0; i < numMonths; i++) {
	    const char *p;

	    p = monthTable[i];
	    if ((c == *p) && (strncmp(p, string, length) == 0)) {
		tokenPtr->lvalue = i + 1;
		tokenPtr->ident = p;
		tokenPtr->length = 0;
		return tokenPtr->id = T_MONTH;
	    }
	}
	/* Test of weekdays. Allow abbreviations greater than 2 characters. */
	for (i = 0; i < numWkDays; i++) {
	    const char *p;

	    p = wkDayTable[i];
	    if ((c == *p) && (strncmp(p, string, length) == 0)) {
		tokenPtr->lvalue = i;
		tokenPtr->length = 0;
		tokenPtr->ident = p;
		return tokenPtr->id = T_WDAY;
	    }
	}
    }
    /* Timezone. */
    for (i = 0; i < numTimezones; i++) {
	IdentTable *p;

	p = tzTable + i;
	/* Test of timezomes. No abbreviations. */
	if ((c == p->name[0]) && (strcmp(p->name, string) == 0)) {
	    tokenPtr->lvalue = p->value;
	    tokenPtr->ident = p->name;
	    tokenPtr->length = 0;
	    return  tokenPtr->id = p->id;
	}
    }
    /* Meridian: am or pm. */
    if (length == 2) {
	int i;

	/* Test of meridian. */
	for (i = 0; i < numMeridians; i++) {
	    const char *p;

	    p = meridianTable[i];
	    if ((c == *p) && (strncmp(p, string, length) == 0)) {
		tokenPtr->lvalue = i;
		tokenPtr->length = 0;
		return tokenPtr->id = T_AMPM;
	    }
	}
    }
    /* Military timezone. Single letter a-z. */
    if (string[1] == '\0') {
	int i;

	/* Test of military timezones. Only one character wide. */
	for (i = 0; i < numMilitaryTimezones; i++) {
	    IdentTable *p;

	    p = milTzTable + i;
	    if (c == p->name[0]) {
		tokenPtr->ident = p->name;
		tokenPtr->length = 1;
		tokenPtr->lvalue = p->value;
		return tokenPtr->id = T_STD;
	    }
	}
    }
    ParseError(datePtr, "unknown token \"%*s\"", length, string);
    return T_UNKNOWN;
}

static Token *
FirstToken(DateParser *datePtr)
{
    Blt_ChainLink link;

    link = Blt_Chain_FirstLink(datePtr->tokens);
    if (link == NULL) {
	return NULL;
    }
    return Blt_Chain_GetValue(link);
}

static Token *
LastToken(DateParser *datePtr)
{
    Blt_ChainLink link;

    link = Blt_Chain_LastLink(datePtr->tokens);
    if (link == NULL) {
	return NULL;
    }
    return Blt_Chain_GetValue(link);
}

static Token *
NextToken(Token *tokenPtr)
{
    Blt_ChainLink link;

    if (tokenPtr->link == NULL) {
	return NULL;
    }
    link = Blt_Chain_NextLink(tokenPtr->link);
    if (link == NULL) {
	return NULL;
    }
    return Blt_Chain_GetValue(link);
}

static Token *
PrevToken(Token *tokenPtr)
{
    Blt_ChainLink link;

    if (tokenPtr->link == NULL) {
	return NULL;
    }
    link = Blt_Chain_PrevLink(tokenPtr->link);
    if (link == NULL) {
	return NULL;
    }
    return Blt_Chain_GetValue(link);
}


static int
NumberOfTokens(DateParser *datePtr) 
{
    return Blt_Chain_GetLength(datePtr->tokens);
}

static const char *
TokenName(Token *tokenPtr) 
{
    if (tokenPtr == NULL) {
	return tokenNames[T_END];
    } 
    return tokenNames[tokenPtr->id];
}


static void
PrintTokens(DateParser *datePtr) 
{
    Token *tokenPtr;

    fprintf(stderr, "tokens = [ ");
    for (tokenPtr = FirstToken(datePtr); tokenPtr != NULL;
	 tokenPtr = NextToken(tokenPtr)) {
	fprintf(stderr, "%s ",TokenName(tokenPtr));
    }
    fprintf(stderr, "]\n");
}

/* 
 * 
 */
static int
ProcessToken(DateParser *datePtr, TokenNumber *idPtr)
{
    char *p;
    TokenNumber id;
    Token *tokenPtr;
    Blt_ChainLink link;

    link = Blt_Chain_AllocLink(sizeof(Token));
    Blt_Chain_LinkAfter(datePtr->tokens, link, NULL);
    tokenPtr = Blt_Chain_GetValue(link);
    tokenPtr->link = link;
    tokenPtr->chain = datePtr->tokens;
    datePtr->currentPtr = tokenPtr;
    p = datePtr->nextCharPtr;
    while (isspace(*p)) {
	p++;				/* Skip leading spaces. */
    }
    if (*p == '/') {
	id = T_SLASH;
	p++;
    } else if (*p == '+') {
	id = T_PLUS;
	p++;
    } else if (*p == ':') {
	id = T_COLON;
	p++;
    } else if (*p == '.') {
	id = T_DOT;
	p++;
    } else if (*p == ',') {
	id = T_COMMA;
	p++;
    } else if (*p == '-') {
	id = T_DASH;
	p++;
    } else if ((*p == 'w') && (isdigit(*(p+1))) && (isdigit(*(p+2)))) {
	tokenPtr->ident = p;
	tokenPtr->lvalue = (p[1] - '0') * 10 + (p[2] - '0');
	id = T_WEEK;
	tokenPtr->length = 3;
	p += tokenPtr->length;
    } else if (isdigit(*p)) {
	char *start;
	char save;

	start = p;
	while (isdigit(*p)) {
	    p++;
	}
	save = *p;
	*p = '\0';
	id = ParseNumber(datePtr, start);
	if (id == T_UNKNOWN) {
	    return TCL_ERROR;
	}
	*p = save;			/* Restore last chararacter. */
    } else if (isalpha(*p)) {
	char name[BUFSIZ];
	int i;
	
	i = 0;
	for (i = 0; ((isalpha(*p)) || (*p == '.')) && (i < 200); p++) {
	    if (*p != '.') {
		name[i] = *p;
		i++;
	    }
	}
	id = ParseString(datePtr, i, name);
	if (id == T_UNKNOWN) {
	    return TCL_ERROR;
	}
    } else if (*p == '\0') {
	id = T_END;
    } else {
	id = T_UNKNOWN;
    }
    *idPtr = datePtr->currentPtr->id = id;
    datePtr->nextCharPtr = p;
    return TCL_OK;
}

static int
MatchPattern(DateParser *datePtr)
{
    int i;

    /* Find the first colon. */
    for (i = 0; i < numDatePatterns; i++) {
	int j;
	Token *tokenPtr;
	Pattern *patPtr;

	patPtr = datePatterns + i;
	if (patPtr->numIds != NumberOfTokens(datePtr)) {
	    continue;
	}
	for (j = 0, tokenPtr = FirstToken(datePtr); 
	     (tokenPtr != NULL) && (j < patPtr->numIds); 
	     tokenPtr = NextToken(tokenPtr), j++) {
	    TokenNumber id;

	    id = patPtr->ids[j];
	    if (tokenPtr->id != id) {
		if ((tokenPtr->id == T_NUMBER) && (tokenPtr->length <= 2)){
		    switch (id) {
		    case T_WDAY:
			if ((tokenPtr->lvalue > 0) && (tokenPtr->lvalue <= 7)) {
			    continue;
			}
			ParseWarning(datePtr, "weekday \"%d\" is out of range", 
				     tokenPtr->lvalue);
			break;

		    case T_MONTH:
			if (tokenPtr->lvalue <= 12) {
			    continue;
			}
			ParseWarning(datePtr, "month \"%d\" is out of range", 
				     tokenPtr->lvalue);
			break;

		    case T_MDAY:
			if (tokenPtr->lvalue <= 31) {
			    continue;
			}
			ParseWarning(datePtr, "day \"%d\" is out of range", 
				     tokenPtr->lvalue);
			break;

		    case T_YEAR:
			continue;

		    default:
			break;
		    }
		}
		break;
	    }
	    if (id == T_END) {
		return i;
	    }
	}
    }
    return -1;
}

static Token *
GetTz(Token *tokenPtr, time_t *mPtr)
{
    time_t m;
    int sign;
    
    sign = 1;
    if (GetId(tokenPtr) == T_DASH) {
	sign = -1;
    }
    /* Verify the next token after the +/- is a number.  */
    tokenPtr = NextToken(tokenPtr);
    if (GetId(tokenPtr) != T_NUMBER) {
	return NULL;
    }
    /* The timezone is in the form NN:NN or NNNN. */
    if (tokenPtr->length == 4) {
	m = ((tokenPtr->lvalue / 100) * 60) + (tokenPtr->lvalue % 100);
    } else if (tokenPtr->length == 2) {
	m = HRS2MINS(tokenPtr->lvalue);
	tokenPtr = NextToken(tokenPtr);
	if (GetId(tokenPtr) != T_COLON) {
	    *mPtr = sign * m;
	    return tokenPtr;
	}
	tokenPtr = NextToken(tokenPtr);
	if ((GetId(tokenPtr) != T_NUMBER) || (tokenPtr->length != 2)) {
	    return NULL;
	}
	m += tokenPtr->lvalue;
    } else {
	return NULL;		/* Error: expecting 2 digit or 4 digit
				 * number after plus or minus. */
    }
    tokenPtr = NextToken(tokenPtr);
    *mPtr = sign * m;
    return tokenPtr;
}

static int
ExtractTimezone(DateParser *datePtr)
{
    int isdst;
    time_t m;
    Token *tokenPtr;

    m = 0;
    isdst = 0;
    for (tokenPtr = LastToken(datePtr); tokenPtr != NULL;
	 tokenPtr = PrevToken(tokenPtr)) {
	switch (tokenPtr->id) {
	case T_DST:			/* (EDT) */
	    isdst = 1;
	    m = HRS2MINS(tokenPtr->lvalue);
	    isdst = TRUE;
	    goto found;
	    
	case T_STD:			/* (EST) */
	    m = HRS2MINS(tokenPtr->lvalue);
	    goto found;

	default:
	    break;
	}
    }
    if (tokenPtr == NULL) {
	return TCL_OK;			/* No timezone found. */
    }
found:
    DeleteToken(tokenPtr);
    datePtr->tzoffset = m;
    datePtr->isdst = isdst;
    datePtr->flags |= PARSE_TZ;
    return TCL_OK;
}


static int
GetTimezone(DateParser *datePtr, Token *tokenPtr, Token **resPtrPtr)
{
    int isdst;
    time_t m;

    isdst = 0;
    switch (GetId(tokenPtr)) {
    case T_PLUS:			/* +00:00 */
    case T_DASH:			/* -00:00 */
	tokenPtr = GetTz(tokenPtr, &m);
	break;

    case T_DST:				/* EDT */
	m = HRS2MINS(tokenPtr->lvalue);
	isdst = TRUE;
	tokenPtr = NextToken(tokenPtr);
	goto done;

    case T_STD:				/* EST */
	m = HRS2MINS(tokenPtr->lvalue);
	tokenPtr = NextToken(tokenPtr);
	goto done;

    default:
	return TCL_CONTINUE;
    }
    if (tokenPtr == NULL) {
	return TCL_ERROR;
    }
 done:
    *resPtrPtr = tokenPtr;
    datePtr->tzoffset = m;
    datePtr->isdst = isdst;
    datePtr->flags |= PARSE_TZ;
    return TCL_OK;
}

static int
ExtractWeekday(DateParser *datePtr)
{
    Token *tokenPtr;

    /* Find the starting pattern "num colon num". */
    for (tokenPtr = FirstToken(datePtr); tokenPtr != NULL; 
	 tokenPtr = NextToken(tokenPtr)) {
	if (tokenPtr->id == T_WDAY) {
	    datePtr->wday = tokenPtr->lvalue;
	    DeleteToken(tokenPtr);
	    return TRUE;
	}
    }
    return FALSE;
}

static int
ExtractSeparator(DateParser *datePtr)
{
    Token *tokenPtr;

    /* Find the date/time separator "t" and remove it. */
    for (tokenPtr = FirstToken(datePtr); tokenPtr != NULL; 
	 tokenPtr = NextToken(tokenPtr)) {
	if ((tokenPtr->id == T_STD) && (tokenPtr->length == 1) &&
	    (tokenPtr->ident[0] == 't')) {
	    Token *nextPtr;

	    /* Check if the 't' is a military timezone or a separator. */
	    nextPtr = NextToken(tokenPtr);
	    if (GetId(nextPtr) != T_END) {
		DeleteToken(tokenPtr);
	    }
	    return TCL_OK;
	}
    }
    return TCL_ERROR;
}

#ifdef notdef
static int
DayOfWeek(int day, int mon, int year)
{
    int cent;
    
    /* adjust months so February is the last one */
    mon -= 2;
    if (mon < 1) {
	mon += 12;
	--year;
    }
    /* split by century */
    cent = year / 100;
    year %= 100;
    return (((26*mon - 2)/10 + day + year + year/4 + cent/4 + 5*cent) % 7);
}
#endif

static int
GetWeekday(time_t year, time_t mon, time_t day, time_t *ydayPtr)
{
    long a, b, c, s, e, f, d, g, n;

    if (mon < 2) {
	a = year - 1;
	b = a / 4 -  a / 100 +  a / 400;
	c = (a-1)/4 - (a-1)/100 + (a-1)/400;
	s = b - c;
	e = 0;
	f = day - 1 +  31 * (mon-1);
    } else {
	a = year;
	b = a/4 - a/100 + a/400;
	c = (a-1)/4 - (a-1)/100 + (a-1)/400;
	s = b-c;
	e = s+1;
	f = day + (153*(mon-3)+2)/5 + 58 + s;
    }
    g = (a + b) % 7;
    d = (f + g - e) % 7;
    n = f + 3 - d;

    if (n < 0) {
	/* the day lies in week 53-(g-s)/5 of the previous year. */
    } else if (n > (364+s)) {
	/* the day lies in week 1 of the coming year. */
    } else {
	/* Otherwise, the day lies in week n/7 + 1 of the current year. */
    }
    *ydayPtr = f + 1;
    return d + 1;			/* Mon=1, Sun=7 */
}

static int
GetDateFromOrdinalDay(DateParser *datePtr, time_t yday)
{
    int i;
    int count;
    
    count = 0;
    for (i = 0; i < 13; i++) {
	if ((count + numDaysMonth[i]) > yday) {
	    datePtr->mon = i + 1;
	    datePtr->mday = yday - count + 1;
	    return TCL_OK;
	}
	count += numDaysMonth[i];
    }
    ParseError(datePtr, "error counting days from \"%d\"", yday);
    return TCL_ERROR;
}
    
/*
 *---------------------------------------------------------------------------
 *
 * FixTokens --
 *
 *	Convert 3, 4, and 8 digit number token ids to specific ids.
 *	This is done for pattern matching. 
 *
 *	3 digit number		oridinal day of year (ddd)
 *	4 digit number		year (yyyy)
 *	7 digit number		year (yyyyddd)
 *	8 digit number		ISO date format (yyyymmdd)
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static void
FixTokens(DateParser *datePtr)
{
    Token *tokenPtr;

    for (tokenPtr = FirstToken(datePtr); tokenPtr != NULL; 
	 tokenPtr = NextToken(tokenPtr)) {
	if (tokenPtr->id == T_NUMBER) {
	    if (tokenPtr->length == 3) {
		tokenPtr->id = T_YDAY;
	    } else if (tokenPtr->length == 4) {
		tokenPtr->id = T_YEAR;
	    } else if (tokenPtr->length == 7) {
		tokenPtr->id = T_ISO7;
	    } else if (tokenPtr->length == 8) {
		tokenPtr->id = T_ISO8;
	    }
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ExtractTime --
 *
 *	Find and extract the time related tokens in the list.  
 *	The time format is
 *
 *		time + ampm + timezone
 *
 *	where ampm and timezone are optional.
 *
 *	The following time patterns are accepted:
 *
 *	hh:mm						10:21
 *	hh:mm:ss					10:21:00
 *	hh:mm:ss:fff	fff is milliseconds.		10:21:00:001
 *	hh:mm:ss.f+	f+ is fraction of seconds.	10:21:00.001
 *	hh:mm:ss,f+	f+ is fraction of seconds.	10:21:00,001
 *	hhmmss		ISO time format			102100
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The matching tokens are removed from the list and the parser
 *	structure is updated with the new information.
 *
 *---------------------------------------------------------------------------
 */
static int
ExtractTime(DateParser *datePtr)
{
    int result;
    Token *nextPtr, *firstPtr, *lastPtr;
    Token *tokenPtr, *hourPtr, *minPtr;

    firstPtr = lastPtr = NULL;
    /* Find the starting pattern "num colon num". */
    for (tokenPtr = FirstToken(datePtr); tokenPtr != NULL; 
	 tokenPtr = NextToken(tokenPtr)) {
	Token *colonPtr;

	hourPtr = firstPtr = tokenPtr;
	if ((GetId(hourPtr) != T_NUMBER) || (hourPtr->length > 2)) {
	    continue;
	}
	colonPtr = NextToken(tokenPtr);
	if (GetId(colonPtr) != T_COLON) {
	    continue;
	}
	minPtr = NextToken(colonPtr);
	if ((GetId(minPtr) != T_NUMBER) || (minPtr->length > 2)) {
	    continue;
	}
	tokenPtr = NextToken(minPtr);
	break;				/* Found the starting pattern */
    }
    if (tokenPtr == NULL) {
	for (tokenPtr = FirstToken(datePtr); tokenPtr != NULL; 
	     tokenPtr = NextToken(tokenPtr)) {
	    if ((GetId(tokenPtr) == T_NUMBER) && 
		((tokenPtr->length == 6) || (tokenPtr->length == 14))) {
		long value;
		/* Iso time format hhhmmss */
		value = tokenPtr->lvalue;
		
		datePtr->sec = value % 100;
		value /= 100;
		datePtr->min = value % 100;
		value /= 100;
		datePtr->hour = value % 100;
		if (tokenPtr->length == 14) {
		    value /= 100;
		    tokenPtr->length = 8;  /* Convert to ISO8 */
		    tokenPtr->lvalue = value;
		    firstPtr = tokenPtr = NextToken(tokenPtr);
		} else {
		    firstPtr = tokenPtr;
		    tokenPtr = NextToken(tokenPtr);
		}
		goto done;
	    }
	}
	return TCL_OK;			/* No time tokens found. */
    }
    datePtr->hour = hourPtr->lvalue;
    datePtr->min = minPtr->lvalue;
    if (GetId(tokenPtr) != T_COLON) {
	goto done;
    }
    tokenPtr = NextToken(tokenPtr);
    if ((GetId(tokenPtr) != T_NUMBER) || (tokenPtr->length > 2)) {
	goto done;
    }
    datePtr->sec = tokenPtr->lvalue;
    tokenPtr = NextToken(tokenPtr);
    if (GetId(tokenPtr) == T_COLON) {
	tokenPtr = NextToken(tokenPtr);
	if ((GetId(tokenPtr) == T_NUMBER) && (tokenPtr->length == 3)) {
	    datePtr->frac = tokenPtr->lvalue * 1e-3;
	    tokenPtr = NextToken(tokenPtr);
	}
    } else if ((GetId(tokenPtr) == T_DOT) || 
	       (GetId(tokenPtr) == T_COMMA)) {
	tokenPtr = NextToken(tokenPtr);
	if ((GetId(tokenPtr) == T_NUMBER)) {
	    double d;

	    d = pow(10.0, tokenPtr->length);
	    datePtr->frac = tokenPtr->lvalue / d;
	    tokenPtr = NextToken(tokenPtr);
	}
    }
 done:
    /* Look for ampm designation. */
    if (GetId(tokenPtr) == T_AMPM) {
	if (datePtr->hour > 12) {
	    fprintf(stderr, "invalid am/pm, already in 24hr format\n");
	}
	if (tokenPtr->lvalue) {
	    datePtr->hour += 12;
	}
	tokenPtr = NextToken(tokenPtr);
    }
    result = GetTimezone(datePtr, tokenPtr, &nextPtr);
    if (result == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (result == TCL_OK) {
	tokenPtr = nextPtr;
    }
    /* Remove the time-related tokens from the list. */
    lastPtr = tokenPtr;
    for (tokenPtr = firstPtr; tokenPtr != NULL; tokenPtr = nextPtr) {
	nextPtr = NextToken(tokenPtr);
	if (tokenPtr == lastPtr) {
	    break;
	}
	DeleteToken(tokenPtr);
    }
    datePtr->flags |= PARSE_TIME;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ExtractDate --
 *
 *	Find and extract the time related tokens in the list.  
 *	The time format is
 *
 *		time + ampm + timezone
 *
 *	where ampm and timezone are optional.
 *
 *	The following time patterns are accepted:
 *
 *	hh:mm						10:21
 *	hh:mm:ss					10:21:00
 *	hh:mm:ss:fff	fff is milliseconds.		10:21:00:001
 *	hh:mm:ss.f+	f+ is fraction of seconds.	10:21:00.001
 *	hhmmss		ISO time format			102100
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The matching tokens are removed from the list and the parser
 *	structure is updated with the new information.
 *
 *---------------------------------------------------------------------------
 */
static int
ExtractDate(DateParser *datePtr)
{
    Token *nextPtr, *tokenPtr;
    int i, patternIndex;
    Pattern *patPtr;

    if (NumberOfTokens(datePtr) == 1) {
	return TCL_OK;
    }
    /* Remove the weekday description. */
    ExtractWeekday(datePtr);
    /*  */
    FixTokens(datePtr);
    patternIndex = MatchPattern(datePtr);
    if (patternIndex < 0) {
	PrintTokens(datePtr);
	ParseError(datePtr, "\nNo matching date pattern");
	return TCL_ERROR;
    }
    Tcl_ResetResult(datePtr->interp);
    /* Process the list against the matching pattern. */
    patPtr = datePatterns + patternIndex;
    assert(patPtr->numIds == NumberOfTokens(datePtr));
    tokenPtr = FirstToken(datePtr); 
    for (i = 0; i < patPtr->numIds; i++, tokenPtr = NextToken(tokenPtr)) {
	TokenNumber id;
	
	id = patPtr->ids[i];
	switch (id) {
	case T_MONTH:
	    datePtr->mon = tokenPtr->lvalue;
	    break;

	case T_YEAR:
	    datePtr->year = tokenPtr->lvalue;
	    if (tokenPtr->length == 2) {
		datePtr->year += 1900;
	    }
	    break;

	case T_WEEK:
	    datePtr->week = tokenPtr->lvalue;
	    break;

	case T_WDAY:
	    datePtr->wday = tokenPtr->lvalue;
	    break;

	case T_MDAY:
	    datePtr->mday = tokenPtr->lvalue;
	    break;

	case T_YDAY:
	    datePtr->yday = tokenPtr->lvalue;
	    break;

	case T_ISO7:
	    {
		long value;
		/* Iso date format */
		value = tokenPtr->lvalue;
		
		datePtr->yday = value % 1000;
		value /= 1000;
		datePtr->year = value;
	    }
	    break;

	case T_ISO8:
	    {
		long value;
		/* Iso date format */
		value = tokenPtr->lvalue;
		
		datePtr->mday = value % 100;
		value /= 100;
		datePtr->mon = value % 100;
		value /= 100;
		datePtr->year = value;
	    }
	    break;
	case T_COMMA:
	case T_DASH:
	case T_DOT:
	    continue;
	case T_END:
	default:
	    break;
	}
    }
    for (tokenPtr = FirstToken(datePtr); tokenPtr != NULL; 
	 tokenPtr = nextPtr) {
	nextPtr = NextToken(tokenPtr);
	DeleteToken(tokenPtr);
    }
    datePtr->flags |= PARSE_DATE;
    return TCL_OK;
}

static INLINE int
IsLeapYear(time_t year) 
{
    return (((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0)));
}

static int
NumberDaysFromMonth(DateParser *datePtr)
{
    int i, numDays;

    numDays = 0;
    for (i = 0; i < (datePtr->mon - 1); i++) {
        numDays += numDaysMonth[i];
    }
    return numDays;
}

static int
NumberDaysFromYear(DateParser *datePtr)
{
    int i, numDays;

    numDays = 0;
    if (datePtr->year >= EPOCH) {
        for (i = EPOCH; i < datePtr->year; i++) {
            numDays += 365 + IsLeapYear(i);
	}
    } else {
        for (i = datePtr->year; i < EPOCH; i++)
            numDays -= 365 + IsLeapYear(i);
    }
    return numDays;
}

/*
 *-----------------------------------------------------------------------------
 *
 * ConvertToSeconds --
 *
 *      Convert a {month, day, year, hours, minutes, seconds, meridian, dst}
 *      tuple into a clock seconds value.
 *
 * Results:
 *      0 or -1 indicating success or failure.
 *
 * Side effects:
 *      Fills TimePtr with the computed value.
 *
 *-----------------------------------------------------------------------------
 */
static int
ConvertToSeconds(DateParser *datePtr, double *timePtr)
{
    double t;
    time_t numDays;

    /* Now that we know the year, set the number of days for Februrary.  */
    numDaysMonth[1] = IsLeapYear(datePtr->year) ? 29 : 28;

#ifdef notdef
    fprintf(stderr, "mon=%ld year=%ld mday=%ld\n", datePtr->mon,
	    datePtr->year, datePtr->mday);
#endif

    /* Check the inputs for validity */
    if (datePtr->mon > 12) {
	ParseError(datePtr, "month \"%ld\" is out of range.", datePtr->mon);
    }
    if (datePtr->year > 9999) {
	ParseError(datePtr, "year \"%ld\" is out of range.", datePtr->year);
    }
    if (datePtr->mday > numDaysMonth[datePtr->mon - 1]) {
	ParseError(datePtr, "day \"%ld\" is out of range for month \"%s\"",
		    datePtr->mday, monthTable[datePtr->mon - 1]);
	return TCL_ERROR;
    }
    if (datePtr->hour > 24) {
	ParseError(datePtr, "hour \"%ld\" is out of range.", datePtr->hour); 
	return TCL_ERROR;
    }
    if (datePtr->min > 59) {
	ParseError(datePtr, "minute \"%ld\", is out of range.", datePtr->min); 
	return TCL_ERROR;
    }
    if (datePtr->sec > 60) {
	ParseError(datePtr, "seconds \"%ld\" is out of range.", datePtr->sec);
	return TCL_ERROR;
    }
    /* Start computing the value.  First determine the number of days
     * represented by the date, then multiply by the number of seconds/day.
     */
    if (datePtr->week > 0) {
	time_t wday, yday, corr, n;
 
	wday = GetWeekday(datePtr->year, 1, 4, &yday);
	corr = wday + 3;
	n = ((datePtr->week) * 7) + (datePtr->wday) + IsLeapYear(datePtr->year);
	yday = n - corr;
	if (GetDateFromOrdinalDay(datePtr, yday - 1) != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    numDays = 0;
    if (datePtr->year > 0) {
	numDays += NumberDaysFromYear(datePtr);
    }
    if (datePtr->yday > 0) {
	numDays += (datePtr->yday - 1);
    } else if (datePtr->mday > 0) {
	numDays += (datePtr->mday - 1) + NumberDaysFromMonth(datePtr);
    }
    t = numDays * (60 * 60 * 24);	/* Convert to seconds. */
#ifdef notdef
    fprintf(stderr, "numDays=%ld t=%.15g\n", numDays, t);
#endif
    /* Add the timezone offset. */
    t += datePtr->tzoffset;

    if (datePtr->isdst > 0) {
	/* datePtr->hour++; */
    }
    /* Add in the time, including the fraction. */
    t += (datePtr->hour * 60 * 60) + (datePtr->min * 60) + datePtr->sec;
    t += datePtr->frac;	

    *timePtr = t;
    return TCL_OK;
}

/* 
 * Date and time formats:
 *
 *	<date> <time>
 *	<date>T<time>
 */
/* 
 * Date formats:
 *
 *	wkday, month day, year 
 *	wkday month day, year
 *	wkday, day. month year
 *	wkday, day month year
 *	wkday day month year
 *	month/day/year
 *	day/month/year
 *	year/month/day
 *	day-month-year
 *	year-month-day
 *	month day
 *	month-day
 *	wkday month day
 *	day. month
 *	wkday day. month
 *	day month
 *	wkday day month
 *	year-month-day
 *	year-month-day
 *	month, year
 *	month year
 */

/* 
 * Time formats:
 *
 *	time <meridian> <tz>
 *	hh
 *	hh:mm
 *	hh:mm:ss
 *	hhmm
 *	hhmmss
 *	ss.frac			- a single number interger or floating point
 *	hh:mm:ss:mmm
 *	hh:mm
 */

/* 
 * Timezone
 *
 *	timezone
 *	-+1234
 *	a-z	
 */

/* 
 * Date formats:
 *
 *	day month yyyy
 *	dd month
 *	dd - month-yyyy
 *	dd . month
 *	dd . month yyyy
 *	dd / month / yyyy
 *	month day
 *	month dd
 *	month dd yyyy
 *	month dd , yyyy
 *	month yyyy
 *	month , yyyy
 *	month - dd
 *	month - dd-yyyy
 *	month / dd / yyyy
 *	yyyy
 *	yyyy - Www
 *	yyyy - Www - D
 *	yyyy - mm
 *	yyyy - mm - dd
 *	yyyy - month
 *	yyyy - month - dd
 *	yyyy / mm / dd
 *	yyyyWww
 *	yyyyWwwD
 *	yyyymmdd

 * mm yy
 * mm yyyy
 * mm dd yy
 * mm dd yyyy
 * mm dd, yyyy
 * mm / dd / yyyy
 * mm - dd - yyyy
 * mm . dd . yyyy
 * yyyy mm dd
 * yyyy - mm - dd
 * yyyy . mm . dd
 * dd mm yyyy
 * dd . mm . yyyy
 * yyyymmdd
 * 
 * dd yyyy
 * mm dd yy
 * mm/dd/yy
 * mm/dd/yyyy
 * yy.mm.dd
 * yyyy.mm.dd
 * dd/mm/yy
 * yyyy-mm-dd
 * yyyymmmdd 8 digit int.
 * dd 
	on DD YYYY HH:MIAM 	Jan 1 2005 1:29PM 1
	MM/DD/YY	 	11/23/98
	MM/DD/YYYY		11/23/1998
	YY.MM.DD	 	72.01.01
	YYYY.MM.DD	 	1972.01.01
	DD/MM/YY	 	19/02/72
	DD/MM/YYYY	 	19/02/1972
	DD.MM.YY	 	25.12.05
	DD.MM.YYYY		25.12.2005
	DD-MM-YY		24-01-98
	DD-MM-YYYY 		24-01-1998
	DD Mon YY	 	04 Jul 06 1
	DD Mon YYYY	 	04 Jul 2006 1
	Mon DD, YY	 	Jan 24, 98 1
	Mon DD, YYYY	 	Jan 24, 1998 1
	HH:MM:SS	 	03:24:53
	Mon DD YYYY HH:MI:SS:MMMAM Apr 28 2006 12:32:29:253PM 1
	MM-DD-YY		01-01-06
	MM-DD-YYYY		01-01-2006
	YY/MM/DD	 	98/11/23
	YYYY/MM/DD	 	1998/11/23
	YYMMDD			980124
	YYYYMMDD 	 	19980124
	DD Mon YYYY HH:MM:SS:MMM 	28 Apr 2006 00:34:55:190 
	HH:MI:SS:MMM	  	11:34:23:013
	YYYY-MM-DD HH:MI:SS 	1972-01-01 13:42:24
	YYYY-MM-DD HH:MI:SS.MMM	1972-02-19 06:35:24.489
	YYYY-MM-DDTHH:MM:SS:MMM 1998-11-23T11:25:43:250
	DD Mon YYYY HH:MI:SS:MMMAM	28 Apr 2006 12:39:32:429AM 1
	DD/MM/YYYY HH:MI:SS:MMMAM  	28/04/2006 12:39:32:429AM

	YY-MM-DD		99-01-24
	YYYY-MM-DD		1999-01-24
	MM/YY		 	08/99
	MM/YYYY		 	12/2005
	YY/MM		 	99/08
	YYYY/MM			2005/12
	Month DD, YYYY		July 04, 2006
	Mon YYYY		Apr 2006 1
	Month YYYY 		February 2006
	DD Month		11 September 
	Month DD		September 11
	DD Month YY		19 February 72 
	DD Month YYYY		11 September 2002 
	MM-YY		 	12-92
	MM-YYYY		 	05-2006
	YY-MM		 	92-12
	YYYY-MM		 	2006-05
	MMDDYY		 	122506
	MMDDYYYY 	 	12252006
	DDMMYY		 	240702
	DDMMYYYY 	 	24072002
	Mon-YY			Sep-02 1
	Mon-YYYY		Sep-2002 
	DD-Mon-YY		25-Dec-05 
	DD-Mon-YYYY 

	mm/dd/yy	 	11/23/98
	mm/dd/yyyy		11/23/1998
	yy.mm.dd	 	72.01.01
	yyyy.mm.dd	 	1972.01.01
	dd/mm/yy	 	19/02/72
	dd/mm/yyyy	 	19/02/1972
	dd.mm.yy	 	25.12.05
	dd.mm.yyyy		25.12.2005
	dd-mm-yy		24-01-98
	dd-mm-yyyy 		24-01-1998
	dd mon yy	 	04 Jul 06 1
	dd mon yyyy	 	04 Jul 2006 1
	mon dd, yy	 	Jan 24, 98 1
	mon dd, yyyy	 	Jan 24, 1998 1
	mm-dd-yy		01-01-06
	mm-dd-yyyy		01-01-2006
	yy/mm/dd	 	98/11/23
	yyyy/mm/dd	 	1998/11/23
	yymmdd			980124
	yyyymmdd 	 	19980124
	dd mon yyyy HH:MM:SS:MMM 	28 Apr 2006 00:34:55:190 
	hh:mi:ss:mmm	  	11:34:23:013
	yyyy-mm-dd HH:MI:SS 	1972-01-01 13:42:24
	yyyy-mm-dd HH:MI:SS.MMM	1972-02-19 06:35:24.489
	yyyy-mm-ddthh:MM:SS:MMM 1998-11-23T11:25:43:250
	dd mon yyyy HH:MI:SS:MMMAM	28 Apr 2006 12:39:32:429AM 1
	dd/mm/yyyy HH:MI:SS:MMMAM  	28/04/2006 12:39:32:429AM

	yy-mm-dd		99-01-24
	yyyy-mm-dd		1999-01-24
	mm/yy		 	08/99
	mm/yyyy		 	12/2005
	yy/mm		 	99/08
	yyyy/mm			2005/12
	month dd, yyyy		July 04, 2006
	mon yyyy		Apr 2006 1
	month yyyy 		February 2006
	dd month		11 September 
	month dd		September 11
	dd month yy		19 February 72 
	dd month yyyy		11 September 2002 
	mm-yy		 	12-92
	mm-yyyy		 	05-2006
	yy-mm		 	92-12
	yyyy-mm		 	2006-05
	mmddyy		 	122506
	mmddyyyy 	 	12252006
	ddmmyy		 	240702
	ddmmyyyy 	 	24072002
	mon-yy			Sep-02
	mon-yyyy		Sep-2002 
	dd-mon-yy		25-Dec-05 
	dd-mon-yyyy 


 */

/* 
 * Valid date patterns
 *
 *	wday month dd year 
 *	wday dd month year
 *	wday dd month
 *	wday month day yyyy
 */
static int
GetDate(Tcl_Interp *interp, const char *string, double *timePtr)
{
    DateParser parser;
    TokenNumber id;

    InitParser(interp, &parser, string);
    /* Create list of tokens from date string. */
    id = T_END;
    do {
	if (ProcessToken(&parser, &id) != TCL_OK) {
	    FreeParser(&parser);
	    return TCL_ERROR;
	}
    } while (id != T_END);

    /* Remove the time/date 'T' separator if one exists. */
    ExtractSeparator(&parser);
    /* Now parse out the time, timezone, and then date. */
    if ((ExtractTime(&parser) != TCL_OK) ||
	(ExtractTimezone(&parser) != TCL_OK) ||
	(ExtractDate(&parser) != TCL_OK)) {
	goto error;
    }
    if (ConvertToSeconds(&parser, timePtr) != TCL_OK) {
	goto error;
    }
    FreeParser(&parser);
    return TCL_OK;
 error:
    FreeParser(&parser);
    return TCL_ERROR;
}

/* 
 * Valid date patterns
 *
 *	wday month dd year 
 *	wday dd month year
 *	wday dd month
 *	wday month day yyyy
 */
int
Blt_GetDate(Tcl_Interp *interp, const char *string, double *timePtr)
{
    /* First see if the date is a number (seconds). */
    if (Tcl_GetDouble(NULL, string, timePtr) == TCL_OK) {
	return TCL_OK;
    }
    return GetDate(interp, string, timePtr);
}

int
Blt_GetDateFromObj(Tcl_Interp *interp, Tcl_Obj *objPtr, double *timePtr)
{
    /* First see if the date is a number (seconds). */
    if (Tcl_GetDoubleFromObj(NULL, objPtr, timePtr) == TCL_OK) {
	return TCL_OK;
    }
    return GetDate(interp, Tcl_GetString(objPtr), timePtr);
}


/*#ifdef WIN32*/
extern char *strptime(const char *buf, const char *fmt, struct tm *tm);
/*#endif*/

static int
GetTime(Tcl_Interp *interp, const char *string, const char *fmt, 
	double *valuePtr)
{
    struct tm tm;
    const char *s;
    double fract;
    time_t sec;

    memset(&tm, 0, sizeof(struct tm));
    s = strptime(string, fmt, &tm);
    if ((s == NULL) || ((*s != '\0') && (*s != '.'))) {
	fprintf(stderr, "s=%s\n", s);
	Tcl_AppendResult(interp, "invalid time conversion for \"", string,
			 "\"", (char *)NULL);
	return TCL_ERROR;
    }
    tm.tm_isdst = -1;
    sec = mktime(&tm);
#ifdef notdef
    fprintf(stderr, "year=%d, month=%d day=%d\n", tm.tm_year, tm.tm_mon,
	    tm.tm_mday);
    fprintf(stderr, "hour=%d, minute=%d sec=%d\n", tm.tm_hour, tm.tm_min,
	    tm.tm_sec);
    fprintf(stderr, "yday=%d, wday=%d isdst=%d\n", tm.tm_yday, tm.tm_wday, tm.tm_isdst);
#endif
    fract = 0.0;
    if (*s == '.') {
	if (Tcl_GetDouble(interp, s, &fract) != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    *valuePtr = (double)sec + fract;
#ifdef notdef
    fprintf(stderr, "string=%s, value=%.17g sec=%ld, fract=%g\n", 
	    string, *valuePtr, sec, fract);
#endif
    return TCL_OK;
}

int
Blt_GetTimeFromObj(Tcl_Interp *interp, Tcl_Obj *objPtr, double *valuePtr) 
{
    const char *string;
    
    if (Tcl_GetDoubleFromObj(NULL, objPtr, valuePtr) == TCL_OK) {
	return TCL_OK;
    }
    string = Tcl_GetString(objPtr);
    return GetTime(interp, string, "%Y-%m-%d %H:%M:%S", valuePtr);
}

int
Blt_GetTime(Tcl_Interp *interp, const char *string, double *valuePtr) 
{
    if (Tcl_GetDouble(NULL, string, valuePtr) == TCL_OK) {
	return TCL_OK;
    }
    return GetTime(interp, string, "%Y-%m-%d %H:%M:%S", valuePtr);
}

static int
FormatOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    FormatSwitches switches;

    /* Process switches  */
    memset(&switches, 0, sizeof(switches));
    if (Blt_ParseSwitches(interp, formatSwitches, objc - 3, objv + 3, &switches,
			  BLT_SWITCH_DEFAULTS) < 0) {
	return TCL_ERROR;
    }
    Blt_FreeSwitches(formatSwitches, (char *)&switches, 0);
    return TCL_OK;
}


static int
ScanOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    double t;
    ScanSwitches switches;
    int result;

    /* Process switches  */
    memset(&switches, 0, sizeof(switches));
    if (Blt_ParseSwitches(interp, scanSwitches, objc - 3, objv + 3, &switches,
			  BLT_SWITCH_DEFAULTS) < 0) {
	return TCL_ERROR;
    }
    if (switches.fmtObjPtr != NULL) {
	const char *string, *fmt;

	string = Tcl_GetString(objv[2]);
	fmt = Tcl_GetString(switches.fmtObjPtr);
	result = GetTime(interp, string, fmt, &t);
    } else {
	result = Blt_GetDateFromObj(interp, objv[2], &t);
    }
    if (result == TCL_OK) {
	char string[200];

	sprintf(string, "%.15g", t);
	Tcl_SetStringObj(Tcl_GetObjResult(interp), string, -1);
    }
    Blt_FreeSwitches(scanSwitches, (char *)&switches, 0);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * DateObjCmd --
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec dateCmdOps[] =
{
    {"scan",    1, ScanOp,        3, 0, "date ?switches?",},
    {"format",  1, FormatOp,      3, 0, "seconds ?switches?",},
};

static int numCmdOps = sizeof(dateCmdOps) / sizeof(Blt_OpSpec);

/*ARGSUSED*/
static int
DateObjCmd(
    ClientData clientData,	/* Interpreter-specific data. */
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Tcl_ObjCmdProc *proc;

    proc = Blt_GetOpFromObj(interp, numCmdOps, dateCmdOps, BLT_OP_ARG1, objc, 
	objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    return (*proc) (clientData, interp, objc, objv);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_DateScanCmdInitProc --
 *
 *	This procedure is invoked to initialize the "tree" command.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Creates the new command and adds a new entry into a global Tcl
 *	associative array.
 *
 *---------------------------------------------------------------------------
 */
int
Blt_DateScanCmdInitProc(Tcl_Interp *interp)
{
    static Blt_CmdSpec cmdSpec = { 
	"date", DateObjCmd, 
    };
    return Blt_InitCmd(interp, "::blt", &cmdSpec);
}