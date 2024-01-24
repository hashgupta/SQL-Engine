#include <sql-engine/writer.h>

#include <fstream>
#include <sstream>
#include <string>
#include <variant>

Writer::Writer(std::string filename_) : filename(std::move(filename_)){};

std::string Writer::convert_to_string(DataItem item) {
  if (auto* v = std::get_if<std::string>(&item.item)) {
    return *v;
  } else if (auto* v = std::get_if<int>(&item.item)) {
    return std::to_string(*v);
  } else if (auto* v = std::get_if<bool>(&item.item)) {
    if (*v) {
      return "True";
    } else {
      return "False";
    }
  } else {
    assert(false);
  }
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
}
