

#include <cstring>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "tinydircpp.h"

#include "framesource.hpp"

void GetFilesInDirectory(std::string dirstr, std::string fileEnding, std::vector<std::string> & ret)
{


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
                std::string name = dirstr+"/"+file.name;
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
                        std::string ak = dirstr+"/"+file.name;
                        ret.push_back(ak);
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
