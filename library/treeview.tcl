
# ======================================================================
#
# treeview.tcl
#
# ----------------------------------------------------------------------
# Bindings for the BLT treeview widget
# ----------------------------------------------------------------------
#
#   AUTHOR:  George Howlett
#            Bell Labs Innovations for Lucent Technologies
#            gah@lucent.com
#            http://www.tcltk.com/blt
#
#      RCS:  $Id: treeview.tcl,v 1.38 2010/01/23 00:25:48 ghowlett Exp $
#
# ----------------------------------------------------------------------
# Copyright (c) 1998  Lucent Technologies, Inc.
# ----------------------------------------------------------------------
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted,
# provided that the above copyright notice appear in all copies and that
# both that the copyright notice and warranty disclaimer appear in
# supporting documentation, and that the names of Lucent Technologies
# any of their entities not be used in advertising or publicity
# pertaining to distribution of the software without specific, written
# prior permission.
#
# Lucent Technologies disclaims all warranties with regard to this
# software, including all implied warranties of merchantability and
# fitness.  In no event shall Lucent be liable for any special, indirect
# or consequential damages or any damages whatsoever resulting from loss
# of use, data or profits, whether in an action of contract, negligence
# or other tortuous action, arising out of or in connection with the use
# or performance of this software.
#
# ======================================================================

namespace eval ::blt::TreeView {
    variable _private
    array set _private {
	afterId -1
	scroll	0
	column	""
	space   off
	x	0
	y	0
    }
}

image create picture ::blt::TreeView::closeIcon -data {
    AAEBAABZACAAAAAAFgAWAAgIAAAAAACPrf8Aj5j/AOn+/wDC//8AJy3kADpE+gAo
    LOMA5///AN7//wAFCLEANkT6AMD8/wDj//8AAAAKAJCt/wDa//8AkJj/ACgt5AAS
    FN4AHynzAAAAEQAAAFYAyOz/AJWt/wDO+P8AAAAGAISP/wAAAF0AN0T6ANb//wA7
    QfgAJC3kAAAAUgDa+f8AAABvAMz3/wAAAGQAka3/AOv+/wDD9/8AKDb6ACUx7wA8
    RPoAKS3kAAAAWQDS//8Awfj/AMj3/wA6Q/oAm6//AML+/wCNmP8AJS3kADhE+gAY
    H9sAzff/AM7//wBha/8Akq3/AJKY/wDz//8APUT6AAAADAB7mP8AKi3kANf7/wAa
    I+4ANET6AJet/wDK//8ANEL6AC1B+gDv//8Ajpj/ACYt5AA5RPoABwm5AAAAfADO
    9/8Ak63/AMH9/wA+RPoAKy3kAOv//wB9i/8AAABQACIt5ADK9/8AAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAjFCkpKSkpKSkpKSkp
    KSkpKSkpQxwAKjMEBAQEBAQEBAQEBAQEBAQEBAw3AERRUVFRUVFRUVFRUVFRUVFR
    UVFRVwALRkZGRkZGRkZGRkZGRkZGRkZGRiAAHTk5OTk5OTk5OTk5OTk5OTk5OTk1
    ADYuLi4uLi4uLi4uLi4uLi4uLi4uNQBMHh4eHh4eHh4eHh4eHh4eHh4eHksABhAQ
    EBAQEBAQEBAQEBAQEBAQEBAFACsJCQkJCQkJCQkJCQkJCQkJCQkJEgA+DQ0NDQ0N
    DQ0NDQ0NDQ0NDQ0NDRIAUggICAgICAgICAgICAgICAgICAgsAFJUVFRUVFRUVFRU
    VFRUVFRUVFRUQQBIKDBYJE9POCQZJ0lJSUlJSUlJSVMARy8vLy8vLy8vLyI9PT09
    PT09PT0HADFCQkJCQkJCQkIXMkUYUDsmDwFACgAfAwMDAwMDAwMDOk4lJSUlJSUl
    LQ4ATVU0ETw8EQJKGxMaAAAAAAAAAAAAAD9WFhYWFhYWFiEVAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA7wEAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAUAHgDbBwsACgAvAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAWgMAAAAAAABUUlVFVklTSU9OLVhGSUxFLgA=
}

