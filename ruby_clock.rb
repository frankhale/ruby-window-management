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

class RubyClock
	require 'WindowManagement'
	require 'WindowManagementConstants'
	require 'time'
	
	def initialize
		@x = WindowManagement::X11.new

		@dpy = @x.open_display(":0")

		unless @dpy
			puts "I cannot open the display, sorry..."
			exit
		end

		root_window = @x.root_window(@dpy, @x.default_screen(@dpy))
		
		@clock_win = @x.create_simple_window(@dpy, root_window, 5, 5, 200, 30, 1)
		
		@x.store_name(@dpy, @clock_win, "Ruby Clock")
		
		@x.select_input(@dpy, @clock_win, WindowManagementConstants::KeyPressMask |
					    WindowManagementConstants::ExposureMask)
		
		@x.map_window(@dpy, @clock_win)
		
		draw_clock
		
		eventloop
	end
	
	def draw_clock
		time = Time.now.httpdate
		@x.clear_window(@dpy, @clock_win)
		@x.draw_string(@dpy, @clock_win, 10, 20, time.to_s)
		@x.sync(@dpy)
	end
	
	def eventloop
		while true
			
			# use the X file descriptor as a timer for 
			# drawing the clock every second
			while @x.do_select_on_x_file_descriptor(1)
				draw_clock
			end
						
			@x.next_event(@dpy)
			
			case @x.event_type
				when WindowManagementConstants::KeyPress
					if @x.get_key_sym == WindowManagementConstants::XKey_q
						@x.destroy_window(@dpy, @clock_win)
						@x.close_display(@dpy)
						exit
					end
		
				when WindowManagementConstants::Expose
					draw_clock
			end
		end
	end
end

rc = RubyClock.new
