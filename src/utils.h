#ifndef THERMALCONDUCTIVITYMPI_UTILS_H
#define THERMALCONDUCTIVITYMPI_UTILS_H

#include <map>

#include "main_config.h"
#include "boost/multi_array.hpp"


void save_map ( const ArrayD2 & heat_map, const std::string & img_path );

void gather_map ( const mpi::communicator & world, int workers_num, const VecPairInt & bounds, ArrayD2 & array );

//void swap_edges (int rank, int workers_num, const mpi::communicator &world, ArrayD2 &array);
VecPairInt get_bounds( int workers_num, int grid_size);

int generate_heat_map ( const VecPairInt & heat_map_conf, ArrayD2 & heat_map_init_state, const Params & params );

void calculation_process ( const mpi::communicator & world, const ArrayD2 & init_grid, const Params & params );
//void next_state(const boost::multi_array<double, 2> & current, boost::multi_array<double, 2> & next, const Params & params);
//bool von_Neumann_criterion(const Params & params);

#endif //THERMALCONDUCTIVITYMPI_UTILS_H
