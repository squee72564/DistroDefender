#ifndef _STORE_NAMESPACEID_HPP
#define _STORE_NAMESPACEID_HPP

#include <string_view>

#include <base/name.hpp>

namespace store
{

class NamespaceId
{
public:
    static const std::size_t PARTS_NAMESPACE_SIZE = 1;

    NamespaceId() = default;
    ~NamespaceId() = default;

    NamespaceId(const base::Name& id)
        : id_{id}
    {
        assertValid();
    }

    static base::RespOrError<NamespaceId> fromName(const base::Name& name)
    {
        NamespaceId namespaceId;

        try
        {
            namespaceId = NamespaceId(name);
        }
        catch (const std::invalid_argument& e)
        {
            return base::Error{e.what()};
        }

        return namespaceId;
    }

    NamespaceId(const NamespaceId& other) = default;
    NamespaceId(NamespaceId&& other) noexcept = default;

    NamespaceId& operator=(const NamespaceId& other) = default;
    NamespaceId& operator=(NamespaceId&& other) noexcept = default;

    friend bool operator==(const NamespaceId& l, const NamespaceId& r)
        { return l.id_ == r.id_; }

    friend bool operator!=(const NamespaceId& l, const NamespaceId& r)
        { return !(l == r); }

    friend std::ostream& operator<<(std::ostream& os, const NamespaceId& obj) { return os << obj.id_; }

    explicit operator std::string() const { return id_.parts()[0]; }

    const base::Name& name() const { return id_; }

    const std::string& str() const { return id_.parts()[0]; }

    bool operator<(const NamespaceId& other) const { return id_ < other.id_; }

private:
    base::Name id_;

    void assertValid()
    {
        if (id_.parts().size() != PARTS_NAMESPACE_SIZE || id_.parts()[0].empty())
        {
            throw std::runtime_error(
                "NamespaceId must have only one part and cannot be empty"
            );
        }
    }
};

} // namespace store

namespace std
{

template <>
struct std::hash<store::NamespaceId>
{
    std::size_t operator()(const store::NamespaceId& k) const
        { return std::hash<base::Name>()(k.name()); }
};

} // namespace std


#endif // _STORE_NAMESPACEID_HPP
