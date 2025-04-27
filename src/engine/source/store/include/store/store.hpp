#ifndef _STORE_HPP
#define _STORE_HPP

#include <memory>
#include <shared_mutex>
#include <vector>
#include <optional>

#include <store/idriver.hpp>
#include <store/istore.hpp>

namespace store
{

class Store : public IStore
{
private:
    static base::Name prefixNS_s;

    std::shared_ptr<IDriver> driver_;

    class DBDocNames;

    std::unique_ptr<DBDocNames> cache_;

    mutable std::shared_mutex mutex_;

    static inline base::Name virtualToRealName(const base::Name& virtualName, const NamespaceId& namespaceId)
    {
        return prefixNS_s + namespaceId.name() + virtualName;
    }

    static inline base::RespOrError<base::Name>
    realToVirtualName(const base::Name& realName)
    {
        const auto& partsRN = realName.parts();

        const auto& prefixNSParts = prefixNS_s.parts();

        if (partsRN.size() < prefixNSParts.size() + NamespaceId::PARTS_NAMESPACE_SIZE + 1)
        {
            return base::Error{"Invalid real name, too short"};
        }
        
        for (auto i = 0; i < prefixNSParts.size(); ++i)
        {
            if (partsRN[i] != prefixNSParts[i])
            {
                return base::Error{"Invalid real name, prefix does not match"};
            }
        }

        auto namespaceName = base::Name(
            std::vector<std::string>(
                partsRN.begin() + prefixNSParts.size(),
                partsRN.begin() + prefixNSParts.size() + NamespaceId::PARTS_NAMESPACE_SIZE
            )
        );

        auto resp = NamespaceId::fromName(namespaceName);

        if (base::isError(resp))
        {
            return base::Error{"Invalid real name, namespace does not match"};
        }

        return base::Name(
            std::vector<std::string>(
                partsRN.begin() + prefixNSParts.size() + NamespaceId::PARTS_NAMESPACE_SIZE,
                partsRN.end()
            )
        );
    }

    static inline Col virtualToRealCol(const Col& virtualCol, const NamespaceId& namespaceId)
    {
        Col realCol{};

        for (const auto& virtualName : virtualCol)
        {
            realCol.emplace_back( virtualToRealName(virtualName, namespaceId) );
        }

        return realCol;
    }

    static inline base::RespOrError<Col> realToVirtualCol(const Col& realCol)
    {
        Col virtualCol{};

        for (const auto& realName : realCol) 
        {
            auto resp = realToVirtualName(realName);

            if (base::isError(resp))
            {
                return base::Error{
                    fmt::format(
                        "Invalid real name '{}' in collection",
                        realName.toStr()
                    )
                };
            }

            virtualCol.emplace_back( base::getResponse<base::Name>(resp) );
        }

        return virtualCol;
    }

public:
    
    Store(std::shared_ptr<IDriver> driver);

    ~Store() override;

    base::RespOrError<Doc> readDoc(const base::Name& name) const override;

    base::RespOrError<Col> readCol(const base::Name& name, const NamespaceId& namespaceId) const override;

    bool existsDoc(const base::Name& name) const override;

    bool existsCol(const base::Name& name, const NamespaceId& namepspaceId) const override;

    std::vector<NamespaceId> listNamespaces() const override;

    std::optional<NamespaceId> getNamespace(const base::Name& name) const override;

    base::OptError createDoc(const base::Name& name, const NamespaceId& namespaceId, const Doc& content) override;

    base::OptError updateDoc(const base::Name& name, const Doc& content) override;

    base::OptError upsertDoc(const base::Name& name, const NamespaceId& namespaceId, const Doc& content) override;

    base::OptError deleteDoc(const base::Name& name) override;

    base::OptError deleteCol(const base::Name& name, const NamespaceId& namespaceId) override;

    base::OptError createInternalDoc(const base::Name& name, const Doc& content) override;

    base::RespOrError<Doc> readInternalDoc(const base::Name& name) const override;

    base::OptError updateInternalDoc(const base::Name& name, const Doc& content) override;

    base::OptError upsertInternalDoc(const base::Name& name, const Doc& content) override;

    base::OptError deleteInternalDoc(const base::Name& name) override;

    base::RespOrError<Col> readInternalCol(const base::Name& name) const override;

    bool existsInternalDoc(const base::Name& name) const override;
};

} // namespace store

#endif // _STORE_HPP
