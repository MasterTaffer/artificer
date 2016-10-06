#include "artifice.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>


#include <iomanip>
#include <sstream>

#include <fstream>

//Function for reading contents of a text file to a string
//Used for reading shaders

std::string readText(const std::string& f, const std::string& extraSearchDir)
{
    std::ifstream file(f, std::ifstream::binary);
    if (!file.is_open())
    {
        //Try again from the extra search dir
        file.open(extraSearchDir + f, std::ifstream::binary);
        if (!file.is_open())
        {
            std::string errmsg = "can't open file '"+ f +"'";
            throw errmsg.c_str();
        }
    }

    file.seekg(0, file.end);
    size_t len = file.tellg();
    file.seekg(0, file.beg);

    char* data = new char[len+1];
    if (data)
        file.read(data,len);
    data[len] = 0;

    std::string d(data,len);
    delete[] data;
    return d;
}

//Get the file path folder
std::string fileFolder(const std::string& f)
{
    size_t p = f.find_last_of("/\\");
    if (p == std::string::npos)
        return "";

    return f.substr(0,p+1);
}

//Update texture from data source
void updateTexture(Texture tex, ubyte* data, int texformat = GL_RGBA)
{
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,tex.width,tex.height,0,texformat,GL_UNSIGNED_BYTE,data);
}

//Set wrap modes for currently bound texture
void applyWrapMode(int wrapmode)
{
    auto t = GL_CLAMP_TO_EDGE;
    if (wrapmode == 1)
        t = GL_REPEAT;
    if (wrapmode == 2)
        t = GL_MIRRORED_REPEAT;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t);
}


void destroyTexture(const Texture& tex)
{
    glDeleteTextures(1,&tex.id);
}

//Generate a new texture
Texture generateTexture(int width, int height, ubyte* data, int wrapmode = 0, int filtering = GL_LINEAR, int texformat = GL_RGBA)
{
    Texture tex;
    glGenTextures(1,&tex.id);
    tex.width = width;
    tex.height = height;

    glBindTexture(GL_TEXTURE_2D, tex.id);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,texformat,GL_UNSIGNED_INT_8_8_8_8,data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    applyWrapMode(wrapmode);

    return tex;
}

void destroyShader(const Shader& s)
{
    glDeleteProgram(s.program);
    glDeleteShader(s.vs);
    glDeleteShader(s.fs);
}

Shader generateShader(const std::string& vs, const std::string& fs)
{

    Shader s;
    s.vs = glCreateShader(GL_VERTEX_SHADER);
    s.fs = glCreateShader(GL_FRAGMENT_SHADER);
    {
        const char* e = vs.c_str();
        int len = vs.size();
        glShaderSource(s.vs,1,&e,&len);
    }

    glCompileShader(s.vs);

    char buf[2560];


    int suc = 0;
    glGetShaderiv(s.vs, GL_COMPILE_STATUS, &suc);

    if (suc == GL_FALSE)
    {
        int d;
        glGetShaderInfoLog(s.vs,2559,&d,buf);
        std::stringstream ss;
        ss << "In vertex shader" << std::endl;
        ss << buf << std::endl;
        throw ss.str().c_str();
    }



    {
        const char* e = fs.c_str();
        int len = fs.size();
        glShaderSource(s.fs,1,&e,&len);
    }
    glCompileShader(s.fs);

    suc = 0;
    glGetShaderiv(s.fs, GL_COMPILE_STATUS, &suc);
    if (suc == GL_FALSE)
    {
        int d;
        glGetShaderInfoLog(s.fs,2559,&d,buf);
        std::stringstream ss;
        ss << "In fragment shader" << std::endl;
        ss << buf << std::endl;
        throw ss.str().c_str();
    }

    s.program = glCreateProgram();
    glAttachShader(s.program,s.fs);
    glAttachShader(s.program,s.vs);
    glLinkProgram(s.program);

    return s;
}

void destroyFramebuffer(const FrameBuffer& fb)
{
    glDeleteFramebuffers(1,&fb.buffer);
    glDeleteTextures(1,&fb.texture);
}

