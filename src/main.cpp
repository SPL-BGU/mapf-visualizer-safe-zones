#include <fstream>
#include <iostream>
#include <regex>
#include <execinfo.h>
#include <signal.h>

#include "../include/param.hpp"
#include "../include/ofApp.hpp"
#include "ofMain.h"

const std::regex r_pos =
    std::regex(R"(\((\d+),(\d+),?([XY]{1}_[A-Z]{4,5})?\),)");

struct args {
    std::string map_file;
    std::string solution_file;
    std::string safe_zones_file;
    bool flg_capture_only = false;
};


void handler(int sig)
{
    void *array[20];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 20);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}


args parse_args(int argc, char *argv[])
{
    args a;
    if (argc < 3 || !std::ifstream(argv[1]) || !std::ifstream(argv[2])) {
        std::cout << "Please check the arguments, e.g.,\n"
                  << "> mapf-visualizer assets/random-32-32-20.map "
                     "assets/demo_random-32-32-20.txt [--safe-zones "
                     "assets/demo_random-32-32-20.safe_zones]"
                  << std::endl;

        std::cout << "Got " << argc << " args" << std::endl;
        for (int i = 0; i < argc; ++i) {
            std::cout << argv[i] << " ";
        }
        std::cout << std::endl;
        std::exit(1);
    }
    a.map_file = argv[1];
    a.solution_file = argv[2];
    for (int i = 3; i < argc; ++i) {
        if (std::string(argv[i]) == "--safe-zones" && i + 1 < argc) {
            a.safe_zones_file = argv[++i];
        } else if (std::string(argv[i]) == "--capture-only") {
            a.flg_capture_only = true;
        } else {  // unknown argument
            std::cout << "Unknown argument: " << argv[i] << std::endl;
            std::exit(1);
        }
    }
    return a;
}

int main(int argc, char *argv[])
{
    signal(SIGSEGV, handler);  // install our handler
    args a = parse_args(argc, argv);

    // load graph
    Graph G(a.map_file.c_str());
    SafeZonesManager *safe_zones_manager = nullptr;

    // load plan
    auto solution_file = std::ifstream(a.solution_file);
    Solution solution;
    std::string line;
    std::smatch m, results;
    while (getline(solution_file, line)) {
        // so we only process real solution lines
        if (line.find(":(") == std::string::npos) continue;

        auto iter = line.cbegin();
        Config c;
        while (iter < line.cend()) {
            auto search_end = std::min(iter + 128, line.cend());
            if (std::regex_search(iter, search_end, m, r_pos)) {
                auto x = std::stoi(m[1].str());
                auto y = std::stoi(m[2].str());
                Orientation o = Orientation::NONE;
                if (m[3].matched) {
                    o = Orientation::from_string(m[3].str());
                }
                c.push_back(Pose(G.U[G.width * y + x], o));
                iter += m[0].length();
            } else {
                break;
            }
        }
        solution.push_back(c);
    }
    solution_file.close();

    if (!a.safe_zones_file.empty()) {
        // load safe zones if provided
        safe_zones_manager =
            new SafeZonesManager(a.safe_zones_file, G.U.size());
        Color::init_agent_groups(
            safe_zones_manager->get_num_agent_groups());
    }

    // visualize
    ofSetupOpenGL(100, 100, OF_WINDOW);
    ofRunApp(new ofApp(&G, &solution, safe_zones_manager, a.flg_capture_only));
    return 0;
}
