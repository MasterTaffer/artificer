#version 330 core
//GLSL fragment shader


out vec4 color; //output color

in vec2 texCoord; //input texture coordinate

//Internal uniforms
uniform sampler2D current; //current input image
uniform int frame; //current frame number

uniform int width; //processing width
uniform int height;	//processing height
	
//There is a sampler2D uniform for each pass
//It is unadvisable to access the texture of the current pass
uniform sampler2D pass_0; //texture access to pass 0
uniform sampler2D pass_1; //texture access to pass 1


//The rendering passes are performed sequentially, starting from 0:
//If you access the pass_3 texture during the pass_0, the pass_3 framebuffer contains
//image data from the previous frame, allowing temporal effects.

//Uniform defined in the configuration file
uniform float brw; //psychedelic blur length

void main()
{
	//Just do some generic fragment shader fun
	//for great, psychedelic effects
	
	//For making the image a little wavy
	vec2 ot;
	ot.x = texCoord.x + sin(texCoord.y*18)/240;
	ot.y = texCoord.y + sin(texCoord.x*18)/240;
	
	float val = cos(texCoord.x*24) / 233 + sin(texCoord.y*24) /233;
	ot += val;
	color = 10 *texture(current,ot);
		
	//Create a slight directional blur
	
	//Make the blur direction rotate over time
	float xm, ym;
	xm = sin(frame/40.0+texCoord.x*0.7 + texCoord.y *1.5);
	ym = cos(frame/40.0-texCoord.y*0.9 + texCoord.x * 0.6);
	
	for (int i = -2; i <= 8; i++)
	{
		color += smoothstep(0.3,0.6,texture(current,texCoord + vec2(xm * i * brw, ym * i * brw)));
		color += smoothstep(0.3,0.6,texture(pass_1,texCoord + vec2(xm * i * brw, ym * i * brw)));
	}
	color /= 22;
	
}

