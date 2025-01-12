/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * bltBind.c --
 *
 * Copyright 2015 George A. Howlett. All rights reserved.  
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are
 *   met:
 *
 *   1) Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2) Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the
 *      distribution.
 *   3) Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *   4) Products derived from this software may not be called "BLT" nor may
 *      "BLT" appear in their names without specific prior written
 *      permission from the author.
 *
 *   THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY EXPRESS OR IMPLIED
 *   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Much of this code is taken from tkCanvas.c
 *
 * Copyright (c) 1991-1994 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
 * Copyright (c) 1998-1999 by Scriptics Corporation.
 *
 *   This software is copyrighted by the Regents of the University of
 *   California, Sun Microsystems, Inc., and other parties.  The following
 *   terms apply to all files associated with the software unless
 *   explicitly disclaimed in individual files.
 * 
 *   The authors hereby grant permission to use, copy, modify, distribute,
 *   and license this software and its documentation for any purpose,
 *   provided that existing copyright notices are retained in all copies
 *   and that this notice is included verbatim in any distributions. No
 *   written agreement, license, or royalty fee is required for any of the
 *   authorized uses.  Modifications to this software may be copyrighted by
 *   their authors and need not follow the licensing terms described here,
 *   provided that the new terms are clearly indicated on the first page of
 *   each file where they apply.
 * 
 *   IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
 *   FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 *   ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
 *   DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 * 
 *   THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 *   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 *   NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, AND
 *   THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
 *   MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *   GOVERNMENT USE: If you are acquiring this software on behalf of the
 *   U.S. government, the Government shall have only "Restricted Rights" in
 *   the software and related documentation as defined in the Federal
 *   Acquisition Regulations (FARs) in Clause 52.227.19 (c) (2).  If you
 *   are acquiring the software on behalf of the Department of Defense, the
 *   software shall be classified as "Commercial Computer Software" and the
 *   Government shall have only "Restricted Rights" as defined in Clause
 *   252.227-7013 (b) (3) of DFARs.  Notwithstanding the foregoing, the
 *   authors grant the U.S. Government and others acting in its behalf
 *   permission to use and distribute the software in accordance with the
 *   terms specified in this license.
 */

#define BUILD_BLT_TK_PROCS 1
#include "bltInt.h"
#include "tkIntDecls.h"
#ifdef HAVE_STRING_H
  #include <string.h>
#endif /* HAVE_STRING_H */

#ifdef HAVE_CTYPE_H
  #include <ctype.h>
#endif /* HAVE_CTYPE_H */

static Tk_EventProc BindProc;

typedef struct _Blt_BindTable BindTable;

#define FULLY_SIMULATE_GRAB     1

/*
 * Binding table procedures.
 */
#define REPICK_IN_PROGRESS      (1<<0)
#define LEFT_GRABBED_OBJECT     (1<<1)

#define ALL_BUTTONS_MASK \
        (Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask)

#ifndef VirtualEventMask
#define VirtualEventMask    (1L << 30)
#endif

#define ALL_VALID_EVENTS_MASK \
        (ButtonMotionMask | Button1MotionMask | Button2MotionMask | \
         Button3MotionMask | Button4MotionMask | Button5MotionMask | \
         ButtonPressMask | ButtonReleaseMask | EnterWindowMask | \
         LeaveWindowMask | KeyPressMask | KeyReleaseMask | \
         PointerMotionMask | VirtualEventMask)

static int buttonMasks[] =
{
    0,                          /* No buttons pressed */
    Button1Mask, Button2Mask, Button3Mask, Button4Mask, Button5Mask,
};

#ifdef notdef
static const char *eventNames[] = {
    "0", "1", 
    "KeyPress",
    "KeyRelease",
    "ButtonPress",
    "ButtonRelease",
    "MotionNotify",
    "EnterNotify",
    "LeaveNotify",
    "FocusIn",
    "FocusOut",
    "KeymapNotify",
    "Expose",
    "GraphicsExpose",
    "NoExpose",
    "VisibilityNotify",
    "CreateNotify",
    "DestroyNotify",
    "UnmapNotify",
    "MapNotify",
    "MapRequest",
    "ReparentNotify",
    "ConfigureNotify",
    "ConfigureRequest",
    "GravityNotify",
    "ResizeRequest",
    "CirculateNotify",
    "CirculateRequest",
    "PropertyNotify",
    "SelectionClear",
    "SelectionRequest",
    "SelectionNotify",
    "ColormapNotify",
    "ClientMessage",
    "MappingNotify",
    "GenericEvent"
};
#endif

/*
 * How to make drag&drop work?
 *
 *      Right now we generate pseudo <Enter> <Leave> events within button
 *      grab on an object.  They're marked NotifyVirtual instead of
 *      NotifyAncestor.  A better solution: generate new-style virtual
 *      <<DragEnter>> <<DragMotion>> <<DragLeave>> events.  These virtual
 *      events don't have to exist as "real" event sequences, like virtual
 *      events do now.
 */

/*
 *---------------------------------------------------------------------------
 *
 * DoEvent --
 *
 *      This procedure is called to invoke binding processing for a new
 *      event that is associated with the current object.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Depends on the bindings for the object.  A binding script could
 *      delete an object, so callers should protect themselves with
 *      Tcl_Preserve and Tcl_Release.
 *
 *---------------------------------------------------------------------------
 */
static void
DoEvent(
    BindTable *bindPtr,                 /* Binding information for widget
                                         * in which event occurred. */
    XEvent *eventPtr,                   /* Real or simulated X event that
                                         * is to be processed. */
    ClientData object,                  /* Object picked. */
    ClientData hint)                    /* Context of object.  */
{
    Blt_Chain tags;

    if ((bindPtr->tkwin == NULL) || (bindPtr->bindingTable == NULL)) {
        return;
    }
    if ((eventPtr->type == KeyPress) || (eventPtr->type == KeyRelease)) {
        object = bindPtr->focusObj;
        hint = bindPtr->focusHint;
    }
    if (object == NULL) {
        return;
    }
    /*
     * Invoke the binding system.
     */
    tags = Blt_Chain_Create();
    if (bindPtr->tagProc == NULL) {
        Blt_Chain_Append(tags, (ClientData)Tk_GetUid("all"));
        Blt_Chain_Append(tags, object);
    } else {
        (*bindPtr->tagProc) (bindPtr, object, hint, tags);
    }
    if (Blt_Chain_GetLength(tags) > 0) {
        int numTags, i;
        ClientData *tagArray;
#define MAX_STATIC_TAGS 64
        ClientData staticTags[MAX_STATIC_TAGS];
        Blt_ChainLink link;
        
        tagArray = staticTags;
        numTags = Blt_Chain_GetLength(tags);
        if (numTags >= MAX_STATIC_TAGS) {
            tagArray = Blt_AssertMalloc(sizeof(ClientData) * numTags);
        } 
        for (i = 0, link = Blt_Chain_FirstLink(tags); link != NULL;
             link = Blt_Chain_NextLink(link), i++) {
            tagArray[i] = Blt_Chain_GetValue(link);
        }
        Tk_BindEvent(bindPtr->bindingTable, eventPtr, bindPtr->tkwin, numTags, 
                tagArray);
        if (tagArray != staticTags) {
            Blt_Free(tagArray);
        }
    }
    Blt_Chain_Destroy(tags);
}

/*
 *---------------------------------------------------------------------------
 *
 * PickCurrentObj --
 *
 *      Picks the object in a binding table give the location of the
 *      pointer (x-y coordinates).  If the current object has changed,
 *      generate a fake exit event on the old current object and a fake
 *      enter event on the new current object.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The current object may change.  If it does, then the commands
 *      associated with object entry and exit could do just about anything.
 *      A binding script could delete the object or parent widget, so
 *      callers should protect themselves with Tcl_Preserve and
 *      Tcl_Release.  This is done automatically if you set
 *      bindPtr->clientData;
 *
 *---------------------------------------------------------------------------
 */
