#include <sql-engine/reader.h>
#include <sstream>
#include <fstream>

Reader::Reader(std::string filename_)
    : filename(std::move(filename_)), data_types(NULL) {
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
        std::vector<DataItem> items;
        
        auto types = this->data_types;
        
        for (int i = 0; i < split_row.size(); i++) {
            
            DataItem item;
            
            if (types[i] == "string") { // String
                
                item.item = split_row[i];
            
            } else if (types[i] == "bool") { // Bool
                
                if (split_row[i] == "True") {
                    
                    item.item = true;
                
                } else if (split_row[i] == "False") {
                    
                    item.item = false;
                
                } else {
                    
                    std::cout << "Expected Boolean data" << std::endl;
                    exit(EXIT_FAILURE);
                
                }
            
            } else if (types[i] == "int") { // Int
                
                item.item = stoi(split_row[i]);
            
            } else {
                
                std::cout << "Invalid Type: " << types[i] << std::endl;
                exit(EXIT_FAILURE);
            
            }
            items.push_back(item);
            
        }
        output.push_back(items);
    }
    return output;
}
Data Reader::get_data() {
    Data new_data;
    std::ifstream stream(filename, std::ifstream::in);
    
    auto line = this->readline(stream);
    
    new_data.column_names = split_next_row(line);

    this->data_types = split_next_row(this->readline(stream));
    
    new_data.rows = this->read_rows(stream);
    
    return new_data;
}

