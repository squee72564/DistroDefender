#ifndef _SERVER_HPP
#define _SERVER_HPP

#include <memory>
#include <thread>
#include <filesystem>

#include <httplib.h>

namespace httpserver
{

enum class Method; // forward declaration

enum class Method
{
    GET = 0,
    POST,
    PUT,
    DELETE,
    ERROR_METHOD
};

namespace
{

    constexpr auto methodToStr(Method method)
    {
        switch (method)
        {
        
            case Method::GET:           return "GET";
            case Method::POST:          return "POST";
            case Method::PUT:           return "PUT";
            case Method::DELETE:        return "DELETE";
            default:
                break;
        }

        return "ERROR_METHOD";
    }

} // namespace

class Server
{
public:
    Server(std::string id);

    ~Server();

    void start(const std::filesystem::path& socketPath, bool useThread = true);

    void stop();

    void addRoute(Method method, const std::string& route, httplib::Server::Handler handler);

    bool isRunning() const noexcept;

private:
    
    std::shared_ptr<httplib::Server> server_;
    std::thread thread_;
    std::string id_;
};

} // namespace httpserver

#endif // _SERVER_HPP