static ClientData
PickCurrentObj(
    BindTable *bindPtr,                 /* Binding table information. */
    XEvent *eventPtr)                   /* Event describing location of mouse
                                         * cursor.  Must be EnterWindow,
                                         * LeaveWindow, ButtonRelease, or
                                         * MotionNotify. */
{
    int buttonDown;
    ClientData newObj;
    ClientData newHint;

    /*
     * Check whether or not a button is down.  If so, we'll log entry and
     * exit into and out of the current object, but not entry into any
     * other object.  This implements a form of grabbing equivalent to what
     * the X server does for windows.
     */
    buttonDown = (bindPtr->state & ALL_BUTTONS_MASK);
    if (!buttonDown) {
        bindPtr->flags &= ~LEFT_GRABBED_OBJECT;
    }

    /*
     * Save information about this event in the widget.  The event in the
     * widget is used for two purposes:
     *
     * 1. Event bindings: if the current object changes, fake events are
     *    generated to allow object-enter and object-leave bindings to
     *    trigger.
     *
     * 2. Reselection: if the current object gets deleted, can use the
     *    saved event to find a new current object.
     *
     * Translate MotionNotify events into EnterNotify events, since that's
     * what gets reported to object handlers.
     */
    if (eventPtr != &bindPtr->pickEvent) {
        if ((eventPtr->type == MotionNotify) ||
            (eventPtr->type == ButtonRelease)) {
            bindPtr->pickEvent.xcrossing.type = EnterNotify;
            bindPtr->pickEvent.xcrossing.serial = eventPtr->xmotion.serial;
            bindPtr->pickEvent.xcrossing.send_event =
                eventPtr->xmotion.send_event;
            bindPtr->pickEvent.xcrossing.display = eventPtr->xmotion.display;
            bindPtr->pickEvent.xcrossing.window = eventPtr->xmotion.window;
            bindPtr->pickEvent.xcrossing.root = eventPtr->xmotion.root;
            bindPtr->pickEvent.xcrossing.subwindow = None;
            bindPtr->pickEvent.xcrossing.time = eventPtr->xmotion.time;
            bindPtr->pickEvent.xcrossing.x = eventPtr->xmotion.x;
            bindPtr->pickEvent.xcrossing.y = eventPtr->xmotion.y;
            bindPtr->pickEvent.xcrossing.x_root = eventPtr->xmotion.x_root;
            bindPtr->pickEvent.xcrossing.y_root = eventPtr->xmotion.y_root;
            bindPtr->pickEvent.xcrossing.mode = NotifyNormal;
            bindPtr->pickEvent.xcrossing.detail = NotifyNonlinear;
            bindPtr->pickEvent.xcrossing.same_screen
                = eventPtr->xmotion.same_screen;
            bindPtr->pickEvent.xcrossing.focus = False;
            bindPtr->pickEvent.xcrossing.state = eventPtr->xmotion.state;
        } else {
            bindPtr->pickEvent = *eventPtr;
        }
    }
    bindPtr->activePick = TRUE;

    /*
     * If this is a recursive call (there's already a partially completed
     * call pending on the stack; it's in the middle of processing a Leave
     * event handler for the old current object) then just return; the
     * pending call will do everything that's needed.
     */
    if (bindPtr->flags & REPICK_IN_PROGRESS) {
        return bindPtr->currentObj;
    }

    /* Initialize the new object to the current object. */
    newObj = bindPtr->currentObj;
    newHint = bindPtr->currentHint;     

    /*
     * A LeaveNotify event automatically means that there's no current
     * object, so the check for closest object can be skipped.
     */
    if (bindPtr->pickEvent.type == LeaveNotify) {
        /* If we've entered an inferior window, there can't be a current
         * object.  Otherwise, the Leave event may be part of a Button
         * event.  (The mode field doesn't seem like it's getting set
         * correctly here.  It should be NotifyGrab or NotifyUngrab.)  In
         * this case, assume we're still on the same object. */
        if (bindPtr->pickEvent.xcrossing.detail == NotifyInferior) {
            newObj = NULL;
            newHint = NULL;
        }
    } else {
         int x, y;
         
         x = bindPtr->pickEvent.xcrossing.x;
         y = bindPtr->pickEvent.xcrossing.y;
         newObj = (*bindPtr->pickProc) (bindPtr->clientData, x, y, &newHint);
     }

    if (((newObj==bindPtr->currentObj) && (newHint==bindPtr->currentHint)) && 
        ((bindPtr->flags & LEFT_GRABBED_OBJECT) == 0)) {
        return bindPtr->currentObj;     /* Nothing to do: the current
                                         * object and hint haven't
                                         * changed. */
    }
#if FULLY_SIMULATE_GRAB
    /* Simulate an implicit grab on button presses by ignoring any other
     * object picked than the current one.. */
    if (((newObj != bindPtr->currentObj) || (newHint!=bindPtr->currentHint)) &&
        (buttonDown)) {
        bindPtr->flags |= LEFT_GRABBED_OBJECT;
        return bindPtr->currentObj;
    }
#endif
    if (!buttonDown) {
        bindPtr->flags &= ~LEFT_GRABBED_OBJECT;
    }

    /* Save the newly picked object and hint. If bindPtr->newObj becomes
     * NULL, this means that Blt_DeleteBindings was called and that the
     * object was destroyed. */
    bindPtr->newObj = newObj;           
    bindPtr->newHint = newHint;

    if (newObj != NULL) {
        Tcl_Preserve(newObj);
    }
    /*
     * Simulate a LeaveNotify event on the previous current object and an
     * EnterNotify event on the new current object.  Remove the "current"
     * tag from the previous current object and place it on the new current
     * object.
     */
    if ((bindPtr->currentObj != NULL) && 
        ((newObj != bindPtr->currentObj) || 
         (newHint != bindPtr->currentHint)) && 
        ((bindPtr->flags & LEFT_GRABBED_OBJECT) == 0)) {
        XEvent event;
        ClientData object;

        event = bindPtr->pickEvent;
        event.type = LeaveNotify;
        /*
         * If the event's detail happens to be NotifyInferior the binding
         * mechanism will discard the event.  To be consistent, always use
         * NotifyAncestor.
         */
        event.xcrossing.detail = NotifyAncestor;
        bindPtr->flags |= REPICK_IN_PROGRESS;
        object = bindPtr->currentObj;
        Tcl_Preserve(object);
        DoEvent(bindPtr, &event, object, bindPtr->currentHint);
        Tcl_Release(object);
        bindPtr->flags &= ~REPICK_IN_PROGRESS;

        /*
         * Note: during DoEvent above, it's possible that bindPtr->newObj
         * got reset to NULL because the object was deleted.
         */
    }

    /*
     * Special note:  it's possible that
     *          bindPtr->newObj == bindPtr->currentObj
     * here.  This can happen, for example, if LEFT_GRABBED_OBJECT was set.
     */

    bindPtr->flags &= ~LEFT_GRABBED_OBJECT;
    bindPtr->currentObj = bindPtr->newObj;
    bindPtr->currentHint = bindPtr->newHint;
    if (bindPtr->currentObj != NULL) {
        XEvent event;
        ClientData object;

        event = bindPtr->pickEvent;
        event.type = EnterNotify;
        event.xcrossing.detail = NotifyAncestor;
        object = bindPtr->currentObj;   /* DoEvent may change the current
                                         * object. */
        Tcl_Preserve(object);
        DoEvent(bindPtr, &event, object, bindPtr->currentHint);
        Tcl_Release(object);
    }
    if (newObj != NULL) {
        Tcl_Release(newObj);
    }
    return bindPtr->currentObj;
}

