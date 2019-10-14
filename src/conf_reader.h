#ifndef THERMALCONDUCTIVITYMPI_CONF_READER_H
#define THERMALCONDUCTIVITYMPI_CONF_READER_H

#include <map>
#include <string>
#include <vector>
#include <iostream>

using MapStrStr = std::map<std::string, std::string>;

MapStrStr read_conf (std::istream &cf, char splitter);
std::vector<std::pair<int, int>> read_heat_map_conf (std::istream &cf, char splitter);

#endif //THERMALCONDUCTIVITYMPI_CONF_READER_H
