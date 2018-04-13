
#include "Vfs.h"

#include <fstream>


Vfs Vfs::sInstance;


istream_ptr Vfs::openInput(const char *fname)
{
    std::unique_ptr<std::ifstream> file(new std::ifstream(fname, std::ios_base::binary));
    if(!file->is_open())
    {
        file.reset();
        std::cerr<< "Failed to open "<<fname<< std::endl;
    }
    return std::move(file);
}
