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

class Generic
	require '../src/WindowManagement'
	require '../src/WindowManagementConstants'
	
	def initialize
		@x = WindowManagement::X11.new

		@dpy = @x.open_display("")

		unless @dpy
			puts "I cannot open the display, sorry..."
			exit
		end

		@screen = @x.default_screen(@dpy)
		
		@root_window = @x.root_window(@dpy, @screen)
		
		@test_win = @x.create_simple_window(@dpy, @root_window, 5, 5, 175, 75, 1)
		@pix_win = @x.create_simple_window(@dpy, @test_win, 0, 0, 1, 1, 0)
		@pix1_win = @x.create_simple_window(@dpy, @test_win, 100, 25, 1, 1, 0)
			
		@x.store_name(@dpy, @test_win, "Pixmap Shape Test")
		
		mask =  WindowManagementConstants::KeyPressMask | WindowManagementConstants::ResizeRedirectMask
		
		@x.set_window_event_mask(@dpy, @test_win, mask)
	
		@x.map_window(@dpy, @test_win)

		@pixmap = @x.xpm_read_file_to_pixmap(@dpy, @pix_win, "pixmaps/my_pixmap.xpm")
		@pixmap1 = @x.xpm_read_file_to_pixmap(@dpy, @pix1_win, "pixmaps/close.xpm")
		
		@shape = @x.query_shape_extension(@dpy)
			
		if(@pixmap)
			@x.resize_window(@dpy, @pix_win, @pixmap["width"], @pixmap["height"])
			@x.resize_window(@dpy, @pix1_win, @pixmap1["width"], @pixmap1["height"])
			
			@x.map_subwindows(@dpy, @test_win)
			
			@x.set_window_background_pixmap(@dpy, @pix_win, @pixmap["pixmap"])
			@x.set_window_background_pixmap(@dpy, @pix1_win, @pixmap1["pixmap"])
			
			@x.clear_window(@dpy, @test_win)
			@x.clear_window(@dpy, @pix_win)
			@x.clear_window(@dpy, @pix1_win)

			if(@shape) 
				set_shape
			end
		end
		
		eventloop
	end
	
	def set_shape
		attr = @x.get_window_attributes(@dpy, @test_win)
		
		@shape = @x.create_simple_window(@dpy, @root_window, 0, 0, attr["width"], attr["height"], 0)
		
		@x.shape_combine_mask(@dpy, @pix_win, WindowManagementConstants::ShapeUnion,
			0,0, @pixmap["pixmap_mask"], WindowManagementConstants::ShapeSet)

		@x.shape_combine_shape(@dpy, @shape, WindowManagementConstants::ShapeBounding,
			0,0, @pix_win, WindowManagementConstants::ShapeUnion, WindowManagementConstants::YXBanded)

		@x.shape_combine_mask(@dpy, @pix1_win, WindowManagementConstants::ShapeUnion,
			0,0, @pixmap1["pixmap_mask"], WindowManagementConstants::ShapeSet)

		@x.shape_combine_shape(@dpy, @shape, WindowManagementConstants::ShapeBounding,
			100,25, @pix1_win, WindowManagementConstants::ShapeUnion, WindowManagementConstants::YXBanded)

		@x.shape_combine_shape(@dpy, @test_win, WindowManagementConstants::ShapeBounding,
			0,0, @shape, WindowManagementConstants::ShapeBounding, WindowManagementConstants::YXBanded)
	end
	
	def eventloop
		while true
			@x.next_event(@dpy)
		
			event = @x.event_type
		
			case event
				when WindowManagementConstants::KeyPress
					if @x.get_key_sym == WindowManagementConstants::XKey_q
						if(@pixmap)
							@x.free_pixmap(@dpy, @pixmap["pixmap"]);
							@x.free_pixmap(@dpy, @pixmap["pixmap_mask"])
							
							@x.free_pixmap(@dpy, @pixmap1["pixmap"]);
							@x.free_pixmap(@dpy, @pixmap1["pixmap_mask"])
						end
						
						@x.destroy_subwindows(@dpy, @test_win)						
						@x.destroy_window(@dpy, @test_win)
						@x.close_display(@dpy)
						exit
					end
				
				when WindowManagementConstants::ResizeRequest
					@need_shaped=true
					set_shape
					
			end # End case
		end
	end
end

pt = Generic.new
