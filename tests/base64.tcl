
package require BLT

if {[info procs test] != "test"} {
    source defs
}

if [file exists ../library] {
    set blt_library ../library
}

#set VERBOSE 1

# Encode 

test base64.1 {encode no args} {
    list [catch {blt::encode} msg] $msg
} {1 {wrong # args: should be "blt::encode formatName string ?switches ...?"}}

test base64.2 {encode badFormat} {
    list [catch {blt::encode badFormat ""} msg] $msg
} {1 {bad format "badFormat": should be hexadecimal, base64, or ascii85}}

# Encode ascii85

test base64.3 {encode ascii85 no args} {
    list [catch {blt::encode ascii85} msg] $msg
} {1 {wrong # args: should be "blt::encode formatName string ?switches ...?"}}

test base64.4 {encode ascii85 -badSwitch} {
    list [catch {blt::encode ascii85 "" -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -brackets 
   -data varName
   -file fileName
   -foldspaces 
   -foldzeros 
   -pad string
   -wrapchars string
   -wraplength number}}

test base64.5 {encode ascii85 missing value} {
    list [catch {blt::encode ascii85 "" -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.6 {encode ascii85 "abc"} {
    list [catch {blt::encode ascii85 "abc"} msg] $msg
} {0 @:E^}

test base64.7 {encode ascii85 wraps by default at 76 characters} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str} msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}

test base64.8 {encode ascii85 bigger text} {
    set str "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure."
    list [catch {blt::encode ascii85 $str} msg] $msg
} {0 {9jqo^BlbD-BleB1DJ+*+F(f,q/0JhKF<GL>Cj@.4Gp$d7F!,L7@<6@)/0JDE
F<G%<+EV:2F!,O<DJ+*.@<*K0@<6L(Df-\0Ec5e;DffZ(EZee.Bl.9pF"AGX
BPCsi+DGm>@3BB/F*&OCAfu2/AKYi(DIb:@FD,*)+C]U=@3BN#EcYf8ATD3s
@q?d$AftVqCh[NqF<G:8+EV:.+Cf>-FD5W8ARlolDIal(DId<j@<?3r@:F%a
+D58'ATD4$Bl@l3De:,-DJs`8ARoFb/0JMK@qB4^F!,R<AKZ&-DfTqBG%G>u
D.RTpAKYo'+CT/5+Cei#DII?(E,9)oF*2M7/c}}

test base64.9 {encode ascii85 -wraplength 40} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -wraplength 40} msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3B
Ec6)5BHVD1AKYW+AS#a%AnbgmA0>;uA0>W0D/a&s
+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}

test base64.7 {encode ascii85 -wraplength 60} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -wraplength 60} msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}

test base64.10 {encode ascii85 -wraplength 70} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -wraplength 70} msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%AnbgmA0>;u
A0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}

test base64.11 {encode ascii85 -wraplength 0} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -wraplength 0} msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}

test base64.12 {encode ascii85 -wraplength badValue} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -wraplength badValue} msg] $msg
} {1 {expected integer but got "badValue"}}

test base64.13 {encode ascii85 -wraplength -3} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -wraplength -3} msg] $msg
} {1 {bad value "-3": can't be negative}}

test base64.14 {encode ascii85 -wraplength 1.5} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -wraplength 1.5} msg] $msg
} {1 {expected integer but got "1.5"}}

test base64.15 {encode ascii85 -wrapchars " wrapping\n"} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -wrapchars " wrapping\n"} msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a% wrapping
AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}


test base64.16 {encode ascii85 -wrapchars "\r\n"} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	string match "*\r\n*" [blt::encode ascii85 $str -wrapchars "\r\n"]
    } msg] $msg
} {0 1}

test base64.17 {encode ascii85 -pad missing value} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -pad} msg] $msg
} {1 {value for "-pad" missing}}

test base64.18 {encode ascii85 -pad "  " } {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -pad "   "} msg] $msg
} {0 {   <+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
   AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}


test base64.19 {encode ascii85 -pad "" } {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -pad ""} msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}


test base64.20 {encode ascii85 -brackets} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -brackets} msg] $msg
} {0 {<-<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#
a%AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD->}}

test base64.21 {encode ascii85 -foldspaces} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "                                                      "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -foldspaces} msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
AnbgmA0<!;yyyyyyyyyyyyy@;]TuC3=B4ARlp%G%G\:FD,5.FCB!%+C]A0GA
\O4ARTTd}}

test base64.22 {encode ascii85 -foldspaces} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "                                                      "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -foldspaces} msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
AnbgmA0<!;yyyyyyyyyyyyy@;]TuC3=B4ARlp%G%G\:FD,5.FCB!%+C]A0GA
\O4ARTTd}}

test base64.23 {encode ascii85 -foldspaces extra arg} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "                                                      "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -foldspaces extraArg} msg] $msg
} {1 {unknown switch "extraArg"
following switches are available:
   -brackets 
   -data varName
   -file fileName
   -foldspaces 
   -foldzeros 
   -pad string
   -wrapchars string
   -wraplength number}}

