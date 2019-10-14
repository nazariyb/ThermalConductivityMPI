#include <string>

#include "utils.h"
#include "main_config.h"
#include "thread_safe_queue.h"

#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/string.hpp>

void swap_edges (int rank, int workers_num, const mpi::communicator &world, ArrayD2 &array)
{
    if (rank != 0) {
        mpi::request reqs[2];
        reqs[0] = world.isend(rank - 1, 1, std::string("hello1"));
        std::string msg;
        reqs[1] = world.irecv(rank - 1, 1, msg);
        mpi::wait_all(reqs, reqs + 2);
        //    TODO: send data to rank-1

    }
    if (rank != workers_num) {
        mpi::request reqs[2];
        reqs[0] = world.isend(rank + 1, 1, std::string("hello2"));
        std::string msg;
        reqs[1] = world.irecv(rank + 1, 1, msg);
        mpi::wait_all(reqs, reqs + 2);
        //    TODO: send data to rank+1
    }
}


VecPairInt get_bounds(int workers_num, int grid_size)
{
    VecPairInt bounds{};
    int step = static_cast<int>(std::round(static_cast<double>(grid_size) / workers_num));
    --grid_size;

    for (int i = 0; i < grid_size; i += step)
        bounds.emplace_back(i, std::min(i + step + 1, grid_size));

    return bounds;
}


int generate_heat_map(const VecPairInt & heat_map_conf, ArrayD2 & heat_map_init_state, const Params & params)
{
    int expected_perimeter = params.gridX * 2 + params.gridY * 2;
    int perimeter{4};
    for (auto & c: heat_map_conf)
        perimeter += c.first;
    if (perimeter != expected_perimeter)
        return -1;

    for (ArrayD2::index i = 0; i != heat_map_init_state.shape()[0]; ++i)
        for (ArrayD2::index j = 0; j != heat_map_init_state.shape()[1]; ++j)
            heat_map_init_state[i][j] = 0;

    int dirs[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    int times_to_insert = heat_map_conf[0].first;
    for(ArrayD2::index row = 0, col = 0, i = 0, j = 0; i < 4;) {
        if (times_to_insert == 0)
        {
            ++j;
            times_to_insert = heat_map_conf[j].first;
        }
        heat_map_init_state[row][col] = heat_map_conf[j].second;
        --times_to_insert;
        if( (dirs[i][0] && ((row += dirs[i][0]) == 0 || row == heat_map_init_state.shape()[0] - 1)) ||
            (dirs[i][1] && ((col += dirs[i][1]) == 0 || col == heat_map_init_state.shape()[1] - 1)))
            ++i;
    }
//    std::cout << "HERE" << std::endl;
//    for (ArrayD2::index i = 0; i != heat_map_init_state.shape()[0]; ++i)
//        for (ArrayD2::index j = 0; j != heat_map_init_state.shape()[1]; ++j)
//            std:: cout << heat_map_init_state[i][j] << std::endl;

    return 0;
}

void calculation_process (const ArrayD2 &init_grid, ImagesQueue &images_queue, const Params &params)
{
    auto current = init_grid;
    auto next = init_grid;
    for (int i = 0; i * params.deltaT < params.time; ++i){
        if ((static_cast<int>(params.deltaT * i) % params.printT) == 0) images_queue.push(current);
        next_state(current, next, params);
        std::swap(current, next);
    }


}
void next_state(const ArrayD2 & current, ArrayD2 & next, const Params & params)
{
    for(int i = 1; i < current.shape()[0] - 1; ++i){
        for(int j = 1; j < current.shape()[1] - 1; ++j){
            next[i][j] = current[i][j] + params.alpha_deltaT * (
            (current[i-1][j] - 2 * current[i][j] + current[i+1][j]) / (params.deltaX * params.deltaX) +
            (current[i][j-1] - 2*current[i][j] + current[i][j+1]) / (params.deltaY * params.deltaY)
            );
        }
    }
}


bool von_Neumann_criterion(const Params & params)
{
    return (params.deltaT <= std::pow(std::max(params.deltaX, params.deltaY), 2) / (4 * params.alpha));
}