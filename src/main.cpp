#include <iostream>
#include <fstream>
#include <map>
#include <string>

#include <png++/png.hpp>
#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/multi_array.hpp>

#include "utils.h"
#include "conf_reader.h"
#include "main_config.h"
#include "thread_safe_queue.h"


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
    const int workers_num = world.size();

    VecPairInt bounds;
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
    params.alpha_deltaT = params.alpha_deltaT * params.alpha;

    auto larger_axis = std::max(params.gridX, params.gridY);

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

        bounds = get_bounds(1, larger_axis);

        int current_worker{0};
        for (auto &p: bounds) {
            auto new_array = (larger_axis == params.gridX)
                             ? heat_map_init_state[boost::indices[range().start(p.first).finish(p.second + 1)][range()]]
                             : heat_map_init_state[boost::indices[range()][range().start(p.first).finish(
                            p.second + 1)]];

            for (ArrayD2::index i = 0; i < new_array.shape()[1]; ++i) {
                for (ArrayD2::index j = 0; j < new_array.shape()[0]; ++j)
                    std::cout << new_array[j][i] << " ";
                std::cout << std::endl;
            }
            std::cout << "----" << std::endl;
            if (current_worker != 0) { }
                //                world.isend(current_worker, 1, new_array);
            else {
                //            TODO: parent takes the last part of data and make calculations



            }

            ++current_worker;

        }

        
    }
    else
    {
        auto step = bounds[world.rank()].second - bounds[world.rank()].first;
        auto XisLarger = larger_axis == params.gridX;
        ArrayD2 array(boost::extents
                      [(XisLarger) ? step : params.gridX]
                      [(XisLarger) ? params.gridY : step]);

        world.irecv(0, 1, array);
        //        TODO: run alculations
    }


//    if (!von_Neumann_criterion(params)){
//        std::cerr << "System is not stable according to von Neumann criterion. So fuck it." << std::endl;
//        return VON_NEUMANN_ERROR;
//    }
    ImagesQueue images_queue;
//    boost::multi_array<double, 3> history_grid(boost::extents[static_cast<int>(conf["gridX"])][static_cast<int>(conf["gridY"])][static_cast<int>(conf["printT"])]);
    //TODO: add first map to array





















    return 0;







}