#!../src/bltwish

package require BLT
set visual [winfo screenvisual .]
source scripts/stipples.tcl

if { [file isdirectory ../library] } {
    set blt_library ../library
}

if { [catch { package require Itk} ] == 0 }  {

    source $blt_library/bltPrintGraph.tcl
    proc PrintDialog { graph } {
	if { [winfo exists .p1] } {
	    return
	}
	toplevel .p1
	update
	blt::PrintGraph .p1.print 
	pack .p1.print -fill both -expand yes 
	set out [.p1.print print $graph]
	if { $out != "" } {
	    set file "saved[lindex $out 0]"
	    set f [open $file "w"]
	    puts $f [lindex $out 1]
	    close $f
	}
	destroy .p1
    }
}

if { $visual != "staticgray" && $visual != "grayscale" } {
    option add *Graph3.Button.Background	red
    option add *TextMarker.Foreground	black
    option add *TextMarker.Background	yellow
    option add *LineMarker.Foreground	black
    option add *LineMarker.Background	yellow
    option add *PolyMarker.Fill		yellow2
    option add *PolyMarker.Outline	""
    option add *PolyMarker.Stipple	fdiagonal1
    option add *activeLine.Color	red4
    option add *activeLine.Fill		red2
    option add *Element.Color		purple
}

option add *Text.font			{ "Serif" 12 }
option add *header.font			{ "Serif" 12 }
option add *footer.font			{ "Serif" 12 }
option add *HighlightThickness		0
option add *plotBorderWidth 		0
option add *plotPadX 		0
option add *plotPadY 		0
set graph [blt::graph .g]
source scripts/graph3.tcl


set text {
This is an example of a bitmap marker.  Try zooming in on 
a region by clicking the left button, moving the pointer, 
and clicking again.  Notice that the bitmap scales too. 
To restore the last view, click on the right button.  
}
blt::htext .header -text $text

blt::htext .footer -text {Hit the %%
    set im [image create picture -file ./images/stopsign.gif]
    button $htext(widget).quit -image $im -command { exit }
    $htext(widget) append $htext(widget).quit 
%% button when you've seen enough. %%
    label $htext(widget).logo -bitmap BLT
    $htext(widget) append $htext(widget).logo 
%%}

blt::table . \
    .header 0,0 -fill x -padx 4 -pady 4\
    $graph 1,0 -fill both  \
    .footer 2,0 -fill x -padx 4 -pady 4

blt::table configure . r0 r2 -resize none

source scripts/ps.tcl

bind $graph <Shift-ButtonPress-1> { 
    MakePsLayout $graph
}

if { [info commands PrintDialog] == "PrintDialog" } {
    after 2000 {
	PrintDialog $graph
    }
}