image create picture ::blt::TreeView::openIcon -data {
    AAEJAAB/ACAAAAAAFgAWAAgIAAAAAADL//8AWJz/ANn//wDR9v8Al/P/AAAAEgAA
    AFcAAAB0AHSJ/wDP9/8AAACRAAAABwCGmP8A5///AKr1/wDJ/v8Az/X/AFhb/wCi
    rf8ATVX+AAAAUwDK3P8AWXb/AAAAKwDs//8AXmX/AKj0/wDH/f8AzP//AK32/wCE
    lf8AVFv/AAAACgAAACcAAABEAHWJ/wAAABEAjOz/AAAAVgDK/v8An8b/AGp1/wDO
    9v8ApPL/AK74/wB4iv8ABAWqAGmT/wBVW/8AWHX/AOv+/wAiK+YA6f//AIjq/wDS
    //8AvPT/AJLw/wAAAAkAu///ANH1/wAAAKUAf63/AAAAQwDg//8AyvT/AML+/wAA
    AC0AISvsAAAAcgDu//8AlMH/AK/4/wByif8Am63/ACk28wDF7v8AV2X/AJXx/wDl
    //8AJy/eAFZb/wDz//8AAAAMAM31/wCzxv8A6v//AAAAAQCJrf8AGR7WAAAAnQB/
    kP8AcIb/AAAAEwAAAE0Ayv//AKv2/wBSW/8Aep//AND2/wCm8v8Am6X/AO///wAA
    ABoAc4n/ANj//wCAlv8Aw+f/AAAA3gAYHMoAd5b/AJOt/wDI/v8AQEj0AAAAjgCX
    p/8AV1v/AJmk/wCF6P8ABgemAAAA5QCP7v8Arvf/AOv//wCZ9P8AAAAWAMX1/5UA
    gQCAJZBDgFOAAIAAgCKAS48XgDKARIBngACARYBHkEKAYoBegACAL4B+kB2AKYAI
    gACAUJE3gEyAWoAYgE2SA4BtgHKADZJAgHGAeIBVkg6AKoBsgFuAa4BBgFSAEYA8
    gASAY4ArgAqAM4hGgHWAIYALgAKAdoA2gCaAeYA5gE6ABYB8gDiBGYB7glaCNYBz
    gACAJ4A+hzuASIAsgGSAG4APgGCAHoB6gC2AMIA9gACAJ4BYhwGAKIAcgHCCEIAo
    gV+AboAjgACAJ4BvkGmAaoAjgACAJ4BKkE+AH4A/gACAJ4ATh2aAFoAugCSACYFo
    gUmAXIA0gF2AAIAVgGWHUoAagAeGI4B9gACAAIAGgFmAYYAxgHSAEoB0gFGAIIAU
    gHeAV4gAggCAOoUhgAyKAJUAlQCVAO8BAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAFAB4A2wcLAAoALwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEYDAAAAAAAAVFJV
    RVZJU0lPTi1YRklMRS4A
}

# Sorting direction icons
# Down arrow
image create picture blt::TreeView::sortdown -data {
    AAEBAAAjACAAAAAACQANAAgIAAA/QQAAyOUAAAAAAABAhAAAgDIAAAAOAAAAAwAA
    zVwAALzpAACo4AAAzf8AAAAnAAAAHAAAnDYAAIiaAAAABgAAzZkAAAARAAB4EQAA
    AAIAAIKhAAC22gAAzJoAAAABAACvYAAAxcAAAMzXAACnSwAAxj8AAAA3AAAADwAA
    jcEAAL6aAAAABAAAFWICAgICExcCAgICAgIPAAsTAgICAhMYCAMMAgICAg0BCgki
    BQICEhkKCgofHQYCHBYaChUgBBMCAgIWChQRAgICAgIWChQRAgICAgIWChQRAgIC
    AgIWChQRAgICAgIWCg4eAgICAgIHEBshAgICAgICAgICAgLvAQAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAABQASANwHAAA1ADoAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAATAQAAAAAAAFRSVUVWSVNJT04tWEZJTEUuAA==
}
# Up arrow 
image create picture blt::TreeView::sortup -data {
    AAEBAAAnACAAAAAACQANAAgIewAAOgAAAABdAACaOwAASgAAAA4AAAADhQAA4YkA
    AJp7AADZTAAAOQAAABFXAAClgwAAQAAAAA00AABsfQAAwAAAAAKKAAD2cAAAS4MA
    AKFXAACkAAAABVkAAKFiAACRiwAA/wAAAAxrAADUAAAAATUAAB2ZAAAKAAAAD4oA
    APUAAAAhjAAAegAAAASLAADWjAAAH2cAAGGMAABHAQEBIh4NEAEBAQEBEgIODQEB
    AQEBBxgWCgEBAQEBBxgWCgEBAQEBBxgWCgEBAQEBBxgWCgEBAQUEExgLIBkbAQAX
    CBgaFAMiASQjGBgYDxwBAQEmHxgGCRsBAQEBIRElIgEBAQEBHQwVAQEBAQEBAQEB
    AQEB7wEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAUAEgDcBwAANQA6AAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAA
    AAAAAAAAAAAAAAAAAAAAAAAAIwEAAAAAAABUUlVFVklTSU9OLVhGSUxFLgA=
}

