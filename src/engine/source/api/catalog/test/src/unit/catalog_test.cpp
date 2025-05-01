#include <gtest/gtest.h>

#include <base/logger.hpp>

#include "catalog_test.hpp"

class CatalogGetTest : public ::testing::TestWithParam<std::tuple<bool, api::catalog::Resource, std::string>>
{
protected:
    void SetUp() override
    {
        logger::testInit();
        m_spCatalog = std::make_unique<api::catalog::Catalog>(getConfig());
    }
    std::unique_ptr<api::catalog::Catalog> m_spCatalog;
};

TEST_P(CatalogGetTest, CatalogCommand)
{
    auto [isFailure, input, output] = GetParam();
    std::variant<std::string, base::Error> result;
    ASSERT_NO_THROW(result = m_spCatalog->getResource(input, "ignored"));
    if (isFailure)
    {
        ASSERT_TRUE(std::holds_alternative<base::Error>(result));
    }
    else
    {
        ASSERT_TRUE(std::holds_alternative<std::string>(result)) << base::getError(result).message;
        ASSERT_EQ(std::get<std::string>(result), output);
    }
}

INSTANTIATE_TEST_SUITE_P(CatalogCommand,
                         CatalogGetTest,
                         ::testing::Values(std::make_tuple(true, successResourceAssetJson, successJson.toStr()),
                                           std::make_tuple(false, successCollectionAssetJson, successCollectionJson),
                                           std::make_tuple(true, failResourceAsset, "")));

class CatalogPostTest : public ::testing::TestWithParam<std::tuple<bool, api::catalog::Resource, std::string>>
{
protected:
    void SetUp() override
    {
        logger::testInit();
        m_spCatalog = std::make_unique<api::catalog::Catalog>(getConfig());
    }
    std::unique_ptr<api::catalog::Catalog> m_spCatalog;
};

TEST_P(CatalogPostTest, CatalogCommand)
{
    auto [isFailure, input, content] = GetParam();

    std::optional<base::Error> error;
    ASSERT_NO_THROW(error = m_spCatalog->postResource(input, "nsId", content));
    if (isFailure)
    {
        ASSERT_TRUE(error);
    }
    else
    {
        ASSERT_FALSE(error);
    }
}

INSTANTIATE_TEST_SUITE_P(CatalogCommand,
                         CatalogPostTest,
                         ::testing::Values(std::make_tuple(false, successCollectionAssetJson, successJson.toStr()),
                                           std::make_tuple(true, successResourceAssetJson, successJson.toStr())));

class CatalogDeleteTest : public ::testing::TestWithParam<std::tuple<bool, api::catalog::Resource>>
{
protected:
    void SetUp() override
    {
        logger::testInit();
        m_spCatalog = std::make_unique<api::catalog::Catalog>(getConfig());
    }
    std::unique_ptr<api::catalog::Catalog> m_spCatalog;
};

TEST_P(CatalogDeleteTest, CatalogCommand)
{
    auto [isFailure, input] = GetParam();

    std::optional<base::Error> error;
    ASSERT_NO_THROW(error = m_spCatalog->deleteResource(input, "ignored"));
    if (isFailure)
    {
        ASSERT_TRUE(error);
    }
    else
    {
        ASSERT_FALSE(error);
    }
}

INSTANTIATE_TEST_SUITE_P(CatalogCommand,
                         CatalogDeleteTest,
                         ::testing::Values(std::make_tuple(true, successResourceAssetJson),
                                           std::make_tuple(false, successCollectionAssetJson),
                                           std::make_tuple(true, failResourceAsset)));

class CatalogValidateTest
    : public ::testing::TestWithParam<std::tuple<bool, api::catalog::Resource, std::string, std::string>>
{
protected:
    void SetUp() override
    {
        logger::testInit();
        m_spCatalog = std::make_unique<api::catalog::Catalog>(getConfig());
    }
    std::unique_ptr<api::catalog::Catalog> m_spCatalog;
};

TEST_P(CatalogValidateTest, CatalogCommand)
{
    auto [isFailure, input, content, ns] = GetParam();

    std::optional<base::Error> error;
    ASSERT_NO_THROW(error = m_spCatalog->putResource(input, content, ns));
    if (isFailure)
    {
        ASSERT_TRUE(error);
    }
    else
    {
        ASSERT_FALSE(error);
    }
}

INSTANTIATE_TEST_SUITE_P(
    CatalogCommand,
    CatalogValidateTest,
    ::testing::Values(std::make_tuple(true, successResourceAssetJson, successJson.toStr(), "user"),
                      std::make_tuple(true, successCollectionAssetJson, successJson.toStr(), "user")));
