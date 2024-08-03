#include <sql-engine/reader.h>
#include <sstream>
#include <fstream>

std::vector<DataItem> process_row(std::vector<std::string> values, std::vector<std::string> data_types) {
    std::vector<DataItem> row;
    for (int i = 0; i < values.size(); i++) {
            
        DataItem item;
        
        if (data_types[i] == "string") { // String
            
            item.item = values[i];
        
        } else if (data_types[i] == "bool") { // Bool

            // refactor into a dictionary mapping
            
            if (values[i] == "True") {
                
                item.item = true;
            
            } else if (values[i] == "False") {
                
                item.item = false;
            
            } else {
                
                std::cout << "Expected Boolean data" << std::endl;
                exit(EXIT_FAILURE);
            
            }
        
        } else if (data_types[i] == "int") { // Int
            
            item.item = stoi(values[i]);
        
        } else {
            
            std::cout << "Invalid Type: " << data_types[i] << std::endl;
            exit(EXIT_FAILURE);
        
        }
        row.push_back(std::move(item));
        
    }
    return row;
    
}

Reader::Reader(std::string filename_) {
    filename = std::move(filename_);
    stream = std::ifstream(filename, std::ifstream::in);
    data_types = split_next_row(this->readline(stream));
}

std::string Reader::readline(std::ifstream &stream) {
    std::string line;
    std::getline(stream, line);
    return line;
}

std::vector<std::string> Reader::readlines(std::ifstream &stream) {
    std::vector<std::string> lines;
    std::string line;
    while (getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::vector<std::string> Reader::split_next_row(std::string line) {
    std::vector<std::string> header;
    std::istringstream buffer(line);
    while( buffer.good() )
    {
        std::string substr;
        getline( buffer, substr, ',' );
        header.push_back( substr );
    }
    return header;
}

std::vector<std::vector<DataItem>> Reader::read_rows(std::ifstream &stream) {
    // data types must be set before running this
    std::vector<std::vector<DataItem>> output;
    std::vector<std::string> lines = this->readlines(stream);
    
    for(auto element : lines) {
        std::vector<std::string> split_row = this->split_next_row(element);

        auto types = this->data_types;
        auto row = process_row(split_row, types);
        output.push_back(row);
    }
    return output;
}
Data Reader::read_data() {
    Data new_data;
    
    auto line = this->readline(stream);
    
    new_data.column_names = split_next_row(line);
    
    new_data.rows = this->read_rows(stream);
    
    return new_data;
}

