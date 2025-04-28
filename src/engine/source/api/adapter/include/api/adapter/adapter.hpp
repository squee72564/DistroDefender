#ifndef _API_ADAPTER_HPP
#define _API_ADAPTER_HPP

#include <variant>

#include <fmt/format.h>

#include <httpserver/server.hpp>
#include <base/json.hpp>
#include <schemas/engine.hpp>

namespace api::adapter
{
using RouteHandler = std::function<void(const httplib::Request&, httplib::Response&)>;

struct Error
{
    httplib::Response res;
};

template <typename Res>
using ResOrErrorResp = std::variant<Res, Error>;

template <typename Res>
inline bool isError(const ResOrErrorResp<Res>& res)
{
    return std::holds_alternative<Error>(res);
}

template <typename Res>
Error getError(const ResOrErrorResp<Res>& res)
{
    return std::get<Error>(res);
}

template <typename Res>
httplib::Response getErrorResp(const ResOrErrorResp<Res>& res)
{
    return getError(res).res;
}

template <typename Res>
Res getRes(const ResOrErrorResp<Res>& res)
{
    return std::get<Res>(res);
}

inline httplib::Response internalErrorResponse(const std::string& message)
{
    json::Json json{};
    json.setTypeMany({
        {"/status", schema::ReturnStatus::ERROR},
        {"/error", message}
    });

    httplib::Response response;
    response.status = httplib::StatusCode::InternalServerError_500;
    response.set_content(json.toStr(), "plain/text");

    return response;
}

httplib::Response userResponse(const json::Json& res)
{
    // Check json validity and maybe validate against a schema?
    // Use internalErrorResponse with the base::Error message
    // if it is invalid for some reason?

    httplib::Response response;
    response.status = httplib::StatusCode::OK_200;
    response.set_content(res.toStr(), "plain/text");
    return response;
}



inline httplib::Response userErrorResponse(const std::string& message)
{
    json::Json json{};
    json.setTypeMany({
        {"/status", schema::ReturnStatus::ERROR},
        {"/error", message}
    });

    httplib::Response response;
    response.status = httplib::StatusCode::BadRequest_400;
    response.set_content(json.toStr(), "plain/text");

    return response;
}

ResOrErrorResp<json::Json> parseRequest(const httplib::Request& req)
{
    if (req.body.empty()){
        return json::Json{};
    }

    json::Json reqJson{req.body.c_str()};
    auto errOpt = reqJson.getParseError();

    if (base::isError(errOpt))
    {
        auto err = base::getError(errOpt);
        auto message = fmt::format(
            "Failed to parse json request: {}",
            err.message
        );

        return Error{userErrorResponse(message)};
    }

    return reqJson;
};

inline httplib::Request createRequest(const json::Json& req)
{

    // Add some kind of validation maybe?

    httplib::Request request;
    request.body = req.toStr();
    request.set_header("Content-Type", "plain/text");

    return request;
}

inline json::Json parseResponse(const httplib::Response& res)
{
    if (res.body.empty())
    {
        return json::Json{};
    }

    json::Json resJson{res.body.c_str()};
    auto errOpt = resJson.getParseError();

    if (base::isError(errOpt))
    {
        auto err = base::getError(errOpt);
        auto message = fmt::format(
            "Failed to parse json response: {}",
            err.message
        );

        throw std::runtime_error(message);
    }
    
    return resJson;
}

template <typename IHandler>
using ReqAndHandler = std::tuple<std::shared_ptr<IHandler>, json::Json>;

template <typename IHandler>
ResOrErrorResp<ReqAndHandler<IHandler>> getReqAndHandler(const httplib::Request& req, const std::weak_ptr<IHandler>& weakHandler)
{
    auto handler = weakHandler.lock();
    if (!handler)
    {
        return Error{
            internalErrorResponse("Error: Handler is not initialized")
        };
    }

    auto jsonReq = parseRequest(req);

    if (isError(jsonReq))
    {
        return getError(jsonReq);
    }

    return ReqAndHandler<IHandler>{handler, getRes(jsonReq)};
}

} // namespace api::adapter

#endif // _API_ADAPTER_HPP
