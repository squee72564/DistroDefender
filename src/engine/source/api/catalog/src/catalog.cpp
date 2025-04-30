#include <api/catalog/catalog.hpp>

#include <base/logger.hpp>

#include <fmt/format.h>

#include <store/utils.hpp>

namespace api::catalog
{

void Config::validate() const
{
    if (!store)
    {
        throw std::runtime_error("Store is not set");
    }

    if (!validator)
    {
        throw std::runtime_error("Assets, environments and schemas validator is not set");
    }
}

Catalog::Catalog(const Config& config)
{
    config.validate();

    store_ = config.store;
    validator_ = config.validator;

    LOG_DEBUG(
        "Engine Catalog(const Config& config): Asset schema name: '{}'.",
        config.assetSchema
    );
}

base::OptError
Catalog::postResource(const Resource& collection,
                      const std::string& namespaceStr,
                      const std::string& content)
{
    LOG_DEBUG(
        "Engine catalog: '{}' method: Collection name: '{}'. Contents: '{}'",
        __func__,
        collection.name_.toStr(),
        content
    );

    if (Resource::Type::COLLECTION != collection.type_)
    {
        return base::Error{
            fmt::format(
                "Expected resource type is '{}', but got '{}'",
                Resource::typeToStr(collection.type_),
                Resource::typeToStr(Resource::Type::COLLECTION)
            )
        };
    }

    base::OptError namespaceError{};

    store::NamespaceId namespaceId = [&namespaceError, &namespaceStr]()
    {
        try
        {
            return store::NamespaceId{base::Name(namespaceStr)};
        }
        catch (const std::exception& e)
        {
            namespaceError = base::Error{
                fmt::format(
                    "Invalid namespace '{}': {}",
                    namespaceStr,
                    e.what()
                )
            };

             return store::NamespaceId{};
        }
    }();

    if (namespaceError)
    {
        return namespaceError;
    }

    json::Json contentJson{content.c_str()};

    base::OptError parseCheck = contentJson.getParseError();

    if (base::isError(parseCheck))
    {
        return base::getError(parseCheck);
    }

    const auto contentNameStr = contentJson.getString("/name");
    if (!contentNameStr)
    {
        return base::Error{
            "Field 'name' is missing in content"
        };
    }
    else if (contentNameStr.value().empty())
    {
        return base::Error{
            "Field 'name' cannot be empty"
        };
    }

    base::Name contentName;
    Resource contentResource;

    try
    {
        contentName = base::Name(contentNameStr.value());
        contentResource = Resource(contentName);
    }
    catch (const std::exception& e)
    {
        return base::Error{
            fmt::format(
                "Invalid content name '{}': {}",
                contentNameStr.value(),
                e.what()
            )
        };
    }

    if (Resource::Type::COLLECTION == contentResource.type_)
    {
        return base::Error{
            fmt::format(
                "The asset '{}' cannot be added to the store:"
                " The name format is not valid as it is "
                "identified as a 'collection'",
                contentNameStr.value()
            )
        };
    }

    for (auto i = 0; i < collection.name_.parts().size(); ++i)
    {
        if (collection.name_.parts()[i] != contentName.parts()[i])
        {
            return base::Error{
                fmt::format(
                    "Invalid content name '{}' for collection '{}'",
                    contentName.toStr(),
                    collection.name_.toStr()
                )
            };
        }
    }

    if (contentResource.validation_)
    {
        const auto validationError = validate(contentResource, namespaceStr, contentJson);

        if (validationError)
        {
            return base::Error{
                fmt::format(
                    "An error ocurred while trying to validate '{}' {}",
                    contentNameStr.value(),
                    validationError.value().message
                )
            };
        }
    }

    const auto storeError = store::utils::add(
        store_, contentResource.name_, namespaceId, contentJson, content
    );

    if (storeError)
    {
        return base::Error{
            fmt::format(
                "Content '{}' could not be added to the store: {}",
                contentNameStr.value(),
                storeError.value().message
            )
        };
    }

    return base::noError();
}

base::OptError
Catalog::checkResourceInNamespace(const Resource& item,
                                  const std::string& namespaceId,
                                  const std::string& operation) const
{
    if (namespaceId.empty())
    {
        return base::Error{
            "Namespace id cannot be empty"
        };
    }

    auto ns =
        [store = store_](const Resource& item) -> base::RespOrError<store::NamespaceId>
    {
        auto ns = store->getNamespace(item.name_);

        if (!ns)
        {
            return base::Error{
                fmt::format(
                    "Resource '{}' does not have associated namespace.",
                    item.name_
                )
            };
        }

        return ns.value();
    }(item);

    if (base::isError(ns))
    {
        return base::Error{
            fmt::format(
                "Could not {} resource '{}': {}",
                operation, item.name_.toStr(),
                base::getError(ns).message
            )
        };
    }

    if (namespaceId != base::getResponse(ns).str())
    {
        return base::Error{
            fmt::format(
                "Could not {} resource '{}': Does not exist in the '{}' namespace",
                operation,
                item.name_.toStr(),
                namespaceId
            )
        };
    }

    return base::noError();
}

base::OptError
Catalog::putResource(const Resource& item,
                     const std::string& content,
                     const std::string& namespaceId)
{
    LOG_DEBUG(
        "Engine catalog: '{}' method: Item name: '{}'.",
        __func__,
        item.name_.toStr()
    );

    if (Resource::Type::DECODER != item.type_ &&
        Resource::Type::RULE != item.type_ &&
        Resource::Type::FILTER != item.type_ &&
        Resource::Type::OUTPUT != item.type_ &&
        Resource::Type::INTEGRATION != item.type_)
    {
        return base::Error{
            fmt::format(
                "Invalid resource type '{}' for PUT operation",
                Resource::typeToStr(item.type_)
            )
        };
    }

    auto error = checkResourceInNamespace(item, namespaceId, "PUT");

    if (base::isError(error))
    {
        return base::getError(error);
    }

    json::Json contentJson{content.c_str()};

    base::OptError parseCheck = contentJson.getParseError();

    if (base::isError(parseCheck))
    {
        return base::getError(parseCheck);
    }

    const auto contentNameStr = contentJson.getString("/name");

    if (!contentNameStr)
    {
        return base::Error{
            "Field 'name' is missing in content"
        };
    }

    base::Name contentName;

    try
    {
        contentName = base::Name(contentNameStr.value());
    }
    catch (const std::exception& e)
    {
        return base::Error{
            fmt::format(
                "Invalid content name '{}': {}",
                contentNameStr.value(),
                e.what()
            )   
        };
    }

    if (contentName != item.name_)
    {
        return base::Error{
            fmt::format(
                "Invalid content name '{}' of '{}' for type '{}'",
                contentNameStr.value(),
                item.name_.toStr(),
                Resource::typeToStr(item.type_)
            )
        };
    }

    if (item.validation_)
    {
        const auto validationError = validate(item, namespaceId, contentJson);

        if (validationError)
        {
            return base::Error{
                fmt::format(
                    "An error occurred while trying to validate '{}' : {}",
                    contentNameStr.value(),
                    validationError.value().message
                )
            };
        }
    }

    const auto storeError = store::utils::update(store_, item.name_, contentJson, content);

    if (storeError)
    {
        return base::Error{
            fmt::format(
                "Content '{}' could not be updated in store: {}",
                contentNameStr.value(),
                storeError.value().message
            )
        };
    }

    return base::noError();
}

base::RespOrError<store::Doc> Catalog::getDoc(const Resource& resource) const
{
    return store::utils::get(
        store_, resource.name_, true
    );
}

base::RespOrError<store::Col> Catalog::getCol(const Resource& resource, const std::string& namespaceId) const
{
    return store_->readCol(resource.name_, store::NamespaceId{base::Name{namespaceId}});
}

base::RespOrError<std::string> Catalog::getResource(const Resource& resource, const std::string& namespaceId) const
{
    if (Resource::Type::COLLECTION == resource.type_)
    {
        store::Col mergedCol{};
        auto colresult = getCol(resource, namespaceId);

        if (base::isError(colresult))
        {
            return base::getError(colresult);
        }

        auto col = base::getResponse<store::Col>(std::move(colresult));
        mergedCol.insert(
           mergedCol.end(),
           std::make_move_iterator(col.begin()),
           std::make_move_iterator(col.end())
        );

        json::Json content;
        content.setArray("");

        for (const auto& item : mergedCol)
        {
            content.appendString("", item.toStr());
        }

        return content.toStr();
    }

    auto error = checkResourceInNamespace(resource, namespaceId, "GET");

    if (base::isError(error))
    {
        return base::getError(error);
    }

    auto docResult = getDoc(resource);

    if (base::isError(docResult))
    {
        return base::getError(docResult);
    }

    auto doc = base::getResponse<store::Doc>(docResult);

    return doc.getJsonDOM("/json")->toStr();
}

base::OptError Catalog::delDoc(const Resource& resource)
{
    return store_->deleteDoc(resource.name_);
}

base::OptError Catalog::delCol(const Resource& resource, const std::string& namespaceId)
{
    return store_->deleteCol(resource.name_, store::NamespaceId{base::Name{namespaceId}});
}

base::OptError Catalog::deleteResource(const Resource& resource, const std::string& namespaceId)
{
    if (Resource::Type::COLLECTION == resource.type_)
    {
        const auto delColError = delCol(resource, namespaceId);

        if (delColError)
        {
            return base::Error{
                fmt::format(
                    "Could not delete collection '{}' : {}\n",
                    resource.name_.toStr(),
                    delColError.value().message
                )
            };
        }

        return base::noError();
    }

    auto error = checkResourceInNamespace(resource, namespaceId, "DELETE");

    if (base::isError(error))
    {
        return base::getError(error);
    }

    return delDoc(resource);
}

base::OptError Catalog::validate(const Resource& item, const std::string& namespaceId, const json::Json& content) const
{
    if (Resource::Type::DECODER != item.type_ &&
        Resource::Type::RULE != item.type_ &&
        Resource::Type::FILTER != item.type_ &&
        Resource::Type::OUTPUT != item.type_ &&
        Resource::Type::INTEGRATION != item.type_)
    {
        return base::Error{
            fmt::format(
                "Invalid resource type '{}'",
                Resource::typeToStr(item.type_)
            )
        };
    }
    
    if (Resource::Type::INTEGRATION == item.type_)
    {
        if (namespaceId.empty())
        {
            return base::Error{
                fmt::format(
                    "Missing /namespaceid parameter for type '{}'",
                    Resource::typeToStr(item.type_)
                )
            };
        }

        return validator_->validateIntegration(content, namespaceId);
    }

    return validator_->validateAsset(content);
}

base::OptError
Catalog::validateResource(const Resource& item, const std::string& namespaceId, const std::string& content) const
{
    if (Resource::Type::DECODER != item.type_ &&
        Resource::Type::RULE != item.type_ &&
        Resource::Type::FILTER != item.type_ &&
        Resource::Type::OUTPUT != item.type_ &&
        Resource::Type::INTEGRATION != item.type_)
    {
        return base::Error{
            fmt::format(
                "Invalid resource type '{}' for VALIDATE operation",
                Resource::typeToStr(item.type_)
            )
        };
    }

    const json::Json contentJson{content.c_str()};

    base::OptError parseError = contentJson.getParseError();

    if (base::isError(parseError))
    {
        return base::getError(parseError);
    }

    return validate(item, namespaceId, contentJson);
}

} // namespace api::catalog
