#include <gtest/gtest.h>

#include <api/adapter/baseHandler_test.hpp>
#include <api/geo/handlers.hpp>

#include <geo/mockManager.hpp>

using namespace api::adapter;
using namespace api::test;
using namespace api::geo;
using namespace api::geo::handlers;
using namespace ::geo::mocks;

using GeoHandlerTest = BaseHandlerTest<::geo::IManager, MockManager>;

TEST_P(GeoHandlerTest, Handler)
{
    auto [reqGetter, handlerGetter, resGetter, mocker] = GetParam();
    handlerTest(reqGetter, handlerGetter, resGetter, iHandler_, mockHandler_, mocker);
}

using HandlerT = Params<::geo::IManager, MockManager>;

INSTANTIATE_TEST_SUITE_P(
    Api,
    GeoHandlerTest,
    ::testing::Values(

        /**************
         * ADD DB
         *************/
        // Success - test 0
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", "path"},
                        {"/type", "city"}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return addDb(geoManager);
            },
            []()
            {
                return userResponse(
                    json::Json{{
                        {"/status", schemas::engine::ReturnStatus::OK}
                    }}
                );
            },
            [](auto& mock)
            {
                EXPECT_CALL(mock, addDb(testing::_, testing::_)).WillOnce(testing::Return(base::noError()));
            }
        ),
        // Handler Error - test 1
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", "path"},
                        {"/type", "city"}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return addDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "error"
                );
            },
            [](auto& mock)
            {
                EXPECT_CALL(mock, addDb(testing::_, testing::_)).WillOnce(testing::Return(base::Error{"error"}));
            }
        ),
        // Wrong Request Type - test 2
        HandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not a json request";
                req.set_header("Content-Type", "text/plain");

                return req;
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return addDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&)
            {}
        ),
        // Empty Path - test 3
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", ""},
                        {"/type", "city"}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return addDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "Path cannot be empty"
                );
            },
            [](auto&)
            {}
        ),
        // Invalid Type - test 4
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", "path"},
                        {"/type", "invalid"}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return addDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "Invalid geo::Type name string 'invalid'"
                );
            },
            [](auto&)
            {}
        ),

        /**************
         * Delete DB
         *************/
        // Success - test 5
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", "path"}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return delDb(geoManager);
            },
            []()
            {
                return userResponse(
                    json::Json{{
                        {"/status", schemas::engine::ReturnStatus::OK}
                    }}
                );
            },
            [](auto& mock)
            {
                EXPECT_CALL(mock, removeDb(testing::_)).WillOnce(testing::Return(base::noError()));
            }
        ),
        // Handler Error - test 6
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", "path"}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return delDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "error"
                );
            },
            [](auto& mock)
            {
                EXPECT_CALL(mock, removeDb(testing::_)).WillOnce(testing::Return(base::Error{"error"}));
            }
        ),
        // Wrong Request Type - test 7
        HandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return delDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&)
            {}
        ),
        // Empty Path - test 8
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", ""}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return delDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "Path cannot be empty"
                );
            },
            [](auto&)
            {}
        ),

        /**************
         * List DBs
         *************/
        // Success - test 9
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{"{}"}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return listDb(geoManager);
            },
            []()
            {
                json::Json jsonRes {{
                    {"/status", schemas::engine::ReturnStatus::OK}
                }};
                jsonRes.setArray("/entries");

                std::vector<::geo::DbInfo> dbs {
                    {"name0", "path0", ::geo::Type::CITY},
                    {"name1", "path1", ::geo::Type::ASN}
                };

                for (const auto& db : dbs)
                {
                    jsonRes.appendJson(
                        "/entries",
                        json::Json{{
                            {"/path", db.path},
                            {"/name", db.name},
                            {"/type", ::geo::typeName(db.type)}
                        }}
                    );
                }

                return userResponse(
                    jsonRes
                );
            },
            [](auto& mock)
            {
                std::vector<::geo::DbInfo> dbs {
                    {"name0", "path0", ::geo::Type::CITY},
                    {"name1", "path1", ::geo::Type::ASN}
                };

                EXPECT_CALL(mock, listDbs()).WillOnce(testing::Return(dbs));
            }
        ),
        // Wrong Request Type - test 10
        HandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return listDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&)
            {}
        ),

        /**************
         * RemoteUpsert DBs
         *************/
        // Success - test 11
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", "path"},
                        {"/type", "city"},
                        {"/dbUrl", "dbUrl"},
                        {"/hashUrl", "hashUrl"}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return remoteUpsertDb(geoManager);
            },
            []()
            {
                return userResponse(
                    json::Json{{
                        {"/status", schemas::engine::ReturnStatus::OK}
                    }}
                );
            },
            [](auto& mock)
            {
                EXPECT_CALL(mock, remoteUpsertDb(testing::_, testing::_, testing::_, testing::_))
                    .WillOnce(testing::Return(base::noError()));
            }
        ),
        // Handler Error - test 12
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", "path"},
                        {"/type", "city"},
                        {"/dbUrl", "dbUrl"},
                        {"/hashUrl", "hashUrl"}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return remoteUpsertDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "error"
                );
            },
            [](auto& mock)
            {
                EXPECT_CALL(mock, remoteUpsertDb(testing::_, testing::_, testing::_, testing::_))
                    .WillOnce(testing::Return(base::Error{"error"}));
            }
        ),
        // Wrong Request Type - test 13
        HandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return remoteUpsertDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&)
            {}
        ),
        // Empty Path - test 14
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", ""},
                        {"/type", "city"},
                        {"/dbUrl", "dbUrl"},
                        {"/hashUrl", "hashUrl"}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return remoteUpsertDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "path cannot be empty"
                );
            },
            [](auto&)
            {}
        ),
        // Empty dbUrl - test 15
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", "path"},
                        {"/type", "city"},
                        {"/dbUrl", ""},
                        {"/hashUrl", "hashUrl"}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return remoteUpsertDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "dbUrl cannot be empty"
                );
            },
            [](auto&)
            {}
        ),
        // Empty hashUrl - test 16
        HandlerT(
            []()
            {
                return createRequest(
                    json::Json{{
                        {"/path", "path"},
                        {"/type", "city"},
                        {"/dbUrl", "dbUrl"},
                        {"/hashUrl", ""}
                    }}
                );
            },
            [](const std::shared_ptr<::geo::IManager>& geoManager)
            {
                return remoteUpsertDb(geoManager);
            },
            []()
            {
                return userErrorResponse(
                    "hashUrl cannot be empty"
                );
            },
            [](auto&)
            {}
        )
    )
);

