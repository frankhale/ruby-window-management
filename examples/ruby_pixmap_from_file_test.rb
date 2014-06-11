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


class PixmapTest
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

		@x.store_name(@dpy, @test_win, "Pixmap Test")
		
		mask =  WindowManagementConstants::KeyPressMask			
		
		@x.set_window_event_mask(@dpy, @test_win, mask)
	
		@x.map_window(@dpy, @test_win)
		
		@pixmap = @x.xpm_read_file_to_pixmap(@dpy, @test_win, "pixmaps/my_pixmap.xpm")
			
		if(@pixmap)
			@x.set_window_background_pixmap(@dpy, @test_win, @pixmap["pixmap"])
			@x.clear_window(@dpy, @test_win)
		end
		
		eventloop
	end
	
	def eventloop
		while true
			@x.next_event(@dpy)
		
			case @x.event_type
				when WindowManagementConstants::KeyPress
					if @x.get_key_sym == WindowManagementConstants::XKey_q

						if(@pixmap)
						@x.free_pixmap(@dpy, @pixmap["pixmap"]);
						@x.free_pixmap(@dpy, @pixmap["pixmap_mask"])
						end
												
						@x.destroy_window(@dpy, @test_win)
						@x.close_display(@dpy)
						exit
					end
		
			end # End case
		end
	end
end

pt = PixmapTest.new
