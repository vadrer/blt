
/*
 * bltWinFont.c --
 *
 * This module implements rotated fonts for the BLT toolkit.  
 *
 * Jacks up the Tk font structure to allow fonts created at various angles.
 * Rotated fonts are created by digging out the Windows font handle from the
 * Tk font and then calling CreateFontIndirect to generate a font for that
 * angle.  The rotated fonts are stored in a hash table.
 *
 *	Copyright 2005-2011 George A Howlett.
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

#define BUILD_BLT_TK_PROCS 1
#include "bltInt.h"
#include <X11/Xutil.h>
#include <ctype.h>
#include "bltAlloc.h"
#include "bltMath.h"
#include "bltString.h"
#include <bltHash.h>
#include "tkDisplay.h"
#include "tkFont.h"
#include "bltFont.h"
#include "bltAfm.h"

#define WINDEBUG		0
#define DEBUG_FONT_SELECTION	0
#define DEBUG_FONT_SELECTION2	0

typedef struct _Blt_Font _Blt_Font;

enum FontsetTypes { 
    FONTSET_UNKNOWN,			/* Unknown font type. */
    FONTSET_TK, 			/* Normal Tk font. */
    FONTSET_WIN 			/* Windows font. */
};

#ifndef HAVE_LIBXFT
#define FC_WEIGHT_THIN		    0
#define FC_WEIGHT_EXTRALIGHT	    40
#define FC_WEIGHT_ULTRALIGHT	    FC_WEIGHT_EXTRALIGHT
#define FC_WEIGHT_LIGHT		    50
#define FC_WEIGHT_BOOK		    75
#define FC_WEIGHT_REGULAR	    80
#define FC_WEIGHT_NORMAL	    FC_WEIGHT_REGULAR
#define FC_WEIGHT_MEDIUM	    100
#define FC_WEIGHT_DEMIBOLD	    180
#define FC_WEIGHT_SEMIBOLD	    FC_WEIGHT_DEMIBOLD
#define FC_WEIGHT_BOLD		    200
#define FC_WEIGHT_EXTRABOLD	    205
#define FC_WEIGHT_ULTRABOLD	    FC_WEIGHT_EXTRABOLD
#define FC_WEIGHT_BLACK		    210
#define FC_WEIGHT_HEAVY		    FC_WEIGHT_BLACK
#define FC_WEIGHT_EXTRABLACK	    215
#define FC_WEIGHT_ULTRABLACK	    FC_WEIGHT_EXTRABLACK

#define FC_SLANT_ROMAN		    0
#define FC_SLANT_ITALIC		    100
#define FC_SLANT_OBLIQUE	    110

#define FC_WIDTH_ULTRACONDENSED	    50
#define FC_WIDTH_EXTRACONDENSED	    63
#define FC_WIDTH_CONDENSED	    75
#define FC_WIDTH_SEMICONDENSED	    87
#define FC_WIDTH_NORMAL		    100
#define FC_WIDTH_SEMIEXPANDED	    113
#define FC_WIDTH_EXPANDED	    125
#define FC_WIDTH_EXTRAEXPANDED	    150
#define FC_WIDTH_ULTRAEXPANDED	    200

#define FC_PROPORTIONAL		    0
#define FC_DUAL			    90
#define FC_MONO			    100
#define FC_CHARCELL		    110

#define FC_ANTIALIAS	    "antialias"		/* Bool (depends) */
#define FC_AUTOHINT	    "autohint"		/* Bool (false) */
#define FC_DECORATIVE	    "decorative"	/* Bool  */
#define FC_EMBEDDED_BITMAP  "embeddedbitmap"	/* Bool  */
#define FC_EMBOLDEN	    "embolden"		/* Bool */
#define FC_FAMILY	    "family"		/* String */
#define FC_GLOBAL_ADVANCE   "globaladvance"	/* Bool (true) */
#define FC_HINTING	    "hinting"		/* Bool (true) */
#define FC_MINSPACE	    "minspace"		/* Bool */
#define FC_OUTLINE	    "outline"		/* Bool */
#define FC_SCALABLE	    "scalable"		/* Bool */
#define FC_SIZE		    "size"		/* Double */
#define FC_SLANT	    "slant"		/* Int */
#define FC_SPACING	    "spacing"		/* Int */
#define FC_STYLE	    "style"		/* String */
#define FC_VERTICAL_LAYOUT  "verticallayout"	/* Bool (false) */
#define FC_WEIGHT	    "weight"		/* Int */
#define FC_WIDTH	    "width"		/* Int */

#endif

#ifndef FC_WEIGHT_EXTRABLACK
#define FC_WEIGHT_EXTRABLACK	    215
#define FC_WEIGHT_ULTRABLACK	    FC_WEIGHT_EXTRABLACK
#endif

typedef struct {
    char *family;
    const char *weight;
    const char *slant;
    const char *width;
    const char *spacing;
    int size;				/* If negative, pixels, otherwise
					 * points */
} TkFontPattern;

typedef struct {
    const char *name;
    int minChars;
    const char *key;
    int value;
    const char *oldvalue;
} FontSpec;
    
static FontSpec fontSpecs[] = {
    { "black",        2, FC_WEIGHT,  FC_WEIGHT_BLACK,     "*"},
    { "bold",         3, FC_WEIGHT,  FC_WEIGHT_BOLD,      "bold"},
    { "book",         3, FC_WEIGHT,  FC_WEIGHT_MEDIUM,	  "medium"},
    { "charcell",     2, FC_SPACING, FC_CHARCELL,	  "c"},
    { "condensed",    2, FC_WIDTH,   FC_WIDTH_CONDENSED,  "condensed"},
    { "demi",         4, FC_WEIGHT,  FC_WEIGHT_BOLD,      "semi"},
    { "demibold",     5, FC_WEIGHT,  FC_WEIGHT_DEMIBOLD,  "semibold"},
    { "dual",         2, FC_SPACING, FC_DUAL,		  "*"},
    { "i",            1, FC_SLANT,   FC_SLANT_ITALIC,	  "italic"},
    { "italic",       2, FC_SLANT,   FC_SLANT_ITALIC,	  "italic"},
    { "light",        1, FC_WEIGHT,  FC_WEIGHT_LIGHT,	  "light"},
    { "medium",       2, FC_WEIGHT,  FC_WEIGHT_MEDIUM,	  "medium"},
    { "mono",         2, FC_SPACING, FC_MONO,		  "m"},
    { "normal",       1, FC_WIDTH,   FC_WIDTH_NORMAL,	  "normal"},
    { "o",            1, FC_SLANT,   FC_SLANT_OBLIQUE,	  "oblique"},
    { "obilque",      2, FC_SLANT,   FC_SLANT_OBLIQUE,	  "oblique"},
    { "overstrike",   2, NULL,       0,			  "*"},
    { "proportional", 1, FC_SPACING, FC_PROPORTIONAL,	  "p"},
    { "r",            1, FC_SLANT,   FC_SLANT_ROMAN,      "roman"},
    { "roman",        2, FC_SLANT,   FC_SLANT_ROMAN,      "roman"},
    { "semibold",     5, FC_WEIGHT,  FC_WEIGHT_DEMIBOLD,  "semibold"},
    { "semicondensed",5, FC_WIDTH,   FC_WIDTH_SEMICONDENSED,  "semicondensed"},
    { "underline",    1, NULL,       0,		          "*"},
};
static int numFontSpecs = sizeof(fontSpecs) / sizeof(FontSpec);

