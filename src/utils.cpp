#include <string>

#include "utils.h"
#include "main_config.h"

#include <png++/png.hpp>

#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>


void save_map ( const ArrayD2 & heat_map, const std::string & img_path )
{
    png::image<png::rgb_pixel> image(heat_map.shape()[0], heat_map.shape()[1]);
    for ( png::uint_32 y = 0; y < image.get_height(); ++y )
        for ( png::uint_32 x = 0; x < image.get_width(); ++x )
            image[y][x] = png::rgb_pixel(( heat_map[x][y] / 100. ) * 255, 0, ( 1. - heat_map[x][y] / 100. ) * 255);
    image.write(img_path);
}


void gather_map ( const mpi::communicator & world, int workers_num, const VecPairInt & bounds, ArrayD2 & array )
{
    auto reqs = new mpi::request[workers_num];
    for ( int w = 1; w <= workers_num; ++w )
    {
        reqs[w - 1] = world.irecv(w, 1, &array[bounds[w].first + 1][0],
                                  static_cast<int>(( bounds[w].second - bounds[w].first ) * array.shape()[1]));
    }
    mpi::wait_all(reqs, reqs + workers_num);
    delete[] reqs;
}


void swap_edge ( int rank, size_t old_row_ind, size_t new_row_ind, const mpi::communicator & world, ArrayD2 & array )
{
    mpi::request reqs[2];

    ArrayD1 new_row(boost::extents[array.shape()[1]]);
    reqs[0] = world.irecv(rank, 1, &new_row[0], array.shape()[1]);

    ArrayD1 old_row = array[boost::indices[old_row_ind][range()]];
    reqs[1] = world.isend(rank, 1, &old_row[0], old_row.shape()[0]);

    mpi::wait_all(reqs, reqs + 1);
    array[new_row_ind] = new_row;

    mpi::wait_all(reqs + 1, reqs + 2);
}


void swap_edges ( const mpi::communicator & world, ArrayD2 & array )
{
    auto rank = world.rank();
    auto workers_num = world.size() - 1;

    if ( rank > 1 )
        swap_edge(rank - 1, 1, 0, world, array);
    if ( rank < workers_num )
        swap_edge(rank + 1, array.shape()[1] - 2, array.shape()[1] - 1, world, array);
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

    //    конструктор масиву походу заповнює його нулями
    //    for (ArrayD2::index i = 0; i != heat_map_init_state.shape()[0]; ++i)
    //        for (ArrayD2::index j = 0; j != heat_map_init_state.shape()[1]; ++j)
    //            heat_map_init_state[i][j] = 0;

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

    return 0;
}

void next_state(const ArrayD2 & current, ArrayD2 & next, const Params & params)
{
    for ( int i = 1; i < current.shape()[0] - 1; ++i )
    {
        for ( int j = 1; j < current.shape()[1] - 1; ++j )
        {
            next[i][j] = current[i][j] + params.alpha_deltaT * (
                    ( current[i - 1][j] - 2 * current[i][j] + current[i + 1][j] ) / ( params.deltaX * params.deltaX ) +
                    ( current[i][j - 1] - 2 * current[i][j] + current[i][j + 1] ) / ( params.deltaY * params.deltaY )
            );
        }
    }
}


bool von_Neumann_criterion(const Params & params)
{
    return (params.deltaT <= std::pow(std::max(params.deltaX, params.deltaY), 2) / (4 * params.alpha));
}


void calculation_process ( const mpi::communicator & world, const ArrayD2 & init_grid, const Params & params )
{
    if ( !von_Neumann_criterion(params))
    {
        //TODO:
    }

    ArrayD2 current = init_grid[boost::indices[range()][range()]];
    ArrayD2 next = init_grid[boost::indices[range()][range()]];

    for ( int i = 0; i * params.deltaT < params.time; ++i )
    {

        if (( static_cast<int>(params.deltaT * i) % params.printT ) == 0 )
            world.send(0, 1, &current[0][0], static_cast<int>(current.shape()[0] * current.shape()[1]));

        next_state(current, next, params);
        std::swap(current, next);

        swap_edges(world, current);
    }
}