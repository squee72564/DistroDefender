#include <api/catalog/handlers.hpp>

namespace api::catalog::handlers
{

adapter::RouteHandler resourcePost(const std::shared_ptr<ICatalog>& catalog)
{
    return [weakCatalog = std::weak_ptr<ICatalog>(catalog)](const httplib::Request& req, httplib::Response& res)
    {
        auto result = adapter::getReqAndHandler<ICatalog>(req, weakCatalog);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [catalog, jsonReq] = adapter::getRes(result);
        
        if (auto err = jsonReq.validate(schemas::catalog::getResourcePostRequestSchema()))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "{}",
                    err->message
                )
            );

            return;
        }

        base::Name name;
        try
        {
            Resource::Type type{
                Resource::strToType(jsonReq.getString("/type").value())
            };
            name = base::Name{Resource::typeToStr(type)};
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "Invalid /type field: {}",
                    e.what()
                )
            );
            return;
        }

        Resource targetResource;

        try
        {
            targetResource = Resource{name};
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        const auto invalid = catalog->postResource(
            targetResource,
            jsonReq.getString("/namespaceid").value(), 
            jsonReq.getString("/content").value()
        );

        if (invalid)
        {
            res = adapter::userErrorResponse(invalid.value().message);
            return;
        }
        
        json::Json response{};
        response.setObject("");
        response.setType("/status", schemas::engine::ReturnStatus::OK);

        res = adapter::userResponse(response);
    };
}

adapter::RouteHandler resourceGet(const std::shared_ptr<ICatalog>& catalog)
{
    return [weakCatalog = std::weak_ptr<ICatalog>(catalog)](const httplib::Request& req, httplib::Response& res)
    {
        auto result = adapter::getReqAndHandler<ICatalog>(req, weakCatalog);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [catalog, jsonReq] = adapter::getRes(result);
        
        if (auto err = jsonReq.validate(schemas::catalog::getResourceGetRequestSchema()))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "{}",
                    err->message
                )
            );

            return;
        }

        base::Name name;
        try
        {
            name = base::Name{jsonReq.getString("/name").value()};
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "Invalid /name field: {}",
                    e.what()
                )
            );
            return;
        }

        Resource targetResource;

        try
        {
            targetResource = Resource{name};
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        const auto queryRes =
            catalog->getResource(targetResource, jsonReq.getString("/namespaceid").value());

        if (base::isError(queryRes))
        {
            res = adapter::userErrorResponse(base::getError(queryRes).message);
            return;
        }
        
        const auto& content = base::getResponse<std::string>(queryRes);
        json::Json response{};
        response.setObject("");
        response.setTypeMany({
            {"/status", schemas::engine::ReturnStatus::OK},
            {"/content", content}
        });

        res = adapter::userResponse(response);
    };
}

adapter::RouteHandler resourceDelete(const std::shared_ptr<ICatalog>& catalog)
{
    return [weakCatalog = std::weak_ptr<ICatalog>(catalog)](const httplib::Request& req, httplib::Response& res)
    {
        auto result = adapter::getReqAndHandler<ICatalog>(req, weakCatalog);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [catalog, jsonReq] = adapter::getRes(result);
        
        if (auto err = jsonReq.validate(schemas::catalog::getResourceDeleteRequestSchema()))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "{}",
                    err->message
                )
            );

            return;
        }

        base::Name name;
        try
        {
            name = base::Name{jsonReq.getString("/name").value()};
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "Invalid /name field: {}",
                    e.what()
                )
            );
            return;
        }

        Resource targetResource;

        try
        {
            targetResource = Resource{name};
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        const auto invalid =
            catalog->deleteResource(targetResource, jsonReq.getString("/namespaceid").value());

        if (invalid)
        {
            res = adapter::userErrorResponse(invalid.value().message);
            return;
        }
        
        json::Json response{{
            {"/status", schemas::engine::ReturnStatus::OK}
        }};

        res = adapter::userResponse(response);
    };
}