static FontSpec weightSpecs[] ={
    { "black",		2, FC_WEIGHT, FC_WEIGHT_BLACK,	    "bold"},
    { "bold",		3, FC_WEIGHT, FC_WEIGHT_BOLD,	    "bold"},
    { "book",		3, FC_WEIGHT, FC_WEIGHT_MEDIUM,	    "*"},
    { "demi",		4, FC_WEIGHT, FC_WEIGHT_BOLD,	    "*"},
    { "demibold",	5, FC_WEIGHT, FC_WEIGHT_DEMIBOLD,   "*"},
    { "extrablack",	6, FC_WEIGHT, FC_WEIGHT_EXTRABLACK, "*"},
    { "extralight",	6, FC_WEIGHT, FC_WEIGHT_EXTRALIGHT, "*"},
    { "heavy",		1, FC_WEIGHT, FC_WEIGHT_HEAVY,      "*"},
    { "light",		1, FC_WEIGHT, FC_WEIGHT_LIGHT,	    "light"},
    { "medium",		1, FC_WEIGHT, FC_WEIGHT_MEDIUM,	    "medium"},
    { "normal",		1, FC_WEIGHT, FC_WEIGHT_MEDIUM,	    "normal"},
    { "regular",	1, FC_WEIGHT, FC_WEIGHT_REGULAR,    "medium"},
    { "semibold",	1, FC_WEIGHT, FC_WEIGHT_SEMIBOLD,   "semibold"},
    { "thin",		1, FC_WEIGHT, FC_WEIGHT_THIN,       "thin"},
    { "ultrablack",	7, FC_WEIGHT, FC_WEIGHT_ULTRABLACK, "*"},
    { "ultrabold",	7, FC_WEIGHT, FC_WEIGHT_ULTRABOLD,  "*"},
    { "ultralight",	6, FC_WEIGHT, FC_WEIGHT_ULTRALIGHT, "*"},
};
static int numWeightSpecs = sizeof(weightSpecs) / sizeof(FontSpec);

static FontSpec slantSpecs[] ={
    { "i",		1, FC_SLANT, FC_SLANT_ITALIC,	"italic"},
    { "italic",		2, FC_SLANT, FC_SLANT_ITALIC,	"italic"},
    { "o",		1, FC_SLANT, FC_SLANT_OBLIQUE,	"oblique"},
    { "obilque",	3, FC_SLANT, FC_SLANT_OBLIQUE,	"oblique"},
    { "r",		1, FC_SLANT, FC_SLANT_ROMAN,	"roman"},
    { "roman",		2, FC_SLANT, FC_SLANT_ROMAN,	"roman"},
};
static int numSlantSpecs = sizeof(slantSpecs) / sizeof(FontSpec);

static FontSpec spacingSpecs[] = {
    { "charcell",     2, FC_SPACING, FC_CHARCELL,	  "c"},
    { "dual",         2, FC_SPACING, FC_DUAL,		  "*"},
    { "mono",         2, FC_SPACING, FC_MONO,		  "m"},
    { "proportional", 1, FC_SPACING, FC_PROPORTIONAL,	  "p"},
};
static int numSpacingSpecs = sizeof(spacingSpecs) / sizeof(FontSpec);

enum XLFDFields { 
    XLFD_FOUNDRY, 
    XLFD_FAMILY, 
    XLFD_WEIGHT, 
    XLFD_SLANT,	
    XLFD_SETWIDTH, 
    XLFD_ADD_STYLE, 
    XLFD_PIXEL_SIZE,
    XLFD_POINT_SIZE, 
    XLFD_RESOLUTION_X, 
    XLFD_RESOLUTION_Y,
    XLFD_SPACING, 
    XLFD_AVERAGE_WIDTH, 
    XLFD_CHARSET,
    XLFD_NUMFIELDS
};

typedef struct {
    const char *name, *aliases[10];
} FontAlias;

static FontAlias xlfdFontAliases[] = {
    { "math",		{"arial", "courier new"}},
    { "serif",		{"times"}},
    { "sans serif",	{ "arial" }},
    { "monospace",	{ "courier new" }},
    { NULL }
};

static int font_initialized = 0;
static Blt_HashTable aliasTable;
static Blt_HashTable fontTable;

static void TkGetFontFamilies(Tk_Window tkwin, Blt_HashTable *tablePtr);

static double
PointsToPixels(Tk_Window tkwin, int size)
{
    double d;

    if (size < 0) {
        return -size;
    }
    d = size * 25.4 / 72.0;
    d *= WidthOfScreen(Tk_Screen(tkwin));
    d /= WidthMMOfScreen(Tk_Screen(tkwin));
    return d;
}

#ifdef notdef
static double
PixelsToPoints(Tk_Window tkwin, int size)		
{
    double d;

    if (size >= 0) {
	return size;
    }
    d = -size * 72.0 / 25.4;
    d *= WidthMMOfScreen(Tk_Screen(tkwin));
    d /= WidthOfScreen(Tk_Screen(tkwin));
    return d;
}

static void
ParseXLFD(const char *fontName, int *argcPtr, char ***argvPtr)
{
    char *p, *pend, *desc, *buf;
    size_t arrayLen, stringLen;
    int count;
    char **field;

    arrayLen = (sizeof(char *) * (XLFD_NUMFIELDS + 1));
    stringLen = strlen(fontName);
    buf = Blt_AssertCalloc(1, arrayLen + stringLen + 1);
    desc = buf + arrayLen;
    strcpy(desc, fontName);
    field = (char **)buf;

    count = 0;
    for (p = desc, pend = p + stringLen; p < pend; p++, count++) {
	char *word;

	field[count] = NULL;
	/* Get the next word, separated by dashes (-). */
	word = p;
	while ((*p != '\0') && (*p != '-')) {
	    if (((*p & 0x80) == 0) && Tcl_UniCharIsUpper(UCHAR(*p))) {
		*p = (char)Tcl_UniCharToLower(UCHAR(*p));
	    }
	    p++;
	}
	if (*p != '\0') {
	    *p = '\0';
	}
	if ((word[0] == '\0') || 
	    (((word[0] == '*') || (word[0] == '?')) && (word[1] == '\0'))) {
	    continue;			/* Field not specified. -- -*- -?- */
	}
	field[count] = word;
    }

    /*
     * An XLFD of the form -adobe-times-medium-r-*-12-*-* is pretty common,
     * but it is (strictly) malformed, because the first * is eliding both the
     * Setwidth and the Addstyle fields. If the Addstyle field is a number,
     * then assume the above incorrect form was used and shift all the rest of
     * the fields right by one, so the number gets interpreted as a pixelsize.
     * This fix is so that we don't get a million reports that "it works under
     * X (as a native font name), but gives a syntax error under Windows (as a
     * parsed set of attributes)".
     */

    if ((count > XLFD_ADD_STYLE) && (field[XLFD_ADD_STYLE] != NULL)) {
	int dummy;

	if (Tcl_GetInt(NULL, field[XLFD_ADD_STYLE], &dummy) == TCL_OK) {
	    int j;
	    
	    for (j = XLFD_NUMFIELDS - 1; j >= XLFD_ADD_STYLE; j--) {
		field[j + 1] = field[j];
	    }
	    field[XLFD_ADD_STYLE] = NULL;
	    count++;
	}
    }
    *argcPtr = count;
    *argvPtr = field;

    field[XLFD_NUMFIELDS] = NULL;
}
#endif

/*
 *---------------------------------------------------------------------------
 *
 * SearchForFontSpec --
 *
 *      Performs a binary search on the array of font specification to find a
 *      partial, anchored match for the given option string.
 *
 * Results:
 *	If the string matches unambiguously the index of the specification in
 *	the array is returned.  If the string does not match, even as an
 *	abbreviation, any operation, -1 is returned.  If the string matches,
 *	but ambiguously -2 is returned.
 *
 *---------------------------------------------------------------------------
 */
static int
SearchForFontSpec(FontSpec *table, int numSpecs, const char *string, int length)
{
    char c;
    int high, low;

    low = 0;
    high = numSpecs - 1;
    c = tolower((unsigned char)string[0]);
    if (length < 0) {
	length = strlen(string);
    }
    while (low <= high) {
	FontSpec *sp;
	int compare;
	int median;
	
	median = (low + high) >> 1;
	sp = table + median;

	/* Test the first character */
	compare = c - sp->name[0];
	if (compare == 0) {
	    /* Now test the entire string */
	    compare = strncasecmp(string, sp->name, length);
	    if (compare == 0) {
		if ((int)length < sp->minChars) {
		    return -2;		/* Ambiguous spec name */
		}
	    }
	}
	if (compare < 0) {
	    high = median - 1;
	} else if (compare > 0) {
	    low = median + 1;
	} else {
	    return median;		/* Spec found. */
	}
    }
    return -1;				/* Can't find spec */
}

static FontSpec *
FindSpec(Tcl_Interp *interp, FontSpec *tablePtr, int numSpecs, const char *string,
	 int length)
{
    int n;
    
    n = SearchForFontSpec(tablePtr, numSpecs, string, length);
    if (n < 0) {
	if (n == -1) {
	    if (interp != NULL) {
		Tcl_AppendResult(interp, "unknown ", tablePtr[0].key,
			     " specification \"", string, "\"", (char *)NULL); 
	    }
	}
	if (n == -2) {
	    if (interp != NULL) {
		Tcl_AppendResult(interp, "ambiguous ", tablePtr[0].key,
			     " specification \"", string, "\"", (char *)NULL); 
	    }
	}
	return NULL;
    }
    return tablePtr + n;
}


