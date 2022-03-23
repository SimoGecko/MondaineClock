# Mondaine Clock
A simple desktop clock widget inspired by the original [Swiss Mondaine Railway](https://ch.mondaine.com/) design.

![Clock](mondaine_clock.gif)


### Running:
Download `MondaineClock.exe` and double-click on it to start it. That's it!  

#### Parameters:
You can optionally pass in the following additional parameters to control the behavior:  
	- `size value`: controls the size of the clock (200 by default).  
	- `posx value`: controls the start X position (lower right corner by default).  
	- `posy value`: controls the start Y position (lower right corner by default).  
	- `framelimit value`: limits the maximum framerate (30 by default).  
	- `sleeptimems value`: amount of milliseconds to wait after each iteration (200 by default = 5fps).  
	- `stop2go 0/1`: turns on or off the stop2go behavior, with the seconds hand briefly waiting at 00 (on by default).  

For example, use `MondaineClock.exe size 200 posx 10 posy 10 stop2go 1` on the commandline.

### Building:
You need to change `sfml` and `extralibs` in `Variables.props` to point to the correct locations on your system.  
Then build `MondaineClock.sln`. The required libraries are statically built, so at the end you can just open `<Config>\MondaineClock.exe`.  

### Todo:
- Make the window tray only.
- Improve the window border anti-aliasing.