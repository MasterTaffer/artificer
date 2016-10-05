#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ARTIFICE_DLL_BUILD
#define ARTIFICE_DLL_TYPE __declspec(dllexport)
#else
#ifdef ARTIFICE_STATIC_BUILD
    #define ARTIFICE_DLL_TYPE
#else
    #define ARTIFICE_DLL_TYPE __declspec(dllimport)
#endif
#endif

//The color formats are 1 byte per component
//=> 4 bytes / 32 bits per pixel


#define artificer_color_format_rgba 0x80
#define artificer_color_format_bgra 0x81

/*
    artificer_parameters:

    INPUT

    config_path: path to a proper Lua config
    width & height: the dimensions of input data
    display_output: whether to display the preview window
    color_format: color format for input & output. Use the constants above

    OUTPUT

    output_width & output_height: size of the processed frame
    error: error number. Equals 0 if no errors.
    error_string: is set when error occurs.
*/

struct artificer_parameters
{
    const char* config_path;
    int display_output;
    int width;
    int height;
    int color_format;

    int output_width;
    int output_height;

    const char* error_string;
    int error;
};

/*
    Get default artificer_parameters

    width, height and config_path parameters are not set by this
*/
artificer_parameters ARTIFICE_DLL_TYPE artificer_default_parameters();

/*
    Initializes the artificer with parameters

    Return: handle to future artificer calls
*/
void* ARTIFICE_DLL_TYPE artificer_start(artificer_parameters* params);

/*
    Processes a single frame

    art: handle returned by artificer_start
    input: input data
    output: output data
    framenum: current frame number

    input and output are in the color format described in the initialization parameters

    input and output can be the same values / can overlap, provided there is enough space for both
    individually.

    framenum can be any arbitrary value, but it generally should be a sequential number describing
    the current position in the input sequence. It is used as a parameter for the shaders & scripts.
*/
void ARTIFICE_DLL_TYPE artificer_process(void* art, unsigned char* input, unsigned char* output, int framenum);

/*
    Ends and frees the artificer

    art: handle returned by artificer_start

    After this call the handle is invalid
*/
void ARTIFICE_DLL_TYPE artificer_end(void* art);

#ifdef __cplusplus
}
#endif
