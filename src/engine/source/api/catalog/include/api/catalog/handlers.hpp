#ifndef _API_CATALOG_HANDLERS_HPP 
#define _API_CATALOG_HANDLERS_HPP 

#include <api/adapter/adapter.hpp>
#include <api/catalog/icatalog.hpp>

#include <schemas/catalog.hpp>

#include <memory>

namespace api::catalog::handlers
{

adapter::RouteHandler resourcePost(const std::shared_ptr<ICatalog>& catalog);
adapter::RouteHandler resourceGet(const std::shared_ptr<ICatalog>& catalog);
adapter::RouteHandler resourceDelete(const std::shared_ptr<ICatalog>& catalog);
adapter::RouteHandler resourcePut(const std::shared_ptr<ICatalog>& catalog);
adapter::RouteHandler resourceValidate(const std::shared_ptr<ICatalog>& catalog);
adapter::RouteHandler getNamespaces(const std::shared_ptr<ICatalog>& catalog);

inline void registerHandlers(const std::shared_ptr<ICatalog>& catalog,
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

#endif // _API_CATALOG_HANDLERS_HPP 
