
package require BLT

set palette blue.rgb
set pi2 [expr 3.14159265358979323846 * 2]

set x [blt::vector create]
set y [blt::vector create]
$x linspace -2 2 100
$y linspace -2 3 100
set x2 [blt::vector create]
$x2 expr { $x * $x }
set y2 [blt::vector create]
$y2 expr { $y * $y }
set e 2.7182818284590452354
set z {}
foreach  i [$y2 values] {
    foreach  j [$x2 values] k [$x values] {
	lappend z [expr $k * pow($e, -$j - $i)]
    }
}
	
blt::contour .g -highlightthickness 0
set mesh [blt::mesh create irregular -y $x -x $y]
.g element create myContour -values $z -mesh $mesh 
.g isoline steps 10 -element myContour 
.g legend configure -hide yes
.g axis configure z -palette $palette
proc UpdateColors {} {
     global usePaletteColors
     if { $usePaletteColors } {
        .g element configure myContour -color palette -fill palette
    } else {
        .g element configure myContour -color black -fill red
    }
}
proc FixPalette {} {
    global palette
    .g axis configure z -palette $palette
}

proc Fix { what } {
    global show

    set bool $show($what)
    .g element configure myContour -show$what $bool
}

array set show {
    boundary 0
    values 0
    symbols 0
    isolines 0
    colormap 0
    symbols 0
    wireframe 0
}

blt::tk::checkbutton .boundary -text "Boundary" -variable show(boundary) \
    -command "Fix boundary"
blt::tk::checkbutton .wireframe -text "Wireframe" -variable show(wireframe) \
    -command "Fix wireframe"
blt::tk::checkbutton .colormap -text "Colormap"  \
    -variable show(colormap) -command "Fix colormap"
blt::tk::checkbutton .isolines -text "Isolines" \
    -variable show(isolines) -command "Fix isolines"
blt::tk::checkbutton .values -text "Values" \
    -variable show(values) -command "Fix values"
blt::tk::checkbutton .symbols -text "Symbols" \
    -variable show(symbols) -command "Fix symbols"
blt::tk::label .label -text ""
blt::tk::checkbutton .interp -text "Use palette colors" \
    -variable usePaletteColors -command "UpdateColors"
blt::combobutton .palettes \
    -textvariable palette \
    -relief sunken \
    -background white \
    -arrowon yes \
    -menu .palettes.menu 
blt::tk::label .palettesl -text "Palettes" 
blt::combomenu .palettes.menu \
    -background white \
    -textvariable palette \
    -height 200 \
    -yscrollbar .palettes.menu.ybar \
    -xscrollbar .palettes.menu.xbar

blt::tk::scrollbar .palettes.menu.xbar 
blt::tk::scrollbar .palettes.menu.ybar

foreach pal [blt::palette names] {
    set pal [string trim $pal ::]
    lappend palettes $pal
}
.palettes.menu listadd [lsort -dictionary $palettes] -command FixPalette

blt::table . \
    0,0 .label -fill x \
    1,0 .g -fill both -rowspan 9 \
    1,1 .boundary -anchor w -cspan 2 \
    2,1 .colormap -anchor w -cspan 2\
    3,1 .isolines -anchor w -cspan 2 \
    4,1 .wireframe -anchor w -cspan 2 \
    5,1 .symbols -anchor w -cspan 2 \
    6,1 .values -anchor w -cspan 2 \
    7,1 .interp -anchor w -cspan 2 \
    8,1 .palettesl -anchor w  \
    8,2 .palettes -fill x
blt::table configure . r* c1 -resize none
blt::table configure . r9 -resize both

foreach key [array names show] {
    set show($key) [.g element cget myContour -show$key]
}

Blt_ZoomStack .g