option add *BltTreeView*Column.increasingIcon blt::TreeView::sortup
option add *BltTreeView*Column.decreasingIcon blt::TreeView::sortdown
option add *BltTreeView.ColumnCommand blt::TreeView::SortColumn

if { $tcl_platform(platform) == "windows" } {
    if { $tk_version >= 8.3 } {
	set cursor "@[file join $blt_library treeview.cur]"
    } else {
	set cursor "size_we"
    }
    option add *BltTreeView.ResizeCursor [list $cursor]
} else {
    option add *BltTreeView.ResizeCursor \
	[list @$blt_library/treeview.xbm $blt_library/treeview_m.xbm black white]
}

# ----------------------------------------------------------------------
#
# Initialize --
#
#	Invoked by internally by Treeview_Init routine.  Initializes
#	the default bindings for the treeview widget entries.  These
#	are local to the widget, so they can't be set through the
#	widget's class bind tags.
#
# ----------------------------------------------------------------------
proc blt::TreeView::Initialize { w } {
    variable _private
    #
    # Active entry bindings
    #
    $w bind Entry <Enter> { 
	%W entry highlight current 
    }
    $w bind Entry <Leave> { 
	%W entry highlight "" 
    }

    #
    # Button bindings
    #
    $w button bind all <ButtonRelease-1> {
	set index [%W nearest %x %y blt::TreeView::_private(who)]
	if { [%W index current] == $index && 
	     $blt::TreeView::_private(who) == "button" } {
	    %W see -anchor nw current
	    %W toggle current
	}
    }
    $w button bind all <Enter> {
	%W button highlight current
    }
    $w button bind all <Leave> {
	%W button highlight ""
    }

    #
    # ButtonPress-1
    #
    #	Performs the following operations:
    #
    #	1. Clears the previous selection.
    #	2. Selects the current entry.
    #	3. Sets the focus to this entry.
    #	4. Scrolls the entry into view.
    #	5. Sets the selection anchor to this entry, just in case
    #	   this is "multiple" mode.
    #
    
    $w bind Entry <ButtonPress-1> { 	
	blt::TreeView::SetSelectionAnchor %W current
    }

    $w bind Entry <Double-ButtonPress-1> {
	if { ![%W cget -flat] } {
	    %W toggle current
	}
    }

    #
    # B1-Motion
    #
    #	For "multiple" mode only.  Saves the current location of the
    #	pointer for auto-scrolling.  Resets the selection mark.  
    #
    $w bind Entry <B1-Motion> { 
	set blt::TreeView::_private(x) %x
	set blt::TreeView::_private(y) %y
	set index [%W nearest %x %y]
	set blt::TreeView::_private(scroll) 1
	if { [%W cget -selectmode] == "multiple" } {
	    %W selection mark $index
	} else {
	    blt::TreeView::SetSelectionAnchor %W $index
	}
    }

    #
    # ButtonRelease-1
    #
    $w bind Entry <ButtonRelease-1> { 
	after cancel $blt::TreeView::_private(afterId)
	set blt::TreeView::_private(afterId) -1
	set blt::TreeView::_private(scroll) 0
	if { [%W cget -selectmode] == "multiple" } {
	    %W selection anchor current
	} else {
	    %W invoke current
	}
    }

    #
    # Shift-ButtonPress-1
    #
    #	For "multiple" mode only.
    #

    $w bind Entry <Shift-ButtonPress-1> { 
	if { [%W cget -selectmode] == "multiple" && [%W selection present] } {
	    if { [%W index anchor] == -1 } {
		%W selection anchor current
	    }
	    set index [%W index anchor]
	    %W selection clearall
	    %W selection set $index current
	} else {
	    blt::TreeView::SetSelectionAnchor %W current
	}
    }
    $w bind Entry <Shift-Double-ButtonPress-1> {
	# do nothing
    }
    $w bind Entry <Shift-B1-Motion> { 
	# do nothing
    }
    $w bind Entry <Shift-ButtonRelease-1> { 
	after cancel $blt::TreeView::_private(afterId)
	set blt::TreeView::_private(afterId) -1
	set blt::TreeView::_private(scroll) 0
    }

    #
    # Control-ButtonPress-1
    #
    #	For "multiple" mode only.  
    #
    $w bind Entry <Control-ButtonPress-1> { 
	if { [%W cget -selectmode] == "multiple" } {
	    set index [%W index current]
	    %W selection toggle $index
	    %W selection anchor $index
	} else {
	    blt::TreeView::SetSelectionAnchor %W current
	}
    }
    $w bind Entry <Control-Double-ButtonPress-1> {
	# do nothing
    }
    $w bind Entry <Control-B1-Motion> { 
	# do nothing
    }
    $w bind Entry <Control-ButtonRelease-1> { 
	after cancel $blt::TreeView::_private(afterId)
	set blt::TreeView::_private(afterId) -1
	set blt::TreeView::_private(scroll) 0
    }

    $w bind Entry <Control-Shift-ButtonPress-1> { 
	if { [%W cget -selectmode] == "multiple" && [%W selection present] } {
	    if { [%W index anchor] == -1 } {
		%W selection anchor current
	    }
	    if { [%W selection includes anchor] } {
		%W selection set anchor current
	    } else {
		%W selection clear anchor current
		%W selection set current
	    }
	} else {
	    blt::TreeView::SetSelectionAnchor %W current
	}
    }
    $w bind Entry <Control-Shift-Double-ButtonPress-1> {
	# do nothing
    }
    $w bind Entry <Control-Shift-B1-Motion> { 
	# do nothing
    }

    $w bind Entry <ButtonRelease-3> { 
	blt::TreeView::EditColumn %W %X %Y
    }

    $w column bind all <Enter> {
	%W column activate current
    }
    $w column bind all <Leave> {
	%W column deactivate
    }
    $w column bind Rule <Enter> {
	%W column activate current
	%W column resize activate current
    }
    $w column bind Rule <Leave> {
	%W column deactivate
	%W column resize deactivate 
    }
    $w column bind Rule <ButtonPress-1> {
	%W column resize anchor %x
    }
    $w column bind Rule <B1-Motion> {
	%W column resize mark %x
    }
    $w column bind Rule <ButtonRelease-1> {
	%W column configure active -width [%W column resize set]
    }
    $w column bind all <ButtonPress-1> {
	set blt::TreeView::_private(column) active
	%W column configure $blt::TreeView::_private(column) \
	    -titlerelief sunken
    }
    $w column bind all <ButtonRelease-1> {
	%W column invoke active
	%W column configure active -titlerelief raised
    }
#     $w bind TextBoxStyle <ButtonPress-3> { 
# 	if { [%W edit -root -test %X %Y] } {
# 	    break
# 	}
#     }
#     $w bind TextBoxStyle <ButtonRelease-3> { 
# 	if { [%W edit -root -test %X %Y] } {
# 	    blt::TreeView::EditColumn %W %X %Y
# 	    break
# 	}
#     }
    $w bind CheckBoxStyle <Enter> { 
	if { [%W column cget current -edit] } {
	    %W style activate current current
	} 
    }
    $w bind CheckBoxStyle <Leave> { 
	%W style deactivate
    }
    $w bind CheckBoxStyle <ButtonPress-1> { 
	if { [%W column cget current -edit] } {
	    break
	}
    }
    $w bind CheckBoxStyle <B1-Motion> { 
	if { [%W column cget current -edit] } {
	    break
	}
    }
    $w bind CheckBoxStyle <ButtonRelease-1> { 
	if { [%W edit -root -test %X %Y] } {
	    %W edit -root %X %Y
	    break
	}
    }
    $w bind ComboBoxStyle <Enter> { 
	if { [%W column cget current -edit] } {
	    %W style activate current current
	} 
    }
    $w bind ComboBoxStyle <Leave> { 
	%W style deactivate
    }
    $w bind ComboBoxStyle <ButtonPress-1> { 
	if { [%W column cget current -edit] } {
	    break
	}
    }
    $w bind ComboBoxStyle <ButtonRelease-1> { 
	if { [%W edit -root -test %X %Y] } {
	    %W edit -root %X %Y
	    break
	}
    }
}

