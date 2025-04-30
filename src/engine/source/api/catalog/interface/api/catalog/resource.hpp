#ifndef _API_CATALOG_RESOURCE_HPP
#define _API_CATALOG_RESOURCE_HPP

#include <cstring>
#include <exception>
#include <string>

#include <fmt/format.h>

#include <base/name.hpp>
#include <schemas/catalog.hpp>

namespace api::catalog
{

class Resource
{
public:

    using Type = schemas::catalog::ResourceType;

    constexpr static auto ASSET = 0;

    constexpr static auto typeToStr(Type type)
    {
        switch(type)
        {
            case Type::DECODER: return "decoder";
            case Type::RULE: return "rule";
            case Type::FILTER: return "filter";
            case Type::OUTPUT: return "output";
            case Type::SCHEMA: return "schema";
            case Type::COLLECTION: return "collection";
            case Type::INTEGRATION: return "integration";
            default: return "unknown";
        }
    }

    constexpr static auto strToType(std::string_view type)
    {
        if (type == typeToStr(Type::DECODER))
        {
            return Type::DECODER;
        }
        else if (type == typeToStr(Type::RULE))
        {
            return Type::RULE;
        }
        else if (type == typeToStr(Type::FILTER))
        {
            return Type::FILTER;
        }
        else if (type == typeToStr(Type::OUTPUT))
        {
            return Type::OUTPUT;
        }
        else if (type == typeToStr(Type::SCHEMA))
        {
            return Type::SCHEMA;
        }
        else if (type == typeToStr(Type::COLLECTION))
        {
            return Type::COLLECTION;
        }
        else if (type == typeToStr(Type::INTEGRATION))
        {
            return Type::INTEGRATION;
        }

        return Type::UNKNOWN; // For unknown types (errors)
    }

    base::Name name_;
    Type type_;
    bool validation_;

    Resource()
        : name_{"ERROR_NAME"}
        , type_{Type::UNKNOWN}
        , validation_{false}
    {}

    Resource(const base::Name& name)
        : name_{name}
        , type_{Type::UNKNOWN}
        , validation_{false}
    {
        const auto& nameParts = name.parts();
        const auto namePartsSize = nameParts.size();

        if (namePartsSize == 1 || namePartsSize == 2)
        {
            type_ = Type::COLLECTION;

            if (Type::UNKNOWN == strToType(nameParts[0].c_str()))
            {
                throw std::runtime_error(
                    fmt::format(
                        "Missing /type parameter or is invalid"
                    )
                );
            }
        }
        else if (namePartsSize == 3)
        {
            type_ = strToType(nameParts[0].c_str());
            
            if (Type::UNKNOWN == type_)
            {
                throw std::runtime_error(
                    fmt::format(
                        "Missing /type parameter or is invalid"
                    )
                );
            }
            else if (Type::COLLECTION == type_)
            {
                throw std::runtime_error(
                    fmt::format(
                        "Missing /type parameter or is invalid"
                    )
                );
            }

            if (Type::DECODER == type_ ||
                Type::RULE == type_    ||
                Type::FILTER == type_  ||
                Type::OUTPUT == type_  ||
                Type::INTEGRATION == type_)
            {
                validation_ = true;
            }
        }
        else
        {
            throw std::runtime_error(
                fmt::format(
                    "Invalid name \"{}\" recieved, a name with 1, 2 or 3 parts was expected",
                    name.toStr()
                )
            );
        }
    }
};

} // namespace api::catalog

#endif // _API_CATALOG_RESOURCE_HPP
