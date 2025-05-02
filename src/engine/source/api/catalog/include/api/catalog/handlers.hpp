#ifndef _API_CATALOG_HANDLERS_HPP 
#define _API_CATALOG_HANDLERS_HPP 

#include <api/adapter/adapter.hpp>
#include <api/catalog/icatalog.hpp>

namespace api::catalog::handlers
{

adapter::RouteHandler resourcePost(const std::shared_ptr<ICatalog>& catalog);
adapter::RouteHandler resourceGet(const std::shared_ptr<ICatalog>& catalog);
adapter::RouteHandler resourceDelete(const std::shared_ptr<ICatalog>& catalog);
adapter::RouteHandler resourcePut(const std::shared_ptr<ICatalog>& catalog);
adapter::RouteHandler resourceValidate(const std::shared_ptr<ICatalog>& catalog);
adapter::RouteHandler getNamespaces(const std::shared_ptr<ICatalog>& catalog);

void registerHandlers(const std::shared_ptr<ICatalog>& catalog,
                      const std::shared_ptr<httpserver::Server>& server);

} // namespace api::catalog::handlers

#endif // _API_CATALOG_HANDLERS_HPP 
