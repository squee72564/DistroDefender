#include <base/utils/singletonLocator.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thread>

template<typename T>
class MockSingletonManager : public  ISingletonManager<T>
{
public:
    ~MockSingletonManager() override = default;

    MOCK_METHOD(T&, instance, (), (override));
};

template <typename T>
class TestInstance
{
private:
    T value;

public:
    TestInstance() = default;

    void set(T val) { value = val; }
    T get() { return value; }
};

template <typename T>
class TestSingletonManager : public ISingletonManager<T>
{
private:
    T testInstance;

public:
    TestSingletonManager() = default;

    T& instance() override { return testInstance; }
};

/**
 * !!!!
 * Use unique instance types in each test to allow parallel execution
 * Defining the same struct in each test should suffice
 * !!!!
 */

TEST(SingletonLocatorTest, RegisterUnregister)
{
    struct Test
    {
    };

    auto registerManager = []()
    {
        SingletonLocator::registerManager<Test, TestSingletonManager<Test>>();
    };

    auto unregisterManager = []()
    {
        SingletonLocator::unregisterManager<Test>(); 
    };

    EXPECT_NO_THROW(registerManager());
    EXPECT_THROW(registerManager(), std::logic_error);


    EXPECT_NO_THROW(unregisterManager());
    EXPECT_THROW(unregisterManager(), std::logic_error);

    EXPECT_NO_THROW(registerManager());
    EXPECT_NO_THROW(unregisterManager());
}

TEST(SingletonLocatorTest, Instance)
{
    struct IntTest
    {
        int value;
    };

    SingletonLocator::registerManager<IntTest, TestSingletonManager<IntTest>>();
    auto& intInstance = SingletonLocator::instance<IntTest>();

    EXPECT_NO_THROW(intInstance.value = 1);
    EXPECT_EQ(intInstance.value, 1);

    struct FloatTest
    {
        float value;
    };

    SingletonLocator::registerManager<FloatTest, TestSingletonManager<FloatTest>>();
    auto& floatInstance = SingletonLocator::instance<FloatTest>();

    EXPECT_NO_THROW(floatInstance.value = 3.14f);
    EXPECT_EQ(floatInstance.value, 3.14f);

    EXPECT_NO_THROW(SingletonLocator::unregisterManager<IntTest>());
    EXPECT_NO_THROW(SingletonLocator::unregisterManager<FloatTest>());

}

TEST(SingletonLocatorTest, Manager)
{
    struct Instance
    {
    };

    auto getManager = []() -> ISingletonManager<Instance>&
    {
        return SingletonLocator::manager<Instance>();
    };

    EXPECT_THROW(getManager(), std::logic_error);

    SingletonLocator::registerManager<Instance, TestSingletonManager<Instance>>();

    EXPECT_NO_THROW(getManager());


    EXPECT_NO_THROW(SingletonLocator::unregisterManager<Instance>());

    EXPECT_THROW(getManager(), std::logic_error);
}

TEST(SingletonLocatorTest, ParallelRegister)
{
    struct Instance
    {
    };

    for (size_t runs = 0; runs < 100; ++runs)
    {

        int sz = 10;

        std::shared_ptr<std::vector<bool>> results =
            std::make_shared<std::vector<bool>>(sz);

        std::vector<std::thread> threads;

        for (int i = 0; i < sz; ++i)
        {
            threads.emplace_back([results, i]()
            {
                try
                {
                    SingletonLocator::registerManager<Instance, TestSingletonManager<Instance>>();
                }
                catch (...)
                {
                    (*results)[i] = false;
                    return;
                }

                (*results)[i] = true;
            });
        }


        for (auto& thread : threads)
        {
            thread.join();
        }

        EXPECT_EQ(std::count(results->begin(), results->end(), true), 1);

        ASSERT_NO_THROW(SingletonLocator::unregisterManager<Instance>());
    }
}

TEST(SingletonLocatorTest, Clear)
{
    struct Instance
    {
    };

    SingletonLocator::registerManager<
        std::shared_ptr<Instance>,
        TestSingletonManager<std::shared_ptr<Instance>>
    >();
    SingletonLocator::instance<std::shared_ptr<Instance>>() = std::make_shared<Instance>();

    auto instance1 = SingletonLocator::instance<std::shared_ptr<Instance>>();
    auto instance2 = SingletonLocator::instance<std::shared_ptr<Instance>>();


    ASSERT_EQ(instance1.get(), instance2.get());

    ASSERT_EQ(instance1.use_count(), 3);

    SingletonLocator::clear();

    ASSERT_EQ(instance1.use_count(), 2);

    SingletonLocator::registerManager<
        std::shared_ptr<Instance>,
        TestSingletonManager<std::shared_ptr<Instance>>
    >();
    SingletonLocator::instance<std::shared_ptr<Instance>>() = std::make_shared<Instance>();

    auto instance3 = SingletonLocator::instance<std::shared_ptr<Instance>>();

    ASSERT_NE(instance1.get(), instance3.get());
    ASSERT_NE(instance2.get(), instance3.get());

    ASSERT_EQ(instance3.use_count(), 2);
    ASSERT_EQ(instance1.use_count(), 2);

    SingletonLocator::clear();


    ASSERT_EQ(instance1.use_count(), 2);
    ASSERT_EQ(instance2.use_count(), 2);
    ASSERT_EQ(instance3.use_count(), 1);
}

TEST(MockSingletonManagerTest, Test)
{
    struct Instance
    {
    };

    SingletonLocator::registerManager<Instance, MockSingletonManager<Instance>>();

    Instance instance;

    auto& manager = static_cast<MockSingletonManager<Instance>&>(
        SingletonLocator::manager<Instance>()
    );

    EXPECT_CALL(
        manager,
        instance()
    )
    .Times(10)
    .WillRepeatedly(testing::ReturnRef(instance));

    std::vector<Instance*> gatheredInstances;
    gatheredInstances.reserve(10);

    for (int i = 0; i < 10; ++i)
    {
        gatheredInstances[i] = &SingletonLocator::instance<Instance>();
    }

    for (std::size_t i = 0; i < gatheredInstances.size(); ++i)
    {
        for (std::size_t j = 0; j < i; ++j)
        {
            EXPECT_EQ(gatheredInstances[i], gatheredInstances[j]);
        }
    }

    SingletonLocator::unregisterManager<Instance>();
}
