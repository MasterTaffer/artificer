--[[
ArtifiCer lua:

Globals:

width, height
	Output image width and height. Describes the resolution of the rendering passes as well.
	If one of the dimensions is -1, it is automatically selected to match the input aspect ratio.
	If both of the dimensions are -1, the input image dimensions are used.

input_wrap
	Describes the OpenGL texture wrapping mode of the input image.
	This might or might not be honored depending on the input image dimensions.
	Possible values: Wrap_Clamp, Wrap_Repeat, Wrap_Mirror
	
passes
	Number of processing passes to perform. For each processing pass, a pass_shader must be defined.
	
pass_shader[i]
	Path to the GLSL fragment shader to use in rendering pass i. The numbering starts from 0
	These must be set.

pass_wrap[i]
	Describes the OpenGL texture wrapping mode for the framebuffer i texture. The numbering start
	from 0. 
	Possible values: Wrap_Clamp, Wrap_Repeat, Wrap_Mirror
	Please note that non power of 2 output dimensions will probably force Wrap_Clamp mode on.

uniforms[u_name]
	Table containing custom uniform definitions. These uniforms are passed to shaders with the key
	as the uniform name. The value may be a number or a function returning a number. 
	
Uniform functions:
	Functions may be assigned to the uniforms table as values. These functions are called with 1
	argument, a table with following key value pairs:
		frame: current frame number
		pass: current processing pass number
]]


width = -1
height = -1

input_wrap = Wrap_Clamp

passes = 2

pass_shader[0] = "main.fs"
pass_shader[1] = "extra.fs"

pass_wrap[0] = Wrap_Clamp
pass_wrap[1] = Wrap_Clamp


--Make up some stuff
print("Making this thing go")
bpm = 140
fps = 24

beatlen = 60 / 140

--Couple of functions
function curbeat(frame)	
	cursec = frame / fps
	return cursec / beatlen
end

function funy (x)
	return math.sin(4+math.floor(curbeat(x.frame) /8)*21)/30;
end

function funx (x)
	return math.cos(4+math.floor(curbeat(x.frame) /8)*21)/30;
end

function funblur (x)
	beatf = curbeat(x.frame) % 1
	
	return beatf*0.01
end

--Bind functions to uniforms
uniforms["brw"] = funblur
uniforms["posy"] = funy
uniforms["posx"] = funx
