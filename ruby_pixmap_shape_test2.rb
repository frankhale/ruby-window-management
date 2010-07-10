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

puts "THIS PROGRAM IS NOT FINISHED AND MAY NOT WORK CORRECTLY"

class Generic
	require 'WindowManagement'
	require 'WindowManagementConstants'
	
	def initialize
		@x = WindowManagement::X11.new

		@dpy = @x.open_display("")

		unless @dpy
			puts "I cannot open the display, sorry..."
			exit
		end

		@screen = @x.default_screen(@dpy)
		
		@root_window = @x.root_window(@dpy, @screen)
		
		@app_width = 300
		@app_height = 200
		
		@title_width = 100
		@title_height = 1 # will equal the height of the title pixmap
	
		@test_win = @x.create_simple_window(@dpy, @root_window, 5, 5, @app_width, @app_height, 1)
		
		@title_win = @x.create_simple_window(@dpy, @test_win, 100, 75, @title_width, @title_height, 1)
		
		#@close_button_win = @x.create_simple_window(@dpy, @test_win, @title_width*2, 75, 1, 1, 0)

		#@iconify_button_win = @x.create_simple_window(@dpy, @test_win, 0, 0, 1, 1, 0)
		#@menu_button_win = @x.create_simple_window(@dpy, @test_win, 0, 0, 1, 1, 0)
		
		#@title_back_pixmap = @x.xpm_read_file_to_pixmap(@dpy, @title_win, "pixmaps/title-back.xpm")
		#@close_button_pixmap = @x.xpm_read_file_to_pixmap(@dpy, @close_button_win, "pixmaps/close.xpm")
		#@iconify_back_pixmap = @x.xpm_read_file_to_pixmap(@dpy, @iconify_button_win, "pixmaps/iconify.xpm")
		#@menu_back_pixmap = @x.xpm_read_file_to_pixmap(@dpy, @menu_button_win, "pixmaps/menu.xpm")	
			
		@x.store_name(@dpy, @test_win, "Pixmap Shape Test 2")
		
		mask =  WindowManagementConstants::KeyPressMask #| WindowManagementConstants::ResizeRedirectMask
		
		@x.set_window_event_mask(@dpy, @test_win, mask)
	
		@x.map_subwindows(@dpy, @test_win)
		@x.map_window(@dpy, @test_win)
		
		@x.clear_window(@dpy, @test_win)
		
		@shape = @x.query_shape_extension(@dpy)
			
		#@x.resize_window(@dpy, @title_win, 100, @title_back_pixmap["height"])
		#@x.resize_window(@dpy, @close_button_win, @close_button_pixmap["width"], @close_button_pixmap["height"])
						
		#@x.set_window_background_pixmap(@dpy, @title_win, @title_back_pixmap["pixmap"])
		#@x.set_window_background_pixmap(@dpy, @close_button_win, @close_button_pixmap["pixmap"])
			
		#@x.map_subwindows(@dpy, @title_win)
			
			
						
			
		
		#if(@shape) 
		#	set_shape
		#end
		
		eventloop
	end
	
	def set_shape
		#attr = @x.get_window_attributes(@dpy, @title_win)
		
		#@shape = @x.create_simple_window(@dpy, @root_window, 0, 0, attr["width"], attr["height"], 0)
		
		#@x.shape_combine_mask(@dpy, @close_button_win, WindowManagementConstants::ShapeBounding,
		#	0,0, @close_button_pixmap["pixmap_mask"], WindowManagementConstants::ShapeSet)
	
		#@x.shape_combine_shape(@dpy, @shape, WindowManagementConstants::ShapeBounding,
		#	@title_width-@close_button_pixmap["width"],0, @close_button_win, WindowManagementConstants::ShapeBounding, WindowManagementConstants::ShapeSet)


		#@x.shape_combine_mask(@dpy, @title_win, WindowManagementConstants::ShapeUnion,
		#	0,0, @title_back_pixmap["pixmap_mask"], WindowManagementConstants::ShapeSet)
	
		#@x.shape_combine_shape(@dpy, @shape, WindowManagementConstants::ShapeBounding,
		#	0,0, @title_win, WindowManagementConstants::ShapeBounding, WindowManagementConstants::YXBanded)


		# Apply the shape to the title window
		#@x.shape_combine_shape(@dpy, @title_win, WindowManagementConstants::ShapeBounding,
		#	0,0, @shape, WindowManagementConstants::ShapeBounding, WindowManagementConstants::YXBanded)


		## -- OLD WORKING CODE BELOW -- ##
		
		#@x.shape_combine_mask(@dpy, @pix_win, WindowManagementConstants::ShapeUnion,
		#	0,0, @pixmap["pixmap_mask"], WindowManagementConstants::ShapeSet)

		#@x.shape_combine_shape(@dpy, @shape, WindowManagementConstants::ShapeBounding,
		#	0,0, @pix_win, WindowManagementConstants::ShapeUnion, WindowManagementConstants::YXBanded)

		#@x.shape_combine_mask(@dpy, @pix1_win, WindowManagementConstants::ShapeUnion,
		#	0,0, @pixmap1["pixmap_mask"], WindowManagementConstants::ShapeSet)

		#@x.shape_combine_shape(@dpy, @shape, WindowManagementConstants::ShapeBounding,
		#	100,25, @pix1_win, WindowManagementConstants::ShapeUnion, WindowManagementConstants::YXBanded)

		#@x.shape_combine_shape(@dpy, @test_win, WindowManagementConstants::ShapeBounding,
		#	0,0, @shape, WindowManagementConstants::ShapeBounding, WindowManagementConstants::YXBanded)
	end
	
	def eventloop
		while true
			@x.next_event(@dpy)
		
			event = @x.event_type
		
			case event
				when WindowManagementConstants::KeyPress
					if @x.get_key_sym == WindowManagementConstants::XKey_q
						#@x.free_pixmap(@dpy, @title_back_pixmap["pixmap"]);
						#@x.free_pixmap(@dpy, @title_back_pixmap["pixmap_mask"])

						#@x.free_pixmap(@dpy, @close_button_pixmap["pixmap"]);
						#@x.free_pixmap(@dpy, @close_button_pixmap["pixmap_mask"])
						
						@x.destroy_subwindows(@dpy, @test_win)						
						@x.destroy_window(@dpy, @test_win)
						@x.close_display(@dpy)
						exit
					end
				
				#when WindowManagementConstants::ResizeRequest
					#@need_shaped=true
					#set_shape
					
			end # End case
		end
	end
end

pt = Generic.new