# ----------------------------------------------------------------------
#
# AutoScroll --
#
#	Invoked when the user is selecting elements in a treeview
#	widget and drags the mouse pointer outside of the widget.
#	Scrolls the view in the direction of the pointer.
#
# ----------------------------------------------------------------------
proc blt::TreeView::AutoScroll { w } {
    variable _private

    if { ![winfo exists $w] } {
	return
    }
    if { !$_private(scroll) } {
	return 
    }
    set x $_private(x)
    set y $_private(y)

    set index [$w nearest $x $y]
    if {$y >= [winfo height $w]} {
	$w yview scroll 1 units
	set neighbor down
    } elseif {$y < 0} {
	$w yview scroll -1 units
	set neighbor up
    } else {
	set neighbor $index
    }
    if { [$w cget -selectmode] == "single" } {
	SetSelectionAnchor $w $neighbor
    } else {
	$w selection mark $index
    }
    set _private(afterId) [after 50 blt::TreeView::AutoScroll $w]
}

proc blt::TreeView::SetSelectionAnchor { w tagOrId } {
    set index [$w index $tagOrId]
    $w selection clearall
    $w see $index
    $w focus $index
    $w selection set $index
    $w selection anchor $index
}

# ----------------------------------------------------------------------
#
# MoveFocus --
#
#	Invoked by KeyPress bindings.  Moves the active selection to
#	the entry <where>, which is an index such as "up", "down",
#	"prevsibling", "nextsibling", etc.
#
# ----------------------------------------------------------------------
proc blt::TreeView::MoveFocus { w tagOrId } {
    catch {$w focus $tagOrId}
    if { [$w cget -selectmode] == "single" } {
        $w selection clearall
        $w selection set focus
	$w selection anchor focus
    }
    $w see focus
}

