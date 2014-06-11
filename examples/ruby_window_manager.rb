#!/usr/bin/ruby
#
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
# Updated: 10 June 2014

require '../src/WindowManagement'
require '../src/WindowManagementConstants'
require './plugin'

# Eventually these will be filled in from a configuration file
$user_plugins = [ "StartupList" ]
$core_plugins = [ "ClientManager", "SimpleWindowDecoration" ]
$startup_list = [ "xterm -sb -ls -fg white -bg black" ]

class StartupList < Plugin

  def initialize(wm)
	  super(wm, "StartupList")
		@type = PluginTypes::Core
		
		# Each plugin has it's list of events it wants to respond to.
		@respond_to_events = {
			"Initialize" => "process_startup_list"
		}
	end
	
	def process_startup_list
		$startup_list.each { |app|
			IO.popen("#{app}")
			puts "Launching : #{app}"		
		}
	end
end

class SimpleWindowDecoration < Plugin
	class Frame
		attr_reader :frame, :window, :transient_for, :name
		
		def initialize(wm, win)
			@@border_width = 1
			
			@wm = wm
			@x = wm.x
			@dpy = wm.dpy
      @root_window = wm.root_window
			@name = @x.fetch_name(@dpy, win)
			
			if(@name==nil)
				@name="no name"
			end
			
			@window = win
			
			@transient_for = @x.get_transient_for_hint(@dpy, win)
			
			attr = @x.get_window_attributes(@dpy, win)
			
			@win_x=attr["x"]
			@win_y=attr["y"]
			@win_width=attr["width"]
			@win_height=attr["height"]
			
			@frame_padding=16

			@frame_x=@win_x
			@frame_y=@win_y
			@frame_width=@win_width+@frame_padding
			@frame_height=@win_height+@frame_padding
			
			mask =  WindowManagementConstants::SubstructureRedirectMask	|
				WindowManagementConstants::SubstructureNotifyMask  	|
				WindowManagementConstants::ButtonPressMask		|
				WindowManagementConstants::ButtonReleaseMask		|
				WindowManagementConstants::EnterWindowMask		|
				WindowManagementConstants::LeaveWindowMask		|
				WindowManagementConstants::ButtonMotionMask		|
				WindowManagementConstants::PropertyChangeMask		|
				WindowManagementConstants::KeyPressMask			|
				WindowManagementConstants::FocusChangeMask		|
				WindowManagementConstants::ExposureMask
						
			@frame = @x.create_simple_window(@dpy, @root_window, @frame_x, @frame_y, @frame_width, @frame_height, @@border_width)

			@x.set_window_border_width(@dpy, win, 0)

			@x.set_window_event_mask(@dpy, @frame, mask)
		
			@x.reparent_window(@dpy, win, @frame, @frame_padding/2, @frame_padding/2)
			
			@x.map_window(@dpy, @frame)

			@x.move_window(@dpy, @frame, 100, 100)

			expose
		end
		
		def destroy()
			#attr = @x.get_window_attributes(@dpy, @window)
			#@x.reparent_window(@dpy, @window, @root_window, attr["x"], attr["y"])
			
			@x.reparent_window(@dpy, @window, @root_window, @frame_x+(@frame_padding/2),@frame_y+(@frame_padding/2))
			@x.destroy_window(@dpy, @frame)
		end
		
		def moveresize(x, y, width, height)
			@x.resize_window(@dpy, @window, width-@frame_padding, height-@frame_padding)
			@x.move_resize_window(@dpy, @frame, x, y, width, height)
		end
		
		def expose
		end
	end

	def initialize(wm)
		super(wm, "SimpleWindowDecoration")
		@type = PluginTypes::Core
		
		@frames = {}

		# colors used for the focused and unfocused border	
		@focus_color = @x.alloc_named_color(@dpy, "#3f5881")
		@unfocus_color = @x.alloc_named_color(@dpy, "#dddddd")
		
		# Each plugin has it's list of events it wants to respond to.
		@respond_to_events = {
			"FrameButtonPress" => "button_handler",
			"FrameButtonRelease" => "button_handler",
			"ReparentExisting" => "reparent_existing",
			"ReparentNew" => "reparent_new",
			"RemoveParent" => "remove_parent",
			"GeometryChangeNotify" => "geometry_change",
			"FrameFocusIn" => "focus",
			"Expose" => "expose",
			"UnmapNotify" => "unmap_notify"
		}
	end

	def focus
		if(@wm.focus_window)
			if(@frames.key?(@wm.focus_window["window"]))
				frame = @frames.fetch(@wm.focus_window["window"])
						
				if(@wm.focus_window["status"]=="focused")
					@x.set_window_background(@dpy, frame.frame, @focus_color["pixel"])
					@x.clear_window(@dpy, frame.frame)
					
					@wm.focus_window = nil
				end
			end
		end

		if(@wm.unfocus_window)
			if(@frames.key?(@wm.unfocus_window["window"]))
				frame = @frames.fetch(@wm.unfocus_window["window"])
				
				if(@wm.unfocus_window["status"]=="unfocused")
					@x.set_window_background(@dpy, frame.frame, @unfocus_color["pixel"])			
					@x.clear_window(@dpy, frame.frame)
					
					@wm.unfocus_window = nil
				end
			end
		end
	end

	def expose
		ex = @x.get_expose_event
		
		@frames.each { |win,f|
			if(f.frame == ex["window"])
				f.expose
			end
		}
	end

	def get_client_name(frame)
		frame.name = @x.fetch_name(@dpy, frame.window)
	end
	
	def button_handler
		button_event = @x.get_button_event
		
		@x.allow_events(@dpy, WindowManagementConstants::ReplayPointer)
		
		if(button_event["type"]==WindowManagementConstants::ButtonPress)
			@frames.each { |win, f|
				if((button_event["window"] == f.window) or (button_event["window"] == f.frame))
					@wm.current_frame = { "frame" => f.frame,
						      "window" => f.window }
						      
					@x.set_input_focus(@dpy, f.window)
				end
			}
		end
		
		if(button_event["type"]==WindowManagementConstants::ButtonRelease)
			@wm.current_frame = nil
		end
	end
	
	def reparent_existing
		# get @wm.app_existing_windows to see what windows exist prior to mapping...
		@wm.existing_app_windows.each { |w| 
		
			unless(@frames.key?(w))
				f = Frame.new(@wm, w)
		
				@frames.store(w, f)
			end
		}
	end
			
	def reparent_new
		new_frame = @x.get_map_request_event

		unless(@frames.key?(new_frame["window"]))
			f = Frame.new(@wm, new_frame["window"])
		
			@frames.store(new_frame["window"], f)
		end
	end
	
	def unmap_notify
		#unmap_notify = @x.get_unmap_notify_event
		
		#if (@frames.key?(unmap_notify["window"]))
			#puts "got an unmap_notify for #{unmap_notify["window"]}"
			#@x.unmap_window(@dpy, @frames.fetch(unmap_notify["window"].frame))	
		#end
	end
	
	def remove_parent		
		destroy_window_event = @x.get_destroy_window_event
		
		if (@frames.key?(destroy_window_event["window"]))
			
			frame = @frames.fetch(destroy_window_event["window"])
			
			if(@frames.key?(frame.window))
				frame.destroy
			
				@frames.delete(frame)
			end
		end
	end
	
	def geometry_change
		if (@wm.window_geometry_changing)
			if(@frames.key?(@wm.window_geometry_info["window"]))
				frame = @frames.fetch(@wm.window_geometry_info["window"])
				
				geo = @wm.window_geometry_info
				
				frame.moveresize(geo["x"], geo["y"],geo["width"],geo["height"])
			end		
		end
	end
