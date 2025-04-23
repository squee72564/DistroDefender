#include <httpserver/server.hpp>
#include <base/logger.hpp>
#include <base/utils/singletonLocator.hpp>
#include <base/utils/singletonLocatorStrategies.hpp>
#include <kvdb/kvdbManager.hpp>
#include <conf/keys.hpp>
#include <conf/conf.hpp>

#include "StackExecutor.hpp"

int main(int argc, char* argv[]) {
    
    // exit handler

    cmd::details::StackExecutor exitHandler{};

    // Initialize Logger
    {
            logger::LoggerConfig cfg;
            cfg.level = logger::LogLevel::Debug;
            logger::start(cfg);

            exitHandler.add( [](){ logger::stop(); } );
            LOG_INFO("Logging initialized.");
    }

    // Load configuration

    auto confManager = conf::Conf(std::make_shared<conf::ConfLoader>());

    try
    {
        confManager.load();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Error loading configuration: {}", e.what());
        exitHandler.execute();
        exit(EXIT_FAILURE);
    }
    
    try
    {
        // Set new log level if config us different than default
        const auto level = logger::strToLevel(
            confManager.get<std::string>(conf::key::LOG_LEVEL)
        );

        const auto currLevel = logger::getLevel();

        if (level != currLevel)
        {
            logger::setLevel(level);
            LOG_DEBUG("Changed log level to '{}'", logger::levelToStr(level));
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Error setting log level from config:\n{}", e.what());
    }

    std::shared_ptr<httpserver::Server> apiServer{nullptr};
    std::shared_ptr<httpserver::Server> engineServer{nullptr};

    std::shared_ptr<kvdbManager::KVDBManager> kvdbManager{nullptr};

    // KVDB
    try {

        kvdbManager::KVDBManagerOptions kvdbOptions{
            confManager.get<std::string>(conf::key::KVDB_PATH), "kvdb"
        };

        kvdbManager = std::make_shared<kvdbManager::KVDBManager>(kvdbOptions);

        kvdbManager->initialize();

        exitHandler.add(
            [kvdbManager]()
            {
                kvdbManager->finalize();
                LOG_INFO("KVDB Terminated.");
            }
        );

        LOG_INFO("KVDB initialized.");
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR("Error Initializing KVDB:\n{}", ex.what());
        exitHandler.execute();
        exit(EXIT_FAILURE);
    }

    try {
        // API SERVER
        {
            apiServer = std::make_shared<httpserver::Server>("API_SERVER");

            exitHandler.add(
                [apiServer]()
                {
                    apiServer->stop();
                    LOG_INFO("Api server shut down.");
                }
            );
            
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

            apiServer->start(
                confManager.get<std::string>(conf::key::SERVER_API_SOCKET)
            );

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

            LOG_DEBUG(
                fmt::format(
                    "ENGINE SERVER ENDPOINT {} REGISTERED",
                    testRoute
                )
            );
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("An error occurred while initializing the modules: {}.", e.what());
        exitHandler.execute();
        exit(EXIT_FAILURE);
    }

    try
    {
        engineServer->start(
            confManager.get<std::string>(conf::key::SERVER_EVENT_SOCKET),
            false
        );
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("An error occurred while running the server: {}.", e.what());
    }

    // Clean exit
    exitHandler.execute();

    return 0;
}
