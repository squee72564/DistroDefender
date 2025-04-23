#ifndef _CONF_CONF_LOADER_HPP
#define _CONF_CONF_LOADER_HPP

#include <base/json.hpp>

namespace conf
{

struct IConfLoader
{
protected:
    virtual json::Json load() const = 0;
public:
    virtual ~IConfLoader() = default;

    json::Json operator()() const { return load(); }
};

class ConfLoader : public IConfLoader
{
public:
    json::Json load() const override;
};

} // namespace conf

#endif // _CONF_CONF_LOADER_HPP