test base64.24 {encode ascii85 -foldzeros} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -foldzeros} msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
AnbgmA0:jPzzzzzz!!$GFA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\n
jEXD}}

test base64.25 {encode ascii85 -data missing value} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.26 {encode ascii85 -data myVar} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	blt::encode ascii85 $str -data myVar
	set myVar
    } msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}

test base64.27 {encode ascii85 -data ""} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	blt::encode ascii85 $str -data ""
	set ""
    } msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}

test base64.28 {encode ascii85 -file missing value} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -file} msg] $msg
} {1 {value for "-file" missing}}

test base64.29 {encode ascii85 -file badDir/badFile} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode ascii85 $str -file /badDir/badFile} msg] $msg
} {1 {couldn't open "/badDir/badFile": no such file or directory}}

test base64.30 {encode ascii85 -file no permissions} {
    file delete /tmp/myFile
    set f [open "/tmp/myFile" "w"]
    close $f
    file attributes /tmp/myFile -permission -w
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    set msg [list [catch {
	blt::encode ascii85 $str -file /tmp/myFile
    } msg] $msg]
    file delete /tmp/myFile
    set msg
} {1 {couldn't open "/tmp/myFile": permission denied}}

test base64.31 {encode ascii85 -file /tmp/myFile} {
    file delete /tmp/myFile
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	blt::encode ascii85 $str -file /tmp/myFile
	set f [open "/tmp/myFile" "r"]
	set contents [read $f]
	close $f
	file delete /tmp/myFile
	set contents
    } msg] $msg
} {0 {<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD}}

# Encode hexadecimal

test base64.32 {encode hexadecimal no args} {
    list [catch {blt::encode hexadecimal} msg] $msg
} {1 {wrong # args: should be "blt::encode formatName string ?switches ...?"}}

test base64.33 {encode hexadecimal -badSwitch} {
    list [catch {blt::encode hexadecimal "" -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -data string
   -file fileName
   -lowercase 
   -pad fileName
   -wrapchars string
   -wraplength number}}

test base64.34 {encode hexadecimal missing value} {
    list [catch {blt::encode hexadecimal "" -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.35 {encode hexadecimal "abc"} {
    list [catch {blt::encode hexadecimal "abc"} msg] $msg
} {0 616263}

test base64.36 {encode hexadecimal wraps by default at 60 characters} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str} msg] $msg
} {0 {5468652073686F72742072656420666F782072616E20717569636B6C7920
7468726F7567682074686520677265656E206669656C6420616E64206A75
6D706564206F766572207468652074616C6C2062726F776E20626561720A
}}

test base64.37 {encode hexadecimal -lowercase} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -lower} msg] $msg
} {0 {5468652073686f72742072656420666f782072616e20717569636b6c7920
7468726f7567682074686520677265656e206669656c6420616e64206a75
6d706564206f766572207468652074616c6c2062726f776e20626561720a
}}

test base64.38 {encode hexadecimal bigger text} {
    set str "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure."
    list [catch {blt::encode hexadecimal $str} msg] $msg
} {0 {4D616E2069732064697374696E677569736865642C206E6F74206F6E6C79
2062792068697320726561736F6E2C206275742062792074686973207369
6E67756C61722070617373696F6E2066726F6D206F7468657220616E696D
616C732C2077686963682069732061206C757374206F6620746865206D69
6E642C20746861742062792061207065727365766572616E6365206F6620
64656C6967687420696E2074686520636F6E74696E75656420616E642069
6E6465666174696761626C652067656E65726174696F6E206F66206B6E6F
776C656467652C2065786365656473207468652073686F72742076656865
6D656E6365206F6620616E79206361726E616C20706C6561737572652E}}

test base64.39 {encode hexadecimal -wraplength 40} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -wraplength 40} msg] $msg
} {0 {5468652073686F72742072656420666F78207261
6E20717569636B6C79207468726F756768207468
6520677265656E206669656C6420616E64206A75
6D706564206F766572207468652074616C6C2062
726F776E20626561720A}}

test base64.40 {encode hexadecimal -wraplength 70} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -wraplength 70} msg] $msg
} {0 {5468652073686F72742072656420666F782072616E20717569636B6C79207468726F75
67682074686520677265656E206669656C6420616E64206A756D706564206F76657220
7468652074616C6C2062726F776E20626561720A}}

test base64.41 {encode hexadecimal -wraplength 0} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -wraplength 0} msg] $msg
} {0 5468652073686F72742072656420666F782072616E20717569636B6C79207468726F7567682074686520677265656E206669656C6420616E64206A756D706564206F766572207468652074616C6C2062726F776E20626561720A}

test base64.42 {encode hexadecimal -wraplength badValue} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -wraplength badValue} msg] $msg
} {1 {expected integer but got "badValue"}}

test base64.43 {encode hexadecimal -wraplength -3} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -wraplength -3} msg] $msg
} {1 {bad value "-3": can't be negative}}

test base64.44 {encode hexadecimal -wraplength 1.5} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -wraplength 1.5} msg] $msg
} {1 {expected integer but got "1.5"}}

