#include <signal.h>

#include <httpserver/server.hpp>
#include <base/logger.hpp>
#include <base/utils/singletonLocator.hpp>
#include <base/utils/singletonLocatorStrategies.hpp>
#include <kvdb/kvdbManager.hpp>
#include <conf/keys.hpp>
#include <conf/conf.hpp>
#include <store/store.hpp>
#include <store/drivers/fileDriver.hpp>

#include "StackExecutor.hpp"

std::shared_ptr<httpserver::Server> g_engineServer{nullptr};

cmd::details::StackExecutor g_exitHandler{};

void sigintHandler(const int signum)
{
    if (g_engineServer)
    {
        g_engineServer.reset();
    }
}

void sigtermHandler(const int signum)
{
    g_exitHandler.execute(); 
}

int main(int argc, char* argv[]) {
    
    // exit handler


    // Initialize Logger
    {
            logger::LoggerConfig cfg;
            cfg.level = logger::LogLevel::Debug;
            logger::start(cfg);

            g_exitHandler.add( [](){ logger::stop(); } );
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
        g_exitHandler.execute();
        exit(EXIT_FAILURE);
    }

    // Set signal [SIGINT]: Ctrl+C handler
    {
        struct sigaction sigIntHandler = {};
        sigIntHandler.sa_handler = sigintHandler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, nullptr);
    }

    // Set signal [SIGINT]: SIGTERM (kill <pid>)
    {
        struct sigaction sigTermHandler = {};
        sigTermHandler.sa_handler = sigtermHandler;
        sigemptyset(&sigTermHandler.sa_mask);
        sigTermHandler.sa_flags = 0;
        sigaction(SIGTERM, &sigTermHandler, nullptr);
    }

    // Set signal [SIGINT]: Broken pipe handler
    {
        struct sigaction sigPipeHandler = {};
        sigPipeHandler.sa_handler = SIG_IGN;
        sigemptyset(&sigPipeHandler.sa_mask);
        sigPipeHandler.sa_flags = 0;
        sigaction(SIGPIPE, &sigPipeHandler, nullptr);
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
    std::shared_ptr<kvdbManager::KVDBManager> kvdbManager{nullptr};
    std::shared_ptr<store::Store> store;

    // KVDB
    try {

        kvdbManager::KVDBManagerOptions kvdbOptions{
            confManager.get<std::string>(conf::key::KVDB_PATH), "kvdb"
        };

        kvdbManager = std::make_shared<kvdbManager::KVDBManager>(kvdbOptions);

        kvdbManager->initialize();

        g_exitHandler.add(
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
        g_exitHandler.execute();
        exit(EXIT_FAILURE);
    }

    // Store
    try
    {
        auto fileStorage = confManager.get<std::string>(conf::key::STORE_PATH);
        auto fileDriver = std::make_shared<store::drivers::FileDriver>(fileStorage);
        store = std::make_shared<store::Store>(fileDriver);
        LOG_INFO("Store Initialized");
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR("Error Initializing Store:\n{}", ex.what());
        g_exitHandler.execute();
        exit(EXIT_FAILURE);
    }

    try {
        // API SERVER
        {
            apiServer = std::make_shared<httpserver::Server>("API_SERVER");

            g_exitHandler.add(
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
            g_engineServer = std::make_shared<httpserver::Server>("EVENT_SERVER");

            auto testRoute = "/test/engine";

            g_engineServer->addRoute(
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
        g_exitHandler.execute();
        exit(EXIT_FAILURE);
    }

    try
    {
        g_engineServer->start(
            confManager.get<std::string>(conf::key::SERVER_EVENT_SOCKET),
            false
        );
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("An error occurred while running the server: {}.", e.what());
    }

    // Clean exit
    g_exitHandler.execute();

    return 0;
}
