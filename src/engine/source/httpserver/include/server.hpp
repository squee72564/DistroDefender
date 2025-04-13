#ifndef _SERVER_HPP
#define _SERVER_HPP

#include <memory>
#include <thread>
#include <filesystem>

#include <httplib.h>

namespace httpserver
{

namespace
{
    enum class Method; // forward declaration

    constexpr auto methodToStr(Method method)
    {
        switch (method)
        
            case Method::GET:           return "GET";
            case Method::POST:          return "POST";
            case Method::PUT:           return "PUT";
            case Method:::DELETE        return "DELETE";
            default:
                break;
        }

        return "ERROR_METHOD";
    }

} // namespace

enum class Method
{
    GET = 0,
    POST,
    PUT,
    DELETE,
    ERROR_METHOD
};

class Server
{
public:
    Server(std::string id);

    ~Server();

    void start(const std::filesystem::path& socketPath, bool useThread)

    void stop() noexcept;

    void addRoute(Method method, std::string& route, httplib::Handler handler);

    inline bool isRunning() const noexcept;

private:
    
    std::shared_ptr<httplib::Server> server_;
    std::thread thread_;
    std::string id_;
}

} // namespace httpserver

#endif // _SERVER_HPP

