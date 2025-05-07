#include <api/archiver/handlers.hpp>

namespace api::archiver::handlers
{

adapter::RouteHandler activateArchiver(const std::shared_ptr<::archiver::IArchiver>& archiver)
{
    return [weakArchiver = std::weak_ptr(archiver)](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::archiver::IArchiver>(req, weakArchiver);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [archiver, jsonReq] = adapter::getRes(result);

        archiver->activate();

        res = adapter::userResponse(
            json::Json{{
                {"/status", schemas::engine::ReturnStatus::OK}
            }}
        );
    };
}

adapter::RouteHandler deactivateArchiver(const std::shared_ptr<::archiver::IArchiver>& archiver)
{
    return [weakArchiver = std::weak_ptr(archiver)](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::archiver::IArchiver>(req, weakArchiver);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [archiver, jsonReq] = adapter::getRes(result);

        archiver->deactivate();

        res = adapter::userResponse(
            json::Json{{
                {"/status", schemas::engine::ReturnStatus::OK}
            }}
        );
    };
}

adapter::RouteHandler getArchiverStatus(const std::shared_ptr<::archiver::IArchiver>& archiver)
{
    return [weakArchiver = std::weak_ptr(archiver)](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::archiver::IArchiver>(req, weakArchiver);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [archiver, jsonReq] = adapter::getRes(result);

        const auto isActive = archiver->isActive();

        res = adapter::userResponse(
            json::Json{{
                {"/status", schemas::engine::ReturnStatus::OK},
                {"/active", isActive}
            }}
        );
    };
}

void registerHandlers(const std::shared_ptr<::archiver::IArchiver>& archiver,
                      const std::shared_ptr<httpserver::Server>& server)
{
    server->addRoute(httpserver::Method::POST, "/archiver/activate", activateArchiver(archiver));
    server->addRoute(httpserver::Method::POST, "/archiver/deactivate", deactivateArchiver(archiver));
    server->addRoute(httpserver::Method::POST, "/archiver/status", getArchiverStatus(archiver));
}

} // namespace api::archiver::handlers
