
/*
 * bltTreeView.c --
 *
 * This module implements an hierarchy widget for the BLT toolkit.
 *
 *	Copyright 1998-2004 George A Howlett.
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

/*
 * TODO:
 *
 * BUGS:
 *   1.  "open" operation should change scroll offset so that as many
 *	 new entries (up to half a screen) can be seen.
 *   2.  "open" needs to adjust the scrolloffset so that the same entry
 *	 is seen at the same place.
 */

#define BUILD_BLT_TK_PROCS 1
#include "bltInt.h"

#ifndef NO_TREEVIEW

#ifdef HAVE_CTYPE_H
#  include <ctype.h>
#endif /* HAVE_CTYPE_H */

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#ifdef HAVE_STRING_H
#  include <string.h>
#endif /* HAVE_STRING_H */

#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif	/* HAVE_LIMITS_H */

#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "bltAlloc.h"
#include "bltList.h"
#include "bltSwitch.h"
#include "bltOp.h"
#include "bltInitCmd.h"
#include "bltTreeView.h"

#define BUTTON_PAD		2
#define BUTTON_IPAD		1
#define BUTTON_SIZE		7
#define COLUMN_PAD		2
#define FOCUS_WIDTH		1
#define ICON_PADX		2
#define ICON_PADY		1
#define INSET_PAD		0
#define LABEL_PADX		0
#define LABEL_PADY		0


#define FCLAMP(x)	((((x) < 0.0) ? 0.0 : ((x) > 1.0) ? 1.0 : (x)))
#define LineWidth(w)	(((w) > 1) ? (w) : 0)

#define CHOOSESTYLE(v,c,e) \
    (((e)->stylePtr != NULL) ? (e)->stylePtr : \
     (((c)->stylePtr != NULL) ? (c)->stylePtr : (v)->stylePtr))

#define GetData(entryPtr, key, objPtrPtr) \
	Blt_Tree_GetValueByKey((Tcl_Interp *)NULL, (entryPtr)->viewPtr->tree, \
	      (entryPtr)->node, key, objPtrPtr)

#define DEF_ICON_WIDTH		16
#define DEF_ICON_HEIGHT		16

#define RULE_AREA		(8)

#define TAG_UNKNOWN		(1<<0)
#define TAG_RESERVED		(1<<1)
#define TAG_USER_DEFINED	(1<<2)
#define TAG_SINGLE		(1<<3)
#define TAG_MULTIPLE		(1<<4)
#define TAG_ALL			(1<<5)


#define NodeToObj(n)		Tcl_NewLongObj(Blt_Tree_NodeId(n))

#define DEF_BUTTON_ACTIVE_BACKGROUND	RGB_WHITE
#define DEF_BUTTON_ACTIVE_BG_MONO	STD_ACTIVE_BG_MONO
#define DEF_BUTTON_ACTIVE_FOREGROUND	STD_ACTIVE_FOREGROUND
#define DEF_BUTTON_ACTIVE_FG_MONO	STD_ACTIVE_FG_MONO
#define DEF_BUTTON_BORDERWIDTH		"1"
#define DEF_BUTTON_CLOSE_RELIEF		"solid"
#define DEF_BUTTON_OPEN_RELIEF		"solid"
#define DEF_BUTTON_NORMAL_BACKGROUND	RGB_WHITE
#define DEF_BUTTON_NORMAL_BG_MONO	STD_NORMAL_BG_MONO
#define DEF_BUTTON_NORMAL_FOREGROUND	STD_NORMAL_FOREGROUND
#define DEF_BUTTON_NORMAL_FG_MONO	STD_NORMAL_FG_MONO
#define DEF_BUTTON_SIZE			"7"

#ifdef WIN32
#define DEF_COLUMN_ACTIVE_TITLE_BG	RGB_GREY85
#else
#define DEF_COLUMN_ACTIVE_TITLE_BG	RGB_GREY90
#endif
#define DEF_COLUMN_ACTIVE_TITLE_FG	STD_ACTIVE_FOREGROUND
#define DEF_COLUMN_ARROWWIDTH		"0"
#define DEF_COLUMN_BACKGROUND		(char *)NULL
#define DEF_COLUMN_BIND_TAGS		"all"
#define DEF_COLUMN_BORDERWIDTH		STD_BORDERWIDTH
#define DEF_COLUMN_COLOR		RGB_BLACK
#define DEF_COLUMN_EDIT			"yes"
#define DEF_COLUMN_FONT			STD_FONT
#define DEF_COLUMN_COMMAND		(char *)NULL
#define DEF_COLUMN_FORMATCOMMAND	(char *)NULL
#define DEF_COLUMN_HIDE			"no"
#define DEF_COLUMN_SHOW			"yes"
#define DEF_COLUMN_JUSTIFY		"center"
#define DEF_COLUMN_MAX			"0"
#define DEF_COLUMN_MIN			"0"
#define DEF_COLUMN_PAD			"2"
#define DEF_COLUMN_RELIEF		"flat"
#define DEF_COLUMN_STATE		"normal"
#define DEF_COLUMN_STYLE		"text"
#define DEF_COLUMN_TITLE_BACKGROUND	STD_NORMAL_BACKGROUND
#define DEF_COLUMN_TITLE_BORDERWIDTH	STD_BORDERWIDTH
#define DEF_COLUMN_TITLE_FONT		STD_FONT_SMALL
#define DEF_COLUMN_TITLE_FOREGROUND	STD_NORMAL_FOREGROUND
#define DEF_COLUMN_TITLE_RELIEF		"raised"
#define DEF_COLUMN_WEIGHT		"1.0"
#define DEF_COLUMN_WIDTH		"0"
#define DEF_COLUMN_RULE_DASHES		"dot"


#define DEF_SORT_COLUMN		(char *)NULL
#define DEF_SORT_COMMAND	(char *)NULL
#define DEF_SORT_DECREASING	"no"
#define DEF_SORT_TYPE		"dictionary"

/* RGB_LIGHTBLUE1 */

#define DEF_ACTIVE_FOREGROUND	RBG_BLACK
#define DEF_ACTIVE_RELIEF	"flat"
#define DEF_ALLOW_DUPLICATES	"yes"
#define DEF_BACKGROUND		RGB_WHITE
#define DEF_ALT_BACKGROUND	RGB_GREY97
#define DEF_BORDERWIDTH		STD_BORDERWIDTH
#define DEF_BUTTON		"auto"
#define DEF_COLUMNCOMMAND	((char *)NULL)
#define DEF_DASHES		"dot"
#define DEF_EXPORT_SELECTION	"no"
#define DEF_FOREGROUND		STD_NORMAL_FOREGROUND
#define DEF_FG_MONO		STD_NORMAL_FG_MONO
#define DEF_FLAT		"no"
#define DEF_FOCUS_DASHES	"dot"
#define DEF_FOCUS_EDIT		"no"
#define DEF_FOCUS_FOREGROUND	STD_ACTIVE_FOREGROUND
#define DEF_FOCUS_FG_MONO	STD_ACTIVE_FG_MONO
#define DEF_FONT		STD_FONT_SMALL
#define DEF_HEIGHT		"400"
#define DEF_HIDE_LEAVES		"no"
#define DEF_HIDE_ROOT		"yes"
#define DEF_FOCUS_HIGHLIGHT_BACKGROUND	STD_NORMAL_BACKGROUND
#define DEF_FOCUS_HIGHLIGHT_COLOR	RGB_BLACK
#define DEF_FOCUS_HIGHLIGHT_WIDTH	"2"
#define DEF_ICONVARIABLE	((char *)NULL)
#define DEF_ICONS "::blt::TreeView::closeIcon ::blt::TreeView::openIcon"
#define DEF_LINECOLOR		RGB_GREY30
#define DEF_LINECOLOR_MONO	STD_NORMAL_FG_MONO
#define DEF_LINESPACING		"0"
#define DEF_LINEWIDTH		"1"
#define DEF_MAKE_PATH		"no"
#define DEF_NEW_TAGS		"no"
#define DEF_NORMAL_BACKGROUND 	STD_NORMAL_BACKGROUND
#define DEF_NORMAL_FG_MONO	STD_ACTIVE_FG_MONO
#define DEF_RELIEF		"sunken"
#define DEF_RESIZE_CURSOR	"arrow"
#define DEF_SCROLL_INCREMENT	"20"
#define DEF_SCROLL_MODE		"hierbox"
#define DEF_SELECT_BACKGROUND 	STD_SELECT_BACKGROUND 
#define DEF_SELECT_BG_MONO  	STD_SELECT_BG_MONO
#define DEF_SELECT_BORDERWIDTH	"1"
#define DEF_SELECT_FOREGROUND 	STD_SELECT_FOREGROUND
#define DEF_SELECT_FG_MONO  	STD_SELECT_FG_MONO
#define DEF_SELECT_MODE		"single"
#define DEF_SELECT_RELIEF	"flat"
#define DEF_SHOW_ROOT		"yes"
#define DEF_SHOW_TITLES		"yes"
#define DEF_SORT_SELECTION	"no"
#define DEF_TAKE_FOCUS		"1"
#define DEF_TEXT_COLOR		STD_NORMAL_FOREGROUND
#define DEF_TEXT_MONO		STD_NORMAL_FG_MONO
#define DEF_TEXTVARIABLE	((char *)NULL)
#define DEF_TRIMLEFT		""
#define DEF_WIDTH		"200"

static const char *sortTypeStrings[] = {
    "dictionary", "ascii", "integer", "real", "command", NULL
};

typedef ClientData (TagProc)(TreeView *viewPtr, const char *string);
typedef int (TreeViewApplyProc)(TreeView *viewPtr, Entry *entryPtr);

static Blt_TreeApplyProc DeleteApplyProc;
static Blt_TreeApplyProc CreateApplyProc;
static CompareProc ExactCompare, GlobCompare, RegexpCompare;
static TreeViewApplyProc ShowEntryApplyProc, HideEntryApplyProc, 
	MapAncestorsApplyProc, FixSelectionsApplyProc;
static Tk_LostSelProc LostSelection;
static TreeViewApplyProc SelectEntryApplyProc;

static Blt_OptionParseProc ObjToIconProc;
static Blt_OptionPrintProc IconToObjProc;
static Blt_OptionFreeProc FreeIconProc;

static Blt_CustomOption iconOption = {
    ObjToIconProc, IconToObjProc, FreeIconProc, NULL,
};

static Blt_OptionParseProc ObjToTreeProc;
static Blt_OptionPrintProc TreeToObjProc;
static Blt_OptionFreeProc FreeTreeProc;
static Blt_CustomOption treeOption = {
    ObjToTreeProc, TreeToObjProc, FreeTreeProc, NULL,
};

static Blt_OptionParseProc ObjToIconsProc;
static Blt_OptionPrintProc IconsToObjProc;
static Blt_OptionFreeProc FreeIconsProc;
static Blt_CustomOption iconsOption = {
    ObjToIconsProc, IconsToObjProc, FreeIconsProc, NULL,
};

static Blt_OptionParseProc ObjToButton;
static Blt_OptionPrintProc ButtonToObj;
static Blt_CustomOption buttonOption = {
    ObjToButton, ButtonToObj, NULL, NULL,
};

static Blt_OptionParseProc ObjToUidProc;
static Blt_OptionPrintProc UidToObjProc;
static Blt_OptionFreeProc FreeUidProc;
static Blt_CustomOption uidOption = {
    ObjToUidProc, UidToObjProc, FreeUidProc, NULL,
};

static Blt_OptionParseProc ObjToScrollmode;
static Blt_OptionPrintProc ScrollmodeToObj;
static Blt_CustomOption scrollmodeOption = {
    ObjToScrollmode, ScrollmodeToObj, NULL, NULL,
};

static Blt_OptionParseProc ObjToSelectmode;
static Blt_OptionPrintProc SelectmodeToObj;
static Blt_CustomOption selectmodeOption = {
    ObjToSelectmode, SelectmodeToObj, NULL, NULL,
};

static Blt_OptionParseProc ObjToSeparator;
static Blt_OptionPrintProc SeparatorToObj;
static Blt_OptionFreeProc FreeSeparator;
static Blt_CustomOption separatorOption = {
    ObjToSeparator, SeparatorToObj, FreeSeparator, NULL,
};

static Blt_OptionParseProc ObjToLabel;
static Blt_OptionPrintProc LabelToObj;
static Blt_OptionFreeProc FreeLabel;
static Blt_CustomOption labelOption = {
    ObjToLabel, LabelToObj, FreeLabel, NULL,
};

static Blt_OptionParseProc ObjToStyles;
static Blt_OptionPrintProc StylesToObj;
static Blt_CustomOption stylesOption = {
    ObjToStyles, StylesToObj, NULL, NULL,
};

static Blt_OptionParseProc ObjToEnum;
static Blt_OptionPrintProc EnumToObj;
static Blt_CustomOption sortTypeOption =
{
    ObjToEnum, EnumToObj, NULL, (ClientData)sortTypeStrings
};

static Blt_OptionParseProc ObjToSortMarkProc;
static Blt_OptionPrintProc SortMarkToObjProc;
static Blt_CustomOption sortMarkOption =
{
    ObjToSortMarkProc, SortMarkToObjProc, (ClientData)0
};

static Blt_OptionParseProc ObjToSortColumnsProc;
static Blt_OptionPrintProc SortColumnsToObjProc;
static Blt_OptionFreeProc FreeSortColumnsProc;
static Blt_CustomOption sortColumnsOption = {
    ObjToSortColumnsProc, SortColumnsToObjProc, FreeSortColumnsProc, 
    (ClientData)0
};

static Blt_OptionParseProc ObjToData;
static Blt_OptionPrintProc DataToObj;
static Blt_CustomOption dataOption =
{
    ObjToData, DataToObj, NULL, (ClientData)0,
};

static Blt_OptionParseProc ObjToStyleProc;
static Blt_OptionPrintProc StyleToObjProc;
static Blt_OptionFreeProc FreeStyleProc;
static Blt_CustomOption styleOption =
{
    /* Contains a pointer to the widget that's currently being
     * configured.  This is used in the custom configuration parse
     * routine for icons.  */
    ObjToStyleProc, StyleToObjProc, FreeStyleProc, NULL,
};

static Blt_ConfigSpec buttonSpecs[] =
{
    {BLT_CONFIG_BACKGROUND, "-activebackground", "activeBackground", 
	"Background", DEF_BUTTON_ACTIVE_BACKGROUND, 
	Blt_Offset(TreeView, button.activeBg), 0},
    {BLT_CONFIG_SYNONYM, "-activebg", "activeBackground", (char *)NULL, 
	(char *)NULL, 0, 0},
    {BLT_CONFIG_SYNONYM, "-activefg", "activeForeground", (char *)NULL, 
	(char *)NULL, 0, 0},
    {BLT_CONFIG_COLOR, "-activeforeground", "activeForeground", "Foreground",
	DEF_BUTTON_ACTIVE_FOREGROUND, 
	Blt_Offset(TreeView, button.activeFgColor), 0},
    {BLT_CONFIG_BACKGROUND, "-background", "background", "Background",
	DEF_BUTTON_NORMAL_BACKGROUND, Blt_Offset(TreeView, button.bg), 0},
    {BLT_CONFIG_SYNONYM, "-bd", "borderWidth", (char *)NULL, (char *)NULL, 0,0},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_PIXELS_NNEG, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_BUTTON_BORDERWIDTH, Blt_Offset(TreeView, button.borderWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_RELIEF, "-closerelief", "closeRelief", "Relief",
	DEF_BUTTON_CLOSE_RELIEF, Blt_Offset(TreeView, button.closeRelief),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_SYNONYM, "-fg", "foreground", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_COLOR, "-foreground", "foreground", "Foreground",
	DEF_BUTTON_NORMAL_FOREGROUND, Blt_Offset(TreeView, button.fgColor), 0},
    {BLT_CONFIG_CUSTOM, "-images", "images", "Icons", (char *)NULL, 
	Blt_Offset(TreeView, button.icons), BLT_CONFIG_NULL_OK, &iconsOption},
    {BLT_CONFIG_RELIEF, "-openrelief", "openRelief", "Relief",
	DEF_BUTTON_OPEN_RELIEF, Blt_Offset(TreeView, button.openRelief),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-size", "size", "Size", DEF_BUTTON_SIZE, 
	Blt_Offset(TreeView, button.reqSize), 0},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};


static Blt_ConfigSpec entrySpecs[] =
{
    {BLT_CONFIG_CUSTOM, "-bindtags", (char *)NULL, (char *)NULL, (char *)NULL, 
	Blt_Offset(Entry, tagsUid), BLT_CONFIG_NULL_OK, &uidOption},
    {BLT_CONFIG_CUSTOM, "-button", (char *)NULL, (char *)NULL, DEF_BUTTON, 
	Blt_Offset(Entry, flags), BLT_CONFIG_DONT_SET_DEFAULT, &buttonOption},
    {BLT_CONFIG_OBJ, "-closecommand", (char *)NULL, (char *)NULL,
	(char *)NULL, Blt_Offset(Entry, closeCmdObjPtr), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_OBJ, "-command", (char *)NULL, (char *)NULL,
	(char *)NULL, Blt_Offset(Entry, cmdObjPtr), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_CUSTOM, "-data", (char *)NULL, (char *)NULL, (char *)NULL, 0, 
	BLT_CONFIG_NULL_OK, &dataOption},
    {BLT_CONFIG_SYNONYM, "-fg", "foreground", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_FONT, "-font", (char *)NULL, (char *)NULL, (char *)NULL, 
	Blt_Offset(Entry, font), 0},
    {BLT_CONFIG_COLOR, "-foreground", "foreground", (char *)NULL, (char *)NULL,
	 Blt_Offset(Entry, color), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_PIXELS_NNEG, "-height", (char *)NULL, (char *)NULL, 
	(char *)NULL, Blt_Offset(Entry, reqHeight), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-icons", (char *)NULL, (char *)NULL, (char *)NULL, 
	Blt_Offset(Entry, icons), BLT_CONFIG_NULL_OK, &iconsOption},
    {BLT_CONFIG_CUSTOM, "-label", (char *)NULL, (char *)NULL, (char *)NULL, 
	Blt_Offset(Entry, labelUid), 0, &labelOption},
    {BLT_CONFIG_OBJ, "-opencommand", (char *)NULL, (char *)NULL, 
	(char *)NULL, Blt_Offset(Entry, openCmdObjPtr), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_CUSTOM, "-styles", (char *)NULL, (char *)NULL, 
	(char *)NULL, Blt_Offset(Entry, values), BLT_CONFIG_NULL_OK, 
	&stylesOption},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

static Blt_ConfigSpec viewSpecs[] =
{
    {BLT_CONFIG_BITMASK, "-allowduplicates", "allowDuplicates", 
	"AllowDuplicates", DEF_ALLOW_DUPLICATES, Blt_Offset(TreeView, flags),
	BLT_CONFIG_DONT_SET_DEFAULT, (Blt_CustomOption *)ALLOW_DUPLICATES},
    {BLT_CONFIG_SYNONYM, "-altbg", "alternateBackground", (char *)NULL,
	(char *)NULL, 0, 0},
    {BLT_CONFIG_BACKGROUND, "-alternatebackground", "alternateBackground", 
	"AlternateBackground", DEF_ALT_BACKGROUND, Blt_Offset(TreeView, altBg), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BITMASK, "-autocreate", "autoCreate", "AutoCreate", 
	DEF_MAKE_PATH, Blt_Offset(TreeView, flags), 
	BLT_CONFIG_DONT_SET_DEFAULT, (Blt_CustomOption *)FILL_ANCESTORS},
    {BLT_CONFIG_BACKGROUND, "-background", "background", "Background",
	DEF_BACKGROUND, Blt_Offset(TreeView, bg), 0},
    {BLT_CONFIG_SYNONYM, "-bd", "borderWidth", (char *)NULL, (char *)NULL, 0,0},
    {BLT_CONFIG_SYNONYM, "-bg", "background", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_PIXELS_NNEG, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_BORDERWIDTH, Blt_Offset(TreeView, borderWidth), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-button", "button", "Button", DEF_BUTTON, 
	Blt_Offset(TreeView, buttonFlags), BLT_CONFIG_DONT_SET_DEFAULT, 
	&buttonOption},
    {BLT_CONFIG_OBJ, "-closecommand", "closeCommand", "CloseCommand",
	(char *)NULL, Blt_Offset(TreeView, closeCmdObjPtr), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_ACTIVE_CURSOR, "-cursor", "cursor", "Cursor", (char *)NULL, 
	Blt_Offset(TreeView, cursor), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_OBJ, "-columncommand", "columnCommand", "ColumnCommand", 
	DEF_COLUMNCOMMAND, Blt_Offset(TreeView, colCmdObjPtr),
	BLT_CONFIG_DONT_SET_DEFAULT | BLT_CONFIG_NULL_OK}, 
    {BLT_CONFIG_DASHES, "-dashes", "dashes", "Dashes", 	DEF_DASHES, 
	Blt_Offset(TreeView, dashes), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_OBJ, "-entrycommand", "entryCommand", "EntryCommand",
	(char *)NULL, Blt_Offset(TreeView, entryCmdObjPtr), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BITMASK, "-exportselection", "exportSelection",
	"ExportSelection", DEF_EXPORT_SELECTION, 
	Blt_Offset(TreeView, selection.flags), BLT_CONFIG_DONT_SET_DEFAULT, 
	(Blt_CustomOption *)SELECT_EXPORT},
    {BLT_CONFIG_SYNONYM, "-fg", "foreground", (char *)NULL, (char *)NULL, 0, 0},
    {BLT_CONFIG_BOOLEAN, "-flat", "flat", "Flat", DEF_FLAT, 
	Blt_Offset(TreeView, flatView), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_DASHES, "-focusdashes", "focusDashes", "FocusDashes",
	DEF_FOCUS_DASHES, Blt_Offset(TreeView, focusDashes), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-focusforeground", "focusForeground", "FocusForeground",
	DEF_FOCUS_FOREGROUND, Blt_Offset(TreeView, focusColor),
	BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_COLOR, "-focusforeground", "focusForeground", "FocusForeground",
	DEF_FOCUS_FG_MONO, Blt_Offset(TreeView, focusColor), 
	BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_FONT, "-font", "font", "Font", DEF_FONT, 
	Blt_Offset(TreeView, font), 0},
    {BLT_CONFIG_COLOR, "-foreground", "foreground", "Foreground", 
	DEF_TEXT_COLOR, Blt_Offset(TreeView, fgColor), BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_COLOR, "-foreground", "foreground", "Foreground", 
	DEF_TEXT_MONO, Blt_Offset(TreeView, fgColor), BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_PIXELS_NNEG, "-height", "height", "Height", DEF_HEIGHT, 
	Blt_Offset(TreeView, reqHeight), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BITMASK, "-hideleaves", "hideLeaves", "HideLeaves",
	DEF_HIDE_LEAVES, Blt_Offset(TreeView, flags), 
	BLT_CONFIG_DONT_SET_DEFAULT, (Blt_CustomOption *)HIDE_LEAVES},
    {BLT_CONFIG_BITMASK, "-hideroot", "hideRoot", "HideRoot", DEF_HIDE_ROOT,
	Blt_Offset(TreeView, flags), BLT_CONFIG_DONT_SET_DEFAULT, 
	(Blt_CustomOption *)HIDE_ROOT},
    {BLT_CONFIG_COLOR, "-highlightbackground", "highlightBackground",
	"HighlightBackground", DEF_FOCUS_HIGHLIGHT_BACKGROUND, 
        Blt_Offset(TreeView, highlightBgColor), 0},
    {BLT_CONFIG_COLOR, "-highlightcolor", "highlightColor", "HighlightColor",
	DEF_FOCUS_HIGHLIGHT_COLOR, Blt_Offset(TreeView, highlightColor), 0},
    {BLT_CONFIG_PIXELS_NNEG, "-highlightthickness", "highlightThickness",
	"HighlightThickness", DEF_FOCUS_HIGHLIGHT_WIDTH, 
	Blt_Offset(TreeView, highlightWidth), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_OBJ, "-iconvariable", "iconVariable", "IconVariable", 
	DEF_TEXTVARIABLE, Blt_Offset(TreeView, iconVarObjPtr), 
        BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_CUSTOM, "-icons", "icons", "Icons", DEF_ICONS, 
	Blt_Offset(TreeView, icons), BLT_CONFIG_NULL_OK, &iconsOption},
    {BLT_CONFIG_COLOR, "-linecolor", "lineColor", "LineColor",
	DEF_LINECOLOR, Blt_Offset(TreeView, lineColor), BLT_CONFIG_COLOR_ONLY},
    {BLT_CONFIG_COLOR, "-linecolor", "lineColor", "LineColor", 
	DEF_LINECOLOR_MONO, Blt_Offset(TreeView, lineColor), 
	BLT_CONFIG_MONO_ONLY},
    {BLT_CONFIG_PIXELS_NNEG, "-linespacing", "lineSpacing", "LineSpacing",
	DEF_LINESPACING, Blt_Offset(TreeView, leader), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-linewidth", "lineWidth", "LineWidth", 
	DEF_LINEWIDTH, Blt_Offset(TreeView, lineWidth), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_OBJ, "-opencommand", "openCommand", "OpenCommand",
	(char *)NULL, Blt_Offset(TreeView, openCmdObjPtr), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_RELIEF, "-relief", "relief", "Relief", DEF_RELIEF, 
	Blt_Offset(TreeView, relief), 0},
    {BLT_CONFIG_CURSOR, "-resizecursor", "resizeCursor", "ResizeCursor",
	DEF_RESIZE_CURSOR, Blt_Offset(TreeView, resizeCursor), 0},
    {BLT_CONFIG_CUSTOM, "-scrollmode", "scrollMode", "ScrollMode",
	DEF_SCROLL_MODE, Blt_Offset(TreeView, scrollMode),
	BLT_CONFIG_DONT_SET_DEFAULT, &scrollmodeOption},
    {BLT_CONFIG_BACKGROUND, "-selectbackground", "selectBackground", 
	"Foreground", DEF_SELECT_BACKGROUND, Blt_Offset(TreeView, selection.bg),
	0},
    {BLT_CONFIG_PIXELS_NNEG, "-selectborderwidth", "selectBorderWidth", 
	"BorderWidth", DEF_SELECT_BORDERWIDTH, 
	Blt_Offset(TreeView, selection.borderWidth), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_OBJ, "-selectcommand", "selectCommand", "SelectCommand",
	(char *)NULL, Blt_Offset(TreeView, selection.cmdObjPtr), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-selectforeground", "selectForeground", "Background",
	DEF_SELECT_FOREGROUND, Blt_Offset(TreeView, selection.fgColor), 0},
    {BLT_CONFIG_CUSTOM, "-selectmode", "selectMode", "SelectMode",
	DEF_SELECT_MODE, Blt_Offset(TreeView, selection.mode), 
	BLT_CONFIG_DONT_SET_DEFAULT, &selectmodeOption},
    {BLT_CONFIG_RELIEF, "-selectrelief", "selectRelief", "Relief",
	DEF_SELECT_RELIEF, Blt_Offset(TreeView, selection.relief),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-separator", "separator", "Separator", (char *)NULL, 
	Blt_Offset(TreeView, pathSep), BLT_CONFIG_NULL_OK, &separatorOption},
    {BLT_CONFIG_BITMASK, "-newtags", "newTags", "newTags", DEF_NEW_TAGS, 
	Blt_Offset(TreeView, flags), BLT_CONFIG_DONT_SET_DEFAULT, 
	(Blt_CustomOption *)TV_NEW_TAGS},
    {BLT_CONFIG_BITMASK, "-showtitles", "showTitles", "ShowTitles",
	DEF_SHOW_TITLES, Blt_Offset(TreeView, flags), 0,
        (Blt_CustomOption *)SHOW_COLUMN_TITLES},
    {BLT_CONFIG_BITMASK, "-sortselection", "sortSelection", "SortSelection",
	DEF_SORT_SELECTION, Blt_Offset(TreeView, selection.flags), 
        BLT_CONFIG_DONT_SET_DEFAULT, (Blt_CustomOption *)SELECT_SORTED},
    {BLT_CONFIG_STRING, "-takefocus", "takeFocus", "TakeFocus",
	DEF_TAKE_FOCUS, Blt_Offset(TreeView, takeFocus), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_OBJ, "-textvariable", "textVariable", "TextVariable", 
	DEF_TEXTVARIABLE, Blt_Offset(TreeView, textVarObjPtr), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STRING, "-tree", "tree", "Tree", (char *)NULL, 
	Blt_Offset(TreeView, treeName), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_STRING, "-trim", "trim", "Trim", DEF_TRIMLEFT, 
	Blt_Offset(TreeView, trimLeft), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_PIXELS_NNEG, "-width", "width", "Width", DEF_WIDTH, 
	Blt_Offset(TreeView, reqWidth), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_OBJ, "-xscrollcommand", "xScrollCommand", "ScrollCommand",
	(char *)NULL, Blt_Offset(TreeView, xScrollCmdObjPtr), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_PIXELS_NNEG, "-xscrollincrement", "xScrollIncrement", 
	"ScrollIncrement", DEF_SCROLL_INCREMENT, 
	Blt_Offset(TreeView, xScrollUnits), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_OBJ, "-yscrollcommand", "yScrollCommand", "ScrollCommand",
	(char *)NULL, Blt_Offset(TreeView, yScrollCmdObjPtr), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_PIXELS_NNEG, "-yscrollincrement", "yScrollIncrement", 
	"ScrollIncrement", DEF_SCROLL_INCREMENT, 
	Blt_Offset(TreeView, yScrollUnits), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};


static Blt_ConfigSpec columnSpecs[] =
{
    {BLT_CONFIG_BACKGROUND, "-activetitlebackground", "activeTitleBackground", 
	"Background", DEF_COLUMN_ACTIVE_TITLE_BG, 
	Blt_Offset(Column, activeTitleBg), 0},
    {BLT_CONFIG_COLOR, "-activetitleforeground", "activeTitleForeground", 
	"Foreground", DEF_COLUMN_ACTIVE_TITLE_FG, 
	Blt_Offset(Column, activeTitleFgColor), 0},
    {BLT_CONFIG_SYNONYM, "-bd", "borderWidth", (char *)NULL, (char *)NULL, 
	0, 0},
    {BLT_CONFIG_CUSTOM, "-bindtags", "bindTags", "BindTags",
	DEF_COLUMN_BIND_TAGS, Blt_Offset(Column, tagsUid),
	BLT_CONFIG_NULL_OK, &uidOption},
    {BLT_CONFIG_PIXELS_NNEG, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_COLUMN_BORDERWIDTH, Blt_Offset(Column, borderWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_OBJ, "-command", "command", "Command",
	DEF_COLUMN_COMMAND, Blt_Offset(Column, cmdObjPtr),
	BLT_CONFIG_DONT_SET_DEFAULT | BLT_CONFIG_NULL_OK}, 
    {BLT_CONFIG_BITMASK_INVERT, "-edit", "edit", "Edit", DEF_COLUMN_STATE, 
	Blt_Offset(Column, flags), BLT_CONFIG_DONT_SET_DEFAULT, 
	(Blt_CustomOption *)COLUMN_READONLY},
    {BLT_CONFIG_CUSTOM, "-decreasingicon", "decreasingIcon", "DecreasingIcon", 
	(char *)NULL, Blt_Offset(Column, sortDown), 
	BLT_CONFIG_NULL_OK | BLT_CONFIG_DONT_SET_DEFAULT, &iconOption},
    {BLT_CONFIG_OBJ, "-formatcommand", "formatCommand", "FormatCommand",
	(char *)NULL, Blt_Offset(Column, fmtCmdPtr), 
	BLT_CONFIG_NULL_OK}, 
    {BLT_CONFIG_BITMASK, "-hide", "hide", "Hide", DEF_COLUMN_HIDE, 
	Blt_Offset(Column, flags), BLT_CONFIG_DONT_SET_DEFAULT, 
	(Blt_CustomOption *)COLUMN_HIDDEN},
    {BLT_CONFIG_CUSTOM, "-icon", "icon", "icon", (char *)NULL, 
	Blt_Offset(Column, titleIcon),
        BLT_CONFIG_NULL_OK | BLT_CONFIG_DONT_SET_DEFAULT, &iconOption},
    {BLT_CONFIG_CUSTOM, "-increasingicon", "increasingIcon", "IncreasingIcon", 
	(char *)NULL, Blt_Offset(Column, sortUp), 
	BLT_CONFIG_NULL_OK | BLT_CONFIG_DONT_SET_DEFAULT, &iconOption},
    {BLT_CONFIG_JUSTIFY, "-justify", "justify", "Justify", DEF_COLUMN_JUSTIFY, 
	Blt_Offset(Column, justify), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-max", "max", "Max", DEF_COLUMN_MAX, 
	Blt_Offset(Column, reqMax), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-min", "min", "Min", DEF_COLUMN_MIN, 
	Blt_Offset(Column, reqMin), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PAD, "-pad", "pad", "Pad", DEF_COLUMN_PAD, 
	Blt_Offset(Column, pad), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_RELIEF, "-relief", "relief", "Relief", DEF_COLUMN_RELIEF, 
	Blt_Offset(Column, relief), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_DASHES, "-ruledashes", "ruleDashes", "RuleDashes",
	DEF_COLUMN_RULE_DASHES, Blt_Offset(Column, ruleDashes),
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BITMASK_INVERT, "-show", "show", "Show", DEF_COLUMN_SHOW, 
	Blt_Offset(Column, flags), BLT_CONFIG_DONT_SET_DEFAULT,
	(Blt_CustomOption *)COLUMN_HIDDEN},
    {BLT_CONFIG_OBJ, "-sortcommand", "sortCommand", "SortCommand",
	DEF_SORT_COMMAND, Blt_Offset(Column, sortCmdObjPtr), 
	BLT_CONFIG_NULL_OK}, 
    {BLT_CONFIG_CUSTOM, "-sorttype", "sortType", "SortType", DEF_SORT_TYPE, 
	Blt_Offset(Column, sortType), BLT_CONFIG_DONT_SET_DEFAULT, 
	&sortTypeOption},
    {BLT_CONFIG_STATE, "-state", "state", "State", DEF_COLUMN_STATE, 
	Blt_Offset(Column, state), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-style", "style", "Style", DEF_COLUMN_STYLE, 
	Blt_Offset(Column, stylePtr), BLT_CONFIG_NULL_OK, &styleOption},
    {BLT_CONFIG_STRING, "-text", "text", "Text",
	(char *)NULL, Blt_Offset(Column, text), 0},
    {BLT_CONFIG_STRING, "-title", "title", "Title", (char *)NULL, 
	Blt_Offset(Column, text), 0},
    {BLT_CONFIG_BACKGROUND, "-titlebackground", "titleBackground", 
	"TitleBackground", DEF_COLUMN_TITLE_BACKGROUND, 
	Blt_Offset(Column, titleBg), 0},
    {BLT_CONFIG_PIXELS_NNEG, "-titleborderwidth", "titleBorderWidth", 
	"TitleBorderWidth", DEF_COLUMN_TITLE_BORDERWIDTH, 
	Blt_Offset(Column, titleBW), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_FONT, "-titlefont", "titleFont", "Font",
	DEF_COLUMN_TITLE_FONT, Blt_Offset(Column, titleFont), 0},
    {BLT_CONFIG_COLOR, "-titleforeground", "titleForeground", "TitleForeground",
	DEF_COLUMN_TITLE_FOREGROUND, 
	Blt_Offset(Column, titleFgColor), 0},
    {BLT_CONFIG_JUSTIFY, "-titlejustify", "titleJustify", "TitleJustify", 
        DEF_COLUMN_JUSTIFY, Blt_Offset(Column, titleJustify), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_RELIEF, "-titlerelief", "titleRelief", "TitleRelief",
	DEF_COLUMN_TITLE_RELIEF, Blt_Offset(Column, titleRelief), 
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_DOUBLE, "-weight", (char *)NULL, (char *)NULL,
	DEF_COLUMN_WEIGHT, Blt_Offset(Column, weight), 
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-width", "width", "Width",
	DEF_COLUMN_WIDTH, Blt_Offset(Column, reqWidth), 
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	(char *)NULL, 0, 0}
};

static Blt_ConfigSpec sortSpecs[] =
{
    {BLT_CONFIG_CUSTOM, "-columns", "columns", "Columns",
	DEF_SORT_COLUMN, Blt_Offset(TreeView, sortInfo.order),
        BLT_CONFIG_NULL_OK | BLT_CONFIG_DONT_SET_DEFAULT, &sortColumnsOption},
    {BLT_CONFIG_OBJ, "-command", "command", "Command",
	DEF_SORT_COMMAND, Blt_Offset(TreeView, sortInfo.cmdObjPtr),
	BLT_CONFIG_DONT_SET_DEFAULT | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BOOLEAN, "-decreasing", "decreasing", "Decreasing",
	DEF_SORT_DECREASING, Blt_Offset(TreeView, sortInfo.decreasing),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-mark", "mark", "SortMark",
	DEF_SORT_COLUMN, Blt_Offset(TreeView, sortInfo.markPtr),
        BLT_CONFIG_NULL_OK | BLT_CONFIG_DONT_SET_DEFAULT, &sortMarkOption},
    {BLT_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	(char *)NULL, 0, 0}
};


typedef struct {
    int mask;
} ChildrenSwitches;

static Blt_SwitchSpec childrenSwitches[] = {
    {BLT_SWITCH_BITMASK, "-exposed", "", (char *)NULL,
	Blt_Offset(ChildrenSwitches, mask), 0, ENTRY_MASK},
    {BLT_SWITCH_END}
};


/* Forward Declarations */
static Blt_BindAppendTagsProc AppendTagsProc;
static Blt_BindPickProc PickItem;
static Blt_TreeApplyProc SortApplyProc;
static Blt_TreeCompareNodesProc CompareNodes;
static Blt_TreeNotifyEventProc TreeEventProc;
static Blt_TreeTraceProc TreeTraceProc;
static Tcl_CmdDeleteProc TreeViewInstCmdDeleteProc;
static Tcl_FreeProc DestroyTreeView;
static Tcl_FreeProc FreeColumn;
static Tcl_FreeProc FreeEntryProc;
static Tcl_IdleProc DisplayTreeView;
static Tcl_ObjCmdProc TreeViewInstCmdProc;
static Tcl_ObjCmdProc TreeViewCmdProc;
static Tk_EventProc TreeViewEventProc;
static Tk_ImageChangedProc IconChangedProc;
static Tk_SelectionProc SelectionProc;

static int ComputeVisibleEntries(TreeView *viewPtr);
static void ComputeLayout(TreeView *viewPtr);
static void DrawRule(TreeView *viewPtr, Column *colPtr, Drawable drawable);

/*
 *---------------------------------------------------------------------------
 *
 * EventuallyRedraw --
 *
 *	Queues a request to redraw the widget at the next idle point.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information gets redisplayed.  Right now we don't do selective
 *	redisplays:  the whole window will be redrawn.
 *
 *---------------------------------------------------------------------------
 */
static void
EventuallyRedraw(TreeView *viewPtr)
{
    if ((viewPtr->tkwin != NULL) && ((viewPtr->flags & REDRAW_PENDING) == 0) &&
	((viewPtr->flags & DONT_UPDATE) == 0)) {
	viewPtr->flags |= REDRAW_PENDING;
	Tcl_DoWhenIdle(DisplayTreeView, viewPtr);
    }
}

static Entry *
NodeToEntry(TreeView *viewPtr, Blt_TreeNode node)
{
    Blt_HashEntry *hPtr;

    hPtr = Blt_FindHashEntry(&viewPtr->entryTable, (char *)node);
    if (hPtr == NULL) {
	Blt_Warn("NodeToEntry: can't find node %s\n", 
		Blt_Tree_NodeLabel(node));
	abort();
	return NULL;
    }
    return Blt_GetHashValue(hPtr);
}


static Entry *
FindEntry(TreeView *viewPtr, Blt_TreeNode node)
{
    Blt_HashEntry *hPtr;

    hPtr = Blt_FindHashEntry(&viewPtr->entryTable, (char *)node);
    if (hPtr == NULL) {
	return NULL;
    }
    return Blt_GetHashValue(hPtr);
}

static Entry *
ParentEntry(Entry *entryPtr)
{
    TreeView *viewPtr = entryPtr->viewPtr; 
    Blt_TreeNode node;

    if (entryPtr->node == Blt_Tree_RootNode(viewPtr->tree)) {
	return NULL;
    }
    node = Blt_Tree_ParentNode(entryPtr->node);
    if (node == NULL) {
	return NULL;
    }
    return NodeToEntry(viewPtr, node);
}

static int
EntryIsHidden(Entry *entryPtr)
{
    TreeView *viewPtr = entryPtr->viewPtr; 

    if ((viewPtr->flags & HIDE_LEAVES) && (Blt_Tree_IsLeaf(entryPtr->node))) {
	return TRUE;
    }
    return (entryPtr->flags & ENTRY_HIDE) ? TRUE : FALSE;
}

#ifdef notdef
static int
EntryIsMapped(Entry *entryPtr)
{
    TreeView *viewPtr = entryPtr->viewPtr; 

    /* Don't check if the entry itself is open, only that its ancestors
     * are. */
    if (EntryIsHidden(entryPtr)) {
	return FALSE;
    }
    if (entryPtr == viewPtr->rootPtr) {
	return TRUE;
    }
    entryPtr = ParentEntry(entryPtr);
    while (entryPtr != viewPtr->rootPtr) {
	if (entryPtr->flags & (ENTRY_CLOSED | ENTRY_HIDE)) {
	    return FALSE;
	}
	entryPtr = ParentEntry(entryPtr);
    }
    return TRUE;
}
#endif

static int
EntryIsSelected(TreeView *viewPtr, Entry *entryPtr)
{
    Blt_HashEntry *hPtr;

    hPtr = Blt_FindHashEntry(&viewPtr->selection.table, (char *)entryPtr);
    return (hPtr != NULL);
}

static Entry *
FirstChild(Entry *entryPtr, unsigned int mask)
{
    Blt_TreeNode node;
    TreeView *viewPtr = entryPtr->viewPtr; 

    for (node = Blt_Tree_FirstChild(entryPtr->node); node != NULL; 
	 node = Blt_Tree_NextSibling(node)) {
	entryPtr = NodeToEntry(viewPtr, node);
	if (((mask & ENTRY_HIDE) == 0) || (!EntryIsHidden(entryPtr))) {
	    return entryPtr;
	}
    }
    return NULL;
}

static Entry *
LastChild(Entry *entryPtr, unsigned int mask)
{
    Blt_TreeNode node;
    TreeView *viewPtr = entryPtr->viewPtr; 

    for (node = Blt_Tree_LastChild(entryPtr->node); node != NULL; 
	 node = Blt_Tree_PrevSibling(node)) {
	entryPtr = NodeToEntry(viewPtr, node);
	if (((mask & ENTRY_HIDE) == 0) || (!EntryIsHidden(entryPtr))) {
	    return entryPtr;
	}
    }
    return NULL;
}

static Entry *
NextSibling(Entry *entryPtr, unsigned int mask)
{
    Blt_TreeNode node;
    TreeView *viewPtr = entryPtr->viewPtr; 

    for (node = Blt_Tree_NextSibling(entryPtr->node); node != NULL; 
	 node = Blt_Tree_NextSibling(node)) {
	entryPtr = NodeToEntry(viewPtr, node);
	if (((mask & ENTRY_HIDE) == 0) || (!EntryIsHidden(entryPtr))) {
	    return entryPtr;
	}
    }
    return NULL;
}

static Entry *
PrevSibling(Entry *entryPtr, unsigned int mask)
{
    Blt_TreeNode node;
    TreeView *viewPtr = entryPtr->viewPtr; 

    for (node = Blt_Tree_PrevSibling(entryPtr->node); node != NULL; 
	 node = Blt_Tree_PrevSibling(node)) {
	entryPtr = NodeToEntry(viewPtr, node);
	if (((mask & ENTRY_HIDE) == 0) ||
	    (!EntryIsHidden(entryPtr))) {
	    return entryPtr;
	}
    }
    return NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * PrevEntry --
 *
 *	Returns the "previous" node in the tree.  This node (in depth-first
 *	order) is its parent if the node has no siblings that are previous to
 *	it.  Otherwise it is the last descendant of the last sibling.  In this
 *	case, descend the sibling's hierarchy, using the last child at any
 *	ancestor, until we we find a leaf.
 *
 *---------------------------------------------------------------------------
 */
static Entry *
PrevEntry(Entry *entryPtr, unsigned int mask)
{
    TreeView *viewPtr = entryPtr->viewPtr; 
    Entry *prevPtr;

    if (entryPtr->node == Blt_Tree_RootNode(viewPtr->tree)) {
	return NULL;			/* The root is the first node. */
    }
    prevPtr = PrevSibling(entryPtr, mask);
    if (prevPtr == NULL) {
	/* There are no siblings previous to this one, so pick the parent. */
	prevPtr = ParentEntry(entryPtr);
    } else {
	/*
	 * Traverse down the right-most thread in order to select the last
	 * entry.  Stop if we find a "closed" entry or reach a leaf.
	 */
	entryPtr = prevPtr;
	while ((entryPtr->flags & mask) == 0) {
	    entryPtr = LastChild(entryPtr, mask);
	    if (entryPtr == NULL) {
		break;			/* Found a leaf. */
	    }
	    prevPtr = entryPtr;
	}
    }
    if (prevPtr == NULL) {
	return NULL;
    }
    return prevPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * NextEntry --
 *
 *	Returns the "next" node in relation to the given node.  The next node
 *	(in depth-first order) is either the first child of the given node the
 *	next sibling if the node has no children (the node is a leaf).  If the
 *	given node is the last sibling, then try it's parent next sibling.
 *	Continue until we either find a next sibling for some ancestor or we
 *	reach the root node.  In this case the current node is the last node
 *	in the tree.
 *
 *---------------------------------------------------------------------------
 */
static Entry *
NextEntry(Entry *entryPtr, unsigned int mask)
{
    TreeView *viewPtr = entryPtr->viewPtr; 
    Entry *nextPtr;
    int ignoreLeaf;

    ignoreLeaf = ((viewPtr->flags & HIDE_LEAVES) && 
		  (Blt_Tree_IsLeaf(entryPtr->node)));

    if ((!ignoreLeaf) && ((entryPtr->flags & mask) == 0)) {
	nextPtr = FirstChild(entryPtr, mask); 
	if (nextPtr != NULL) {
	    return nextPtr;		/* Pick the first sub-node. */
	}
    }
    /* 
     * Back up until to a level where we can pick a "next sibling".  For the
     * last entry we'll thread our way back to the root.
     */
    while (entryPtr != viewPtr->rootPtr) {
	nextPtr = NextSibling(entryPtr, mask);
	if (nextPtr != NULL) {
	    return nextPtr;
	}
	entryPtr = ParentEntry(entryPtr);
    }
    return NULL;			/* At root, no next node. */
}

static const char *
GetEntryPath(TreeView *viewPtr, Entry *entryPtr, int checkEntryLabel, 
	    Tcl_DString *resultPtr)
{
    const char **names;		       /* Used the stack the component names. */
    const char *staticSpace[64+2];
    int level;
    int i;

    level = Blt_Tree_NodeDepth(entryPtr->node);
    if (viewPtr->rootPtr->labelUid == NULL) {
	level--;
    }
    if (level > 64) {
	names = Blt_AssertMalloc((level + 2) * sizeof(char *));
    } else {
	names = staticSpace;
    }
    for (i = level; i >= 0; i--) {
	Blt_TreeNode node;

	/* Save the name of each ancestor in the name array. */
	if (checkEntryLabel) {
	    names[i] = GETLABEL(entryPtr);
	} else {
	    names[i] = Blt_Tree_NodeLabel(entryPtr->node);
	}
	node = Blt_Tree_ParentNode(entryPtr->node);
	if (node != NULL) {
	    entryPtr = NodeToEntry(viewPtr, node);
	}
    }
    Tcl_DStringInit(resultPtr);
    if (level >= 0) {
	if ((viewPtr->pathSep == SEPARATOR_LIST) || 
	    (viewPtr->pathSep == SEPARATOR_NONE)) {
	    for (i = 0; i <= level; i++) {
		Tcl_DStringAppendElement(resultPtr, names[i]);
	    }
	} else {
	    Tcl_DStringAppend(resultPtr, names[0], -1);
	    for (i = 1; i <= level; i++) {
		Tcl_DStringAppend(resultPtr, viewPtr->pathSep, -1);
		Tcl_DStringAppend(resultPtr, names[i], -1);
	    }
	}
    } else {
	if ((viewPtr->pathSep != SEPARATOR_LIST) &&
	    (viewPtr->pathSep != SEPARATOR_NONE)) {
	    Tcl_DStringAppend(resultPtr, viewPtr->pathSep, -1);
	}
    }
    if (names != staticSpace) {
	Blt_Free(names);
    }
    return Tcl_DStringValue(resultPtr);
}

/*
 * Preprocess the command string for percent substitution.
 */
static Tcl_Obj *
PercentSubst(TreeView *viewPtr, Entry *entryPtr, Tcl_Obj *cmdObjPtr)
{
    const char *last, *p;
    const char *fullName;
    Tcl_DString ds;
    const char *string;
    Tcl_Obj *objPtr;

    objPtr = Tcl_NewStringObj("", 0);
    /*
     * Get the full path name of the node, in case we need to substitute for
     * it.
     */
    Tcl_DStringInit(&ds);
    fullName = GetEntryPath(viewPtr, entryPtr, TRUE, &ds);
    /* Append the widget name and the node .t 0 */
    string = Tcl_GetString(cmdObjPtr);
    for (last = p = string; *p != '\0'; p++) {
	if (*p == '%') {
	    const char *string;
	    char buf[3];

	    if (p > last) {
		Tcl_AppendToObj(objPtr, last, p - last);
	    }
	    switch (*(p + 1)) {
	    case '%':		/* Percent sign */
		string = "%";
		break;
	    case 'W':		/* Widget name */
		string = Tk_PathName(viewPtr->tkwin);
		break;
	    case 'P':		/* Full pathname */
		string = fullName;
		break;
	    case 'p':		/* Name of the node */
		string = GETLABEL(entryPtr);
		break;
	    case '#':		/* Node identifier */
		string = Blt_Tree_NodeIdAscii(entryPtr->node);
		break;
	    default:
		if (*(p + 1) == '\0') {
		    p--;
		}
		buf[0] = *p, buf[1] = *(p + 1), buf[2] = '\0';
		string = buf;
		break;
	    }
	    {
		int needQuotes = FALSE;
		const char *q;

		for (q = string; *q != '\0'; q++) {
		    if (*q == ' ') {
			needQuotes = TRUE;
			break;
		    }
		}
		if (needQuotes) {
		    Tcl_AppendToObj(objPtr, "{", 1);
		    Tcl_AppendToObj(objPtr, string, -1);
		    Tcl_AppendToObj(objPtr, "}", 1);
		} else {
		    Tcl_AppendToObj(objPtr, string, -1);
		}
	    }
	    p++;
	    last = p + 1;
	}
    }
    if (p > last) {
	Tcl_AppendToObj(objPtr, last, p-last);
    }
    Tcl_DStringFree(&ds);
    return objPtr;
}

static int
OpenEntry(TreeView *viewPtr, Entry *entryPtr)
{
    Tcl_Obj *cmdObjPtr;

    if ((entryPtr->flags & ENTRY_CLOSED) == 0) {
	return TCL_OK;			/* Entry is already open. */
    }
    entryPtr->flags &= ~ENTRY_CLOSED;
    viewPtr->flags |= LAYOUT_PENDING;

    /*
     * If there's a "open" command proc specified for the entry, use that
     * instead of the more general "open" proc for the entire treeview.
     * Be careful because the "open" command may perform an update.
     */
    cmdObjPtr = CHOOSE(viewPtr->openCmdObjPtr, entryPtr->openCmdObjPtr);
    if (cmdObjPtr != NULL) {
	int result;

	cmdObjPtr = PercentSubst(viewPtr, entryPtr, cmdObjPtr);
	Tcl_IncrRefCount(cmdObjPtr);
	Tcl_Preserve(entryPtr);
	result = Tcl_EvalObjEx(viewPtr->interp, cmdObjPtr, TCL_EVAL_GLOBAL);
	Tcl_Release(entryPtr);
	Tcl_DecrRefCount(cmdObjPtr);
	if (result != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

static int
CloseEntry(TreeView *viewPtr, Entry *entryPtr)
{
    Tcl_Obj *cmdObjPtr;

    if (entryPtr->flags & ENTRY_CLOSED) {
	return TCL_OK;			/* Entry is already closed. */
    }
    entryPtr->flags |= ENTRY_CLOSED;
    viewPtr->flags |= LAYOUT_PENDING;

    /*
     * Invoke the entry's "close" command, if there is one. Otherwise try the
     * treeview's global "close" command.
     */
    cmdObjPtr = CHOOSE(viewPtr->closeCmdObjPtr, entryPtr->closeCmdObjPtr);
    if (cmdObjPtr != NULL) {
	int result;

	cmdObjPtr = PercentSubst(viewPtr, entryPtr, cmdObjPtr);
	Tcl_IncrRefCount(cmdObjPtr);
	Tcl_Preserve(entryPtr);
	result = Tcl_EvalObjEx(viewPtr->interp, cmdObjPtr, TCL_EVAL_GLOBAL);
	Tcl_Release(entryPtr);
	Tcl_DecrRefCount(cmdObjPtr);
	if (result != TCL_OK) {
	    viewPtr->flags |= DIRTY;
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}


static void
AddValue(Entry *entryPtr, Column *colPtr)
{
    if (Blt_TreeView_FindValue(entryPtr, colPtr) == NULL) {
	Tcl_Obj *objPtr;

	if (GetData(entryPtr, colPtr->key, &objPtr) == TCL_OK) {
	    Value *valuePtr;

	    /* Add a new value only if a data entry exists. */
	    valuePtr = Blt_Pool_AllocItem(entryPtr->viewPtr->valuePool, 
			 sizeof(Value));
	    valuePtr->columnPtr = colPtr;
	    valuePtr->nextPtr = entryPtr->values;
	    valuePtr->textPtr = NULL;
	    valuePtr->width = valuePtr->height = 0;
	    valuePtr->stylePtr = NULL;
	    valuePtr->fmtString = NULL;
	    entryPtr->values = valuePtr;
	}
    }
    entryPtr->viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
    entryPtr->flags |= ENTRY_DIRTY;
}

/*
 *---------------------------------------------------------------------------
 *
 * SelectCmdProc --
 *
 *      Invoked at the next idle point whenever the current selection changes.
 *      Executes some application-specific code in the -selectcommand option.
 *      This provides a way for applications to handle selection changes.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      TCL code gets executed for some application-specific task.
 *
 *---------------------------------------------------------------------------
 */
static void
SelectCmdProc(ClientData clientData) 
{
    TreeView *viewPtr = clientData;

    viewPtr->selection.flags &= ~SELECT_PENDING;
    Tcl_Preserve(viewPtr);
    if (viewPtr->selection.cmdObjPtr != NULL) {
	if (Tcl_EvalObjEx(viewPtr->interp, viewPtr->selection.cmdObjPtr, 
		TCL_EVAL_GLOBAL) != TCL_OK) {
	    Tcl_BackgroundError(viewPtr->interp);
	}
    }
    Tcl_Release(viewPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * EventuallyInvokeSelectCmd --
 *
 *      Queues a request to execute the -selectcommand code associated with
 *      the widget at the next idle point.  Invoked whenever the selection
 *      changes.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      TCL code gets executed for some application-specific task.
 *
 *---------------------------------------------------------------------------
 */
static void
EventuallyInvokeSelectCmd(TreeView *viewPtr)
{
    if (!(viewPtr->selection.flags & SELECT_PENDING)) {
	viewPtr->selection.flags |= SELECT_PENDING;
	Tcl_DoWhenIdle(SelectCmdProc, viewPtr);
    }
}

static void
ClearSelection(TreeView *viewPtr)
{
    Blt_DeleteHashTable(&viewPtr->selection.table);
    Blt_InitHashTable(&viewPtr->selection.table, BLT_ONE_WORD_KEYS);
    Blt_Chain_Reset(viewPtr->selection.list);
    EventuallyRedraw(viewPtr);
    if (viewPtr->selection.cmdObjPtr != NULL) {
	EventuallyInvokeSelectCmd(viewPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * GetEntryIcon --
 *
 * 	Selects the correct image for the entry's icon depending upon the
 * 	current state of the entry: active/inactive normal/selected.
 *
 *		active - normal
 *		active - selected
 *		inactive - normal
 *		inactive - selected
 *
 * Results:
 *	Returns the image for the icon.
 *
 *---------------------------------------------------------------------------
 */
static Icon
GetEntryIcon(TreeView *viewPtr, Entry *entryPtr)
{
    Icon *icons;
    Icon icon;

    icons = CHOOSE(viewPtr->icons, entryPtr->icons);
    icon = NULL;
    if (icons != NULL) {		/* Selected or normal icon? */
	icon = icons[0];
	if (((entryPtr->flags & ENTRY_CLOSED) == 0) && (icons[1] != NULL)) {
	    icon = icons[1];
	}
    }
    return icon;
}

static void
SelectEntry(TreeView *viewPtr, Entry *entryPtr)
{
    int isNew;
    Blt_HashEntry *hPtr;
    Icon icon;
    const char *label;
    Selection *selectPtr = &viewPtr->selection;

    hPtr = Blt_CreateHashEntry(&selectPtr->table, (char *)entryPtr, &isNew);
    if (isNew) {
	Blt_ChainLink link;

	link = Blt_Chain_Append(selectPtr->list, entryPtr);
	Blt_SetHashValue(hPtr, link);
    }
    label = GETLABEL(entryPtr);
    if ((viewPtr->textVarObjPtr != NULL) && (label != NULL)) {
	Tcl_Obj *objPtr;
	
	objPtr = Tcl_NewStringObj(label, -1);
	if (Tcl_ObjSetVar2(viewPtr->interp, viewPtr->textVarObjPtr, NULL, 
		objPtr, TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG) == NULL) {
	    return;
	}
    }
    icon = GetEntryIcon(viewPtr, entryPtr);
    if ((viewPtr->iconVarObjPtr != NULL) && (icon != NULL)) {
	Tcl_Obj *objPtr;
	
	objPtr = Tcl_NewStringObj(TreeView_IconName(icon), -1);
	if (Tcl_ObjSetVar2(viewPtr->interp, viewPtr->iconVarObjPtr, NULL, 
		objPtr, TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG) == NULL) {
	    return;
	}
    }
}

static void
DeselectEntry(TreeView *viewPtr, Entry *entryPtr)
{
    Blt_HashEntry *hPtr;

    hPtr = Blt_FindHashEntry(&viewPtr->selection.table, (char *)entryPtr);
    if (hPtr != NULL) {
	Blt_ChainLink link;

	link = Blt_GetHashValue(hPtr);
	Blt_Chain_DeleteLink(viewPtr->selection.list, link);
	Blt_DeleteHashEntry(&viewPtr->selection.table, hPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * LostSelection --
 *
 *	This procedure is called back by Tk when the selection is grabbed
 *	away.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The existing selection is unhighlighted, and the window is
 *	marked as not containing a selection.
 *
 *---------------------------------------------------------------------------
 */
static void
LostSelection(ClientData clientData)
{
    TreeView *viewPtr = clientData;

    if ((viewPtr->selection.flags & SELECT_EXPORT) == 0) {
	return;
    }
    ClearSelection(viewPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * SelectRange --
 *
 *	Sets the selection flag for a range of nodes.  The range is
 *	determined by two pointers which designate the first/last
 *	nodes of the range.
 *
 * Results:
 *	Always returns TCL_OK.
 *
 *---------------------------------------------------------------------------
 */
static int
SelectRange(TreeView *viewPtr, Entry *fromPtr, Entry *toPtr)
{
    if (viewPtr->flatView) {
	int i;

	if (fromPtr->flatIndex > toPtr->flatIndex) {
	    for (i = fromPtr->flatIndex; i >= toPtr->flatIndex; i--) {
		SelectEntryApplyProc(viewPtr, viewPtr->flatArr[i]);
	    }
	} else {
	    for (i = fromPtr->flatIndex; i <= toPtr->flatIndex; i++) {
		SelectEntryApplyProc(viewPtr, viewPtr->flatArr[i]);
	    }
	}
    } else {
	Entry *entryPtr, *nextPtr;
	IterProc *proc;
	/* From the range determine the direction to select entries. */

	proc = (Blt_Tree_IsBefore(toPtr->node, fromPtr->node)) 
	    ? PrevEntry : NextEntry;
	for (entryPtr = fromPtr; entryPtr != NULL; entryPtr = nextPtr) {
	    nextPtr = (*proc)(entryPtr, ENTRY_MASK);
	    SelectEntryApplyProc(viewPtr, entryPtr);
	    if (entryPtr == toPtr) {
		break;
	    }
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * GetCurrentColumn --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Column *
GetCurrentColumn(TreeView *viewPtr)
{
    ClientData hint;
    Column *colPtr;

    colPtr = NULL;
    hint = Blt_GetCurrentHint(viewPtr->bindTable);
    if ((hint == ITEM_COLUMN_TITLE) || (hint == ITEM_COLUMN_RULE)) {
	colPtr = Blt_GetCurrentItem(viewPtr->bindTable);
	if ((colPtr == NULL) || (colPtr->flags & DELETED)) {
	    return NULL;
	}
    } else if (hint >= ITEM_STYLE) {
	Value *valuePtr = hint;
	
	colPtr = valuePtr->columnPtr;
    }
    return colPtr;
}

static Column *
NearestColumn(TreeView *viewPtr, int x, int y, ClientData *contextPtr)
{
    if (viewPtr->numVisible > 0) {
	Blt_ChainLink link;

	/*
	 * Determine if the pointer is over the rightmost portion of the
	 * column.  This activates the rule.
	 */
	x = WORLDX(viewPtr, x);
	for(link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL;
	    link = Blt_Chain_NextLink(link)) {
	    Column *colPtr;
	    int right;

	    colPtr = Blt_Chain_GetValue(link);
	    right = colPtr->worldX + colPtr->width;
	    if ((x >= colPtr->worldX) && (x <= right)) {
		if (contextPtr != NULL) {
		    *contextPtr = NULL;
		    if ((viewPtr->flags & SHOW_COLUMN_TITLES) && 
			(y >= viewPtr->inset) &&
			(y < (viewPtr->titleHeight + viewPtr->inset))) {
			*contextPtr = (x >= (right - RULE_AREA)) 
			    ? ITEM_COLUMN_RULE : ITEM_COLUMN_TITLE;
		    } 
		}
		return colPtr;
	    }
	}
    }
    return NULL;
}

static int
GetColumn(Tcl_Interp *interp, TreeView *viewPtr, Tcl_Obj *objPtr, 
	  Column **colPtrPtr)
{
    const char *string;
    char c;
    int index;

    string = Tcl_GetString(objPtr);
    c = string[0];
    if ((c == 't') && (strcmp(string, "treeView") == 0)) {
	*colPtrPtr = &viewPtr->treeColumn;
    } else if ((c == 'c') && (strcmp(string, "current") == 0)){ 
	*colPtrPtr = GetCurrentColumn(viewPtr);
    } else if ((c == 'a') && (strcmp(string, "active") == 0)){ 
	*colPtrPtr = viewPtr->colActivePtr;
    } else if ((isdigit(c)) && 
	       (Tcl_GetIntFromObj(NULL, objPtr, &index) == TCL_OK)) {
	Blt_ChainLink link;

	if (index >= Blt_Chain_GetLength(viewPtr->columns)) {
	    Tcl_AppendResult(interp, "bad column index \"", string, "\"",
			     (char *)NULL);
	    return TCL_ERROR;
	}
	link = Blt_Chain_GetNthLink(viewPtr->columns, index);
	*colPtrPtr = Blt_Chain_GetValue(link);
    } else {
	Blt_HashEntry *hPtr;
    
	hPtr = Blt_FindHashEntry(&viewPtr->columnTable, 
		Blt_Tree_GetKey(viewPtr->tree, string));
	if (hPtr == NULL) {
	    if (interp != NULL) {
		Tcl_AppendResult(interp, "can't find column \"", string, 
			"\" in \"", Tk_PathName(viewPtr->tkwin), "\"", 
			(char *)NULL);
	    }
	    return TCL_ERROR;
	} 
	*colPtrPtr = Blt_GetHashValue(hPtr);
    }
    return TCL_OK;
}

static void
TraceColumn(TreeView *viewPtr, Column *colPtr)
{
    Blt_Tree_CreateTrace(viewPtr->tree, NULL /* Node */, colPtr->key, NULL,
	TREE_TRACE_FOREIGN_ONLY | TREE_TRACE_WRITES | TREE_TRACE_UNSETS, 
	TreeTraceProc, viewPtr);
}


static void
TraceColumns(TreeView *viewPtr)
{
    Blt_ChainLink link;

    for(link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Column *colPtr;

	colPtr = Blt_Chain_GetValue(link);
	Blt_Tree_CreateTrace(
		viewPtr->tree, 
		NULL		/* Node */, 
		colPtr->key	/* Key pattern */, 
		NULL		/* Tag */,
	        TREE_TRACE_FOREIGN_ONLY|TREE_TRACE_WRITES|TREE_TRACE_UNSETS, 
	        TreeTraceProc	/* Callback routine */, 
		viewPtr		/* Client data */);
    }
}


/*
 *---------------------------------------------------------------------------
 *
 * GetUid --
 *
 *	Gets or creates a unique string identifier.  Strings are reference
 *	counted.  The string is placed into a hashed table local to the
 *	treeview.
 *
 * Results:
 *	Returns the pointer to the hashed string.
 *
 *---------------------------------------------------------------------------
 */
static UID
GetUid(TreeView *viewPtr, const char *string)
{
    Blt_HashEntry *hPtr;
    int isNew;
    size_t refCount;

    hPtr = Blt_CreateHashEntry(&viewPtr->uidTable, string, &isNew);
    if (isNew) {
	refCount = 1;
    } else {
	refCount = (size_t)Blt_GetHashValue(hPtr);
	refCount++;
    }
    Blt_SetHashValue(hPtr, refCount);
    return Blt_GetHashKey(&viewPtr->uidTable, hPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * FreeUid --
 *
 *	Releases the uid.  Uids are reference counted, so only when the
 *	reference count is zero (i.e. no one else is using the string) is the
 *	entry removed from the hash table.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static void
FreeUid(TreeView *viewPtr, UID uid)
{
    Blt_HashEntry *hPtr;
    size_t refCount;

    hPtr = Blt_FindHashEntry(&viewPtr->uidTable, uid);
    assert(hPtr != NULL);
    refCount = (size_t)Blt_GetHashValue(hPtr);
    refCount--;
    if (refCount > 0) {
	Blt_SetHashValue(hPtr, refCount);
    } else {
	Blt_DeleteHashEntry(&viewPtr->uidTable, hPtr);
    }
}

/*ARGSUSED*/
static void
FreeTreeProc(ClientData clientData, Display *display, char *widgRec, int offset)
{
    Blt_Tree *treePtr = (Blt_Tree *)(widgRec + offset);

    if (*treePtr != NULL) {
	Blt_TreeNode root;
	TreeView *viewPtr = clientData;

	/* 
	 * Release the current tree, removing any entry fields. 
	 */
	root = Blt_Tree_RootNode(*treePtr);
	Blt_Tree_Apply(root, DeleteApplyProc, viewPtr);
	ClearSelection(viewPtr);
	Blt_Tree_Close(*treePtr);
	*treePtr = NULL;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToTreeProc --
 *
 *	Convert the string representing the name of a tree object 
 *	into a tree token.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left
 *	in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToTreeProc(
    ClientData clientData,		  /* Not used. */
    Tcl_Interp *interp,		          /* Interpreter to send results back
					   * to */
    Tk_Window tkwin,			  /* Not used. */
    Tcl_Obj *objPtr,		          /* Tcl_Obj representing the new
					   * value. */  
    char *widgRec,
    int offset,				  /* Offset to field in structure */
    int flags)	
{
    Blt_Tree tree = *(Blt_Tree *)(widgRec + offset);

    if (Blt_Tree_Attach(interp, tree, Tcl_GetString(objPtr)) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TreeToObj --
 *
 * Results:
 *	The string representation of the button boolean is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
TreeToObjProc(
    ClientData clientData,		  /* Not used. */
    Tcl_Interp *interp,		
    Tk_Window tkwin,			  /* Not used. */
    char *widgRec,
    int offset,				  /* Offset to field in structure */
    int flags)	
{
    Blt_Tree tree = *(Blt_Tree *)(widgRec + offset);

    if (tree == NULL) {
	return Tcl_NewStringObj("", -1);
    } else {
	return Tcl_NewStringObj(Blt_Tree_Name(tree), -1);
    }
}


/*
 *---------------------------------------------------------------------------
 *
 * ObjToScrollmode --
 *
 *	Convert the string reprsenting a scroll mode, to its numeric
 *	form.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left
 *	in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToScrollmode(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* New legend position string */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    char *string;
    char c;
    int *modePtr = (int *)(widgRec + offset);

    string = Tcl_GetString(objPtr);
    c = string[0];
    if ((c == 'l') && (strcmp(string, "listbox") == 0)) {
	*modePtr = BLT_SCROLL_MODE_LISTBOX;
    } else if ((c == 't') && (strcmp(string, "treeview") == 0)) {
	*modePtr = BLT_SCROLL_MODE_HIERBOX;
    } else if ((c == 'c') && (strcmp(string, "canvas") == 0)) {
	*modePtr = BLT_SCROLL_MODE_CANVAS;
    } else {
	Tcl_AppendResult(interp, "bad scroll mode \"", string,
	    "\": should be \"treeview\", \"listbox\", or \"canvas\"", 
		(char *)NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ScrollmodeToObj --
 *
 * Results:
 *	The string representation of the button boolean is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
ScrollmodeToObj(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    int mode = *(int *)(widgRec + offset);

    switch (mode) {
    case BLT_SCROLL_MODE_LISTBOX:
	return Tcl_NewStringObj("listbox", -1);
    case BLT_SCROLL_MODE_HIERBOX:
	return Tcl_NewStringObj("hierbox", -1);
    case BLT_SCROLL_MODE_CANVAS:
	return Tcl_NewStringObj("canvas", -1);
    default:
	return Tcl_NewStringObj("unknown scroll mode", -1);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToSelectmode --
 *
 *	Convert the string reprsenting a scroll mode, to its numeric
 *	form.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left
 *	in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToSelectmode(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* Tcl_Obj representing the new
					 * value. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    char *string;
    char c;
    int *modePtr = (int *)(widgRec + offset);

    string = Tcl_GetString(objPtr);
    c = string[0];
    if ((c == 's') && (strcmp(string, "single") == 0)) {
	*modePtr = SELECT_MODE_SINGLE;
    } else if ((c == 'm') && (strcmp(string, "multiple") == 0)) {
	*modePtr = SELECT_MODE_MULTIPLE;
    } else if ((c == 'a') && (strcmp(string, "active") == 0)) {
	*modePtr = SELECT_MODE_SINGLE;
    } else {
	Tcl_AppendResult(interp, "bad select mode \"", string,
	    "\": should be \"single\" or \"multiple\"", (char *)NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SelectmodeToObj --
 *
 * Results:
 *	The string representation of the button boolean is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
SelectmodeToObj(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    int mode = *(int *)(widgRec + offset);

    switch (mode) {
    case SELECT_MODE_SINGLE:
	return Tcl_NewStringObj("single", -1);
    case SELECT_MODE_MULTIPLE:
	return Tcl_NewStringObj("multiple", -1);
    default:
	return Tcl_NewStringObj("unknown scroll mode", -1);
    }
}


/*
 *---------------------------------------------------------------------------
 *
 * ObjToButton --
 *
 *	Convert a string to one of three values.
 *		0 - false, no, off
 *		1 - true, yes, on
 *		2 - auto
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left in
 *	interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToButton(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* Tcl_Obj representing the new
					 * value. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    char *string;
    int *flagsPtr = (int *)(widgRec + offset);

    string = Tcl_GetString(objPtr);
    if ((string[0] == 'a') && (strcmp(string, "auto") == 0)) {
	*flagsPtr &= ~BUTTON_MASK;
	*flagsPtr |= BUTTON_AUTO;
    } else {
	int bool;

	if (Tcl_GetBooleanFromObj(interp, objPtr, &bool) != TCL_OK) {
	    return TCL_ERROR;
	}
	*flagsPtr &= ~BUTTON_MASK;
	if (bool) {
	    *flagsPtr |= BUTTON_SHOW;
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonToObj --
 *
 * Results:
 *	The string representation of the button boolean is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
ButtonToObj(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    int bool;
    unsigned int buttonFlags = *(int *)(widgRec + offset);

    bool = (buttonFlags & BUTTON_MASK);
    if (bool == BUTTON_AUTO) {
	return Tcl_NewStringObj("auto", 4);
    } else {
	return Tcl_NewBooleanObj(bool);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToScrollmode --
 *
 *	Convert the string reprsenting a scroll mode, to its numeric
 *	form.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left
 *	in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToSeparator(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* Tcl_Obj representing the new
					 * value. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    char **sepPtr = (char **)(widgRec + offset);
    char *string;

    string = Tcl_GetString(objPtr);
    if (string[0] == '\0') {
	*sepPtr = SEPARATOR_LIST;
    } else if (strcmp(string, "none") == 0) {
	*sepPtr = SEPARATOR_NONE;
    } else {
	*sepPtr = Blt_AssertStrdup(string);
    } 
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SeparatorToObj --
 *
 * Results:
 *	The string representation of the separator is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
SeparatorToObj(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    char *separator = *(char **)(widgRec + offset);

    if (separator == SEPARATOR_NONE) {
	return Tcl_NewStringObj("", -1);
    } else if (separator == SEPARATOR_LIST) {
	return Tcl_NewStringObj("list", -1);
    }  else {
	return Tcl_NewStringObj(separator, -1);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * FreeSeparator --
 *
 *	Free the UID from the widget record, setting it to NULL.
 *
 * Results:
 *	The UID in the widget record is set to NULL.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
FreeSeparator(ClientData clientData, Display *display, char *widgRec, 
	      int offset)
{
    char **sepPtr = (char **)(widgRec + offset);

    if ((*sepPtr != SEPARATOR_LIST) && (*sepPtr != SEPARATOR_NONE)) {
	Blt_Free(*sepPtr);
	*sepPtr = SEPARATOR_NONE;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToLabel --
 *
 *	Convert the string representing the label. 
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left
 *	in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToLabel(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* Tcl_Obj representing the new
					 * value. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    UID *labelPtr = (UID *)(widgRec + offset);
    char *string;

    string = Tcl_GetString(objPtr);
    if (string[0] != '\0') {
	TreeView *viewPtr = clientData;

	*labelPtr = GetUid(viewPtr, string);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TreeToObj --
 *
 * Results:
 *	The string of the entry's label is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
LabelToObj(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,		
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    UID labelUid = *(UID *)(widgRec + offset);
    const char *string;

    if (labelUid == NULL) {
	Entry *entryPtr  = (Entry *)widgRec;

	string = Blt_Tree_NodeLabel(entryPtr->node);
    } else {
	string = labelUid;
    }
    return Tcl_NewStringObj(string, -1);
}

/*ARGSUSED*/
static void
FreeLabel(ClientData clientData, Display *display, char *widgRec, int offset)
{
    UID *labelPtr = (UID *)(widgRec + offset);

    if (*labelPtr != NULL) {
	TreeView *viewPtr = clientData;

	FreeUid(viewPtr, *labelPtr);
	*labelPtr = NULL;
    }
}

static ColumnStyle *
FindStyle(Tcl_Interp *interp, TreeView *viewPtr, const char *styleName)
{
    Blt_HashEntry *hPtr;

    hPtr = Blt_FindHashEntry(&viewPtr->styleTable, styleName);
    if (hPtr == NULL) {
	if (interp != NULL) {
	    Tcl_AppendResult(interp, "can't find cell style \"", styleName, 
		"\"", (char *)NULL);
	}
	return NULL;
    }
    return Blt_GetHashValue(hPtr);
}

static int
GetStyle(Tcl_Interp *interp, TreeView *viewPtr, const char *name, 
	 ColumnStyle **stylePtrPtr)
{
    ColumnStyle *stylePtr;

    stylePtr = FindStyle(interp, viewPtr, name);
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    stylePtr->refCount++;
    *stylePtrPtr = stylePtr;
    return TCL_OK;
}

#ifdef notdef
int
GetStyleFromObj(Tcl_Interp *interp, TreeView *viewPtr, 
			     Tcl_Obj *objPtr, ColumnStyle **stylePtrPtr)
{
    return GetStyle(interp, viewPtr, Tcl_GetString(objPtr), 
				 stylePtrPtr);
}
#endif

static INLINE Blt_Bg
GetStyleBackground(Column *colPtr)
{ 
    ColumnStyle *stylePtr;
    Blt_Bg bg;

    bg = NULL;
    stylePtr = colPtr->stylePtr;
    if (stylePtr != NULL) {
	bg = (stylePtr->flags & STYLE_HIGHLIGHT) ? 
	    stylePtr->highlightBg : stylePtr->bg;
    }
    if (bg == NULL) {
	bg = colPtr->viewPtr->bg;
    }
    return bg;
}

static INLINE Blt_Font
GetStyleFont(Column *colPtr)
{
    ColumnStyle *stylePtr;

    stylePtr = colPtr->stylePtr;
    if ((stylePtr != NULL) && (stylePtr->font != NULL)) {
	return stylePtr->font;
    }
    return colPtr->viewPtr->font;
}

static INLINE XColor *
GetStyleForeground(Column *colPtr)
{
    ColumnStyle *stylePtr;

    stylePtr = colPtr->stylePtr;
    if ((stylePtr != NULL) && (stylePtr->fgColor != NULL)) {
	return stylePtr->fgColor;
    }
    return colPtr->viewPtr->fgColor;
}

static void
FreeStyle(ColumnStyle *stylePtr)
{
    stylePtr->refCount--;
    /* Remove the style from the hash table so that it's name can be used.*/
    /* If no cell is using the style, remove it.*/
    if (stylePtr->refCount <= 0) {
	TreeView *viewPtr;

	viewPtr = stylePtr->viewPtr;
	iconOption.clientData = viewPtr;
	Blt_FreeOptions(stylePtr->classPtr->specsPtr, (char *)stylePtr, 
		viewPtr->display, 0);
	(*stylePtr->classPtr->freeProc)(stylePtr); 
	if (stylePtr->hashPtr != NULL) {
	    Blt_DeleteHashEntry(&viewPtr->styleTable, stylePtr->hashPtr);
	} 
	if (stylePtr->link != NULL) {
	    /* Only user-generated styles will be in the list. */
	    Blt_Chain_DeleteLink(viewPtr->userStyles, stylePtr->link);
	}
	Blt_Free(stylePtr);
    } 
}

static ColumnStyle *
CreateStyle(Tcl_Interp *interp,
     TreeView *viewPtr,			/* Blt_TreeView_ widget. */
     int type,				/* Type of style: either
					 * STYLE_TEXTBOX,
					 * STYLE_COMBOBOX, or
					 * STYLE_CHECKBOX */
    const char *styleName,		/* Name of the new style. */
    int objc,
    Tcl_Obj *const *objv)
{    
    Blt_HashEntry *hPtr;
    int isNew;
    ColumnStyle *stylePtr;
    
    hPtr = Blt_CreateHashEntry(&viewPtr->styleTable, styleName, &isNew);
    if (!isNew) {
	if (interp != NULL) {
	    Tcl_AppendResult(interp, "cell style \"", styleName, 
			     "\" already exists", (char *)NULL);
	}
	return NULL;
    }
    /* Create the new marker based upon the given type */
    switch (type) {
    case STYLE_TEXTBOX:
	stylePtr = Blt_TreeView_CreateTextBoxStyle(viewPtr, hPtr);
	break;
    case STYLE_COMBOBOX:
	stylePtr = Blt_TreeView_CreateComboBoxStyle(viewPtr, hPtr);
	break;
    case STYLE_CHECKBOX:
	stylePtr = Blt_TreeView_CreateCheckBoxStyle(viewPtr, hPtr);
	break;
    default:
	return NULL;
    }
    iconOption.clientData = viewPtr;
    if (Blt_ConfigureComponentFromObj(interp, viewPtr->tkwin, styleName, 
	stylePtr->classPtr->className, stylePtr->classPtr->specsPtr, 
	objc, objv, (char *)stylePtr, 0) != TCL_OK) {
	FreeStyle(stylePtr);
	return NULL;
    }
    return stylePtr;
}


/*
 *---------------------------------------------------------------------------
 *
 * ObjToStyles --
 *
 *	Convert the list representing the field-name style-name pairs into
 *	stylePtr's.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left in
 *	interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToStyles(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* Tcl_Obj representing the new
					 * value. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    Entry *entryPtr = (Entry *)widgRec;
    TreeView *viewPtr;
    Tcl_Obj **objv;
    int objc;
    int i;

    if (Tcl_ListObjGetElements(interp, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    if (objc & 1) {
	Tcl_AppendResult(interp, "odd number of field/style pairs in \"",
			 Tcl_GetString(objPtr), "\"", (char *)NULL);
	return TCL_ERROR;
    }
    viewPtr = entryPtr->viewPtr;
    for (i = 0; i < objc; i += 2) {
	Value *valuePtr;
	ColumnStyle *stylePtr;
	Column *colPtr;
	const char *string;
	
	if (GetColumn(interp, viewPtr, objv[i], &colPtr)!=TCL_OK) {
	    return TCL_ERROR;
	}
	valuePtr = Blt_TreeView_FindValue(entryPtr, colPtr);
	if (valuePtr == NULL) {
	    return TCL_ERROR;
	}
	string = Tcl_GetString(objv[i+1]);
	stylePtr = NULL;
	if ((*string != '\0') && (GetStyle(interp, viewPtr, string,
		&stylePtr) != TCL_OK)) {
	    return TCL_ERROR;			/* No data ??? */
	}
	if (valuePtr->stylePtr != NULL) {
	    FreeStyle(valuePtr->stylePtr);
	}
	valuePtr->stylePtr = stylePtr;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StylesToObj --
 *
 * Results:
 *	The string representation of the button boolean is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
StylesToObj(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,		
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    Entry *entryPtr = (Entry *)widgRec;
    Value *vp;
    Tcl_Obj *listObjPtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for (vp = entryPtr->values; vp != NULL; vp = vp->nextPtr) {
	const char *styleName;

	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewStringObj(vp->columnPtr->key, -1));
	styleName = (vp->stylePtr != NULL) ? vp->stylePtr->name : "";
	Tcl_ListObjAppendElement(interp, listObjPtr, 
				 Tcl_NewStringObj(styleName, -1));
    }
    return listObjPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * FreeUidProc --
 *
 *	Free the UID from the widget record, setting it to NULL.
 *
 * Results:
 *	The UID in the widget record is set to NULL.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
FreeUidProc(ClientData clientData, Display *display, char *widgRec, int offset)
{
    UID *uidPtr = (UID *)(widgRec + offset);

    if (*uidPtr != NULL) {
	TreeView *viewPtr = clientData;

	FreeUid(viewPtr, *uidPtr);
	*uidPtr = NULL;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToUidProc --
 *
 *	Converts the string to a Uid. Uid's are hashed, reference counted
 *	strings.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToUidProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* Tcl_Obj representing the new
					 * value. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    TreeView *viewPtr = clientData;
    UID *uidPtr = (UID *)(widgRec + offset);

    *uidPtr = GetUid(viewPtr, Tcl_GetString(objPtr));
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * UidToObjProc --
 *
 *	Returns the uid as a string.
 *
 * Results:
 *	The fill style string is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
UidToObjProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    UID uid = *(UID *)(widgRec + offset);

    if (uid == NULL) {
	return Tcl_NewStringObj("", -1);
    } else {
	return Tcl_NewStringObj(uid, -1);
    }
}


/*
 *---------------------------------------------------------------------------
 *
 * IconChangedProc
 *
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
/* ARGSUSED */
static void
IconChangedProc(
    ClientData clientData,
    int x,				/* Not used. */
    int y,				/* Not used. */
    int width,				/* Not used. */
    int height,				/* Not used. */
    int imageWidth,			/* Not used. */
    int imageHeight)			/* Not used. */
{
    TreeView *viewPtr = clientData;

    viewPtr->flags |= (DIRTY | LAYOUT_PENDING | SCROLL_PENDING);
    EventuallyRedraw(viewPtr);
}

Icon
Blt_TreeView_GetIcon(TreeView *viewPtr, const char *iconName)
{
    Blt_HashEntry *hPtr;
    int isNew;
    struct _Icon *iconPtr;

    hPtr = Blt_CreateHashEntry(&viewPtr->iconTable, iconName, &isNew);
    if (isNew) {
	Tk_Image tkImage;
	int width, height;

	tkImage = Tk_GetImage(viewPtr->interp, viewPtr->tkwin, (char *)iconName, 
		IconChangedProc, viewPtr);
	if (tkImage == NULL) {
	    Blt_DeleteHashEntry(&viewPtr->iconTable, hPtr);
	    return NULL;
	}
	Tk_SizeOfImage(tkImage, &width, &height);
	iconPtr = Blt_AssertMalloc(sizeof(struct _Icon));
	iconPtr->tkImage = tkImage;
	iconPtr->hashPtr = hPtr;
	iconPtr->refCount = 1;
	iconPtr->width = width;
	iconPtr->height = height;
	Blt_SetHashValue(hPtr, iconPtr);
    } else {
	iconPtr = Blt_GetHashValue(hPtr);
	iconPtr->refCount++;
    }
    return iconPtr;
}

void
Blt_TreeView_FreeIcon(TreeView *viewPtr, Icon icon)
{
    struct _Icon *iconPtr = icon;

    iconPtr->refCount--;
    if (iconPtr->refCount == 0) {
	Blt_DeleteHashEntry(&viewPtr->iconTable, iconPtr->hashPtr);
	Tk_FreeImage(iconPtr->tkImage);
	Blt_Free(iconPtr);
    }
}

static void
DumpIconTable(TreeView *viewPtr)
{
    Blt_HashEntry *hPtr;
    Blt_HashSearch iter;
    struct _Icon *iconPtr;

    for (hPtr = Blt_FirstHashEntry(&viewPtr->iconTable, &iter);
	 hPtr != NULL; hPtr = Blt_NextHashEntry(&iter)) {
	iconPtr = Blt_GetHashValue(hPtr);
	Tk_FreeImage(iconPtr->tkImage);
	Blt_Free(iconPtr);
    }
    Blt_DeleteHashTable(&viewPtr->iconTable);
}

/*ARGSUSED*/
static void
FreeIconsProc(ClientData clientData, Display *display, char *widgRec, 
	      int offset)
{
    Icon **iconsPtr = (Icon **)(widgRec + offset);

    if (*iconsPtr != NULL) {
	Icon *ip;
	TreeView *viewPtr = clientData;

	for (ip = *iconsPtr; *ip != NULL; ip++) {
	    Blt_TreeView_FreeIcon(viewPtr, *ip);
	}
	Blt_Free(*iconsPtr);
	*iconsPtr = NULL;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToIconsProc --
 *
 *	Convert a list of image names into Tk images.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left in
 *	interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToIconsProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* Tcl_Obj representing the new
					 * value. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    Tcl_Obj **objv;
    TreeView *viewPtr = clientData;
    Icon **iconPtrPtr = (Icon **)(widgRec + offset);
    Icon *icons;
    int objc;
    int result;

    result = TCL_OK;
    icons = NULL;
    if (Tcl_ListObjGetElements(interp, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    if (objc > 0) {
	int i;
	
	icons = Blt_AssertMalloc(sizeof(Icon *) * (objc + 1));
	for (i = 0; i < objc; i++) {
	    icons[i] = Blt_TreeView_GetIcon(viewPtr, Tcl_GetString(objv[i]));
	    if (icons[i] == NULL) {
		result = TCL_ERROR;
		break;
	    }
	}
	icons[i] = NULL;
    }
    *iconPtrPtr = icons;
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * IconsToObjProc --
 *
 *	Converts the icon into its string representation (its name).
 *
 * Results:
 *	The name of the icon is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
IconsToObjProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    Icon *icons = *(Icon **)(widgRec + offset);
    Tcl_Obj *listObjPtr;
    
    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    if (icons != NULL) {
	Icon *iconPtr;

	for (iconPtr = icons; *iconPtr != NULL; iconPtr++) {
	    Tcl_Obj *objPtr;

	    objPtr = Tcl_NewStringObj(Blt_Image_Name((*iconPtr)->tkImage), -1);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);

	}
    }
    return listObjPtr;
}

/*ARGSUSED*/
static void
FreeIconProc(
    ClientData clientData,
    Display *display,		/* Not used. */
    char *widgRec,
    int offset)
{
    Icon *iconPtr = (Icon *)(widgRec + offset);
    TreeView *viewPtr = clientData;

    if (*iconPtr != NULL) {
	Blt_TreeView_FreeIcon(viewPtr, *iconPtr);
	*iconPtr = NULL;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToIconProc --
 *
 *	Convert the name of an icon into a Tk image.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left in
 *	interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToIconProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to report results */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* Tcl_Obj representing the value. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    TreeView *viewPtr = clientData;
    Icon *iconPtr = (Icon *)(widgRec + offset);
    Icon icon;
    int length;
    const char *string;

    string = Tcl_GetStringFromObj(objPtr, &length);
    icon = NULL;
    if (length > 0) {
	icon = Blt_TreeView_GetIcon(viewPtr, string);
	if (icon == NULL) {
	    return TCL_ERROR;
	}
    }
    if (*iconPtr != NULL) {
	Blt_TreeView_FreeIcon(viewPtr, *iconPtr);
    }
    *iconPtr = icon;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * IconToObjProc --
 *
 *	Converts the icon into its string representation (its name).
 *
 * Results:
 *	The name of the icon is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
IconToObjProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    Icon icon = *(Icon *)(widgRec + offset);

    if (icon == NULL) {
	return Tcl_NewStringObj("", -1);
    } else {
	return Tcl_NewStringObj(Blt_Image_Name((icon)->tkImage), -1);
    }
}


/*
 *---------------------------------------------------------------------------
 *
 * ObjToEnum --
 *
 *	Converts the string into its enumerated type.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToEnum(
    ClientData clientData,	/* Vectors of valid strings. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,
    char *widgRec,		/* Widget record. */
    int offset,			/* Offset of field in record */
    int flags)			
{
    int *enumPtr = (int *)(widgRec + offset);
    char c;
    char **p;
    int i;
    int count;
    char *string;

    string = Tcl_GetString(objPtr);
    c = string[0];
    count = 0;
    for (p = (char **)clientData; *p != NULL; p++) {
	if ((c == p[0][0]) && (strcmp(string, *p) == 0)) {
	    *enumPtr = count;
	    return TCL_OK;
	}
	count++;
    }
    *enumPtr = -1;

    Tcl_AppendResult(interp, "bad value \"", string, "\": should be ", 
	(char *)NULL);
    p = (char **)clientData; 
    if (count > 0) {
	Tcl_AppendResult(interp, p[0], (char *)NULL);
    }
    for (i = 1; i < (count - 1); i++) {
	Tcl_AppendResult(interp, " ", p[i], ", ", (char *)NULL);
    }
    if (count > 1) {
	Tcl_AppendResult(interp, " or ", p[count - 1], ".", (char *)NULL);
    }
    return TCL_ERROR;
}

/*
 *---------------------------------------------------------------------------
 *
 * EnumToObj --
 *
 *	Returns the string associated with the enumerated type.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
EnumToObj(
    ClientData clientData,	/* List of strings. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,		/* Widget record */
    int offset,			/* Offset to field in structure */
    int flags)	
{
    int value = *(int *)(widgRec + offset);
    char **strings = (char **)clientData;
    char **p;
    int count;

    count = 0;
    for (p = strings; *p != NULL; p++) {
	if (value == count) {
	    return Tcl_NewStringObj(*p, -1);
	}
	count++;
    }
    return Tcl_NewStringObj("unknown value", -1);
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToSortMarkProc --
 *
 *	Convert the string reprsenting a column, to its numeric
 *	form.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left
 *	in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToSortMarkProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* New legend position string */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    TreeView *viewPtr = (TreeView *)widgRec;
    Column **colPtrPtr = (Column **)(widgRec + offset);
    const char *string;

    string = Tcl_GetString(objPtr);
    if (string[0] == '\0') {
	*colPtrPtr = NULL;		/* Don't display mark. */
    } else {
	if (GetColumn(interp, viewPtr, objPtr, colPtrPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SortMarkToObjProc --
 *
 * Results:
 *	The string representation of the column is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
SortMarkToObjProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Column *colPtr = *(Column **)(widgRec + offset);

    if (colPtr == NULL) {
	return Tcl_NewStringObj("", -1);
    } else {
	return Tcl_NewStringObj(colPtr->key, -1);
    }
}

/*ARGSUSED*/
static void
FreeSortColumnsProc(ClientData clientData, Display *display, char *widgRec, 
		  int offset)
{
    Blt_Chain *chainPtr = (Blt_Chain *)(widgRec + offset);

    if (*chainPtr != NULL) {
	Blt_Chain_Destroy(*chainPtr);
	*chainPtr = NULL;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToSortColumnsProc --
 *
 *	Convert the string reprsenting a column, to its numeric
 *	form.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left
 *	in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToSortColumnsProc(ClientData clientData, Tcl_Interp *interp, Tk_Window tkwin,
		   Tcl_Obj *objPtr, char *widgRec, int offset, int flags)	
{
    TreeView *viewPtr = (TreeView *)widgRec;
    Blt_Chain *chainPtr = (Blt_Chain *)(widgRec + offset);
    Blt_Chain chain;
    int i, objc;
    Tcl_Obj **objv;

    if (Tcl_ListObjGetElements(interp, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    chain = Blt_Chain_Create();
    for (i = 0; i < objc; i++) {
	Column *colPtr;

	if (GetColumn(interp, viewPtr, objv[i], &colPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (colPtr == NULL) {
	    continue;
	}
	Blt_Chain_Append(chain, colPtr);
    }
    if (*chainPtr != NULL) {
	Blt_Chain_Destroy(*chainPtr);
    }
    *chainPtr = chain;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SortColumnsToObjProc --
 *
 * Results:
 *	The string representation of the column is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
SortColumnsToObjProc(ClientData clientData, Tcl_Interp *interp, Tk_Window tkwin,
		   char *widgRec, int offset, int flags)	
{
    Blt_Chain chain = *(Blt_Chain *)(widgRec + offset);
    Blt_ChainLink link;
    Tcl_Obj *listObjPtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for (link = Blt_Chain_FirstLink(chain); link != NULL; 
	 link = Blt_Chain_NextLink(link)) {
	Column *colPtr;
	Tcl_Obj *objPtr;

	colPtr = Blt_Chain_GetValue(link);
	objPtr = Tcl_NewStringObj(colPtr->key, -1);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    }
    return listObjPtr;
}


/*
 *---------------------------------------------------------------------------
 *
 * ObjToData --
 *
 *	Convert the string reprsenting a scroll mode, to its numeric
 *	form.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left
 *	in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToData(
    ClientData clientData,	/* Node of entry. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* Tcl_Obj representing new data. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    Tcl_Obj **objv;
    Entry *entryPtr = (Entry *)widgRec;
    char *string;
    int objc;
    int i;

    string = Tcl_GetString(objPtr);
    if (*string == '\0') {
	return TCL_OK;
    } 
    if (Tcl_ListObjGetElements(interp, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    if (objc == 0) {
	return TCL_OK;
    }
    if (objc & 0x1) {
	Tcl_AppendResult(interp, "data \"", string, 
		 "\" must be in even name-value pairs", (char *)NULL);
	return TCL_ERROR;
    }
    for (i = 0; i < objc; i += 2) {
	Column *colPtr;
	TreeView *viewPtr = entryPtr->viewPtr;

	if (GetColumn(interp, viewPtr, objv[i], &colPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (colPtr == NULL) {
	    continue;
	}
	if (Blt_Tree_SetValueByKey(viewPtr->interp, viewPtr->tree, 
		entryPtr->node, colPtr->key, objv[i + 1]) != TCL_OK) {
	    return TCL_ERROR;
	}
	AddValue(entryPtr, colPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * DataToObj --
 *
 * Results:
 *	The string representation of the data is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
DataToObj(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,
    int offset,				/* Offset to field in structure */
    int flags)	
{
    Tcl_Obj *listObjPtr, *objPtr;
    Entry *entryPtr = (Entry *)widgRec;
    Value *valuePtr;

    /* Add the key-value pairs to a new Tcl_Obj */
    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for (valuePtr = entryPtr->values; valuePtr != NULL; 
	valuePtr = valuePtr->nextPtr) {
	objPtr = Tcl_NewStringObj(valuePtr->columnPtr->key, -1);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	if (GetData(entryPtr, valuePtr->columnPtr->key, &objPtr) != TCL_OK) {
	    objPtr = Tcl_NewStringObj("", -1);
	    Tcl_IncrRefCount(objPtr);
	} 
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    }
    return listObjPtr;
}

/*ARGSUSED*/
static void
FreeStyleProc(ClientData clientData, Display *display, char *widgRec, 
	      int offset)
{
    ColumnStyle **stylePtrPtr = (ColumnStyle **)(widgRec + offset);

    if (*stylePtrPtr != NULL) {
	FreeStyle(*stylePtrPtr);
	*stylePtrPtr = NULL;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToStyleProc --
 *
 *	Convert the name of an icon into a treeview style.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.
 *	Otherwise, TCL_ERROR is returned and an error message is left in
 *	interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToStyleProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,		/* Interpreter to send results back to */
    Tk_Window tkwin,		/* Not used. */
    Tcl_Obj *objPtr,		/* Tcl_Obj representing the new value. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    TreeView *viewPtr = clientData;
    ColumnStyle **stylePtrPtr = (ColumnStyle **)(widgRec + offset);
    ColumnStyle *stylePtr;
    const char *string;

    stylePtr = NULL;
    string = Tcl_GetString(objPtr);
    if ((string != NULL) && (string[0] != '\0')) {
	if (GetStyle(interp, viewPtr, Tcl_GetString(objPtr), 
				  &stylePtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	stylePtr->flags |= STYLE_DIRTY;
    }
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
    if (*stylePtrPtr != NULL) {
	FreeStyle(*stylePtrPtr);
    }
    *stylePtrPtr = stylePtr;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleToObjProc --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
StyleToObjProc(
    ClientData clientData,	/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,		/* Not used. */
    char *widgRec,
    int offset,			/* Offset to field in structure */
    int flags)	
{
    ColumnStyle *stylePtr = *(ColumnStyle **)(widgRec + offset);

    if (stylePtr == NULL) {
	return Tcl_NewStringObj("", -1);
    } else {
	return Tcl_NewStringObj(stylePtr->name, -1);
    }
}

static int
Apply(
    TreeView *viewPtr,
    Entry *entryPtr,			/* Root entry of subtree. */
    TreeViewApplyProc *proc,		/* Procedure called for each entry. */
    unsigned int flags)
{
    if ((flags & ENTRY_HIDE) && (EntryIsHidden(entryPtr))) {
	return TCL_OK;			/* Hidden node. */
    }
    if ((flags & entryPtr->flags) & ENTRY_HIDE) {
	return TCL_OK;			/* Hidden node. */
    }
    if ((flags | entryPtr->flags) & ENTRY_CLOSED) {
	Entry *childPtr;
	Blt_TreeNode node, next;

	for (node = Blt_Tree_FirstChild(entryPtr->node); node != NULL; 
	     node = next) {
	    next = Blt_Tree_NextSibling(node);
	    /* 
	     * Get the next child before calling Apply recursively.
	     * This is because the apply callback may delete the node and its
	     * link.
	     */
	    childPtr = NodeToEntry(viewPtr, node);
	    if (Apply(viewPtr, childPtr, proc, flags) != TCL_OK) {
		return TCL_ERROR;
	    }
	}
    }
    if ((*proc) (viewPtr, entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SkipSeparators --
 *
 *	Moves the character pointer past one of more separators.
 *
 * Results:
 *	Returns the updates character pointer.
 *
 *---------------------------------------------------------------------------
 */
static const char *
SkipSeparators(const char *path, const char *separator, int length)
{
    while ((path[0] == separator[0]) && 
	   (strncmp(path, separator, length) == 0)) {
	path += length;
    }
    return path;
}

/*
 *---------------------------------------------------------------------------
 *
 * DeleteNode --
 *
 *	Delete the node and its descendants.  Don't remove the root node,
 *	though.  If the root node is specified, simply remove all its
 *	children.
 *
 *---------------------------------------------------------------------------
 */
static void
DeleteNode(TreeView *viewPtr, Blt_TreeNode node)
{
    Blt_TreeNode root;

    if (!Blt_Tree_TagTableIsShared(viewPtr->tree)) {
	Blt_Tree_ClearTags(viewPtr->tree, node);
    }
    root = Blt_Tree_RootNode(viewPtr->tree);
    if (node == root) {
	Blt_TreeNode next;
	/* Don't delete the root node. Simply clean out the tree. */
	for (node = Blt_Tree_FirstChild(node); node != NULL; node = next) {
	    next = Blt_Tree_NextSibling(node);
	    Blt_Tree_DeleteNode(viewPtr->tree, node);
	}	    
    } else if (Blt_Tree_IsAncestor(root, node)) {
	Blt_Tree_DeleteNode(viewPtr->tree, node);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * SplitPath --
 *
 *	Returns the trailing component of the given path.  Trailing separators
 *	are ignored.
 *
 * Results:
 *	Returns the string of the tail component.
 *
 *---------------------------------------------------------------------------
 */
static int
SplitPath(TreeView *viewPtr, const char *path, long *depthPtr, 
	  const char ***argvPtr)
{
    int skipLen, pathLen;
    long depth;
    size_t listSize;
    char **argv;
    char *p;
    char *sep;

    if (viewPtr->pathSep == SEPARATOR_LIST) {
	int numElem;
	if (Tcl_SplitList(viewPtr->interp, path, &numElem, argvPtr) 
	    != TCL_OK) {
	    return TCL_ERROR;
	}
	*depthPtr = (long)numElem;
	return TCL_OK;
    }
    pathLen = strlen(path);
    skipLen = strlen(viewPtr->pathSep);
    path = SkipSeparators(path, viewPtr->pathSep, skipLen);
    depth = pathLen / skipLen;

    listSize = (depth + 1) * sizeof(char *);
    argv = Blt_AssertMalloc(listSize + (pathLen + 1));
    p = (char *)argv + listSize;
    strcpy(p, path);

    sep = strstr(p, viewPtr->pathSep);
    depth = 0;
    while ((*p != '\0') && (sep != NULL)) {
	*sep = '\0';
	argv[depth++] = p;
	p = (char *)SkipSeparators(sep + skipLen, viewPtr->pathSep, skipLen);
	sep = strstr(p, viewPtr->pathSep);
    }
    if (*p != '\0') {
	argv[depth++] = p;
    }
    argv[depth] = NULL;
    *depthPtr = depth;
    *argvPtr = (const char **)argv;
    return TCL_OK;
}


static Entry *
LastEntry(TreeView *viewPtr, Entry *entryPtr, unsigned int mask)
{
    Entry *nextPtr;

    nextPtr = LastChild(entryPtr, mask);
    while (nextPtr != NULL) {
	entryPtr = nextPtr;
	nextPtr = LastChild(entryPtr, mask);
    }
    return entryPtr;
}


/*
 *---------------------------------------------------------------------------
 *
 * ShowEntryApplyProc --
 *
 * Results:
 *	Always returns TCL_OK.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ShowEntryApplyProc(TreeView *viewPtr, Entry *entryPtr)
{
    entryPtr->flags &= ~ENTRY_HIDE;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * HideEntryApplyProc --
 *
 * Results:
 *	Always returns TCL_OK.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
HideEntryApplyProc(TreeView *viewPtr, Entry *entryPtr)
{
    entryPtr->flags |= ENTRY_HIDE;
    return TCL_OK;
}


static void
MapAncestors(TreeView *viewPtr, Entry *entryPtr)
{
    while (entryPtr != viewPtr->rootPtr) {
	entryPtr = ParentEntry(entryPtr);
	if (entryPtr->flags & (ENTRY_CLOSED | ENTRY_HIDE)) {
	    viewPtr->flags |= (DIRTY | LAYOUT_PENDING);
	    entryPtr->flags &= ~(ENTRY_CLOSED | ENTRY_HIDE);
	} 
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * MapAncestorsApplyProc --
 *
 *	If a node in mapped, then all its ancestors must be mapped also.  This
 *	routine traverses upwards and maps each unmapped ancestor.  It's
 *	assumed that for any mapped ancestor, all it's ancestors will already
 *	be mapped too.
 *
 * Results:
 *	Always returns TCL_OK.
 *
 *---------------------------------------------------------------------------
 */
static int
MapAncestorsApplyProc(TreeView *viewPtr, Entry *entryPtr)
{
    /*
     * Make sure that all the ancestors of this entry are mapped too.
     */
    while (entryPtr != viewPtr->rootPtr) {
	entryPtr = ParentEntry(entryPtr);
	if ((entryPtr->flags & (ENTRY_HIDE | ENTRY_CLOSED)) == 0) {
	    break;		/* Assume ancestors are also mapped. */
	}
	entryPtr->flags &= ~(ENTRY_HIDE | ENTRY_CLOSED);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * FindPath --
 *
 *	Finds the node designated by the given path.  Each path component is
 *	searched for as the tree is traversed.
 *
 *	A leading character string is trimmed off the path if it matches the
 *	one designated (see the -trimleft option).
 *
 *	If no separator is designated (see the -separator configuration
 *	option), the path is considered a TCL list.  Otherwise the each
 *	component of the path is separated by a character string.  Leading and
 *	trailing separators are ignored.  Multiple separators are treated as
 *	one.
 *
 * Results:
 *	Returns the pointer to the designated node.  If any component can't be
 *	found, NULL is returned.
 *
 *---------------------------------------------------------------------------
 */
static Entry *
FindPath(TreeView *viewPtr, Entry *rootPtr, const char *path)
{
    Blt_TreeNode child;
    const char **argv;
    const char *name;
    long numComp;
    const char **p;
    Entry *entryPtr;

    /* Trim off characters that we don't want */
    if (viewPtr->trimLeft != NULL) {
	const char *s1, *s2;
	
	/* Trim off leading character string if one exists. */
	for (s1 = path, s2 = viewPtr->trimLeft; *s2 != '\0'; s2++, s1++) {
	    if (*s1 != *s2) {
		break;
	    }
	}
	if (*s2 == '\0') {
	    path = s1;
	}
    }
    if (*path == '\0') {
	return rootPtr;
    }
    name = path;
    entryPtr = rootPtr;
    if (viewPtr->pathSep == SEPARATOR_NONE) {
	child = Blt_Tree_FindChild(entryPtr->node, name);
	if (child == NULL) {
	    goto error;
	}
	return NodeToEntry(viewPtr, child);
    }

    if (SplitPath(viewPtr, path, &numComp, &argv) != TCL_OK) {
	return NULL;
    }
    for (p = argv; *p != NULL; p++) {
	name = *p;
	child = Blt_Tree_FindChild(entryPtr->node, name);
	if (child == NULL) {
	    Blt_Free(argv);
	    goto error;
	}
	entryPtr = NodeToEntry(viewPtr, child);
    }
    Blt_Free(argv);
    return entryPtr;
 error:
    {
	Tcl_DString ds;

	GetEntryPath(viewPtr, entryPtr, FALSE, &ds);
	Tcl_AppendResult(viewPtr->interp, "can't find node \"", name,
		 "\" in parent node \"", Tcl_DStringValue(&ds), "\"", 
		(char *)NULL);
	Tcl_DStringFree(&ds);
    }
    return NULL;

}

/*
 *---------------------------------------------------------------------------
 *
 * NearestEntry --
 *
 *	Finds the entry closest to the given screen X-Y coordinates in the
 *	viewport.
 *
 * Results:
 *	Returns the pointer to the closest node.  If no node is visible (nodes
 *	may be hidden), NULL is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Entry *
NearestEntry(TreeView *viewPtr, int x, int y, int selectOne)
{
    Entry *lastPtr;
    Entry **p;

    /*
     * We implicitly can pick only visible entries.  So make sure that the
     * tree exists.
     */
    if (viewPtr->numVisible == 0) {
	return NULL;
    }
    if (y < viewPtr->titleHeight) {
	return (selectOne) ? viewPtr->visibleArr[0] : NULL;
    }
    /*
     * Since the entry positions were previously computed in world
     * coordinates, convert Y-coordinate from screen to world coordinates too.
     */
    y = WORLDY(viewPtr, y);
    lastPtr = viewPtr->visibleArr[0];
    for (p = viewPtr->visibleArr; *p != NULL; p++) {
	Entry *entryPtr;

	entryPtr = *p;
	/*
	 * If the start of the next entry starts beyond the point, use the last
	 * entry.
	 */
	if (entryPtr->worldY > y) {
	    return (selectOne) ? entryPtr : NULL;
	}
	if (y < (entryPtr->worldY + entryPtr->height)) {
	    return entryPtr;		/* Found it. */
	}
	lastPtr = entryPtr;
    }
    return (selectOne) ? lastPtr : NULL;
}


static int
GetEntryFromSpecialId(TreeView *viewPtr, const char *string, 
		      Entry **entryPtrPtr)
{
    Blt_TreeNode node;
    Entry *fromPtr, *entryPtr;
    char c;

    entryPtr = NULL;
    fromPtr = viewPtr->fromPtr;
    if (fromPtr == NULL) {
	fromPtr = viewPtr->focusPtr;
    } 
    if (fromPtr == NULL) {
	fromPtr = viewPtr->rootPtr;
    }
    c = string[0];
    if (c == '@') {
	int x, y;

	if (Blt_GetXY(viewPtr->interp, viewPtr->tkwin, string, &x, &y) == TCL_OK) {
	    *entryPtrPtr = NearestEntry(viewPtr, x, y, TRUE);
	}
    } else if ((c == 'b') && (strcmp(string, "bottom") == 0)) {
	if (viewPtr->flatView) {
	    entryPtr = viewPtr->flatArr[viewPtr->numEntries - 1];
	} else {
	    entryPtr = LastEntry(viewPtr, viewPtr->rootPtr, ENTRY_MASK);
	}
    } else if ((c == 't') && (strcmp(string, "top") == 0)) {
	if (viewPtr->flatView) {
	    entryPtr = viewPtr->flatArr[0];
	} else {
	    entryPtr = viewPtr->rootPtr;
	    if (viewPtr->flags & HIDE_ROOT) {
		entryPtr = NextEntry(entryPtr, ENTRY_MASK);
	    }
	}
    } else if ((c == 'e') && (strcmp(string, "end") == 0)) {
	entryPtr = LastEntry(viewPtr, viewPtr->rootPtr, ENTRY_MASK);
    } else if ((c == 'a') && (strcmp(string, "anchor") == 0)) {
	entryPtr = viewPtr->selection.anchorPtr;
    } else if ((c == 'f') && (strcmp(string, "focus") == 0)) {
	entryPtr = viewPtr->focusPtr;
	if ((entryPtr == viewPtr->rootPtr) && (viewPtr->flags & HIDE_ROOT)) {
	    entryPtr = NextEntry(viewPtr->rootPtr, ENTRY_MASK);
	}
    } else if ((c == 'r') && (strcmp(string, "root") == 0)) {
	entryPtr = viewPtr->rootPtr;
    } else if ((c == 'p') && (strcmp(string, "parent") == 0)) {
	if (fromPtr != viewPtr->rootPtr) {
	    entryPtr = ParentEntry(fromPtr);
	}
    } else if ((c == 'c') && (strcmp(string, "current") == 0)) {
	/* Can't trust picked item, if entries have been added or deleted. */
	if (!(viewPtr->flags & DIRTY)) {
	    ClientData hint;

	    hint = Blt_GetCurrentHint(viewPtr->bindTable);
	    if ((hint == ITEM_ENTRY) || (hint == ITEM_ENTRY_BUTTON) ||
		(hint >= ITEM_STYLE)) {
		TreeViewObj *objPtr;

		objPtr = Blt_GetCurrentItem(viewPtr->bindTable);
		if ((objPtr != NULL) && ((objPtr->flags & DELETED) == 0)) {
		    entryPtr = (Entry *)objPtr;
		}
	    }
	}
    } else if ((c == 'u') && (strcmp(string, "up") == 0)) {
	entryPtr = fromPtr;
	if (viewPtr->flatView) {
	    int i;

	    i = entryPtr->flatIndex - 1;
	    if (i >= 0) {
		entryPtr = viewPtr->flatArr[i];
	    }
	} else {
	    entryPtr = PrevEntry(fromPtr, ENTRY_MASK);
	    if (entryPtr == NULL) {
		entryPtr = fromPtr;
	    }
	    if ((entryPtr == viewPtr->rootPtr) && 
		(viewPtr->flags & HIDE_ROOT)) {
		entryPtr = NextEntry(entryPtr, ENTRY_MASK);
	    }
	}
    } else if ((c == 'd') && (strcmp(string, "down") == 0)) {
	entryPtr = fromPtr;
	if (viewPtr->flatView) {
	    int i;

	    i = entryPtr->flatIndex + 1;
	    if (i < viewPtr->numEntries) {
		entryPtr = viewPtr->flatArr[i];
	    }
	} else {
	    entryPtr = NextEntry(fromPtr, ENTRY_MASK);
	    if (entryPtr == NULL) {
		entryPtr = fromPtr;
	    }
	    if ((entryPtr == viewPtr->rootPtr) && 
		(viewPtr->flags & HIDE_ROOT)) {
		entryPtr = NextEntry(entryPtr, ENTRY_MASK);
	    }
	}
    } else if (((c == 'l') && (strcmp(string, "last") == 0)) ||
	       ((c == 'p') && (strcmp(string, "prev") == 0))) {
	entryPtr = fromPtr;
	if (viewPtr->flatView) {
	    int i;

	    i = entryPtr->flatIndex - 1;
	    if (i < 0) {
		i = viewPtr->numEntries - 1;
	    }
	    entryPtr = viewPtr->flatArr[i];
	} else {
	    entryPtr = PrevEntry(fromPtr, ENTRY_MASK);
	    if (entryPtr == NULL) {
		entryPtr = LastEntry(viewPtr, viewPtr->rootPtr, ENTRY_MASK);
	    }
	    if ((entryPtr == viewPtr->rootPtr) && 
		(viewPtr->flags & HIDE_ROOT)) {
		entryPtr = NextEntry(entryPtr, ENTRY_MASK);
	    }
	}
    } else if ((c == 'n') && (strcmp(string, "next") == 0)) {
	entryPtr = fromPtr;
	if (viewPtr->flatView) {
	    int i;

	    i = entryPtr->flatIndex + 1; 
	    if (i >= viewPtr->numEntries) {
		i = 0;
	    }
	    entryPtr = viewPtr->flatArr[i];
	} else {
	    entryPtr = NextEntry(fromPtr, ENTRY_MASK);
	    if (entryPtr == NULL) {
		if (viewPtr->flags & HIDE_ROOT) {
		    entryPtr = NextEntry(viewPtr->rootPtr,ENTRY_MASK);
		} else {
		    entryPtr = viewPtr->rootPtr;
		}
	    }
	}
    } else if ((c == 'n') && (strcmp(string, "nextsibling") == 0)) {
	node = Blt_Tree_NextSibling(fromPtr->node);
	if (node != NULL) {
	    entryPtr = NodeToEntry(viewPtr, node);
	}
    } else if ((c == 'p') && (strcmp(string, "prevsibling") == 0)) {
	node = Blt_Tree_PrevSibling(fromPtr->node);
	if (node != NULL) {
	    entryPtr = NodeToEntry(viewPtr, node);
	}
    } else if ((c == 'v') && (strcmp(string, "view.top") == 0)) {
	if (viewPtr->numVisible > 0) {
	    entryPtr = viewPtr->visibleArr[0];
	}
    } else if ((c == 'v') && (strcmp(string, "view.bottom") == 0)) {
	if (viewPtr->numVisible > 0) {
	    entryPtr = viewPtr->visibleArr[viewPtr->numVisible - 1];
	} 
    } else {
	return TCL_ERROR;
    }
    *entryPtrPtr = entryPtr;
    return TCL_OK;
}

static int
GetTagIter(TreeView *viewPtr, char *tagName, TagIterator *iterPtr)
{
    
    iterPtr->tagType = TAG_RESERVED | TAG_SINGLE;
    iterPtr->entryPtr = NULL;

    if (strcmp(tagName, "all") == 0) {
	iterPtr->entryPtr = viewPtr->rootPtr;
	iterPtr->tagType |= TAG_ALL;
    } else {
	Blt_HashTable *tablePtr;

	tablePtr = Blt_Tree_TagHashTable(viewPtr->tree, tagName);
	if (tablePtr != NULL) {
	    Blt_HashEntry *hPtr;
	    
	    iterPtr->tagType = TAG_USER_DEFINED; /* Empty tags are not an
						  * error. */
	    hPtr = Blt_FirstHashEntry(tablePtr, &iterPtr->cursor); 
	    if (hPtr != NULL) {
		Blt_TreeNode node;

		node = Blt_GetHashValue(hPtr);
		iterPtr->entryPtr = NodeToEntry(viewPtr, node);
		if (tablePtr->numEntries > 1) {
		    iterPtr->tagType |= TAG_MULTIPLE;
		}
	    }
	}  else {
	    iterPtr->tagType = TAG_UNKNOWN;
	    Tcl_AppendResult(viewPtr->interp, "can't find tag or id \"", tagName, 
		"\" in \"", Tk_PathName(viewPtr->tkwin), "\"", (char *)NULL);
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

/*ARGSUSED*/
static void
AddEntryTags(Tcl_Interp *interp, TreeView *viewPtr, Entry *entryPtr, 
	     Blt_Chain tags)
{
    Blt_HashEntry *hPtr;
    Blt_HashSearch cursor;

    for (hPtr = Blt_Tree_FirstTag(viewPtr->tree, &cursor); hPtr != NULL; 
	hPtr = Blt_NextHashEntry(&cursor)) {
	Blt_TreeTagEntry *tPtr;

	tPtr = Blt_GetHashValue(hPtr);
	hPtr = Blt_FindHashEntry(&tPtr->nodeTable, (char *)entryPtr->node);
	if (hPtr != NULL) {
	    Blt_Chain_Append(tags, 
		(ClientData)GetUid(viewPtr, tPtr->tagName));
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * AddTag --
 *
 *---------------------------------------------------------------------------
 */
static int
AddTag(TreeView *viewPtr, Blt_TreeNode node, const char *tagName)
{
    Entry *entryPtr;

    if (strcmp(tagName, "root") == 0) {
	Tcl_AppendResult(viewPtr->interp, "can't add reserved tag \"",
			 tagName, "\"", (char *)NULL);
	return TCL_ERROR;
    }
    if (isdigit(UCHAR(tagName[0]))) {
	long inode;
	
	if (Blt_GetLong(NULL, tagName, &inode) == TCL_OK) {
	    Tcl_AppendResult(viewPtr->interp, "invalid tag \"", tagName, 
			     "\": can't be a number.", (char *)NULL);
	    return TCL_ERROR;
	} 
    }
    if (tagName[0] == '@') {
	Tcl_AppendResult(viewPtr->interp, "invalid tag \"", tagName, 
		"\": can't start with \"@\"", (char *)NULL);
	return TCL_ERROR;
    } 
    viewPtr->fromPtr = NULL;
    if (GetEntryFromSpecialId(viewPtr, tagName, &entryPtr) == TCL_OK) {
	Tcl_AppendResult(viewPtr->interp, "invalid tag \"", tagName, 
		"\": is a special id", (char *)NULL);
	return TCL_ERROR;
    }
    /* Add the tag to the node. */
    Blt_Tree_AddTag(viewPtr->tree, node, tagName);
    return TCL_OK;
}
    
static Entry *
FirstTaggedEntry(TagIterator *iterPtr)
{
    return iterPtr->entryPtr;
}

static int
GetEntryIterator(TreeView *viewPtr, Tcl_Obj *objPtr, 
			 TagIterator *iterPtr)
{
    char *tagName;
    Entry *entryPtr;
    long inode;

    tagName = Tcl_GetString(objPtr); 
    viewPtr->fromPtr = NULL;
    if ((isdigit(UCHAR(tagName[0]))) && 
	(Blt_GetLongFromObj(viewPtr->interp, objPtr, &inode) == TCL_OK)) {
	Blt_TreeNode node;

	node = Blt_Tree_GetNode(viewPtr->tree, inode);
	if (node == NULL) {
	    Tcl_AppendResult(viewPtr->interp, "can't find node \"", 
			     Tcl_GetString(objPtr), "\"", (char *)NULL);
	    return TCL_ERROR;
	}
	iterPtr->entryPtr = NodeToEntry(viewPtr, node);
	iterPtr->tagType = (TAG_RESERVED | TAG_SINGLE);
    } else if (GetEntryFromSpecialId(viewPtr, tagName, &entryPtr) == TCL_OK) {
	iterPtr->entryPtr = entryPtr;
	iterPtr->tagType = (TAG_RESERVED | TAG_SINGLE);
    } else {
	if (GetTagIter(viewPtr, tagName, iterPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

static Entry *
NextTaggedEntry(TagIterator *iterPtr)
{
    Entry *entryPtr;

    entryPtr = NULL;
    if (iterPtr->entryPtr != NULL) {
	TreeView *viewPtr = iterPtr->entryPtr->viewPtr;

	if (iterPtr->tagType & TAG_ALL) {
	    entryPtr = NextEntry(iterPtr->entryPtr, 0);
	} else if (iterPtr->tagType & TAG_MULTIPLE) {
	    Blt_HashEntry *hPtr;
	    
	    hPtr = Blt_NextHashEntry(&iterPtr->cursor);
	    if (hPtr != NULL) {
		Blt_TreeNode node;

		node = Blt_GetHashValue(hPtr);
		entryPtr = NodeToEntry(viewPtr, node);
	    }
	} 
	iterPtr->entryPtr = entryPtr;
    }
    return entryPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * GetEntryFromObj2 --
 *
 *	Converts a string into node pointer.  The string may be in one of the
 *	following forms:
 *
 *	    NNN			- inode.
 *	    "active"		- Currently active node.
 *	    "anchor"		- anchor of selected region.
 *	    "current"		- Currently picked node in bindtable.
 *	    "focus"		- The node currently with focus.
 *	    "root"		- Root node.
 *	    "end"		- Last open node in the entire hierarchy.
 *	    "next"		- Next open node from the currently active
 *				  node. Wraps around back to top.
 *	    "last"		- Previous open node from the currently active
 *				  node. Wraps around back to bottom.
 *	    "up"		- Next open node from the currently active
 *				  node. Does not wrap around.
 *	    "down"		- Previous open node from the currently active
 *				  node. Does not wrap around.
 *	    "nextsibling"	- Next sibling of the current node.
 *	    "prevsibling"	- Previous sibling of the current node.
 *	    "parent"		- Parent of the current node.
 *	    "view.top"		- Top of viewport.
 *	    "view.bottom"	- Bottom of viewport.
 *	    @x,y		- Closest node to the specified X-Y position.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.  The
 *	pointer to the node is returned via nodePtr.  Otherwise, TCL_ERROR is
 *	returned and an error message is left in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
static int
GetEntryFromObj2(TreeView *viewPtr, Tcl_Obj *objPtr, Entry **entryPtrPtr)
{
    Tcl_Interp *interp;
    char *string;
    TagIterator iter;
    long inode;

    interp = viewPtr->interp;

    string = Tcl_GetString(objPtr);
    *entryPtrPtr = NULL;
    if ((isdigit(UCHAR(string[0]))) && 
	(Blt_GetLongFromObj(interp, objPtr, &inode) == TCL_OK)) {
	Blt_TreeNode node;

	node = Blt_Tree_GetNode(viewPtr->tree, inode);
	if (node != NULL) {
	    *entryPtrPtr = NodeToEntry(viewPtr, node);
	}
	return TCL_OK;		/* Node Id. */
    }
    if (GetEntryFromSpecialId(viewPtr, string, entryPtrPtr) == TCL_OK) {
	return TCL_OK;		/* Special Id. */
    }
    if (GetTagIter(viewPtr, string, &iter) != TCL_OK) {
	return TCL_ERROR;
    }
    if (iter.tagType & TAG_MULTIPLE) {
	Tcl_AppendResult(interp, "more than one entry tagged as \"", string, 
		"\"", (char *)NULL);
	return TCL_ERROR;
    }
    *entryPtrPtr = iter.entryPtr;
    return TCL_OK;		/* Singleton tag. */
}

static int
GetEntryFromObj(TreeView *viewPtr, Tcl_Obj *objPtr, Entry **entryPtrPtr)
{
    viewPtr->fromPtr = NULL;
    return GetEntryFromObj2(viewPtr, objPtr, entryPtrPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * static GetEntry --
 *
 *	Returns an entry based upon its index.
 *
 * Results:
 *	If the string is successfully converted, TCL_OK is returned.  The
 *	pointer to the node is returned via nodePtr.  Otherwise, TCL_ERROR is
 *	returned and an error message is left in interpreter's result field.
 *
 *---------------------------------------------------------------------------
 */
static int
GetEntry(TreeView *viewPtr, Tcl_Obj *objPtr, Entry **entryPtrPtr)
{
    Entry *entryPtr;

    if (GetEntryFromObj(viewPtr, objPtr, &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (entryPtr == NULL) {
	Tcl_ResetResult(viewPtr->interp);
	Tcl_AppendResult(viewPtr->interp, "can't find entry \"", 
		Tcl_GetString(objPtr), "\" in \"", Tk_PathName(viewPtr->tkwin), 
		"\"", (char *)NULL);
	return TCL_ERROR;
    }
    *entryPtrPtr = entryPtr;
    return TCL_OK;
}

static Blt_TreeNode 
GetNthNode(Blt_TreeNode parent, long position)
{
    Blt_TreeNode node;
    long count;

    count = 0;
    for(node = Blt_Tree_FirstChild(parent); node != NULL; 
	node = Blt_Tree_NextSibling(node)) {
	if (count == position) {
	    return node;
	}
    }
    return Blt_Tree_LastChild(parent);
}


/*
 *---------------------------------------------------------------------------
 *
 * SelectEntryApplyProc --
 *
 *	Sets the selection flag for a node.  The selection flag is
 *	set/cleared/toggled based upon the flag set in the treeview widget.
 *
 * Results:
 *	Always returns TCL_OK.
 *
 *---------------------------------------------------------------------------
 */
static int
SelectEntryApplyProc(TreeView *viewPtr, Entry *entryPtr)
{
    Blt_HashEntry *hPtr;

    switch (viewPtr->selection.flags & SELECT_MASK) {
    case SELECT_CLEAR:
	DeselectEntry(viewPtr, entryPtr);
	break;

    case SELECT_SET:
	SelectEntry(viewPtr, entryPtr);
	break;

    case SELECT_TOGGLE:
	hPtr = Blt_FindHashEntry(&viewPtr->selection.table, (char *)entryPtr);
	if (hPtr != NULL) {
	    DeselectEntry(viewPtr, entryPtr);
	} else {
	    SelectEntry(viewPtr, entryPtr);
	}
	break;
    }
    return TCL_OK;
}



/*
 *---------------------------------------------------------------------------
 *
 * PruneSelection --
 *
 *	The root entry being deleted or closed.  Deselect any of its
 *	descendants that are currently selected.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      If any of the entry's descendants are deselected the widget is
 *      redrawn and the a selection command callback is invoked (if there's
 *      one configured).
 *
 *---------------------------------------------------------------------------
 */
static void
PruneSelection(TreeView *viewPtr, Entry *rootPtr)
{
    Blt_ChainLink link, next;
    Entry *entryPtr;
    int changed;

    /* 
     * Check if any of the currently selected entries are a descendant of of
     * the current root entry.  Deselect the entry and indicate that the
     * treeview widget needs to be redrawn.
     */
    changed = FALSE;
    for (link = Blt_Chain_FirstLink(viewPtr->selection.list); link != NULL; 
	 link = next) {
	next = Blt_Chain_NextLink(link);
	entryPtr = Blt_Chain_GetValue(link);
	if (Blt_Tree_IsAncestor(rootPtr->node, entryPtr->node)) {
	    DeselectEntry(viewPtr, entryPtr);
	    changed = TRUE;
	}
    }
    if (changed) {
	EventuallyRedraw(viewPtr);
	if (viewPtr->selection.cmdObjPtr != NULL) {
	    EventuallyInvokeSelectCmd(viewPtr);
	}
    }
}

static int
ConfigureEntry(TreeView *viewPtr, Entry *entryPtr, int objc, 
	       Tcl_Obj *const *objv, int flags)
{
    GC newGC;
    Blt_ChainLink link;
    Column *colPtr;

    iconsOption.clientData = viewPtr;
    uidOption.clientData = viewPtr;
    labelOption.clientData = viewPtr;
    if (Blt_ConfigureWidgetFromObj(viewPtr->interp, viewPtr->tkwin, 
	entrySpecs, objc, objv, (char *)entryPtr, flags) != TCL_OK) {
	return TCL_ERROR;
    }
    /* 
     * Check if there are values that need to be added 
     */
    for(link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	colPtr = Blt_Chain_GetValue(link);
	AddValue(entryPtr, colPtr);
    }

    newGC = NULL;
    if ((entryPtr->font != NULL) || (entryPtr->color != NULL)) {
	Blt_Font font;
	XColor *colorPtr;
	XGCValues gcValues;
	unsigned long gcMask;

	font = entryPtr->font;
	if (font == NULL) {
	    font = GetStyleFont(&viewPtr->treeColumn);
	}
	colorPtr = CHOOSE(viewPtr->fgColor, entryPtr->color);
	gcMask = GCForeground | GCFont;
	gcValues.foreground = colorPtr->pixel;
	gcValues.font = Blt_Font_Id(font);
	newGC = Tk_GetGC(viewPtr->tkwin, gcMask, &gcValues);
    }
    if (entryPtr->gc != NULL) {
	Tk_FreeGC(viewPtr->display, entryPtr->gc);
    }
    /* Assume all changes require a new layout. */
    entryPtr->gc = newGC;
    entryPtr->flags |= ENTRY_LAYOUT_PENDING;
    if (Blt_ConfigModified(entrySpecs, "-font", (char *)NULL)) {
	viewPtr->flags |= UPDATE;
    }
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

int
Blt_TreeView_SetEntryValue(Tcl_Interp *interp, TreeView *viewPtr, 
			   Entry *entryPtr, Column *colPtr, const char *value)
{
    int valid;
    ColumnStyle *stylePtr;

    valid = TRUE;
    stylePtr = NULL;
    if (colPtr != &viewPtr->treeColumn) {
	Value *valuePtr;

	valuePtr = Blt_TreeView_FindValue(entryPtr, colPtr);
	if (valuePtr != NULL) {
	    stylePtr = valuePtr->stylePtr;
	}
    }
    if (stylePtr == NULL) {
	stylePtr = colPtr->stylePtr;
    }
    if (stylePtr->validateCmdObjPtr != NULL) {
	Tcl_Obj *cmdObjPtr, *objPtr;
	int result;

	cmdObjPtr= PercentSubst(viewPtr, entryPtr, stylePtr->validateCmdObjPtr);
	objPtr = Tcl_NewStringObj(value, -1);
	Tcl_ListObjAppendElement(interp, cmdObjPtr, objPtr);
	Tcl_IncrRefCount(cmdObjPtr);
	Tcl_Preserve(entryPtr);
	result = Tcl_EvalObjEx(viewPtr->interp, cmdObjPtr, TCL_EVAL_GLOBAL);
	Tcl_Release(entryPtr);
	Tcl_DecrRefCount(cmdObjPtr);
	if (result == TCL_OK) {
	    value = Tcl_GetString(Tcl_GetObjResult(interp));
	} else {
	    valid = FALSE;
	}
    }
    if (valid) {
	if (colPtr == &viewPtr->treeColumn) {
	    if (entryPtr->labelUid != NULL) {
		FreeUid(viewPtr, entryPtr->labelUid);
	    }
	    if (value == NULL) {
		entryPtr->labelUid = GetUid(viewPtr, "");
	    } else {
		entryPtr->labelUid = GetUid(viewPtr, value);
	    }
	} else {
	    Tcl_Obj *objPtr;
	    
	    objPtr = Tcl_NewStringObj(value, -1);
	    if (Blt_Tree_SetValueByKey(interp, viewPtr->tree, entryPtr->node, 
		colPtr->key, objPtr) != TCL_OK) {
		Tcl_DecrRefCount(objPtr);
		return TCL_ERROR;
	    }
	    entryPtr->flags |= ENTRY_DIRTY;
	}	
    }
    if (viewPtr != NULL) {
	ConfigureEntry(viewPtr, entryPtr, 0, NULL, 
		BLT_CONFIG_OBJV_ONLY);
    }
    return TCL_OK;
}

static void
ConfigureButtons(TreeView *viewPtr)
{
    GC newGC;
    Button *buttonPtr = &viewPtr->button;
    XGCValues gcValues;
    unsigned long gcMask;

    gcMask = GCForeground;
    gcValues.foreground = buttonPtr->fgColor->pixel;
    newGC = Tk_GetGC(viewPtr->tkwin, gcMask, &gcValues);
    if (buttonPtr->normalGC != NULL) {
	Tk_FreeGC(viewPtr->display, buttonPtr->normalGC);
    }
    buttonPtr->normalGC = newGC;

    gcMask = GCForeground;
    gcValues.foreground = buttonPtr->activeFgColor->pixel;
    newGC = Tk_GetGC(viewPtr->tkwin, gcMask, &gcValues);
    if (buttonPtr->activeGC != NULL) {
	Tk_FreeGC(viewPtr->display, buttonPtr->activeGC);
    }
    buttonPtr->activeGC = newGC;

    buttonPtr->width = buttonPtr->height = ODD(buttonPtr->reqSize);
    if (buttonPtr->icons != NULL) {
	int i;

	for (i = 0; i < 2; i++) {
	    int width, height;

	    if (buttonPtr->icons[i] == NULL) {
		break;
	    }
	    width = TreeView_IconWidth(buttonPtr->icons[i]);
	    height = TreeView_IconWidth(buttonPtr->icons[i]);
	    if (buttonPtr->width < width) {
		buttonPtr->width = width;
	    }
	    if (buttonPtr->height < height) {
		buttonPtr->height = height;
	    }
	}
    }
    buttonPtr->width += 2 * buttonPtr->borderWidth;
    buttonPtr->height += 2 * buttonPtr->borderWidth;
}


static void
DestroyValue(TreeView *viewPtr, Value *valuePtr)
{
    if (valuePtr->stylePtr != NULL) {
	FreeStyle(valuePtr->stylePtr);
    }
    if (valuePtr->textPtr != NULL) {
	Blt_Free(valuePtr->textPtr);
	valuePtr->textPtr = NULL;
    }
}

static void
FreeEntryProc(DestroyData data)
{
    TreeView *viewPtr;
    Entry *entryPtr = (Entry *)data;
    
    viewPtr = entryPtr->viewPtr;
    Blt_Pool_FreeItem(viewPtr->entryPool, entryPtr);
}

static void
DestroyEntry(Entry *entryPtr)
{
    TreeView *viewPtr;
    
    entryPtr->flags |= DELETED;		/* Mark the entry as destroyed. */

    viewPtr = entryPtr->viewPtr;
    if (entryPtr == viewPtr->activePtr) {
	viewPtr->activePtr = ParentEntry(entryPtr);
    }
    if (entryPtr == viewPtr->activeBtnPtr) {
	viewPtr->activeBtnPtr = NULL;
    }
    if (entryPtr == viewPtr->focusPtr) {
	viewPtr->focusPtr = ParentEntry(entryPtr);
	Blt_SetFocusItem(viewPtr->bindTable, viewPtr->focusPtr, ITEM_ENTRY);
    }
    if (entryPtr == viewPtr->selection.anchorPtr) {
	viewPtr->selection.markPtr = viewPtr->selection.anchorPtr = NULL;
    }
    DeselectEntry(viewPtr, entryPtr);
    PruneSelection(viewPtr, entryPtr);
    Blt_DeleteBindings(viewPtr->bindTable, entryPtr);
    if (entryPtr->hashPtr != NULL) {
	Blt_DeleteHashEntry(&viewPtr->entryTable, entryPtr->hashPtr);
    }
    entryPtr->node = NULL;

    iconsOption.clientData = viewPtr;
    uidOption.clientData = viewPtr;
    labelOption.clientData = viewPtr;
    Blt_FreeOptions(entrySpecs, (char *)entryPtr, viewPtr->display, 0);
    if (viewPtr->rootPtr == entryPtr) {
	Blt_TreeNode root;

	/* Restore the root node back to the top of the tree. */
	root = Blt_Tree_RootNode(viewPtr->tree);
	viewPtr->rootPtr = NodeToEntry(viewPtr,root);
    }
    if (!Blt_Tree_TagTableIsShared(viewPtr->tree)) {
	/* Don't clear tags unless this client is the only one using
	 * the tag table.*/
	Blt_Tree_ClearTags(viewPtr->tree, entryPtr->node);
    }
    if (entryPtr->gc != NULL) {
	Tk_FreeGC(viewPtr->display, entryPtr->gc);
    }
    /* Delete the chain of data values from the entry. */
    if (entryPtr->values != NULL) {
	Value *valuePtr, *nextPtr;
	
	for (valuePtr = entryPtr->values; valuePtr != NULL; 
	     valuePtr = nextPtr) {
	    nextPtr = valuePtr->nextPtr;
	    DestroyValue(viewPtr, valuePtr);
	}
	entryPtr->values = NULL;
    }
    if (entryPtr->fullName != NULL) {
	Blt_Free(entryPtr->fullName);
    }
    if (entryPtr->textPtr != NULL) {
	Blt_Free(entryPtr->textPtr);
	entryPtr->textPtr = NULL;
    }
    Tcl_EventuallyFree(entryPtr, FreeEntryProc);
}




/*
 *---------------------------------------------------------------------------
 *
 * CreateEntry --
 *
 *	This procedure is called by the Tree object when a node is created and
 *	inserted into the tree.  It adds a new treeview entry field to the node.
 *
 * Results:
 *	Returns the entry.
 *
 *---------------------------------------------------------------------------
 */
static int
CreateEntry(
    TreeView *viewPtr,
    Blt_TreeNode node,			/* Node that has just been created. */
    int objc,
    Tcl_Obj *const *objv,
    int flags)
{
    Entry *entryPtr;
    int isNew;
    Blt_HashEntry *hPtr;

    hPtr = Blt_CreateHashEntry(&viewPtr->entryTable, (char *)node, &isNew);
    if (isNew) {
	/* Create the entry structure */
	entryPtr = Blt_Pool_AllocItem(viewPtr->entryPool, sizeof(Entry));
	memset(entryPtr, 0, sizeof(Entry));
	entryPtr->flags = (unsigned short)(viewPtr->buttonFlags | ENTRY_CLOSED);
	entryPtr->viewPtr = viewPtr;
	entryPtr->hashPtr = hPtr;
	entryPtr->labelUid = NULL;
	entryPtr->node = node;
	Blt_SetHashValue(hPtr, entryPtr);

    } else {
	entryPtr = Blt_GetHashValue(hPtr);
    }
    if (ConfigureEntry(viewPtr, entryPtr, objc, objv, flags) 
	!= TCL_OK) {
	DestroyEntry(entryPtr);
	return TCL_ERROR;		/* Error configuring the entry. */
    }
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}


/*ARGSUSED*/
static int
CreateApplyProc(Blt_TreeNode node, ClientData clientData, int order)
{
    TreeView *viewPtr = clientData; 
    return CreateEntry(viewPtr, node, 0, NULL, 0);
}

/*ARGSUSED*/
static int
DeleteApplyProc(Blt_TreeNode node, ClientData clientData, int order)
{
    TreeView *viewPtr = clientData;
    /* 
     * Unsetting the tree value triggers a call back to destroy the entry and
     * also releases the Tcl_Obj that contains it.
     */
    return Blt_Tree_UnsetValueByKey(viewPtr->interp, viewPtr->tree, node, 
	viewPtr->treeColumn.key);
}

static int
TreeEventProc(ClientData clientData, Blt_TreeNotifyEvent *eventPtr)
{
    Blt_TreeNode node;
    TreeView *viewPtr = clientData; 

    node = Blt_Tree_GetNode(eventPtr->tree, eventPtr->inode);
    switch (eventPtr->type) {
    case TREE_NOTIFY_CREATE:
	return CreateEntry(viewPtr, node, 0, NULL, 0);
    case TREE_NOTIFY_DELETE:
	/*  
	 * Deleting the tree node triggers a call back to free the treeview
	 * entry that is associated with it.
	 */
	if (node != NULL) {
	    Entry *entryPtr;

	    entryPtr = FindEntry(viewPtr, node);
	    if (entryPtr != NULL) {
		DestroyEntry(entryPtr);
		viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
		EventuallyRedraw(viewPtr);
	    }
	}
	break;
    case TREE_NOTIFY_RELABEL:
	if (node != NULL) {
	    Entry *entryPtr;

	    entryPtr = NodeToEntry(viewPtr, node);
	    entryPtr->flags |= ENTRY_DIRTY;
	}
	/*FALLTHRU*/
    case TREE_NOTIFY_MOVE:
    case TREE_NOTIFY_SORT:
	viewPtr->flags |= (LAYOUT_PENDING | RESORT | DIRTY);
	EventuallyRedraw(viewPtr);
	break;
    default:
	/* empty */
	break;
    }	
    return TCL_OK;
}

Value *
Blt_TreeView_FindValue(Entry *entryPtr, Column *colPtr)
{
    Value *vp;

    for (vp = entryPtr->values; vp != NULL; vp = vp->nextPtr) {
	if (vp->columnPtr == colPtr) {
	    return vp;
	}
    }
    return NULL;
}


/*
 *---------------------------------------------------------------------------
 *
 * TreeTraceProc --
 *
 *	Mirrors the individual values of the tree object (they must also be
 *	listed in the widget's columns chain). This is because it must track and
 *	save the sizes of each individual data entry, rather than re-computing
 *	all the sizes each time the widget is redrawn.
 *
 *	This procedure is called by the Tree object when a node data value is
 *	set unset.
 *
 * Results:
 *	Returns TCL_OK.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
TreeTraceProc(
    ClientData clientData,
    Tcl_Interp *interp,
    Blt_TreeNode node,			/* Node that has just been updated. */
    Blt_TreeKey key,			/* Key of value that's been updated. */
    unsigned int flags)
{
    Blt_HashEntry *hPtr;
    TreeView *viewPtr = clientData; 
    Column *colPtr;
    Entry *entryPtr;
    Value *valuePtr, *nextPtr, *lastPtr;
    
    hPtr = Blt_FindHashEntry(&viewPtr->entryTable, (char *)node);
    if (hPtr == NULL) {
	return TCL_OK;			/* Not a node that we're interested
					 * in. */
    }
    entryPtr = Blt_GetHashValue(hPtr);
    flags &= TREE_TRACE_WRITES | TREE_TRACE_READS | TREE_TRACE_UNSETS;
    switch (flags) {
    case TREE_TRACE_WRITES:
	hPtr = Blt_FindHashEntry(&viewPtr->columnTable, key);
	if (hPtr == NULL) {
	    return TCL_OK;		/* Data value isn't used by widget. */
	}
	colPtr = Blt_GetHashValue(hPtr);
	if (colPtr != &viewPtr->treeColumn) {
	    AddValue(entryPtr, colPtr);
	}
	entryPtr->flags |= ENTRY_DIRTY;
	EventuallyRedraw(viewPtr);
	viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
	break;

    case TREE_TRACE_UNSETS:
	lastPtr = NULL;
	for(valuePtr = entryPtr->values; valuePtr != NULL; valuePtr = nextPtr) {
	    nextPtr = valuePtr->nextPtr;
	    if (valuePtr->columnPtr->key == key) { 
		DestroyValue(viewPtr, valuePtr);
		if (lastPtr == NULL) {
		    entryPtr->values = nextPtr;
		} else {
		    lastPtr->nextPtr = nextPtr;
		}
		entryPtr->flags |= ENTRY_DIRTY;
		EventuallyRedraw(viewPtr);
		viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
		break;
	    }
	    lastPtr = valuePtr;
	}		
	break;

    default:
	break;
    }
    return TCL_OK;
}

static void
FormatValue(Entry *entryPtr, Value *valuePtr)
{
    Column *colPtr;
    Tcl_Obj *resultObjPtr;
    Tcl_Obj *valueObjPtr;

    colPtr = valuePtr->columnPtr;
    if (GetData(entryPtr, colPtr->key, &valueObjPtr) != TCL_OK) {
	return;				/* No data ??? */
    }
    if (valuePtr->fmtString != NULL) {
	Blt_Free(valuePtr->fmtString);
    }
    valuePtr->fmtString = NULL;
    if (valueObjPtr == NULL) {
	return;
    }
    if (colPtr->fmtCmdPtr  != NULL) {
	Tcl_Interp *interp = entryPtr->viewPtr->interp;
	Tcl_Obj *cmdObjPtr, *objPtr;
	int result;

	cmdObjPtr = Tcl_DuplicateObj(colPtr->fmtCmdPtr);
	objPtr = Tcl_NewLongObj(Blt_Tree_NodeId(entryPtr->node));
	Tcl_ListObjAppendElement(interp, cmdObjPtr, objPtr);
	Tcl_ListObjAppendElement(interp, cmdObjPtr, valueObjPtr);
	Tcl_IncrRefCount(cmdObjPtr);
	result = Tcl_EvalObjEx(interp, cmdObjPtr, TCL_EVAL_GLOBAL);
	Tcl_DecrRefCount(cmdObjPtr);
	if (result != TCL_OK) {
	    Tcl_BackgroundError(interp);
	    return;
	}
	resultObjPtr = Tcl_GetObjResult(interp);
    } else {
	resultObjPtr = valueObjPtr;
    }
    valuePtr->fmtString = Blt_Strdup(Tcl_GetString(resultObjPtr));
}


static void
GetValueSize(Entry *entryPtr, Value *valuePtr, ColumnStyle *stylePtr)
{
    valuePtr->width = valuePtr->height = 0;
    FormatValue(entryPtr, valuePtr);
    stylePtr = CHOOSESTYLE(entryPtr->viewPtr, valuePtr->columnPtr, valuePtr);
    /* Measure the text string. */
    (*stylePtr->classPtr->geomProc)(stylePtr, valuePtr);
}

static void
GetRowExtents(Entry *entryPtr, int *widthPtr, int *heightPtr)
{
    Value *valuePtr;
    int width, height;			/* Compute dimensions of row. */

    width = height = 0;
    for (valuePtr = entryPtr->values; valuePtr != NULL; 
	 valuePtr = valuePtr->nextPtr) {
	ColumnStyle *stylePtr;
	int valueWidth;			/* Width of individual value.  */

	stylePtr = CHOOSESTYLE(entryPtr->viewPtr, valuePtr->columnPtr,valuePtr);
	if ((entryPtr->flags & ENTRY_DIRTY) || (stylePtr->flags & STYLE_DIRTY)){
	    GetValueSize(entryPtr, valuePtr, stylePtr);
	}
	if (valuePtr->height > height) {
	    height = valuePtr->height;
	}
	valueWidth = valuePtr->width;
	width += valueWidth;
    }	    
    *widthPtr = width;
    *heightPtr = height;
}


static ClientData
EntryTag(TreeView *viewPtr, const char *string)
{
    Blt_HashEntry *hPtr;
    int isNew;				/* Not used. */

    hPtr = Blt_CreateHashEntry(&viewPtr->entryTagTable, string, &isNew);
    return Blt_GetHashKey(&viewPtr->entryTagTable, hPtr);
}

static ClientData
ButtonTag(TreeView *viewPtr, const char *string)
{
    Blt_HashEntry *hPtr;
    int isNew;				/* Not used. */

    hPtr = Blt_CreateHashEntry(&viewPtr->buttonTagTable, string, &isNew);
    return Blt_GetHashKey(&viewPtr->buttonTagTable, hPtr);
}

static ClientData
ColumnTag(TreeView *viewPtr, const char *string)
{
    Blt_HashEntry *hPtr;
    int isNew;				/* Not used. */

    hPtr = Blt_CreateHashEntry(&viewPtr->columnTagTable, string, &isNew);
    return Blt_GetHashKey(&viewPtr->columnTagTable, hPtr);
}

#ifdef notdef
static ClientData
StyleTag(TreeView *viewPtr, const char *string)
{
    Blt_HashEntry *hPtr;
    int isNew;				/* Not used. */

    hPtr = Blt_CreateHashEntry(&viewPtr->styleTagTable, string, &isNew);
    return Blt_GetHashKey(&viewPtr->styleTagTable, hPtr);
}
#endif

static void
AddTags(TreeView *viewPtr, Blt_Chain tags, const char *string, TagProc *tagProc)
{
    int argc;
    const char **argv;
    
    if (Tcl_SplitList((Tcl_Interp *)NULL, string, &argc, &argv) == TCL_OK) {
	const char **p;

	for (p = argv; *p != NULL; p++) {
	    Blt_Chain_Append(tags, (*tagProc)(viewPtr, *p));
	}
	Blt_Free(argv);
    }
}

static void
AppendTagsProc(
    Blt_BindTable table,
    ClientData object,			/* Object picked. */
    ClientData hint,			/* Context of object. */
    Blt_Chain tags)			/* (out) List of binding ids to be
					 * applied for this object. */
{
    TreeView *viewPtr;
    TreeViewObj *objPtr;

    objPtr = object;
    if (objPtr->flags & DELETED) {
	return;
    }
    viewPtr = objPtr->viewPtr;
    if (hint == (ClientData)ITEM_ENTRY_BUTTON) {
	Entry *entryPtr = object;

	Blt_Chain_Append(tags, ButtonTag(viewPtr, "Button"));
	if (entryPtr->tagsUid != NULL) {
	    AddTags(viewPtr, tags, entryPtr->tagsUid, ButtonTag);
	} else {
	    Blt_Chain_Append(tags, ButtonTag(viewPtr, "Entry"));
	    Blt_Chain_Append(tags, ButtonTag(viewPtr, "all"));
	}
    } else if (hint == (ClientData)ITEM_COLUMN_TITLE) {
	Column *colPtr = object;

	Blt_Chain_Append(tags, ColumnTag(viewPtr, colPtr->key));
	if (colPtr->tagsUid != NULL) {
	    AddTags(viewPtr, tags, colPtr->tagsUid, ColumnTag);
	}
    } else if (hint == ITEM_COLUMN_RULE) {
	Blt_Chain_Append(tags, ColumnTag(viewPtr, "Rule"));
    } else {
	Entry *entryPtr = object;

	Blt_Chain_Append(tags, entryPtr);
	if (entryPtr->tagsUid != NULL) {
	    AddTags(viewPtr, tags, entryPtr->tagsUid, EntryTag);
	} else if (hint == ITEM_ENTRY){
	    Blt_Chain_Append(tags, EntryTag(viewPtr, "Entry"));
	    Blt_Chain_Append(tags, EntryTag(viewPtr, "all"));
	} else {
	    Value *valuePtr = hint;

	    if (valuePtr != NULL) {
		ColumnStyle *stylePtr = valuePtr->stylePtr;

		stylePtr = CHOOSESTYLE(viewPtr, valuePtr->columnPtr, valuePtr);
		Blt_Chain_Append(tags, 
	            EntryTag(viewPtr, stylePtr->name));
		Blt_Chain_Append(tags, 
		    EntryTag(viewPtr, valuePtr->columnPtr->key));
		Blt_Chain_Append(tags, 
		    EntryTag(viewPtr, 
			stylePtr->classPtr->className));
#ifndef notdef
		Blt_Chain_Append(tags, EntryTag(viewPtr, "Entry"));
		Blt_Chain_Append(tags, EntryTag(viewPtr, "all"));
#endif
	    }
	}
    }
}

/*ARGSUSED*/
static ClientData
PickItem(
    ClientData clientData,
    int x, int y,			/* Screen coordinates of the test
					 * point. */
    ClientData *hintPtr)		/* (out) Context of item selected:
					 * should be ITEM_ENTRY,
					 * ITEM_ENTRY_BUTTON, ITEM_COLUMN_TITLE,
					 * ITEM_COLUMN_RULE, or ITEM_STYLE. */
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    Column *colPtr;

    if (hintPtr != NULL) {
	*hintPtr = NULL;
    }
    if (viewPtr->flags & DIRTY) {
	/* Can't trust the selected entry if nodes have been added or
	 * deleted. So recompute the layout. */
	if (viewPtr->flags & LAYOUT_PENDING) {
	    ComputeLayout(viewPtr);
	} 
	ComputeVisibleEntries(viewPtr);
    }
    colPtr = NearestColumn(viewPtr, x, y, hintPtr);
    if ((*hintPtr != NULL) && (viewPtr->flags & SHOW_COLUMN_TITLES)) {
	return colPtr;
    }
    if (viewPtr->numVisible == 0) {
	return NULL;
    }
    entryPtr = NearestEntry(viewPtr, x, y, FALSE);
    if (entryPtr == NULL) {
	return NULL;
    }
    x = WORLDX(viewPtr, x);
    y = WORLDY(viewPtr, y);
    if (hintPtr != NULL) {
	*hintPtr = ITEM_ENTRY;
	if (colPtr != NULL) {
	    Value *valuePtr;

	    valuePtr = Blt_TreeView_FindValue(entryPtr, colPtr);
	    if (valuePtr != NULL) {
		ColumnStyle *stylePtr;
	
		stylePtr = CHOOSESTYLE(viewPtr, colPtr, valuePtr);
		if ((stylePtr->classPtr->identProc == NULL) ||
		    ((*stylePtr->classPtr->identProc)(entryPtr, valuePtr, 
			stylePtr, x, y))) {
		    *hintPtr = valuePtr;
		} 
	    }
	}
	if (entryPtr->flags & ENTRY_HAS_BUTTON) {
	    Button *buttonPtr = &viewPtr->button;
	    int left, right, top, bottom;
	    
	    left = entryPtr->worldX + entryPtr->buttonX - BUTTON_PAD;
	    right = left + buttonPtr->width + 2 * BUTTON_PAD;
	    top = entryPtr->worldY + entryPtr->buttonY - BUTTON_PAD;
	    bottom = top + buttonPtr->height + 2 * BUTTON_PAD;
	    if ((x >= left) && (x < right) && (y >= top) && (y < bottom)) {
		*hintPtr = (ClientData)ITEM_ENTRY_BUTTON;
	    }
	}
    }
    return entryPtr;
}

static void
GetEntryExtents(TreeView *viewPtr, Entry *entryPtr)
{
    int entryWidth, entryHeight;
    int width, height;

    /*
     * FIXME: Use of DIRTY flag inconsistent.  When does it
     *	      mean "dirty entry"? When does it mean "dirty column"?
     *	      Does it matter? probably
     */
    if ((entryPtr->flags & ENTRY_DIRTY) || (viewPtr->flags & UPDATE)) {
	Blt_Font font;
	Blt_FontMetrics fontMetrics;
	Icon *icons;
	const char *label;

	entryPtr->iconWidth = entryPtr->iconHeight = 0;
	icons = CHOOSE(viewPtr->icons, entryPtr->icons);
	if (icons != NULL) {
	    int i;
	    
	    for (i = 0; i < 2; i++) {
		if (icons[i] == NULL) {
		    break;
		}
		if (entryPtr->iconWidth < TreeView_IconWidth(icons[i])) {
		    entryPtr->iconWidth = TreeView_IconWidth(icons[i]);
		}
		if (entryPtr->iconHeight < TreeView_IconHeight(icons[i])) {
		    entryPtr->iconHeight = TreeView_IconHeight(icons[i]);
		}
	    }
	    entryPtr->iconWidth  += 2 * ICON_PADX;
	    entryPtr->iconHeight += 2 * ICON_PADY;
	} else if ((icons == NULL) || (icons[0] == NULL)) {
	    entryPtr->iconWidth = DEF_ICON_WIDTH;
	    entryPtr->iconHeight = DEF_ICON_HEIGHT;
	}
	entryHeight = MAX(entryPtr->iconHeight, viewPtr->button.height);
	font = entryPtr->font;
	if (font == NULL) {
	    font = GetStyleFont(&viewPtr->treeColumn);
	}
	if (entryPtr->fullName != NULL) {
	    Blt_Free(entryPtr->fullName);
	    entryPtr->fullName = NULL;
	}
	if (entryPtr->textPtr != NULL) {
	    Blt_Free(entryPtr->textPtr);
	    entryPtr->textPtr = NULL;
	}
	
	Blt_Font_GetMetrics(font, &fontMetrics);
	entryPtr->lineHeight = fontMetrics.linespace;
	entryPtr->lineHeight += 2 * (FOCUS_WIDTH + LABEL_PADY + 
		viewPtr->selection.borderWidth) + viewPtr->leader;

	label = GETLABEL(entryPtr);
	if (label[0] == '\0') {
	    width = height = entryPtr->lineHeight;
	} else {
	    TextStyle ts;

	    Blt_Ts_InitStyle(ts);
	    Blt_Ts_SetFont(ts, font);
	    if (viewPtr->flatView) {
		Tcl_DString ds;

		GetEntryPath(viewPtr, entryPtr, TRUE, &ds);
		entryPtr->fullName = Blt_AssertStrdup(Tcl_DStringValue(&ds));
		Tcl_DStringFree(&ds);
		entryPtr->textPtr = Blt_Ts_CreateLayout(entryPtr->fullName, -1,
			&ts);
	    } else {
		entryPtr->textPtr = Blt_Ts_CreateLayout(label, -1, &ts);
	    }
	    width = entryPtr->textPtr->width;
	    height = entryPtr->textPtr->height;
	}
	width  += 2 * (FOCUS_WIDTH+LABEL_PADX+viewPtr->selection.borderWidth);
	height += 2 * (FOCUS_WIDTH+LABEL_PADY+viewPtr->selection.borderWidth);
	width = ODD(width);
	if (entryPtr->reqHeight > height) {
	    height = entryPtr->reqHeight;
	} 
	height = ODD(height);
	entryWidth = width;
	if (entryHeight < height) {
	    entryHeight = height;
	}
	entryPtr->labelWidth = width;
	entryPtr->labelHeight = height;
    } else {
	entryHeight = entryPtr->labelHeight;
	entryWidth = entryPtr->labelWidth;
    }
    entryHeight = MAX3(entryPtr->iconHeight, entryPtr->lineHeight, 
		       entryPtr->labelHeight);

    /*  
     * Find the maximum height of the data value entries. This also has the
     * side effect of contributing the maximum width of the column.
     */
    GetRowExtents(entryPtr, &width, &height);
    if (entryHeight < height) {
	entryHeight = height;
    }
    entryPtr->width = entryWidth + COLUMN_PAD;
    entryPtr->height = entryHeight + viewPtr->leader;

    /*
     * Force the height of the entry to an even number. This is to make the
     * dots or the vertical line segments coincide with the start of the
     * horizontal lines.
     */
    if (entryPtr->height & 0x01) {
	entryPtr->height++;
    }
    entryPtr->flags &= ~ENTRY_DIRTY;
}

static void
ConfigureColumn(TreeView *viewPtr, Column *colPtr)
{
    Drawable drawable;
    GC newGC;
    XGCValues gcValues;
    int ruleDrawn;
    unsigned long gcMask;
    unsigned int aw, ah, iw, ih, tw, th;
    Blt_Bg bg;

    colPtr->titleWidth = colPtr->titleHeight = 0;

    gcMask = GCForeground | GCFont;
    gcValues.font = Blt_Font_Id(colPtr->titleFont);

    /* Normal title text */
    gcValues.foreground = colPtr->titleFgColor->pixel;
    newGC = Tk_GetGC(viewPtr->tkwin, gcMask, &gcValues);
    if (colPtr->titleGC != NULL) {
	Tk_FreeGC(viewPtr->display, colPtr->titleGC);
    }
    colPtr->titleGC = newGC;

    /* Active title text */
    gcValues.foreground = colPtr->activeTitleFgColor->pixel;
    newGC = Tk_GetGC(viewPtr->tkwin, gcMask, &gcValues);
    if (colPtr->activeTitleGC != NULL) {
	Tk_FreeGC(viewPtr->display, colPtr->activeTitleGC);
    }
    colPtr->activeTitleGC = newGC;

    colPtr->titleWidth = 2 * TITLE_PADX;
    colPtr->titleHeight = 2 * TITLE_PADY;

    iw = ih = 0;
    if (colPtr->titleIcon != NULL) {
	iw = TreeView_IconWidth(colPtr->titleIcon);
	ih = TreeView_IconHeight(colPtr->titleIcon);
	colPtr->titleWidth += iw;
    }
    tw = th = 0;
    if (colPtr->text != NULL) {
	TextStyle ts;

	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, colPtr->titleFont);
	Blt_Ts_GetExtents(&ts, colPtr->text,  &tw, &th);
	colPtr->textWidth = tw;
	colPtr->textHeight = th;
	colPtr->titleWidth += tw;
	if (iw > 0) {
	    colPtr->titleWidth += TITLE_PADX;
	}
    }
    if ((colPtr->sortUp != NULL) && (colPtr->sortDown != NULL)) {
	aw = MAX(TreeView_IconWidth(colPtr->sortUp), 
		 TreeView_IconWidth(colPtr->sortDown));
	ah = MAX(TreeView_IconHeight(colPtr->sortUp), 
		 TreeView_IconHeight(colPtr->sortDown));
    } else {
	aw = ah = 17;
    }
    colPtr->arrowWidth = aw;
    colPtr->arrowHeight = ah;
    colPtr->titleWidth += aw + TITLE_PADX;
    colPtr->titleHeight += MAX3(ih, th, ah);
    gcMask = (GCFunction | GCLineWidth | GCLineStyle | GCForeground);

    /* 
     * If the rule is active, turn it off (i.e. draw again to erase
     * it) before changing the GC.  If the color changes, we won't be
     * able to erase the old line, since it will no longer be
     * correctly XOR-ed with the background.
     */
    drawable = Tk_WindowId(viewPtr->tkwin);
    ruleDrawn = ((viewPtr->flags & RULE_ACTIVE_COLUMN) &&
		 (viewPtr->colActiveTitlePtr == colPtr) && 
		 (drawable != None));
    if (ruleDrawn) {
	DrawRule(viewPtr, colPtr, drawable);
    }
    /* XOR-ed rule column divider */ 
    gcValues.line_width = LineWidth(colPtr->ruleLineWidth);
    gcValues.foreground = GetStyleForeground(colPtr)->pixel;
    if (LineIsDashed(colPtr->ruleDashes)) {
	gcValues.line_style = LineOnOffDash;
    } else {
	gcValues.line_style = LineSolid;
    }
    gcValues.function = GXxor;

    bg = GetStyleBackground(colPtr);
    gcValues.foreground ^= Blt_Bg_BorderColor(bg)->pixel; 
    newGC = Blt_GetPrivateGC(viewPtr->tkwin, gcMask, &gcValues);
    if (LineIsDashed(colPtr->ruleDashes)) {
	Blt_SetDashes(viewPtr->display, newGC, &colPtr->ruleDashes);
    }
    if (colPtr->ruleGC != NULL) {
	Blt_FreePrivateGC(viewPtr->display, colPtr->ruleGC);
    }
    colPtr->ruleGC = newGC;
    if (ruleDrawn) {
	DrawRule(viewPtr, colPtr, drawable);
    }
    colPtr->flags |= COLUMN_DIRTY;
    viewPtr->flags |= UPDATE;
}

static void
FreeColumn(DestroyData data) 
{
    TreeView *viewPtr;
    Column *colPtr = (Column *)data;

    viewPtr = colPtr->viewPtr;
    if (colPtr != &viewPtr->treeColumn) {
	Blt_Free(colPtr);
    }
}

static void
DestroyColumn(Column *colPtr)
{
    Blt_HashEntry *hPtr;
    TreeView *viewPtr;

    colPtr->flags |= DELETED;		/* Mark the column as destroyed. */

    viewPtr = colPtr->viewPtr;
    uidOption.clientData = viewPtr;
    iconOption.clientData = viewPtr;
    styleOption.clientData = viewPtr;
    Blt_FreeOptions(columnSpecs, (char *)colPtr, viewPtr->display, 0);
    if (colPtr->titleGC != NULL) {
	Tk_FreeGC(viewPtr->display, colPtr->titleGC);
    }
    if (colPtr->ruleGC != NULL) {
	Blt_FreePrivateGC(viewPtr->display, colPtr->ruleGC);
    }
    hPtr = Blt_FindHashEntry(&viewPtr->columnTable, colPtr->key);
    if (hPtr != NULL) {
	Blt_DeleteHashEntry(&viewPtr->columnTable, hPtr);
    }
    if (colPtr->link != NULL) {
	Blt_Chain_DeleteLink(viewPtr->columns, colPtr->link);
    }
    if (colPtr == &viewPtr->treeColumn) {
	colPtr->link = NULL;
    } else {
	Tcl_EventuallyFree(colPtr, FreeColumn);
    }
}

static void
DestroyColumns(TreeView *viewPtr)
{
    if (viewPtr->columns != NULL) {
	Blt_ChainLink link;
	
	for (link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL;
	     link = Blt_Chain_NextLink(link)) {
	    Column *colPtr;

	    colPtr = Blt_Chain_GetValue(link);
	    colPtr->link = NULL;
	    colPtr->hashPtr = NULL;
	    DestroyColumn(colPtr);
	}
	Blt_Chain_Destroy(viewPtr->columns);
	viewPtr->columns = NULL;
    }
    Blt_DeleteHashTable(&viewPtr->columnTable);
}

static int
InitColumn(TreeView *viewPtr, Column *colPtr, const char *name, 
	   const char *defTitle)
{
    Blt_HashEntry *hPtr;
    int isNew;

    colPtr->key = Blt_Tree_GetKeyFromInterp(viewPtr->interp, name);
    colPtr->text = Blt_AssertStrdup(defTitle);
    colPtr->justify = TK_JUSTIFY_CENTER;
    colPtr->relief = TK_RELIEF_FLAT;
    colPtr->borderWidth = 1;
    colPtr->pad.side1 = colPtr->pad.side2 = 2;
    colPtr->state = STATE_NORMAL;
    colPtr->weight = 1.0;
    colPtr->ruleLineWidth = 1;
    colPtr->viewPtr = viewPtr;
    colPtr->titleBW = 2;
    colPtr->titleRelief = TK_RELIEF_RAISED;
    colPtr->titleIcon = NULL;
    colPtr->sortType = SORT_DICTIONARY;
    hPtr = Blt_CreateHashEntry(&viewPtr->columnTable, colPtr->key, &isNew);
    Blt_SetHashValue(hPtr, colPtr);
    colPtr->hashPtr = hPtr;

    uidOption.clientData = viewPtr;
    iconOption.clientData = viewPtr;
    styleOption.clientData = viewPtr;
    if (Blt_ConfigureComponentFromObj(viewPtr->interp, viewPtr->tkwin, name, 
	"Column", columnSpecs, 0, (Tcl_Obj **)NULL, (char *)colPtr, 0) 
	!= TCL_OK) {
	DestroyColumn(colPtr);
	return TCL_ERROR;
    }
    return TCL_OK;
}

static Column *
CreateColumn(TreeView *viewPtr, Tcl_Obj *nameObjPtr, int objc, 
	     Tcl_Obj *const *objv)
{
    Column *colPtr;

    colPtr = Blt_AssertCalloc(1, sizeof(Column));
    if (InitColumn(viewPtr, colPtr, Tcl_GetString(nameObjPtr),
	Tcl_GetString(nameObjPtr)) != TCL_OK) {
	return NULL;
    }
    uidOption.clientData = viewPtr;
    iconOption.clientData = viewPtr;
    styleOption.clientData = viewPtr;
    if (Blt_ConfigureComponentFromObj(viewPtr->interp, viewPtr->tkwin, 
	colPtr->key, "Column", columnSpecs, objc, objv, (char *)colPtr, 
	BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	DestroyColumn(colPtr);
	return NULL;
    }
    ConfigureColumn(viewPtr, colPtr);
    return colPtr;
}

static int
InvokeCompare(Column *colPtr, Entry *e1, Entry *e2, Tcl_Obj *cmdPtr)
{
    int result;
    Tcl_Obj *cmdObjPtr, *objPtr;
    TreeView *viewPtr;

    viewPtr = colPtr->viewPtr;
    cmdObjPtr = Tcl_DuplicateObj(cmdPtr);
    objPtr = Tcl_NewStringObj(Tk_PathName(viewPtr->tkwin), -1);
    Tcl_ListObjAppendElement(viewPtr->interp, cmdObjPtr, objPtr);
    objPtr = Tcl_NewLongObj(Blt_Tree_NodeId(e1->node));
    Tcl_ListObjAppendElement(viewPtr->interp, cmdObjPtr, objPtr);
    objPtr = Tcl_NewLongObj(Blt_Tree_NodeId(e2->node));
    Tcl_ListObjAppendElement(viewPtr->interp, cmdObjPtr, objPtr);
    objPtr = Tcl_NewStringObj(colPtr->key, -1);		
    Tcl_ListObjAppendElement(viewPtr->interp, cmdObjPtr, objPtr);
	     
    if (viewPtr->flatView) {
	objPtr = Tcl_NewStringObj(e1->fullName, -1);
	Tcl_ListObjAppendElement(viewPtr->interp, cmdObjPtr, objPtr);
	objPtr = Tcl_NewStringObj(e2->fullName, -1);
	Tcl_ListObjAppendElement(viewPtr->interp, cmdObjPtr, objPtr);
    } else {
	objPtr = Tcl_NewStringObj(GETLABEL(e1), -1);
	Tcl_ListObjAppendElement(viewPtr->interp, cmdObjPtr, objPtr);
	objPtr = Tcl_NewStringObj(GETLABEL(e2), -1);
	Tcl_ListObjAppendElement(viewPtr->interp, cmdObjPtr, objPtr);
    }
    Tcl_IncrRefCount(cmdObjPtr);
    result = Tcl_EvalObjEx(viewPtr->interp, cmdObjPtr, TCL_EVAL_GLOBAL);
    Tcl_DecrRefCount(cmdObjPtr);
    if ((result != TCL_OK) ||
	(Tcl_GetIntFromObj(viewPtr->interp, Tcl_GetObjResult(viewPtr->interp), 
		&result) != TCL_OK)) {
	Tcl_BackgroundError(viewPtr->interp);
    }
    Tcl_ResetResult(viewPtr->interp);
    return result;
}

static TreeView *treeViewInstance;

static int
CompareIntegers(const char *key, Entry *e1, Entry *e2)
{
    int i1, i2;
    Tcl_Obj *obj1, *obj2;

    if ((GetData(e1, key, &obj1) != TCL_OK) ||
	(Tcl_GetIntFromObj(NULL, obj1, &i1) != TCL_OK)) {
	obj1 = NULL;
    }
    if ((GetData(e2, key, &obj2) != TCL_OK) ||
	(Tcl_GetIntFromObj(NULL, obj2, &i2) != TCL_OK)) {
	obj2 = NULL;
    }
    if ((obj1 != NULL) && (obj2 != NULL)) {
	return i1 - i2;			/* Both A and B exist. */
    } 
    if (obj1 != NULL) {
	return 1;			/* A exists, B doesn't */
    } 
    if (obj2 != NULL) {
	return -1;			/* B exists, A doesn't */
    }
    return 0;				/* Both A and B don't exist. */
}

static int
CompareDoubles(const char *key, Entry *e1, Entry *e2)
{
    double d1, d2;
    Tcl_Obj *obj1, *obj2;

    if ((GetData(e1, key, &obj1) != TCL_OK) ||
	(Tcl_GetDoubleFromObj(NULL, obj1, &d1) != TCL_OK)) {
	obj1 = NULL;
    }
    if ((GetData(e2, key, &obj2) != TCL_OK) ||
	(Tcl_GetDoubleFromObj(NULL, obj2, &d2) != TCL_OK)) {
	obj2 = NULL;
    }
    if ((obj1 != NULL) && (obj2 != NULL)) {
	return (d1 > d2) ? 1 : (d1 < d2) ? -1 : 0;
    } 
    if (obj1 != NULL) {
	return 1;			/* A exists, B doesn't */
    } 
    if (obj2 != NULL) {
	return -1;			/* B exists, A doesn't */
    }
    return 0;				/* Both A and B don't exist. */
}

static int
CompareDictionaryStrings(const char *key, Entry *e1, Entry *e2)
{
    const char *s1, *s2;
    Tcl_Obj *obj1, *obj2;

    s1 = s2 = NULL;
    if (GetData(e1, key, &obj1) == TCL_OK) {
	s1 = Tcl_GetString(obj1);
    }
    if (GetData(e2, key, &obj2) == TCL_OK) {
	s2 = Tcl_GetString(obj2);
    }
    if ((s1 != NULL) && (s2 != NULL)) {
	return Blt_DictionaryCompare(s1, s2);
    } 
    if (s1 != NULL) {
	return 1;			/* A exists, B doesn't */
    } 
    if (s2 != NULL) {
	return -1;			/* B exists, A doesn't */
    }
    return 0;				/* Both A and B don't exist. */
}

static int
CompareAsciiStrings(const char *key, Entry *e1, Entry *e2)
{
    const char *s1, *s2;
    Tcl_Obj *obj1, *obj2;
    
    s1 = s2 = NULL;
    if (GetData(e1, key, &obj1) == TCL_OK) {
	s1 = Tcl_GetString(obj1);
    }
    if (GetData(e2, key, &obj2) == TCL_OK) {
	s2 = Tcl_GetString(obj2);
    }
    if ((s1 != NULL) && (s2 != NULL)) {
	return strcmp(s1, s2);
    } 
    if (s1 != NULL) {
	return 1;			/* A exists, B doesn't */
    } 
    if (s2 != NULL) {
	return -1;			/* B exists, A doesn't */
    }
    return 0;				/* Both A and B don't exist. */
}

static int
CompareEntries(const void *a, const void *b)
{
    Blt_ChainLink link;
    Entry *e1 = *(Entry **)a;
    Entry *e2 = *(Entry **)b;
    TreeView *viewPtr;
    int result;

    viewPtr = e1->viewPtr;
    if (e1->fullName == NULL) {
	Tcl_DString ds;
	
	Tcl_DStringInit(&ds);
	GetEntryPath(viewPtr, e1, TRUE, &ds);
	e1->fullName = Blt_AssertStrdup(Tcl_DStringValue(&ds));
	Tcl_DStringFree(&ds);
    }
    if (e2->fullName == NULL) {
	Tcl_DString ds;
	
	Tcl_DStringInit(&ds);
	GetEntryPath(viewPtr, e2, TRUE, &ds);
	e2->fullName = Blt_AssertStrdup(Tcl_DStringValue(&ds));
	Tcl_DStringFree(&ds);
    }
    result = 0;

    for (link = Blt_Chain_FirstLink(viewPtr->sortInfo.order); link != NULL;
	 link = Blt_Chain_NextLink(link)) {
	Column *colPtr;
	SortType sortType;
	Tcl_Obj *cmdObjPtr;

	colPtr = Blt_Chain_GetValue(link);
	sortType = colPtr->sortType;
	cmdObjPtr = NULL;
	if (sortType == SORT_COMMAND) {
	    /* Get the command for sorting. */
	    cmdObjPtr = colPtr->sortCmdObjPtr;
	    if (cmdObjPtr == NULL) {
		cmdObjPtr = viewPtr->sortInfo.cmdObjPtr;
	    }
	    if (cmdObjPtr == NULL) {
		sortType = SORT_DICTIONARY; /* If the command doesn't exist,
					     * revert to dictionary sort. */
	    }
	}
	if (colPtr == &viewPtr->treeColumn) {
	    /* Handle the tree view column specially. */
	    if (sortType == SORT_COMMAND) {
		result = InvokeCompare(colPtr, e1, e2, cmdObjPtr);
	    } else {
		const char *s1, *s2;

		if (viewPtr->flatView) {
		    s1 = e1->fullName;
		    s2 = e2->fullName;
		} else {
		    s1 = GETLABEL(e1);
		    s2 = GETLABEL(e2);
		} 
		if (sortType == SORT_ASCII) {
		    result = strcmp(s1, s2);
		} else {
		    result = Blt_DictionaryCompare(s1, s2);
		}
	    }
	} else {
	    switch (sortType) {
	    case SORT_ASCII:
		result = CompareAsciiStrings(colPtr->key, e1, e2);
		break;

	    case SORT_DICTIONARY:
		result = CompareDictionaryStrings(colPtr->key, e1, e2);
		break;

	    case SORT_INTEGER:
		result = CompareIntegers(colPtr->key, e1, e2);
		break;
	    
	    case SORT_REAL:
		result = CompareDoubles(colPtr->key, e1, e2);
		break;

	    case SORT_COMMAND:
		result = InvokeCompare(colPtr, e1, e2, cmdObjPtr);
		break;

	    default:
		fprintf(stderr, "col is %s sorttype=%d\n", colPtr->key,
			colPtr->sortType);
		abort();
	    }
	}
	if (result != 0) {
	    break;			/* Found difference */
	}
    }
    if (result == 0) {
	result = strcmp(e1->fullName, e2->fullName);
    }
    if (viewPtr->sortInfo.decreasing) {
	return -result;
    } 
    return result;
}


/*
 *---------------------------------------------------------------------------
 *
 * CompareNodes --
 *
 *	Comparison routine (used by qsort) to sort a chain of subnodes.
 *
 * Results:
 *	1 is the first is greater, -1 is the second is greater, 0
 *	if equal.
 *
 *---------------------------------------------------------------------------
 */
static int
CompareNodes(Blt_TreeNode *n1Ptr, Blt_TreeNode *n2Ptr)
{
    TreeView *viewPtr = treeViewInstance;
    Entry *e1, *e2;

    e1 = NodeToEntry(viewPtr, *n1Ptr);
    e2 = NodeToEntry(viewPtr, *n2Ptr);
    return CompareEntries(&e1, &e2);
}

/*
 *---------------------------------------------------------------------------
 *
 * SortApplyProc --
 *
 *	Sorts the subnodes at a given node.
 *
 * Results:
 *	Always returns TCL_OK.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SortApplyProc(Blt_TreeNode node, ClientData clientData, int order)
{
    TreeView *viewPtr = clientData;

    if (!Blt_Tree_IsLeaf(node)) {
	Blt_Tree_SortNode(viewPtr->tree, node, CompareNodes);
    }
    return TCL_OK;
}
 
/*
 *---------------------------------------------------------------------------
 *
 * SortFlatView --
 *
 *	Sorts the flatten array of entries.
 *
 *---------------------------------------------------------------------------
 */
static void
SortFlatView(TreeView *viewPtr)
{
    SortInfo *sortPtr;

    sortPtr = &viewPtr->sortInfo;
    viewPtr->flags &= ~SORT_PENDING;
    if (viewPtr->numEntries < 2) {
	return;
    }
    if (viewPtr->flags & SORTED) {
	int first, last;
	Entry *hold;

	if (sortPtr->decreasing == sortPtr->viewIsDecreasing){
	    return;
	}

	/* 
	 * The view is already sorted but in the wrong direction. 
	 * Reverse the entries in the array.
	 */
 	for (first = 0, last = viewPtr->numEntries - 1; last > first; 
	     first++, last--) {
	    hold = viewPtr->flatArr[first];
	    viewPtr->flatArr[first] = viewPtr->flatArr[last];
	    viewPtr->flatArr[last] = hold;
	}
	sortPtr->viewIsDecreasing = sortPtr->decreasing;
	viewPtr->flags |= SORTED | LAYOUT_PENDING;
	return;
    }
    qsort((char *)viewPtr->flatArr, viewPtr->numEntries, sizeof(Entry *),
	  (QSortCompareProc *)CompareEntries);

    sortPtr->viewIsDecreasing = sortPtr->decreasing;
    viewPtr->flags |= SORTED;
}

/*
 *---------------------------------------------------------------------------
 *
 * SortTreeView --
 *
 *	Sorts the tree array of entries.
 *
 *---------------------------------------------------------------------------
 */
static void
SortTreeView(TreeView *viewPtr)
{
    viewPtr->flags &= ~SORT_PENDING;
    treeViewInstance = viewPtr;
    Blt_Tree_Apply(viewPtr->rootPtr->node, SortApplyProc, viewPtr);
    viewPtr->sortInfo.viewIsDecreasing = viewPtr->sortInfo.decreasing;
}

/*
 * TreeView Procedures
 */

/*
 *---------------------------------------------------------------------------
 *
 * CreateTreeView --
 *
 *---------------------------------------------------------------------------
 */
static TreeView *
CreateTreeView(Tcl_Interp *interp, Tcl_Obj *objPtr)
{
    Tk_Window tkwin;
    TreeView *viewPtr;
    char *name;
    int result;

    name = Tcl_GetString(objPtr);
    tkwin = Tk_CreateWindowFromPath(interp, Tk_MainWindow(interp), name,
	(char *)NULL);
    if (tkwin == NULL) {
	return NULL;

    }
    Tk_SetClass(tkwin, "BltTreeView");

    viewPtr = Blt_AssertCalloc(1, sizeof(TreeView));
    viewPtr->tkwin = tkwin;
    viewPtr->display = Tk_Display(tkwin);
    viewPtr->interp = interp;
    viewPtr->flags = (HIDE_ROOT | SHOW_COLUMN_TITLES | DIRTY | 
		      LAYOUT_PENDING | REPOPULATE);
    viewPtr->leader = 0;
    viewPtr->dashes = 1;
    viewPtr->highlightWidth = 2;
    viewPtr->borderWidth = 2;
    viewPtr->relief = TK_RELIEF_SUNKEN;
    viewPtr->scrollMode = BLT_SCROLL_MODE_HIERBOX;
    viewPtr->button.closeRelief = viewPtr->button.openRelief = TK_RELIEF_SOLID;
    viewPtr->reqWidth = 0;
    viewPtr->reqHeight = 0;
    viewPtr->xScrollUnits = viewPtr->yScrollUnits = 20;
    viewPtr->lineWidth = 1;
    viewPtr->button.borderWidth = 1;
    viewPtr->columns = Blt_Chain_Create();
    viewPtr->buttonFlags = BUTTON_AUTO;
    viewPtr->userStyles = Blt_Chain_Create();
    viewPtr->sortInfo.markPtr = NULL;
    viewPtr->selection.borderWidth = 1;
    viewPtr->selection.relief = TK_RELIEF_FLAT;
    viewPtr->selection.mode = SELECT_MODE_SINGLE;
    viewPtr->selection.list = Blt_Chain_Create();
    Blt_InitHashTable(&viewPtr->selection.table, BLT_ONE_WORD_KEYS);
    Blt_InitHashTableWithPool(&viewPtr->entryTable, BLT_ONE_WORD_KEYS);
    Blt_InitHashTable(&viewPtr->columnTable, BLT_ONE_WORD_KEYS);
    Blt_InitHashTable(&viewPtr->iconTable, BLT_STRING_KEYS);
    Blt_InitHashTable(&viewPtr->uidTable, BLT_STRING_KEYS);
    Blt_InitHashTable(&viewPtr->styleTable, BLT_STRING_KEYS);
    viewPtr->bindTable = Blt_CreateBindingTable(interp, tkwin, viewPtr, 
	PickItem, AppendTagsProc);
    Blt_InitHashTable(&viewPtr->entryTagTable, BLT_STRING_KEYS);
    Blt_InitHashTable(&viewPtr->columnTagTable, BLT_STRING_KEYS);
    Blt_InitHashTable(&viewPtr->buttonTagTable, BLT_STRING_KEYS);
    Blt_InitHashTable(&viewPtr->styleTagTable, BLT_STRING_KEYS);

    viewPtr->entryPool = Blt_Pool_Create(BLT_FIXED_SIZE_ITEMS);
    viewPtr->valuePool = Blt_Pool_Create(BLT_FIXED_SIZE_ITEMS);
    Blt_SetWindowInstanceData(tkwin, viewPtr);
    viewPtr->cmdToken = Tcl_CreateObjCommand(interp,Tk_PathName(viewPtr->tkwin),
	TreeViewInstCmdProc, viewPtr, TreeViewInstCmdDeleteProc);

    Tk_CreateSelHandler(viewPtr->tkwin, XA_PRIMARY, XA_STRING, SelectionProc,
	viewPtr, XA_STRING);
    Tk_CreateEventHandler(viewPtr->tkwin, ExposureMask | StructureNotifyMask |
	FocusChangeMask, TreeViewEventProc, viewPtr);
    /* 
     * Create a default style. This must exist before we can create the
     * treeview column.
     */  
    viewPtr->stylePtr = CreateStyle(interp, viewPtr, STYLE_TEXTBOX, "text", 
	0, (Tcl_Obj **)NULL);
    if (viewPtr->stylePtr == NULL) {
	return NULL;
    }
    /*
     * By default create a tree. The name will be the same as the widget
     * pathname.
     */
    viewPtr->tree = Blt_Tree_Open(interp, Tk_PathName(viewPtr->tkwin), 
	TREE_CREATE);
    if (viewPtr->tree == NULL) {
	return NULL;
    }
    /* Create a default column to display the view of the tree. */
    result = InitColumn(viewPtr, &viewPtr->treeColumn, "treeView", "");
    if (result != TCL_OK) {
	return NULL;
    }
    Blt_Chain_Append(viewPtr->columns, &viewPtr->treeColumn);
    return viewPtr;
}

static void
TeardownEntries(TreeView *viewPtr)
{
    Blt_HashSearch iter;
    Blt_HashEntry *hPtr;

    /* Release the current tree, removing any entry fields. */
    for (hPtr = Blt_FirstHashEntry(&viewPtr->entryTable, &iter); hPtr != NULL; 
	 hPtr = Blt_NextHashEntry(&iter)) {
	Entry *entryPtr;

	entryPtr = Blt_GetHashValue(hPtr);
	entryPtr->hashPtr = NULL;
	DestroyEntry(entryPtr);
    }
    Blt_DeleteHashTable(&viewPtr->entryTable);
}


/*
 *---------------------------------------------------------------------------
 *
 * DestroyTreeView --
 *
 * 	This procedure is invoked by Tcl_EventuallyFree or Tcl_Release to
 * 	clean up the internal structure of a TreeView at a safe time (when
 * 	no-one is using it anymore).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Everything associated with the widget is freed up.
 *
 *---------------------------------------------------------------------------
 */
static void
DestroyTreeView(DestroyData dataPtr)	/* Pointer to the widget record. */
{
    Blt_ChainLink link;
    TreeView *viewPtr = (TreeView *)dataPtr;
    Button *buttonPtr;
    ColumnStyle *stylePtr;

    TeardownEntries(viewPtr);
    if (viewPtr->tree != NULL) {
	Blt_Tree_Close(viewPtr->tree);
	viewPtr->tree = NULL;
    }
    treeOption.clientData = viewPtr;
    iconsOption.clientData = viewPtr;
    Blt_FreeOptions(viewSpecs, (char *)viewPtr, viewPtr->display, 0);
    Blt_FreeOptions(sortSpecs, (char *)viewPtr, viewPtr->display, 0);
    if (viewPtr->tkwin != NULL) {
	Tk_DeleteSelHandler(viewPtr->tkwin, XA_PRIMARY, XA_STRING);
    }
    if (viewPtr->lineGC != NULL) {
	Tk_FreeGC(viewPtr->display, viewPtr->lineGC);
    }
    if (viewPtr->focusGC != NULL) {
	Blt_FreePrivateGC(viewPtr->display, viewPtr->focusGC);
    }
    if (viewPtr->selection.gc != NULL) {
	Blt_FreePrivateGC(viewPtr->display, viewPtr->selection.gc);
    }
    if (viewPtr->visibleArr != NULL) {
	Blt_Free(viewPtr->visibleArr);
    }
    if (viewPtr->flatArr != NULL) {
	Blt_Free(viewPtr->flatArr);
    }
    if (viewPtr->levelInfo != NULL) {
	Blt_Free(viewPtr->levelInfo);
    }
    buttonPtr = &viewPtr->button;
    if (buttonPtr->activeGC != NULL) {
	Tk_FreeGC(viewPtr->display, buttonPtr->activeGC);
    }
    if (buttonPtr->normalGC != NULL) {
	Tk_FreeGC(viewPtr->display, buttonPtr->normalGC);
    }
    if (viewPtr->stylePtr != NULL) {
	FreeStyle(viewPtr->stylePtr);
    }
    DestroyColumns(viewPtr);
    Blt_DestroyBindingTable(viewPtr->bindTable);
    Blt_Chain_Destroy(viewPtr->selection.list);
    Blt_DeleteHashTable(&viewPtr->entryTagTable);
    Blt_DeleteHashTable(&viewPtr->columnTagTable);
    Blt_DeleteHashTable(&viewPtr->buttonTagTable);
    Blt_DeleteHashTable(&viewPtr->styleTagTable);

    /* Remove any user-specified style that might remain. */
    for (link = Blt_Chain_FirstLink(viewPtr->userStyles); link != NULL;
	 link = Blt_Chain_NextLink(link)) {
	stylePtr = Blt_Chain_GetValue(link);
	stylePtr->link = NULL;
	FreeStyle(stylePtr);
    }
    Blt_Chain_Destroy(viewPtr->userStyles);
    if (viewPtr->comboWin != NULL) {
	Tk_DestroyWindow(viewPtr->comboWin);
    }
    Blt_DeleteHashTable(&viewPtr->styleTable);
    Blt_DeleteHashTable(&viewPtr->selection.table);
    Blt_DeleteHashTable(&viewPtr->uidTable);
    Blt_Pool_Destroy(viewPtr->entryPool);
    Blt_Pool_Destroy(viewPtr->valuePool);
    DumpIconTable(viewPtr);
    Blt_Free(viewPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * TreeViewEventProc --
 *
 * 	This procedure is invoked by the Tk dispatcher for various events on
 * 	treeview widgets.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	When the window gets deleted, internal structures get cleaned up.
 *	When it gets exposed, it is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static void
TreeViewEventProc(ClientData clientData, XEvent *eventPtr)
{
    TreeView *viewPtr = clientData;

    if (eventPtr->type == Expose) {
	if (eventPtr->xexpose.count == 0) {
	    EventuallyRedraw(viewPtr);
	    Blt_PickCurrentItem(viewPtr->bindTable);
	}
    } else if (eventPtr->type == ConfigureNotify) {
	viewPtr->flags |= (LAYOUT_PENDING | SCROLL_PENDING);
	EventuallyRedraw(viewPtr);
    } else if ((eventPtr->type == FocusIn) || (eventPtr->type == FocusOut)) {
	if (eventPtr->xfocus.detail != NotifyInferior) {
	    if (eventPtr->type == FocusIn) {
		viewPtr->flags |= FOCUS;
	    } else {
		viewPtr->flags &= ~FOCUS;
	    }
	    EventuallyRedraw(viewPtr);
	}
    } else if (eventPtr->type == DestroyNotify) {
	if (viewPtr->tkwin != NULL) {
	    viewPtr->tkwin = NULL;
	    Tcl_DeleteCommandFromToken(viewPtr->interp, viewPtr->cmdToken);
	}
	if (viewPtr->flags & REDRAW_PENDING) {
	    Tcl_CancelIdleCall(DisplayTreeView, viewPtr);
	}
	if (viewPtr->selection.flags & SELECT_PENDING) {
	    Tcl_CancelIdleCall(SelectCmdProc, viewPtr);
	}
	Tcl_EventuallyFree(viewPtr, DestroyTreeView);
    }
}

/* Selection Procedures */
/*
 *---------------------------------------------------------------------------
 *
 * SelectionProc --
 *
 *	This procedure is called back by Tk when the selection is requested by
 *	someone.  It returns part or all of the selection in a buffer provided
 *	by the caller.
 *
 * Results:
 *	The return value is the number of non-NULL bytes stored at buffer.
 *	Buffer is filled (or partially filled) with a NUL-terminated string
 *	containing part or all of the selection, as given by offset and
 *	maxBytes.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static int
SelectionProc(
    ClientData clientData,		/* Information about the widget. */
    int offset,				/* Offset within selection of first
					 * character to be returned. */
    char *buffer,			/* Location in which to place
					 * selection. */
    int maxBytes)			/* Maximum number of bytes to place at
					 * buffer, not including terminating
					 * NULL character. */
{
    Tcl_DString ds;
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    int size;

    if ((viewPtr->selection.flags & SELECT_EXPORT) == 0) {
	return -1;
    }
    /*
     * Retrieve the names of the selected entries.
     */
    Tcl_DStringInit(&ds);
    if (viewPtr->selection.flags & SELECT_SORTED) {
	Blt_ChainLink link;

	for (link = Blt_Chain_FirstLink(viewPtr->selection.list); 
	     link != NULL; link = Blt_Chain_NextLink(link)) {
	    entryPtr = Blt_Chain_GetValue(link);
	    Tcl_DStringAppend(&ds, GETLABEL(entryPtr), -1);
	    Tcl_DStringAppend(&ds, "\n", -1);
	}
    } else {
	for (entryPtr = viewPtr->rootPtr; entryPtr != NULL; 
	     entryPtr = NextEntry(entryPtr, ENTRY_MASK)) {
	    if (EntryIsSelected(viewPtr, entryPtr)) {
		Tcl_DStringAppend(&ds, GETLABEL(entryPtr), -1);
		Tcl_DStringAppend(&ds, "\n", -1);
	    }
	}
    }
    size = Tcl_DStringLength(&ds) - offset;
    strncpy(buffer, Tcl_DStringValue(&ds) + offset, maxBytes);
    Tcl_DStringFree(&ds);
    buffer[maxBytes] = '\0';
    return (size > maxBytes) ? maxBytes : size;
}

/*
 *---------------------------------------------------------------------------
 *
 * TreeViewInstCmdDeleteProc --
 *
 *	This procedure is invoked when a widget command is deleted.  If the
 *	widget isn't already in the process of being destroyed, this command
 *	destroys it.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The widget is destroyed.
 *
 *---------------------------------------------------------------------------
 */
static void
TreeViewInstCmdDeleteProc(ClientData clientData)
{
    TreeView *viewPtr = clientData;

    /*
     * This procedure could be invoked either because the window was destroyed
     * and the command was then deleted (in which case tkwin is NULL) or
     * because the command was deleted, and then this procedure destroys the
     * widget.
     */
    if (viewPtr->tkwin != NULL) {
	Tk_Window tkwin;

	tkwin = viewPtr->tkwin;
	viewPtr->tkwin = NULL;
	Tk_DestroyWindow(tkwin);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureTreeView --
 *
 *	Updates the GCs and other information associated with the treeview
 *	widget.
 *
 * Results:
 *	The return value is a standard TCL result.  If TCL_ERROR is returned,
 *	then interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font, etc. get
 *	set for viewPtr; old resources get freed, if there were any.  The widget
 *	is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static int
ConfigureTreeView(Tcl_Interp *interp, TreeView *viewPtr)	
{
    GC newGC;
    XGCValues gcValues;
    unsigned long gcMask;

    /*
     * GC for dotted vertical line.
     */
    gcMask = (GCForeground | GCLineWidth);
    gcValues.foreground = viewPtr->lineColor->pixel;
    gcValues.line_width = viewPtr->lineWidth;
    if (viewPtr->dashes > 0) {
	gcMask |= (GCLineStyle | GCDashList);
	gcValues.line_style = LineOnOffDash;
	gcValues.dashes = viewPtr->dashes;
    }
    newGC = Tk_GetGC(viewPtr->tkwin, gcMask, &gcValues);
    if (viewPtr->lineGC != NULL) {
	Tk_FreeGC(viewPtr->display, viewPtr->lineGC);
    }
    viewPtr->lineGC = newGC;

    /*
     * GC for active label. Dashed outline.
     */
    gcMask = GCForeground | GCLineStyle;
    gcValues.foreground = viewPtr->focusColor->pixel;
    gcValues.line_style = (LineIsDashed(viewPtr->focusDashes))
	? LineOnOffDash : LineSolid;
    newGC = Blt_GetPrivateGC(viewPtr->tkwin, gcMask, &gcValues);
    if (LineIsDashed(viewPtr->focusDashes)) {
	viewPtr->focusDashes.offset = 2;
	Blt_SetDashes(viewPtr->display, newGC, &viewPtr->focusDashes);
    }
    if (viewPtr->focusGC != NULL) {
	Blt_FreePrivateGC(viewPtr->display, viewPtr->focusGC);
    }
    viewPtr->focusGC = newGC;

    /*
     * GC for selection. Dashed outline.
     */
    gcMask = GCForeground | GCLineStyle;
    gcValues.foreground = viewPtr->selection.fgColor->pixel;
    gcValues.line_style = (LineIsDashed(viewPtr->focusDashes))
	? LineOnOffDash : LineSolid;
    newGC = Blt_GetPrivateGC(viewPtr->tkwin, gcMask, &gcValues);
    if (LineIsDashed(viewPtr->focusDashes)) {
	viewPtr->focusDashes.offset = 2;
	Blt_SetDashes(viewPtr->display, newGC, &viewPtr->focusDashes);
    }
    if (viewPtr->selection.gc != NULL) {
	Blt_FreePrivateGC(viewPtr->display, viewPtr->selection.gc);
    }
    viewPtr->selection.gc = newGC;

    ConfigureButtons(viewPtr);
    viewPtr->inset = viewPtr->highlightWidth + viewPtr->borderWidth + INSET_PAD;

    /*
     * If the tree object was changed, we need to setup the new one.
     */
    if (Blt_ConfigModified(viewSpecs, "-tree", (char *)NULL)) {
	TeardownEntries(viewPtr);
	Blt_InitHashTableWithPool(&viewPtr->entryTable, BLT_ONE_WORD_KEYS);
	ClearSelection(viewPtr);
	if (Blt_Tree_Attach(interp, viewPtr->tree, viewPtr->treeName) 
	    != TCL_OK) {
	    return TCL_ERROR;
	}
	viewPtr->flags |= REPOPULATE;
    }

    /*
     * These options change the layout of the box.  Mark the widget for update.
     */
    if (Blt_ConfigModified(viewSpecs, "-font", "-linespacing", "-*width", 
	"-height", "-hide*", "-tree", "-flat", (char *)NULL)) {
	viewPtr->flags |= (LAYOUT_PENDING | SCROLL_PENDING);
    }
    /*
     * If the tree view was changed, mark all the nodes dirty (we'll be
     * switching back to either the full path name or the label) and free the
     * array representing the flattened view of the tree.
     */
    if (Blt_ConfigModified(viewSpecs, "-hideleaves", "-flat", (char *)NULL)) {
	Entry *entryPtr;
	
	viewPtr->flags |= DIRTY;
	/* Mark all entries dirty. */
	for (entryPtr = viewPtr->rootPtr; entryPtr != NULL; 
	     entryPtr = NextEntry(entryPtr, 0)) {
	    entryPtr->flags |= ENTRY_DIRTY;
	}
	if ((!viewPtr->flatView) && (viewPtr->flatArr != NULL)) {
	    Blt_Free(viewPtr->flatArr);
	    viewPtr->flatArr = NULL;
	}
    }

    if (viewPtr->flags & REPOPULATE) {
	Blt_TreeNode root;

	Blt_Tree_CreateEventHandler(viewPtr->tree, TREE_NOTIFY_ALL, 
		TreeEventProc, viewPtr);
	TraceColumns(viewPtr);
	root = Blt_Tree_RootNode(viewPtr->tree);

	/* Automatically add view-entry values to the new tree. */
	Blt_Tree_Apply(root, CreateApplyProc, viewPtr);
	viewPtr->focusPtr = viewPtr->rootPtr = 
	    NodeToEntry(viewPtr,root);
	viewPtr->selection.markPtr = viewPtr->selection.anchorPtr = NULL;
	Blt_SetFocusItem(viewPtr->bindTable, viewPtr->rootPtr, ITEM_ENTRY);

	/* Automatically open the root node. */
	if (OpenEntry(viewPtr, viewPtr->rootPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (viewPtr->flags & TV_NEW_TAGS) {
	    Blt_Tree_NewTagTable(viewPtr->tree);
	}
	viewPtr->flags &= ~REPOPULATE;
    }

    if (Blt_ConfigModified(viewSpecs, "-font", "-color", (char *)NULL)) {
	ConfigureColumn(viewPtr, &viewPtr->treeColumn);
    }
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

static void
ConfigureStyle(TreeView *viewPtr, ColumnStyle *stylePtr)
{
    (*stylePtr->classPtr->configProc)(stylePtr);
    stylePtr->flags |= STYLE_DIRTY;
    EventuallyRedraw(viewPtr);
}


/*
 *---------------------------------------------------------------------------
 *
 * ResetCoordinates --
 *
 *	Determines the maximum height of all visible entries.
 *
 *	1. Sets the worldY coordinate for all mapped/open entries.
 *	2. Determines if entry needs a button.
 *	3. Collects the minimum height of open/mapped entries. (Do for all
 *	   entries upon insert).
 *	4. Figures out horizontal extent of each entry (will be width of 
 *	   tree view column).
 *	5. Collects maximum icon size for each level.
 *	6. The height of its vertical line
 *
 * Results:
 *	Returns 1 if beyond the last visible entry, 0 otherwise.
 *
 * Side effects:
 *	The array of visible nodes is filled.
 *
 *---------------------------------------------------------------------------
 */
static void
ResetCoordinates(TreeView *viewPtr, Entry *entryPtr, int *yPtr, 
		 int *indexPtr)
{
    int depth, height;

    entryPtr->worldY = -1;
    entryPtr->vertLineLength = -1;
    entryPtr->worldY = -1;
    if ((entryPtr != viewPtr->rootPtr) && (EntryIsHidden(entryPtr))) {
	return;				/* If the entry is hidden, then do
					 * nothing. */
    }
    entryPtr->worldY = *yPtr;
    height = MAX3(entryPtr->lineHeight, entryPtr->iconHeight, 
		  viewPtr->button.height);
    entryPtr->vertLineLength = -(*yPtr + height / 2);
    *yPtr += entryPtr->height;
    entryPtr->flatIndex = *indexPtr;
    (*indexPtr)++;
    depth = DEPTH(viewPtr, entryPtr->node) + 1;
    if (viewPtr->levelInfo[depth].labelWidth < entryPtr->labelWidth) {
	viewPtr->levelInfo[depth].labelWidth = entryPtr->labelWidth;
    }
    if (viewPtr->levelInfo[depth].iconWidth < entryPtr->iconWidth) {
	viewPtr->levelInfo[depth].iconWidth = entryPtr->iconWidth;
    }
    viewPtr->levelInfo[depth].iconWidth |= 0x01;
    if ((entryPtr->flags & ENTRY_CLOSED) == 0) {
	Entry *bottomPtr, *childPtr;

	bottomPtr = entryPtr;
	for (childPtr = FirstChild(entryPtr, ENTRY_HIDE); childPtr != NULL; 
	     childPtr = NextSibling(childPtr, ENTRY_HIDE)){
	    ResetCoordinates(viewPtr, childPtr, yPtr, indexPtr);
	    bottomPtr = childPtr;
	}
	height = MAX3(bottomPtr->lineHeight, bottomPtr->iconHeight, 
		      viewPtr->button.height);
	entryPtr->vertLineLength += bottomPtr->worldY + height / 2;
    }
}

static void
AdjustColumns(TreeView *viewPtr)
{
    Blt_ChainLink link;
    Column *lastPtr;
    double weight;
    int growth;
    int numOpen;

    growth = VPORTWIDTH(viewPtr) - viewPtr->worldWidth;
    lastPtr = NULL;
    numOpen = 0;
    weight = 0.0;
    /* Find out how many columns still have space available */
    for (link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL;
	 link = Blt_Chain_NextLink(link)) {
	Column *colPtr;

	colPtr = Blt_Chain_GetValue(link);
	if (colPtr->flags & COLUMN_HIDDEN) {
	    continue;
	}
	lastPtr = colPtr;
	if ((colPtr->weight == 0.0) || (colPtr->width >= colPtr->max) || 
	    (colPtr->reqWidth > 0)) {
	    continue;
	}
	numOpen++;
	weight += colPtr->weight;
    }

    while ((numOpen > 0) && (weight > 0.0) && (growth > 0)) {
	int ration;

	ration = (int)(growth / weight);
	if (ration == 0) {
	    ration = 1;
	}
	for (link = Blt_Chain_FirstLink(viewPtr->columns); 
	     link != NULL; link = Blt_Chain_NextLink(link)) {
	    Column *colPtr;
	    int size, avail;

	    colPtr = Blt_Chain_GetValue(link);
	    if (colPtr->flags & COLUMN_HIDDEN) {
		continue;
	    }
	    lastPtr = colPtr;
	    if ((colPtr->weight == 0.0) || (colPtr->width >= colPtr->max) || 
		(colPtr->reqWidth > 0)) {
		continue;
	    }
	    size = (int)(ration * colPtr->weight);
	    if (size > growth) {
		size = growth; 
	    }
	    avail = colPtr->max - colPtr->width;
	    if (size > avail) {
		size = avail;
		numOpen--;
		weight -= colPtr->weight;
	    }
	    colPtr->width += size;
	    growth -= size;
	}
    }
    if ((growth > 0) && (lastPtr != NULL)) {
	lastPtr->width += growth;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ComputeFlatLayout --
 *
 *	Recompute the layout when entries are opened/closed, inserted/deleted,
 *	or when text attributes change (such as font, linespacing).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The world coordinates are set for all the opened entries.
 *
 *---------------------------------------------------------------------------
 */
static void
ComputeFlatLayout(TreeView *viewPtr)
{
    Blt_ChainLink link;
    Column *colPtr;
    Entry **p;
    Entry *entryPtr;
    int count;
    int maxX;
    int y;

    /* 
     * Pass 1:	Reinitialize column sizes and loop through all nodes. 
     *
     *		1. Recalculate the size of each entry as needed. 
     *		2. The maximum depth of the tree. 
     *		3. Minimum height of an entry.  Dividing this by the
     *		   height of the widget gives a rough estimate of the 
     *		   maximum number of visible entries.
     *		4. Build an array to hold level information to be filled
     *		   in on pass 2.
     */
    if (viewPtr->flags & (DIRTY | UPDATE)) {
	long index;

	/* Reset the positions of all the columns and initialize the column
	 * used to track the widest value. */
	index = 0;
	for (link = Blt_Chain_FirstLink(viewPtr->columns); 
	     link != NULL; link = Blt_Chain_NextLink(link)) {
	    colPtr = Blt_Chain_GetValue(link);
	    colPtr->maxWidth = 0;
	    colPtr->max = SHRT_MAX;
	    if (colPtr->reqMax > 0) {
		colPtr->max = colPtr->reqMax;
	    }
	    colPtr->index = index;
	    index++;
	}

	/* If the view needs to be resorted, free the old view. */
	if ((viewPtr->flags & (DIRTY|RESORT|SORT_PENDING|TV_SORT_AUTO)) && 
	     (viewPtr->flatArr != NULL)) {
	    Blt_Free(viewPtr->flatArr);
	    viewPtr->flatArr = NULL;
	}

	/* Recreate the flat view of all the open and not-hidden entries. */
	if (viewPtr->flatArr == NULL) {
	    count = 0;
	    /* Count the number of open entries to allocate for the array. */
	    for (entryPtr = viewPtr->rootPtr; entryPtr != NULL; 
		entryPtr = NextEntry(entryPtr, ENTRY_MASK)) {
		if ((viewPtr->flags & HIDE_ROOT) && 
		    (entryPtr == viewPtr->rootPtr)) {
		    continue;
		}
		count++;
	    }
	    viewPtr->numEntries = count;

	    /* Allocate an array for the flat view. */
	    viewPtr->flatArr = Blt_AssertCalloc((count + 1), 
		sizeof(Entry *));
	    /* Fill the array with open and not-hidden entries */
	    p = viewPtr->flatArr;
	    for (entryPtr = viewPtr->rootPtr; entryPtr != NULL; 
		entryPtr = NextEntry(entryPtr, ENTRY_MASK)) {
		if ((viewPtr->flags & HIDE_ROOT) && 
		    (entryPtr == viewPtr->rootPtr)) {
		    continue;
		}
		*p++ = entryPtr;
	    }
	    *p = NULL;
	    viewPtr->flags &= ~SORTED;		/* Indicate the view isn't
						 * sorted. */
	}

	/* Collect the extents of the entries in the flat view. */
	viewPtr->depth = 0;
	viewPtr->minHeight = SHRT_MAX;
	for (p = viewPtr->flatArr; *p != NULL; p++) {
	    entryPtr = *p;
	    GetEntryExtents(viewPtr, entryPtr);
	    if (viewPtr->minHeight > entryPtr->height) {
		viewPtr->minHeight = entryPtr->height;
	    }
	    entryPtr->flags &= ~ENTRY_HAS_BUTTON;
	}
	if (viewPtr->levelInfo != NULL) {
	    Blt_Free(viewPtr->levelInfo);
	}
	viewPtr->levelInfo = 
	    Blt_AssertCalloc(viewPtr->depth+2, sizeof(LevelInfo));
	viewPtr->flags &= ~(DIRTY | UPDATE | RESORT);
	if (viewPtr->flags & TV_SORT_AUTO) {
	    /* If we're auto-sorting, schedule the view to be resorted. */
	    viewPtr->flags |= SORT_PENDING;
	}
    } 

    if (viewPtr->flags & SORT_PENDING) {
	SortFlatView(viewPtr);
    }

    viewPtr->levelInfo[0].labelWidth = viewPtr->levelInfo[0].x = 
	    viewPtr->levelInfo[0].iconWidth = 0;
    /* 
     * Pass 2:	Loop through all open/mapped nodes. 
     *
     *		1. Set world y-coordinates for entries. We must defer
     *		   setting the x-coordinates until we know the maximum 
     *		   icon sizes at each level.
     *		2. Compute the maximum depth of the tree. 
     *		3. Build an array to hold level information.
     */
    y = 0;			
    count = 0;
    for(p = viewPtr->flatArr; *p != NULL; p++) {
	entryPtr = *p;
	entryPtr->flatIndex = count++;
	entryPtr->worldY = y;
	entryPtr->vertLineLength = 0;
	y += entryPtr->height;
	if (viewPtr->levelInfo[0].labelWidth < entryPtr->labelWidth) {
	    viewPtr->levelInfo[0].labelWidth = entryPtr->labelWidth;
	}
	if (viewPtr->levelInfo[0].iconWidth < entryPtr->iconWidth) {
	    viewPtr->levelInfo[0].iconWidth = entryPtr->iconWidth;
	}
    }
    viewPtr->levelInfo[0].iconWidth |= 0x01;
    viewPtr->worldHeight = y;		/* Set the scroll height of the
					 * hierarchy. */
    if (viewPtr->worldHeight < 1) {
	viewPtr->worldHeight = 1;
    }
    maxX = viewPtr->levelInfo[0].iconWidth + viewPtr->levelInfo[0].labelWidth;
    viewPtr->treeColumn.maxWidth = maxX;
    viewPtr->treeWidth = maxX;
    viewPtr->flags |= VIEWPORT;
}

/*
 *---------------------------------------------------------------------------
 *
 * ComputeTreeLayout --
 *
 *	Recompute the layout when entries are opened/closed, inserted/deleted,
 *	or when text attributes change (such as font, linespacing).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The world coordinates are set for all the opened entries.
 *
 *---------------------------------------------------------------------------
 */
static void
ComputeTreeLayout(TreeView *viewPtr)
{
    int y;
    int index;
    /* 
     * Pass 1:	Reinitialize column sizes and loop through all nodes. 
     *
     *		1. Recalculate the size of each entry as needed. 
     *		2. The maximum depth of the tree. 
     *		3. Minimum height of an entry.  Dividing this by the
     *		   height of the widget gives a rough estimate of the 
     *		   maximum number of visible entries.
     *		4. Build an array to hold level information to be filled
     *		   in on pass 2.
     */
    if (viewPtr->flags & DIRTY) {
	Blt_ChainLink link;
	Entry *ep;
	long index;

	index = 0;
	for (link = Blt_Chain_FirstLink(viewPtr->columns); 
	     link != NULL; link = Blt_Chain_NextLink(link)) {
	    Column *colPtr;

	    colPtr = Blt_Chain_GetValue(link);
	    colPtr->maxWidth = 0;
	    colPtr->max = SHRT_MAX;
	    if (colPtr->reqMax > 0) {
		colPtr->max = colPtr->reqMax;
	    }
	    colPtr->index = index;
	    index++;
	}
	viewPtr->minHeight = SHRT_MAX;
	viewPtr->depth = 0;
	for (ep = viewPtr->rootPtr; ep != NULL; 
	     ep = NextEntry(ep, 0)){
	    GetEntryExtents(viewPtr, ep);
	    if (viewPtr->minHeight > ep->height) {
		viewPtr->minHeight = ep->height;
	    }
	    /* 
	     * Determine if the entry should display a button (indicating that
	     * it has children) and mark the entry accordingly.
	     */
	    ep->flags &= ~ENTRY_HAS_BUTTON;
	    if (ep->flags & BUTTON_SHOW) {
		ep->flags |= ENTRY_HAS_BUTTON;
	    } else if (ep->flags & BUTTON_AUTO) {
		if (FirstChild(ep, ENTRY_HIDE) != NULL) {
		    ep->flags |= ENTRY_HAS_BUTTON;
		}
	    }
	    /* Determine the depth of the tree. */
	    if (viewPtr->depth < DEPTH(viewPtr, ep->node)) {
		viewPtr->depth = DEPTH(viewPtr, ep->node);
	    }
	}
	if (viewPtr->levelInfo != NULL) {
	    Blt_Free(viewPtr->levelInfo);
	}
	viewPtr->levelInfo = Blt_AssertCalloc(viewPtr->depth+2, 
					      sizeof(LevelInfo));
	viewPtr->flags &= ~(DIRTY | RESORT);
    }
    if (viewPtr->flags & (TV_SORT_AUTO | SORT_PENDING)) {
	SortTreeView(viewPtr);
    }
    { 
	size_t i;

	for (i = 0; i <= (viewPtr->depth + 1); i++) {
	    viewPtr->levelInfo[i].labelWidth = viewPtr->levelInfo[i].x = 
		viewPtr->levelInfo[i].iconWidth = 0;
	}
    }
    /* 
     * Pass 2:	Loop through all open/mapped nodes. 
     *
     *		1. Set world y-coordinates for entries. We must defer
     *		   setting the x-coordinates until we know the maximum 
     *		   icon sizes at each level.
     *		2. Compute the maximum depth of the tree. 
     *		3. Build an array to hold level information.
     */
    y = 0;
    if (viewPtr->flags & HIDE_ROOT) {
	/* If the root entry is to be hidden, cheat by offsetting the
	 * y-coordinates by the height of the entry. */
	y = -(viewPtr->rootPtr->height);
    } 
    index = 0;
    ResetCoordinates(viewPtr, viewPtr->rootPtr, &y, &index);
    viewPtr->worldHeight = y;		/* Set the scroll height of the
					 * hierarchy. */
    if (viewPtr->worldHeight < 1) {
	viewPtr->worldHeight = 1;
    }
    {
	int maxX;
	int sum;
	size_t i;

	sum = maxX = 0;
	i = 0;
	for (/*empty*/; i <= (viewPtr->depth + 1); i++) {
	    int x;

	    sum += viewPtr->levelInfo[i].iconWidth;
	    if (i <= viewPtr->depth) {
		viewPtr->levelInfo[i + 1].x = sum;
	    }
	    x = sum;
	    if (((viewPtr->flags & HIDE_ROOT) == 0) || (i > 1)) {
		x += viewPtr->levelInfo[i].labelWidth;
	    }
	    if (x > maxX) {
		maxX = x;
	    }
	}
	viewPtr->treeColumn.maxWidth = maxX;
	viewPtr->treeWidth = maxX;
    }
}

static void
LayoutColumns(TreeView *viewPtr)
{
    Blt_ChainLink link;
    int sum;

    /* The width of the widget (in world coordinates) is the sum of the column
     * widths. */

    viewPtr->worldWidth = viewPtr->titleHeight = 0;
    sum = 0;
    for (link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL;
	 link = Blt_Chain_NextLink(link)) {
	Column *colPtr;
	
	colPtr = Blt_Chain_GetValue(link);
	colPtr->width = 0;
	if (colPtr->flags & COLUMN_HIDDEN) {
	    continue;
	}
	if ((viewPtr->flags & SHOW_COLUMN_TITLES) &&
	    (viewPtr->titleHeight < colPtr->titleHeight)) {
	    viewPtr->titleHeight = colPtr->titleHeight;
	}
	if (colPtr->reqWidth > 0) {
	    colPtr->width = colPtr->reqWidth;
	} else {
	    int colWidth;

	    colWidth = colPtr->maxWidth + PADDING(colPtr->pad);
	    /* The computed width of a column is the maximum of the title
	     * width and the widest entry. */
	    colPtr->width = MAX(colPtr->titleWidth, colWidth);
	    /* Check that the width stays within any constraints that have
	     * been set. */
	    if ((colPtr->reqMin > 0) && (colPtr->reqMin > colPtr->width)) {
		colPtr->width = colPtr->reqMin;
	    }
	    if ((colPtr->reqMax > 0) && (colPtr->reqMax < colPtr->width)) {
		colPtr->width = colPtr->reqMax;
	    }
	}
	colPtr->width += 2 * colPtr->titleBW;
	colPtr->worldX = sum;
	sum += colPtr->width;
    }
    viewPtr->worldWidth = sum;
    if (VPORTWIDTH(viewPtr) > sum) {
	AdjustColumns(viewPtr);
    }
    sum = 0;
    for (link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL;
	 link = Blt_Chain_NextLink(link)) {
	Column *colPtr;

	colPtr = Blt_Chain_GetValue(link);
	colPtr->worldX = sum;
	sum += colPtr->width;
    }
    if (viewPtr->titleHeight > 0) {
	/* If any headings are displayed, add some extra padding to the
	 * height. */
	viewPtr->titleHeight += 4;
    }
    /* viewPtr->worldWidth += 10; */
    if (viewPtr->yScrollUnits < 1) {
	viewPtr->yScrollUnits = 1;
    }
    if (viewPtr->xScrollUnits < 1) {
	viewPtr->xScrollUnits = 1;
    }
    if (viewPtr->worldWidth < 1) {
	viewPtr->worldWidth = 1;
    }
    viewPtr->flags |= SCROLL_PENDING;
}

/*
 *---------------------------------------------------------------------------
 *
 * ComputeLayout --
 *
 *	Recompute the layout when entries are opened/closed, inserted/deleted,
 *	or when text attributes change (such as font, linespacing).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The world coordinates are set for all the opened entries.
 *
 *---------------------------------------------------------------------------
 */
static void
ComputeLayout(TreeView *viewPtr)
{
    Blt_ChainLink link;
    Column *colPtr;
    Entry *entryPtr;
    Value *valuePtr;

    if (viewPtr->flatView) {
	ComputeFlatLayout(viewPtr);
    } else {
	ComputeTreeLayout(viewPtr);
    }

    /*
     * Determine the width of each column based upon the entries that as open
     * (not hidden).  The widest entry in a column determines the width of that
     * column.
     */
    /* Initialize the columns. */
    for (link = Blt_Chain_FirstLink(viewPtr->columns); 
	 link != NULL; link = Blt_Chain_NextLink(link)) {
	colPtr = Blt_Chain_GetValue(link);
	colPtr->maxWidth = 0;
	colPtr->max = SHRT_MAX;
	if (colPtr->reqMax > 0) {
	    colPtr->max = colPtr->reqMax;
	}
    }
    /* The treeview column width was computed earlier. */
    viewPtr->treeColumn.maxWidth = viewPtr->treeWidth;

    /* 
     * Look at all open entries and their values.  Determine the column widths
     * by tracking the maximum width value in each column.
     */
    for (entryPtr = viewPtr->rootPtr; entryPtr != NULL; 
	 entryPtr = NextEntry(entryPtr, ENTRY_MASK)) {
	for (valuePtr = entryPtr->values; valuePtr != NULL; 
	     valuePtr = valuePtr->nextPtr) {
	    if (valuePtr->columnPtr->maxWidth < valuePtr->width) {
		valuePtr->columnPtr->maxWidth = valuePtr->width;
	    }
	}	    
    }
    /* Now layout the columns with the proper sizes. */
    LayoutColumns(viewPtr);
    viewPtr->flags &= ~(LAYOUT_PENDING | DIRTY);
}

#ifdef notdef
static void
PrintFlags(TreeView *viewPtr, char *string)
{    
    fprintf(stderr, "%s: flags=", string);
    if (viewPtr->flags & LAYOUT_PENDING) {
	fprintf(stderr, "layout ");
    }
    if (viewPtr->flags & REDRAW_PENDING) {
	fprintf(stderr, "redraw ");
    }
    if (viewPtr->flags & SCROLLX) {
	fprintf(stderr, "xscroll ");
    }
    if (viewPtr->flags & SCROLLY) {
	fprintf(stderr, "yscroll ");
    }
    if (viewPtr->flags & FOCUS) {
	fprintf(stderr, "focus ");
    }
    if (viewPtr->flags & DIRTY) {
	fprintf(stderr, "dirty ");
    }
    if (viewPtr->flags & UPDATE) {
	fprintf(stderr, "update ");
    }
    if (viewPtr->flags & RESORT) {
	fprintf(stderr, "resort ");
    }
    if (viewPtr->flags & SORTED) {
	fprintf(stderr, "sorted ");
    }
    if (viewPtr->flags & SORT_PENDING) {
	fprintf(stderr, "sort_pending ");
    }
    if (viewPtr->flags & REDRAW_BORDERS) {
	fprintf(stderr, "borders ");
    }
    if (viewPtr->flags & VIEWPORT) {
	fprintf(stderr, "viewport ");
    }
    fprintf(stderr, "\n");
}
#endif

/*
 *---------------------------------------------------------------------------
 *
 * ComputeVisibleEntries --
 *
 *	The entries visible in the viewport (the widget's window) are inserted
 *	into the array of visible nodes.
 *
 * Results:
 *	Returns 1 if beyond the last visible entry, 0 otherwise.
 *
 * Side effects:
 *	The array of visible nodes is filled.
 *
 *---------------------------------------------------------------------------
 */
static int
ComputeVisibleEntries(TreeView *viewPtr)
{
    int height;
    int numSlots;
    int maxX;
    int xOffset, yOffset;

    xOffset = Blt_AdjustViewport(viewPtr->xOffset, viewPtr->worldWidth,
	VPORTWIDTH(viewPtr), viewPtr->xScrollUnits, viewPtr->scrollMode);
    yOffset = Blt_AdjustViewport(viewPtr->yOffset, 
	viewPtr->worldHeight, VPORTHEIGHT(viewPtr), viewPtr->yScrollUnits, 
	viewPtr->scrollMode);

    if ((xOffset != viewPtr->xOffset) || (yOffset != viewPtr->yOffset)) {
	viewPtr->yOffset = yOffset;
	viewPtr->xOffset = xOffset;
	viewPtr->flags |= VIEWPORT;
    }
    height = VPORTHEIGHT(viewPtr);

    /* Allocate worst case number of slots for entry array. */
    numSlots = (height / viewPtr->minHeight) + 3;
    if (numSlots != viewPtr->numVisible) {
	if (viewPtr->visibleArr != NULL) {
	    Blt_Free(viewPtr->visibleArr);
	}
	viewPtr->visibleArr = Blt_AssertCalloc(numSlots + 1, 
					       sizeof(Entry *));
    }
    viewPtr->numVisible = 0;
    viewPtr->visibleArr[numSlots] = viewPtr->visibleArr[0] = NULL;

    if (viewPtr->rootPtr->flags & ENTRY_HIDE) {
	return TCL_OK;			/* Root node is hidden. */
    }
    /* Find the node where the view port starts. */
    if (viewPtr->flatView) {
	Entry **epp;

	/* Find the starting entry visible in the viewport. It can't be hidden
	 * or any of it's ancestors closed. */
    again:
	for (epp = viewPtr->flatArr; *epp != NULL; epp++) {
	    if (((*epp)->worldY + (*epp)->height) > viewPtr->yOffset) {
		break;
	    }
	}	    
	/*
	 * If we can't find the starting node, then the view must be scrolled
	 * down, but some nodes were deleted.  Reset the view back to the top
	 * and try again.
	 */
	if (*epp == NULL) {
	    if (viewPtr->yOffset == 0) {
		return TCL_OK;		/* All entries are hidden. */
	    }
	    viewPtr->yOffset = 0;
	    goto again;
	}

	maxX = 0;
	height += viewPtr->yOffset;
	for (/*empty*/; *epp != NULL; epp++) {
	    int x;

	    (*epp)->worldX = LEVELX(0) + viewPtr->treeColumn.worldX;
	    x = (*epp)->worldX + ICONWIDTH(0) + (*epp)->width;
	    if (x > maxX) {
		maxX = x;
	    }
	    if ((*epp)->worldY >= height) {
		break;
	    }
	    viewPtr->visibleArr[viewPtr->numVisible] = *epp;
	    viewPtr->numVisible++;
	}
	viewPtr->visibleArr[viewPtr->numVisible] = NULL;
    } else {
	Entry *ep;

	ep = viewPtr->rootPtr;
	while ((ep->worldY + ep->height) <= viewPtr->yOffset) {
	    for (ep = LastChild(ep, ENTRY_HIDE); ep != NULL; 
		 ep = PrevSibling(ep, ENTRY_HIDE)) {
		if (ep->worldY <= viewPtr->yOffset) {
		    break;
		}
	    }
	    /*
	     * If we can't find the starting node, then the view must be
	     * scrolled down, but some nodes were deleted.  Reset the view
	     * back to the top and try again.
	     */
	    if (ep == NULL) {
		if (viewPtr->yOffset == 0) {
		    return TCL_OK;	/* All entries are hidden. */
		}
		viewPtr->yOffset = 0;
		continue;
	    }
	}
	
	height += viewPtr->yOffset;
	maxX = 0;
	viewPtr->treeColumn.maxWidth = viewPtr->treeWidth;

	for (; ep != NULL; ep = NextEntry(ep, ENTRY_MASK)){
	    int x;
	    int level;

	    /*
	     * Compute and save the entry's X-coordinate now that we know the
	     * maximum level offset for the entire widget.
	     */
	    level = DEPTH(viewPtr, ep->node);
	    ep->worldX = LEVELX(level) + viewPtr->treeColumn.worldX;
	    x = ep->worldX + ICONWIDTH(level) + ICONWIDTH(level+1) + ep->width;
	    if (x > maxX) {
		maxX = x;
	    }
	    if (ep->worldY >= height) {
		break;
	    }
	    viewPtr->visibleArr[viewPtr->numVisible] = ep;
	    viewPtr->numVisible++;
	}
	viewPtr->visibleArr[viewPtr->numVisible] = NULL;
    }
    /*
     * Note:	It's assumed that the view port always starts at or
     *		over an entry.  Check that a change in the hierarchy
     *		(e.g. closing a node) hasn't left the viewport beyond
     *		the last entry.  If so, adjust the viewport to start
     *		on the last entry.
     */
    if (viewPtr->xOffset > (viewPtr->worldWidth - viewPtr->xScrollUnits)) {
	viewPtr->xOffset = viewPtr->worldWidth - viewPtr->xScrollUnits;
    }
    if (viewPtr->yOffset > (viewPtr->worldHeight - viewPtr->yScrollUnits)) {
	viewPtr->yOffset = viewPtr->worldHeight - viewPtr->yScrollUnits;
    }
    viewPtr->xOffset = Blt_AdjustViewport(viewPtr->xOffset, 
	viewPtr->worldWidth, VPORTWIDTH(viewPtr), viewPtr->xScrollUnits, 
	viewPtr->scrollMode);
    viewPtr->yOffset = Blt_AdjustViewport(viewPtr->yOffset,
	viewPtr->worldHeight, VPORTHEIGHT(viewPtr), viewPtr->yScrollUnits,
	viewPtr->scrollMode);

    viewPtr->flags &= ~DIRTY;
    Blt_PickCurrentItem(viewPtr->bindTable);
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * DrawLines --
 *
 * 	Draws vertical lines for the ancestor nodes.  While the entry of the
 * 	ancestor may not be visible, its vertical line segment does extent
 * 	into the viewport.  So walk back up the hierarchy drawing lines
 * 	until we get to the root.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Vertical lines are drawn for the ancestor nodes.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawLines(
    TreeView *viewPtr,			/* Widget record containing the
					 * attribute information for buttons. */
    GC gc,
    Drawable drawable)			/* Pixmap or window to draw into. */
{
    Entry **epp;
    Button *buttonPtr;
    Entry *entryPtr;		/* Entry to be drawn. */

    entryPtr = viewPtr->visibleArr[0];
    while (entryPtr != viewPtr->rootPtr) {
	int level;
	
	entryPtr = ParentEntry(entryPtr);
	if (entryPtr == NULL) {
	    break;
	}
	level = DEPTH(viewPtr, entryPtr->node);
	if (entryPtr->vertLineLength > 0) {
	    int ax, ay, by;
	    int x, y;
	    int height;

	    /*
	     * World X-coordinates aren't computed for entries that are
	     * outside the viewport.  So for each off-screen ancestor node
	     * compute it here too.
	     */
	    entryPtr->worldX = LEVELX(level) + viewPtr->treeColumn.worldX;
	    x = SCREENX(viewPtr, entryPtr->worldX);
	    y = SCREENY(viewPtr, entryPtr->worldY);
	    height = MAX3(entryPtr->lineHeight, entryPtr->iconHeight, 
			  viewPtr->button.height);
	    ax = x + ICONWIDTH(level) + ICONWIDTH(level + 1) / 2;
	    ay = y + height / 2;
	    by = ay + entryPtr->vertLineLength;
	    if ((entryPtr == viewPtr->rootPtr) && (viewPtr->flags & HIDE_ROOT)){
		Entry *nextPtr;
		int h;

		/* If the root node is hidden, go to the next entry to start
		 * the vertical line. */
		nextPtr = NextEntry(viewPtr->rootPtr, ENTRY_MASK);
		h = MAX3(nextPtr->lineHeight, nextPtr->iconHeight, 
			 viewPtr->button.height);
		ay = SCREENY(viewPtr, nextPtr->worldY) + h / 2;
	    }
	    /*
	     * Clip the line's Y-coordinates at the viewport's borders.
	     */
	    if (ay < 0) {
		ay &= 0x1;		/* Make sure the dotted line starts on
					 * the same even/odd pixel. */
	    }
	    if (by > Tk_Height(viewPtr->tkwin)) {
		by = Tk_Height(viewPtr->tkwin);
	    }
	    if ((ay < Tk_Height(viewPtr->tkwin)) && (by > 0)) {
		ay |= 0x1;
		XDrawLine(viewPtr->display, drawable, gc, ax, ay, ax, by);
	    }
	}
    }
    buttonPtr = &viewPtr->button;
    for (epp = viewPtr->visibleArr; *epp != NULL; epp++) {
	int x, y, w, h;
	int buttonY, level;
	int x1, x2, y1, y2;

	entryPtr = *epp;
	/* Entry is open, draw vertical line. */
	x = SCREENX(viewPtr, entryPtr->worldX);
	y = SCREENY(viewPtr, entryPtr->worldY);
	level = DEPTH(viewPtr, entryPtr->node);
	w = ICONWIDTH(level);
	h = MAX3(entryPtr->lineHeight, entryPtr->iconHeight, buttonPtr->height);
	entryPtr->buttonX = (w - buttonPtr->width) / 2;
	entryPtr->buttonY = (h - buttonPtr->height) / 2;
	buttonY = y + entryPtr->buttonY;
	x1 = x + (w / 2);
	y1 = buttonY + (buttonPtr->height / 2);
	x2 = x1 + (ICONWIDTH(level) + ICONWIDTH(level + 1)) / 2;
	if (Blt_Tree_ParentNode(entryPtr->node) != NULL) {
	    /*
	     * For every node except root, draw a horizontal line from the
	     * vertical bar to the middle of the icon.
	     */
	    y1 |= 0x1;
	    XDrawLine(viewPtr->display, drawable, gc, x1, y1, x2, y1);
	}
	if (((entryPtr->flags & ENTRY_CLOSED) == 0) && 
	    (entryPtr->vertLineLength > 0)) {
	    y2 = y1 + entryPtr->vertLineLength;
	    if (y2 > Tk_Height(viewPtr->tkwin)) {
		y2 = Tk_Height(viewPtr->tkwin); /* Clip line at window border.*/
	    }
	    XDrawLine(viewPtr->display, drawable, gc, x2, y1, x2, y2);
	}
    }	
}

static void
DrawRule(
    TreeView *viewPtr,			/* Widget record containing the
					 * attribute information for rules. */
    Column *colPtr,
    Drawable drawable)			/* Pixmap or window to draw into. */
{
    int x, y1, y2;

    x = SCREENX(viewPtr, colPtr->worldX) + 
	colPtr->width + viewPtr->ruleMark - viewPtr->ruleAnchor - 1;

    y1 = viewPtr->titleHeight + viewPtr->inset;
    y2 = Tk_Height(viewPtr->tkwin) - viewPtr->inset;
    XDrawLine(viewPtr->display, drawable, colPtr->ruleGC, x, y1, x, y2);
    viewPtr->flags = TOGGLE(viewPtr->flags, RULE_ACTIVE_COLUMN);
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawButton --
 *
 * 	Draws a button for the given entry. The button is drawn centered in the
 * 	region immediately to the left of the origin of the entry (computed in
 * 	the layout routines). The height and width of the button were previously
 * 	calculated from the average row height.
 *
 *		button height = entry height - (2 * some arbitrary padding).
 *		button width = button height.
 *
 *	The button may have a border.  The symbol (either a plus or minus) is
 *	slight smaller than the width or height minus the border.
 *
 *	    x,y origin of entry
 *
 *              +---+
 *              | + | icon label
 *              +---+
 *             closed
 *
 *           |----|----| horizontal offset
 *
 *              +---+
 *              | - | icon label
 *              +---+
 *              open
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	A button is drawn for the entry.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawButton(
    TreeView *viewPtr,			/* Widget record containing the
					 * attribute information for buttons. */
    Entry *entryPtr,			/* Entry. */
    Drawable drawable,			/* Pixmap or window to draw into. */
    int x, int y)
{
    Blt_Bg bg;
    Button *buttonPtr = &viewPtr->button;
    Icon icon;
    int relief;
    int width, height;

    bg = (entryPtr == viewPtr->activeBtnPtr) 
	? buttonPtr->activeBg : buttonPtr->bg;
    relief = (entryPtr->flags & ENTRY_CLOSED) 
	? buttonPtr->closeRelief : buttonPtr->openRelief;
    if (relief == TK_RELIEF_SOLID) {
	relief = TK_RELIEF_FLAT;
    }
    Blt_Bg_FillRectangle(viewPtr->tkwin, drawable, bg, x, y, buttonPtr->width, 
	buttonPtr->height, buttonPtr->borderWidth, relief);

    x += buttonPtr->borderWidth;
    y += buttonPtr->borderWidth;
    width  = buttonPtr->width  - (2 * buttonPtr->borderWidth);
    height = buttonPtr->height - (2 * buttonPtr->borderWidth);

    icon = NULL;
    if (buttonPtr->icons != NULL) {	/* Open or close button icon? */
	icon = buttonPtr->icons[0];
	if (((entryPtr->flags & ENTRY_CLOSED) == 0) && 
	    (buttonPtr->icons[1] != NULL)) {
	    icon = buttonPtr->icons[1];
	}
    }
    if (icon != NULL) {			/* Icon or rectangle? */
	Tk_RedrawImage(TreeView_IconBits(icon), 0, 0, width, height, 
		drawable, x, y);
    } else {
	int top, bottom, left, right;
	XSegment segments[6];
	int count;
	GC gc;

	gc = (entryPtr == viewPtr->activeBtnPtr)
	    ? buttonPtr->activeGC : buttonPtr->normalGC;
	if (relief == TK_RELIEF_FLAT) {
	    /* Draw the box outline */

	    left = x - buttonPtr->borderWidth;
	    top = y - buttonPtr->borderWidth;
	    right = left + buttonPtr->width - 1;
	    bottom = top + buttonPtr->height - 1;

	    segments[0].x1 = left;
	    segments[0].x2 = right;
	    segments[0].y2 = segments[0].y1 = top;
	    segments[1].x2 = segments[1].x1 = right;
	    segments[1].y1 = top;
	    segments[1].y2 = bottom;
	    segments[2].x2 = segments[2].x1 = left;
	    segments[2].y1 = top;
	    segments[2].y2 = bottom;
#ifdef WIN32
	    segments[2].y2++;
#endif
	    segments[3].x1 = left;
	    segments[3].x2 = right;
	    segments[3].y2 = segments[3].y1 = bottom;
#ifdef WIN32
	    segments[3].x2++;
#endif
	}
	top = y + height / 2;
	left = x + BUTTON_IPAD;
	right = x + width - BUTTON_IPAD;

	segments[4].y1 = segments[4].y2 = top;
	segments[4].x1 = left;
	segments[4].x2 = right - 1;
#ifdef WIN32
	segments[4].x2++;
#endif

	count = 5;
	if (entryPtr->flags & ENTRY_CLOSED) { /* Draw the vertical line for the
					       * plus. */
	    top = y + BUTTON_IPAD;
	    bottom = y + height - BUTTON_IPAD;
	    segments[5].y1 = top;
	    segments[5].y2 = bottom - 1;
	    segments[5].x1 = segments[5].x2 = x + width / 2;
#ifdef WIN32
	    segments[5].y2++;
#endif
	    count = 6;
	}
	XDrawSegments(viewPtr->display, drawable, gc, segments, count);
    }
}


static int
DrawImage(
    TreeView *viewPtr,			/* Widget record containing the
					 * attribute information for
					 * buttons. */
    Entry *entryPtr,			/* Entry to display. */
    Drawable drawable,			/* Pixmap or window to draw into. */
    int x, int y)
{
    Icon icon;

    icon = GetEntryIcon(viewPtr, entryPtr);

    if (icon != NULL) {			/* Icon or default icon bitmap? */
	int entryHeight;
	int level;
	int maxY;
	int top, bottom;
	int topInset, botInset;
	int width, height;

	level = DEPTH(viewPtr, entryPtr->node);
	entryHeight = MAX3(entryPtr->lineHeight, entryPtr->iconHeight, 
		viewPtr->button.height);
	height = TreeView_IconHeight(icon);
	width = TreeView_IconWidth(icon);
	if (viewPtr->flatView) {
	    x += (ICONWIDTH(0) - width) / 2;
	} else {
	    x += (ICONWIDTH(level + 1) - width) / 2;
	}	    
	y += (entryHeight - height) / 2;
	botInset = viewPtr->inset - INSET_PAD;
	topInset = viewPtr->titleHeight + viewPtr->inset;
	maxY = Tk_Height(viewPtr->tkwin) - botInset;
	top = 0;
	bottom = y + height;
	if (y < topInset) {
	    height += y - topInset;
	    top = -y + topInset;
	    y = topInset;
	} else if (bottom >= maxY) {
	    height = maxY - y;
	}
	Tk_RedrawImage(TreeView_IconBits(icon), 0, top, width, height, 
		drawable, x, y);
    } 
    return (icon != NULL);
}

static int
DrawLabel(
    TreeView *viewPtr,			/* Widget record. */
    Entry *entryPtr,			/* Entry attribute information. */
    Drawable drawable,			/* Pixmap or window to draw into. */
    int x, int y,
    int maxLength,
    TkRegion rgn)			
{
    const char *label;
    int entryHeight;
    int width, height;			/* Width and height of label. */
    int isFocused, isSelected, isActive;

    entryHeight = MAX3(entryPtr->lineHeight, entryPtr->iconHeight, 
       viewPtr->button.height);
    isFocused = ((entryPtr == viewPtr->focusPtr) && (viewPtr->flags & FOCUS));
    isSelected = EntryIsSelected(viewPtr, entryPtr);
    isActive = (entryPtr == viewPtr->activePtr);

    /* Includes padding, selection 3-D border, and focus outline. */
    width = entryPtr->labelWidth;
    height = entryPtr->labelHeight;

    /* Center the label, if necessary, vertically along the entry row. */
    if (height < entryHeight) {
	y += (entryHeight - height) / 2;
    }
    if (isFocused) {			/* Focus outline */
	if (isSelected) {
	    XColor *color;

	    color = viewPtr->selection.fgColor;
	    XSetForeground(viewPtr->display, viewPtr->focusGC, color->pixel);
	}
	if (width > maxLength) {
	    width = maxLength | 0x1;	/* Width has to be odd for the dots in
					 * the focus rectangle to align. */
	}
        if (rgn != NULL) {
            TkSetRegion(viewPtr->display, viewPtr->focusGC, rgn);
        }	
	XDrawRectangle(viewPtr->display, drawable, viewPtr->focusGC, x, y+1, 
		       width - 1, height - 3);
	if (isSelected) {
	    XSetForeground(viewPtr->display, viewPtr->focusGC, 
		viewPtr->focusColor->pixel);
	}
        if (rgn != NULL) {
            XSetClipMask(viewPtr->display, viewPtr->focusGC, None);
        }	
    }
    x += FOCUS_WIDTH + LABEL_PADX + viewPtr->selection.borderWidth;
    y += FOCUS_WIDTH + LABEL_PADY + viewPtr->selection.borderWidth;

    label = GETLABEL(entryPtr);
    if ((label[0] != '\0') && (maxLength > 0)) {
	TextStyle ts;
	Blt_Font font;
	XColor *color;

	font = entryPtr->font;
	if (font == NULL) {
	    font = GetStyleFont(&viewPtr->treeColumn);
	}
	if (isSelected) {
	    color = viewPtr->selection.fgColor;
	} else if (entryPtr->color != NULL) {
	    color = entryPtr->color;
	} else {
	    color = GetStyleForeground(&viewPtr->treeColumn);
	}
	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, font);
	Blt_Ts_SetForeground(ts, color);
	Blt_Ts_SetFontClipRegion(ts, rgn);
	Blt_Ts_SetMaxLength(ts, maxLength);
	Blt_Ts_DrawLayout(viewPtr->tkwin, drawable, entryPtr->textPtr, &ts, 
		x, y);
	if (isActive) {
	    Blt_Ts_UnderlineLayout(viewPtr->tkwin, drawable, entryPtr->textPtr, 
		&ts, x, y);
	}
    }
    return entryHeight;
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawValue --
 *
 * 	Draws a column value for the given entry.  
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	A button is drawn for the entry.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawValue(
    TreeView *viewPtr,			/* Widget record. */
    Entry *entryPtr,			/* Node of entry to be drawn. */
    Value *valuePtr,
    Drawable drawable,			/* Pixmap or window to draw into. */
    int x, int y)
{
    ColumnStyle *stylePtr;

    stylePtr = CHOOSESTYLE(viewPtr, valuePtr->columnPtr, valuePtr);
    (*stylePtr->classPtr->drawProc)(entryPtr, valuePtr, drawable, stylePtr, 
	x, y);
}

static void
DisplayValue(TreeView *viewPtr, Entry *entryPtr, Value *valuePtr)
{
    int sx, sy, x, y;
    int w, h;
    int pixWidth, pixHeight;
    int x1, x2, y1, y2;
    Column *colPtr;
    ColumnStyle *stylePtr;
    Blt_Bg bg;
    int overlap;

    stylePtr = valuePtr->stylePtr;
    if (stylePtr == NULL) {
	stylePtr = valuePtr->columnPtr->stylePtr;
    }
    if (stylePtr->cursor != None) {
	if (valuePtr == viewPtr->activeValuePtr) {
	    Tk_DefineCursor(viewPtr->tkwin, stylePtr->cursor);
	} else {
	    if (viewPtr->cursor != None) {
		Tk_DefineCursor(viewPtr->tkwin, viewPtr->cursor);
	    } else {
		Tk_UndefineCursor(viewPtr->tkwin);
	    }
	}
    }
    colPtr = valuePtr->columnPtr;
    x = SCREENX(viewPtr, colPtr->worldX) + colPtr->pad.side1;
    y = SCREENY(viewPtr, entryPtr->worldY);
    h = entryPtr->height - 2;
    w = valuePtr->columnPtr->width - PADDING(colPtr->pad);

    y1 = viewPtr->titleHeight + viewPtr->inset;
    y2 = Tk_Height(viewPtr->tkwin) - viewPtr->inset;
    x1 = viewPtr->inset;
    x2 = Tk_Width(viewPtr->tkwin) - viewPtr->inset;

    if (((x + w) < x1) || (x > x2) || ((y + h) < y1) || (y > y2)) {
	return;				/* Value is entirely clipped. */
    }

    /* Draw the background of the value. */
    if ((valuePtr == viewPtr->activeValuePtr) ||
	(!EntryIsSelected(viewPtr, entryPtr))) {
	bg = GetStyleBackground(colPtr);
    } else {
	bg = CHOOSE(viewPtr->selection.bg, stylePtr->selBg);
    }
    /*FIXME*/
    /* bg = CHOOSE(viewPtr->selBg, stylePtr->selBg);  */
    overlap = FALSE;
    /* Clip the drawable if necessary */
    sx = sy = 0;
    pixWidth = w, pixHeight = h;
    if (x < x1) {
	pixWidth -= x1 - x;
	sx += x1 - x;
	x = x1;
	overlap = TRUE;
    }
    if ((x + w) >= x2) {
	pixWidth -= (x + w) - x2;
	overlap = TRUE;
    }
    if (y < y1) {
	pixHeight -= y1 - y;
	sy += y1 - y;
	y = y1;
	overlap = TRUE;
    }
    if ((y + h) >= y2) {
	pixHeight -= (y + h) - y2;
	overlap = TRUE;
    }
    if (overlap) {
	Drawable drawable;

	drawable = Blt_GetPixmap(viewPtr->display, Tk_WindowId(viewPtr->tkwin), 
		pixWidth, pixHeight, Tk_Depth(viewPtr->tkwin));
	Blt_Bg_FillRectangle(viewPtr->tkwin, drawable, bg, 0, 0, 
		pixWidth, pixHeight, 0, TK_RELIEF_FLAT);
	DrawValue(viewPtr, entryPtr, valuePtr, drawable, sx, sy);
	XCopyArea(viewPtr->display, drawable, Tk_WindowId(viewPtr->tkwin), 
		  viewPtr->lineGC, 0, 0, pixWidth, pixHeight, x, y+1);
	Tk_FreePixmap(viewPtr->display, drawable);
    } else {
	Drawable drawable;

	drawable = Tk_WindowId(viewPtr->tkwin);
	Blt_Bg_FillRectangle(viewPtr->tkwin, drawable, bg, x, y+1, w, h, 
		0, TK_RELIEF_FLAT);
	DrawValue(viewPtr, entryPtr, valuePtr, drawable, x, y);
    }
}


/*
 *---------------------------------------------------------------------------
 *
 * DrawFlatEntry --
 *
 * 	Draws a button for the given entry.  Note that buttons should only be
 * 	drawn if the entry has sub-entries to be opened or closed.  It's the
 * 	responsibility of the calling routine to ensure this.
 *
 *	The button is drawn centered in the region immediately to the left of
 *	the origin of the entry (computed in the layout routines). The height
 *	and width of the button were previously calculated from the average row
 *	height.
 *
 *		button height = entry height - (2 * some arbitrary padding).
 *		button width = button height.
 *
 *	The button has a border.  The symbol (either a plus or minus) is slight
 *	smaller than the width or height minus the border.
 *
 *	    x,y origin of entry
 *
 *              +---+
 *              | + | icon label
 *              +---+
 *             closed
 *
 *           |----|----| horizontal offset
 *
 *              +---+
 *              | - | icon label
 *              +---+
 *              open
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	A button is drawn for the entry.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawFlatEntry(
    TreeView *viewPtr,			/* Widget record containing the
					 * attribute information for
					 * buttons. */
    Entry *entryPtr,			/* Entry to be drawn. */
    Drawable drawable)			/* Pixmap or window to draw into. */
{
    int level;
    int x, y, xMax;

    entryPtr->flags &= ~ENTRY_REDRAW;

    x = SCREENX(viewPtr, entryPtr->worldX);
    y = SCREENY(viewPtr, entryPtr->worldY);
    if (!DrawImage(viewPtr, entryPtr, drawable, x, y)) {
	x -= (DEF_ICON_WIDTH * 2) / 3;
    }
    level = 0;
    x += ICONWIDTH(level);
    /* Entry label. */
    xMax = SCREENX(viewPtr, viewPtr->treeColumn.worldX) + 
	viewPtr->treeColumn.width - viewPtr->treeColumn.titleBW - 
	viewPtr->treeColumn.pad.side2;
    DrawLabel(viewPtr, entryPtr, drawable, x, y, xMax - x, NULL);
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawTreeEntry --
 *
 * 	Draws a button for the given entry.  Note that buttons should only be
 * 	drawn if the entry has sub-entries to be opened or closed.  It's the
 * 	responsibility of the calling routine to ensure this.
 *
 *	The button is drawn centered in the region immediately to the left of
 *	the origin of the entry (computed in the layout routines). The height
 *	and width of the button were previously calculated from the average
 *	row height.
 *
 *		button height = entry height - (2 * some arbitrary padding).
 *		button width = button height.
 *
 *	The button has a border.  The symbol (either a plus or minus) is
 *	slight smaller than the width or height minus the border.
 *
 *	    x,y origin of entry
 *
 *              +---+
 *              | + | icon label
 *              +---+
 *             closed
 *
 *           |----|----| horizontal offset
 *
 *              +---+
 *              | - | icon label
 *              +---+
 *              open
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	A button is drawn for the entry.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawTreeEntry(
    TreeView *viewPtr,			/* Widget record. */
    Entry *entryPtr,		/* Entry to be drawn. */
    Drawable drawable)			/* Pixmap or window to draw into. */
{
    Button *buttonPtr = &viewPtr->button;
    int level;
    int width, height;
    int x, y, xMax;

    entryPtr->flags &= ~ENTRY_REDRAW;
    x = SCREENX(viewPtr, entryPtr->worldX);
    y = SCREENY(viewPtr, entryPtr->worldY);

    level = DEPTH(viewPtr, entryPtr->node);
    width = ICONWIDTH(level);
    height = MAX3(entryPtr->lineHeight, entryPtr->iconHeight, 
	buttonPtr->height);

    entryPtr->buttonX = (width - buttonPtr->width) / 2;
    entryPtr->buttonY = (height - buttonPtr->height) / 2;

    if ((entryPtr->flags & ENTRY_HAS_BUTTON) && (entryPtr != viewPtr->rootPtr)){
	/*
	 * Except for the root, draw a button for every entry that needs one.
	 * The displayed button can be either an icon (Tk image) or a line
	 * drawing (rectangle with plus or minus sign).
	 */
	DrawButton(viewPtr, entryPtr, drawable, x + entryPtr->buttonX, 
		y + entryPtr->buttonY);
    }
    x += ICONWIDTH(level);

    if (!DrawImage(viewPtr, entryPtr, drawable, x, y)) {
	x -= (DEF_ICON_WIDTH * 2) / 3;
    }
    x += ICONWIDTH(level + 1);

    /* Entry label. */
    xMax = SCREENX(viewPtr, viewPtr->treeColumn.worldX) + 
	viewPtr->treeColumn.width - viewPtr->treeColumn.titleBW - 
	viewPtr->treeColumn.pad.side2;
    DrawLabel(viewPtr, entryPtr, drawable, x, y, xMax - x, NULL);
}

static void
DrawColumnTitle(TreeView *viewPtr, Column *colPtr, Drawable drawable, 
		int x, int y)
{
    Blt_Bg bg;
    XColor *fg;
    int dw, dx;
    int colWidth, colHeight, need;
    int needArrow;
    int y0;

    if (viewPtr->titleHeight < 1) {
	return;
    }
    y0 = y;
    dx = x;
    colWidth = colPtr->width;
    colHeight = viewPtr->titleHeight;
    dw = colPtr->width;
    if (colPtr->index == (Blt_Chain_GetLength(viewPtr->columns) - 1)) {
	/* If there's any room left over, let the last column take it. */
	dw = Tk_Width(viewPtr->tkwin) - x;
    }
    if (colPtr == viewPtr->colActiveTitlePtr) {
	bg = colPtr->activeTitleBg;
	fg = colPtr->activeTitleFgColor;
    } else {
	bg = colPtr->titleBg;
	fg = colPtr->titleFgColor;
    }
    if (bg == NULL) {
	bg = viewPtr->bg;
    }
    if (fg == NULL) {
	fg = viewPtr->fgColor;
    }
    /* Clear the title area by drawing the background. */
    Blt_Bg_FillRectangle(viewPtr->tkwin, drawable, bg, dx, y, dw, 
	colHeight, 0, TK_RELIEF_FLAT);
    colWidth  -= 2 * (colPtr->titleBW + TITLE_PADX);
    colHeight -= 2 * colPtr->titleBW;
    x += colPtr->titleBW + TITLE_PADX;
    y += colPtr->titleBW;
    needArrow = (colPtr == viewPtr->sortInfo.markPtr);

    need = colPtr->titleWidth - 2 * TITLE_PADX;
    if (!needArrow) {
	need -= colPtr->arrowWidth + TITLE_PADX;
    }
    if (colWidth > need) {
	switch (colPtr->titleJustify) {
	case TK_JUSTIFY_RIGHT:
	    x += colWidth - need;
	    break;
	case TK_JUSTIFY_CENTER:
	    x += (colWidth - need) / 2;
	    break;
	case TK_JUSTIFY_LEFT:
	    break;
	}
    }
    if (colPtr->titleIcon != NULL) {
	int ix, iy, iw, ih, gap;

	ih = TreeView_IconHeight(colPtr->titleIcon);
	iw = TreeView_IconWidth(colPtr->titleIcon);
	ix = x;
	/* Center the icon vertically.  We already know the column title is at
	 * least as tall as the icon. */
	iy = y;
        if (colHeight > ih) {
	    iy += (colHeight - ih) / 2;
	}
	Tk_RedrawImage(TreeView_IconBits(colPtr->titleIcon), 0, 0, iw, ih, 
		drawable, ix, iy);
	gap = (colPtr->textWidth > 0) ? TITLE_PADX : 0;
	x += iw + gap;
	colWidth -= iw + gap;
    }
    if (colPtr->textWidth > 0) {
	TextStyle ts;
	int ty;
	int maxLength;

	ty = y;
	maxLength = colWidth;
	if (colHeight > colPtr->textHeight) {
	    ty += (colHeight - colPtr->textHeight) / 2;
	}
	if (needArrow) {
	    maxLength -= colPtr->arrowWidth + TITLE_PADX;
	}
	Blt_Ts_InitStyle(ts);
	Blt_Ts_SetFont(ts, colPtr->titleFont);
	Blt_Ts_SetForeground(ts, fg);
	Blt_Ts_SetMaxLength(ts, maxLength);
	Blt_Ts_DrawText(viewPtr->tkwin, drawable, colPtr->text, -1, &ts, x, ty);
	x += MIN(colPtr->textWidth, maxLength);
    }
    if (needArrow) {
	int ay, ax, aw, ah;
	SortInfo *sortPtr;

	sortPtr = &viewPtr->sortInfo;
	ax = x + TITLE_PADX;
	ay = y;
	aw = colPtr->arrowWidth;
	ah = colPtr->arrowHeight;
	if (colHeight > colPtr->arrowHeight) {
	    ay += (colHeight - colPtr->arrowHeight) / 2;
	}
	if ((sortPtr->decreasing) && (colPtr->sortUp != NULL)) {
	    Tk_RedrawImage(TreeView_IconBits(colPtr->sortUp), 0, 0, aw, ah, 
		drawable, ax, ay);
	} else if (colPtr->sortDown != NULL) {
	    Tk_RedrawImage(TreeView_IconBits(colPtr->sortDown), 0, 0, aw, ah, 
		drawable, ax, ay);
	} else {
	    Blt_DrawArrow(viewPtr->display, drawable, fg, ax, ay, aw, ah, 
		colPtr->titleBW, (sortPtr->decreasing) ? ARROW_UP : ARROW_DOWN);
	}
    }
    Blt_Bg_DrawRectangle(viewPtr->tkwin, drawable, bg, dx, y0, 
	dw, viewPtr->titleHeight, colPtr->titleBW, colPtr->titleRelief);
}

static void
DisplayColumnTitle(TreeView *viewPtr, Column *colPtr, Drawable drawable)
{
    int x, y, x1, x2;
    int clipped;

    y = viewPtr->inset;
    x1 = x = SCREENX(viewPtr, colPtr->worldX);
    x2 = x1 + colPtr->width;
    if ((x1 >= (Tk_Width(viewPtr->tkwin) - viewPtr->inset)) ||
	(x2 <= viewPtr->inset)) {
	return;				/* Column starts after the window or
					 * ends before the the window. */
    }
    clipped = FALSE;
    if (x1 < viewPtr->inset) {
	x1 = viewPtr->inset;
	clipped = TRUE;
    }
    if (x2 > (Tk_Width(viewPtr->tkwin) - viewPtr->inset)) {
	x2 = Tk_Width(viewPtr->tkwin) - viewPtr->inset;
	clipped = TRUE;
    }
    if (clipped) {
	long w, dx;
	Pixmap pixmap;

	w = x2 - x1;
	dx = x1 - x;
	/* Draw into a pixmap and then copy it into the drawable.  */
	pixmap = Blt_GetPixmap(viewPtr->display, Tk_WindowId(viewPtr->tkwin), 
		w, viewPtr->titleHeight, Tk_Depth(viewPtr->tkwin));
	DrawColumnTitle(viewPtr, colPtr, pixmap, -dx, 0);
	XCopyArea(viewPtr->display, pixmap, drawable, colPtr->titleGC,
		  0, 0, w, viewPtr->titleHeight, x + dx, viewPtr->inset);
	Tk_FreePixmap(viewPtr->display, pixmap);
    } else {
	DrawColumnTitle(viewPtr, colPtr, drawable, x, y);
    }
}

static void
DrawColumnTitles(TreeView *viewPtr, Drawable drawable)
{
    Blt_ChainLink link;
    Column *colPtr;
    int x;

    if (viewPtr->titleHeight < 1) {
	return;
    }
    for (link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL;
	 link = Blt_Chain_NextLink(link)) {
	colPtr = Blt_Chain_GetValue(link);
	if (colPtr->flags & COLUMN_HIDDEN) {
	    continue;
	}
	x = SCREENX(viewPtr, colPtr->worldX);
	if ((x + colPtr->width) < 0) {
	    continue;			/* Don't draw columns before the left
					 * edge. */
	}
	if (x > Tk_Width(viewPtr->tkwin)) {
	    break;			/* Discontinue when a column starts
					 * beyond the right edge. */
	}
	DisplayColumnTitle(viewPtr, colPtr, drawable);
    }
}

static void
DrawNormalBackground(TreeView *viewPtr, Drawable drawable, int x, int w, 
		     Column *colPtr)
{
    Blt_Bg bg;

    bg = GetStyleBackground(colPtr);
    /* This also fills the background where there are no entries. */
    Blt_Bg_FillRectangle(viewPtr->tkwin, drawable, bg, x, 0, w, 
	Tk_Height(viewPtr->tkwin), 0, TK_RELIEF_FLAT);
    if (viewPtr->altBg != NULL) {
	Entry **epp;

 	for (epp = viewPtr->visibleArr; *epp != NULL; epp++) {
	    if ((*epp)->flatIndex & 0x1) {
		int y;

		y = SCREENY(viewPtr, (*epp)->worldY);
		Blt_Bg_FillRectangle(viewPtr->tkwin, drawable, 
			viewPtr->altBg, x, y, w, (*epp)->height, 
			viewPtr->selection.borderWidth, 
			viewPtr->selection.relief);
	    }
	}
    }
}

static void
DrawSelectionBackground(TreeView *viewPtr, Drawable drawable, int x, int w)
{
    Entry **epp;

    /* 
     * Draw the backgrounds of selected entries first.  The vertical lines
     * connecting child entries will be draw on top.
     */
    for (epp = viewPtr->visibleArr; *epp != NULL; epp++) {
	if (EntryIsSelected(viewPtr, *epp)) {
	    Blt_Bg_FillRectangle(viewPtr->tkwin, drawable, 
		viewPtr->selection.bg, x, SCREENY(viewPtr, (*epp)->worldY), 
		w, (*epp)->height, viewPtr->selection.borderWidth, 
		viewPtr->selection.relief);
	}
    }
}

static void
DrawTreeView(TreeView *viewPtr, Drawable drawable, int x)
{
    Entry **epp;
    int count;

    count = 0;
    for (epp = viewPtr->visibleArr; *epp != NULL; epp++) {
	(*epp)->flags &= ~ENTRY_SELECTED;
	if (EntryIsSelected(viewPtr, *epp)) {
	    (*epp)->flags |= ENTRY_SELECTED;
	    count++;
	}
    }
    if ((viewPtr->lineWidth > 0) && (viewPtr->numVisible > 0)) { 
	/* Draw all the vertical lines from topmost node. */
	DrawLines(viewPtr, viewPtr->lineGC, drawable);
	if (count > 0) {
	    TkRegion rgn;

	    rgn = TkCreateRegion();
	    for (epp = viewPtr->visibleArr; *epp != NULL; epp++) {
		if ((*epp)->flags & ENTRY_SELECTED) {
		    XRectangle r;

		    r.x = 0;
		    r.y = SCREENY(viewPtr, (*epp)->worldY);
		    r.width = Tk_Width(viewPtr->tkwin);
		    r.height = (*epp)->height;
		    TkUnionRectWithRegion(&r, rgn, rgn);
		}
	    }
	    TkSetRegion(viewPtr->display, viewPtr->selection.gc, rgn);
	    DrawLines(viewPtr, viewPtr->selection.gc, drawable);
	    XSetClipMask(viewPtr->display, viewPtr->selection.gc, None);
	    TkDestroyRegion(rgn);
	}
    }
    for (epp = viewPtr->visibleArr; *epp != NULL; epp++) {
	DrawTreeEntry(viewPtr, *epp, drawable);
    }
}


static void
DrawFlatView(TreeView *viewPtr, Drawable drawable, int x)
{
    Entry **epp;

    for (epp = viewPtr->visibleArr; *epp != NULL; epp++) {
	DrawFlatEntry(viewPtr, *epp, drawable);
    }
}

static void
DrawOuterBorders(TreeView *viewPtr, Drawable drawable)
{
    /* Draw 3D border just inside of the focus highlight ring. */
    if (viewPtr->borderWidth > 0) {
	Blt_Bg_DrawRectangle(viewPtr->tkwin, drawable, viewPtr->bg,
	    viewPtr->highlightWidth, viewPtr->highlightWidth,
	    Tk_Width(viewPtr->tkwin) - 2 * viewPtr->highlightWidth,
	    Tk_Height(viewPtr->tkwin) - 2 * viewPtr->highlightWidth,
	    viewPtr->borderWidth, viewPtr->relief);
    }
    /* Draw focus highlight ring. */
    if (viewPtr->highlightWidth > 0) {
	XColor *color;
	GC gc;

	color = (viewPtr->flags & FOCUS)
	    ? viewPtr->highlightColor : viewPtr->highlightBgColor;
	gc = Tk_GCForColor(color, drawable);
	Tk_DrawFocusHighlight(viewPtr->tkwin, gc, viewPtr->highlightWidth,
	    drawable);
    }
    viewPtr->flags &= ~REDRAW_BORDERS;
}

/*
 *---------------------------------------------------------------------------
 *
 * DisplayTreeView --
 *
 * 	This procedure is invoked to display the widget.
 *
 *      Recompute the layout of the text if necessary. This is necessary if the
 *      world coordinate system has changed.  Specifically, the following may
 *      have occurred:
 *
 *	  1.  a text attribute has changed (font, linespacing, etc.).
 *	  2.  an entry's option changed, possibly resizing the entry.
 *
 *      This is deferred to the display routine since potentially many of
 *      these may occur.
 *
 *	Set the vertical and horizontal scrollbars.  This is done here since the
 *	window width and height are needed for the scrollbar calculations.
 *
 * Results:
 *	None.
 *
 * Side effects:
 * 	The widget is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static void
DisplayTreeView(ClientData clientData)	/* Information about widget. */
{
    Blt_ChainLink link;
    Pixmap drawable; 
    TreeView *viewPtr = clientData;
    int reqWidth, reqHeight;
    int count;

    viewPtr->flags &= ~REDRAW_PENDING;
    if (viewPtr->tkwin == NULL) {
	return;				/* Window has been destroyed. */
    }
#ifdef notdef
    fprintf(stderr, "DisplayTreeView %s\n", Tk_PathName(viewPtr->tkwin));
#endif
    if (viewPtr->flags & LAYOUT_PENDING) {
	/*
	 * Recompute the layout when entries are opened/closed,
	 * inserted/deleted, or when text attributes change (such as font,
	 * linespacing).
	 */
	ComputeLayout(viewPtr);
    }
    if (viewPtr->flags & (SCROLL_PENDING | DIRTY)) {
	int width, height;
	/* 
	 * Scrolling means that the view port has changed and that the visible
	 * entries need to be recomputed.
	 */
	ComputeVisibleEntries(viewPtr);
	width = VPORTWIDTH(viewPtr);
	height = VPORTHEIGHT(viewPtr);
	if ((viewPtr->flags & SCROLLX) && (viewPtr->xScrollCmdObjPtr != NULL)) {
	    Blt_UpdateScrollbar(viewPtr->interp, viewPtr->xScrollCmdObjPtr, 
		viewPtr->xOffset, viewPtr->xOffset + width, viewPtr->worldWidth);
	}
	if ((viewPtr->flags & SCROLLY) && (viewPtr->yScrollCmdObjPtr != NULL)) {
	    Blt_UpdateScrollbar(viewPtr->interp, viewPtr->yScrollCmdObjPtr,
		viewPtr->yOffset, viewPtr->yOffset+height, viewPtr->worldHeight);
	}
	viewPtr->flags &= ~SCROLL_PENDING;
    }

    reqHeight = (viewPtr->reqHeight > 0) ? viewPtr->reqHeight : 
	viewPtr->worldHeight + viewPtr->titleHeight + 2 * viewPtr->inset + 1;
    reqWidth = (viewPtr->reqWidth > 0) ? viewPtr->reqWidth : 
	viewPtr->worldWidth + 2 * viewPtr->inset;
    if ((reqWidth != Tk_ReqWidth(viewPtr->tkwin)) || 
	(reqHeight != Tk_ReqHeight(viewPtr->tkwin))) {
	Tk_GeometryRequest(viewPtr->tkwin, reqWidth, reqHeight);
    }
#ifdef notdef
    if (viewPtr->reqWidth == 0) {
	int w;
	/* 
	 * The first time through this routine, set the requested width to the
	 * computed width.  All we want is to automatically set the width of
	 * the widget, not dynamically grow/shrink it as attributes change.
	 */
	w = viewPtr->worldWidth + 2 * viewPtr->inset;
	Tk_GeometryRequest(viewPtr->tkwin, w, viewPtr->reqHeight);
    }
#endif
    if (!Tk_IsMapped(viewPtr->tkwin)) {
	return;
    }
    drawable = Blt_GetPixmap(viewPtr->display, Tk_WindowId(viewPtr->tkwin), 
	Tk_Width(viewPtr->tkwin), Tk_Height(viewPtr->tkwin), 
	Tk_Depth(viewPtr->tkwin));

    if ((viewPtr->focusPtr == NULL) && (viewPtr->numVisible > 0)) {
	/* Re-establish the focus entry at the top entry. */
	viewPtr->focusPtr = viewPtr->visibleArr[0];
    }
    viewPtr->flags |= VIEWPORT;
    if ((viewPtr->flags & RULE_ACTIVE_COLUMN) && (viewPtr->colResizePtr!=NULL)){
	DrawRule(viewPtr, viewPtr->colResizePtr, drawable);
    }
    count = 0;
    for (link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL; 
	 link = Blt_Chain_NextLink(link)) {
	Column *colPtr;
	int x;

	colPtr = Blt_Chain_GetValue(link);
	colPtr->flags &= ~COLUMN_DIRTY;
	if (colPtr->flags & COLUMN_HIDDEN) {
	    continue;
	}
	x = SCREENX(viewPtr, colPtr->worldX);
	if ((x + colPtr->width) < 0) {
	    continue;			/* Don't draw columns before the left
					 * edge. */
	}
	if (x > Tk_Width(viewPtr->tkwin)) {
	    break;			/* Discontinue when a column starts
					 * beyond the right edge. */
	}
	/* Clear the column background. */
	DrawNormalBackground(viewPtr, drawable, x, colPtr->width, colPtr);
	DrawSelectionBackground(viewPtr, drawable, x, colPtr->width);
	if (colPtr != &viewPtr->treeColumn) {
	    Entry **epp;
	    
	    for (epp = viewPtr->visibleArr; *epp != NULL; epp++) {
		Value *vp;
		
		/* Check if there's a corresponding value in the entry. */
		vp = Blt_TreeView_FindValue(*epp, colPtr);
		if (vp != NULL) {
		    DrawValue(viewPtr, *epp, vp, drawable, 
			x + colPtr->pad.side1, SCREENY(viewPtr,(*epp)->worldY));
		}
	    }
	} else {
	    if (viewPtr->flatView) {
		DrawFlatView(viewPtr, drawable, x);
	    } else {
		DrawTreeView(viewPtr, drawable, x);
	    }
	}
 	if (colPtr->relief != TK_RELIEF_FLAT) {
	    Blt_Bg bg;

	    /* Draw a 3D border around the column. */
	    bg = GetStyleBackground(colPtr);
	    Blt_Bg_DrawRectangle(viewPtr->tkwin, drawable, bg, x, 0, 
		colPtr->width, Tk_Height(viewPtr->tkwin), 
		colPtr->borderWidth, colPtr->relief);
	}
	count++;
    }
    if (count == 0) {
	Blt_Bg_FillRectangle(viewPtr->tkwin, drawable, viewPtr->bg, 0, 0, 
		Tk_Width(viewPtr->tkwin), Tk_Height(viewPtr->tkwin), 
		viewPtr->borderWidth, viewPtr->relief);
    }
    if (viewPtr->flags & SHOW_COLUMN_TITLES) {
	DrawColumnTitles(viewPtr, drawable);
    }
    DrawOuterBorders(viewPtr, drawable);
    if ((viewPtr->flags & COLUMN_RULE_NEEDED) &&
	(viewPtr->colResizePtr != NULL)) {
	DrawRule(viewPtr, viewPtr->colResizePtr, drawable);
    }
    /* Now copy the new view to the window. */
    XCopyArea(viewPtr->display, drawable, Tk_WindowId(viewPtr->tkwin), 
	viewPtr->lineGC, 0, 0, Tk_Width(viewPtr->tkwin), 
	Tk_Height(viewPtr->tkwin), 0, 0);
    Tk_FreePixmap(viewPtr->display, drawable);
    viewPtr->flags &= ~VIEWPORT;
}



static int
DisplayLabel(TreeView *viewPtr, Entry *entryPtr, 
		       Drawable drawable)
{
    Blt_Bg bg;
    TkRegion rgn;
    XRectangle r;
    Icon icon;
    int level;
    int y2, y1, x1, x2;
    int x, y, xMax, w, h;

    x = SCREENX(viewPtr, entryPtr->worldX);
    y = SCREENY(viewPtr, entryPtr->worldY);
    h = entryPtr->height - 1;
    w = viewPtr->treeColumn.width - 
	(entryPtr->worldX - viewPtr->treeColumn.worldX);
    xMax = SCREENX(viewPtr, viewPtr->treeColumn.worldX) + 
	viewPtr->treeColumn.width - viewPtr->treeColumn.titleBW - 
	viewPtr->treeColumn.pad.side2;

    icon = GetEntryIcon(viewPtr, entryPtr);
    entryPtr->flags |= ENTRY_ICON;
    if (viewPtr->flatView) {
	x += ICONWIDTH(0);
	w -= ICONWIDTH(0);
	if (icon == NULL) {
	    x -= (DEF_ICON_WIDTH * 2) / 3;
	}
    } else {
	level = DEPTH(viewPtr, entryPtr->node);
	if (!viewPtr->flatView) {
	    x += ICONWIDTH(level);
	    w -= ICONWIDTH(level);
	}
	if (icon != NULL) {
	    x += ICONWIDTH(level + 1);
	    w -= ICONWIDTH(level + 1);
	}
    }
    if (EntryIsSelected(viewPtr, entryPtr)) {
	bg = viewPtr->selection.bg;
    } else {
	bg = GetStyleBackground(&viewPtr->treeColumn);
	if ((viewPtr->altBg != NULL) && (entryPtr->flatIndex & 0x1))  {
	    bg = viewPtr->altBg;
	}
    }
    x1 = viewPtr->inset;
    x2 = Tk_Width(viewPtr->tkwin) - viewPtr->inset;
    y1 = viewPtr->inset + viewPtr->titleHeight;
    y2 = Tk_Height(viewPtr->tkwin) - viewPtr->inset - INSET_PAD;

    /* Verify that the label is currently visible on screen. */
    if (((x + w) <  x1) || (x > x2) || ((y + h) < y1) || (y > y2)) {
	return 0;
    }
    r.x = x1;
    r.y = y1;
    r.width = x2 - x1;
    r.height = y2 - y1; 
    rgn = TkCreateRegion();
    TkUnionRectWithRegion(&r, rgn, rgn);

    /* Clear the entry label background. */
    Blt_Bg_SetClipRegion(viewPtr->tkwin, bg, rgn);
    Blt_Bg_FillRectangle(viewPtr->tkwin, drawable, bg, x, y, w, h, 0, 
	TK_RELIEF_FLAT);
    Blt_Bg_UnsetClipRegion(viewPtr->tkwin, bg);
    DrawLabel(viewPtr, entryPtr, drawable, x, y, xMax - x, rgn);
    TkDestroyRegion(rgn);
    return 1;
}


static void
DisplayButton(TreeView *viewPtr, Entry *entryPtr)
{
    Drawable drawable;
    int sx, sy, dx, dy;
    int width, height;
    int left, right, top, bottom;

    dx = SCREENX(viewPtr, entryPtr->worldX) + entryPtr->buttonX;
    dy = SCREENY(viewPtr, entryPtr->worldY) + entryPtr->buttonY;
    width = viewPtr->button.width;
    height = viewPtr->button.height;

    top = viewPtr->titleHeight + viewPtr->inset;
    bottom = Tk_Height(viewPtr->tkwin) - viewPtr->inset;
    left = viewPtr->inset;
    right = Tk_Width(viewPtr->tkwin) - viewPtr->inset;

    if (((dx + width) < left) || (dx > right) ||
	((dy + height) < top) || (dy > bottom)) {
	return;			/* Value is clipped. */
    }
    drawable = Blt_GetPixmap(viewPtr->display, Tk_WindowId(viewPtr->tkwin), 
	width, height, Tk_Depth(viewPtr->tkwin));
    /* Draw the background of the value. */
    DrawButton(viewPtr, entryPtr, drawable, 0, 0);

    /* Clip the drawable if necessary */
    sx = sy = 0;
    if (dx < left) {
	width -= left - dx;
	sx += left - dx;
	dx = left;
    }
    if ((dx + width) >= right) {
	width -= (dx + width) - right;
    }
    if (dy < top) {
	height -= top - dy;
	sy += top - dy;
	dy = top;
    }
    if ((dy + height) >= bottom) {
	height -= (dy + height) - bottom;
    }
    XCopyArea(viewPtr->display, drawable, Tk_WindowId(viewPtr->tkwin), 
      viewPtr->lineGC, sx, sy, width,  height, dx, dy);
    Tk_FreePixmap(viewPtr->display, drawable);
}


/*
 *---------------------------------------------------------------------------
 *
 * BindOp --
 *
 *	  .t bind tagOrId sequence command
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
BindOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    ClientData object;
    Entry *entryPtr;
    const char *string;
    long inode;

    /*
     * Entries are selected by id only.  All other strings are interpreted as
     * a binding tag.
     */
    string = Tcl_GetString(objv[2]);
    if ((isdigit(UCHAR(string[0]))) && 
	(Blt_GetLongFromObj(viewPtr->interp, objv[2], &inode) == TCL_OK)) {
	Blt_TreeNode node;

	node = Blt_Tree_GetNode(viewPtr->tree, inode);
	object = NodeToEntry(viewPtr, node);
    } else if (GetEntryFromSpecialId(viewPtr, string, &entryPtr) == TCL_OK) {
	if (entryPtr != NULL) {
	    return TCL_OK;	/* Special id doesn't currently exist. */
	}
	object = entryPtr;
    } else {
	/* Assume that this is a binding tag. */
	object = EntryTag(viewPtr, string);
    } 
    return Blt_ConfigureBindingsFromObj(interp, viewPtr->bindTable, object, 
	 objc - 3, objv + 3);
}


/*
 *---------------------------------------------------------------------------
 *
 * BboxOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
BboxOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int i;
    Entry *entryPtr;
    int x1, y1, x2, y2;
    int screen;
    const char *string;
    Tcl_Obj *listObjPtr;

    if (viewPtr->flags & (DIRTY | LAYOUT_PENDING)) {
	/*
	 * The layout is dirty.  Recompute it now, before we use the world
	 * dimensions.  But remember, the "bbox" operation isn't valid for
	 * hidden entries (since they're not visible, they don't have world
	 * coordinates).
	 */
	ComputeLayout(viewPtr);
    }
    x1 = viewPtr->worldWidth;
    y1 = viewPtr->worldHeight;
    x2 = y2 = 0;

    screen = FALSE;
    string = Tcl_GetString(objv[2]);
    if ((string[0] == '-') && (strcmp(string, "-screen") == 0)) {
	screen = TRUE;
	objc--, objv++;
    }
    for (i = 2; i < objc; i++) {
	int x, h, d;
	const char *string;
	int yBot;

	string = Tcl_GetString(objv[i]);
	if ((string[0] == 'a') && (strcmp(string, "all") == 0)) {
	    x1 = y1 = 0;
	    x2 = viewPtr->worldWidth;
	    y2 = viewPtr->worldHeight;
	    break;
	}
	if (GetEntryFromObj(viewPtr, objv[i], &entryPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (entryPtr == NULL) {
	    continue;
	}
	if (entryPtr->flags & ENTRY_HIDE) {
	    continue;
	}
	yBot = entryPtr->worldY + entryPtr->height;
	h = VPORTHEIGHT(viewPtr);
	if ((yBot <= viewPtr->yOffset) &&
	    (entryPtr->worldY >= (viewPtr->yOffset + h))) {
	    continue;
	}
	if (y2 < yBot) {
	    y2 = yBot;
	}
	if (y1 > entryPtr->worldY) {
	    y1 = entryPtr->worldY;
	}
	d = DEPTH(viewPtr, entryPtr->node);
	x = entryPtr->worldX + ICONWIDTH(d) + ICONWIDTH(d + 1);
	if (x2 < (x + entryPtr->width)) {
	    x2 = x + entryPtr->width;
	}
	if (x1 > x) {
	    x1 = x;
	}
    }
    if (screen) {
	int w, h;

	w = VPORTWIDTH(viewPtr);
	h = VPORTHEIGHT(viewPtr);

	/*
	 * Do a min-max text for the intersection of the viewport and the
	 * computed bounding box.  If there is no intersection, return the
	 * empty string.
	 */
	if ((x2 < viewPtr->xOffset) || (y2 < viewPtr->yOffset) ||
	    (x1 >= (viewPtr->xOffset + w)) || (y1 >= (viewPtr->yOffset + h))) {
	    return TCL_OK;
	}
	x1 = SCREENX(viewPtr, x1);
	y1 = SCREENY(viewPtr, y1);
	x2 = SCREENX(viewPtr, x2);
	y2 = SCREENY(viewPtr, y2);
    }

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewIntObj(x1));
    Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewIntObj(y1));
    Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewIntObj(x2 - x1 + 1));
    Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewIntObj(y2 - y1 + 1));
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonActivateOp --
 *
 *	Selects the button to appear active.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ButtonActivateOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *oldPtr, *newPtr;
    char *string;

    string = Tcl_GetString(objv[3]);
    if (string[0] == '\0') {
	newPtr = NULL;
    } else if (GetEntryFromObj(viewPtr, objv[3], &newPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (viewPtr->treeColumn.flags & COLUMN_HIDDEN) {
	return TCL_OK;
    }
    if ((newPtr != NULL) && !(newPtr->flags & ENTRY_HAS_BUTTON)) {
	newPtr = NULL;
    }
    oldPtr = viewPtr->activeBtnPtr;
    viewPtr->activeBtnPtr = newPtr;
    if (!(viewPtr->flags & REDRAW_PENDING) && (newPtr != oldPtr)) {
	if ((oldPtr != NULL) && (oldPtr != viewPtr->rootPtr)) {
	    DisplayButton(viewPtr, oldPtr);
	}
	if ((newPtr != NULL) && (newPtr != viewPtr->rootPtr)) {
	    DisplayButton(viewPtr, newPtr);
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonBindOp --
 *
 *	  .t bind tag sequence command
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ButtonBindOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	     Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    ClientData object;
    char *string;

    string = Tcl_GetString(objv[3]);
    /* Assume that this is a binding tag. */
    object = ButtonTag(viewPtr, string);
    return Blt_ConfigureBindingsFromObj(interp, viewPtr->bindTable, object, 
	objc - 4, objv + 4);
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonCgetOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ButtonCgetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	     Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;

    return Blt_ConfigureValueFromObj(interp, viewPtr->tkwin, buttonSpecs, 
	(char *)viewPtr, objv[3], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonConfigureOp --
 *
 * 	This procedure is called to process a list of configuration options
 * 	database, in order to reconfigure the one of more entries in the
 * 	widget.
 *
 *	  .h button configure option value
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font, etc. get
 *	set for viewPtr; old resources get freed, if there were any.  The
 *	hypertext is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static int
ButtonConfigureOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		  Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;

    if (objc == 3) {
	return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, buttonSpecs, 
		(char *)viewPtr, (Tcl_Obj *)NULL, 0);
    } else if (objc == 4) {
	return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, buttonSpecs, 
		(char *)viewPtr, objv[3], 0);
    }
    iconsOption.clientData = viewPtr;
    if (Blt_ConfigureWidgetFromObj(viewPtr->interp, viewPtr->tkwin, buttonSpecs,
	 objc - 3, objv + 3, (char *)viewPtr, BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	return TCL_ERROR;
    }
    ConfigureButtons(viewPtr);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ButtonOp --
 *
 *	This procedure handles button operations.
 *
 * Results:
 *	A standard TCL result.
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec buttonOps[] =
{
    {"activate",  1, ButtonActivateOp,  4, 4, "tagOrId",},
    {"bind",      1, ButtonBindOp,      4, 6, "tagName ?sequence command?",},
    {"cget",      2, ButtonCgetOp,      4, 4, "option",},
    {"configure", 2, ButtonConfigureOp, 3, 0, "?option value?...",},
    {"highlight", 1, ButtonActivateOp,  4, 4, "tagOrId",},
};

static int numButtonOps = sizeof(buttonOps) / sizeof(Blt_OpSpec);

static int
ButtonOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	 Tcl_Obj *const *objv)
{
    Tcl_ObjCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, numButtonOps, buttonOps, BLT_OP_ARG2, objc, 
	objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc)(clientData, interp, objc, objv);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * CgetOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
CgetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;

    return Blt_ConfigureValueFromObj(interp, viewPtr->tkwin, viewSpecs,
	(char *)viewPtr, objv[2], 0);
}


/*ARGSUSED*/
static int
ChrootOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    long inode;

    if (objc == 3) {
	Entry *entryPtr;

	if (GetEntryFromObj(viewPtr, objv[2], &entryPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	viewPtr->flags |= (LAYOUT_PENDING | DIRTY | REPOPULATE);
	viewPtr->rootPtr = entryPtr;
	EventuallyRedraw(viewPtr);
    }
    inode = Blt_Tree_NodeId(viewPtr->rootPtr->node);
    Tcl_SetLongObj(Tcl_GetObjResult(interp), inode);
    return TCL_OK;
}

/*ARGSUSED*/
static int
CloseOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    TagIterator iter;
    int recurse, result;
    int i;

    recurse = FALSE;

    if (objc > 2) {
	char *string;
	int length;

	string = Tcl_GetStringFromObj(objv[2], &length);
	if ((string[0] == '-') && (length > 1) && 
	    (strncmp(string, "-recurse", length) == 0)) {
	    objv++, objc--;
	    recurse = TRUE;
	}
    }
    for (i = 2; i < objc; i++) {
	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    /* 
	     * Clear the selections for any entries that may have become
	     * hidden by closing the node.
	     */
	    PruneSelection(viewPtr, entryPtr);
	    
	    /*
	     *  Check if either the "focus" entry or selection anchor is in
	     *  this hierarchy.  Must move it or disable it before we close
	     *  the node.  Otherwise it may be deleted by a TCL "close"
	     *  script, and we'll be left pointing to a bogus memory location.
	     */
	    if ((viewPtr->focusPtr != NULL) && 
		(Blt_Tree_IsAncestor(entryPtr->node, viewPtr->focusPtr->node))){
		viewPtr->focusPtr = entryPtr;
		Blt_SetFocusItem(viewPtr->bindTable, viewPtr->focusPtr, 
			ITEM_ENTRY);
	    }
	    if ((viewPtr->selection.anchorPtr != NULL) && 
		(Blt_Tree_IsAncestor(entryPtr->node, 
			viewPtr->selection.anchorPtr->node))) {
		viewPtr->selection.markPtr = viewPtr->selection.anchorPtr=NULL;
	    }
	    if ((viewPtr->activePtr != NULL) && 
		(Blt_Tree_IsAncestor(entryPtr->node,viewPtr->activePtr->node))){
		viewPtr->activePtr = entryPtr;
	    }
	    if (recurse) {
		result = Apply(viewPtr, entryPtr, CloseEntry, 0);
	    } else {
		result = CloseEntry(viewPtr, entryPtr);
	    }
	    if (result != TCL_OK) {
		return TCL_ERROR;
	    }	
	}
    }
    /* Closing a node may affect the visible entries and the the world layout
     * of the entries. */
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY /*| RESORT */);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnActivateOp --
 *
 *	Selects the button to appear active.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnActivateOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Column *colPtr, *activePtr;
    
    if (GetColumn(interp, viewPtr, objv[3], &colPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (colPtr == NULL) {
	return TCL_OK;
    }
    if ((colPtr->flags & COLUMN_HIDDEN) || (colPtr->state == STATE_DISABLED)) {
	return TCL_OK;
    }
    activePtr = viewPtr->colActiveTitlePtr;
    viewPtr->colActiveTitlePtr = viewPtr->colActivePtr = colPtr;

    /* If we aren't already queued to redraw the widget, try to directly draw
     * into window. */
    if ((viewPtr->flags & REDRAW_PENDING) == 0) {
	Drawable drawable;

	drawable = Tk_WindowId(viewPtr->tkwin);
	if (activePtr != NULL) {
	    DisplayColumnTitle(viewPtr, activePtr, drawable);
	}
	DisplayColumnTitle(viewPtr, colPtr, drawable);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnBindOp --
 *
 *	  .t bind tag sequence command
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnBindOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	     Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    ClientData object;
    Column *colPtr;

    if ((GetColumn(NULL, viewPtr, objv[3], &colPtr) == TCL_OK) && 
	(colPtr != NULL)) {
	object = ColumnTag(viewPtr, colPtr->key);
    } else {
	object = ColumnTag(viewPtr, Tcl_GetString(objv[3]));
    }
    return Blt_ConfigureBindingsFromObj(interp, viewPtr->bindTable, object,
	objc - 4, objv + 4);
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnCgetOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnCgetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	     Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Column *colPtr;

    if (GetColumn(interp, viewPtr, objv[3], &colPtr) != TCL_OK){
	return TCL_ERROR;
    }
    if (colPtr == NULL) {
	return TCL_OK;
    }
    return Blt_ConfigureValueFromObj(interp, viewPtr->tkwin, columnSpecs, 
	(char *)colPtr, objv[4], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnConfigureOp --
 *
 * 	This procedure is called to process a list of configuration
 *	options database, in order to reconfigure the one of more
 *	entries in the widget.
 *
 *	  .h entryconfigure node node node node option value
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font,
 *	etc. get set for viewPtr; old resources get freed, if there
 *	were any.  The hypertext is redisplayed.
 *
 *	.tv column configure col ?option value?
 *---------------------------------------------------------------------------
 */
static int
ColumnConfigureOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		  Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Column *colPtr;

    uidOption.clientData = viewPtr;
    iconOption.clientData = viewPtr;
    styleOption.clientData = viewPtr;

    if (GetColumn(interp, viewPtr, objv[3], &colPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (colPtr == NULL) {
	return TCL_OK;
    }
    if (objc == 4) {
	return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, columnSpecs, 
		(char *)colPtr, (Tcl_Obj *)NULL, 0);
    } else if (objc == 5) {
	return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, columnSpecs, 
		(char *)colPtr, objv[4], 0);
    }
    if (Blt_ConfigureWidgetFromObj(viewPtr->interp, viewPtr->tkwin, columnSpecs,
	objc - 4, objv + 4, (char *)colPtr, BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	return TCL_ERROR;
    }
    ConfigureColumn(viewPtr, colPtr);

    /*FIXME: Makes every change redo everything. */
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnDeactivateOp --
 *
 *	Deactivates all columns.  All column titles will be redraw in their
 *	normal foreground/background colors.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnDeactivateOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		   Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Column *activePtr;

    activePtr = viewPtr->colActiveTitlePtr;
    viewPtr->colActiveTitlePtr = viewPtr->colActivePtr = NULL;
    /* If we aren't already queued to redraw the widget, try to directly draw
     * into window. */
    if ((viewPtr->flags & REDRAW_PENDING) == 0) {
	if (activePtr != NULL) {
	    Drawable drawable;

	    drawable = Tk_WindowId(viewPtr->tkwin);
	    DisplayColumnTitle(viewPtr, activePtr, drawable);
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnDeleteOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnDeleteOp(ClientData clientData, Tcl_Interp *interp, int objc,
	       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int i;

    for (i = 3; i < objc; i++) {
	Column *colPtr;
	Entry *entryPtr;

	if (GetColumn(interp, viewPtr, objv[i], &colPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (colPtr == NULL) {
	    continue;
	}
	if (colPtr == &viewPtr->treeColumn) {
	    continue;			/* Can't delete the treeView column,
					 * so just ignore the request. */
	}
	/* Traverse the tree deleting values associated with the column.  */
	for (entryPtr = viewPtr->rootPtr; entryPtr != NULL;
	    entryPtr = NextEntry(entryPtr, 0)) {
	    if (entryPtr != NULL) {
		Value *valuePtr, *lastPtr, *nextPtr;
		
		lastPtr = NULL;
		for (valuePtr = entryPtr->values; valuePtr != NULL; 
		     valuePtr = nextPtr) {
		    nextPtr = valuePtr->nextPtr;
		    if (valuePtr->columnPtr == colPtr) {
			DestroyValue(viewPtr, valuePtr);
			if (lastPtr == NULL) {
			    entryPtr->values = nextPtr;
			} else {
			    lastPtr->nextPtr = nextPtr;
			}
			break;
		    }
		    lastPtr = valuePtr;
		}
	    }
	}
	DestroyColumn(colPtr);
    }
    /* Deleting a column may affect the height of an entry. */
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY /*| RESORT */);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnExistsOp --
 *
 *	.tv column exists $field
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnExistsOp(ClientData clientData, Tcl_Interp *interp, int objc,
	       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int exists;
    Column *colPtr;

    exists = FALSE;
    if ((GetColumn(NULL, viewPtr, objv[3], &colPtr) == TCL_OK) &&
	(colPtr != NULL)) {
	exists = TRUE;
    }
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), exists);
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * ColumnIndexOp --
 *
 *	Returns the index of the column.
 *
 *	.tv column index column
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnIndexOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	      Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    long index;
    Column *colPtr;

    index = -1;
    if ((GetColumn(NULL, viewPtr, objv[3], &colPtr) == TCL_OK) &&
	(colPtr != NULL)) {
	index = colPtr->index;
    }
    Tcl_SetLongObj(Tcl_GetObjResult(interp), index);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnInsertOp --
 *
 *	Add new columns to the tree.
 *
 *	.tv column insert position name ?option values?
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnInsertOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Blt_ChainLink before;
    long insertPos;
    Column *colPtr;
    Entry *entryPtr;

    if (Blt_GetPositionFromObj(viewPtr->interp, objv[3], &insertPos) != TCL_OK){
	return TCL_ERROR;
    }
    if ((insertPos == -1) || 
	(insertPos >= Blt_Chain_GetLength(viewPtr->columns))) {
	before = NULL;		/* Insert at end of list. */
    } else {
	before =  Blt_Chain_GetNthLink(viewPtr->columns, insertPos);
    }
    if (GetColumn(NULL, viewPtr, objv[4], &colPtr) == TCL_OK) {
	Tcl_AppendResult(interp, "column \"", Tcl_GetString(objv[4]), 
			 "\" already exists", (char *)NULL);
	return TCL_ERROR;
    }
    colPtr = CreateColumn(viewPtr, objv[4], objc - 5, objv + 5);
    if (colPtr == NULL) {
	return TCL_ERROR;
    }
    if (before == NULL) {
	colPtr->link = Blt_Chain_Append(viewPtr->columns, colPtr);
    } else {
	colPtr->link = Blt_Chain_NewLink();
	Blt_Chain_SetValue(colPtr->link, colPtr);
	Blt_Chain_LinkBefore(viewPtr->columns, colPtr->link, before);
    }
    /* 
     * Traverse the tree adding column entries where needed.
     */
    for(entryPtr = viewPtr->rootPtr; entryPtr != NULL;
	entryPtr = NextEntry(entryPtr, 0)) {
	AddValue(entryPtr, colPtr);
    }
    TraceColumn(viewPtr, colPtr);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnCurrentOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnCurrentOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Column *colPtr;

    colPtr = GetCurrentColumn(viewPtr);
    if (colPtr != NULL) {
	Tcl_SetStringObj(Tcl_GetObjResult(interp), colPtr->key, -1);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnInvokeOp --
 *
 * 	This procedure is called to invoke a column command.
 *
 *	  .h column invoke columnName
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnInvokeOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Column *colPtr;
    char *string;
    Tcl_Obj *cmdObjPtr;

    string = Tcl_GetString(objv[3]);
    if (string[0] == '\0') {
	return TCL_OK;
    }
    if (GetColumn(interp, viewPtr, objv[3], &colPtr) != TCL_OK){
	return TCL_ERROR;
    }
    if (colPtr == NULL) {
	return TCL_OK;
    }
    cmdObjPtr = colPtr->cmdObjPtr;
    if (cmdObjPtr == NULL) {
	cmdObjPtr = viewPtr->colCmdObjPtr;
    }
    if ((colPtr->state == STATE_NORMAL) && (cmdObjPtr != NULL)) {
	int result;
	Tcl_Obj *objPtr;

	cmdObjPtr = Tcl_DuplicateObj(cmdObjPtr);
	objPtr = Tcl_NewStringObj(Tk_PathName(viewPtr->tkwin), -1);
	Tcl_ListObjAppendElement(viewPtr->interp, cmdObjPtr, objPtr);
	objPtr = Tcl_NewStringObj(colPtr->key, -1);
	Tcl_ListObjAppendElement(viewPtr->interp, cmdObjPtr, objPtr);
	Tcl_Preserve(viewPtr);
	Tcl_Preserve(colPtr);
	Tcl_IncrRefCount(cmdObjPtr);
	result = Tcl_EvalObjEx(viewPtr->interp, cmdObjPtr, TCL_EVAL_GLOBAL);
	Tcl_DecrRefCount(cmdObjPtr);
	Tcl_Release(colPtr);
	Tcl_Release(viewPtr);
	return result;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnMoveOp --
 *
 *	Move a column.
 *
 * .h column move field1 position
 *---------------------------------------------------------------------------
 */

/*
 *---------------------------------------------------------------------------
 *
 * ColumnNamesOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnNamesOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	      Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Blt_ChainLink link;
    Tcl_Obj *listObjPtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for(link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Column *colPtr;
	Tcl_Obj *objPtr;

	colPtr = Blt_Chain_GetValue(link);

	objPtr = Tcl_NewStringObj(colPtr->key, -1);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*ARGSUSED*/
static int
ColumnNearestOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int x, y;			/* Screen coordinates of the test point. */
    Column *colPtr;
    ClientData hint;
    int checkTitle;
#ifdef notdef
    int isRoot;

    isRoot = FALSE;
    string = Tcl_GetString(objv[3]);

    if (strcmp("-root", string) == 0) {
	isRoot = TRUE;
	objv++, objc--;
    }
    if (objc != 5) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), " ", Tcl_GetString(objv[1]), 
		Tcl_GetString(objv[2]), " ?-root? x y\"", (char *)NULL);
	return TCL_ERROR;
			 
    }
#endif
    if (Tk_GetPixelsFromObj(interp, viewPtr->tkwin, objv[3], &x) != TCL_OK) {
	return TCL_ERROR;
    } 
    y = 0;
    checkTitle = FALSE;
    if (objc == 5) {
	if (Tk_GetPixelsFromObj(interp, viewPtr->tkwin, objv[4], &y) != TCL_OK) {
	    return TCL_ERROR;
	}
	checkTitle = TRUE;
    }
    colPtr = NearestColumn(viewPtr, x, y, &hint);
    if ((checkTitle) && (hint == NULL)) {
	colPtr = NULL;
    }
    if (colPtr != NULL) {
	Tcl_SetStringObj(Tcl_GetObjResult(interp), colPtr->key, -1);
    }
    return TCL_OK;
}

static void
UpdateMark(TreeView *viewPtr, int newMark)
{
    Drawable drawable;
    Column *cp;
    int dx;
    int width;

    cp = viewPtr->colResizePtr;
    if (cp == NULL) {
	return;
    }
    drawable = Tk_WindowId(viewPtr->tkwin);
    if (drawable == None) {
	return;
    }

    /* Erase any existing rule. */
    if (viewPtr->flags & RULE_ACTIVE_COLUMN) { 
	DrawRule(viewPtr, cp, drawable);
    }
    
    dx = newMark - viewPtr->ruleAnchor; 
    width = cp->width - (PADDING(cp->pad) + 2 * cp->borderWidth);
    if ((cp->reqMin > 0) && ((width + dx) < cp->reqMin)) {
	dx = cp->reqMin - width;
    }
    if ((cp->reqMax > 0) && ((width + dx) > cp->reqMax)) {
	dx = cp->reqMax - width;
    }
    if ((width + dx) < 4) {
	dx = 4 - width;
    }
    viewPtr->ruleMark = viewPtr->ruleAnchor + dx;

    /* Redraw the rule if required. */
    if (viewPtr->flags & COLUMN_RULE_NEEDED) {
	DrawRule(viewPtr, cp, drawable);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnResizeActivateOp --
 *
 *	Turns on/off the resize cursor.
 *
 *	$t column resize activate $col
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnResizeActivateOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Column *colPtr;

    if (GetColumn(interp, viewPtr, objv[4], &colPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (viewPtr->resizeCursor != None) {
	Tk_DefineCursor(viewPtr->tkwin, viewPtr->resizeCursor);
    } 
    viewPtr->colResizePtr = colPtr;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnResizeAnchorOp --
 *
 *	Set the anchor for the resize.
 *
 *	$t column resize anchor $x
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnResizeAnchorOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		     Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int x;

    if (Tcl_GetIntFromObj(NULL, objv[4], &x) != TCL_OK) {
	return TCL_ERROR;
    } 
    viewPtr->ruleAnchor = x;
    viewPtr->flags |= COLUMN_RULE_NEEDED;
    UpdateMark(viewPtr, x);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnResizeDectivateOp --
 *
 *	Turns off the resize cursor.
 *
 *	$t column resize deactivate
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnResizeDeactivateOp(ClientData clientData, Tcl_Interp *interp, int objc, 
			 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    if (viewPtr->cursor != None) {
	Tk_DefineCursor(viewPtr->tkwin, viewPtr->cursor);
    } else {
	Tk_UndefineCursor(viewPtr->tkwin);
    }
    viewPtr->colResizePtr = NULL;
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnResizeMarkOp --
 *
 *	Sets the resize mark.  The distance between the mark and the anchor
 *	is the delta to change the width of the active column.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnResizeMarkOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		   Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int x;

    if (Tcl_GetIntFromObj(NULL, objv[4], &x) != TCL_OK) {
	return TCL_ERROR;
    } 
    viewPtr->flags |= COLUMN_RULE_NEEDED;
    UpdateMark(viewPtr, x);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColumnResizeSetOp --
 *
 *	Returns the new width of the column including the resize delta.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ColumnResizeSetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		  Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    viewPtr->flags &= ~COLUMN_RULE_NEEDED;
    UpdateMark(viewPtr, viewPtr->ruleMark);
    if (viewPtr->colResizePtr != NULL) {
	int width, delta;
	Column *colPtr;

	colPtr = viewPtr->colResizePtr;
	delta = (viewPtr->ruleMark - viewPtr->ruleAnchor);
	width = viewPtr->colResizePtr->width + delta - 
	    (PADDING(colPtr->pad) + 2 * colPtr->borderWidth) - 1;
	Tcl_SetIntObj(Tcl_GetObjResult(interp), width);
    }
    return TCL_OK;
}

static Blt_OpSpec columnResizeOps[] =
{ 
    {"activate",   2, ColumnResizeActivateOp,   5, 5, "column",},
    {"anchor",     2, ColumnResizeAnchorOp,     5, 5, "x",},
    {"deactivate", 1, ColumnResizeDeactivateOp, 4, 4, "",},
    {"mark",       1, ColumnResizeMarkOp,       5, 5, "x",},
    {"set",        1, ColumnResizeSetOp,        4, 4, "",},
};

static int numColumnResizeOps = sizeof(columnResizeOps) / sizeof(Blt_OpSpec);

/*
 *---------------------------------------------------------------------------
 *
 * ColumnResizeOp --
 *
 *---------------------------------------------------------------------------
 */
static int
ColumnResizeOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    Tcl_ObjCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, numColumnResizeOps, columnResizeOps, 
	BLT_OP_ARG3, objc, objv,0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (clientData, interp, objc, objv);
    return result;
}


static Blt_OpSpec columnOps[] =
{
    {"activate",   1, ColumnActivateOp,   4, 4, "field",},
    {"bind",       1, ColumnBindOp,       4, 6, "tagName ?sequence command?",},
    {"cget",       2, ColumnCgetOp,       5, 5, "field option",},
    {"configure",  2, ColumnConfigureOp,  4, 0, "field ?option value?...",},
    {"current",    2, ColumnCurrentOp,    3, 3, "",},
    {"deactivate", 3, ColumnDeactivateOp, 3, 3, "",},
    {"delete",     3, ColumnDeleteOp,     3, 0, "?field...?",},
    {"exists",     1, ColumnExistsOp,     4, 4, "field",},
    {"index",      3, ColumnIndexOp,      4, 4, "field",},
    {"insert",     3, ColumnInsertOp,     5, 0, 
	"position field ?field...? ?option value?...",},
    {"invoke",     3, ColumnInvokeOp,     4, 4, "field",},
    {"names",      2, ColumnNamesOp,      3, 3, "",},
    {"nearest",    2, ColumnNearestOp,    4, 5, "x ?y?",},
    {"resize",     1, ColumnResizeOp,     3, 0, "arg",},
};
static int numColumnOps = sizeof(columnOps) / sizeof(Blt_OpSpec);

/*
 *---------------------------------------------------------------------------
 *
 * ColumnOp --
 *
 *---------------------------------------------------------------------------
 */
static int
ColumnOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	 Tcl_Obj *const *objv)
{
    Tcl_ObjCmdProc *proc;

    int result;
    proc = Blt_GetOpFromObj(interp, numColumnOps, columnOps, BLT_OP_ARG2, 
	objc, objv,0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (clientData, interp, objc, objv);
    return result;
}


/* StartCmd */

/*
 * bltTvCmd.c --
 *
 * This module implements an hierarchy widget for the BLT toolkit.
 *
 *	Copyright 1998-2004 George A Howlett.
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

/*
 * TODO:
 *
 * BUGS:
 *   1.  "open" operation should change scroll offset so that as many
 *	 new entries (up to half a screen) can be seen.
 *   2.  "open" needs to adjust the scrolloffset so that the same entry
 *	 is seen at the same place.
 */

/*
 *---------------------------------------------------------------------------
 *
 * TreeView operations
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
FocusOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    long inode;

    if (objc == 3) {
	Entry *entryPtr;

	if (GetEntryFromObj(viewPtr, objv[2], &entryPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	if ((entryPtr != NULL) && (entryPtr != viewPtr->focusPtr)) {
	    if (entryPtr->flags & ENTRY_HIDE) {
		/* Doesn't make sense to set focus to a node you can't see. */
		MapAncestors(viewPtr, entryPtr);
	    }
	    /* Changing focus can only affect the visible entries.  The entry
	     * layout stays the same. */
	    if (viewPtr->focusPtr != NULL) {
		viewPtr->focusPtr->flags |= ENTRY_REDRAW;
	    } 
	    entryPtr->flags |= ENTRY_REDRAW;
	    viewPtr->flags |= SCROLL_PENDING;
	    viewPtr->focusPtr = entryPtr;
	}
	EventuallyRedraw(viewPtr);
    }
    Blt_SetFocusItem(viewPtr->bindTable, viewPtr->focusPtr, ITEM_ENTRY);
    inode = -1;
    if (viewPtr->focusPtr != NULL) {
	inode = Blt_Tree_NodeId(viewPtr->focusPtr->node);
    }
    Tcl_SetLongObj(Tcl_GetObjResult(interp), inode);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureOp --
 *
 * 	This procedure is called to process an objv/objc list, plus the Tk
 * 	option database, in order to configure (or reconfigure) the widget.
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font, etc. get
 *	set for viewPtr; old resources get freed, if there were any.  The widget
 *	is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static int
ConfigureOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	    Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;

    if (objc == 2) {
	return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, 
		viewSpecs, (char *)viewPtr, (Tcl_Obj *)NULL, 0);
    } else if (objc == 3) {
	return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, 
		viewSpecs, (char *)viewPtr, objv[2], 0);
    } 
    iconsOption.clientData = viewPtr;
    treeOption.clientData = viewPtr;
    if (Blt_ConfigureWidgetFromObj(interp, viewPtr->tkwin, viewSpecs, 
	objc - 2, objv + 2, (char *)viewPtr, BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	return TCL_ERROR;
    }
    if (ConfigureTreeView(interp, viewPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*ARGSUSED*/
static int
CurselectionOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Tcl_Obj *listObjPtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    if (viewPtr->selection.flags & SELECT_SORTED) {
	Blt_ChainLink link;

	for (link = Blt_Chain_FirstLink(viewPtr->selection.list); link != NULL;
	     link = Blt_Chain_NextLink(link)) {
	    Entry *entryPtr;
	    Tcl_Obj *objPtr;

	    entryPtr = Blt_Chain_GetValue(link);
	    objPtr = NodeToObj(entryPtr->node);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
			
	}
    } else {
	Entry *entryPtr;

	for (entryPtr = viewPtr->rootPtr; entryPtr != NULL; 
	     entryPtr = NextEntry(entryPtr, ENTRY_MASK)) {

	    if (EntryIsSelected(viewPtr, entryPtr)) {
		Tcl_Obj *objPtr;

		objPtr = NodeToObj(entryPtr->node);
		Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	    }
	}
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*ARGSUSED*/
static int
EditOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    char *string;
    int isRoot, isTest;
    int x, y;

    isRoot = isTest = FALSE;
    string = Tcl_GetString(objv[2]);
    if (strcmp("-root", string) == 0) {
	isRoot = TRUE;
	objv++, objc--;
    }
    string = Tcl_GetString(objv[2]);
    if (strcmp("-test", string) == 0) {
	isTest = TRUE;
	objv++, objc--;
    }
    if (objc != 4) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), " ", Tcl_GetString(objv[1]), 
			" ?-root? x y\"", (char *)NULL);
	return TCL_ERROR;
			 
    }
    if ((Tcl_GetIntFromObj(interp, objv[2], &x) != TCL_OK) ||
	(Tcl_GetIntFromObj(interp, objv[3], &y) != TCL_OK)) {
	return TCL_ERROR;
    }
    if (isRoot) {
	int rootX, rootY;

	Tk_GetRootCoords(viewPtr->tkwin, &rootX, &rootY);
	x -= rootX;
	y -= rootY;
    }
    entryPtr = NearestEntry(viewPtr, x, y, FALSE);
    if (entryPtr != NULL) {
	Blt_ChainLink link;
	int worldX;

	worldX = WORLDX(viewPtr, x);
	for (link = Blt_Chain_FirstLink(viewPtr->columns); link != NULL; 
	     link = Blt_Chain_NextLink(link)) {
	    Column *columnPtr;

	    columnPtr = Blt_Chain_GetValue(link);
	    if (columnPtr->flags & COLUMN_READONLY) {
		continue;		/* Column isn't editable. */
	    }
	    if ((worldX >= columnPtr->worldX) && 
		(worldX < (columnPtr->worldX + columnPtr->width))) {
		ColumnStyle *stylePtr;
	
		stylePtr = NULL;
		if (columnPtr != &viewPtr->treeColumn) {
		    Value *valuePtr;
		
		    valuePtr = Blt_TreeView_FindValue(entryPtr, columnPtr);
		    if (valuePtr == NULL) {
			continue;
		    }
		    stylePtr = valuePtr->stylePtr;
		} 
		if (stylePtr == NULL) {
		    stylePtr = columnPtr->stylePtr;
		}
		if ((columnPtr->flags & COLUMN_READONLY) || 
		     (stylePtr->classPtr->editProc == NULL)) {
		    continue;
		}
		if (!isTest) {
		    if ((*stylePtr->classPtr->editProc)(entryPtr, columnPtr, 
			stylePtr) != TCL_OK) {
			return TCL_ERROR;
		    }
		    EventuallyRedraw(viewPtr);
		}
		Tcl_SetBooleanObj(Tcl_GetObjResult(interp), TRUE);
		return TCL_OK;
	    }
	}
    }
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), FALSE);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * EntryActivateOp --
 *
 *	Selects the entry to appear active.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EntryActivateOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *newPtr, *oldPtr;
    char *string;

    string = Tcl_GetString(objv[3]);
    if (string[0] == '\0') {
	newPtr = NULL;
    } else if (GetEntryFromObj(viewPtr, objv[3], &newPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (viewPtr->treeColumn.flags & COLUMN_HIDDEN) {
	return TCL_OK;
    }
    oldPtr = viewPtr->activePtr;
    viewPtr->activePtr = newPtr;
    if (!(viewPtr->flags & REDRAW_PENDING) && (newPtr != oldPtr)) {
	Drawable drawable;
	drawable = Tk_WindowId(viewPtr->tkwin);
	if (oldPtr != NULL) {
	    DisplayLabel(viewPtr, oldPtr, drawable);
	}
	if (newPtr != NULL) {
	    DisplayLabel(viewPtr, newPtr, drawable);
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * EntryCgetOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EntryCgetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	    Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;

    if (GetEntry(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    return Blt_ConfigureValueFromObj(interp, viewPtr->tkwin, entrySpecs, 
	(char *)entryPtr, objv[4], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * EntryConfigureOp --
 *
 * 	This procedure is called to process a list of configuration options
 * 	database, in order to reconfigure the one of more entries in the
 * 	widget.
 *
 *	  .h entryconfigure node node node node option value
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font, etc. get
 *	set for viewPtr; old resources get freed, if there were any.  The
 *	hypertext is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static int
EntryConfigureOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int numIds, configObjc;
    Tcl_Obj *const *configObjv;
    int i;
    Entry *entryPtr;
    TagIterator iter;
    char *string;

    /* Figure out where the option value pairs begin */
    objc -= 3, objv += 3;
    for (i = 0; i < objc; i++) {
	string = Tcl_GetString(objv[i]);
	if (string[0] == '-') {
	    break;
	}
    }
    numIds = i;			/* # of tags or ids specified */
    configObjc = objc - i;	/* # of options specified */
    configObjv = objv + i;	/* Start of options in objv  */

    iconsOption.clientData = viewPtr;
    uidOption.clientData = viewPtr;

    for (i = 0; i < numIds; i++) {
	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    if (configObjc == 0) {
		return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, 
			entrySpecs, (char *)entryPtr, (Tcl_Obj *)NULL, 0);
	    } else if (configObjc == 1) {
		return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, 
			entrySpecs, (char *)entryPtr, configObjv[0], 0);
	    }
	    if (ConfigureEntry(viewPtr, entryPtr, configObjc, configObjv, 
			       BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
		return TCL_ERROR;
	    }
	}
    }
    viewPtr->flags |= (DIRTY | LAYOUT_PENDING | SCROLL_PENDING /*| RESORT */);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * EntryIsOpenOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EntryIsBeforeOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *e1Ptr, *e2Ptr;
    int bool;

    if ((GetEntry(viewPtr, objv[3], &e1Ptr) != TCL_OK) ||
	(GetEntry(viewPtr, objv[4], &e2Ptr) != TCL_OK)) {
	return TCL_ERROR;
    }
    bool = Blt_Tree_IsBefore(e1Ptr->node, e2Ptr->node);
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), bool);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * EntryIsExposedOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EntryIsExposedOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    int bool;

    if (GetEntry(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    bool = ((entryPtr->flags & ENTRY_HIDE) == 0);
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), bool);
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * EntryIsHiddenOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EntryIsHiddenOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    int bool;

    if (GetEntry(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    bool = (entryPtr->flags & ENTRY_HIDE);
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), bool);
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * EntryIsOpenOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EntryIsOpenOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	      Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    int bool;

    if (GetEntry(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    bool = ((entryPtr->flags & ENTRY_CLOSED) == 0);
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), bool);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * EntryChildrenOp --
 *
 *	$treeview entry children $gid ?switches?
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EntryChildrenOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *parentPtr;
    Tcl_Obj *listObjPtr;
    ChildrenSwitches switches;
    Entry *entryPtr;

    if (GetEntry(viewPtr, objv[3], &parentPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    switches.mask = 0;
    if (Blt_ParseSwitches(interp, childrenSwitches, objc - 4, objv + 4, 
	&switches, BLT_SWITCH_DEFAULTS) < 0) {
	return TCL_ERROR;
    }
    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);

    for (entryPtr = FirstChild(parentPtr, switches.mask); entryPtr != NULL; 
	 entryPtr = NextSibling(entryPtr, switches.mask)) {
	Tcl_Obj *objPtr;

	objPtr = NodeToObj(entryPtr->node);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * EntryDeleteOp --
 *
 *	.tv entry degree $entry
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EntryDegreeOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	      Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *parentPtr, *entryPtr;
    long count;

    if (GetEntry(viewPtr, objv[3], &parentPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    count = 0;
    for (entryPtr = FirstChild(parentPtr, ENTRY_HIDE); entryPtr != NULL; 
	 entryPtr = NextSibling(entryPtr, ENTRY_HIDE)) {
	count++;
    }
    Tcl_SetLongObj(Tcl_GetObjResult(interp), count);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * EntryDeleteOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
EntryDeleteOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	      Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;

    if (GetEntry(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (objc == 5) {
	long entryPos;
	Blt_TreeNode node;
	/*
	 * Delete a single child node from a hierarchy specified by its
	 * numeric position.
	 */
	if (Blt_GetPositionFromObj(interp, objv[3], &entryPos) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (entryPos >= (long)Blt_Tree_NodeDegree(entryPtr->node)) {
	    return TCL_OK;	/* Bad first index */
	}
	if (entryPos == END) {
	    node = Blt_Tree_LastChild(entryPtr->node);
	} else {
	    node = GetNthNode(entryPtr->node, entryPos);
	}
	DeleteNode(viewPtr, node);
    } else {
	long firstPos, lastPos;
	Blt_TreeNode node, first, last, next;
	long numEntries;
	/*
	 * Delete range of nodes in hierarchy specified by first/last
	 * positions.
	 */
	if ((Blt_GetPositionFromObj(interp, objv[4], &firstPos) != TCL_OK) ||
	    (Blt_GetPositionFromObj(interp, objv[5], &lastPos) != TCL_OK)) {
	    return TCL_ERROR;
	}
	numEntries = Blt_Tree_NodeDegree(entryPtr->node);
	if (numEntries == 0) {
	    return TCL_OK;
	}
	if (firstPos == END) {
	    firstPos = numEntries - 1;
	}
	if (firstPos >= numEntries) {
	    Tcl_AppendResult(interp, "first position \"", 
		Tcl_GetString(objv[4]), " is out of range", (char *)NULL);
	    return TCL_ERROR;
	}
	if ((lastPos == END) || (lastPos >= numEntries)) {
	    lastPos = numEntries - 1;
	}
	if (firstPos > lastPos) {
	    Tcl_AppendResult(interp, "bad range: \"", Tcl_GetString(objv[4]), 
		" > ", Tcl_GetString(objv[5]), "\"", (char *)NULL);
	    return TCL_ERROR;
	}
	first = GetNthNode(entryPtr->node, firstPos);
	last = GetNthNode(entryPtr->node, lastPos);
	for (node = first; node != NULL; node = next) {
	    next = Blt_Tree_NextSibling(node);
	    DeleteNode(viewPtr, node);
	    if (node == last) {
		break;
	    }
	}
    }
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY /*| RESORT */);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * EntrySizeOp --
 *
 *	Counts the number of entries at this node.
 *
 * Results:
 *	A standard TCL result.  If an error occurred TCL_ERROR is returned and
 *	interp->result will contain an error message.  Otherwise, TCL_OK is
 *	returned and interp->result contains the number of entries.
 *
 *---------------------------------------------------------------------------
 */
static int
EntrySizeOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	    Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    int length, recurse;
    long sum;
    char *string;

    recurse = FALSE;
    string = Tcl_GetStringFromObj(objv[3], &length);
    if ((string[0] == '-') && (length > 1) &&
	(strncmp(string, "-recurse", length) == 0)) {
	objv++, objc--;
	recurse = TRUE;
    }
    if (objc == 3) {
	Tcl_AppendResult(interp, "missing node argument: should be \"",
	    Tcl_GetString(objv[0]), " entry open node\"", (char *)NULL);
	return TCL_ERROR;
    }
    if (GetEntry(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }

    if (recurse) {
	sum = Blt_Tree_Size(entryPtr->node);
    } else {
	sum = Blt_Tree_NodeDegree(entryPtr->node);
    }
    Tcl_SetLongObj(Tcl_GetObjResult(interp), sum);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * EntryOp --
 *
 *	This procedure handles entry operations.
 *
 * Results:
 *	A standard TCL result.
 *
 *---------------------------------------------------------------------------
 */

static Blt_OpSpec entryOps[] =
{
    {"activate",  1, EntryActivateOp,  4, 4, "tagOrId",},
    /*bbox*/
    /*bind*/
    {"cget",      2, EntryCgetOp,      5, 5, "tagOrId option",},
    {"children",  2, EntryChildrenOp,  4, 0, "tagOrId ?switches?",},
    /*close*/
    {"configure", 2, EntryConfigureOp, 4, 0, 
	"tagOrId ?tagOrId...? ?option value?...",},
    {"degree",    3, EntryDegreeOp,    4, 4, "tagOrId",},
    {"delete",    3, EntryDeleteOp,    5, 6, "tagOrId firstPos ?lastPos?",},
    /*focus*/
    /*hide*/
    {"highlight", 1, EntryActivateOp,  4, 4, "tagOrId",},
    /*index*/
    {"isbefore",  3, EntryIsBeforeOp,  5, 5, "tagOrId tagOrId",},
    {"isexposed", 3, EntryIsExposedOp, 4, 4, "tagOrId",},
    {"ishidden",  3, EntryIsHiddenOp,  4, 4, "tagOrId",},
    {"isopen",    3, EntryIsOpenOp,    4, 4, "tagOrId",},
    /*move*/
    /*nearest*/
    /*open*/
    /*see*/
    /*show*/
    {"size",      1, EntrySizeOp,      4, 5, "?-recurse? tagOrId",},
    /*toggle*/
};
static int numEntryOps = sizeof(entryOps) / sizeof(Blt_OpSpec);

static int
EntryOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    Tcl_ObjCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, numEntryOps, entryOps, BLT_OP_ARG2, objc, 
	objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (clientData, interp, objc, objv);
    return result;
}

/*ARGSUSED*/
static int
ExactCompare(Tcl_Interp *interp, const char *name, const char *pattern)
{
    return (strcmp(name, pattern) == 0);
}

/*ARGSUSED*/
static int
GlobCompare(Tcl_Interp *interp, const char *name, const char *pattern)
{
    return Tcl_StringMatch(name, pattern);
}

static int
RegexpCompare(Tcl_Interp *interp, const char *name, const char *pattern)
{
    return Tcl_RegExpMatch(interp, name, pattern);
}

/*
 *---------------------------------------------------------------------------
 *
 * FindOp --
 *
 *	Find one or more nodes based upon the pattern provided.
 *
 * Results:
 *	A standard TCL result.  The interpreter result will contain a list of
 *	the node serial identifiers.
 *
 *---------------------------------------------------------------------------
 */
static int
FindOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *firstPtr, *lastPtr;
    int numMatches, maxMatches;
    char c;
    int length;
    CompareProc *compareProc;
    IterProc *nextProc;
    int invertMatch;		/* normal search mode (matching entries) */
    char *namePattern, *fullPattern;
    int i;
    int result;
    char *pattern, *option;
    Blt_List options;
    Blt_ListNode node;
    char *addTag, *withTag;
    Entry *entryPtr;
    char *string;
    Tcl_Obj *listObjPtr, *objPtr, *execCmdObjPtr;

    invertMatch = FALSE;
    maxMatches = 0;
    namePattern = fullPattern = NULL;
    execCmdObjPtr = NULL;
    compareProc = ExactCompare;
    nextProc = NextEntry;
    options = Blt_List_Create(BLT_ONE_WORD_KEYS);
    withTag = addTag = NULL;

    entryPtr = viewPtr->rootPtr;
    /*
     * Step 1:  Process flags for find operation.
     */
    for (i = 2; i < objc; i++) {
	string = Tcl_GetStringFromObj(objv[i], &length);
	if (string[0] != '-') {
	    break;
	}
	option = string + 1;
	length--;
	c = option[0];
	if ((c == 'e') && (length > 2) &&
	    (strncmp(option, "exact", length) == 0)) {
	    compareProc = ExactCompare;
	} else if ((c == 'g') && (strncmp(option, "glob", length) == 0)) {
	    compareProc = GlobCompare;
	} else if ((c == 'r') && (strncmp(option, "regexp", length) == 0)) {
	    compareProc = RegexpCompare;
	} else if ((c == 'n') && (length > 1) &&
	    (strncmp(option, "nonmatching", length) == 0)) {
	    invertMatch = TRUE;
	} else if ((c == 'n') && (length > 1) &&
	    (strncmp(option, "name", length) == 0)) {
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    i++;
	    namePattern = Tcl_GetString(objv[i]);
	} else if ((c == 'f') && (strncmp(option, "full", length) == 0)) {
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    i++;
	    fullPattern = Tcl_GetString(objv[i]);
	} else if ((c == 'e') && (length > 2) &&
	    (strncmp(option, "exec", length) == 0)) {
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    i++;
	    execCmdObjPtr = objv[i];
	} else if ((c == 'a') && (length > 1) &&
		   (strncmp(option, "addtag", length) == 0)) {
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    i++;
	    addTag = Tcl_GetString(objv[i]);
	} else if ((c == 't') && (length > 1) && 
		   (strncmp(option, "tag", length) == 0)) {
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    i++;
	    withTag = Tcl_GetString(objv[i]);
	} else if ((c == 'c') && (strncmp(option, "count", length) == 0)) {
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    i++;
	    if (Tcl_GetIntFromObj(interp, objv[i], &maxMatches) != TCL_OK) {
		return TCL_ERROR;
	    }
	    if (maxMatches < 0) {
		Tcl_AppendResult(interp, "bad match count \"", objv[i],
		    "\": should be a positive number", (char *)NULL);
		Blt_List_Destroy(options);
		return TCL_ERROR;
	    }
	} else if ((option[0] == '-') && (option[1] == '\0')) {
	    break;
	} else {
	    /*
	     * Verify that the switch is actually an entry configuration
	     * option.
	     */
	    if (Blt_ConfigureValueFromObj(interp, viewPtr->tkwin, entrySpecs, 
		(char *)entryPtr, objv[i], 0) != TCL_OK) {
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "bad find switch \"", string, "\"",
		    (char *)NULL);
		Blt_List_Destroy(options);
		return TCL_ERROR;
	    }
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    /* Save the option in the list of configuration options */
	    node = Blt_List_GetNode(options, (char *)objv[i]);
	    if (node == NULL) {
		node = Blt_List_CreateNode(options, (char *)objv[i]);
		Blt_List_AppendNode(options, node);
	    }
	    i++;
	    Blt_List_SetValue(node, Tcl_GetString(objv[i]));
	}
    }

    if ((objc - i) > 2) {
	Blt_List_Destroy(options);
	Tcl_AppendResult(interp, "too many args", (char *)NULL);
	return TCL_ERROR;
    }
    /*
     * Step 2:  Find the range of the search.  Check the order of two
     *		nodes and arrange the search accordingly.
     *
     *	Note:	Be careful to treat "end" as the end of all nodes, instead
     *		of the end of visible nodes.  That way, we can search the
     *		entire tree, even if the last folder is closed.
     */
    firstPtr = viewPtr->rootPtr;	/* Default to root node */
    lastPtr = LastEntry(viewPtr, firstPtr, 0);

    if (i < objc) {
	string = Tcl_GetString(objv[i]);
	if ((string[0] == 'e') && (strcmp(string, "end") == 0)) {
	    firstPtr = LastEntry(viewPtr, viewPtr->rootPtr, 0);
	} else if (GetEntry(viewPtr, objv[i], &firstPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	i++;
    }
    if (i < objc) {
	string = Tcl_GetString(objv[i]);
	if ((string[0] == 'e') && (strcmp(string, "end") == 0)) {
	    lastPtr = LastEntry(viewPtr, viewPtr->rootPtr, 0);
	} else if (GetEntry(viewPtr, objv[i], &lastPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    if (Blt_Tree_IsBefore(lastPtr->node, firstPtr->node)) {
	nextProc = PrevEntry;
    }
    numMatches = 0;

    /*
     * Step 3:	Search through the tree and look for nodes that match the
     *		current pattern specifications.  Save the name of each of
     *		the matching nodes.
     */
    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for (entryPtr = firstPtr; entryPtr != NULL; 
	 entryPtr = (*nextProc) (entryPtr, 0)) {
	if (namePattern != NULL) {
	    result = (*compareProc)(interp, Blt_Tree_NodeLabel(entryPtr->node),
		     namePattern);
	    if (result == invertMatch) {
		goto nextEntry;	/* Failed to match */
	    }
	}
	if (fullPattern != NULL) {
	    Tcl_DString ds;

	    GetEntryPath(viewPtr, entryPtr, FALSE, &ds);
	    result = (*compareProc) (interp, Tcl_DStringValue(&ds),fullPattern);
	    Tcl_DStringFree(&ds);
	    if (result == invertMatch) {
		goto nextEntry;	/* Failed to match */
	    }
	}
	if (withTag != NULL) {
	    result = Blt_Tree_HasTag(viewPtr->tree, entryPtr->node, withTag);
	    if (result == invertMatch) {
		goto nextEntry;	/* Failed to match */
	    }
	}
	for (node = Blt_List_FirstNode(options); node != NULL;
	    node = Blt_List_NextNode(node)) {
	    objPtr = (Tcl_Obj *)Blt_List_GetKey(node);
	    Tcl_ResetResult(interp);
	    Blt_ConfigureValueFromObj(interp, viewPtr->tkwin, entrySpecs, 
		(char *)entryPtr, objPtr, 0);
	    pattern = Blt_List_GetValue(node);
	    objPtr = Tcl_GetObjResult(interp);
	    result = (*compareProc) (interp, Tcl_GetString(objPtr), pattern);
	    if (result == invertMatch) {
		goto nextEntry;	/* Failed to match */
	    }
	}
	/* 
	 * Someone may actually delete the current node in the "exec"
	 * callback.  Preserve the entry.
	 */
	Tcl_Preserve(entryPtr);
	if (execCmdObjPtr != NULL) {
	    Tcl_Obj *cmdObjPtr;

	    cmdObjPtr = PercentSubst(viewPtr, entryPtr, execCmdObjPtr);
	    Tcl_IncrRefCount(cmdObjPtr);
	    result = Tcl_EvalObjEx(viewPtr->interp, cmdObjPtr, TCL_EVAL_GLOBAL);
	    Tcl_DecrRefCount(cmdObjPtr);
	    if (result != TCL_OK) {
		Tcl_Release(entryPtr);
		goto error;
	    }
	}
	/* A NULL node reference in an entry indicates that the entry
	 * was deleted, but its memory not released yet. */
	if (entryPtr->node != NULL) {
	    /* Finally, save the matching node name. */
	    objPtr = NodeToObj(entryPtr->node);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	    if (addTag != NULL) {
		if (AddTag(viewPtr, entryPtr->node, addTag) != TCL_OK) {
		    goto error;
		}
	    }
	}
	    
	Tcl_Release(entryPtr);
	numMatches++;
	if ((numMatches == maxMatches) && (maxMatches > 0)) {
	    break;
	}
      nextEntry:
	if (entryPtr == lastPtr) {
	    break;
	}
    }
    Tcl_ResetResult(interp);
    Blt_List_Destroy(options);
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;

  missingArg:
    Tcl_AppendResult(interp, "missing argument for find option \"", objv[i],
	"\"", (char *)NULL);
  error:
    Blt_List_Destroy(options);
    return TCL_ERROR;
}


/*
 *---------------------------------------------------------------------------
 *
 * GetOp --
 *
 *	Converts one or more node identifiers to its path component.  The path
 *	may be either the single entry name or the full path of the entry.
 *
 * Results:
 *	A standard TCL result.  The interpreter result will contain a list of
 *	the convert names.
 *
 *---------------------------------------------------------------------------
 */
static int
GetOp(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    TagIterator iter;
    Entry *entryPtr;
    int useFullName;
    int i;
    Tcl_DString d1, d2;
    int count;

    useFullName = FALSE;
    if (objc > 2) {
	char *string;

	string = Tcl_GetString(objv[2]);
	if ((string[0] == '-') && (strcmp(string, "-full") == 0)) {
	    useFullName = TRUE;
	    objv++, objc--;
	}
    }
    Tcl_DStringInit(&d1);	/* Result. */
    Tcl_DStringInit(&d2);	/* Last element. */
    count = 0;
    for (i = 2; i < objc; i++) {
	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    Tcl_DStringSetLength(&d2, 0);
	    count++;
	    if (entryPtr->node != NULL) {
		if (useFullName) {
		    GetEntryPath(viewPtr, entryPtr, FALSE, &d2);
		} else {
		    Tcl_DStringAppend(&d2,Blt_Tree_NodeLabel(entryPtr->node),-1);
		}
		Tcl_DStringAppendElement(&d1, Tcl_DStringValue(&d2));
	    }
	}
    }
    /* This handles the single element list problem. */
    if (count == 1) {
	Tcl_DStringResult(interp, &d2);
	Tcl_DStringFree(&d1);
    } else {
	Tcl_DStringResult(interp, &d1);
	Tcl_DStringFree(&d2);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SearchAndApplyToTree --
 *
 *	Searches through the current tree and applies a procedure to matching
 *	nodes.  The search specification is taken from the following
 *	command-line arguments:
 *
 *      ?-exact? ?-glob? ?-regexp? ?-nonmatching?
 *      ?-data string?
 *      ?-name string?
 *      ?-full string?
 *      ?--?
 *      ?inode...?
 *
 * Results:
 *	A standard TCL result.  If the result is valid, and if the nonmatchPtr
 *	is specified, it returns a boolean value indicating whether or not the
 *	search was inverted.  This is needed to fix things properly for the
 *	"hide nonmatching" case.
 *
 *---------------------------------------------------------------------------
 */
static int
SearchAndApplyToTree(TreeView *viewPtr, Tcl_Interp *interp, int objc, 
	     Tcl_Obj *const *objv, TreeViewApplyProc *proc, int *nonMatchPtr)
{
    CompareProc *compareProc;
    int invertMatch;			/* Normal search mode (matching
					 * entries) */
    char *namePattern, *fullPattern;
    int i;
    int length;
    int result;
    char *option, *pattern;
    char c;
    Blt_List options;
    Entry *entryPtr;
    Blt_ListNode node;
    char *string;
    char *withTag;
    Tcl_Obj *objPtr;
    TagIterator iter;

    options = Blt_List_Create(BLT_ONE_WORD_KEYS);
    invertMatch = FALSE;
    namePattern = fullPattern = NULL;
    compareProc = ExactCompare;
    withTag = NULL;

    entryPtr = viewPtr->rootPtr;
    for (i = 2; i < objc; i++) {
	string = Tcl_GetStringFromObj(objv[i], &length);
	if (string[0] != '-') {
	    break;
	}
	option = string + 1;
	length--;
	c = option[0];
	if ((c == 'e') && (strncmp(option, "exact", length) == 0)) {
	    compareProc = ExactCompare;
	} else if ((c == 'g') && (strncmp(option, "glob", length) == 0)) {
	    compareProc = GlobCompare;
	} else if ((c == 'r') && (strncmp(option, "regexp", length) == 0)) {
	    compareProc = RegexpCompare;
	} else if ((c == 'n') && (length > 1) &&
	    (strncmp(option, "nonmatching", length) == 0)) {
	    invertMatch = TRUE;
	} else if ((c == 'f') && (strncmp(option, "full", length) == 0)) {
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    i++;
	    fullPattern = Tcl_GetString(objv[i]);
	} else if ((c == 'n') && (length > 1) &&
	    (strncmp(option, "name", length) == 0)) {
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    i++;
	    namePattern = Tcl_GetString(objv[i]);
	} else if ((c == 't') && (length > 1) && 
		   (strncmp(option, "tag", length) == 0)) {
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    i++;
	    withTag = Tcl_GetString(objv[i]);
	} else if ((option[0] == '-') && (option[1] == '\0')) {
	    break;
	} else {
	    /*
	     * Verify that the switch is actually an entry configuration
	     * option.
	     */
	    if (Blt_ConfigureValueFromObj(interp, viewPtr->tkwin, entrySpecs, 
		(char *)entryPtr, objv[i], 0) != TCL_OK) {
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "bad switch \"", string,
	    "\": must be -exact, -glob, -regexp, -name, -full, or -nonmatching",
		    (char *)NULL);
		return TCL_ERROR;
	    }
	    if ((i + 1) == objc) {
		goto missingArg;
	    }
	    /* Save the option in the list of configuration options */
	    node = Blt_List_GetNode(options, (char *)objv[i]);
	    if (node == NULL) {
		node = Blt_List_CreateNode(options, (char *)objv[i]);
		Blt_List_AppendNode(options, node);
	    }
	    i++;
	    Blt_List_SetValue(node, Tcl_GetString(objv[i]));
	}
    }

    if ((namePattern != NULL) || (fullPattern != NULL) ||
	(Blt_List_GetLength(options) > 0)) {
	/*
	 * Search through the tree and look for nodes that match the current
	 * spec.  Apply the input procedure to each of the matching nodes.
	 */
	for (entryPtr = viewPtr->rootPtr; entryPtr != NULL; 
	     entryPtr = NextEntry(entryPtr, 0)) {
	    if (namePattern != NULL) {
		result = (*compareProc) (interp, 
			Blt_Tree_NodeLabel(entryPtr->node), namePattern);
		if (result == invertMatch) {
		    continue;		/* Failed to match */
		}
	    }
	    if (fullPattern != NULL) {
		Tcl_DString ds;

		GetEntryPath(viewPtr, entryPtr, FALSE, &ds);
		result = (*compareProc) (interp, Tcl_DStringValue(&ds), 
			fullPattern);
		Tcl_DStringFree(&ds);
		if (result == invertMatch) {
		    continue;		/* Failed to match */
		}
	    }
	    if (withTag != NULL) {
		result = Blt_Tree_HasTag(viewPtr->tree, entryPtr->node, withTag);
		if (result == invertMatch) {
		    continue;		/* Failed to match */
		}
	    }
	    for (node = Blt_List_FirstNode(options); node != NULL;
		node = Blt_List_NextNode(node)) {
		objPtr = (Tcl_Obj *)Blt_List_GetKey(node);
		Tcl_ResetResult(interp);
		if (Blt_ConfigureValueFromObj(interp, viewPtr->tkwin, 
			entrySpecs, (char *)entryPtr, objPtr, 0) != TCL_OK) {
		    return TCL_ERROR;	/* This shouldn't happen. */
		}
		pattern = Blt_List_GetValue(node);
		objPtr = Tcl_GetObjResult(interp);
		result = (*compareProc)(interp, Tcl_GetString(objPtr), pattern);
		if (result == invertMatch) {
		    continue;		/* Failed to match */
		}
	    }
	    /* Finally, apply the procedure to the node */
	    (*proc) (viewPtr, entryPtr);
	}
	Tcl_ResetResult(interp);
	Blt_List_Destroy(options);
    }
    /*
     * Apply the procedure to nodes that have been specified individually.
     */
    for ( /*empty*/ ; i < objc; i++) {
	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    if ((*proc) (viewPtr, entryPtr) != TCL_OK) {
		return TCL_ERROR;
	    }
	}
    }
    if (nonMatchPtr != NULL) {
	*nonMatchPtr = invertMatch;	/* return "inverted search" status */
    }
    return TCL_OK;

  missingArg:
    Blt_List_Destroy(options);
    Tcl_AppendResult(interp, "missing pattern for search option \"", objv[i],
	"\"", (char *)NULL);
    return TCL_ERROR;

}

static int
FixSelectionsApplyProc(TreeView *viewPtr, Entry *entryPtr)
{
    if (entryPtr->flags & ENTRY_HIDE) {
	DeselectEntry(viewPtr, entryPtr);
	if ((viewPtr->focusPtr != NULL) &&
	    (Blt_Tree_IsAncestor(entryPtr->node, viewPtr->focusPtr->node))) {
	    if (entryPtr != viewPtr->rootPtr) {
		entryPtr = ParentEntry(entryPtr);
		viewPtr->focusPtr = (entryPtr == NULL) 
		    ? viewPtr->focusPtr : entryPtr;
		Blt_SetFocusItem(viewPtr->bindTable, viewPtr->focusPtr, 
				 ITEM_ENTRY);
	    }
	}
	if ((viewPtr->selection.anchorPtr != NULL) &&
	    (Blt_Tree_IsAncestor(entryPtr->node, 
				 viewPtr->selection.anchorPtr->node))) {
	    viewPtr->selection.markPtr = viewPtr->selection.anchorPtr = NULL;
	}
	if ((viewPtr->activePtr != NULL) &&
	    (Blt_Tree_IsAncestor(entryPtr->node, viewPtr->activePtr->node))) {
	    viewPtr->activePtr = NULL;
	}
	PruneSelection(viewPtr, entryPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * HideOp --
 *
 *	Hides one or more nodes.  Nodes can be specified by their inode, or by
 *	matching a name or data value pattern.  By default, the patterns are
 *	matched exactly.  They can also be matched using glob-style and
 *	regular expression rules.
 *
 * Results:
 *	A standard TCL result.
 *
 *---------------------------------------------------------------------------
 */
static int
HideOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int status, nonmatching;

    status = SearchAndApplyToTree(viewPtr, interp, objc, objv, 
	HideEntryApplyProc, &nonmatching);

    if (status != TCL_OK) {
	return TCL_ERROR;
    }
    /*
     * If this was an inverted search, scan back through the tree and make
     * sure that the parents for all visible nodes are also visible.  After
     * all, if a node is supposed to be visible, its parent can't be hidden.
     */
    if (nonmatching) {
	Apply(viewPtr, viewPtr->rootPtr, MapAncestorsApplyProc, 0);
    }
    /*
     * Make sure that selections are cleared from any hidden nodes.  This
     * wasn't done earlier--we had to delay it until we fixed the visibility
     * status for the parents.
     */
    Apply(viewPtr, viewPtr->rootPtr, FixSelectionsApplyProc, 0);

    /* Hiding an entry only effects the visible nodes. */
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ShowOp --
 *
 *	Mark one or more nodes to be exposed.  Nodes can be specified by their
 *	inode, or by matching a name or data value pattern.  By default, the
 *	patterns are matched exactly.  They can also be matched using
 *	glob-style and regular expression rules.
 *
 * Results:
 *	A standard TCL result.
 *
 *---------------------------------------------------------------------------
 */
static int
ShowOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    if (SearchAndApplyToTree(viewPtr, interp, objc, objv, ShowEntryApplyProc,
	    (int *)NULL) != TCL_OK) {
	return TCL_ERROR;
    }
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * IndexOp --
 *
 *	Converts one of more words representing indices of the entries in the
 *	treeview widget to their respective serial identifiers.
 *
 * Results:
 *	A standard TCL result.  Interp->result will contain the identifier of
 *	each inode found. If an inode could not be found, then the serial
 *	identifier will be the empty string.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
IndexOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    char *string;
    Entry *fromPtr;
    int usePath;
    long inode;

    usePath = FALSE;
    fromPtr = NULL;
    string = Tcl_GetString(objv[2]);
    if ((string[0] == '-') && (strcmp(string, "-path") == 0)) {
	usePath = TRUE;
	objv++, objc--;
    }
    if ((string[0] == '-') && (strcmp(string, "-at") == 0)) {
	if (GetEntry(viewPtr, objv[3], &fromPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	objv += 2, objc -= 2;
    }
    if (objc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), 
		" index ?-at tagOrId? ?-path? tagOrId\"", 
		(char *)NULL);
	return TCL_ERROR;
    }
    viewPtr->fromPtr = fromPtr;
    if (viewPtr->fromPtr == NULL) {
	viewPtr->fromPtr = viewPtr->focusPtr;
    }
    if (viewPtr->fromPtr == NULL) {
	viewPtr->fromPtr = viewPtr->rootPtr;
    }
    inode = -1;
    if (usePath) {
	if (fromPtr == NULL) {
	    fromPtr = viewPtr->rootPtr;
	}
	string = Tcl_GetString(objv[2]);
	entryPtr = FindPath(viewPtr, fromPtr, string);
	if (entryPtr != NULL) {
	    inode = Blt_Tree_NodeId(entryPtr->node);
	}
    } else {
	if ((GetEntryFromObj2(viewPtr, objv[2], &entryPtr) == TCL_OK) && 
	    (entryPtr != NULL)) {
	    inode = Blt_Tree_NodeId(entryPtr->node);
	}
    }
    Tcl_SetLongObj(Tcl_GetObjResult(interp), inode);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * InsertOp --
 *
 *	Add new entries into a hierarchy.  If no node is specified, new
 *	entries will be added to the root of the hierarchy.
 *
 *---------------------------------------------------------------------------
 */
static int
InsertOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Blt_TreeNode node, parent;
    Tcl_Obj *const *options;
    Tcl_Obj *listObjPtr;
    Entry *rootPtr;
    const char **argv;
    const char **p;
    const char *path;
    char *string;
    int count;
    int n;
    long depth;
    long insertPos;

    rootPtr = viewPtr->rootPtr;
    string = Tcl_GetString(objv[2]);
    if ((string[0] == '-') && (strcmp(string, "-at") == 0)) {
	if (objc > 2) {
	    if (GetEntry(viewPtr, objv[3], &rootPtr) != TCL_OK) {
		return TCL_ERROR;
	    }
	    objv += 2, objc -= 2;
	} else {
	    Tcl_AppendResult(interp, "missing argument for \"-at\" flag",
		     (char *)NULL);
	    return TCL_ERROR;
	}
    }
    if (objc == 2) {
	Tcl_AppendResult(interp, "missing position argument", (char *)NULL);
	return TCL_ERROR;
    }
    if (Blt_GetPositionFromObj(interp, objv[2], &insertPos) != TCL_OK) {
	return TCL_ERROR;
    }
    node = NULL;
    objc -= 3, objv += 3;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    while (objc > 0) {
	path = Tcl_GetString(objv[0]);
	objv++, objc--;

	/*
	 * Count the option-value pairs that follow.  Count until we spot one
	 * that looks like an entry name (i.e. doesn't start with a minus
	 * "-").
	 */
	for (count = 0; count < objc; count += 2) {
	    string = Tcl_GetString(objv[count]);
	    if (string[0] != '-') {
		break;
	    }
	}
	if (count > objc) {
	    count = objc;
	}
	options = objv;
	objc -= count, objv += count;

	if (viewPtr->trimLeft != NULL) {
	    const char *s1, *s2;

	    /* Trim off leading character string if one exists. */
	    for (s1 = path, s2 = viewPtr->trimLeft; *s2 != '\0'; s2++, s1++) {
		if (*s1 != *s2) {
		    break;
		}
	    }
	    if (*s2 == '\0') {
		path = s1;
	    }
	}
	/* Split the path and find the parent node of the path. */
	argv = &path;
	depth = 1;
	if (viewPtr->pathSep != SEPARATOR_NONE) {
	    if (SplitPath(viewPtr, path, &depth, &argv) != TCL_OK) {
		goto error;
	    }
	    if (depth == 0) {
		Blt_Free(argv);
		continue;		/* Root already exists. */
	    }
	}
	parent = rootPtr->node;
	depth--;		

	/* Verify each component in the path preceding the tail.  */
	for (n = 0, p = argv; n < depth; n++, p++) {
	    node = Blt_Tree_FindChild(parent, *p);
	    if (node == NULL) {
		if ((viewPtr->flags & FILL_ANCESTORS) == 0) {
		    Tcl_AppendResult(interp, "can't find path component \"",
		         *p, "\" in \"", path, "\"", (char *)NULL);
		    goto error;
		}
		node = Blt_Tree_CreateNode(viewPtr->tree, parent, *p, END);
		if (node == NULL) {
		    goto error;
		}
	    }
	    parent = node;
	}
	node = NULL;
	if (((viewPtr->flags & ALLOW_DUPLICATES) == 0) && 
	    (Blt_Tree_FindChild(parent, *p) != NULL)) {
	    Tcl_AppendResult(interp, "entry \"", *p, "\" already exists in \"",
		 path, "\"", (char *)NULL);
	    goto error;
	}
	node = Blt_Tree_CreateNode(viewPtr->tree, parent, *p, insertPos);
	if (node == NULL) {
	    goto error;
	}
	if (CreateEntry(viewPtr, node, count, options, 0) != TCL_OK) {
	    goto error;
	}
	if (argv != &path) {
	    Blt_Free(argv);
	}
	Tcl_ListObjAppendElement(interp, listObjPtr, NodeToObj(node));
    }
    viewPtr->flags |= (LAYOUT_PENDING | SCROLL_PENDING | DIRTY /*| RESORT */);
    EventuallyRedraw(viewPtr);
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;

  error:
    if (argv != &path) {
	Blt_Free(argv);
    }
    Tcl_DecrRefCount(listObjPtr);
    if (node != NULL) {
	DeleteNode(viewPtr, node);
    }
    return TCL_ERROR;
}


/*
 *---------------------------------------------------------------------------
 *
 * DeleteOp --
 *
 *	Deletes nodes from the hierarchy. Deletes one or more entries (except
 *	root). In all cases, nodes are removed recursively.
 *
 *	Note: There's no need to explicitly clean up Entry structures 
 *	      or request a redraw of the widget. When a node is 
 *	      deleted in the tree, all of the Tcl_Objs representing
 *	      the various data fields are also removed.  The treeview 
 *	      widget store the Entry structure in a data field. So it's
 *	      automatically cleaned up when FreeEntryInternalRep is
 *	      called.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
DeleteOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    TagIterator iter;
    Entry *entryPtr;
    int i;

    for (i = 2; i < objc; i++) {
	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    if (entryPtr == viewPtr->rootPtr) {
		Blt_TreeNode next, node;

		/* 
		 *   Don't delete the root node.  We implicitly assume that
		 *   even an empty tree has at a root.  Instead delete all the
		 *   children regardless if they're closed or hidden.
		 */
		for (node = Blt_Tree_FirstChild(entryPtr->node); node != NULL; 
		     node = next) {
		    next = Blt_Tree_NextSibling(node);
		    DeleteNode(viewPtr, node);
		}
	    } else {
		DeleteNode(viewPtr, entryPtr->node);
	    }
	}
    } 
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * InvokeOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
InvokeOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    TagIterator iter;
    Entry *entryPtr;
    int i;

    for (i = 2; i < objc; i++) {
	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    Tcl_Obj *cmdObjPtr;

	    cmdObjPtr = (entryPtr->cmdObjPtr != NULL) ? 
		entryPtr->cmdObjPtr : viewPtr->entryCmdObjPtr;
	    if (cmdObjPtr != NULL) {
		int result;
		
		cmdObjPtr = PercentSubst(viewPtr, entryPtr, cmdObjPtr);
		Tcl_IncrRefCount(cmdObjPtr);
		Tcl_Preserve(entryPtr);
		result = Tcl_EvalObjEx(interp, cmdObjPtr, TCL_EVAL_GLOBAL);
		Tcl_Release(entryPtr);
		Tcl_DecrRefCount(cmdObjPtr);
		if (result != TCL_OK) {
		    return TCL_ERROR;
		}
	    }
	}
    } 
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * MoveOp --
 *
 *	Move an entry into a new location in the hierarchy.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
MoveOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Blt_TreeNode parent;
    Entry *srcPtr, *destPtr;
    char c;
    int action;
    char *string;
    TagIterator iter;

#define MOVE_INTO	(1<<0)
#define MOVE_BEFORE	(1<<1)
#define MOVE_AFTER	(1<<2)
    if (GetEntryIterator(viewPtr, objv[2], &iter) != TCL_OK) {
	return TCL_ERROR;
    }
    string = Tcl_GetString(objv[3]);
    c = string[0];
    if ((c == 'i') && (strcmp(string, "into") == 0)) {
	action = MOVE_INTO;
    } else if ((c == 'b') && (strcmp(string, "before") == 0)) {
	action = MOVE_BEFORE;
    } else if ((c == 'a') && (strcmp(string, "after") == 0)) {
	action = MOVE_AFTER;
    } else {
	Tcl_AppendResult(interp, "bad position \"", string,
	    "\": should be into, before, or after", (char *)NULL);
	return TCL_ERROR;
    }
    if (GetEntry(viewPtr, objv[4], &destPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    for (srcPtr = FirstTaggedEntry(&iter); srcPtr != NULL; 
	 srcPtr = NextTaggedEntry(&iter)) {
	/* Verify they aren't ancestors. */
	if (Blt_Tree_IsAncestor(srcPtr->node, destPtr->node)) {
	    Tcl_DString ds;
	    const char *path;

	    path = GetEntryPath(viewPtr, srcPtr, 1, &ds);
	    Tcl_AppendResult(interp, "can't move node: \"", path, 
			"\" is an ancestor of \"", Tcl_GetString(objv[4]), 
			"\"", (char *)NULL);
	    Tcl_DStringFree(&ds);
	    return TCL_ERROR;
	}
	parent = Blt_Tree_ParentNode(destPtr->node);
	if (parent == NULL) {
	    action = MOVE_INTO;
	}
	switch (action) {
	case MOVE_INTO:
	    Blt_Tree_MoveNode(viewPtr->tree, srcPtr->node, destPtr->node, 
			     (Blt_TreeNode)NULL);
	    break;
	    
	case MOVE_BEFORE:
	    Blt_Tree_MoveNode(viewPtr->tree, srcPtr->node, parent, destPtr->node);
	    break;
	    
	case MOVE_AFTER:
	    Blt_Tree_MoveNode(viewPtr->tree, srcPtr->node, parent, 
			     Blt_Tree_NextSibling(destPtr->node));
	    break;
	}
    }
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY /*| RESORT */);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*ARGSUSED*/
static int
NearestOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	  Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Button *buttonPtr = &viewPtr->button;
    int x, y;			/* Screen coordinates of the test point. */
    Entry *entryPtr;
    int isRoot;
    char *string;

    isRoot = FALSE;
    string = Tcl_GetString(objv[2]);
    if (strcmp("-root", string) == 0) {
	isRoot = TRUE;
	objv++, objc--;
    } 
    if (objc < 4) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", 
		Tcl_GetString(objv[0]), " ", Tcl_GetString(objv[1]), 
		" ?-root? x y\"", (char *)NULL);
	return TCL_ERROR;
			 
    }
    if ((Tk_GetPixelsFromObj(interp, viewPtr->tkwin, objv[2], &x) != TCL_OK) ||
	(Tk_GetPixelsFromObj(interp, viewPtr->tkwin, objv[3], &y) != TCL_OK)) {
	return TCL_ERROR;
    }
    if (viewPtr->numVisible == 0) {
	return TCL_OK;
    }
    if (isRoot) {
	int rootX, rootY;

	Tk_GetRootCoords(viewPtr->tkwin, &rootX, &rootY);
	x -= rootX;
	y -= rootY;
    }
    entryPtr = NearestEntry(viewPtr, x, y, TRUE);
    if (entryPtr == NULL) {
	return TCL_OK;
    }
    x = WORLDX(viewPtr, x);
    y = WORLDY(viewPtr, y);
    if (objc > 4) {
	const char *where;
	int labelX, labelY, depth;
	Icon icon;

	where = "";
	if (entryPtr->flags & ENTRY_HAS_BUTTON) {
	    int buttonX, buttonY;

	    buttonX = entryPtr->worldX + entryPtr->buttonX;
	    buttonY = entryPtr->worldY + entryPtr->buttonY;
	    if ((x >= buttonX) && (x < (buttonX + buttonPtr->width)) &&
		(y >= buttonY) && (y < (buttonY + buttonPtr->height))) {
		where = "button";
		goto done;
	    }
	} 
	depth = DEPTH(viewPtr, entryPtr->node);

	icon = GetEntryIcon(viewPtr, entryPtr);
	if (icon != NULL) {
	    int iconWidth, iconHeight, entryHeight;
	    int iconX, iconY;
	    
	    entryHeight = MAX(entryPtr->iconHeight, viewPtr->button.height);
	    iconHeight = TreeView_IconHeight(icon);
	    iconWidth = TreeView_IconWidth(icon);
	    iconX = entryPtr->worldX + ICONWIDTH(depth);
	    iconY = entryPtr->worldY;
	    if (viewPtr->flatView) {
		iconX += (ICONWIDTH(0) - iconWidth) / 2;
	    } else {
		iconX += (ICONWIDTH(depth + 1) - iconWidth) / 2;
	    }	    
	    iconY += (entryHeight - iconHeight) / 2;
	    if ((x >= iconX) && (x <= (iconX + iconWidth)) &&
		(y >= iconY) && (y < (iconY + iconHeight))) {
		where = "icon";
		goto done;
	    }
	}
	labelX = entryPtr->worldX + ICONWIDTH(depth);
	labelY = entryPtr->worldY;
	if (!viewPtr->flatView) {
	    labelX += ICONWIDTH(depth + 1) + 4;
	}	    
	if ((x >= labelX) && (x < (labelX + entryPtr->labelWidth)) &&
	    (y >= labelY) && (y < (labelY + entryPtr->labelHeight))) {
	    where = "label";
	}
    done:
	if (Tcl_SetVar(interp, Tcl_GetString(objv[4]), where, 
		TCL_LEAVE_ERR_MSG) == NULL) {
	    return TCL_ERROR;
	}
    }
    Tcl_SetObjResult(interp, NodeToObj(entryPtr->node));
    return TCL_OK;
}


/*ARGSUSED*/
static int
OpenOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    TagIterator iter;
    int recurse, result;
    int i;

    recurse = FALSE;
    if (objc > 2) {
	int length;
	char *string;

	string = Tcl_GetStringFromObj(objv[2], &length);
	if ((string[0] == '-') && (length > 1) && 
	    (strncmp(string, "-recurse", length) == 0)) {
	    objv++, objc--;
	    recurse = TRUE;
	}
    }
    for (i = 2; i < objc; i++) {
	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    if (recurse) {
		result = Apply(viewPtr, entryPtr, OpenEntry, 0);
	    } else {
		result = OpenEntry(viewPtr, entryPtr);
	    }
	    if (result != TCL_OK) {
		return TCL_ERROR;
	    }
	    /* Make sure ancestors of this node aren't hidden. */
	    MapAncestors(viewPtr, entryPtr);
	}
    }
    /*FIXME: This is only for flattened entries.  */
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY /*| RESORT */);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * RangeOp --
 *
 *	Returns the node identifiers in a given range.
 *
 *---------------------------------------------------------------------------
 */
static int
RangeOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr, *firstPtr, *lastPtr;
    unsigned int mask;
    int length;
    Tcl_Obj *listObjPtr, *objPtr;
    char *string;

    mask = 0;
    string = Tcl_GetStringFromObj(objv[2], &length);
    if ((string[0] == '-') && (length > 1) && 
	(strncmp(string, "-open", length) == 0)) {
	objv++, objc--;
	mask |= ENTRY_CLOSED;
    }
    if (GetEntry(viewPtr, objv[2], &firstPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (objc > 3) {
	if (GetEntry(viewPtr, objv[3], &lastPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
    } else {
	lastPtr = LastEntry(viewPtr, firstPtr, mask);
    }    
    if (mask & ENTRY_CLOSED) {
	if (firstPtr->flags & ENTRY_HIDE) {
	    Tcl_AppendResult(interp, "first node \"", Tcl_GetString(objv[2]), 
		"\" is hidden.", (char *)NULL);
	    return TCL_ERROR;
	}
	if (lastPtr->flags & ENTRY_HIDE) {
	    Tcl_AppendResult(interp, "last node \"", Tcl_GetString(objv[3]), 
		"\" is hidden.", (char *)NULL);
	    return TCL_ERROR;
	}
    }

    /*
     * The relative order of the first/last markers determines the
     * direction.
     */
    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    if (Blt_Tree_IsBefore(lastPtr->node, firstPtr->node)) {
	for (entryPtr = lastPtr; entryPtr != NULL; 
	     entryPtr = PrevEntry(entryPtr, mask)) {
	    objPtr = NodeToObj(entryPtr->node);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	    if (entryPtr == firstPtr) {
		break;
	    }
	}
    } else {
	for (entryPtr = firstPtr; entryPtr != NULL; 
	     entryPtr = NextEntry(entryPtr, mask)) {
	    objPtr = NodeToObj(entryPtr->node);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	    if (entryPtr == lastPtr) {
		break;
	    }
	}
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ScanOp --
 *
 *	Implements the quick scan.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ScanOp(ClientData clientData, Tcl_Interp *interp, int objc, 
       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int x, y;
    char c;
    int length;
    int oper;
    char *string;
    Tk_Window tkwin;

#define SCAN_MARK	1
#define SCAN_DRAGTO	2
    string = Tcl_GetStringFromObj(objv[2], &length);
    c = string[0];
    tkwin = viewPtr->tkwin;
    if ((c == 'm') && (strncmp(string, "mark", length) == 0)) {
	oper = SCAN_MARK;
    } else if ((c == 'd') && (strncmp(string, "dragto", length) == 0)) {
	oper = SCAN_DRAGTO;
    } else {
	Tcl_AppendResult(interp, "bad scan operation \"", string,
	    "\": should be either \"mark\" or \"dragto\"", (char *)NULL);
	return TCL_ERROR;
    }
    if ((Blt_GetPixelsFromObj(interp, tkwin, objv[3], PIXELS_ANY, &x) 
	 != TCL_OK) ||
	(Blt_GetPixelsFromObj(interp, tkwin, objv[4], PIXELS_ANY, &y) 
	 != TCL_OK)) {
	return TCL_ERROR;
    }
    if (oper == SCAN_MARK) {
	viewPtr->scanAnchorX = x;
	viewPtr->scanAnchorY = y;
	viewPtr->scanX = viewPtr->xOffset;
	viewPtr->scanY = viewPtr->yOffset;
    } else {
	int worldX, worldY;
	int dx, dy;

	dx = viewPtr->scanAnchorX - x;
	dy = viewPtr->scanAnchorY - y;
	worldX = viewPtr->scanX + (10 * dx);
	worldY = viewPtr->scanY + (10 * dy);

	if (worldX < 0) {
	    worldX = 0;
	} else if (worldX >= viewPtr->worldWidth) {
	    worldX = viewPtr->worldWidth - viewPtr->xScrollUnits;
	}
	if (worldY < 0) {
	    worldY = 0;
	} else if (worldY >= viewPtr->worldHeight) {
	    worldY = viewPtr->worldHeight - viewPtr->yScrollUnits;
	}
	viewPtr->xOffset = worldX;
	viewPtr->yOffset = worldY;
	viewPtr->flags |= SCROLL_PENDING;
	EventuallyRedraw(viewPtr);
    }
    return TCL_OK;
}

/*ARGSUSED*/
static int
SeeOp(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    int width, height;
    int x, y;
    Tk_Anchor anchor;
    int left, right, top, bottom;
    char *string;

    string = Tcl_GetString(objv[2]);
    anchor = TK_ANCHOR_W;	/* Default anchor is West */
    if ((string[0] == '-') && (strcmp(string, "-anchor") == 0)) {
	if (objc == 3) {
	    Tcl_AppendResult(interp, "missing \"-anchor\" argument",
		(char *)NULL);
	    return TCL_ERROR;
	}
	if (Tk_GetAnchorFromObj(interp, objv[3], &anchor) != TCL_OK) {
	    return TCL_ERROR;
	}
	objc -= 2, objv += 2;
    }
    if (objc == 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", objv[0],
	    "see ?-anchor anchor? tagOrId\"", (char *)NULL);
	return TCL_ERROR;
    }
    if (GetEntryFromObj(viewPtr, objv[2], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (entryPtr == NULL) {
	return TCL_OK;
    }
    if (entryPtr->flags & ENTRY_HIDE) {
	/*
	 * If the entry wasn't previously exposed, its world coordinates
	 * aren't likely to be valid.  So re-compute the layout before we try
	 * to see the viewport to the entry's location.
	 */
	MapAncestors(viewPtr, entryPtr);
	viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
    }
    if (viewPtr->flags & (LAYOUT_PENDING | DIRTY)) {
	ComputeLayout(viewPtr);
    }
    width = VPORTWIDTH(viewPtr);
    height = VPORTHEIGHT(viewPtr);

    /*
     * XVIEW:	If the entry is left or right of the current view, adjust
     *		the offset.  If the entry is nearby, adjust the view just
     *		a bit.  Otherwise, center the entry.
     */
    left = viewPtr->xOffset;
    right = viewPtr->xOffset + width;

    switch (anchor) {
    case TK_ANCHOR_W:
    case TK_ANCHOR_NW:
    case TK_ANCHOR_SW:
	x = 0;
	break;
    case TK_ANCHOR_E:
    case TK_ANCHOR_NE:
    case TK_ANCHOR_SE:
	x = entryPtr->worldX + entryPtr->width + 
	    ICONWIDTH(DEPTH(viewPtr, entryPtr->node)) - width;
	break;
    default:
	if (entryPtr->worldX < left) {
	    x = entryPtr->worldX;
	} else if ((entryPtr->worldX + entryPtr->width) > right) {
	    x = entryPtr->worldX + entryPtr->width - width;
	} else {
	    x = viewPtr->xOffset;
	}
	break;
    }
    /*
     * YVIEW:	If the entry is above or below the current view, adjust
     *		the offset.  If the entry is nearby, adjust the view just
     *		a bit.  Otherwise, center the entry.
     */
    top = viewPtr->yOffset;
    bottom = viewPtr->yOffset + height;
    switch (anchor) {
    case TK_ANCHOR_N:
	y = viewPtr->yOffset;
	break;
    case TK_ANCHOR_NE:
    case TK_ANCHOR_NW:
	y = entryPtr->worldY - (height / 2);
	break;
    case TK_ANCHOR_S:
    case TK_ANCHOR_SE:
    case TK_ANCHOR_SW:
	y = entryPtr->worldY + entryPtr->height - height;
	break;
    default:
	if (entryPtr->worldY < top) {
	    y = entryPtr->worldY;
	} else if ((entryPtr->worldY + entryPtr->height) > bottom) {
	    y = entryPtr->worldY + entryPtr->height - height;
	} else {
	    y = viewPtr->yOffset;
	}
	break;
    }
    if ((y != viewPtr->yOffset) || (x != viewPtr->xOffset)) {
	/* viewPtr->xOffset = x; */
	viewPtr->yOffset = y;
	viewPtr->flags |= SCROLL_PENDING;
    }
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SelectionAnchorOp --
 *
 *	Sets the selection anchor to the element given by a index.  The
 *	selection anchor is the end of the selection that is fixed while
 *	dragging out a selection with the mouse.  The index "anchor" may be
 *	used to refer to the anchor element.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The selection changes.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SelectionAnchorOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		  Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;

    if (GetEntryFromObj(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    /* Set both the anchor and the mark. Indicates that a single entry
     * is selected. */
    viewPtr->selection.anchorPtr = entryPtr;
    viewPtr->selection.markPtr = NULL;
    if (entryPtr != NULL) {
	Tcl_SetObjResult(interp, NodeToObj(entryPtr->node));
    }
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * SelectionClearallOp
 *
 *	Clears the entire selection.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The selection changes.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SelectionClearallOp(ClientData clientData, Tcl_Interp *interp, int objc,
		    Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;

    ClearSelection(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SelectionIncludesOp
 *
 *	Returns 1 if the element indicated by index is currently
 *	selected, 0 if it isn't.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The selection changes.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SelectionIncludesOp(ClientData clientData, Tcl_Interp *interp, int objc,
		    Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    int bool;

    if (GetEntryFromObj(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    bool = FALSE;
    if (entryPtr != NULL) {
	bool = EntryIsSelected(viewPtr, entryPtr);
    }
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), bool);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SelectionMarkOp --
 *
 *	Sets the selection mark to the element given by a index.  The
 *	selection anchor is the end of the selection that is movable while
 *	dragging out a selection with the mouse.  The index "mark" may be used
 *	to refer to the anchor element.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The selection changes.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SelectionMarkOp(ClientData clientData, Tcl_Interp *interp, int objc,
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;

    if (GetEntryFromObj(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (viewPtr->selection.anchorPtr == NULL) {
	Tcl_AppendResult(interp, "selection anchor must be set first", 
		 (char *)NULL);
	return TCL_ERROR;
    }
    if (viewPtr->selection.markPtr != entryPtr) {
	Blt_ChainLink link, next;

	/* Deselect entry from the list all the way back to the anchor. */
	for (link = Blt_Chain_LastLink(viewPtr->selection.list); link != NULL; 
	     link = next) {
	    Entry *selectPtr;

	    next = Blt_Chain_PrevLink(link);
	    selectPtr = Blt_Chain_GetValue(link);
	    if (selectPtr == viewPtr->selection.anchorPtr) {
		break;
	    }
	    DeselectEntry(viewPtr, selectPtr);
	}
	viewPtr->selection.flags &= ~SELECT_MASK;
	viewPtr->selection.flags |= SELECT_SET;
	SelectRange(viewPtr, viewPtr->selection.anchorPtr, entryPtr);
	Tcl_SetObjResult(interp, NodeToObj(entryPtr->node));
	viewPtr->selection.markPtr = entryPtr;

	EventuallyRedraw(viewPtr);
	if (viewPtr->selection.cmdObjPtr != NULL) {
	    EventuallyInvokeSelectCmd(viewPtr);
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SelectionPresentOp
 *
 *	Returns 1 if there is a selection and 0 if it isn't.
 *
 * Results:
 *	A standard TCL result.  interp->result will contain a boolean string
 *	indicating if there is a selection.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SelectionPresentOp(ClientData clientData, Tcl_Interp *interp, int objc,
		   Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int bool;

    bool = (Blt_Chain_GetLength(viewPtr->selection.list) > 0);
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), bool);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SelectionSetOp
 *
 *	Selects, deselects, or toggles all of the elements in the range
 *	between first and last, inclusive, without affecting the selection
 *	state of elements outside that range.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The selection changes.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SelectionSetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    char *string;

    viewPtr->selection.flags &= ~SELECT_MASK;
    if (viewPtr->flags & (DIRTY | LAYOUT_PENDING)) {
	/*
	 * The layout is dirty.  Recompute it now so that we can use
	 * view.top and view.bottom for nodes.
	 */
	ComputeLayout(viewPtr);
    }
    string = Tcl_GetString(objv[2]);
    switch (string[0]) {
    case 's':
	viewPtr->selection.flags |= SELECT_SET;
	break;
    case 'c':
	viewPtr->selection.flags |= SELECT_CLEAR;
	break;
    case 't':
	viewPtr->selection.flags |= SELECT_TOGGLE;
	break;
    }
    if (objc > 4) {
	Entry *firstPtr, *lastPtr;

	if (GetEntryFromObj(viewPtr, objv[3], &firstPtr) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (firstPtr == NULL) {
	    return TCL_OK;		/* Didn't pick an entry. */
	}
	if ((firstPtr->flags & ENTRY_HIDE) && 
	    (!(viewPtr->selection.flags & SELECT_CLEAR))) {
	    if (objc > 4) {
		Tcl_AppendResult(interp, "can't select hidden node \"", 
			Tcl_GetString(objv[3]), "\"", (char *)NULL);
		return TCL_ERROR;
	    } else {
		return TCL_OK;
	    }
	}
	lastPtr = firstPtr;
	if (objc > 4) {
	    if (GetEntry(viewPtr, objv[4], &lastPtr) != TCL_OK) {
		return TCL_ERROR;
	    }
	    if ((lastPtr->flags & ENTRY_HIDE) && 
		(!(viewPtr->selection.flags & SELECT_CLEAR))) {
		Tcl_AppendResult(interp, "can't select hidden node \"", 
			Tcl_GetString(objv[4]), "\"", (char *)NULL);
		return TCL_ERROR;
	    }
	}
	if (firstPtr == lastPtr) {
	    SelectEntryApplyProc(viewPtr, firstPtr);
	} else {
	    SelectRange(viewPtr, firstPtr, lastPtr);
	}
	/* Set both the anchor and the mark. Indicates that a single entry is
	 * selected. */
	if (viewPtr->selection.anchorPtr == NULL) {
	    viewPtr->selection.anchorPtr = firstPtr;
	}
    } else {
	Entry *entryPtr;
	TagIterator iter;

	if (GetEntryIterator(viewPtr, objv[3], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    if ((entryPtr->flags & ENTRY_HIDE) && 
		((viewPtr->selection.flags & SELECT_CLEAR) == 0)) {
		continue;
	    }
	    SelectEntryApplyProc(viewPtr, entryPtr);
	}
	/* Set both the anchor and the mark. Indicates that a single entry is
	 * selected. */
	if (viewPtr->selection.anchorPtr == NULL) {
	    viewPtr->selection.anchorPtr = entryPtr;
	}
    }
    if (viewPtr->selection.flags & SELECT_EXPORT) {
	Tk_OwnSelection(viewPtr->tkwin, XA_PRIMARY, LostSelection, viewPtr);
    }
    EventuallyRedraw(viewPtr);
    if (viewPtr->selection.cmdObjPtr != NULL) {
	EventuallyInvokeSelectCmd(viewPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SelectionOp --
 *
 *	This procedure handles the individual options for text selections.
 *	The selected text is designated by start and end indices into the text
 *	pool.  The selected segment has both a anchored and unanchored ends.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The selection changes.
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec selectionOps[] =
{
    {"anchor",   1, SelectionAnchorOp,   4, 4, "tagOrId",},
    {"clear",    5, SelectionSetOp,      4, 5, "first ?last?",},
    {"clearall", 6, SelectionClearallOp, 3, 3, "",},
    {"includes", 1, SelectionIncludesOp, 4, 4, "tagOrId",},
    {"mark",     1, SelectionMarkOp,     4, 4, "tagOrId",},
    {"present",  1, SelectionPresentOp,  3, 3, "",},
    {"set",      1, SelectionSetOp,      4, 5, "first ?last?",},
    {"toggle",   1, SelectionSetOp,      4, 5, "first ?last?",},
};
static int numSelectionOps = sizeof(selectionOps) / sizeof(Blt_OpSpec);

static int
SelectionOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	    Tcl_Obj *const *objv)
{
    Tcl_ObjCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, numSelectionOps, selectionOps, BLT_OP_ARG2, 
	objc, objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc) (clientData, interp, objc, objv);
    return result;
}


static int
SortAutoOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	   Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    if (objc == 4) {
	int bool;
	int isAuto;

	isAuto = ((viewPtr->flags & TV_SORT_AUTO) != 0);
	if (Tcl_GetBooleanFromObj(interp, objv[3], &bool) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (isAuto != bool) {
	    viewPtr->flags |= (LAYOUT_PENDING | DIRTY | RESORT);
	    EventuallyRedraw(viewPtr);
	}
	if (bool) {
	    viewPtr->flags |= TV_SORT_AUTO;
	} else {
	    viewPtr->flags &= ~TV_SORT_AUTO;
	}
    }
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp),(viewPtr->flags & TV_SORT_AUTO));
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SortCgetOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
SortCgetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	   Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    return Blt_ConfigureValueFromObj(interp, viewPtr->tkwin, sortSpecs, 
	(char *)viewPtr, objv[3], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * SortConfigureOp --
 *
 * 	This procedure is called to process a list of configuration
 *	options database, in order to reconfigure the one of more
 *	entries in the widget.
 *
 *	  .h sort configure option value
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font,
 *	etc. get set for viewPtr; old resources get freed, if there
 *	were any.  The hypertext is redisplayed.
 *
 *---------------------------------------------------------------------------
 */
static int
SortConfigureOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Tcl_Obj *oldCmdPtr;
    Column *oldColumn;

    if (objc == 3) {
	return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, sortSpecs, 
		(char *)viewPtr, (Tcl_Obj *)NULL, 0);
    } else if (objc == 4) {
	return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, sortSpecs, 
		(char *)viewPtr, objv[3], 0);
    }
    oldColumn = viewPtr->sortInfo.markPtr;
    oldCmdPtr = viewPtr->sortInfo.cmdObjPtr;
    if (Blt_ConfigureWidgetFromObj(interp, viewPtr->tkwin, sortSpecs, 
	objc - 3, objv + 3, (char *)viewPtr, BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((oldColumn != viewPtr->sortInfo.markPtr)|| 
	(oldCmdPtr != viewPtr->sortInfo.cmdObjPtr)) {
	viewPtr->flags &= ~SORTED;
	viewPtr->flags |= (DIRTY | RESORT);
    } 
    if (viewPtr->flags & TV_SORT_AUTO) {
	viewPtr->flags |= SORT_PENDING;
    }
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}


/*ARGSUSED*/
/* .
 * SortChildren --
 *
 *	Sort the tree node 
 *
 *	tv sort children $node 
 *
 */
static int
SortChildrenOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int i;

    treeViewInstance = viewPtr;
    for (i = 3; i < objc; i++) {
	Entry *entryPtr;
	TagIterator iter;

	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    Blt_Tree_SortNode(viewPtr->tree, entryPtr->node, CompareNodes);
	}
    }
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY | UPDATE);
    viewPtr->flags &= ~(SORT_PENDING | RESORT);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SortListOp
 *
 *	Sorts the flatten array of entries.
 *
 *	.tv sort list $node $col
 *
 *---------------------------------------------------------------------------
 */
static int
SortListOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr, *childPtr;
    long i, numChildren, count;
    Blt_TreeNode *nodes;
    Tcl_Obj *listObjPtr;

    if (GetEntry(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    numChildren = Blt_Tree_NodeDegree(entryPtr->node);
    if (numChildren < 2) {
	return TCL_OK;
    }
    nodes = Blt_Malloc((numChildren) * sizeof(Blt_TreeNode));
    if (nodes == NULL) {
	Tcl_AppendResult(interp, "can't allocate sorting array.", (char *)NULL);
	return TCL_ERROR;	/* Out of memory. */
    }
    count = 0;
    for (childPtr = FirstChild(entryPtr, ENTRY_HIDE); childPtr != NULL; 
	 childPtr = NextSibling(childPtr, ENTRY_HIDE)) {
	nodes[count] = childPtr->node;
	count++;
    }
    treeViewInstance = viewPtr;
    qsort(nodes, count, sizeof(Blt_TreeNode), (QSortCompareProc *)CompareNodes);
    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for (i = 0; i < count; i++) {
	long inode;

	inode = Blt_Tree_NodeId(nodes[i]);
	Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewLongObj(inode));
    }
    Blt_Free(nodes);
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SortOnceOp
 *
 *	Sorts the tree.
 *
 *	.tv sort once
 *
 *---------------------------------------------------------------------------
 */
static int
SortOnceOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	   Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    if (viewPtr->flatView) {
	SortFlatView(viewPtr);
    } else {
	SortTreeView(viewPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * SortOp --
 *
 *	Comparison routine (used by qsort) to sort a chain of subnodes.
 *	A simple string comparison is performed on each node name.
 *
 *	.h sort auto
 *	.h sort once root -recurse root
 *
 * Results:
 *	1 is the first is greater, -1 is the second is greater, 0
 *	if equal.
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec sortOps[] =
{
    {"auto",      1, SortAutoOp,      3, 4, "?boolean?",},
    {"cget",      2, SortCgetOp,      4, 4, "option",},
    {"children",  2, SortChildrenOp,  3, 0, "node...",},
    {"configure", 2, SortConfigureOp, 3, 0, "?option value?...",},
    {"list",      1, SortListOp,      4, 4, "node",},
    {"once",      1, SortOnceOp,      3, 3, "",},
};
static int numSortOps = sizeof(sortOps) / sizeof(Blt_OpSpec);

/*ARGSUSED*/
static int
SortOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		    Tcl_Obj *const *objv)
{
    Tcl_ObjCmdProc *proc;
    int result;

    proc = Blt_GetOpFromObj(interp, numSortOps, sortOps, BLT_OP_ARG2, objc, 
	    objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc)(clientData, interp, objc, objv);
    return result;
}


/*
 *---------------------------------------------------------------------------
 *
 * StyleActivateOp --
 *
 * 	Turns on highlighting for a particular style.
 *
 *	  .t style activate entry column
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleActivateOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Value *oldValuePtr;
    Column *colPtr;
    Entry *entryPtr;
    Value *valuePtr;

    oldValuePtr = viewPtr->activeValuePtr;
    if (GetEntry(viewPtr, objv[3], &entryPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if (GetColumn(interp, viewPtr, objv[4], &colPtr) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((colPtr == NULL) || (entryPtr == NULL)) {
	valuePtr = NULL;
    } else {
	valuePtr = Blt_TreeView_FindValue(entryPtr, colPtr);
    }
    if (valuePtr != oldValuePtr) {
	if (oldValuePtr != NULL) {
	    /* Deactivate old value */
	    DisplayValue(viewPtr, entryPtr, oldValuePtr);
	}
	if (valuePtr == NULL) {
	    /* Mark as deactivate */
	    viewPtr->activePtr = NULL;
	    viewPtr->colActivePtr = NULL;
	    viewPtr->activeValuePtr = NULL;
	} else {
	    /* Activate new value. */
	    viewPtr->activePtr = entryPtr;
	    viewPtr->colActivePtr = colPtr;
	    viewPtr->activeValuePtr = valuePtr;
	    DisplayValue(viewPtr, entryPtr, valuePtr);
	}
    }
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * StyleCgetOp --
 *
 *	  .t style cget "styleName" -background
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleCgetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	    Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    ColumnStyle *stylePtr;

    stylePtr = FindStyle(interp, viewPtr, Tcl_GetString(objv[3]));
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    return Blt_ConfigureValueFromObj(interp, viewPtr->tkwin, 
	stylePtr->classPtr->specsPtr, (char *)viewPtr, objv[4], 0);
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleCheckBoxOp --
 *
 *	  .t style checkbox "styleName" -background blue
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleCheckBoxOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    ColumnStyle *stylePtr;

    stylePtr = CreateStyle(interp, viewPtr, STYLE_CHECKBOX, 
	Tcl_GetString(objv[3]), objc - 4, objv + 4);
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    stylePtr->link = Blt_Chain_Append(viewPtr->userStyles, stylePtr);
    ConfigureStyle(viewPtr, stylePtr);
    Tcl_SetObjResult(interp, objv[3]);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleComboBoxOp --
 *
 *	  .t style combobox "styleName" -background blue
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleComboBoxOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    ColumnStyle *stylePtr;

    stylePtr = CreateStyle(interp, viewPtr, STYLE_COMBOBOX, 
	Tcl_GetString(objv[3]), objc - 4, objv + 4);
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    stylePtr->link = Blt_Chain_Append(viewPtr->userStyles, stylePtr);
    ConfigureStyle(viewPtr, stylePtr);
    Tcl_SetObjResult(interp, objv[3]);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleConfigureOp --
 *
 * 	This procedure is called to process a list of configuration options
 * 	database, in order to reconfigure a style.
 *
 *	  .t style configure "styleName" option value
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font, etc. get
 *	set for stylePtr; old resources get freed, if there were any.
 *
 *---------------------------------------------------------------------------
 */
static int
StyleConfigureOp(ClientData clientData, Tcl_Interp *interp, int objc,
		 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    ColumnStyle *stylePtr;

    stylePtr = FindStyle(interp, viewPtr, Tcl_GetString(objv[3]));
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    if (objc == 4) {
	return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, 
	    stylePtr->classPtr->specsPtr, (char *)stylePtr, (Tcl_Obj *)NULL, 0);
    } else if (objc == 5) {
	return Blt_ConfigureInfoFromObj(interp, viewPtr->tkwin, 
		stylePtr->classPtr->specsPtr, (char *)stylePtr, objv[5], 0);
    }
    iconOption.clientData = viewPtr;
    if (Blt_ConfigureWidgetFromObj(interp, viewPtr->tkwin, 
	stylePtr->classPtr->specsPtr, objc - 4, objv + 4, (char *)stylePtr, 
	BLT_CONFIG_OBJV_ONLY) != TCL_OK) {
	return TCL_ERROR;
    }
    (*stylePtr->classPtr->configProc)(stylePtr);
    stylePtr->flags |= STYLE_DIRTY;
    viewPtr->flags |= (LAYOUT_PENDING | DIRTY);
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleDeactivateOp --
 *
 * 	Turns on highlighting for all styles
 *
 *	  .t style deactivate 
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleDeactivateOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Value *oldValuePtr;

    oldValuePtr = viewPtr->activeValuePtr;
    viewPtr->activeValuePtr = NULL;
    if ((oldValuePtr != NULL)  && (viewPtr->activePtr != NULL)) {
	DisplayValue(viewPtr, viewPtr->activePtr, oldValuePtr);
    }
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * StyleForgetOp --
 *
 * 	Eliminates one or more style names.  A style still may be in use after
 * 	its name has been officially removed.  Only its hash table entry is
 * 	removed.  The style itself remains until its reference count returns
 * 	to zero (i.e. no one else is using it).
 *
 *	  .t style forget "styleName"...
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 *---------------------------------------------------------------------------
 */
static int
StyleForgetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	      Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    ColumnStyle *stylePtr;
    int i;

    for (i = 3; i < objc; i++) {
	stylePtr = FindStyle(interp, viewPtr, Tcl_GetString(objv[i]));
	if (stylePtr == NULL) {
	    return TCL_ERROR;
	}
	/* 
	 * Removing the style from the hash tables frees up the style
	 * name again.  The style itself may not be removed until it's
	 * been released by everything using it.
	 */
	if (stylePtr->hashPtr != NULL) {
	    Blt_DeleteHashEntry(&viewPtr->styleTable, stylePtr->hashPtr);
	    stylePtr->hashPtr = NULL;
	} 
	FreeStyle(stylePtr);
    }
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleHighlightOp --
 *
 * 	Turns on/off highlighting for a particular style.
 *
 *	  .t style highlight styleName on|off
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleHighlightOp(ClientData clientData, Tcl_Interp *interp, int objc, 
		 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    ColumnStyle *stylePtr;
    int bool, oldBool;

    stylePtr = FindStyle(interp, viewPtr, Tcl_GetString(objv[3]));
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    if (Tcl_GetBooleanFromObj(interp, objv[4], &bool) != TCL_OK) {
	return TCL_ERROR;
    }
    oldBool = ((stylePtr->flags & STYLE_HIGHLIGHT) != 0);
    if (oldBool != bool) {
	if (bool) {
	    stylePtr->flags |= STYLE_HIGHLIGHT;
	} else {
	    stylePtr->flags &= ~STYLE_HIGHLIGHT;
	}
	EventuallyRedraw(viewPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleNamesOp --
 *
 * 	Lists the names of all the current styles in the treeview widget.
 *
 *	  .t style names
 *
 * Results:
 *	Always TCL_OK.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleNamesOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	     Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Blt_HashEntry *hPtr;
    Blt_HashSearch cursor;
    Tcl_Obj *listObjPtr, *objPtr;
    ColumnStyle *stylePtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    for (hPtr = Blt_FirstHashEntry(&viewPtr->styleTable, &cursor); hPtr != NULL;
	 hPtr = Blt_NextHashEntry(&cursor)) {
	stylePtr = Blt_GetHashValue(hPtr);
	objPtr = Tcl_NewStringObj(stylePtr->name, -1);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleSetOp --
 *
 * 	Sets a style for a given key for all the ids given.
 *
 *	  .t style set styleName key node...
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then interp->result
 *	contains an error message.
 *
 *---------------------------------------------------------------------------
 */
static int
StyleSetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	   Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Blt_TreeKey key;
    ColumnStyle *stylePtr;
    int i;

    stylePtr = FindStyle(interp, viewPtr, Tcl_GetString(objv[3]));
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    key = Blt_Tree_GetKey(viewPtr->tree, Tcl_GetString(objv[4]));
    stylePtr->flags |= STYLE_LAYOUT;
    for (i = 5; i < objc; i++) {
	Entry *entryPtr;
	TagIterator iter;

	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    Value *vp;

	    for (vp = entryPtr->values; vp != NULL; vp = vp->nextPtr) {
		if (vp->columnPtr->key == key) {
		    ColumnStyle *oldStylePtr;

		    stylePtr->refCount++;
		    oldStylePtr = vp->stylePtr;
		    vp->stylePtr = stylePtr;
		    if (oldStylePtr != NULL) {
			FreeStyle(oldStylePtr);
		    }
		    break;
		}
	    }
	}
    }
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleTextBoxOp --
 *
 *	  .t style text "styleName" -background blue
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
StyleTextBoxOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	       Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    ColumnStyle *stylePtr;

    stylePtr = CreateStyle(interp, viewPtr, STYLE_TEXTBOX, 
	Tcl_GetString(objv[3]), objc - 4, objv + 4);
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    stylePtr->link = Blt_Chain_Append(viewPtr->userStyles, stylePtr);
    ConfigureStyle(viewPtr, stylePtr);
    Tcl_SetObjResult(interp, objv[3]);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleUnsetOp --
 *
 * 	Removes a style for a given key for all the ids given.
 *	The cell's style is returned to its default state.
 *
 *	  .t style unset styleName key node...
 *
 * Results:
 *	A standard TCL result.  If TCL_ERROR is returned, then
 *	interp->result contains an error message.
 *
 *---------------------------------------------------------------------------
 */
static int
StyleUnsetOp(ClientData clientData, Tcl_Interp *interp, int objc,
	     Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Blt_TreeKey key;
    ColumnStyle *stylePtr;
    int i;

    stylePtr = FindStyle(interp, viewPtr, Tcl_GetString(objv[3]));
    if (stylePtr == NULL) {
	return TCL_ERROR;
    }
    key = Blt_Tree_GetKey(viewPtr->tree, Tcl_GetString(objv[4]));
    stylePtr->flags |= STYLE_LAYOUT;
    for (i = 5; i < objc; i++) {
	TagIterator iter;
	Entry *entryPtr;

	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    Value *valuePtr;

	    for (valuePtr = entryPtr->values; valuePtr != NULL; 
		 valuePtr = valuePtr->nextPtr) {
		if (valuePtr->columnPtr->key == key) {
		    if (valuePtr->stylePtr != NULL) {
			FreeStyle(valuePtr->stylePtr);
			valuePtr->stylePtr = NULL;
		    }
		    break;
		}
	    }
	}
    }
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * StyleOp --
 *
 *	.t style activate $node $column
 *	.t style activate 
 *	.t style cget "highlight" -foreground
 *	.t style configure "highlight" -fg blue -bg green
 *	.t style checkbox "highlight"
 *	.t style deactivate
 *	.t style highlight "highlight" on|off
 *	.t style combobox "highlight"
 *	.t style text "highlight"
 *	.t style forget "highlight"
 *	.t style get "mtime" $node
 *	.t style names
 *	.t style set "mtime" "highlight" all
 *	.t style unset "mtime" all
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec styleOps[] = {
    {"activate",    1, StyleActivateOp,    5, 5, "entry column",},
    {"cget",        2, StyleCgetOp,        5, 5, "styleName option",},
    {"checkbox",    2, StyleCheckBoxOp,    4, 0, "styleName options...",},
    {"combobox",    3, StyleComboBoxOp,    4, 0, "styleName options...",},
    {"configure",   3, StyleConfigureOp,   4, 0, "styleName options...",},
    {"deactivate",  1, StyleDeactivateOp,  3, 3, "",},
    {"forget",      1, StyleForgetOp,      3, 0, "styleName...",},
    {"highlight",   1, StyleHighlightOp,   5, 5, "styleName boolean",},
    {"names",       1, StyleNamesOp,       3, 3, "",}, 
    {"set",         1, StyleSetOp,         6, 6, "key styleName tagOrId...",},
    {"textbox",     1, StyleTextBoxOp,     4, 0, "styleName options...",},
    {"unset",       1, StyleUnsetOp,       5, 5, "key tagOrId",},
};

static int numStyleOps = sizeof(styleOps) / sizeof(Blt_OpSpec);

static int
StyleOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    int result;
    Tcl_ObjCmdProc *proc;

    proc = Blt_GetOpFromObj(interp, numStyleOps, styleOps, BLT_OP_ARG2, objc, 
	objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc)(clientData, interp, objc, objv);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * TagForgetOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
TagForgetOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	    Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int i;

    for (i = 3; i < objc; i++) {
	Blt_Tree_ForgetTag(viewPtr->tree, Tcl_GetString(objv[i]));
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TagNamesOp --
 *
 *---------------------------------------------------------------------------
 */
static int
TagNamesOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	   Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Tcl_Obj *listObjPtr, *objPtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
    objPtr = Tcl_NewStringObj("all", -1);
    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    if (objc == 3) {
	Blt_HashEntry *hPtr;
	Blt_HashSearch cursor;
	Blt_TreeTagEntry *tPtr;

	objPtr = Tcl_NewStringObj("root", -1);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	for (hPtr = Blt_Tree_FirstTag(viewPtr->tree, &cursor); hPtr != NULL;
	     hPtr = Blt_NextHashEntry(&cursor)) {
	    tPtr = Blt_GetHashValue(hPtr);
	    objPtr = Tcl_NewStringObj(tPtr->tagName, -1);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	}
    } else {
	int i;

	for (i = 3; i < objc; i++) {
	    Blt_Chain tags;
	    Blt_ChainLink link;
	    Entry *entryPtr;

	    if (GetEntry(viewPtr, objv[i], &entryPtr) != TCL_OK) {
		return TCL_ERROR;
	    }
	    tags = Blt_Chain_Create();
	    AddEntryTags(interp, viewPtr, entryPtr, tags);
	    for (link = Blt_Chain_FirstLink(tags); link != NULL; 
		 link = Blt_Chain_NextLink(link)) {
		objPtr = Tcl_NewStringObj(Blt_Chain_GetValue(link), -1);
		Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	    }
	    Blt_Chain_Destroy(tags);
	}
    }
    Tcl_SetObjResult(interp, listObjPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TagNodesOp --
 *
 *---------------------------------------------------------------------------
 */
static int
TagNodesOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	   Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Blt_HashTable nodeTable;
    int i;

    Blt_InitHashTable(&nodeTable, BLT_ONE_WORD_KEYS);
    for (i = 3; i < objc; i++) {
	TagIterator iter;
	Entry *entryPtr;

	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    int isNew;

	    Blt_CreateHashEntry(&nodeTable, (char *)entryPtr->node, &isNew);
	}
    }
    {
	Blt_HashEntry *hPtr;
	Blt_HashSearch cursor;
	Tcl_Obj *listObjPtr;

	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
	for (hPtr = Blt_FirstHashEntry(&nodeTable, &cursor); hPtr != NULL; 
	     hPtr = Blt_NextHashEntry(&cursor)) {
	    Blt_TreeNode node;
	    Tcl_Obj *objPtr;
	    
	    node = (Blt_TreeNode)Blt_GetHashKey(&nodeTable, hPtr);
	    objPtr = Tcl_NewLongObj(Blt_Tree_NodeId(node));
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	}
	Tcl_SetObjResult(interp, listObjPtr);
    }
    Blt_DeleteHashTable(&nodeTable);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TagAddOp --
 *
 *---------------------------------------------------------------------------
 */
static int
TagAddOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    int i;
    char *tagName;
    TagIterator iter;

    tagName = Tcl_GetString(objv[3]);
    viewPtr->fromPtr = NULL;
    if (strcmp(tagName, "root") == 0) {
	Tcl_AppendResult(interp, "can't add reserved tag \"", tagName, "\"", 
		(char *)NULL);
	return TCL_ERROR;
    }
    if (isdigit(UCHAR(tagName[0]))) {
	long nodeId;
	
	if (Blt_GetLongFromObj(NULL, objv[3], &nodeId) == TCL_OK) {
	    Tcl_AppendResult(viewPtr->interp, "invalid tag \"", tagName, 
			     "\": can't be a number.", (char *)NULL);
	    return TCL_ERROR;
	} 
    }
    if (tagName[0] == '@') {
	Tcl_AppendResult(viewPtr->interp, "invalid tag \"", tagName, 
		"\": can't start with \"@\"", (char *)NULL);
	return TCL_ERROR;
    } 
    if (GetEntryFromSpecialId(viewPtr, tagName, &entryPtr) == TCL_OK) {
	Tcl_AppendResult(interp, "invalid tag \"", tagName, 
		 "\": is a special id", (char *)NULL);
	return TCL_ERROR;
    }
    for (i = 4; i < objc; i++) {
	if (GetEntryIterator(viewPtr, objv[i], &iter) != TCL_OK) {
	    return TCL_ERROR;
	}
	for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	     entryPtr = NextTaggedEntry(&iter)) {
	    if (AddTag(viewPtr, entryPtr->node, tagName) != TCL_OK) {
		return TCL_ERROR;
	    }
	}
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TagDeleteOp --
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
TagDeleteOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	    Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    char *tagName;
    Blt_HashTable *tablePtr;

    tagName = Tcl_GetString(objv[3]);
    tablePtr = Blt_Tree_TagHashTable(viewPtr->tree, tagName);
    if (tablePtr != NULL) {
        int i;

        for (i = 4; i < objc; i++) {
	    Entry *entryPtr;
	    TagIterator iter;

	    if (GetEntryIterator(viewPtr, objv[i], &iter)!= TCL_OK) {
		return TCL_ERROR;
	    }
	    for (entryPtr = FirstTaggedEntry(&iter); 
		entryPtr != NULL; 
		entryPtr = NextTaggedEntry(&iter)) {
		Blt_HashEntry *hPtr;

	        hPtr = Blt_FindHashEntry(tablePtr, (char *)entryPtr->node);
	        if (hPtr != NULL) {
		    Blt_DeleteHashEntry(tablePtr, hPtr);
	        }
	   }
       }
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TagOp --
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec tagOps[] = {
    {"add",    1, TagAddOp,    5, 0, "tag id...",},
    {"delete", 1, TagDeleteOp, 5, 0, "tag id...",},
    {"forget", 1, TagForgetOp, 4, 0, "tag...",},
    {"names",  2, TagNamesOp,  3, 0, "?id...?",}, 
    {"nodes",  2, TagNodesOp,  4, 0, "tag ?tag...?",},
};

static int numTagOps = sizeof(tagOps) / sizeof(Blt_OpSpec);

static int
TagOp(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv)
{
    int result;
    Tcl_ObjCmdProc *proc;

    proc = Blt_GetOpFromObj(interp, numTagOps, tagOps, BLT_OP_ARG2, objc, objv, 
	0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    result = (*proc)(clientData, interp, objc, objv);
    return result;
}

/*ARGSUSED*/
static int
ToggleOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	 Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    Entry *entryPtr;
    TagIterator iter;
    int result;

    if (GetEntryIterator(viewPtr, objv[2], &iter) != TCL_OK) {
	return TCL_ERROR;
    }
    result = TCL_OK;			/* Suppress compiler warning. */
    for (entryPtr = FirstTaggedEntry(&iter); entryPtr != NULL; 
	 entryPtr = NextTaggedEntry(&iter)) {
	if (entryPtr == NULL) {
	    return TCL_OK;
	}
	if (entryPtr->flags & ENTRY_CLOSED) {
	    result = OpenEntry(viewPtr, entryPtr);
	} else {
	    PruneSelection(viewPtr, viewPtr->focusPtr);
	    if ((viewPtr->focusPtr != NULL) && 
		(Blt_Tree_IsAncestor(entryPtr->node, viewPtr->focusPtr->node))){
		viewPtr->focusPtr = entryPtr;
		Blt_SetFocusItem(viewPtr->bindTable, entryPtr, ITEM_ENTRY);
	    }
	    if ((viewPtr->selection.anchorPtr != NULL) &&
		(Blt_Tree_IsAncestor(entryPtr->node, 
			viewPtr->selection.anchorPtr->node))) {
		viewPtr->selection.anchorPtr = NULL;
	    }
	    result = CloseEntry(viewPtr, entryPtr);
	}
    }
    viewPtr->flags |= SCROLL_PENDING;
    EventuallyRedraw(viewPtr);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * UpdatesOp --
 *
 *	.tv updates false
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
UpdatesOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	  Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int state;

    if (objc == 3) {
	if (Tcl_GetBooleanFromObj(interp, objv[2], &state) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (state) {
	    viewPtr->flags &= ~DONT_UPDATE;
	    viewPtr->flags |= LAYOUT_PENDING | DIRTY;
	    EventuallyRedraw(viewPtr);
	} else {
	    viewPtr->flags |= DONT_UPDATE;
	}
    } else {
	state = (viewPtr->flags & DONT_UPDATE) ? FALSE : TRUE;
    }
    Tcl_SetBooleanObj(Tcl_GetObjResult(interp), state);
    return TCL_OK;
}

static int
XViewOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int width, worldWidth;

    width = VPORTWIDTH(viewPtr);
    worldWidth = viewPtr->worldWidth;
    if (objc == 2) {
	double fract;
	Tcl_Obj *listObjPtr;

	/*
	 * Note that we are bounding the fractions between 0.0 and 1.0
	 * to support the "canvas"-style of scrolling.
	 */
	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	fract = (double)viewPtr->xOffset / worldWidth;
	fract = FCLAMP(fract);
	Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewDoubleObj(fract));
	fract = (double)(viewPtr->xOffset + width) / worldWidth;
	fract = FCLAMP(fract);
	Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewDoubleObj(fract));
	Tcl_SetObjResult(interp, listObjPtr);
	return TCL_OK;
    }
    if (Blt_GetScrollInfoFromObj(interp, objc - 2, objv + 2, &viewPtr->xOffset,
	    worldWidth, width, viewPtr->xScrollUnits, viewPtr->scrollMode) 
	    != TCL_OK) {
	return TCL_ERROR;
    }
    viewPtr->flags |= SCROLLX;
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

static int
YViewOp(ClientData clientData, Tcl_Interp *interp, int objc, 
	Tcl_Obj *const *objv)
{
    TreeView *viewPtr = clientData;
    int height, worldHeight;

    height = VPORTHEIGHT(viewPtr);
    worldHeight = viewPtr->worldHeight;
    if (objc == 2) {
	double fract;
	Tcl_Obj *listObjPtr;

	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	/* Report first and last fractions */
	fract = (double)viewPtr->yOffset / worldHeight;
	fract = FCLAMP(fract);
	Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewDoubleObj(fract));
	fract = (double)(viewPtr->yOffset + height) / worldHeight;
	fract = FCLAMP(fract);
	Tcl_ListObjAppendElement(interp, listObjPtr, Tcl_NewDoubleObj(fract));
	Tcl_SetObjResult(interp, listObjPtr);
	return TCL_OK;
    }
    if (Blt_GetScrollInfoFromObj(interp, objc - 2, objv + 2, &viewPtr->yOffset,
	    worldHeight, height, viewPtr->yScrollUnits, viewPtr->scrollMode)
	!= TCL_OK) {
	return TCL_ERROR;
    }
    viewPtr->flags |= SCROLL_PENDING;
    EventuallyRedraw(viewPtr);
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TreeViewInstCmdProc --
 *
 * 	This procedure is invoked to process commands on behalf of the
 * 	treeview widget.
 *
 * Results:
 *	A standard TCL result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *---------------------------------------------------------------------------
 */
static Blt_OpSpec viewOps[] =
{
    {"bbox",         2, BboxOp,          3, 0, "tagOrId...",}, 
    {"bind",         2, BindOp,          3, 5, "tagName ?sequence command?",}, 
    {"button",       2, ButtonOp,        2, 0, "args",},
    {"cget",         2, CgetOp,          3, 3, "option",}, 
    {"chroot",       2, ChrootOp,        2, 3, "tagOrId",}, 
    {"close",        2, CloseOp,         2, 0, "?-recurse? tagOrId...",}, 
    {"column",       3, ColumnOp,	 2, 0, "oper args",}, 
    {"configure",    3, ConfigureOp,     2, 0, "?option value?...",},
    {"curselection", 2, CurselectionOp,  2, 2, "",},
    {"delete",       1, DeleteOp,        2, 0, "tagOrId ?tagOrId...?",}, 
    {"edit",         2, EditOp,          4, 6, "?-root|-test? x y",},
    {"entry",        2, EntryOp,         2, 0, "oper args",},
    {"find",         2, FindOp,          2, 0, "?flags...? ?first last?",}, 
    {"focus",        2, FocusOp,         3, 3, "tagOrId",}, 
    {"get",          1, GetOp,           2, 0, "?-full? tagOrId ?tagOrId...?",},
    {"hide",         1, HideOp,          2, 0, "?-exact? ?-glob? ?-regexp? ?-nonmatching? ?-name string? ?-full string? ?-data string? ?--? ?tagOrId...?",},
    {"index",        3, IndexOp,         3, 6, "?-at tagOrId? ?-path? string",},
    {"insert",       3, InsertOp,        3, 0, 
	"?-at tagOrId? position label ?label...? ?option value?",},
    {"invoke",       3, InvokeOp,        3, 3, "tagOrId",}, 
    {"move",         1, MoveOp,          5, 5, 
	"tagOrId into|before|after tagOrId",},
    {"nearest",      1, NearestOp,       4, 5, "x y ?varName?",}, 
    {"open",         1, OpenOp,          2, 0, "?-recurse? tagOrId...",}, 
    {"range",        1, RangeOp,         4, 5, "?-open? tagOrId tagOrId",},
    {"scan",         2, ScanOp,          5, 5, "dragto|mark x y",},
    {"see",          3, SeeOp,           3, 0, "?-anchor anchor? tagOrId",},
    {"selection",    3, SelectionOp,     2, 0, "oper args",},
    {"show",         2, ShowOp,          2, 0, "?-exact? ?-glob? ?-regexp? ?-nonmatching? ?-name string? ?-full string? ?-data string? ?--? ?tagOrId...?",},
    {"sort",         2, SortOp,		 2, 0, "args",},
    {"style",        2, StyleOp,         2, 0, "args",},
    {"tag",          2, TagOp,           2, 0, "oper args",},
    {"toggle",       2, ToggleOp,        3, 3, "tagOrId",},
    {"updates",      1, UpdatesOp,       2, 3, "?bool?",},
    {"xview",        1, XViewOp,         2, 5, 
	"?moveto fract? ?scroll number what?",},
    {"yview",        1, YViewOp,         2, 5, 
	"?moveto fract? ?scroll number what?",},
};

static int numViewOps = sizeof(viewOps) / sizeof(Blt_OpSpec);

static int
TreeViewInstCmdProc(ClientData clientData, Tcl_Interp *interp, int objc,
		    Tcl_Obj *const *objv)
{
    Tcl_ObjCmdProc *proc;
    TreeView *viewPtr = clientData;
    int result;

    proc = Blt_GetOpFromObj(interp, numViewOps, viewOps, BLT_OP_ARG1, 
	objc, objv, 0);
    if (proc == NULL) {
	return TCL_ERROR;
    }
    Tcl_Preserve(viewPtr);
    result = (*proc) (clientData, interp, objc, objv);
    Tcl_Release(viewPtr);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * TreeViewCmd --
 *
 * 	This procedure is invoked to process the TCL command that corresponds to
 * 	a widget managed by this module. See the user documentation for details
 * 	on what it does.
 *
 * Results:
 *	A standard TCL result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *---------------------------------------------------------------------------
 */
/* ARGSUSED */
static int
TreeViewCmdProc(
    ClientData clientData,		/* Main window associated with
					 * interpreter. */
    Tcl_Interp *interp,			/* Current interpreter. */
    int objc,				/* Number of arguments. */
    Tcl_Obj *const *objv)		/* Argument strings. */
{
    TreeView *viewPtr;
    Tcl_Obj *initObjv[2];
    char *string;
    int result;

    string = Tcl_GetString(objv[0]);
    if (objc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", string, 
		" pathName ?option value?...\"", (char *)NULL);
	return TCL_ERROR;
    }
    viewPtr = CreateTreeView(interp, objv[1]);
    if (viewPtr == NULL) {
	goto error;
    }

    /*
     * Invoke a procedure to initialize various bindings on treeview entries.
     * If the procedure doesn't already exist, source it from
     * "$blt_library/treeview.tcl".  We deferred sourcing the file until now so
     * that the variable $blt_library could be set within a script.
     */
    if (!Blt_CommandExists(interp, "::blt::TreeView::Initialize")) {
	if (Tcl_GlobalEval(interp, 
		"source [file join $blt_library treeview.tcl]") != TCL_OK) {
	    char info[200];

	    Blt_FormatString(info, 200, "\n    (while loading bindings for %.50s)", 
		    Tcl_GetString(objv[0]));
	    Tcl_AddErrorInfo(interp, info);
	    goto error;
	}
    }
    /* 
     * Initialize the widget's configuration options here. The options need to
     * be set first, so that entry, column, and style components can use them
     * for their own GCs.
     */
    iconsOption.clientData = viewPtr;
    treeOption.clientData = viewPtr;
    if (Blt_ConfigureWidgetFromObj(interp, viewPtr->tkwin, viewSpecs, 
	objc - 2, objv + 2, (char *)viewPtr, 0) != TCL_OK) {
	goto error;
    }
    if (Blt_ConfigureComponentFromObj(interp, viewPtr->tkwin, "button", 
	"Button", buttonSpecs, 0, (Tcl_Obj **)NULL, (char *)viewPtr,
	0) != TCL_OK) {
	goto error;
    }

    /* 
     * Rebuild the widget's GC and other resources that are predicated by the
     * widget's configuration options.  Do the same for the default column.
     */
    if (ConfigureTreeView(interp, viewPtr) != TCL_OK) {
	goto error;
    }
    uidOption.clientData = viewPtr;
    iconOption.clientData = viewPtr;
    styleOption.clientData = viewPtr;
    if (Blt_ConfigureComponentFromObj(viewPtr->interp, viewPtr->tkwin, 
	"treeView", "Column", columnSpecs, 0, (Tcl_Obj **)NULL, 
	(char *)&viewPtr->treeColumn, 0) != TCL_OK) {
	goto error;
    }
    ConfigureColumn(viewPtr, &viewPtr->treeColumn);
    ConfigureStyle(viewPtr, viewPtr->stylePtr);

    /*
     * Invoke a procedure to initialize various bindings on treeview entries.
     * If the procedure doesn't already exist, source it from
     * "$blt_library/treeview.tcl".  We deferred sourcing the file until now
     * so that the variable $blt_library could be set within a script.
     */
    initObjv[0] = Tcl_NewStringObj("::blt::TreeView::Initialize", -1);
    initObjv[1] = objv[1];
    Tcl_IncrRefCount(initObjv[0]);
    Tcl_IncrRefCount(initObjv[1]);
    result = Tcl_EvalObjv(interp, 2, initObjv, TCL_EVAL_GLOBAL);
    Tcl_DecrRefCount(initObjv[1]);
    Tcl_DecrRefCount(initObjv[0]);
    if (result != TCL_OK) {
	goto error;
    }
    Tcl_SetStringObj(Tcl_GetObjResult(interp), Tk_PathName(viewPtr->tkwin), -1);
    return TCL_OK;
  error:
    if (viewPtr != NULL) {
	Tk_DestroyWindow(viewPtr->tkwin);
    }
    return TCL_ERROR;
}

int
Blt_TreeViewCmdInitProc(Tcl_Interp *interp)
{
    static Blt_CmdSpec cmdSpecs[] = { 
	{ "treeview", TreeViewCmdProc, },
    };

    return Blt_InitCmds(interp, "::blt", cmdSpecs, 1);
}

#endif /*NO_TREEVIEW*/