end

class ClientManager < Plugin
	
	# This will represent each individual client, just a simple
	# class to hold only what's needed
	class Client
		attr_accessor :window
		
		def initialize(win)
			@window=win
		end
	end
	
	def initialize(wm)
		super(wm, "ClientManager")
		@type = PluginTypes::Core
		
		# Each plugin has it's list of events it wants to respond to.
		@respond_to_events = { 
			"ClientButtonNotify" => "button_handler",
			"Initialize" => "query_window_tree",
			"MapRequest" => "map_request",
			"DestroyNotify" => "destroy_notify",
			"FocusIn" => "focus",
			"MotionNotify" => "motion_notify"
		}
		
		@clients = {} # hash of clients where window id points to a client object

		# Used to keep track of the current focused client
		# the -1 is stored in it's window property and is used
		# to denote that we currently don't have a focused client.
		@focused_client = Client.new(-1)
	end

	def new_client(win)
		c = Client.new(win)
		@clients.store(win, c)

		@x.grab_button(@dpy, 1, WindowManagementConstants::ShiftMask, c.window, WindowManagementConstants::ButtonPressMask)
		@x.grab_button(@dpy, 3, WindowManagementConstants::ShiftMask, c.window, WindowManagementConstants::ButtonPressMask)
			
		@x.map_window(@dpy, c.window)
		@x.select_input(@dpy, c.window, WindowManagementConstants::FocusChangeMask)

		@wm.app_windows.push(win) # lets tell the WM about what windows are actual apps
	end

	def button_handler
		button_event = @x.get_button_event
		
		if (button_event["window"]!=@root_window) 
		
			unless(@focused_client.window == button_event["window"])
				if(button_event["type"] == WindowManagementConstants::ButtonPress)	
					unless(button_event["window"]==@root_window)
						if(@clients.key?(button_event["window"]))
					
							if(@wm.current_frame==nil)
								@wm.current_frame = { "frame" => button_event["window"],
						      			"window" => button_event["window"] }
							end
						
							@x.set_input_focus(@dpy, @clients.fetch(button_event["window"]).window)		
						end
					end
				end
			end
		
			if (button_event["type"] == WindowManagementConstants::ButtonPress)
			
				if(@wm.current_frame)
					@x.raise_window(@dpy, @wm.current_frame["frame"])
						
					@x.grab_pointer(@dpy, @wm.current_frame["frame"], WindowManagementConstants::PointerMotionMask|WindowManagementConstants::ButtonReleaseMask)
			
					@attr = @x.get_window_attributes(@dpy, @wm.current_frame["frame"])
					
					@start=button_event
				end
			
			elsif(button_event["type"] == WindowManagementConstants::ButtonRelease)	
				@x.ungrab_pointer(@dpy)
			
				@wm.window_geometry_changing=false
			
				@wm.current_frame=nil
			end
		end
	end
	
	def focus
		focus_change = @x.get_focus_change_event

		if(@focused_client.window!=focus_change["window"] and focus_change["window"]!=@root_window)
			if (focus_change["type"] == WindowManagementConstants::FocusIn)
				if(@clients.key?(focus_change["window"]))
					
					window_list = @wm.app_windows #@x.query_tree(@dpy, @root_window)

					# see if these windows are still in X's window list
					event_window = check_sync(window_list, focus_change["window"])
					focus_window = check_sync(window_list, @focused_client.window)

					if(event_window or focus_window)
						if(focus_window)
							
							@wm.unfocus_window = { "status" => "unfocused",
								"window" => @focused_client.window }
						end
						
						@focused_client=@clients.fetch(focus_change["window"])
					
						@wm.focus_window = { "status" => "focused",
							"window" => @focused_client.window }
					end
				end
			end
		end
	end

	def destroy_notify
		destroy_window_event = @x.get_destroy_window_event
		
		if(@clients.key?(destroy_window_event["window"]))
			if(@focused_client.window==destroy_window_event["window"])
				@focused_client.window=-1
			end
						
			@clients.delete(destroy_window_event["window"])
			
			@wm.app_windows.delete(destroy_window_event["window"]) # update the WM's list of app windows
		end
	end
	
	def query_window_tree
		window_list = @x.query_tree(@dpy, @root_window)

		window_list.each { |win|
			new_client(win)
			@wm.existing_app_windows.push(win)
		}
	end

	def map_request
		map_request_event = @x.get_map_request_event

		unless(@clients.key?(map_request_event["window"]))
			new_client(map_request_event["window"])
		end
	end

	# Needed for focus events to determine if the window that received the event
	# is still in X's window tree. Lots of the time the events for Focus come out of order
	# like after a DestroyNotfiy. 
	def check_sync(window_list, check_win)
		window_list.each { |w|
			if(w==check_win)
				return true
			end
		}
		
		return false
	end
	
	def motion_notify
		motion_event = @x.get_motion_event
		
		if(@wm.current_frame)
			if (@start)
				unless(@wm.window_geometry_changing==true)
					@wm.window_geometry_changing=true
				end
					
				xdiff = motion_event["x_root"] - @start["x_root"]
				ydiff = motion_event["y_root"] - @start["y_root"]
									
				move_x = @attr["x"] + (@start["button"]==1 ? xdiff : 0)
				move_y = @attr["y"] + (@start["button"]==1 ? ydiff : 0)
				move_width = [100, @attr["width"] + (@start["button"]==3 ? xdiff : 0)].max
				move_height = [100, @attr["height"] + (@start["button"]==3 ? ydiff : 0)].max

				@wm.window_geometry_info = {
					"window" =>  @wm.current_frame["window"],
					"x" => move_x,
					"y" => move_y,
					"width" => move_width,
					"height" => move_height
				}
			end
		end
	end
