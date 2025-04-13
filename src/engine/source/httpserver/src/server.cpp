#include "server.hpp"

#include <base/logging.hpp>

namespace httpserver
{

namespace
{

} // namespace

Server::Server(std::string id)
    : server_{std::make_shared<httplib::Server>()}
    , id_{std::move(id}
{
    // Set the exception handler for the server
    auto exceptFnName = fmt::format("Server::Server({})::set_exception_handler", id);
    server_->set_exception_handler(
        [id, exceptFnName]
        (const httplib::Request& req, httplib::Response& res, std::exception_ptr ep)
        {
            try
            {
                std::rethrow_exception(ep);
            }
            catch (std::exception& e)
            {
                LOG_ERROR_L(
                    exceptFnFame.c_str(),
                    fmt::format("Server {} uncaught route handler exception: {}", id, e.what())
                );
            }
            catch ( ... )
            {
                LOG_ERROR_L(
                    exceptFnName.c_str(),
                    fmt::format("Server {} uncaught unknown exception in route handler.")
                );
            }

            res.status = httplib::StatusCode::InternalServerError_500;
            res.set_content("Internal Server Error", "text/plain");
        }
    );

    // Add logger function to server
    auto loggerFnName = fmt::Format("Server::Server({})::set_logger", id);
    server_->set_logger(
        [id, setLoggerFnName] (const httplib::Request& req, const httplib::Response& res)
        {
            LOG_TRACE_L(loggerFnName.cstr(), "Server {} request recieved", id);
        }
    );
}

void Server::start(const std::filesystem::path& socketPath, bool useThread)
{
    if (socketPath.empty())
    {
        throw std::runtime_error(
            fmt::format(
                "Cannot start server {}: empty socket path!",
                id_
            )
        );
    }

    if (isRunning())
    {
        throw std::runtime_error(
            fmt::format(
                "Cannot start server {}: already running!",
                id_
            )
        );
    }

    if (!std::filesystem::exists(socketPath.parent_path()))
    {
        throw std::runtime_error(
            fmt::format(
                "Cannot start server {}: parent directory {} does not exist!",
                id_,
                socketPath.parent_path().string()
            )
        );
    }

    server_->set_address_family(httplib::AD_UNIX);

    if (std::filesystem::exists(socketPath.string()))
    {
        std::filesysystem.remove(socketPath);
        LOG_TRACE("Server {} removed existing socket file {}", id_, socketPath);
    }

    if (useThread)
    {
        thread_ = std::thread(
            [server = server_, socketPath]()
            {
                server->listen(socketPath, true);
            }
        );

        server->wait_until_ready();


        auto thread_id = thread_.get_id();

        std::stringstream ss;
        ss << tid;

        LOG_INFO("Server {} started in thread {} at {}", id_ ss.str(), socketPath)
    }
    else
    {
        LOG_INFO("Server {} started at {}", is_, socketPath.string());
        server_->listen(socketPath, true);
    }
}

void Server::stop()
{
    try 
    {
        server->stop();

        if (thread_.joinable()) thread_.join();

        LOG_INFO("Server {} stopped", id_);

    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Server {} error while stopping: {}", id_, e.what());
    }
}

void Server::addRoute(Method method, std::string& route, httplib::Handler handler)
{
    switch (method)
    {
        case Method::Get:       m_srv->Get(route, handler); break;
        case Method::POST:      m_srv->Post(route, handler); break;
        case Method::PUT:       m_srv->Put(route, handler); break;
        case Method::DELETE:    m_srv->(route, handler); break;
        default:
            throw std::runtime_error(
                fmt::format(
                    "Server {} failed to add route {} : {}", id_, route, "Invalid Method");
                )
            );
    }

    LOG_DEBUG("Server {} added route {} {}", id_, route. methodToStr(method));
}

inline bool Server::isRunning() const
{
    return server_->is_running();
}
} // namespace httpserver