adapter::RouteHandler resourcePut(const std::shared_ptr<ICatalog>& catalog)
{
    return [weakCatalog = std::weak_ptr<ICatalog>(catalog)](const httplib::Request& req, httplib::Response& res)
    {
        auto result = adapter::getReqAndHandler<ICatalog>(req, weakCatalog);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [catalog, jsonReq] = adapter::getRes(result);
        
        if (auto err = jsonReq.validate(schemas::catalog::getResourcePutRequestSchema()))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "{}",
                    err->message
                )
            );

            return;
        }

        base::Name name;
        try
        {
            name = base::Name{jsonReq.getString("/name").value()};
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "Invalid /name field: {}",
                    e.what()
                )
            );
            return;
        }

        Resource targetResource;

        try
        {
            targetResource = Resource{name};
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        const auto invalid = catalog->putResource(
            targetResource,
            jsonReq.getString("/content").value(),
            jsonReq.getString("/namespaceid").value()
        );

        if (invalid)
        {
            res = adapter::userErrorResponse(invalid.value().message);
            return;
        }
        
        json::Json response{};
        response.setObject("");
        response.setType("/status", schemas::engine::ReturnStatus::OK);

        res = adapter::userResponse(response);
    };
}

adapter::RouteHandler resourceValidate(const std::shared_ptr<ICatalog>& catalog)
{
    return [weakCatalog = std::weak_ptr<ICatalog>(catalog)](const httplib::Request& req, httplib::Response& res)
    {
        auto result = adapter::getReqAndHandler<ICatalog>(req, weakCatalog);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [catalog, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::catalog::getResourceValidateSchema()))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "{}",
                    err->message
                )
            );

            return;
        }

        base::Name name;
        try
        {
            name = base::Name{jsonReq.getString("/name").value()};
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "Invalid /name field: {}",
                    e.what()
                )
            );
            return;
        }

        Resource targetResource;

        try
        {
            targetResource = Resource{name};
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        const auto queryRes = catalog->validateResource(
            targetResource,
            jsonReq.getString("/namespaceid").value(),
            jsonReq.getString("/content").value()
        );

        if (base::isError(queryRes))
        {
            res = adapter::userErrorResponse(base::getError(queryRes).message);
            return;
        }
        
        json::Json response{};
        response.setObject("");
        response.setType("/status", schemas::engine::ReturnStatus::OK);

        res = adapter::userResponse(response);
    };
}

adapter::RouteHandler getNamespaces(const std::shared_ptr<ICatalog>& catalog)
{
    return [weakCatalog = std::weak_ptr<ICatalog>(catalog)](const httplib::Request& req, httplib::Response& res)
    {
        auto result = adapter::getReqAndHandler<ICatalog>(req, weakCatalog);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [catalog, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::catalog::getResourceGetNamespaceRequestSchema()))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "{}",
                    err->message
                )
            );

            return;
        }

        json::Json jsonRes{{
            {"/status", schemas::engine::ReturnStatus::OK},

        }};
        jsonRes.setArray("/namespaces");

        const auto namespaces = catalog->getAllNamespaces();

        for (const auto& namespaceid : namespaces)
        {
            jsonRes.appendString("/namespaces", namespaceid.str());
        }

        res = adapter::userResponse(jsonRes);
    };
}

void registerHandlers(const std::shared_ptr<ICatalog>& catalog,
                             const std::shared_ptr<httpserver::Server>& server)
{
    server->addRoute(httpserver::Method::POST, "/catalog/resource/post", resourcePost(catalog));
    server->addRoute(httpserver::Method::GET, "/catalog/resource/get", resourceGet(catalog));
    server->addRoute(httpserver::Method::DELETE, "/catalog/resource/delete", resourceDelete(catalog));
    server->addRoute(httpserver::Method::PUT, "/catalog/resource/put", resourcePut(catalog));
    server->addRoute(httpserver::Method::POST, "/catalog/resource/validate", resourceValidate(catalog));
    server->addRoute(httpserver::Method::GET, "/catalog/namespaces/get", getNamespaces(catalog));

}
} // namespace api::catalog::handlers
