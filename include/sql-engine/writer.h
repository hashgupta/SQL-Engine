#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include "reader.h"

struct DataItemVisitor {
    public:
    std::string operator()(const std::string str) const;
    std::string operator()(const int int_val) const;
    std::string operator()(const bool bool_val) const;
};

class Writer {
    std::string filename;
    std::vector<std::string> data_types;
    
    public:
    Writer (std::string filename_);

    std::string convert_to_string(DataItem item);
    void write_data_to_file(std::vector<std::vector<DataItem>> content);
};