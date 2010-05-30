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

class WindowResizeTest
	require 'WindowManagement'
	require 'WindowManagementConstants'
	
	def initialize
		@x = WindowManagement::X11.new

		@dpy = @x.open_display(":0")

		unless @dpy
			puts "I cannot open the display, sorry..."
			exit
		end

		@root_window = @x.root_window(@dpy)
		
		@test_win = @x.create_simple_window(@dpy, @root_window, 5, 5, 640, 480, 1)

		@min_size=50

		@resize_win_x = 140
		@resize_win_y = 100

		@resize_win_width = 320
		@resize_win_height = 240

		@resize_handle_width = 30
		@resize_handle_height = 5

		@resize_main_win = @x.create_simple_window(@dpy, @test_win, 5, 5, @resize_win_width, @resize_win_height, 1)

		@resize_top_left_win = @x.create_simple_window(@dpy, @resize_main_win, 0, 0, @resize_handle_width, @resize_handle_height, 1)
		@resize_top_right_win = @x.create_simple_window(@dpy, @resize_main_win, 0, 0, @resize_handle_width, @resize_handle_height, 1)
		@resize_bottom_left_win = @x.create_simple_window(@dpy, @resize_main_win, 0, 0, @resize_handle_width, @resize_handle_height, 1)
		@resize_bottom_right_win = @x.create_simple_window(@dpy, @resize_main_win, 0, 0, @resize_handle_width, @resize_handle_height, 1)

		@x.store_name(@dpy, @test_win, "Window")
		
		mask =  WindowManagementConstants::KeyPressMask		
		@x.set_window_event_mask(@dpy, @test_win, mask)
	
		mask =  WindowManagementConstants::ButtonPressMask | WindowManagementConstants::ButtonReleaseMask
		
		@x.set_window_event_mask(@dpy, @resize_top_left_win, mask)
		@x.set_window_event_mask(@dpy, @resize_top_right_win, mask)
		@x.set_window_event_mask(@dpy, @resize_bottom_left_win, mask)
		@x.set_window_event_mask(@dpy, @resize_bottom_right_win, mask)
	
		@x.map_subwindows(@dpy, @resize_main_win)
		@x.map_subwindows(@dpy, @test_win)
		@x.map_window(@dpy, @test_win)

		@x.move_window(@dpy, @resize_main_win, @resize_win_x, @resize_win_y)
		
		place_handles
		
		eventloop
	end

	def place_handles
		@x.move_window(@dpy, @resize_top_left_win, -1, -1)
		@x.move_window(@dpy, @resize_top_right_win, @resize_win_width-(@resize_handle_width+1), -1)
		@x.move_window(@dpy, @resize_bottom_left_win, -1, @resize_win_height-(@resize_handle_height+1))
		@x.move_window(@dpy, @resize_bottom_right_win, @resize_win_width-(@resize_handle_width+1), @resize_win_height-(@resize_handle_height+1))
	end

	def motion_notify
		motion_event = @x.get_motion_event

		if(@start)
			xdiff = motion_event["x_root"] - @start["x_root"]
			ydiff = motion_event["y_root"] - @start["y_root"]

			case motion_event["window"]
				when @resize_top_left_win
					resize_top_left(xdiff, ydiff)

				when @resize_bottom_left_win
					resize_bottom_left(xdiff, ydiff)

				when @resize_top_right_win
					resize_top_right(xdiff, ydiff)
		
				when @resize_bottom_right_win
					resize_bottom_right(xdiff, ydiff)
			end
	
			if(@resize_win_width < @min_size or @resize_win_height < @min_size)
				return
			end
	
			@x.move_resize_window(@dpy, @resize_main_win, @resize_win_x, @resize_win_y, @resize_win_width, @resize_win_height)

			place_handles
		end
	end

	def resize_top_left(x, y)
		@resize_win_x = @attr["x"]+x
		@resize_win_y = @attr["y"]+y
		@resize_win_width = @attr["width"]-x
		@resize_win_height = @attr["height"]-y
	end
	
	def resize_bottom_left(x,y)
		@resize_win_x = @attr["x"]+x
		@resize_win_width = @attr["width"]-x
		@resize_win_height = @attr["height"]+y
	end
	
	def resize_top_right(x,y)
		@resize_win_y = @attr["y"]+y
		@resize_win_width = @attr["width"]+x
		@resize_win_height = @attr["height"]-y
	end
	
	def resize_bottom_right(x,y)
		@resize_win_width = @attr["width"]+x
		@resize_win_height = @attr["height"]+y
	end
	
	def eventloop
	
		while true
			@x.next_event(@dpy)
		
			case @x.event_type
				when WindowManagementConstants::KeyPress
					if @x.get_key_sym == WindowManagementConstants::XKey_q
						@x.destroy_subwindows(@dpy, @resize_main_win)
						@x.destroy_window(@dpy, @test_win)
						@x.close_display(@dpy)
						exit
					end

				when WindowManagementConstants::MotionNotify
					motion_notify
		
				when WindowManagementConstants::ButtonPress
					button_event = @x.get_button_event
					
					unless(button_event["window"]==@resize_main_win)
						@x.grab_pointer(@dpy, button_event["window"] , WindowManagementConstants::PointerMotionMask|WindowManagementConstants::ButtonReleaseMask)
						@attr = @x.get_window_attributes(@dpy, @resize_main_win)
						@start=button_event
					end
					
				when WindowManagementConstants::ButtonRelease
					@x.ungrab_pointer(@dpy)
	
		
			end # End case
		end
	end
end

pt = WindowResizeTest.new
