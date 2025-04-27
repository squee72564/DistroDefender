#ifndef _STORE_IDRIVER_HPP
#define _STORE_IDRIVER_HPP

#include <base/error.hpp>
#include <base/json.hpp>
#include <base/name.hpp>

namespace store
{

using Doc = json::Json;
using Col = std::vector<base::Name>;

class IDriver
{
public:
    virtual ~IDriver() = default;

    virtual base::OptError createDoc(const base::Name& name, const Doc& content) = 0;

    virtual base::RespOrError<Doc> readDoc(const base::Name& name) const = 0;

    virtual base::OptError updateDoc(const base::Name& name, const Doc& content) = 0;

    virtual base::OptError upsertDoc(const base::Name& name, const Doc& content) = 0;

    virtual base::OptError deleteDoc(const base::Name& name) = 0;

    virtual base::RespOrError<Col> readCol(const base::Name& name) const = 0;

    virtual base::RespOrError<Col> readRoot() const = 0;

    virtual base::OptError deleteCol(const base::Name& name) = 0;

    virtual bool exists(const base::Name& name) const = 0;

    virtual bool existsDoc(const base::Name& name) const = 0;

    virtual bool existsCol(const base::Name& name) const = 0;
};

} // namespace store

#endif // _STORE_IDRIVER_HPP
