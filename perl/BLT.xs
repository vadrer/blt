#define PERL_NO_GET_CONTEXT     /* we want efficiency */
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <tcl.h>
typedef Tcl_Interp *Tcl;


DLLEXPORT int Treectrl_Init(Tcl_Interp *interp);

MODULE = Tcl::Tk::Tkwidget::BLT	PACKAGE = Tcl::Tk::Tkwidget::BLT

PROTOTYPES: DISABLE

int
BLT_Init(interp)
	Tcl	interp
    CODE:
	RETVAL = BLT_Init(interp);
    OUTPUT:
	RETVAL