static void
MakeAliasTable(Tk_Window tkwin)
{
    Blt_HashTable familyTable;
    FontAlias *fp;
    FontAlias *table;

    Blt_InitHashTable(&familyTable, TCL_STRING_KEYS);
    TkGetFontFamilies(tkwin, &familyTable);
    Blt_InitHashTable(&aliasTable, TCL_STRING_KEYS);
    table = xlfdFontAliases;
    for(fp = table; fp->name != NULL; fp++) {
	Blt_HashEntry *hPtr;
	const char **alias;
	   
	for (alias = fp->aliases; *alias != NULL; alias++) {
	    hPtr = Blt_FindHashEntry(&familyTable, *alias);
	    if (hPtr != NULL) {
		int isNew;
		
		hPtr = Blt_CreateHashEntry(&aliasTable, fp->name, &isNew);
		Blt_SetHashValue(hPtr, *alias);
		break;
	    }
	}
    }
    Blt_DeleteHashTable(&familyTable);
}

static const char *
GetAlias(const char *family)
{
    Blt_HashEntry *hPtr;
    char *lower;
    
    lower = Blt_AssertStrdup(family);
    hPtr = Blt_FindHashEntry(&aliasTable, lower);
    Blt_Free(lower);
    if (hPtr != NULL) {
	return Blt_GetHashValue(hPtr);
    }
    return family;
}

static Blt_Font_CanRotateProc		TkCanRotateProc;
static Blt_Font_DrawProc		TkDrawProc;
static Blt_Font_FamilyProc		TkFamilyProc;
static Blt_Font_FreeProc		TkFreeProc;
static Blt_Font_GetMetricsProc		TkGetMetricsProc;
static Blt_Font_IdProc			TkIdProc;
static Blt_Font_MeasureProc		TkMeasureProc;
static Blt_Font_NameProc		TkNameProc;
static Blt_Font_PostscriptNameProc	TkPostscriptNameProc;
static Blt_Font_TextWidthProc		TkTextWidthProc;
static Blt_Font_UnderlineProc		TkUnderlineProc;

static Blt_FontClass tkFontClass = {
    FONTSET_TK,
    TkCanRotateProc,			/* Blt_Font_CanRotateProc */
    TkDrawProc,				/* Blt_Font_DrawProc */
    TkFamilyProc,			/* Blt_Font_FamilyProc */
    TkFreeProc,				/* Blt_Font_FreeProc */
    TkGetMetricsProc,			/* Blt_Font_GetMetricsProc */
    TkIdProc,				/* Blt_Font_IdProc */
    TkMeasureProc,			/* Blt_Font_MeasureProc */
    TkNameProc,				/* Blt_Font_NameProc */
    TkPostscriptNameProc,		/* Blt_Font_PostscriptNameProc */
    TkTextWidthProc,			/* Blt_Font_TextWidthProc */
    TkUnderlineProc,			/* Blt_Font_UnderlineProc */
};

static TkFontPattern *
TkNewFontPattern(void)
{
    TkFontPattern *patternPtr;

    patternPtr = Blt_Calloc(1, sizeof(TkFontPattern));
    return patternPtr;
}

static void
TkFreeFontPattern(TkFontPattern *patternPtr)
{
    if (patternPtr->family != NULL) {
	Blt_Free((char *)patternPtr->family);
    }
    Blt_Free(patternPtr);
}

static int CALLBACK
TkFontFamilyEnumProc(
    ENUMLOGFONT *lfPtr,			/* Logical-font data. */
    NEWTEXTMETRIC *tmPtr,		/* Physical-font data (not used). */
    int fontType,			/* Type of font (not used). */
    LPARAM lParam)			/* Result object to hold result. */
{
    Blt_HashEntry *hPtr;
    Blt_HashTable *tablePtr;
    Tcl_DString ds;
    const char *faceName;
    Tcl_Encoding encoding;
    int isNew;

    tablePtr = (Blt_HashTable *)lParam;
    faceName = lfPtr->elfLogFont.lfFaceName;
    encoding = Tcl_GetEncoding(NULL, "Unicode");
    Tcl_ExternalToUtfDString(encoding, faceName, -1, &ds);
    faceName = Tcl_DStringValue(&ds);
    Blt_LowerCase((char *)faceName);
    hPtr = Blt_CreateHashEntry(tablePtr, faceName, &isNew);
    Blt_SetHashValue(hPtr, NULL);
    Tcl_DStringFree(&ds);
    return 1;
}

static void
TkGetFontFamilies(Tk_Window tkwin, Blt_HashTable *tablePtr)
{    
    HDC hDC;
    HWND hWnd;
    Window window;

    window = Tk_WindowId(tkwin);
    hWnd = (window == None) ? (HWND)NULL : Tk_GetHWND(window);
    hDC = GetDC(hWnd);

    /*
     * On any version NT, there may fonts with international names.  
     * Use the NT-only Unicode version of EnumFontFamilies to get the 
     * font names.  If we used the ANSI version on a non-internationalized 
     * version of NT, we would get font names with '?' replacing all 
     * the international characters.
     *
     * On a non-internationalized verson of 95, fonts with international
     * names are not allowed, so the ANSI version of EnumFontFamilies will 
     * work.  On an internationalized version of 95, there may be fonts with 
     * international names; the ANSI version will work, fetching the 
     * name in the system code page.  Can't use the Unicode version of 
     * EnumFontFamilies because it only exists under NT.
     */

    if (Blt_GetPlatformId() == VER_PLATFORM_WIN32_NT) {
	EnumFontFamiliesW(hDC, NULL, (FONTENUMPROCW)TkFontFamilyEnumProc,
		(LPARAM)tablePtr);
    } else {
	EnumFontFamiliesA(hDC, NULL, (FONTENUMPROCA)TkFontFamilyEnumProc,
		(LPARAM)tablePtr);
    }	    
    ReleaseDC(hWnd, hDC);
}

/*
 *---------------------------------------------------------------------------
 *
 * TkParseTkDesc --
 *
 *	Parses an array of Tcl_Objs as a Tk style font description .  
 *	
 *	      "family [size] [optionList]"
 *
 * Results:
 *	Returns a pattern structure, filling in with the necessary fields.
 *	Returns NULL if objv doesn't contain a  Tk font description.
 *
 * Side effects:
 *	Memory is allocated for the font pattern and the its strings.
 *
 *---------------------------------------------------------------------------
 */
static TkFontPattern *
TkParseTkDesc(Tcl_Interp *interp, int objc, Tcl_Obj **objv)
{
    TkFontPattern *patternPtr;
    Tcl_Obj **aobjv;
    int aobjc;
    int i;

    patternPtr = TkNewFontPattern();

    /* Font family. */
    {
	char *family, *dash;

	family = Tcl_GetString(objv[0]);
	dash = strchr(family, '-');
	if (dash != NULL) {
	    int size;
	    
	    if (Tcl_GetInt(NULL, dash + 1, &size) != TCL_OK) {
		goto error;
	    }
	    patternPtr->size = size;
	}
	if (dash != NULL) {
	    *dash = '\0';
	}
	patternPtr->family = Blt_AssertStrdup(GetAlias(family));
	if (dash != NULL) {
	    *dash = '-';
	}
	objv++, objc--;
    }
    if (objc > 0) {
	int size;

	if (Tcl_GetIntFromObj(NULL, objv[0], &size) == TCL_OK) {
	    patternPtr->size = size;
	    objv++, objc--;
	}
    }
    aobjc = objc, aobjv = objv;
    if (objc > 0) {
	if (Tcl_ListObjGetElements(NULL, objv[0], &aobjc, &aobjv) != TCL_OK) {
	    goto error;
	}
    }
    for (i = 0; i < aobjc; i++) {
	FontSpec *specPtr;
	const char *key;
	int length;

	key = Tcl_GetStringFromObj(aobjv[i], &length);
	specPtr = FindSpec(interp, fontSpecs, numFontSpecs, key, length);
	if (specPtr == NULL) {
	    goto error;
	}
	if (specPtr->key == NULL) {
	    continue;
	}
	if (strcmp(specPtr->key, FC_WEIGHT) == 0) {
	    patternPtr->weight = specPtr->oldvalue;
	} else if (strcmp(specPtr->key, FC_SLANT) == 0) {
	    patternPtr->slant = specPtr->oldvalue;
	} else if (strcmp(specPtr->key, FC_SPACING) == 0) {
	    patternPtr->spacing = specPtr->oldvalue;
	} else if (strcmp(specPtr->key, FC_WIDTH) == 0) {
	    patternPtr->width = specPtr->oldvalue;
	}
    }
    return patternPtr;
 error:
    TkFreeFontPattern(patternPtr);
    return NULL;
}	

