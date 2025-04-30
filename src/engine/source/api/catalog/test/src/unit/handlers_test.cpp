#include <gtest/gtest.h>

#include <api/adapter/baseHandler_test.hpp>
#include <api/catalog/handlers.hpp>
#include <api/catalog/mockCatalog.hpp>

#include <schemas/catalog.hpp>
#include <schemas/engine.hpp>

#include <base/json.hpp>

using namespace api::adapter;
using namespace api::test;
using namespace api::catalog;
using namespace api::catalog::handlers;
using namespace api::catalog::mocks;

using CatalogHandlerTest = BaseHandlerTest<ICatalog, MockCatalog>;

TEST_P(CatalogHandlerTest, Handler)
{
    auto [reqGetter, handlerGetter, resGetter, mocker] = GetParam();
    handlerTest(reqGetter, handlerGetter, resGetter, iHandler_, mockHandler_, mocker);
}

using HandlerT = Params<ICatalog, MockCatalog>;

INSTANTIATE_TEST_SUITE_P(
    Api,
    CatalogHandlerTest,
    ::testing::Values(
        /***********************************************************************
         * PostResource
         **********************************************************************/
        // Success 1
        HandlerT(
            []()
            {
                json::Json jsonReq{"{}"};
                jsonReq.setTypeMany({
                    {"/type", "decoder"},
                    {"/content", "content"},
                    {"/namespaceid", "ns"}
                });
                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog) { return resourcePost(catalog); },
            []()
            {
                json::Json jsonRes("{}");
                jsonRes.setType("/status", schemas::engine::ReturnStatus::OK);
                return userResponse(jsonRes);
            },
            [](auto& mock) {
                EXPECT_CALL(mock, postResource(testing::_, testing::_, testing::_))
                    .WillOnce(testing::Return(base::noError()));
            }
        ),
        // Handler Error 2
        HandlerT(
            []()
            {
                json::Json jsonReq{"{}"};
                jsonReq.setTypeMany({
                    {"/type", "decoder"},
                    {"/content", "content"},
                    {"/namespaceid", "ns"}
                });
                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePost(catalog); },
            []()
            { return userErrorResponse("error"); },
            [](auto& mock)
            {
                EXPECT_CALL(mock, postResource(testing::_, testing::_, testing::_))
                    .WillOnce(testing::Return(base::Error {"error"}));
            }
        ),
        // Wrong request type 3
        HandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePost(catalog); },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&) {}
        ),
        // Invalid type param 4
        HandlerT(
            []()
            {
                json::Json reqBody{{
                    {"/type", "invalid"}
                }};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePost(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        // Missing type param 5
        HandlerT(
            []()
            {
                json::Json reqBody{"{}"};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePost(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        // Missing content param 6
        HandlerT(
            []()
            {
                json::Json reqBody{};
                reqBody.setType("/type", "decoder");

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePost(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        // Invalid namespaceid param 7
        HandlerT(
            []()
            {
                json::Json reqBody{{
                    {"/type", "decoder"},
                    {"/content", "ns"}
                }};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePost(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        /***********************************************************************
         * GetResource
         **********************************************************************/
        // Success 8
        HandlerT(
            []()
            {
                json::Json jsonReq{{
                    {"/name", "decoder/test/0"},
                    {"/namespaceid", "ns"}
                }};

                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceGet(catalog); },
            []()
            {
                json::Json jsonRes{{
                    {"/status", schemas::engine::ReturnStatus::OK},
                    {"/content", "content"}
                }};

                return userResponse(jsonRes);
            },
            [](auto& mock)
            {
                EXPECT_CALL(mock, getResource(testing::_, testing::_))
                    .WillOnce(testing::Return(base::RespOrError<std::string> {"content"}));
            }
        ),
        // Handler Error 9 
        HandlerT(
            []()
            {
                json::Json jsonReq{{
                    {"/name", "decoder/test/0"},
                    {"/namespaceid", "ns"}
                }};

                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceGet(catalog); },
            []()
            { return userErrorResponse("error"); },
            [](auto& mock) {
                EXPECT_CALL(mock, getResource(testing::_, testing::_)).WillOnce(testing::Return(base::Error {"error"}));
            }
        ),
        // Wrong request type 10
        HandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceGet(catalog); },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&) {}
        ),
        // Missing name param 11
        HandlerT(
            []()
            {
                json::Json reqBody{"{}"};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceGet(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        // Missing namespaceid param 12
        HandlerT(
            []()
            {
                json::Json reqBody{{
                    {"/name", "decoder/test/0"},
                }};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceGet(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        /***********************************************************************
         * DeleteResource
         **********************************************************************/
        // Success 13
        HandlerT(
            []()
            {
                json::Json jsonReq{{
                    {"/name", "decoder/test/0"},
                    {"/namespaceid", "ns"}
                }};

                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceDelete(catalog); },
            []()
            {
                json::Json jsonRes{{
                    {"/status", schemas::engine::ReturnStatus::OK}
                }};

                return userResponse(jsonRes);
            },
            [](auto& mock)
            { EXPECT_CALL(mock, deleteResource(testing::_, testing::_)).WillOnce(testing::Return(base::noError())); }
        ),
        // Handler Error
        HandlerT(
            []()
            {
                json::Json jsonReq{{
                    {"/name", "decoder/test/0"},
                    {"/namespaceid", "ns"}
                }};

                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceDelete(catalog); },
            []()
            { return userErrorResponse("error"); },
            [](auto& mock) {
                EXPECT_CALL(mock, deleteResource(testing::_, testing::_))
                    .WillOnce(testing::Return(base::Error {"error"}));
            }
        ),
        // Wrong request type
        HandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceDelete(catalog); },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&) {}
        ),
        // Missing name param
        HandlerT(
            []()
            {
                json::Json reqBody{"{}"};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceDelete(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        // Missing namespaceid param
        HandlerT(
            []()
            {
                json::Json reqBody{{
                    {"/name", "decoder/test/0"}
                }};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceDelete(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        /***********************************************************************
         * PutResource
         **********************************************************************/
        // Success
        HandlerT(
            []()
            {
                json::Json jsonReq{{
                    {"/name", "decoder/test/0"},
                    {"/content", "content"},
                    {"/namespaceid", "ns"}
                }};

                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePut(catalog); },
            []()
            {
                json::Json jsonRes{{
                    {"/status", schemas::engine::ReturnStatus::OK}
                }};

                return userResponse(jsonRes);
            },
            [](auto& mock) {
                EXPECT_CALL(mock, putResource(testing::_, testing::_, testing::_))
                    .WillOnce(testing::Return(base::noError()));
            }
        ),
        // Handler Error
        HandlerT(
            []()
            {
                json::Json jsonReq{{
                    {"/name", "decoder/test/0"},
                    {"/content", "content"},
                    {"/namespaceid", "ns"}
                }};

                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePut(catalog); },
            []()
            { return userErrorResponse("error"); },
            [](auto& mock)
            {
                EXPECT_CALL(mock, putResource(testing::_, testing::_, testing::_))
                    .WillOnce(testing::Return(base::Error {"error"}));
            }
        ),
        // Wrong request type
        HandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePut(catalog); },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&) {}
        ),
        // Missing name param
        HandlerT(
            []()
            {
                json::Json reqBody{"{}"};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePut(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        // Missing content param
        HandlerT(
            []()
            {
                json::Json reqBody{{
                    {"/name", "decoder/test/0"}
                }};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePut(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        // Missing namespaceid param
        HandlerT(
            []()
            {
                json::Json reqBody{{
                    {"/name", "decoder/test/0"},
                    {"/content", "content"}
                }};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourcePut(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        /***********************************************************************
         * ValidateResource
         **********************************************************************/
        // Success
        HandlerT(
            []()
            {
                json::Json jsonReq{{
                    {"/name", "decoder/test/0"},
                    {"/content", "content"},
                    {"/namespaceid", "ns"}
                }};

                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceValidate(catalog); },
            []()
            {
                json::Json jsonRes{{
                    {"/status", schemas::engine::ReturnStatus::OK}
                }};

                return userResponse(jsonRes);
            },
            [](auto& mock)
            {
                EXPECT_CALL(mock, validateResource(testing::_, testing::_, testing::_))
                    .WillOnce(testing::Return(base::noError()));
            }
        ),
        // Handler Error
        HandlerT(
            []()
            {
                json::Json jsonReq{{
                    {"/name", "decoder/test/0"},
                    {"/content", "content"},
                    {"/namespaceid", "ns"}
                }};

                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceValidate(catalog); },
            []()
            { return userErrorResponse("error"); },
            [](auto& mock)
            {
                EXPECT_CALL(mock, validateResource(testing::_, testing::_, testing::_))
                    .WillOnce(testing::Return(base::Error {"error"}));
            }
        ),
        // Wrong request type
        HandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceValidate(catalog); },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&) {}
        ),
        // Missing name param
        HandlerT(
            []()
            {
                json::Json reqBody{"{}"};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceValidate(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        // Missing content param
        HandlerT(
            []()
            {
                json::Json reqBody{{
                    {"/name", "decoder/test/0"}
                }};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceValidate(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        // Missing namespaceid param
        HandlerT(
            []()
            {
                json::Json reqBody{{
                    {"/name", "decoder/test/0"},
                    {"/content", "content"}

                }};

                httplib::Request req;
                req.body = reqBody.toStr();
                req.set_header("Content-Type", "plain/text");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return resourceValidate(catalog); },
            []()
            { return userErrorResponse("Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"); },
            [](auto&) {}
        ),
        /***********************************************************************
         * GetNamespaces
         **********************************************************************/
        // Success
        HandlerT(
            []()
            {
                json::Json jsonReq{};
                return createRequest(jsonReq);
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return getNamespaces(catalog); },
            []()
            {

                json::Json jsonRes{{
                    {"/status", schemas::engine::ReturnStatus::OK}
                }};

                jsonRes.setArray("/namespaces");
                jsonRes.appendString("/namespaces", "ns1");
                jsonRes.appendString("/namespaces", "ns2");

                return userResponse(jsonRes);
            },
            [](auto& mock) {
                EXPECT_CALL(mock, getAllNamespaces())
                    .WillOnce(testing::Return(std::vector<store::NamespaceId> {
                        base::Name{"ns1"}, base::Name{"ns2"}
                    }));
            }
        ),
        // Wrong request type
        HandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<ICatalog>& catalog)
            { return getNamespaces(catalog); },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&) {}
        )
    )
);
