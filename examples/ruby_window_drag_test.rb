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

class RubyX11Test
	require '../src/WindowManagement'
	require '../src/WindowManagementConstants'
	
	def initialize
		@x = WindowManagement::X11.new

		@dpy = @x.open_display(":0")

		unless @dpy
			puts "I cannot open the display, sorry..."
			exit
		end

		@screen = @x.default_screen(@dpy)
		
		@root_window = @x.root_window(@dpy, @screen)
		
		@test_win = @x.create_simple_window(@dpy, @root_window, 5, 5, 640, 480, 1)
		
		@win1 = @x.create_simple_window(@dpy, @test_win, 25, 25, 200, 50, 1)
		@win2 = @x.create_simple_window(@dpy, @test_win, 100, 100, 200, 50, 1)
		@win3 = @x.create_simple_window(@dpy, @test_win, 175, 175, 200, 50, 1)
		
		@x.store_name(@dpy, @test_win, "Window / Drag / Discover")
		
		mask =  WindowManagementConstants::ButtonPressMask		|
			WindowManagementConstants::ButtonReleaseMask		|
			WindowManagementConstants::ButtonMotionMask		|
			WindowManagementConstants::KeyPressMask			|
			WindowManagementConstants::ExposureMask			
		
		@x.set_window_event_mask(@dpy, @test_win, mask)
		@x.set_window_event_mask(@dpy, @win1, mask)
		@x.set_window_event_mask(@dpy, @win2, mask)
		@x.set_window_event_mask(@dpy, @win3, mask)
		
		@x.map_window(@dpy, @test_win)
		@x.map_subwindows(@dpy, @test_win)
		
		paint_wins
		
		eventloop
	end
	
	def paint_wins
		@x.draw_string(@dpy, @win1, 5, 15, "win id = #{@win1.to_s}")
		@x.draw_string(@dpy, @win2, 5, 15, "win id = #{@win2.to_s}")
		@x.draw_string(@dpy, @win3, 5, 15, "win id = #{@win3.to_s}")
	end
	
	def eventloop
	
		while true
			@x.next_event(@dpy)
		
			case @x.event_type
				when WindowManagementConstants::KeyPress
					if @x.get_key_sym == WindowManagementConstants::XKey_q
						@x.destroy_subwindows(@dpy, @test_win)
						@x.destroy_window(@dpy, @test_win)
						@x.close_display(@dpy)
						exit
					end
						
				when WindowManagementConstants::MotionNotify
					motion_event = @x.get_motion_event
					
					unless (@motion==true)
						@motion=true
					end
					
					if (@start && (motion_event["window"] != @test_win))
											
						xdiff = motion_event["x_root"] - @start["x_root"]
						ydiff = motion_event["y_root"] - @start["y_root"]
											
						move_x = @attr["x"] + (@start["button"]==1 ? xdiff : 0)
						move_y = @attr["y"] + (@start["button"]==1 ? ydiff : 0)
					
						@x.move_window(@dpy, motion_event["window"],move_x,move_y)
					end

				when WindowManagementConstants::ButtonPress
					button_event = @x.get_button_event
					
					@x.raise_window(@dpy, button_event["window"])
					
					if (button_event["window"]!=@test_win) then
						@attr = @x.get_window_attributes(@dpy, button_event["window"])
						@start=button_event
					end
			
				when WindowManagementConstants::ButtonRelease
					button_event = @x.get_button_event
					
					if(@motion == true)
					
					window_list = @x.query_tree(@dpy, @test_win)

					pointer = @x.query_pointer(@dpy, @test_win)

					pointer_x = pointer["win_x"]
					pointer_y = pointer["win_y"]

					@dragged_list = []

					window_list.each { 
						|win| 
						
						if ((win != button_event["window"]) && (win != @test_win))
						
							attr = @x.get_window_attributes(@dpy, win)
							
							@window_x = attr["x"]
							@window_y = attr["y"]
							window_width_max = @window_x + attr["width"]
							window_height_max = @window_y + attr["height"]
							
							test_for_x = ( (pointer_x >= @window_x) && (pointer_x <= window_width_max) )
							test_for_y = ( (pointer_y >= @window_y) && (pointer_y <= window_height_max) )
																					
							if ( test_for_x && test_for_y )
								@dragged_list.push(win)															
							end
						end
					}
					
					# At this point we know what window we are over relative to the mouse pointer					
					unless ( @dragged_list.length==0 )
						@x.clear_window(@dpy, @test_win)
						@x.draw_string(@dpy, @test_win, 5, 450, "Window #{button_event["window"]} dragged onto #{@dragged_list.last}")
					end
										
					@motion=false

					end
			
				when WindowManagementConstants::Expose
					paint_wins
			
			end # End case
		end
	end
end

rt = RubyX11Test.new
