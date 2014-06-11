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

#ifndef _RUBY_X11_
#define _RUBY_X11_

#include <ruby.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>

static VALUE ruby_X11_init(VALUE self);
static VALUE ruby_opendisplay(VALUE self, VALUE display_str);
static VALUE ruby_closedisplay(VALUE self, VALUE dpy);
static VALUE ruby_getrootwindow(VALUE self, VALUE dpy, VALUE screen);
static VALUE ruby_querytree(VALUE self, VALUE dpy, VALUE root);
static VALUE ruby_createfontcursor(VALUE self, VALUE dpy, VALUE cursor);
static VALUE ruby_freecursor(VALUE self, VALUE dpy, VALUE cursor);
static VALUE ruby_definecursor(VALUE self, VALUE win, VALUE dpy, VALUE cursor);
static VALUE ruby_mapraised(VALUE self,VALUE dpy, VALUE win);
static VALUE ruby_drawline(VALUE self, VALUE dpy, VALUE win, VALUE x1, VALUE y1, VALUE x2, VALUE y2);
static VALUE ruby_setwindowborderwidth(VALUE self, VALUE dpy, VALUE win, VALUE width);
static VALUE ruby_grabserver(VALUE self, VALUE dpy);
static VALUE ruby_ungrabserver(VALUE self, VALUE dpy);
static VALUE ruby_sendconfigurenotify(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y, VALUE width, VALUE height);
static VALUE ruby_storename(VALUE self, VALUE win, VALUE dpy, VALUE name);
static VALUE ruby_eventspending(VALUE self, VALUE dpy);
static VALUE ruby_destroywindow(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_createsimplewindow(VALUE self, VALUE dpy, VALUE parent, VALUE x, VALUE y, VALUE width, VALUE height, VALUE border_width);
static VALUE ruby_clearwindow(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_mapwindow(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_mapsubwindows(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_unmapwindow(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_unmapsubwindows(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_sync(VALUE self, VALUE dpy);
static VALUE ruby_nextevent(VALUE self, VALUE dpy);
static VALUE ruby_eventtype(VALUE self);
static VALUE ruby_flush(VALUE self, VALUE dpy);
static VALUE ruby_setoverrideredirecttrue(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_setwindoweventmask(VALUE self, VALUE dpy, VALUE win, VALUE mask);
static VALUE ruby_resizewindow(VALUE self, VALUE dpy, VALUE win, VALUE width, VALUE height);
static VALUE ruby_moveresizewindow(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y, VALUE width, VALUE height);
static VALUE ruby_reparentwindow(VALUE self, VALUE dpy, VALUE win, VALUE parent, VALUE x, VALUE y);
static VALUE ruby_getkeysym(VALUE self);
static VALUE ruby_geteventtype(VALUE self);
static VALUE ruby_drawstring(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y, VALUE str);
static VALUE ruby_interatom(VALUE self, VALUE dpy, VALUE atom_name, VALUE only_if_exists);
static VALUE ruby_selectinput(VALUE self, VALUE dpy, VALUE win, VALUE mask);
static VALUE ruby_loadfont(VALUE self, VALUE dpy, VALUE font_str);
static VALUE ruby_freefont(VALUE self, VALUE dpy, VALUE font);
static VALUE ruby_xres(VALUE self, VALUE dpy);
static VALUE ruby_yres(VALUE self, VALUE dpy);
static VALUE ruby_fetchname(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_raisewindow(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_lowerwindow(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_allowevents(VALUE self, VALUE dpy, VALUE mode);
static VALUE ruby_gettransientforhint(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_connectionnumber(VALUE self, VALUE dpy);
static VALUE ruby_setinputfocus(VALUE self, VALUE dpy, VALUE win); 
static VALUE ruby_grabpointer(VALUE self, VALUE dpy, VALUE win, VALUE mask);
static VALUE ruby_ungrabpointer(VALUE self, VALUE dpy);
static VALUE ruby_getwindowattributes(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_grabkey(VALUE self, VALUE dpy, VALUE key, VALUE modifiers, VALUE win);
static VALUE ruby_ungrabkey(VALUE self, VALUE dpy, VALUE key, VALUE modifiers, VALUE win);
static VALUE ruby_grabbutton(VALUE self, VALUE dpy, VALUE button, VALUE modifiers, VALUE win, VALUE mask);
static VALUE ruby_ungrabbutton(VALUE self, VALUE dpy, VALUE button, VALUE modifiers, VALUE win);
static VALUE ruby_movewindow(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y);
static VALUE ruby_remove_from_saveset(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_killclient(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_textextents(VALUE self, VALUE dpy, VALUE font, VALUE str);
static VALUE ruby_drawrectangle(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y, VALUE width, VALUE height); 
static VALUE ruby_fillrectangle(VALUE self, VALUE dpy, VALUE win, VALUE gc, VALUE x, VALUE y, VALUE width, VALUE height); 
static VALUE ruby_querypointer(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_installcolormap(VALUE self, VALUE dpy, VALUE colormap);
static VALUE ruby_addtosaveset(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_allocnamedcolor(VALUE self, VALUE dpy, VALUE colstr);
static VALUE ruby_setwindowbackground(VALUE self, VALUE dpy, VALUE win,  VALUE color);
static VALUE ruby_creategc(VALUE self, VALUE dpy, VALUE win, VALUE gcvalues);
static VALUE ruby_destroysubwindow(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_freegc(VALUE self, VALUE dpy, VALUE gc);
static VALUE ruby_getwmnormalhints(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_setwindowborder(VALUE self, VALUE dpy, VALUE win, VALUE border_pixel);
static VALUE ruby_warppointer(VALUE self, VALUE dpy, VALUE src_w, VALUE dest_w, VALUE src_x, VALUE src_y, VALUE src_width, VALUE src_height, VALUE dest_x, VALUE dest_y);

/* EVENT STRUCT GETTERS! 
*
* These pass back a hash with the data we'd get in the event struct for each event
*/
static VALUE ruby_getbuttonevent(VALUE self);
static VALUE ruby_getcrossingevent(VALUE self);
static VALUE ruby_getmaprequestevent(VALUE self);
static VALUE ruby_getkeyevent(VALUE self);
static VALUE ruby_getmotionevent(VALUE self);
static VALUE ruby_getconfigurerequestevent(VALUE self);
static VALUE ruby_getunmapnotifyevent(VALUE self);
static VALUE ruby_geterrorevent(VALUE self);
static VALUE ruby_getexposeevent(VALUE self);
static VALUE ruby_getcolormapevent(VALUE self);
static VALUE ruby_getdestroywindowevent(VALUE self);
static VALUE ruby_getclientmessageevent(VALUE self);
static VALUE ruby_getpropertyevent(VALUE self);
static VALUE ruby_getfocuschangeevent(VALUE self);
static VALUE ruby_circulatesubwindows(VALUE self, VALUE dpy, VALUE win, VALUE direction);
static VALUE ruby_circulatesubwindowsup(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_circulatesubwindowsdown(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_restackwindows(VALUE self, VALUE dpy, VALUE windows);
static VALUE ruby_getdefaultgc(VALUE self, VALUE dpy);
static VALUE ruby_getgeometry(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_defaultscreen(VALUE self, VALUE dpy);
static VALUE ruby_whitepixel(VALUE self, VALUE dpy, VALUE scr);
static VALUE ruby_blackpixel(VALUE self, VALUE dpy, VALUE scr);
static VALUE ruby_iconifywindow(VALUE self, VALUE dpy, VALUE win, VALUE scr);
static VALUE ruby_withdrawwindow(VALUE self, VALUE dpy, VALUE win, VALUE scr);
static VALUE ruby_cleararea(VALUE self, VALUE dpy, VALUE win, VALUE x, VALUE y, VALUE w, VALUE h, VALUE exposures);
static VALUE ruby_destroysubwindows(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_geticonname(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_seticonname(VALUE self, VALUE dpy, VALUE win, VALUE name);

/* Specialized functions below */

static VALUE ruby_setwmstate(VALUE self, VALUE dpy, VALUE win, VALUE wm_state_atom, VALUE wm_state);
static VALUE ruby_getwmstate(VALUE self, VALUE dpy, VALUE win, VALUE wm_state_atom);

static VALUE ruby_passdownconfigurerequest(VALUE self, VALUE dpy);

/* Shape Extension functions ( shape.h) */

static VALUE ruby_getshapeevent(VALUE self);
static VALUE ruby_shapequeryextension(VALUE self, VALUE dpy);
static VALUE ruby_shapequeryversion(VALUE self, VALUE dpy);
static VALUE ruby_shapecombineregion(VALUE self, VALUE dpy, VALUE dest, VALUE dest_kind, VALUE x_off, VALUE y_off, VALUE region, VALUE op);
static VALUE ruby_shapecombinerectangles(VALUE self, VALUE dpy, VALUE dest, VALUE dest_kind, VALUE x_off, VALUE y_off, VALUE rectangles, VALUE op, VALUE ordering);
static VALUE ruby_shapecombinemask(VALUE self, VALUE dpy, VALUE dest, VALUE dest_kind, VALUE x_off, VALUE y_off, VALUE src, VALUE op);
static VALUE ruby_shapecombineshape(VALUE self, VALUE dpy, VALUE dest, VALUE dest_kind, VALUE x_off, VALUE y_off, VALUE src, VALUE src_kind, VALUE op);
static VALUE ruby_shapequeryextents(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_shapeselectinput(VALUE self, VALUE dpy, VALUE win, VALUE mask);
static VALUE ruby_shapeoffsetshape(VALUE self, VALUE dpy, VALUE win, VALUE dest_kind, VALUE x_off, VALUE y_off);
static VALUE ruby_shapeselectinput(VALUE self, VALUE dpy, VALUE win, VALUE mask);
static VALUE ruby_shapeinputselected(VALUE self, VALUE dpy, VALUE win);
static VALUE ruby_shapegetrectangles(VALUE self, VALUE dpy, VALUE win, VALUE kind);

/* Pixmap functions */

static VALUE ruby_xpmreadfiletopixmap(VALUE self, VALUE dpy, VALUE drawable, VALUE filename);
static VALUE ruby_freepixmap(VALUE self, VALUE dpy, VALUE pixmap);
static VALUE ruby_setwindowbackgroundpixmap(VALUE self, VALUE dpy, VALUE win, VALUE pixmap);
static VALUE ruby_xpmcreatepixmapfromdata(VALUE self, VALUE dpy, VALUE drawable, VALUE data);

/* Xft functions */

static VALUE ruby_loadxftfont(VALUE self, VALUE dpy, VALUE font_str);
/*static VALUE ruby_xftdrawstring8(VALUE self, VALUE xftdraw, VALUE color, VALUE font, VALUE x, VALUE y, VALUE string);*/

/* Specialized Functions */

int handleXError(Display *dpy, XErrorEvent *e);

/* The init function for this extension, does nothing! */

void Init_WindowManagement();

#endif
