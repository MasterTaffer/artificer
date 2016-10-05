
#include <string>
#include <vector>
#include "gl.hpp"

#include "cartifice.h"
#include "framesource.hpp"

#include <iomanip>
#include <sstream>
#include <iostream>

struct Arguments
{
    std::string config;
    std::string frameInput;
    std::string frameOutput;
    bool displayOutput = false;
};

//returns a positive number on success,
int doStringArg(int argc, const char** argv, std::string* res)
{
    argc--;
    argv++;
    if (argc == 0)
        return -1;
    *res = std::string(*argv);
    return 2;
}

//returns -1 on success,
//otherwise returns the index of the illegal parameter
int doArgs(int argc, const char** argv, Arguments& arg)
{

    //simply go through all the arguments, and match the "dash character"

    //some fancy pointer hax
    std::vector<std::pair<char, std::string*>> stringargs;
    stringargs.push_back({'c',&arg.config});
    stringargs.push_back({'i',&arg.frameInput});
    stringargs.push_back({'o',&arg.frameOutput});

    std::vector<std::pair<char, bool*>> boolargs;
    boolargs.push_back({'d',&arg.displayOutput});

    //store the argument count
    int fullargc = argc;

    //for every argument we decrement the argc and increment argv
    while (argc)
    {
        if (argv[0][0] != '-')
        {
            return -1;
        }

        //we only use the second character
        char c = argv[0][1];
        if (c == 0)
            return -1;

        //we also demand that the argument is exactly 2 characters long
        if (argv[0][2] != 0)
            return -1;

        //just try and match every possible choice
        for (auto& p : stringargs)
        {
            if (p.first == c)
            {
                int off = doStringArg(argc,argv,p.second);
                if (off <= 0)
                    return fullargc - argc;
                argc -= off;
                argv += off;
                c = 0;
                break;
            }
        }

        //use c==0 as break value
        if (!c)
            continue;

        for (auto& p : boolargs)
        {
            if (p.first == c)
            {
                *(p.second) = true;

                int off = 1;
                argc -= off;
                argv += off;
                c = 0;
                break;
            }
        }
        if (!c)
            continue;


        return fullargc - argc;
    }
    return -1;
}

void printUsage()
{
    std::cout << "Usage" << std::endl;
    std::cout << "   artificer -c config -i inputfolder [-o outputprefix] [-d]"<< std::endl;
    std::cout << ""<< std::endl;
    std::cout << "  d: display output preview"<< std::endl;
}



void doGL(int argc, const char** argv)
{
    if (argc <= 1)
        return printUsage();

    Arguments args;
    argc--;
    argv++;

	//Argument parsing...
    int ret = doArgs(argc, argv, args);

    if (ret != -1)
    {
        std::cout << "Illegal argument " << argv[ret] << std::endl;
        return printUsage();
    }

    if (!args.config.size() || !args.frameInput.size())
    {
        return printUsage();
    }

    std::string config = args.config;

    std::cout << "Loading configuration from " << config <<  std::endl;

    std::string framesource = args.frameInput;
    std::string frameout = args.frameOutput;

    FrameSource fs(framesource);


    artificer_parameters params = artificer_default_parameters();

    //The default parameters aren't fun at all

    //config_path must be changed
    params.config_path = config.c_str();

    //input width and height must be changed as well
    params.width = fs.getWidth();
    params.height = fs.getHeight();

    //the default color format is rgba, but just for the show (and compatibility)
    params.color_format = artificer_color_format_rgba;

    //we want the display window
    if (args.displayOutput)
   		params.display_output = 1;
	else
		params.display_output = 0;

    std::cout << "Input: " << params.width << ", " << params.height << std::endl;

    void *art;
    art = artificer_start(&params);

    //the artificer_start modifies params a bit
    //providing us the output_width and output_height
    //and errors, if any occurred

    if (params.error > 0)
    {
        std::cout << params.error_string << std::endl;
        return;
    }

    //input buffer must contain space for all input pixels
    std::vector<unsigned char> ibuf;
    ibuf.resize(fs.getWidth()*fs.getHeight()*4); //times 4 because RGBA pixels are 4 bytes

    //we could use the same input and output buffers
    //but we (probably) have the memory
    std::vector<unsigned char> obuf;
    obuf.resize(params.output_width * params.output_height * 4);

    std::cout << "Artificer: " << params.output_width << ", " << params.output_height << std::endl;

    for (size_t i = 0; i < fs.getLength(); i++)
    {
        //Get a frame from our cool framesource
        fs.getFrame(i,&ibuf[0]);

        //process the frame
        artificer_process(art, &ibuf[0], &obuf[0], i);

        //if we're actually outputting the frames somewhere
        if (frameout.size())
        {

            std::stringstream ss;
            ss << std::setfill('0') << std::setw(8) << i;
            std::string num = ss.str();

            write_png(frameout+num+".png", params.output_width,  params.output_height, &obuf[0], 4);
        }

    }


    artificer_end(art);
}



int main(int argc, const char** argv)
{
    doGL(argc, argv);
}
