#include <api/catalog/resource.hpp>
#include <gtest/gtest.h>

using namespace api::catalog;

TEST(CatalogResourceTest, BuildsCollections)
{
    auto nameDec = base::Name(Resource::typeToStr(Resource::Type::DECODER));
    auto nameRule = base::Name(Resource::typeToStr(Resource::Type::RULE));
    auto nameOuput = base::Name(Resource::typeToStr(Resource::Type::OUTPUT));
    auto nameFilter = base::Name(Resource::typeToStr(Resource::Type::FILTER));
    auto nameSchema = base::Name(Resource::typeToStr(Resource::Type::SCHEMA));
    auto nameIntegration = base::Name(Resource::typeToStr(Resource::Type::INTEGRATION));

    Resource resource;

    ASSERT_NO_THROW(resource = Resource(nameDec));
    ASSERT_EQ(resource.name_, nameDec);
    ASSERT_EQ(resource.type_, Resource::Type::COLLECTION);
    ASSERT_FALSE(resource.validation_);

    ASSERT_NO_THROW(resource = Resource(nameRule));
    ASSERT_EQ(resource.name_, nameRule);
    ASSERT_EQ(resource.type_, Resource::Type::COLLECTION);
    ASSERT_FALSE(resource.validation_);

    ASSERT_NO_THROW(resource = Resource(nameOuput));
    ASSERT_EQ(resource.name_, nameOuput);
    ASSERT_EQ(resource.type_, Resource::Type::COLLECTION);
    ASSERT_FALSE(resource.validation_);

    ASSERT_NO_THROW(resource = Resource(nameFilter));
    ASSERT_EQ(resource.name_, nameFilter);
    ASSERT_EQ(resource.type_, Resource::Type::COLLECTION);
    ASSERT_FALSE(resource.validation_);

    ASSERT_NO_THROW(resource = Resource(nameSchema));
    ASSERT_EQ(resource.name_, nameSchema);
    ASSERT_EQ(resource.type_, Resource::Type::COLLECTION);
    ASSERT_FALSE(resource.validation_);

    ASSERT_NO_THROW(resource = Resource(nameIntegration));
    ASSERT_EQ(resource.name_, nameIntegration);
    ASSERT_EQ(resource.type_, Resource::Type::COLLECTION);
    ASSERT_FALSE(resource.validation_);
}

TEST(CatalogResourceTest, BuildsCollectionErrorType)
{
    auto name = base::Name("non_existing_type");
    Resource resource;

    ASSERT_THROW(resource = Resource(name), std::runtime_error);
}

TEST(CatalogResourceTest, BuildsAssetsPolicy)
{
    auto nameDec = base::Name({"decoder", "name", "version"});
    auto nameRule = base::Name({"rule", "name", "version"});
    auto nameOuput = base::Name({"output", "name", "version"});
    auto nameFilter = base::Name({"filter", "name", "version"});
    auto namePolicy = base::Name({"policy", "name", "version"});
    auto nameIntegration = base::Name({"integration", "name", "version"});

    Resource resource;

    ASSERT_NO_THROW(resource = Resource(nameDec));
    ASSERT_EQ(resource.name_, nameDec);
    ASSERT_EQ(resource.type_, Resource::Type::DECODER);
    ASSERT_TRUE(resource.validation_);

    ASSERT_NO_THROW(resource = Resource(nameRule));
    ASSERT_EQ(resource.name_, nameRule);
    ASSERT_EQ(resource.type_, Resource::Type::RULE);
    ASSERT_TRUE(resource.validation_);

    ASSERT_NO_THROW(resource = Resource(nameOuput));
    ASSERT_EQ(resource.name_, nameOuput);
    ASSERT_EQ(resource.type_, Resource::Type::OUTPUT);
    ASSERT_TRUE(resource.validation_);

    ASSERT_NO_THROW(resource = Resource(nameFilter));
    ASSERT_EQ(resource.name_, nameFilter);
    ASSERT_EQ(resource.type_, Resource::Type::FILTER);
    ASSERT_TRUE(resource.validation_);

    ASSERT_NO_THROW(resource = Resource(nameIntegration));
    ASSERT_EQ(resource.name_, nameIntegration);
    ASSERT_EQ(resource.type_, Resource::Type::INTEGRATION);
    ASSERT_TRUE(resource.validation_);
}

TEST(CatalogResourceTest, BuildsAssetPolicyErrorType)
{
    auto name = base::Name({"non_existing_type", "name", "version"});
    Resource resource;

    ASSERT_THROW(resource = Resource(name), std::runtime_error);
}

TEST(CatalogResourceTest, BuildsSchema)
{
    auto name = base::Name({"schema", "name", "version"});
    Resource resource;

    ASSERT_NO_THROW(resource = Resource(name));
    ASSERT_EQ(resource.name_, name);
    ASSERT_EQ(resource.type_, Resource::Type::SCHEMA);
    ASSERT_FALSE(resource.validation_);
}

TEST(CatalogResourceTest, BuildsSchemaErrorType)
{
    auto name = base::Name({"non_existing_type", "name", "version"});
    Resource resource;

    ASSERT_THROW(resource = Resource(name), std::runtime_error);
}

TEST(CatalogResourceTest, BuildsErrorNameParts)
{
    auto nameLess = base::Name(std::vector<std::string> {"first", "second"});
    auto nameMore = base::Name({"first", "second", "third", "fourth"});
    Resource resource;

    ASSERT_THROW(resource = Resource(nameLess), std::runtime_error);
    ASSERT_THROW(resource = Resource(nameMore), std::runtime_error);
}
