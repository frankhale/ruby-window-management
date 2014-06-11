/* 
 * A small extension to provide access to a subset of Xlib from Ruby 
 * Copyright (C) 2010 Frank Hale <frankhale@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Started: 28 January 2010 
 * Date: 30 May 2010
 */

#include "ruby_x11.h"

int screen;

/* Eventually we need to look at allowing for an XEvent to be passed to the functions
* that need it rather than having this yucky global XEvent that all the functions assume
* will be the only one ever needed. But again I'm not the best X programmer so there might
* only need to be one.
*/
XEvent event;

/* MACRO's for casting Ruby FIXNUM back to desired type */
#define DISPLAY(dpy) (Display*)(FIX2LONG(dpy))
#define FONT(font) (XFontStruct*)(FIX2LONG(font))
/*#define XFTFONT(xftfont) (XftFont*)(FIX2LONG(xftfont))
#define XFTCOLOR(xftcolor) (XftColor)(FIX2LONG(xftcolor))*/
#define _GC(gc) (GC)(FIX2LONG(gc))

fd_set fd;
struct timeval tv;
int xfd;

static VALUE rb_mWindowManagement;
static VALUE rb_cX11;

static VALUE
ruby_X11_init(VALUE self)
{
  /* We don't need to do anything to set the X11 class up... */
  
  return self;
}

static VALUE
ruby_opendisplay(VALUE self, VALUE display_str)
{
  Display *dpy = NULL;
  Check_Type(display_str, T_STRING);
  
  char* dpy_string = StringValuePtr(display_str);
  
  if(RSTRING_LEN(StringValue(display_str))>0)
    //if(RSTRING(StringValue(display_str))->len>0)
  dpy = XOpenDisplay(dpy_string);
  else
    dpy = XOpenDisplay(NULL);
  
  if(!dpy)
  {
    return Qfalse;
  }
  else 
  {
    screen=DefaultScreen(dpy);
    xfd = ConnectionNumber (dpy);
    
    XSetErrorHandler(handleXError);
    
    return LONG2FIX(dpy);
  }
}

static VALUE
ruby_closedisplay(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  XCloseDisplay(DISPLAY(dpy));
  
  return self;    
}

static VALUE
ruby_getrootwindow(VALUE self, VALUE dpy, VALUE screen)
{
  Window root_window;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(screen, T_FIXNUM);
  
  root_window = RootWindow(DISPLAY(dpy), FIX2LONG(screen));
  
  return LONG2FIX(root_window);
}

static VALUE 
ruby_querytree(VALUE self, VALUE dpy, VALUE root)
{
  unsigned int nwins, i;
  Window dummyw1, dummyw2, *wins;
  VALUE window_list;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(root, T_FIXNUM);
  
  window_list = rb_ary_new();
  
  XQueryTree(DISPLAY(dpy), FIX2LONG(root), &dummyw1, &dummyw2, &wins, &nwins);
  for (i = 0; i < nwins; i++) 
  {
    rb_ary_push(window_list, LONG2FIX(wins[i]));
  }
  XFree(wins);
  
  return window_list;     
}

static VALUE 
ruby_createfontcursor(VALUE self, VALUE dpy, VALUE cursor)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(cursor, T_FIXNUM);
  
  Cursor c = XCreateFontCursor(DISPLAY(dpy), FIX2LONG(cursor));
  
  return LONG2FIX(c);
}

static VALUE 
ruby_freecursor(VALUE self, VALUE dpy, VALUE cursor)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(cursor, T_FIXNUM);
  
  XFreeCursor(DISPLAY(dpy), FIX2LONG(cursor));
  
  return self;
}

static VALUE ruby_passdownconfigurerequest(VALUE self, VALUE dpy)
{
  XWindowChanges wc;      
  
  /* This is used to pass back the configure request to X for handling
  *
  * Xterm used (maybe it still does) to require that it get a configure
  * request before it was mapped so that it'll have the correct size. If
  * not it would be 1 pixel wide by 1 pixel tall.
  */
  
  Check_Type(dpy, T_FIXNUM);
  
  wc.x = event.xconfigurerequest.x;
  wc.y = event.xconfigurerequest.y;
  wc.width = event.xconfigurerequest.width;
  wc.height = event.xconfigurerequest.height;
  wc.sibling = event.xconfigurerequest.above;
  wc.stack_mode = event.xconfigurerequest.detail;
  
  XConfigureWindow(DISPLAY(dpy), event.xconfigurerequest.window, event.xconfigurerequest.value_mask, &wc);
  
  return self;
}

static VALUE 
ruby_definecursor(VALUE self, VALUE dpy, VALUE win, VALUE cursor)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(cursor, T_FIXNUM);
  
  XDefineCursor(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(cursor));
  
  return self;
}

static VALUE 
ruby_mapraised(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XMapRaised(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_drawline(VALUE self, VALUE win, VALUE dpy, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(x1, T_FIXNUM);
  Check_Type(y1, T_FIXNUM);
  Check_Type(x2, T_FIXNUM);
  Check_Type(y2, T_FIXNUM);
  
  /* We are cutting a corner here in that we are assuming we only want to use the Default GC
  * this needs to be changed later to allow it to be more flexible.
  */
  
  XDrawLine(DISPLAY(dpy), FIX2LONG(win), DefaultGC(dpy, screen), FIX2LONG(x1), FIX2LONG(y1), FIX2LONG(x2), FIX2LONG(y2));
  
  return self;
}

static VALUE 
ruby_setwindowborderwidth(VALUE self, VALUE dpy, VALUE win, VALUE width)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(width, T_FIXNUM);
  
  XSetWindowBorderWidth(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(width));
  
  return self;
}

static VALUE 
ruby_grabserver(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  XGrabServer(DISPLAY(dpy));
  
  return self;
}

static VALUE 
ruby_ungrabserver(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  XUngrabServer(DISPLAY(dpy));
  
  return self;
}

static VALUE 
ruby_sendconfigurenotify(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y, VALUE width, VALUE height)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(x, T_FIXNUM);
  Check_Type(y, T_FIXNUM);
  Check_Type(width, T_FIXNUM);
  Check_Type(height, T_FIXNUM);
  
  XConfigureEvent ce;
  
  Window _win = FIX2LONG(win);
  
  ce.type = ConfigureNotify;
  ce.event = _win;
  ce.window = _win;
  ce.x = FIX2LONG(x);
  ce.y = FIX2LONG(y);
  ce.width = FIX2LONG(width);
  ce.height = FIX2LONG(height);
  ce.border_width = 0;
  ce.above = None;
  ce.override_redirect = 0;
  
  XSendEvent(DISPLAY(dpy), _win, False, StructureNotifyMask, (XEvent *)&ce);
  
  return self;    
}

static VALUE 
ruby_storename(VALUE self, VALUE dpy, VALUE win, VALUE name)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(name, T_STRING);
  
  XStoreName(DISPLAY(dpy), FIX2LONG(win), StringValuePtr(name));
  
  return self;
}

static VALUE 
ruby_eventspending(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  return LONG2FIX(XPending(DISPLAY(dpy)));
}