# ----------------------------------------------------------------------
#
# MovePage --
#
#	Invoked by KeyPress bindings.  Pages the current view up or
#	down.  The <where> argument should be either "top" or
#	"bottom".
#
# ----------------------------------------------------------------------
proc blt::TreeView::MovePage { w where } {

    # If the focus is already at the top/bottom of the window, we want
    # to scroll a page. It's really one page minus an entry because we
    # want to see the last entry on the next/last page.
    if { [$w index focus] == [$w index view.$where] } {
        if {$where == "top"} {
	    $w yview scroll -1 pages
	    $w yview scroll 1 units
        } else {
	    $w yview scroll 1 pages
	    $w yview scroll -1 units
        }
    }
    update

    # Adjust the entry focus and the view.  Also activate the entry.
    # just in case the mouse point is not in the widget.
    $w entry highlight view.$where
    $w focus view.$where
    $w see view.$where
    if { [$w cget -selectmode] == "single" } {
        $w selection clearall
        $w selection set focus
    }
}

# ----------------------------------------------------------------------
#
# NextMatch --
#
#	Invoked by KeyPress bindings.  Searches for an entry that
#	starts with the letter <char> and makes that entry active.
#
# ----------------------------------------------------------------------
proc blt::TreeView::NextMatch { w key } {
    if {[string match {[ -~]} $key]} {
	set last [$w index focus]
	if { $last == -1 } {
	    return;			# No focus. 
	}
	set next [$w index next]
	if { $next == -1 } {
	    set next $last
	}
	while { $next != $last } {
	    set label [$w entry cget $next -label]
	    set label [string index $label 0]
	    if { [string tolower $label] == [string tolower $key] } {
		break
	    }
	    set next [$w index -at $next next]
	}
	$w focus $next
	if {[$w cget -selectmode] == "single"} {
	    $w selection clearall
	    $w selection set focus
	}
	$w see focus
    }
}

#------------------------------------------------------------------------
#
# InsertText --
#
#	Inserts a text string into an entry at the insertion cursor.  
#	If there is a selection in the entry, and it covers the point 
#	of the insertion cursor, then delete the selection before 
#	inserting.
#
# Arguments:
#	w 	Widget where to insert the text.
#	text	Text string to insert (usually just a single character)
#
#------------------------------------------------------------------------
proc blt::TreeView::InsertText { w text } {
    if { [string length $text] > 0 } {
	set index [$w index insert]
	if { ($index >= [$w index sel.first]) && 
	     ($index <= [$w index sel.last]) } {
	    $w delete sel.first sel.last
	}
	$w insert $index $text
    }
}

#------------------------------------------------------------------------
#
# Transpose -
#
#	This procedure implements the "transpose" function for entry
#	widgets.  It tranposes the characters on either side of the
#	insertion cursor, unless the cursor is at the end of the line.
#	In this case it transposes the two characters to the left of
#	the cursor.  In either case, the cursor ends up to the right
#	of the transposed characters.
#
# Arguments:
#	w 	The entry window.
#
#------------------------------------------------------------------------
proc blt::TreeView::Transpose { w } {
    set i [$w index insert]
    if {$i < [$w index end]} {
	incr i
    }
    set first [expr {$i-2}]
    if {$first < 0} {
	return
    }
    set new [string index [$w get] [expr {$i-1}]][string index [$w get] $first]
    $w delete $first $i
    $w insert insert $new
}

#------------------------------------------------------------------------
#
# GetSelection --
#
#	Returns the selected text of the entry with respect to the
#	-show option.
#
# Arguments:
#	w          Entry window from which the text to get
#
#------------------------------------------------------------------------

