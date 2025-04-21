#include <kvdb/refCounter.hpp>

namespace kvdbManager
{

void RefCounter::addRef(const std::string& name, const uint32_t times)
{
    refMap_[name] += times;
}

void RefCounter::removeRef(const std::string& name)
{
    auto it = refMap_.find(name);

    if (it != refMap_.end())
    {
        auto& [key, val] = *it;
        val--;
        if (0 == val)
        {
            refMap_.erase(it);
        }
    }
}

uint32_t RefCounter::count(const std::string& name) const
{
    auto it = refMap_.find(name);

    if (it != refMap_.end())
    {
        return it->second;
    }

    return 0;
}

bool RefCounter::empty() const
{
    return refMap_.empty();
}

std::vector<std::string> RefCounter::getRefNames() const
{
    std::vector<std::string> names;

    for (const auto& [name, count] : refMap_)
    {
        names.push_back(name);
    }

    return names;
}

const std::map<std::string, uint32_t>& RefCounter::getRefMap() const
{
    return refMap_;
}


} // namespace kvdbManager
