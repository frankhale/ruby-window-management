# A small extension to provide access to a subset of Xlib from Ruby 
# Copyright (C) 2010 Frank Hale <frankhale@gmail.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# By: Frank Hale <frankhale@gmail.com>
# Started On or About: 12 June 2005
#
# Updated License: 30 May 2010


module WindowManagementConstants
	
	None = 0
	
	PointerRoot = 1
	
	CurrentTime = 0
	
	False = 0
	True = 1
	
	IncludeInferiors =  1
	
	# Not all of this is supported yet
	
	InputOutput 	= 1
	InputOnly 	= 2
	
	# Useful header is X.h
	
	BadAccess = 10
	
	# Cursors
	XC_num_glyphs 		= 154
	XC_X_cursor 		= 0
	XC_arrow 		= 2
	XC_based_arrow_down 	= 4
	XC_based_arrow_up 	= 6
	XC_boat 		= 8
	XC_bogosity 		= 10
	XC_bottom_left_corner 	= 12
	XC_bottom_right_corner 	= 14
	XC_bottom_side 		= 16
	XC_bottom_tee 		= 18
	XC_box_spiral 		= 20
	XC_center_ptr 		= 22
	XC_circle 		= 24
	XC_clock 		= 26
	XC_coffee_mug 		= 28
	XC_cross 		= 30
	XC_cross_reverse 	= 32
	XC_crosshair 		= 34
	XC_diamond_cross 	= 36
	XC_dot 			= 38
	XC_dotbox 		= 40
	XC_double_arrow 	= 42
	XC_draft_large 		= 44
	XC_draft_small 		= 46
	XC_draped_box 		= 48
	XC_exchange 		= 50
	XC_fleur 		= 52
	XC_gobbler 		= 54
	XC_gumby		= 56
	XC_hand1 		= 58
	XC_hand2 		= 60
	XC_heart 		= 62
	XC_icon 		= 64
	XC_iron_cross 		= 66
	XC_left_ptr 		= 68
	XC_left_side 		= 70
	XC_left_tee 		= 72
	XC_leftbutton 		= 74
	XC_ll_angle 		= 76
	XC_lr_angle 		= 78
	XC_man 			= 80
	XC_middlebutton 	= 82
	XC_mouse 		= 84
	XC_pencil 		= 86
	XC_pirate 		= 88
	XC_plus 		= 90
	XC_question_arrow 	= 92
	XC_right_ptr 		= 94
	XC_right_side 		= 96
	XC_right_tee 		= 98
	XC_rightbutton 		= 100
	XC_rtl_logo 		= 102
	XC_sailboat 		= 104
	XC_sb_down_arrow 	= 106
	XC_sb_h_double_arrow 	= 108
	XC_sb_left_arrow 	= 110
	XC_sb_right_arrow 	= 112
	XC_sb_up_arrow 		= 114
	XC_sb_v_double_arrow 	= 116
	XC_shuttle 		= 118
	XC_sizing 		= 120
	XC_spider 		= 122
	XC_spraycan 		= 124
	XC_star 		= 126
	XC_target 		= 128
	XC_tcross 		= 130
	XC_top_left_arrow 	= 132
	XC_top_left_corner 	= 134
	XC_top_right_corner 	= 136
	XC_top_side 		= 138
	XC_top_tee 		= 140
	XC_trek 		= 142
	XC_ul_angle 		= 144
	XC_umbrella 		= 146
	XC_ur_angle 		= 148
	XC_watch 		= 150
	XC_xterm 		= 152
	
	# Property Events
	XA_WM_NAME 		= 39
	XA_WM_NORMAL_HINTS 	= 40
	XA_WM_SIZE_HINTS 	= 41
	XA_WM_TRANSIENT_FOR 	= 68
	
	# Keys
	XKey_F1 		= 0xFFBE
	XKey_q 			= 0x071
	
	# ConfigureWindow stuff
 	CWX			= (1<<0)
 	CWY			= (1<<1)
 	CWWidth			= (1<<2)
 	CWHeight		= (1<<3)
 	CWBorderWidth		= (1<<4)
 	CWSibling		= (1<<5)
	CWStackMode		= (1<<6)
	
	
	# Key mod masks
	ShiftMask		= (1<<0)
	LockMask		= (1<<1)
 	ControlMask		= (1<<2)
	Mod1Mask		= (1<<3)
	Mod2Mask		= (1<<4)
	Mod3Mask		= (1<<5)
	Mod4Mask		= (1<<6)
	Mod5Mask		= (1<<7)
	
	AnyModifier		= (1<<15)
	
	# X Event masks
	KeyPressMask 		= (1<<0)
	KeyReleaseMask 		= (1<<1)
	ButtonPressMask 	= (1<<2)
	ButtonReleaseMask 	= (1<<3) 
	EnterWindowMask 	= (1<<4)
	LeaveWindowMask 	= (1<<5)
	PointerMotionMask 	= (1<<6) 
	PointerMotionHintMask	= (1<<7) 
	Button1MotionMask	= (1<<8) 
	Button2MotionMask	= (1<<9)
	Button3MotionMask	= (1<<10)
	Button4MotionMask	= (1<<11)
	Button5MotionMask	= (1<<12)
	ButtonMotionMask	= (1<<13)
	KeymapStateMask		= (1<<14)
	ExposureMask 		= (1<<15)	
	VisibilityChangeMask	= (1<<16)
	StructureNotifyMask	= (1<<17)
	ResizeRedirectMask	= (1<<18)
	SubstructureNotifyMask	= (1<<19)
	SubstructureRedirectMask= (1<<20)
	FocusChangeMask		= (1<<21)
	PropertyChangeMask	= (1<<22)
	ColormapChangeMask	= (1<<23)
	
	# Borrowed from aewm, they just make sense!
	
	ChildMask = (SubstructureRedirectMask|SubstructureNotifyMask)
	ButtonMask = (ButtonPressMask|ButtonReleaseMask)
	MouseMask = (ButtonMask|PointerMotionMask)
	
	# Window states
	WithdrawnState 		= 0	
	NormalState 		= 1	
	IconicState 		= 3	
	
	# Window states
	IsUnmapped		= 0
	IsUnviewable		= 1
	IsViewable		= 2
	
	# X Events
	KeyPress 		= 2
	KeyRelease 		= 3
	ButtonPress		= 4
	ButtonRelease		= 5
	MotionNotify		= 6
	EnterNotify		= 7
	LeaveNotify		= 8
	FocusIn			= 9
	FocusOut		= 10
	KeymapNotify		= 11
	Expose 			= 12
	GraphicsExpose		= 13
	NoExpose		= 14
	VisibilityNotify	= 15
	CreateNotify		= 16
	DestroyNotify		= 17
	UnmapNotify		= 18
	MapNotify		= 19
	MapRequest		= 20
	ReparentNotify		= 21
	ConfigureNotify		= 22
	ConfigureRequest	= 23
	GravityNotify		= 24
	ResizeRequest		= 25
	CirculateNotify		= 26
	CirculateRequest	= 27
	PropertyNotify		= 28
	SelectionClear		= 29
	SelectionRequest	= 30
	SelectionNotify		= 31
	ColormapNotify		= 32
	ClientMessage		= 33
	MappingNotify		= 34 
	
	# GC function masks
	GXclear			= 0x0	
	GXand			= 0x1		
	GXandReverse		= 0x2		
	GXcopy 			= 0x3
	GXandInverted 		= 0x4	
	GXnoop			= 0x5		
	GXxor			= 0x6		
	GXor			= 0x7		
	GXnor			= 0x8		
	GXequiv			= 0x9		
	GXinvert		= 0xa		
	GXorReverse		= 0xb		
	GXcopyInverted		= 0xc	
	GXorInverted		= 0xd	
	GXnand			= 0xe		
	GXset 			= 0xf		
	
 	GCFunction 		= (1<<0)
 	GCPlaneMask 		= (1<<1)
 	GCForeground 		= (1<<2)
 	GCBackground 		= (1<<3)
 	GCLineWidth  		= (1<<4)
 	GCLineStyle  		= (1<<5)
 	GCCapStyle   		= (1<<6)
 	GCJoinStyle		= (1<<7)
 	GCFillStyle		= (1<<8)
 	GCFillRule		= (1<<9)
 	GCTile			= (1<<10)
 	GCStipple		= (1<<11)
 	GCTileStipXOrigin	= (1<<12)
 	GCTileStipYOrigin	= (1<<13)
 	GCFont			= (1<<14)
 	GCSubwindowMode		= (1<<15)
 	GCGraphicsExposures	= (1<<16)
 	GCClipXOrigin		= (1<<17)
 	GCClipYOrigin		= (1<<18)
 	GCClipMask		= (1<<19)
 	GCDashOffset		= (1<<20)
	GCDashList		= (1<<21)
	GCArcMode		= (1<<22)
	
	# flags argument in size hints 
	USPosition 	= (1<<0) # user specified x, y 
	USSize		= (1<<1) # user specified width, height 
	PPosition 	= (1<<2) # program specified position 
	PSize		= (1<<3) # program specified size 
	PMinSize 	= (1<<4) # program specified minimum size 
	PMaxSize 	= (1<<5) # program specified maximum size 
	PResizeInc 	= (1<<6) # program specified resize increments 
	PAspect		= (1<<7) # program specified min and max aspect ratios 
	PBaseSize 	= (1<<8) # program specified base for incrementing 
	PWinGravity 	= (1<<9) # program specified window gravity 
	
	PAllHints  = (PPosition|PSize|PMinSize|PMaxSize|PResizeInc|PAspect)
	
	# Used for XCirculateSubwindows
	RaiseLowest     = 0
	LowerHighest	= 1
	
 	X_ShapeQueryVersion		=0
 	X_ShapeRectangles		=1
 	X_ShapeMask			=2
 	X_ShapeCombine			=3
 	X_ShapeOffset			=4
 	X_ShapeQueryExtents		=5
 	X_ShapeSelectInput		=6
 	X_ShapeInputSelected		=7
 	X_ShapeGetRectangles		=8
 	
 	ShapeSet			=0
 	ShapeUnion			=1
 	ShapeIntersect			=2
 	ShapeSubtract			=3
 	ShapeInvert			=4
 	
 	ShapeBounding			=0
 	ShapeClip			=1
 	
 	ShapeNotifyMask			=(1<<0)
 	ShapeNotify			=0
 	
 	ShapeNumberEvents		=(ShapeNotify + 1)
	
	# SetClipRectangles ordering 
	
 	Unsorted 	= 0
 	YSorted		= 1
 	YXSorted 	= 2
 	YXBanded 	= 3
	
	# GrabPointer, GrabButton, GrabKeyboard, GrabKey Modes 
	
	GrabModeSync	= 0
	GrabModeAsync	= 1
	
	
	# AllowEvents modes 
	
 	AsyncPointer	= 0
 	SyncPointer	= 1
 	ReplayPointer	= 2
 	AsyncKeyboard	= 3
 	SyncKeyboard	= 4
 	ReplayKeyboard	= 5
 	AsyncBoth	= 6
 	SyncBoth	= 7
 	
	# Used in SetInputFocus, GetInputFocus 
	
	RevertToNone		= None
	RevertToPointerRoot	= PointerRoot
	RevertToParent		= 2
	
	# Window stacking method (in configureWindow) 
	
	Above		= 0
	Below		= 1
	TopIf		= 2
	BottomIf	= 3
	Opposite	= 4
	
end