/*
 *---------------------------------------------------------------------------
 *
 * BindProc --
 *
 *      This procedure is invoked by the Tk dispatcher to handle events
 *      associated with bindings on objects.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Depends on the command invoked as part of the binding
 *      (if there was any).
 *
 *---------------------------------------------------------------------------
 */
static void
BindProc(
    ClientData clientData,      /* Pointer to widget structure. */
    XEvent *eventPtr)           /* Pointer to X event that just happened. */
{
    BindTable *bindPtr = clientData;
    int mask;
    ClientData object;

    Tcl_Preserve(bindPtr->clientData);
    /*
     * This code below keeps track of the current modifier state in
     * bindPtr->state.  This information is used to defer repicks of the
     * current object while buttons are down.
     */
    switch (eventPtr->type) {
    case ButtonPress:
    case ButtonRelease:
        mask = 0;
        if ((eventPtr->xbutton.button >= Button1) &&
            (eventPtr->xbutton.button <= Button5)) {
            mask = buttonMasks[eventPtr->xbutton.button];
        }
        /*
         * For button press events, repick the current object using the
         * button state before the event, then process the event.  For
         * button release events, first process the event, then repick the
         * current object using the button state *after* the event (the
         * button has logically gone up before we change the current
         * object).
         */
        if (eventPtr->type == ButtonPress) {
            /*
             * On a button press, first repick the current object using the
             * button state before the event, the process the event.
             */

            bindPtr->state = eventPtr->xbutton.state;
            PickCurrentObj(bindPtr, eventPtr);
            bindPtr->state ^= mask;
            object = bindPtr->currentObj;
            Tcl_Preserve(object);
            DoEvent(bindPtr, eventPtr, object, bindPtr->currentHint);
            Tcl_Release(object);
        } else {
            /*
             * Button release: first process the event, with the button
             * still considered to be down.  Then repick the current object
             * under the assumption that the button is no longer down.
             */
            bindPtr->state = eventPtr->xbutton.state;
            object = bindPtr->currentObj;
            Tcl_Preserve(object);
            DoEvent(bindPtr, eventPtr, object, bindPtr->currentHint);
            Tcl_Release(object);
            eventPtr->xbutton.state ^= mask;
            bindPtr->state = eventPtr->xbutton.state;
            PickCurrentObj(bindPtr, eventPtr);
            eventPtr->xbutton.state ^= mask;
        }
        break;

    case EnterNotify:
    case LeaveNotify:
        bindPtr->state = eventPtr->xcrossing.state;
        PickCurrentObj(bindPtr, eventPtr);
        break;

    case MotionNotify:
        bindPtr->state = eventPtr->xmotion.state;
        PickCurrentObj(bindPtr, eventPtr);
        object = bindPtr->currentObj;
        Tcl_Preserve(object);
        DoEvent(bindPtr, eventPtr, object, bindPtr->currentHint);
        Tcl_Release(object);
        break;

    case KeyPress:
    case KeyRelease:
        bindPtr->state = eventPtr->xkey.state;
        PickCurrentObj(bindPtr, eventPtr);
        object = bindPtr->currentObj;
        Tcl_Preserve(object);
        DoEvent(bindPtr, eventPtr, object, bindPtr->currentHint);
        Tcl_Release(object);
        break;
    }
    Tcl_Release(bindPtr->clientData);
}

