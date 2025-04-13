#include "server.hpp"

#include <base/logger.hpp>

namespace httpserver
{

namespace
{

} // namespace

Server::Server(std::string id)
    : server_{std::make_shared<httplib::Server>()}
    , id_{std::move(id)}
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
                    exceptFnName.c_str(),
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
    auto loggerFnName = fmt::format("Server::Server({})::set_logger", id);
    server_->set_logger(
        [id, loggerFnName] (const httplib::Request& req, const httplib::Response& res)
        {
            LOG_TRACE_L(loggerFnName.c_str(), "Server {} request recieved", id);
        }
    );
}

Server::~Server()
{
    stop();
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

    server_->set_address_family(AF_UNIX);

    if (std::filesystem::exists(socketPath.string()))
    {
        std::filesystem::remove(socketPath);
        LOG_TRACE("Server {} removed existing socket file {}", id_, socketPath.string());
    }

    if (useThread)
    {
        thread_ = std::thread(
            [server = server_, socketPath]()
            {
                server->listen(socketPath, true);
            }
        );

        server_->wait_until_ready();


        auto thread_id = thread_.get_id();

        std::stringstream ss;
        ss << thread_id;

        LOG_INFO("Server {} started in thread {} at {}", id_, ss.str(), socketPath.string());
    }
    else
    {
        LOG_INFO("Server {} started at {}", id_, socketPath.string());
        server_->listen(socketPath, true);
    }
}

void Server::stop()
{
    try 
    {
        server_->stop();

        if (thread_.joinable()) thread_.join();

        LOG_INFO("Server {} stopped", id_);

    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Server {} error while stopping: {}", id_, e.what());
    }
}

void Server::addRoute(Method method, const std::string& route, httplib::Server::Handler handler)
{
    switch (method)
    {
        case Method::GET:       server_->Get(route, handler); break;
        case Method::POST:      server_->Post(route, handler); break;
        case Method::PUT:       server_->Put(route, handler); break;
        case Method::DELETE:    server_->Delete(route, handler); break;
        default:
            throw std::runtime_error(
                fmt::format(
                    "Server {} failed to add route {} : {}", id_, route, "Invalid Method"
                )
            );
    }

    LOG_DEBUG("Server {} added route {} {}", id_, route, methodToStr(method));
}

bool Server::isRunning() const noexcept
{
    return server_->is_running();
}

} // namespace httpserver
