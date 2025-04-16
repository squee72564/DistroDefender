#ifndef _BASE_SINGLETON_LOCATOR_HPP
#define _BASE_SINGLETON_LOCATOR_HPP

#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <type_traits>
#include <typeindex>

class BaseSingletonManager
{
public:
    virtual ~BaseSingletonManager() = default;
};

template <typename Instance>
class ISingletonManager : public BaseSingletonManager
{
public:
    ~ISingletonManager() override = default;

    virtual Instance& instance() = 0;
};

class SingletonLocator
{
private:
    static std::shared_mutex& registryMutex()
    {
        static std::shared_mutex m_registryMutex;
        return m_registryMutex;
    }

    static auto& strategyRegistry()
    {
        static std::unordered_map<
            std::type_index,
            std::unique_ptr<BaseSingletonManager>
        > m_strategyRegistry;

        return m_strategyRegistry;
    }
public:
    template<typename Instance, class Strategy>
    static void registerManager()
    {
        static_assert(
            std::is_base_of_v<ISingletonManager<Instance>, Strategy>,
            "Strategy must inherit from ISingletonManager for the specified Instance type."
        );

        static_assert(
            std::is_default_constructible_v<Strategy>,
            "Strategy must be default constructable."
        );

        std::unique_lock lock(registryMutex());

        auto& registry = strategyRegistry();
        
        if (registry.find(std::type_index(typeid(Instance))) != registry.end())
        {
            throw std::logic_error("Manager is already registered for this type!");
        }

        registry[std::type_index(typeid(Instance))] = std::make_unique<Strategy>();
    }

    template <typename Instance>
    static void unregisterManager()
    {
        std::unique_lock lock(registryMutex());

        auto& registry = strategyRegistry();

        if (registry.find(std::type_index(typeid(Instance))) == registry.end())
        {
            throw std::logic_error("No manager registered for this type.");
        }

        registry.erase(std::type_index(typeid(Instance)));
    }
    
    template <typename Instance>
    static Instance& instance()
    {
        std::shared_lock lock(registryMutex());
        auto& registry = strategyRegistry();

        auto it = registry.find(std::type_index(typeid(Instance)));

        if (it == registry.end())
        {
            throw std::logic_error("No manager registered for this type.");
        }

        return static_cast<ISingletonManager<Instance>*>(it->second.get())->instance();
    }

    template <typename Instance>
    static ISingletonManager<Instance>& manager()
    {
        std::shared_lock lock(registryMutex());

        auto& registry = strategyRegistry();

        auto it = registry.find(std::type_index(typeid(Instance)));

        if (it == registry.end())
        {
            throw std::logic_error("No manager registered for this type.");
        }

        return static_cast<ISingletonManager<Instance>&>(*it->second);
    }

    static void clear()
    {
        std::unique_lock lock(registryMutex());

        strategyRegistry().clear();
    }
};

#endif // _BASE_SINGLETON_LOCATOR_HPP
