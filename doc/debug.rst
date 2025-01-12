
==========
blt::debug
==========

------------------------------------------------
Print TCL commands before execution
------------------------------------------------

.. include:: man.rst
.. include:: toc.rst

SYNOPSIS
--------

**blt::debug** ?\ *debugLevel*\ ? ?\ *switches* ...\?

DESCRIPTION
-----------

The **blt::debug** command provides a simple tracing facility for TCL
commands.  Each TCL command line is printed on standard error before it is
executed. The output consists of the command line both before and after
substitutions have occurred.

*DebugLevel* is an integer then it gives a distance (down the procedure
calling stack) before stopping tracing of commands.  If *debugLevel* is
"0", no tracing is performed. This is the default.

If no *debugLevel* argument is given, the current debug level is printed.

*Switches* may be any of the following.

  **-file** *fileName*
    Write the debugging information to the designated file. *FileName* is
    the name of the file to be written.  If *fileName* starts with the
    '@' character, then the following characters are the name of a
    previously opened TCL channel.  By default, debugging information
    is written to standard error.

KEYWORDS
--------

debug

COPYRIGHT
---------

2015 George A. Howlett. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 1) Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2) Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the distribution.
 3) Neither the name of the authors nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.
 4) Products derived from this software may not be called "BLT" nor may
    "BLT" appear in their names without specific prior written permission
    from the author.

THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