int
Blt_ConfigureBindings(
    Tcl_Interp *interp,
    BindTable *bindPtr,
    ClientData object,
    int argc,
    const char **argv)
{
    const char *command;
    unsigned long mask;
    const char *seq;

    if (argc == 0) {
        Tk_GetAllBindings(interp, bindPtr->bindingTable, object);
        return TCL_OK;
    }
    if (argc == 1) {
        command = Tk_GetBinding(interp, bindPtr->bindingTable, object, argv[0]);
        if (command == NULL) {
            Tcl_AppendResult(interp, "can't find event \"", argv[0], "\"",
                             (char *)NULL);
            return TCL_ERROR;
        }
        Tcl_SetStringObj(Tcl_GetObjResult(interp), command, -1);
        return TCL_OK;
    }

    seq = argv[0];
    command = argv[1];

    if (command[0] == '\0') {
        return Tk_DeleteBinding(interp, bindPtr->bindingTable, object, seq);
    }

    if (command[0] == '+') {
        mask = Tk_CreateBinding(interp, bindPtr->bindingTable, object, seq,
                command + 1, TRUE);
    } else {
        mask = Tk_CreateBinding(interp, bindPtr->bindingTable, object, seq,
                command, FALSE);
    }
    if (mask == 0) {
        Tcl_AppendResult(interp, "event mask can't be zero for \"", object, "\"",
                             (char *)NULL);
        return TCL_ERROR;
    }
    if (mask & (unsigned)~ALL_VALID_EVENTS_MASK) {
        Tk_DeleteBinding(interp, bindPtr->bindingTable, object, seq);
        Tcl_ResetResult(interp);
        Tcl_AppendResult(interp, "requested illegal events; ",
                 "only key, button, motion, enter, leave, and virtual ",
                 "events may be used", (char *)NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}


int
Blt_ConfigureBindingsFromObj(
    Tcl_Interp *interp,
    BindTable *bindPtr,
    ClientData object,
    int objc,
    Tcl_Obj *const *objv)
{
    const char *command;
    unsigned long mask;
    const char *seq;
    const char *string;

    if (objc == 0) {
        Tk_GetAllBindings(interp, bindPtr->bindingTable, object);
        return TCL_OK;
    }
    string = Tcl_GetString(objv[0]);
    if (objc == 1) {
        command = Tk_GetBinding(interp, bindPtr->bindingTable, object, string);
        if (command == NULL) {
            Tcl_ResetResult(interp);
            Tcl_AppendResult(interp, "invalid binding event \"", string, "\"", 
                (char *)NULL);
            return TCL_ERROR;
        }
        Tcl_SetStringObj(Tcl_GetObjResult(interp), command, -1);
        return TCL_OK;
    }

    seq = string;
    command = Tcl_GetString(objv[1]);

    if (command[0] == '\0') {
        return Tk_DeleteBinding(interp, bindPtr->bindingTable, object, seq);
    }

    if (command[0] == '+') {
        mask = Tk_CreateBinding(interp, bindPtr->bindingTable, object, seq,
                command + 1, TRUE);
    } else {
        mask = Tk_CreateBinding(interp, bindPtr->bindingTable, object, seq,
                command, FALSE);
    }
    if (mask == 0) {
        return TCL_ERROR;
    }
    if (mask & (unsigned)~ALL_VALID_EVENTS_MASK) {
        Tk_DeleteBinding(interp, bindPtr->bindingTable, object, seq);
        Tcl_ResetResult(interp);
        Tcl_AppendResult(interp, "requested illegal events; ",
                 "only key, button, motion, enter, leave, and virtual ",
                 "events may be used", (char *)NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}

Blt_BindTable
Blt_CreateBindingTable(Tcl_Interp *interp, Tk_Window tkwin, 
                       ClientData clientData, Blt_BindPickProc *pickProc,
                       Blt_BindAppendTagsProc *tagProc)
{
    unsigned int mask;
    BindTable *bindPtr;

    bindPtr = Blt_AssertCalloc(1, sizeof(BindTable));
    bindPtr->bindingTable = Tk_CreateBindingTable(interp);
    bindPtr->clientData = clientData;
    bindPtr->tkwin = tkwin;
    bindPtr->pickProc = pickProc;
    bindPtr->tagProc = tagProc;
    mask = (KeyPressMask | KeyReleaseMask | ButtonPressMask |
        ButtonReleaseMask | EnterWindowMask | LeaveWindowMask |
        PointerMotionMask);
    Tk_CreateEventHandler(tkwin, mask, BindProc, bindPtr);
    return bindPtr;
}

void
Blt_DestroyBindingTable(BindTable *bindPtr)
{
    unsigned int mask;

    Tk_DeleteBindingTable(bindPtr->bindingTable);
    mask = (KeyPressMask | KeyReleaseMask | ButtonPressMask |
        ButtonReleaseMask | EnterWindowMask | LeaveWindowMask |
        PointerMotionMask);
    Tk_DeleteEventHandler(bindPtr->tkwin, mask, BindProc, bindPtr);
    Blt_Free(bindPtr);
}

void
Blt_PickCurrentItem(BindTable *bindPtr)
{
    if (bindPtr->activePick) {
        PickCurrentObj(bindPtr, &bindPtr->pickEvent);
    }
}

void
Blt_DeleteBindings(BindTable *bindPtr, ClientData object)
{
    Tk_DeleteAllBindings(bindPtr->bindingTable, object);

    /*
     * If this is the object currently picked, we need to repick one.
     */
    if (bindPtr->currentObj == object) {
        bindPtr->currentObj = NULL;
        bindPtr->currentHint = NULL;
    }
    if (bindPtr->newObj == object) {
        bindPtr->newObj = NULL;
        bindPtr->newHint = NULL;
    }
    if (bindPtr->focusObj == object) {
        bindPtr->focusObj = NULL;
        bindPtr->focusHint = NULL;
    }
}

void
Blt_MoveBindingTable(
    BindTable *bindPtr,
    Tk_Window tkwin)
{
    unsigned int mask;

    mask = (KeyPressMask | KeyReleaseMask | ButtonPressMask |
        ButtonReleaseMask | EnterWindowMask | LeaveWindowMask |
        PointerMotionMask);
    if (bindPtr->tkwin != NULL) {
        Tk_DeleteEventHandler(bindPtr->tkwin, mask, BindProc, bindPtr);
    }
    Tk_CreateEventHandler(tkwin, mask, BindProc, bindPtr);
    bindPtr->tkwin = tkwin;
}

/*
 * The following union is used to hold the detail information from an
 * XEvent (including Tk's XVirtualEvent extension).
 */
typedef union {
    KeySym      keySym;     /* KeySym that corresponds to xkey.keycode. */
    int         button;     /* Button that was pressed (xbutton.button). */
    Tk_Uid      name;       /* Tk_Uid of virtual event. */
    ClientData  clientData; /* Used when type of Detail is unknown, and to
                             * ensure that all bytes of Detail are initialized
                             * when this structure is used in a hash key. */
} Detail;


/*
 * The following structure defines a pattern, which is matched against X
 * events as part of the process of converting X events into TCL commands.
 */
typedef struct {
    int eventType;              /* Type of X event, e.g. ButtonPress. */
    int needMods;               /* Mask of modifiers that must be
                                 * present (0 means no modifiers are
                                 * required). */
    Detail detail;              /* Additional information that must
                                 * match event.  Normally this is 0,
                                 * meaning no additional information
                                 * must match.  For KeyPress and
                                 * KeyRelease events, a keySym may
                                 * be specified to select a
                                 * particular keystroke (0 means any
                                 * keystrokes).  For button events,
                                 * specifies a particular button (0
                                 * means any buttons are OK).  For virtual
                                 * events, specifies the Tk_Uid of the
                                 * virtual event name (never 0). */
} Pattern;

typedef struct {
    const char *name;           /* Name of modifier. */
    int mask;                   /* Button/modifier mask value, such as
                                 * Button1Mask. */
    int flags;                  /* Various flags; see below for
                                 * definitions. */
} EventModifier;

/*
 * Flags for EventModifier structures:
 *
 * DOUBLE -             Non-zero means duplicate this event,
 *                      e.g. for double-clicks.
 * TRIPLE -             Non-zero means triplicate this event,
 *                      e.g. for triple-clicks.
 * QUADRUPLE -          Non-zero means quadruple this event,
 *                      e.g. for 4-fold-clicks.
 * MULT_CLICKS -        Combination of all of above.
 */

#define DOUBLE          (1<<0)
#define TRIPLE          (1<<1)
#define QUADRUPLE       (1<<2)
#define MULT_CLICKS     (DOUBLE|TRIPLE|QUADRUPLE)

#define META_MASK       (AnyModifier<<1)
#define ALT_MASK        (AnyModifier<<2)

static EventModifier eventModifiers[] = {
    {"Alt",             ALT_MASK,       0},
    {"Any",             0,              0},     /* Ignored: historical relic. */
    {"B1",              Button1Mask,    0},
    {"B2",              Button2Mask,    0},
    {"B3",              Button3Mask,    0},
    {"B4",              Button4Mask,    0},
    {"B5",              Button5Mask,    0},
    {"Button1",         Button1Mask,    0},
    {"Button2",         Button2Mask,    0},
    {"Button3",         Button3Mask,    0},
    {"Button4",         Button4Mask,    0},
    {"Button5",         Button5Mask,    0},
    {"Command",         Mod1Mask,       0},
    {"Control",         ControlMask,    0},
    {"Double",          0,              DOUBLE},
    {"Lock",            LockMask,       0},
    {"M",               META_MASK,      0},
    {"M1",              Mod1Mask,       0},
    {"M2",              Mod2Mask,       0},
    {"M3",              Mod3Mask,       0},
    {"M4",              Mod4Mask,       0},
    {"M5",              Mod5Mask,       0},
    {"Meta",            META_MASK,      0},
    {"Mod1",            Mod1Mask,       0},
    {"Mod2",            Mod2Mask,       0},
    {"Mod3",            Mod3Mask,       0},
    {"Mod4",            Mod4Mask,       0},
    {"Mod5",            Mod5Mask,       0},
    {"Option",          Mod2Mask,       0},
    {"Quadruple",       0,              QUADRUPLE},
    {"Shift",           ShiftMask,      0},
    {"Triple",          0,              TRIPLE},
};

typedef struct {
    const char *name;           /* Name of event. */
    int type;                   /* Event type for X, such as
                                 * ButtonPress. */
    int eventMask;              /* Mask bits (for XSelectInput)
                                 * for this event type. */
} EventInfo;

/*
 * Note:  some of the masks below are an OR-ed combination of
 * several masks.  This is necessary because X doesn't report
 * up events unless you also ask for down events.  Also, X
 * doesn't report button state in motion events unless you've
 * asked about button events.
 */

static EventInfo events[] = {
    {"Activate",        ActivateNotify,         ActivateMask},
    {"Button",          ButtonPress,            ButtonPressMask},
    {"ButtonPress",     ButtonPress,            ButtonPressMask},
    {"ButtonRelease",   ButtonRelease, ButtonPressMask|ButtonReleaseMask},
    {"Circulate",       CirculateNotify,        StructureNotifyMask},
    {"CirculateRequest", CirculateRequest,      SubstructureRedirectMask},
    {"Colormap",        ColormapNotify,         ColormapChangeMask},
    {"Configure",       ConfigureNotify,        StructureNotifyMask},
    {"ConfigureRequest", ConfigureRequest,      SubstructureRedirectMask},
    {"Create",          CreateNotify,           SubstructureNotifyMask},
    {"Deactivate",      DeactivateNotify,       ActivateMask},
    {"Destroy",         DestroyNotify,          StructureNotifyMask},
    {"Enter",           EnterNotify,            EnterWindowMask},
    {"Expose",          Expose,                 ExposureMask},
    {"FocusIn",         FocusIn,                FocusChangeMask},
    {"FocusOut",        FocusOut,               FocusChangeMask},
    {"Gravity",         GravityNotify,          StructureNotifyMask},
    {"Key",             KeyPress,               KeyPressMask},
    {"KeyPress",        KeyPress,               KeyPressMask},
    {"KeyRelease",      KeyRelease,             KeyPressMask|KeyReleaseMask},
    {"Leave",           LeaveNotify,            LeaveWindowMask},
    {"Map",             MapNotify,              StructureNotifyMask},
    {"MapRequest",      MapRequest,             SubstructureRedirectMask},
    {"Motion",          MotionNotify, ButtonPressMask|PointerMotionMask},
    {"MouseWheel",      MouseWheelEvent,        MouseWheelMask},
    {"Property",        PropertyNotify,         PropertyChangeMask},
    {"Reparent",        ReparentNotify,         StructureNotifyMask},
    {"ResizeRequest",   ResizeRequest,          ResizeRedirectMask},
    {"Unmap",           UnmapNotify,            StructureNotifyMask},
    {"Visibility",      VisibilityNotify,       VisibilityChangeMask},
};

/*
 * The defines and table below are used to classify events into
 * various groups.  The reason for this is that logically identical
 * fields (e.g. "state") appear at different places in different
 * types of events.  The classification masks can be used to figure
 * out quickly where to extract information from events.
 */

#define KEY                     0x1
#define BUTTON                  0x2
#define MOTION                  0x4
#define CROSSING                0x8
#define FOCUS                   0x10
#define EXPOSE                  0x20
#define VISIBILITY              0x40
#define CREATE                  0x80
#define DESTROY                 0x100
#define UNMAP                   0x200
#define MAP                     0x400
#define REPARENT                0x800
#define CONFIG                  0x1000
#define GRAVITY                 0x2000
#define CIRC                    0x4000
#define PROP                    0x8000
#define COLORMAP                0x10000
#define VIRTUAL                 0x20000
#define ACTIVATE                0x40000
#define MAPREQ                  0x80000
#define CONFIGREQ               0x100000
#define RESIZEREQ               0x200000
#define CIRCREQ                 0x400000

#define KEY_BUTTON_MOTION_VIRTUAL       (KEY|BUTTON|MOTION|VIRTUAL)
#define KEY_BUTTON_MOTION_CROSSING      (KEY|BUTTON|MOTION|CROSSING|VIRTUAL)

static int flagArray[TK_LASTEVENT+1] = {
   /* Not used */               0,
   /* Not used */               0,
   /* KeyPress */               KEY,
   /* KeyRelease */             KEY,
   /* ButtonPress */            BUTTON,
   /* ButtonRelease */          BUTTON,
   /* MotionNotify */           MOTION,
   /* EnterNotify */            CROSSING,
   /* LeaveNotify */            CROSSING,
   /* FocusIn */                FOCUS,
   /* FocusOut */               FOCUS,
   /* KeymapNotify */           0,
   /* Expose */                 EXPOSE,
   /* GraphicsExpose */         EXPOSE,
   /* NoExpose */               0,
   /* VisibilityNotify */       VISIBILITY,
   /* CreateNotify */           CREATE,
   /* DestroyNotify */          DESTROY,
   /* UnmapNotify */            UNMAP,
   /* MapNotify */              MAP,
   /* MapRequest */             MAPREQ,
   /* ReparentNotify */         REPARENT,
   /* ConfigureNotify */        CONFIG,
   /* ConfigureRequest */       CONFIGREQ,
   /* GravityNotify */          GRAVITY,
   /* ResizeRequest */          RESIZEREQ,
   /* CirculateNotify */        CIRC,
   /* CirculateRequest */       0,
   /* PropertyNotify */         PROP,
   /* SelectionClear */         0,
   /* SelectionRequest */       0,
   /* SelectionNotify */        0,
   /* ColormapNotify */         COLORMAP,
   /* ClientMessage */          0,
   /* MappingNotify */          0,
#ifdef GenericEvent
   /* GenericEvent */           0,
#endif
   /* VirtualEvent */           VIRTUAL,
   /* Activate */               ACTIVATE,           
   /* Deactivate */             ACTIVATE,
   /* MouseWheel */             KEY
};


static EventModifier *
FindModifier(const char *string)
{
    int high, low;
    char c;

    low = 0;
    high = (sizeof(eventModifiers) / sizeof(EventModifier)) - 1;
    c = string[0];
    while (low <= high) {
        EventModifier *modPtr;
        int compare;
        int median;
        
        median = (low + high) >> 1;
        modPtr = eventModifiers + median;

        /* Test the first character */
        compare = c - modPtr->name[0];
        if (compare == 0) {
            compare = strcmp(string, modPtr->name);
        }
        if (compare < 0) {
            high = median - 1;
        } else if (compare > 0) {
            low = median + 1;
        } else {
            return modPtr;      /* Modifier found. */
        }
    }
    return NULL;                /* Can't find modifier */
}

static EventInfo *
FindEvent(const char *string)
{
    int high, low;
    char c;

    low = 0;
    high = (sizeof(events) / sizeof(EventInfo)) - 1;
    c = string[0];
    while (low <= high) {
        EventInfo *infoPtr;
        int compare;
        int median;
        
        median = (low + high) >> 1;
        infoPtr = events + median;

        /* Test the first character */
        compare = c - infoPtr->name[0];
        if (compare == 0) {
            compare = strcmp(string, infoPtr->name);
        }
        if (compare < 0) {
            high = median - 1;
        } else if (compare > 0) {
            low = median + 1;
        } else {
            return infoPtr;     /* Event found. */
        }
    }
    return NULL;                /* Can't find event. */
}


/*
 *----------------------------------------------------------------------
 *
 * GetField --
 *
 *      Used to parse pattern descriptions.  Copies up to
 *      size characters from p to copy, stopping at end of
 *      string, space, "-", ">", or whenever size is
 *      exceeded.
 *
 * Results:
 *      The return value is a pointer to the character just
 *      after the last one copied (usually "-" or space or
 *      ">", but could be anything if size was exceeded).
 *      Also places NULL-terminated string (up to size
 *      character, including NULL), at copy.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static char *
GetField(p, copy, size)
    char *p;                    /* Pointer to part of pattern. */
    char *copy;                 /* Place to copy field. */
    int size;                   /* Maximum number of characters to
                                 * copy. */
{
    while ((*p != '\0') && !isspace(UCHAR(*p)) && (*p != '>')
            && (*p != '-') && (size > 1)) {
        *copy = *p;
        p++;
        copy++;
        size--;
    }
    *copy = '\0';
    return p;
}

static int
ParseEventDescription(Tcl_Interp *interp, const char **eventStringPtr,
                      Pattern *patPtr, unsigned long *eventMaskPtr)
{
    char *p;
    unsigned long eventMask;
    int count, eventFlags;
#define FIELD_SIZE 48
    char field[FIELD_SIZE];
    EventInfo *infoPtr;

    Tcl_DString copy;
    Tcl_DStringInit(&copy);
    p = Tcl_DStringAppend(&copy, *eventStringPtr, -1);

    patPtr->eventType = -1;
    patPtr->needMods = 0;
    patPtr->detail.clientData = 0;

    eventMask = 0;
    count = 1;
    
    /*
     * Handle simple ASCII characters.
     */

    if (*p != '<') {
        char string[2];

        patPtr->eventType = KeyPress;
        eventMask = KeyPressMask;
        string[0] = *p;
        string[1] = 0;
        patPtr->detail.keySym = XStringToKeysym(string);
        if (patPtr->detail.keySym == NoSymbol) {
            if (isprint(UCHAR(*p))) {
                patPtr->detail.keySym = *p;
            } else {
                char buf[64];
                
                sprintf(buf, "bad ASCII character 0x%x", (unsigned char) *p);
                Tcl_SetResult(interp, buf, TCL_VOLATILE);
                count = 0;
                goto done;
            }
        }
        p++;
        goto end;
    }

    /*
     * A physical event description consists of:
     *
     * 1. open angle bracket.
     * 2. any number of modifiers, each followed by spaces
     *    or dashes.
     * 3. an optional event name.
     * 4. an option button or keysym name.  Either this or
     *    item 3 *must* be present;  if both are present
     *    then they are separated by spaces or dashes.
     * 5. a close angle bracket.
     */

    p++;

    while (1) {
        EventModifier *modPtr;
        p = GetField(p, field, FIELD_SIZE);
        if (*p == '>') {
            /*
             * This solves the problem of, e.g., <Control-M> being
             * misinterpreted as Control + Meta + missing keysym
             * instead of Control + KeyPress + M.
             */
             break;
        }
        modPtr = FindModifier(field);
        if (modPtr == NULL) {
            break;
        }
        patPtr->needMods |= modPtr->mask;
        if (modPtr->flags & (MULT_CLICKS)) {
            int i = modPtr->flags & MULT_CLICKS;
            count = 2;
            while (i >>= 1) count++;
        }
        while ((*p == '-') || isspace(UCHAR(*p))) {
            p++;
        }
    }

    eventFlags = 0;
    infoPtr = FindEvent(field);
    if (infoPtr != NULL) {
        patPtr->eventType = infoPtr->type;
        eventFlags = flagArray[infoPtr->type];
        eventMask = infoPtr->eventMask;
        while ((*p == '-') || isspace(UCHAR(*p))) {
            p++;
        }
        p = GetField(p, field, FIELD_SIZE);
    }
    if (*field != '\0') {
        if ((*field >= '1') && (*field <= '5') && (field[1] == '\0')) {
            if (eventFlags == 0) {
                patPtr->eventType = ButtonPress;
                eventMask = ButtonPressMask;
            } else if (eventFlags & KEY) {
                goto getKeysym;
            } else if ((eventFlags & BUTTON) == 0) {
                Tcl_AppendResult(interp, "specified button \"", field,
                        "\" for non-button event", (char *) NULL);
                count = 0;
                goto done;
            }
            patPtr->detail.button = (*field - '0');
        } else {
            getKeysym:
            patPtr->detail.keySym = XStringToKeysym(field);
            if (patPtr->detail.keySym == NoSymbol) {
                Tcl_AppendResult(interp, "bad event type or keysym \"",
                        field, "\"", (char *)NULL);
                count = 0;
                goto done;
            }
            if (eventFlags == 0) {
                patPtr->eventType = KeyPress;
                eventMask = KeyPressMask;
            } else if ((eventFlags & KEY) == 0) {
                Tcl_AppendResult(interp, "specified keysym \"", field,
                        "\" for non-key event", (char *)NULL);
                count = 0;
                goto done;
            }
        }
    } else if (eventFlags == 0) {
        Tcl_AppendResult(interp, "no event type or button # or keysym", 
                         (char *)NULL);
        count = 0;
        goto done;
    }

    while ((*p == '-') || isspace(UCHAR(*p))) {
        p++;
    }
    if (*p != '>') {
        while (*p != '\0') {
            p++;
            if (*p == '>') {
                Tcl_AppendResult(interp, 
                                 "extra characters after detail in binding",
                                 (char *)NULL);
                count = 0;
                goto done;
            }
        }
        Tcl_AppendResult(interp, "missing \">\" in binding", (char *)NULL);
        count = 0;
        goto done;
    }
    p++;

end:
    *eventStringPtr += (p - Tcl_DStringValue(&copy));
    *eventMaskPtr |= eventMask;
done:
    Tcl_DStringFree(&copy);
    return count;
}

struct _TkStateMap {
    int numKey;                 /* Integer representation of a value. */
    const char *strKey;         /* String representation of a value. */
};

static TkStateMap notifyMode[] = {
    {NotifyNormal,              "NotifyNormal"},
    {NotifyGrab,                "NotifyGrab"},
    {NotifyUngrab,              "NotifyUngrab"},
    {NotifyWhileGrabbed,        "NotifyWhileGrabbed"},
    {-1, NULL}
};

static TkStateMap notifyDetail[] = {
    {NotifyAncestor,            "NotifyAncestor"},
    {NotifyVirtual,             "NotifyVirtual"},
    {NotifyInferior,            "NotifyInferior"},
    {NotifyNonlinear,           "NotifyNonlinear"},
    {NotifyNonlinearVirtual,    "NotifyNonlinearVirtual"},
    {NotifyPointer,             "NotifyPointer"},
    {NotifyPointerRoot,         "NotifyPointerRoot"},
    {NotifyDetailNone,          "NotifyDetailNone"},
    {-1, NULL}
};

static TkStateMap circPlace[] = {
    {PlaceOnTop,                "PlaceOnTop"},
    {PlaceOnBottom,             "PlaceOnBottom"},
    {-1, NULL}
};

static TkStateMap visNotify[] = {
    {VisibilityUnobscured,          "VisibilityUnobscured"},
    {VisibilityPartiallyObscured,   "VisibilityPartiallyObscured"},
    {VisibilityFullyObscured,       "VisibilityFullyObscured"},
    {-1, NULL}
};

#ifdef notdef
static TkStateMap configureRequestDetail[] = {
    {None,              "None"},
    {Above,             "Above"},
    {Below,             "Below"},
    {BottomIf,          "BottomIf"},
    {TopIf,             "TopIf"},
    {Opposite,          "Opposite"},
    {-1, NULL}
};

static TkStateMap propNotify[] = {
    {PropertyNewValue,  "NewValue"},
    {PropertyDelete,    "Delete"},
    {-1, NULL}
};
#endif

/*
 *---------------------------------------------------------------------------
 *
 * HandleEventGenerate --
 *
 *      Helper function for the "event generate" command.  Generate and
 *      process an XEvent, constructed from information parsed from the
 *      event description string and its optional arguments.
 *
 *      argv[0] contains name of the target window.
 *      argv[1] contains pattern string for one event (e.g, <Control-v>).
 *      argv[2..argc-1] contains -field/option pairs for specifying
 *                      additional detail in the generated event.
 *
 *      Either virtual or physical events can be generated this way.
 *      The event description string must contain the specification
 *      for only one event.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      When constructing the event, 
 *       event.xany.serial is filled with the current X serial number.
 *       event.xany.window is filled with the target window.
 *       event.xany.display is filled with the target window's display.
 *      Any other fields in eventPtr which are not specified by the pattern
 *      string or the optional arguments, are set to 0.
 *
 *      The event may be handled sychronously or asynchronously, depending
 *      on the value specified by the optional "-when" option.  The
 *      default setting is synchronous.
 *
 *---------------------------------------------------------------------------
 */
static int
SendEventCmd(ClientData clientData, Tcl_Interp *interp, int objc, 
         Tcl_Obj *const *objv)
{
    union {
        XEvent generic;    
        XVirtualEvent virtual;
    } event;
    XEvent *eventPtr;
    const char *p;
    char *name;
    Window window;
    Display *display;
    Tk_Window tkwin;
    int count, flags, i, number, warp;
#ifdef notdef
    int synch;
    Tcl_QueuePosition pos;
#endif
    Pattern pat;
    unsigned long eventMask;
    static const char *fieldStrings[] = {
        "-when",        "-above",       "-borderwidth", "-button",
        "-count",       "-delta",       "-detail",      "-focus",
        "-height",
        "-keycode",     "-keysym",      "-mode",        "-override",
        "-place",       "-root",        "-rootx",       "-rooty",
        "-sendevent",   "-serial",      "-state",       "-subwindow",
        "-time",        "-warp",        "-width",       "-window",
        "-x",           "-y",   NULL
    };
    enum field {
        EVENT_WHEN,     EVENT_ABOVE,    EVENT_BORDER,   EVENT_BUTTON,
        EVENT_COUNT,    EVENT_DELTA,    EVENT_DETAIL,   EVENT_FOCUS,
        EVENT_HEIGHT,
        EVENT_KEYCODE,  EVENT_KEYSYM,   EVENT_MODE,     EVENT_OVERRIDE,
        EVENT_PLACE,    EVENT_ROOT,     EVENT_ROOTX,    EVENT_ROOTY,
        EVENT_SEND,     EVENT_SERIAL,   EVENT_STATE,    EVENT_SUBWINDOW,
        EVENT_TIME,     EVENT_WARP,     EVENT_WIDTH,    EVENT_WINDOW,
        EVENT_X,        EVENT_Y
    };
    tkwin = Tk_MainWindow(interp);
    if (Blt_GetWindowFromObj(interp, objv[1], &window) != TCL_OK) {
        return TCL_ERROR;
    }
    name = Tcl_GetStringFromObj(objv[2], NULL);

    display = Tk_Display(tkwin);
    p = name;
    eventMask = 0;
    count = ParseEventDescription(interp, &p, &pat, &eventMask);
    if (count == 0) {
        return TCL_ERROR;
    }
    if (count != 1) {
        Tcl_AppendResult(interp, "Double or Triple modifier not allowed",
                         (char *)NULL);
        return TCL_ERROR;
    }
    if (*p != '\0') {
        Tcl_AppendResult(interp, "only one event specification allowed",
                         (char *)NULL);
        return TCL_ERROR;
    }

    memset((VOID *) &event.generic, 0, sizeof(event));
    eventPtr = &event.generic;
    eventPtr->xany.type = pat.eventType;
    eventPtr->xany.serial = NextRequest(display);
    eventPtr->xany.send_event = False;
    eventPtr->xany.window = window;
    eventPtr->xany.display = display;

    flags = flagArray[eventPtr->xany.type];
    if (flags & DESTROY) {
        /*
         * Event DestroyNotify should be generated by destroying 
         * the window.
         */
        XDestroyWindow(display, window);
        return TCL_OK;
    }
    if (flags & (KEY_BUTTON_MOTION_VIRTUAL)) {
        eventPtr->xkey.state = pat.needMods;
        if ((flags & KEY) && (eventPtr->xany.type != MouseWheelEvent)) {
            TkpSetKeycodeAndState(tkwin, pat.detail.keySym, eventPtr);
        } else if (flags & BUTTON) {
            eventPtr->xbutton.button = pat.detail.button;
        } else if (flags & VIRTUAL) {
            event.virtual.name = pat.detail.name;
        }
    }
    if (flags & (CREATE|UNMAP|MAP|REPARENT|CONFIG|GRAVITY|CIRC)) {
        eventPtr->xcreatewindow.window = eventPtr->xany.window;
    }

    if (flags & (KEY_BUTTON_MOTION_VIRTUAL|CROSSING)) {
        eventPtr->xkey.x_root = -1;
        eventPtr->xkey.y_root = -1;
    }

    /*
     * Process the remaining arguments to fill in additional fields
     * of the event.
     */

    warp = 0;
#ifdef notdef
    synch = 1;
    pos = TCL_QUEUE_TAIL;
#endif
    for (i = 3; i < objc; i += 2) {
        Tcl_Obj *optionPtr, *valuePtr;
        int index;
        
        optionPtr = objv[i];
        valuePtr = objv[i + 1];

        if (Tcl_GetIndexFromObj(interp, optionPtr, fieldStrings, "option",
                TCL_EXACT, &index) != TCL_OK) {
            return TCL_ERROR;
        }
        if ((objc & 1) == 0) {
            /*
             * This test occurs after Tcl_GetIndexFromObj() so that
             * "event generate <Button> -xyz" will return the error message
             * that "-xyz" is a bad option, rather than that the value
             * for "-xyz" is missing.
             */

            Tcl_AppendResult(interp, "value for \"",
                    Tcl_GetStringFromObj(optionPtr, NULL), "\" missing",
                    (char *)NULL);
            return TCL_ERROR;
        }

        switch ((enum field) index) {
        case EVENT_WARP: {
            if (Tcl_GetBooleanFromObj(interp, valuePtr, &warp) != TCL_OK) {
                return TCL_ERROR;
            }
            if (!(flags & (KEY_BUTTON_MOTION_VIRTUAL))) {
                goto badopt;
            }
            break;
        }
        case EVENT_WHEN: {
#ifdef notdef
            pos = (Tcl_QueuePosition) TkFindStateNumObj(interp, optionPtr, 
                queuePosition, valuePtr);
            if ((int) pos < -1) {
                return TCL_ERROR;
            }
            synch = 0;
            if ((int) pos == -1) {
                synch = 1;
            }
#endif
            break;
        }
        case EVENT_ABOVE: {
            Window window2;

            if (Blt_GetWindowFromObj(interp, valuePtr, &window2) != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & CONFIG) {
                eventPtr->xconfigure.above = window2;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_BORDER: {
            if (Tk_GetPixelsFromObj(interp, tkwin, valuePtr, &number)!=TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & (CREATE|CONFIG)) {
                eventPtr->xcreatewindow.border_width = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_BUTTON: {
            if (Tcl_GetIntFromObj(interp, valuePtr, &number) != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & BUTTON) {
                eventPtr->xbutton.button = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_COUNT: {
            if (Tcl_GetIntFromObj(interp, valuePtr, &number) != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & EXPOSE) {
                eventPtr->xexpose.count = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_DELTA: {
            if (Tcl_GetIntFromObj(interp, valuePtr, &number) != TCL_OK) {
                return TCL_ERROR;
            }
            if ((flags & KEY) && (eventPtr->xkey.type == MouseWheelEvent)) {
                eventPtr->xkey.keycode = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_DETAIL: {
            number = TkFindStateNumObj(interp, optionPtr, notifyDetail, 
                                       valuePtr);
            if (number < 0) {
                return TCL_ERROR;
            }
            if (flags & FOCUS) {
                eventPtr->xfocus.detail = number;
            } else if (flags & CROSSING) {
                eventPtr->xcrossing.detail = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_FOCUS: {
            if (Tcl_GetBooleanFromObj(interp, valuePtr, &number) != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & CROSSING) {
                eventPtr->xcrossing.focus = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_HEIGHT: {
            if (Tk_GetPixelsFromObj(interp, tkwin, valuePtr, &number)!= TCL_OK){
                return TCL_ERROR;
            }
            if (flags & EXPOSE) {
                eventPtr->xexpose.height = number;
            } else if (flags & CONFIG) {
                eventPtr->xconfigure.height = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_KEYCODE: {
            if (Tcl_GetIntFromObj(interp, valuePtr, &number) != TCL_OK) {
                return TCL_ERROR;
            }
            if ((flags & KEY) && (eventPtr->xkey.type != MouseWheelEvent)) {
                eventPtr->xkey.keycode = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_KEYSYM: {
            KeySym keysym;
            char *value;
            
            value = Tcl_GetStringFromObj(valuePtr, NULL);
            keysym = TkStringToKeysym(value);
            if (keysym == NoSymbol) {
                Tcl_AppendResult(interp, "unknown keysym \"", value, "\"",
                                 (char *)NULL);
                return TCL_ERROR;
            }
            
            TkpSetKeycodeAndState(tkwin, keysym, eventPtr);
            if (eventPtr->xkey.keycode == 0) {
                Tcl_AppendResult(interp, "no keycode for keysym \"", value,
                                 "\"", (char *)NULL);
                return TCL_ERROR;
            }
            if (!(flags & KEY) || (eventPtr->xkey.type == MouseWheelEvent)) {
                goto badopt;
            }
            break;
        }
        case EVENT_MODE: {
            number = TkFindStateNumObj(interp, optionPtr, notifyMode,
                                       valuePtr);
            if (number < 0) {
                return TCL_ERROR;
            }
            if (flags & CROSSING) {
                eventPtr->xcrossing.mode = number;
            } else if (flags & FOCUS) {
                eventPtr->xfocus.mode = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_OVERRIDE: {
            if (Tcl_GetBooleanFromObj(interp, valuePtr, &number) != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & CREATE) {
                eventPtr->xcreatewindow.override_redirect = number;
            } else if (flags & MAP) {
                eventPtr->xmap.override_redirect = number;
            } else if (flags & REPARENT) {
                eventPtr->xreparent.override_redirect = number;
            } else if (flags & CONFIG) {
                eventPtr->xconfigure.override_redirect = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_PLACE: {
            number = TkFindStateNumObj(interp, optionPtr, circPlace,
                                       valuePtr);
            if (number < 0) {
                return TCL_ERROR;
            }
            if (flags & CIRC) {
                eventPtr->xcirculate.place = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_ROOT: {
            Window window2;

            if (Blt_GetWindowFromObj(interp, valuePtr, &window2) != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & (KEY_BUTTON_MOTION_VIRTUAL|CROSSING)) {
                eventPtr->xkey.root = window2;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_ROOTX: {
            if (Tk_GetPixelsFromObj(interp, tkwin, valuePtr, &number)
                != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & (KEY_BUTTON_MOTION_VIRTUAL|CROSSING)) {
                eventPtr->xkey.x_root = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_ROOTY: {
            if (Tk_GetPixelsFromObj(interp, tkwin, valuePtr, &number)
                != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & (KEY_BUTTON_MOTION_VIRTUAL|CROSSING)) {
                eventPtr->xkey.y_root = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_SEND: {
            const char *value;
            
            value = Tcl_GetStringFromObj(valuePtr, NULL);
            if (isdigit(UCHAR(value[0]))) {
                /*
                 * Allow arbitrary integer values for the field; they
                 * are needed by a few of the tests in the Tk test suite.
                 */
                
                if (Tcl_GetIntFromObj(interp, valuePtr, &number)
                    != TCL_OK) {
                    return TCL_ERROR;
                }
            } else {
                if (Tcl_GetBooleanFromObj(interp, valuePtr, &number)
                    != TCL_OK) {
                    return TCL_ERROR;
                }
            }
            eventPtr->xany.send_event = number;
            break;
        }
        case EVENT_SERIAL: {
            if (Tcl_GetIntFromObj(interp, valuePtr, &number) != TCL_OK) {
                return TCL_ERROR;
            }
            eventPtr->xany.serial = number;
            break;
        }
        case EVENT_STATE: {
            if (flags & (KEY_BUTTON_MOTION_VIRTUAL|CROSSING)) {
                if (Tcl_GetIntFromObj(interp, valuePtr, &number)
                    != TCL_OK) {
                    return TCL_ERROR;
                }
                if (flags & (KEY_BUTTON_MOTION_VIRTUAL)) {
                    eventPtr->xkey.state = number;
                } else {
                    eventPtr->xcrossing.state = number;
                }
            } else if (flags & VISIBILITY) {
                number = TkFindStateNumObj(interp, optionPtr, visNotify,
                                           valuePtr);
                if (number < 0) {
                    return TCL_ERROR;
                }
                eventPtr->xvisibility.state = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_SUBWINDOW: {
            Window window2;

            if (Blt_GetWindowFromObj(interp, valuePtr, &window2) != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & (KEY_BUTTON_MOTION_VIRTUAL|CROSSING)) {
                eventPtr->xkey.subwindow = window2;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_TIME: {
            if (Tcl_GetIntFromObj(interp, valuePtr, &number) != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & (KEY_BUTTON_MOTION_VIRTUAL|CROSSING)) {
                eventPtr->xkey.time = (Time) number;
            } else if (flags & PROP) {
                eventPtr->xproperty.time = (Time) number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_WIDTH: {
            if (Tk_GetPixelsFromObj(interp, tkwin, valuePtr, &number)
                != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & EXPOSE) {
                eventPtr->xexpose.width = number;
            } else if (flags & (CREATE|CONFIG)) {
                eventPtr->xcreatewindow.width = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_WINDOW: {
            Window window2;

            if (Blt_GetWindowFromObj(interp, valuePtr, &window2) != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & (CREATE|UNMAP|MAP|REPARENT|CONFIG
                         |GRAVITY|CIRC)) {
                eventPtr->xcreatewindow.window = window2;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_X: {
            if (Tk_GetPixelsFromObj(interp, tkwin, valuePtr, &number)
                != TCL_OK) {
                return TCL_ERROR;
            }
            if (flags & (KEY_BUTTON_MOTION_VIRTUAL|CROSSING)) { 
                eventPtr->xkey.x = number;
                /*
                 * Only modify rootx as well if it hasn't been changed.
                 */
                if (eventPtr->xkey.x_root == -1) {
                    int rootX, rootY;
                    
                    Tk_GetRootCoords(tkwin, &rootX, &rootY);
                    eventPtr->xkey.x_root = rootX + number;
                }
            } else if (flags & EXPOSE) {
                eventPtr->xexpose.x = number;
            } else if (flags & (CREATE|CONFIG|GRAVITY)) { 
                eventPtr->xcreatewindow.x = number;
            } else if (flags & REPARENT) {              
                eventPtr->xreparent.x = number;
            } else {
                goto badopt;
            }
            break;
        }
        case EVENT_Y: {
            if (Tk_GetPixelsFromObj(interp, tkwin, valuePtr, &number)
                        != TCL_OK) {
                    return TCL_ERROR;
                }
                if (flags & (KEY_BUTTON_MOTION_VIRTUAL|CROSSING)) {
                    eventPtr->xkey.y = number;
                    /*
                     * Only modify rooty as well if it hasn't been changed.
                     */
                    if (eventPtr->xkey.y_root == -1) {
                        int rootX, rootY;

                        Tk_GetRootCoords(tkwin, &rootX, &rootY);
                        eventPtr->xkey.y_root = rootY + number;
                    }
                } else if (flags & EXPOSE) {
                    eventPtr->xexpose.y = number;
                } else if (flags & (CREATE|CONFIG|GRAVITY)) {
                    eventPtr->xcreatewindow.y = number;
                } else if (flags & REPARENT) {
                    eventPtr->xreparent.y = number;
                } else {
                    goto badopt;
                }
                break;
            }
        }
        continue;
        
        badopt:
        Tcl_AppendResult(interp, name, " event doesn't accept \"",
                Tcl_GetStringFromObj(optionPtr, NULL), "\" option", 
                (char *)NULL);
        return TCL_ERROR;
    }
    if (!XSendEvent(display, window, False, pat.eventType, eventPtr)) {
        Blt_Warn("synthethic event failed\n");
    }
    return TCL_OK;
}

int
Blt_SendEventCmdInitProc(Tcl_Interp *interp)
{
    static Blt_CmdSpec cmdSpec = { 
        "sendevent", SendEventCmd, 
    };
    return Blt_InitCmd(interp, "::blt", &cmdSpec);
}
