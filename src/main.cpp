#include <iostream>
#include <string>

#include <boost/mpi.hpp>
#include <boost/filesystem.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/multi_array.hpp>

#include "utils.h"
#include "conf_reader.h"
#include "main_config.h"


Params conf_map_to_Params(MapStrStr & conf_map)
{
    return Params{
        0.,
        0.,
        std::stod(conf_map["conductivity"]),
        std::stod(conf_map["capacity"]),
        std::stod(conf_map["density"]),
        std::stod(conf_map["deltaX"]),
        std::stod(conf_map["deltaY"]),
        std::stod(conf_map["deltaT"]),
        std::stoi(conf_map["gridX"]),
        std::stoi(conf_map["gridY"]),
        std::stoi(conf_map["printT"]),
        std::stoi(conf_map["time"]),
    };
}


int main (int argc, char * argv[])
{
    mpi::environment env{argc, argv};
    mpi::communicator world;
    const int workers_num = world.size() - 1;

    // set name of configuration file
    std::string config_file("../config.dat");
    //    if (argc >= 2) { config_file = argv[1]; }

    // try to open configuration file
    std::ifstream cf(config_file);
    if (!cf.is_open()) {
        std::cerr << "Error while opening configuration file.\n"
                     "Check filename or path to file: "
                  << config_file
                  << std::endl;
        _exit(OPEN_FILE_ERROR);
    }

    // try to read configurations and save them to map conf
    MapStrStr conf;
    try {
        conf = read_conf(cf, '=');
        cf.close();
    }
    catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        _exit(READ_FILE_ERROR);
    }

    //    convert map to struct
    auto params = conf_map_to_Params(conf);
    params.alpha = params.conductivity / (params.density * params.capacity);
    params.alpha_deltaT = params.deltaT * params.alpha;
    auto bounds = get_bounds(workers_num, params.gridX);

    if ( world.rank() == 0 ) // parent process
    {
        //      open and read file with heat map init state configuration
        std::ifstream hmcf("../heat_map.conf");
        if (!hmcf.is_open()) {
            std::cerr << "Error while opening configuration file.\n"
                         "Check filename or path to file: "
                      << config_file
                      << std::endl;
            _exit(OPEN_FILE_ERROR);
        }

        VecPairInt heat_map_conf;
        try {
            heat_map_conf = read_heat_map_conf(hmcf, ':');
            hmcf.close();
        }
        catch (std::exception &ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            _exit(READ_FILE_ERROR);
        }

        //      generate init state of heat map according to given configurations ('heat_map.conf')
        ArrayD2 heat_map_init_state(boost::extents[params.gridX][params.gridY]);
        if (generate_heat_map(heat_map_conf, heat_map_init_state, params) == -1)
        {
            std::cerr << "Grid size given in 'config.dat' does not match with grid size from 'heat_map.conf.conf'"
                      << std::endl;
            _exit(WRONG_MAP_SIZE_ERROR);
        }

        if ( !von_Neumann_criterion(params))
        {
            std::cerr << "Von Neumann Criterion is not satisfied" << std::endl;
            _exit(VON_NEUMANN_ERROR);
        }

        //      send child processes their parts of map
        send_children_their_parts(world, workers_num, heat_map_init_state, bounds);

        std::string img_path {"../img_cache"};
        boost::filesystem::create_directory(img_path);
        ArrayD2 result_map(boost::extents[params.gridX][params.gridY]);

        int max_value_in_map {0};
        for ( auto & v : heat_map_conf )
            if ( v.second > max_value_in_map )
                max_value_in_map = v.second;

        //      gather partial maps from child processes and save them as one image
        for ( int i = 0; i < params.time / params.printT; ++i )
        {
            gather_map(world, workers_num, bounds, result_map);
            save_map(result_map, img_path + "/save_" + std::to_string(i) + ".png", max_value_in_map);
        }

    }
    else // child process
    {
        auto current_bound = bounds[world.rank() - 1];
        auto sizeX = current_bound.second - current_bound.first + 1;
        ArrayD2 partial_map(boost::extents[sizeX][params.gridY]);

        //      receive its part of map
        world.recv(0, 1, &partial_map[0][0], sizeX * params.gridY);

        //      start calculation process
        calculation_process(world, partial_map, params);
    }

    return 0;
}