test base64.45 {encode hexadecimal -wrapchars " wrapping\n"} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -wrapchars " wrapping\n"} msg] $msg
} {0 {5468652073686F72742072656420666F782072616E20717569636B6C7920 wrapping
7468726F7567682074686520677265656E206669656C6420616E64206A75 wrapping
6D706564206F766572207468652074616C6C2062726F776E20626561720A wrapping
}}

test base64.46 {encode hexadecimal -wrapchars "\r\n"} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	string match "*\r\n*" [blt::encode hexadecimal $str -wrapchars "\r\n"]
    } msg] $msg
} {0 1}

test base64.47 {encode hexadecimal -pad missing value} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -pad} msg] $msg
} {1 {value for "-pad" missing}}

test base64.48 {encode hexadecimal -pad "  " } {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -pad "   "} msg] $msg
} {0 {   5468652073686F72742072656420666F782072616E20717569636B6C7920
   7468726F7567682074686520677265656E206669656C6420616E64206A75
   6D706564206F766572207468652074616C6C2062726F776E20626561720A
}}

test base64.49 {encode hexadecimal -pad "" } {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -pad ""} msg] $msg
} {0 {5468652073686F72742072656420666F782072616E20717569636B6C7920
7468726F7567682074686520677265656E206669656C6420616E64206A75
6D706564206F766572207468652074616C6C2062726F776E20626561720A
}}

test base64.50 {encode hexadecimal -data missing value} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.51 {encode hexadecimal -data myVar} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	blt::encode hexadecimal $str -data myVar
	set myVar
    } msg] $msg
} {0 {5468652073686F72742072656420666F782072616E20717569636B6C7920
7468726F7567682074686520677265656E206669656C6420616E64206A75
6D706564206F766572207468652074616C6C2062726F776E20626561720A
}}

test base64.52 {encode hexadecimal -data ""} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	blt::encode hexadecimal $str -data ""
	set ""
    } msg] $msg
} {0 {5468652073686F72742072656420666F782072616E20717569636B6C7920
7468726F7567682074686520677265656E206669656C6420616E64206A75
6D706564206F766572207468652074616C6C2062726F776E20626561720A
}}

test base64.53 {encode hexadecimal -file missing value} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -file} msg] $msg
} {1 {value for "-file" missing}}

test base64.54 {encode hexadecimal -file badDir/badFile} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode hexadecimal $str -file /badDir/badFile} msg] $msg
} {1 {couldn't open "/badDir/badFile": no such file or directory}}

test base64.55 {encode hexadecimal -file no permissions} {
    file delete /tmp/myFile
    set f [open "/tmp/myFile" "w"]
    close $f
    file attributes /tmp/myFile -permission -w
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    set msg [list [catch {
	blt::encode hexadecimal $str -file /tmp/myFile
    } msg] $msg]
    file delete /tmp/myFile
    set msg
} {1 {couldn't open "/tmp/myFile": permission denied}}

test base64.56 {encode hexadecimal -file /tmp/myFile} {
    file delete /tmp/myFile
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	blt::encode hexadecimal $str -file /tmp/myFile
	set f [open "/tmp/myFile" "r"]
	set contents [read $f]
	close $f
	file delete /tmp/myFile
	set contents
    } msg] $msg
} {0 {5468652073686F72742072656420666F782072616E20717569636B6C7920
7468726F7567682074686520677265656E206669656C6420616E64206A75
6D706564206F766572207468652074616C6C2062726F776E20626561720A
}}

# Encode base64

test base64.57 {encode base64 no args} {
    list [catch {blt::encode base64} msg] $msg
} {1 {wrong # args: should be "blt::encode formatName string ?switches ...?"}}


test base64.58 {encode base64 -badSwitch} {
    list [catch {blt::encode base64 "" -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -data string
   -file fileName
   -pad fileName
   -wrapchars string
   -wraplength number}}

test base64.59 {encode base64 "abc"} {
    list [catch {blt::encode base64 "abc"} msg] $msg
} {0 YWJj}

test base64.60 {encode base64 wraps by default at 76 characters} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str} msg] $msg
} {0 {VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3JlZW4gZmllbGQgYW5k
IGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK}}


test base64.61 {encode base64 bigger text} {
    set str "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure."
    list [catch {blt::encode base64 $str} msg] $msg
} {0 {TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz
IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg
dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu
dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo
ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=}}


test base64.62 {encode base64 -wraplength 40} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -wraplength 40} msg] $msg
} {0 {VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkg
dGhyb3VnaCB0aGUgZ3JlZW4gZmllbGQgYW5kIGp1
bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK
}}

test base64.63 {encode base64 -wraplength 70} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -wraplength 70} msg] $msg
} {0 {VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3JlZW4gZmllbG
QgYW5kIGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK}}

test base64.64 {encode base64 -wraplength 0} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -wraplength 0} msg] $msg
} {0 VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3JlZW4gZmllbGQgYW5kIGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK}

test base64.65 {encode base64 -wraplength badValue} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -wraplength badValue} msg] $msg
} {1 {expected integer but got "badValue"}}

