#ifndef _BUILDER_IVALIDATOR_HPP
#define _BUILDER_IVALIDATOR_HPP

#include <base/error.hpp>
#include <base/json.hpp>

namespace builder
{

class IValidator
{
public:
    virtual ~IValidator() = default;

    virtual base::OptError validateIntegration(
                    const json::Json& json,
                    const std::string& namespaceId) const = 0;

    virtual base::OptError validateAsset(const json::Json& json) const = 0;

    virtual base::OptError validatePolicy(const json::Json& json) const = 0;
};

} // namespace builder

#endif // _BUILDER_IVALIDATOR_HPP
