#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include "reader.h"

class Writer {
    std::string filename;
    std::vector<std::string> data_types;
    
    public:
    Writer (std::string filename_);

    std::string convert_to_string(DataItem item);
    void write_data_to_file(std::vector<std::vector<DataItem>> content);
};