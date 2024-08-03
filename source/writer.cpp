#include <sql-engine/writer.h>

#include <fstream>
#include <sstream>
#include <string>
#include <variant>

std::string DataItemVisitor::operator()(const std::string str) const {
  return str;
};

std::string DataItemVisitor::operator()(const int int_val) const {
  return std::to_string(int_val);
};

std::string DataItemVisitor::operator()(const bool bool_val) const {
  if (bool_val) {
      return "True";
    } else {
      return "False";
    }
};

Writer::Writer(std::string filename_) : filename(std::move(filename_)){};

std::string Writer::convert_to_string(DataItem item) {
  DataItemVisitor visitor;
  return std::visit(visitor, item.item);
}

void Writer::write_data_to_file(std::vector<std::vector<DataItem>> data) {
  std::ofstream stream(filename);

  for (std::vector<DataItem> row : data) {
    std::string output = "";
    for (DataItem element : row) {
      output += this->convert_to_string(element) + ",";
    }
    stream << output + "\n";
  }

  stream.close();
}