FrameBuffer generateFramebuffer(int width, int height, int wrapmode = 0, int filtering = GL_LINEAR, int texformat = GL_RGBA)
{
    FrameBuffer fb;
    glGenFramebuffers(1,&fb.buffer);
    glGenTextures(1,&fb.texture);

    fb.width = width;
    fb.height = height;

    glBindTexture(GL_TEXTURE_2D, fb.texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,texformat,GL_UNSIGNED_INT_8_8_8_8,nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    applyWrapMode(wrapmode);

    glBindFramebuffer(GL_FRAMEBUFFER,fb.buffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fb.texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return fb;
}

void Artificer::seterror(const std::string& e)
{
    std::stringstream ss;
    ss << errorb << "Error: " << e << std::endl;
    errorb = ss.str();
    error = 1;

}

void Artificer::printerror()
{
    auto i = glGetError();
    if (i != GL_NO_ERROR)
    {
        std::stringstream ss;
        ss << errorb << "GL Error: " << i << std::endl;
        errorb = ss.str();
        error = 1;
    }
}

Uniform Artificer::findUniforms(uint program)
{
    Uniform f;
    f.current = glGetUniformLocation(program, "current");
    for (int j = 0; j < passes; j++)
    {
        std::stringstream ss;
        ss << "pass_" << j;
        std::string name = ss.str();
        f.passes.push_back(glGetUniformLocation(program, name.c_str()));
    }
    for (size_t i = 0; i < uniformNames.size(); i++)
    {
        auto val =glGetUniformLocation(program, uniformNames[i].c_str());
        f.custom.push_back(val);

    }
    f.framenum = glGetUniformLocation(program, "frame");
    f.width = glGetUniformLocation(program, "width");
    f.height = glGetUniformLocation(program, "height");
    return f;
}

void Artificer::init(const char* config, int inputW, int inputH, bool towindow, int texf)
{
    window = nullptr;

    if (inputW <= 0 || inputH <= 0 || !config)
    {
        seterror("Invalid input parameters");
        return;
    }
    error = 0;
    texformat = texf;

    outputToWindow = towindow;
    try
    {
        at = ArtConfOpen(config);
    }
    catch (const char* e)
    {
        seterror(e);
        return;
    }


    //Get the extra search directory from the config directory
    std::string extraSearchDir = fileFolder(config);

    //Lua configurations:

    width = ArtConfGetGlobalInt(at,"width");
    height = ArtConfGetGlobalInt(at,"height");

    //If width / height are negative, automatically determine the components
    if (width <= 0 && height <= 0)
    {
        width = inputW;
        height = inputH;
    }
    else if (width <= 0)
    {
        width = inputW * height / inputH;
    }
    else if (height <= 0)
    {
        height = inputH * width / inputW;
    }


    //count of rendering passes
    passes = ArtConfGetGlobalInt(at,"passes");

    //the wrapmodes for rendering passes
    //nil is okay
    std::vector<int> wrapmodes;
    for (int i = 0; i < passes; i++)
    {
        wrapmodes.push_back(ArtConfGetArrayInt(at,"pass_wrap",i));
    }

    //get the fragment shaders
    std::vector<std::string> passshaders;
    for (int i = 0; i < passes; i++)
    {
        passshaders.push_back(ArtConfGetArrayString(at,"pass_shader",i));
    }

    //get the variant uniforms:
    //they might be a plain value or a function
    auto customUniforms = ArtConfGetVariantTable(at,"uniforms");
    for (auto b : customUniforms)
    {
        uniformNames.push_back(b.first);
        uniformValues.push_back(b.second);
    }

    //input wrapping mode
    int inputWrap = ArtConfGetGlobalInt(at,"input_wrap");

    //read the shaders
    std::string fsdef, vsdef;
    std::vector<std::string> fssec, vssec;

    try
    {
        {

            if (outputToWindow)
            {
                fsdef = readText("default.fs", extraSearchDir);
                vsdef = readText("defaultblit.vs", extraSearchDir);
            }

            for (int i = 0; i < passes; i++)
            {
                std::string frags = passshaders[i];
                std::string verts = "default.vs";

                fssec.push_back(readText(frags, extraSearchDir));
                vssec.push_back(readText(verts, extraSearchDir));
            }
        }
    }
    catch (const char* e)
    {
        seterror(e);
        return;
    }


    if(!glfwInit())
    {
        seterror("Failed to initialize glfw");
        return;
    }

    if (!outputToWindow)
        glfwWindowHint(GLFW_VISIBLE, 0);

    window = glfwCreateWindow(320, 240, "Artifice dummy", nullptr, nullptr);

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        seterror("Failed to initialize GLEW");
        return;
    }

    //if we're outputting to the window, we need a simple shader for that too
    if (outputToWindow)
    {
        try
        {
            blitshader = generateShader(vsdef,fsdef);
            blitshader_uniforms = findUniforms(blitshader.program);

        }
        catch (const char* ee)
        {
            seterror(std::string("While compiling blit shader: ")+std::string(ee));
            return;
        }
    }

    //generate framebuffers
    for (int i = 0; i < passes; i++)
    {
        secbuffers.push_back(generateFramebuffer(width,height,wrapmodes[i],GL_LINEAR,texformat));
        printerror();
    }

    //and shaders for them
    for (int i = 0; i < passes; i++)
    {
        try
        {
            secshaders.push_back(generateShader(vssec[i],fssec[i]));
            printerror();
            secuniforms.push_back(findUniforms(secshaders[i].program));
        }
        catch (const char* ee)
        {
            seterror(std::string("While compiling shader for pass ")+std::to_string(i)+": "+std::string(ee));
            return;
        }


    }

    //define a viewport filling quad
    float quad[] = {-1,1, -1,-1, 1,-1,   1,-1, 1,1, -1,1};

    //make a buffer for it
    glDisable(GL_CULL_FACE);
    glGenBuffers(1, &quadvbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadvbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * 2 * 2 * sizeof(float), quad, GL_STATIC_DRAW);

    //and enable the position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 *sizeof(float), nullptr);

    //and finally, generate a texture for the input image
    inputTexture = generateTexture(inputW,inputH,nullptr,inputWrap,GL_LINEAR,texformat);

}
void Artificer::processFrame(ubyte* source, ubyte* to, int framenum)
{
    error = 0;
    errorb.clear();
    glfwMakeContextCurrent(window);

    auto draw = []()
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    };

    //binds texture to unit 0 (which is bound to gl uniform "current")
    auto bindToCurrent = [](int id)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,id);
    };

    //binds all the framebuffer textures to units 1..n
    auto bindPassess = [&]()
    {
        for (int i = 0; i < passes; i++)
        {
            glActiveTexture(GL_TEXTURE1 + i);
            glBindTexture(GL_TEXTURE_2D, secbuffers[i].texture);
        }
    };


    int passnum = 0;

    FrameBuffer lastfb;

    //to suppress the "maybe unitialized" warning when
    //drawing lastfb to screen

    lastfb.texture = 0;

    //passes is always (or should be) above 0
    //so lastfb will be modified in the loop below

    //update the input texture from data source
    updateTexture(inputTexture,source,texformat);


    glViewport(0,0,width,height);
    glBindBuffer(GL_ARRAY_BUFFER, quadvbo);


    for (int i2 = 0; i2 < passes; i2++)
    {
        //bind buffer and correct shader
        glBindFramebuffer(GL_FRAMEBUFFER,secbuffers[i2].buffer);
        glUseProgram(secshaders[i2].program);

        {
            Uniform u = secuniforms[i2];

            //The values passed to Lua on variant function calls
            std::vector<std::pair<double,std::string>> frameParams;
            frameParams.push_back({framenum,"frame"});
            frameParams.push_back({passnum,"pass"});

            glUniform1i(u.current, 0);
            for (size_t ij = 0; ij < u.passes.size(); ij++)
            {
                glUniform1i(u.passes[ij], ij+1);
            }

            //custom uniforms, variant function calls
            for (size_t j = 0; j < u.custom.size(); j++)
            {
                if (u.custom[j] >= 0)
                {
                    double var = ArtConfVariant(at,uniformValues[j],frameParams);
                    glUniform1f(u.custom[j], var);
                }
            }

            glUniform1i(u.framenum, framenum);
            glUniform1i(u.width, width);
            glUniform1i(u.height, height);
        }

        bindToCurrent(inputTexture.id);

        bindPassess();

        draw();

        lastfb = secbuffers[i2];
        passnum += 1;
    }

    //and read pixels to output buffer
    glReadPixels(0,0,width,height,texformat,GL_UNSIGNED_BYTE,to);


    if (outputToWindow)
    {
        int owidth;
        int oheight;
        //In case the window was resized
        glfwGetWindowSize(window,&owidth,&oheight);

        //Bind the screen
        glBindFramebuffer(GL_FRAMEBUFFER,0);

        //use the correct program and bind the uniform jic
        glUseProgram(blitshader.program);
        glUniform1i(blitshader_uniforms.current, 0);

        //in case the window was resized...
        glViewport(0,0,owidth,oheight);

        //get the texture from the last framebuffer drawn to
        bindToCurrent(lastfb.texture);

        draw();

        //swap & poll
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}

void Artificer::deInit()
{
    ArtConfClose(at);

    //if initialization had failed, the window is a nullptr
    if (window)
    {

        glDeleteBuffers(1, &quadvbo);

        glfwMakeContextCurrent(window);

        if (outputToWindow)
            destroyShader(blitshader);

        for (const Shader& r : secshaders)
            destroyShader(r);

        for (const FrameBuffer& r : secbuffers)
            destroyFramebuffer(r);

        destroyTexture(inputTexture);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

