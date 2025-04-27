#include <store/store.hpp>

#include <algorithm>
#include <list>
#include <set>

#include <base/logger.hpp>

namespace
{

std::vector<base::Name>
cutName(const std::vector<base::Name>& names,
        const std::size_t size,
        const std::size_t depth = 1)
{
    std::set<base::Name> set{};
    const auto nSize{size + depth};

    for (const auto& name : names)
    {
        if (name.parts().size() >= nSize)
        {
            auto itBegin = name.parts().begin();
            auto itEnd = itBegin + nSize;

            set.emplace( std::vector<std::string>{itBegin, itEnd} );
        }
    }

    return std::vector<base::Name>{set.begin(), set.end()};
}

} // namespace

namespace store
{

base::Name Store::prefixNS_s{"namespaces"};

class Store::DBDocNames
{
private:
    std::unordered_map<base::Name, NamespaceId> nameToNs_;
    std::unordered_multimap<NamespaceId, base::Name> nsToNames_;

public:
    std::optional<NamespaceId> getNamespaceId(const base::Name& name) const
    {
        auto it = nameToNs_.find(name);
        if (it == nameToNs_.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    std::vector<base::Name> getDocumentKeys(const NamespaceId& namespaceId) const
    {
        auto range{nsToNames_.equal_range(namespaceId)};
        std::vector<base::Name> names;
        names.reserve(std::distance(range.first, range.second));

        std::transform(
            range.first,
            range.second,
            std::back_inserter(names),
            [](const auto& pair) { return pair.second; }
        );

        return names;
    }

    std::vector<base::Name> getDocumentKeys() const
    {
        std::vector<base::Name> names;
        names.reserve(nameToNs_.size());

        std::transform(
            nameToNs_.begin(),
            nameToNs_.end(),
            std::back_inserter(names),
            [](const auto& pair){ return pair.first; }
        );

        return names;
    }

    bool existsName(const base::Name& name) const
    {
        return nameToNs_.count(name) > 0;
    }

    bool isPrefix(const base::Name& prefix,
                  const base::Name& name,
                  bool strict = true) const
    {
        if (strict)
        {
            if (prefix.parts().size() >= name.parts().size())
            {
                return false;
            }
        }
        else if (prefix.parts().size() > name.parts().size())
        {
            return false;
        }

        return std::equal(
            prefix.parts().cbegin(),
            prefix.parts().cend(),
            name.parts().cbegin()
        );
    }

    bool existsPrefixName(const base::Name& prefix, bool strict = true) const
    {
        const auto found {
            std::find_if(
                nameToNs_.cbegin(),
                nameToNs_.cend(),
                [&](const auto& pair) { return isPrefix(prefix, pair.first, strict); }
            )
        };

        return found != nameToNs_.cend();
    }

    std::vector<base::Name> filterByPrefix(const base::Name& prefix, const NamespaceId& namespaceId, const bool strict = true)
    {
        std::vector<base::Name> names;

        for (const auto& [ns, name] : nsToNames_)
        {
            if (ns == namespaceId && isPrefix(prefix, name, strict))
            {
                names.emplace_back(name);
            }
        }

        return names;
    }

    std::vector<NamespaceId> getNamespaceIds() const
    {
        std::set<NamespaceId> set{};

        for (const auto& [namespaceId, name] : nsToNames_)
        {
            set.insert(namespaceId);
        }

        return std::vector<NamespaceId>{set.cbegin(), set.cend()};
    }

    bool changeNamespaceId(const base::Name& name, const NamespaceId& namespaceId)
    {
        auto itKeyToNs = nameToNs_.find(name);

        if (itKeyToNs == nameToNs_.end())
        {
            return false;
        }

        auto oldNamespaceId = itKeyToNs->second;

        itKeyToNs->second = namespaceId;

        auto range = nsToNames_.equal_range(oldNamespaceId);

        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->second == name)
            {
                nsToNames_.erase(it);
                break;
            }
        }

        nsToNames_.insert( {namespaceId, name} );

        return true;
    }

    bool add(const base::Name& name, const NamespaceId& namespaceId)
    {
        if (nameToNs_.find(name) != nameToNs_.cend())
        {
            return false;
        }

        nameToNs_.insert( {name, namespaceId} );
        nsToNames_.insert( {namespaceId, name} );

        return true;
    }

