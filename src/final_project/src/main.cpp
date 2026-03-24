#include "GameEngine.h"
#include "Profiler.h"
#include "Helpers.h"
#include "cxxopts.hpp"

int main(int argc, char* argv[]) {
    PROFILE_FUNCTION();

    cxxopts::Options options("MyProgram", "One line description of MyProgram");

    options.add_options()
        ("c,config", "Path to settings.json", cxxopts::value<std::string>()->default_value("config/settings.json"));

    auto args = options.parse(argc, argv);

    GameEngine eng(args["c"].as<std::string>());
    eng.run();
}