proc blt::TreeView::GetSelection { w } {
    set text [string range [$w get] [$w index sel.first] \
                       [expr [$w index sel.last] - 1]]
    if {[$w cget -show] != ""} {
	regsub -all . $text [string index [$w cget -show] 0] text
    }
    return $text
}

proc blt::TreeView::EditColumn { w x y } {
    $w see current
    if { [winfo exists $w.edit] } {
	destroy $w.edit
	return
    }
    if { ![$w edit -root -test $x $y] } {
	return
    }
    $w edit -root $x $y
    update
    if { [winfo exists $w.edit] } {
	focus $w.edit
	$w.edit selection range 0 end
	blt::grab push $w.edit
	tkwait window $w.edit
	blt::grab pop $w.edit
    }
}

#
# SortColumn --
#
#	This is called when the column title button is pressed to sort
#	the table according to this column.  Clicking again will change
#	the order of the sort (increasing or decreasing).
#
proc blt::TreeView::SortColumn { w col } {
    set old [$w sort cget -mark]
    set decreasing 0
    if { $old == $col } {
	set decreasing [$w sort cget -decreasing]
	set decreasing [expr !$decreasing]
    }
    $w sort configure -decreasing $decreasing \
	-columns [list $col treeView] \
	-mark $col
    blt::busy hold $w
    $w sort once
    update
    blt::busy release $w
}

# 
# ButtonPress assignments
#
#	B1-Enter	start auto-scrolling
#	B1-Leave	stop auto-scrolling
#	ButtonPress-2	start scan
#	B2-Motion	adjust scan
#	ButtonRelease-2 stop scan
#

bind BltTreeView <ButtonPress-2> {
    set blt::TreeView::_private(cursor) [%W cget -cursor]
    %W configure -cursor hand1
    %W scan mark %x %y
}

bind BltTreeView <B2-Motion> {
    %W scan dragto %x %y
}

bind BltTreeView <ButtonRelease-2> {
    %W configure -cursor $blt::TreeView::_private(cursor)
}

bind BltTreeView <B1-Leave> {
    if { $blt::TreeView::_private(scroll) } {
	blt::TreeView::AutoScroll %W 
    }
}

bind BltTreeView <B1-Enter> {
    after cancel $blt::TreeView::_private(afterId)
    set blt::TreeView::_private(afterId) -1
}

# 
# KeyPress assignments
#
#	Up			
#	Down
#	Shift-Up
#	Shift-Down
#	Prior (PageUp)
#	Next  (PageDn)
#	Left
#	Right
#	space		Start selection toggle of entry currently with focus.
#	Return		Start selection toggle of entry currently with focus.
#	Home
#	End
#	F1
#	F2
#	ASCII char	Go to next open entry starting with character.
#
# KeyRelease
#
#	space		Stop selection toggle of entry currently with focus.
#	Return		Stop selection toggle of entry currently with focus.


bind BltTreeView <KeyPress-Up> {
    blt::TreeView::MoveFocus %W up
    if { $blt::TreeView::_private(space) } {
	%W selection toggle focus
    }
}

bind BltTreeView <KeyPress-Down> {
    blt::TreeView::MoveFocus %W down
    if { $blt::TreeView::_private(space) } {
	%W selection toggle focus
    }
}

bind BltTreeView <Shift-KeyPress-Up> {
    blt::TreeView::MoveFocus %W prevsibling
}

bind BltTreeView <Shift-KeyPress-Down> {
    blt::TreeView::MoveFocus %W nextsibling
}

bind BltTreeView <KeyPress-Prior> {
    blt::TreeView::MovePage %W top
}

bind BltTreeView <KeyPress-Next> {
    blt::TreeView::MovePage %W bottom
}

bind BltTreeView <KeyPress-Left> {
    %W close focus
}
bind BltTreeView <KeyPress-Right> {
    %W open focus
    %W see focus -anchor w
}

bind BltTreeView <KeyPress-space> {
    if { [%W cget -selectmode] == "single" } {
	if { [%W selection includes focus] } {
	    %W selection clearall
	} else {
	    %W selection clearall
	    %W selection set focus
	}
    } else {
	%W selection toggle focus
    }
    set blt::TreeView::_private(space) on
}

bind BltTreeView <KeyRelease-space> { 
    set blt::TreeView::_private(space) off
}

bind BltTreeView <KeyPress-Return> {
    blt::TreeView::MoveFocus %W focus
    set blt::TreeView::_private(space) on
}

bind BltTreeView <KeyRelease-Return> { 
    set blt::TreeView::_private(space) off
}

bind BltTreeView <KeyPress> {
    blt::TreeView::NextMatch %W %A
}

