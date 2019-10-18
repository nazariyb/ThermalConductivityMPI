#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include "conf_reader.h"

std::vector<std::string> split_line (const std::string &line, const char delimiter)
{
    std::string sub;
    std::string::size_type pos = 0;
    std::string::size_type old_pos = 0;
    std::vector<std::string> target_vector;
    bool flag = true;

    while (flag) {
        pos = line.find_first_of(delimiter, pos);
        if (pos == std::string::npos) {
            flag = false;
            pos = line.size();
        }
        sub = line.substr(old_pos, pos - old_pos);
        sub.erase(std::remove(sub.begin(), sub.end(), ' '), sub.end());
        target_vector.push_back(sub);
        old_pos = ++pos;
    }
    return target_vector;
}


MapStrStr read_conf ( std::istream & cf, char delimiter )
{
    std::ios::fmtflags flags(cf.flags());
    cf.exceptions(std::ifstream::badbit);

    std::map<std::string, std::string> res;
    std::string line;
    try {
        std::string key;

        while (getline(cf, line)) {
            auto target_vector = split_line(line, delimiter);
            res[target_vector[0]] = target_vector[1];
        }

    } catch (std::ios_base::failure &fail) {
        cf.flags(flags);
        throw;
    }

    return res;
}


std::vector<std::pair<int, int>> read_heat_map_conf ( std::istream & cf, char delimiter )
{
    std::ios::fmtflags flags(cf.flags());
    cf.exceptions(std::ifstream::badbit);

    std::vector<std::pair<int, int>> res;
    std::string line;
    try {
        std::string key;

        while (getline(cf, line)) {
            auto target_vector = split_line(line, delimiter);
            res.emplace_back(std::stoi(target_vector[0]), std::stoi(target_vector[1]));
        }

    } catch (std::ios_base::failure &fail) {
        cf.flags(flags);
        throw;
    }

    return res;
}
