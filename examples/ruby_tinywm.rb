#!/usr/bin/ruby
#
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

class TinyWM
	require '../src/WindowManagement'
	require '../src/WindowManagementConstants'

	def initialize
		init
		eventloop
	end

	def init
		@x = WindowManagement::X11.new
		
		@dpy = @x.open_display("")
		
		if @dpy then
			puts "Starting the Ruby Tiny Window Manager..."
		end

		@leftArrowCursor = @x.create_font_cursor(@dpy, WindowManagementConstants::XC_left_ptr)

		@screen = @x.default_screen(@dpy)
		@root_window = @x.root_window(@dpy, @screen)

		@x.define_cursor(@dpy, @root_window, @leftArrowCursor)

		mask =  WindowManagementConstants::SubstructureRedirectMask	|
			WindowManagementConstants::SubstructureNotifyMask  	|
			WindowManagementConstants::ButtonPressMask		|
			WindowManagementConstants::ButtonReleaseMask		|
			WindowManagementConstants::EnterWindowMask		|
			WindowManagementConstants::ButtonMotionMask		|
			WindowManagementConstants::KeyPressMask			
		
		@x.set_window_event_mask(@dpy, @root_window, mask)
		
		querywindowtree
	end
	
	def querywindowtree
		window_list = @x.query_tree(@dpy, @root_window)

		window_list.each do |win|
			@x.map_window(@dpy, win)
		end
	end
	
	def shutdown_wm
		@x.free_cursor(@dpy, @leftArrowCursor)
		@x.close_display(@dpy)
		exit
	end
	
	def eventloop
		
		@x.grab_button(@dpy, 1, WindowManagementConstants::ShiftMask, @root_window, WindowManagementConstants::ButtonPressMask)
		@x.grab_button(@dpy, 3, WindowManagementConstants::ShiftMask, @root_window, WindowManagementConstants::ButtonPressMask)
				
		while true 
			
			trap("INT") {
				puts "(Trapped SIGINT) Stopping the Ruby Tiny Window Manager..."					
				shutdown_wm				
			}
			
			@x.next_event(@dpy)
			
			case @x.event_type
			
				when WindowManagementConstants::KeyPress
					if @x.get_key_sym == WindowManagementConstants::XKey_F1
						puts "Stopping the Ruby Tiny Window Manager..."
						shutdown_wm
					end
				
				when WindowManagementConstants::MotionNotify
					motion_event = @x.get_motion_event
					
					if (@start && (motion_event["window"] != motion_event["root"]))
					
						xdiff = motion_event["x_root"] - @start["x_root"]
						ydiff = motion_event["y_root"] - @start["y_root"]
											
						move_x = @attr["x"] + (@start["button"]==1 ? xdiff : 0)
						move_y = @attr["y"] + (@start["button"]==1 ? ydiff : 0)
						move_width = @attr["width"] + (@start["button"]==3 ? xdiff : 0)
						move_height = @attr["height"] + (@start["button"]==3 ? ydiff : 0)
					
					@x.move_resize_window(@dpy, motion_event["window"],
						move_x,
						move_y,
						[100, move_width].max,
						[100, move_height].max
					)
					end
				
				when WindowManagementConstants::ButtonPress
					button_event = @x.get_button_event
					
					@x.set_input_focus(@dpy, button_event["subwindow"])
					if (button_event["subwindow"]!=0) then
						@x.raise_window(@dpy, button_event["subwindow"])
						
						@x.grab_pointer(@dpy, button_event["subwindow"], WindowManagementConstants::PointerMotionMask|WindowManagementConstants::ButtonReleaseMask)

						@attr = @x.get_window_attributes(@dpy, button_event["subwindow"])
				
						@start=button_event
					end
											
				when WindowManagementConstants::ButtonRelease
					@x.ungrab_pointer(@dpy)
				
				when WindowManagementConstants::EnterNotify
					crossing_event = @x.get_crossing_event
					@x.set_input_focus(@dpy, crossing_event["window"])
				
				when WindowManagementConstants::MapRequest
					map_request_event = @x.get_map_request_event
					@x.map_window(@dpy, map_request_event["window"])
					@x.select_input(@dpy, map_request_event["window"], WindowManagementConstants::EnterWindowMask)
				
				when WindowManagementConstants::ConfigureRequest
					@x.pass_down_configure_request(@dpy)
					
			end
		
		end
	end

end

TinyWM.new
