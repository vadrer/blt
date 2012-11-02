
/*
 * bltGrLine2.c --
 *
 * This module implements line graph and stripchart elements for the BLT graph
 * widget.
 *
 *	Copyright (c) 1993 George A Howlett.
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

#ifdef HAVE_STRING_H
#  include <string.h>
#endif /* HAVE_STRING_H */

#include <X11/Xutil.h>
#include "bltMath.h"
#include "bltBind.h"
#include "bltPs.h"
#include "bltBg.h"
#include "tkDisplay.h"
#include "bltImage.h"
#include "bltBitmap.h"
#include "bltPicture.h"
#include "bltGraph.h"
#include "bltGrAxis.h"
#include "bltGrLegd.h"
#include "bltGrElem.h"
#include "bltPainter.h"

#define JCLAMP(c)	((((c) < 0.0) ? 0.0 : ((c) > 1.0) ? 1.0 : (c)))

#define SQRT_PI		1.77245385090552
#define S_RATIO		0.886226925452758

/* Trace flags. */
#define RECOUNT		(1<<10)		/* Trace needs to be fixed. */

/* Flags for trace's point and segments. */
#define VISIBLE		(1<<0)		/* Point is on visible on screen. */
#define KNOT		(1<<1)		/* Point is a knot, original data 
					 * point. */
#define SYMBOL		(1<<2)		/* Point is designated to have a
					 * symbol. This is only used when
					 * reqMaxSymbols is non-zero. */
#define ACTIVE_POINT	(1<<3)		/* Point is active. This is only used
					 * when numActiveIndices is greater
					 * than zero. */
/* Flags describing visibility of error bars. */
#define XLOW		(1<<6)		/* Segment is part of the low x-value
					 * error bar. */
#define XHIGH		(1<<7)		/* Segment is part of the high x-value
					 * error bar. */
#define XERROR		(XHIGH | XLOW)	/* Display both low and high error bars 
					 * for x-coordinates. */
#define YLOW		(1<<8)		/* Segment is part of the low y-value
					 * error bar. */
#define YHIGH		(1<<9)		/* Segment is part of the high y-value
					 * error bar. */
#define YERROR		(YHIGH | YLOW)	/* Display both low and high error
					 * bars for y-coordinates. */

#define NOTPLAYING(g,i) \
    (((g)->play.enabled) && (((i) < (g)->play.t1) || ((i) > (g)->play.t2)))

#define PLAYING(g,i) \
    ((!(g)->play.enabled) || (((i) >= (g)->play.t1) && ((i) <= (g)->play.t2)))

#define DRAWN(t,f)     (((f) & (t)->drawFlags) == (t)->drawFlags)

#define BROKEN_TRACE(dir,last,next) \
    (((((dir) & PEN_DECREASING) == 0) && ((next) < (last))) || \
     ((((dir) & PEN_INCREASING) == 0) && ((next) > (last))))

/*
 * XDrawLines() points: XMaxRequestSize(dpy) - 3
 * XFillPolygon() points:  XMaxRequestSize(dpy) - 4
 * XDrawSegments() segments:  (XMaxRequestSize(dpy) - 3) / 2
 * XDrawRectangles() rectangles:  (XMaxRequestSize(dpy) - 3) / 2
 * XFillRectangles() rectangles:  (XMaxRequestSize(dpy) - 3) / 2
 * XDrawArcs() or XFillArcs() arcs:  (XMaxRequestSize(dpy) - 3) / 3
 */

#define MAX_DRAWPOINTS(d)	Blt_MaxRequestSize(d, sizeof(XPoint))
#define MAX_DRAWLINES(d)	Blt_MaxRequestSize(d, sizeof(XPoint))
#define MAX_DRAWPOLYGON(d)	Blt_MaxRequestSize(d, sizeof(XPoint))
#define MAX_DRAWSEGMENTS(d)	Blt_MaxRequestSize(d, sizeof(XSegment))
#define MAX_DRAWRECTANGLES(d)	Blt_MaxRequestSize(d, sizeof(XRectangle))
#define MAX_DRAWARCS(d)		Blt_MaxRequestSize(d, sizeof(XArc))

#define COLOR_DEFAULT	(XColor *)1
#define PATTERN_SOLID	((Pixmap)1)

#define PEN_INCREASING  1		/* Draw line segments for only those
					 * data points whose abscissas are
					 * monotonically increasing in
					 * order. */
#define PEN_DECREASING  2		/* Lines will be drawn between only
					 * those points whose abscissas are
					 * decreasing in order. */

#define PEN_BOTH_DIRECTIONS	(PEN_INCREASING | PEN_DECREASING)

/* Lines will be drawn between points regardless of the ordering of the
 * abscissas */


#define SMOOTH_NONE		0	/* Line segments */
#define SMOOTH_STEP		1	/* Step-and-hold */
#define SMOOTH_NATURAL		2	/* Natural cubic spline */
#define SMOOTH_QUADRATIC	3	/* Quadratic spline */
#define SMOOTH_CATROM		4	/* Catrom spline */

#define SMOOTH_PARAMETRIC	8	/* Parametric spline */

typedef struct {
    const char *name;
    int flags;
} SmoothingTable;

static SmoothingTable smoothingTable[] = {
    { "none",			SMOOTH_NONE				},
    { "linear",			SMOOTH_NONE				},
    { "step",			SMOOTH_STEP				},
    { "natural",		SMOOTH_NATURAL				},
    { "cubic",			SMOOTH_NATURAL				},
    { "quadratic",		SMOOTH_QUADRATIC			},
    { "catrom",			SMOOTH_CATROM				},
    { "parametriccubic",	SMOOTH_NATURAL | SMOOTH_PARAMETRIC	},
    { "parametricquadratic",	SMOOTH_QUADRATIC | SMOOTH_PARAMETRIC	},
    { (char *)NULL,		0					}
};

/* Symbol types for line elements */
typedef enum {
    SYMBOL_NONE,
    SYMBOL_SQUARE,
    SYMBOL_CIRCLE,
    SYMBOL_DIAMOND,
    SYMBOL_PLUS,
    SYMBOL_CROSS,
    SYMBOL_SPLUS,
    SYMBOL_SCROSS,
    SYMBOL_TRIANGLE,
    SYMBOL_ARROW,
    SYMBOL_BITMAP,
    SYMBOL_IMAGE
} SymbolType;

typedef struct {
    const char *name;
    int minChars;
    SymbolType type;
} SymbolTable;

static SymbolTable symbolTable[] = {
    { "arrow",	  1, SYMBOL_ARROW,	},
    { "circle",	  2, SYMBOL_CIRCLE,	},
    { "cross",	  2, SYMBOL_CROSS,	}, 
    { "diamond",  1, SYMBOL_DIAMOND,	}, 
    { "image",    1, SYMBOL_IMAGE,	}, 
    { "none",	  1, SYMBOL_NONE,	}, 
    { "plus",	  1, SYMBOL_PLUS,	}, 
    { "scross",	  2, SYMBOL_SCROSS,	}, 
    { "splus",	  2, SYMBOL_SPLUS,	}, 
    { "square",	  2, SYMBOL_SQUARE,	}, 
    { "triangle", 1, SYMBOL_TRIANGLE,	}, 
    { NULL,       0, 0			}, 
};

typedef struct _LineElement LineElement;

typedef struct {
    SymbolType type;			/* Type of symbol to be drawn/printed */
    int size;				/* Requested size of symbol in pixels */
    XColor *outlineColor;		/* Outline color */
    int outlineWidth;			/* Width of the outline */
    GC outlineGC;			/* Outline graphics context */
    XColor *fillColor;			/* Normal fill color */
    GC fillGC;				/* Fill graphics context */

    Tk_Image image;			/* This is used of image symbols.  */

    /* The last two fields are used only for bitmap symbols. */
    Pixmap bitmap;			/* Bitmap to determine
					 * foreground/background pixels of the
					 * symbol */
    Pixmap mask;			/* Bitmap representing the transparent
					 * pixels of the symbol */
} Symbol;

typedef struct {
    const char *name;			/* Pen style identifier.  If NULL pen
					 * was statically allocated. */
    ClassId classId;			/* Type of pen */
    const char *typeId;			/* String token identifying the type of
					 * pen. */
    unsigned int flags;			/* Indicates if the pen element is
					 * active or normal */
    int refCount;			/* Reference count for elements using
					 * this pen. */
    Blt_HashEntry *hashPtr;
    Blt_ConfigSpec *configSpecs;	/* Configuration specifications */
    PenConfigureProc *configProc;
    PenDestroyProc *destroyProc;
    Graph *graphPtr;			/* Graph that the pen is associated
					 * with. */

    /* Symbol attributes. */
    Symbol symbol;			/* Element symbol type */

    /* Trace attributes. */
    Blt_Dashes traceDashes;		/* Dash on-off list value */
    XColor *traceColor;			/* Line segment color */
    XColor *traceOffColor;		/* Line segment dash gap color */
    GC traceGC;				/* Line segment graphics context */
    int traceWidth;			/* Width of the line segments. If
					 * lineWidth is 0, no line will be
					 * drawn, only symbols. */

    /* Error bar attributes. */
    unsigned int errorFlags;		/* Indicates error bars to display. */
    int errorLineWidth;			/* Width of the error bar segments. */
    int errorCapWidth;			/* Width of the cap on error bars. */
    XColor *errorColor;			/* Color of the error bar. */
    GC errorGC;				/* Error bar graphics context. */

    /* Show value attributes. */
    unsigned int valueFlags;		/* Indicates whether to display text
					 * of the data value.  Values are x,
					 * y, both, or none. */
    const char *valueFormat;		/* A printf format string. */
    TextStyle valueStyle;		/* Text attributes (color, font,
					 * rotation, etc.) of the value. */
} LinePen;

/* 
 * A TraceSegment represents the individual line segment (which is part of an
 * error bar) in a trace.  Included is the both the index of the data point 
 * it is associated with and the flags or the point (if it's active, etc).
 */
typedef struct _TraceSegment {
    struct _TraceSegment *next;		/* Pointer to next point in trace. */
    float x1, y1, x2, y2;		/* Screen coordinate of the point. */
    int index;				/* Index of this coordinate pointing
					 * back to the raw world values in the
					 * individual data arrays. This index
					 * is replicated for generated
					 * values. */
    unsigned int flags;			/* Flags associated with a segment are
					 * described below. */
} TraceSegment;

/* 
 * A TracePoint represents the individual point in a trace. 
 */
typedef struct _TracePoint {
    struct _TracePoint *next;		/* Pointer to next point in trace. */
    float x, y;				/* Screen coordinate of the point. */
    int index;				/* Index of this coordinate pointing
					 * back to the raw world values in the
					 * individual data arrays. This index
					 * is replicated for generated
					 * values. */
    unsigned int flags;			/* Flags associated with a point are
					 * described below. */
} TracePoint;


/* 
 * A trace represents a polyline of connected line segments using the same
 * line and symbol style.  They are stored in a chain of traces.
 */
typedef struct _Trace {
    LineElement *elemPtr;
    TracePoint *head, *tail;
    int numPoints;			/* # of points in the trace. */
    Blt_ChainLink link;
    LinePen *penPtr;
    int symbolSize;
    int errorCapWidth;
    unsigned short flags;		/* Flags associated with a trace are
					 * described blow. */
    unsigned short drawFlags;		/* Flags for individual points and 
					 * segments when drawing the trace. */
    TraceSegment *segments;		/* Segments used for errorbars. */
    int numSegments;
    Point2d *fillPts;			/* Polygon representing the area under
					 * the curve.  May be a degenerate
					 * polygon. */
    int numFillPts;
} Trace;

typedef struct {
    Weight weight;			/* Weight range where this pen is
					 * valid. */
    LinePen *penPtr;			/* Pen to use. */
} LineStyle;

struct _LineElement {
    GraphObj obj;			/* Must be first field in element. */
    unsigned int flags;		
    Blt_HashEntry *hashPtr;

    /* Fields specific to elements. */
    Blt_ChainLink link;			/* Element's link in display list. */
    const char *label;			/* Label displayed in legend */
    unsigned short row, col;		/* Position of the entry in the
					 * legend. */
    int legendRelief;			/* Relief of label in legend. */
    Axis2d axes;			/* X-axis and Y-axis mapping the
					 * element */
    ElemValues x, y, w;			/* Contains array of floating point
					 * graph coordinate values. Also holds
					 * min/max * and the number of
					 * coordinates */
    Blt_HashTable activeTable;		/* Table of indices which indicate
					 * which data points are active (drawn
					 * * with "active" colors). */
    int numActiveIndices;		/* Number of active data points.
					 * Special case: if numActiveIndices <
					 * 0 and the active bit is set in
					 * "flags", then all data points are
					 * drawn active. */
    ElementProcs *procsPtr;
    Blt_ConfigSpec *configSpecs;	/* Configuration specifications. */
    LinePen *activePenPtr;		/* Standard Pens */
    LinePen *normalPenPtr;
    LinePen *builtinPenPtr;
    Blt_Chain styles;			/* Palette of pens. */

    /* Symbol scaling */
    int scaleSymbols;			/* If non-zero, the symbols will scale
					 * in size as the graph is zoomed
					 * in/out.  */

    double xRange, yRange;		/* Initial X-axis and Y-axis ranges:
					 * used to scale the size of element's
					 * symbol. */
    int state;

    /* The line element specific fields start here. */

    ElemValues xError;			/* Relative/symmetric X error values. */
    ElemValues yError;			/* Relative/symmetric Y error values. */
    ElemValues xHigh, xLow;		/* Absolute/asymmetric X-coordinate
					 * high/low error values. */
    ElemValues yHigh, yLow;		/* Absolute/asymmetric Y-coordinate
					 * high/low error values. */
    LinePen builtinPen;
    int errorCapWidth;			/* Length of cap on error bars */

    /* Line smoothing */
    unsigned int reqSmooth;		/* Requested smoothing function to use
					 * for connecting the data points */
    unsigned int smooth;		/* Smoothing function used. */
    float rTolerance;			/* Tolerance to reduce the number of
					 * points displayed. */

    /* Drawing-related data structures. */

    /* Area-under-curve fill attributes. */
    XColor *fillFgColor;
    XColor *fillBgColor;
    GC fillGC;

    Blt_Bg fillBg;			/* Fill background for area under 
					 * curve. */

    int reqMaxSymbols;			/* Indicates the interval the draw
					 * symbols.  Zero (and one) means draw
					 * all symbols. */
    int penDir;				/* Indicates if a change in the pen
					 * direction should be considered a
					 * retrace (line segment is not
					 * drawn). */
    Blt_Chain traces;			/* List of traces (a trace is a series
					 * of contiguous line segments).  New
					 * traces are generated when either
					 * the next segment changes the pen
					 * direction, or the end point is
					 * clipped by the plotting area. */
    Blt_Pool pointPool;
    Blt_Pool segmentPool;
    GraphColormap *colormapPtr;
};

static Blt_OptionParseProc ObjToSmoothProc;
static Blt_OptionPrintProc SmoothToObjProc;
static Blt_CustomOption smoothOption =
{
    ObjToSmoothProc, SmoothToObjProc, NULL, (ClientData)0
};

static Blt_OptionParseProc ObjToPenDirProc;
static Blt_OptionPrintProc PenDirToObjProc;
static Blt_CustomOption penDirOption =
{
    ObjToPenDirProc, PenDirToObjProc, NULL, (ClientData)0
};

static Blt_OptionParseProc ObjToErrorBarsProc;
static Blt_OptionPrintProc ErrorBarsToObjProc;
static Blt_CustomOption errorbarsOption =
{
    ObjToErrorBarsProc, ErrorBarsToObjProc, NULL, (ClientData)0
};

static Blt_OptionFreeProc FreeSymbolProc;
static Blt_OptionParseProc ObjToSymbolProc;
static Blt_OptionPrintProc SymbolToObjProc;
static Blt_CustomOption symbolOption =
{
    ObjToSymbolProc, SymbolToObjProc, FreeSymbolProc, (ClientData)0
};

static Blt_OptionFreeProc FreeColormapProc;
static Blt_OptionParseProc ObjToColormapProc;
static Blt_OptionPrintProc ColormapToObjProc;
static Blt_CustomOption colormapOption =
{
    ObjToColormapProc, ColormapToObjProc, FreeColormapProc, (ClientData)0
};

BLT_EXTERN Blt_CustomOption bltLineStylesOption;
BLT_EXTERN Blt_CustomOption bltColorOption;
BLT_EXTERN Blt_CustomOption bltValuesOption;
BLT_EXTERN Blt_CustomOption bltValuePairsOption;
BLT_EXTERN Blt_CustomOption bltLinePenOption;
BLT_EXTERN Blt_CustomOption bltXAxisOption;
BLT_EXTERN Blt_CustomOption bltYAxisOption;

#define DEF_ACTIVE_PEN		"activeLine"
#define DEF_AXIS_X		"x"
#define DEF_AXIS_Y		"y"
#define DEF_DATA		(char *)NULL
#define DEF_FILL_COLOR    	"defcolor"
#define DEF_HIDE		"no"
#define DEF_LABEL		(char *)NULL
#define DEF_LABEL_RELIEF	"flat"
#define DEF_MAX_SYMBOLS		"0"
#define DEF_PATTERN_BG		(char *)NULL
#define DEF_PATTERN_FG		"black"
#define DEF_COLORMAP		(char *)NULL
#define DEF_REDUCE		"0.0"
#define DEF_SCALE_SYMBOLS	"yes"
#define DEF_SMOOTH		"linear"
#define DEF_STATE		"normal"
#define DEF_STIPPLE		(char *)NULL
#define DEF_STYLES		""
#define DEF_SYMBOL		"circle"
#define DEF_TAGS		"all"
#define DEF_X_DATA		(char *)NULL
#define DEF_Y_DATA		(char *)NULL
#define DEF_PEN_ACTIVE_COLOR	RGB_BLUE
#define DEF_PEN_COLOR		RGB_NAVYBLUE
#define DEF_PEN_DASHES		(char *)NULL
#define DEF_PEN_DASHES		(char *)NULL
#define DEF_PEN_DIRECTION	"both"

#define DEF_PEN_ERRORBARS		"both"
#define DEF_PEN_ERRORBAR_CAPWIDTH	"0"
#define DEF_PEN_ERRORBAR_COLOR		"defcolor"
#define DEF_PEN_ERRORBAR_LINEWIDTH	"1"

#define DEF_PEN_FILL_COLOR    		"defcolor"
#define DEF_PEN_LINEWIDTH		"1"
#define DEF_PEN_NORMAL_COLOR		RGB_NAVYBLUE
#define DEF_PEN_OFFDASH_COLOR    	(char *)NULL
#define DEF_PEN_OUTLINE_COLOR		"defcolor"
#define DEF_PEN_OUTLINE_WIDTH 		"1"
#define DEF_PEN_PIXELS			"0.1i"
#define DEF_PEN_SHOW_VALUES		"no"
#define DEF_PEN_SYMBOL			"circle"
#define DEF_PEN_TYPE			"line"
#define DEF_PEN_VALUE_ANCHOR		"s"
#define DEF_PEN_VALUE_ANGLE		(char *)NULL
#define DEF_PEN_VALUE_COLOR		RGB_BLACK
#define DEF_PEN_VALUE_FONT		STD_FONT_NUMBERS
#define DEF_PEN_VALUE_FORMAT		"%g"

