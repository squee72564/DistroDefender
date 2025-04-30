#ifndef _CATALOG_HPP
#define _CATALOG_HPP

#include <functional>
#include <memory>
#include <unordered_map>

#include <api/catalog/icatalog.hpp>
#include <builder/ivalidator.hpp>

namespace api::catalog
{
struct Config
{
    std::shared_ptr<store::IStore> store;
    std::shared_ptr<builder::IValidator> validator;
    std::string assetSchema;
    std::string environmentSchema;

    void validate() const;
};

class Catalog : public ICatalog
{
private:
    std::shared_ptr<store::IStore> store_;
    std::shared_ptr<builder::IValidator> validator_;

    base::OptError validate(const Resource& item,
                            const std::string& namespaceId,
                            const json::Json& content) const;

    base::RespOrError<store::Doc> getDoc(const Resource& resource) const;

    base::RespOrError<store::Col> getCol(const Resource& resource,
                                         const std::string& namespaceId) const;

    base::OptError delDoc(const Resource& resrouce);

    base::OptError delCol(const Resource& resource, const std::string& namespaceId);

    base::OptError checkResourceInNamespace(const Resource& item,
                                            const std::string& namespaceId,
                                            const std::string& operation) const;

public:

    Catalog(const Config& config);
    ~Catalog() override = default;

    Catalog(const Catalog&) = delete;
    Catalog operator=(const Catalog&) = delete;

    base::OptError
    postResource(const Resource& collection,
                 const std::string& namespaceStr,
                 const std::string& content) override;

    base::OptError
    putResource(const Resource& item,
                const std::string& content,
                const std::string& namespaceId) override;

    base::RespOrError<std::string>
    getResource(const Resource& resource,
                const std::string& namespaceId) const override;

    base::OptError
    deleteResource(const Resource& resource, const std::string& namespaceId) override;

    base::OptError
    validateResource(const Resource& item,
                     const std::string& namespaceId,
                     const std::string& content) const override;

    inline std::vector<store::NamespaceId>
    getAllNamespaces() const override { return store_->listNamespaces(); }

};

} // namespace api::catalog

#endif // _CATALOG_HPP
