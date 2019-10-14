#ifndef THERMALCONDUCTIVITYMPI_MAIN_CONFIG_H
#define THERMALCONDUCTIVITYMPI_MAIN_CONFIG_H

#include "thread_safe_queue.h"
#include "boost/multi_array.hpp"
#include "boost/mpi.hpp"

namespace mpi = boost::mpi;
using range = boost::multi_array_types::index_range;
using VecPairInt = std::vector<std::pair<int, int>>;
using ArrayD2 = boost::multi_array<double, 2>;
using ImagesQueue = thread_safe_queue<ArrayD2>;

enum Error
    {
    OPEN_FILE_ERROR = 2, READ_FILE_ERROR, WRITE_FILE_ERROR, READ_ARCHIVE_ERROR, VON_NEUMANN_ERROR, WRONG_MAP_SIZE_ERROR
    };

using Params = struct Params{
    double alpha;
    double alpha_deltaT;
    double conductivity;
    double capacity;
    double density;
    double deltaX;
    double deltaY;
    double deltaT;
    int gridX;
    int gridY;
    int printT;
    int time;
};

#endif //THERMALCONDUCTIVITYMPI_MAIN_CONFIG_H
