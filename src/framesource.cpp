

#include <cstring>
#include <algorithm>
#include <vector>
#include <locale>
#include <codecvt>
#include <string>

#ifdef TINYDIR_WSTRING
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "tinydircpp.h"

#include "framesource.hpp"

void GetFilesInDirectory(std::string dstr, std::string fEnding, std::vector<std::string> & ret)
{
#ifdef TINYDIR_WSTRING
	auto dirstr = converter.from_bytes(dstr);
	auto fileEnding = converter.from_bytes(fEnding);
#else
	auto dirstr = dstr;
	auto fileEnding = fEnding;
#endif

    tinydir_dir dir;
    tinydir_open(&dir, dirstr.c_str());

    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if (file.name[0] != '.')
        {
            if (file.is_dir)
            {

            }
            else
            {
				auto name = dirstr+SLASH+file.name;
                int l = name.length();
                int fel = fileEnding.length();
                if (l >= fel)
                {
                    bool match = true;
                    for (int i = 0; i < fel; i++)
                    {
                        if (name[i+l-fel] != fileEnding[i])
                        {
                            match = false;
                            break;
                        }
                    }
                    if (match)
                    {
                        auto ak = dirstr+SLASH+file.name;
						#ifdef TINYDIR_WSTRING
						ret.push_back(converter.to_bytes(ak));
						#else	
						ret.push_back(ak);
						#endif
                    }
                }
            }
        }
        tinydir_next(&dir);
    }
    tinydir_close(&dir);

}


void write_png(std::string f,int w,int h,unsigned char* data, int c)
{
    stbi_write_png(f.c_str(), w, h, c, data,0);
}

void FrameSource::getFrame(unsigned int index, unsigned char* to)
{

    int x,y,c;
    auto p = stbi_load(files[index].c_str(),&x,&y,&c,4);
    //On error, just do nothing
    if (!p)
        return;

    //If our frame sizes are different for some reason,
    //instead of crashing, just display garbled output
    int min = x*y;
    if (width*height < min)
        min = width*height;

    memcpy(to,p,min*4);

    stbi_image_free(p);
    return;
}


FrameSource::FrameSource(const std::string& dir)
{
    GetFilesInDirectory(dir,".png",files);
    std::sort(files.begin(),files.end());
    if (files.size() > 0)
    {
        int w, h, c;
        stbi_info(files[0].c_str(), &w, &h, &c);
        width = w;
        height = h;
    }
    else
    {
        //if there's no input files, use these dummy params
        width = 1;
        height = 1;
    }
}