static Blt_ConfigSpec lineSpecs[] =
{
    {BLT_CONFIG_CUSTOM, "-activepen", "activePen", "ActivePen",
	DEF_ACTIVE_PEN, Blt_Offset(LineElement, activePenPtr),
	BLT_CONFIG_NULL_OK, &bltLinePenOption},
    {BLT_CONFIG_COLOR, "-areaforeground", "areaForeground", "AreaForeground",
	DEF_PATTERN_FG, Blt_Offset(LineElement, fillFgColor), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BACKGROUND, "-areabackground", "areaBackground", 
	"AreaBackground", DEF_PATTERN_BG, Blt_Offset(LineElement, fillBg),
	 BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_LIST, "-bindtags", "bindTags", "BindTags", DEF_TAGS, 
	Blt_Offset(LineElement, obj.tags), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-color", "color", "Color", DEF_PEN_COLOR, 
	Blt_Offset(LineElement, builtinPen.traceColor), 0},
    {BLT_CONFIG_DASHES, "-dashes", "dashes", "Dashes", DEF_PEN_DASHES, 
	Blt_Offset(LineElement, builtinPen.traceDashes), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_CUSTOM, "-data", "data", "Data", DEF_DATA, 0, 0, 
	&bltValuePairsOption},
    {BLT_CONFIG_CUSTOM, "-errorbars", "errorBars", "ErrorBars",
	DEF_PEN_ERRORBARS, Blt_Offset(LineElement, builtinPen.errorFlags), 
        BLT_CONFIG_DONT_SET_DEFAULT, &errorbarsOption},
    {BLT_CONFIG_CUSTOM, "-errorbarcolor", "errorBarColor", "ErrorBarColor",
	DEF_PEN_ERRORBAR_COLOR, 
	Blt_Offset(LineElement, builtinPen.errorColor), 0, &bltColorOption},
    {BLT_CONFIG_PIXELS_NNEG,"-errorbarlinewidth", "errorBarLineWidth", 
	"ErrorBarLineWidth", DEF_PEN_ERRORBAR_LINEWIDTH, 
	Blt_Offset(LineElement, builtinPen.errorLineWidth),
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-errorbarcapwith", "errorBarCapWidth", 
	"ErrorBarCapWidth", DEF_PEN_ERRORBAR_CAPWIDTH, 
	Blt_Offset(LineElement, builtinPen.errorCapWidth),
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-fill", "fill", "Fill", DEF_PEN_FILL_COLOR, 
	Blt_Offset(LineElement, builtinPen.symbol.fillColor), 
	BLT_CONFIG_NULL_OK, &bltColorOption},
    {BLT_CONFIG_BITMASK, "-hide", "hide", "Hide", DEF_HIDE, 
        Blt_Offset(LineElement, flags), BLT_CONFIG_DONT_SET_DEFAULT,
        (Blt_CustomOption *)HIDE},
    {BLT_CONFIG_STRING, "-label", "label", "Label", (char *)NULL, 
	Blt_Offset(LineElement, label), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_RELIEF, "-legendrelief", "legendRelief", "LegendRelief",
	DEF_LABEL_RELIEF, Blt_Offset(LineElement, legendRelief),
	BLT_CONFIG_DONT_SET_DEFAULT}, 
    {BLT_CONFIG_PIXELS_NNEG, "-linewidth", "lineWidth", "LineWidth",
	DEF_PEN_LINEWIDTH, Blt_Offset(LineElement, builtinPen.traceWidth),
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-mapx", "mapX", "MapX",
        DEF_AXIS_X, Blt_Offset(LineElement, axes.x), 0, &bltXAxisOption},
    {BLT_CONFIG_CUSTOM, "-mapy", "mapY", "MapY",
	DEF_AXIS_Y, Blt_Offset(LineElement, axes.y), 0, &bltYAxisOption},
    {BLT_CONFIG_INT_NNEG, "-maxsymbols", "maxSymbols", "MaxSymbols",
	DEF_MAX_SYMBOLS, Blt_Offset(LineElement, reqMaxSymbols),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-offdash", "offDash", "OffDash", 
	DEF_PEN_OFFDASH_COLOR, 
	Blt_Offset(LineElement, builtinPen.traceOffColor),
	BLT_CONFIG_NULL_OK, &bltColorOption},
    {BLT_CONFIG_CUSTOM, "-outline", "outline", "Outline", 
	DEF_PEN_OUTLINE_COLOR, 
	Blt_Offset(LineElement, builtinPen.symbol.outlineColor), 
	0, &bltColorOption},
    {BLT_CONFIG_PIXELS_NNEG, "-outlinewidth", "outlineWidth", "OutlineWidth",
	DEF_PEN_OUTLINE_WIDTH, 
	Blt_Offset(LineElement, builtinPen.symbol.outlineWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-pen", "pen", "Pen", (char *)NULL, 
	Blt_Offset(LineElement, normalPenPtr), BLT_CONFIG_NULL_OK, 
	&bltLinePenOption},
    {BLT_CONFIG_CUSTOM, "-colormap", "colormap", "Colormap", DEF_COLORMAP, 
	Blt_Offset(LineElement, colormapPtr), 0, &colormapOption},
    {BLT_CONFIG_PIXELS_NNEG, "-pixels", "pixels", "Pixels", DEF_PEN_PIXELS, 
	Blt_Offset(LineElement, builtinPen.symbol.size), GRAPH | STRIPCHART}, 
    {BLT_CONFIG_FLOAT, "-reduce", "reduce", "Reduce",
	DEF_REDUCE, Blt_Offset(LineElement, rTolerance),
	GRAPH | STRIPCHART | BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_BOOLEAN, "-scalesymbols", "scaleSymbols", "ScaleSymbols",
	DEF_SCALE_SYMBOLS, Blt_Offset(LineElement, scaleSymbols),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_FILL, "-showvalues", "showValues", "ShowValues",
	DEF_PEN_SHOW_VALUES, Blt_Offset(LineElement, builtinPen.valueFlags),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-smooth", "smooth", "Smooth", DEF_SMOOTH, 
	Blt_Offset(LineElement, reqSmooth), BLT_CONFIG_DONT_SET_DEFAULT, 
	&smoothOption},
    {BLT_CONFIG_STATE, "-state", "state", "State", DEF_STATE, 
	Blt_Offset(LineElement, state), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-styles", "styles", "Styles", DEF_STYLES, 
	Blt_Offset(LineElement, styles), 0, &bltLineStylesOption},
    {BLT_CONFIG_CUSTOM, "-symbol", "symbol", "Symbol", DEF_PEN_SYMBOL, 
	Blt_Offset(LineElement, builtinPen.symbol), 
	BLT_CONFIG_DONT_SET_DEFAULT, &symbolOption},
    {BLT_CONFIG_CUSTOM, "-trace", "trace", "Trace", DEF_PEN_DIRECTION, 
	Blt_Offset(LineElement, penDir), 
	BLT_CONFIG_DONT_SET_DEFAULT, &penDirOption},
    {BLT_CONFIG_ANCHOR, "-valueanchor", "valueAnchor", "ValueAnchor",
	DEF_PEN_VALUE_ANCHOR, 
	Blt_Offset(LineElement, builtinPen.valueStyle.anchor), 0},
    {BLT_CONFIG_COLOR, "-valuecolor", "valueColor", "ValueColor",
	DEF_PEN_VALUE_COLOR, 
	Blt_Offset(LineElement, builtinPen.valueStyle.color), 0},
    {BLT_CONFIG_FONT, "-valuefont", "valueFont", "ValueFont",
	DEF_PEN_VALUE_FONT, 
	Blt_Offset(LineElement, builtinPen.valueStyle.font), 0},
    {BLT_CONFIG_STRING, "-valueformat", "valueFormat", "ValueFormat",
	DEF_PEN_VALUE_FORMAT, Blt_Offset(LineElement, builtinPen.valueFormat),
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_FLOAT, "-valuerotate", "valueRotate", "ValueRotate",
	DEF_PEN_VALUE_ANGLE, 
	Blt_Offset(LineElement, builtinPen.valueStyle.angle), 0},
    {BLT_CONFIG_CUSTOM, "-weights", "weights", "Weights", (char *)NULL, 
	Blt_Offset(LineElement, w), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-x", "xData", "XData", (char *)NULL, 
        Blt_Offset(LineElement, x), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-xdata", "xData", "XData", (char *)NULL, 
	Blt_Offset(LineElement, x), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-xerror", "xError", "XError", (char *)NULL, 
	Blt_Offset(LineElement, xError), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-xhigh", "xHigh", "XHigh", (char *)NULL, 
	Blt_Offset(LineElement, xHigh), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-xlow", "xLow", "XLow", (char *)NULL, 
	Blt_Offset(LineElement, xLow), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-y", "yData", "YData", (char *)NULL, 
	Blt_Offset(LineElement, y), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-ydata", "yData", "YData", (char *)NULL, 
	Blt_Offset(LineElement, y), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-yerror", "yError", "YError", (char *)NULL, 
	Blt_Offset(LineElement, yError), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-yhigh", "yHigh", "YHigh", (char *)NULL, 
	Blt_Offset(LineElement, yHigh), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-ylow", "yLow", "YLow", (char *)NULL, 
	Blt_Offset(LineElement, yLow), 0, &bltValuesOption},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

static Blt_ConfigSpec stripSpecs[] =
{
    {BLT_CONFIG_CUSTOM, "-activepen", "activePen", "ActivePen",
	DEF_ACTIVE_PEN, Blt_Offset(LineElement, activePenPtr), 
	BLT_CONFIG_NULL_OK, &bltLinePenOption},
    {BLT_CONFIG_COLOR, "-areaforeground", "areaForeground", "areaForeground",
	DEF_PATTERN_FG, Blt_Offset(LineElement, fillFgColor), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_BACKGROUND, "-areabackground", "areaBackground", 
	"areaBackground", DEF_PATTERN_BG, Blt_Offset(LineElement, fillBg), 
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_LIST, "-bindtags", "bindTags", "BindTags", DEF_TAGS, 
	Blt_Offset(LineElement, obj.tags), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_COLOR, "-color", "color", "Color",
	DEF_PEN_COLOR, Blt_Offset(LineElement, builtinPen.traceColor), 0},
    {BLT_CONFIG_DASHES, "-dashes", "dashes", "Dashes", DEF_PEN_DASHES, 
	Blt_Offset(LineElement, builtinPen.traceDashes), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_CUSTOM, "-data", "data", "Data", DEF_DATA, 0, 0, 
	&bltValuePairsOption},
    {BLT_CONFIG_CUSTOM, "-errorbars", "errorBars", "ErrorBars",
	DEF_PEN_ERRORBARS, Blt_Offset(LineElement, builtinPen.errorFlags), 
        BLT_CONFIG_DONT_SET_DEFAULT, &errorbarsOption},
    {BLT_CONFIG_CUSTOM, "-errorbarcolor", "errorBarColor", "ErrorBarColor",
	DEF_PEN_ERRORBAR_COLOR, 
	Blt_Offset(LineElement, builtinPen.errorColor), 0, &bltColorOption},
    {BLT_CONFIG_PIXELS_NNEG, "-errorbarlinewidth", "errorBarLineWidth", 
	"ErrorBarLineWidth", DEF_PEN_ERRORBAR_LINEWIDTH, 
	Blt_Offset(LineElement, builtinPen.errorLineWidth),
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-errorbarcapwidth", "errorBarCapWidth", 
	"ErrorBarCapWidth", DEF_PEN_ERRORBAR_CAPWIDTH, 
	Blt_Offset(LineElement, builtinPen.errorCapWidth), 
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-fill", "fill", "Fill", DEF_PEN_FILL_COLOR, 
	Blt_Offset(LineElement, builtinPen.symbol.fillColor),
	BLT_CONFIG_NULL_OK, &bltColorOption},
    {BLT_CONFIG_BITMASK, "-hide", "hide", "Hide", DEF_HIDE, 
	Blt_Offset(LineElement, flags), BLT_CONFIG_DONT_SET_DEFAULT,
        (Blt_CustomOption *)HIDE},
    {BLT_CONFIG_STRING, "-label", "label", "Label", (char *)NULL, 
	Blt_Offset(LineElement, label), BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_RELIEF, "-legendrelief", "legendRelief", "LegendRelief",
	DEF_LABEL_RELIEF, Blt_Offset(LineElement, legendRelief),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-linewidth", "lineWidth", "LineWidth", 
	DEF_PEN_LINEWIDTH, Blt_Offset(LineElement, builtinPen.traceWidth), 
        BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-mapx", "mapX", "MapX", DEF_AXIS_X, 
	Blt_Offset(LineElement, axes.x), 0, &bltXAxisOption},
    {BLT_CONFIG_CUSTOM, "-mapy", "mapY", "MapY", DEF_AXIS_Y, 
	Blt_Offset(LineElement, axes.y), 0, &bltYAxisOption},
    {BLT_CONFIG_INT_NNEG, "-maxsymbols", "maxSymbols", "MaxSymbols",
	DEF_MAX_SYMBOLS, Blt_Offset(LineElement, reqMaxSymbols),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-offdash", "offDash", "OffDash", 
	DEF_PEN_OFFDASH_COLOR, 
	Blt_Offset(LineElement, builtinPen.traceOffColor),
	BLT_CONFIG_NULL_OK, &bltColorOption},
    {BLT_CONFIG_CUSTOM, "-outline", "outline", "Outline",
	DEF_PEN_OUTLINE_COLOR, 
	Blt_Offset(LineElement, builtinPen.symbol.outlineColor), 0, 
	&bltColorOption},
    {BLT_CONFIG_PIXELS_NNEG, "-outlinewidth", "outlineWidth", "OutlineWidth",
	DEF_PEN_OUTLINE_WIDTH, 
	Blt_Offset(LineElement, builtinPen.symbol.outlineWidth),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-pen", "pen", "Pen", (char *)NULL, 
	Blt_Offset(LineElement, normalPenPtr), BLT_CONFIG_NULL_OK, 
	&bltLinePenOption},
    {BLT_CONFIG_PIXELS_NNEG, "-pixels", "pixels", "Pixels", DEF_PEN_PIXELS, 
	Blt_Offset(LineElement, builtinPen.symbol.size), 0},
    {BLT_CONFIG_BOOLEAN, "-scalesymbols", "scaleSymbols", "ScaleSymbols",
	DEF_SCALE_SYMBOLS, Blt_Offset(LineElement, scaleSymbols),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_FILL, "-showvalues", "showValues", "ShowValues",
	DEF_PEN_SHOW_VALUES, Blt_Offset(LineElement, builtinPen.valueFlags),
	BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-smooth", "smooth", "Smooth", DEF_SMOOTH, 
        Blt_Offset(LineElement, reqSmooth), BLT_CONFIG_DONT_SET_DEFAULT, 
	&smoothOption},
    {BLT_CONFIG_CUSTOM, "-styles", "styles", "Styles", DEF_STYLES, 
	Blt_Offset(LineElement, styles), 0, &bltLineStylesOption},
    {BLT_CONFIG_CUSTOM, "-symbol", "symbol", "Symbol", DEF_PEN_SYMBOL, 
	Blt_Offset(LineElement, builtinPen.symbol), 
	BLT_CONFIG_DONT_SET_DEFAULT, &symbolOption},
    {BLT_CONFIG_ANCHOR, "-valueanchor", "valueAnchor", "ValueAnchor",
	DEF_PEN_VALUE_ANCHOR, 
        Blt_Offset(LineElement, builtinPen.valueStyle.anchor), 0},
    {BLT_CONFIG_COLOR, "-valuecolor", "valueColor", "ValueColor",
	DEF_PEN_VALUE_COLOR, 
	Blt_Offset(LineElement, builtinPen.valueStyle.color), 0},
    {BLT_CONFIG_FONT, "-valuefont", "valueFont", "ValueFont",
	DEF_PEN_VALUE_FONT, 
	Blt_Offset(LineElement, builtinPen.valueStyle.font), 0},
    {BLT_CONFIG_STRING, "-valueformat", "valueFormat", "ValueFormat",
	DEF_PEN_VALUE_FORMAT, Blt_Offset(LineElement, builtinPen.valueFormat),
	BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_FLOAT, "-valuerotate", "valueRotate", "ValueRotate",
	DEF_PEN_VALUE_ANGLE, 
	Blt_Offset(LineElement, builtinPen.valueStyle.angle),0},
    {BLT_CONFIG_CUSTOM, "-weights", "weights", "Weights", (char *)NULL, 
	Blt_Offset(LineElement, w), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-x", "xData", "XData", (char *)NULL, 
	Blt_Offset(LineElement, x), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-xdata", "xData", "XData", (char *)NULL, 
	Blt_Offset(LineElement, x), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-y", "yData", "YData", (char *)NULL, 
	Blt_Offset(LineElement, y), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-xerror", "xError", "XError", (char *)NULL, 
	Blt_Offset(LineElement, xError), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-ydata", "yData", "YData", (char *)NULL, 
	Blt_Offset(LineElement, y), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-yerror", "yError", "YError", (char *)NULL, 
	Blt_Offset(LineElement, yError), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-xhigh", "xHigh", "XHigh", (char *)NULL, 
	Blt_Offset(LineElement, xHigh), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-xlow", "xLow", "XLow", (char *)NULL, 
	Blt_Offset(LineElement, xLow), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-yhigh", "yHigh", "YHigh", (char *)NULL, 
	Blt_Offset(LineElement, xHigh), 0, &bltValuesOption},
    {BLT_CONFIG_CUSTOM, "-ylow", "yLow", "YLow", (char *)NULL, 
	Blt_Offset(LineElement, yLow), 0, &bltValuesOption},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

static Blt_ConfigSpec penSpecs[] =
{
    {BLT_CONFIG_COLOR, "-color", "color", "Color", DEF_PEN_ACTIVE_COLOR, 
	Blt_Offset(LinePen, traceColor), ACTIVE_PEN},
    {BLT_CONFIG_COLOR, "-color", "color", "Color", DEF_PEN_NORMAL_COLOR, 
	Blt_Offset(LinePen, traceColor), NORMAL_PEN},
    {BLT_CONFIG_DASHES, "-dashes", "dashes", "Dashes", DEF_PEN_DASHES, 
	Blt_Offset(LinePen, traceDashes), BLT_CONFIG_NULL_OK | ALL_PENS},
    {BLT_CONFIG_CUSTOM, "-errorbars", "errorBars", "ErrorBars",
	DEF_PEN_ERRORBARS, Blt_Offset(LinePen, errorFlags),
        BLT_CONFIG_DONT_SET_DEFAULT, &errorbarsOption},
    {BLT_CONFIG_CUSTOM, "-errorbarcolor", "errorBarColor", "ErrorBarColor",
	DEF_PEN_ERRORBAR_COLOR, Blt_Offset(LinePen, errorColor), 
	ALL_PENS, &bltColorOption},
    {BLT_CONFIG_PIXELS_NNEG, "-errorbarlinewidth", "errorBarLineWidth", 
        "ErrorBarLineWidth", DEF_PEN_ERRORBAR_LINEWIDTH, 
	Blt_Offset(LinePen, errorLineWidth), 
        ALL_PENS | BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_PIXELS_NNEG, "-errorbarcapwidth", "errorBarCapWidth", 
	"ErrorBarCapWidth", DEF_PEN_ERRORBAR_CAPWIDTH, 
	Blt_Offset(LinePen, errorCapWidth), BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-fill", "fill", "Fill", DEF_PEN_FILL_COLOR, 
	Blt_Offset(LinePen, symbol.fillColor), BLT_CONFIG_NULL_OK | ALL_PENS, 
	&bltColorOption},
    {BLT_CONFIG_PIXELS_NNEG, "-linewidth", "lineWidth", "LineWidth", 
	DEF_PEN_LINEWIDTH, Blt_Offset(LinePen, traceWidth), 
	ALL_PENS | BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-offdash", "offDash", "OffDash", DEF_PEN_OFFDASH_COLOR,
	Blt_Offset(LinePen, traceOffColor), BLT_CONFIG_NULL_OK | ALL_PENS, 
	&bltColorOption},
    {BLT_CONFIG_CUSTOM, "-outline", "outline", "Outline", DEF_PEN_OUTLINE_COLOR,
	Blt_Offset(LinePen, symbol.outlineColor), ALL_PENS, &bltColorOption},
    {BLT_CONFIG_PIXELS_NNEG, "-outlinewidth", "outlineWidth", "OutlineWidth",
	DEF_PEN_OUTLINE_WIDTH, Blt_Offset(LinePen, symbol.outlineWidth),
	BLT_CONFIG_DONT_SET_DEFAULT | ALL_PENS},
    {BLT_CONFIG_PIXELS_NNEG, "-pixels", "pixels", "Pixels", DEF_PEN_PIXELS, 
	Blt_Offset(LinePen, symbol.size), ALL_PENS},
    {BLT_CONFIG_FILL, "-showvalues", "showValues", "ShowValues",
	DEF_PEN_SHOW_VALUES, Blt_Offset(LinePen, valueFlags),
	ALL_PENS | BLT_CONFIG_DONT_SET_DEFAULT},
    {BLT_CONFIG_CUSTOM, "-symbol", "symbol", "Symbol", DEF_PEN_SYMBOL, 
	Blt_Offset(LinePen, symbol), BLT_CONFIG_DONT_SET_DEFAULT | ALL_PENS, 
	&symbolOption},
    {BLT_CONFIG_STRING, "-type", (char *)NULL, (char *)NULL, DEF_PEN_TYPE, 
	Blt_Offset(Pen, typeId), ALL_PENS | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_ANCHOR, "-valueanchor", "valueAnchor", "ValueAnchor",
	DEF_PEN_VALUE_ANCHOR, Blt_Offset(LinePen, valueStyle.anchor), ALL_PENS},
    {BLT_CONFIG_COLOR, "-valuecolor", "valueColor", "ValueColor",
	DEF_PEN_VALUE_COLOR, Blt_Offset(LinePen, valueStyle.color), ALL_PENS},
    {BLT_CONFIG_FONT, "-valuefont", "valueFont", "ValueFont",
	DEF_PEN_VALUE_FONT, Blt_Offset(LinePen, valueStyle.font), ALL_PENS},
    {BLT_CONFIG_STRING, "-valueformat", "valueFormat", "ValueFormat",
	DEF_PEN_VALUE_FORMAT, Blt_Offset(LinePen, valueFormat),
	ALL_PENS | BLT_CONFIG_NULL_OK},
    {BLT_CONFIG_FLOAT, "-valuerotate", "valueRotate", "ValueRotate",
	DEF_PEN_VALUE_ANGLE, Blt_Offset(LinePen, valueStyle.angle), ALL_PENS},
    {BLT_CONFIG_END, NULL, NULL, NULL, NULL, 0, 0}
};

typedef double (DistanceProc)(int x, int y, Point2d *p, Point2d *q, Point2d *t);

/* Forward declarations */
static PenConfigureProc ConfigurePenProc;
static PenDestroyProc DestroyPenProc;
static ElementClosestProc ClosestProc;
static ElementConfigProc ConfigureProc;
static ElementDestroyProc DestroyProc;
static ElementDrawProc DrawActiveProc;
static ElementDrawProc DrawNormalProc;
static ElementDrawSymbolProc DrawSymbolProc;
static ElementFindProc FindProc;
static ElementExtentsProc ExtentsProc;
static ElementToPostScriptProc ActiveToPostScriptProc;
static ElementToPostScriptProc NormalToPostScriptProc;
static ElementSymbolToPostScriptProc SymbolToPostScriptProc;
static ElementMapProc MapProc;
static DistanceProc DistanceToYProc;
static DistanceProc DistanceToXProc;
static DistanceProc DistanceToLineProc;
static Blt_Bg_ChangedProc BackgroundChangedProc;

#ifdef WIN32

static int tkpWinRopModes[] =
{
    R2_BLACK,				/* GXclear */
    R2_MASKPEN,				/* GXand */
    R2_MASKPENNOT,			/* GXandReverse */
    R2_COPYPEN,				/* GXcopy */
    R2_MASKNOTPEN,			/* GXandInverted */
    R2_NOT,				/* GXnoop */
    R2_XORPEN,				/* GXxor */
    R2_MERGEPEN,			/* GXor */
    R2_NOTMERGEPEN,			/* GXnor */
    R2_NOTXORPEN,			/* GXequiv */
    R2_NOT,				/* GXinvert */
    R2_MERGEPENNOT,			/* GXorReverse */
    R2_NOTCOPYPEN,			/* GXcopyInverted */
    R2_MERGENOTPEN,			/* GXorInverted */
    R2_NOTMASKPEN,			/* GXnand */
    R2_WHITE				/* GXset */
};

#endif

#ifndef notdef
INLINE static int
Round(double x)
{
    return (int) (x + ((x < 0.0) ? -0.5 : 0.5));
}
#else 
#define Round Round
#endif

/*
 *---------------------------------------------------------------------------
 * 	Custom configuration option (parse and print) routines
 *---------------------------------------------------------------------------
 */
static void
DestroySymbol(Display *display, Symbol *symbolPtr)
{
    if (symbolPtr->image != NULL) {
	Tk_FreeImage(symbolPtr->image);
	symbolPtr->image = NULL;
    }
    if (symbolPtr->bitmap != None) {
	Tk_FreeBitmap(display, symbolPtr->bitmap);
	symbolPtr->bitmap = None;
    }
    if (symbolPtr->mask != None) {
	Tk_FreeBitmap(display, symbolPtr->mask);
	symbolPtr->mask = None;
    }
    symbolPtr->type = SYMBOL_NONE;
}
/*
 *---------------------------------------------------------------------------
 *
 * ImageChangedProc
 *
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
/* ARGSUSED */
static void
ImageChangedProc(
    ClientData clientData,
    int x, int y, int w, int h,		/* Not used. */
    int imageWidth, int imageHeight)	/* Not used. */
{
    Element *elemPtr;
    Graph *graphPtr;

    elemPtr = clientData;
    elemPtr->flags |= MAP_ITEM;
    graphPtr = elemPtr->obj.graphPtr;
    graphPtr->flags |= CACHE_DIRTY;
    Blt_EventuallyRedrawGraph(graphPtr);
}

/*ARGSUSED*/
static void
FreeSymbolProc(
    ClientData clientData,		/* Not used. */
    Display *display,			/* Not used. */
    char *widgRec,
    int offset)
{
    Symbol *symbolPtr = (Symbol *)(widgRec + offset);

    DestroySymbol(display, symbolPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToSymbolProc --
 *
 *	Convert the string representation of a line style or symbol name into
 *	its numeric form.
 *
 * Results:
 *	The return value is a standard TCL result.  The symbol type is written
 *	into the widget record.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToSymbolProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to report results */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* String representing symbol type */
    char *widgRec,			/* Element information record */
    int offset,				/* Offset to field in structure */
    int flags)				/* Not used. */
{
    Symbol *symbolPtr = (Symbol *)(widgRec + offset);
    const char *string;

    {
	int length;
	SymbolTable *entryPtr;
	char c;

	string = Tcl_GetStringFromObj(objPtr, &length);
	if (length == 0) {
	    DestroySymbol(Tk_Display(tkwin), symbolPtr);
	    symbolPtr->type = SYMBOL_NONE;
	    return TCL_OK;
	}
	c = string[0];
	for (entryPtr = symbolTable; entryPtr->name != NULL; entryPtr++) {
	    if (length < entryPtr->minChars) {
		continue;
	    }
	    if ((c == entryPtr->name[0]) && 
		(strncmp(string, entryPtr->name, length) == 0)) {
		DestroySymbol(Tk_Display(tkwin), symbolPtr);
		symbolPtr->type = entryPtr->type;
		return TCL_OK;
	    }
	}
    }
    {
	Tk_Image tkImage;
	Element *elemPtr = (Element *)widgRec;

	tkImage = Tk_GetImage(interp, tkwin, string, ImageChangedProc, elemPtr);
	if (tkImage != NULL) {
	    DestroySymbol(Tk_Display(tkwin), symbolPtr);
	    symbolPtr->image = tkImage;
	    symbolPtr->type = SYMBOL_IMAGE;
	    return TCL_OK;
	}
    }
    {
	Pixmap bitmap, mask;
	Tcl_Obj **objv;
	int objc;

	if ((Tcl_ListObjGetElements(NULL, objPtr, &objc, &objv) != TCL_OK) || 
	    (objc > 2)) {
	    goto error;
	}
	bitmap = mask = None;
	if (objc > 0) {
	    bitmap = Tk_AllocBitmapFromObj((Tcl_Interp *)NULL, tkwin, objv[0]);
	    if (bitmap == None) {
		goto error;
	    }
	}
	if (objc > 1) {
	    mask = Tk_AllocBitmapFromObj((Tcl_Interp *)NULL, tkwin, objv[1]);
	    if (mask == None) {
		goto error;
	    }
	}
	DestroySymbol(Tk_Display(tkwin), symbolPtr);
	symbolPtr->bitmap = bitmap;
	symbolPtr->mask = mask;
	symbolPtr->type = SYMBOL_BITMAP;
	return TCL_OK;
    }
 error:
    Tcl_AppendResult(interp, "bad symbol \"", string, 
	"\": should be \"none\", \"circle\", \"square\", \"diamond\", "
	"\"plus\", \"cross\", \"splus\", \"scross\", \"triangle\", "
	"\"arrow\" or the name of a bitmap", (char *)NULL);
    return TCL_ERROR;
}

/*
 *---------------------------------------------------------------------------
 *
 * SymbolToObjProc --
 *
 *	Convert the symbol value into a string.
 *
 * Results:
 *	The string representing the symbol type or line style is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
SymbolToObjProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,
    char *widgRec,			/* Element information record */
    int offset,				/* Offset to field in structure */
    int flags)				/* Not used. */
{
    Symbol *symbolPtr = (Symbol *)(widgRec + offset);

    if (symbolPtr->type == SYMBOL_BITMAP) {
	Tcl_Obj *listObjPtr, *objPtr;
	const char *name;

	listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
	name = Tk_NameOfBitmap(Tk_Display(tkwin), symbolPtr->bitmap);
	objPtr = Tcl_NewStringObj(name, -1);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	if (symbolPtr->mask == None) {
	    objPtr = Tcl_NewStringObj("", -1);
	} else {
	    name = Tk_NameOfBitmap(Tk_Display(tkwin), symbolPtr->mask);
	    objPtr = Tcl_NewStringObj(name, -1);
	}
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	return listObjPtr;
    } else {
	SymbolTable *entryPtr;

	for (entryPtr = symbolTable; entryPtr->name != NULL; entryPtr++) {
	    if (entryPtr->type == symbolPtr->type) {
		return Tcl_NewStringObj(entryPtr->name, -1);
	    }
	}
	return Tcl_NewStringObj("?unknown symbol type?", -1);
    }
}

/*
 * ObjToErrorBarsProc --
 *
 *	Convert the string representation of a errorbar flags into its numeric
 *	form.
 *
 * Results:
 *	The return value is a standard TCL result.  The errorbar flags is
 *	written into the widget record.
 */
/*ARGSUSED*/
static int
ObjToErrorBarsProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to return results. */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* String representing smooth type */
    char *widgRec,			/* Element information record */
    int offset,				/* Offset to field in structure */
    int flags)				/* Not used. */
{
    int *flagsPtr = (int *)(widgRec + offset);
    unsigned int mask;
    int i;
    int objc;
    Tcl_Obj **objv;

    if (Tcl_ListObjGetElements(NULL, objPtr, &objc, &objv) != TCL_OK) {
	return TCL_ERROR;
    }
    mask = 0;
    for (i = 0; i < objc; i++) {
	const char *string;

	string = Tcl_GetString(objv[i]);
	if (strcmp(string, "x") == 0) {
	    mask |= XERROR;
	} else if (strcmp(string, "y") == 0) {
	    mask |= YERROR;
	} else if (strcmp(string, "xhigh") == 0) {
	    mask |= XHIGH;
	} else if (strcmp(string, "yhigh") == 0) {
	    mask |= YHIGH;
	} else if (strcmp(string, "xlow") == 0) {
	    mask |= XLOW;
	} else if (strcmp(string, "ylow") == 0) {
	    mask |= YLOW;
	} else if (strcmp(string, "both") == 0) {
	    mask |= YERROR | XERROR;
	} else {
	    Tcl_AppendResult(interp, "bad errorbar value \"", string, 
		"\": should be x, y, xhigh, yhigh, xlow, ylow, or both", 
		(char *)NULL);
	    return TCL_ERROR;
	}
    }
    *flagsPtr = mask;
    return TCL_OK;
}

/*
 * ErrorBarsToObjProc --
 *
 *	Convert the error flags value into a list of strings.
 *
 * Results:
 *	The list representing the errorbar flags is returned.
 */
/*ARGSUSED*/
static Tcl_Obj *
ErrorBarsToObjProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Not used. */
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,			/* Element information record */
    int offset,				/* Offset to field in structure */
    int flags)				/* Not used. */
{
    int mask = *(int *)(widgRec + offset);
    Tcl_Obj *objPtr, *listObjPtr;

    listObjPtr = Tcl_NewListObj(0, (Tcl_Obj **)NULL);
    if ((mask & XERROR) && (mask & YERROR)) {
	objPtr = Tcl_NewStringObj("both", 4);
	Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
    } else {
	if (mask & XERROR) {
	    objPtr = Tcl_NewStringObj("x", 1);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	} else if (mask & XHIGH) {
	    objPtr = Tcl_NewStringObj("xhigh", 5);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	} else if (mask & XLOW) {
	    objPtr = Tcl_NewStringObj("xlow", 4);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	}
	if (mask & YERROR) {
	    objPtr = Tcl_NewStringObj("y", 1);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	} else if (mask & YHIGH) {
	    objPtr = Tcl_NewStringObj("yhigh", 5);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	} else if (mask & YLOW) {
	    objPtr = Tcl_NewStringObj("ylow", 4);
	    Tcl_ListObjAppendElement(interp, listObjPtr, objPtr);
	}
    }
    return listObjPtr;
}


/*
 *---------------------------------------------------------------------------
 *
 * NameOfSmooth --
 *
 *	Converts the smooth value into its string representation.
 *
 * Results:
 *	The static string representing the smooth type is returned.
 *
 *---------------------------------------------------------------------------
 */
static const char *
NameOfSmooth(unsigned int flags)
{
    SmoothingTable *entryPtr;

    for (entryPtr = smoothingTable; entryPtr->name != NULL; entryPtr++) {
	if (entryPtr->flags == flags) {
	    return entryPtr->name;
	}
    }
    return "unknown smooth value";
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToSmoothProc --
 *
 *	Convert the string representation of a line style or smooth name
 *	into its numeric form.
 *
 * Results:
 *	The return value is a standard TCL result.  The smooth type is
 *	written into the widget record.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToSmoothProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* String representing smooth type */
    char *widgRec,			/* Element information record */
    int offset,				/* Offset to field in structure */
    int flags)				/* Not used. */
{
    unsigned int *flagsPtr = (unsigned int *)(widgRec + offset);
    SmoothingTable *entryPtr;
    const char *string;
    char c;

    string = Tcl_GetString(objPtr);
    c = string[0];
    for (entryPtr = smoothingTable; entryPtr->name != NULL; entryPtr++) {
	if ((c == entryPtr->name[0]) && (strcmp(string, entryPtr->name) == 0)) {
	    *flagsPtr = entryPtr->flags;
	    return TCL_OK;
	}
    }
    Tcl_AppendResult(interp, "bad smooth value \"", string, "\": should be \
linear, step, natural, or quadratic", (char *)NULL);
    return TCL_ERROR;
}

/*
 *---------------------------------------------------------------------------
 *
 * SmoothToObjProc --
 *
 *	Convert the smooth value into a string.
 *
 * Results:
 *	The string representing the smooth type or line style is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
SmoothToObjProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Not used. */
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,			/* Element information record */
    int offset,				/* Offset to field in structure */
    int flags)				/* Not used. */
{
    int smooth = *(int *)(widgRec + offset);

    return Tcl_NewStringObj(NameOfSmooth(smooth), -1);
}


