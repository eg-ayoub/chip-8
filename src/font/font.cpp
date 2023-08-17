#include <fstream>
#include <regex>

#include <font/font.hpp>
#include <spdlog/spdlog.h>

font::Font::Font()
{
    // allocate
    this->fontdata = new std::vector<std::byte>(FONT_DATA_SIZE);

    // load default font
    spdlog::info("loading default font");
    for (std::size_t i = 0; i < FONT_DATA_SIZE; i++)
    {
        this->fontdata->at(i) = (std::byte)DEFAULT_FONT[i];
    }
}

font::Font::Font(std::string filename)
{
    // allocate
    this->fontdata = new std::vector<std::byte>(FONT_DATA_SIZE);

    // load the font file
    spdlog::info("loading font {}", filename);
    // file exists already checked
    std::ifstream font_file;
    font_file.open(filename);
    // parse line by line
    std::regex byte_regex("0b([01]{8})");
    int counter = 0;
    std::string line;
    while (std::getline(font_file, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, byte_regex) && match.size() > 1)
        {
            this->fontdata->at(counter) = (std::byte)std::stoi(match.str(1));
            spdlog::trace("matched {}", this->fontdata->at(counter));
            counter++;
        }
        else
        {
            spdlog::trace("no match {}", line);
        }
    }

    // final checks
    if (counter == FONT_DATA_SIZE)
    {
        spdlog::info("proper font file");
    }
    else
    {
        spdlog::warn("bad font file");
        spdlog::info("reverting to default font");
        for (std::size_t i = 0; i < FONT_DATA_SIZE; i++)
        {
            this->fontdata->at(i) = (std::byte)DEFAULT_FONT[i];
        }
    }
}

std::vector<std::byte> *font::Font::data()
{
    return this->fontdata;
}

font::Font::~Font()
{
    this->fontdata->clear();
    delete this->fontdata;
}