bind BltTreeView <KeyPress-Home> {
    blt::TreeView::MoveFocus %W top
}

bind BltTreeView <KeyPress-End> {
    blt::TreeView::MoveFocus %W bottom
}

bind BltTreeView <KeyPress-F1> {
    %W open -r root
}

bind BltTreeView <KeyPress-F2> {
    eval %W close -r [%W entry children root] 
}

if {[string equal "x11" [tk windowingsystem]]} {
    bind BltTreeView <4> {
	%W yview scroll -5 units
    }
    bind BltTreeView <5> {
	%W yview scroll 5 units
    }
} else {
    bind BltTreeView <MouseWheel> {
	%W yview scroll [expr {- (%D / 120) * 4}] units
    }
}


#
# Differences between id "current" and operation nearest.
#
#	set index [$w index current]
#	set index [$w nearest $x $y]
#
#	o Nearest gives you the closest entry.
#	o current is "" if
#	   1) the pointer isn't over an entry.
#	   2) the pointer is over a open/close button.
#	   3) 
#

#
#  Edit mode assignments
#
#	ButtonPress-3   Enables/disables edit mode on entry.  Sets focus to 
#			entry.
#
#  KeyPress
#
#	Left		Move insertion position to previous.
#	Right		Move insertion position to next.
#	Up		Move insertion position up one line.
#	Down		Move insertion position down one line.
#	Return		End edit mode.
#	Shift-Return	Line feed.
#	Home		Move to first position.
#	End		Move to last position.
#	ASCII char	Insert character left of insertion point.
#	Del		Delete character right of insertion point.
#	Delete		Delete character left of insertion point.
#	Ctrl-X		Cut
#	Ctrl-V		Copy
#	Ctrl-P		Paste
#	
#  KeyRelease
#
#	ButtonPress-1	Start selection if in entry, otherwise clear selection.
#	B1-Motion	Extend/reduce selection.
#	ButtonRelease-1 End selection if in entry, otherwise use last
#			selection.
#	B1-Enter	Disabled.
#	B1-Leave	Disabled.
#	ButtonPress-2	Same as above.
#	B2-Motion	Same as above.
#	ButtonRelease-2	Same as above.
#	
#


# Standard Motif bindings:

bind BltTreeViewEditor <ButtonPress-1> {
    %W icursor @%x,%y
    %W selection clear
}

bind BltTreeViewEditor <Left> {
    %W icursor last
    %W selection clear
}

bind BltTreeViewEditor <Right> {
    %W icursor next
    %W selection clear
}

bind BltTreeViewEditor <Shift-Left> {
    set new [expr {[%W index insert] - 1}]
    if {![%W selection present]} {
	%W selection from insert
	%W selection to $new
    } else {
	%W selection adjust $new
    }
    %W icursor $new
}

bind BltTreeViewEditor <Shift-Right> {
    set new [expr {[%W index insert] + 1}]
    if {![%W selection present]} {
	%W selection from insert
	%W selection to $new
    } else {
	%W selection adjust $new
    }
    %W icursor $new
}

bind BltTreeViewEditor <Home> {
    %W icursor 0
    %W selection clear
}
bind BltTreeViewEditor <Shift-Home> {
    set new 0
    if {![%W selection present]} {
	%W selection from insert
	%W selection to $new
    } else {
	%W selection adjust $new
    }
    %W icursor $new
}
bind BltTreeViewEditor <End> {
    %W icursor end
    %W selection clear
}
bind BltTreeViewEditor <Shift-End> {
    set new end
    if {![%W selection present]} {
	%W selection from insert
	%W selection to $new
    } else {
	%W selection adjust $new
    }
    %W icursor $new
}

bind BltTreeViewEditor <Delete> {
    if { [%W selection present]} {
	%W delete sel.first sel.last
    } else {
	%W delete insert
    }
}

bind BltTreeViewEditor <BackSpace> {
    if { [%W selection present] } {
	%W delete sel.first sel.last
    } else {
	set index [expr [%W index insert] - 1]
	if { $index >= 0 } {
	    %W delete $index $index
	}
    }
}

bind BltTreeViewEditor <Control-space> {
    %W selection from insert
}

bind BltTreeViewEditor <Select> {
    %W selection from insert
}

bind BltTreeViewEditor <Control-Shift-space> {
    %W selection adjust insert
}

bind BltTreeViewEditor <Shift-Select> {
    %W selection adjust insert
}

bind BltTreeViewEditor <Control-slash> {
    %W selection range 0 end
}

bind BltTreeViewEditor <Control-backslash> {
    %W selection clear
}

bind BltTreeViewEditor <KeyPress> {
    blt::TreeView::InsertText %W %A
}

