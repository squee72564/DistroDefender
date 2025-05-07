#ifndef _API_ARCHIVER_HANDLERS_HPP
#define _API_ARCHIVER_HANDLERS_HPP

#include <api/adapter/adapter.hpp>
#include <archiver/iarchiver.hpp>

#include <schemas/engine.hpp>

namespace api::archiver::handlers
{
adapter::RouteHandler activateArchiver(const std::shared_ptr<::archiver::IArchiver>& archiver);

adapter::RouteHandler deactivateArchiver(const std::shared_ptr<::archiver::IArchiver>& archiver);

adapter::RouteHandler getArchiverStatus(const std::shared_ptr<::archiver::IArchiver>& archiver);

void registerHandlers(const std::shared_ptr<::archiver::IArchiver>& archiver,
                      const std::shared_ptr<httpserver::Server>& server);

} // namespace api::archiver::handlers

#endif // _API_ARCHIVER_HANDLERS_HPP
