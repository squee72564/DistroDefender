#ifndef _KVDB_REF_COUNTER_HPP
#define _KVDB_REF_COUNTER_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace kvdbManager
{

class RefCounter
{
public:
    RefCounter() = default;

    ~RefCounter() { refMap_.clear(); }

    void addRef(const std::string& name, const uint32_t times = 1);

    void removeRef(const std::string& name);

    uint32_t count(const std::string& name) const;

    bool empty() const;

    std::vector<std::string> getRefNames() const;

    const std::map<std::string, uint32_t>& getRefMap() const;

private:
    
    std::map<std::string, uint32_t> refMap_;
};

} // namespace kvdbManager

#endif // _KVDB_REF_COUNTER_HPP
