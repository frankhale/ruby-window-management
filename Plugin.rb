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

module PluginTypes
	Core = 1
	Initialization = 2	# Possibly for some config plugin
end

class Plugin
	attr_reader :type, :respond_to_events
	attr_accessor :name

	# We need to pass the plugin the window manager object so we can have access
	# to window manager specific stuff...
	def initialize(wm, n)
		@name = n
		@type = "" # eventually for specific types of plugins
		@wm=wm
		
		# For convenience
		@x=@wm.x
		@dpy = @wm.dpy
		@root_window = @wm.root_window
		
		# So far this is the meat and potatoes of the plugin system
		# Basically the plugin assigns an event type with a method handler
		# This allows for a pretty damn flexible plugin architechure
		@respond_to_events={}
		
		puts "Plugin #{@name} Initialized..."
	end

	def unload
		# Would do any cleaning up in here...
		puts "Plugin #{@name} Unloaded..."
	end
end