end

class WM
	# Allows plugins to access some WM specific info...
	attr_reader :x, :name, :dpy, :root_window, :screen
	attr_accessor :existing_app_windows, :app_windows, :window_geometry_changing, :window_geometry_info, 
		      :current_frame, :focus_window, :unfocus_window

	def initialize
		@name="Ruby Window Manager"
		@plugins = []
		@app_windows = []
		@existing_app_windows = [] # existed prior to WM starting
		
		@window_geometry_changing=false
		@window_geometry_info={}
		
		@focus_window = {}
		@unfocus_window = {}
		
		@current_frame= nil
		
		init
		event_loop
	end

	def init
		@x = WindowManagement::X11.new
		
		@dpy = @x.open_display("")
		
		if @dpy then
			puts "Starting #{@name}..."
		end

    @screen = @x.default_screen(@dpy)
		@root_window = @x.root_window(@dpy, @screen)

		@leftArrowCursor = @x.create_font_cursor(@dpy, WindowManagementConstants::XC_left_ptr)

		@x.define_cursor(@dpy, @root_window, @leftArrowCursor)

		mask =  WindowManagementConstants::SubstructureRedirectMask	|
			WindowManagementConstants::SubstructureNotifyMask  	|
			WindowManagementConstants::ButtonPressMask		|
			WindowManagementConstants::ButtonReleaseMask		|
			WindowManagementConstants::EnterWindowMask		|
			WindowManagementConstants::LeaveWindowMask		|
			WindowManagementConstants::ButtonMotionMask		|
			WindowManagementConstants::PropertyChangeMask		|
			WindowManagementConstants::KeyPressMask			|
			WindowManagementConstants::FocusChangeMask		|
			WindowManagementConstants::ExposureMask
		
		@x.set_window_event_mask(@dpy, @root_window, mask)
		
		@shape = @x.query_shape_extension(@dpy)
		
		load_plugins($core_plugins)
		load_plugins($user_plugins)
		
		# Here we could call dispatch on any Initialization plugins
		plugin_dispatch(PluginTypes::Initialization, "Initialize")
		plugin_dispatch(PluginTypes::Core, "Initialize")
		plugin_dispatch(PluginTypes::Core, "ReparentExisting")
	end
	
	def load_plugins(plugin_list)
		# Load Plugins
		plugin_list.each { 
			|plugin| 
			p = Object.class_eval(plugin).new(self)
			@plugins.push(p) # push the object into an array so 
					 # we can call methods on it later
		}
	end
	
	def shutdown_wm
		# shutdown plugins
		plugin_dispatch(PluginTypes::Core, "Shutdown")
		plugin_dispatch(PluginTypes::Initialization, "Shutdown")
		
		# clean up X
		@x.free_cursor(@dpy, @leftArrowCursor)
		@x.close_display(@dpy)

		puts "Shutting down #{@name}..."

		exit
	end
	
	# This function dispatches events to plugins
	def plugin_dispatch(type, event)
		@plugins.each { 
			|plugin|  
		
			if (plugin.type==type)
				if(plugin.respond_to_events.has_key?(event))
					plugin.send plugin.respond_to_events[event]
				end
			end
		}
	end
	
	def event_loop
		while true 
			trap("INT") {
				shutdown_wm				
			}
			
			@x.next_event(@dpy)
			
			type = @x.event_type
			
			case type
				when WindowManagementConstants::KeyPress
					if @x.get_key_sym == WindowManagementConstants::XKey_F1
						shutdown_wm
					end
				
				when WindowManagementConstants::DestroyNotify
					plugin_dispatch(PluginTypes::Core, "RemoveParent")
					plugin_dispatch(PluginTypes::Core, "DestroyNotify")
				
				when WindowManagementConstants::FocusIn
					plugin_dispatch(PluginTypes::Core, "FocusIn")
					plugin_dispatch(PluginTypes::Core, "FrameFocusIn")
					
				when WindowManagementConstants::FocusOut
					plugin_dispatch(PluginTypes::Core, "FocusOut")
					plugin_dispatch(PluginTypes::Core, "FrameFocusOut")
				
				when WindowManagementConstants::MotionNotify
					plugin_dispatch(PluginTypes::Core, "MotionNotify")
					plugin_dispatch(PluginTypes::Core, "GeometryChangeNotify")
									
				when WindowManagementConstants::ButtonPress
					plugin_dispatch(PluginTypes::Core, "FrameButtonPress")
					plugin_dispatch(PluginTypes::Core, "ClientButtonNotify")
					plugin_dispatch(PluginTypes::Core, "ButtonPress")
											
				when WindowManagementConstants::ButtonRelease
					plugin_dispatch(PluginTypes::Core, "FrameButtonRelease")
					plugin_dispatch(PluginTypes::Core, "ClientButtonNotify")
					plugin_dispatch(PluginTypes::Core, "ButtonRelease")
									
				when WindowManagementConstants::EnterNotify
					plugin_dispatch(PluginTypes::Core, "EnterNotify")
				
				when WindowManagementConstants::MapRequest
					map_request = @x.get_map_request_event
					attr = @x.get_window_attributes(@dpy, map_request["window"])
					
					name = @x.fetch_name(@dpy, map_request["window"])
					
					#unless((attr["override_redirect"] == true) and 
          unless ((attr["map_state"] == WindowManagementConstants::IsUnviewable) and (attr["map_state"] == WindowManagementConstants::IsUnmapped))
          	  plugin_dispatch(PluginTypes::Core, "MapRequest")
							plugin_dispatch(PluginTypes::Core, "ReparentNew")
					end
				
				when WindowManagementConstants::Expose
					plugin_dispatch(PluginTypes::Core, "Expose")
				
				when WindowManagementConstants::UnmapNotify
					plugin_dispatch(PluginTypes::Core, "UnmapNotify")
				
				when WindowManagementConstants::ConfigureRequest
					plugin_dispatch(PluginTypes::Core, "ConfigureRequest")
					# satisfy xterms dumb ass lets call XConfigureWindow
					# and let it do its magic... Otherwise it'd have a 
					# size of (1,1) which is just stupid as no other app
					# seems to need to have a configure request before
					# mapping
					@x.pass_down_configure_request(@dpy)	
				else
					if(type == @shape["shape_event"])
						puts "Got shape event : #{@shape["shape_event"]}"
						
						plugin_dispatch(PluginTypes::Core, "ShapeNotify")
					end
			end
		end
	end
end

wm = WM.new
