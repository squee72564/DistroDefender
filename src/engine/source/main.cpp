#include <httpserver/server.hpp>
#include <base/logger.hpp>
#include <base/utils/singletonLocator.hpp>
#include <base/utils/singletonLocatorStrategies.hpp>
#include <kvdb/kvdbManager.hpp>
#include <conf/keys.hpp>
#include <conf/conf.hpp>

int main(int argc, char* argv[]) {
    
    // Initialize Logger
    {
            logger::LoggerConfig cfg;
            cfg.level = logger::LogLevel::Debug;
            logger::start(cfg);
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
        exit(EXIT_FAILURE);
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

        LOG_INFO("KVDB initialized.");
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR(fmt::format("Error Initializing KVDB:\n{}", ex.what()));
        exit(EXIT_FAILURE);
    }

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

    return 0;
}
