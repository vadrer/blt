package require BLT
set data {}
set numPoints 1000
set numIsolines 10
set xv [blt::vector create -length $numPoints]
set yv [blt::vector create -length $numPoints]
set zv [blt::vector create -length $numPoints]
$xv random 10
$yv random 100
$xv expr {$xv*10.0}
$yv expr {$yv*10.0}
$zv expr {abs(sin($xv*$yv/70.0))}

blt::contour .g -highlightthickness 0
set mesh [blt::mesh create cloud -x $xv -y $yv]
.g element create sine -values $zv -mesh $mesh 
.g isoline steps $numIsolines -element sine
.g axis configure z \
    -palette rainbow \
    -colorbarthickness 15 \
    -margin right \
    -tickdirection out \
    -rotate -90 \
    -title "Units"  
.g legend configure -hide yes
proc UpdateColors {} {
    global usePaletteColors
    if { $usePaletteColors } {
	.g element configure sine -color palette -fill palette
    } else {
        .g element configure sine -color black -fill red
    }
}

proc Fix { what } {
    global show

    set bool $show($what)
    if { [scan $what "isoline%d" number] == 1 } {
	.g isoline configure $what -show $bool
	return
    }
    if { 0 && $what == "colormap" } {
	if { $bool } {
	    set color palette
	} else {
	    set color blue
	}
	.g element configure sine -color $color
    }
    .g element configure sine -show$what $bool
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
set usePaletteColors 0

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
blt::tk::checkbutton .interp -text "Use palette colors" \
    -variable usePaletteColors -command "UpdateColors"
blt::table . \
    0,0 .g -fill both -rowspan 15 \
    0,1 .boundary -anchor w \
    1,1 .colormap -anchor w \
    2,1 .isolines -anchor w \
    3,1 .wireframe -anchor w \
    4,1 .symbols -anchor w \
    5,1 .values -anchor w \
    6,1 .interp -anchor w 
foreach key [array names show] {
    set show($key) [.g element cget sine -show$key]
}

Blt_ZoomStack .g

set count 7
foreach name [lsort -dictionary [.g isoline names]] {
    blt::tk::checkbutton .$name -text "$name"  \
	-variable show($name) -command "Fix $name"
    .g isoline configure $name -show yes -element sine
    set show($name) [.g isoline cget $name -show]
    blt::table . \
	$count,1 .$name -anchor w 
    incr count
}

blt::table configure . r* c1 -resize none
blt::table configure . r$count -resize both
blt::table . \
    0,0 .g -fill both -rowspan [expr $count+1]

.g postscript output contour2.ps -landscape yes
