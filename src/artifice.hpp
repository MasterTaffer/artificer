#pragma once
#include "artconf.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

typedef unsigned int uint;
typedef unsigned char ubyte;

struct FrameBuffer
{
    uint width;
    uint height;
    uint buffer;
    uint texture;
};

struct Uniform
{
    std::vector<uint> passes;
    uint current;
    uint framenum;

    uint width;
    uint height;
    std::vector<uint> custom;
};

struct Shader
{
    uint vs;
    uint fs;
    uint program;
};

struct Texture
{
    uint width;
    uint height;
    uint id;
};


class Artificer
{
public:

    //Important stuff:
    //
    //Fields are only for reading
    //
    //Methods might fill the field "errorb" with error information
    //
    //Methods you should use

    //init(const char* config, int inputW, int inputH, bool towindow = false, int texf = GL_RGBA)
    //Initialize

    //config: path to a proper Lua config
    //inputW & inputH: the dimensions of input data
    //towindow: whether to display the preview window
    //texf: color format for input & output. Supported values: GL_RGBA, GL_BGRA

    //After calling init() the following fields have meaningful values:
    //width, height: output width & height
    //passes: number of rendering passes


    //processFrame(unsigned char* source, unsigned char* to, int framenum)
    //Process a single frame

    //source: buffer containing (input width * input height) pixels
    //to: buffer with space for (output width * output height) pixels

    //deInit()
    //Deinitialize the system

    //Error code. 0 if no error
    int error;

    //Error message
    std::string errorb;


    //Number of rendering passes to perform

    //One could temporarily decrease the number of passes
    //by changing this. It might have some adverse effects
    int passes = 0;

    //Modifying these will  not have good results
    //Output width
    int width;

    //Output height
    int height;


    void init(const char* config, int inputW, int inputH, bool towindow = false, int texf = GL_RGBA);
    void processFrame(ubyte* source, ubyte* to, int framenum);
    void deInit();

    //All sorts of probably useless internal stuff below

    //Framebuffers
    std::vector<FrameBuffer> secbuffers;

    //Shaders
    std::vector<Shader> secshaders;

    //Shader uniform locations
    std::vector<Uniform> secuniforms;

    //Custom uniform bindings and names
    std::vector<Variant> uniformValues;
    std::vector<std::string> uniformNames;

    //Configuration unit
    ArtConf at;

    //Texture format used for input / output
    unsigned int texformat;
    bool outputToWindow = false;

    //GLFW window
    GLFWwindow* window = nullptr;

    //name of GL vertex buffer for simple screen filling polygon
    uint quadvbo;

    //Texture used for input
    Texture inputTexture;

    //shader for blitting the image to screen
    Uniform blitshader_uniforms;
    Shader blitshader;

    //Checks for GL error and sets the correct values
    void printerror();


    //Set error
    void seterror(const std::string&);

    //Finds all the uniforms for a program
    Uniform findUniforms(uint program);
};
