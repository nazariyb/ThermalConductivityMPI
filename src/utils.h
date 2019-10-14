#ifndef THERMALCONDUCTIVITYMPI_UTILS_H
#define THERMALCONDUCTIVITYMPI_UTILS_H

#include <map>

#include "main_config.h"
#include "boost/multi_array.hpp"

VecPairInt get_bounds(int workers_num, int grid_size);
int generate_heat_map(const std::vector<std::pair<int, int>> & heat_map_conf, ArrayD2 & heat_map_init_state, const Params & params);
void next_state(const boost::multi_array<double, 2> & current, boost::multi_array<double, 2> & next, const Params & params);
bool von_Neumann_criterion(const Params & params);

#endif //THERMALCONDUCTIVITYMPI_UTILS_H
