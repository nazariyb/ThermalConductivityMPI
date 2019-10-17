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
    std::cout << world.rank() << ", " << world.size() << '\n';
    const int workers_num = world.size() - 1;

    // set name of configuration file
    std::string config_file("../config.dat");
    if (argc >= 2) { config_file = argv[1]; }

    // try to open configuration file
    std::ifstream cf(config_file);
    if (!cf.is_open()) {
        std::cerr << "Error while opening configuration file.\n"
                     "Check filename or path to file: "
                  << config_file
                  << std::endl;
        return OPEN_FILE_ERROR;
    }

    // try to read configurations and save them to map conf
    MapStrStr conf;
    try {
        conf = read_conf(cf, '=');
        cf.close();
    }
    catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return READ_FILE_ERROR;
    }

    auto params = conf_map_to_Params(conf);
    params.alpha = params.conductivity / (params.density * params.capacity);
    params.alpha_deltaT = params.deltaT * params.alpha;
    auto bounds = get_bounds(workers_num, params.gridX);

    if (world.rank() == 0)
    {
        std::ifstream hmcf("../heat_map");
        if (!hmcf.is_open()) {
            std::cerr << "Error while opening configuration file.\n"
                         "Check filename or path to file: "
                      << config_file
                      << std::endl;
            return OPEN_FILE_ERROR;
        }

        VecPairInt heat_map_conf;
        try {
            heat_map_conf = read_heat_map_conf(hmcf, ':');
            hmcf.close();
        }
        catch (std::exception &ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            return READ_FILE_ERROR;
        }

        ArrayD2 heat_map_init_state(boost::extents[params.gridX][params.gridY]);
        if (generate_heat_map(heat_map_conf, heat_map_init_state, params) == -1)
        {
            std::cerr << "Learn calculus, asshole!" << std::endl;
            return WRONG_MAP_SIZE_ERROR;
        }

        for (int w = 1; w < workers_num + 1; ++w)
        {
//            std::cout << "started sending data to worker #" << w << " of " << workers_num << std::endl;
            auto new_array = heat_map_init_state
            [boost::indices[range().start(bounds[w - 1].first).finish(bounds[w - 1].second + 1)]
                    [range()]];

//            for (ArrayD2::index i = 0; i < new_array.shape()[1]; ++i) {
//                for (ArrayD2::index j = 0; j < new_array.shape()[0]; ++j)
//                    std::cout << new_array[j][i] << " ";
//                std::cout << std::endl;
//            }
//            std::cout << "----" << std::endl;
            world.send(w, 1, &new_array[0][0],
                        static_cast<int>(new_array.shape()[0] * new_array.shape()[1]));
//            std::cout << "data in range [" << bounds[w - 1].first << "-" << bounds[w - 1].second << "] is sent to worker #" << w << std::endl;
        }

        std::string img_path {"../img_cache"};
        boost::filesystem::create_directory(img_path);
        ArrayD2 result_map(boost::extents[params.gridX][params.gridY]);

        for ( int i = 0; i < params.time / params.printT; ++i )
        {
            gather_map(world, workers_num, bounds, result_map);
            save_map(result_map, img_path + "/save_" + std::to_string(i) + ".png");
        }

    }
    else
    {
//        std::cout << "#" << world.rank() << " started" << std::endl;
        auto current_bound = bounds[world.rank() - 1];
        auto sizeX = current_bound.second - current_bound.first + 1;
        ArrayD2 partial_map(boost::extents[sizeX][params.gridY]);

        world.recv(0, 1, &partial_map[0][0], sizeX * params.gridY);
//        std::cout << world.rank() << " got:" << std::endl;
//        for (ArrayD2::index i = 0; i < partial_map.shape()[1]; ++i) {
//            for (ArrayD2::index j = 0; j < partial_map.shape()[0]; ++j)
//                std::cout << partial_map[j][i] << " ";
//            std::cout << std::endl;
//        }
//        std::cout << "----" << std::endl;

        calculation_process(world, partial_map, params);
    }

    return 0;
}