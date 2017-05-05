# artificer

**OpenGL video/image processing utility.**

* A rather simple interface for processing frames/images with OpenGL pixel shaders.
* Multiple rendering passes per frame
* Somewhat configurable with Lua

##Dependencies

* GLFW 3, used for creating the OpenGL context
* GLEW, used for wrangling them OpenGL extensions
* Lua 5.3, used for configuration language
* C++11 standard library

The command line interface also uses:
* [stb_image.h & stb_image_write.h](https://github.com/nothings/stb)
* [tinydircpp](https://github.com/iamOgunyinka/tinydircpp)

## Compilation

You can use CMake, and it might straight up work. In case it doesn't, you can manually compile the command line interface:

* Compile artconf.cpp, cartifice.cpp, framesource.cpp, gl.cpp and main.cpp with C++11 standard
* The Lua (preferably 5.3), GLFW and GLEW includes must be available
* Link with Lua, GLFW and GLEW

To compile without the command line interface, simply leave the framesource.cpp and main.cpp out.

## Usage

Documentation of the API is within [cartifice.h](src/cartifice.h). Usage example can be found in the command line interface example ([main.cpp](src/main.cpp)).
Check the usage of artificer_start(), artificer_process() and artificer_end().

Documentation on the configuration scripts can be found in [test.lua](test/test.lua). OpenGL uniforms and such are demonstrated within [main.fs](test/main.fs) and [extra.fs](test/extra.fs). 

**Other things of note**

The program requires a default vertex shader, "default.vs". A "default.fs" might be used too, if no fragment shader is specified for a pass. "defaultblit.vs" and "default.fs" must be present if using the display output functionality (which displays the frame output in a window).

These are provided in the [test](test) folder, and it will hardly be necessary to modify them.



