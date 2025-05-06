#include <api/geo/handlers.hpp>
#include <schemas/geo.hpp>
#include <schemas/engine.hpp>

#include <base/json.hpp>
#include <base/error.hpp>

#include <fmt/format.h>

namespace api::geo::handlers
{

adapter::RouteHandler addDb(const std::shared_ptr<::geo::IManager>& geoManager)
{
    return [weakGeoManager = std::weak_ptr<::geo::IManager>(geoManager)](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::geo::IManager>(req, weakGeoManager);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [geoManager, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::geo::getDBPostRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );
            return;
        }

        auto path = jsonReq.getString("/path").value();
        ::geo::Type type{};
        
        if (path.empty())
        {
            res = adapter::userErrorResponse("Path cannot be empty");
            return;
        }

        try
        {
            type = ::geo::typeFromName(jsonReq.getString("/type").value());
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        const auto invalid = geoManager->addDb(path, type);

        if (base::isError(invalid))
        {
            res = adapter::userErrorResponse(
                base::getError(invalid).message
            );
            return;
        }

        json::Json jsonRes{
            {"/status", schemas::engine::ReturnStatus::OK} 
        };

        res = adapter::userResponse(jsonRes);
    };
}

adapter::RouteHandler delDb(const std::shared_ptr<::geo::IManager>& geoManager)
{
    return [weakGeoManager = std::weak_ptr<::geo::IManager>(geoManager)](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::geo::IManager>(req, weakGeoManager);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [geoManager, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::geo::getDBDeleteRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );
            return;
        }

        auto path = jsonReq.getString("/path").value();
        
        if (path.empty())
        {
            res = adapter::userErrorResponse("Path cannot be empty");
            return;
        }

        const auto invalid = geoManager->removeDb(path);

        if (base::isError(invalid))
        {
            res = adapter::userErrorResponse(
                base::getError(invalid).message
            );
            return;
        }

        json::Json jsonRes{
            {"/status", schemas::engine::ReturnStatus::OK} 
        };

        res = adapter::userResponse(jsonRes);
    };
}

adapter::RouteHandler listDb(const std::shared_ptr<::geo::IManager>& geoManager)
{
    return [weakGeoManager = std::weak_ptr<::geo::IManager>(geoManager)](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::geo::IManager>(req, weakGeoManager);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [geoManager, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::geo::getDBListRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );
            return;
        }

        const auto dbs = geoManager->listDbs();


        json::Json jsonRes{
            {"/status", schemas::engine::ReturnStatus::OK}
        };
        jsonRes.setArray("/entries");

        for (const auto& db : dbs)
        {
            jsonRes.appendJson(
                "/entries",
                json::Json{
                    {"/path", db.path},
                    {"/name", db.name},
                    {"/type", ::geo::typeName(db.type)}
                }
            );
        }

        res = adapter::userResponse(jsonRes);
    };
}

adapter::RouteHandler remoteUpsertDb(const std::shared_ptr<::geo::IManager>& geoManager)
{
    return [weakGeoManager = std::weak_ptr<::geo::IManager>(geoManager)](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::geo::IManager>(req, weakGeoManager);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [geoManager, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::geo::getDBRemoteUpsertRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );
            return;
        }

        auto path = jsonReq.getString("/path").value();
        auto dbUrl = jsonReq.getString("/dbUrl").value();
        auto hashUrl = jsonReq.getString("/hashUrl").value();
        ::geo::Type type{};
        
        if (path.empty())
        {
            res = adapter::userErrorResponse("path cannot be empty");
            return;
        }

        if (dbUrl.empty())
        {
            res = adapter::userErrorResponse("dbUrl cannot be empty");
            return;
        }

        if (hashUrl.empty())
        {
            res = adapter::userErrorResponse("hashUrl cannot be empty");
            return;
        }

        try
        {
            type = ::geo::typeFromName(jsonReq.getString("/type").value());
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        const auto invalid = geoManager->remoteUpsertDb(path, type, dbUrl, hashUrl);

        if (base::isError(invalid))
        {
            res = adapter::userErrorResponse(
                base::getError(invalid).message
            );
            return;
        }

        json::Json jsonRes{
            {"/status", schemas::engine::ReturnStatus::OK} 
        };

        res = adapter::userResponse(jsonRes);
    };
}

void registerHandlers(const std::shared_ptr<::geo::IManager>& geoManager,
                      const std::shared_ptr<httpserver::Server>& server)
{
    server->addRoute(httpserver::Method::POST, "/geo/db/add", addDb(geoManager));
    server->addRoute(httpserver::Method::POST, "/geo/db/del", delDb(geoManager));
    server->addRoute(httpserver::Method::POST, "/geo/db/list", listDb(geoManager));
    server->addRoute(httpserver::Method::POST, "/geo/db/remoteUpsert", remoteUpsertDb(geoManager));
}

} // namespace api::geo::handlers
