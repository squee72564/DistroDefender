#include <iostream>

#include <base/logger.hpp>

int main(int argc, char* argv[]) {
    
    // Initialize Logger
    {
            logger::LoggerConfig cfg;
            cfg.level = logger::LogLevel::Info;
            logger::start(cfg);
            LOG_INFO("Logging initialized.");
    }

    return 0;
}
