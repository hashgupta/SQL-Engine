#pragma once

#include <fstream>
#include <iostream>
#include <vector>

struct DataItem {
    std::variant<std::string, int, bool> item;
};

struct Data {
    std::vector<std::string> column_names;
    std::vector<std::vector<DataItem>> rows;
};

class Reader {
    std::string filename;
    std::vector<std::string> data_types;
    std::ifstream stream;
    
    public:
    Reader (std::string filename_);

    inline std::string readline(std::ifstream &stream);
    inline std::vector<std::string> readlines(std::ifstream &stream);
    std::vector<std::string> split_next_row(std::string line);
    std::vector<std::vector<DataItem>> read_rows(std::ifstream &stream);
    Data read_data();
};