/*
 *---------------------------------------------------------------------------
 *
 * TkParseNameValuePairs --
 *
 *	Given Tcl_Obj list of name value pairs, parse the list saving in the
 *	values in a font pattern structure.
 *	
 *	      "-family family -size size -weight weight"
 *
 * Results:
 *	Returns a pattern structure, filling in with the necessary fields.
 *	Returns NULL if objv doesn't contain a valid name-value list 
 *	describing a font.
 *
 * Side effects:
 *	Memory is allocated for the font pattern and the its strings.
 *
 *---------------------------------------------------------------------------
 */
static TkFontPattern *
TkParseNameValuePairs(Tcl_Interp *interp, Tcl_Obj *objPtr) 
{
    TkFontPattern *patternPtr;
    Tcl_Obj **objv;
    int objc;
    int i;

    if ((Tcl_ListObjGetElements(NULL, objPtr, &objc, &objv) != TCL_OK) ||
	(objc < 1)) {
	return NULL;			/* Can't split list or list is
					 * empty. */
    }
    if (objc & 0x1) {
	if (interp != NULL) {
	    Tcl_AppendResult(interp, "odd number of elements, missing value",
			 (char *)NULL);
	}
	return NULL;			/* Odd number of elements in list. */
    }
    patternPtr = TkNewFontPattern();
    for (i = 0; i < objc; i += 2) {
	const char *key, *value;
	int length;

	key = Tcl_GetString(objv[i]);
	value = Tcl_GetStringFromObj(objv[i+1], &length);
	if (strcmp(key, "-family") == 0) {
	    if (patternPtr->family != NULL) {
		Blt_Free(patternPtr->family);
	    }
	    patternPtr->family = Blt_AssertStrdup(GetAlias(value));
	} else if (strcmp(key, "-size") == 0) {
	    int size;

	    if (Tcl_GetIntFromObj(interp, objv[i+1], &size) != TCL_OK) {
		goto error;
	    }
#ifdef notdef
	    patternPtr->size = PointsToPixels(Tk_MainWindow(interp), size);
#else
	    patternPtr->size = size;
#endif
	} else if (strcmp(key, "-weight") == 0) {
	    FontSpec *specPtr;

	    specPtr = FindSpec(interp, weightSpecs, numWeightSpecs, value, 
		length);
	    if (specPtr == NULL) {
		goto error;
	    }
	    patternPtr->weight = specPtr->oldvalue;
	} else if (strcmp(key, "-slant") == 0) {
	    FontSpec *specPtr;

	    specPtr = FindSpec(interp, slantSpecs, numSlantSpecs, value, length);
	    if (specPtr == NULL) {
		goto error;
	    }
	    patternPtr->slant = specPtr->oldvalue;
	} else if (strcmp(key, "-spacing") == 0) {
	    FontSpec *specPtr;

	    specPtr = FindSpec(interp, spacingSpecs, numSpacingSpecs, value, 
		length);
	    if (specPtr == NULL) {
		goto error;
	    }
	    patternPtr->spacing = specPtr->oldvalue;
	} else if (strcmp(key, "-hint") == 0) {
	    /* Ignore */
	} else if (strcmp(key, "-rgba") == 0) {
	    /* Ignore */
	} else if (strcmp(key, "-underline") == 0) {
	    /* Ignore */
	} else if (strcmp(key, "-overstrike") == 0) {
	    /* Ignore */
	} else {
	    /* Ignore */
	}
    }
#if DEBUG_FONT_SELECTION
    fprintf(stderr, "found TkAttrList => Tk font \"%s\"\n", 
	    Tcl_GetString(objPtr));
#endif
    return patternPtr;
 error:
    TkFreeFontPattern(patternPtr);
    return NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * TkParseFontObj --
 *
 *	Given the name of a Tk font object, get its configuration values 
 *	save the data in a font pattern structure.
 *	
 *	      "-family family -size size -weight weight"
 *
 * Results:
 *	Returns a pattern structure, filling in with the necessary fields.
 *	Returns NULL if objv doesn't contain a valid name-value list 
 *	describing a font.
 *
 * Side effects:
 *	Memory is allocated for the font pattern and the its strings.
 *
 *---------------------------------------------------------------------------
 */
static TkFontPattern *
TkParseFontObj(Tcl_Interp *interp, Tcl_Obj *objPtr) 
{
    TkFontPattern *patternPtr;
    Tcl_Obj *cmd[3];
    int result;

    patternPtr = NULL;
    cmd[0] = Tcl_NewStringObj("font", -1);
    cmd[1] = Tcl_NewStringObj("configure", -1);
    cmd[2] = objPtr;
    Tcl_IncrRefCount(cmd[0]);
    Tcl_IncrRefCount(cmd[1]);
    Tcl_IncrRefCount(cmd[2]);
    result = Tcl_EvalObjv(interp, 3, cmd, 0);
    Tcl_DecrRefCount(cmd[2]);
    Tcl_DecrRefCount(cmd[1]);
    Tcl_DecrRefCount(cmd[0]);
    if (result == TCL_OK) {
	patternPtr = TkParseNameValuePairs(interp, Tcl_GetObjResult(interp));
    }
    Tcl_ResetResult(interp);
#if DEBUG_FONT_SELECTION
    if (patternPtr != NULL) {
	fprintf(stderr, "found FontObject => Tk font \"%s\"\n", 
	    Tcl_GetString(objPtr));
    }
#endif
    return patternPtr;
}

/* 
 *---------------------------------------------------------------------------
 *
 * TkGetPattern --
 * 
 *	Parses the font description so that the font can rewritten with an
 *	aliased font name.  This allows us to use
 *
 *	  "Sans Serif", "Serif", "Math", "Monospace"
 *
 *	font names that correspond to the proper font regardless if the
 *	standard X fonts or XFT fonts are being used.
 *
 *	Leave XLFD font descriptions alone.  Let users describe exactly the
 *	font they wish.
 *
 *---------------------------------------------------------------------------
 */
static TkFontPattern *
TkGetPattern(Tcl_Interp *interp, Tcl_Obj *objPtr)
{
    TkFontPattern *patternPtr;
    const char *desc;

    desc = Tcl_GetString(objPtr);
    while (isspace(UCHAR(*desc))) {
	desc++;				/* Skip leading blanks. */
    }
    if (*desc == '-') {
	/* 
	 * Case 1: XLFD font description or Tk attribute list.   
	 *
	 *   If the font description starts with a '-', it could be either an
	 *   old fashion XLFD font description or a Tk font attribute
	 *   option-value list.
	 */
	patternPtr = TkParseNameValuePairs(interp, objPtr);
	if (patternPtr == NULL) {
	    return NULL;		/* XLFD font description */
	}
    } else if (*desc == '*') {
	return NULL;			/* XLFD font description */
    } else if (strpbrk(desc, "::") != NULL) {
	patternPtr = TkParseFontObj(interp, objPtr);
    } else {
	int objc;
	Tcl_Obj **objv;
	/* 
	 * Case 3: Tk-style description.   
	 */
	if ((Tcl_ListObjGetElements(NULL, objPtr, &objc, &objv) != TCL_OK) || 
	    (objc < 1)) {
	    return NULL;		/* Can't split into a list or
					 * list is empty. */
	}
	patternPtr = NULL;
	if (objc == 1) {
	    /* 
	     * Case 3a: Tk font object name.
	     *
	     *   Assuming that Tk font object names won't contain whitespace,
	     *   see if its a font object.
	     */
	    patternPtr = TkParseFontObj(interp, objv[0]);
	} 
	if (patternPtr == NULL) {
	    /* 
	     * Case 3b: List of font attributes in the form "family size
	     *		?attrs?"
	     */
	    patternPtr = TkParseTkDesc(interp, objc, objv);
	}
    }	
    return patternPtr;
}

static void
TkWriteXLFDDescription(Tk_Window tkwin, TkFontPattern *patternPtr, 
		       Tcl_DString *resultPtr)
{
    int size;
    
    /* Rewrite the font description using the aliased family. */
    Tcl_DStringInit(resultPtr);

    /* Family */
    if (patternPtr->family != NULL) {
	Tcl_DStringAppendElement(resultPtr, "-family");
	Tcl_DStringAppendElement(resultPtr, patternPtr->family);
    }
    /* Weight */
    if (patternPtr->weight != NULL) {
	Tcl_DStringAppendElement(resultPtr, "-weight");
	Tcl_DStringAppendElement(resultPtr, patternPtr->weight);
    }
    /* Slant */
    if (patternPtr->slant != NULL) {
	Tcl_DStringAppendElement(resultPtr, "-slant");
	Tcl_DStringAppendElement(resultPtr, patternPtr->slant);
    }
    /* Width */
    if (patternPtr->width != NULL) {
	Tcl_DStringAppendElement(resultPtr, "-width");
	Tcl_DStringAppendElement(resultPtr, patternPtr->width);
    }
    /* Size */
    Tcl_DStringAppendElement(resultPtr, "-size");
    size = (int)(PointsToPixels(tkwin, patternPtr->size) + 0.5);
    size = patternPtr->size;
    Tcl_DStringAppendElement(resultPtr, Blt_Itoa(size));
}
    

static Tk_Font 
TkGetFontFromObj(Tcl_Interp *interp, Tk_Window tkwin, Tcl_Obj *objPtr)
{
    Tk_Font tkFont;
    TkFontPattern *patternPtr;

    if (!font_initialized) {
	Blt_InitHashTable(&fontTable, BLT_STRING_KEYS);
	MakeAliasTable(tkwin);
	font_initialized++;
    }
    patternPtr = TkGetPattern(interp, objPtr);
    if (patternPtr == NULL) {
	tkFont = Tk_GetFont(interp, tkwin, Tcl_GetString(objPtr));
    } else {
	Tcl_DString ds;

	/* Rewrite the font description using the aliased family. */
	TkWriteXLFDDescription(tkwin, patternPtr, &ds);
	tkFont = Tk_GetFont(interp, tkwin, Tcl_DStringValue(&ds));
	Tcl_DStringFree(&ds);
	TkFreeFontPattern(patternPtr);
    }
    return tkFont;
}


static const char *
TkNameProc(_Blt_Font *fontPtr) 
{
    return Tk_NameOfFont(fontPtr->clientData);
}

static const char *
TkFamilyProc(_Blt_Font *fontPtr) 
{
    return ((TkFont *)fontPtr->clientData)->fa.family;
}

static Font
TkIdProc(_Blt_Font *fontPtr) 
{
    return Tk_FontId(fontPtr->clientData);
}

static void
TkGetMetricsProc(_Blt_Font *fontPtr, Blt_FontMetrics *fmPtr)
{
    TkFont *tkFontPtr = fontPtr->clientData;
    Tk_FontMetrics fm;

    Tk_GetFontMetrics(fontPtr->clientData, &fm);
    fmPtr->ascent = fm.ascent;
    fmPtr->descent = fm.descent;
    fmPtr->linespace = fm.linespace;
    fmPtr->tabWidth = tkFontPtr->tabWidth;
    fmPtr->underlinePos = tkFontPtr->underlinePos;
    fmPtr->underlineHeight = tkFontPtr->underlineHeight;
}

static int
TkMeasureProc(_Blt_Font *fontPtr, const char *text, int numBytes, int max, 
		   int flags, int *lengthPtr)
{
    return Tk_MeasureChars(fontPtr->clientData, text, numBytes, max, flags, 
	lengthPtr);
}

static int
TkTextWidthProc(_Blt_Font *fontPtr, const char *string, int numBytes)
{
    return Tk_TextWidth(fontPtr->clientData, string, numBytes);
}    

static void
TkDrawProc(
    Display *display,			/* Display on which to draw. */
    Drawable drawable,			/* Window or pixmap in which to
					 * draw. */
    GC gc,				/* Graphics context for drawing
					 * characters. */
    _Blt_Font *fontPtr,			/* Font in which characters will be
					 * drawn; must be the same as font
					 * used in GC. */
    int depth,				/* Not used. */
    float angle,			/* Not used. */
    const char *text,			/* UTF-8 string to be displayed.  Need
					 * not be '\0' terminated.  All Tk
					 * meta-characters (tabs, control
					 * characters, and newlines) should be
					 * stripped out of the string that is
					 * passed to this function.  If they
					 * are not stripped out, they will be
					 * displayed as regular printing
					 * characters. */
    int numBytes,				/* Number of bytes in string. */
    int x, int y)			/* Coordinates at which to place
					 * origin of string when drawing. */
{
    if (fontPtr->rgn != NULL) {
	TkSetRegion(display, gc, fontPtr->rgn);
	Tk_DrawChars(display, drawable, gc, fontPtr->clientData, text, numBytes, 
		x, y);
	XSetClipMask(display, gc, None);
    } else {
	Tk_DrawChars(display, drawable, gc, fontPtr->clientData, text, numBytes, 
		x, y);
    }
}

static int
TkPostscriptNameProc(_Blt_Font *fontPtr, Tcl_DString *resultPtr) 
{
    TkFont *tkFontPtr;
    unsigned int flags;

    tkFontPtr = (TkFont *)fontPtr->clientData;
    flags = 0;
    if (tkFontPtr->fa.slant != TK_FS_ROMAN) {
	flags |= FONT_ITALIC;
    }
    if (tkFontPtr->fa.weight != TK_FW_NORMAL) {
	flags |= FONT_BOLD;
    }
    Blt_Afm_GetPostscriptName(tkFontPtr->fa.family, flags, resultPtr);
    return tkFontPtr->fa.size;
}

static int
TkCanRotateProc(_Blt_Font *fontPtr, float angle) 
{
    return FALSE;
}

static void
TkFreeProc(_Blt_Font *fontPtr) 
{
    Tk_FreeFont(fontPtr->clientData);
    Blt_Free(fontPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * TkUnderlineProc --
 *
 *	This procedure draws an underline for a given range of characters in a
 *	given string.  It doesn't draw the characters (which are assumed to
 *	have been displayed previously); it just draws the underline.  This
 *	procedure would mainly be used to quickly underline a few characters
 *	without having to construct an underlined font.  To produce properly
 *	underlined text, the appropriate underlined font should be constructed
 *	and used.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information gets displayed in "drawable".
 *
 *---------------------------------------------------------------------------
 */
static void
TkUnderlineProc(
    Display *display,			/* Display on which to draw. */
    Drawable drawable,			/* Window or pixmap in which to
					 * draw. */
    GC gc,				/* Graphics context for actually
					 * drawing line. */
    _Blt_Font *fontPtr,			/* Font used in GC; must have been
					 * allocated by Tk_GetFont().  Used
					 * for character dimensions, etc. */
    const char *text,			/* String containing characters to be
					 * underlined or overstruck. */
    int textLen,			/* Unused. */
    int x, int y,			/* Coordinates at which first
					 * character of string is drawn. */
    int first,				/* Byte offset of the first
					 * character. */
    int last,				/* Byte offset after the last
					 * character. */
    int xMax)
{
    Tk_UnderlineChars(display, drawable, gc, fontPtr->clientData, text, x, y, 
	first, last);
}

static Blt_Font_CanRotateProc		WinCanRotateProc;
static Blt_Font_DrawProc		WinDrawProc;
static Blt_Font_FamilyProc		WinFamilyProc;
static Blt_Font_FreeProc		WinFreeProc;
static Blt_Font_GetMetricsProc		WinGetMetricsProc;
static Blt_Font_IdProc			WinIdProc;
static Blt_Font_MeasureProc		WinMeasureProc;
static Blt_Font_NameProc		WinNameProc;
static Blt_Font_PostscriptNameProc	WinPostscriptNameProc;
static Blt_Font_TextWidthProc		WinTextWidthProc;
static Blt_Font_UnderlineProc		WinUnderlineProc;

static Blt_FontClass winFontClass = {
    FONTSET_WIN,
    WinCanRotateProc,			/* Blt_Font_CanRotateProc */
    WinDrawProc,			/* Blt_Font_DrawProc */
    WinFamilyProc,			/* Blt_Font_FamilyProc */
    WinFreeProc,			/* Blt_Font_FreeProc */
    WinGetMetricsProc,			/* Blt_Font_GetMetricsProc */
    WinIdProc,				/* Blt_Font_IdProc */
    WinMeasureProc,			/* Blt_Font_MeasureProc */
    WinNameProc,			/* Blt_Font_NameProc */
    WinPostscriptNameProc,		/* Blt_Font_PostscriptNameProc */
    WinTextWidthProc,			/* Blt_Font_TextWidthProc */
    WinUnderlineProc,			/* Blt_Font_UnderlineProc */
};

/* 
 * Windows font container.
 */
typedef struct {
    const char *name;			/* Name of the font. Points to the
					 * hash table key. */
    int refCount;			/* Reference count for this structure.
					 * When refCount reaches zero, it
					 * means to free the resources
					 * associated with this structure. */
    Blt_HashEntry *hashPtr;		/* Pointer to this entry in global
					 * font hash table. Used to remove the
					 * entry from the table. */
    Blt_HashTable fontTable;		/* Hash table containing an Win32 font
					 * for each angle it's used at. Will
					 * always contain a 0 degree entry. */
    Tk_Font tkfont;			/* The zero degree Tk font.  We use it
					 * to get the Win32 font handle to
					 * generate non-zero degree rotated
					 * fonts. */
} WinFontset;

/*
 *---------------------------------------------------------------------------
 *
 * WinCreateRotatedFont --
 *
 *	Creates a rotated copy of the given font.  This only works for
 *	TrueType fonts.
 *
 * Results:
 *	Returns the newly create font or NULL if the font could not be
 *	created.
 *
 *---------------------------------------------------------------------------
 */
static HFONT
WinCreateRotatedFont(
    TkFont *fontPtr,			/* Font identifier (actually a
					 * Tk_Font) */
    long angle10)
{					/* Number of degrees to rotate font */
    TkFontAttributes *faPtr;		/* Set of attributes to match. */
    HFONT hfont;
    LOGFONTW lf;

    faPtr = &fontPtr->fa;
    ZeroMemory(&lf, sizeof(LOGFONT));
    lf.lfHeight = -faPtr->size;
    if (lf.lfHeight < 0) {
	HDC dc;

	dc = GetDC(NULL);
	lf.lfHeight = -MulDiv(faPtr->size, GetDeviceCaps(dc, LOGPIXELSY), 72);
	ReleaseDC(NULL, dc);
    }
    lf.lfWidth = 0;
    lf.lfEscapement = lf.lfOrientation = angle10;
#define TK_FW_NORMAL	0
    lf.lfWeight = (faPtr->weight == TK_FW_NORMAL) ? FW_NORMAL : FW_BOLD;
    lf.lfItalic = faPtr->slant;
    lf.lfUnderline = faPtr->underline;
    lf.lfStrikeOut = faPtr->overstrike;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfQuality = ANTIALIASED_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

    hfont = NULL;
    if (faPtr->family == NULL) {
	lf.lfFaceName[0] = '\0';
    } else {
#if (_TCL_VERSION >= _VERSION(8,1,0)) 
	Tcl_DString ds;
	Tcl_Encoding encoding;

	encoding = Tcl_GetEncoding(NULL, "unicode");
	Tcl_UtfToExternalDString(encoding, faPtr->family, -1, &ds);
	if (Blt_GetPlatformId() == VER_PLATFORM_WIN32_NT) {
	    Tcl_UniChar *src, *dst;
	    
	    /*
	     * We can only store up to LF_FACESIZE wide characters
	     */
	    if (Tcl_DStringLength(&ds) >= (LF_FACESIZE * sizeof(WCHAR))) {
		Tcl_DStringSetLength(&ds, LF_FACESIZE);
	    }
	    src = (Tcl_UniChar *)Tcl_DStringValue(&ds);
	    dst = (Tcl_UniChar *)lf.lfFaceName;
	    while (*src != '\0') {
		*dst++ = *src++;
	    }
	    *dst = '\0';
	    hfont = CreateFontIndirectW((LOGFONTW *)&lf);
	} else {
	    /*
	     * We can only store up to LF_FACESIZE characters
	     */
	    if (Tcl_DStringLength(&ds) >= LF_FACESIZE) {
		Tcl_DStringSetLength(&ds, LF_FACESIZE);
	    }
	    strcpy((char *)lf.lfFaceName, Tcl_DStringValue(&ds));
	    hfont = CreateFontIndirectA((LOGFONTA *)&lf);
	}
	Tcl_DStringFree(&ds);
#else
	strncpy((char *)lf.lfFaceName, faPtr->family, LF_FACESIZE - 1);
	lf.lfFaceName[LF_FACESIZE] = '\0';
#endif /* _TCL_VERSION >= 8.1.0 */
    }

    if (hfont == NULL) {
#if WINDEBUG
	PurifyPrintf("can't create font: %s\n", Blt_LastError());
#endif
    } else { 
	HFONT oldFont;
	TEXTMETRIC tm;
	HDC hdc;
	int result;

	/* Check if the rotated font is really a TrueType font. */

	hdc = GetDC(NULL);		/* Get the desktop device context */
	oldFont = SelectFont(hdc, hfont);
	result = ((GetTextMetrics(hdc, &tm)) && 
		  (tm.tmPitchAndFamily & TMPF_TRUETYPE));
	(void)SelectFont(hdc, oldFont);
	ReleaseDC(NULL, hdc);
	if (!result) {
#if WINDEBUG
	    PurifyPrintf("not a true type font\n");
#endif
	    DeleteFont(hfont);
	    return NULL;
	}
    }
    return hfont;
}

static void
WinDestroyFont(WinFontset *setPtr)
{
    Blt_HashEntry *hPtr;
    Blt_HashSearch cursor;
    
    for (hPtr = Blt_FirstHashEntry(&setPtr->fontTable, &cursor); 
	 hPtr != NULL; hPtr = Blt_NextHashEntry(&cursor)) {
	HFONT hfont;
	
	hfont = Blt_GetHashValue(hPtr);
	DeleteFont(hfont);
    }
    Tk_FreeFont(setPtr->tkfont);
    Blt_DeleteHashTable(&setPtr->fontTable);
    Blt_DeleteHashEntry(&fontTable, setPtr->hashPtr);
    Blt_Free(setPtr);
}

/* 
 *---------------------------------------------------------------------------
 *
 * WinGetFontFromObj --
 * 
 *	Opens a Tk font based on the description in the Tcl_Obj.  We first
 *	parse the description and if necessary rewrite it using the proper
 *	font aliases.  The font names
 *
 *	  "Sans Serif", "Serif", "Math", "Monospace"
 *
 *	correspond to the proper font regardless if the standard X fonts or
 *	XFT fonts are being used.
 *
 *	Leave XLFD font descriptions alone.  Let users describe exactly the
 *	font they wish.
 *
 *	Outside of reimplementing the Tk font mechanism, rewriting the
 *	font allows use to better handle programs that must work with
 *	X servers with and without the XRender extension.  It means 
 *	that the widget's default font settings do not have to use
 *	XLFD fonts even if XRender is available.
 *	
 *---------------------------------------------------------------------------
 */
static WinFontset *
WinGetFontFromObj(Tcl_Interp *interp, Tk_Window tkwin, Tcl_Obj *objPtr)
{
    Blt_HashEntry *hPtr;
    WinFontset *setPtr;
    const char *desc;
    int isNew;

    desc = Tcl_GetString(objPtr);
    while (isspace(UCHAR(*desc))) {
	desc++;				/* Skip leading blanks. */
    }
    /* Is the font already in the cache? */
    hPtr = Blt_CreateHashEntry(&fontTable, desc, &isNew);
    if (isNew) {
	Tk_Font tkFont;

	tkFont = TkGetFontFromObj(interp, tkwin, objPtr);
	if (tkFont == NULL) {
	    Blt_DeleteHashEntry(&fontTable, hPtr);
	    return NULL;
	}
	setPtr = Blt_AssertCalloc(1, sizeof(WinFontset));
	setPtr->refCount = 1;
	setPtr->tkfont = tkFont;
	setPtr->name = Blt_GetHashKey(&fontTable, hPtr);
	setPtr->hashPtr = hPtr;
	Blt_SetHashValue(hPtr, setPtr);
	Blt_InitHashTable(&setPtr->fontTable, BLT_ONE_WORD_KEYS);
    } else {
	setPtr = Tcl_GetHashValue(hPtr);
	setPtr->refCount++;
    }
    return setPtr;
}

static const char *
WinNameProc(_Blt_Font *fontPtr) 
{
    WinFontset *setPtr = fontPtr->clientData;

    return Tk_NameOfFont(setPtr->tkfont);
}

static const char *
WinFamilyProc(_Blt_Font *fontPtr) 
{
    WinFontset *setPtr = fontPtr->clientData;

    return ((TkFont *)setPtr->tkfont)->fa.family;
}

static Font
WinIdProc(_Blt_Font *fontPtr) 
{
    WinFontset *setPtr = fontPtr->clientData;

    return Tk_FontId(setPtr->tkfont);
}

static void
WinGetMetricsProc(_Blt_Font *fontPtr, Blt_FontMetrics *fmPtr)
{
    WinFontset *setPtr = fontPtr->clientData;
    TkFont *tkFontPtr = (TkFont *)setPtr->tkfont;
    Tk_FontMetrics fm;

    Tk_GetFontMetrics(setPtr->tkfont, &fm);
    fmPtr->ascent = fm.ascent;
    fmPtr->descent = fm.descent;
    fmPtr->linespace = fm.linespace;
    fmPtr->tabWidth = tkFontPtr->tabWidth;
    fmPtr->underlinePos = tkFontPtr->underlinePos;
    fmPtr->underlineHeight = tkFontPtr->underlineHeight;
}

static int
WinMeasureProc(_Blt_Font *fontPtr, const char *text, int numBytes,
		   int max, int flags, int *lengthPtr)
{
    WinFontset *setPtr = fontPtr->clientData;

    return Tk_MeasureChars(setPtr->tkfont, text, numBytes, max, flags, 
		lengthPtr);
}

static int
WinTextWidthProc(_Blt_Font *fontPtr, const char *text, int numBytes)
{
    WinFontset *setPtr = fontPtr->clientData;

    return Tk_TextWidth(setPtr->tkfont, text, numBytes);
}    


static void
WinDrawProc(
    Display *display,			/* Display on which to draw. */
    Drawable drawable,			/* Window or pixmap in which to
					 * draw. */
    GC gc,				/* Graphics context for drawing
					 * characters. */
    _Blt_Font *fontPtr,			/* Font in which characters will be
					 * drawn; must be the same as font
					 * used in GC. */
    int depth,				/* Not used. */
    float angle,			/* Not used. */
    const char *text,			/* UTF-8 string to be displayed.  Need
					 * not be '\0' terminated.  All Tk
					 * meta-characters (tabs, control
					 * characters, and newlines) should be
					 * stripped out of the string that is
					 * passed to this function.  If they
					 * are not stripped out, they will be
					 * displayed as regular printing
					 * characters. */
    int numBytes,				/* Number of bytes in string. */
    int x, int y)			/* Coordinates at which to place
					 * origin of string when drawing. */
{
    WinFontset *setPtr = fontPtr->clientData;

    if (angle != 0.0) {
	long angle10;
	Blt_HashEntry *hPtr;
    
	angle *= 10.0f;
	angle10 = ROUND(angle);
	hPtr = Blt_FindHashEntry(&setPtr->fontTable, (char *)angle10);
	if (hPtr == NULL) {
	    Blt_Warn("can't find font %s at %g rotated\n", setPtr->name, 
		angle);
           return;			/* Can't find instance at requested
					 * angle. */
	}
	display->request++;
	if (drawable != None) {
	    HDC hdc;
	    HFONT hfont;
	    TkWinDCState state;
	    
	    hfont = Blt_GetHashValue(hPtr);
	    hdc = TkWinGetDrawableDC(display, drawable, &state);
	    Blt_TextOut(hdc, gc, hfont, text, numBytes, x, y);
	    TkWinReleaseDrawableDC(drawable, hdc, &state);
	}
    } else {
	Tk_DrawChars(display, drawable, gc, setPtr->tkfont, text, numBytes, 
		     x, y);
    }
}


static int
WinPostscriptNameProc(_Blt_Font *fontPtr, Tcl_DString *resultPtr) 
{
    WinFontset *setPtr = fontPtr->clientData;

    return Tk_PostscriptFontName(setPtr->tkfont, resultPtr);
}

static int
WinCanRotateProc(_Blt_Font *fontPtr, float angle) 
{
    Blt_HashEntry *hPtr;
    HFONT hfont;
    WinFontset *setPtr = fontPtr->clientData;
    int isNew;
    long angle10;

    angle *= 10.0f;
    angle10 = ROUND(angle);
    if (angle == 0L) {
	return TRUE;
    }
    hPtr = Blt_CreateHashEntry(&setPtr->fontTable, (char *)angle10, &isNew);
    if (!isNew) {
	return TRUE;			/* Rotated font already exists. */
    }
    /* Create and add rotated font to this set of fonts. */
    hfont = WinCreateRotatedFont((TkFont *)Tk_FontId(setPtr->tkfont), angle10);
    if (hfont == NULL) {
	Blt_DeleteHashEntry(&setPtr->fontTable, hPtr);
	return FALSE;
    }
    Blt_SetHashValue(hPtr, hfont);
    return TRUE;
}

/* 
 * WinFreeProc --
 *
 *	Free the fontset. The fontset if destroyed if its reference count is
 *	zero.
 */
static void
WinFreeProc(_Blt_Font *fontPtr) 
{
    WinFontset *setPtr = fontPtr->clientData;

    setPtr->refCount--;
    if (setPtr->refCount <= 0) {
	WinDestroyFont(setPtr);
	fontPtr->clientData = NULL;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * WinUnderlineProc --
 *
 *	This procedure draws an underline for a given range of characters in a
 *	given string.  It doesn't draw the characters (which are assumed to
 *	have been displayed previously); it just draws the underline.  This
 *	procedure would mainly be used to quickly underline a few characters
 *	without having to construct an underlined font.  To produce properly
 *	underlined text, the appropriate underlined font should be constructed
 *	and used.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information gets displayed in "drawable".
 *
 *---------------------------------------------------------------------------
 */
static void
WinUnderlineProc(
    Display *display,			/* Display on which to draw. */
    Drawable drawable,			/* Window or pixmap in which to
					 * draw. */
    GC gc,				/* Graphics context for actually
					 * drawing line. */
    _Blt_Font *fontPtr,			/* Font used in GC; must have been
					 * allocated by Tk_GetFont().  Used
					 * for character dimensions, etc. */
    const char *string,			/* String containing characters to be
					 * underlined or overstruck. */
    int textLen,			/* Unused. */
    int x, int y,			/* Coordinates at which first
					 * character of string is drawn. */
    int first,				/* Byte offset of the first
					 * character. */
    int last,				/* Byte offset after the last
					 * character. */
    int xMax)
{
    WinFontset *setPtr = fontPtr->clientData;

    Tk_UnderlineChars(display, drawable, gc, setPtr->tkfont, string, x, y, 
	first, last);
}


/*
 *---------------------------------------------------------------------------
 *
 * Blt_GetFontFromObj -- 
 *
 *	Given a string description of a font, map the description to a
 *	corresponding Tk_Font that represents the font.
 *
 * Results:
 *	The return value is token for the font, or NULL if an error prevented
 *	the font from being created.  If NULL is returned, an error message
 *	will be left in the interp's result.
 *
 * Side effects:
 *	The font is added to an internal database with a reference count.  For
 *	each call to this procedure, there should eventually be a call to
 *	Tk_FreeFont() or Tk_FreeFontFromObj() so that the database is cleaned
 *	up when fonts aren't in use anymore.
 *
 *---------------------------------------------------------------------------
 */
Blt_Font
Blt_GetFontFromObj(
    Tcl_Interp *interp,			/* Interp for database and error
					 * return. */
    Tk_Window tkwin,			/* For display on which font will be
					 * used. */
    Tcl_Obj *objPtr)			/* String describing font, as: named
					 * font, native format, or parseable 
					 * string. */
{
    _Blt_Font *fontPtr; 
   
    fontPtr = Blt_AssertCalloc(1, sizeof(_Blt_Font));
    if (!font_initialized) {
	Blt_InitHashTable(&fontTable, BLT_STRING_KEYS);
	MakeAliasTable(tkwin);
	font_initialized++;
    }
    fontPtr->interp = interp;
    fontPtr->display = Tk_Display(tkwin);
    /* Try to get a windows rotated first.  If that fails, fall back to the
     * normal Tk font.  We rotate the font by drawing into a bitmap and
     * rotating the bitmap.  */
    fontPtr->clientData = WinGetFontFromObj(interp, tkwin, objPtr);
    if (fontPtr->clientData != NULL) {
	fontPtr->classPtr = &winFontClass;
    } else {
	fontPtr->clientData = TkGetFontFromObj(interp, tkwin, objPtr);
	if (fontPtr->clientData != NULL) {
	    fontPtr->classPtr = &tkFontClass;
	} else {
#if DEBUG_FONT_SELECTION
	    Blt_Warn("FAILED to find either Win or Tk font \"%s\"\n", 
		     Tcl_GetString(objPtr));
#endif
	    Blt_Free(fontPtr);
	    return NULL;		/* Failed to find either Win or Tk
					 * fonts. */
	}
    }
    return fontPtr;			/* Found Tk font. */
}


Blt_Font
Blt_AllocFontFromObj(
    Tcl_Interp *interp,			/* Interp for database and error
					 * return. */
    Tk_Window tkwin,			/* For screen on which font will be
					 * used. */
    Tcl_Obj *objPtr)			/* Object describing font, as: named
					 * font, native format, or parseable 
					 * string. */
{
    return Blt_GetFontFromObj(interp, tkwin, objPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_GetFont -- 
 *
 *	Given a string description of a font, map the description to a
 *	corresponding Tk_Font that represents the font.
 *
 * Results:
 *	The return value is token for the font, or NULL if an error prevented
 *	the font from being created.  If NULL is returned, an error message
 *	will be left in interp's result object.
 *
 * Side effects:
 * 	The font is added to an internal database with a reference count.  For
 * 	each call to this procedure, there should eventually be a call to
 * 	Blt_Font_Free so that the database is cleaned up when fonts aren't in
 * 	use anymore.
 *
 *---------------------------------------------------------------------------
 */

Blt_Font
Blt_GetFont(
    Tcl_Interp *interp,			/* Interp for database and error
					 * return. */
    Tk_Window tkwin,			/* For screen on which font will be
					 * used. */
    const char *string)			/* Object describing font, as: named
					 * font, native format, or parseable 
					 * string. */
{
    Blt_Font font;
    Tcl_Obj *objPtr;

    objPtr = Tcl_NewStringObj(string, strlen(string));
    Tcl_IncrRefCount(objPtr);
    font = Blt_GetFontFromObj(interp, tkwin, objPtr);
    Tcl_DecrRefCount(objPtr);
    return font;
}

Tcl_Interp *
Blt_Font_GetInterp(_Blt_Font *fontPtr) 
{
    return fontPtr->interp;
}

int
Blt_TextWidth(_Blt_Font *fontPtr, const char *string, int length)
{
    if (Blt_Afm_IsPrinting()) {
	int width;

	width = Blt_Afm_TextWidth(fontPtr, string, length);
	if (width >= 0) {
	    return width;
	}
    }
    return (*fontPtr->classPtr->textWidthProc)(fontPtr, string, length);
}

void
Blt_Font_GetMetrics(_Blt_Font *fontPtr, Blt_FontMetrics *fmPtr)
{
    if (Blt_Afm_IsPrinting()) {
	if (Blt_Afm_GetMetrics(fontPtr, fmPtr) == TCL_OK) {
	    return;
	}
    }
    (*fontPtr->classPtr->getMetricsProc)(fontPtr, fmPtr);
}

static int
GetFile(Tcl_Interp *interp, const char *fontName, Tcl_DString *namePtr, 
	Tcl_DString *valuePtr)
{
    HKEY hkey;
    char *name;
    char *value;
    const char *fileName;
    const char *fontSubKey;
    unsigned long maxBytesKey, maxBytesName, maxBytesValue;
    unsigned long numSubKeys, numValues;
    int result, length;
    int i;

    if (Blt_GetPlatformId() == VER_PLATFORM_WIN32_NT) {
	fontSubKey = "Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    } else {
	fontSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Fonts";
    }
    length = strlen(fontName);
    fileName = NULL;
    /* Open the registry key. */
    result = RegOpenKeyEx(
	HKEY_LOCAL_MACHINE,		/* Predefined key. */
	fontSubKey,			/* Registry subkey. */
	0,				/* Reserved. Must be zero. */
	KEY_READ,			/* Access rights. */
	&hkey);				/* (out) Handle to resulting key. */
    if (result != ERROR_SUCCESS) {
	Tcl_AppendResult(interp, "can't open font registry: ", 
		Blt_LastError(), (char *)NULL);
	return TCL_ERROR;
    }
    /* Get the number of values. */
    result = RegQueryInfoKey(
	hkey,				/* key handle */
	NULL,				/* Buffer for class name */
	NULL,				/* Size of class string. */
	NULL,				/* Reserved. Must be NULL. */ 
	&numSubKeys,			/* # of subkeys. */
	&maxBytesKey,			/* Longest subkey size. */
	NULL,				/* Longest class string. */
	&numValues,			/* # of values for this key. */
	&maxBytesName,			/* Longest value name  */
	&maxBytesValue,			/* Longest value data */
	NULL,				/* Security descriptor. */
	NULL);				/* Last write time. */
    if (result != ERROR_SUCCESS) {
        Tcl_AppendResult(interp, "can't query registry info: ", 
		Blt_LastError(), (char *)NULL);
	goto error;
    }
    Tcl_DStringSetLength(namePtr, maxBytesName);
    name = Tcl_DStringValue(namePtr);
    Tcl_DStringSetLength(valuePtr, maxBytesValue);
    value = Tcl_DStringValue(valuePtr);
    /* Look for the font value. */
    for (i = 0; i < numValues; i++) {
	unsigned long numBytesName, numBytesValue;
	unsigned long regType;

   	numBytesName = maxBytesName;	
   	numBytesValue = maxBytesValue;	
	result = RegEnumValue(
		hkey, 
		i, 
		name, 
		&numBytesName, 
		NULL, 			/* Reserved. Must be NULL. */
		&regType, 		/* (out) Registry value type  */
		(unsigned char *)value, 
		&numBytesValue);
	if (result != ERROR_SUCCESS) {
	    Tcl_AppendResult(interp, "can't retrieve registry value: ", 
		Blt_LastError(), (char *)NULL);
	    goto error;
	}
      	/* Perform anchored case-insensitive string comparison. */
	if (strncasecmp(name, fontName, length) == 0) {
	    fileName = value;
	    break;
	}
    }
    if (fileName == NULL) {
	Tcl_AppendResult(interp, "can't find font \"", fontName, "\": ",
		Blt_LastError(), (char *)NULL);
    }
 error:
    RegCloseKey(hkey);
    if (fileName == NULL) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

Tcl_Obj *
Blt_Font_GetFile(Tcl_Interp *interp, Tcl_Obj *objPtr, double *sizePtr)
{
    Tcl_DString nameStr, valueStr;
    Tcl_Obj *fileObjPtr;
    TkFontPattern *patternPtr;
    int result;

    if (!font_initialized) {
	Blt_InitHashTable(&fontTable, BLT_STRING_KEYS);
	MakeAliasTable(Tk_MainWindow(interp));
	font_initialized++;
    }
    patternPtr = TkGetPattern(interp, objPtr);
    if (patternPtr == NULL) {
	return NULL;
    }
    fileObjPtr = NULL;
    Tcl_DStringInit(&nameStr);
    Tcl_DStringInit(&valueStr);
    *sizePtr = patternPtr->size;

    result = GetFile(interp, patternPtr->family, &nameStr, &valueStr);
    TkFreeFontPattern(patternPtr);
    Tcl_DStringFree(&nameStr);

    if (result == TCL_OK) {
	const char *root;

	root = getenv("SYSTEMROOT");
	if (root == NULL) {
	    root = "c:/WINDOWS";
	}
	fileObjPtr = Tcl_NewStringObj(root, -1);
        Tcl_IncrRefCount(fileObjPtr);
	Tcl_AppendToObj(fileObjPtr, "/fonts/", -1);
	Tcl_AppendToObj(fileObjPtr, Tcl_DStringValue(&valueStr),
			Tcl_DStringLength(&valueStr));
    }
    Tcl_DStringFree(&valueStr);
    return fileObjPtr;
}
