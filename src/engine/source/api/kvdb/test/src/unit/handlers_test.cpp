#include <gtest/gtest.h>

#include <api/adapter/baseHandler_test.hpp>
#include <api/kvdb/handlers.hpp>
#include <base/json.hpp>
#include <kvdb/mockKVDBHandler.hpp>
#include <kvdb/mockKVDBManager.hpp>

#include <schemas/engine.hpp>

using namespace api::adapter;
using namespace api::test;
using namespace api::kvdb;
using namespace api::kvdb::handlers;
using namespace ::kvdb::mocks;

using KvdbHandlerTest = BaseHandlerTest<::kvdbManager::IKVDBManager, MockKVDBManager>;

TEST_P(KvdbHandlerTest, Handler)
{
    auto [reqGetter, handlerGetter, resGetter, mocker] = GetParam();
    handlerTest(reqGetter, handlerGetter, resGetter, iHandler_, mockHandler_, mocker);
}

using HandlerT = Params<::kvdbManager::IKVDBManager, MockKVDBManager>;

INSTANTIATE_TEST_SUITE_P(
    Api,
    KvdbHandlerTest,
    ::testing::Values(
        /* ******************
        * MANAGER GET
        * ******************/
        // Success
        HandlerT( // test 0
            []() // reqGetter
            {
                return createRequest(json::Json{"{}"});
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerGet(kvdb);
            },
            []() // resGetter
            {
                json::Json resJson{{
                    {"/status", schemas::engine::ReturnStatus::OK}
                }};

                resJson.setArray("/dbs");
                resJson.appendString("/dbs", "");

                return userResponse(resJson);
            },
            [](auto& mock) // Mocker
            {
                EXPECT_CALL(mock, listDBs(testing::_)).WillOnce(testing::Return(std::vector<std::string>{""}));
            }
        ),
        /* ******************
        * MANAGER POST 
        * ******************/
        // Success
        HandlerT( // test 1
            []() // reqGetter
            {
                return createRequest(json::Json{{ {"/name", "name"} }});
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerPost(kvdb);
            },
            []() // resGetter
            {
                json::Json resJson{{
                    {"/status", schemas::engine::ReturnStatus::OK}
                }};

                return userResponse(resJson);
            },
            [](auto& mock) // Mocker
            {
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(false));
                EXPECT_CALL(mock, createDB(testing::_)).WillOnce(testing::Return(base::noError()));
            }
        ),
        // Handler Error
        HandlerT( // test 2
            []() // reqGetter
            {
                return createRequest(json::Json{"{}"});
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerPost(kvdb);
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"
                );
            },
            [](auto& mock) // Mocker
            {}
        ),
        // Database already exists
        HandlerT( // test 3
            []() // reqGetter
            {
                return createRequest(json::Json{{ {"/name", "name"} }});
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerPost(kvdb);
            },
            []() // resGetter
            {
                return userErrorResponse("The Database already exists.");
            },
            [](auto& mock) // Mocker
            {
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
            }
        ),
        // Failture creating database
        HandlerT( // test 4
            []() // reqGetter
            {
                return createRequest(json::Json{{ {"/name", "name"} }});
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerPost(kvdb);
            },
            []() // resGetter
            {
                return userErrorResponse("The Database could not be created. Error: error.");
            },
            [](auto& mock) // Mocker
            {
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(false));
                EXPECT_CALL(mock, createDB(testing::_)).WillOnce(testing::Return(base::Error{"error"}));
            }
        ),
        /* ******************
        * MANAGER DELETE 
        * ******************/
        // Success
        HandlerT( // test 5
            []() // reqGetter
            {
                return createRequest(json::Json{{ {"/name", "name"} }});
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerDelete(kvdb);
            },
            []() // resGetter
            {
                return userResponse(
                    json::Json{{
                        {"/status", schemas::engine::ReturnStatus::OK}
                    }}
                );
            },
            [](auto& mock) // Mocker
            {
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, deleteDB(testing::_)).WillOnce(testing::Return(base::noError()));
            }
        ),
        //Handler Error
        HandlerT( // test 6
            []() // reqGetter
            {
                return createRequest(json::Json{"{}"});
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerDelete(kvdb);
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"
                );
            },
            [](auto& mock) // Mocker
            {}
        ),
        // Database does not exist
        HandlerT( // test 7
            []() // reqGetter
            {
                return createRequest(json::Json{{ {"/name", "name"} }});
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerDelete(kvdb);
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "The KVDB 'name' does not exist."
                );
            },
            [](auto& mock) // Mocker
            {
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(false));
            }
        ),
        // Failure deleting data base
        HandlerT( // test 8
            []() // reqGetter
            {
                return createRequest(json::Json{{ {"/name", "name"} }});
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerDelete(kvdb);
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "The Database could not be deleted. Error: error"
                );
            },
            [](auto& mock) // Mocker
            {
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, deleteDB(testing::_)).WillOnce(testing::Return(base::Error{"error"}));
            }
        ),
        /* ******************
        * MANAGER DUMP 
        * ******************/
        // Success
        HandlerT( // test 9
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/page", 1},
                        {"/records", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerDump(kvdb, "any_scope");
            },
            []() // resGetter
            {
                json::Json jsonRes {{
                    {"/status", schemas::engine::ReturnStatus::OK}
                }};

                jsonRes.setArray("/entries");
                
                jsonRes.appendJson(
                    "/entries",
                    json::Json{{
                        {"/key", "key1"},
                        {"/value", 1}
                    }}
                );

                return userResponse(jsonRes);
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_)).WillOnce(testing::Return(mockKvdbHandler));
                
                const std::list<std::pair<std::string, std::string>> mockList {{"key1", "1"}};

                EXPECT_CALL(*mockKvdbHandler, dump(testing::_, testing::_)).WillOnce(testing::Return(mockList));
            }
        ),
        // Invalid page
        HandlerT( // test 10
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/page", 0},
                        {"/records", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerDump(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse("Field /page must be greater than 0");
            },
            [](auto& mock) // Mocker
            {}
        ),
        // Invalid record
        HandlerT( // test 11
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/page", 1},
                        {"/records", 0}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerDump(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse("Field /records must be greater than 0");
            },
            [](auto& mock) // Mocker
            {}
        ),
        // Dump error
        HandlerT( // test 12
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/page", 1},
                        {"/records", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerDump(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse("error");
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_)).WillOnce(testing::Return(mockKvdbHandler));
                EXPECT_CALL(*mockKvdbHandler, dump(testing::_, testing::_)).WillOnce(testing::Return(base::Error{"error"}));
            }
        ),
        // Invalid value
        HandlerT( // test 13
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/page", 1},
                        {"/records", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return managerDump(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Parse error at offset 0: Invalid value. For key 'key1' and value value1"
                );
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_)).WillOnce(testing::Return(mockKvdbHandler));

                const std::list<std::pair<std::string, std::string>> mockList{{"key1", "value1"}};
                EXPECT_CALL(*mockKvdbHandler, dump(testing::_, testing::_)).WillOnce(testing::Return(mockList));
            }
        ),
        /* ******************
        * DB GET
        * ******************/
        // Success
        HandlerT( // test 14
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/key", "key1"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbGet(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userResponse(
                    json::Json{{
                        {"/status", schemas::engine::ReturnStatus::OK},
                        {"/value", 1}
                    }}
                );
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_)).WillOnce(testing::Return(mockKvdbHandler));

                EXPECT_CALL(*mockKvdbHandler, get(testing::_)).WillOnce(testing::Return("1"));
            }
        ),
        // Missing name
        HandlerT( // test 15
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/key", "key1"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbGet(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"
                );
            },
            [](auto& mock) // Mocker
            {}
        ),
        // Missing key
        HandlerT( // test 16
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbGet(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"
                );
            },
            [](auto& mock) // Mocker
            {}
        ),
        // DB does not exist
        HandlerT( // test 17
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/key", "key1"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbGet(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "The KVDB 'name' does not exist."
                );
            },
            [](auto& mock) // Mocker
            {
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(false));
            }
        ),
        // get Error
        HandlerT( // test 18
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/key", "key1"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbGet(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "error"
                );
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_)).WillOnce(testing::Return(mockKvdbHandler));
                EXPECT_CALL(*mockKvdbHandler, get(testing::_)).WillOnce(testing::Return(base::Error{"error"}));
            }
        ),
        // Invalid argument
        HandlerT( // test 19
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/key", "key1"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbGet(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Parse error at offset 0: Invalid value. For value hello"
                );
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_)).WillOnce(testing::Return(mockKvdbHandler));
                EXPECT_CALL(*mockKvdbHandler, get(testing::_)).WillOnce(testing::Return("hello"));
            }
        ),
        /* ******************
        * DB DELETE
        * ******************/
        // Sucess
        HandlerT( // test 20
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/key", "key1"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbDelete(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userResponse(
                    json::Json{{
                        {"/status", schemas::engine::ReturnStatus::OK}
                    }}
                );
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_)).WillOnce(testing::Return(mockKvdbHandler));
                EXPECT_CALL(*mockKvdbHandler, remove(testing::_)).WillOnce(testing::Return(base::noError()));
            }
        ),
        // Missing name
        HandlerT( // test 21
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/key", "key1"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbDelete(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"
                );
            },
            [](auto& mock) // Mocker
            {}
        ),
        // Missing key
        HandlerT( // test 22
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbDelete(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"
                );
            },
            [](auto& mock) // Mocker
            {}
        ),
        // DB does not exist
        HandlerT( // test 23
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/key", "key1"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbDelete(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "The KVDB 'name' does not exist."
                );
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(false));
            }
        ),
        // Error removing DB
        HandlerT( // test 24
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/key", "key1"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbDelete(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "error"
                );
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_)).WillOnce(testing::Return(mockKvdbHandler));
                EXPECT_CALL(*mockKvdbHandler,  remove(testing::_)).WillOnce(testing::Return(base::Error{"error"}));
            }
        ),
        /* ******************
        * DB PUT
        * ******************/
        // Sucess
        HandlerT( // test 25
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/entry/key", "key1"},
                        {"/entry/value", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbPut(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userResponse(
                    json::Json{{
                        {"/status", schemas::engine::ReturnStatus::OK}
                    }}
                );
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_)).WillOnce(testing::Return(mockKvdbHandler));
                EXPECT_CALL(*mockKvdbHandler, set(testing::_, testing::Matcher<const std::string&>(testing::Eq("1")))).WillOnce(testing::Return(base::noError()));
            }
        ),
        // Missing name
        HandlerT( // test 26
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/entry/key", "key1"},
                        {"/entry/value", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbPut(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#', Document path: '#'\n"
                );
            },
            [](auto& mock) // Mocker
            {}
        ),
        // Missing key
        HandlerT( // test 27
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/entry/value", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbPut(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#/properties/entry', Document path: '#/entry'\n"
                );
            },
            [](auto& mock) // Mocker
            {}
        ),
        // Missing value
        HandlerT( // test 28
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/entry/key", "key1"}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbPut(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Schema validation failed: Invalid schema Keyword: 'required'. Schema path: '#/properties/entry', Document path: '#/entry'\n"
                );
            },
            [](auto& mock) // Mocker
            {}
        ),
        // Invalid entry/key
        HandlerT( // test 29
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/entry/key", ""},
                        {"/entry/value", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbPut(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "Field /key is empty"
                );
            },
            [](auto& mock) // Mocker
            {}
        ),
        // DB does not exist
        HandlerT( // test 30
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/entry/key", "key1"},
                        {"/entry/value", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbPut(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "The KVDB 'name' does not exist."
                );
            },
            [](auto& mock) // Mocker
            {
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(false));
            }
        ),
        // Error setting
        HandlerT( // test 31
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/entry/key", "key1"},
                        {"/entry/value", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbPut(kvdb, "any_scope");
            },
            []() // resGetter
            {
                return userErrorResponse(
                    "error"
                );
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_)).WillOnce(testing::Return(mockKvdbHandler));
                EXPECT_CALL(*mockKvdbHandler, set(testing::_, testing::Matcher<const std::string&>(testing::Eq("1")))).WillOnce(testing::Return(base::Error{"error"}));
            }
        ),
        /* ******************
        * DB SEARCH 
        * ******************/
        HandlerT( // test 32
            []() // reqGetter
            {
                return createRequest(
                    json::Json{{
                        {"/name", "name"},
                        {"/prefix", "prefix"},
                        {"/page", 1},
                        {"/records", 1}
                    }}
                );
            },
            [](const std::shared_ptr<::kvdbManager::IKVDBManager>& kvdb) // handlerGetter
            {
                return dbSearch(kvdb, "any_scope");
            },
            []() // resGetter
            {
                json::Json jsonRes{{
                    {"/status", schemas::engine::ReturnStatus::OK},
                }};

                jsonRes.setArray("/entries");

                jsonRes.appendJson(
                    "/entries",
                    json::Json{{
                        {"/key", "key1"},
                        {"/value", 1}
                    }}
                );

                return userResponse(
                    jsonRes
                );
            },
            [](auto& mock) // Mocker
            {
                auto mockKvdbHandler = std::make_shared<MockKVDBHandler>();
                EXPECT_CALL(mock, existsDB(testing::_)).WillOnce(testing::Return(true));
                EXPECT_CALL(mock, getKVDBHandler(testing::_, testing::_))
                    .WillOnce(testing::Return(mockKvdbHandler));
                const std::list<std::pair<std::string, std::string>> mockList {{"key1", "1"}};
                EXPECT_CALL(*mockKvdbHandler, search(testing::_, testing::_, testing::_))
                    .WillOnce(testing::Return(mockList));
            }
        )

    )
);