    void del(const base::Name& name)
    {
        auto itKeyToNs = nameToNs_.find(name);

        if (itKeyToNs == nameToNs_.end())
        {
            return;
        }

        auto range = nsToNames_.equal_range(itKeyToNs->second);

        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->second == name)
            {
                nameToNs_.erase(itKeyToNs);
                nsToNames_.erase(it);
                return;
            }
        }
    }

    void delCol(const base::Name& name, const NamespaceId& namespaceId)
    {
        auto range = nsToNames_.equal_range(namespaceId);

        for (auto it = range.first; it != range.second;)
        {
            if (isPrefix(name, it->second))
            {
                nameToNs_.erase(it->second);
                it = nsToNames_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    bool existsNamespaceId(const NamespaceId& namespaceId) const
    {
        return nsToNames_.find(namespaceId) != nsToNames_.cend();
    }
};

Store::Store(std::shared_ptr<IDriver> driver)
    : driver_{std::move(driver)}
    , cache_{std::make_unique<DBDocNames>()}
    , mutex_{}
{
    if (driver_ == nullptr)
    {
        throw std::runtime_error("Store driver cannot be null");
    }

    auto visitor = [this](const base::Name& name, const NamespaceId& nsid, auto& visitorRef) -> void
    {
        if (driver_->existsDoc(name))
        {
            const auto virtualNameR = realToVirtualName(name);
            const auto& virtualName = std::get_if<base::Name>(&virtualNameR);

            if (!virtualName)
            {
                throw std::runtime_error(
                    fmt::format(
                        "Invalid document name '{}'",
                        name.toStr()
                    )
                );
            }

            if (!cache_->add(*virtualName, nsid))
            {
                LOG_WARNING_L(
                    "Document '{}' already exists in some namespace, "
                    "namespace is not consistent, will be ignored",
                    name.toStr()
                );
            }

            return;
        }

        const auto result = driver_->readCol(name);

        if (const auto err = std::get_if<base::Error>(&result))
        {
            throw std::runtime_error(
                fmt::format(
                    "Error loading collection '{}': {}",
                    name.toStr(),
                    err->message
                )
            );
        }

        const auto& list = std::get<Col>(result);

        for (const auto& subname : list)
        {
            visitorRef(subname, nsid, visitorRef);
        }
    };

    if (!driver_->existsCol(prefixNS_s))
    {
        return;
    }

    const auto namespaces = driver_->readCol(prefixNS_s);

    if (const auto err = std::get_if<base::Error>(&namespaces))
    {
        throw std::runtime_error(
            fmt::format(
                "Error loading namespaces: {}",
                err->message
            )
        );
    }

    const auto& list = std::get<Col>(namespaces);

    for (const auto& name : list)
    {
        if (name.parts().size() == prefixNS_s.parts().size() + NamespaceId::PARTS_NAMESPACE_SIZE
            && driver_->existsCol(name))
        {
            visitor(name, NamespaceId(base::Name{name.parts().back()}), visitor);
            LOG_DEBUG("Loaded namespace '{}'", name.toStr());
        }
        else
        {
            throw std::runtime_error(
                fmt::format(
                    "Invalid namespace '{}', part size: {}, is collection: {}",
                    name.toStr(),
                    name.parts().size(),
                    driver_->existsCol(name)
                )
            );
        }
    }
}

Store::~Store() = default;

std::optional<NamespaceId> Store::getNamespace(const base::Name& name) const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    return cache_->getNamespaceId(name);
}

base::RespOrError<Doc> Store::readDoc(const base::Name& name) const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto& namespaceId = cache_->getNamespaceId(name);
    if (!namespaceId)
    {
        return base::Error{"Document does not exist"};
    }

    auto rname = virtualToRealName(name, *namespaceId);

    return driver_->readDoc(rname);
}

std::vector<NamespaceId> Store::listNamespaces() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return cache_->getNamespaceIds();
}

base::RespOrError<Col> Store::readCol(const base::Name& name,
                                      const NamespaceId& namespaceId) const
{
    std::shared_lock<std::shared_mutex> locl(mutex_);

    auto vcol = cache_->filterByPrefix(name, namespaceId);
    if (vcol.empty())
    {
        return base::Error{
            fmt::format(
                "Collection '{}' does not exist on namespace '{}'",
                name.toStr(),
                namespaceId.name().toStr()
            )
        };
    }

    return cutName(vcol, name.parts().size());
}

bool Store::existsDoc(const base::Name& name) const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return cache_->existsName(name);
}

