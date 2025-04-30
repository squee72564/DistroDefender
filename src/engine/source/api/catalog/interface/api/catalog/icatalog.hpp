#ifndef _API_CATALOG_ICATALOG_HPP
#define _API_CATALOG_ICATALOG_HPP

#include <api/catalog/resource.hpp>
#include <base/error.hpp>
#include <store/istore.hpp>

namespace api::catalog
{

class ICatalog
{
public:
    virtual ~ICatalog() = default;

    virtual base::OptError
    postResource(const Resource& collection, const std::string& namespaceStr, const std::string& content) = 0;

    virtual base::OptError
    putResource(const Resource& item, const std::string& content, const std::string& namespaceId) = 0;

    virtual base::RespOrError<std::string>
    getResource(const Resource& resource, const std::string& namespaceId) const = 0;

    virtual base::OptError
    deleteResource(const Resource& resource, const std::string& namespaceId) = 0;

    virtual base::OptError
    validateResource(const Resource& item, const std::string& namespaceId, const std::string& content) const = 0;

    virtual std::vector<store::NamespaceId>
    getAllNamespaces() const = 0;
};

} // namespace api::catalog

#endif // _API_CATALOG_ICATALOG_HPP