/*
 *---------------------------------------------------------------------------
 *
 * ObjToPenDirProc --
 *
 *	Convert the string representation of a line style or symbol name
 *	into its numeric form.
 *
 * Results:
 *	The return value is a standard TCL result.  The symbol type is
 *	written into the widget record.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToPenDirProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* String representing pen direction */
    char *widgRec,			/* Element information record */
    int offset,				/* Offset to field in structure */
    int flags)				/* Not used. */
{
    int *penDirPtr = (int *)(widgRec + offset);
    int length;
    char c;
    char *string;

    string = Tcl_GetStringFromObj(objPtr, &length);
    c = string[0];
    if ((c == 'i') && (strncmp(string, "increasing", length) == 0)) {
	*penDirPtr = PEN_INCREASING;
    } else if ((c == 'd') && (strncmp(string, "decreasing", length) == 0)) {
	*penDirPtr = PEN_DECREASING;
    } else if ((c == 'b') && (strncmp(string, "both", length) == 0)) {
	*penDirPtr = PEN_BOTH_DIRECTIONS;
    } else {
	Tcl_AppendResult(interp, "bad trace value \"", string,
	    "\" : should be \"increasing\", \"decreasing\", or \"both\"",
	    (char *)NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * NameOfPenDir --
 *
 *	Convert the pen direction into a string.
 *
 * Results:
 *	The static string representing the pen direction is returned.
 *
 *---------------------------------------------------------------------------
 */
static const char *
NameOfPenDir(int penDir)
{
    switch (penDir) {
    case PEN_INCREASING:
	return "increasing";
    case PEN_DECREASING:
	return "decreasing";
    case PEN_BOTH_DIRECTIONS:
	return "both";
    default:
	return "unknown trace direction";
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * PenDirToObj --
 *
 *	Convert the pen direction into a string.
 *
 * Results:
 *	The string representing the pen drawing direction is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
PenDirToObjProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Not used. */
    Tk_Window tkwin,			/* Not used. */
    char *widgRec,			/* Element information record */
    int offset,				/* Offset to field in structure */
    int flags)				/* Not used. */
{
    int penDir = *(int *)(widgRec + offset);

    return Tcl_NewStringObj(NameOfPenDir(penDir), -1);
}

/*
 *---------------------------------------------------------------------------
 *
 * ColormapChangedProc
 *
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
/* ARGSUSED */
static void
ColormapChangedProc(GraphColormap *cmapPtr, ClientData clientData, 
		    unsigned int flags)
{
    LineElement *elemPtr = clientData;
    Graph *graphPtr;

    if (flags & COLORMAP_DELETE_NOTIFY) {
	cmapPtr->palette = NULL;
    }
    elemPtr->flags |= MAP_ITEM;
    graphPtr = cmapPtr->graphPtr;
    graphPtr->flags |= CACHE_DIRTY;
    Blt_EventuallyRedrawGraph(graphPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * FreeColormapProc --
 *
 *	Releases the colormap.  The notifier for the colormap component is
 *	deleted.
 *
 * Results:
 *	The return value is a standard TCL result.  The colormap token is
 *	written into the widget record.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
FreeColormapProc(ClientData clientData, Display *display, char *widgRec,
		 int offset)
{
    GraphColormap **cmapPtrPtr = (GraphColormap **)(widgRec + offset);
    LineElement *elemPtr = (LineElement *)widgRec;
    
    Blt_Colormap_DeleteNotifier(*cmapPtrPtr, elemPtr);
    *cmapPtrPtr = NULL;
}

/*
 *---------------------------------------------------------------------------
 *
 * ObjToColormapProc --
 *
 *	Convert the string representation of a colormap into its token.
 *
 * Results:
 *	The return value is a standard TCL result.  The colormap token is
 *	written into the widget record.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ObjToColormapProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,			/* Interpreter to send results back
					 * to */
    Tk_Window tkwin,			/* Not used. */
    Tcl_Obj *objPtr,			/* String representing symbol type */
    char *widgRec,			/* Element information record */
    int offset,				/* Offset to field in structure */
    int flags)				/* Not used. */
{
    GraphColormap **cmapPtrPtr = (GraphColormap **)(widgRec + offset);
    LineElement *elemPtr = (LineElement *)widgRec;
    const char *string;
    
    string = Tcl_GetString(objPtr);
    if ((string == NULL) || (string[0] == '\0')) {
	FreeColormapProc(clientData, Tk_Display(tkwin), widgRec, offset);
	return TCL_OK;
    }
    if (Blt_Colormap_Get(interp, elemPtr->obj.graphPtr, objPtr, cmapPtrPtr) 
	!= TCL_OK) {
	return TCL_ERROR;
    }
    if (*cmapPtrPtr != NULL) {
	Blt_Colormap_CreateNotifier(*cmapPtrPtr, ColormapChangedProc, elemPtr);
    }
    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * ColormapToObjProc --
 *
 *	Convert the colormap token into a string.
 *
 * Results:
 *	The string representing the colormap is returned.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static Tcl_Obj *
ColormapToObjProc(
    ClientData clientData,		/* Not used. */
    Tcl_Interp *interp,
    Tk_Window tkwin,
    char *widgRec,			/* Element information record */
    int offset,				/* Offset to field in structure */
    int flags)				/* Not used. */
{
    GraphColormap *cmapPtr = *(GraphColormap **)(widgRec + offset);

    if (cmapPtr == NULL) {
	return Tcl_NewStringObj("", -1);
    } 
    return Tcl_NewStringObj(cmapPtr->name, -1);
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigurePenProc --
 *
 *	Sets up the appropriate configuration parameters in the GC.  It is
 *	assumed the parameters have been previously set by a call to
 *	Blt_ConfigureWidget.
 *
 * Results:
 *	The return value is a standard TCL result.  If TCL_ERROR is returned,
 *	then interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information such as line width, line style, color
 *	etc. get set in a new GC.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ConfigurePenProc(Graph *graphPtr, Pen *basePtr)
{
    LinePen *penPtr = (LinePen *)basePtr;
    unsigned long gcMask;
    GC newGC;
    XGCValues gcValues;
    XColor *colorPtr;

    /*
     * Set the outline GC for this pen: GCForeground is outline color.
     * GCBackground is the fill color (only used for bitmap symbols).
     */
    gcMask = (GCLineWidth | GCForeground);
    colorPtr = penPtr->symbol.outlineColor;
    if (colorPtr == COLOR_DEFAULT) {
	colorPtr = penPtr->traceColor;
    }
    gcValues.foreground = colorPtr->pixel;
    if (penPtr->symbol.type == SYMBOL_BITMAP) {
	colorPtr = penPtr->symbol.fillColor;
	if (colorPtr == COLOR_DEFAULT) {
	    colorPtr = penPtr->traceColor;
	}
	/*
	 * Set a clip mask if either
	 *	1) no background color was designated or
	 *	2) a masking bitmap was specified.
	 *
	 * These aren't necessarily the bitmaps we'll be using for clipping. But
	 * this makes it unlikely that anyone else will be sharing this GC when
	 * we set the clip origin (at the time the bitmap is drawn).
	 */
	if (colorPtr != NULL) {
	    gcValues.background = colorPtr->pixel;
	    gcMask |= GCBackground;
	    if (penPtr->symbol.mask != None) {
		gcValues.clip_mask = penPtr->symbol.mask;
		gcMask |= GCClipMask;
	    }
	} else {
	    gcValues.clip_mask = penPtr->symbol.bitmap;
	    gcMask |= GCClipMask;
	}
    }
    gcValues.line_width = LineWidth(penPtr->symbol.outlineWidth);
    newGC = Tk_GetGC(graphPtr->tkwin, gcMask, &gcValues);
    if (penPtr->symbol.outlineGC != NULL) {
	Tk_FreeGC(graphPtr->display, penPtr->symbol.outlineGC);
    }
    penPtr->symbol.outlineGC = newGC;

    /* Fill GC for symbols: GCForeground is fill color */

    gcMask = (GCLineWidth | GCForeground);
    colorPtr = penPtr->symbol.fillColor;
    if (colorPtr == COLOR_DEFAULT) {
	colorPtr = penPtr->traceColor;
    }
    newGC = NULL;
    if (colorPtr != NULL) {
	gcValues.foreground = colorPtr->pixel;
	newGC = Tk_GetGC(graphPtr->tkwin, gcMask, &gcValues);
    }
    if (penPtr->symbol.fillGC != NULL) {
	Tk_FreeGC(graphPtr->display, penPtr->symbol.fillGC);
    }
    penPtr->symbol.fillGC = newGC;

    /* Line segments */

    gcMask = (GCLineWidth | GCForeground | GCLineStyle | GCCapStyle |
	GCJoinStyle);
    gcValues.cap_style = CapButt;
    gcValues.join_style = JoinRound;
    gcValues.line_style = LineSolid;
    gcValues.line_width = LineWidth(penPtr->traceWidth);

    colorPtr = penPtr->traceOffColor;
    if (colorPtr == COLOR_DEFAULT) {
	colorPtr = penPtr->traceColor;
    }
    if (colorPtr != NULL) {
	gcMask |= GCBackground;
	gcValues.background = colorPtr->pixel;
    }
    gcValues.foreground = penPtr->traceColor->pixel;
    if (LineIsDashed(penPtr->traceDashes)) {
	gcValues.line_width = penPtr->traceWidth;
	gcValues.line_style = (colorPtr == NULL) ? 
	    LineOnOffDash : LineDoubleDash;
    }
    newGC = Blt_GetPrivateGC(graphPtr->tkwin, gcMask, &gcValues);
    if (LineIsDashed(penPtr->traceDashes)) {
	penPtr->traceDashes.offset = penPtr->traceDashes.values[0] / 2;
	Blt_SetDashes(graphPtr->display, newGC, &penPtr->traceDashes);
    }
    if (penPtr->traceGC != NULL) {
	Blt_FreePrivateGC(graphPtr->display, penPtr->traceGC);
    }
    penPtr->traceGC = newGC;

    gcMask = (GCLineWidth | GCForeground);
    colorPtr = penPtr->errorColor;
    if (colorPtr == COLOR_DEFAULT) {
	colorPtr = penPtr->traceColor;
    }
    gcValues.line_width = LineWidth(penPtr->errorLineWidth);
    gcValues.foreground = colorPtr->pixel;
    newGC = Tk_GetGC(graphPtr->tkwin, gcMask, &gcValues);
    if (penPtr->errorGC != NULL) {
	Tk_FreeGC(graphPtr->display, penPtr->errorGC);
    }
    penPtr->errorGC = newGC;

    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * DestroyPenProc --
 *
 *	Release memory and resources allocated for the style.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Everything associated with the pen style is freed up.
 *
 *---------------------------------------------------------------------------
 */
static void
DestroyPenProc(Graph *graphPtr, Pen *basePtr)
{
    LinePen *penPtr = (LinePen *)basePtr;

    Blt_Ts_FreeStyle(graphPtr->display, &penPtr->valueStyle);
    if (penPtr->symbol.outlineGC != NULL) {
	Tk_FreeGC(graphPtr->display, penPtr->symbol.outlineGC);
    }
    if (penPtr->symbol.fillGC != NULL) {
	Tk_FreeGC(graphPtr->display, penPtr->symbol.fillGC);
    }
    if (penPtr->errorGC != NULL) {
	Tk_FreeGC(graphPtr->display, penPtr->errorGC);
    }
    if (penPtr->traceGC != NULL) {
	Blt_FreePrivateGC(graphPtr->display, penPtr->traceGC);
    }
    if (penPtr->symbol.bitmap != None) {
	Tk_FreeBitmap(graphPtr->display, penPtr->symbol.bitmap);
	penPtr->symbol.bitmap = None;
    }
    if (penPtr->symbol.mask != None) {
	Tk_FreeBitmap(graphPtr->display, penPtr->symbol.mask);
	penPtr->symbol.mask = None;
    }
}


static void
InitPen(LinePen *penPtr)
{
    Blt_Ts_InitStyle(penPtr->valueStyle);
    penPtr->errorLineWidth = 1;
    penPtr->errorFlags = XERROR | YERROR;
    penPtr->configProc = ConfigurePenProc;
    penPtr->configSpecs = penSpecs;
    penPtr->destroyProc = DestroyPenProc;
    penPtr->flags = NORMAL_PEN;
    penPtr->symbol.bitmap = penPtr->symbol.mask = None;
    penPtr->symbol.outlineColor = penPtr->symbol.fillColor = COLOR_DEFAULT;
    penPtr->symbol.outlineWidth = penPtr->traceWidth = 1;
    penPtr->symbol.type = SYMBOL_CIRCLE;
    penPtr->valueFlags = SHOW_NONE;
}

Pen *
Blt_CreateLinePen2(Graph *graphPtr, ClassId id, Blt_HashEntry *hPtr)
{
    LinePen *penPtr;

    penPtr = Blt_AssertCalloc(1, sizeof(LinePen));
    penPtr->name = Blt_GetHashKey(&graphPtr->penTable, hPtr);
    penPtr->classId = id;
    penPtr->graphPtr = graphPtr;
    penPtr->hashPtr = hPtr;
    InitPen(penPtr);
    if (strcmp(penPtr->name, "activeLine") == 0) {
	penPtr->flags = ACTIVE_PEN;
    }
    Blt_SetHashValue(hPtr, penPtr);
    return (Pen *)penPtr;
}

/*
 *---------------------------------------------------------------------------
 *
 *	In this section, the routines deal with building and filling the
 *	element's data structures with transformed screen coordinates.  They
 *	are triggered from TranformLine which is called whenever the data or
 *	coordinates axes have changed and new screen coordinates need to be
 *	calculated.
 *
 *---------------------------------------------------------------------------
 */

/*
 *---------------------------------------------------------------------------
 *
 * ScaleSymbol --
 *
 *	Returns the scaled size for the line element. Scaling depends upon when
 *	the base line ranges for the element were set and the current range of
 *	the graph.
 *
 * Results:
 *	The new size of the symbol, after considering how much the graph has
 *	been scaled, is returned.
 *
 *---------------------------------------------------------------------------
 */
static int
ScaleSymbol(LineElement *elemPtr, int normalSize)
{
    int maxSize;
    double scale;
    int newSize;

    scale = 1.0;
    if (elemPtr->scaleSymbols) {
	double xRange, yRange;

	xRange = (elemPtr->axes.x->max - elemPtr->axes.x->min);
	yRange = (elemPtr->axes.y->max - elemPtr->axes.y->min);
	if (elemPtr->flags & SCALE_SYMBOL) {
	    /* Save the ranges as a baseline for future scaling. */
	    elemPtr->xRange = xRange;
	    elemPtr->yRange = yRange;
	    elemPtr->flags &= ~SCALE_SYMBOL;
	} else {
	    double xScale, yScale;

	    /* Scale the symbol by the smallest change in the X or Y axes */
	    xScale = elemPtr->xRange / xRange;
	    yScale = elemPtr->yRange / yRange;
	    scale = MIN(xScale, yScale);
	}
    }
    newSize = Round(normalSize * scale);

    /*
     * Don't let the size of symbols go unbounded. Both X and Win32 drawing
     * routines assume coordinates to be a signed short int.
     */
    maxSize = (int)MIN(elemPtr->obj.graphPtr->hRange, 
		       elemPtr->obj.graphPtr->vRange);
    if (newSize > maxSize) {
	newSize = maxSize;
    }

    /* Make the symbol size odd so that its center is a single pixel. */
    newSize |= 0x01;
    return newSize;
}

/*
 *---------------------------------------------------------------------------
 *
 * RemoveHead --
 *
 *	Removes the point at the head of the trace.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The trace is shrunk.
 *
 *---------------------------------------------------------------------------
 */
static void
RemoveHead(LineElement *elemPtr, Trace *tracePtr)
{
    TracePoint *p;

    p = tracePtr->head;
    tracePtr->head = p->next;
    if (tracePtr->tail == p) {
	tracePtr->tail = tracePtr->head;
    }
    Blt_Pool_FreeItem(elemPtr->pointPool, p);
    tracePtr->numPoints--;
}

/*
 *---------------------------------------------------------------------------
 *
 * FreeTrace --
 *
 *	Frees the memory assoicated with a trace.
 *	Note:  The points and segments of the trace are freed enmass when 
 *	       destroying the memory poll assoicated with the element.
 *
 * Results:
 *	Returns a pointer to the new trace.
 *
 * Side Effects:
 *	The trace is prepended to the element's list of traces.
 *
 *---------------------------------------------------------------------------
 */
static void
FreeTrace(Blt_Chain traces, Trace *tracePtr)
{
    if (tracePtr->link != NULL) {
	Blt_Chain_DeleteLink(traces, tracePtr->link);
    }
    if (tracePtr->fillPts != NULL) {
	Blt_Free(tracePtr->fillPts);
    }
    Blt_Free(tracePtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * FixTraces --
 *
 *	Fixes the trace by recounting the number of points.
 *	
 * Results:
 *	None.
 *
 * Side Effects:
 *	Removes the trace if it is empty.
 *
 *---------------------------------------------------------------------------
 */
static void
FixTraces(Blt_Chain traces)
{
    Blt_ChainLink link, next;

    for (link = Blt_Chain_FirstLink(traces); link != NULL; link = next) {
	Trace *tracePtr;
	int count;
	TracePoint *p, *q;

	next = Blt_Chain_NextLink(link);
	tracePtr = Blt_Chain_GetValue(link);
	if ((tracePtr->flags & RECOUNT) == 0) {
	    continue;
	}
	/* Count the number of points in the trace. */
	count = 0;
	q = NULL;
	for (p = tracePtr->head; p != NULL; p = p->next) {
	    count++;
	    q = p;
	}
	if (count == 0) {
	    /* Empty trace, remove it. */
	    FreeTrace(traces, tracePtr);
	} else {
	    /* Reset the number of points and the tail pointer. */
	    tracePtr->numPoints = count;
	    tracePtr->flags &= ~RECOUNT;
	    tracePtr->tail = q;
	}
    }
}

#ifdef notdef
static void
DumpFlags(unsigned int flags)
{
    if (flags & VISIBLE) {
	fprintf(stderr, "visible ");
    }
    if (flags & KNOT) {
	fprintf(stderr, "knot ");
    }
    if (flags & SYMBOL) {
	fprintf(stderr, "symbol ");
    }
    if (flags & ACTIVE_POINT) {
	fprintf(stderr, "active ");
    }
    if (flags & XHIGH) {
	fprintf(stderr, "xhigh ");
    }
    if (flags & XLOW) {
	fprintf(stderr, "xlow ");
    }
    if (flags & XLOW) {
	fprintf(stderr, "xlow ");
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DumpPoints --
 *
 *	Creates a new trace and prepends to the list of traces for 
 *	this element.
 *
 * Results:
 *	Returns a pointer to the new trace.
 *
 * Side Effects:
 *	The trace is prepended to the element's list of traces.
 *
 *---------------------------------------------------------------------------
 */
static void
DumpPoints(Trace *tracePtr)
{
    TracePoint *p;
    int i;

    fprintf(stderr, " element \"%s\", trace %lx: # of points = %d\n",
	    tracePtr->elemPtr->obj.name, (unsigned long)tracePtr, 
	    tracePtr->numPoints);
    for (i = 0, p = tracePtr->head; p != NULL; p = p->next, i++) {
	fprintf(stderr, "   point %d: x=%g y=%g index=%d flags=",
		i, p->x, p->y, p->index);
	DumpFlags(p->flags);
	fprintf(stderr, "\n");
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DumpTraces --
 *
 *	Creates a new trace and prepends to the list of traces for 
 *	this element.
 *
 * Results:
 *	Returns a pointer to the new trace.
 *
 * Side Effects:
 *	The trace is prepended to the element's list of traces.
 *
 *---------------------------------------------------------------------------
 */
static void
DumpTraces(LineElement *elemPtr)
{
    fprintf(stderr, "element \"%s\": # of points = %ld, # of traces = %ld\n", 
	    elemPtr->obj.name, (long)NUMBEROFPOINTS(elemPtr), 
	    (long int)Blt_Chain_GetLength(elemPtr->traces));
}

/*
 *---------------------------------------------------------------------------
 *
 * DumpSegments --
 *
 *	Creates a new trace and prepends to the list of traces for 
 *	this element.
 *
 * Results:
 *	Returns a pointer to the new trace.
 *
 * Side Effects:
 *	The trace is prepended to the element's list of traces.
 *
 *---------------------------------------------------------------------------
 */
static void
DumpSegments(Trace *tracePtr)
{
    TraceSegment *s;
    int i;

    fprintf(stderr, " element \"%s\", tracePtr %lx: # of segments = %d\n",
	    tracePtr->elemPtr->obj.name, (unsigned long)tracePtr, 
	    tracePtr->numSegments);
    for (i = 0, s = tracePtr->segments; s != NULL; s = s->next, i++) {
	fprintf(stderr, "   segment %d: x1=%g y1=%g, x2=%g y2=%g index=%d flags=%x\n",
		i, s->x1, s->y1, s->x2, s->y2, s->index, s->flags);
    }
}
#endif

/*
 *---------------------------------------------------------------------------
 *
 * NewTrace --
 *
 *	Creates a new trace and prepends to the list of traces for 
 *	this element.
 *
 * Results:
 *	Returns a pointer to the new trace.
 *
 * Side Effects:
 *	The trace is prepended to the element's list of traces.
 *
 *---------------------------------------------------------------------------
 */
static INLINE Trace *
NewTrace(LineElement *elemPtr)
{
    Trace *tracePtr;

    tracePtr = Blt_AssertCalloc(1, sizeof(Trace));
    if (elemPtr->traces == NULL) {
	elemPtr->traces = Blt_Chain_Create();
    }
    tracePtr->link = Blt_Chain_Prepend(elemPtr->traces, tracePtr);
    tracePtr->elemPtr = elemPtr;
    tracePtr->penPtr = NORMALPEN(elemPtr);
    return tracePtr;
}

/*
 *---------------------------------------------------------------------------
 *
 * NewPoint --
 *
 *	Creates a new point 
 *
 * Results:
 *	Returns a pointer to the new trace.
 *
 * Side Effects:
 *	The trace is prepended to the element's list of traces.
 *
 *---------------------------------------------------------------------------
 */
static INLINE TracePoint *
NewPoint(LineElement *elemPtr, double x, double y, int index)
{
    TracePoint *p;
    Region2d exts;

    p = Blt_Pool_AllocItem(elemPtr->pointPool, sizeof(TracePoint));
    p->next = NULL;
    p->flags = 0;
    p->x = x;
    p->y = y;
    Blt_GraphExtents(elemPtr, &exts);
    if (PointInRegion(&exts, p->x, p->y)) {
	p->flags |= VISIBLE;
    }
    p->index = index;
    return p;
}

/*
 *---------------------------------------------------------------------------
 *
 * NewSegment --
 *
 *	Creates a new segment of the trace's errorbars.
 *
 * Results:
 *	Returns a pointer to the new trace.
 *
 * Side Effects:
 *	The trace is prepended to the element's list of traces.
 *
 *---------------------------------------------------------------------------
 */
static INLINE TraceSegment *
NewSegment(LineElement *elemPtr, float x1, float y1, float x2, float y2, 
	   int index, unsigned int flags)
{
    TraceSegment *s;

    s = Blt_Pool_AllocItem(elemPtr->segmentPool, sizeof(TraceSegment));
    s->x1 = x1;
    s->y1 = y1;
    s->x2 = x2;
    s->y2 = y2;
    s->index = index;
    s->flags = flags | VISIBLE;
    s->next = NULL;
    return s;
}

/*
 *---------------------------------------------------------------------------
 *
 * AddSegment --
 *
 *	Appends a line segment point to the given trace.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The trace's counter is incremented.
 *
 *---------------------------------------------------------------------------
 */
static INLINE void
AddSegment(Trace *tracePtr, TraceSegment *s)
{
    
    if (tracePtr->segments == NULL) {
	tracePtr->segments = s;
    } else {
	s->next = tracePtr->segments;
	tracePtr->segments = s;
    }
    tracePtr->numSegments++;
}

/*
 *---------------------------------------------------------------------------
 *
 * AppendPoint --
 *
 *	Appends the point to the given trace.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The trace's counter is incremented.
 *
 *---------------------------------------------------------------------------
 */
static INLINE void
AppendPoint(Trace *tracePtr, TracePoint *p)
{
    if (tracePtr->head == NULL) {
	tracePtr->tail = tracePtr->head = p;
    } else {
	assert(tracePtr->tail != NULL);
	tracePtr->tail->next = p;
	tracePtr->tail = p;
    }
    tracePtr->numPoints++;
}

static void
ResetElement(LineElement *elemPtr) 
{
    Blt_ChainLink link, next;

    if (elemPtr->pointPool != NULL) {
	Blt_Pool_Destroy(elemPtr->pointPool);
    }
    elemPtr->pointPool = Blt_Pool_Create(BLT_FIXED_SIZE_ITEMS);

    if (elemPtr->segmentPool != NULL) {
	Blt_Pool_Destroy(elemPtr->segmentPool);
    }
    elemPtr->segmentPool = Blt_Pool_Create(BLT_FIXED_SIZE_ITEMS);

    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL; 
	 link = next) {
	Trace *tracePtr;

	next = Blt_Chain_NextLink(link);
	tracePtr = Blt_Chain_GetValue(link);
	FreeTrace(elemPtr->traces, tracePtr);
    }
    if (elemPtr->traces != NULL) {
	Blt_Chain_Destroy(elemPtr->traces);
	elemPtr->traces = NULL;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * GetScreenPoints --
 *
 *	Generates a list of transformed screen coordinates from the data
 *	points.  Coordinates with Inf, -Inf, or NaN values are considered
 *	holes in the data and will create new traces.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory is allocated for the list of coordinates.
 *
 *---------------------------------------------------------------------------
 */
static void
GetScreenPoints(LineElement *elemPtr)
{
    Graph *graphPtr = elemPtr->obj.graphPtr;
    Trace *tracePtr;
    TracePoint *q;
    int i, n;
    double *x, *y;

    tracePtr = NULL;
    q = NULL;
    n = NUMBEROFPOINTS(elemPtr);
    x = elemPtr->x.values;
    y = elemPtr->y.values;
    for (i = 0; i < n; i++) {
	int broken;
	int j;
	TracePoint *p;
	Point2d r;

	j = i;
	while ((j < n) && ((!FINITE(x[j]))||(!FINITE(y[j])))) {
	    j++;	    /* Skip holes in the data. */
	}
	if (j == n) {
	    break;
	}
	r = Blt_Map2D(graphPtr, x[j], y[j], &elemPtr->axes);
	p = NewPoint(elemPtr, r.x, r.y, j);
	p->flags |= KNOT;
	broken = TRUE;
	if ((i == j) && (q != NULL)) {
	    broken = BROKEN_TRACE(elemPtr->penDir, p->x, q->x);
	} else {
	    i = j;
	}
	if (broken) {
	    if ((tracePtr == NULL) || (tracePtr->numPoints > 0)) {
		tracePtr = NewTrace(elemPtr);
	    }
	}
	AppendPoint(tracePtr, p);
	q = p;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * GenerateSteps --
 *
 *	Add points to the list of coordinates for step-and-hold type
 *	smoothing.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Points are added to the list of screen coordinates.
 *
 *---------------------------------------------------------------------------
 */
static void
GenerateSteps(Trace *tracePtr)
{
    Graph *graphPtr = tracePtr->elemPtr->obj.graphPtr;
    TracePoint *q, *p;

    for (p = tracePtr->head, q = p->next; q != NULL; q = q->next) {
	TracePoint *t;
	
	/* 
	 *         q
	 *         |
	 *  p ---- t
	 */
	if (graphPtr->inverted) {
	    t = NewPoint(tracePtr->elemPtr, p->x, q->y, p->index);
	} else {
	    t = NewPoint(tracePtr->elemPtr, q->x, p->y, p->index);
	}
 	/* Insert the new point between the two knots. */
	t->next = q;
	p->next = t;
	tracePtr->numPoints++;
	p = q;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * GenerateSpline --
 *
 *	Computes a cubic or quadratic spline and adds extra points to the 
 *	list of coordinates for smoothing.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Points are added to the list of screen coordinates.
 *
 *---------------------------------------------------------------------------
 */
static void
GenerateSpline(Trace *tracePtr)
{
    Blt_Spline spline;
    Graph *graphPtr = tracePtr->elemPtr->obj.graphPtr;
    LineElement *elemPtr = tracePtr->elemPtr;
    Point2d *points;
    TracePoint *p, *q;
    int i;

    /* FIXME: 1) handle inverted graph. 2) automatically flip to parametric
     * spline if non-monotonic. */
    for (p = tracePtr->head, q = p->next; q != NULL; q = q->next) {
	if (q->x <= p->x) {
	    return;			/* Points are not monotonically
					 * increasing */
	}
	p = q;
    }
    p = tracePtr->head;
    q = tracePtr->tail;
    if (((p->x > (double)graphPtr->right)) || 
	((q->x < (double)graphPtr->left))) {
	return;				/* All points are clipped. This only
					 * works if x is monotonically
					 * increasing. */
    }

    /*
     * The spline is computed in screen coordinates instead of data points so
     * that we can select the abscissas of the interpolated points from each
     * pixel horizontally across the plotting area.
     */
    if (graphPtr->right <= graphPtr->left) {
	return;
    }
    points = Blt_AssertMalloc(tracePtr->numPoints * sizeof(Point2d));

    /* Populate the interpolated point array with the original x-coordinates
     * and extra interpolated x-coordinates for each horizontal pixel that the
     * line segment contains. Do this only for pixels that are on screen  */
    for (i = 0, p = tracePtr->head; p != NULL; p = p->next, i++) {
	/* Add the original x-coordinate */
	points[i].x = p->x;
	points[i].y = p->y;
    }
    spline = Blt_CreateSpline(points, tracePtr->numPoints, elemPtr->smooth);
    if (spline == NULL) {
	return;				/* Can't interpolate. */
    }
    for (i = 0, p = tracePtr->head, q = p->next; q != NULL; q = q->next, i++) {
	/* Is any part of the interval (line segment) in the plotting area?  */
	if ((p->flags | q->flags) & VISIBLE) {
	    TracePoint *lastp;
	    double x, last;
	    
	    /*  Interpolate segments that lie on the screen. */
	    x = p->x + 1.0;
	    /*
	     * Since the line segment may be partially clipped on the left or
	     * right side, the points to interpolate are always interior to
	     * the plotting area.
	     *
	     *           left			    right
	     *      x1----|---------------------------|---x2
	     *
	     * Pick the max of the starting X-coordinate and the left edge and
	     * the min of the last X-coordinate and the right edge.
	     */
	    x = MAX(x, (double)graphPtr->left);
	    last = MIN(q->x, (double)graphPtr->right);

	    /* Add the extra x-coordinates to the interval. */
	    lastp = p;
	    while (x < last) {
		Point2d p1;
		TracePoint *t;

		p1 = Blt_EvaluateSpline(spline, i, x);
		t = NewPoint(elemPtr, p1.x, p1.y, p->index);
#ifdef notdef
	 fprintf(stderr, "new point x=%g new=%g,%g start(%d)=%g,%g index=%d\n", 
		    x, p1.x, p1.y, i, p->x, p->y, p->index);
#endif
		/* Insert the new point in to line segment. */
		t->next = lastp->next; 
		lastp->next = t;
		lastp = t;
		tracePtr->numPoints++;
		x++;
	    }
	    assert(lastp->next == q);
	}
	p = q;
    }
#ifdef notdef
    DumpPoints(tracePtr);
#endif
    Blt_Free(points);
    Blt_FreeSpline(spline);
}

/*
 *---------------------------------------------------------------------------
 *
 * GenerateParametricSplineOld --
 *
 *	Computes a spline based upon the data points, returning a new (larger)
 *	coordinate array or points.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The temporary arrays for screen coordinates and data map are updated
 *	based upon spline.
 *
 * FIXME:  Can't interpolate knots along the Y-axis.   Need to break
 *	   up point array into interchangable X and Y vectors earlier. *	   Pass extents (left/right or top/bottom) as parameters.
 *
 *---------------------------------------------------------------------------
 */
static void
GenerateParametricSplineOld(Trace *tracePtr)
{
    LineElement *elemPtr = tracePtr->elemPtr;
    Region2d exts;
    Point2d *xpoints, *ypoints;
    double *distance;
    int i, count;
    double total;
    TracePoint *p, *q;
    Blt_Spline xspline, yspline;
    int smooth;

    Blt_GraphExtents(elemPtr, &exts);
    xpoints = ypoints = NULL;
    xspline = yspline = NULL;
    distance = NULL;

    /* 
     * Populate the x2 array with both the original X-coordinates and extra
     * X-coordinates for each horizontal pixel that the line segment contains.
     */
    xpoints = Blt_Malloc(tracePtr->numPoints * sizeof(Point2d));
    ypoints = Blt_Malloc(tracePtr->numPoints * sizeof(Point2d));
    distance = Blt_Malloc(tracePtr->numPoints * sizeof(double));
    if ((xpoints == NULL) || (ypoints == NULL)) {
	goto error;
    }
    p = tracePtr->head;
    ypoints[0].x = xpoints[0].x = 0;
    xpoints[0].y = p->x;
    ypoints[0].y = p->y;
    distance[0] = 0;
    count = 1;
    total = 0.0;
#ifdef notdef
	fprintf(stderr, "orig #%d %g,%g d=%g px=%g,%g py=%g,%g\n", 
		0, p->x, p->y, total, xpoints[0].x, xpoints[0].y, 
		ypoints[0].x, ypoints[0].y);
#endif
    for (q = p->next; q != NULL; q = q->next) {
	double d;

	/* Distance of original point to p. */
	d = hypot(q->x - p->x, q->y - p->y);
	total += d;
        xpoints[count].x = ypoints[count].x = total;
        xpoints[count].y = q->x;
        ypoints[count].y = q->y;
	distance[count] = total;
#ifdef notdef
	fprintf(stderr, "orig #%d %g,%g d=%g px=%g,%g py=%g,%g\n", 
		count, q->x, q->y, total, xpoints[count].x, xpoints[count].y, 
		ypoints[count].x, ypoints[count].y);
#endif
	count++;
	p = q;
    }
    smooth = elemPtr->smooth & ~SMOOTH_PARAMETRIC;
    xspline = Blt_CreateSpline(xpoints, tracePtr->numPoints, smooth);
    yspline = Blt_CreateSpline(ypoints, tracePtr->numPoints, smooth);
    if ((xspline == NULL) || (yspline == NULL)) {
	goto error;
    }
	
    count = 0;
    for (i = 0, p = tracePtr->head, q = p->next; q != NULL; q = q->next, i++) {
	Point2d p1, p2;
	double dp, dq;
	TracePoint *lastp;

	if (((p->flags | q->flags) & VISIBLE) == 0) {
	    continue;			/* Line segment isn't visible. */
	}

        p1.x = p->x, p1.y = p->y;
	p2.x = q->x, p2.y = q->y;
	Blt_LineRectClip(&exts, &p1, &p2);
	
	/* Distance of original point to p. */
	dp = hypot(p1.x - p->x, p1.y - p->y);
	/* Distance of original point to q. */
	dq = hypot(p2.x - p->x, p2.y - p->y);

	dp += 2;
	dq -= 2;
	lastp = p;
	while(dp <= dq) {
	    Point2d px, py;
	    double d;
	    TracePoint *t;

	    d = dp + distance[i];
	    /* Point is indicated by its interval and parameter t. */
	    px = Blt_EvaluateSpline(xspline, i, d);
	    py = Blt_EvaluateSpline(yspline, i, d);
#ifdef notdef
	    fprintf(stderr, "time x=%g new=%g,%g start(%d)=%g,%g d=%g\n", x, 
		    px.y, py.y, i, p->x, p->y, distance[i]);
#endif
	    t = NewPoint(elemPtr, px.y, py.y, p->index);
	    /* Insert the new point in to line segment. */
	    t->next = lastp->next; 
	    lastp->next = t;
	    lastp = t;
	    tracePtr->numPoints++;
	    dp += 2;
	}
	p = q;
    }
 error:
    if (xpoints != NULL) {
	Blt_Free(xpoints);
    }
    if (xspline != NULL) {
	Blt_FreeSpline(xspline);
    }
    if (ypoints != NULL) {
	Blt_Free(ypoints);
    }
    if (yspline != NULL) {
	Blt_FreeSpline(yspline);
    }
    if (distance != NULL) {
	Blt_Free(distance);
    }
}

#ifdef notdef
/*
 *---------------------------------------------------------------------------
 *
 * GenerateParametricSpline --
 *
 *	Computes a spline based upon the data points, returning a new (larger)
 *	coordinate array or points.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The temporary arrays for screen coordinates and data map are updated
 *	based upon spline.
 *
 * FIXME:  Can't interpolate knots along the Y-axis.   Need to break
 *	   up point array into interchangable X and Y vectors earlier. *	   Pass extents (left/right or top/bottom) as parameters.
 *
 *---------------------------------------------------------------------------
 */
static void
GenerateParametricCubicSpline(Trace *tracePtr)
{
    LineElement *elemPtr = tracePtr->elemPtr;
    Region2d exts;
    Point2d *points, *iPts;
    int isize, niPts;
    int result;
    int i, count;
    TracePoint *p, *q;

    Blt_GraphExtents(elemPtr, &exts);

    /* 
     * Populate the x2 array with both the original X-coordinates and extra
     * X-coordinates for each horizontal pixel that the line segment contains.
     */
    points = Blt_AssertMalloc(tracePtr->numPoints * sizeof(Point2d));
    p = tracePtr->head;
    points[0].x = p->x;
    points[0].y = p->y;
    count = 1;
    for (i = 1, q = p->next; q != NULL; q = q->next, i++) {
	Point2d p1, p2;

        points[i].x = q->x;
        points[i].y = q->y;

        p1.x = p->x, p1.y = p->y;
        p2.x = q->x, p2.y = q->y;
	count++;
        if (Blt_LineRectClip(&exts, &p1, &p2)) {
	    count += (int)(hypot(p2.x - p1.x, p2.y - p1.y) * 0.5);
	}
	p = q;
    }
    isize = count;
    iPts = Blt_AssertMalloc(isize * 2 * sizeof(Point2d));

    count = 0;
    for (i = 0, p = tracePtr->head, q = p->next; q != NULL; q = q->next, i++) {
	Point2d p1, p2;

	if (((p->flags | q->flags) & VISIBLE) == 0) {
	    p = q;
	    continue;
	}
        p1.x = p->x, p1.y = p->y;
	p2.x = q->x, p2.y = q->y;

        /* Add the original x-coordinate */
        iPts[count].x = (double)i;
        iPts[count].y = 0.0;
        count++;

        if (Blt_LineRectClip(&exts, &p1, &p2)) {
	    double dp, dq;

	    /* Distance of original point to p. */
            dp = hypot(p1.x - p->x, p1.y - p->y);
	    /* Distance of original point to q. */
            dq = hypot(p2.x - p->x, p2.y - p->y);
            dp += 2.0;
	    while(dp <= dq) {
		/* Point is indicated by its interval and parameter t. */
		iPts[count].x = (double)i;
		iPts[count].y =  dp / dq;
		count++;
		dp += 2.0;
	    }
	    p = q;
	}
    }
    iPts[count].x = (double)i;
    iPts[count].y = 0.0;
    count++;
    niPts = count;
    result = FALSE;
    result = Blt_ComputeNaturalParametricSpline(points, tracePtr->numPoints, 
	&exts, FALSE, iPts, niPts);
    if (!result) {
        /* The spline interpolation failed.  We will fall back to the current
         * coordinates and do no smoothing (standard line segments).  */
        elemPtr->smooth = SMOOTH_NONE;
        Blt_Free(iPts);
	return;
    } 
    for (i = 0; i < tracePtr->numPoints; i++) {
	fprintf(stderr, "original[%d] = %g,%g\n", i, points[i].x, points[i].y);
    }
    for (i = 0; i < niPts; i++) {
	fprintf(stderr, "interpolated[%d] = %g,%g\n", i, iPts[i].x, iPts[i].y);
    }
    /* Now insert points into trace. */
    for (i = 0, p = tracePtr->head, q = p->next; q != NULL; q = q->next) {
	if ((i < niPts) && (p->x == iPts[i].x) && (p->y == iPts[i].y)) {
	    TracePoint *lastp;

	    fprintf(stderr, "found knot %g,%g in array[%d]\n", p->x, p->y, i);
	    /* Found a knot. Add points until the next knot. */
	    i++;
	    lastp = p;
	    while ((i < niPts) && (q->x != iPts[i].x) && 
		   (q->y != iPts[i].y)) {
		TracePoint *t;
		
	    fprintf(stderr, "comparing endpoint %g,%g in array[%d of %d]=%g,%g\n", 
		    q->x, q->y, i, niPts, iPts[i].x, iPts[i].y);
	    i++;

		t = NewPoint(elemPtr, iPts[i].x, iPts[i].y, p->index);
		t->next = lastp->next;
		lastp->next = t;
		lastp = t;
		tracePtr->numPoints++;
		i++;
	    } 
	} else {
	    fprintf(stderr, "comparing point %g,%g in array[%d of %d]=%g,%g\n", 
		    p->x, p->y, i, niPts, iPts[i].x, iPts[i].y);
	    i++;
	}
	p = q;
    }
}
#endif

/*
 *---------------------------------------------------------------------------
 *
 * GenerateCatromSpline --
 *
 *	Computes a catrom parametric spline.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Points are added to the list of screen coordinates.
 *
 *---------------------------------------------------------------------------
 */
static void
GenerateCatromSpline(Trace *tracePtr)
{
    Blt_Spline spline;
    Graph *graphPtr = tracePtr->elemPtr->obj.graphPtr;
    LineElement *elemPtr = tracePtr->elemPtr;
    Point2d *points;
    TracePoint *p, *q;
    Region2d exts;
    int i;

    /*
     * The spline is computed in screen coordinates instead of data points so
     * that we can select the abscissas of the interpolated points from each
     * pixel horizontally across the plotting area.
     */
    if (graphPtr->right <= graphPtr->left) {
	return;				/* No space in plotting area. */
    }
    points = Blt_AssertMalloc(tracePtr->numPoints * sizeof(Point2d));

    /* Populate the interpolated point array with the original x-coordinates
     * and extra interpolated x-coordinates for each horizontal pixel that the
     * line segment contains. Do this only for pixels that are on screen  */
    p = tracePtr->head;
    points[0].x = p->x;
    points[0].y = p->y;
    for (i = 1, p = p->next; p != NULL; p = p->next, i++) {
	/* Add the original x-coordinate */
	points[i].x = p->x;
	points[i].y = p->y;
    }
    spline = Blt_CreateCatromSpline(points, tracePtr->numPoints);
    if (spline == NULL) {
	return;				/* Can't interpolate. */
    }
    Blt_GraphExtents(elemPtr, &exts);
    for (i = 0, p = tracePtr->head, q = p->next; q != NULL; q = q->next, i++) {
	Point2d p1, p2;
	double d, dp, dq;
	TracePoint *lastp;

	if (((p->flags | q->flags) & VISIBLE) == 0) {
	    p = q;
	    continue;			/* Line segment isn't visible. */
	}

	/* Distance of the line entire segment. */
        d  = hypot(q->x - p->x, q->y - p->y);

        p1.x = p->x, p1.y = p->y;
	p2.x = q->x, p2.y = q->y;
	Blt_LineRectClip(&exts, &p1, &p2);
	
	/* Distance from last knot to p (start of generated points). */
	dp = hypot(p1.x - p->x, p1.y - p->y);
	/* Distance from last knot to q (end of generated points). */
	dq = hypot(p2.x - p->x, p2.y - p->y);

	dp += 2;
	dq -= 2;
	lastp = p;
	while (dp <= dq) {
	    Point2d p1;
	    TracePoint *t;

	    /* Point is indicated by its interval and parameter u which is the
	     * distance [0..1] of the point on the line segment. */
	    p1 = Blt_EvaluateCatromSpline(spline, i, dp / d);
	    t = NewPoint(elemPtr, p1.x, p1.y, p->index);
	    /* Insert the new point in to line segment. */
	    t->next = lastp->next; 
	    lastp->next = t;
	    lastp = t;
	    tracePtr->numPoints++;
	    dp += 2;
	    assert(t->next == q);
	}
	p = q;
    }
    Blt_Free(points);
    Blt_FreeCatromSpline(spline);
}

/*
 *---------------------------------------------------------------------------
 *
 * SmoothElement --
 *
 *	Computes a cubic or quadratic spline and adds extra points to the 
 *	list of coordinates for smoothing.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Points are added to the list of screen coordinates.
 *
 *---------------------------------------------------------------------------
 */
static void
SmoothElement(LineElement *elemPtr)
{
    Blt_ChainLink link;

    FixTraces(elemPtr->traces);
    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;

	tracePtr = Blt_Chain_GetValue(link);
#ifdef notdef
	fprintf(stderr, "SmoothElement(%s) n=%d tw=%d\n", elemPtr->obj.name,
		tracePtr->numPoints, tracePtr->penPtr->traceWidth);
#endif
	if ((tracePtr->numPoints < 2) || (tracePtr->penPtr->traceWidth == 0)) {
	    continue;
	}
	switch (elemPtr->smooth) {
	case SMOOTH_STEP:
	    GenerateSteps(tracePtr);
	    break;

	case SMOOTH_QUADRATIC:
	case SMOOTH_NATURAL:
	    if (tracePtr->numPoints > 3) {
		GenerateSpline(tracePtr);
	    }
	    break;

	case SMOOTH_QUADRATIC | SMOOTH_PARAMETRIC:
	case SMOOTH_NATURAL | SMOOTH_PARAMETRIC:
	    if (tracePtr->numPoints > 3) {
		GenerateParametricSplineOld(tracePtr);
	    }
	    break;

	case SMOOTH_CATROM:
	    if (tracePtr->numPoints > 3) {
		GenerateCatromSpline(tracePtr);
	    }
	    break;

	default:
	    break;
	}
    }
}


static double
DistanceToLineProc(
    int x, int y,			/* Sample X-Y coordinate. */
    Point2d *p, Point2d *q,		/* End points of the line segment. */
    Point2d *t)				/* (out) Point on line segment. */
{
    double right, left, top, bottom;

    *t = Blt_GetProjection(x, y, p, q);
    if (p->x > q->x) {
	right = p->x, left = q->x;
    } else {
	left = p->x, right = q->x;
    }
    if (p->y > q->y) {
	bottom = p->y, top = q->y;
    } else {
	top = p->y, bottom = q->y;
    }
    if (t->x > right) {
	t->x = right;
    } else if (t->x < left) {
	t->x = left;
    }
    if (t->y > bottom) {
	t->y = bottom;
    } else if (t->y < top) {
	t->y = top;
    }
    return hypot((t->x - x), (t->y - y));
}

static double
DistanceToXProc(
    int x, int y,			/* Search X-Y coordinate. */
    Point2d *p, 
    Point2d *q,				/* End points of the line segment. */
    Point2d *t)				/* (out) Point on line segment. */
{
    double dx, dy;
    double d;

    if (p->x > q->x) {
	if ((x > p->x) || (x < q->x)) {
	    return DBL_MAX;		/* X-coordinate outside line segment. */
	}
    } else {
	if ((x > q->x) || (x < p->x)) {
	    return DBL_MAX;		/* X-coordinate outside line segment. */
	}
    }
    dx = p->x - q->x;
    dy = p->y - q->y;
    t->x = (double)x;
    if (FABS(dx) < DBL_EPSILON) {
	double d1, d2;
	/* 
	 * Same X-coordinate indicates a vertical line.  Pick the closest end
	 * point.
	 */
	d1 = p->y - y;
	d2 = q->y - y;
	if (FABS(d1) < FABS(d2)) {
	    t->y = p->y, d = d1;
	} else {
	    t->y = q->y, d = d2;
	}
    } else if (FABS(dy) < DBL_EPSILON) {
	/* Horizontal line. */
	t->y = p->y, d = p->y - y;
    } else {
	double m, b;
		
	m = dy / dx;
	b = p->y - (m * p->x);
	t->y = (x * m) + b;
	d = y - t->y;
    }
   return FABS(d);
}

static double
DistanceToYProc(
    int x, int y,			/* Search X-Y coordinate. */
    Point2d *p, Point2d *q,		/* End points of the line segment. */
    Point2d *t)				/* (out) Point on line segment. */
{
    double dx, dy;
    double d;

    if (p->y > q->y) {
	if ((y > p->y) || (y < q->y)) {
	    return DBL_MAX;
	}
    } else {
	if ((y > q->y) || (y < p->y)) {
	    return DBL_MAX;
	}
    }
    dx = p->x - q->x;
    dy = p->y - q->y;
    t->y = y;
    if (FABS(dy) < DBL_EPSILON) {
	double d1, d2;

	/* Save Y-coordinate indicates an horizontal line. Pick the closest end
	 * point. */
	d1 = p->x - x;
	d2 = q->x - x;
	if (FABS(d1) < FABS(d2)) {
	    t->x = p->x, d = d1;
	} else {
	    t->x = q->x, d = d2;
	}
    } else if (FABS(dx) < DBL_EPSILON) {
	/* Vertical line. */
	t->x = p->x, d = p->x - x;
    } else {
	double m, b;
	
	m = dy / dx;
	b = p->y - (m * p->x);
	t->x = (y - b) / m;
	d = x - t->x;
    }
    return FABS(d);
}

/*
 *---------------------------------------------------------------------------
 *
 * ClosestPoint --
 *
 *	Find the element whose data point is closest to the given screen
 *	coordinate.
 *
 * Results:
 *	If a new minimum distance is found, the information regarding
 *	it is returned via searchPtr.
 *
 *---------------------------------------------------------------------------
 */
static int
ClosestPoint(
    LineElement *elemPtr,		/* Line element to be searched. */
    ClosestSearch *s)			/* Assorted information related to
					 * searching for the closest point */
{
    Blt_ChainLink link;
    double closestDistance;
    int closestIndex;
    Graph *graphPtr;

    closestDistance = s->dist;
    closestIndex = -1;
    graphPtr = elemPtr->obj.graphPtr;

    /*
     * Instead of testing each data point in graph coordinates, look at the
     * points of each trace (mapped screen coordinates). The advantages are
     *   1) only examine points that are visible (unclipped), and
     *   2) the computed distance is already in screen coordinates.
     */
    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;
	TracePoint *p;

	tracePtr = Blt_Chain_GetValue(link);
	for (p = tracePtr->head; p != NULL; p = p->next) {
	    double dx, dy;
	    double d;

	    if ((p->flags & KNOT) == 0) {
		continue;
	    }
	    if (!PLAYING(graphPtr, p->index)) {
		continue;
	    }
	    dx = (double)(s->x - p->x);
	    dy = (double)(s->y - p->y);
	    if (s->along == SEARCH_BOTH) {
		d = hypot(dx, dy);
	    } else if (s->along == SEARCH_X) {
		d = dx;
	    } else if (s->along == SEARCH_Y) {
		d = dy;
	    } else {
		/* This can't happen */
		continue;
	    }
	    if (d < closestDistance) {
		closestIndex = p->index;
		closestDistance = d;
	    }
	}
    }
    if (closestDistance < s->dist) {
	s->item    = elemPtr;
	s->dist    = closestDistance;
	s->index   = closestIndex;
	s->point.x = elemPtr->x.values[closestIndex];
	s->point.y = elemPtr->y.values[closestIndex];
	return TRUE;
    }
    return FALSE;
}

/*
 *---------------------------------------------------------------------------
 *
 * ClosestSegment --
 *
 *	Find the line segment closest to the given window coordinate in the
 *	element.
 *
 * Results:
 *	If a new minimum distance is found, the information regarding it is
 *	returned via searchPtr.
 *
 *---------------------------------------------------------------------------
 */
static int
ClosestSegment(
    Graph *graphPtr,			/* Graph widget record */
    LineElement *elemPtr,
    ClosestSearch *s,			/* Info about closest point in
					 * element */
    DistanceProc *distProc)
{
    Blt_ChainLink link;
    Point2d closestPoint;
    double closestDistance;
    int closestIndex;

    closestIndex = -1;			/* Suppress compiler warning. */
    closestDistance = s->dist;
    closestPoint.x = closestPoint.y = 0; /* Suppress compiler warning. */
    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;
	TracePoint *p, *q;

	tracePtr = Blt_Chain_GetValue(link);
	for (p = tracePtr->head, q = p->next; q != NULL; q = q->next) {
	    Point2d p1, p2, b;
	    double d;

	    if (!PLAYING(graphPtr, p->index)) {
		continue;
	    }
	    p1.x = p->x, p1.y = p->y;
	    p2.x = q->x, p2.y = q->y;
	    d = (*distProc)(s->x, s->y, &p1, &p2, &b);
	    if (d < closestDistance) {
		closestPoint    = b;
		closestIndex    = p->index;
		closestDistance = d;
	    }
	    p = q;
	}
    }
    if (closestDistance < s->dist) {
	s->dist = closestDistance;
	s->item	 = (Element *)elemPtr;
	s->index = closestIndex;
	s->point = Blt_InvMap2D(graphPtr, closestPoint.x, closestPoint.y, 
				&elemPtr->axes);
	return TRUE;
    }
    return FALSE;
}


/*
 *---------------------------------------------------------------------------
 *
 * ClosestProc --
 *
 *	Find the closest point or line segment (if interpolated) to the given
 *	window coordinate in the line element.
 *
 * Results:
 *	Returns the distance of the closest point among other information.
 *
 *---------------------------------------------------------------------------
 */
static void
ClosestProc(Graph *graphPtr, Element *basePtr, ClosestSearch *searchPtr)
{
    LineElement *elemPtr = (LineElement *)basePtr;
    int mode;

    mode = searchPtr->mode;
    if (mode == SEARCH_AUTO) {
	LinePen *penPtr;

	penPtr = NORMALPEN(elemPtr);
	mode = SEARCH_POINTS;
	if ((NUMBEROFPOINTS(elemPtr) > 1) && (penPtr->traceWidth > 0)) {
	    mode = SEARCH_TRACES;
	}
    }
    if (mode == SEARCH_POINTS) {
	ClosestPoint(elemPtr, searchPtr);
    } else {
	DistanceProc *distProc;
	int found;

	if (searchPtr->along == SEARCH_X) {
	    distProc = DistanceToXProc;
	} else if (searchPtr->along == SEARCH_Y) {
	    distProc = DistanceToYProc;
	} else {
	    distProc = DistanceToLineProc;
	}
	found = ClosestSegment(graphPtr, elemPtr, searchPtr, distProc);
	if ((!found) && (searchPtr->along != SEARCH_BOTH)) {
	    ClosestPoint(elemPtr, searchPtr);
	}
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * FindProc --
 *
 *	Find all the points within the designate circle on the screen.
 *
 * Results:
 *	Returns a list of the indices of the points that are within the
 *	search radius.
 *
 *---------------------------------------------------------------------------
 */
static Blt_Chain 
FindProc(Graph *graphPtr, Element *basePtr, int x, int y, int r)
{
    LineElement *elemPtr = (LineElement *)basePtr;
    Blt_ChainLink link;
    Blt_Chain chain;

    /*
     * Instead of testing each data point in graph coordinates, look at the
     * points of each trace (mapped screen coordinates). The advantages are
     *   1) only examine points that are visible (unclipped), and
     *   2) the computed distance is already in screen coordinates.
     */
    chain = Blt_Chain_Create();
    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;
	TracePoint *p;

	tracePtr = Blt_Chain_GetValue(link);
	for (p = tracePtr->head; p != NULL; p = p->next) {
	    double dx, dy;
	    double d;

	    if ((p->flags & KNOT) == 0) {
		continue;
	    }
	    if (!PLAYING(graphPtr, p->index)) {
		continue;
	    }
	    dx = (double)(x - p->x);
	    dy = (double)(y - p->y);
	    d = hypot(dx, dy);
	    if (d < r) {
		Blt_Chain_Append(chain, (ClientData)((long)p->index));
	    }
	}
    }
    return chain;
}

/*
 *---------------------------------------------------------------------------
 *
 * ExtentsProc --
 *
 *	Retrieves the range of the line element
 *
 * Results:
 *	Returns the number of data points in the element.
 *
 *---------------------------------------------------------------------------
 */
static void
ExtentsProc(Element *basePtr)
{
    LineElement *elemPtr = (LineElement *)basePtr;
    double xMin, xMax, yMin, yMax;
    double xPosMin, yPosMin;
    Region2d exts;
    int i;
    int np;

    exts.top = exts.left = DBL_MAX;
    exts.bottom = exts.right = -DBL_MAX;
    np = NUMBEROFPOINTS(elemPtr);
    if (np < 1) {
	return;
    } 
    xMin = yMin = xPosMin = yPosMin = DBL_MAX;
    xMax = yMax = -DBL_MAX;
    for (i = 0; i < np; i++) {
	double x, y;

	x = elemPtr->x.values[i];
	y = elemPtr->y.values[i];
	if ((!FINITE(x)) || (!FINITE(y))) {
	    continue;			/* Ignore holes in the data. */
	}
	if (x < xMin) {
	    xMin = x;
	} 
	if (x > xMax) {
	    xMax = x;
	}
	if ((x > 0.0) && (xPosMin > x)) {
	    xPosMin = x;
	}
	if (y < yMin) {
	    yMin = y;
	} 
	if (y > yMax) {
	    yMax = y;
	}
	if ((y > 0.0) && (yPosMin > y)) {
	    yPosMin = y;
	}
    }
    exts.right = xMax;
    if ((xMin <= 0.0) && (elemPtr->axes.x->logScale)) {
	exts.left = xPosMin;
    } else {
	exts.left = xMin;
    }
    exts.bottom = yMax;
    if ((yMin <= 0.0) && (elemPtr->axes.y->logScale)) {
	exts.top = yPosMin;
    } else {
	exts.top = yMin;
    }
#ifdef notdef
    /* Correct the data limits for error bars */
    if (elemPtr->xError.numValues > 0) {
	int i;
	
	np = MIN(elemPtr->xError.numValues, np);
	for (i = 0; i < np; i++) {
	    double x;

	    x = elemPtr->x.values[i] + elemPtr->xError.values[i];
	    if (x > exts.right) {
		exts.right = x;
	    }
	    x = elemPtr->x.values[i] - elemPtr->xError.values[i];
	    if (elemPtr->axes.x->logScale) {
		if (x < 0.0) {
		    x = -x;		/* Mirror negative values, instead of
					 * ignoring them. */
		}
		if ((x > DBL_MIN) && (x < exts.left)) {
		    exts.left = x;
		}
	    } else if (x < exts.left) {
		exts.left = x;
	    }
	}		     
    } else {
	if ((elemPtr->xHigh.numValues > 0) && 
	    (elemPtr->xHigh.max > exts.right)) {
	    exts.right = elemPtr->xHigh.max;
	}
	if (elemPtr->xLow.numValues > 0) {
	    double left;
	    
	    if ((elemPtr->xLow.min <= 0.0) && 
		(elemPtr->axes.x->logScale)) {
		left = Blt_FindElemValuesMinimum(&elemPtr->xLow, DBL_MIN);
	    } else {
		left = elemPtr->xLow.min;
	    }
	    if (left < exts.left) {
		exts.left = left;
	    }
	}
    }
    
    if (elemPtr->yError.numValues > 0) {
	int i;
	
	np = MIN(elemPtr->yError.numValues, np);
	for (i = 0; i < np; i++) {
	    double y;

	    y = elemPtr->y.values[i] + elemPtr->yError.values[i];
	    if (y > exts.bottom) {
		exts.bottom = y;
	    }
	    y = elemPtr->y.values[i] - elemPtr->yError.values[i];
	    if (elemPtr->axes.y->logScale) {
		if (y < 0.0) {
		    y = -y;		/* Mirror negative values, instead of
					 * ignoring them. */
		}
		if ((y > DBL_MIN) && (y < exts.left)) {
		    exts.top = y;
		}
	    } else if (y < exts.top) {
		exts.top = y;
	    }
	}		     
    } else {
	if ((elemPtr->yHigh.numValues > 0) && 
	    (elemPtr->yHigh.max > exts.bottom)) {
	    exts.bottom = elemPtr->yHigh.max;
	}
	if (elemPtr->yLow.numValues > 0) {
	    double top;
	    
	    if ((elemPtr->yLow.min <= 0.0) && 
		(elemPtr->axes.y->logScale)) {
		top = Blt_FindElemValuesMinimum(&elemPtr->yLow, DBL_MIN);
	    } else {
		top = elemPtr->yLow.min;
	    }
	    fprintf(stderr, "top=%g exttop=%g\n", top, exts.top);
	    if (top < exts.top) {
		exts.top = top;
	    }
	}
    }
#endif
    if (elemPtr->axes.x->valueRange.min > exts.left) {
	elemPtr->axes.x->valueRange.min = exts.left;
    }
    if (elemPtr->axes.x->valueRange.max < exts.right) {
	elemPtr->axes.x->valueRange.max = exts.right;
    }
    if (elemPtr->axes.y->valueRange.min > exts.top) {
	elemPtr->axes.y->valueRange.min = exts.top;
    }
    if (elemPtr->axes.y->valueRange.max < exts.bottom) {
	elemPtr->axes.y->valueRange.max = exts.bottom;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * BackgroundChangedProc
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
BackgroundChangedProc(ClientData clientData)
{
    Element *elemPtr = clientData;
    Graph *graphPtr;

    graphPtr = elemPtr->obj.graphPtr;
    if (graphPtr->tkwin != NULL) {
	graphPtr->flags |= REDRAW_WORLD;
	Blt_EventuallyRedrawGraph(graphPtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ConfigureProc --
 *
 *	Sets up the appropriate configuration parameters in the GC.  It is
 *	assumed the parameters have been previously set by a call to
 *	Blt_ConfigureWidget.
 *
 * Results:
 *	The return value is a standard TCL result.  If TCL_ERROR is returned,
 *	then interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information such as line width, line style, color
 *	etc. get set in a new GC.
 *
 *---------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ConfigureProc(Graph *graphPtr, Element *basePtr)
{
    LineElement *elemPtr = (LineElement *)basePtr;
    unsigned long gcMask;
    XGCValues gcValues;
    GC newGC;
    Blt_ChainLink link;
    LineStyle *stylePtr;

    if (ConfigurePenProc(graphPtr, (Pen *)&elemPtr->builtinPen) != TCL_OK) {
	return TCL_ERROR;
    }
    /*
     * Point to the static normal/active pens if no external pens have been
     * selected.
     */
    link = Blt_Chain_FirstLink(elemPtr->styles);
    if (link == NULL) {
	link = Blt_Chain_AllocLink(sizeof(LineStyle));
	Blt_Chain_LinkAfter(elemPtr->styles, link, NULL);
    } 
    stylePtr = Blt_Chain_GetValue(link);
    stylePtr->penPtr = NORMALPEN(elemPtr);
    if (elemPtr->fillBg != NULL) {
	Blt_Bg_SetChangedProc(elemPtr->fillBg, BackgroundChangedProc, 
		elemPtr);
    }
    /*
     * Set the outline GC for this pen: GCForeground is outline color.
     * GCBackground is the fill color (only used for bitmap symbols).
     */
    gcMask = 0;
    if (elemPtr->fillFgColor != NULL) {
	gcMask |= GCForeground;
	gcValues.foreground = elemPtr->fillFgColor->pixel;
    }
    if (elemPtr->fillBgColor != NULL) {
	gcMask |= GCBackground;
	gcValues.background = elemPtr->fillBgColor->pixel;
    }
    newGC = Tk_GetGC(graphPtr->tkwin, gcMask, &gcValues);
    if (elemPtr->fillGC != NULL) {
	Tk_FreeGC(graphPtr->display, elemPtr->fillGC);
    }
    elemPtr->fillGC = newGC;

    if (Blt_ConfigModified(elemPtr->configSpecs, "-scalesymbols", 
			   (char *)NULL)) {
	elemPtr->flags |= (MAP_ITEM | SCALE_SYMBOL);
    }
    if (Blt_ConfigModified(elemPtr->configSpecs, "-pixels", "-trace", 
	"-*data", "-smooth", "-map*", "-label", "-hide", "-x", "-y", 
	"-areabackground", (char *)NULL)) {
	elemPtr->flags |= MAP_ITEM;
    }
    return TCL_OK;
}


/*
 *---------------------------------------------------------------------------
 *
 * MapAreaUnderTrace --
 *
 *      Maps the polygon representing the area under the curve of each the
 *      trace. This must be done after the spline interpolation but before
 *      mapping polylines which may split the traces further.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory is freed and allocated for the index array.
 *
 *---------------------------------------------------------------------------
 */
static void
MapAreaUnderTrace(Trace *tracePtr)
{
    Graph *graphPtr;
    int n;
    Point2d *points, *clipPts;
    Region2d exts;

    n = tracePtr->numPoints + 3;
    points = Blt_AssertMalloc(sizeof(Point2d) * n);
    graphPtr = tracePtr->elemPtr->obj.graphPtr;
    if (graphPtr->inverted) {
	double minX;
	TracePoint *p;
	int count;

	count = 0;
	minX = (double)tracePtr->elemPtr->axes.y->screenMin;
	for (p = tracePtr->head; p != NULL; p = p->next) {
	    points[count].x = p->x + 1;
	    points[count].y = p->y;
	    if (points[count].x < minX) {
		minX = points[count].x;
	    }
	    count++;
	}	
	/* Add edges to make (if necessary) the polygon fill to the bottom of
	 * plotting window */
	points[count].x = minX;
	points[count].y = points[count - 1].y;
	count++;
	points[count].x = minX;
	points[count].y = points[0].y; 
	count++;
	points[count] = points[0];
    } else {
	double maxY;
	TracePoint *p;
	int count;

	count = 0;
	maxY = (double)tracePtr->elemPtr->axes.y->bottom;
	for (p = tracePtr->head; p != NULL; p = p->next) {
	    points[count].x = p->x + 1;
	    points[count].y = p->y;
	    if (points[count].y > maxY) {
		maxY = points[count].y;
	    }
	    count++;
	}	
	/* Add edges to extend the fill polygon to the bottom of plotting
	 * window */
	points[count].x = points[count - 1].x;
	points[count].y = maxY;
	count++;
	points[count].x = points[0].x; 
	points[count].y = maxY;
	count++;
	points[count] = points[0];
    }
    Blt_GraphExtents(tracePtr->elemPtr, &exts);

    clipPts = Blt_AssertMalloc(sizeof(Point2d) * n * 3);
    n = Blt_PolyRectClip(&exts, points, n - 1, clipPts);
    Blt_Free(points);

    if (n < 3) {
	Blt_Free(clipPts);
    } else {
	tracePtr->fillPts = clipPts;
	tracePtr->numFillPts = n;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * MapAreaUnderCurve --
 *
 *      Maps the polygon representing the area under the curve of each the
 *      trace. This must be done after the spline interpolation but before
 *      mapping polylines which may split the traces further.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory is freed and allocated for the index array.
 *
 *---------------------------------------------------------------------------
 */
static void
MapAreaUnderCurve(LineElement *elemPtr)
{
    Blt_ChainLink link;

    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;

	tracePtr = Blt_Chain_GetValue(link);
	MapAreaUnderTrace(tracePtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * MapActiveSymbols --
 *
 *	Creates an array of points of the active graph coordinates.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory is freed and allocated for the active point array.
 *
 *---------------------------------------------------------------------------
 */
static void
MapActiveSymbols(LineElement *elemPtr)
{
    Blt_ChainLink link;

    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;
	TracePoint *p;

	tracePtr = Blt_Chain_GetValue(link);
	for (p = tracePtr->head; p != NULL; p = p->next) {
	    Blt_HashEntry *hPtr;
	    long lindex;

	    p->flags &= ~ACTIVE_POINT;
	    lindex = (long)p->index;
	    hPtr = Blt_FindHashEntry(&elemPtr->activeTable, (char *)lindex);
	    if (hPtr != NULL) {
		p->flags |= ACTIVE_POINT;
	    }
	}
    }
}

#ifdef notdef
/*
 *---------------------------------------------------------------------------
 *
 * ReducePoints --
 *
 *	Generates a coordinate array of transformed screen coordinates from
 *	the data points.
 *
 * Results:
 *	The transformed screen coordinates are returned.
 *
 * Side effects:
 *	Memory is allocated for the coordinate array.
 *
 *---------------------------------------------------------------------------
 */
static void
ReducePoints(MapInfo *mapPtr, double tolerance)
{
    int i, np;
    Point2d *screenPts;
    int *map, *simple;

    simple    = Blt_AssertMalloc(tracePtr->numPoints * sizeof(int));
    map	      = Blt_AssertMalloc(tracePtr->numPoints * sizeof(int));
    screenPts = Blt_AssertMalloc(tracePtr->numPoints * sizeof(Point2d));
    np = Blt_SimplifyLine(origPts, 0, tracePtr->nScreenPts - 1, 
	tolerance, simple);
    for (i = 0; i < np; i++) {
	int k;

	k = simple[i];
	screenPts[i] = mapPtr->screenPts[k];
	map[i] = mapPtr->map[k];
    }
#ifdef notdef
    if (np < mapPtr->nScreenPts) {
	fprintf(stderr, "reduced from %d to %d\n", mapPtr->nScreenPts, np);
    }
#endif
    Blt_Free(mapPtr->screenPts);
    Blt_Free(mapPtr->map);
    Blt_Free(simple);
    mapPtr->screenPts = screenPts;
    mapPtr->map = map;
    mapPtr->nScreenPts = np;
}
#endif

/*
 *---------------------------------------------------------------------------
 *
 * MapErrorBars --
 *
 *	Creates two arrays of points and pen indices, filled with the screen
 *	coordinates of the visible
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory is freed and allocated for the index array.
 *
 *---------------------------------------------------------------------------
 */
static void
MapErrorBars(LineElement *elemPtr)
{
    Graph *graphPtr;
    Blt_ChainLink link;
    Region2d exts;

    graphPtr = elemPtr->obj.graphPtr;
    Blt_GraphExtents(elemPtr, &exts);
    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;
	TracePoint *p;

	tracePtr = Blt_Chain_GetValue(link);
	for (p = tracePtr->head; p != NULL; p = p->next) {
	    double x, y;
	    double xHigh, xLow, yHigh, yLow;
	    int ec2;
	    
	    if ((p->flags & (KNOT | VISIBLE)) != (KNOT | VISIBLE)) {
		continue;		/* Error bars only at specified
					 * points */
	    }
	    x = elemPtr->x.values[p->index];
	    y = elemPtr->y.values[p->index];
	    ec2 = tracePtr->errorCapWidth / 2;
	    if (elemPtr->xHigh.numValues > p->index) {
		xHigh = elemPtr->xHigh.values[p->index];
	    } else if (elemPtr->xError.numValues > p->index) {
		xHigh = elemPtr->x.values[p->index] + 
		    elemPtr->xError.values[p->index];
	    } else {
		xHigh = Blt_NaN();
	    }
	    if (FINITE(xHigh)) {
		Point2d high, p1, p2;
		/* 
		 *             |      
		 *   x,y ----xhigh,y      
		 *             |          
		 */
		
		p1 = high = Blt_Map2D(graphPtr, xHigh, y, &elemPtr->axes);
		p2.x = p->x, p2.y = p->y;
		/* Stem from the low x to point x at y. */
		if (Blt_LineRectClip(&exts, &p1, &p2)) {
		    TraceSegment *stem;

		    stem = NewSegment(elemPtr, p1.x, p1.y, p2.x, p2.y, p->index,
				  p->flags | XHIGH);
		    AddSegment(tracePtr, stem);
		}
		/* Cap from high + errorCapWith  */
		if (graphPtr->inverted) {
		    p1.y = p2.y = high.y;
		    p1.x = high.x-ec2;
		    p2.x = high.x+ec2;
		} else {
		    p1.x = p2.x = high.x;
		    p1.y = high.y-ec2;
		    p2.y = high.y+ec2;
		}
		if (Blt_LineRectClip(&exts, &p1, &p2)) {
		    TraceSegment *cap;

		    cap = NewSegment(elemPtr, p1.x, p1.y, p2.x, p2.y,
			p->index, p->flags | XHIGH);
		    AddSegment(tracePtr, cap);
		}
	    }
	    if (elemPtr->xLow.numValues > p->index) {
		xLow = elemPtr->xLow.values[p->index];
	    } else if (elemPtr->xError.numValues > p->index) {
		xLow = elemPtr->x.values[p->index] - 
		    elemPtr->xError.values[p->index];
	    } else {
		xLow = Blt_NaN();
	    }
	    if (FINITE(xLow)) {
		Point2d low, p1, p2;
		
		/* 
		 *     |      
		 *   xlow,y----x,y
		 *     |          
		 */
		p1 = low = Blt_Map2D(graphPtr, xLow, y, &elemPtr->axes);
		p2.x = p->x, p2.y = p->y;
		/* Stem from the low x to point x at y. */
		if (Blt_LineRectClip(&exts, &p1, &p2)) {
		    TraceSegment *stem;

		    stem = NewSegment(elemPtr, p1.x, p1.y, p2.x, p2.y, p->index,
			p->flags | XLOW);
		    AddSegment(tracePtr, stem);
		}
		/* Cap from low + errorCapWith  */
		if (graphPtr->inverted) {
		    p1.y = p2.y = low.y;
		    p1.x = low.x-ec2;
		    p2.x = low.x+ec2;
		} else {
		    p1.x = p2.x = low.x;
		    p1.y = low.y-ec2;
		    p2.y = low.y+ec2;
		}
		if (Blt_LineRectClip(&exts, &p1, &p2)) {
		    TraceSegment *cap;

		    cap = NewSegment(elemPtr, p1.x, p1.y, p2.x, p2.y, p->index,
			p->flags | XLOW);
		    AddSegment(tracePtr, cap);
		}
	    }
	    if (elemPtr->yHigh.numValues > p->index) {
		yHigh = elemPtr->yHigh.values[p->index];
	    } else if (elemPtr->yError.numValues > p->index) {
		yHigh = elemPtr->x.values[p->index] - 
		    elemPtr->yError.values[p->index];
	    } else {
		yHigh = Blt_NaN();
	    }
	    if (FINITE(yHigh)) {
		Point2d high, p1, p2;
		
		/* 
		 *   --x,yhigh--
		 *        | 
		 *        |
		 *       x,y
		 */
		p1 = high = Blt_Map2D(graphPtr, x, yHigh, &elemPtr->axes);
		p2.x = p->x, p2.y = p->y;
		/* Stem from the low x to point x at y. */
		if (Blt_LineRectClip(&exts, &p1, &p2)) {
		    TraceSegment *stem;

		    stem = NewSegment(elemPtr, p1.x, p1.y, p2.x, p2.y, p->index,
			p->flags | YHIGH);
		    AddSegment(tracePtr, stem);
		}
		if (graphPtr->inverted) {
		    p1.x = p2.x = high.x;
		    p1.y = high.y-ec2;
		    p2.y = high.y+ec2;
		} else {
		    p1.y = p2.y = high.y;
		    p1.x = high.x-ec2;
		    p2.x = high.x+ec2;
		}
		if (Blt_LineRectClip(&exts, &p1, &p2)) {
		    TraceSegment *cap;

		    cap = NewSegment(elemPtr, p1.x, p1.y, p2.x, p2.y, p->index,
			p->flags | YHIGH);
		    AddSegment(tracePtr, cap);
		}
	    }
	    if (elemPtr->yLow.numValues > p->index) {
		yLow = elemPtr->yLow.values[p->index];
	    } else if (elemPtr->yError.numValues > p->index) {
		yLow = elemPtr->x.values[p->index] - 
		    elemPtr->yError.values[p->index];
	    } else {
		yLow = Blt_NaN();
	    }
	    if (FINITE(yLow)) {
		Point2d low, p1, p2;
		/* 
		 *       x,y
		 *        | 
		 *        |
		 *    --ylow,y--
		 */
		p1 = low = Blt_Map2D(graphPtr, x, yLow, &elemPtr->axes);
		p2.x = p->x, p2.y = p->y;
		/* Stem from the low x to point x at y. */
		if (Blt_LineRectClip(&exts, &p1, &p2)) {
		    TraceSegment *stem;

		    stem = NewSegment(elemPtr, p1.x, p1.y, p2.x, p2.y, p->index,
			p->flags | YLOW);
		    AddSegment(tracePtr, stem);
		}
		if (graphPtr->inverted) {
		    p1.x = p2.x = low.x;
		    p1.y = low.y-ec2;
		    p2.y = low.y+ec2;
		} else {
		    p1.y = p2.y = low.y;
		    p1.x = low.x-ec2;
		    p2.x = low.x+ec2;
		}
		if (Blt_LineRectClip(&exts, &p1, &p2)) {
		    TraceSegment *cap;

		    cap = NewSegment(elemPtr, p1.x, p1.y, p2.x, p2.y, p->index,
			p->flags | YLOW);
		    AddSegment(tracePtr, cap);
		}
	    }
	}
    }
}
    
/*
 *---------------------------------------------------------------------------
 *
 * MapPolyline --
 *
 *	Adjust the trace by testing each segment of the trace to the graph
 *	area.  If the segment is totally off screen, remove it from the trace.
 *	If one end point is off screen, replace it with the clipped point.
 *	Create new traces as necessary.
 *	
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory is  allocated for the line segment array.
 *
 *---------------------------------------------------------------------------
 */
static void
MapPolyline(LineElement *elemPtr, Trace *tracePtr)
{
    TracePoint *p, *q;
    Region2d exts;

    Blt_GraphExtents(elemPtr, &exts);
    for (p = tracePtr->head, q = p->next; q != NULL; q = q->next) {
	if (p->flags & q->flags & VISIBLE) {
	    p = q;
	    continue;			/* Segment is visible. */
	}
	/* Clip required. */
	if (p->flags & VISIBLE) {	/* Last point is off screen. */
	    Point2d p1, p2;

 	    p1.x = p->x, p1.y = p->y;
	    p2.x = q->x, p2.y = q->y;
	    if (Blt_LineRectClip(&exts, &p1, &p2)) {
		TracePoint *t;
		Trace *newPtr;

		/* Last point is off screen.  Add the clipped end the current
		 * trace. */
		t = NewPoint(elemPtr, p2.x, p2.y, q->index);
		t->flags = VISIBLE;
		tracePtr->flags |= RECOUNT;
		tracePtr->tail = t;
		p->next = t;		/* Point t terminates the trace. */

		/* Create a new trace and attach the current chain to it. */
		newPtr = NewTrace(elemPtr);
		newPtr->flags |= RECOUNT;
		newPtr->head = newPtr->tail = q;
		newPtr->symbolSize = tracePtr->symbolSize;
		newPtr->penPtr = tracePtr->penPtr;
		newPtr->errorCapWidth = tracePtr->errorCapWidth;
		tracePtr = newPtr;
	    }
	} else if (q->flags & VISIBLE) {  /* First point in offscreen. */
	    Point2d p1, p2;

	    /* First point is off screen.  Replace it with the clipped end. */
	    p1.x = p->x, p1.y = p->y;
	    p2.x = q->x, p2.y = q->y;
	    if (Blt_LineRectClip(&exts, &p1, &p2)) {
		p->x = p1.x;
		p->y = p1.y;
		/* The replaced point is now visible but longer a knot. */
		p->flags |= VISIBLE;
		p->flags &= ~KNOT;
	    }
	} else {
	    /* Segment is offscreen. Remove the first point. */
	    assert(tracePtr->head == p);
	    RemoveHead(elemPtr, tracePtr);
	}
	p = q;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * MapPolylines --
 *
 *	Creates an array of line segments of the graph coordinates.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory is  allocated for the line segment array.
 *
 *---------------------------------------------------------------------------
 */
static void
MapPolylines(LineElement *elemPtr)
{
    Blt_ChainLink link;

    /* Step 1: Process traces by clipping them against the plot area. */
    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL; 
	 link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;

	tracePtr = Blt_Chain_GetValue(link);
	if ((tracePtr->numPoints < 2) || (tracePtr->penPtr->traceWidth == 0)) {
	    continue;
	}
	MapPolyline(elemPtr, tracePtr);
    }
    /* Step 2: Fix traces that have been split. */
    FixTraces(elemPtr->traces);
}

static LinePen *
WeightToPen(LineElement *elemPtr, double weight)
{
    Blt_ChainLink link;

    for (link = Blt_Chain_LastLink(elemPtr->styles); link != NULL;  
	 link = Blt_Chain_PrevLink(link)) {
	LineStyle *stylePtr;
	
	stylePtr = Blt_Chain_GetValue(link);
	if (stylePtr->weight.range > 0.0) {
	    double norm;
	    
	    norm = (weight - stylePtr->weight.min) / stylePtr->weight.range;
	    if (((norm - 1.0) <= DBL_EPSILON) && 
		(((1.0 - norm) - 1.0) <= DBL_EPSILON)) {
		return stylePtr->penPtr;
	    }
	}
    }
    return NORMALPEN(elemPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * MapStyles --
 *
 *	Splits traces based on the pen used.  May create many more traces
 *	if the traces change pens frequently.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	New traces may be created.  Traces may be split.
 *
 *---------------------------------------------------------------------------
 */
static void
MapStyles(LineElement *elemPtr)
{
    Blt_ChainLink link;

    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;
	TracePoint *p, *q;
	LinePen *penPtr;

	tracePtr = Blt_Chain_GetValue(link);
	/* For each point in the trace, see what pen it corresponds to. */
	p = tracePtr->head;
	
	if (elemPtr->w.numValues > p->index) {
	    penPtr = WeightToPen(elemPtr,elemPtr->w.values[p->index]);
	} else {
	    penPtr = NORMALPEN(elemPtr);
	}
	tracePtr->symbolSize = ScaleSymbol(elemPtr, penPtr->symbol.size);
	tracePtr->errorCapWidth = (penPtr->errorCapWidth > 0) 
	    ? penPtr->errorCapWidth : tracePtr->symbolSize;
	tracePtr->penPtr = penPtr;

	for (q = p->next; q != NULL; q = q->next) {
	    LinePen *penPtr;

	    if (elemPtr->w.numValues > q->index) {
		penPtr = WeightToPen(elemPtr, elemPtr->w.values[q->index]);
	    } else {
		penPtr = NORMALPEN(elemPtr);
	    }
	    if (penPtr != tracePtr->penPtr) {
		TracePoint *t;
		/* 
		 * If the mapped style is not the current style, create a new
		 * trace of that style and break the trace.
		 */

		/* Create a copy of the current point and insert it as new end
		 * point for the current trace.  This point will not be a
		 * knot. */
		t = NewPoint(elemPtr, q->x, q->y, q->index);
		tracePtr->tail = t;
		p->next = t;		/* Point t terminates the trace. */

		/* Now create a new trace.  The first point will be the
		 * current point. The pen for the trace is the current pen. */
		tracePtr->flags |= RECOUNT;
		tracePtr = NewTrace(elemPtr);
		tracePtr->penPtr = penPtr;
		tracePtr->symbolSize = ScaleSymbol(elemPtr,penPtr->symbol.size);
		tracePtr->errorCapWidth = (penPtr->errorCapWidth > 0) 
		    ? penPtr->errorCapWidth : tracePtr->symbolSize;
		AppendPoint(tracePtr, q);
		tracePtr->flags |= RECOUNT;
	    }
	    p = q;
	}
    }
    /* Step 2: Fix traces that have been split. */
    FixTraces(elemPtr->traces);
}

/*
 * MapProc --
 *
 *	Converts the graph coordinates into screen coordinates representing
 *	the line element.  The screen coordinates are stored in a linked list
 *	or points representing a set of connected point (a trace).  Generated
 *	points may be added to the traces because of smoothing.  Points may be
 *	removed if the are off screen.  A trace may contain one or more points.
 *
 *	Originally all points are in a single list (trace).  They are
 *	processed and possibly split into new traces until each trace
 *	represents a contiguous set of on-screen points using the same line
 *	style (pen).  New traces are broken when the points go off screen or
 *	use a different line style than the previous point.
 *	
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory is (re)allocated for the points.
 */
static void
MapProc(Graph *graphPtr, Element *basePtr)
{
    LineElement *elemPtr = (LineElement *)basePtr;
    int n;

    ResetElement(elemPtr);
    n = NUMBEROFPOINTS(elemPtr);
    if (n < 1) {
	return;				/* No data points */
    }
    GetScreenPoints(elemPtr);
    elemPtr->smooth = elemPtr->reqSmooth;
    if (n > 1) {
	/* Note to users: For scatter plots, don't turn on smoothing.  We
	 * can't check if the traceWidth is 0, because we haven't mapped
	 * styles yet.  But we need to smooth before the traces get split by
	 * styles. */
	if (elemPtr->smooth != SMOOTH_NONE) {
	    SmoothElement(elemPtr);
	}
#ifdef notdef
	if (elemPtr->rTolerance > 0.0) {
	    ReducePoints(&mi, elemPtr->rTolerance);
	}
#endif
    }
    if ((elemPtr->fillBg != NULL) || (elemPtr->colormapPtr != NULL)) {
	MapAreaUnderCurve(elemPtr);
    }
    /* Split traces based upon style.  The pen associated with the trace
     * determines if polylines or symbols are required, the size of the
     * errorbars and symbols, etc. */
    MapStyles(elemPtr);
    if (n > 1) {
	MapPolylines(elemPtr);
    }
    if (elemPtr->numActiveIndices >= 0) {
	MapActiveSymbols(elemPtr);
    }
    /* This has to be done last since we don't split the errorbar segments
     * when we split a trace.  */
    MapErrorBars(elemPtr);
}


static int
GradientColorProc(Blt_Paintbrush *brushPtr, int x, int y)
{
    Blt_Pixel color;
    Graph *graphPtr;
    LineElement *elemPtr = brushPtr->clientData;
    GraphColormap *cmapPtr;
    double value;
    Point2d point;

    graphPtr = elemPtr->obj.graphPtr;
    cmapPtr = elemPtr->colormapPtr;
    point = Blt_InvMap2D(graphPtr, x, y, &elemPtr->axes);
    if (cmapPtr->axisPtr == elemPtr->axes.y) {
	value = point.y;
    } else if (cmapPtr->axisPtr == elemPtr->axes.x) {
	value = point.x;
    } else {
	return 0x0;
    }
#ifdef notdef
    fprintf(stderr, "value=%g, min=%g max=%g x=%d,y=%d axis=%s\n", value, cmapPtr->min, cmapPtr->max, x, y, cmapPtr->axisPtr->obj.name);
#endif
    color.u32 = Blt_Palette_GetColorFromAbsoluteValue(brushPtr->palette, value,
	cmapPtr->min, cmapPtr->max);
    return color.u32;
}

static void
GetPolygonBBox(XPoint *points, int n, int *leftPtr, int *rightPtr, int *topPtr, 
	       int *bottomPtr)
{
    XPoint *p, *pend;
    int left, right, bottom, top;

    /* Determine the bounding box of the polygon. */
    left = right = points[0].x;
    top = bottom = points[0].y;
    for (p = points, pend = p + n; p < pend; p++) {
	if (p->x < left) {
	    left = p->x;
	} 
	if (p->x > right) {
	    right = p->x;
	}
	if (p->y < top) {
	    top = p->y;
	} 
	if (p->y > bottom) {
	    bottom = p->y;
	}
    }
    if (leftPtr != NULL) {
	*leftPtr = left;
    }
    if (rightPtr != NULL) {
	*rightPtr = right;
    }
    if (topPtr != NULL) {
	*topPtr = top;
    }
    if (bottomPtr != NULL) {
	*bottomPtr = bottom;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawGradientPolygon --
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawGradientPolygon(Graph *graphPtr, Drawable drawable, LineElement *elemPtr, 
		    int n, XPoint *points)
{
    Blt_Paintbrush brush;
    Blt_Picture bg;
    Blt_Painter painter;
    int i;
    int w, h;
    int x1, x2, y1, y2;
    Point2f *vertices;

    if (n < 3) {
	return;				/* Not enough points for polygon */
    }
    if (elemPtr->colormapPtr->palette == NULL) {
	return;				/* No palette defined. */
    }
    /* Grab the rectangular background that covers the polygon. */
    GetPolygonBBox(points, n, &x1, &x2, &y1, &y2);
    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    bg = Blt_DrawableToPicture(graphPtr->tkwin, drawable, x1, y1, w, h, 1.0);
    if (bg == NULL) {
	return;				/* Background is obscured. */
    }
    vertices = Blt_AssertMalloc(n * sizeof(Point2f));
    /* Translate the polygon */
    for (i = 0; i < n; i++) {
	vertices[i].x = (float)(points[i].x - x1);
	vertices[i].y = (float)(points[i].y - y1);
    }
    Blt_Colormap_Init(elemPtr->colormapPtr);
    Blt_Paintbrush_Init(&brush);
    Blt_Paintbrush_SetOrigin(&brush, -x1, -y1);
    Blt_Paintbrush_SetPalette(&brush, elemPtr->colormapPtr->palette);
    Blt_Paintbrush_SetColorProc(&brush, GradientColorProc, elemPtr);
    Blt_PaintPolygon(bg, n, vertices, &brush);
    Blt_Free(vertices);
    painter = Blt_GetPainter(graphPtr->tkwin, 1.0);
    Blt_PaintPicture(painter, drawable, bg, 0, 0, w, h, x1, y1, 0);
    Blt_FreePicture(bg);
}

/* 
 * DrawAreaUnderCurve --
 *
 *	Draws the polygons under the traces.
 */
static void
DrawAreaUnderCurve(Graph *graphPtr, Drawable drawable, LineElement *elemPtr)
{
    Blt_ChainLink link;

    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL; 
	 link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;

	tracePtr = Blt_Chain_GetValue(link);
	if ((tracePtr->numFillPts > 0) && 
	    ((elemPtr->fillBg != NULL) || (elemPtr->colormapPtr != NULL))) {
	    XPoint *points;
	    int i;

	    points = Blt_AssertMalloc(sizeof(XPoint) * tracePtr->numFillPts);
	    for (i = 0; i < tracePtr->numFillPts; i++) {
		points[i].x = tracePtr->fillPts[i].x;
		points[i].y = tracePtr->fillPts[i].y;
	    }
	    if (elemPtr->colormapPtr != NULL) {
		DrawGradientPolygon(graphPtr, drawable, elemPtr,
				    tracePtr->numFillPts, points);
	    } else {
		Blt_Bg_SetOrigin(graphPtr->tkwin, elemPtr->fillBg, 0, 0);
		Blt_Bg_FillPolygon(graphPtr->tkwin, drawable, elemPtr->fillBg, 
			points, tracePtr->numFillPts, 0, TK_RELIEF_FLAT);
		Blt_Free(points);
	    }
	}
    }
}


#ifdef WIN32

/* 
 * DrawPolyline --
 *
 *	Draws the connected line segments representing the trace.
 *
 *	This MSWindows version arbitrarily breaks traces greater than one
 *	hundred points that are wide lines, into smaller pieces.
 */
static void
DrawPolyline(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
	     LinePen *penPtr)
{
    HBRUSH brush, oldBrush;
    HDC dc;
    HPEN pen, oldPen;
    POINT *points;
    TkWinDCState state;
    int maxPoints;			/* Maximum # of points in a single
					 * polyline. */
    TracePoint *p;
    int count;

    /*  
     * If the line is wide (> 1 pixel), arbitrarily break the line in sections
     * of 100 points.  This bit of weirdness has to do with wide geometric
     * pens.  The longer the polyline, the slower it draws.  The trade off is
     * that we lose dash and cap uniformity for unbearably slow polyline
     * draws.
     */
    if (penPtr->traceGC->line_width > 1) {
	maxPoints = 100;
    } else {
	maxPoints = Blt_MaxRequestSize(graphPtr->display, sizeof(POINT)) - 1;
    }
    points = Blt_AssertMalloc((maxPoints + 1) * sizeof(POINT));

    dc = TkWinGetDrawableDC(graphPtr->display, drawable, &state);

    /* FIXME: Add clipping region here. */

    pen = Blt_GCToPen(dc, penPtr->traceGC);
    oldPen = SelectPen(dc, pen);
    brush = CreateSolidBrush(penPtr->traceGC->foreground);
    oldBrush = SelectBrush(dc, brush);
    SetROP2(dc, tkpWinRopModes[penPtr->traceGC->function]);

    count = 0;
    for (p = tracePtr->head; p != NULL; p = p->next) {
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	points[count].x = Round(p->x);
	points[count].y = Round(p->y);
	count++;
	if (count >= maxPoints) {
	    Polyline(dc, points, count);
	    points[0] = points[count - 1];
	    count = 1;
	}
    }
    if (count > 1) {
	Polyline(dc, points, count);
    }
    Blt_Free(points);
    DeletePen(SelectPen(dc, oldPen));
    DeleteBrush(SelectBrush(dc, oldBrush));
    TkWinReleaseDrawableDC(drawable, dc, &state);
}

#else

/* 
 * DrawPolyline --
 *
 *	Draws the connected line segments representing the trace.
 *
 *	This X11 version arbitrarily breaks traces greater than the server
 *	request size, into smaller pieces.
 */
static void
DrawPolyline(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
	     LinePen *penPtr)
{
    TracePoint *p;
    XPoint *points;
    int maxPoints;
    int count;

    maxPoints = MAX_DRAWLINES(graphPtr->display);
    if (maxPoints > tracePtr->numPoints) {
	maxPoints = tracePtr->numPoints;
    } 
    points = Blt_AssertMalloc((maxPoints + 1) * sizeof(XPoint));
    count = 0;			/* Counter for points */
    for (p = tracePtr->head; p != NULL; p = p->next) {
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	points[count].x = Round(p->x);
	points[count].y = Round(p->y);
	count++;
	if (count >= maxPoints) {
	    XDrawLines(graphPtr->display, drawable, penPtr->traceGC, points,
		       count, CoordModeOrigin);
	    points[0] = points[count - 1];
	    count = 1;
	}
    }
    if (count > 1) {
	XDrawLines(graphPtr->display, drawable, penPtr->traceGC, points,
		   count, CoordModeOrigin);
    }
    Blt_Free(points);
}
#endif /* WIN32 */

/* 
 * DrawErrorBars --
 *
 *	Draws the segments representing the parts of the the error bars.  As
 *	many segments are draw as once as can fit into an X server request.
 *
 *	Errorbars are only drawn at the knots of the trace (i.e. original
 *	points, not generated).  The "play" function can limit what bars are
 *	drawn.
 */
static void 
DrawErrorBars(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
	      LinePen *penPtr)
{
    XSegment *segments;
    TraceSegment *s;
    int count;
    int maxSegments;

    maxSegments = MAX_DRAWSEGMENTS(graphPtr->display);
    if (maxSegments > tracePtr->numSegments) {
	maxSegments = tracePtr->numSegments;
    }
    segments = Blt_Malloc(maxSegments * sizeof(XSegment));
    if (segments == NULL) {
	return;
    }
    count = 0;				/* Counter for segments */
    tracePtr->flags |= KNOT;
    for (s = tracePtr->segments; s != NULL; s = s->next) {
	if ((s->flags & penPtr->errorFlags) == 0) {
	    continue;
	}
	if ((!PLAYING(graphPtr, s->index)) || (!DRAWN(tracePtr, s->flags))) {
	    continue;
	}
	segments[count].x1 = (short int)Round(s->x1);
	segments[count].y1 = (short int)Round(s->y1);
	segments[count].x2 = (short int)Round(s->x2);
	segments[count].y2 = (short int)Round(s->y2);
	count++;
	if (count >= maxSegments) {
	    XDrawSegments(graphPtr->display, drawable, penPtr->errorGC, 
			  segments, count);
	    count = 0;
	}
    }
    if (count > 0) {
	XDrawSegments(graphPtr->display, drawable, penPtr->errorGC, 
		      segments, count);
    }
    tracePtr->drawFlags &= ~(YERROR | XERROR);
    Blt_Free(segments);
}

/* 
 * DrawValues --
 *
 *	Draws text of the numeric values of the point.
 *
 *	Values are only drawn at the knots of the trace (i.e. original points,
 *	not generated).  The "play" function can limit what value are drawn.
 */
static void
DrawValues(Graph *graphPtr, Drawable drawable, Trace *tracePtr, LinePen *penPtr)
{
    TracePoint *p;
    const char *fmt;
    
    fmt = penPtr->valueFormat;
    if (fmt == NULL) {
	fmt = "%g";
    }
    for (p = tracePtr->head; p != NULL; p = p->next) {
	double x, y;
	char string[200];

	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	x = tracePtr->elemPtr->x.values[p->index];
	y = tracePtr->elemPtr->y.values[p->index];
	if (penPtr->valueFlags == SHOW_X) {
	    Blt_FormatString(string, TCL_DOUBLE_SPACE, fmt, x); 
	} else if (penPtr->valueFlags == SHOW_Y) {
	    Blt_FormatString(string, TCL_DOUBLE_SPACE, fmt, y); 
	} else if (penPtr->valueFlags == SHOW_BOTH) {
	    Blt_FormatString(string, TCL_DOUBLE_SPACE, fmt, x);
	    strcat(string, ",");
	    Blt_FormatString(string + strlen(string), TCL_DOUBLE_SPACE, fmt, y);
	}
	Blt_DrawText(graphPtr->tkwin, drawable, string, 
	     &penPtr->valueStyle, Round(p->x), Round(p->y));
    }
}

/* 
 * DrawPointSymbols --
 *
 *	Draws the symbols of the trace as points.
 *
 *	Symbols are only drawn at the knots of the trace (i.e. original points,
 *	not generated).  The "play" function can limit what points are drawn.
 */
static void
DrawPointSymbols(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
		 LinePen *penPtr)
{
    TracePoint *p;
    XPoint *points;
    int count;
    int maxPoints;

    maxPoints = MAX_DRAWPOINTS(graphPtr->display);
    if (maxPoints > tracePtr->numPoints) {
	maxPoints = tracePtr->numPoints;
    }
    points = Blt_Malloc(maxPoints * sizeof(XPoint));
    if (points == NULL) {
	return;
    }
    count = 0;				/* Counter for points. */
    for (p = tracePtr->head; p != NULL; p = p->next) {
	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	points[count].x = Round(p->x);
	points[count].y = Round(p->y);
	count++;
	if (count >= maxPoints) {
	    XDrawPoints(graphPtr->display, drawable, penPtr->symbol.fillGC, 
			points, count, CoordModeOrigin);
	    count = 0;
	}
    }
    if (count > 0) {
	XDrawPoints(graphPtr->display, drawable, penPtr->symbol.fillGC, 
		    points, count, CoordModeOrigin);
    }
    Blt_Free(points);
}


#ifdef WIN32

/* 
 * DrawCircleSymbols --
 *
 *	Draws the symbols of the trace as circles.  The outlines of circles
 *	are drawn after circles are filled.  This is speed tradeoff: drawn
 *	many circles at once, or drawn one symbol at a time.
 *
 *	Symbols are only drawn at the knots of the trace (i.e. original points,
 *	not generated).  The "play" function can limit what circles are drawn.
 *
 */
static void
DrawCircleSymbols(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
		  LinePen *penPtr, int size)
{
    HBRUSH brush, oldBrush;
    HPEN pen, oldPen;
    HDC dc;
    TkWinDCState state;
    int r;
    TracePoint *p;

    r = (int)ceil(size * 0.5);
    if (drawable == None) {
	return;				/* Huh? */
    }
    if ((penPtr->symbol.fillGC == NULL) && 
	(penPtr->symbol.outlineWidth == 0)) {
	return;
    }
    dc = TkWinGetDrawableDC(graphPtr->display, drawable, &state);
    /* SetROP2(dc, tkpWinRopModes[penPtr->symbol.fillGC->function]); */
    if (penPtr->symbol.fillGC != NULL) {
	brush = CreateSolidBrush(penPtr->symbol.fillGC->foreground);
    } else {
	brush = GetStockBrush(NULL_BRUSH);
    }
    if (penPtr->symbol.outlineWidth > 0) {
	pen = Blt_GCToPen(dc, penPtr->symbol.outlineGC);
    } else {
	pen = GetStockPen(NULL_PEN);
    }
    oldPen = SelectPen(dc, pen);
    oldBrush = SelectBrush(dc, brush);
    for (p = tracePtr->head; p != NULL; p = p->next) {
	int rx, ry;

	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	rx = Round(p->x);
	ry = Round(p->y);
	Ellipse(dc, rx - r, ry - r, rx + r + 1, ry + r + 1);
    }
    DeleteBrush(SelectBrush(dc, oldBrush));
    DeletePen(SelectPen(dc, oldPen));
    TkWinReleaseDrawableDC(drawable, dc, &state);
}

#else

/* 
 * DrawCircleSymbols --
 *
 *	Draws the symbols of the trace as circles.  The outlines of circles
 *	are drawn after circles are filled.  This is speed tradeoff: draw
 *	many circles at once, or drawn one symbol at a time.
 *
 *	Symbols are only drawn at the knots of the trace (i.e. original points,
 *	not generated).  The "play" function can limit what circles are drawn.
 *
 */
static void
DrawCircleSymbols(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
		  LinePen *penPtr, int size)
{
    XArc *arcs;
    TracePoint *p;
    int maxArcs;
    int r, s, count;

    r = (int)ceil(size * 0.5);
    s = r + r;
    maxArcs = MAX_DRAWARCS(graphPtr->display);
    if (maxArcs > tracePtr->numPoints) {
	maxArcs = tracePtr->numPoints;
    }
    arcs = Blt_Malloc(maxArcs * sizeof(XArc));
    if (arcs == NULL) {
	return;
    }
    count = 0;				/* Counter for arcs. */
    for (p = tracePtr->head; p != NULL; p = p->next) {
	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	arcs[count].x = Round(p->x - r);
	arcs[count].y = Round(p->y - r);
	arcs[count].width = (unsigned short)s;
	arcs[count].height = (unsigned short)s;
	arcs[count].angle1 = 0;
	arcs[count].angle2 = 23040;
	count++;
	if (count >= maxArcs) {
	    if (penPtr->symbol.fillGC != NULL) {
		XFillArcs(graphPtr->display, drawable, penPtr->symbol.fillGC, 
			arcs, count);
	    }
	    if (penPtr->symbol.outlineWidth > 0) {
		XDrawArcs(graphPtr->display, drawable, penPtr->symbol.outlineGC,
			  arcs, count);
	    }
	    count = 0;
	}
    }
    if (count > 0) {
	if (penPtr->symbol.fillGC != NULL) {
	    XFillArcs(graphPtr->display, drawable, penPtr->symbol.fillGC, 
		      arcs, count);
	}
	if (penPtr->symbol.outlineWidth > 0) {
	    XDrawArcs(graphPtr->display, drawable, penPtr->symbol.outlineGC,
		      arcs, count);
	}
    }
    Blt_Free(arcs);
}

#endif

/* 
 * DrawSquareSymbols --
 *
 *	Draws the symbols of the trace as squares.  The outlines of squares
 *	are drawn after squares are filled.  This is speed tradeoff: draw
 *	many squares at once, or drawn one symbol at a time.
 *
 *	Symbols are only drawn at the knots of the trace (i.e. original points,
 *	not generated).  The "play" function can limit what squares are drawn.
 *
 */
static void
DrawSquareSymbols(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
		  LinePen *penPtr, int size)
{
    XRectangle *rectangles;
    TracePoint *p;
    int maxRectangles;
    int r, s, count;

    r = (int)ceil(size * S_RATIO * 0.5);
    s = r + r;
    maxRectangles = MAX_DRAWRECTANGLES(graphPtr->display);
    if (maxRectangles > tracePtr->numPoints) {
	maxRectangles = tracePtr->numPoints;
    }
    rectangles = Blt_Malloc(maxRectangles * sizeof(XRectangle));
    if (rectangles == NULL) {
	return;
    }
    count = 0;				/* Counter for rectangles. */
    for (p = tracePtr->head; p != NULL; p = p->next) {
	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	rectangles[count].x = Round(p->x - r);
	rectangles[count].y = Round(p->y - r);
	rectangles[count].width = s;
	rectangles[count].height = s;
	count++;
	if (count >= maxRectangles) {
	    if (penPtr->symbol.fillGC != NULL) {
		XFillRectangles(graphPtr->display, drawable, 
			penPtr->symbol.fillGC, rectangles, count);
	    }
	    if (penPtr->symbol.outlineWidth > 0) {
		XDrawRectangles(graphPtr->display, drawable, 
			penPtr->symbol.outlineGC, rectangles, count);
	    }
	    count = 0;
	}
    }
    if (count > 0) {
	if (penPtr->symbol.fillGC != NULL) {
	    XFillRectangles(graphPtr->display, drawable, penPtr->symbol.fillGC,
			    rectangles, count);
	}
	if (penPtr->symbol.outlineWidth > 0) {
	    XDrawRectangles(graphPtr->display, drawable, 
		penPtr->symbol.outlineGC, rectangles, count);
	}
    }
    Blt_Free(rectangles);
}

/* 
 * DrawSkinnyCrossPlusSymbols --
 *
 *	Draws the symbols of the trace as single line crosses or pluses.  
 *
 *	Symbols are only drawn at the knots of the trace (i.e. original points,
 *	not generated).  The "play" function can limit what symbols are drawn.
 */
static void
DrawSkinnyCrossPlusSymbols(Graph *graphPtr, Drawable drawable, Trace *tracePtr,
			   LinePen *penPtr, int size)
{
    TracePoint *p;
    XPoint pattern[13];			/* Template for polygon symbols */
    XSegment *segments;
    int maxSegments;
    int r, count;

    r = (int)ceil(size * 0.5);
    maxSegments = MAX_DRAWSEGMENTS(graphPtr->display);
    if (maxSegments < (tracePtr->numPoints * 2)) {
	maxSegments = tracePtr->numPoints * 2;
    }
    segments = Blt_Malloc(maxSegments * sizeof(XSegment));
    if (segments == NULL) {
	return;
    }
    if (penPtr->symbol.type == SYMBOL_SCROSS) {
	r = Round((double)r * M_SQRT1_2);
	pattern[3].y = pattern[2].x = pattern[0].x = pattern[0].y = -r;
	pattern[3].x = pattern[2].y = pattern[1].y = pattern[1].x = r;
    } else {
	pattern[0].y = pattern[1].y = pattern[2].x = pattern[3].x = 0;
	pattern[0].x = pattern[2].y = -r;
	pattern[1].x = pattern[3].y = r;
    }
    count = 0;				/* Counter for segments. */
    for (p = tracePtr->head; p != NULL; p = p->next) {
	int rx, ry;

	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	rx = Round(p->x);
	ry = Round(p->y);
	segments[count].x1 = pattern[0].x + rx;
	segments[count].y1 = pattern[0].y + ry;
	segments[count].x2 = pattern[1].x + rx;
	segments[count].y2 = pattern[1].y + ry;
	count++;
	segments[count].x1 = pattern[2].x + rx;
	segments[count].y1 = pattern[2].y + ry;
	segments[count].x2 = pattern[3].x + rx;
	segments[count].y2 = pattern[3].y + ry;
	count++;
	if (count >= maxSegments) {
	    XDrawSegments(graphPtr->display, drawable, 	
		  penPtr->symbol.outlineGC, segments, count);
	    count = 0;
	}
    }
    if (count > 0) {
	XDrawSegments(graphPtr->display, drawable, penPtr->symbol.outlineGC, 
		      segments, count);
    }
    Blt_Free(segments);
}

static void
DrawCrossPlusSymbols(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
		     LinePen *penPtr, int size)
{
    TracePoint *p;
    XPoint polygon[13];
    XPoint pattern[13];
    int r;
    int d;			/* Small delta for cross/plus
				 * thickness */
    
    r = (int)ceil(size * S_RATIO * 0.5);
    d = (r / 3);
    /*
     *
     *          2   3       The plus/cross symbol is a closed polygon
     *                      of 12 points. The diagram to the left
     *    0,12  1   4    5  represents the positions of the points
     *           x,y        which are computed below. The extra
     *     11  10   7    6  (thirteenth) point connects the first and
     *                      last points.
     *          9   8
     */
    pattern[0].x = pattern[11].x = pattern[12].x = -r;
    pattern[2].x = pattern[1].x = pattern[10].x = pattern[9].x = -d;
    pattern[3].x = pattern[4].x = pattern[7].x = pattern[8].x = d;
    pattern[5].x = pattern[6].x = r;
    pattern[2].y = pattern[3].y = -r;
    pattern[0].y = pattern[1].y = pattern[4].y = pattern[5].y =
	pattern[12].y = -d;
    pattern[11].y = pattern[10].y = pattern[7].y = pattern[6].y = d;
    pattern[9].y = pattern[8].y = r;
    
    if (penPtr->symbol.type == SYMBOL_CROSS) {
	int i;

	/* For the cross symbol, rotate the points by 45 degrees. */
	for (i = 0; i < 12; i++) {
	    double dx, dy;
	    
	    dx = (double)pattern[i].x * M_SQRT1_2;
	    dy = (double)pattern[i].y * M_SQRT1_2;
	    pattern[i].x = Round(dx - dy);
	    pattern[i].y = Round(dx + dy);
	}
	pattern[12] = pattern[0];
    }
    for (p = tracePtr->head; p != NULL; p = p->next) {
	int rx, ry;
	int i;

	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	rx = Round(p->x);
	ry = Round(p->y);
	for (i = 0; i < 13; i++) {
	    polygon[i].x = pattern[i].x + rx;
	    polygon[i].y = pattern[i].y + ry;
	}
	if (penPtr->symbol.fillGC != NULL) {
	    XFillPolygon(graphPtr->display, drawable, penPtr->symbol.fillGC, 
			 polygon, 13, Complex, CoordModeOrigin);
	}
	if (penPtr->symbol.outlineWidth > 0) {
	    XDrawLines(graphPtr->display, drawable, penPtr->symbol.outlineGC, 
			polygon, 13, CoordModeOrigin);
	}
    }
}

static void
DrawTriangleArrowSymbols(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
			 LinePen *penPtr, int size)
{
    XPoint pattern[4];
    double b;
    int b2, h1, h2;
    TracePoint *p;

#define H_RATIO		1.1663402261671607
#define B_RATIO		1.3467736870885982
#define TAN30		0.57735026918962573
#define COS30		0.86602540378443871
    b = Round(size * B_RATIO * 0.7);
    b2 = Round(b * 0.5);
    h2 = Round(TAN30 * b2);
    h1 = Round(b2 / COS30);
    /*
     *
     *                      The triangle symbol is a closed polygon
     *           0,3         of 3 points. The diagram to the left
     *                      represents the positions of the points
     *           x,y        which are computed below. The extra
     *                      (fourth) point connects the first and
     *      2           1   last points.
     *
     */
    
    if (penPtr->symbol.type == SYMBOL_ARROW) {
	pattern[3].x = pattern[0].x = 0;
	pattern[3].y = pattern[0].y = h1;
	pattern[1].x = b2;
	pattern[2].y = pattern[1].y = -h2;
	pattern[2].x = -b2;
    } else {
	pattern[3].x = pattern[0].x = 0;
	pattern[3].y = pattern[0].y = -h1;
	pattern[1].x = b2;
	pattern[2].y = pattern[1].y = h2;
	pattern[2].x = -b2;
    }
    for (p = tracePtr->head; p != NULL; p = p->next) {
	XPoint polygon[4];
	int rx, ry;
	int i;

	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	rx = Round(p->x);
	ry = Round(p->y);
	for (i = 0; i < 4; i++) {
	    polygon[i].x = pattern[i].x + rx;
	    polygon[i].y = pattern[i].y + ry;
	}
	if (penPtr->symbol.fillGC != NULL) {
	    XFillPolygon(graphPtr->display, drawable, penPtr->symbol.fillGC, 
			 polygon, 4, Convex, CoordModeOrigin);
	}
	if (penPtr->symbol.outlineWidth > 0) {
	    XDrawLines(graphPtr->display, drawable, penPtr->symbol.outlineGC, 
		       polygon, 4, CoordModeOrigin);
	}
    }
}


static void
DrawDiamondSymbols(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
		   LinePen *penPtr, int size)
{
    TracePoint *p;
    XPoint pattern[5];
    int r1;
    /*
     *
     *                      The diamond symbol is a closed polygon
     *            1         of 4 points. The diagram to the left
     *                      represents the positions of the points
     *       0,4 x,y  2     which are computed below. The extra
     *                      (fifth) point connects the first and
     *            3         last points.
     *
     */
    r1 = (int)ceil(size * 0.5);
    pattern[1].y = pattern[0].x = -r1;
    pattern[2].y = pattern[3].x = pattern[0].y = pattern[1].x = 0;
    pattern[3].y = pattern[2].x = r1;
    pattern[4] = pattern[0];
    
    for (p = tracePtr->head; p != NULL; p = p->next) {
	XPoint polygon[5];
	int rx, ry;
	int i;

	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	rx = Round(p->x);
	ry = Round(p->y);
	for (i = 0; i < 5; i++) {
	    polygon[i].x = pattern[i].x + rx;
	    polygon[i].y = pattern[i].y + ry;
	}
	if (penPtr->symbol.fillGC != NULL) {
	    XFillPolygon(graphPtr->display, drawable, penPtr->symbol.fillGC, 
		polygon, 5, Convex, CoordModeOrigin);
	}
	if (penPtr->symbol.outlineWidth > 0) {
	    XDrawLines(graphPtr->display, drawable, penPtr->symbol.outlineGC, 
		polygon, 5, CoordModeOrigin);
	}
    } 
}

static void
DrawImageSymbols(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
		 LinePen *penPtr, int size)
{
    int w, h;
    int dx, dy;
    TracePoint *p;

    Tk_SizeOfImage(penPtr->symbol.image, &w, &h);
    dx = w / 2;
    dy = h / 2;
    for (p = tracePtr->head; p != NULL; p = p->next) {
	int x, y;

	if (!DRAWN(tracePtr, p->flags)) {
#ifdef notdef
	    fprintf(stderr, "not drawn p->x=%g, p->y=%g p->index=%d ", p->x, 
		    p->y, p->index);
	    fprintf(stderr, "drawflags=");
	    DumpFlags(tracePtr->drawFlags);
	    fprintf(stderr, "  ");
	    fprintf(stderr, "p->flags=");
	    DumpFlags(p->flags);
	    fprintf(stderr, "\n");
#endif
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
#ifdef notdef
	    fprintf(stderr, "not playing flags=%x p->x=%d, p->y=%d p->flags=%x\n",
		    tracePtr->drawFlags, p->x, p->y, p->flags);
#endif
	    continue;
	}
#ifdef notdef
	fprintf(stderr, "drawing p->x=%g, p->y=%g p->index=%d ", p->x, p->y,
		p->index);
	fprintf(stderr, "drawflags=");
	DumpFlags(tracePtr->drawFlags);
	fprintf(stderr, "  ");
	fprintf(stderr, "p->flags=");
	DumpFlags(p->flags);
	fprintf(stderr, "\n");
#endif
	x = Round(p->x) - dx;
	y = Round(p->y) - dy;
	Tk_RedrawImage(penPtr->symbol.image, 0, 0, w, h, drawable, x, y);
    }
}

static void
DrawBitmapSymbols(Graph *graphPtr, Drawable drawable, Trace *tracePtr, 
		  LinePen *penPtr, int size)
{
    Pixmap bitmap, mask;
    int w, h, bw, bh;
    double scale, sx, sy;
    int dx, dy;
    TracePoint *p;

    Tk_SizeOfBitmap(graphPtr->display, penPtr->symbol.bitmap, &w, &h);
    mask = None;
    
    /*
     * Compute the size of the scaled bitmap.  Stretch the bitmap to fit
     * a nxn bounding box.
     */
    sx = (double)size / (double)w;
    sy = (double)size / (double)h;
    scale = MIN(sx, sy);
    bw = (int)(w * scale);
    bh = (int)(h * scale);
    
    XSetClipMask(graphPtr->display, penPtr->symbol.outlineGC, None);
    if (penPtr->symbol.mask != None) {
	mask = Blt_ScaleBitmap(graphPtr->tkwin, penPtr->symbol.mask,
			       w, h, bw, bh);
	XSetClipMask(graphPtr->display, penPtr->symbol.outlineGC, mask);
    }
    bitmap = Blt_ScaleBitmap(graphPtr->tkwin, penPtr->symbol.bitmap, w, h, bw, 
			     bh);
    if (penPtr->symbol.fillGC == NULL) {
	XSetClipMask(graphPtr->display, penPtr->symbol.outlineGC, bitmap);
    }
    dx = bw / 2;
    dy = bh / 2;
    for (p = tracePtr->head; p != NULL; p = p->next) {
	int x, y;

	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	x = Round(p->x) - dx;
	y = Round(p->y) - dy;
	if ((penPtr->symbol.fillGC == NULL) || (mask !=None)) {
	    XSetClipOrigin(graphPtr->display, penPtr->symbol.outlineGC, x, y);
	}
	XCopyPlane(graphPtr->display, bitmap, drawable, 
		   penPtr->symbol.outlineGC, 0, 0, bw, bh, x, y, 1);
    }
    Tk_FreePixmap(graphPtr->display, bitmap);
    if (mask != None) {
	Tk_FreePixmap(graphPtr->display, mask);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawSymbols --
 *
 * 	Draw the symbols centered at the each given x,y coordinate in the array
 * 	of points.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Draws a symbol at each coordinate given.  If active, only those
 *	coordinates which are currently active are drawn.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawSymbols(
    Graph *graphPtr,			/* Graph widget record */
    Drawable drawable,			/* Pixmap or window to draw into */
    Trace *tracePtr,
    LinePen *penPtr)
{
    int size;

    if (tracePtr->elemPtr->reqMaxSymbols > 0) {
	TracePoint *p;
	int count;

	/* Mark the symbols that should be displayed. */
	count = 0;
	for (p = tracePtr->head; p != NULL; p = p->next) {
	    if (p->flags & KNOT) {
		if ((count % tracePtr->elemPtr->reqMaxSymbols) == 0) {
		    p->flags |= SYMBOL;
		}
	    }
	    count++;
	}
    }
    tracePtr->drawFlags |= KNOT | VISIBLE;
    if (tracePtr->elemPtr->reqMaxSymbols > 0) {
	tracePtr->drawFlags |= SYMBOL;
    }
    size = tracePtr->symbolSize;
    if (size < 3) {
	if (penPtr->symbol.fillGC != NULL) {
	    DrawPointSymbols(graphPtr, drawable, tracePtr, penPtr);
	}
	return;
    }
    switch (penPtr->symbol.type) {
    case SYMBOL_NONE:
	break;
	
    case SYMBOL_SQUARE:
	DrawSquareSymbols(graphPtr, drawable, tracePtr, penPtr, size);
	break;
	
    case SYMBOL_CIRCLE:
	DrawCircleSymbols(graphPtr, drawable, tracePtr, penPtr, size);
	break;
	
    case SYMBOL_SPLUS:
    case SYMBOL_SCROSS:
	DrawSkinnyCrossPlusSymbols(graphPtr, drawable, tracePtr, penPtr, size);
	break;
	
    case SYMBOL_PLUS:
    case SYMBOL_CROSS:
	DrawCrossPlusSymbols(graphPtr, drawable, tracePtr, penPtr, size);
	break;
	
    case SYMBOL_DIAMOND:
	DrawDiamondSymbols(graphPtr, drawable, tracePtr, penPtr, size);
	break;
	
    case SYMBOL_TRIANGLE:
    case SYMBOL_ARROW:
	DrawTriangleArrowSymbols(graphPtr, drawable, tracePtr, penPtr, size);
	break;
	
    case SYMBOL_IMAGE:
	DrawImageSymbols(graphPtr, drawable, tracePtr, penPtr, size);
	break;
	
    case SYMBOL_BITMAP:
	DrawBitmapSymbols(graphPtr, drawable, tracePtr, penPtr, size);
	break;
    }
    tracePtr->drawFlags &= ~(KNOT | VISIBLE | SYMBOL);
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawTrace --
 *
 *	Draws everything associated with the element's trace. This includes
 *	the polyline, line symbols, errorbars, polygon representing the
 *	area under the curve, and values.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	X drawing commands are output.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawTrace(Graph *graphPtr, Drawable drawable, Trace *tracePtr)
{
    LinePen *penPtr;

    /* Set the pen to use when drawing the trace.  */
    penPtr = tracePtr->penPtr;
    tracePtr->drawFlags = 0;

    /* Draw error bars at knots (original points). */
    if (tracePtr->numSegments > 0) {
	DrawErrorBars(graphPtr, drawable, tracePtr, penPtr);
    }
    /* Draw values at knots (original points). */
    if (penPtr->valueFlags != SHOW_NONE) {
	DrawValues(graphPtr, drawable, tracePtr, penPtr);
    }	
    if (penPtr->traceWidth > 0) {
	DrawPolyline(graphPtr, drawable, tracePtr, penPtr);
    }
    /* Draw symbols at knots (original points). */
    if (penPtr->symbol.type != SYMBOL_NONE) {
	DrawSymbols(graphPtr, drawable, tracePtr, penPtr);
    }
}


static void
DrawActiveTrace(Graph *graphPtr, Drawable drawable, Trace *tracePtr)
{
    LinePen *penPtr;

    /* Set the pen to use when drawing the trace.  */
    penPtr = tracePtr->elemPtr->activePenPtr;
    tracePtr->drawFlags = 0;

    /* Draw error bars at original points. */
    if (tracePtr->numSegments > 0) {
	DrawErrorBars(graphPtr, drawable, tracePtr, penPtr);
    }
    /* Draw values at original points. */
    if (penPtr->valueFlags != SHOW_NONE) {
	DrawValues(graphPtr, drawable, tracePtr, penPtr);
    }	
    if ((tracePtr->elemPtr->numActiveIndices < 0) && 
	(penPtr->traceWidth > 0)) {
	DrawPolyline(graphPtr, drawable, tracePtr, penPtr);
    }
    /* Draw symbols at original points. */
    if (penPtr->symbol.type != SYMBOL_NONE) {
	if (tracePtr->elemPtr->numActiveIndices >= 0) {
	    /* Indicate that we only want to draw active symbols. */
 	    tracePtr->drawFlags |= ACTIVE_POINT;
	}
	DrawSymbols(graphPtr, drawable, tracePtr, penPtr);
	tracePtr->drawFlags &= ~ACTIVE_POINT;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawActiveProc --
 *
 *	Draws the connected line(s) representing the element. If the line is
 *	made up of non-line symbols and the line width parameter has been set
 *	(linewidth > 0), the element will also be drawn as a line (with the
 *	linewidth requested).  The line may consist of separate line segments.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	X drawing commands are output.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawActiveProc(Graph *graphPtr, Drawable drawable, Element *basePtr)
{
    LineElement *elemPtr = (LineElement *)basePtr;
    Blt_ChainLink link;

    if ((elemPtr->flags & ACTIVE_PENDING) && (elemPtr->numActiveIndices >= 0)) {
	MapActiveSymbols(elemPtr);
    }
    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;

	tracePtr = Blt_Chain_GetValue(link);
	DrawActiveTrace(graphPtr, drawable, tracePtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawNormalProc --
 *
 *	Draws the connected line(s) representing the element. If the line is
 *	made up of non-line symbols and the line width parameter has been set
 *	(linewidth > 0), the element will also be drawn as a line (with the
 *	linewidth requested).  The line may consist of separate line segments.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	X drawing commands are output.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawNormalProc(Graph *graphPtr, Drawable drawable, Element *basePtr)
{
    LineElement *elemPtr = (LineElement *)basePtr;
    Blt_ChainLink link;

    /* Fill area under curve. Only for non-active elements. */
    DrawAreaUnderCurve(graphPtr, drawable, elemPtr);
    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;

	tracePtr = Blt_Chain_GetValue(link);
	tracePtr->drawFlags = 0;
	DrawTrace(graphPtr, drawable, tracePtr);
    }
}


static void
SetLineAttributes(Blt_Ps ps, LinePen *penPtr)
{
    /* Set the attributes of the line (color, dashes, linewidth) */
    Blt_Ps_XSetLineAttributes(ps, penPtr->traceColor,
	penPtr->traceWidth, &penPtr->traceDashes, CapButt, JoinMiter);
    if ((LineIsDashed(penPtr->traceDashes)) && 
	(penPtr->traceOffColor != NULL)) {
	Blt_Ps_Append(ps, "/DashesProc {\n  gsave\n    ");
	Blt_Ps_XSetBackground(ps, penPtr->traceOffColor);
	Blt_Ps_Append(ps, "    ");
	Blt_Ps_XSetDashes(ps, (Blt_Dashes *)NULL);
	Blt_Ps_Append(ps, "stroke\n  grestore\n} def\n");
    } else {
	Blt_Ps_Append(ps, "/DashesProc {} def\n");
    }
}

static void 
ErrorBarsToPostScript(Blt_Ps ps, Trace *tracePtr, LinePen *penPtr)
{
    Graph *graphPtr = tracePtr->elemPtr->obj.graphPtr;
    TraceSegment *s;

    SetLineAttributes(ps, penPtr);
    Blt_Ps_Append(ps, "% start segments\n");
    Blt_Ps_Append(ps, "newpath\n");
    for (s = tracePtr->segments; s != NULL; s = s->next) {
	if (!DRAWN(tracePtr, s->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, s->index)) {
	    continue;
	}
	Blt_Ps_Format(ps, "  %g %g moveto %g %g lineto\n", 
		s->x1, s->y1, s->x2, s->y2);
	Blt_Ps_Append(ps, "DashesProc stroke\n");
    }
    Blt_Ps_Append(ps, "% end segments\n");
}

static void
ValuesToPostScript(Blt_Ps ps, Trace *tracePtr, LinePen *penPtr)
{
    Graph *graphPtr = tracePtr->elemPtr->obj.graphPtr;
    TracePoint *p;
    const char *fmt;
    
    fmt = penPtr->valueFormat;
    if (fmt == NULL) {
	fmt = "%g";
    }
    for (p = tracePtr->head; p != NULL; p = p->next) {
	double x, y;
	char string[TCL_DOUBLE_SPACE * 2 + 2];

	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	x = tracePtr->elemPtr->x.values[p->index];
	y = tracePtr->elemPtr->y.values[p->index];
	if (penPtr->valueFlags == SHOW_X) {
	    Blt_FormatString(string, TCL_DOUBLE_SPACE, fmt, x); 
	} else if (penPtr->valueFlags == SHOW_Y) {
	    Blt_FormatString(string, TCL_DOUBLE_SPACE, fmt, y); 
	} else if (penPtr->valueFlags == SHOW_BOTH) {
	    Blt_FormatString(string, TCL_DOUBLE_SPACE, fmt, x);
	    strcat(string, ",");
	    Blt_FormatString(string + strlen(string), TCL_DOUBLE_SPACE, fmt, y);
	}
	Blt_Ps_DrawText(ps, string, &penPtr->valueStyle, x, y);
    }
}

static void
PolylineToPostScript(Blt_Ps ps, Trace *tracePtr, LinePen *penPtr)
{
    Graph *graphPtr = tracePtr->elemPtr->obj.graphPtr;
    Point2d *points;
    TracePoint *p;
    int count;

    SetLineAttributes(ps, penPtr);
    points = Blt_AssertMalloc(tracePtr->numPoints * sizeof(Point2d));
    count = 0;
    for (p = tracePtr->head; p != NULL; p = p->next) {
	if (!PLAYING(graphPtr, p->index)) {
	    continue;
	}
	points[count].x = p->x;
	points[count].y = p->y;
	count++;
    }
    Blt_Ps_Append(ps, "% start trace\n");
    Blt_Ps_DrawPolyline(ps, count, points);
    Blt_Ps_Append(ps, "% end trace\n");
    Blt_Free(points);
}


/* 
 * AreaUnderCurveToPostScript --
 *
 *	Draws the polygons under the traces.
 */
static void
AreaUnderCurveToPostScript(Blt_Ps ps, LineElement *elemPtr)
{
    Blt_ChainLink link;

    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL; 
	 link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;

	tracePtr = Blt_Chain_GetValue(link);
	if ((tracePtr->numFillPts > 0) && (elemPtr->fillBg != NULL)) {
	    /* Create a path to use for both the polygon and its outline. */
	    Blt_Ps_Append(ps, "% start fill area\n");
	    Blt_Ps_Polyline(ps, tracePtr->numFillPts, tracePtr->fillPts);
	    /* If the background fill color was specified, draw the polygon in a
	     * solid fashion with that color.  */
	    if (tracePtr->elemPtr->fillBgColor != NULL) {
		Blt_Ps_XSetBackground(ps, tracePtr->elemPtr->fillBgColor);
		Blt_Ps_Append(ps, "gsave fill grestore\n");
	    }
	    Blt_Ps_XSetForeground(ps, tracePtr->elemPtr->fillFgColor);
	    if (tracePtr->elemPtr->fillBg != NULL) {
		Blt_Ps_Append(ps, "gsave fill grestore\n");
		/* TBA: Transparent tiling is the hard part. */
	    } else {
		Blt_Ps_Append(ps, "gsave fill grestore\n");
	    }
	    Blt_Ps_Append(ps, "% end fill area\n");
	}
    }
}


/*
 *---------------------------------------------------------------------------
 *
 * GetSymbolPostScriptInfo --
 *
 *	Set up the PostScript environment with the macros and attributes needed
 *	to draw the symbols of the element.
 *
 * Results:
 *	None.
 *
 *---------------------------------------------------------------------------
 */
static void
GetSymbolPostScriptInfo(Blt_Ps ps, LineElement *elemPtr, LinePen *penPtr, 
			int size)
{
    XColor *outlineColor, *fillColor, *defaultColor;

    /* Set line and foreground attributes */
    outlineColor = penPtr->symbol.outlineColor;
    fillColor    = penPtr->symbol.fillColor;
    defaultColor = penPtr->traceColor;

    if (fillColor == COLOR_DEFAULT) {
	fillColor = defaultColor;
    }
    if (outlineColor == COLOR_DEFAULT) {
	outlineColor = defaultColor;
    }
    if (penPtr->symbol.type == SYMBOL_NONE) {
	Blt_Ps_XSetLineAttributes(ps, defaultColor, penPtr->traceWidth + 2,
		 &penPtr->traceDashes, CapButt, JoinMiter);
    } else {
	Blt_Ps_XSetLineWidth(ps, penPtr->symbol.outlineWidth);
	Blt_Ps_XSetDashes(ps, (Blt_Dashes *)NULL);
    }

    /*
     * Build a PostScript procedure to draw the symbols.  For bitmaps, paint
     * both the bitmap and its mask. Otherwise fill and stroke the path formed
     * already.
     */
    Blt_Ps_Append(ps, "\n/DrawSymbolProc {\n");
    switch (penPtr->symbol.type) {
    case SYMBOL_NONE:
	break;				/* Do nothing */
    case SYMBOL_BITMAP:
	{
	    int w, h;
	    double sx, sy, scale;
	    Graph *graphPtr = elemPtr->obj.graphPtr;

	    /*
	     * Compute how much to scale the bitmap.  Don't let the scaled
	     * bitmap exceed the bounding square for the symbol.
	     */
	    Tk_SizeOfBitmap(graphPtr->display, penPtr->symbol.bitmap, &w, &h);
	    sx = (double)size / (double)w;
	    sy = (double)size / (double)h;
	    scale = MIN(sx, sy);

	    if ((penPtr->symbol.mask != None) && (fillColor != NULL)) {
		Blt_Ps_VarAppend(ps, "\n  % Bitmap mask is \"",
		    Tk_NameOfBitmap(graphPtr->display, penPtr->symbol.mask),
		    "\"\n\n  ", (char *)NULL);
		Blt_Ps_XSetBackground(ps, fillColor);
		Blt_Ps_DrawBitmap(ps, graphPtr->display, penPtr->symbol.mask, 
			scale, scale);
	    }
	    Blt_Ps_VarAppend(ps, "\n  % Bitmap symbol is \"",
		Tk_NameOfBitmap(graphPtr->display, penPtr->symbol.bitmap),
		"\"\n\n  ", (char *)NULL);
	    Blt_Ps_XSetForeground(ps, outlineColor);
	    Blt_Ps_DrawBitmap(ps, graphPtr->display, penPtr->symbol.bitmap, 
		scale, scale);
	}
	break;
    default:
	if (fillColor != NULL) {
	    Blt_Ps_Append(ps, "  ");
	    Blt_Ps_XSetBackground(ps, fillColor);
	    Blt_Ps_Append(ps, "  gsave fill grestore\n");
	}
	if ((outlineColor != NULL) && (penPtr->symbol.outlineWidth > 0)) {
	    Blt_Ps_Append(ps, "  ");
	    Blt_Ps_XSetForeground(ps, outlineColor);
	    Blt_Ps_Append(ps, "  stroke\n");
	}
	break;
    }
    Blt_Ps_Append(ps, "} def\n\n");
}

/*
 *---------------------------------------------------------------------------
 *
 * SymbolsToPostScript --
 *
 * 	Draw a symbol centered at the given x,y window coordinate based upon
 * 	the element symbol type and size.
 *
 * Results:
 *	None.
 *
 * Problems:
 *	Most notable is the round-off errors generated when calculating the
 *	centered position of the symbol.
 *
 *---------------------------------------------------------------------------
 */
static void
SymbolsToPostScript(Blt_Ps ps, Trace *tracePtr, LinePen *penPtr)
{
    TracePoint *p;
    double size;
    static const char *symbolMacros[] =
    {
	"Li", "Sq", "Ci", "Di", "Pl", "Cr", "Sp", "Sc", "Tr", "Ar", "Bm", 
	(char *)NULL,
    };

    GetSymbolPostScriptInfo(ps, tracePtr->elemPtr, penPtr, 
	    tracePtr->symbolSize);
    size = (double)tracePtr->symbolSize;
    switch (penPtr->symbol.type) {
    case SYMBOL_SQUARE:
    case SYMBOL_CROSS:
    case SYMBOL_PLUS:
    case SYMBOL_SCROSS:
    case SYMBOL_SPLUS:
	size = (double)Round(size * S_RATIO);
	break;
    case SYMBOL_TRIANGLE:
    case SYMBOL_ARROW:
	size = (double)Round(size * 0.7);
	break;
    case SYMBOL_DIAMOND:
	size = (double)Round(size * M_SQRT1_2);
	break;

    default:
	break;
    }
    tracePtr->drawFlags |= KNOT;
    if (tracePtr->elemPtr->reqMaxSymbols > 0) {
	tracePtr->drawFlags |= SYMBOL;
    }
    for (p = tracePtr->head; p != NULL; p = p->next) {
	if (!DRAWN(tracePtr, p->flags)) {
	    continue;
	}
	Blt_Ps_Format(ps, "%g %g %g %s\n", p->x, p->y, size, 
		symbolMacros[penPtr->symbol.type]);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * SymbolToPostScriptProc --
 *
 * 	Draw a symbol centered at the given x,y window coordinate based upon
 * 	the element symbol type and size.
 *
 * Results:
 *	None.
 *
 * Problems:
 *	Most notable is the round-off errors generated when calculating the
 *	centered position of the symbol.
 *
 *---------------------------------------------------------------------------
 */
static void
SymbolToPostScriptProc(
    Graph *graphPtr,			/* Graph widget record */
    Blt_Ps ps,
    Element *basePtr,			/* Line element information */
    double x, double y,			/* Center position of symbol */
    int size)				/* Size of element */
{
    LineElement *elemPtr = (LineElement *)basePtr;
    LinePen *penPtr;
    double symbolSize;
    static const char *symbolMacros[] =
    {
	"Li", "Sq", "Ci", "Di", "Pl", "Cr", "Sp", "Sc", "Tr", "Ar", "Bm", 
	(char *)NULL,
    };

    penPtr = NORMALPEN(elemPtr);
    GetSymbolPostScriptInfo(ps, elemPtr, penPtr, size);

    symbolSize = (double)size;
    switch (penPtr->symbol.type) {
    case SYMBOL_SQUARE:
    case SYMBOL_CROSS:
    case SYMBOL_PLUS:
    case SYMBOL_SCROSS:
    case SYMBOL_SPLUS:
	symbolSize = (double)Round(size * S_RATIO);
	break;
    case SYMBOL_TRIANGLE:
    case SYMBOL_ARROW:
	symbolSize = (double)Round(size * 0.7);
	break;
    case SYMBOL_DIAMOND:
	symbolSize = (double)Round(size * M_SQRT1_2);
	break;

    default:
	break;
    }

    Blt_Ps_Format(ps, "%g %g %g %s\n", x, y, symbolSize, 
		  symbolMacros[penPtr->symbol.type]);
}

/*
 *---------------------------------------------------------------------------
 *
 * TraceToPostScript --
 *
 *	Draws everything associated with the element's trace. This includes
 *	the polyline, line symbols, errorbars, polygon representing the
 *	area under the curve, and values.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	X drawing commands are output.
 *
 *---------------------------------------------------------------------------
 */
static void
TraceToPostScript(Blt_Ps ps, Trace *tracePtr)
{
    LinePen *penPtr;

    /* Set the pen to use when drawing the trace.  */
    penPtr = tracePtr->penPtr;

    /* Draw error bars at original points. */
    if (tracePtr->numSegments > 0) {
	ErrorBarsToPostScript(ps, tracePtr, penPtr);
    }
    /* Draw values at original points. */
    if (penPtr->valueFlags != SHOW_NONE) {
	ValuesToPostScript(ps, tracePtr, penPtr);
    }	
    /* Polyline for the trace. */
    if (penPtr->traceWidth > 0) {
	PolylineToPostScript(ps, tracePtr, penPtr);
    }
    /* Draw symbols at original points. */
    if (penPtr->symbol.type != SYMBOL_NONE) {
	SymbolsToPostScript(ps, tracePtr, penPtr);
    }
}


/*
 *---------------------------------------------------------------------------
 *
 * NormalToPostScriptProc --
 *
 *	Similar to the DrawLine procedure, prints PostScript related commands to
 *	form the connected line(s) representing the element.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	PostScript pen width, dashes, and color settings are changed.
 *
 *---------------------------------------------------------------------------
 */
static void
NormalToPostScriptProc(Graph *graphPtr, Blt_Ps ps, Element *basePtr)
{
    LineElement *elemPtr = (LineElement *)basePtr;
    Blt_ChainLink link;

    AreaUnderCurveToPostScript(ps, elemPtr);
    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;

	tracePtr = Blt_Chain_GetValue(link);
	tracePtr->drawFlags = 0;
	TraceToPostScript(ps, tracePtr);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ActiveTraceToPostScript --
 *
 *	Draws everything associated with the element's trace. This includes
 *	the polyline, line symbols, errorbars, polygon representing the
 *	area under the curve, and values.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	X drawing commands are output.
 *
 *---------------------------------------------------------------------------
 */
static void
ActiveTraceToPostScript(Blt_Ps ps, Trace *tracePtr)
{
    LinePen *penPtr;

    /* Set the pen to use when drawing the trace.  */
    penPtr = tracePtr->elemPtr->activePenPtr;

    /* Draw error bars at original points. */
    if (tracePtr->numSegments > 0) {
	ErrorBarsToPostScript(ps, tracePtr, penPtr);
    }
    /* Draw values at original points. */
    if (penPtr->valueFlags != SHOW_NONE) {
	ValuesToPostScript(ps, tracePtr, penPtr);
    }	
    if ((tracePtr->elemPtr->numActiveIndices < 0) && (penPtr->traceWidth > 0)) {
	PolylineToPostScript(ps, tracePtr, penPtr);
    }
    /* Draw symbols at original points. */
    if (penPtr->symbol.type != SYMBOL_NONE) {
	if (tracePtr->elemPtr->numActiveIndices >= 0) {
	    /* Indicate that we only want to draw active symbols. */
	    tracePtr->drawFlags |= ACTIVE_POINT;
	}
	SymbolsToPostScript(ps, tracePtr, penPtr);
	tracePtr->drawFlags &= ~ACTIVE_POINT;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * ActiveToPostScriptProc --
 *
 *	Similar to the DrawLine procedure, prints PostScript related commands to
 *	form the connected line(s) representing the element.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	PostScript pen width, dashes, and color settings are changed.
 *
 *---------------------------------------------------------------------------
 */
static void
ActiveToPostScriptProc(Graph *graphPtr, Blt_Ps ps, Element *basePtr)
{
    LineElement *elemPtr = (LineElement *)basePtr;
    Blt_ChainLink link;

    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL;
	link = Blt_Chain_NextLink(link)) {
	Trace *tracePtr;

	tracePtr = Blt_Chain_GetValue(link);
	tracePtr->drawFlags = 0;
	ActiveTraceToPostScript(ps, tracePtr);
    }
}


#ifdef WIN32
/* 
 * DrawCircleSymbol --
 *
 *	Draws the symbols of the trace as circles.  The outlines of circles
 *	are drawn after circles are filled.  This is speed tradeoff: drawn
 *	many circles at once, or drawn one symbol at a time.
 *
 *	Symbols are only drawn at the knots of the trace (i.e. original points,
 *	not generated).  The "play" function can limit what circles are drawn.
 *
 */
static void
DrawCircleSymbol(Graph *graphPtr, Drawable drawable, LinePen *penPtr, 
		 int x, int y, int size)
{
    HBRUSH brush, oldBrush;
    HPEN pen, oldPen;
    HDC dc;
    TkWinDCState state;
    int r;

    r = (int)ceil(size * 0.5);
    if (drawable == None) {
	return;				/* Huh? */
    }
    if ((penPtr->symbol.fillGC == NULL) && 
	(penPtr->symbol.outlineWidth == 0)) {
	return;
    }
    dc = TkWinGetDrawableDC(graphPtr->display, drawable, &state);
    /* SetROP2(dc, tkpWinRopModes[penPtr->symbol.fillGC->function]); */
    if (penPtr->symbol.fillGC != NULL) {
	brush = CreateSolidBrush(penPtr->symbol.fillGC->foreground);
    } else {
	brush = GetStockBrush(NULL_BRUSH);
    }
    if (penPtr->symbol.outlineWidth > 0) {
	pen = Blt_GCToPen(dc, penPtr->symbol.outlineGC);
    } else {
	pen = GetStockPen(NULL_PEN);
    }
    oldPen = SelectPen(dc, pen);
    oldBrush = SelectBrush(dc, brush);
    Ellipse(dc, x - r, y - r, x + r + 1, y + r + 1);
    DeleteBrush(SelectBrush(dc, oldBrush));
    DeletePen(SelectPen(dc, oldPen));
    TkWinReleaseDrawableDC(drawable, dc, &state);
}

#else

/* 
 * DrawCircleSymbol --
 *
 *	Draws the symbols of the trace as circles.  The outlines of circles
 *	are drawn after circles are filled.  This is speed tradeoff: draw
 *	many circles at once, or drawn one symbol at a time.
 *
 *	Symbols are only drawn at the knots of the trace (i.e. original points,
 *	not generated).  The "play" function can limit what circles are drawn.
 *
 */
static void
DrawCircleSymbol(Graph *graphPtr, Drawable drawable, LinePen *penPtr, 
		 int x, int y, int size)
{
    int r, s;

    r = (int)ceil(size * 0.5);
    s = r + r;
    if (penPtr->symbol.fillGC != NULL) {
	XFillArc(graphPtr->display, drawable, penPtr->symbol.fillGC, 
		  x - r, y - r,  s,  s, 0, 23040);
    }
    if (penPtr->symbol.outlineWidth > 0) {
	XDrawArc(graphPtr->display, drawable, penPtr->symbol.outlineGC, 
		  x - r, y - r,  s,  s, 0, 23040);
    }
}

#endif	/* WIN32 */

/* 
 * DrawSquareSymbol --
 *
 *	Draws the symbols of the trace as squares.  The outlines of squares
 *	are drawn after squares are filled.  This is speed tradeoff: draw
 *	many squares at once, or drawn one symbol at a time.
 *
 *	Symbols are only drawn at the knots of the trace (i.e. original points,
 *	not generated).  The "play" function can limit what squares are drawn.
 *
 */
static void
DrawSquareSymbol(Graph *graphPtr, Drawable drawable, LinePen *penPtr, 
		 int x, int y, int size)
{
    int r, s;

    r = (int)ceil(size * S_RATIO * 0.5);
    s = r + r;
    if (penPtr->symbol.fillGC != NULL) {
	XFillRectangle(graphPtr->display, drawable, penPtr->symbol.fillGC, 
			x - r, y - r,  s, s);
    }
    if (penPtr->symbol.outlineWidth > 0) {
	XDrawRectangle(graphPtr->display, drawable, penPtr->symbol.outlineGC, 
			x - r, y - r,  s, s);
    }
}

/* 
 * DrawSkinnyCrossPlusSymbol --
 *
 *	Draws the symbols of the trace as single line crosses or pluses.  
 *
 *	Symbols are only drawn at the knots of the trace (i.e. original points,
 *	not generated).  The "play" function can limit what symbols are drawn.
 */
static void
DrawSkinnyCrossPlusSymbol(Graph *graphPtr, Drawable drawable, LinePen *penPtr, 
			  int x, int y, int size)
{
    XPoint pattern[13];			/* Template for polygon symbols */
    XSegment segments[2];
    int r;

    r = (int)ceil(size * 0.5);
    if (penPtr->symbol.type == SYMBOL_SCROSS) {
	r = Round((double)r * M_SQRT1_2);
	pattern[3].y = pattern[2].x = pattern[0].x = pattern[0].y = -r;
	pattern[3].x = pattern[2].y = pattern[1].y = pattern[1].x = r;
    } else {
	pattern[0].y = pattern[1].y = pattern[2].x = pattern[3].x = 0;
	pattern[0].x = pattern[2].y = -r;
	pattern[1].x = pattern[3].y = r;
    }
    segments[0].x1 = pattern[0].x + x;
    segments[0].y1 = pattern[0].y + y;
    segments[0].x2 = pattern[1].x + x;
    segments[0].y2 = pattern[1].y + y;
    segments[1].x1 = pattern[2].x + x;
    segments[1].y1 = pattern[2].y + y;
    segments[1].x2 = pattern[3].x + x;
    segments[1].y2 = pattern[3].y + y;
    XDrawSegments(graphPtr->display, drawable, penPtr->symbol.outlineGC, 
	segments, 2);
}

static void
DrawCrossPlusSymbol(Graph *graphPtr, Drawable drawable, LinePen *penPtr, 
		    int x, int y, int size)
{
    XPoint polygon[13];
    int r;
    int d;			/* Small delta for cross/plus
				 * thickness */
    int i;

    r = (int)ceil(size * S_RATIO * 0.5);
    d = (r / 3);
    /*
     *
     *          2   3       The plus/cross symbol is a closed polygon
     *                      of 12 points. The diagram to the left
     *    0,12  1   4    5  represents the positions of the points
     *           x,y        which are computed below. The extra
     *     11  10   7    6  (thirteenth) point connects the first and
     *                      last points.
     *          9   8
     */
    polygon[0].x = polygon[11].x = polygon[12].x = -r;
    polygon[2].x = polygon[1].x = polygon[10].x = polygon[9].x = -d;
    polygon[3].x = polygon[4].x = polygon[7].x = polygon[8].x = d;
    polygon[5].x = polygon[6].x = r;
    polygon[2].y = polygon[3].y = -r;
    polygon[0].y = polygon[1].y = polygon[4].y = polygon[5].y =
	polygon[12].y = -d;
    polygon[11].y = polygon[10].y = polygon[7].y = polygon[6].y = d;
    polygon[9].y = polygon[8].y = r;
    
    if (penPtr->symbol.type == SYMBOL_CROSS) {
	int i;

	/* For the cross symbol, rotate the points by 45 degrees. */
	for (i = 0; i < 12; i++) {
	    double dx, dy;
	    
	    dx = (double)polygon[i].x * M_SQRT1_2;
	    dy = (double)polygon[i].y * M_SQRT1_2;
	    polygon[i].x = Round(dx - dy);
	    polygon[i].y = Round(dx + dy);
	}
	polygon[12] = polygon[0];
    }
    for (i = 0; i < 13; i++) {
	polygon[i].x += x;
	polygon[i].y += y;
    }
    if (penPtr->symbol.fillGC != NULL) {
	XFillPolygon(graphPtr->display, drawable, penPtr->symbol.fillGC, 
	     polygon, 13, Complex, CoordModeOrigin);
    }
    if (penPtr->symbol.outlineWidth > 0) {
	XDrawLines(graphPtr->display, drawable, penPtr->symbol.outlineGC, 
	   polygon, 13, CoordModeOrigin);
    }
}

static void
DrawTriangleArrowSymbol(Graph *graphPtr, Drawable drawable, LinePen *penPtr, 
			int x, int y, int size)
{
    XPoint polygon[4];
    double b;
    int b2, h1, h2;
    int i;

#define H_RATIO		1.1663402261671607
#define B_RATIO		1.3467736870885982
#define TAN30		0.57735026918962573
#define COS30		0.86602540378443871
    b = Round(size * B_RATIO * 0.7);
    b2 = Round(b * 0.5);
    h2 = Round(TAN30 * b2);
    h1 = Round(b2 / COS30);
    /*
     *
     *                      The triangle symbol is a closed polygon
     *           0,3         of 3 points. The diagram to the left
     *                      represents the positions of the points
     *           x,y        which are computed below. The extra
     *                      (fourth) point connects the first and
     *      2           1   last points.
     *
     */
    
    if (penPtr->symbol.type == SYMBOL_ARROW) {
	polygon[3].x = polygon[0].x = 0;
	polygon[3].y = polygon[0].y = h1;
	polygon[1].x = b2;
	polygon[2].y = polygon[1].y = -h2;
	polygon[2].x = -b2;
    } else {
	polygon[3].x = polygon[0].x = 0;
	polygon[3].y = polygon[0].y = -h1;
	polygon[1].x = b2;
	polygon[2].y = polygon[1].y = h2;
	polygon[2].x = -b2;
    }
    for (i = 0; i < 4; i++) {
	polygon[i].x += x;
	polygon[i].y += y;
    }
    if (penPtr->symbol.fillGC != NULL) {
	XFillPolygon(graphPtr->display, drawable, penPtr->symbol.fillGC, 
		     polygon, 4, Convex, CoordModeOrigin);
    }
    if (penPtr->symbol.outlineWidth > 0) {
	XDrawLines(graphPtr->display, drawable, penPtr->symbol.outlineGC, 
		   polygon, 4, CoordModeOrigin);
    }
}

static void
DrawDiamondSymbol(Graph *graphPtr, Drawable drawable, LinePen *penPtr, 
		  int x, int y, int size)
{
    XPoint polygon[5];
    int r;
    int i;

    /*
     *
     *                      The diamond symbol is a closed polygon
     *            1         of 4 points. The diagram to the left
     *                      represents the positions of the points
     *       0,4 x,y  2     which are computed below. The extra
     *                      (fifth) point connects the first and
     *            3         last points.
     *
     */
    r = (int)ceil(size * 0.5);
    polygon[1].y = polygon[0].x = -r;
    polygon[2].y = polygon[3].x = polygon[0].y = polygon[1].x = 0;
    polygon[3].y = polygon[2].x = r;
    polygon[4] = polygon[0];
    
    for (i = 0; i < 5; i++) {
	polygon[i].x += x;
	polygon[i].y += y;
    }
    if (penPtr->symbol.fillGC != NULL) {
	XFillPolygon(graphPtr->display, drawable, penPtr->symbol.fillGC, 
		     polygon, 5, Convex, CoordModeOrigin);
    }
    if (penPtr->symbol.outlineWidth > 0) {
	XDrawLines(graphPtr->display, drawable, penPtr->symbol.outlineGC, 
		   polygon, 5, CoordModeOrigin);
    }
}

static void
DrawImageSymbol(Graph *graphPtr, Drawable drawable, LinePen *penPtr, 
		int x, int y, int size)
{
    int w, h;
    int dx, dy;

    Tk_SizeOfImage(penPtr->symbol.image, &w, &h);
    dx = w / 2;
    dy = h / 2;
    x = x - dx;
    y = y - dy;
    Tk_RedrawImage(penPtr->symbol.image, 0, 0, w, h, drawable, x, y);
}

static void
DrawBitmapSymbol(Graph *graphPtr, Drawable drawable, LinePen *penPtr, 
		 int x, int y, int size)
{
    Pixmap bitmap, mask;
    int w, h, bw, bh;
    double scale, sx, sy;
    int dx, dy;

    Tk_SizeOfBitmap(graphPtr->display, penPtr->symbol.bitmap, &w, &h);
    mask = None;
    
    /*
     * Compute the size of the scaled bitmap.  Stretch the bitmap to fit
     * a nxn bounding box.
     */
    sx = (double)size / (double)w;
    sy = (double)size / (double)h;
    scale = MIN(sx, sy);
    bw = (int)(w * scale);
    bh = (int)(h * scale);
    
    XSetClipMask(graphPtr->display, penPtr->symbol.outlineGC, None);
    if (penPtr->symbol.mask != None) {
	mask = Blt_ScaleBitmap(graphPtr->tkwin, penPtr->symbol.mask, w, h, 
		bw, bh);
	XSetClipMask(graphPtr->display, penPtr->symbol.outlineGC, mask);
    }
    bitmap = Blt_ScaleBitmap(graphPtr->tkwin, penPtr->symbol.bitmap, w, h, bw, 
			     bh);
    if (penPtr->symbol.fillGC == NULL) {
	XSetClipMask(graphPtr->display, penPtr->symbol.outlineGC, bitmap);
    }
    dx = bw / 2;
    dy = bh / 2;
    x = x - dx;
    y = y - dy;
    if ((penPtr->symbol.fillGC == NULL) || (mask !=None)) {
	XSetClipOrigin(graphPtr->display, penPtr->symbol.outlineGC, x, y);
    }
    XCopyPlane(graphPtr->display, bitmap, drawable, penPtr->symbol.outlineGC, 
	0, 0, bw, bh, x, y, 1);
    Tk_FreePixmap(graphPtr->display, bitmap);
    if (mask != None) {
	Tk_FreePixmap(graphPtr->display, mask);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawSymbol --
 *
 * 	Draw the symbols centered at the each given x,y coordinate in the
 * 	array of points.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Draws a symbol at each coordinate given.  If active, only those
 *	coordinates which are currently active are drawn.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawSymbol(
    Graph *graphPtr,			/* Graph widget record */
    Drawable drawable,			/* Pixmap or window to draw into */
    LinePen *penPtr, 
    int x, int y, int size)
{
    switch (penPtr->symbol.type) {
    case SYMBOL_NONE:
	break;
	
    case SYMBOL_SQUARE:
	DrawSquareSymbol(graphPtr, drawable, penPtr, x, y, size);
	break;
	
    case SYMBOL_CIRCLE:
	DrawCircleSymbol(graphPtr, drawable, penPtr, x, y, size);
	break;
	
    case SYMBOL_SPLUS:
    case SYMBOL_SCROSS:
	DrawSkinnyCrossPlusSymbol(graphPtr, drawable, penPtr, x, y, size);
	break;
	
    case SYMBOL_PLUS:
    case SYMBOL_CROSS:
	DrawCrossPlusSymbol(graphPtr, drawable, penPtr, x, y, size);
	break;
	
    case SYMBOL_DIAMOND:
	DrawDiamondSymbol(graphPtr, drawable, penPtr, x, y, size);
	break;
	
    case SYMBOL_TRIANGLE:
    case SYMBOL_ARROW:
	DrawTriangleArrowSymbol(graphPtr, drawable, penPtr, x, y, size);
	break;
	
    case SYMBOL_IMAGE:
	DrawImageSymbol(graphPtr, drawable, penPtr, x, y, size);
	break;
	
    case SYMBOL_BITMAP:
	DrawBitmapSymbol(graphPtr, drawable, penPtr, x, y, size);
	break;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DrawSymbolProc --
 *
 * 	Draw the symbol centered at the each given x,y coordinate.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Draws a symbol at the coordinate given.
 *
 *---------------------------------------------------------------------------
 */
static void
DrawSymbolProc(
    Graph *graphPtr,			/* Graph widget record */
    Drawable drawable,			/* Pixmap or window to draw into */
    Element *basePtr,			/* Line element information */
    int x, int y,			/* Center position of symbol */
    int size)				/* Size of symbol. */
{
    LineElement *elemPtr = (LineElement *)basePtr;
    LinePen *penPtr;

    penPtr = NORMALPEN(elemPtr);
    if (penPtr->traceWidth > 0) {
	/*
	 * Draw an extra line offset by one pixel from the previous to give a
	 * thicker appearance.  This is only for the legend entry.  This
	 * routine is never called for drawing the actual line segments.
	 */
	XDrawLine(graphPtr->display, drawable, penPtr->traceGC, 
		  x - size, y, x + size, y);
	XDrawLine(graphPtr->display, drawable, penPtr->traceGC, 
		  x - size, y + 1, x + size, y + 1);
    }
    if (penPtr->symbol.type != SYMBOL_NONE) {
	DrawSymbol(graphPtr, drawable, penPtr, x, y, size);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * DestroyProc --
 *
 *	Release memory and resources allocated for the line element.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Everything associated with the line element is freed up.
 *
 *---------------------------------------------------------------------------
 */
static void
DestroyProc(Graph *graphPtr, Element *basePtr)
{
    LineElement *elemPtr = (LineElement *)basePtr;
    Blt_ChainLink link, next;

    DestroyPenProc(graphPtr, (Pen *)&elemPtr->builtinPen);
    if (elemPtr->activePenPtr != NULL) {
	Blt_FreePen((Pen *)elemPtr->activePenPtr);
    }
    if (elemPtr->styles != NULL) {
	Blt_FreeStyles(elemPtr->styles);
	Blt_Chain_Destroy(elemPtr->styles);
    }
    if (elemPtr->pointPool != NULL) {
	Blt_Pool_Destroy(elemPtr->pointPool);
    }
    if (elemPtr->segmentPool != NULL) {
	Blt_Pool_Destroy(elemPtr->segmentPool);
    }
    for (link = Blt_Chain_FirstLink(elemPtr->traces); link != NULL; 
	 link = next) {
	Trace *tracePtr;

	next = Blt_Chain_NextLink(link);
	tracePtr = Blt_Chain_GetValue(link);
	FreeTrace(elemPtr->traces, tracePtr);
    }
    if (elemPtr->fillGC != NULL) {
	Tk_FreeGC(graphPtr->display, elemPtr->fillGC);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * Blt_LineElement --
 *
 *	Allocate memory and initialize methods for the new line element.
 *
 * Results:
 *	The pointer to the newly allocated element structure is returned.
 *
 * Side effects:
 *	Memory is allocated for the line element structure.
 *
 *---------------------------------------------------------------------------
 */

static ElementProcs lineProcs =
{
    ClosestProc,			/* Finds the closest element/data
					 * point */
    ConfigureProc,			/* Configures the element. */
    DestroyProc,			/* Destroys the element. */
    DrawActiveProc,			/* Draws active element */
    DrawNormalProc,			/* Draws normal element */
    DrawSymbolProc,			/* Draws the element symbol. */
    ExtentsProc,			/* Find the extents of the element's
					 * data. */
    FindProc,				/* Find the points withi the search
					 * radius. */
    ActiveToPostScriptProc,		/* Prints active element. */
    NormalToPostScriptProc,		/* Prints normal element. */
    SymbolToPostScriptProc,		/* Prints the line's symbol. */
    MapProc				/* Compute element's screen
					 * coordinates. */
};

Element *
Blt_LineElement2(Graph *graphPtr, ClassId id, Blt_HashEntry *hPtr)
{
    LineElement *elemPtr;

    elemPtr = Blt_AssertCalloc(1, sizeof(LineElement));
    elemPtr->procsPtr = &lineProcs;
    elemPtr->configSpecs = (id == CID_ELEM_LINE) ? lineSpecs : stripSpecs;
    elemPtr->obj.name = Blt_GetHashKey(&graphPtr->elements.nameTable, hPtr);
    Blt_GraphSetObjectClass(&elemPtr->obj, id);
    elemPtr->flags = SCALE_SYMBOL;
    elemPtr->obj.graphPtr = graphPtr;
    /* By default an element's name and label are the same. */
    elemPtr->label = Blt_AssertStrdup(elemPtr->obj.name);
    elemPtr->legendRelief = TK_RELIEF_FLAT;
    elemPtr->penDir = PEN_BOTH_DIRECTIONS;
    elemPtr->styles = Blt_Chain_Create();
    elemPtr->reqSmooth = SMOOTH_NONE;
    elemPtr->builtinPenPtr = &elemPtr->builtinPen;
    InitPen(elemPtr->builtinPenPtr);
    elemPtr->builtinPenPtr->graphPtr = graphPtr;
    elemPtr->builtinPenPtr->classId = id;
    bltLineStylesOption.clientData = (ClientData)sizeof(LineStyle);
    elemPtr->hashPtr = hPtr;
    Blt_SetHashValue(hPtr, elemPtr);
    return (Element *)elemPtr;
}