bool Store::existsCol(const base::Name& name, const NamespaceId& namespaceId) const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto col{cache_->filterByPrefix(name, namespaceId)};
    return !col.empty();
}

base::OptError Store::createDoc(const base::Name& name,
                                const NamespaceId& namespaceId,
                                const Doc& content)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (cache_->existsName(name))
    {
        return base::Error{
            "Document already exists"
        };
    }

    auto rName = virtualToRealName(name, namespaceId);

    auto error = driver_->createDoc(rName, content);

    if (error)
    {
        return error;
    }

    cache_->add(name, namespaceId);

    return std::nullopt;
}

base::OptError Store::updateDoc(const base::Name& name, const Doc& content)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    auto namespaceId = cache_->getNamespaceId(name);

    if (!namespaceId)
    {
        return base::Error{
            "Document does not exist"
        };
    }

    auto rName = virtualToRealName(name, *namespaceId);

    return driver_->updateDoc(rName, content);
}

base::OptError Store::upsertDoc(const base::Name& name,
                                const NamespaceId& namespaceId,
                                const Doc& content)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto namespaceIdCache = cache_->getNamespaceId(name);

    if (namespaceIdCache && namespaceIdCache != namespaceId)
    {
        return base::Error{
            "Document already exists in another namespace"
        };  
    }

    auto rName = virtualToRealName(name, namespaceId);
    auto error = driver_->upsertDoc(rName, content);

    if (error)
    {
        return error;
    }

    if (!namespaceIdCache)
    {
        cache_->add(name, namespaceId);
    }

    return std::nullopt;
}

base::OptError Store::deleteDoc(const base::Name& name)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    const auto& namespaceId = cache_->getNamespaceId(name);

    if (!namespaceId)
    {
        return base::Error{
            "Document does not exist"
        };
    }

    auto rName = virtualToRealName(name, *namespaceId);

    auto error = driver_->deleteDoc(rName);

    if (error)
    {
        return error;
    }

    cache_->del(name);

    return std::nullopt;
}

base::OptError Store::deleteCol(const base::Name& name, const NamespaceId& namespaceId)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (cache_->filterByPrefix(name, namespaceId).empty())
    {
        return base::Error{
            "Collection does not exist"
        };
    }

    auto error = driver_->deleteCol(virtualToRealName(name, namespaceId));

    if (error)
    {
        return error;
    }

    cache_->delCol(name, namespaceId);

    return std::nullopt;
}

base::OptError Store::createInternalDoc(const base::Name& name, const Doc& content)
{
    if (name.parts()[0] == prefixNS_s.parts()[0])
    {
        return base::Error{
            fmt::format(
                "Invalid write internal document name '{}', cannot start with '{}'",
                name.toStr(),
                prefixNS_s.parts()[0]
            )
        };
    }

    return driver_->createDoc(name, content);
}

base::RespOrError<Doc> Store::readInternalDoc(const base::Name& name) const
{
    return driver_->readDoc(name);
}

base::OptError Store::updateInternalDoc(const base::Name& name, const Doc& content)
{
    if (name.parts()[0] == prefixNS_s.parts()[0])
    {
        return base::Error{
            fmt::format(
                "Invalid update internal document name '{}', cannot start with '{}'",
                name.toStr(),
                prefixNS_s.parts()[0]
            )
        };
    }

    return driver_->updateDoc(name, content);
}

base::OptError Store::upsertInternalDoc(const base::Name& name, const Doc& content)
{
    if (name.parts()[0] == prefixNS_s.parts()[0])
    {
        return base::Error{
            fmt::format(
                "Invalid update internal document name '{}', cannot start with '{}'",
                name.toStr(),
                prefixNS_s.parts()[0]
            )
        };
    }

    if (driver_->existsDoc(name))
    {
        return driver_->updateDoc(name, content);
    }

    return driver_->createDoc(name, content);
}

base::OptError Store::deleteInternalDoc(const base::Name& name)
{
    if (name.parts()[0] == prefixNS_s.parts()[0])
    {
        return base::Error{
            fmt::format(
                "Invalid delete internal document name '{}', cannot start with '{}'",
                name.toStr(),
                prefixNS_s.parts()[0]
            )
        };
    }

    return driver_->deleteDoc(name);
}

base::RespOrError<Col> Store::readInternalCol(const base::Name& name) const
{
    return driver_->readCol(name);
}

bool Store::existsInternalDoc(const base::Name& name) const
{
    return driver_->existsDoc(name);
}

} // namespace store
