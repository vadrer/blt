package require BLT

blt::paneset .ps  -bg grey -width 700 \
    -sashthickness 3 -background red \
    -sashborderwidth 1 -sashrelief sunken \
    -sashpad 1 

option add *Divisions 4
blt::graph .ps.g -bg \#CCCCFF -height 300
blt::barchart .ps.b -bg \#FFCCCC -height 300
blt::barchart .ps.b2 -bg \#CCFFCC -height 300

.ps add -window .ps.g -fill both
.ps add -window .ps.b -fill both 
.ps add -window .ps.b2 -fill both

focus .ps

blt::table . \
    0,0 .ps -fill both

blt::table configure . r1 -resize none