test base64.66 {encode base64 -wraplength -3} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -wraplength -3} msg] $msg
} {1 {bad value "-3": can't be negative}}

test base64.67 {encode base64 -wraplength 1.5} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -wraplength 1.5} msg] $msg
} {1 {expected integer but got "1.5"}}

test base64.68 {encode base64 -wrapchars " wrapping\n"} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -wrapchars " wrapping\n"} msg] $msg
} {0 {VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3JlZW4gZmllbGQgYW5k wrapping
IGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK}}

test base64.69 {encode base64 -wrapchars "\r\n"} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	string match "*\r\n*" [blt::encode base64 $str -wrapchars "\r\n"]
    } msg] $msg
} {0 1}

test base64.70 {encode base64 -pad missing value} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -pad} msg] $msg
} {1 {value for "-pad" missing}}

test base64.71 {encode base64 -pad "  " } {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -pad "   "} msg] $msg
} {0 {   VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3JlZW4gZmllbGQgYW5k
   IGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK}}

test base64.72 {encode base64 -pad "" } {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -pad ""} msg] $msg
} {0 {VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3JlZW4gZmllbGQgYW5k
IGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK}}

test base64.73 {encode base64 -data missing value} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.74 {encode base64 -data myVar} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	blt::encode base64 $str -data myVar
	set myVar
    } msg] $msg
} {0 {VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3JlZW4gZmllbGQgYW5k
IGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK}}


test base64.75 {encode base64 -data ""} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	blt::encode base64 $str -data ""
	set ""
    } msg] $msg
} {0 {VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3JlZW4gZmllbGQgYW5k
IGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK}}

test base64.76 {encode base64 -file missing value} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -file} msg] $msg
} {1 {value for "-file" missing}}

