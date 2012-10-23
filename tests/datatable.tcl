
package require BLT

if {[info procs test] != "test"} {
    source defs
}
if [file exists ../library] {
    set blt_library ../library
}

#set VERBOSE 1
test datatable.1 {datatable no args} {
    list [catch {blt::datatable} msg] $msg
} {1 {wrong # args: should be one of...
  blt::datatable create ?name?
  blt::datatable destroy tableName...
  blt::datatable exists tableName
  blt::datatable load tableName libpath
  blt::datatable names ?pattern?...}}

test datatable.2 {datatable create #auto} {
    list [catch {blt::datatable create #auto} msg] $msg
} {0 ::datatable0}

test datatable.2 {datatable exists datatable0} {
    list [catch {blt::datatable exists datatable0} msg] $msg
} {0 1}

test datatable.2 {datatable destroy datatable0} {
    list [catch {blt::datatable destroy datatable0} msg] $msg
} {0 {}}

test datatable.2 {datatable exists datatable0} {
    list [catch {blt::datatable exists datatable0} msg] $msg
} {0 0}

test datatable.2 {datatable destroy datatable0} {
    list [catch {blt::datatable destroy datatable0} msg] $msg
} {1 {can't find table "datatable0"}}

test datatable.3 {datatable create #auto.suffix} {
    list [catch {
	blt::datatable create #auto.suffix
    } msg] $msg
} {0 ::datatable0.suffix}

test datatable.4 {datatable create prefix.#auto} {
    list [catch {
	blt::datatable create prefix.#auto
    } msg] $msg
} {0 ::prefix.datatable0}

test datatable.5 {datatable create prefix.#auto.suffix} {
    list [catch {
	blt::datatable create prefix.#auto.suffix
    } msg] $msg
} {0 ::prefix.datatable0.suffix}

test datatable.6 {datatable create prefix.#auto.suffix.#auto} {
    list [catch {
	blt::datatable create prefix.#auto.suffix.#auto
    } msg] $msg
} {0 ::prefix.datatable0.suffix.#auto}

test datatable.7 {blt::datatable destroy [blt::datatable names *datatable0*]} {
    list [catch {
	eval blt::datatable destroy [blt::datatable names *datatable0*]
    } msg] $msg
} {0 {}}

test datatable.8 {blt::datatable create} {
    list [catch {
	blt::datatable create
    } msg] $msg
} {0 ::datatable0}

test datatable.9 {blt::datatable create} {
    list [catch {
	blt::datatable create
    } msg] $msg
} {0 ::datatable1}

test datatable.10 {blt::datatable create fred} {
    list [catch {
	blt::datatable create fred
    } msg] $msg
} {0 ::fred}

test datatable.11 {blt::datatable create fred} {
    list [catch {
	blt::datatable create fred
    } msg] $msg
} {1 {a command "::fred" already exists}}

test datatable.12 {blt::datatable create if} {
    list [catch {
	blt::datatable create if
    } msg] $msg
} {1 {a command "::if" already exists}}

test datatable.13 {blt::datatable create (bad namespace)} {
    list [catch {
	blt::datatable create badNs::fred
    } msg] $msg
} {1 {unknown namespace "badNs"}}

test datatable.14 {blt::datatable create (wrong # args)} {
    list [catch {
	blt::datatable create a b
    } msg] $msg
} {1 {wrong # args: should be "blt::datatable create ?name?"}}

test datatable.15 {names} {
    list [catch {
	blt::datatable names
    } msg] [lsort $msg]
} {0 {::datatable0 ::datatable1 ::fred}}

test datatable.16 {names pattern)} {
    list [catch {
	blt::datatable names ::datatable*
    } msg] [lsort $msg]
} {0 {::datatable0 ::datatable1}}

test datatable.17 {names badPattern)} {
    list [catch {
	blt::datatable names badPattern*
    } msg] $msg
} {0 {}}

test datatable.18 {names pattern arg (wrong # args)} {
    list [catch {
	blt::datatable names pattern arg
    } msg] $msg
} {1 {wrong # args: should be "blt::datatable names ?pattern?..."}}

test datatable.19 {destroy (wrong # args)} {
    list [catch {
	blt::datatable destroy
    } msg] $msg
} {1 {wrong # args: should be "blt::datatable destroy tableName..."}}

test datatable.20 {destroy badName} {
    list [catch {
	blt::datatable destroy badName
    } msg] $msg
} {1 {can't find table "badName"}}

test datatable.21 {destroy fred} {
    list [catch {
	blt::datatable destroy fred
    } msg] $msg
} {0 {}}

test datatable.22 {destroy datatable0 datatable1} {
    list [catch {
	blt::datatable destroy datatable0 datatable1
    } msg] $msg
} {0 {}}

test datatable.23 {create} {
    list [catch {
	blt::datatable create
    } msg] $msg
} {0 ::datatable0}

test datatable.24 {datatable0} {
    list [catch {
	datatable0
    } msg] $msg
} {1 {wrong # args: should be one of...
  datatable0 add table ?switches?
  datatable0 append row column value ?value...?
  datatable0 attach args...
  datatable0 column op args...
  datatable0 copy table
  datatable0 dir path ?switches?
  datatable0 dump ?switches?
  datatable0 duplicate ?table?
  datatable0 emptyvalue ?newValue?
  datatable0 exists row column
  datatable0 export format args...
  datatable0 find expr ?switches?
  datatable0 get row column ?defValue?
  datatable0 import format args...
  datatable0 keys ?column...?
  datatable0 lappend row column value ?value...?
  datatable0 limits ?column?
  datatable0 lookup ?value...?
  datatable0 maximum ?column?
  datatable0 minimum ?column?
  datatable0 notify op args...
  datatable0 numcolumns ?number?
  datatable0 numrows ?number?
  datatable0 restore ?switches?
  datatable0 row op args...
  datatable0 set ?row column value?...
  datatable0 sort ?flags...?
  datatable0 trace op args...
  datatable0 unset row column ?row column?}}

test datatable.25 {datatable0 badOp} {
    list [catch {
	datatable0 badOp
    } msg] $msg
} {1 {bad operation "badOp": should be one of...
  datatable0 add table ?switches?
  datatable0 append row column value ?value...?
  datatable0 attach args...
  datatable0 column op args...
  datatable0 copy table
  datatable0 dir path ?switches?
  datatable0 dump ?switches?
  datatable0 duplicate ?table?
  datatable0 emptyvalue ?newValue?
  datatable0 exists row column
  datatable0 export format args...
  datatable0 find expr ?switches?
  datatable0 get row column ?defValue?
  datatable0 import format args...
  datatable0 keys ?column...?
  datatable0 lappend row column value ?value...?
  datatable0 limits ?column?
  datatable0 lookup ?value...?
  datatable0 maximum ?column?
  datatable0 minimum ?column?
  datatable0 notify op args...
  datatable0 numcolumns ?number?
  datatable0 numrows ?number?
  datatable0 restore ?switches?
  datatable0 row op args...
  datatable0 set ?row column value?...
  datatable0 sort ?flags...?
  datatable0 trace op args...
  datatable0 unset row column ?row column?}}

test datatable.26 {datatable0 column (wrong \# args)} {
    list [catch {
	datatable0 column
    } msg] $msg
} {1 {wrong # args: should be "datatable0 column op args..."}}


test datatable.27 {datatable0 column badOp} {
    list [catch {
	datatable0 column badOp
    } msg] $msg
} {1 {bad operation "badOp": should be one of...
  datatable0 column copy src dest ?switches...?
  datatable0 column count ?column?
  datatable0 column create ?switches?
  datatable0 column delete column...
  datatable0 column duplicate column...
  datatable0 column exists column
  datatable0 column extend label ?label...?
  datatable0 column get column ?switches?
  datatable0 column index column
  datatable0 column indices column ?column...?
  datatable0 column join table ?switches?
  datatable0 column label column ?label?
  datatable0 column labels ?labelList?
  datatable0 column move from to ?count?
  datatable0 column names ?pattern...?
  datatable0 column notify column ?flags? command
  datatable0 column set column row value...
  datatable0 column tag op args...
  datatable0 column trace column how command
  datatable0 column type column ?type?
  datatable0 column unset column ?indices...?
  datatable0 column values column ?valueList?}}

test datatable.28 {datatable0 row (wrong \# args)} {
    list [catch {
	datatable0 row
    } msg] $msg
} {1 {wrong # args: should be "datatable0 row op args..."}}


test datatable.29 {datatable0 row badOp} {
    list [catch {
	datatable0 row badOp
    } msg] $msg
} {1 {bad operation "badOp": should be one of...
  datatable0 row copy src dest ?switches...?
  datatable0 row count ?row?
  datatable0 row create ?switches...?
  datatable0 row delete ?row...?
  datatable0 row duplicate ?row...?
  datatable0 row exists row
  datatable0 row extend label ?label...?
  datatable0 row get row ?switches?
  datatable0 row index row
  datatable0 row indices row ?row...?
  datatable0 row join table ?switches?
  datatable0 row label row ?label?
  datatable0 row labels ?labelList?
  datatable0 row move from to ?count?
  datatable0 row names ?pattern...?
  datatable0 row notify row ?flags? command
  datatable0 row set row column value...
  datatable0 row tag op args...
  datatable0 row trace row how command
  datatable0 row unset row ?indices...?
  datatable0 row values row ?valueList?}}

test datatable.30 {datatable0 numcolumns} {
    list [catch {datatable0 numcolumns} msg] $msg
} {0 0}

test datatable.31 {datatable0 column names} {
    list [catch {datatable0 column names} msg] $msg
} {0 {}}

test datatable.32 {datatable0 numcolumns badArg} {
    list [catch {datatable0 numcolumns badArg} msg] $msg
} {1 {expected integer but got "badArg"}}

test datatable.33 {datatable0 column -label xyz create} {
    list [catch {datatable0 column -label xyz create} msg] $msg
} {1 {bad operation "-label": should be one of...
  datatable0 column copy src dest ?switches...?
  datatable0 column count ?column?
  datatable0 column create ?switches?
  datatable0 column delete column...
  datatable0 column duplicate column...
  datatable0 column exists column
  datatable0 column extend label ?label...?
  datatable0 column get column ?switches?
  datatable0 column index column
  datatable0 column indices column ?column...?
  datatable0 column join table ?switches?
  datatable0 column label column ?label?
  datatable0 column labels ?labelList?
  datatable0 column move from to ?count?
  datatable0 column names ?pattern...?
  datatable0 column notify column ?flags? command
  datatable0 column set column row value...
  datatable0 column tag op args...
  datatable0 column trace column how command
  datatable0 column type column ?type?
  datatable0 column unset column ?indices...?
  datatable0 column values column ?valueList?}}

test datatable.34 {column extend 5} {
    list [catch {datatable0 column extend 5} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.35 {numcolumns} {
    list [catch {datatable0 numcolumns} msg] $msg
} {0 5}

test datatable.36 {column index end} {
    list [catch {datatable0 column index end} msg] $msg
} {0 4}

test datatable.37 {row extend 5} {
    list [catch {datatable0 row extend 5} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.38 {numrows length} {
    list [catch {datatable0 numrows} msg] $msg
} {0 5}

test datatable.39 {column index end} {
    list [catch {datatable0 row index end} msg] $msg
} {0 4}

test datatable.40 {column index all} {
    list [catch {datatable0 column index all} msg] $msg
} {0 -1}

test datatable.41 {column indices all} {
    list [catch {datatable0 column indices all} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.42 {column indices 0-end} {
    list [catch {datatable0 column indices 0-end} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.43 {column indices range=0-end} {
    list [catch {datatable0 column indices range=0-end} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.44 {column indices 0-all} {
    list [catch {datatable0 column indices 0-all} msg] $msg
} {1 {unknown column specification "0-all" in ::datatable0}}

test datatable.45 {column indices range=0-all} {
    list [catch {datatable0 column indices range=0-all} msg] $msg
} {1 {multiple columns specified by "all"}}

test datatable.46 {column indices 1-4} {
    list [catch {datatable0 column indices 1-4} msg] $msg
} {0 {1 2 3 4}}

test datatable.47 {column indices 5-2} {
    list [catch {datatable0 column indices 5-2} msg] $msg
} {1 {unknown column specification "5-2" in ::datatable0}}

test datatable.48 {column indices 4-2} {
    list [catch {datatable0 column indices 4-2} msg] $msg
} {0 {}}

test datatable.49 {column index end} {
    list [catch {datatable0 column index end} msg] $msg
} {0 4}

test datatable.50 {column index end badArg} {
    list [catch {datatable0 column index end badArg} msg] $msg
} {1 {wrong # args: should be "datatable0 column index column"}}

test datatable.51 {column label 0} {
    list [catch {datatable0 column label 0} msg] $msg
} {0 c1}

test datatable.52 {column label 0 myLabel} {
    list [catch {datatable0 column label 0 myLabel} msg] $msg
} {0 {}}

test datatable.53 {column label 0 myLabel} {
    list [catch {datatable0 column label 0 myLabel} msg] $msg
} {0 {}}

test datatable.54 {column label 1 myLabel} {
    list [catch {datatable0 column label 1 myLabel} msg] $msg
} {0 {}}

test datatable.55 {column label 1 myLabel2} {
    list [catch {datatable0 column label 1 myLabel2} msg] $msg
} {0 {}}

test datatable.56 {column label 1 myLabel2} {
    list [catch {datatable0 column label 1 myLabel2} msg] $msg
} {0 {}}

test datatable.57 {column labels} {
    list [catch {datatable0 column labels} msg] [lsort $msg]
} {0 {c3 c4 c5 myLabel myLabel2}}

test datatable.58 {column index myLabel} {
    list [catch {datatable0 column index myLabel} msg] $msg
} {0 0}

test datatable.59 {column label 0 newLabel} {
    list [catch {datatable0 column label 0 newLabel} msg] $msg
} {0 {}}

test datatable.60 {column labels} {
    list [catch {datatable0 column labels} msg] [lsort $msg]
} {0 {c3 c4 c5 myLabel2 newLabel}}

test datatable.61 {column index myLabel2} {
    list [catch {datatable0 column index myLabel2} msg] $msg
} {0 1}

test datatable.62 {column label 0} {
    list [catch {datatable0 column label 0} msg] $msg
} {0 newLabel}

test datatable.63 {column label end end} {
    list [catch {datatable0 column label end end} msg] $msg
} {0 {}}

test datatable.64 {column label end endLabel} {
    list [catch {datatable0 column label end endLabel} msg] $msg
} {0 {}}

test datatable.65 {column label end label-with-minus} {
    list [catch {datatable0 column label end label-with-minus} msg] $msg
} {0 {}}

test datatable.66 {column label end 1abc} {
    list [catch {datatable0 column label end 1abc} msg] $msg
} {0 {}}

test datatable.67 {column label end -abc} {
    list [catch {datatable0 column label end -abc} msg] $msg
} {1 {column label "-abc" can't start with a '-'.}}

test datatable.68 {column indices 0-4} {
    list [catch {datatable0 column indices 0-4} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.69 {column indices range=0-4} {
    list [catch {datatable0 column indices range=0-4} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.70 {column label 0-4 c1 } {
    list [catch {datatable0 column label 0-4 c1} msg] $msg
} {1 {multiple columns specified by "0-4"}}

test datatable.71 {column label 0 c1 1 c2 2 c3 3 c4 4 c5 } {
    list [catch {datatable0 column label 0 c1 1 c2 2 c3 3 c4 4 c5} msg] $msg
} {0 {}}

test datatable.72 {column label 0} {
    list [catch {datatable0 column label 0} msg] $msg
} {0 c1}

test datatable.73 {column label 1} {
    list [catch {datatable0 column label 1} msg] $msg
} {0 c2}

test datatable.74 {column label 2} {
    list [catch {datatable0 column label 2} msg] $msg
} {0 c3}

test datatable.75 {column label 3} {
    list [catch {datatable0 column label 3} msg] $msg
} {0 c4}

test datatable.76 {column label 4} {
    list [catch {datatable0 column label 4} msg] $msg
} {0 c5}

test datatable.77 {column label 5} {
    list [catch {datatable0 column label 5} msg] $msg
} {1 {bad column index "5"}}

test datatable.78 {column names} {
    list [catch {datatable0 column names} msg] $msg
} {0 {c1 c2 c3 c4 c5}}

test datatable.79 {column names c*} {
    list [catch {datatable0 column names c*} msg] $msg
} {0 {c1 c2 c3 c4 c5}}

test datatable.80 {column names {*[1-2]}} {
    list [catch {datatable0 column names {*[1-2]}} msg] $msg
} {0 {c1 c2}}

test datatable.81 {column names noMatch} {
    list [catch {datatable0 column names noMatch} msg] $msg
} {0 {}}

test datatable.82 {row label 0-4 r1} {
    list [catch {datatable0 row label 0-4 r1} msg] $msg
} {1 {multiple rows specified by "0-4"}}

test datatable.83 {row label 0 r1 1 r2 2 r3 3 r4 4 r5} {
    list [catch {datatable0 row label 0 r1 1 r2 2 r3 3 r4 4 r5} msg] $msg
} {0 {}}

test datatable.84 {row names} {
    list [catch {datatable0 row names} msg] $msg
} {0 {r1 r2 r3 r4 r5}}

test datatable.85 {row names r*} {
    list [catch {datatable0 row names r*} msg] $msg
} {0 {r1 r2 r3 r4 r5}}

test datatable.86 {row names noMatch} {
    list [catch {datatable0 row names noMatch} msg] $msg
} {0 {}}

test datatable.87 {column get} {
    list [catch {datatable0 column get} msg] $msg
} {1 {wrong # args: should be "datatable0 column get column ?switches?"}}

test datatable.88 {column get c1} {
    list [catch {datatable0 column get c1} msg] $msg
} {0 {0 {} 1 {} 2 {} 3 {} 4 {}}}

test datatable.89 {column get badColumn} {
    list [catch {datatable0 column get badColumn} msg] $msg
} {1 {unknown column specification "badColumn" in ::datatable0}}

test datatable.90 {column values} {
    list [catch {datatable0 column values} msg] $msg
} {1 {wrong # args: should be "datatable0 column values column ?valueList?"}}

test datatable.91 {column values c1} {
    list [catch {
	datatable0 column values c1
    } msg] $msg
} {0 {{} {} {} {} {}}}

test datatable.92 {column values c1 {1.01 2.01 3.01 4.01 5.01}} {
    list [catch {
	datatable0 column values c1 {1.01 2.01 3.01 4.01 5.01}
    } msg] $msg
} {0 {}}

test datatable.93 {column get 0} {
    list [catch {datatable0 column get 0} msg] $msg
} {0 {0 1.01 1 2.01 2 3.01 3 4.01 4 5.01}}

test datatable.94 {column set all 0 1.0 1 2.0 2 3.0 3 4.0 4 5.0} {
    list [catch {
	datatable0 column set all 0 1.0 1 2.0 2 3.0 3 4.0 4 5.0
    } msg] $msg
} {0 {}}

test datatable.95 {column get all} {
    list [catch {datatable0 column get all} msg] $msg
} {1 {multiple columns specified by "all"}}

test datatable.96 {column get 0-1} {
    list [catch {datatable0 column get 0-1} msg] $msg
} {1 {multiple columns specified by "0-1"}}

test datatable.97 {datatable0 column get 1} {
    list [catch {datatable0 column get 1} msg] $msg
} {0 {0 1.0 1 2.0 2 3.0 3 4.0 4 5.0}}

test datatable.98 {datatable0 column get 2} {
    list [catch {datatable0 column get 2} msg] $msg
} {0 {0 1.0 1 2.0 2 3.0 3 4.0 4 5.0}}

test datatable.99 {datatable0 column get end} {
    list [catch {datatable0 column get end} msg] $msg
} {0 {0 1.0 1 2.0 2 3.0 3 4.0 4 5.0}}

test datatable.100 {datatable0 column set 0 2 a 1 b 0 c} {
    list [catch {datatable0 column set 0 2 a 1 b 0 c} msg] $msg
} {0 {}}

test datatable.101 {column values 0} {
    list [catch {datatable0 column values 0} msg] $msg
} {0 {c b a 4.0 5.0}}

test datatable.102 {column set end 0 x 1 y} {
    list [catch {datatable0 column set end 0 x 1 y} msg] $msg
} {0 {}}

test datatable.103 {column values end} {
    list [catch {datatable0 column values end} msg] $msg
} {0 {x y 3.0 4.0 5.0}}

test datatable.104 {column index c5} {
    list [catch {datatable0 column index c5} msg] $msg
} {0 4}

test datatable.105 {column indices all} {
    list [catch {datatable0 column indices all} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.106 {column index -1} {
    list [catch {datatable0 column index -1} msg] $msg
} {0 -1}

test datatable.107 {column index 1000} {
    list [catch {datatable0 column index 1000} msg] $msg
} {0 -1}

test datatable.108 {column type 0} {
    list [catch {datatable0 column type 0} msg] $msg
} {0 string}

test datatable.109 {column type 1 integer} {
    list [catch {datatable0 column type 1 integer} msg] $msg
} {1 {expected integer but got "1.0"}}

test datatable.110 {column type 3 double} {
    list [catch {datatable0 column type 3 double} msg] $msg
} {0 double}

test datatable.111 {column type 0 string} {
    list [catch {datatable0 column type 0 string} msg] $msg
} {0 string}

test datatable.112 {column type 0 string badArg} {
    list [catch {datatable0 column type 0 string badArg} msg] $msg
} {1 {wrong # args: should be "datatable0 column type column ?type?"}}

test datatable.113 {column type badTag string} {
    list [catch {datatable0 column type badTag string} msg] $msg
} {1 {unknown column specification "badTag" in ::datatable0}}

test datatable.114 {column type all string} {
    list [catch {datatable0 column type all string} msg] $msg
} {0 {string string string string string}}

test datatable.115 {column tag badOp} {
    list [catch {datatable0 column tag badOp} msg] $msg
} {1 {bad tag operation "badOp": should be one of...
  datatable0 column tag add tag ?column...?
  datatable0 column tag delete tag ?column...?
  datatable0 column tag exists tag ?column?
  datatable0 column tag forget ?tag...?
  datatable0 column tag get column ?pattern...?
  datatable0 column tag indices ?tag...?
  datatable0 column tag labels ?tag...?
  datatable0 column tag range from to ?tag...?
  datatable0 column tag search column ?pattern?
  datatable0 column tag set column tag...
  datatable0 column tag unset column tag...}}

test datatable.116 {column tag (missing args)} {
    list [catch {datatable0 column tag} msg] $msg
} {1 {wrong # args: should be one of...
  datatable0 column tag add tag ?column...?
  datatable0 column tag delete tag ?column...?
  datatable0 column tag exists tag ?column?
  datatable0 column tag forget ?tag...?
  datatable0 column tag get column ?pattern...?
  datatable0 column tag indices ?tag...?
  datatable0 column tag labels ?tag...?
  datatable0 column tag range from to ?tag...?
  datatable0 column tag search column ?pattern?
  datatable0 column tag set column tag...
  datatable0 column tag unset column tag...}}

test datatable.117 {datatable0 column tag badOp} {
    list [catch {datatable0 column tag badOp} msg] $msg
} {1 {bad tag operation "badOp": should be one of...
  datatable0 column tag add tag ?column...?
  datatable0 column tag delete tag ?column...?
  datatable0 column tag exists tag ?column?
  datatable0 column tag forget ?tag...?
  datatable0 column tag get column ?pattern...?
  datatable0 column tag indices ?tag...?
  datatable0 column tag labels ?tag...?
  datatable0 column tag range from to ?tag...?
  datatable0 column tag search column ?pattern?
  datatable0 column tag set column tag...
  datatable0 column tag unset column tag...}}

test datatable.118 {datatable0 column tag add} {
    list [catch {datatable0 column tag add} msg] $msg
} {1 {wrong # args: should be "datatable0 column tag add tag ?column...?"}}

test datatable.119 {datatable0 column tag add newTag (no columns)} {
    list [catch {datatable0 column tag add newTag} msg] $msg
} {0 {}}

test datatable.120 {datatable0 column tag add newTag badIndex} {
    list [catch {datatable0 column tag add newTag badIndex} msg] $msg
} {1 {unknown column specification "badIndex" in ::datatable0}}
    
test datatable.121 {datatable0 column tag add newTag 0} {
    list [catch {datatable0 column tag add newTag 0} msg] $msg
} {0 {}}

test datatable.122 {datatable0 column tag add newTag1 0 1 2} {
    list [catch {datatable0 column tag add newTag1 0 1 2} msg] $msg
} {0 {}}

test datatable.123 {datatable0 column tag add newTag2 all} {
    list [catch {datatable0 column tag add newTag2 all} msg] $msg
} {0 {}}

test datatable.124 {datatable0 column tag search} {
    list [catch {datatable0 column tag search} msg] $msg
} {1 {wrong # args: should be "datatable0 column tag search column ?pattern?"}}

test datatable.125 {datatable0 column tag search 0} {
    list [catch {datatable0 column tag search 0} msg] [lsort $msg]
} {0 {all newTag newTag1 newTag2}}

test datatable.126 {datatable0 column tag search 0 *Tag*} {
    list [catch {datatable0 column tag search 0 *Tag*} msg] [lsort $msg]
} {0 {newTag newTag1 newTag2}}

test datatable.127 {datatable0 column tag search all} {
    list [catch {datatable0 column tag search all} msg] $msg
} {0 {newTag2 newTag newTag1 all end}}

test datatable.128 {datatable0 column tag search end} {
    list [catch {datatable0 column tag search end} msg] $msg
} {0 {newTag2 all end}}

test datatable.129 {datatable0 column tag search end end} {
    list [catch {datatable0 column tag search end end} msg] $msg
} {0 end}

test datatable.130 {datatable0 column tag search end e*} {
    list [catch {datatable0 column tag search end e*} msg] $msg
} {0 end}

test datatable.131 {datatable0 column tag delete} {
    list [catch {datatable0 column tag delete} msg] $msg
} {1 {wrong # args: should be "datatable0 column tag delete tag ?column...?"}}

test datatable.132 {datatable0 column tag delete badTag} {
    list [catch {datatable0 column tag delete badTag} msg] $msg
} {0 {}}

test datatable.133 {datatable0 column tag delete newTag1} {
    list [catch {datatable0 column tag delete newTag1} msg] $msg
} {0 {}}

test datatable.134 {datatable0 column tag delete newTags1 1} {
    list [catch {datatable0 column tag delete newTag1 1} msg] $msg
} {0 {}}

test datatable.135 {column tag delete newTag2 1} {
    list [catch {datatable0 column tag delete newTag2 1} msg] $msg
} {0 {}}

test datatable.136 {column tag delete badTag 1} {
    list [catch {datatable0 column tag delete badTag 1} msg] $msg
} {1 {unknown column tag "badTag"}}

test datatable.137 {column tag delete someTag 1000} {
    list [catch {datatable0 column tag delete someTag 1000} msg] $msg
} {1 {bad column index "1000"}}

test datatable.138 {column tag delete end 1} {
    list [catch {datatable0 column tag delete end 1} msg] $msg
} {0 {}}

test datatable.139 {column tag delete all 1} {
    list [catch {datatable0 column tag delete all 1} msg] $msg
} {0 {}}

test datatable.140 {column tag forget} {
    list [catch {datatable0 column tag forget} msg] $msg
} {0 {}}

test datatable.141 {column tag forget all} {
    list [catch {datatable0 column tag forget all} msg] $msg
} {0 {}}

test datatable.142 {column tag forget newTag1} {
    list [catch {datatable0 column tag forget newTag1} msg] $msg
} {0 {}}

test datatable.143 {column tag forget newTag1} {
    list [catch {datatable0 column tag forget newTag1} msg] $msg
} {1 {unknown column tag "newTag1"}}

test datatable.144 {column tag indices} {
    list [catch {datatable0 column tag indices} msg] $msg
} {0 {}}

test datatable.145 {column tag indices all} {
    list [catch {datatable0 column tag indices all} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.146 {column tag indices end} {
    list [catch {datatable0 column tag indices end} msg] $msg
} {0 4}

test datatable.147 {column tag indices newTag} {
    list [catch {datatable0 column tag indices newTag} msg] $msg
} {0 0}

test datatable.148 {column tag range 0 2 midTag} {
    list [catch {datatable0 column tag range 0 2 midTag} msg] $msg
} {0 {}}

test datatable.149 {column tag indices midTag} {
    list [catch {datatable0 column tag indices midTag} msg] $msg
} {0 {0 1 2}}

test datatable.150 {column tag range 0 end myTag} {
    list [catch {datatable0 column tag range 0 end myTag} msg] $msg
} {0 {}}

test datatable.151 {column tag indices myTag} {
    list [catch {datatable0 column tag indices myTag} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.152 {column tag range -1 0 myTag} {
    list [catch {datatable0 column tag range -1 0 myTag} msg] $msg
} {1 {unknown column specification "-1" in ::datatable0}}

test datatable.153 {column tag range 0 -1 myTag} {
    list [catch {datatable0 column tag range 0 -1 myTag} msg] $msg
} {1 {unknown column specification "-1" in ::datatable0}}

test datatable.154 {column tag range 0 1000 myTag} {
    list [catch {datatable0 column tag range 0 1000 myTag} msg] $msg
} {1 {bad column index "1000"}}

test datatable.155 {column unset} {
    list [catch {datatable0 column unset} msg] $msg
} {1 {wrong # args: should be "datatable0 column unset column ?indices...?"}}

test datatable.156 {column unset 0} {
    list [catch {datatable0 column unset 0} msg] $msg
} {0 {}}

test datatable.157 {column unset 0 end} {
    list [catch {datatable0 column unset 0 end} msg] $msg
} {0 {}}

test datatable.158 {column extend 4 badArg } {
    list [catch {datatable0 column extend 4 badArg} msg] $msg
} {1 {column label "4" can't be a number.}}

test datatable.159 {column extend} {
    list [catch {datatable0 column extend} msg] $msg
} {1 {wrong # args: should be "datatable0 column extend label ?label...?"}}

test datatable.160 {column extend -1} {
    list [catch {datatable0 column extend -1} msg] $msg
} {1 {bad count "-1": # columns can't be negative.}}

if 0 {
test datatable.161 {column extend 10000000000 } {
    list [catch {datatable0 column extend 10000000000} msg] $msg
} {1 {can't extend table by 10000000000 columns: out of memory.}}
}
test datatable.162 {column extend -10 } {
    list [catch {datatable0 column extend -10} msg] $msg
} {1 {bad count "-10": # columns can't be negative.}}

test datatable.163 {column names} {
    list [catch {datatable0 column names} msg] $msg
} {0 {c1 c2 c3 c4 c5 c6 c7}}

test datatable.164 {column extend 10 } {
    list [catch {datatable0 column extend 10} msg] $msg
} {0 {7 8 9 10 11 12 13 14 15 16}}

test datatable.165 {column label 5 c6 6 c7 7 c8...} {
    list [catch {
	datatable0 column label \
		5 x6 6 x7 7 x8 8 x9 9 x10 10 x11 11 x12 12 x13 13 x14 14 x15
    } msg] $msg
} {0 {}}

test datatable.166 {column names} {
    list [catch {datatable0 column names} msg] $msg
} {0 {c1 c2 c3 c4 c5 x6 x7 x8 x9 x10 x11 x12 x13 x14 x15 c16 c17}}

test datatable.167 {column delete 9 } {
    list [catch {datatable0 column delete 9} msg] $msg
} {0 {}}

test datatable.168 {column names} {
    list [catch {datatable0 column names} msg] $msg
} {0 {c1 c2 c3 c4 c5 x6 x7 x8 x9 x11 x12 x13 x14 x15 c16 c17}}

test datatable.169 {column delete 10 } {
    list [catch {datatable0 column delete 10} msg] $msg
} {0 {}}

test datatable.170 {column names} {
    list [catch {datatable0 column names} msg] $msg
} {0 {c1 c2 c3 c4 c5 x6 x7 x8 x9 x11 x13 x14 x15 c16 c17}}

test datatable.171 {numcolumns} {
    list [catch {datatable0 numcolumns} msg] $msg
} {0 15}

test datatable.172 {column create} {
    list [catch {datatable0 column create} msg] $msg
} {0 15}

test datatable.173 {column names} {
    list [catch {datatable0 column names} msg] $msg
} {0 {c1 c2 c3 c4 c5 x6 x7 x8 x9 x11 x13 x14 x15 c16 c17 c18}}

test datatable.174 {column label end fred} {
    list [catch {
	datatable0 column label end fred
	datatable0 column names
    } msg] $msg
} {0 {c1 c2 c3 c4 c5 x6 x7 x8 x9 x11 x13 x14 x15 c16 c17 fred}}

test datatable.175 {column label end c18} {
    list [catch {
	datatable0 column label end c18
	datatable0 column names
    } msg] $msg
} {0 {c1 c2 c3 c4 c5 x6 x7 x8 x9 x11 x13 x14 x15 c16 c17 c18}}

test datatable.176 {column create -before 1 -badSwitch} {
    list [catch {datatable0 column create -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -after column
   -after row
   -before column
   -before row
   -label string
   -tags tags
   -type type}}

test datatable.177 {datatable0 column create -badSwitch -before 1} {
    list [catch {datatable0 column create -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -after column
   -after row
   -before column
   -before row
   -label string
   -tags tags
   -type type}}

test datatable.178 {datatable0 column create -before 1 -badSwitch arg} {
    list [catch {datatable0 column create -badSwitch arg} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -after column
   -after row
   -before column
   -before row
   -label string
   -tags tags
   -type type}}

test datatable.179 {datatable0 column create -before 1 -label nc1} {
    list [catch {datatable0 column create -before 1 -label nc1} msg] $msg
} {0 1}

test datatable.180 {datatable0 column create -before 2 -label nc2} {
    list [catch {datatable0 column create -before 2 -label nc2} msg] $msg
} {0 2}

test datatable.181 {datatable0 column create -after 2 -label nc3} {
    list [catch {datatable0 column create -after 2 -label nc3} msg] $msg
} {0 3}

test datatable.182 {datatable0 numcolumns} {
    list [catch {datatable0 numcolumns} msg] $msg
} {0 19}

test datatable.183 {datatable0 column index end} {
    list [catch {datatable0 column index end} msg] $msg
} {0 18}

test datatable.184 {datatable0 column names} {
    list [catch {datatable0 column names} msg] $msg
} {0 {c1 nc1 nc2 nc3 c2 c3 c4 c5 x6 x7 x8 x9 x11 x13 x14 x15 c16 c17 c18}}

test datatable.185 {datatable0 column create -after end} {
    list [catch {datatable0 column create -after end} msg] $msg
} {0 19}

test datatable.186 {datatable0 column create -after 0} {
    list [catch {datatable0 column create -after 0} msg] $msg
} {0 1}

test datatable.187 {datatable0 column create -label -one} {
    list [catch {datatable0 column create -label -one} msg] $msg
} {1 {column label "-one" can't start with a '-'.}}

test datatable.188 {datatable0 column create -label abc-one} {
    list [catch {datatable0 column create -label abc-one} msg] $msg
} {0 21}

test datatable.189 {datatable0 column create -label} {
    list [catch {datatable0 column create -label} msg] $msg
} {1 {value for "-label" missing}}


test datatable.190 {datatable0 column create -before -1} {
    list [catch {datatable0 column create -before -1} msg] $msg
} {1 {unknown column specification "-1" in ::datatable0}}

test datatable.191 {datatable0 numcolumns} {
    list [catch {datatable0 numcolumns} msg] $msg
} {0 22}

test datatable.192 {datatable0 column index fred} {
    list [catch {datatable0 column index fred} msg] $msg
} {0 -1}

test datatable.193 {datatable0 column index one} {
    list [catch {datatable0 column index one} msg] $msg
} {0 -1}

test datatable.194 {datatable0 column index end} {
    list [catch {datatable0 column index end} msg] $msg
} {0 21}

test datatable.195 {datatable0 column create -after 40} {
    list [catch {datatable0 column create -after 40} msg] $msg
} {1 {bad column index "40"}}

test datatable.196 {datatable0 column create -tags {myTag1 myTag2}} {
    list [catch {
	datatable0 column create -tags {myTag1 myTag2}
    } msg] $msg
} {0 22}

test datatable.197 {datatable0 column create -after end -tags {myTag1 myTag2}} {
    list [catch {
	datatable0 column create -after end -tags {myTag1 myTag2} 
    } msg] $msg
} {0 23}

test datatable.198 {datatable0 column tag indices myTag1 myTag2} {
    list [catch {
	datatable0 column tag indices myTag1 myTag2
    } msg] $msg
} {0 {22 23}}

test datatable.199 {datatable0 column tag indices myTag1 myTag2} {
    list [catch {
	datatable0 column tag indices myTag1 myTag2
    } msg] $msg
} {0 {22 23}}

test datatable.200 {blt::datatable create} {
    list [catch { blt::datatable create } msg] $msg
} {0 ::datatable1}

test datatable.201 {datatable1 numrows} {
    list [catch { datatable1 numrows } msg] $msg
} {0 0}

test datatable.202 {datatable1 numcolumns} {
    list [catch { datatable1 numrows } msg] $msg
} {0 0}

set orig [blt::datatable create]
$orig restore -data {
    i 5 6 0 0
    c 0 c1 string {}
    c 1 c2 string {}
    c 2 c3 string {}
    c 3 c4 string {}
    c 4 c5 string {}
    c 5 c6 string {}
    r 0 r1 {}
    r 1 r2 {}
    r 2 r3 {}
    r 3 r4 {}
    r 4 r5 {}
    d 0 0 1.0
    d 1 0 2.0
    d 2 0 3.0
    d 3 0 4.0
    d 4 0 5.0
    d 0 1 1.1
    d 1 1 2.1
    d 2 1 3.1
    d 3 1 4.1
    d 4 1 5.1
    d 0 2 1.2
    d 1 2 2.2
    d 2 2 3.2
    d 3 2 4.2
    d 4 2 5.2
    d 0 3 1.3
    d 1 3 2.3
    d 2 3 3.3
    d 3 3 4.3
    d 4 3 5.3
    d 0 4 1.4
    d 1 4 2.4
    d 2 4 3.4
    d 3 4 4.4
    d 4 4 5.4
    d 0 5 1.5
    d 1 5 2.5
    d 2 5 3.5
    d 3 5 4.5
    d 4 5 5.5
}

if 0 {
test datatable.203 {datatable1 dump} {
    list [catch {
	datatable1 import csv -file t2.csv
	datatable1 dump
    } msg] $msg
} {0 {i 5 6 0 0
c 0 c1 string {}
c 1 c2 string {}
c 2 c3 string {}
c 3 c4 string {}
c 4 c5 string {}
c 5 c6 string {}
r 0 r1 {}
r 1 r2 {}
r 2 r3 {}
r 3 r4 {}
r 4 r5 {}
d 0 0 1.0
d 1 0 2.0
d 2 0 3.0
d 3 0 4.0
d 4 0 5.0
d 0 1 1.1
d 1 1 2.1
d 2 1 3.1
d 3 1 4.1
d 4 1 5.1
d 0 2 1.2
d 1 2 2.2
d 2 2 3.2
d 3 2 4.2
d 4 2 5.2
d 0 3 1.3
d 1 3 2.3
d 2 3 3.3
d 3 3 4.3
d 4 3 5.3
d 0 4 1.4
d 1 4 2.4
d 2 4 3.4
d 3 4 4.4
d 4 4 5.4
d 0 5 1.5
d 1 5 2.5
d 2 5 3.5
d 3 5 4.5
d 4 5 5.5
}}

}
test datatable.204 {datatable1 restore -data} {
    list [catch {
	set table [blt::datatable create]
	$table restore -data {
	    i 5 6 0 0
	    c 0 c1 string {}
	    c 1 c2 string {}
	    c 2 c3 string {}
	    c 3 c4 string {}
	    c 4 c5 string {}
	    c 5 c6 string {}
	    r 0 r1 {}
	    r 1 r2 {}
	    r 2 r3 {}
	    r 3 r4 {}
	    r 4 r5 {}
	    d 0 0 1.0
	    d 1 0 2.0
	    d 2 0 3.0
	    d 3 0 4.0
	    d 4 0 5.0
	    d 0 1 1.1
	    d 1 1 2.1
	    d 2 1 3.1
	    d 3 1 4.1
	    d 4 1 5.1
	    d 0 2 1.2
	    d 1 2 2.2
	    d 2 2 3.2
	    d 3 2 4.2
	    d 4 2 5.2
	    d 0 3 1.3
	    d 1 3 2.3
	    d 2 3 3.3
	    d 3 3 4.3
	    d 4 3 5.3
	    d 0 4 1.4
	    d 1 4 2.4
	    d 2 4 3.4
	    d 3 4 4.4
	    d 4 4 5.4
	    d 0 5 1.5
	    d 1 5 2.5
	    d 2 5 3.5
	    d 3 5 4.5
	    d 4 5 5.5
	}
    } msg] $msg
} {0 {}}

test datatable.205 {datatable0 column move} {
    list [catch {datatable0 column move} msg] $msg
} {1 {wrong # args: should be "datatable0 column move from to ?count?"}}

test datatable.206 {datatable0 column move -1} {
    list [catch {datatable0 column move -1} msg] $msg
} {1 {wrong # args: should be "datatable0 column move from to ?count?"}}

test datatable.207 {datatable0 column move 50 50} {
    list [catch {datatable0 column move 50 50} msg] $msg
} {1 {bad column index "50"}}

test datatable.208 {datatable0 column move 10 50} {
    list [catch {datatable0 column move 10 50} msg] $msg
} {1 {bad column index "50"}}

test datatable.209 {datatable0 column move all 10} {
    list [catch {datatable0 column move all 10} msg] $msg
} {1 {multiple columns specified by "all"}}

test datatable.210 {column label} {
    list [catch {
	set nCols [datatable0 numcolumns]
	for { set i 0 } { $i < $nCols } { incr i } {
	    datatable0 column label $i c$i
	}
    } msg] $msg
} {0 {}}

test datatable.211 {duplicate} {
    list [catch { 
	set before [$orig column names]
	set dup [$orig duplicate]
	set after [$dup column names]
	blt::datatable destroy $dup
	expr { $before == $after }
    } msg] $msg
} {0 1}

test datatable.212 {duplicate} {
    list [catch { datatable1 duplicate datatable1 } msg] $msg
} {0 {}}

test datatable.213 {column names} {
    list [catch { $orig column names } msg] $msg
} {0 {c1 c2 c3 c4 c5 c6}}

test datatable.214 {row names} {
    list [catch { $orig row names } msg] $msg
} {0 {r1 r2 r3 r4 r5}}

test datatable.215 {duplicate} {
    list [catch { 
	set before [datatable1 column names]
	datatable1 duplicate datatable1 
	set after [datatable1 column names]
	expr { $before == $after }
    } msg] $msg
} {0 1}

test datatable.216 {duplicate} {
    list [catch { 
	set dup [$orig duplicate]
	set list [$dup column names]
	blt::datatable destroy $dup
	set list
    } msg] $msg
} {0 {c1 c2 c3 c4 c5 c6}}

test datatable.217 {duplicate} {
    list [catch { 
	datatable1 duplicate $orig
	datatable1 numcolumns
    } msg] $msg
} {0 6}

test datatable.218 {duplicate} {
    list [catch { 
	datatable1 duplicate $orig
	datatable1 numrows
    } msg] $msg
} {0 5}


test datatable.219 {duplicate} {
    list [catch { 
	datatable1 duplicate $orig
	datatable1 column names
    } msg] $msg
} {0 {c1 c2 c3 c4 c5 c6}}

test datatable.220 {datatable1 column move 0 9  0} {
    list [catch { 
	# This should be a no-op.
	datatable1 duplicate $orig
	set before [datatable1 column names]
	datatable0 column move 0 9 0
	set after [datatable1 column names]
	expr {$before == $after}
    } msg] $msg
} {0 1}

test datatable.221 {column move 0 0} {
    list [catch { 
	# This should be a no-op.
	datatable1 duplicate $orig
	set before [datatable1 column names]
	datatable0 column move 0 0
	set after [datatable1 column names]
	expr {$before == $after}
    } msg] $msg
} {0 1}

test datatable.222 {duplicate} {
    list [catch { 
	set before [$orig column names]
	set dup [$orig duplicate]
	set after [$dup column names]
	blt::datatable destroy $dup
	expr { $before == $after }
    } msg] $msg
} {0 1}

test datatable.223 {duplicate} {
    list [catch { datatable1 duplicate datatable1 } msg] $msg
} {0 {}}

test datatable.224 {column names} {
    list [catch { $orig column names } msg] $msg
} {0 {c1 c2 c3 c4 c5 c6}}

test datatable.225 {row names} {
    list [catch { $orig row names } msg] $msg
} {0 {r1 r2 r3 r4 r5}}

test datatable.226 {duplicate} {
    list [catch { 
	set before [datatable1 column names]
	datatable1 duplicate datatable1 
	set after [datatable1 column names]
	expr { $before == $after }
    } msg] $msg
} {0 1}

test datatable.227 {duplicate} {
    list [catch { 
	set dup [$orig duplicate]
	set list [$dup column names]
	blt::datatable destroy $dup
	set list
    } msg] $msg
} {0 {c1 c2 c3 c4 c5 c6}}

test datatable.228 {duplicate} {
    list [catch { 
	datatable1 duplicate $orig
	datatable1 numcolumns
    } msg] $msg
} {0 6}

test datatable.229 {duplicate} {
    list [catch { 
	datatable1 duplicate $orig
	datatable1 numrows
    } msg] $msg
} {0 5}


test datatable.230 {duplicate} {
    list [catch { 
	datatable1 duplicate $orig
	datatable1 column names
    } msg] $msg
} {0 {c1 c2 c3 c4 c5 c6}}


test datatable.231 {duplicate} {
    list [catch { 
	datatable1 duplicate $orig
	datatable1 column names
    } msg] $msg
} {0 {c1 c2 c3 c4 c5 c6}}

test datatable.232 {column move 0 2} {
    list [catch {
	datatable1 duplicate $orig
	datatable1 column move 0 2
	datatable1 column names
    } msg] $msg
} {0 {c2 c3 c1 c4 c5 c6}}

test datatable.233 {datatable0 column move -1 2} {
    list [catch {datatable0 column move -1 2} msg] $msg
} {1 {unknown column specification "-1" in ::datatable0}}

test datatable.234 {datatable0 column move 0 end} {
    list [catch {
	datatable1 duplicate $orig
	datatable1 column move 0 end
	datatable1 column names
    } msg] $msg
} {0 {c2 c3 c4 c5 c6 c1}}

test datatable.235 {find expr} {
    list [catch {
	datatable1 find { $c1 > 3.1 }
    } msg] $msg
} {0 {3 4}}

test datatable.236 {find expr} {
    list [catch {
	datatable1 find { $0 > 3.1 }
    } msg] $msg
} {0 {3 4}}

test datatable.237 {column values} {
    list [catch {
	datatable1 column values c1
    } msg] $msg
} {0 {1.0 2.0 3.0 4.0 5.0}}

test datatable.237 {column label all sameLabel} {
    list [catch {
	datatable1 column label 0 sameLabel
	datatable1 column label 1 sameLabel
	datatable1 column label 2 sameLabel
    } msg] $msg
} {0 {}}

test datatable.237 {column indices sameLabel} {
    list [catch {
	datatable1 column indices sameLabel
    } msg] [lsort $msg]
} {0 {0 1 2}}

#exit 0
test datatable.238 {datatable0 column trace} {
    list [catch {datatable0 column trace} msg] $msg
} {1 {wrong # args: should be "datatable0 column trace column how command"}}

test datatable.239 {column trace all} {
    list [catch {datatable0 column trace all} msg] $msg
} {1 {wrong # args: should be "datatable0 column trace column how command"}}
    
test datatable.240 {column trace end} {
    list [catch {datatable0 column trace end} msg] $msg
} {1 {wrong # args: should be "datatable0 column trace column how command"}}

test datatable.241 {column trace 1} {
    list [catch {datatable0 column trace 1} msg] $msg
} {1 {wrong # args: should be "datatable0 column trace column how command"}}

test datatable.242 {column trace 1 rwuc} {
    list [catch {datatable0 column trace 1 rwuc} msg] $msg
} {1 {wrong # args: should be "datatable0 column trace column how command"}}

test datatable.243 {column trace all rwuc} {
    list [catch {datatable0 column trace all rwuc} msg] $msg
} {1 {wrong # args: should be "datatable0 column trace column how command"}}

proc Doit { args } { 
    global mylist; lappend mylist $args 
}

test datatable.244 {column trace all rwuc Doit} {
    list [catch {datatable1 column trace all rwuc Doit} msg] $msg
} {0 trace0}

test datatable.245 {trace info trace0} {
    list [catch {datatable1 trace info trace0} msg] $msg
} {0 {id trace0 column all flags rwuc command Doit}}

test datatable.246 {test create trace} {
    list [catch {
	set mylist {}
	datatable1 set all 0 20
	set mylist
    } msg] $msg
} {0 {{::datatable1 0 0 w} {::datatable1 1 0 w} {::datatable1 2 0 w} {::datatable1 3 0 w} {::datatable1 4 0 w}}}

test datatable.247 {test read trace} {
    list [catch {
	set mylist {}
	datatable1 column get 0
	set mylist
	} msg] $msg
} {0 {{::datatable1 0 0 r} {::datatable1 1 0 r} {::datatable1 2 0 r} {::datatable1 3 0 r} {::datatable1 4 0 r}}}

test datatable.248 {test write trace} {
    list [catch {
	set mylist {}
	datatable1 column set 1 1 a 2 b 3 c 4 d 5 e
	set mylist
	} msg] $msg
} {0 {{::datatable1 1 1 w} {::datatable1 2 1 w} {::datatable1 3 1 w} {::datatable1 4 1 w} {::datatable1 5 1 wc}}}

test datatable.249 {test unset trace} {
    list [catch {
	set mylist {}
	datatable1 column unset 0 all
	set mylist
	} msg] $msg
} {0 {{::datatable1 0 0 u} {::datatable1 1 0 u} {::datatable1 2 0 u} {::datatable1 3 0 u} {::datatable1 4 0 u}}}

test datatable.250 {trace delete} {
    list [catch {datatable1 trace delete} msg] $msg
} {0 {}}

if 0 {
puts stderr [datatable0 column type all double]
datatable0 column set 4 1 1.0 2 2.0 3 3.0 4 4.0 5 5.0
datatable0 column set 5 1 1.0 2 2.0 3 3.0 4 4.0 5 5.0
datatable0 column set 6 1 1.0 2 2.0 3 3.0 4 4.0 5 5.0
puts stderr [datatable0 export csv]
puts stderr [datatable0 export xml]
puts stderr 1,1=[datatable0 get 1 1]
}
test datatable.251 {trace delete badId} {
    list [catch {datatable1 trace delete badId} msg] $msg
} {1 {unknown trace "badId"}}

test datatable.252 {trace delete trace0} {
    list [catch {datatable1 trace delete trace0} msg] $msg
} {0 {}}

test datatable.253 {export -file} {
    list [catch {datatable1 export -file} msg] $msg
} {1 {can't export "-file": format not registered}}

test datatable.254 {export csv -file} {
    list [catch {datatable1 export csv -file} msg] $msg
} {1 {value for "-file" missing}}

test datatable.255 {exportfile csv -file /badDir/badFile } {
    list [catch {datatable1 export csv -file /badDir/badFile} msg] $msg
} {1 {couldn't open "/badDir/badFile": no such file or directory}}

test datatable.256 {exportfile csv -file @badChannel } {
    list [catch {datatable1 export csv -file @badChannel} msg] $msg
} {1 {can not find channel named "badChannel"}}

test datatable.257 {export csv -file table.csv} {
    list [catch {datatable1 export csv -file table.csv} msg] $msg
} {0 {}}

test datatable.258 {dup} {
    list [catch {datatable1 duplicate $orig} msg] $msg
} {0 {}}

test datatable.259 {column type all double} {
    list [catch {datatable1 column type all double} msg] $msg
} {0 {double double double double double double}}

test datatable.260 {export csv -rowlabels -columnlabels} {
    list [catch {datatable1 export csv -rowlabels -columnlabels} msg] $msg
} {0 {"*BLT*","c1","c2","c3","c4","c5","c6"
"r57",1.0,1.1,1.2,1.3,1.4,1.5
"r58",2.0,2.1,2.2,2.3,2.4,2.5
"r59",3.0,3.1,3.2,3.3,3.4,3.5
"r60",4.0,4.1,4.2,4.3,4.4,4.5
"r61",5.0,5.1,5.2,5.3,5.4,5.5
}}

test datatable.261 {column type all string} {
    list [catch {datatable0 column type all string} msg] $msg
} {0 {string string string string string string string string string string string string string string string string string string string string string string string string}}


test datatable.262 {dump -file table.dump} {
    list [catch {datatable0 dump -file table.dump} msg] $msg
} {0 {}}

test datatable.263 {datatable0 set (no args)} {
    list [catch { datatable0 set } msg] $msg
} {1 {wrong # args: should be "datatable0 set ?row column value?..."}}

test datatable.264 {datatable0 set 1 (no column)} {
    list [catch { datatable0 set 1 } msg] $msg
} {1 {wrong # args: should be "datatable0 set ?row column value?..."}}

test datatable.265 {datatable0 set 1 1 (no value)} {
    list [catch { datatable0 set 1 1 } msg] $msg
} {1 {wrong # args: should be "datatable0 set ?row column value?..."}}

test datatable.266 {datatable0 set 1 1 1.0 3.0 (too many args)} {
    list [catch { datatable0 set 1 1 1.0 3.0} msg] $msg
} {1 {wrong # args: should be "datatable0 set ?row column value?..."}}

test datatable.267 {datatable0 set 1 1 1.0} {
    list [catch { datatable0 set 1 1 1.0 }  msg] $msg
} {0 {}}

test datatable.268 {datatable0 get 1 1} {
    list [catch { datatable0 get 1 1 }  msg] $msg
} {0 1.0}

test datatable.269 {datatable0 row exists newRow} {
    list [catch { datatable0 row exists newRow } msg] $msg
} {0 0}

test datatable.270 {datatable0 set newRow 1 abc} {
    list [catch { datatable0 set newRow 1 abc } msg] $msg
} {0 {}}

test datatable.271 {datatable0 get newRow 1} {
    list [catch { datatable0 get newRow 1 } msg] $msg
} {0 abc}

test datatable.272 {datatable0 row exists newRow} {
    list [catch { datatable0 row exists newRow } msg] $msg
} {0 1}

test datatable.273 {datatable0 column exists newColumn} {
    list [catch { datatable0 column exists newColumn } msg] $msg
} {0 0}

test datatable.274 {datatable0 set 1 newColumn def} {
    list [catch { datatable0 set 1 newColumn def } msg] $msg
} {0 {}}

test datatable.275 {datatable0 get 1 newColumn} {
    list [catch { datatable0 get 1 newColumn } msg] $msg
} {0 def}

test datatable.276 {datatable0 row delete newRow} {
    list [catch { datatable0 row delete newRow } msg] $msg
} {0 {}}

test datatable.277 {datatable0 row exists newRow} {
    list [catch { datatable0 row exists newRow } msg] $msg
} {0 0}

test datatable.278 {datatable0 column delete newColumn} {
    list [catch { datatable0 column delete newColumn } msg] $msg
} {0 {}}

test datatable.279 {datatable0 column exists newColumn} {
    list [catch { datatable0 row exists newRow } msg] $msg
} {0 0}

test datatable.280 {datatable0 set newRow newColumn abc} {
    list [catch { datatable0 set newRow newColumn abc } msg] $msg
} {0 {}}

test datatable.281 {datatable0 row delete newRow} {
    list [catch { datatable0 row delete newRow } msg] $msg
} {0 {}}

test datatable.282 {datatable0 column delete newColumn} {
    list [catch { datatable0 column delete newColumn } msg] $msg
} {0 {}}

test datatable.283 {datatable0 column type 1 double} {
    list [catch { datatable0 column type 1 double } msg] $msg
} {0 double}

test datatable.284 {datatable0 set 1 1 1.0} {
    list [catch { 
	datatable0 set 1 1 1.0 
	datatable0 get 1 1
    } msg] $msg
} {0 1.0}

test datatable.285 {datatable0 set end 1 1.0 end 2 2.0 end 3 3.0 end 4 4.0 } {
    list [catch { 
	datatable0 set end 1 1.0  end 2 2.0 end 3 3.0 end 4 4.0 
    } msg] $msg
} {0 {}}

test datatable.286 {datatable0 set 6 1 1.0 6 } {
    list [catch { 
	datatable0 set 6 1 1.0  6
    } msg] $msg
} {1 {wrong # args: should be "datatable0 set ?row column value?..."}}

test datatable.287 {datatable0 set 1 1 abc} {
    list [catch { datatable0 set 1 1 abc } msg] $msg
} {1 {expected floating-point number but got "abc"}}

test datatable.288 {datatable0 column type 1 string} {
    list [catch { datatable0 column type 1 string } msg] $msg
} {0 string}

test datatable.289 {datatable0 unset 1 1} {
    list [catch { datatable0 unset 1 1} msg] $msg
} {0 {}}

test datatable.290 {datatable0 append 1 1 abc (from empty)} {
    list [catch { datatable0 append 1 1 abc} msg] $msg
} {0 {}}

test datatable.291 {datatable0 get 1 1} {
    list [catch { datatable0 get 1 1 } msg] $msg
} {0 abc}

test datatable.292 {datatable0 append 1 1 def (from empty)} {
    list [catch { datatable0 append 1 1 def} msg] $msg
} {0 {}}

test datatable.293 {datatable0 get 1 1} {
    list [catch { datatable0 get 1 1 } msg] $msg
} {0 abcdef}

test datatable.294 {datatable0 get 1 2 defValue } {
    list [catch { datatable0 get 2 1 defValue } msg] $msg
} {0 defValue}

test datatable.295 {datatable0 get 1 2 defValue (too many args) } {
    list [catch { datatable0 get 2 1 defValue extraArg } msg] $msg
} {1 {wrong # args: should be "datatable0 get row column ?defValue?"}}

test datatable.296 {datatable0 append 1 1 123 456 789 (from empty)} {
    list [catch { datatable0 append 1 1 123 456 789 } msg] $msg
} {0 {}}

test datatable.297 {datatable0 get 1 1} {
    list [catch { datatable0 get 1 1 } msg] $msg
} {0 abcdef123456789}

test datatable.298 {datatable0 unset 1 1} {
    list [catch { datatable0 unset 1 1} msg] $msg
} {0 {}}

test datatable.299 {datatable0 unset 1 1} {
    list [catch { datatable0 unset 1 1} msg] $msg
} {0 {}}

test datatable.300 {datatable0 unset 1 1 1 } {
    list [catch { datatable0 unset 1 1 1 } msg] $msg
} {1 {wrong # args: should be "datatable0 unset ?row column?...}}

test datatable.301 {datatable0 unset end 1 end 2 end 3 end 4 } {
    list [catch { datatable0 unset end 1  end 2 end 3 end 4 } msg] $msg
} {0 {}}

test datatable.302 {datatable0 unset 0 0 } {
    list [catch { datatable0 unset 0 0 } msg] $msg
} {0 {}}

test datatable.303 {datatable0 unset 10000 10000 } {
    list [catch { datatable0 unset 10000 10000 } msg] $msg
} {0 {}}

test datatable.304 {datatable0 unset 10000 10000 } {
    list [catch { datatable0 unset 10000 10000 } msg] $msg
} {0 {}}

#---------------------

test datatable.305 {blt::datatable create} {
    list [catch {blt::datatable create} msg] $msg
} {0 ::datatable4}

test datatable.306 {column extend 5} {
    list [catch {datatable4 column extend 5} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.307 {numrows} {
    list [catch {datatable4 numrows} msg] $msg
} {0 0}

test datatable.308 {numrows -10} {
    list [catch {datatable4 numrows -10} msg] $msg
} {1 {bad count "-10": # columns can't be negative.}}

test datatable.309 {numrows badArg} {
    list [catch {datatable4 numrows badArg} msg] $msg
} {1 {expected integer but got "badArg"}}

test datatable.310 {numrows 10} {
    list [catch {datatable4 numrows 10} msg] $msg
} {0 10}

test datatable.311 {numcolumns 10} {
    list [catch {datatable4 numrows 10} msg] $msg
} {0 10}

test datatable.312 {numrows 0} {
    list [catch {datatable4 numrows 0} msg] $msg
} {0 0}

test datatable.313 {numcolumns 0} {
    list [catch {datatable4 numcolumns 0} msg] $msg
} {0 0}

test datatable.314 {column names} {
    list [catch {datatable4 column names} msg] $msg
} {0 {}}

test datatable.315 {row names} {
    list [catch {datatable4 row names} msg] $msg
} {0 {}}

test datatable.316 {row -label xyz create} {
    list [catch {datatable4 row -label xyz create} msg] $msg
} {1 {bad operation "-label": should be one of...
  datatable4 row copy src dest ?switches...?
  datatable4 row count ?row?
  datatable4 row create ?switches...?
  datatable4 row delete ?row...?
  datatable4 row duplicate ?row...?
  datatable4 row exists row
  datatable4 row extend label ?label...?
  datatable4 row get row ?switches?
  datatable4 row index row
  datatable4 row indices row ?row...?
  datatable4 row join table ?switches?
  datatable4 row label row ?label?
  datatable4 row labels ?labelList?
  datatable4 row move from to ?count?
  datatable4 row names ?pattern...?
  datatable4 row notify row ?flags? command
  datatable4 row set row column value...
  datatable4 row tag op args...
  datatable4 row trace row how command
  datatable4 row unset row ?indices...?
  datatable4 row values row ?valueList?}}

test datatable.317 {row extend 5} {
    list [catch {datatable4 row extend 5} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.318 {numrows} {
    list [catch {datatable4 numrows} msg] $msg
} {0 5}

test datatable.319 {row index end} {
    list [catch {datatable4 row index end} msg] $msg
} {0 4}

test datatable.320 {row indices all} {
    list [catch {datatable1 row indices all} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.321 {row indices 0-end} {
    list [catch {datatable1 row indices "0-end" } msg] $msg
} {0 {0 1 2 3 4}}

test datatable.322 {row indices 1-all} {
    list [catch {datatable1 row indices "0-all" } msg] $msg
} {1 {unknown row specification "0-all" in ::datatable1}}

test datatable.323 {row indices 2-4} {
    list [catch {datatable1 row indices 2-4} msg] $msg
} {0 {2 3 4}}

test datatable.324 {row indices 2-5} {
    list [catch {datatable1 row indices 2-5} msg] $msg
} {1 {unknown row specification "2-5" in ::datatable1}}

test datatable.325 {row indices 5-2} {
    list [catch {datatable1 row indices 5-2} msg] $msg
} {1 {unknown row specification "5-2" in ::datatable1}}

test datatable.326 {row indices 4-2} {
    list [catch {datatable1 row indices 4-2} msg] $msg
} {0 {}}

test datatable.327 {row index end} {
    list [catch {datatable1 row index end} msg] $msg
} {0 4}

test datatable.328 {blt::datatable create} {
    list [catch {datatable4 duplicate $orig} msg] $msg
} {0 {}}


test datatable.329 {row index end badArg} {
    list [catch {datatable1 row index end badArg} msg] $msg
} {1 {wrong # args: should be "datatable1 row index row"}}

test datatable.330 {row label 0} {
    list [catch {datatable1 row label 0} msg] $msg
} {0 r57}

test datatable.331 {row label 0 myLabel} {
    list [catch {datatable1 row label 0 myLabel} msg] $msg
} {0 {}}

test datatable.332 {row label 0} {
    list [catch {datatable1 row label 0} msg] $msg
} {0 myLabel}

test datatable.333 {row label 1 myLabel} {
    list [catch {datatable1 row label 1 myLabel} msg] $msg
} {0 {}}

test datatable.334 {row label 0} {
    list [catch {datatable1 row label 0} msg] $msg
} {0 myLabel}

test datatable.335 {row label end end} {
    list [catch {datatable1 row label end end} msg] $msg
} {0 {}}

test datatable.336 {row label end endLabel} {
    list [catch {datatable1 row label end endLabel} msg] $msg
} {0 {}}

test datatable.337 {row label end 1abc} {
    list [catch {datatable1 row label end 1abc} msg] $msg
} {0 {}}

test datatable.338 {row label end label-with-minus} {
    list [catch {datatable1 row label end label-with-minus} msg] $msg
} {0 {}}

test datatable.339 {row label end -abc} {
    list [catch {datatable1 row label end -abc} msg] $msg
} {1 {row label "-abc" can't start with a '-'.}}

test datatable.340 {row names *Label} {
    list [catch {datatable1 row names *Label} msg] $msg
} {0 {myLabel myLabel}}

test datatable.341 {row names} {
    list [catch {datatable1 row names} msg] $msg
} {0 {myLabel myLabel r59 r60 label-with-minus}}

test datatable.342 {row names r*} {
    list [catch {datatable1 row names r*} msg] $msg
} {0 {r59 r60}}

test datatable.343 {row names *-with-*} {
    list [catch {datatable1 row names *-with-*} msg] $msg
} {0 label-with-minus}

test datatable.344 {datatable1 row names badPattern} {
    list [catch {datatable1 row names badPattern} msg] $msg
} {0 {}}

test datatable.345 {datatable1 row get myLabel} {
    list [catch {datatable1 row get myLabel} msg] $msg
} {1 {multiple rows specified by "myLabel"}}

test datatable.346 {datatable1 row get} {
    list [catch {datatable1 row get} msg] $msg
} {1 {wrong # args: should be "datatable1 row get row ?switches?"}}

test datatable.347 {datatable1 row set myLabel} {
    list [catch {datatable1 row set myLabel} msg] $msg
} {1 {wrong # args: should be "datatable1 row set row column value..."}}

test datatable.348 {row set all 1 1.0 2 2.0 3 3.0 4 4.0 5 5.0} {
    list [catch {
	datatable1 row set all 1 1.0 2 2.0 3 3.0 4 4.0 5 5.0
    } msg] $msg
} {0 {}}

test datatable.349 {datatable1 row values 1} {
    list [catch {datatable1 row values 1} msg] $msg
} {0 {2.0 1.0 2.0 3.0 4.0 5.0}}

test datatable.350 {datatable1 row values all} {
    list [catch {datatable1 row values all} msg] $msg
} {1 {multiple rows specified by "all"}}

test datatable.351 {datatable1 row values 1-2} {
    list [catch {datatable1 row values 1-2} msg] $msg
} {1 {multiple rows specified by "1-2"}}

test datatable.352 {datatable1 row values 2} {
    list [catch {datatable1 row values 2} msg] $msg
} {0 {3.0 1.0 2.0 3.0 4.0 5.0}}

test datatable.353 {datatable1 row values 3} {
    list [catch {datatable1 row values 3} msg] $msg
} {0 {4.0 1.0 2.0 3.0 4.0 5.0}}

test datatable.354 {datatable1 row values end} {
    list [catch {datatable1 row values end} msg] $msg
} {0 {5.0 1.0 2.0 3.0 4.0 5.0}}

datatable1 duplicate $orig
datatable1 column type 0 double
test datatable.355 {datatable1 row values 0 { a b c }} {
    list [catch {datatable1 row values 0 { a b c }} msg] $msg
} {1 {expected floating-point number but got "a"}}

test datatable.356 {datatable1 row values 0 { 11.1 12.2 13.3 }} {
    list [catch {datatable1 row values 0 { 11.1 12.2 13.3 }} msg] $msg
} {0 {}}

test datatable.357 {datatable1 row values 1} {
    list [catch {datatable1 row values 1} msg] $msg
} {0 {2.0 2.1 2.2 2.3 2.4 2.5}}

test datatable.358 {datatable1 row values end { -1 -2 }} {
    list [catch {datatable1 row values end { -1 -2 }} msg] $msg
} {0 {}}

test datatable.359 {datatable1 row values end} {
    list [catch {datatable1 row values end} msg] $msg
} {0 {-1.0 -2 5.2 5.3 5.4 5.5}}

test datatable.360 {datatable1 row index label-with-minus} {
    list [catch {datatable1 row index label-with-minus} msg] $msg
} {0 -1}

test datatable.361 {datatable1 row indices all} {
    list [catch {datatable1 row indices all} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.362 {datatable1 row index -1} {
    list [catch {datatable1 row index -1} msg] $msg
} {0 -1}

test datatable.363 {datatable1 row index 1000} {
    list [catch {datatable1 row index 1000} msg] $msg
} {0 -1}

test datatable.364 {datatable1 row tag badOp} {
    list [catch {datatable1 row tag badOp} msg] $msg
} {1 {bad tag operation "badOp": should be one of...
  datatable1 row tag add tag ?row...?
  datatable1 row tag delete tag ?row...?
  datatable1 row tag exists tag ?row?
  datatable1 row tag forget ?tag...?
  datatable1 row tag get row ?pattern...?
  datatable1 row tag indices ?tag...?
  datatable1 row tag labels ?tag...?
  datatable1 row tag range from to ?tag...?
  datatable1 row tag search row ?pattern?
  datatable1 row tag set row tag...
  datatable1 row tag unset row tag...}}

test datatable.365 {datatable1 row tag (missing args)} {
    list [catch {datatable1 row tag} msg] $msg
} {1 {wrong # args: should be one of...
  datatable1 row tag add tag ?row...?
  datatable1 row tag delete tag ?row...?
  datatable1 row tag exists tag ?row?
  datatable1 row tag forget ?tag...?
  datatable1 row tag get row ?pattern...?
  datatable1 row tag indices ?tag...?
  datatable1 row tag labels ?tag...?
  datatable1 row tag range from to ?tag...?
  datatable1 row tag search row ?pattern?
  datatable1 row tag set row tag...
  datatable1 row tag unset row tag...}}

test datatable.366 {datatable1 row tag badOp} {
    list [catch {datatable1 row tag badOp} msg] $msg
} {1 {bad tag operation "badOp": should be one of...
  datatable1 row tag add tag ?row...?
  datatable1 row tag delete tag ?row...?
  datatable1 row tag exists tag ?row?
  datatable1 row tag forget ?tag...?
  datatable1 row tag get row ?pattern...?
  datatable1 row tag indices ?tag...?
  datatable1 row tag labels ?tag...?
  datatable1 row tag range from to ?tag...?
  datatable1 row tag search row ?pattern?
  datatable1 row tag set row tag...
  datatable1 row tag unset row tag...}}

test datatable.367 {datatable1 row tag add} {
    list [catch {datatable1 row tag add} msg] $msg
} {1 {wrong # args: should be "datatable1 row tag add tag ?row...?"}}

test datatable.368 {datatable1 row tag add 1} {
    list [catch {datatable1 row tag add 1} msg] $msg
} {1 {tag "1" can't be a number.}}

test datatable.369 {datatable1 row tag add tag badIndex} {
    list [catch {datatable1 row tag add tag badIndex} msg] $msg
} {1 {unknown row specification "badIndex" in ::datatable1}}

test datatable.370 {datatable1 row tag add newTag 1} {
    list [catch {datatable1 row tag add newTag 1} msg] $msg
} {0 {}}

test datatable.371 {datatable1 row tag add newTag1 1} {
    list [catch {datatable1 row tag add newTag1 1} msg] $msg
} {0 {}}

test datatable.372 {datatable1 row tag add newTag2 1} {
    list [catch {datatable1 row tag add newTag2 1} msg] $msg
} {0 {}}

test datatable.373 {datatable1 row tag search} {
    list [catch {datatable1 row tag search} msg] $msg
} {1 {wrong # args: should be "datatable1 row tag search row ?pattern?"}}

test datatable.374 {datatable1 row tag search 1} {
    list [catch {datatable1 row tag search 1} msg] [lsort $msg]
} {0 {all newTag newTag1 newTag2}}

test datatable.375 {datatable1 row tag search 1*Tag*} {
    list [catch {datatable1 row tag search 1 *Tag*} msg] [lsort $msg]
} {0 {newTag newTag1 newTag2}}

test datatable.376 {datatable1 row tag search all} {
    list [catch {datatable1 row tag search all} msg] $msg
} {0 {newTag2 newTag newTag1 all end}}

test datatable.377 {datatable1 row tag search end} {
    list [catch {datatable1 row tag search end} msg] $msg
} {0 {all end}}

test datatable.378 {datatable1 row tag search end end} {
    list [catch {datatable1 row tag search end end} msg] $msg
} {0 end}

test datatable.379 {datatable1 row tag search end e*} {
    list [catch {datatable1 row tag search end e*} msg] $msg
} {0 end}

test datatable.380 {datatable1 row tag delete} {
    list [catch {datatable1 row tag delete} msg] $msg
} {1 {wrong # args: should be "datatable1 row tag delete tag ?row...?"}}

test datatable.381 {datatable1 row tag delete someTag} {
    list [catch {datatable1 row tag delete someTag} msg] $msg
} {0 {}}

test datatable.382 {datatable1 row tag delete newTag1 1} {
    list [catch {datatable1 row tag delete newTag1 1} msg] $msg
} {0 {}}

test datatable.383 {datatable1 row tag delete newTag1 1} {
    list [catch {datatable1 row tag delete newTag1 1} msg] $msg
} {0 {}}

test datatable.384 {datatable1 row tag delete newTag2 1} {
    list [catch {datatable1 row tag delete newTag2 1} msg] $msg
} {0 {}}

test datatable.385 {datatable1 row tag delete badTag 1} {
    list [catch {datatable1 row tag delete badTag 1} msg] $msg
} {1 {unknown row tag "badTag"}}

test datatable.386 {datatable1 row tag delete someTag 1000} {
    list [catch {datatable1 row tag delete someTag 1000} msg] $msg
} {1 {bad row index "1000"}}

test datatable.387 {datatable1 row tag delete end 1} {
    list [catch {datatable1 row tag delete end 1} msg] $msg
} {0 {}}

test datatable.388 {datatable1 row tag delete all 1} {
    list [catch {datatable1 row tag delete all 1} msg] $msg
} {0 {}}

test datatable.389 {datatable1 row tag forget} {
    list [catch {datatable1 row tag forget} msg] $msg
} {0 {}}

test datatable.390 {row tag forget all} {
    list [catch {datatable1 row tag forget all} msg] $msg
} {0 {}}

test datatable.391 {row tag forget newTag1} {
    list [catch {datatable1 row tag forget newTag1} msg] $msg
} {0 {}}

test datatable.392 {row tag forget newTag1} {
    list [catch {datatable1 row tag forget newTag1} msg] $msg
} {1 {unknown row tag "newTag1"}}

test datatable.393 {row tag indices} {
    list [catch {datatable1 row tag indices} msg] $msg
} {0 {}}

test datatable.394 {row tag indices all} {
    list [catch {datatable1 row tag indices all} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.395 {row tag indices end} {
    list [catch {datatable1 row tag indices end} msg] $msg
} {0 4}

test datatable.396 {row tag indices newTag} {
    list [catch {datatable1 row tag indices newTag} msg] $msg
} {0 1}

test datatable.397 {row tag range 1 3 midTag} {
    list [catch {datatable1 row tag range 1 3 midTag} msg] $msg
} {0 {}}

test datatable.398 {row tag indices midTag} {
    list [catch {datatable1 row tag indices midTag} msg] $msg
} {0 {1 2 3}}

test datatable.399 {row tag range 0 end myTag} {
    list [catch {datatable1 row tag range 0 end myTag} msg] $msg
} {0 {}}

test datatable.400 {row tag indices myTag} {
    list [catch {datatable1 row tag indices myTag} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.401 {row tag range -1 0 myTag} {
    list [catch {datatable1 row tag range -1 0 myTag} msg] $msg
} {1 {unknown row specification "-1" in ::datatable1}}

test datatable.402 {row tag range 0 -1 myTag} {
    list [catch {datatable1 row tag range 0 -1 myTag} msg] $msg
} {1 {unknown row specification "-1" in ::datatable1}}

test datatable.403 {row tag range 0 1000 myTag} {
    list [catch {datatable1 row tag range 0 1000 myTag} msg] $msg
} {1 {bad row index "1000"}}

test datatable.404 {row unset} {
    list [catch {datatable1 row unset} msg] $msg
} {1 {wrong # args: should be "datatable1 row unset row ?indices...?"}}

test datatable.405 {row unset 0 all} {
    list [catch {datatable1 row unset 0 all} msg] $msg
} {0 {}}

test datatable.406 {row values 0} {
    list [catch {datatable1 row values 0} msg] $msg
} {0 {{} {} {} {} {} {}}}

test datatable.407 {row get 0} {
    list [catch {datatable1 row values 0} msg] $msg
} {0 {{} {} {} {} {} {}}}

test datatable.408 {dump } {
    list [catch {datatable1 dump} msg] $msg
} {0 {i 5 6 0 0
c 0 c1 double {}
c 1 c2 string {}
c 2 c3 string {}
c 3 c4 string {}
c 4 c5 string {}
c 5 c6 string {}
r 0 r62 {myTag}
r 1 r63 {myTag midTag newTag}
r 2 r64 {myTag midTag}
r 3 r65 {myTag midTag}
r 4 r66 {myTag}
d 1 0 2.0
d 2 0 3.0
d 3 0 4.0
d 4 0 -1
d 1 1 2.1
d 2 1 3.1
d 3 1 4.1
d 4 1 -2
d 1 2 2.2
d 2 2 3.2
d 3 2 4.2
d 4 2 5.2
d 1 3 2.3
d 2 3 3.3
d 3 3 4.3
d 4 3 5.3
d 1 4 2.4
d 2 4 3.4
d 3 4 4.4
d 4 4 5.4
d 1 5 2.5
d 2 5 3.5
d 3 5 4.5
d 4 5 5.5
}}

test datatable.409 {export csv} {
    list [catch {datatable1 export csv} msg] $msg
} {0 {,,,,,
2.0,"2.1","2.2","2.3","2.4","2.5"
3.0,"3.1","3.2","3.3","3.4","3.5"
4.0,"4.1","4.2","4.3","4.4","4.5"
-1,"-2","5.2","5.3","5.4","5.5"
}}

test datatable.410 {export xml} {
    list [catch {datatable1 export xml} msg] $msg
} {0 {<root>
  <row name="r62"/>
  <row name="r63" c1="2.0" c2="2.1" c3="2.2" c4="2.3" c5="2.4" c6="2.5"/>
  <row name="r64" c1="3.0" c2="3.1" c3="3.2" c4="3.3" c5="3.4" c6="3.5"/>
  <row name="r65" c1="4.0" c2="4.1" c3="4.2" c4="4.3" c5="4.4" c6="4.5"/>
  <row name="r66" c1="-1" c2="-2" c3="5.2" c4="5.3" c5="5.4" c6="5.5"/>
</root>
}}

exit 0
test datatable.411 {dump} {
    list [catch {datatable1 dump} msg] $msg
} {0 {i 5 5 0 0
c 1 c1 string {}
c 2 c2 string {}
c 3 c3 string {}
c 4 c4 string {}
c 5 c5 string {}
r 1 myLabel {myTag midTag newTag}
r 2 myLabel {myTag midTag}
r 3 r3 {myTag midTag}
r 4 r4 {myTag}
r 5 label-with-minus {myTag}
d 2 1 1.0
d 3 1 1.0
d 4 1 1.0
d 5 1 x
d 2 2 2.0
d 3 2 2.0
d 4 2 2.0
d 5 2 y
d 2 3 3.0
d 3 3 3.0
d 4 3 3.0
d 5 3 3.0
d 2 4 4.0
d 3 4 4.0
d 4 4 4.0
d 5 4 4.0
d 2 5 5.0
d 3 5 5.0
d 4 5 5.0
d 5 5 5.0
}}

test datatable.412 {datatable1 row get 1 defValue} {
    list [catch {
	datatable1 emptyvalue defValue
	set out [datatable1 row get 1]
	datatable1 emptyvalue ""
	eval list $out
    } msg] $msg
} {0 {0 2.0 1 2.1 2 2.2 3 2.3 4 2.4 5 2.5}}

test datatable.413 {datatable1 row unset 1 end} {
    list [catch {datatable1 row unset 1 end} msg] $msg
} {0 {}}

test datatable.414 {datatable1 row extend 5 badArg } {
    list [catch {datatable1 row extend 5 badArg} msg] $msg
} {1 {row label "5" can't be a number.}}

test datatable.415 {datatable1 row extend} {
    list [catch {datatable1 row extend} msg] $msg
} {1 {wrong # args: should be "datatable1 row extend label ?label...?"}}

test datatable.416 {datatable1 row extend myRow} {
    list [catch {datatable1 row extend myRow} msg] $msg
} {0 7}

if 0 {
test datatable.417 {datatable1 row extend 10000000000 } {
    list [catch {datatable1 row extend 10000000000} msg] $msg
} {1 {can't extend table by 10000000000 rows: out of memory.}}
}

test datatable.418 {datatable1 row extend -10 } {
    list [catch {datatable1 row extend -10} msg] $msg
} {1 {bad count "-10": # rows can't be negative.}}

test datatable.419 {datatable1 row extend 10 } {
    list [catch {datatable1 row extend 10} msg] $msg
} {0 {9 10 11 12 13 14 15 16 17 18}}

test datatable.420 {datatable1 row names} {
    list [catch {datatable1 row names} msg] $msg
} {0 {myLabel myLabel r3 r4 label-with-minus r6 r7 myRow r9 r10 r11 r12 r13 r14 r15 r16 r17 r18}}

test datatable.421 {datatable1 row delete 10 } {
    list [catch {datatable1 row delete 10} msg] $msg
} {0 {}}

test datatable.422 {datatable1 row names} {
    list [catch {datatable1 row names} msg] $msg
} {0 {myLabel myLabel r3 r4 label-with-minus r6 r7 myRow r9 r11 r12 r13 r14 r15 r16 r17 r18}}

test datatable.423 {datatable1 row delete 10 } {
    list [catch {datatable1 row delete 10} msg] $msg
} {0 {}}

test datatable.424 {datatable1 numrows} {
    list [catch {datatable1 numrows} msg] $msg
} {0 16}

test datatable.425 {datatable1 row create} {
    list [catch {datatable1 row create} msg] $msg
} {0 17}

test datatable.426 {datatable1 row create -before 1 -badSwitch} {
    list [catch {datatable1 row create -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -after column
   -after row
   -before column
   -before row
   -label string
   -tags tags
   -type type}}

test datatable.427 {datatable1 row create -badSwitch -before 1} {
    list [catch {datatable1 row create -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -after column
   -after row
   -before column
   -before row
   -label string
   -tags tags
   -type type}}

test datatable.428 {datatable1 row create -before 1 -badSwitch arg} {
    list [catch {datatable1 row create -badSwitch arg} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -after column
   -after row
   -before column
   -before row
   -label string
   -tags tags
   -type type}}


test datatable.429 {datatable1 row create -before 1 -label one} {
    list [catch {datatable1 row create -before 1 -label one} msg] $msg
} {0 1}

test datatable.430 {datatable1 row create -before 2 -label two} {
    list [catch {datatable1 row create -before 2 -label two} msg] $msg
} {0 2}

test datatable.431 {datatable1 row create -after 3 -label three} {
    list [catch {datatable1 row create -after 3 -label three} msg] $msg
} {0 4}

test datatable.432 {datatable1 numrows} {
    list [catch {datatable1 numrows} msg] $msg
} {0 20}

test datatable.433 {datatable1 row names} {
    list [catch {datatable1 row names} msg] $msg
} {0 {one two myLabel three myLabel r3 r4 label-with-minus r6 r7 myRow r9 r12 r13 r14 r15 r16 r17 r18 r19}}

test datatable.434 {datatable1 row index end} {
    list [catch {datatable1 row index end} msg] $msg
} {0 20}

test datatable.435 {datatable1 row names} {
    list [catch {datatable1 row names} msg] $msg
} {0 {one two myLabel three myLabel r3 r4 label-with-minus r6 r7 myRow r9 r12 r13 r14 r15 r16 r17 r18 r19}}

test datatable.436 {datatable1 row create -after end} {
    list [catch {datatable1 row create -after end} msg] $msg
} {0 21}

test datatable.437 {datatable1 row create -after 1} {
    list [catch {datatable1 row create -after 1} msg] $msg
} {0 2}

test datatable.438 {datatable1 row create -label one} {
    list [catch {datatable1 row create -label one} msg] $msg
} {0 23}

test datatable.439 {datatable1 row create -label} {
    list [catch {datatable1 row create -label} msg] $msg
} {1 {value for "-label" missing}}

test datatable.440 {datatable1 row create -before 0} {
    list [catch {datatable1 row create -before 0} msg] $msg
} {1 {bad row index "0"}}

test datatable.441 {datatable1 numrows} {
    list [catch {datatable1 numrows} msg] $msg
} {0 23}

test datatable.442 {datatable1 row index fred} {
    list [catch {datatable1 row index fred} msg] $msg
} {0 -1}

test datatable.443 {datatable1 row index one} {
    list [catch {datatable1 row index one} msg] $msg
} {0 23}

test datatable.444 {datatable1 row index end} {
    list [catch {datatable1 row index end} msg] $msg
} {0 23}

test datatable.445 {datatable1 row create -after 40} {
    list [catch {datatable1 row create -after 40} msg] $msg
} {1 {bad row index "40"}}

test datatable.446 {datatable1 row create -tags {myTag1 myTag2}} {
    list [catch {
	datatable1 row create -tags {myTag1 myTag2}
    } msg] $msg
} {0 24}

test datatable.447 {datatable1 row create -after end -tags {myTag1 myTag2}} {
    list [catch {
	datatable1 row create -after end -tags {myTag1 myTag2} 
    } msg] $msg
} {0 25}

test datatable.448 {datatable1 row tag indices myTag1 myTag2} {
    list [catch {
	datatable1 row tag indices myTag1 myTag2
    } msg] $msg
} {0 {24 25}}

test datatable.449 {datatable1 row tag indices myTag1 myTag2} {
    list [catch {
	datatable1 row tag indices myTag1 myTag2
    } msg] $msg
} {0 {24 25}}

test datatable.450 {datatable1 row move} {
    list [catch {datatable1 row move} msg] $msg
} {1 {wrong # args: should be "datatable1 row move from to ?count?"}}

test datatable.451 {datatable1 row move 0} {
    list [catch {datatable1 row move 0} msg] $msg
} {1 {wrong # args: should be "datatable1 row move from to ?count?"}}

test datatable.452 {datatable1 row move 0 0} {
    list [catch {datatable1 row move 0 0} msg] $msg
} {1 {bad row index "0"}}

test datatable.453 {datatable1 row move 10 0} {
    list [catch {datatable1 row move 10 0} msg] $msg
} {1 {bad row index "0"}}

test datatable.454 {datatable1 row move all 10} {
    list [catch {datatable1 row move all 10} msg] $msg
} {1 {multiple rows specified by "all"}}

test datatable.455 {datatable1 row move 1 10 0} {
    list [catch { 
	datatable1 row move 1 10 0
	datatable1 row names
    } msg] $msg
} {0 {one r24 two myLabel three myLabel r3 r4 label-with-minus r6 r7 end r9 r12 r13 r14 r15 r16 r17 r18 r19 r23 one r26 r27}}

test datatable.456 {datatable1 row move 1 1} {
    list [catch { 
	datatable1 row move 1 1 
	datatable1 row names
    } msg] $msg
} {0 {one r24 two myLabel three myLabel r3 r4 label-with-minus r6 r7 end r9 r12 r13 r14 r15 r16 r17 r18 r19 r23 one r26 r27}}

test datatable.457 {datatable1 row move 1 10} {
    list [catch { 
	datatable1 row move 1 10 
	datatable1 row names
    } msg] $msg
} {0 {r24 two myLabel three myLabel r3 r4 label-with-minus r6 one r7 end r9 r12 r13 r14 r15 r16 r17 r18 r19 r23 one r26 r27}}

test datatable.458 {datatable1 row move 1 2} {
    list [catch {
	datatable1 row move 1 2
	datatable1 row names
    } msg] $msg
} {0 {two r24 myLabel three myLabel r3 r4 label-with-minus r6 one r7 end r9 r12 r13 r14 r15 r16 r17 r18 r19 r23 one r26 r27}}

test datatable.459 {export csv} {
    list [catch {datatable1 export csv} msg] $msg
} {0 {"*BLT*","c1","c2","c3","c4","c5"
"two",,,,,
"r24",,,,,
"myLabel",,,,,
"three",,,,,
"myLabel","1.0","2.0","3.0","4.0","5.0"
"r3","1.0","2.0","3.0","4.0","5.0"
"r4","1.0","2.0","3.0","4.0","5.0"
"label-with-minus",,,,,
"r6",,,,,
"one",,,,,
"r7",,,,,
"end",,,,,
"r9",,,,,
"r12",,,,,
"r13",,,,,
"r14",,,,,
"r15",,,,,
"r16",,,,,
"r17",,,,,
"r18",,,,,
"r19",,,,,
"r23",,,,,
"one",,,,,
"r26",,,,,
"r27",,,,,
}}

test datatable.460 {datatable1 row move 0 2} {
    list [catch {datatable1 row move 0 2} msg] $msg
} {1 {bad row index "0"}}

test datatable.461 {datatable1 row move 1 17} {
    list [catch {datatable1 row move 1 17} msg] $msg
} {0 {}}

#exit 0
test datatable.462 {datatable1 row trace} {
    list [catch {datatable1 row trace} msg] $msg
} {1 {wrong # args: should be "datatable1 row trace row how command"}}

test datatable.463 {datatable1 row trace all} {
    list [catch {datatable1 row trace all} msg] $msg
} {1 {wrong # args: should be "datatable1 row trace row how command"}}
    
test datatable.464 {datatable1 row trace end} {
    list [catch {datatable1 row trace end} msg] $msg
} {1 {wrong # args: should be "datatable1 row trace row how command"}}

test datatable.465 {datatable1 row trace 1} {
    list [catch {datatable1 row trace 1} msg] $msg
} {1 {wrong # args: should be "datatable1 row trace row how command"}}

test datatable.466 {datatable1 row trace 1 rwuc} {
    list [catch {datatable1 row trace 1 rwuc} msg] $msg
} {1 {wrong # args: should be "datatable1 row trace row how command"}}

test datatable.467 {datatable1 row trace all rwuc} {
    list [catch {datatable1 row trace all rwuc} msg] $msg
} {1 {wrong # args: should be "datatable1 row trace row how command"}}

proc Doit { args } { 
    global mylist; lappend mylist $args 
}

test datatable.468 {datatable1 row trace all rwuc Doit} {
    list [catch {datatable1 row trace all rwuc Doit} msg] $msg
} {0 trace0}

test datatable.469 {datatable1 trace info trace0} {
    list [catch {datatable1 trace info trace0} msg] $msg
} {0 {id trace0 row all flags rwuc command Doit}}

test datatable.470 {test create trace} {
    list [catch {
	set mylist {}
	datatable1 set all 1 20
	set mylist
    } msg] $msg
} {0 {{::datatable1 1 1 wc} {::datatable1 2 1 wc} {::datatable1 3 1 wc} {::datatable1 4 1 w} {::datatable1 5 1 w} {::datatable1 6 1 w} {::datatable1 7 1 wc} {::datatable1 8 1 wc} {::datatable1 9 1 wc} {::datatable1 10 1 wc} {::datatable1 11 1 wc} {::datatable1 12 1 wc} {::datatable1 13 1 wc} {::datatable1 14 1 wc} {::datatable1 15 1 wc} {::datatable1 16 1 wc} {::datatable1 17 1 wc} {::datatable1 18 1 wc} {::datatable1 19 1 wc} {::datatable1 20 1 wc} {::datatable1 21 1 wc} {::datatable1 22 1 wc} {::datatable1 23 1 wc} {::datatable1 24 1 wc} {::datatable1 25 1 wc}}}

test datatable.471 {test read trace} {
    list [catch {
	set mylist {}
	datatable1 row get 1 -valuesonly
	set mylist
	} msg] $msg
} {0 {{::datatable1 1 1 r} {::datatable1 1 2 r} {::datatable1 1 3 r} {::datatable1 1 4 r} {::datatable1 1 5 r}}}

test datatable.472 {test write trace} {
    list [catch {
	set mylist {}
	datatable1 row values 1 {a b c d e}
	set mylist
	} msg] $msg
} {0 {{::datatable1 1 1 w} {::datatable1 1 2 wc} {::datatable1 1 3 wc} {::datatable1 1 4 wc} {::datatable1 1 5 wc}}}

test datatable.473 {test unset trace} {
    list [catch {
	set mylist {}
	datatable1 row unset 1 
	set mylist
	} msg] $msg
} {0 {{::datatable1 1 1 u} {::datatable1 1 2 u} {::datatable1 1 3 u} {::datatable1 1 4 u} {::datatable1 1 5 u}}}

test datatable.474 {datatable1 trace delete} {
    list [catch {datatable1 trace delete} msg] $msg
} {0 {}}

#---------------------

test datatable.475 {datatable1 trace} {
    list [catch {datatable1 trace} msg] $msg
} {1 {wrong # args: should be one of...
  datatable1 trace create row column how command
  datatable1 trace delete traceId...
  datatable1 trace info traceId
  datatable1 trace names }}

test datatable.476 {datatable1 trace create} {
    list [catch {datatable1 trace create} msg] $msg
} {1 {wrong # args: should be "datatable1 trace create row column how command"}}

test datatable.477 {datatable1 trace create 1} {
    list [catch {datatable1 trace create 1} msg] $msg
} {1 {wrong # args: should be "datatable1 trace create row column how command"}}

test datatable.478 {datatable1 trace create 1 1 } {
    list [catch {datatable1 trace create 1 1 } msg] $msg
} {1 {wrong # args: should be "datatable1 trace create row column how command"}}

test datatable.479 {datatable1 trace create 1 1 rwuc} {
    list [catch {datatable1 trace create 1 1 rwuc} msg] $msg
} {1 {wrong # args: should be "datatable1 trace create row column how command"}}

proc Doit args { global mylist; lappend mylist $args }

test datatable.480 {datatable1 trace names} {
    list [catch {datatable1 trace names} msg] $msg
} {0 trace0}

test datatable.481 {datatable1 trace create 1 1 rwuc Doit} {
    list [catch {datatable1 trace create 1 1 rwuc Doit} msg] $msg
} {0 trace1}

test datatable.482 {datatable1 trace names} {
    list [catch {datatable1 trace names} msg] $msg
} {0 {trace1 trace0}}

test datatable.483 {datatable1 trace info trace1} {
    list [catch {datatable1 trace info trace1} msg] $msg
} {0 {id trace1 row 1 column 1 flags rwuc command Doit}}

test datatable.484 {test create trace} {
    list [catch {
	set mylist {}
	datatable1 set 1 1 "newValue"
	set mylist
	} msg] $msg
} {0 {{::datatable1 1 1 wc} {::datatable1 1 1 wc}}}

test datatable.485 {test read trace} {
    list [catch {
	set mylist {}
	datatable1 get 1 1
	set mylist
	} msg] $msg
} {0 {{::datatable1 1 1 r} {::datatable1 1 1 r}}}

test datatable.486 {test write trace} {
    list [catch {
	set mylist {}
	datatable1 column values 1 { a b c e d }
	set mylist
	} msg] $msg
} {0 {{::datatable1 1 1 w} {::datatable1 1 1 w} {::datatable1 2 1 w} {::datatable1 3 1 w} {::datatable1 4 1 w} {::datatable1 5 1 w}}}

test datatable.487 {trace delete trace0} {
    list [catch {datatable1 trace delete trace0} msg] $msg
} {0 {}}

test datatable.488 {test write trace} {
    list [catch {
	set mylist {}
	datatable1 row values 1 { a b c e d }
	set mylist
	} msg] $msg
} {0 {{::datatable1 1 1 w}}}

test datatable.489 {test write trace} {
    list [catch {
	set mylist {}
	datatable1 set 1 1 "nextValue"
	set mylist
	} msg] $msg
} {0 {{::datatable1 1 1 w}}}

test datatable.490 {test unset trace} {
    list [catch {
	set mylist {}
	datatable1 unset 1 1
	set mylist
	} msg] $msg
} {0 {{::datatable1 1 1 u}}}

test datatable.491 {datatable1 trace delete} {
    list [catch {datatable1 trace delete} msg] $msg
} {0 {}}

test datatable.492 {datatable1 trace delete badId} {
    list [catch {datatable1 trace delete badId} msg] $msg
} {1 {unknown trace "badId"}}

test datatable.493 {datatable1 trace names} {
    list [catch {datatable1 trace names} msg] $msg
} {0 trace1}

test datatable.494 {datatable1 trace names badArg} {
    list [catch {datatable1 trace names badArg} msg] $msg
} {1 {wrong # args: should be "datatable1 trace names "}}

test datatable.495 {datatable1 trace delete trace1} {
    list [catch {datatable1 trace delete trace1} msg] $msg
} {0 {}}

test datatable.496 {test create trace} {
    list [catch {
	set mylist {}
	datatable0 set all newKey 20
	set mylist
	} msg] $msg
} {0 {}}

test datatable.497 {test unset trace} {
    list [catch {
	set mylist {}
	datatable0 unset all newKey
	set mylist
	} msg] $msg
} {0 {}}

test datatable.498 {datatable0 dump -badSwitch} {
    list [catch {datatable0 dump -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"}}


test datatable.499 {datatable0 dump -rows 1} {
    list [catch {datatable0 dump -rows 1} msg] $msg
} {0 {i 1 22 0 0
c 1 one string
c 2 c14 string
c 3 two string
c 4 myLabel image {myTag midTag newTag}
c 5 three string
c 6 c2 image {myTag midTag}
c 7 c3 image {myTag midTag}
c 8 c4 image myTag
c 9 endLabel image myTag
c 10 c1 string
c 11 c5 string
c 12 c6 string
c 13 c7 string
c 14 c10 string
c 15 c11 string
c 16 c12 string
c 17 c13 string
c 18 c8 string
c 19 c9 string
c 20 c15 string
c 21 c16 string {myTag2 myTag1}
c 22 c17 string {myTag2 myTag1}
r 1 r1
d 1 6 1.0
d 1 7 1.0
d 1 8 1.0
}}


test datatable.500 {datatable0 dump -columns 1} {
    list [catch {datatable0 dump -columns 1} msg] $msg
} {0 {i 5 1 0 0
c 1 one string
r 1 r1
r 2 r2
r 3 r3
r 4 r4
r 5 r5
}}

test datatable.501 {dump -rows badTag} {
    list [catch {datatable0 dump -rows badTag} msg] $msg
} {1 {can't find row tag "badTag" in ::datatable0}}

test datatable.502 {dump -columns badTag} {
    list [catch {datatable0 dump -columns badTag} msg] $msg
} {1 {can't find column tag "badTag" in ::datatable0}}

test datatable.503 {dump -rows 1 -columns 1} {
    list [catch {datatable0 dump -rows 1 -columns 1} msg] $msg
} {0 {i 1 1 0 0
c 1 one string
r 1 r1
}}

test datatable.504 {dump -file myout.dump} {
    list [catch {datatable0 dump -file myout.dump} msg] $msg
} {0 {}}

test datatable.505 {blt::datatable destroy datatable0} {
    list [catch {blt::datatable destroy datatable0} msg] $msg
} {0 {}}

test datatable.506 {blt::datatable create} {
    list [catch {blt::datatable create} msg] $msg
} {0 ::datatable0}

test datatable.507 {datatable0 column names} {
    list [catch {datatable0 column names} msg] $msg
} {0 {}}

test datatable.508 {datatable0 dump} {
    list [catch {datatable0 dump} msg] $msg
} {0 {i 0 0 0 0
}}

test datatable.509 {datatable0 restore} {
    list [catch {datatable0 restore} msg] $msg
} {0 {}}

test datatable.510 {datatable0 dump} {
    list [catch {datatable0 dump} msg] $msg
} {0 {i 5 22 0 0
c 1 one string
c 2 c14 string
c 3 two string
c 4 myLabel image {myTag midTag newTag}
c 5 three string
c 6 c2 image {myTag midTag}
c 7 c3 image {myTag midTag}
c 8 c4 image myTag
c 9 endLabel image myTag
c 10 c1 string
c 11 c5 string
c 12 c6 string
c 13 c7 string
c 14 c10 string
c 15 c11 string
c 16 c12 string
c 17 c13 string
c 18 c8 string
c 19 c9 string
c 20 c15 string
c 21 c16 string {myTag2 myTag1}
c 22 c17 string {myTag2 myTag1}
r 1 r1
r 2 r2
r 3 r3
r 4 r4
r 5 r5
d 1 6 1.0
d 1 7 1.0
d 1 8 1.0
d 2 6 2.0
d 2 7 2.0
d 2 8 2.0
d 3 6 3.0
d 3 7 3.0
d 3 8 3.0
d 4 6 4.0
d 4 7 4.0
d 4 8 4.0
d 5 6 5.0
d 5 7 5.0
d 5 8 5.0
}}

exit 0
#----------------------

test datatable.511 {datatable0 column tag names badNode} {
    list [catch {datatable0 column tag names badNode} msg] $msg
} {1 {can't find tag or id "badNode" in ::datatable0}}

test datatable.512 {datatable0 column tag names all} {
    list [catch {datatable0 column tag names all} msg] $msg
} {1 {multiple columns specified by "all"}}

test datatable.513 {datatable0 column tag names root} {
    list [catch {datatable0 column tag names root} msg] $msg
} {0 {all hi newTag root tag2}}

test datatable.514 {datatable0 column tag names 0 1} {
    list [catch {datatable0 column tag names 0 1} msg] $msg
} {0 {all hi newTag root tag2}}

test datatable.515 {datatable0 column tag nodes (missing arg)} {
    list [catch {datatable0 column tag nodes} msg] $msg
} {1 {wrong # args: should be "datatable0 column tag nodes tag ?tag...?"}}

test datatable.516 {datatable0 column tag nodes root badTag} {
    list [catch {datatable0 column tag nodes root badTag} msg] $msg
} {1 {can't find a tag "badTag"}}

test datatable.517 {datatable0 column tag nodes root tag2} {
    list [catch {datatable0 column tag nodes root tag2} msg] $msg
} {0 {0 1 2 3 4}}


test datatable.518 {datatable0 create 0} {
    list [catch {datatable0 create 0} msg] $msg
} {0 2}

test datatable.519 {datatable0 create root} {
    list [catch {datatable0 create root} msg] $msg
} {0 3}

test datatable.520 {datatable0 create all} {
    list [catch {datatable0 create all} msg] $msg
} {1 {multiple columns specified by "all"}}

test datatable.521 {datatable0 create 0 -at badPosition} {
    list [catch {datatable0 create 0 -at badPosition} msg] $msg
} {1 {expected integer but got "badPosition"}}

test datatable.522 {datatable0 create 0 -at -1} {
    list [catch {datatable0 create 0 -at -1} msg] $msg
} {1 {bad value "-1": can't be negative}}

test datatable.523 {datatable0 create 0 -at 1000} {
    list [catch {datatable0 create 0 -at 1000} msg] $msg
} {0 4}

test datatable.524 {datatable0 create 0 -at (no arg)} {
    list [catch {datatable0 create 0 -at} msg] $msg
} {1 {value for "-at" missing}}

test datatable.525 {datatable0 create 0 -tags myTag} {
    list [catch {datatable0 create 0 -tags myTag} msg] $msg
} {0 5}

test datatable.526 {datatable0 insert 0 -tags {myTag1 myTag2} } {
    list [catch {datatable0 insert 0 -tags {myTag1 myTag2}} msg] $msg
} {0 6}

test datatable.527 {datatable0 insert 0 -tags root} {
    list [catch {datatable0 insert 0 -tags root} msg] $msg
} {1 {can't add reserved tag "root"}}

test datatable.528 {datatable0 insert 0 -tags (missing arg)} {
    list [catch {datatable0 insert 0 -tags} msg] $msg
} {1 {value for "-tags" missing}}

test datatable.529 {datatable0 insert 0 -label myLabel -tags thisTag} {
    list [catch {datatable0 insert 0 -label myLabel -tags thisTag} msg] $msg
} {0 8}

test datatable.530 {datatable0 insert 0 -label (missing arg)} {
    list [catch {datatable0 insert 0 -label} msg] $msg
} {1 {value for "-label" missing}}

test datatable.531 {datatable0 insert 1 -tags thisTag} {
    list [catch {datatable0 insert 1 -tags thisTag} msg] $msg
} {0 9}

test datatable.532 {datatable0 insert 1 -data key (missing value)} {
    list [catch {datatable0 insert 1 -data key} msg] $msg
} {1 {missing value for "key"}}

test datatable.533 {datatable0 insert 1 -data {key value}} {
    list [catch {datatable0 insert 1 -data {key value}} msg] $msg
} {0 11}

test datatable.534 {datatable0 insert 1 -data {key1 value1 key2 value2}} {
    list [catch {datatable0 insert 1 -data {key1 value1 key2 value2}} msg] $msg
} {0 12}

test datatable.535 {get} {
    list [catch {
	datatable0 get 12
    } msg] $msg
} {0 {key1 value1 key2 value2}}

test datatable.536 {datatable0 children} {
    list [catch {datatable0 children} msg] $msg
} {1 {wrong # args: should be "datatable0 children node ?first? ?last?"}}

test datatable.537 {datatable0 children 0} {
    list [catch {datatable0 children 0} msg] $msg
} {0 {1 2 3 4 5 6 8}}

test datatable.538 {datatable0 children root} {
    list [catch {datatable0 children root} msg] $msg
} {0 {1 2 3 4 5 6 8}}

test datatable.539 {datatable0 children 1} {
    list [catch {datatable0 children 1} msg] $msg
} {0 {9 11 12}}

test datatable.540 {datatable0 insert myTag} {
    list [catch {datatable0 insert myTag} msg] $msg
} {0 13}

test datatable.541 {datatable0 children myTag} {
    list [catch {datatable0 children myTag} msg] $msg
} {0 13}

test datatable.542 {datatable0 children root 0 end} {
    list [catch {datatable0 children root 0 end} msg] $msg
} {0 {1 2 3 4 5 6 8}}

test datatable.543 {datatable0 children root 2} {
    list [catch {datatable0 children root 2} msg] $msg
} {0 3}

test datatable.544 {datatable0 children root 2 end} {
    list [catch {datatable0 children root 2 end} msg] $msg
} {0 {3 4 5 6 8}}

test datatable.545 {datatable0 children root end end} {
    list [catch {datatable0 children root end end} msg] $msg
} {0 8}

test datatable.546 {datatable0 children root 0 2} {
    list [catch {datatable0 children root 0 2} msg] $msg
} {0 {1 2 3}}

test datatable.547 {datatable0 children root -1 -20} {
    list [catch {datatable0 children root -1 -20} msg] $msg
} {0 {}}

test datatable.548 {datatable0 firstchild (missing arg)} {
    list [catch {datatable0 firstchild} msg] $msg
} {1 {wrong # args: should be "datatable0 firstchild node"}}

test datatable.549 {datatable0 firstchild root} {
    list [catch {datatable0 firstchild root} msg] $msg
} {0 1}

test datatable.550 {datatable0 lastchild (missing arg)} {
    list [catch {datatable0 lastchild} msg] $msg
} {1 {wrong # args: should be "datatable0 lastchild node"}}

test datatable.551 {datatable0 lastchild root} {
    list [catch {datatable0 lastchild root} msg] $msg
} {0 8}

test datatable.552 {datatable0 nextsibling (missing arg)} {
    list [catch {datatable0 nextsibling} msg] $msg
} {1 {wrong # args: should be "datatable0 nextsibling node"}}

test datatable.553 {datatable0 nextsibling 1)} {
    list [catch {datatable0 nextsibling 1} msg] $msg
} {0 2}

test datatable.554 {datatable0 nextsibling 2)} {
    list [catch {datatable0 nextsibling 2} msg] $msg
} {0 3}

test datatable.555 {datatable0 nextsibling 3)} {
    list [catch {datatable0 nextsibling 3} msg] $msg
} {0 4}

test datatable.556 {datatable0 nextsibling 4)} {
    list [catch {datatable0 nextsibling 4} msg] $msg
} {0 5}

test datatable.557 {datatable0 nextsibling 5)} {
    list [catch {datatable0 nextsibling 5} msg] $msg
} {0 6}

test datatable.558 {datatable0 nextsibling 6)} {
    list [catch {datatable0 nextsibling 6} msg] $msg
} {0 8}

test datatable.559 {datatable0 nextsibling 8)} {
    list [catch {datatable0 nextsibling 8} msg] $msg
} {0 -1}

test datatable.560 {datatable0 nextsibling all)} {
    list [catch {datatable0 nextsibling all} msg] $msg
} {1 { than one node tagged as "all"}}

test datatable.561 {datatable0 nextsibling badTag)} {
    list [catch {datatable0 nextsibling badTag} msg] $msg
} {1 {can't find tag or id "badTag" in ::datatable0}}

test datatable.562 {datatable0 nextsibling -1)} {
    list [catch {datatable0 nextsibling -1} msg] $msg
} {1 {can't find tag or id "-1" in ::datatable0}}

test datatable.563 {datatable0 prevsibling 2)} {
    list [catch {datatable0 prevsibling 2} msg] $msg
} {0 1}

test datatable.564 {datatable0 prevsibling 1)} {
    list [catch {datatable0 prevsibling 1} msg] $msg
} {0 -1}

test datatable.565 {datatable0 prevsibling -1)} {
    list [catch {datatable0 prevsibling -1} msg] $msg
} {1 {can't find tag or id "-1" in ::datatable0}}

test datatable.566 {datatable0 root)} {
    list [catch {datatable0 root} msg] $msg
} {0 0}

test datatable.567 {datatable0 root badArg)} {
    list [catch {datatable0 root badArgs} msg] $msg
} {1 {wrong # args: should be "datatable0 root "}}

test datatable.568 {datatable0 parent (missing arg))} {
    list [catch {datatable0 parent} msg] $msg
} {1 {wrong # args: should be "datatable0 parent node"}}

test datatable.569 {datatable0 parent root)} {
    list [catch {datatable0 parent root} msg] $msg
} {0 -1}

test datatable.570 {datatable0 parent 1)} {
    list [catch {datatable0 parent 1} msg] $msg
} {0 0}

test datatable.571 {datatable0 parent myTag)} {
    list [catch {datatable0 parent myTag} msg] $msg
} {0 0}

test datatable.572 {datatable0 next (missing arg))} {
    list [catch {datatable0 next} msg] $msg
} {1 {wrong # args: should be "datatable0 next node"}}


test datatable.573 {datatable0 next (extra arg))} {
    list [catch {datatable0 next root root} msg] $msg
} {1 {wrong # args: should be "datatable0 next node"}}

test datatable.574 {datatable0 next root} {
    list [catch {datatable0 next root} msg] $msg
} {0 1}

test datatable.575 {datatable0 next 1)} {
    list [catch {datatable0 next 1} msg] $msg
} {0 9}

test datatable.576 {datatable0 next 2)} {
    list [catch {datatable0 next 2} msg] $msg
} {0 3}

test datatable.577 {datatable0 next 3)} {
    list [catch {datatable0 next 3} msg] $msg
} {0 4}

test datatable.578 {datatable0 next 4)} {
    list [catch {datatable0 next 4} msg] $msg
} {0 5}

test datatable.579 {datatable0 next 5)} {
    list [catch {datatable0 next 5} msg] $msg
} {0 13}

test datatable.580 {datatable0 next 6)} {
    list [catch {datatable0 next 6} msg] $msg
} {0 8}

test datatable.581 {datatable0 next 8)} {
    list [catch {datatable0 next 8} msg] $msg
} {0 -1}

test datatable.582 {datatable0 previous 1)} {
    list [catch {datatable0 previous 1} msg] $msg
} {0 0}

test datatable.583 {datatable0 previous 0)} {
    list [catch {datatable0 previous 0} msg] $msg
} {0 -1}

test datatable.584 {datatable0 previous 8)} {
    list [catch {datatable0 previous 8} msg] $msg
} {0 6}

test datatable.585 {datatable0 depth (no arg))} {
    list [catch {datatable0 depth} msg] $msg
} {1 {wrong # args: should be "datatable0 depth node"}}

test datatable.586 {datatable0 depth root))} {
    list [catch {datatable0 depth root} msg] $msg
} {0 0}

test datatable.587 {datatable0 depth myTag))} {
    list [catch {datatable0 depth myTag} msg] $msg
} {0 1}

test datatable.588 {datatable0 depth myTag))} {
    list [catch {datatable0 depth myTag} msg] $msg
} {0 1}


test datatable.589 {datatable0 dumpdata 1} {
    list [catch {datatable0 dumpdata 1} msg] $msg
} {0 {-1 1 {node1} {} {}
1 9 {node1 node9} {} {thisTag}
1 11 {node1 node11} {key value} {}
1 12 {node1 node12} {key1 value1 key2 value2} {}
}}

test datatable.590 {datatable0 dumpdata this} {
    list [catch {datatable0 dumpdata myTag} msg] $msg
} {0 {-1 5 {node5} {} {myTag}
5 13 {node5 node13} {} {}
}}

test datatable.591 {datatable0 dumpdata 1 badArg (too many args)} {
    list [catch {datatable0 dumpdata 1 badArg} msg] $msg
} {1 {wrong # args: should be "datatable0 dumpdata node"}}

test datatable.592 {datatable0 dumpdata 11} {
    list [catch {datatable0 dumpdata 11} msg] $msg
} {0 {-1 11 {node11} {key value} {}
}}

test datatable.593 {datatable0 dumpdata all} {
    list [catch {datatable0 dumpdata all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.594 {datatable0 dumpdata all} {
    list [catch {datatable0 dumpdata all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.595 {datatable0 dumpfile 0 test.dump} {
    list [catch {datatable0 dumpfile 0 test.dump} msg] $msg
} {0 {}}

test datatable.596 {datatable0 get 9} {
    list [catch {datatable0 get 9} msg] $msg
} {0 {}}

test datatable.597 {datatable0 get all} {
    list [catch {datatable0 get all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.598 {datatable0 get root} {
    list [catch {datatable0 get root} msg] $msg
} {0 {}}

test datatable.599 {datatable0 get 9 key} {
    list [catch {datatable0 get root} msg] $msg
} {0 {}}

test datatable.600 {datatable0 get 12} {
    list [catch {datatable0 get 12} msg] $msg
} {0 {key1 value1 key2 value2}}

test datatable.601 {datatable0 get 12 key1} {
    list [catch {datatable0 get 12 key1} msg] $msg
} {0 value1}

test datatable.602 {datatable0 get 12 key2} {
    list [catch {datatable0 get 12 key2} msg] $msg
} {0 value2}

test datatable.603 {datatable0 get 12 key1 defValue } {
    list [catch {datatable0 get 12 key1 defValue} msg] $msg
} {0 value1}

test datatable.604 {datatable0 get 12 key100 defValue } {
    list [catch {datatable0 get 12 key100 defValue} msg] $msg
} {0 defValue}

test datatable.605 {datatable0 index (missing arg) } {
    list [catch {datatable0 index} msg] $msg
} {1 {wrong # args: should be "datatable0 index name"}}

test datatable.606 {datatable0 index 0 10 (extra arg) } {
    list [catch {datatable0 index 0 10} msg] $msg
} {1 {wrong # args: should be "datatable0 index name"}}

test datatable.607 {datatable0 index 0} {
    list [catch {datatable0 index 0} msg] $msg
} {0 0}

test datatable.608 {datatable0 index root} {
    list [catch {datatable0 index root} msg] $msg
} {0 0}

test datatable.609 {datatable0 index all} {
    list [catch {datatable0 index all} msg] $msg
} {0 -1}

test datatable.610 {datatable0 index myTag} {
    list [catch {datatable0 index myTag} msg] $msg
} {0 5}

test datatable.611 {datatable0 index thisTag} {
    list [catch {datatable0 index thisTag} msg] $msg
} {0 -1}

test datatable.612 {datatable0 is (no args)} {
    list [catch {datatable0 is} msg] $msg
} {1 {wrong # args: should be one of...
  datatable0 is ancestor node1 node2
  datatable0 is before node1 node2
  datatable0 is leaf node
  datatable0 is link node
  datatable0 is root node}}

test datatable.613 {datatable0 is badOp} {
    list [catch {datatable0 is badOp} msg] $msg
} {1 {bad operation "badOp": should be one of...
  datatable0 is ancestor node1 node2
  datatable0 is before node1 node2
  datatable0 is leaf node
  datatable0 is link node
  datatable0 is root node}}

test datatable.614 {datatable0 is before} {
    list [catch {datatable0 is before} msg] $msg
} {1 {wrong # args: should be "datatable0 is before node1 node2"}}

test datatable.615 {datatable0 is before 0 10 20} {
    list [catch {datatable0 is before 0 10 20} msg] $msg
} {1 {wrong # args: should be "datatable0 is before node1 node2"}}

test datatable.616 {datatable0 is before 0 12} {
    list [catch {datatable0 is before 0 12} msg] $msg
} {0 1}

test datatable.617 {datatable0 is before 12 0} {
    list [catch {datatable0 is before 12 0} msg] $msg
} {0 0}

test datatable.618 {datatable0 is before 0 0} {
    list [catch {datatable0 is before 0 0} msg] $msg
} {0 0}

test datatable.619 {datatable0 is before root 0} {
    list [catch {datatable0 is before root 0} msg] $msg
} {0 0}

test datatable.620 {datatable0 is before 0 all} {
    list [catch {datatable0 is before 0 all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.621 {datatable0 is ancestor} {
    list [catch {datatable0 is ancestor} msg] $msg
} {1 {wrong # args: should be "datatable0 is ancestor node1 node2"}}

test datatable.622 {datatable0 is ancestor 0 12 20} {
    list [catch {datatable0 is ancestor 0 12 20} msg] $msg
} {1 {wrong # args: should be "datatable0 is ancestor node1 node2"}}

test datatable.623 {datatable0 is ancestor 0 12} {
    list [catch {datatable0 is ancestor 0 12} msg] $msg
} {0 1}

test datatable.624 {datatable0 is ancestor 12 0} {
    list [catch {datatable0 is ancestor 12 0} msg] $msg
} {0 0}

test datatable.625 {datatable0 is ancestor 1 2} {
    list [catch {datatable0 is ancestor 1 2} msg] $msg
} {0 0}

test datatable.626 {datatable0 is ancestor root 0} {
    list [catch {datatable0 is ancestor root 0} msg] $msg
} {0 0}

test datatable.627 {datatable0 is ancestor 0 all} {
    list [catch {datatable0 is ancestor 0 all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.628 {datatable0 is root (missing arg)} {
    list [catch {datatable0 is root} msg] $msg
} {1 {wrong # args: should be "datatable0 is root node"}}

test datatable.629 {datatable0 is root 0 20 (extra arg)} {
    list [catch {datatable0 is root 0 20} msg] $msg
} {1 {wrong # args: should be "datatable0 is root node"}}

test datatable.630 {datatable0 is root 0} {
    list [catch {datatable0 is root 0} msg] $msg
} {0 1}

test datatable.631 {datatable0 is root 12} {
    list [catch {datatable0 is root 12} msg] $msg
} {0 0}

test datatable.632 {datatable0 is root 1} {
    list [catch {datatable0 is root 1} msg] $msg
} {0 0}

test datatable.633 {datatable0 is root root} {
    list [catch {datatable0 is root root} msg] $msg
} {0 1}

test datatable.634 {datatable0 is root all} {
    list [catch {datatable0 is root all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.635 {datatable0 is leaf (missing arg)} {
    list [catch {datatable0 is leaf} msg] $msg
} {1 {wrong # args: should be "datatable0 is leaf node"}}

test datatable.636 {datatable0 is leaf 0 20 (extra arg)} {
    list [catch {datatable0 is leaf 0 20} msg] $msg
} {1 {wrong # args: should be "datatable0 is leaf node"}}

test datatable.637 {datatable0 is leaf 0} {
    list [catch {datatable0 is leaf 0} msg] $msg
} {0 0}

test datatable.638 {datatable0 is leaf 12} {
    list [catch {datatable0 is leaf 12} msg] $msg
} {0 1}

test datatable.639 {datatable0 is leaf 1} {
    list [catch {datatable0 is leaf 1} msg] $msg
} {0 0}

test datatable.640 {datatable0 is leaf root} {
    list [catch {datatable0 is leaf root} msg] $msg
} {0 0}

test datatable.641 {datatable0 is leaf all} {
    list [catch {datatable0 is leaf all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.642 {datatable0 is leaf 1000} {
    list [catch {datatable0 is leaf 1000} msg] $msg
} {1 {can't find tag or id "1000" in ::datatable0}}

test datatable.643 {datatable0 is leaf badTag} {
    list [catch {datatable0 is leaf badTag} msg] $msg
} {1 {can't find tag or id "badTag" in ::datatable0}}

test datatable.644 {datatable0 set (missing arg)} {
    list [catch {datatable0 set} msg] $msg
} {1 {wrong # args: should be "datatable0 set node ?key value...?"}}

test datatable.645 {datatable0 set 0 (missing arg)} {
    list [catch {datatable0 set 0} msg] $msg
} {0 {}}

test datatable.646 {datatable0 set 0 key (missing arg)} {
    list [catch {datatable0 set 0 key} msg] $msg
} {1 {missing value for field "key"}}

test datatable.647 {datatable0 set 0 key value} {
    list [catch {datatable0 set 0 key value} msg] $msg
} {0 {}}

test datatable.648 {datatable0 set 0 key1 value1 key2 value2 key3 value3} {
    list [catch {datatable0 set 0 key1 value1 key2 value2 key3 value3} msg] $msg
} {0 {}}

test datatable.649 {datatable0 set 0 key1 value1 key2 (missing arg)} {
    list [catch {datatable0 set 0 key1 value1 key2} msg] $msg
} {1 {missing value for field "key2"}}

test datatable.650 {datatable0 set 0 key value} {
    list [catch {datatable0 set 0 key value} msg] $msg
} {0 {}}

test datatable.651 {datatable0 set 0 key1 value1 key2 (missing arg)} {
    list [catch {datatable0 set 0 key1 value1 key2} msg] $msg
} {1 {missing value for field "key2"}}

test datatable.652 {datatable0 set all} {
    list [catch {datatable0 set all} msg] $msg
} {0 {}}

test datatable.653 {datatable0 set all abc 123} {
    list [catch {datatable0 set all abc 123} msg] $msg
} {0 {}}

test datatable.654 {datatable0 set root} {
    list [catch {datatable0 set root} msg] $msg
} {0 {}}

test datatable.655 {datatable0 restore stuff} {
    list [catch {
	set data [datatable0 dumpdata root]
	blt::datatable create
	datatable1 restore root $data
	set data [datatable1 dumpdata root]
	blt::datatable destroy datatable1
	set data
	} msg] $msg
} {0 {-1 0 {::datatable0} {key value key1 value1 key2 value2 key3 value3 abc 123} {}
0 1 {::datatable0 node1} {abc 123} {}
1 9 {::datatable0 node1 node9} {abc 123} {thisTag}
1 11 {::datatable0 node1 node11} {key value abc 123} {}
1 12 {::datatable0 node1 node12} {key1 value1 key2 value2 abc 123} {}
0 2 {::datatable0 node2} {abc 123} {}
0 3 {::datatable0 node3} {abc 123} {}
0 4 {::datatable0 node4} {abc 123} {}
0 5 {::datatable0 node5} {abc 123} {myTag}
5 13 {::datatable0 node5 node13} {abc 123} {}
0 6 {::datatable0 node6} {abc 123} {myTag2 myTag1}
0 8 {::datatable0 myLabel} {abc 123} {thisTag}
}}

test datatable.656 {datatable0 restorefile 0 test.dump} {
    list [catch {
	blt::datatable create
	datatable1 restorefile root test.dump
	set data [datatable1 dumpdata root]
	blt::datatable destroy datatable1
	set data
	} msg] $msg
} {0 {-1 0 {::datatable0} {} {}
0 1 {::datatable0 node1} {} {}
1 9 {::datatable0 node1 node9} {} {thisTag}
1 11 {::datatable0 node1 node11} {key value} {}
1 12 {::datatable0 node1 node12} {key1 value1 key2 value2} {}
0 2 {::datatable0 node2} {} {}
0 3 {::datatable0 node3} {} {}
0 4 {::datatable0 node4} {} {}
0 5 {::datatable0 node5} {} {myTag}
5 13 {::datatable0 node5 node13} {} {}
0 6 {::datatable0 node6} {} {myTag2 myTag1}
0 8 {::datatable0 myLabel} {} {thisTag}
}}


test datatable.657 {datatable0 unset 0 key1} {
    list [catch {datatable0 unset 0 key1} msg] $msg
} {0 {}}

test datatable.658 {datatable0 get 0} {
    list [catch {datatable0 get 0} msg] $msg
} {0 {key value key2 value2 key3 value3 abc 123}}

test datatable.659 {datatable0 unset 0 key2 key3} {
    list [catch {datatable0 unset 0 key2 key3} msg] $msg
} {0 {}}

test datatable.660 {datatable0 get 0} {
    list [catch {datatable0 get 0} msg] $msg
} {0 {key value abc 123}}

test datatable.661 {datatable0 unset 0} {
    list [catch {datatable0 unset 0} msg] $msg
} {0 {}}

test datatable.662 {datatable0 get 0} {
    list [catch {datatable0 get 0} msg] $msg
} {0 {}}

test datatable.663 {datatable0 unset all abc} {
    list [catch {datatable0 unset all abc} msg] $msg
} {0 {}}

test datatable.664 {datatable0 restore stuff} {
    list [catch {
	set data [datatable0 dumpdata root]
	blt::datatable create datatable1
	datatable1 restore root $data
	set data [datatable1 dumpdata root]
	blt::datatable destroy datatable1
	set data
	} msg] $msg
} {0 {-1 0 {::datatable0} {} {}
0 1 {::datatable0 node1} {} {}
1 9 {::datatable0 node1 node9} {} {thisTag}
1 11 {::datatable0 node1 node11} {key value} {}
1 12 {::datatable0 node1 node12} {key1 value1 key2 value2} {}
0 2 {::datatable0 node2} {} {}
0 3 {::datatable0 node3} {} {}
0 4 {::datatable0 node4} {} {}
0 5 {::datatable0 node5} {} {myTag}
5 13 {::datatable0 node5 node13} {} {}
0 6 {::datatable0 node6} {} {myTag2 myTag1}
0 8 {::datatable0 myLabel} {} {thisTag}
}}

test datatable.665 {datatable0 restore (missing arg)} {
    list [catch {datatable0 restore} msg] $msg
} {1 {wrong # args: should be "datatable0 restore node dataString ?switches?"}}

test datatable.666 {datatable0 restore 0 badString} {
    list [catch {datatable0 restore 0 badString} msg] $msg
} {1 {line #1: wrong # elements in restore entry}}

test datatable.667 {datatable0 restore 0 {} arg (extra arg)} {
    list [catch {datatable0 restore 0 {} arg} msg] $msg
} {1 {unknown option "arg"}}


test datatable.668 {datatable0 size (missing arg)} {
    list [catch {datatable0 size} msg] $msg
} {1 {wrong # args: should be "datatable0 size node"}}

test datatable.669 {datatable0 size 0} {
    list [catch {datatable0 size 0} msg] $msg
} {0 12}

test datatable.670 {datatable0 size all} {
    list [catch {datatable0 size all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.671 {datatable0 size 0 10 (extra arg)} {
    list [catch {datatable0 size 0 10} msg] $msg
} {1 {wrong # args: should be "datatable0 size node"}}

test datatable.672 {datatable0 delete (missing arg)} {
    list [catch {datatable0 delete} msg] $msg
} {1 {wrong # args: should be "datatable0 delete node ?node...?"}}

test datatable.673 {datatable0 delete 11} {
    list [catch {datatable0 delete 11} msg] $msg
} {0 {}}

test datatable.674 {datatable0 delete 11} {
    list [catch {datatable0 delete 11} msg] $msg
} {1 {can't find tag or id "11" in ::datatable0}}

test datatable.675 {datatable0 delete 9 12} {
    list [catch {datatable0 delete 9 12} msg] $msg
} {0 {}}

test datatable.676 {datatable0 dumpdata 0} {
    list [catch {datatable0 dump 0} msg] $msg
} {0 {-1 0 {::datatable0} {} {}
0 1 {::datatable0 node1} {} {}
0 2 {::datatable0 node2} {} {}
0 3 {::datatable0 node3} {} {}
0 4 {::datatable0 node4} {} {}
0 5 {::datatable0 node5} {} {myTag}
5 13 {::datatable0 node5 node13} {} {}
0 6 {::datatable0 node6} {} {myTag2 myTag1}
0 8 {::datatable0 myLabel} {} {thisTag}
}}

test datatable.677 {delete all} {
    list [catch {
	set data [datatable0 dump root]
	blt::datatable create
	datatable1 restore root $data
	datatable1 delete all
	set data [datatable1 dump root]
	blt::datatable destroy datatable1
	set data
	} msg] $msg
} {0 {-1 0 {::datatable0} {} {}
}}

test datatable.678 {delete all all} {
    list [catch {
	set data [datatable0 dump root]
	blt::datatable create
	datatable1 restore root $data
	datatable1 delete all all
	set data [datatable1 dump root]
	blt::datatable destroy datatable1
	set data
	} msg] $msg
} {0 {-1 0 {::datatable0} {} {}
}}

test datatable.679 {datatable0 apply (missing arg)} {
    list [catch {datatable0 apply} msg] $msg
} {1 {wrong # args: should be "datatable0 apply node ?switches?"}}

test datatable.680 {datatable0 apply 0} {
    list [catch {datatable0 apply 0} msg] $msg
} {0 {}}

test datatable.681 {datatable0 apply 0 -badOption} {
    list [catch {datatable0 apply 0 -badOption} msg] $msg
} {1 {unknown option "-badOption"}}

test datatable.682 {datatable0 apply badTag} {
    list [catch {datatable0 apply badTag} msg] $msg
} {1 {can't find tag or id "badTag" in ::datatable0}}

test datatable.683 {datatable0 apply all} {
    list [catch {datatable0 apply all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.684 {datatable0 apply myTag -precommand lappend} {
    list [catch {
	set mylist {}
	datatable0 apply myTag -precommand {lappend mylist}
	set mylist
    } msg] $msg
} {0 {5 13}}

test datatable.685 {datatable0 apply root -precommand lappend} {
    list [catch {
	set mylist {}
	datatable0 apply root -precommand {lappend mylist}
	set mylist
    } msg] $msg
} {0 {0 1 2 3 4 5 13 6 8}}

test datatable.686 {datatable0 apply -precommand -postcommand} {
    list [catch {
	set mylist {}
	datatable0 apply root -precommand {lappend mylist} \
		-postcommand {lappend mylist}
	set mylist
    } msg] $msg
} {0 {0 1 1 2 2 3 3 4 4 5 13 13 5 6 6 8 8 0}}

test datatable.687 {datatable0 apply root -precommand lappend -depth 1} {
    list [catch {
	set mylist {}
	datatable0 apply root -precommand {lappend mylist} -depth 1
	set mylist
    } msg] $msg
} {0 {0 1 2 3 4 5 6 8}}


test datatable.688 {datatable0 apply root -precommand -depth 0} {
    list [catch {
	set mylist {}
	datatable0 apply root -precommand {lappend mylist} -depth 0
	set mylist
    } msg] $msg
} {0 0}

test datatable.689 {datatable0 apply root -precommand -tag myTag} {
    list [catch {
	set mylist {}
	datatable0 apply root -precommand {lappend mylist} -tag myTag
	set mylist
    } msg] $msg
} {0 5}


test datatable.690 {datatable0 apply root -precommand -key key1} {
    list [catch {
	set mylist {}
	datatable0 set myTag key1 0.0
	datatable0 apply root -precommand {lappend mylist} -key key1
	datatable0 unset myTag key1
	set mylist
    } msg] $msg
} {0 5}

test datatable.691 {datatable0 apply root -postcommand -regexp node.*} {
    list [catch {
	set mylist {}
	datatable0 set myTag key1 0.0
	datatable0 apply root -precommand {lappend mylist} -regexp {node5} 
	datatable0 unset myTag key1
	set mylist
    } msg] $msg
} {0 5}

test datatable.692 {datatable0 find (missing arg)} {
    list [catch {datatable0 find} msg] $msg
} {1 {wrong # args: should be "datatable0 find node ?switches?"}}

test datatable.693 {datatable0 find 0} {
    list [catch {datatable0 find 0} msg] $msg
} {0 {1 2 3 4 13 5 6 8 0}}

test datatable.694 {datatable0 find root} {
    list [catch {datatable0 find root} msg] $msg
} {0 {1 2 3 4 13 5 6 8 0}}

test datatable.695 {datatable0 find 0 -glob node*} {
    list [catch {datatable0 find root -glob node*} msg] $msg
} {0 {1 2 3 4 13 5 6}}

test datatable.696 {datatable0 find 0 -glob nobody} {
    list [catch {datatable0 find root -glob nobody} msg] $msg
} {0 {}}

test datatable.697 {datatable0 find 0 -regexp {node[0-3]}} {
    list [catch {datatable0 find root -regexp {node[0-3]}} msg] $msg
} {0 {1 2 3 13}}

test datatable.698 {datatable0 find 0 -regexp {.*[A-Z].*}} {
    list [catch {datatable0 find root -regexp {.*[A-Z].*}} msg] $msg
} {0 8}

test datatable.699 {datatable0 find 0 -exact myLabel} {
    list [catch {datatable0 find root -exact myLabel} msg] $msg
} {0 8}

test datatable.700 {datatable0 find 0 -exact myLabel -invert} {
    list [catch {datatable0 find root -exact myLabel -invert} msg] $msg
} {0 {1 2 3 4 13 5 6 0}}

test datatable.701 {datatable0 find 3 -exact node3} {
    list [catch {datatable0 find 3 -exact node3} msg] $msg
} {0 3}

test datatable.702 {datatable0 find 0 -nocase -exact mylabel} {
    list [catch {datatable0 find 0 -nocase -exact mylabel} msg] $msg
} {0 8}

test datatable.703 {datatable0 find 0 -nocase} {
    list [catch {datatable0 find 0 -nocase} msg] $msg
} {0 {1 2 3 4 13 5 6 8 0}}

test datatable.704 {datatable0 find 0 -path -nocase -glob *node1* } {
    list [catch {datatable0 find 0 -path -nocase -glob *node1*} msg] $msg
} {0 {1 13}}

test datatable.705 {datatable0 find 0 -count 5 } {
    list [catch {datatable0 find 0 -count 5} msg] $msg
} {0 {1 2 3 4 13}}

test datatable.706 {datatable0 find 0 -count -5 } {
    list [catch {datatable0 find 0 -count -5} msg] $msg
} {1 {bad value "-5": can't be negative}}

test datatable.707 {datatable0 find 0 -count badValue } {
    list [catch {datatable0 find 0 -count badValue} msg] $msg
} {1 {expected integer but got "badValue"}}

test datatable.708 {datatable0 find 0 -count badValue } {
    list [catch {datatable0 find 0 -count badValue} msg] $msg
} {1 {expected integer but got "badValue"}}

test datatable.709 {datatable0 find 0 -leafonly} {
    list [catch {datatable0 find 0 -leafonly} msg] $msg
} {0 {1 2 3 4 13 6 8}}

test datatable.710 {datatable0 find 0 -leafonly -glob {node[18]}} {
    list [catch {datatable0 find 0 -glob {node[18]} -leafonly} msg] $msg
} {0 1}

test datatable.711 {datatable0 find 0 -depth 0} {
    list [catch {datatable0 find 0 -depth 0} msg] $msg
} {0 0}

test datatable.712 {datatable0 find 0 -depth 1} {
    list [catch {datatable0 find 0 -depth 1} msg] $msg
} {0 {1 2 3 4 5 6 8 0}}

test datatable.713 {datatable0 find 0 -depth 2} {
    list [catch {datatable0 find 0 -depth 2} msg] $msg
} {0 {1 2 3 4 13 5 6 8 0}}

test datatable.714 {datatable0 find 0 -depth 20} {
    list [catch {datatable0 find 0 -depth 20} msg] $msg
} {0 {1 2 3 4 13 5 6 8 0}}

test datatable.715 {datatable0 find 1 -depth 0} {
    list [catch {datatable0 find 1 -depth 0} msg] $msg
} {0 1}

test datatable.716 {datatable0 find 1 -depth 1} {
    list [catch {datatable0 find 1 -depth 1} msg] $msg
} {0 1}

test datatable.717 {datatable0 find 1 -depth 2} {
    list [catch {datatable0 find 1 -depth 2} msg] $msg
} {0 1}

test datatable.718 {datatable0 find all} {
    list [catch {datatable0 find all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.719 {datatable0 find badTag} {
    list [catch {datatable0 find badTag} msg] $msg
} {1 {can't find tag or id "badTag" in ::datatable0}}

test datatable.720 {datatable0 find 0 -addtag hi} {
    list [catch {datatable0 find 0 -addtag hi} msg] $msg
} {0 {1 2 3 4 13 5 6 8 0}}

test datatable.721 {datatable0 find 0 -addtag all} {
    list [catch {datatable0 find 0 -addtag all} msg] $msg
} {0 {1 2 3 4 13 5 6 8 0}}

test datatable.722 {datatable0 find 0 -addtag root} {
    list [catch {datatable0 find 0 -addtag root} msg] $msg
} {1 {can't add reserved tag "root"}}

test datatable.723 {datatable0 find 0 -exec {lappend list} -leafonly} {
    list [catch {
	set list {}
	datatable0 find 0 -exec {lappend list} -leafonly
	set list
	} msg] $msg
} {0 {1 2 3 4 13 6 8}}

test datatable.724 {datatable0 find 0 -tag root} {
    list [catch {datatable0 find 0 -tag root} msg] $msg
} {0 0}

test datatable.725 {datatable0 find 0 -tag myTag} {
    list [catch {datatable0 find 0 -tag myTag} msg] $msg
} {0 5}

test datatable.726 {datatable0 find 0 -tag badTag} {
    list [catch {datatable0 find 0 -tag badTag} msg] $msg
} {0 {}}

test datatable.727 {datatable0 tag (missing args)} {
    list [catch {datatable0 tag} msg] $msg
} {1 {wrong # args: should be "datatable0 tag args"}}

test datatable.728 {datatable0 tag badOp} {
    list [catch {datatable0 tag badOp} msg] $msg
} {1 {bad operation "badOp": should be one of...
  datatable0 tag add tag node...
  datatable0 tag delete tag node...
  datatable0 tag dump tag...
  datatable0 tag exists node tag...
  datatable0 tag forget tag...
  datatable0 tag get node ?pattern...?
  datatable0 tag names ?node...?
  datatable0 tag nodes tag ?tag...?
  datatable0 tag set node tag...
  datatable0 tag unset node tag...}}

test datatable.729 {datatable0 tag add} {
    list [catch {datatable0 tag add} msg] $msg
} {1 {wrong # args: should be "datatable0 tag add tag node..."}}

test datatable.730 {datatable0 tag add tag} {
    list [catch {datatable0 tag add tag} msg] $msg
} {1 {wrong # args: should be "datatable0 tag add tag node..."}}

test datatable.731 {datatable0 tag add tag badNode} {
    list [catch {datatable0 tag add tag badNode} msg] $msg
} {1 {can't find tag or id "badNode" in ::datatable0}}

test datatable.732 {datatable0 tag add newTag root} {
    list [catch {datatable0 tag add newTag root} msg] $msg
} {0 {}}

test datatable.733 {datatable0 tag add newTag all} {
    list [catch {datatable0 tag add newTag all} msg] $msg
} {0 {}}

test datatable.734 {datatable0 tag add tag2 0 1 2 3 4} {
    list [catch {datatable0 tag add tag2 0 1 2 3 4} msg] $msg
} {0 {}}

test datatable.735 {datatable0 tag add tag2 0 1 2 3 4 1000} {
    list [catch {datatable0 tag add tag2 0 1 2 3 4 1000} msg] $msg
} {1 {can't find tag or id "1000" in ::datatable0}}

test datatable.736 {datatable0 tag names} {
    list [catch {datatable0 tag names} msg] $msg
} {0 {all hi myTag myTag1 myTag2 newTag root tag2 thisTag}}

test datatable.737 {datatable0 tag names badNode} {
    list [catch {datatable0 tag names badNode} msg] $msg
} {1 {can't find tag or id "badNode" in ::datatable0}}

test datatable.738 {datatable0 tag names all} {
    list [catch {datatable0 tag names all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.739 {datatable0 tag names root} {
    list [catch {datatable0 tag names root} msg] $msg
} {0 {all hi newTag root tag2}}

test datatable.740 {datatable0 tag names 0 1} {
    list [catch {datatable0 tag names 0 1} msg] $msg
} {0 {all hi newTag root tag2}}

test datatable.741 {datatable0 tag nodes (missing arg)} {
    list [catch {datatable0 tag nodes} msg] $msg
} {1 {wrong # args: should be "datatable0 tag nodes tag ?tag...?"}}

test datatable.742 {datatable0 tag nodes root badTag} {
    list [catch {datatable0 tag nodes root badTag} msg] $msg
} {1 {can't find a tag "badTag"}}

test datatable.743 {datatable0 tag nodes root tag2} {
    list [catch {datatable0 tag nodes root tag2} msg] $msg
} {0 {0 1 2 3 4}}

test datatable.744 {datatable0 ancestor (missing arg)} {
    list [catch {datatable0 ancestor} msg] $msg
} {1 {wrong # args: should be "datatable0 ancestor node1 node2"}}

test datatable.745 {datatable0 ancestor 0 (missing arg)} {
    list [catch {datatable0 ancestor 0} msg] $msg
} {1 {wrong # args: should be "datatable0 ancestor node1 node2"}}

test datatable.746 {datatable0 ancestor 0 10} {
    list [catch {datatable0 ancestor 0 10} msg] $msg
} {1 {can't find tag or id "10" in ::datatable0}}

test datatable.747 {datatable0 ancestor 0 4} {
    list [catch {datatable0 ancestor 0 4} msg] $msg
} {0 0}

test datatable.748 {datatable0 ancestor 1 8} {
    list [catch {datatable0 ancestor 1 8} msg] $msg
} {0 0}

test datatable.749 {datatable0 ancestor root 0} {
    list [catch {datatable0 ancestor root 0} msg] $msg
} {0 0}

test datatable.750 {datatable0 ancestor 8 8} {
    list [catch {datatable0 ancestor 8 8} msg] $msg
} {0 8}

test datatable.751 {datatable0 ancestor 0 all} {
    list [catch {datatable0 ancestor 0 all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.752 {datatable0 ancestor 7 9} {
    list [catch {
	set n1 1; set n2 1;
	for { set i 0 } { $i < 4 } { incr i } {
	    set n1 [datatable0 insert $n1]
	    set n2 [datatable0 insert $n2]
	}
	datatable0 ancestor $n1 $n2
	} msg] $msg
} {0 1}

test datatable.753 {datatable0 path (missing arg)} {
    list [catch {datatable0 path} msg] $msg
} {1 {wrong # args: should be "datatable0 path node"}}

test datatable.754 {datatable0 path root} {
    list [catch {datatable0 path root} msg] $msg
} {0 {}}

test datatable.755 {datatable0 path 0} {
    list [catch {datatable0 path 0} msg] $msg
} {0 {}}

test datatable.756 {datatable0 path 15} {
    list [catch {datatable0 path 15} msg] $msg
} {0 {node1 node15}}

test datatable.757 {datatable0 path all} {
    list [catch {datatable0 path all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.758 {datatable0 path 0 1 2 4 (extra args)} {
    list [catch {datatable0 path 0 1 2 4} msg] $msg
} {1 {wrong # args: should be "datatable0 path node"}}

test datatable.759 {datatable0 tag forget} {
    list [catch {datatable0 tag forget} msg] $msg
} {1 {wrong # args: should be "datatable0 tag forget tag..."}}

test datatable.760 {datatable0 tag forget badTag} {
    list [catch {
	datatable0 tag forget badTag
	lsort [datatable0 tag names]
    } msg] $msg
} {0 {all hi myTag myTag1 myTag2 newTag root tag2 thisTag}}

test datatable.761 {datatable0 tag forget hi} {
    list [catch {
	datatable0 tag forget hi
	lsort [datatable0 tag names]
    } msg] $msg
} {0 {all myTag myTag1 myTag2 newTag root tag2 thisTag}}

test datatable.762 {datatable0 tag forget tag1 tag2} {
    list [catch {
	datatable0 tag forget myTag1 myTag2
	lsort [datatable0 tag names]
    } msg] $msg
} {0 {all myTag newTag root tag2 thisTag}}

test datatable.763 {datatable0 tag forget all} {
    list [catch {
	datatable0 tag forget all
	lsort [datatable0 tag names]
    } msg] $msg
} {0 {all myTag newTag root tag2 thisTag}}

test datatable.764 {datatable0 tag forget root} {
    list [catch {
	datatable0 tag forget root
	lsort [datatable0 tag names]
    } msg] $msg
} {0 {all myTag newTag root tag2 thisTag}}

test datatable.765 {datatable0 tag delete} {
    list [catch {datatable0 tag delete} msg] $msg
} {1 {wrong # args: should be "datatable0 tag delete tag node..."}}

test datatable.766 {datatable0 tag delete tag} {
    list [catch {datatable0 tag delete tag} msg] $msg
} {1 {wrong # args: should be "datatable0 tag delete tag node..."}}

test datatable.767 {datatable0 tag delete tag 0} {
    list [catch {datatable0 tag delete tag 0} msg] $msg
} {0 {}}

test datatable.768 {datatable0 tag delete root 0} {
    list [catch {datatable0 tag delete root 0} msg] $msg
} {1 {can't delete reserved tag "root"}}

test datatable.769 {datatable0 attach} {
    list [catch {datatable0 attach} msg] $msg
} {0 ::datatable0}

test datatable.770 {datatable0 attach datatable2 datatable3} {
    list [catch {datatable0 attach datatable2 datatable3} msg] $msg
} {1 {wrong # args: should be "datatable0 attach ?datatable?"}}

test datatable.771 {datatable1 attach datatable0} {
    list [catch {
	blt::datatable create
	datatable1 attach datatable0
	datatable1 dump 0
	} msg] $msg
} {0 {-1 0 {::datatable0} {} {}
0 2 {::datatable0 node2} {} {}
2 1 {::datatable0 node2 node1} {} {}
1 14 {::datatable0 node2 node1 node14} {} {}
14 16 {::datatable0 node2 node1 node14 node16} {} {}
16 18 {::datatable0 node2 node1 node14 node16 node18} {} {}
18 20 {::datatable0 node2 node1 node14 node16 node18 node20} {} {}
1 15 {::datatable0 node2 node1 node15} {} {}
15 17 {::datatable0 node2 node1 node15 node17} {} {}
17 19 {::datatable0 node2 node1 node15 node17 node19} {} {}
19 21 {::datatable0 node2 node1 node15 node17 node19 node21} {} {}
0 3 {::datatable0 node3} {} {}
0 4 {::datatable0 node4} {} {}
0 5 {::datatable0 node5} {} {}
5 13 {::datatable0 node5 node13} {} {}
0 6 {::datatable0 node6} {} {}
0 8 {::datatable0 myLabel} {} {}
}}

test datatable.772 {datatable1 attach} {
    list [catch {datatable1 attach} msg] $msg
} {0 ::datatable0}


test datatable.773 {blt::datatable destroy datatable1} {
    list [catch {blt::datatable destroy datatable1} msg] $msg
} {0 {}}

test datatable.774 {datatable0 find root -badFlag} {
    list [catch {datatable0 find root -badFlag} msg] $msg
} {1 {unknown option "-badFlag"}}

test datatable.775 {datatable0 find root -order} {
    list [catch {datatable0 find root -order} msg] $msg
} {1 {value for "-order" missing}}

test datatable.776 {datatable0 find root ...} {
    list [catch {datatable0 find root -order preorder -order postorder -order inorder} msg] $msg
} {0 {20 18 16 14 1 21 19 17 15 2 0 3 4 13 5 6 8}}

test datatable.777 {datatable0 find root -order preorder} {
    list [catch {datatable0 find root -order preorder} msg] $msg
} {0 {0 2 1 14 16 18 20 15 17 19 21 3 4 5 13 6 8}}

test datatable.778 {datatable0 find root -order postorder} {
    list [catch {datatable0 find root -order postorder} msg] $msg
} {0 {20 18 16 14 21 19 17 15 1 2 3 4 13 5 6 8 0}}

test datatable.779 {datatable0 find root -order inorder} {
    list [catch {datatable0 find root -order inorder} msg] $msg
} {0 {20 18 16 14 1 21 19 17 15 2 0 3 4 13 5 6 8}}

test datatable.780 {datatable0 find root -order breadthfirst} {
    list [catch {datatable0 find root -order breadthfirst} msg] $msg
} {0 {0 2 3 4 5 6 8 1 13 14 15 16 17 18 19 20 21}}

test datatable.781 {datatable0 set all key1 myValue} {
    list [catch {datatable0 set all key1 myValue} msg] $msg
} {0 {}}

test datatable.782 {datatable0 set 15 key1 123} {
    list [catch {datatable0 set 15 key1 123} msg] $msg
} {0 {}}

test datatable.783 {datatable0 set 16 key1 1234 key2 abc} {
    list [catch {datatable0 set 16 key1 123 key2 abc} msg] $msg
} {0 {}}

test datatable.784 {datatable0 find root -key } {
    list [catch {datatable0 find root -key} msg] $msg
} {1 {value for "-key" missing}}

test datatable.785 {datatable0 find root -key noKey} {
    list [catch {datatable0 find root -key noKey} msg] $msg
} {0 {}}

test datatable.786 {datatable0 find root -key key1} {
    list [catch {datatable0 find root -key key1} msg] $msg
} {0 {20 18 16 14 21 19 17 15 1 2 3 4 13 5 6 8 0}}

test datatable.787 {datatable0 find root -key key2} {
    list [catch {datatable0 find root -key key2} msg] $msg
} {0 16}

test datatable.788 {datatable0 find root -key key2 -exact notThere } {
    list [catch {datatable0 find root -key key2 -exact notThere } msg] $msg
} {0 {}}

test datatable.789 {datatable0 find root -key key1 -glob notThere } {
    list [catch {datatable0 find root -key key2 -exact notThere } msg] $msg
} {0 {}}

test datatable.790 {datatable0 find root -key badKey -regexp notThere } {
    list [catch {datatable0 find root -key key2 -exact notThere } msg] $msg
} {0 {}}

test datatable.791 {datatable0 find root -key key1 -glob 12*} {
    list [catch {datatable0 find root -key key1 -glob 12*} msg] $msg
} {0 {16 15}}

test datatable.792 {datatable0 sort} {
    list [catch {datatable0 sort} msg] $msg
} {1 {wrong # args: should be "datatable0 sort ?flags...?"}}

test datatable.793 {datatable0 sort all} {
    list [catch {datatable0 sort all} msg] $msg
} {1 {more than one node tagged as "all"}}

test datatable.794 {datatable0 sort -recurse} {
    list [catch {datatable0 sort -recurse} msg] $msg
} {1 {can't find tag or id "-recurse" in ::datatable0}}

test datatable.795 {datatable0 sort 0} {
    list [catch {datatable0 sort 0} msg] $msg
} {0 {8 2 3 4 5 6}}

test datatable.796 {datatable0 sort 0 -recurse} {
    list [catch {datatable0 sort 0 -recurse} msg] $msg
} {0 {0 8 1 2 3 4 5 6 13 14 15 16 17 18 19 20 21}}

test datatable.797 {datatable0 sort 0 -decreasing -key} {
    list [catch {datatable0 sort 0 -decreasing -key} msg] $msg
} {1 {value for "-key" missing}}

test datatable.798 {datatable0 sort 0 -re} {
    list [catch {datatable0 sort 0 -re} msg] $msg
} {1 {ambiguous option "-re"}}

test datatable.799 {datatable0 sort 0 -decreasing} {
    list [catch {datatable0 sort 0 -decreasing} msg] $msg
} {0 {6 5 4 3 2 8}}

test datatable.800 {datatable0 sort 0} {
    list [catch {
	set list {}
	foreach n [datatable0 sort 0] {
	    lappend list [datatable0 label $n]
	}	
	set list
    } msg] $msg
} {0 {myLabel node2 node3 node4 node5 node6}}

test datatable.801 {datatable0 sort 0 -decreasing} {
    list [catch {datatable0 sort 0 -decreasing} msg] $msg
} {0 {6 5 4 3 2 8}}


test datatable.802 {datatable0 sort 0 -decreasing -key} {
    list [catch {datatable0 sort 0 -decreasing -key} msg] $msg
} {1 {value for "-key" missing}}

test datatable.803 {datatable0 sort 0 -decreasing -key key1} {
    list [catch {datatable0 sort 0 -decreasing -key key1} msg] $msg
} {0 {8 6 5 4 3 2}}

test datatable.804 {datatable0 sort 0 -decreasing -recurse -key key1} {
    list [catch {datatable0 sort 0 -decreasing -recurse -key key1} msg] $msg
} {0 {15 16 0 1 2 3 4 5 6 8 13 14 17 18 19 20 21}}

test datatable.805 {datatable0 sort 0 -decreasing -key key1} {
    list [catch {
	set list {}
	foreach n [datatable0 sort 0 -decreasing -key key1] {
	    lappend list [datatable0 get $n key1]
	}
	set list
    } msg] $msg
} {0 {myValue myValue myValue myValue myValue myValue}}


test datatable.806 {datatable0 index 1-firstchild} {
    list [catch {datatable0 index 1-firstchild} msg] $msg
} {0 14}

test datatable.807 {datatable0 index root-to-firstchild} {
    list [catch {datatable0 index root-to-firstchild} msg] $msg
} {0 2}

test datatable.808 {datatable0 label root-to-parent} {
    list [catch {datatable0 label root-to-parent} msg] $msg
} {1 {can't find tag or id "root-to-parent" in ::datatable0}}

test datatable.809 {datatable0 index root-to-parent} {
    list [catch {datatable0 index root-to-parent} msg] $msg
} {0 -1}

test datatable.810 {datatable0 index root-to-lastchild} {
    list [catch {datatable0 index root-to-lastchild} msg] $msg
} {0 8}

test datatable.811 {datatable0 index root-to-next} {
    list [catch {datatable0 index root-to-next} msg] $msg
} {0 2}

test datatable.812 {datatable0 index root-to-previous} {
    list [catch {datatable0 index root-to-previous} msg] $msg
} {0 -1}

test datatable.813 {datatable0 label root-to-previous} {
    list [catch {datatable0 label root-to-previous} msg] $msg
} {1 {can't find tag or id "root-to-previous" in ::datatable0}}

test datatable.814 {datatable0 index 1-previous} {
    list [catch {datatable0 index 1-previous} msg] $msg
} {0 2}

test datatable.815 {datatable0 label root-to-badModifier} {
    list [catch {datatable0 label root-to-badModifier} msg] $msg
} {1 {can't find tag or id "root-to-badModifier" in ::datatable0}}

test datatable.816 {datatable0 index root-to-badModifier} {
    list [catch {datatable0 index root-to-badModifier} msg] $msg
} {0 -1}

test datatable.817 {datatable0 index root-to-firstchild-to-parent} {
    list [catch {datatable0 index root-to-firstchild-to-parent} msg] $msg
} {0 0}

test datatable.818 {datatable0 trace} {
    list [catch {datatable0 trace} msg] $msg
} {1 {wrong # args: should be one of...
  datatable0 trace create node key how command
  datatable0 trace delete id...
  datatable0 trace info id
  datatable0 trace names }}


test datatable.819 {datatable0 trace create} {
    list [catch {datatable0 trace create} msg] $msg
} {1 {wrong # args: should be "datatable0 trace create node key how command"}}

test datatable.820 {datatable0 trace create root} {
    list [catch {datatable0 trace create root} msg] $msg
} {1 {wrong # args: should be "datatable0 trace create node key how command"}}

test datatable.821 {datatable0 trace create root * } {
    list [catch {datatable0 trace create root * } msg] $msg
} {1 {wrong # args: should be "datatable0 trace create node key how command"}}

test datatable.822 {datatable0 trace create root * rwuc} {
    list [catch {datatable0 trace create root * rwuc} msg] $msg
} {1 {wrong # args: should be "datatable0 trace create node key how command"}}

proc Doit args { global mylist; lappend mylist $args }

test datatable.823 {datatable0 trace create all newKey rwuc Doit} {
    list [catch {datatable0 trace create all newKey rwuc Doit} msg] $msg
} {0 trace0}

test datatable.824 {datatable0 trace info trace0} {
    list [catch {datatable0 trace info trace0} msg] $msg
} {0 {all newKey {} Doit}}

test datatable.825 {test create trace} {
    list [catch {
	set mylist {}
	datatable0 set all newKey 20
	set mylist
	} msg] $msg
} {0 {{::datatable0 0 newKey wc} {::datatable0 2 newKey wc} {::datatable0 1 newKey wc} {::datatable0 14 newKey wc} {::datatable0 16 newKey wc} {::datatable0 18 newKey wc} {::datatable0 20 newKey wc} {::datatable0 15 newKey wc} {::datatable0 17 newKey wc} {::datatable0 19 newKey wc} {::datatable0 21 newKey wc} {::datatable0 3 newKey wc} {::datatable0 4 newKey wc} {::datatable0 5 newKey wc} {::datatable0 13 newKey wc} {::datatable0 6 newKey wc} {::datatable0 8 newKey wc}}}

test datatable.826 {test read trace} {
    list [catch {
	set mylist {}
	datatable0 get root newKey
	set mylist
	} msg] $msg
} {0 {{::datatable0 0 newKey r}}}

test datatable.827 {test write trace} {
    list [catch {
	set mylist {}
	datatable0 set all newKey 21
	set mylist
	} msg] $msg
} {0 {{::datatable0 0 newKey w} {::datatable0 2 newKey w} {::datatable0 1 newKey w} {::datatable0 14 newKey w} {::datatable0 16 newKey w} {::datatable0 18 newKey w} {::datatable0 20 newKey w} {::datatable0 15 newKey w} {::datatable0 17 newKey w} {::datatable0 19 newKey w} {::datatable0 21 newKey w} {::datatable0 3 newKey w} {::datatable0 4 newKey w} {::datatable0 5 newKey w} {::datatable0 13 newKey w} {::datatable0 6 newKey w} {::datatable0 8 newKey w}}}

test datatable.828 {test unset trace} {
    list [catch {
	set mylist {}
	datatable0 set all newKey 21
	set mylist
	} msg] $msg
} {0 {{::datatable0 0 newKey w} {::datatable0 2 newKey w} {::datatable0 1 newKey w} {::datatable0 14 newKey w} {::datatable0 16 newKey w} {::datatable0 18 newKey w} {::datatable0 20 newKey w} {::datatable0 15 newKey w} {::datatable0 17 newKey w} {::datatable0 19 newKey w} {::datatable0 21 newKey w} {::datatable0 3 newKey w} {::datatable0 4 newKey w} {::datatable0 5 newKey w} {::datatable0 13 newKey w} {::datatable0 6 newKey w} {::datatable0 8 newKey w}}}

test datatable.829 {datatable0 trace delete} {
    list [catch {datatable0 trace delete} msg] $msg
} {0 {}}

test datatable.830 {datatable0 trace delete badId} {
    list [catch {datatable0 trace delete badId} msg] $msg
} {1 {unknown trace "badId"}}

test datatable.831 {datatable0 trace delete trace0} {
    list [catch {datatable0 trace delete trace0} msg] $msg
} {0 {}}

test datatable.832 {test create trace} {
    list [catch {
	set mylist {}
	datatable0 set all newKey 20
	set mylist
	} msg] $msg
} {0 {}}

test datatable.833 {test unset trace} {
    list [catch {
	set mylist {}
	datatable0 unset all newKey
	set mylist
	} msg] $msg
} {0 {}}


test datatable.834 {datatable0 notify} {
    list [catch {datatable0 notify} msg] $msg
} {1 {wrong # args: should be one of...
  datatable0 notify create ?flags? command
  datatable0 notify delete notifyId...
  datatable0 notify info notifyId
  datatable0 notify names }}


test datatable.835 {datatable0 notify create} {
    list [catch {datatable0 notify create} msg] $msg
} {1 {wrong # args: should be "datatable0 notify create ?flags? command"}}

test datatable.836 {datatable0 notify create -allevents} {
    list [catch {datatable0 notify create -allevents Doit} msg] $msg
} {0 notify0}

test datatable.837 {datatable0 notify info notify0} {
    list [catch {datatable0 notify info notify0} msg] $msg
} {0 {notify0 {-create -delete -move -sort -relabel} {Doit}}}

test datatable.838 {datatable0 notify info badId} {
    list [catch {datatable0 notify info badId} msg] $msg
} {1 {unknown notify name "badId"}}

test datatable.839 {datatable0 notify info} {
    list [catch {datatable0 notify info} msg] $msg
} {1 {wrong # args: should be "datatable0 notify info notifyId"}}

test datatable.840 {datatable0 notify names} {
    list [catch {datatable0 notify names} msg] $msg
} {0 notify0}


test datatable.841 {test create notify} {
    list [catch {
	set mylist {}
	datatable0 insert 1 -tags test
	set mylist
	} msg] $msg
} {0 {{-create 22}}}

test datatable.842 {test move notify} {
    list [catch {
	set mylist {}
	datatable0 move 8 test
	set mylist
	} msg] $msg
} {0 {{-move 8}}}

test datatable.843 {test sort notify} {
    list [catch {
	set mylist {}
	datatable0 sort 0 -reorder 
	set mylist
	} msg] $msg
} {0 {{-sort 0}}}

test datatable.844 {test relabel notify} {
    list [catch {
	set mylist {}
	datatable0 label test "newLabel"
	set mylist
	} msg] $msg
} {0 {{-relabel 22}}}

test datatable.845 {test delete notify} {
    list [catch {
	set mylist {}
	datatable0 delete test
	set mylist
	} msg] $msg
} {0 {{-delete 8} {-delete 22}}}


test datatable.846 {datatable0 notify delete badId} {
    list [catch {datatable0 notify delete badId} msg] $msg
} {1 {unknown notify name "badId"}}


test datatable.847 {test create notify} {
    list [catch {
	set mylist {}
	datatable0 set all newKey 20
	set mylist
	} msg] $msg
} {0 {}}

test datatable.848 {test delete notify} {
    list [catch {
	set mylist {}
	datatable0 unset all newKey
	set mylist
	} msg] $msg
} {0 {}}

test datatable.849 {test delete notify} {
    list [catch {
	set mylist {}
	datatable0 unset all newKey
	set mylist
	} msg] $msg
} {0 {}}

test datatable.850 {datatable0 copy} {
    list [catch {datatable0 copy} msg] $msg
} {1 {wrong # args: should be "datatable0 copy srcNode ?destDatatable? destNode ?switches?"}}

test datatable.851 {datatable0 copy root} {
    list [catch {datatable0 copy root} msg] $msg
} {1 {wrong # args: should be "datatable0 copy srcNode ?destDatatable? destNode ?switches?"}}

test datatable.852 {datatable0 copy root 14} {
    list [catch {datatable0 copy root 14} msg] $msg
} {0 23}

test datatable.853 {datatable0 copy 14 root} {
    list [catch {datatable0 copy 14 root} msg] $msg
} {0 24}

test datatable.854 {datatable0 copy root 14 -recurse} {
    list [catch {datatable0 copy root 14 -recurse} msg] $msg
} {1 {can't make cyclic copy: source node is an ancestor of the destination}}

test datatable.855 {datatable0 copy 2 3 -recurse -tags} {
    list [catch {datatable0 copy 2 3 -recurse -tags} msg] $msg
} {0 25}

test datatable.856 {datatable0 copy 2 3 -recurse -overwrite} {
    list [catch {
	blt::datatable create datatable1
	foreach node [datatable0 children root] {
	    datatable0 copy $node datatable1 root -recurse 
	}
	foreach node [datatable0 children root] {
	    datatable0 copy $node datatable1 root -recurse 
	}
	datatable1 dump root
    } msg] $msg
} {0 {-1 0 {::datatable1} {} {}
0 1 {::datatable1 node2} {key1 myValue} {}
1 2 {::datatable1 node2 node1} {key1 myValue} {}
2 3 {::datatable1 node2 node1 node14} {key1 myValue} {}
3 4 {::datatable1 node2 node1 node14 node16} {key1 123 key2 abc} {}
4 5 {::datatable1 node2 node1 node14 node16 node18} {key1 myValue} {}
5 6 {::datatable1 node2 node1 node14 node16 node18 node20} {key1 myValue} {}
3 7 {::datatable1 node2 node1 node14 ::datatable0} {key1 myValue} {}
2 8 {::datatable1 node2 node1 node15} {key1 123} {}
8 9 {::datatable1 node2 node1 node15 node17} {key1 myValue} {}
9 10 {::datatable1 node2 node1 node15 node17 node19} {key1 myValue} {}
10 11 {::datatable1 node2 node1 node15 node17 node19 node21} {key1 myValue} {}
0 12 {::datatable1 node3} {key1 myValue} {}
12 13 {::datatable1 node3 node2} {key1 myValue} {}
13 14 {::datatable1 node3 node2 node1} {key1 myValue} {}
14 15 {::datatable1 node3 node2 node1 node14} {key1 myValue} {}
15 16 {::datatable1 node3 node2 node1 node14 node16} {key1 123 key2 abc} {}
16 17 {::datatable1 node3 node2 node1 node14 node16 node18} {key1 myValue} {}
17 18 {::datatable1 node3 node2 node1 node14 node16 node18 node20} {key1 myValue} {}
15 19 {::datatable1 node3 node2 node1 node14 ::datatable0} {key1 myValue} {}
14 20 {::datatable1 node3 node2 node1 node15} {key1 123} {}
20 21 {::datatable1 node3 node2 node1 node15 node17} {key1 myValue} {}
21 22 {::datatable1 node3 node2 node1 node15 node17 node19} {key1 myValue} {}
22 23 {::datatable1 node3 node2 node1 node15 node17 node19 node21} {key1 myValue} {}
0 24 {::datatable1 node4} {key1 myValue} {}
0 25 {::datatable1 node5} {key1 myValue} {}
25 26 {::datatable1 node5 node13} {key1 myValue} {}
0 27 {::datatable1 node6} {key1 myValue} {}
0 28 {::datatable1 node14} {key1 myValue} {}
0 29 {::datatable1 node2} {key1 myValue} {}
29 30 {::datatable1 node2 node1} {key1 myValue} {}
30 31 {::datatable1 node2 node1 node14} {key1 myValue} {}
31 32 {::datatable1 node2 node1 node14 node16} {key1 123 key2 abc} {}
32 33 {::datatable1 node2 node1 node14 node16 node18} {key1 myValue} {}
33 34 {::datatable1 node2 node1 node14 node16 node18 node20} {key1 myValue} {}
31 35 {::datatable1 node2 node1 node14 ::datatable0} {key1 myValue} {}
30 36 {::datatable1 node2 node1 node15} {key1 123} {}
36 37 {::datatable1 node2 node1 node15 node17} {key1 myValue} {}
37 38 {::datatable1 node2 node1 node15 node17 node19} {key1 myValue} {}
38 39 {::datatable1 node2 node1 node15 node17 node19 node21} {key1 myValue} {}
0 40 {::datatable1 node3} {key1 myValue} {}
40 41 {::datatable1 node3 node2} {key1 myValue} {}
41 42 {::datatable1 node3 node2 node1} {key1 myValue} {}
42 43 {::datatable1 node3 node2 node1 node14} {key1 myValue} {}
43 44 {::datatable1 node3 node2 node1 node14 node16} {key1 123 key2 abc} {}
44 45 {::datatable1 node3 node2 node1 node14 node16 node18} {key1 myValue} {}
45 46 {::datatable1 node3 node2 node1 node14 node16 node18 node20} {key1 myValue} {}
43 47 {::datatable1 node3 node2 node1 node14 ::datatable0} {key1 myValue} {}
42 48 {::datatable1 node3 node2 node1 node15} {key1 123} {}
48 49 {::datatable1 node3 node2 node1 node15 node17} {key1 myValue} {}
49 50 {::datatable1 node3 node2 node1 node15 node17 node19} {key1 myValue} {}
50 51 {::datatable1 node3 node2 node1 node15 node17 node19 node21} {key1 myValue} {}
0 52 {::datatable1 node4} {key1 myValue} {}
0 53 {::datatable1 node5} {key1 myValue} {}
53 54 {::datatable1 node5 node13} {key1 myValue} {}
0 55 {::datatable1 node6} {key1 myValue} {}
0 56 {::datatable1 node14} {key1 myValue} {}
}}

puts stderr "done testing datatablecmd.tcl"

exit 0

















