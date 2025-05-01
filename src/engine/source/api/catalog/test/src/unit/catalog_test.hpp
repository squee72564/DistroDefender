#ifndef _CATATLOG_TEST_SHARED_HPP
#define _CATATLOG_TEST_SHARED_HPP

#include <memory>

#include <fmt/format.h>

#include <api/catalog/catalog.hpp>
#include <builder/mockValidator.hpp>
#include <store/mockStore.hpp>

using namespace store::mocks;

const base::Name successName{{"decoder", "name", "ok"}};
const base::Name failName{{"decoder", "name", "fail"}};

const store::Doc successJson{fmt::format(
    "{{\"name\": \"{}\"}}", successName.toStr().c_str()
)};

const store::Col successCollection{successName};

const std::string successCollectionJson{
    fmt::format("[\"{}\"]", successName.toStr().c_str()
)};

const store::Doc validJson{R"({})"};
const store::Doc invalidJson{R"([])"};

const std::string schema{R"({"type": "object")"};

const base::Name successSchemaName{{"schema", "name", "ok"}};
const base::Name failSchemaName{{"schema", "name", "fail"}};

const api::catalog::Resource successResourceAssetJson {
    base::Name{{
        api::catalog::Resource::typeToStr(api::catalog::Resource::Type::DECODER),
        successName.parts()[1],
        successName.parts()[2]
    }}
};

const api::catalog::Resource failResourceAsset {
    base::Name{{
        api::catalog::Resource::typeToStr(api::catalog::Resource::Type::DECODER),
        failName.parts()[1],
        failName.parts()[2]
    }}
};

const api::catalog::Resource successCollectionAssetJson {
    base::Name{
        api::catalog::Resource::typeToStr(api::catalog::Resource::Type::DECODER)
    }
};

inline api::catalog::Config getConfig(bool schemaOk = true)
{
    api::catalog::Config config;

    auto mockStore = std::make_shared<MockStore>();

    config.store = mockStore;
    config.validator = std::make_shared<builder::mocks::MockValidator>();

    EXPECT_CALL(*mockStore, readDoc(testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name) -> base::RespOrError<store::Doc>
            {
                if (name.parts()[2] == successName.parts()[2])
                {
                    return successJson;
                }

                return base::Error{"error"};
            }
        ));

    EXPECT_CALL(*mockStore, readCol(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name, const store::NamespaceId& namespaceid) -> base::RespOrError<store::Col>
            {
                if (name == successCollectionAssetJson.name_)
                {
                    return storeReadColResp(successCollection);
                }

                return base::Error{"error"};
            }
        ));

    EXPECT_CALL(*mockStore, createDoc(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name, const store::NamespaceId& namespaceid, const store::Doc& content) -> base::OptError
            {
                if (name.parts()[2] == successName.parts()[2])
                {
                    return base::OptError{base::noError()};
                }

                return base::OptError{base::Error{"error"}};
            }
        ));

    EXPECT_CALL(*mockStore, updateDoc(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name, const store::Doc& content) -> base::OptError
            {
                if (name.parts()[2] == successName.parts()[2])
                {
                    return base::OptError{base::noError()};
                }

                return base::OptError{base::Error{"error"}};
            }
        ));

    EXPECT_CALL(*mockStore, upsertDoc(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name, const store::NamespaceId& namespaceid, const store::Doc& content) -> base::OptError
            {
                if (name.parts()[2] == successName.parts()[2])
                {
                    return base::OptError{base::noError()};
                }

                return base::OptError{base::Error{"error"}};
            }
        ));

    EXPECT_CALL(*mockStore, deleteDoc(testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name) -> base::OptError
            {
                if (name.parts()[2] == successName.parts()[2])
                {
                    return base::OptError{base::noError()};
                }

                return base::OptError{base::Error{"error"}};
            }
        ));

    EXPECT_CALL(*mockStore, deleteCol(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name, const store::NamespaceId& namespaceid) -> base::OptError
            {
                if (name == successCollectionAssetJson.name_)
                {
                    return base::OptError{base::noError()};
                }

                return base::OptError{base::Error{"error"}};
            }
        ));

    if (schemaOk)
    {
        config.assetSchema = successSchemaName.toStr();
        config.environmentSchema = successSchemaName.toStr();
    }
    else
    {
        config.assetSchema = failSchemaName.toStr();
        config.environmentSchema = failSchemaName.toStr();
    }

    return config;
}

#endif // _CATATLOG_TEST_SHARED_HPP