# Ignore all Alt, Meta, and Control keypresses unless explicitly bound.
# Otherwise, if a widget binding for one of these is defined, the
# <KeyPress> class binding will also fire and insert the character,
# which is wrong.  Ditto for Escape, Return, and Tab.

bind BltTreeViewEditor <Alt-KeyPress> {
    # nothing
}

bind BltTreeViewEditor <Meta-KeyPress> {
    # nothing
}

bind BltTreeViewEditor <Control-KeyPress> {
    # nothing
}

bind BltTreeViewEditor <Escape> { 
    %W cancel 
}

bind BltTreeViewEditor <Return> { 
    %W apply 
}

bind BltTreeViewEditor <Shift-Return> {
    blt::TreeView::InsertText %W "\n"
}

bind BltTreeViewEditor <KP_Enter> {
    # nothing
}

bind BltTreeViewEditor <Tab> {
    # nothing
}

if {![string compare $tcl_platform(platform) "macintosh"]} {
    bind BltTreeViewEditor <Command-KeyPress> {
	# nothing
    }
}

# On Windows, paste is done using Shift-Insert.  Shift-Insert already
# generates the <<Paste>> event, so we don't need to do anything here.
if { [string compare $tcl_platform(platform) "windows"] != 0 } {
    bind BltTreeViewEditor <Insert> {
	catch {blt::TreeView::InsertText %W [selection get -displayof %W]}
    }
}

# Additional emacs-like bindings:
bind BltTreeViewEditor <ButtonRelease-3> {
    set parent [winfo parent %W]
    %W cancel
}

bind BltTreeViewEditor <Control-a> {
    %W icursor 0
    %W selection clear
}

bind BltTreeViewEditor <Control-b> {
    %W icursor [expr {[%W index insert] - 1}]
    %W selection clear
}

bind BltTreeViewEditor <Control-d> {
    %W delete insert
}

bind BltTreeViewEditor <Control-e> {
    %W icursor end
    %W selection clear
}

bind BltTreeViewEditor <Control-f> {
    %W icursor [expr {[%W index insert] + 1}]
    %W selection clear
}

bind BltTreeViewEditor <Control-h> {
    if {[%W selection present]} {
	%W delete sel.first sel.last
    } else {
	set index [expr [%W index insert] - 1]
	if { $index >= 0 } {
	    %W delete $index $index
	}
    }
}

bind BltTreeViewEditor <Control-k> {
    %W delete insert end
}

if 0 {
    bind BltTreeViewEditor <Control-t> {
	blt::TreeView::Transpose %W
    }
    bind BltTreeViewEditor <Meta-b> {
	%W icursor [blt::TreeView::PreviousWord %W insert]
	%W selection clear
    }
    bind BltTreeViewEditor <Meta-d> {
	%W delete insert [blt::TreeView::NextWord %W insert]
    }
    bind BltTreeViewEditor <Meta-f> {
	%W icursor [blt::TreeView::NextWord %W insert]
	%W selection clear
    }
    bind BltTreeViewEditor <Meta-BackSpace> {
	%W delete [blt::TreeView::PreviousWord %W insert] insert
    }
    bind BltTreeViewEditor <Meta-Delete> {
	%W delete [blt::TreeView::PreviousWord %W insert] insert
    }
    # tkEntryNextWord -- Returns the index of the next word position
    # after a given position in the entry.  The next word is platform
    # dependent and may be either the next end-of-word position or the
    # next start-of-word position after the next end-of-word position.
    #
    # Arguments:
    # w -		The entry window in which the cursor is to move.
    # start -	Position at which to start search.
    
    if {![string compare $tcl_platform(platform) "windows"]}  {
	proc blt::TreeView::NextWord {w start} {
	    set pos [tcl_endOfWord [$w get] [$w index $start]]
	    if {$pos >= 0} {
		set pos [tcl_startOfNextWord [$w get] $pos]
	    }
	    if {$pos < 0} {
		return end
	    }
	    return $pos
	}
    } else {
	proc blt::TreeView::NextWord {w start} {
	    set pos [tcl_endOfWord [$w get] [$w index $start]]
	    if {$pos < 0} {
		return end
	    }
	    return $pos
	}
    }
    
    # PreviousWord --
    #
    # Returns the index of the previous word position before a given
    # position in the entry.
    #
    # Arguments:
    # w -		The entry window in which the cursor is to move.
    # start -	Position at which to start search.
    
    proc blt::TreeView::PreviousWord {w start} {
	set pos [tcl_startOfPreviousWord [$w get] [$w index $start]]
	if {$pos < 0} {
	    return 0
	}
	return $pos
    }
}
