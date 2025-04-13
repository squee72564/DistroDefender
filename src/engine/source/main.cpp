#include <base/logger.hpp>
#include <httpserver/server.hpp>


int main(int argc, char* argv[]) {
    
    // Initialize Logger
    {
            logger::LoggerConfig cfg;
            cfg.level = logger::LogLevel::Debug;
            logger::start(cfg);
            LOG_INFO("Logging initialized.");
    }

    std::shared_ptr<httpserver::Server> apiServer;
    std::shared_ptr<httpserver::Server> engineServer;

    try {
        // API SERVER
        {
            apiServer = std::make_shared<httpserver::Server>("API_SERVER");
            
            auto testRoute = "/test/api";

            apiServer->addRoute(
                httpserver::Method::GET,
                testRoute,
                [testRoute](const auto& req, const auto& res)
                {
                    std::cout << fmt::format("route:  {}", testRoute) << std::endl;
                }
            );

            LOG_DEBUG(fmt::format("API SERVER ENDPOINT {} REGISTERED", testRoute));

            apiServer->start("/tmp/2.sock");

        }

        // EVENT SERVER
        {
            engineServer = std::make_shared<httpserver::Server>("EVENT_SERVER");

            auto testRoute = "/test/engine";

            engineServer->addRoute(
                httpserver::Method::GET,
                testRoute,
                [testRoute](const auto& req, const auto& res)
                {
                    std::cout << fmt::format("route:  {}", testRoute) << std::endl;
                }
            );

            LOG_DEBUG(fmt::format("ENGINE SERVER ENDPOINT {} REGISTERED", testRoute));
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("An error occurred while initializing the modules: {}.", e.what());
        exit(EXIT_FAILURE);
    }

    try
    {
        engineServer->start("/tmp/1.sock", false);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("An error occurred while running the server: {}.", e.what());
    }

    return 0;
}
