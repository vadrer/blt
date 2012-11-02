
# Test switches
#
package require BLT
package require blt_sftp

if {[info procs test] != "test"} {
    source defs
}

if [file exists ../library] {
    set blt_library ../library
}

set file [info script]
set host wolverine
#set VERBOSE 1


if 1 {
test sftp.1 {sftp no args} {
    list [catch {blt::sftp} msg] $msg
} {1 {wrong # args: should be one of...
  blt::sftp create ?name? ?switches?
  blt::sftp destroy name...
  blt::sftp names ?pattern?...}}

test sftp.2 {sftp create -host $host} {
    list [catch { blt::sftp create -host $host } msg] $msg
} {0 ::sftp0}

test sftp.3 {sftp create -host $host} {
    list [catch { blt::sftp create -host $host } msg] $msg
} {0 ::sftp1}

test sftp.4 {sftp create #auto -host $host} {
    list [catch {blt::sftp create #auto -host $host} msg] $msg
} {0 ::sftp2}

test sftp.5 {sftp create #auto.suffix -host $host } {
    list [catch {blt::sftp create #auto.suffix -host $host} msg] $msg
} {0 ::sftp3.suffix}

test sftp.6 {sftp create prefix.#auto -host $host } {
    list [catch {blt::sftp create prefix.#auto -host $host } msg] $msg
} {0 ::prefix.sftp4}

test sftp.7 {sftp create prefix.#auto.suffix -host $host } {
    list [catch {blt::sftp create prefix.#auto.suffix -host $host} msg] $msg
} {0 ::prefix.sftp5.suffix}

test sftp.8 {sftp create prefix.#auto.suffix.#auto -host $host } {
    list [catch {
	blt::sftp create prefix.#auto.suffix.#auto -host $host 
    } msg] $msg
} {0 ::prefix.sftp6.suffix.#auto}

test sftp.9 {sftp names} {
    list [catch {blt::sftp names} msg] [lsort $msg]
} {0 {::prefix.sftp4 ::prefix.sftp5.suffix ::prefix.sftp6.suffix.#auto ::sftp0 ::sftp1 ::sftp2 ::sftp3.suffix}}

test sftp.10 {sftp names *sftp0*} {
    list [catch {blt::sftp names *sftp0*} msg] $msg
} {0 ::sftp0}

test sftp.11 {sftp names badName} {
    list [catch {blt::sftp names badName} msg] $msg
} {0 {}}

test sftp.12 {sftp names *sftp0* *sftp5*} {
    list [catch {blt::sftp names *sftp0* *sftp5*} msg] [lsort $msg]
} {0 {::prefix.sftp5.suffix ::sftp0}}

test sftp.13 {sftp destroy [sftp names]} {
    list [catch {eval blt::sftp destroy [blt::sftp names]} msg] $msg
} {0 {}}

test sftp.14 {sftp names} {
    list [catch {blt::sftp names} msg] $msg
} {0 {}}

test sftp.15 {sftp create -badSwitch} {
    list [catch {blt::sftp create -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -user string
   -host string
   -password string
   -publickey fileName
   -timeout seconds}}

test sftp.16 {sftp create -host badHostName } {
    list [catch {blt::sftp create -host badHostName} msg] $msg
} {1 {can't connect to "badHostName": unknown hostname}}
}

test sftp.17 {sftp create mySftp -host $host } {
    list [catch {set sftp [blt::sftp create mySftp -host $host] } msg] $msg
} {0 ::mySftp}

test sftp.18 {create mySftp} {
    list [catch {blt::sftp create mySftp} msg] $msg
} {1 {a command "::mySftp" already exists}}

test sftp.19 {create if} {
    list [catch {blt::sftp create if} msg] $msg
} {1 {a command "::if" already exists}}

test sftp.20 {sftp create (bad namespace)} {
    list [catch {blt::sftp create badName::fred} msg] $msg
} {1 {unknown namespace "badName"}}

test sftp.21 {badOp} {
    list [catch {$sftp badOp}  msg] $msg
} {1 {bad operation "badOp": should be one of...
  ::mySftp atime path ?seconds?
  ::mySftp chdir ?path?
  ::mySftp chgrp path ?gid ?-recurse??
  ::mySftp chmod path ?mode ?-recurse??
  ::mySftp delete path ?switches?
  ::mySftp dir path ?switches?
  ::mySftp exists path
  ::mySftp get path ?file? ?switches?
  ::mySftp isdirectory path
  ::mySftp isfile path
  ::mySftp lstat path varName
  ::mySftp mkdir path ?-mode mode?
  ::mySftp mtime path ?seconds?
  ::mySftp normalize path
  ::mySftp put file ?path? ?switches?
  ::mySftp pwd 
  ::mySftp read path ?switches?
  ::mySftp readable path
  ::mySftp readlink path
  ::mySftp rename old new ?-force?
  ::mySftp rmdir path
  ::mySftp size path
  ::mySftp slink path link
  ::mySftp stat path varName
  ::mySftp type path
  ::mySftp writable path
  ::mySftp write path string ?switches?}}

test sftp.22 {c} {
    list [catch {$sftp c}  msg] $msg
} {1 {ambiguous operation "c" matches:  chdir chgrp chmod}}

test sftp.23 {writ} {
    list [catch {$sftp writ}  msg] $msg
} {1 {ambiguous operation "writ" matches:  writable write}}

test sftp.24 {rea} {
    list [catch {$sftp rea}  msg] $msg
} {1 {ambiguous operation "rea" matches:  read readable readlink}}

test sftp.25 {read} {
    list [catch {$sftp read}  msg] $msg
} {1 {wrong # args: should be "::mySftp read path ?switches?"}}

test sftp.26 {mkdir /badDir} {
    list [catch {$sftp mkdir /badDir} msg] $msg
} {1 {can't make directory "/badDir": permission denied}}

test sftp.27 {rmdir test_directory} {
    list [catch {
	if { [$sftp exists test_directory] } {
	    $sftp delete test_directory -force
	}
    } msg] $msg
} {0 {}}

test sftp.28 {mkdir test_directory} {
    list [catch {$sftp mkdir test_directory}  msg] $msg
} {0 {}}

test sftp.29 {mkdir test_directory} {
    list [catch {$sftp mkdir test_directory} msg] $msg
} {1 {can't make directory "test_directory": directory already exists}}

test sftp.30 {mkdir test_directory/test1} {
    list [catch {$sftp mkdir test_directory/test1} msg] $msg
} {0 {}}

test sftp.31 {mkdir test_directory/test2 -mode 0777} {
    list [catch {$sftp mkdir test_directory/test2 -mode 0777} msg] $msg
} {0 {}}

test sftp.32 {dir test_directory} {
    list [catch {$sftp dir test_directory} msg] [lsort $msg]
} {0 {. .. test1 test2}}

test sftp.33 {put} {
    list [catch {$sftp put} msg] $msg
} {1 {wrong # args: should be "::mySftp put file ?path? ?switches?"}}

test sftp.34 {put badFile } {
    list [catch {$sftp put badFile} msg] $msg
} {1 {can't open "badFile": no such file or directory}}

test sftp.35 {exists test_directory} {
    list [catch {$sftp exists test_directory} msg] $msg
} {0 1}

test sftp.36 {exists test_directory/badFile } {
    list [catch {$sftp exists test_directory/badFile } msg] $msg
} {0 0}

test sftp.37 {exists test_directory/file1} {
    list [catch {$sftp exists test_directory/file1} msg] $msg
} {0 0}

test sftp.38 {exists test_directory/$file} {
    list [catch {$sftp exists test_directory/$file} msg] $msg
} {0 0}

test sftp.39 {put file test_directory} {
    list [catch {$sftp put $file test_directory} msg] $msg
} {0 {}}

test sftp.40 {atime test_directory/$file 10} {
    list [catch {$sftp atime test_directory/$file 10} msg] $msg
} {0 10}

test sftp.41 {atime test_directory/$file} {
    list [catch {$sftp atime test_directory/$file} msg] $msg
} {0 10}

test sftp.42 {atime} {
    list [catch {$sftp atime} msg] $msg
} {1 {wrong # args: should be "::mySftp atime path ?seconds?"}}

test sftp.43 {atime too many arguments } {
    list [catch {$sftp atime too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp atime path ?seconds?"}}

test sftp.44 {atime badFile} {
    list [catch {$sftp atime badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.45 {mtime test_directory/$file 10} {
    list [catch {$sftp mtime test_directory/$file 10} msg] $msg
} {0 10}

test sftp.46 {mtime test_directory/$file} {
    list [catch {$sftp mtime test_directory/$file} msg] $msg
} {0 10}

test sftp.47 {mtime} {
    list [catch {$sftp mtime} msg] $msg
} {1 {wrong # args: should be "::mySftp mtime path ?seconds?"}}

test sftp.48 {mtime too many arguments } {
    list [catch {$sftp mtime too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp mtime path ?seconds?"}}

test sftp.49 {mtime badFile} {
    list [catch {$sftp mtime badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.50 {chmod test_directory/$file} {
    list [catch {$sftp chmod test_directory/$file} msg] $msg
} {0 00640}

test sftp.51 {chmod} {
    list [catch {$sftp chmod} msg] $msg
} {1 {wrong # args: should be "::mySftp chmod path ?mode ?-recurse??"}}


test sftp.52 {chmod -badSwitch } {
    list [catch {$sftp chmod test_directory/$file 0666 -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -recurse }}

test sftp.53 {chmod badFile} {
    list [catch {$sftp chmod badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}


test sftp.54 {chmod test_directory/$file 0666} {
    list [catch {$sftp chmod test_directory/$file 0666} msg] $msg
} {0 {}}

test sftp.55 {chmod test_directory/$file} {
    list [catch {
	set mode [$sftp chmod test_directory/$file]
    } msg] $msg
} {0 00666}

test sftp.56 {chmod test_directory/$file abc} {
    list [catch {$sftp chmod test_directory/$file abc} msg] $msg
} {1 {bad mode "abc"}}

test sftp.57 {size test_directory/$file} {
    list [catch {$sftp size test_directory/$file} msg] $msg
} [list 0 [file size $file]]

test sftp.58 {chmod test_directory/$file +rw} {
    list [catch {$sftp chmod test_directory/$file +rw} msg] $msg
} {0 {}}

test sftp.59 {chmod test_directory/$file a-rw} {
    list [catch {$sftp chmod test_directory/$file a-rw} msg] $msg
} {0 {}}

test sftp.60 {chmod test_directory/$file g+rw} {
    list [catch {$sftp chmod test_directory/$file g+rw} msg] $msg
} {0 {}}

test sftp.61 {chmod test_directory/$file u+rw} {
    list [catch {$sftp chmod test_directory/$file u+rw} msg] $msg
} {0 {}}

test sftp.62 {chmod test_directory/$file g+rw} {
    list [catch {$sftp chmod test_directory/$file g+rw} msg] $msg
} {0 {}}

test sftp.63 {chmod test_directory/$file u+rw} {
    list [catch {$sftp chmod test_directory/$file u+rw} msg] $msg
} {0 {}}

test sftp.64 {chmod test_directory/$file g+rw} {
    list [catch {$sftp chmod test_directory/$file g+rw} msg] $msg
} {0 {}}

test sftp.65 {chmod test_directory/$file u+rw} {
    list [catch {$sftp chmod test_directory/$file u+rw} msg] $msg
} {0 {}}

test sftp.66 {chmod test_directory/$file u+rw} {
    list [catch {$sftp chmod test_directory/$file u+rw} msg] $msg
} {0 {}}

test sftp.67 {chmod test_directory/$file o+rw} {
    list [catch {$sftp chmod test_directory/$file o+rw} msg] $msg
} {0 {}}

test sftp.68 {chmod test_directory/$file a+rw} {
    list [catch {$sftp chmod test_directory/$file a+rw} msg] $msg
} {0 {}}

test sftp.69 {chmod test_directory/$file o+r,g-rw,u+w } {
    list [catch {$sftp chmod test_directory/$file o+r,g-rw,u+w } msg] $msg
} {0 {}}

test sftp.70 {chmod test_directory/$file o-rwx -recurse } {
    list [catch {$sftp chmod test_directory/$file o-rwx -recurse } msg] $msg
} {0 {}}

test sftp.71 {chmod test_directory/$file $mode } {
    list [catch {$sftp chmod test_directory/$file $mode } msg] $msg
} {0 {}}


test sftp.72 {size badFile} {
    list [catch {$sftp size badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.73 {size} {
    list [catch {$sftp size} msg] $msg
} {1 {wrong # args: should be "::mySftp size path"}}

test sftp.74 {size too many arguments } {
    list [catch {$sftp size too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp size path"}}

test sftp.75 {size badFile} {
    list [catch {$sftp size badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.76 {chgrp test_directory/$file} {
    list [catch {$sftp chgrp test_directory/$file} msg] $msg
} {0 100}

test sftp.77 {chgrp badFile} {
    list [catch {$sftp chgrp badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.78 {chgrp} {
    list [catch {$sftp chgrp} msg] $msg
} {1 {wrong # args: should be "::mySftp chgrp path ?gid ?-recurse??"}}

test sftp.79 {chgrp -badswitch } {
    list [catch {$sftp chgrp test_directory/$file 100 -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -recurse }}

test sftp.80 {isdirectory test_directory} {
    list [catch {$sftp isdirectory test_directory} msg] $msg
} {0 1}

test sftp.81 {isdirectory test_directory/$file} {
    list [catch {$sftp isdirectory test_directory/$file} msg] $msg
} {0 0}

test sftp.82 {isdirectory ~} {
    list [catch {$sftp isdirectory ~} msg] $msg
} {0 1}

test sftp.83 {isdirectory ~/test_directory} {
    list [catch {$sftp isdirectory ~/test_directory} msg] $msg
} {0 1}

test sftp.84 {isdirectory ~/test_directory/$file} {
    list [catch {$sftp isdirectory ~/test_directory/$file} msg] $msg
} {0 0}

test sftp.85 {isdirectory badFile} {
    list [catch {$sftp isdirectory badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.86 {isdirectory ~/badFile} {
    list [catch {$sftp isdirectory ~/badFile} msg] $msg
} {1 {can't stat "~/badFile": no such file or directory}}

test sftp.87 {isdirectory} {
    list [catch {$sftp isdirectory} msg] $msg
} {1 {wrong # args: should be "::mySftp isdirectory path"}}

test sftp.88 {isdirectory too many arguments } {
    list [catch {$sftp isdirectory too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp isdirectory path"}}

test sftp.89 {isfile test_directory} {
    list [catch {$sftp isfile test_directory} msg] $msg
} {0 0}

test sftp.90 {isfile test_directory/$file} {
    list [catch {$sftp isfile test_directory/$file} msg] $msg
} {0 1}

test sftp.91 {isfile} {
    list [catch {$sftp isfile} msg] $msg
} {1 {wrong # args: should be "::mySftp isfile path"}}

test sftp.92 {isfile too many arguments } {
    list [catch {$sftp isfile too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp isfile path"}}

test sftp.93 {isfile badFile} {
    list [catch {$sftp isfile badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.94 {type test_directory} {
    list [catch {$sftp type test_directory} msg] $msg
} {0 directory}

test sftp.95 {type test_directory/$file} {
    list [catch {$sftp type test_directory/$file} msg] $msg
} {0 file}

test sftp.96 {type} {
    list [catch {$sftp type} msg] $msg
} {1 {wrong # args: should be "::mySftp type path"}}

test sftp.97 {type too many arguments } {
    list [catch {$sftp type too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp type path"}}

test sftp.98 {type badFile} {
    list [catch {$sftp type badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.99 {exists test_directory} {
    list [catch {$sftp exists test_directory} msg] $msg
} {0 1}

test sftp.100 {exists test_directory/$file} {
    list [catch {$sftp exists test_directory/$file} msg] $msg
} {0 1}

test sftp.101 {exists badFile} {
    list [catch {$sftp exists badFile} msg] $msg
} {0 0}

test sftp.102 {exists} {
    list [catch {$sftp exists} msg] $msg
} {1 {wrong # args: should be "::mySftp exists path"}}

test sftp.103 {exists too many arguments } {
    list [catch {$sftp exists too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp exists path"}}


test sftp.104 {writable test_directory} {
    list [catch {$sftp writable test_directory} msg] $msg
} {0 1}

test sftp.105 {writable test_directory/$file} {
    list [catch {$sftp writable test_directory/$file} msg] $msg
} {0 1}

test sftp.106 {writable badFile} {
    list [catch {$sftp writable badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.107 {writable} {
    list [catch {$sftp writable} msg] $msg
} {1 {wrong # args: should be "::mySftp writable path"}}

test sftp.108 {writable too many arguments } {
    list [catch {$sftp writable too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp writable path"}}


test sftp.109 {readable test_directory} {
    list [catch {$sftp readable test_directory} msg] $msg
} {0 1}

test sftp.110 {type readable_directory/$file} {
    list [catch {$sftp readable test_directory/$file} msg] $msg
} {0 1}

test sftp.111 {readable badFile} {
    list [catch {$sftp readable badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.112 {readable} {
    list [catch {$sftp readable} msg] $msg
} {1 {wrong # args: should be "::mySftp readable path"}}

test sftp.113 {readable too many arguments } {
    list [catch {$sftp readable too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp readable path"}}


test sftp.114 {normalize test_directory} {
    list [catch {$sftp normalize test_directory} msg] $msg
} {0 1}

test sftp.115 {normalize test_directory/$file} {
    list [catch {$sftp normalize test_directory/$file} msg] $msg
} {0 /home/gah/test_directory/sftp.tcl}

test sftp.116 {normalize badFile} {
    list [catch {$sftp normalize badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.117 {readable} {
    list [catch {$sftp normalize} msg] $msg
} {1 {wrong # args: should be "::mySftp normalize path"}}

test sftp.118 {normalize too many arguments } {
    list [catch {$sftp normalize too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp normalize path"}}

test sftp.119 {readlink test_directory} {
    list [catch {$sftp readlink test_directory} msg] $msg
} {1 {can't read link "test_directory": not a link}}

test sftp.120 {readlink test_directory/$file} {
    list [catch {$sftp readlink test_directory/$file} msg] $msg
} {1 {can't read link "test_directory/sftp.tcl": not a link}}

test sftp.121 {readlink badFile} {
    list [catch {$sftp readlink badFile} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.122 {readlink} {
    list [catch {$sftp readlink} msg] $msg
} {1 {wrong # args: should be "::mySftp readlink path"}}

test sftp.123 {readlink too many arguments } {
    list [catch {$sftp readlink too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp readlink path"}}

test sftp.124 {stat test_directory varName} {
    list [catch {
	array unset myArray
	$sftp stat test_directory myArray
	list $myArray(size) $myArray(mode) $myArray(type)
    } msg] $msg
} {0 {120 00750 directory}}

test sftp.125 {stat test_directory/$file myArray} {
    list [catch {
	array unset myArray
	$sftp stat test_directory/$file myArray
	list $myArray(size) $myArray(mode) $myArray(type)
    } msg] $msg
} [list 0 [list [file size $file] 00666 file]]

test sftp.126 {stat badFile} {
    list [catch {$sftp stat badFile myArray} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.127 {stat} {
    list [catch {$sftp stat} msg] $msg
} {1 {wrong # args: should be "::mySftp stat path varName"}}

test sftp.128 {stat too many arguments } {
    list [catch {$sftp stat too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp stat path varName"}}

test sftp.129 {lstat test_directory varName} {
    list [catch {
	array unset myArray
	$sftp lstat test_directory myArray
	list $myArray(size) $myArray(mode) $myArray(type)
    } msg] $msg
} {0 {120 00750 directory}}

test sftp.130 {lstat test_directory/$file myArray} {
    list [catch {
	array unset myArray
	$sftp lstat test_directory/$file myArray
	list $myArray(size) $myArray(mode) $myArray(type)
    } msg] $msg
} [list 0 [list [file size $file] 00666 file]]

test sftp.131 {lstat badFile} {
    list [catch {$sftp lstat badFile myArray} msg] $msg
} {1 {can't stat "badFile": no such file or directory}}

test sftp.132 {lstat} {
    list [catch {$sftp lstat} msg] $msg
} {1 {wrong # args: should be "::mySftp lstat path varName"}}

test sftp.133 {lstat too many arguments } {
    list [catch {$sftp lstat too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp lstat path varName"}}

test sftp.134 {pwd} {
    list [catch {$sftp pwd} msg] $msg
} {0 1}

test sftp.135 {pwd too many arguments} {
    list [catch {$sftp pwd too many arguments} msg] $msg
} {1 {wrong # args: should be "::mySftp pwd "}}

exit 1

test sftp.136 {exists test_directory/test1} {
    list [catch {$sftp exists test_directory/test1} msg] $msg
} {0 1}

test sftp.137 {exists test_directory/$file} {
    list [catch {$sftp exists test_directory/$file} msg] $msg
} {0 0}

test sftp.138 {put file test_directory/file1} {
    list [catch {$sftp put $file test_directory/file1} msg] $msg
} {0 {}}

test sftp.139 {get} {
    list [catch {$sftp get} msg] $msg
} {1 {wrong # args: should be "::mySftp get path ?file? ?switches?"}}

test sftp.140 {get badFile } {
    list [catch {$sftp get badFile} msg] $msg
} {1 {can't get attributes for "./badFile": no such file or directory}}

test sftp.141 {get badFile -badSwitch } {
    list [catch {$sftp get badFile -badSwitch } msg] $msg
} {1 {can't get attributes for "./badFile": no such file or directory}}

test sftp.142 {isfile test_directory} {
    list [catch {$sftp isfile test_directory} msg] $msg
} {0 0}

test sftp.143 {isdirectory test_directory} {
    list [catch {$sftp isdirectory test_directory} msg] $msg
} {0 1}

test sftp.144 {get file -badSwitch } {
    list [catch {$sftp get $file -badSwitch } msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -cancel varName
   -maxsize number
   -progress command
   -resume bool
   -timeout seconds}}

test sftp.145 {get $file } {
    list [catch {$sftp get test_directory/$file } msg] $msg
} {0 {}}


exit 0

test sftp.146 {create fred} {
    list [catch {blt::sftp create fred} msg] $msg
} {1 {a command "::fred" already exists}}

test sftp.147 {create if} {
    list [catch {blt::sftp create if} msg] $msg
} {1 {a command "::if" already exists}}

test sftp.148 {sftp create (bad namespace)} {
    list [catch {blt::sftp create badName::fred} msg] $msg
} {1 {unknown namespace "badName"}}

test sftp.149 {sftp create (wrong # args)} {
    list [catch {blt::sftp create a b} msg] $msg
} {1 {wrong # args: should be "blt::sftp create ?name?"}}

test sftp.150 {sftp names} {
    list [catch {blt::sftp names} msg] [lsort $msg]
} {0 {::fred ::sftp0 ::sftp1}}

test sftp.151 {sftp names pattern)} {
    list [catch {blt::sftp names ::sftp*} msg] [lsort $msg]
} {0 {::sftp0 ::sftp1}}

test sftp.152 {sftp names badPattern)} {
    list [catch {blt::sftp names badPattern*} msg] $msg
} {0 {}}

test sftp.153 {sftp names pattern arg (wrong # args)} {
    list [catch {blt::sftp names pattern arg} msg] $msg
} {1 {wrong # args: should be "blt::sftp names ?pattern?..."}}

test sftp.154 {sftp destroy (wrong # args)} {
    list [catch {blt::sftp destroy} msg] $msg
} {1 {wrong # args: should be "blt::sftp destroy name..."}}

test sftp.155 {sftp destroy badSftp} {
    list [catch {blt::sftp destroy badSftp} msg] $msg
} {1 {can't find a sftp named "badSftp"}}

test sftp.156 {sftp destroy fred} {
    list [catch {blt::sftp destroy fred} msg] $msg
} {0 {}}

test sftp.157 {sftp destroy sftp0 sftp1} {
    list [catch {blt::sftp destroy sftp0 sftp1} msg] $msg
} {0 {}}

test sftp.158 {create} {
    list [catch {blt::sftp create} msg] $msg
} {0 ::sftp0}

exit 0
















