#include <string>
#include <vector>

void write_png(std::string,int,int,unsigned char*, int);
class FrameSource
{
public:
	std::vector<std::string> files;
	int width = 0;
	int height = 0;
	int getWidth()
	{
		return width;
	}
	int getHeight()
	{
		return height;
	}
	unsigned int getLength()
	{
		return files.size();
	}

    void getFrame(unsigned int index, unsigned char*);

	FrameSource(const std::string&);
};