static VALUE 
ruby_destroywindow(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XDestroyWindow(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_destroysubwindow(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XDestroySubwindows(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_createsimplewindow(VALUE self, VALUE dpy, VALUE parent, VALUE x, VALUE y, VALUE width, VALUE height, VALUE border_width)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(parent, T_FIXNUM);
  Check_Type(x, T_FIXNUM);
  Check_Type(y, T_FIXNUM);
  Check_Type(width, T_FIXNUM);
  Check_Type(height, T_FIXNUM);
  Check_Type(border_width, T_FIXNUM);
  
  Window win = XCreateSimpleWindow(
    DISPLAY(dpy), 
    FIX2LONG(parent), 
    FIX2LONG(x), 
    FIX2LONG(y), 
    FIX2LONG(width), 
    FIX2LONG(height), 
    FIX2LONG(border_width), 
    BlackPixel(DISPLAY(dpy), screen), 
    WhitePixel(DISPLAY(dpy), screen));      
  
  return LONG2FIX(win);
}

static VALUE 
ruby_clearwindow(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XClearWindow(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_mapwindow(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XMapWindow(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_mapsubwindows(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XMapSubwindows(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_unmapwindow(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XUnmapWindow(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_unmapsubwindows(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XUnmapSubwindows(DISPLAY(dpy), FIX2LONG(win));  
  
  return self;
}

static VALUE 
ruby_sync(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  XSync(DISPLAY(dpy), False);
  
  return self;
}

static VALUE 
ruby_nextevent(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  XNextEvent(DISPLAY(dpy), &event);
  
  return self;
}

static VALUE 
ruby_eventtype(VALUE self)
{
  return LONG2FIX(event.type);
}

static VALUE 
ruby_flush(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  XFlush(DISPLAY(dpy));
  
  return self;    
}

static VALUE 
ruby_setoverrideredirecttrue(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XSetWindowAttributes sattr;
  
  sattr.override_redirect=True;
  
  XChangeWindowAttributes(DISPLAY(dpy), FIX2LONG(win), CWOverrideRedirect, &sattr);
  
  return self;    
}

static VALUE 
ruby_setwindoweventmask(VALUE self, VALUE dpy, VALUE win, VALUE mask)
{
  XSetWindowAttributes sattr;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(mask, T_FIXNUM);     
  
  sattr.event_mask=FIX2LONG(mask);
  
  XChangeWindowAttributes(DISPLAY(dpy), FIX2LONG(win), CWEventMask, &sattr);      
  
  return self;
}

static VALUE 
ruby_resizewindow(VALUE self, VALUE dpy, VALUE win, VALUE width, VALUE height)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(width, T_FIXNUM);
  Check_Type(height, T_FIXNUM);
  
  XResizeWindow(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(width), FIX2LONG(height));
  
  return self;
}

static VALUE 
ruby_moveresizewindow(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y, VALUE width, VALUE height)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);      
  Check_Type(x, T_FIXNUM);
  Check_Type(y, T_FIXNUM);
  Check_Type(width, T_FIXNUM);
  Check_Type(height, T_FIXNUM);
  
  XMoveResizeWindow(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(x), FIX2LONG(y), FIX2LONG(width), FIX2LONG(height));
  
  return self;
}

static VALUE 
ruby_reparentwindow(VALUE self, VALUE dpy, VALUE win, VALUE parent, VALUE x, VALUE y)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(parent, T_FIXNUM);
  Check_Type(x, T_FIXNUM);
  Check_Type(y, T_FIXNUM);
  
  XReparentWindow(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(parent), FIX2LONG(x), FIX2LONG(y));
  
  return self;    
}

static VALUE 
ruby_getkeysym(VALUE self)
{
  return LONG2FIX(XLookupKeysym(&event.xkey, 0));
}

static VALUE 
ruby_geteventtype(VALUE self)
{
  return (LONG2FIX(event.type));  
}

static VALUE 
ruby_drawstring(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y, VALUE str)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);      
  Check_Type(x, T_FIXNUM);
  Check_Type(y, T_FIXNUM);
  Check_Type(str, T_STRING);
  
  char* cstr =  StringValuePtr(str);
  
  /* Assuming DefaultGC is bad and needs to change */
  
  XDrawString(DISPLAY(dpy), FIX2LONG(win), DefaultGC(DISPLAY(dpy), screen), FIX2LONG(x), FIX2LONG(y), cstr,  RSTRING_LEN(StringValue(str)));
  
  return self;
}

static VALUE 
ruby_interatom(VALUE self, VALUE dpy, VALUE atom_name, VALUE only_if_exists)
{
  Atom atom=None;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(atom_name, T_STRING);
  Check_Type(only_if_exists, T_FIXNUM);
  
  atom = XInternAtom(DISPLAY(dpy), StringValuePtr(atom_name), FIX2LONG(only_if_exists));
  
  return LONG2FIX(atom);
}

static VALUE 
ruby_selectinput(VALUE self, VALUE dpy, VALUE win, VALUE mask)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(mask, T_FIXNUM);             
  
  XSelectInput(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(mask));
  
  return self;
}

static VALUE
ruby_loadxftfont(VALUE self, VALUE dpy, VALUE font_str)
{
  VALUE xftfont_hash;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(font_str, T_STRING);
  
  XftFont *xftfont = XftFontOpenXlfd(DISPLAY(dpy), DefaultScreen(dpy), StringValuePtr(font_str));
  
  if(!xftfont) return Qfalse;
  
  xftfont_hash = rb_hash_new();
  
  /*rb_hash_aset(font_hash,  rb_str_new2("pointer"), LONG2FIX(font));
  rb_hash_aset(font_hash,  rb_str_new2("fid"), LONG2FIX(font->fid));*/
  
  return xftfont_hash;
}

static VALUE 
ruby_loadfont(VALUE self, VALUE dpy, VALUE font_str)
{
  VALUE font_hash;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(font_str, T_STRING);
  
  XFontStruct* font = XLoadQueryFont(DISPLAY(dpy), StringValuePtr(font_str));
  
  if(!font) return Qfalse;
  
  font_hash = rb_hash_new();
  
  rb_hash_aset(font_hash,  rb_str_new2("pointer"), LONG2FIX(font));
  rb_hash_aset(font_hash,  rb_str_new2("fid"), LONG2FIX(font->fid));
  
  return font_hash;
}

static VALUE 
ruby_freefont(VALUE self, VALUE dpy, VALUE font)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(font, T_HASH);       
  
  VALUE font_pointer = rb_hash_aref(font, rb_str_new2("pointer"));
  
  XFreeFont(DISPLAY(dpy), FONT(font_pointer));
  
  return self;
}

static VALUE 
ruby_xres(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  return LONG2FIX(WidthOfScreen(ScreenOfDisplay(DISPLAY(dpy), screen)));
}

static VALUE 
ruby_yres(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  return LONG2FIX(HeightOfScreen(ScreenOfDisplay(DISPLAY(dpy), screen)));
}

static VALUE 
ruby_fetchname(VALUE self, VALUE dpy, VALUE win)
{
  VALUE r_name;
  char* name;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XFetchName(DISPLAY(dpy), FIX2LONG(win), &name);
  
  if(name!=NULL)
    r_name = rb_str_new2(name);
  else
    r_name = rb_str_new2("unknown name");
  
  XFree(name);
  
  return r_name;
}

static VALUE 
ruby_raisewindow(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XRaiseWindow(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_lowerwindow(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XLowerWindow(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_allowevents(VALUE self, VALUE dpy, VALUE mode)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(mode, T_FIXNUM);
  
  XAllowEvents(DISPLAY(dpy), FIX2LONG(mode), CurrentTime);
  
  return self;
}

static VALUE 
ruby_gettransientforhint(VALUE self, VALUE dpy, VALUE win)
{
  Window trans;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XGetTransientForHint(DISPLAY(dpy), FIX2LONG(win), &trans);
  
  return self;
}

static VALUE 
ruby_connectionnumber(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  return LONG2FIX(ConnectionNumber(DISPLAY(dpy)));
}

static VALUE
ruby_doselectonxfiledescriptor(VALUE self, VALUE seconds)
{
  Check_Type(seconds, T_FIXNUM);
  
  tv.tv_usec = 0;
  tv.tv_sec = FIX2LONG(seconds);
  FD_ZERO (&fd);
  FD_SET (xfd, &fd);
  
  if (select (xfd + 1, &fd, 0, 0, &tv) == 0)
    return Qtrue;
  else
    return Qfalse;
}

static VALUE 
ruby_getbuttonevent(VALUE self)
{
  VALUE buttonpressevent_hash;
  
  buttonpressevent_hash = rb_hash_new();
  
  rb_hash_aset(buttonpressevent_hash,  rb_str_new2("type"), LONG2FIX(event.xbutton.type));
  rb_hash_aset(buttonpressevent_hash,  rb_str_new2("window"), LONG2FIX(event.xbutton.window));
  rb_hash_aset(buttonpressevent_hash,  rb_str_new2("root"), LONG2FIX(event.xbutton.root));
  rb_hash_aset(buttonpressevent_hash,  rb_str_new2("subwindow"), LONG2FIX(event.xbutton.subwindow));
  rb_hash_aset(buttonpressevent_hash,  rb_str_new2("x"), LONG2FIX(event.xbutton.x));
  rb_hash_aset(buttonpressevent_hash,  rb_str_new2("y"), LONG2FIX(event.xbutton.y));
  rb_hash_aset(buttonpressevent_hash,  rb_str_new2("x_root"), LONG2FIX(event.xbutton.x_root));
  rb_hash_aset(buttonpressevent_hash,  rb_str_new2("y_root"), LONG2FIX(event.xbutton.y_root));
  rb_hash_aset(buttonpressevent_hash,  rb_str_new2("button"), LONG2FIX(event.xbutton.button));
  
  return buttonpressevent_hash;
}

static VALUE 
ruby_getcrossingevent(VALUE self)
{
  VALUE xcrossingevent_hash;
  
  xcrossingevent_hash = rb_hash_new();
  
  rb_hash_aset(xcrossingevent_hash,  rb_str_new2("type"), LONG2FIX(event.xcrossing.type));
  rb_hash_aset(xcrossingevent_hash,  rb_str_new2("window"), LONG2FIX(event.xcrossing.window));
  rb_hash_aset(xcrossingevent_hash,  rb_str_new2("root"), LONG2FIX(event.xcrossing.root));
  rb_hash_aset(xcrossingevent_hash,  rb_str_new2("subwindow"), LONG2FIX(event.xcrossing.subwindow));
  rb_hash_aset(xcrossingevent_hash,  rb_str_new2("x"), LONG2FIX(event.xcrossing.x));
  rb_hash_aset(xcrossingevent_hash,  rb_str_new2("y"), LONG2FIX(event.xcrossing.y));
  rb_hash_aset(xcrossingevent_hash,  rb_str_new2("x_root"), LONG2FIX(event.xcrossing.x_root));
  rb_hash_aset(xcrossingevent_hash,  rb_str_new2("y_root"), LONG2FIX(event.xcrossing.y_root));
  
  return xcrossingevent_hash;     
}

static VALUE 
ruby_getmaprequestevent(VALUE self)
{
  VALUE xmaprequest_hash;
  
  xmaprequest_hash = rb_hash_new();
  
  rb_hash_aset(xmaprequest_hash,  rb_str_new2("type"), LONG2FIX(event.xmaprequest.type));
  rb_hash_aset(xmaprequest_hash,  rb_str_new2("parent"), LONG2FIX(event.xmaprequest.parent));
  rb_hash_aset(xmaprequest_hash,  rb_str_new2("window"), LONG2FIX(event.xmaprequest.window));
  
  return xmaprequest_hash;
}

static VALUE 
ruby_getkeyevent(VALUE self)
{
  VALUE xkey_hash;
  
  xkey_hash = rb_hash_new();
  
  rb_hash_aset(xkey_hash,  rb_str_new2("window"), LONG2FIX(event.xkey.window));
  rb_hash_aset(xkey_hash,  rb_str_new2("root"), LONG2FIX(event.xkey.root));
  rb_hash_aset(xkey_hash,  rb_str_new2("subwindow"), LONG2FIX(event.xkey.subwindow));
  rb_hash_aset(xkey_hash,  rb_str_new2("x"), LONG2FIX(event.xkey.x));
  rb_hash_aset(xkey_hash,  rb_str_new2("y"), LONG2FIX(event.xkey.y));
  rb_hash_aset(xkey_hash,  rb_str_new2("x_root"), LONG2FIX(event.xkey.x_root));
  rb_hash_aset(xkey_hash,  rb_str_new2("y_root"), LONG2FIX(event.xkey.y_root));
  
  return xkey_hash;       
}

static VALUE 
ruby_getmotionevent(VALUE self)
{
  VALUE xmotion_hash;
  
  xmotion_hash = rb_hash_new();
  
  rb_hash_aset(xmotion_hash,  rb_str_new2("window"), LONG2FIX(event.xmotion.window));
  rb_hash_aset(xmotion_hash,  rb_str_new2("root"), LONG2FIX(event.xmotion.root));
  rb_hash_aset(xmotion_hash,  rb_str_new2("subwindow"), LONG2FIX(event.xmotion.subwindow));
  rb_hash_aset(xmotion_hash,  rb_str_new2("x"), LONG2FIX(event.xmotion.x));
  rb_hash_aset(xmotion_hash,  rb_str_new2("y"), LONG2FIX(event.xmotion.y));
  rb_hash_aset(xmotion_hash,  rb_str_new2("x_root"), LONG2FIX(event.xmotion.x_root));
  rb_hash_aset(xmotion_hash,  rb_str_new2("y_root"), LONG2FIX(event.xmotion.y_root));
  
  return xmotion_hash;    
}

static VALUE 
ruby_getconfigurerequestevent(VALUE self)
{
  VALUE xconfigurerequest_hash;
  
  xconfigurerequest_hash = rb_hash_new();
  
  rb_hash_aset(xconfigurerequest_hash,  rb_str_new2("parent"), LONG2FIX(event.xconfigurerequest.parent));
  rb_hash_aset(xconfigurerequest_hash,  rb_str_new2("window"), LONG2FIX(event.xconfigurerequest.window));
  rb_hash_aset(xconfigurerequest_hash,  rb_str_new2("x"), LONG2FIX(event.xconfigurerequest.x));
  rb_hash_aset(xconfigurerequest_hash,  rb_str_new2("y"), LONG2FIX(event.xconfigurerequest.y));
  rb_hash_aset(xconfigurerequest_hash,  rb_str_new2("width"), LONG2FIX(event.xconfigurerequest.width));
  rb_hash_aset(xconfigurerequest_hash,  rb_str_new2("height"), LONG2FIX(event.xconfigurerequest.height));
  rb_hash_aset(xconfigurerequest_hash,  rb_str_new2("border_width"), LONG2FIX(event.xconfigurerequest.border_width));
  rb_hash_aset(xconfigurerequest_hash,  rb_str_new2("above"), LONG2FIX(event.xconfigurerequest.above));   
  rb_hash_aset(xconfigurerequest_hash,  rb_str_new2("detail"), LONG2FIX(event.xconfigurerequest.detail));
  rb_hash_aset(xconfigurerequest_hash,  rb_str_new2("value_mask"), LONG2FIX(event.xconfigurerequest.value_mask)); 
  
  return xconfigurerequest_hash;  
}

static VALUE 
ruby_setinputfocus(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XSetInputFocus(DISPLAY(dpy), FIX2LONG(win), RevertToPointerRoot, CurrentTime);
  
  return self;
}

static VALUE 
ruby_grabpointer(VALUE self, VALUE dpy, VALUE win, VALUE mask)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(mask, T_FIXNUM);
  
  /* Again we are cheating here because we are assuming that we only wanna pass a window and a mask
  * This should be changed to allow the different grab modes and other information if it's needed later!
  */
  
  XGrabPointer(DISPLAY(dpy), FIX2LONG(win), True, FIX2LONG(mask), GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
  
  return self;
}

static VALUE 
ruby_ungrabpointer(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  XUngrabPointer(DISPLAY(dpy), CurrentTime);
  
  return self;
}

static VALUE 
ruby_grabkey(VALUE self, VALUE dpy, VALUE key, VALUE modifiers, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);      
  Check_Type(key, T_STRING);
  Check_Type(modifiers, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XGrabKey(DISPLAY(dpy), XKeysymToKeycode(DISPLAY(dpy), XStringToKeysym(StringValuePtr(key))), FIX2LONG(modifiers), FIX2LONG(win),
    True, GrabModeAsync, GrabModeAsync);
  
  return self;
}

static VALUE 
ruby_ungrabkey(VALUE self, VALUE dpy, VALUE key, VALUE modifiers, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(key, T_STRING);
  Check_Type(modifiers, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XUngrabKey(DISPLAY(dpy), XKeysymToKeycode(DISPLAY(dpy), XStringToKeysym(StringValuePtr(key))), FIX2LONG(modifiers), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_grabbutton(VALUE self, VALUE dpy, VALUE button, VALUE modifiers, VALUE win, VALUE mask)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(button, T_FIXNUM);
  Check_Type(modifiers, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(mask, T_FIXNUM);
  
  XGrabButton(DISPLAY(dpy), FIX2LONG(button), FIX2LONG(modifiers), FIX2LONG(win), True, FIX2LONG(mask), GrabModeAsync,
    GrabModeAsync, None, None);
  
  return self;
}

static VALUE 
ruby_ungrabbutton(VALUE self, VALUE dpy, VALUE button, VALUE modifiers, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(button, T_FIXNUM);
  Check_Type(modifiers, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XUngrabButton(DISPLAY(dpy), FIX2LONG(button), FIX2LONG(modifiers), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_getwindowattributes(VALUE self, VALUE dpy, VALUE win)
{
  XWindowAttributes attr;
  VALUE attr_hash;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XGetWindowAttributes(DISPLAY(dpy), FIX2LONG(win), &attr);
  
  attr_hash = rb_hash_new();
  
  rb_hash_aset(attr_hash, rb_str_new2("x"), LONG2FIX(attr.x));
  rb_hash_aset(attr_hash, rb_str_new2("y"), LONG2FIX(attr.y));
  rb_hash_aset(attr_hash, rb_str_new2("width"), LONG2FIX(attr.width));
  rb_hash_aset(attr_hash, rb_str_new2("height"), LONG2FIX(attr.height));
  rb_hash_aset(attr_hash, rb_str_new2("class"), LONG2FIX(attr.class));
  rb_hash_aset(attr_hash, rb_str_new2("border_width"), LONG2FIX(attr.border_width));
  rb_hash_aset(attr_hash, rb_str_new2("colormap"), LONG2FIX(attr.colormap));
  rb_hash_aset(attr_hash, rb_str_new2("map_state"), LONG2FIX(attr.map_state));
  rb_hash_aset(attr_hash, rb_str_new2("override_redirect"), ( (attr.override_redirect) ? Qtrue : Qfalse  ) );
  rb_hash_aset(attr_hash, rb_str_new2("bit_gravity"), LONG2FIX(attr.bit_gravity));
  rb_hash_aset(attr_hash, rb_str_new2("win_gravity"), LONG2FIX(attr.win_gravity));        
  rb_hash_aset(attr_hash, rb_str_new2("map_state"), LONG2FIX(attr.map_state));
  
  return attr_hash;
}

static VALUE 
ruby_movewindow(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(x, T_FIXNUM);
  Check_Type(y, T_FIXNUM);
  
  XMoveWindow(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(x), FIX2LONG(y));
  
  return self;
}

static VALUE 
ruby_remove_from_saveset(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XRemoveFromSaveSet(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_killclient(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XKillClient(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_textextents(VALUE self, VALUE dpy, VALUE font, VALUE str)
{
  /* This function probably isn't finished, there is more information associated with XTextExtents
  * For now we pass back some useful information in a nice Ruby hash! =) 
  */
  
  VALUE extent_info_hash;
  int direction = 0;
  int charcount = 0;
  int font_ascent = 0;
  int font_descent = 0;
  XCharStruct overall;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(font, T_FIXNUM);
  Check_Type(str, T_STRING);
  
  charcount = RSTRING_LEN(StringValue(str));
  
  XTextExtents(
    FONT(font), 
    StringValuePtr(str), 
    charcount, 
    &direction, 
    &font_ascent, 
    &font_descent, 
    &overall
    );
  
  extent_info_hash = rb_hash_new();
  
  rb_hash_aset(extent_info_hash,  rb_str_new2("font_ascent"), LONG2FIX(font_ascent));
  rb_hash_aset(extent_info_hash,  rb_str_new2("font_descent"), LONG2FIX(font_descent));
  rb_hash_aset(extent_info_hash,  rb_str_new2("overall_ascent"), LONG2FIX(overall.ascent));
  rb_hash_aset(extent_info_hash,  rb_str_new2("overall_descent"), LONG2FIX(overall.descent));
  rb_hash_aset(extent_info_hash,  rb_str_new2("overall_width"), LONG2FIX(overall.width));
  
  return extent_info_hash;
}

static VALUE 
ruby_drawrectangle(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y, VALUE width, VALUE height)
{
  /* For now we will use the DefaultGC but it will be changed later */
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(x, T_FIXNUM);
  Check_Type(y, T_FIXNUM);
  Check_Type(width, T_FIXNUM);
  Check_Type(height, T_FIXNUM);
  
  /* DefaultGC will be changed later to allow for any GC */
  XDrawRectangle(DISPLAY(dpy), FIX2LONG(win), DefaultGC(dpy,screen), FIX2LONG(x),FIX2LONG(y),FIX2LONG(width),FIX2LONG(height));
  
  return self;
}

static VALUE ruby_fillrectangle(VALUE self, VALUE dpy, VALUE win, VALUE gc, VALUE x, VALUE y, VALUE width, VALUE height)
{
  /* For now we will use the DefaultGC but it will be changed later */
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(gc, T_FIXNUM);
  Check_Type(x, T_FIXNUM);
  Check_Type(y, T_FIXNUM);
  Check_Type(width, T_FIXNUM);
  Check_Type(height, T_FIXNUM);
  
  /* DefaultGC will be changed later to allow for any GC */
  XFillRectangle(DISPLAY(dpy), FIX2LONG(win), _GC(gc), FIX2LONG(x),FIX2LONG(y),FIX2LONG(width),FIX2LONG(height));
  
  return self;
}

static VALUE ruby_querypointer(VALUE self, VALUE dpy, VALUE win)
{
  VALUE mouse_position_hash;
  Window root_return=None;
  Window child_return=None;
  int root_x=0;
  int root_y=0;
  int win_x=0;
  int win_y=0;
  unsigned int mask_return=0;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  mouse_position_hash = rb_hash_new();
  
  XQueryPointer(DISPLAY(dpy), FIX2LONG(win), &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask_return);
  
  rb_hash_aset(mouse_position_hash,  rb_str_new2("win"), LONG2FIX(child_return));
  rb_hash_aset(mouse_position_hash,  rb_str_new2("root"), LONG2FIX(root_return));
  rb_hash_aset(mouse_position_hash,  rb_str_new2("root_x"), LONG2FIX(root_x));
  rb_hash_aset(mouse_position_hash,  rb_str_new2("root_y"), LONG2FIX(root_y));
  rb_hash_aset(mouse_position_hash,  rb_str_new2("win_x"), LONG2FIX(win_x));
  rb_hash_aset(mouse_position_hash,  rb_str_new2("win_y"), LONG2FIX(win_y));
  rb_hash_aset(mouse_position_hash,  rb_str_new2("mask"), LONG2FIX(mask_return));         
  
  return mouse_position_hash;
}

static VALUE 
ruby_getunmapnotifyevent(VALUE self)
{
  VALUE unmaprequestevent_hash;
  
  unmaprequestevent_hash = rb_hash_new();
  
  rb_hash_aset(unmaprequestevent_hash,  rb_str_new2("send_event"), ( (event.xunmap.send_event) ? Qtrue : Qfalse ) );
  rb_hash_aset(unmaprequestevent_hash,  rb_str_new2("event"), LONG2FIX(event.xunmap.event) );
  rb_hash_aset(unmaprequestevent_hash,  rb_str_new2("window"), LONG2FIX(event.xunmap.window) );
  rb_hash_aset(unmaprequestevent_hash,  rb_str_new2("from_configure"), ( (event.xunmap.from_configure) ? Qtrue : Qfalse ) );
  
  return unmaprequestevent_hash;
}

static VALUE 
ruby_geterrorevent(VALUE self)
{
  VALUE errorevent_hash;
  
  errorevent_hash = rb_hash_new();
  
  rb_hash_aset(errorevent_hash,  rb_str_new2("serial"), LONG2FIX(event.xerror.serial) );
  rb_hash_aset(errorevent_hash,  rb_str_new2("error_code"), LONG2FIX(event.xerror.error_code) );
  rb_hash_aset(errorevent_hash,  rb_str_new2("request_code"), LONG2FIX(event.xerror.request_code) );
  rb_hash_aset(errorevent_hash,  rb_str_new2("minor_code"), LONG2FIX(event.xerror.minor_code) );
  rb_hash_aset(errorevent_hash,  rb_str_new2("resourceid"), LONG2FIX(event.xerror.resourceid) );
  
  return errorevent_hash; 
}

static VALUE 
ruby_setwmstate(VALUE self, VALUE dpy, VALUE win, VALUE wm_state_atom, VALUE wm_state)
{
  CARD32 data[2];
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(wm_state_atom, T_FIXNUM);
  Check_Type(wm_state, T_FIXNUM);
  
  data[0] = wm_state;
  data[1] = None; 
  
  XChangeProperty(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(wm_state_atom), FIX2LONG(wm_state),
    32, PropModeReplace, (unsigned char *)data, 2);
  
  return self;
}

static VALUE 
ruby_getwmstate(VALUE self, VALUE dpy, VALUE win, VALUE wm_state_atom)
{
  Atom real_type; 
  int real_format;
  long state = WithdrawnState;
  unsigned long items_read, bytes_left;
  unsigned char *data;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(wm_state_atom, T_FIXNUM);
  
  if (XGetWindowProperty(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(wm_state_atom), 0L, 2L, False,
    FIX2LONG(wm_state_atom), &real_type, &real_format, &items_read, &bytes_left,
  &data) == Success && items_read) 
  {
    state = *(long *)data;
    XFree(data);
  }
  
  return LONG2FIX(state);
}

static VALUE ruby_installcolormap(VALUE self, VALUE dpy, VALUE colormap)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(colormap, T_FIXNUM);
  
  XInstallColormap(DISPLAY(dpy), (Colormap) FIX2LONG(colormap));
  
  return self;
}

static VALUE 
ruby_addtosaveset(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XAddToSaveSet(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_allocnamedcolor(VALUE self, VALUE dpy, VALUE colstr)
{
  XColor col, dummy;
  
  VALUE color_hash;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(colstr, T_STRING);
  
  color_hash = rb_hash_new();
  
  XAllocNamedColor(DISPLAY(dpy), DefaultColormap(DISPLAY(dpy), DefaultScreen(DISPLAY(dpy))), StringValuePtr(colstr), &col, &dummy);
  
  /*rb_hash_aset(color_hash,  rb_str_new2("pointer"), INT2NUM(col));*/
  rb_hash_aset(color_hash,  rb_str_new2("pixel"), LONG2FIX(col.pixel));
  
  return color_hash;
}

static VALUE 
ruby_getexposeevent(VALUE self)
{
  VALUE xexpose_hash;
  
  xexpose_hash = rb_hash_new();
  
  rb_hash_aset(xexpose_hash,  rb_str_new2("window"), LONG2FIX(event.xexpose.window));
  rb_hash_aset(xexpose_hash,  rb_str_new2("x"), LONG2FIX(event.xexpose.x));
  rb_hash_aset(xexpose_hash,  rb_str_new2("y"), LONG2FIX(event.xexpose.y));
  rb_hash_aset(xexpose_hash,  rb_str_new2("width"), LONG2FIX(event.xexpose.width));
  rb_hash_aset(xexpose_hash,  rb_str_new2("height"), LONG2FIX(event.xexpose.height));
  rb_hash_aset(xexpose_hash,  rb_str_new2("count"), LONG2FIX(event.xexpose.count));
  
  return xexpose_hash;    
}

static VALUE 
ruby_getcolormapevent(VALUE self)
{
  VALUE xcolormap_hash;
  
  xcolormap_hash = rb_hash_new();
  
  rb_hash_aset(xcolormap_hash,  rb_str_new2("window"), LONG2FIX(event.xcolormap.window));
  rb_hash_aset(xcolormap_hash,  rb_str_new2("colormap"), LONG2FIX(event.xcolormap.colormap));
  rb_hash_aset(xcolormap_hash,  rb_str_new2("new"), ((event.xcolormap.new) ? Qtrue : Qfalse) );
  rb_hash_aset(xcolormap_hash,  rb_str_new2("state"), LONG2FIX(event.xcolormap.state));
  
  return xcolormap_hash;  
}

static VALUE 
ruby_getdestroywindowevent(VALUE self)
{
  VALUE xdestroywindow_hash;
  
  xdestroywindow_hash = rb_hash_new();
  
  rb_hash_aset(xdestroywindow_hash,  rb_str_new2("event"), LONG2FIX(event.xdestroywindow.event));
  rb_hash_aset(xdestroywindow_hash,  rb_str_new2("window"), LONG2FIX(event.xdestroywindow.window));
  
  return xdestroywindow_hash;     
}

static VALUE 
ruby_getclientmessageevent(VALUE self)
{
  VALUE xclient_hash;
  
  xclient_hash = rb_hash_new();
  
  rb_hash_aset(xclient_hash,  rb_str_new2("window"), LONG2FIX(event.xclient.window));
  rb_hash_aset(xclient_hash,  rb_str_new2("message_type"), LONG2FIX(event.xclient.message_type)); 
  rb_hash_aset(xclient_hash,  rb_str_new2("format"), LONG2FIX(event.xclient.format));
  
  return xclient_hash;
}

static VALUE 
ruby_getpropertyevent(VALUE self)
{
  VALUE xproperty_hash;
  
  xproperty_hash = rb_hash_new();
  
  rb_hash_aset(xproperty_hash,  rb_str_new2("window"), LONG2FIX(event.xproperty.window));
  rb_hash_aset(xproperty_hash,  rb_str_new2("atom"), LONG2FIX(event.xproperty.atom));     
  
  return xproperty_hash;
}

static VALUE 
ruby_getfocuschangeevent(VALUE self)
{       
  VALUE xfocus_hash;
  
  xfocus_hash = rb_hash_new();
  
  rb_hash_aset(xfocus_hash,  rb_str_new2("type"), LONG2FIX(event.xfocus.type));
  rb_hash_aset(xfocus_hash,  rb_str_new2("window"), LONG2FIX(event.xfocus.window));
  rb_hash_aset(xfocus_hash,  rb_str_new2("send_event"), ((event.xfocus.send_event) ? Qtrue : Qfalse ) );
  rb_hash_aset(xfocus_hash,  rb_str_new2("mode"), LONG2FIX(event.xfocus.mode));
  rb_hash_aset(xfocus_hash,  rb_str_new2("detail"), LONG2FIX(event.xfocus.detail));
  
  return xfocus_hash;
}

/* This function is *not* going to support the full range of XGCValues
* It's sole purpose is to create very basic GC's.
*/
static VALUE 
ruby_creategc(VALUE self, VALUE dpy, VALUE win, VALUE gcvalues)
{
  XGCValues gv;   
  GC new_gc;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(gcvalues, T_HASH);   
  
  VALUE function = rb_hash_aref(gcvalues, rb_str_new2("function"));
  VALUE foreground = rb_hash_aref(gcvalues, rb_str_new2("foreground"));
  VALUE background = rb_hash_aref(gcvalues, rb_str_new2("background"));
  VALUE font = rb_hash_aref(gcvalues, rb_str_new2("font"));
  VALUE line_width = rb_hash_aref(gcvalues, rb_str_new2("line_width"));
  VALUE subwindow_mode = rb_hash_aref(gcvalues, rb_str_new2("subwindow_mode"));
  VALUE mask = rb_hash_aref(gcvalues, rb_str_new2("mask"));
  
  if(function != Qnil)
    gv.function = FIX2LONG(function);
  
  if(foreground != Qnil)
    gv.foreground = FIX2LONG(foreground);
  
  if(background != Qnil)
    gv.background = FIX2LONG(background);
  
  if(font != Qnil)
    gv.font = FIX2LONG(font);
  
  if(line_width != Qnil)
    gv.line_width = FIX2LONG(line_width);
  
  if(subwindow_mode != Qnil)
    gv.subwindow_mode = FIX2LONG(subwindow_mode);
  
  if(mask == Qnil)
    mask = 0;       
  
  new_gc = XCreateGC(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(mask), &gv);
  
  return LONG2FIX(new_gc);
}

static VALUE 
ruby_freegc(VALUE self, VALUE dpy, VALUE gc)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(gc, T_FIXNUM);
  
  XFreeGC(DISPLAY(dpy), _GC(gc)); 
  
  return self;
}

static VALUE 
ruby_setwindowbackground(VALUE self, VALUE dpy, VALUE win,  VALUE color)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(color, T_FIXNUM);
  
  XSetWindowBackground(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(color));
  
  return self;
}

static VALUE 
ruby_getwmnormalhints(VALUE self, VALUE dpy, VALUE win)
{
  XSizeHints      *size;
  long            dummy;
  VALUE           size_hash;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  size = XAllocSizeHints();
  
  XGetWMNormalHints(DISPLAY(dpy), FIX2LONG(win), size, &dummy);
  
  size_hash = rb_hash_new();
  
  rb_hash_aset(size_hash,  rb_str_new2("flags"), LONG2FIX(size->flags));
  rb_hash_aset(size_hash,  rb_str_new2("x"), LONG2FIX(size->x));
  rb_hash_aset(size_hash,  rb_str_new2("y"), LONG2FIX(size->y));
  rb_hash_aset(size_hash,  rb_str_new2("width"), LONG2FIX(size->width));
  rb_hash_aset(size_hash,  rb_str_new2("height"), LONG2FIX(size->height));
  rb_hash_aset(size_hash,  rb_str_new2("min_width"), LONG2FIX(size->min_width));
  rb_hash_aset(size_hash,  rb_str_new2("min_height"), LONG2FIX(size->min_height));
  rb_hash_aset(size_hash,  rb_str_new2("max_width"), LONG2FIX(size->max_width));
  rb_hash_aset(size_hash,  rb_str_new2("max_height"), LONG2FIX(size->max_height));
  rb_hash_aset(size_hash,  rb_str_new2("width_inc"), LONG2FIX(size->width_inc));
  rb_hash_aset(size_hash,  rb_str_new2("height_inc"), LONG2FIX(size->height_inc));
  rb_hash_aset(size_hash,  rb_str_new2("min_aspect_x"), LONG2FIX(size->min_aspect.x));
  rb_hash_aset(size_hash,  rb_str_new2("min_aspect_y"), LONG2FIX(size->min_aspect.y));
  rb_hash_aset(size_hash,  rb_str_new2("max_aspect_x"), LONG2FIX(size->max_aspect.x));
  rb_hash_aset(size_hash,  rb_str_new2("max_aspect_y"), LONG2FIX(size->max_aspect.y));
  rb_hash_aset(size_hash,  rb_str_new2("base_width"), LONG2FIX(size->base_width));
  rb_hash_aset(size_hash,  rb_str_new2("base_height"), LONG2FIX(size->base_height));
  rb_hash_aset(size_hash,  rb_str_new2("win_gravity"), LONG2FIX(size->win_gravity));
  
  XFree(size);
  
  return size_hash;
}

static VALUE 
ruby_setwindowborder(VALUE self, VALUE dpy, VALUE win, VALUE border_pixel)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(border_pixel, T_FIXNUM);
  
  XSetWindowBorder(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(border_pixel));
  
  return self;
}

static VALUE 
ruby_warppointer(VALUE self, VALUE dpy, VALUE src_w, VALUE dest_w, 
  VALUE src_x, VALUE src_y, VALUE src_width, VALUE src_height, 
  VALUE dest_x, VALUE dest_y)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(src_w, T_FIXNUM);
  Check_Type(dest_w, T_FIXNUM);
  Check_Type(src_x, T_FIXNUM);
  Check_Type(src_y, T_FIXNUM);
  Check_Type(src_width, T_FIXNUM);
  Check_Type(src_height, T_FIXNUM);
  Check_Type(dest_x, T_FIXNUM);
  Check_Type(dest_y, T_FIXNUM);
  
  XWarpPointer(DISPLAY(dpy), FIX2LONG(src_w), FIX2LONG(dest_w), FIX2LONG(src_x),
    FIX2LONG(src_y), FIX2LONG(src_width), FIX2LONG(src_height), FIX2LONG(dest_x), FIX2LONG(dest_y));
  
  
  return self;
}

static VALUE 
ruby_circulatesubwindows(VALUE self, VALUE dpy, VALUE win, VALUE direction)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(direction, T_FIXNUM);
  
  XCirculateSubwindows(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(direction));
  
  return self;
}

static VALUE ruby_circulatesubwindowsup(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XCirculateSubwindowsUp(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE ruby_circulatesubwindowsdown(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XCirculateSubwindowsDown(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE ruby_restackwindows(VALUE self, VALUE dpy, VALUE windows)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(windows, T_ARRAY);
  
  XRestackWindows(DISPLAY(dpy), RARRAY_PTR(windows), RARRAY_LEN(windows));
  
  return self;
}

static VALUE 
ruby_shapequeryextension(VALUE self, VALUE dpy)
{
  int shape=-1;
  int shape_event=-1;
  int dummy;
  VALUE shape_query_hash;
  
  Check_Type(dpy, T_FIXNUM);
  
  shape = XShapeQueryExtension(DISPLAY(dpy), &shape_event, &dummy);       
  
  if(shape)
  {
    shape_query_hash = rb_hash_new();
    
    rb_hash_aset(shape_query_hash,  rb_str_new2("shape_event"), LONG2FIX(shape_event));
    
  } else
    shape_query_hash = Qnil;
  
  return shape_query_hash;
}

static VALUE 
ruby_shapequeryversion(VALUE self, VALUE dpy)
{
  int major, minor;
  VALUE shape_version_hash;
  
  Check_Type(dpy, T_FIXNUM);
  
  shape_version_hash = rb_hash_new();
  
  XShapeQueryVersion(DISPLAY(dpy), &major, &minor);
  
  rb_hash_aset(shape_version_hash,  rb_str_new2("major_version"), LONG2FIX(major));
  rb_hash_aset(shape_version_hash,  rb_str_new2("minor_version"), LONG2FIX(minor));
  
  return shape_version_hash;
}


static VALUE 
ruby_shapecombineregion(VALUE self, VALUE dpy, VALUE dest, VALUE dest_kind, VALUE x_off, VALUE y_off, VALUE region, VALUE op)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(dest, T_FIXNUM);
  Check_Type(dest_kind, T_FIXNUM);
  Check_Type(x_off, T_FIXNUM);
  Check_Type(y_off, T_FIXNUM);
  Check_Type(region, T_FIXNUM);
  Check_Type(op, T_FIXNUM);
  
  XShapeCombineRegion(DISPLAY(dpy), FIX2LONG(dest), FIX2LONG(dest_kind), FIX2LONG(x_off), FIX2LONG(y_off), (Region) FIX2LONG(region), FIX2LONG(op));
  
  return self;
}

static VALUE 
ruby_shapecombinerectangles(VALUE self, VALUE dpy, VALUE dest, VALUE dest_kind, 
  VALUE x_off, VALUE y_off, VALUE rectangles, VALUE op, VALUE ordering)
{
  XRectangle *rects;
  VALUE *rarray_ptr;
  int rarray_size=0, i=0;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(dest, T_FIXNUM);
  Check_Type(dest_kind, T_FIXNUM);
  Check_Type(x_off, T_FIXNUM);
  Check_Type(y_off, T_FIXNUM);
  Check_Type(rectangles, T_ARRAY);
  Check_Type(op, T_FIXNUM);
  Check_Type(ordering, T_FIXNUM);
  
  /* Convert the Ruby array of rects to a C array of rects */
  rarray_ptr = RARRAY_PTR(rectangles);
  rarray_size = RARRAY_LEN(rectangles);
  
  rects = (XRectangle*) malloc((rarray_size+1)*sizeof(XRectangle));
  
  for (i=0; i < rarray_size; i++, rarray_ptr++) {
    XRectangle actual_rect;
    VALUE rect_hash = (*rarray_ptr);
    int x=0, y=0, width=0, height=0;
    
    x = rb_hash_aref(rect_hash, rb_str_new2("x"));
    y = rb_hash_aref(rect_hash, rb_str_new2("y"));
    width = rb_hash_aref(rect_hash, rb_str_new2("width"));
    height = rb_hash_aref(rect_hash, rb_str_new2("height"));
    
    actual_rect.x = x;
    actual_rect.y = y;
    actual_rect.width = width;
    actual_rect.height = height;
    
    rects[i] = actual_rect;
  }
  
  XShapeCombineRectangles(DISPLAY(dpy), FIX2LONG(dest), FIX2LONG(dest_kind), FIX2LONG(x_off), FIX2LONG(y_off), rects, RARRAY_LEN(rectangles), FIX2LONG(op), FIX2LONG(ordering));
  
  XFree(rects);
  
  return self;
}

static VALUE 
ruby_shapecombinemask(VALUE self, VALUE dpy, VALUE dest, VALUE dest_kind, VALUE x_off, 
  VALUE y_off, VALUE src, VALUE op)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(dest, T_FIXNUM);
  Check_Type(dest_kind, T_FIXNUM);
  Check_Type(x_off, T_FIXNUM);
  Check_Type(y_off, T_FIXNUM);
  Check_Type(src, T_FIXNUM);
  Check_Type(op, T_FIXNUM);
  
  XShapeCombineMask(DISPLAY(dpy), FIX2LONG(dest), FIX2LONG(dest_kind), FIX2LONG(x_off), FIX2LONG(y_off), FIX2LONG(src), FIX2LONG(op));             
  
  return self;    
}

static VALUE ruby_shapecombineshape(VALUE self, VALUE dpy, VALUE dest, VALUE dest_kind, 
  VALUE x_off, VALUE y_off, VALUE src, VALUE src_kind, VALUE op)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(dest, T_FIXNUM);
  Check_Type(dest_kind, T_FIXNUM);
  Check_Type(x_off, T_FIXNUM);
  Check_Type(y_off, T_FIXNUM);
  Check_Type(src, T_FIXNUM);
  Check_Type(src_kind, T_FIXNUM);
  Check_Type(op, T_FIXNUM);
  
  XShapeCombineShape(DISPLAY(dpy), FIX2LONG(dest), FIX2LONG(dest_kind), FIX2LONG(x_off), FIX2LONG(y_off), FIX2LONG(src), FIX2LONG(src_kind), FIX2LONG(op));
  
  return self;
}

static VALUE 
ruby_shapeselectinput(VALUE self, VALUE dpy, VALUE win, VALUE mask)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(mask, T_FIXNUM);
  
  XShapeSelectInput(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(mask));
  
  return self;
}

static VALUE 
ruby_xpmreadfiletopixmap(VALUE self, VALUE dpy, VALUE drawable, VALUE filename)
{
  Pixmap pixmap, pixmap_mask;
  XpmAttributes pixmap_attr;
  
  VALUE pixmap_hash;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(drawable, T_FIXNUM);
  Check_Type(filename, T_STRING);
  
  pixmap_attr.valuemask = XpmSize;
  
  if(XpmReadFileToPixmap(DISPLAY(dpy), FIX2LONG(drawable), StringValuePtr(filename), &pixmap, &pixmap_mask, &pixmap_attr)==XpmSuccess)
  {
    pixmap_hash = rb_hash_new();
    
    rb_hash_aset(pixmap_hash,  rb_str_new2("pixmap"), LONG2FIX(pixmap));
    rb_hash_aset(pixmap_hash,  rb_str_new2("pixmap_mask"), LONG2FIX(pixmap_mask));
    rb_hash_aset(pixmap_hash,  rb_str_new2("width"), LONG2FIX(pixmap_attr.width));
    rb_hash_aset(pixmap_hash,  rb_str_new2("height"), LONG2FIX(pixmap_attr.height));
    
  } else
    pixmap_hash=Qnil;
  
  return pixmap_hash;
}

static VALUE 
ruby_freepixmap(VALUE self, VALUE dpy, VALUE pixmap)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(pixmap, T_FIXNUM);
  
  XFreePixmap(DISPLAY(dpy), FIX2LONG(pixmap));
  
  return self;
}

static VALUE 
ruby_setwindowbackgroundpixmap(VALUE self, VALUE dpy, VALUE win, VALUE pixmap)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(pixmap, T_FIXNUM);
  
  XSetWindowBackgroundPixmap(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(pixmap));
  
  return self;
}

static VALUE 
ruby_xpmcreatepixmapfromdata(VALUE self, VALUE dpy, VALUE drawable, VALUE data)
{
  Pixmap pixmap, pixmap_mask;
  XpmAttributes pixmap_attr;
  char** actual_data;
  int rarray_size;
  int i;
  
  VALUE pixmap_hash;
  VALUE *rarray_ptr;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(drawable, T_FIXNUM);
  Check_Type(data, T_ARRAY);
  
  pixmap_attr.valuemask = XpmSize;
  
  /* Convert Ruby array to char** */
  rarray_ptr = RARRAY_PTR(data);
  rarray_size = RARRAY_LEN(data);
  
  actual_data = (char **) malloc((rarray_size+1)*sizeof(char *));
  
  for (i=0; i < rarray_size; i++, rarray_ptr++) {
    actual_data[i]= StringValuePtr(*rarray_ptr); 
  }
  actual_data[i]=NULL; 
  /* End */
  
  if(XpmCreatePixmapFromData(DISPLAY(dpy), FIX2LONG(drawable), actual_data, &pixmap, &pixmap_mask, &pixmap_attr)==XpmSuccess)
  {
    pixmap_hash = rb_hash_new();
    
    rb_hash_aset(pixmap_hash,  rb_str_new2("pixmap"), LONG2FIX(pixmap));
    rb_hash_aset(pixmap_hash,  rb_str_new2("pixmap_mask"), LONG2FIX(pixmap_mask));
    rb_hash_aset(pixmap_hash,  rb_str_new2("width"), LONG2FIX(pixmap_attr.width));
    rb_hash_aset(pixmap_hash,  rb_str_new2("height"), LONG2FIX(pixmap_attr.height));
    
  } else {
    pixmap_hash = Qnil;
  }
  
  free(actual_data);
  
  return pixmap_hash;
}

static VALUE ruby_getshapeevent(VALUE self)
{
  VALUE shape_event_hash;
  
  XShapeEvent *xshape = (XShapeEvent*) &event;
  
  if(xshape) 
  {
    shape_event_hash = rb_hash_new();
    
    rb_hash_aset(shape_event_hash,  rb_str_new2("type"), LONG2FIX(xshape->type));
    rb_hash_aset(shape_event_hash,  rb_str_new2("send_event"), ((xshape->send_event) ? Qtrue : Qfalse) );
    rb_hash_aset(shape_event_hash,  rb_str_new2("window"), LONG2FIX(xshape->window));
    rb_hash_aset(shape_event_hash,  rb_str_new2("kind"), LONG2FIX(xshape->kind));
    rb_hash_aset(shape_event_hash,  rb_str_new2("x"), LONG2FIX(xshape->x));
    rb_hash_aset(shape_event_hash,  rb_str_new2("y"), LONG2FIX(xshape->y));
    rb_hash_aset(shape_event_hash,  rb_str_new2("width"), LONG2FIX(xshape->width));
    rb_hash_aset(shape_event_hash,  rb_str_new2("height"), LONG2FIX(xshape->height));
    rb_hash_aset(shape_event_hash,  rb_str_new2("shaped"), ( (xshape->shaped) ? Qtrue : Qfalse ) );
    
  } else
    shape_event_hash = Qnil;
  
  return shape_event_hash;
}

static VALUE ruby_shapequeryextents(VALUE self, VALUE dpy, VALUE win)
{
  int x_bounding=0, y_bounding=0;
  unsigned int w_bounding=0, h_bounding=0; 
  int x_clip=0, y_clip=0, bounding_shape=0, clip_shaped=0; 
  unsigned int w_clip=0, h_clip=0;
  
  VALUE shape_query_extents_hash;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  shape_query_extents_hash = rb_hash_new();
  
  XShapeQueryExtents(DISPLAY(dpy), FIX2LONG(win), &bounding_shape, &x_bounding, &y_bounding,
    &w_bounding, &h_bounding, &clip_shaped, &x_clip, &y_clip, &w_clip, &h_clip);
  
  rb_hash_aset(shape_query_extents_hash,  rb_str_new2("bounding_shape"), bounding_shape);
  rb_hash_aset(shape_query_extents_hash,  rb_str_new2("x_bounding"), x_bounding);
  rb_hash_aset(shape_query_extents_hash,  rb_str_new2("y_bounding"), y_bounding);
  rb_hash_aset(shape_query_extents_hash,  rb_str_new2("w_bounding"), w_bounding);
  rb_hash_aset(shape_query_extents_hash,  rb_str_new2("h_bounding"), h_bounding);
  rb_hash_aset(shape_query_extents_hash,  rb_str_new2("clip_shaped"), clip_shaped);
  rb_hash_aset(shape_query_extents_hash,  rb_str_new2("x_clip"), x_clip);
  rb_hash_aset(shape_query_extents_hash,  rb_str_new2("y_clip"), y_clip);
  rb_hash_aset(shape_query_extents_hash,  rb_str_new2("w_clip"), w_clip);
  rb_hash_aset(shape_query_extents_hash,  rb_str_new2("h_clip"), h_clip);
  
  return shape_query_extents_hash;
}

static VALUE 
ruby_shapeoffsetshape(VALUE self, VALUE dpy, VALUE win, VALUE dest_kind, VALUE x_off, VALUE y_off)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(dest_kind, T_FIXNUM);
  Check_Type(x_off, T_FIXNUM);
  Check_Type(y_off, T_FIXNUM);
  
  XShapeOffsetShape(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(dest_kind), FIX2LONG(x_off), FIX2LONG(y_off));
  
  return self;
}

static VALUE 
ruby_shapeinputselected(VALUE self, VALUE dpy, VALUE win)
{
  unsigned long mask = 0;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  mask = XShapeInputSelected(DISPLAY(dpy), FIX2LONG(win));
  
  return INT2NUM(mask);
}

static VALUE 
ruby_shapegetrectangles(VALUE self, VALUE dpy, VALUE win, VALUE kind)
{
  XRectangle *rects;
  int count=0, ordering=0, i=0;;
  
  VALUE rect_array, overall_hash;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(kind, T_FIXNUM);
  
  rects = XShapeGetRectangles(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(kind), &count, &ordering);
  
  rect_array = rb_ary_new();
  
  for (i = 0; i < count; i++) 
  {
    VALUE rect_hash;
    
    rect_hash = rb_hash_new();
    
    rb_hash_aset(rect_hash,  rb_str_new2("x"), LONG2FIX(rects[i].x));
    rb_hash_aset(rect_hash,  rb_str_new2("y"), LONG2FIX(rects[i].y));
    rb_hash_aset(rect_hash,  rb_str_new2("width"), LONG2FIX(rects[i].width));
    rb_hash_aset(rect_hash,  rb_str_new2("height"), LONG2FIX(rects[i].height));
    
    rb_ary_push(rect_array, rect_hash);
  }
  
  XFree(rects);
  
  overall_hash = rb_hash_new();
  
  rb_hash_aset(overall_hash,  rb_str_new2("ordering"), LONG2FIX(ordering));
  rb_hash_aset(overall_hash,  rb_str_new2("rects"), rect_array);
  
  return overall_hash;
}

static VALUE 
ruby_getdefaultgc(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  return LONG2FIX( DefaultGC(DISPLAY(dpy), screen) );
}

static VALUE 
ruby_getgeometry(VALUE self, VALUE dpy, VALUE win)
{
  Window root_return=None;
  int x_return=0, y_return=0;
  unsigned int width_return=0, height_return=0, border_width_return=0, depth_return=0;
  
  VALUE geometry_hash;
  
  Check_Type(dpy, T_FIXNUM);
  
  XGetGeometry(DISPLAY(dpy), FIX2LONG(win), &root_return, &x_return, &y_return, 
    &width_return, &height_return, &border_width_return, &depth_return);
  
  geometry_hash = rb_hash_new();
  
  rb_hash_aset(geometry_hash,  rb_str_new2("root"), LONG2FIX(root_return));
  rb_hash_aset(geometry_hash,  rb_str_new2("x"), LONG2FIX(x_return));
  rb_hash_aset(geometry_hash,  rb_str_new2("y"), LONG2FIX(y_return));
  rb_hash_aset(geometry_hash,  rb_str_new2("width"), LONG2FIX(width_return));
  rb_hash_aset(geometry_hash,  rb_str_new2("height"), LONG2FIX(height_return));
  rb_hash_aset(geometry_hash,  rb_str_new2("border_width"), LONG2FIX(root_return));
  rb_hash_aset(geometry_hash,  rb_str_new2("depth"), LONG2FIX(root_return));
  
  return geometry_hash;
}

static VALUE 
ruby_defaultscreen(VALUE self, VALUE dpy)
{
  Check_Type(dpy, T_FIXNUM);
  
  return LONG2FIX(DefaultScreen(DISPLAY(dpy)));   
}

static VALUE 
ruby_whitepixel(VALUE self, VALUE dpy, VALUE scr)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(scr, T_FIXNUM);      
  
  return LONG2FIX(WhitePixel(DISPLAY(dpy), FIX2LONG(scr)));
}

static VALUE 
ruby_blackpixel(VALUE self, VALUE dpy, VALUE scr)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(scr, T_FIXNUM);      
  
  return LONG2FIX(BlackPixel(DISPLAY(dpy), FIX2LONG(scr)));
}

static VALUE 
ruby_iconifywindow(VALUE self, VALUE dpy, VALUE win, VALUE scr)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(scr, T_FIXNUM);      
  
  XIconifyWindow(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(scr));
  
  return self;
}

static VALUE 
ruby_withdrawwindow(VALUE self, VALUE dpy, VALUE win, VALUE scr)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(scr, T_FIXNUM);      
  
  XWithdrawWindow(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(scr));
  
  return self;
}

static VALUE 
ruby_cleararea(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y, VALUE w, VALUE h, VALUE exposures)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(x, T_FIXNUM);
  Check_Type(y, T_FIXNUM);
  Check_Type(w, T_FIXNUM);
  Check_Type(h, T_FIXNUM);
  Check_Type(exposures, T_FIXNUM);
  
  XClearArea(DISPLAY(dpy), FIX2LONG(win), FIX2LONG(x), FIX2LONG(y), FIX2LONG(w), FIX2LONG(h), ((exposures) ? Qtrue : Qfalse));
  
  return self;
}

static VALUE ruby_destroysubwindows(VALUE self, VALUE dpy, VALUE win)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XDestroySubwindows(DISPLAY(dpy), FIX2LONG(win));
  
  return self;
}

static VALUE 
ruby_geticonname(VALUE self, VALUE dpy, VALUE win)
{
  char* name;
  VALUE rname;
  
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  
  XGetIconName(DISPLAY(dpy), FIX2LONG(win), &name);
  
  rname = rb_str_new2(name);
  
  XFree(name);
  
  return rname;
}

static VALUE 
ruby_seticonname(VALUE self, VALUE dpy, VALUE win, VALUE name)
{
  Check_Type(dpy, T_FIXNUM);
  Check_Type(win, T_FIXNUM);
  Check_Type(name, T_STRING);
  
  XSetIconName(DISPLAY(dpy), FIX2LONG(win), RSTRING_PTR(StringValue(name)));
  
  return self;
}

void Init_WindowManagement()
{
  rb_mWindowManagement = rb_define_module ("WindowManagement");
  rb_cX11 = rb_define_class_under(rb_mWindowManagement, "X11", rb_cObject);
  
  rb_define_method(rb_cX11, "open_display", ruby_opendisplay, 1);
  rb_define_method(rb_cX11, "close_display", ruby_closedisplay, 1);
  rb_define_method(rb_cX11, "root_window", ruby_getrootwindow, 2);
  rb_define_method(rb_cX11, "query_tree", ruby_querytree,2);
  rb_define_method(rb_cX11, "create_font_cursor", ruby_createfontcursor, 2);
  rb_define_method(rb_cX11, "free_cursor", ruby_freecursor, 2);
  rb_define_method(rb_cX11, "pass_down_configure_request", ruby_passdownconfigurerequest, 1);
  rb_define_method(rb_cX11, "define_cursor", ruby_definecursor, 3);       
  rb_define_method(rb_cX11, "map_raised", ruby_mapraised, 2);
  rb_define_method(rb_cX11, "set_window_border_width", ruby_setwindowborderwidth, 3);
  rb_define_method(rb_cX11, "grab_server", ruby_grabserver, 1);
  rb_define_method(rb_cX11, "ungrab_server", ruby_ungrabserver, 1);
  rb_define_method(rb_cX11, "send_configure_notify", ruby_sendconfigurenotify, 6);
  rb_define_method(rb_cX11, "store_name", ruby_storename, 3);
  rb_define_method(rb_cX11, "events_pending", ruby_eventspending, 1);
  rb_define_method(rb_cX11, "destroy_window", ruby_destroywindow, 2);
  rb_define_method(rb_cX11, "clear_window", ruby_clearwindow, 2);
  rb_define_method(rb_cX11, "map_window", ruby_mapwindow, 2);
  rb_define_method(rb_cX11, "map_subwindows", ruby_mapsubwindows, 2);
  rb_define_method(rb_cX11, "unmap_window", ruby_unmapwindow, 2);
  rb_define_method(rb_cX11, "unmap_subwindows", ruby_unmapsubwindows, 2);
  rb_define_method(rb_cX11, "sync", ruby_sync, 1);
  rb_define_method(rb_cX11, "next_event", ruby_nextevent, 1);
  rb_define_method(rb_cX11, "event_type", ruby_eventtype, 0);
  rb_define_method(rb_cX11, "flush", ruby_flush, 1);
  rb_define_method(rb_cX11, "set_override_redirect_true", ruby_setoverrideredirecttrue, 2);
  rb_define_method(rb_cX11, "set_window_event_mask", ruby_setwindoweventmask, 3);
  rb_define_method(rb_cX11, "resize_window", ruby_resizewindow, 4);
  rb_define_method(rb_cX11, "move_resize_window", ruby_moveresizewindow, 6);
  rb_define_method(rb_cX11, "reparent_window", ruby_reparentwindow, 5);
  rb_define_method(rb_cX11, "get_key_sym", ruby_getkeysym, 0);
  rb_define_method(rb_cX11, "event_type", ruby_geteventtype, 0);
  rb_define_method(rb_cX11, "draw_string", ruby_drawstring, 5);
  rb_define_method(rb_cX11, "intern_atom", ruby_interatom, 3);
  rb_define_method(rb_cX11, "select_input", ruby_selectinput, 3);
  rb_define_method(rb_cX11, "load_font", ruby_loadfont, 2);
  rb_define_method(rb_cX11, "free_font", ruby_freefont, 2);
  rb_define_method(rb_cX11, "xres", ruby_xres, 1);
  rb_define_method(rb_cX11, "yres", ruby_yres, 1);
  rb_define_method(rb_cX11, "fetch_name", ruby_fetchname, 2);
  rb_define_method(rb_cX11, "raise_window", ruby_raisewindow, 2);
  rb_define_method(rb_cX11, "lower_window", ruby_lowerwindow, 2);
  rb_define_method(rb_cX11, "allow_events", ruby_allowevents, 2);
  rb_define_method(rb_cX11, "get_transient_for_hint", ruby_gettransientforhint, 2);
  rb_define_method(rb_cX11, "create_simple_window", ruby_createsimplewindow, 7);
  rb_define_method(rb_cX11, "connection_number", ruby_connectionnumber, 1);
  rb_define_method(rb_cX11, "do_select_on_x_file_descriptor", ruby_doselectonxfiledescriptor, 1);
  rb_define_method(rb_cX11, "get_button_event", ruby_getbuttonevent, 0);
  rb_define_method(rb_cX11, "set_input_focus", ruby_setinputfocus, 2);
  rb_define_method(rb_cX11, "get_crossing_event", ruby_getcrossingevent, 0);
  rb_define_method(rb_cX11, "get_map_request_event", ruby_getmaprequestevent,0);
  rb_define_method(rb_cX11, "get_configure_request_event", ruby_getconfigurerequestevent, 0);
  rb_define_method(rb_cX11, "grab_pointer", ruby_grabpointer, 3);
  rb_define_method(rb_cX11, "ungrab_pointer", ruby_ungrabpointer, 1);     
  rb_define_method(rb_cX11, "grab_key", ruby_grabkey, 4);
  rb_define_method(rb_cX11, "ungrab_key", ruby_ungrabkey, 4);
  rb_define_method(rb_cX11, "grab_button", ruby_grabbutton, 5);
  rb_define_method(rb_cX11, "ungrab_button", ruby_ungrabbutton, 4);
  rb_define_method(rb_cX11, "get_key_event", ruby_getkeyevent, 0);
  rb_define_method(rb_cX11, "get_motion_event", ruby_getmotionevent,0);
  rb_define_method(rb_cX11, "get_unmap_notify_event", ruby_getunmapnotifyevent, 0);
  rb_define_method(rb_cX11, "get_error_event", ruby_geterrorevent, 0);
  rb_define_method(rb_cX11, "get_window_attributes", ruby_getwindowattributes,2);
  rb_define_method(rb_cX11, "move_window", ruby_movewindow, 4);
  rb_define_method(rb_cX11, "remove_from_saveset", ruby_remove_from_saveset, 2);
  rb_define_method(rb_cX11, "kill_client", ruby_killclient, 2);
  rb_define_method(rb_cX11, "text_extents", ruby_textextents, 3);
  rb_define_method(rb_cX11, "draw_rectangle", ruby_drawrectangle, 6);
  rb_define_method(rb_cX11, "draw_line", ruby_drawline, 6);
  rb_define_method(rb_cX11, "fill_rectangle", ruby_fillrectangle, 7);
  rb_define_method(rb_cX11, "query_pointer", ruby_querypointer, 2);
  rb_define_method(rb_cX11, "set_wm_state", ruby_setwmstate, 4);
  rb_define_method(rb_cX11, "get_wm_state", ruby_getwmstate, 3);
  rb_define_method(rb_cX11, "install_colormap", ruby_installcolormap, 2);
  rb_define_method(rb_cX11, "add_to_save_set", ruby_addtosaveset, 2);
  rb_define_method(rb_cX11, "alloc_named_color", ruby_allocnamedcolor, 2);
  rb_define_method(rb_cX11, "get_expose_event", ruby_getexposeevent, 0);
  rb_define_method(rb_cX11, "get_colormap_event", ruby_getcolormapevent, 0);
  rb_define_method(rb_cX11, "get_destroy_window_event", ruby_getdestroywindowevent, 0);
  rb_define_method(rb_cX11, "get_client_message_event", ruby_getclientmessageevent, 0);
  rb_define_method(rb_cX11, "get_property_event", ruby_getpropertyevent, 0);
  rb_define_method(rb_cX11, "get_focus_change_event", ruby_getfocuschangeevent, 0);
  rb_define_method(rb_cX11, "set_window_background", ruby_setwindowbackground, 3);        
  rb_define_method(rb_cX11, "destroy_subwindows", ruby_destroysubwindow, 2);
  rb_define_method(rb_cX11, "free_gc", ruby_freegc, 2);
  rb_define_method(rb_cX11, "get_wm_normal_hints", ruby_getwmnormalhints, 2);
  rb_define_method(rb_cX11, "set_window_border",ruby_setwindowborder, 3);
  rb_define_method(rb_cX11, "warp_pointer", ruby_warppointer, 9);
  rb_define_method(rb_cX11, "circulate_subwindows", ruby_circulatesubwindows, 3);
  rb_define_method(rb_cX11, "circulate_subwindows_up", ruby_circulatesubwindowsup, 2);
  rb_define_method(rb_cX11, "circulate_subwindows_down", ruby_circulatesubwindowsdown, 2);        
  rb_define_method(rb_cX11, "restack_windows", ruby_restackwindows, 2);
  rb_define_method(rb_cX11, "get_default_gc", ruby_getdefaultgc, 1);
  rb_define_method(rb_cX11, "get_geometry", ruby_getgeometry, 2);
  rb_define_method(rb_cX11, "default_screen", ruby_defaultscreen, 1);
  rb_define_method(rb_cX11, "white_pixel", ruby_whitepixel, 2);
  rb_define_method(rb_cX11, "black_pixel", ruby_blackpixel, 2);
  rb_define_method(rb_cX11, "iconify_window", ruby_iconifywindow, 3);
  rb_define_method(rb_cX11, "withdraw_window", ruby_withdrawwindow, 3);   
  rb_define_method(rb_cX11, "get_icon_name", ruby_geticonname, 2);
  
  /* Shape Extension Functions */
  
  rb_define_method(rb_cX11, "get_shape_event", ruby_getshapeevent, 0);
  rb_define_method(rb_cX11, "query_shape_extension", ruby_shapequeryextension, 1);
  rb_define_method(rb_cX11, "shape_query_version", ruby_shapequeryversion, 1);
  rb_define_method(rb_cX11, "shape_combine_region", ruby_shapecombineregion, 7);
  rb_define_method(rb_cX11, "shape_combine_rectangles", ruby_shapecombinerectangles, 8);
  rb_define_method(rb_cX11, "shape_combine_shape", ruby_shapecombineshape, 8);
  rb_define_method(rb_cX11, "shape_combine_mask", ruby_shapecombinemask, 7);
  rb_define_method(rb_cX11, "shape_select_input", ruby_shapeselectinput, 3);
  rb_define_method(rb_cX11, "shape_query_extents", ruby_shapequeryextents, 3);
  rb_define_method(rb_cX11, "shape_offset_shape", ruby_shapeoffsetshape, 5);
  rb_define_method(rb_cX11, "shape_input_selected", ruby_shapeinputselected, 2);
  rb_define_method(rb_cX11, "shape_get_rectangles", ruby_shapegetrectangles, 3);
  rb_define_method(rb_cX11, "clear_area", ruby_cleararea, 7);
  rb_define_method(rb_cX11, "destroy_subwindows", ruby_destroysubwindows, 2);
  rb_define_method(rb_cX11, "set_icon_name", ruby_seticonname, 3);
  rb_define_method(rb_cX11, "create_gc", ruby_creategc, 3);
  
  /* XPM / Pixmap functions */
  
  rb_define_method(rb_cX11, "xpm_read_file_to_pixmap", ruby_xpmreadfiletopixmap, 3);
  rb_define_method(rb_cX11, "free_pixmap", ruby_freepixmap, 2);
  rb_define_method(rb_cX11, "set_window_background_pixmap", ruby_setwindowbackgroundpixmap, 3);
  rb_define_method(rb_cX11, "xpm_create_pixmap_from_data", ruby_xpmcreatepixmapfromdata, 3);
  
  /* Xft functions */
  
  rb_define_method(rb_cX11, "load_xft_font", ruby_loadxftfont, 2);
  
  
  /* Initializer for the BIG class */     
  
  rb_define_method(rb_cX11, "initialize", ruby_X11_init, 0);
}

/* Specialized Functions */

int handleXError(Display *dpy, XErrorEvent *e)
{
	if (e->error_code == BadAccess && 
		e->resourceid == RootWindow(dpy, DefaultScreen(dpy)) ) 
	{
		printf("The root window is being used at the moment!\n");
		exit(1);
	}
	
	return 0;
}
