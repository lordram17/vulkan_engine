#include <vector>
#include <string>
#include <fstream>

class SimpleFileReader 
{
private:

public:

    static std::vector<char> ReadFile (const std::string& filename)
    {
        // ate : start reading at the end of the file (this allows getting the size to allocate buffer)
        // binary : read the file as a binary file (this avoids text transformation)
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if(!file.is_open())
        {
            throw std::runtime_error("Failed to open file :" + filename);
        }

        size_t filesize = (size_t) file.tellg(); // returns the location of the pointer (called input pointer : pointer that keeps track of position in a stream when reading it (output pointer if writing))
        std::vector<char> buffer(filesize);
        file.seekg(0); //move the position of the input pointer to the beginning of the file
        file.read(buffer.data(), filesize);
        file.close();
        return buffer;
    }
};