test base64.77 {encode base64 -file badDir/badFile} {
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {blt::encode base64 $str -file /badDir/badFile} msg] $msg
} {1 {couldn't open "/badDir/badFile": no such file or directory}}

test base64.78 {encode base64 -file no permissions} {
    file delete /tmp/myFile
    set f [open "/tmp/myFile" "w"]
    close $f
    file attributes /tmp/myFile -permission -w
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    set msg [list [catch {
	blt::encode base64 $str -file /tmp/myFile
    } msg] $msg]
    file delete /tmp/myFile
    set msg
} {1 {couldn't open "/tmp/myFile": permission denied}}

test base64.79 {encode base64 -file /tmp/myFile} {
    file delete /tmp/myFile
    set str ""
    append str "The short red fox ran quickly through the green field "
    append str "and jumped over the tall brown bear\n"
    list [catch {
	blt::encode base64 $str -file /tmp/myFile
	set f [open "/tmp/myFile" "r"]
	set contents [read $f]
	close $f
	file delete /tmp/myFile
	set contents
    } msg] $msg
} {0 {VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3JlZW4gZmllbGQgYW5k
IGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK}}

# Decode

test base64.80 {decode no args} {
    list [catch {blt::decode} msg] $msg
} {1 {wrong # args: should be "blt::decode formatName string ?switches ...?"}}

test base64.81 {decode badFormat} {
    list [catch {blt::decode badFormat ""} msg] $msg
} {1 {bad format "badFormat": should be hexadecimal, base64, or ascii85}}

# Decode ascii85

test base64.82 {decode ascii85 no args} {
    list [catch {blt::decode ascii85} msg] $msg
} {1 {wrong # args: should be "blt::decode formatName string ?switches ...?"}}

test base64.83 {decode ascii85 -badSwitch} {
    list [catch {blt::decode ascii85 "" -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -data string
   -file fileName
   -ignorebadchars }}

test base64.84 {decode ascii85 missing value} {
    list [catch {blt::decode ascii85 "" -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.85 {decode ascii85 "abc"} {
    list [catch {blt::decode ascii85 {@:E^}} msg] $msg
} {0 abc}

test base64.86 {decode ascii85} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
	AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD
    }
    list [catch {blt::decode ascii85 $str} msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.87 {decode ascii85 bigger text} {
    set str {
	9jqo^BlbD-BleB1DJ+*+F(f,q/0JhKF<GL>Cj@.4Gp$d7F!,L7@<6@)/0JDE
	F<G%<+EV:2F!,O<DJ+*.@<*K0@<6L(Df-\0Ec5e;DffZ(EZee.Bl.9pF"AGX
	BPCsi+DGm>@3BB/F*&OCAfu2/AKYi(DIb:@FD,*)+C]U=@3BN#EcYf8ATD3s
	@q?d$AftVqCh[NqF<G:8+EV:.+Cf>-FD5W8ARlolDIal(DId<j@<?3r@:F%a
	+D58'ATD4$Bl@l3De:,-DJs`8ARoFb/0JMK@qB4^F!,R<AKZ&-DfTqBG%G>u
	D.RTpAKYo'+CT/5+Cei#DII?(E,9)oF*2M7/c
    }
    list [catch {blt::decode ascii85 $str} msg] $msg
} {0 {Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.}}


test base64.88 {decode ascii85 "abc" with whitespace} {
    set str {

                           @:E^
    }
    list [catch {blt::decode ascii85 $str} msg] $msg
} {0 abc}

test base64.89 {decode ascii85 -foldspaces} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
        AnbgmA0<!;yyyyyyyyyyyyy@;]TuC3=B4ARlp%G%G\:FD,5.FCB!%+C]A0GA
       \O4ARTTd
    }
    list [catch {blt::decode ascii85 $str} msg] $msg
} {0 {The short red fox ran quickly through the green field                                                       and jumped over the tall brown bear
}}

test base64.90 {decode ascii85  -ignorebadchars} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx      
	AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD
    }
    list [catch {blt::decode ascii85 $str -ignorebadchars} msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.91 {decode ascii85} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx      
	AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD
    }
    list [catch {blt::decode ascii85 $str} msg] $msg
} {1 {invalid character found at 72}}

test base64.92 {decode ascii85 -folezeros} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
        AnbgmA0:jPzzzzzz!!$GFA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\n
        jEXD
    }
    list [catch {
	string map {\0 "_"} [blt::decode ascii85 $str] 
    } msg] $msg
} {0 {The short red fox ran quickly through the green field ____________________________and jumped over the tall brown bear
}}

test base64.93 {decode ascii85 -data missing value} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
	AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD
    }
    list [catch {blt::decode ascii85 $str -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.94 {decode ascii85 -data myVar} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
	AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD
    }
    list [catch {
	blt::decode ascii85 $str -data myVar
	set myVar
    } msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.95 {decode ascii85 -data ""} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
	AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD
    }
    list [catch {
	blt::decode ascii85 $str -data ""
	set ""
    } msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.96 {decode ascii85 -file missing value} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
	AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD
    }
    list [catch {blt::decode ascii85 $str -file} msg] $msg
} {1 {value for "-file" missing}}

test base64.97 {decode ascii85 -file badDir/badFile} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
	AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD
    }
    list [catch {blt::decode ascii85 $str -file /badDir/badFile} msg] $msg
} {1 {couldn't open "/badDir/badFile": no such file or directory}}

test base64.98 {decode ascii85 -file no permissions} {
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
	AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD
    }
    file delete /tmp/myFile
    set f [open "/tmp/myFile" "w"]
    close $f
    file attributes /tmp/myFile -permission -w
    set msg [list [catch {
	blt::decode ascii85 $str -file /tmp/myFile
    } msg] $msg]
    file delete /tmp/myFile
    set msg
} {1 {couldn't open "/tmp/myFile": permission denied}}

test base64.99 {decode ascii85 -file /tmp/myFile} {
    file delete /tmp/myFile
    set str {
	<+ohcF(fK4F<GU8A0>K&GT_$8DBNqABk(ppGp%3BEc6)5BHVD1AKYW+AS#a%
	AnbgmA0>;uA0>W0D/a&s+E)F7EZfI;AKZ)'Cht5'Ec6/>+C\njEXD
    }
    list [catch {
	blt::decode ascii85 $str -file /tmp/myFile
	set f [open "/tmp/myFile" "r"]
	set contents [read $f]
	close $f
	file delete /tmp/myFile
	set contents
    } msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.100 {decode ascii85 to max value} {
    list [catch {
	blt::encode hex [blt::decode ascii85 {s8W-!}] -lower
    } msg] $msg
} {0 ffffffff}

test base64.101 {decode ascii85 ""} {
    list [catch {blt::decode ascii85 ""} msg] $msg
} {0 {}}

test base64.102 {decode ascii85 ""} {
    list [catch {blt::decode ascii85 "ab~cd"} msg] $msg
} {1 {invalid character found at 3}}

test base64.103 {decode ascii85 "abcze"} {
    list [catch {blt::decode ascii85 "abcze"} msg] $msg
} {1 {invalid character found at 4}}

test base64.104 {decode ascii85 "abcde5"} {
    list [catch {
	blt::encode hex [blt::decode ascii85 "abcde5"] -lower
    } msg] $msg
} {0 c989a3a2}

test base64.105 {decode ascii85 "lots of zeros"} {
    list [catch {
	foreach enc [list !! !!! !!!! z z!!] {
	    lappend res [blt::decode ascii85 $enc]
	}
	string map {\00 _} $res
    } msg] $msg
} {0 {_ __ ___ ____ _____}}

test base64.106 {decode ascii85 "zbcde"} {
    list [catch {
	blt::encode hex [blt::decode ascii85 "zbcde"] -lower
    } msg] $msg
} {0 00000000ccafa3}

test base64.107 {decode ascii85 "azcde"} {
    list [catch {blt::decode ascii85 "azcde"} msg] $msg
} {1 {invalid character found at 2}}

test base64.108 {decode ascii85 "abzde"} {
    list [catch {blt::decode ascii85 "abzde"} msg] $msg
} {1 {invalid character found at 3}}

test base64.109 {decode ascii85 "abcze"} {
    list [catch {blt::decode ascii85 "abcze"} msg] $msg
} {1 {invalid character found at 4}}

test base64.110 {decode ascii85 "abcdz"} {
    list [catch {blt::decode ascii85 "abcdz"} msg] $msg
} {1 {invalid character found at 5}}

test base64.111 {decode ascii85 "abcdez"} {
    list [catch {
	blt::encode hex [blt::decode ascii85 "abcdez"] -lower
    } msg] $msg
} {0 c989a3a200000000}

test base64.112 {decode ascii85 "ybcde"} {
    list [catch {
	blt::encode hex [blt::decode ascii85 "ybcde"] -lower
    } msg] $msg
} {0 20202020ccafa3}

test base64.113 {decode ascii85 "aycde"} {
    list [catch {blt::decode ascii85 "aycde"} msg] $msg
} {1 {invalid character found at 2}}

test base64.114 {decode ascii85 "abyde"} {
    list [catch {blt::decode ascii85 "abyde"} msg] $msg
} {1 {invalid character found at 3}}

test base64.115 {decode ascii85 "abcye"} {
    list [catch {blt::decode ascii85 "abcye"} msg] $msg
} {1 {invalid character found at 4}}

test base64.116 {decode ascii85 "abcdy"} {
    list [catch {blt::decode ascii85 "abcdy"} msg] $msg
} {1 {invalid character found at 5}}

test base64.117 {decode ascii85 "abcdey"} {
    list [catch {
	blt::encode hex [blt::decode ascii85 "abcdey"] -lower
    } msg] $msg
} {0 c989a3a220202020}


# Decode hexadecimal

test base64.118 {decode hexadecimal no args} {
    list [catch {blt::decode hexadecimal} msg] $msg
} {1 {wrong # args: should be "blt::decode formatName string ?switches ...?"}}

test base64.119 {decode hexadecimal -badSwitch} {
    list [catch {blt::decode hexadecimal "" -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -data string
   -file fileName
   -ignorebadchars }}

test base64.120 {decode hexadecimal missing value} {
    list [catch {blt::decode hexadecimal "" -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.121 {decode hexadecimal "abc"} {
    list [catch {blt::decode hexadecimal 616263} msg] $msg
} {0 abc}

test base64.122 {decode hexadecimal} {
    set str {
	5468652073686F72742072656420666F782072616E20717569636B6C7920
	7468726F7567682074686520677265656E206669656C6420616E64206A75
	6D706564206F766572207468652074616C6C2062726F776E20626561720A
    }
    list [catch {blt::decode hexadecimal $str} msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.123 {decode hexadecimal bigger text} {
    set str {
	4D616E2069732064697374696E677569736865642C206E6F74206F6E6C79
	2062792068697320726561736F6E2C206275742062792074686973207369
	6E67756C61722070617373696F6E2066726F6D206F7468657220616E696D
	616C732C2077686963682069732061206C757374206F6620746865206D69
	6E642C20746861742062792061207065727365766572616E6365206F6620
	64656C6967687420696E2074686520636F6E74696E75656420616E642069
	6E6465666174696761626C652067656E65726174696F6E206F66206B6E6F
	776C656467652C2065786365656473207468652073686F72742076656865
	6D656E6365206F6620616E79206361726E616C20706C6561737572652E
    }
    list [catch {blt::decode hexadecimal $str} msg] $msg
} {0 {Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.}}


test base64.124 {decode hexadecimal "abc" with whitespace} {
    set str {

                           616263		
    }
    list [catch {blt::decode hexadecimal $str} msg] $msg
} {0 abc}

test base64.125 {decode hexadecimal -data missing value} {
    set str {
	5468652073686F72742072656420666F782072616E20717569636B6C7920
	7468726F7567682074686520677265656E206669656C6420616E64206A75
	6D706564206F766572207468652074616C6C2062726F776E20626561720A
    }
    list [catch {blt::decode hexadecimal $str -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.126 {decode hexadecimal -data myVar} {
    set str {
	5468652073686F72742072656420666F782072616E20717569636B6C7920
	7468726F7567682074686520677265656E206669656C6420616E64206A75
	6D706564206F766572207468652074616C6C2062726F776E20626561720A
    }
    list [catch {
	blt::decode hexadecimal $str -data myVar
	set myVar
    } msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.127 {decode hexadecimal -data ""} {
    set str {
	5468652073686F72742072656420666F782072616E20717569636B6C7920
	7468726F7567682074686520677265656E206669656C6420616E64206A75
	6D706564206F766572207468652074616C6C2062726F776E20626561720A
    }
    list [catch {
	blt::decode hexadecimal $str -data ""
	set ""
    } msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.128 {decode hexadecimal -file missing value} {
    set str {
	5468652073686F72742072656420666F782072616E20717569636B6C7920
	7468726F7567682074686520677265656E206669656C6420616E64206A75
	6D706564206F766572207468652074616C6C2062726F776E20626561720A
    }
    list [catch {blt::decode hexadecimal $str -file} msg] $msg
} {1 {value for "-file" missing}}

test base64.129 {decode hexadecimal -file badDir/badFile} {
    set str {
	5468652073686F72742072656420666F782072616E20717569636B6C7920
	7468726F7567682074686520677265656E206669656C6420616E64206A75
	6D706564206F766572207468652074616C6C2062726F776E20626561720A
    }
    list [catch {blt::decode hexadecimal $str -file /badDir/badFile} msg] $msg
} {1 {couldn't open "/badDir/badFile": no such file or directory}}

test base64.130 {decode hexadecimal -file no permissions} {
    set str {
	5468652073686F72742072656420666F782072616E20717569636B6C7920
	7468726F7567682074686520677265656E206669656C6420616E64206A75
	6D706564206F766572207468652074616C6C2062726F776E20626561720A
    }
    file delete /tmp/myFile
    set f [open "/tmp/myFile" "w"]
    close $f
    file attributes /tmp/myFile -permission -w
    set msg [list [catch {
	blt::decode hexadecimal $str -file /tmp/myFile
    } msg] $msg]
    file delete /tmp/myFile
    set msg
} {1 {couldn't open "/tmp/myFile": permission denied}}

test base64.131 {decode hexadecimal -file /tmp/myFile} {
    file delete /tmp/myFile
    set str {
	5468652073686F72742072656420666F782072616E20717569636B6C7920
	7468726F7567682074686520677265656E206669656C6420616E64206A75
	6D706564206F766572207468652074616C6C2062726F776E20626561720A
    }
    list [catch {
	blt::decode hexadecimal $str -file /tmp/myFile
	set f [open "/tmp/myFile" "r"]
	set contents [read $f]
	close $f
	file delete /tmp/myFile
	set contents
    } msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.132 {decode hexadecimal to max value} {
    list [catch {
	blt::encode hex [blt::decode hexadecimal {FFFFFFFF}] -lower
    } msg] $msg
} {0 ffffffff}

test base64.133 {decode hexadecimal ""} {
    list [catch {blt::decode hexadecimal ""} msg] $msg
} {0 {}}

test base64.134 {decode hexadecimal "~bcde"} {
    list [catch {blt::decode hexadecimal "~bcde"} msg] $msg
} {1 {invalid character found at 1}}

test base64.135 {decode hexadecimal "a~bcde"} {
    list [catch {blt::decode hexadecimal "a~bcde"} msg] $msg
} {1 {invalid character found at 2}}

test base64.136 {decode hexadecimal "ab~de"} {
    list [catch {blt::decode hexadecimal "ab~de"} msg] $msg
} {1 {invalid character found at 3}}

test base64.137 {decode hexadecimal "abc~e"} {
    list [catch {blt::decode hexadecimal "abc~e"} msg] $msg
} {1 {invalid character found at 4}}

test base64.138 {decode hexadecimal "abcd~"} {
    list [catch {blt::decode hexadecimal "abcd~"} msg] $msg
} {1 {invalid character found at 5}}

test base64.139 {decode hexadecimal "~616263" -ignorebadchars} {
    list [catch {blt::decode hexadecimal "~616263" -ignorebadchars} msg] $msg
} {0 abc}

test base64.140 {decode hexadecimal "6~16263" -ignorebadchars} {
    list [catch {blt::decode hexadecimal "6~16263" -ignorebadchars} msg] $msg
} {0 abc}

test base64.141 {decode hexadecimal "61~6263" -ignorebadchars} {
    list [catch {blt::decode hexadecimal "61~6263" -ignorebadchars} msg] $msg
} {0 abc}

test base64.142 {decode hexadecimal "616~263" -ignorebadchars} {
    list [catch {blt::decode hexadecimal "616~263" -ignorebadchars} msg] $msg
} {0 abc}

test base64.143 {decode hexadecimal "6162~63" -ignorebadchars} {
    list [catch {blt::decode hexadecimal "6162~63" -ignorebadchars} msg] $msg
} {0 abc}

test base64.144 {decode hexadecimal "61626~3" -ignorebadchars} {
    list [catch {blt::decode hexadecimal "61626~3" -ignorebadchars} msg] $msg
} {0 abc}

test base64.145 {decode hexadecimal "616263~" -ignorebadchars} {
    list [catch {blt::decode hexadecimal "616263~" -ignorebadchars} msg] $msg
} {0 abc}

test base64.146 {decode hexadecimal "a"} {
    list [catch {blt::decode hexadecimal "a"} msg] $msg
} {1 {odd number of hexadecimal digits.}}

test base64.147 {decode hexadecimal "abc"} {
    list [catch {blt::decode hexadecimal "abc"} msg] $msg
} {1 {odd number of hexadecimal digits.}}

# Decode base64

test base64.148 {decode base64 no args} {
    list [catch {blt::decode base64} msg] $msg
} {1 {wrong # args: should be "blt::decode formatName string ?switches ...?"}}

test base64.149 {decode base64 -badSwitch} {
    list [catch {blt::decode base64 "" -badSwitch} msg] $msg
} {1 {unknown switch "-badSwitch"
following switches are available:
   -data string
   -file fileName
   -ignorebadchars }}

test base64.150 {decode base64 missing value} {
    list [catch {blt::decode base64 "" -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.151 {decode base64 "abc"} {
    list [catch {blt::decode base64 YWJj} msg] $msg
} {0 abc}

test base64.152 {decode base64} {
    set str {
	VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3Jl
	ZW4gZmllbGQgYW5kIGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK
    }
    list [catch {blt::decode base64 $str} msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.153 {decode base64 bigger text} {
    set str {
	TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24s
	IGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmlt
	YWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBw
	ZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBp
	bmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRz
	IHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=
    }
    list [catch {blt::decode base64 $str} msg] $msg
} {0 {Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.}}


test base64.154 {decode base64 "abc" with whitespace} {
    set str {

                           YWJj		
    }
    list [catch {blt::decode base64 $str} msg] $msg
} {0 abc}

test base64.155 {decode base64 -data missing value} {
    set str {
	VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3Jl
	ZW4gZmllbGQgYW5kIGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK
    }
    list [catch {blt::decode base64 $str -data} msg] $msg
} {1 {value for "-data" missing}}

test base64.156 {decode base64 -data myVar} {
    set str {
	VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3Jl
	ZW4gZmllbGQgYW5kIGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK
    }
    list [catch {
	blt::decode base64 $str -data myVar
	set myVar
    } msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.157 {decode base64 -data ""} {
    set str {
	VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3Jl
	ZW4gZmllbGQgYW5kIGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK
    }
    list [catch {
	blt::decode base64 $str -data ""
	set ""
    } msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.158 {decode base64 -file missing value} {
    set str {
	VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3Jl
	ZW4gZmllbGQgYW5kIGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK
    }
    list [catch {blt::decode base64 $str -file} msg] $msg
} {1 {value for "-file" missing}}

test base64.159 {decode base64 -file badDir/badFile} {
    set str {
	VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3Jl
	ZW4gZmllbGQgYW5kIGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK
    }
    list [catch {blt::decode base64 $str -file /badDir/badFile} msg] $msg
} {1 {couldn't open "/badDir/badFile": no such file or directory}}

test base64.160 {decode base64 -file no permissions} {
    set str {
	VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3Jl
	ZW4gZmllbGQgYW5kIGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK
    }
    file delete /tmp/myFile
    set f [open "/tmp/myFile" "w"]
    close $f
    file attributes /tmp/myFile -permission -w
    set msg [list [catch {
	blt::decode base64 $str -file /tmp/myFile
    } msg] $msg]
    file delete /tmp/myFile
    set msg
} {1 {couldn't open "/tmp/myFile": permission denied}}

test base64.161 {decode base64 -file /tmp/myFile} {
    file delete /tmp/myFile
    set str {
	VGhlIHNob3J0IHJlZCBmb3ggcmFuIHF1aWNrbHkgdGhyb3VnaCB0aGUgZ3Jl
	ZW4gZmllbGQgYW5kIGp1bXBlZCBvdmVyIHRoZSB0YWxsIGJyb3duIGJlYXIK
    }
    list [catch {
	blt::decode base64 $str -file /tmp/myFile
	set f [open "/tmp/myFile" "r"]
	set contents [read $f]
	close $f
	file delete /tmp/myFile
	set contents
    } msg] $msg
} {0 {The short red fox ran quickly through the green field and jumped over the tall brown bear
}}

test base64.162 {decode base64 to max value} {
    list [catch {
	blt::encode hex [blt::decode base64 {////}] -lower
    } msg] $msg
} {0 ffffff}

test base64.163 {decode base64 ""} {
    list [catch {blt::decode base64 ""} msg] $msg
} {0 {}}

test base64.164 {decode base64 "a~bcd"} {
    list [catch {blt::decode base64 "a~bcd"} msg] $msg
} {1 {invalid character found at 2}}

test base64.165 {decode base64 "ab~d"} {
    list [catch {blt::decode base64 "ab~d"} msg] $msg
} {1 {invalid character found at 3}}

test base64.166 {decode base64 "abc~"} {
    list [catch {blt::decode base64 "abc~"} msg] $msg
} {1 {invalid character found at 4}}

test base64.167 {decode base64 "~YWJj" -ignorebadchars} {
    list [catch {blt::decode base64 "~YWJj" -ignorebadchars} msg] $msg
} {0 abc}

test base64.168 {decode base64 "Y~WJj" -ignorebadchars} {
    list [catch {blt::decode base64 "Y~WJj" -ignorebadchars} msg] $msg
} {0 abc}

test base64.169 {decode base64 "YW~Jj" -ignorebadchars} {
    list [catch {blt::decode base64 "YW~Jj" -ignorebadchars} msg] $msg
} {0 abc}

test base64.170 {decode base64 "YWJ~j" -ignorebadchars} {
    list [catch {blt::decode base64 "YWJ~j" -ignorebadchars} msg] $msg
} {0 abc}

test base64.171 {decode base64 "YWJj~" -ignorebadchars} {
    list [catch {blt::decode base64 "YWJj~" -ignorebadchars} msg] $msg
} {0 abc}

test base64.172 {decode base64 "a"} {
    list [catch {blt::decode base64 "a"} msg] $msg
} {1 {premature end of base64 data}}

test base64.173 {decode base64 "ab"} {
    list [catch {blt::decode base64 "ab"} msg] $msg
} {1 {premature end of base64 data}}

test base64.174 {decode base64 "abc"} {
    list [catch {blt::decode base64 "abc"} msg] $msg
} {1 {premature end of base64 data}}

exit 0


