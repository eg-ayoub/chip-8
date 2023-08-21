#include <iostream>
#include <cxxopts.hpp>
#include <spdlog/spdlog.h>

#include <application.hpp>

int main(int argc, char *argv[])
{
    // parse cli args
    cxxopts::Options options("Chip-8", "Run of the mill chip-8 emulator");

    options.add_options()("d,debug", "Enable debug mode", cxxopts::value<bool>()->default_value("false"))("r,rom", "Path to rom", cxxopts::value<std::string>())("f,font", "Path to font", cxxopts::value<std::string>()->default_value("nofont"))("i,instructions", "Number of instructions per second", cxxopts::value<uint>()->default_value("500"))("h,help", "Print usage");

    cxxopts::ParseResult result = options.parse(argc, argv);

    // help
    if (result.count("help") || !result.count("rom"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    int retcode = 0;

    // setup logger
    spdlog::set_level(spdlog::level::info);
    if (result["debug"].as<bool>())
    {
        spdlog::set_level(spdlog::level::debug);
    }

    // initialize app
    spdlog::info("initializing chip-8");
    application::Application *app;
    try
    {
        app = new application::Application(result["instructions"].as<uint>(), result["rom"].as<std::string>(), result["font"].as<std::string>());
        app->init();
    }
    catch (std::runtime_error &e)
    {
        spdlog::error("Failed to initialize chip-8 : {}", e.what());
        retcode = 1;
    }

    // run app
    if (retcode == 0)
    {
        try
        {
            spdlog::info("running chip-8");
            app->run();
        }
        catch (std::runtime_error &e)
        {
            spdlog::error("Runtime error : {}", e.what());
            retcode = 1;
        }
    }
    
    // exit gracefully
    delete app;
    spdlog::info("exiting");
    exit(retcode);
}
