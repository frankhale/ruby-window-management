# A small extension to provide access to a subset of Xlib from Ruby 
# Copyright (C) 2011 Frank Hale <frankhale@gmail.com>
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
# Updated: 27 Feb 2011

class Generic
	require '../src/WindowManagement'
	require '../src/WindowManagementConstants'
	
	def initialize
		@x = WindowManagement::X11.new
				
		@dpy = @x.open_display(":0")

		@screen = @x.default_screen(@dpy)
		
		unless @dpy
			puts "I cannot open the display, sorry..."
			exit
		end

		@root_window = @x.root_window(@dpy, @screen)
		
		@test_win = @x.create_simple_window(@dpy, @root_window, 5, 5, 640, 480, 1)

		@x.store_name(@dpy, @test_win, "Window")
		
		mask =  WindowManagementConstants::KeyPressMask			
		
		@x.set_window_event_mask(@dpy, @test_win, mask)
	
		@x.map_window(@dpy, @test_win)
		
		eventloop
	end
	
	def eventloop
	
		while true
			@x.next_event(@dpy)
		
			case @x.event_type
				when WindowManagementConstants::KeyPress
					if @x.get_key_sym == WindowManagementConstants::XKey_q
						@x.destroy_window(@dpy, @test_win)
						@x.close_display(@dpy)
						exit
					end
		
			end # End case
		end
	end
end

pt = Generic.new
