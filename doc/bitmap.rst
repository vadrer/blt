
===============
blt::bitmap
===============

--------------------------------------
Define a new bitmap from a TCL script.
--------------------------------------

.. include:: man.rst
.. include:: toc.rst

SYNOPSIS
--------

**blt::bitmap define** *bitmapName* *data* ?\ *option* *value* ... ?

**blt::bitmap compose** *bitmapName* *text* ?\ *option* *value* ... ?

**blt::bitmap exists** *bitmapName*

**blt::bitmap source** *bitmapName*

**blt::bitmap data** *bitmapName*

**blt::bitmap height** *bitmapName*

**blt::bitmap width** *bitmapName*


DESCRIPTION
-----------

The **blt::bitmap** command lets you create new bitmaps directly from your
TCL script.  The bitmap can be specified as a list of data or a text string
which is converted into a bitmap.  You can arbitrarily scale or rotate the
bitmap too.

INTRODUCTION
------------

Bitmaps are commonly used within Tk.  In label and button widgets, you
display bitmaps them instead of text strings and in the canvas and text
widgets, they're used for stippling.  But Tk let's you can create new
bitmaps only by reading the bitmap data from a file.  This makes bitmaps
cumbersome to manage, especially in packaging the program as a **wish**
script, since each bitmap must be its own file.  It would be nicer if you
could create new bitmaps directly from your TCL script.

The **blt::bitmap** command lets you do just that.  You can specify the
bitmap as in various formats (such as the X11 bitmap format).  You can also
compose a bitmap from a text string.  The **blt::bitmap** command also lets
you and arbitrarily rotate or scale the bitmap.  For example, you could use
this to create button widgets with the text label rotated 90 degrees.

OPERATIONS
----------

The following operations are available for **blt::bitmap**:

**blt::bitmap compose** *bitmapName* *text* ?\ *option* *value* ... ?
  Creates a bitmap *bitmapName* from the text string *text*.
  A bitmap *bitmapName* can not already exist.  
  The following options are available.

  **-font** *fontName* 
    Specifies a font to use when drawing text into the bitmap.  *FontName*
    can be in any form accepted by the Tk **font** command. The default is
    "{San Serif} 9".

  **-rotate** *numDegrees*
    Specifies the angle of rotation of the text in the bitmap.  *NumDegrees* is
    a real number representing the angle in degrees.  The default is "0.0".

  **-scale** *value*
    Specifies the scale of the bitmap.  *Value* is a real number
    representing the scale.  A scale of 1.0 indicates no scaling is
    necessary, while 2.0 would double the size of the bitmap.  There is no
    way to specify differents scales for the width and height of the
    bitmap.  The default scale is "1.0".

**blt::bitmap data** *bitmapName* 
  Returns a list of both the dimensions of the bitmap *bitmapName* and its
  source data.

**blt::bitmap define** *bitmapName* *data* ?\ *option* *value* ... ?
  Associates *bitmapName* with in-memory bitmap data so that
  *bitmapName* can be used in later calls to **Tk_GetBitmap**.  The
  *bitmapName* argument is the name of the bitmap; it must not
  previously have been defined in either a call to **Tk_DefineBitmap** or
  **blt::bitmap**.  The argument *data* describes the bitmap to be
  created.  It is either the X11 bitmap format (a C structure) or a list of
  two lists: the dimensions and source data.  The dimensions are a list of
  two numbers which are the width and height of the bitmap.  The source
  data is a list of hexadecimal values in a format similar to the X11 or
  X10 bitmap format.  The values may be optionally separated by commas and
  do not need to be prefixed with "0x".  The following options are
  available.

  **-rotate** *numDegrees*
      Specifies how many degrees to rotate the bitmap.  *NumDegrees* is a real
      number representing the angle.  The default is "0.0" degrees.

  **-scale** *value*
      Specifies how to scale the bitmap.  *Value* is a real number
      representing the scale.  A scale of 1.0 indicates no scaling is
      necessary, while 2.0 would double the size of the bitmap.  There is
      no way to specify differents scales for the width and height of the
      bitmap.  The default scale is "1.0".

**blt::bitmap exists** *bitmapName*
  Returns "1" if a bitmap *bitmapName* exists, otherwise "0". 

**blt::bitmap height** *bitmapName* 
  Returns the height in pixels of the bitmap *bitmapName*.

**blt::bitmap source** *bitmapName*
  Returns the source data of the bitmap *bitmapName*. The source data is a 
  list of the hexadecimal values.  

**blt::bitmap width** *bitmapName*
  Returns the width in pixels of the bitmap *bitmapName*.

EXAMPLE
-------

You can define a new bitmap with the **define** operation.  For example,
let's say you are using the X11 bitmap "gray1".  Normally to use it, you
would specify the location of the file.

 ::

    label .l -bitmap @/usr/X11R6/include/X11/bitmaps/gray1

But you can simply cut and paste the contents of "gray1" into the 
**blt::bitmap** command.

 ::

     blt::bitmap define gray1 {
         #define gray1_width 2
         #define gray1_height 2
         static char gray1_bits[] = {
            0x01, 0x02};
     }
     label .l -bitmap gray1

Tk will recognize "gray1" as a bitmap which can now be used with any
widget that accepts bitmaps.

 ::

     .barchart element configure elem1 -stipple gray1

The bitmap data can be specified in a mulitude of forms.  The following
commands are all equivalent.

 ::

     blt::bitmap define gray1 {
         #define gray1_width 2
         #define gray1_height 2
         static char gray1_bits[] = {
            0x01, 0x02 };
     }
     blt::bitmap define gray1 { { 2 2 } { 0x01, 0x02 } }
     blt::bitmap define gray1 { { 2 2 } { 0x01 0x02 } }
     blt::bitmap define gray1 { { 2 2 } { 1 2 } }

Either the data is in the standard X11 bitmap form, or it's a list of two
lists. The first list contains the height and width of the bitmap.  The
second list is the bitmap source data.  Each element of that list is an
hexadecimal number specifying which pixels are foreground (1) and which are
background (0) of the bitmap.  Note that the format of the source data is
exactly that of the XBM format.

You can scale or rotate the bitmap as you create it, by using the
**-scale** or **-rotate** options.

 ::

     blt::bitmap define gray1 {
         #define gray1_width 2
         #define gray1_height 2
         static char gray1_bits[] = {
            0x01, 0x02};
     } -scale 2.0 -rotate 90.0

In addition, you can compose bitmaps from text strings.  This makes it
easy to create rotated buttons or labels.  The text string can have
multi-line.  

 ::

    blt::bitmap compose rot_text "This is rotated\\ntext" \\
        -rotate 90.0 -font fixed

There are also a number of ways to query bitmaps.  This isn't limited
to bitmaps that you create, but any bitmap.

 ::

    blt::bitmap exists rot_text
    blt::bitmap width rot_text
    blt::bitmap height rot_text
    blt::bitmap data rot_text
    blt::bitmap source rot_text

The **exists** operation indicates if a bitmap by that name is defined.
You can query the dimensions of the bitmap using the **width** and
**height** operations. The **data** operation returns the list of the data
used to create the bitmap.  For example, you could query the data of a
bitmap and **send** it across the network to another Tk application.

 ::

    set data [blt::bitmap data @/usr/X11R6/include/X11/bitmaps/ghost.xbm]
    send {wish #2} blt::bitmap define ghost $data

LIMITATIONS
-----------

Tk currently offers no way of destroying bitmaps.  Once a bitmap is
created, it exists until the application terminates.

KEYWORDS
--------

bitmap

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
