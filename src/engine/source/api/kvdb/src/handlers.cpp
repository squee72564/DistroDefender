#include <exception>
#include <string>

#include <api/kvdb/handlers.hpp>
#include <api/adapter/helpers.hpp>
#include <base/json.hpp>
#include <base/utils/stringUtils.hpp>

#include <schemas/kvdb.hpp>
#include <schemas/engine.hpp>

#include <fmt/format.h>

namespace api::kvdb::handlers
{

constexpr auto MESSAGE_DB_NOT_EXISTS = "The KVDB '{}' does not exist.";
constexpr auto MESSAGE_NAME_EMPTY = "Field /name is empty";
constexpr auto MESSAGE_KEY_EMPTY = "Field /key is empty";

adapter::RouteHandler managerGet(std::shared_ptr<kvdbManager::IKVDBManager> kvdbManager)
{
    return [wKvdb = std::weak_ptr<::kvdbManager::IKVDBManager>(kvdbManager)](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::kvdbManager::IKVDBManager>(req, wKvdb);
        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [kvdb, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::kvdb::getManagerGetRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );

            return;
        }

        bool must_be_loaded = jsonReq.exists("/must_be_loaded") ? jsonReq.getBool("/must_be_loaded").value() : false;

        json::Json resJson{{
            {"/status", schemas::engine::ReturnStatus::OK}
        }};

        resJson.setArray("/dbs");

        auto kvdbLists = kvdb->listDBs(must_be_loaded);

        if (!kvdbLists.empty())
        {
            for (const auto& dbName : kvdbLists)
            {
                resJson.appendString("/dbs", dbName);
            }
        }

        res = adapter::userResponse(resJson);
    };
}

adapter::RouteHandler managerPost(std::shared_ptr<kvdbManager::IKVDBManager> kvdbManager)
{
    return [wKvdb = std::weak_ptr<::kvdbManager::IKVDBManager>(kvdbManager)](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::kvdbManager::IKVDBManager>(req, wKvdb);
        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [kvdb, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::kvdb::getManagerPostRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );

            return;
        }

        std::string name{};
        std::string path {jsonReq.exists("/path") ? jsonReq.getString("/path").value() : ""};

        try
        {
            name = jsonReq.getString("/name").value();

            if (name.empty())
            {
                throw std::runtime_error(MESSAGE_NAME_EMPTY);
            }
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(
                e.what()
            );

            return;
        }

        if (kvdb->existsDB(name))
        {
            res = adapter::userErrorResponse(
                "The Database already exists."
            );
            return;
        }

        base::OptError resultCreate{};

        if (!path.empty())
        {
            resultCreate = kvdb->createDB(name, path); 
        }
        else
        {
            resultCreate = kvdb->createDB(name); 
        }

        if (base::isError(resultCreate))
        {
             res = adapter::userErrorResponse(
                fmt::format(
                    "The Database could not be created. Error: {}.",
                    resultCreate.value().message
                )
             );

             return;
        }

        json::Json resJson{{
            {"/status", schemas::engine::ReturnStatus::OK}
        }};

        res = adapter::userResponse(resJson);
    };
}

adapter::RouteHandler managerDelete(std::shared_ptr<kvdbManager::IKVDBManager> kvdbManager)
{
    return [wKvdb = std::weak_ptr<::kvdbManager::IKVDBManager>(kvdbManager)](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::kvdbManager::IKVDBManager>(req, wKvdb);
        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [kvdb, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::kvdb::getManagerDeleteRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );

            return;
        }

        std::string name{};

        try
        {
            name = jsonReq.getString("/name").value();

            if (name.empty())
            {
                throw std::runtime_error(MESSAGE_NAME_EMPTY);
            }
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(
                e.what()
            );

            return;
        }

        if (!kvdb->existsDB(name))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    MESSAGE_DB_NOT_EXISTS,
                    name
                )
            );
            return;
        }

        base::OptError resultDelete = kvdb->deleteDB(name);

        if (base::isError(resultDelete))
        {
             res = adapter::userErrorResponse(
                fmt::format(
                    "The Database could not be deleted. Error: {}",
                    resultDelete.value().message
                )
             );

             return;
        }

        json::Json resJson{{
            {"/status", schemas::engine::ReturnStatus::OK}
        }};

        res = adapter::userResponse(resJson);
    };
}

adapter::RouteHandler managerDump(std::shared_ptr<kvdbManager::IKVDBManager> kvdbManager,
                                  const std::string& kvdbScopeName)
{
    return [wKvdb = std::weak_ptr<::kvdbManager::IKVDBManager>(kvdbManager), kvdbScopeName](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::kvdbManager::IKVDBManager>(req, wKvdb);
        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [kvdb, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::kvdb::getManagerDumpRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );

            return;
        }

        int32_t page_t = jsonReq.exists("/page") ? jsonReq.getInt32("/page").value() : DEFAULT_HANDLER_PAGE;
        int32_t records_t = jsonReq.exists("/records") ? jsonReq.getInt32("/records").value() : DEFAULT_HANDLER_RECORDS;

        if (page_t <= 0)
        {
            res = adapter::userErrorResponse(
                "Field /page must be greater than 0"
            );
            return;
        }

        if (records_t <= 0)
        {
            res = adapter::userErrorResponse(
                "Field /records must be greater than 0"
            );
            return;
        }

        uint32_t page = page_t;
        uint32_t records = records_t;

        std::string name{};
        try
        {

            name = jsonReq.getString("/name").value();

            if (name.empty())
            {
                throw std::runtime_error(MESSAGE_NAME_EMPTY);
            }
        }
        catch( const std::exception& e)
        {
            res = adapter::userErrorResponse(
                e.what()
            );
            return;
        }

        if (!kvdb->existsDB(name))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    MESSAGE_DB_NOT_EXISTS,
                    name
                )
            );

            return;
        }

        auto resultHandler = kvdb->getKVDBHandler(name, kvdbScopeName);

        if (base::isError(resultHandler))
        {
            res = adapter::userErrorResponse(
                base::getError(resultHandler).message
            );
            return;
        }

        auto handler = std::move(base::getResponse(resultHandler));
        auto dumpRes = handler->dump(page, records);

        if (base::isError(dumpRes))
        {
            res = adapter::userErrorResponse(
                base::getError(dumpRes).message        
            );
            return;
        }

        const auto& dump = base::getResponse(dumpRes);

        json::Json resJson{{
            {"/status", schemas::engine::ReturnStatus::OK}
        }};

        resJson.setArray("/entries");

        for (const auto& [key, value] : dump)
        {
            json::Json valueJson{value};

            if (auto err = valueJson.getParseError())
            {
                res = adapter::userErrorResponse(
                    fmt::format(
                        "{} For key '{}' and value {}",
                        err->message,
                        key,
                        value
                    )
                );
                return;
            }

            resJson.appendJson(
                "/entries",
                json::Json{{
                    {"/key", key},
                    {"/value", valueJson}
                }}
            );
        }

        res = adapter::userResponse(resJson);
    };
}

adapter::RouteHandler dbGet(std::shared_ptr<kvdbManager::IKVDBManager> kvdbManager,
                            const std::string& kvdbScopeName)
{
    return [wKvdb = std::weak_ptr<::kvdbManager::IKVDBManager>(kvdbManager), kvdbScopeName](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::kvdbManager::IKVDBManager>(req, wKvdb);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [kvdb, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::kvdb::getDBGetRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );

            return;
        }

        std::string name{};
        std::string key{};

        try
        {
            name = jsonReq.getString("/name").value();
            if (name.empty())
            {
                throw std::runtime_error(MESSAGE_NAME_EMPTY);
            }
            key = jsonReq.getString("/key").value();
            if (key.empty())
            {
                throw std::runtime_error(MESSAGE_KEY_EMPTY);
            }
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        if (!kvdb->existsDB(name))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    MESSAGE_DB_NOT_EXISTS,
                    name
                )
            );
            return;
        }

        auto resultHandler = kvdb->getKVDBHandler(name, kvdbScopeName);

        if (base::isError(resultHandler))
        {
            res = adapter::userErrorResponse(
                base::getError(resultHandler).message
            );
            return;
        }

        auto handler = std::move(base::getResponse(resultHandler));

        const auto resultGet = handler->get(key);

        if (base::isError(resultGet))
        {
            res = adapter::userErrorResponse(
                base::getError(resultGet).message
            );
            return;
        }

        auto getValue = base::getResponse(resultGet);
        json::Json valueJson {getValue};

        if (auto err = valueJson.getParseError())
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "{} For value {}",
                    err->message,
                    getValue
                )
            );
            return;
        }

        json::Json jsonRes{{
            {"/status", schemas::engine::ReturnStatus::OK},
            {"/value", json::Json(base::getResponse(resultGet))}
        }};

        res = adapter::userResponse(jsonRes);
    };

}

adapter::RouteHandler dbDelete(std::shared_ptr<kvdbManager::IKVDBManager> kvdbManager,
                               const std::string& kvdbScopeName)
{
    return [wKvdb = std::weak_ptr<::kvdbManager::IKVDBManager>(kvdbManager), kvdbScopeName](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::kvdbManager::IKVDBManager>(req, wKvdb);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [kvdb, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::kvdb::getDBDeleteRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );

            return;
        }

        std::string name{};
        std::string key{};

        try
        {
            name = jsonReq.getString("/name").value();
            if (name.empty())
            {
                throw std::runtime_error(MESSAGE_NAME_EMPTY);
            }
            key = jsonReq.getString("/key").value();
            if (key.empty())
            {
                throw std::runtime_error(MESSAGE_KEY_EMPTY);
            }
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        if (!kvdb->existsDB(name))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    MESSAGE_DB_NOT_EXISTS,
                    name
                )
            );

            return;
        }

        auto resultHandler = kvdb->getKVDBHandler(name, kvdbScopeName);

        if (base::isError(resultHandler))
        {
            res = adapter::userErrorResponse(
                base::getError(resultHandler).message
            );
            return;
        }

        auto handler = std::move(base::getResponse(resultHandler));

        const auto removeError = handler->remove(key);

        if (base::isError(removeError))
        {
            res = adapter::userErrorResponse(
                base::getError(removeError).message
            );
            return;
        }

        res = adapter::userResponse(
            json::Json{{
                {"/status", schemas::engine::ReturnStatus::OK}
            }}
        );
    };
}

adapter::RouteHandler dbPut(std::shared_ptr<kvdbManager::IKVDBManager> kvdbManager,
                            const std::string& kvdbScopeName)
{
    return [wKvdb = std::weak_ptr<::kvdbManager::IKVDBManager>(kvdbManager), kvdbScopeName](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::kvdbManager::IKVDBManager>(req, wKvdb);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [kvdb, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::kvdb::getDBPutRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );

            return;
        }

        std::string name{};

        try
        {
            name = jsonReq.getString("/name").value();
            if (name.empty())
            {
                throw std::runtime_error(MESSAGE_NAME_EMPTY);
            }

            if (!jsonReq.exists("/entry/key"))
            {
                throw std::runtime_error("Missing /entry/key");
            }

            if (!jsonReq.exists("/entry/value"))
            {
                throw std::runtime_error("Missing /entry/value");
            }
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        auto entryKey = jsonReq.getString("/entry/key").value();
        auto entryValueJson =  jsonReq.getJson("/entry/value").value();

        if (auto err = entryValueJson.getParseError())
        {
            res = adapter::userErrorResponse(
                err->message
            );
            return;
        };

        auto entryValue = entryValueJson.toStr();

        if (entryKey.empty())
        {
            res = adapter::userErrorResponse(MESSAGE_KEY_EMPTY);
            return;
        }

        if (entryValue.empty())
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    "Field /value is empty"
                )
            );
            return;
        }

        if (!kvdb->existsDB(name))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    MESSAGE_DB_NOT_EXISTS,
                    name
                )
            );
            return;
        }

        auto resultHandler = kvdb->getKVDBHandler(name, kvdbScopeName);

        if (base::isError(resultHandler))
        {
            res = adapter::userErrorResponse(
                base::getError(resultHandler).message
            );
            return;
        }

        auto handler = std::move(base::getResponse(resultHandler));

        const auto setError = handler->set(entryKey, entryValue);

        if (base::isError(setError))
        {
            res = adapter::userErrorResponse(base::getError(setError).message);
            return;
        }

        res = adapter::userResponse(
            json::Json{{
                {"/status", schemas::engine::ReturnStatus::OK}
            }}
        );
    };
}

adapter::RouteHandler dbSearch(std::shared_ptr<kvdbManager::IKVDBManager> kvdbManager,
                               const std::string& kvdbScopeName)
{
    return [wKvdb = std::weak_ptr<::kvdbManager::IKVDBManager>(kvdbManager), kvdbScopeName](const auto& req, auto& res)
    {
        auto result = adapter::getReqAndHandler<::kvdbManager::IKVDBManager>(req, wKvdb);

        if (adapter::isError(result))
        {
            res = adapter::getErrorResp(result);
            return;
        }

        auto [kvdb, jsonReq] = adapter::getRes(result);

        if (auto err = jsonReq.validate(schemas::kvdb::getDBSearchRequestSchema()))
        {
            res = adapter::userErrorResponse(
                err->message
            );

            return;
        }

        int32_t page_t = jsonReq.exists("/page")
            ? jsonReq.getInt32("/page").value() : DEFAULT_HANDLER_PAGE;

        int32_t records_t = jsonReq.exists("/records")
            ? jsonReq.getInt32("/records").value() : DEFAULT_HANDLER_RECORDS;

        uint32_t page = page_t < 0 ? 0 : page_t;
        uint32_t records = records_t < 0 ? 0 : records_t;

        std::string name{};
        std::string prefix{};

        try
        {
            name = jsonReq.getString("/name").value();

            if (name.empty())
            {
                throw std::runtime_error(MESSAGE_NAME_EMPTY);
            }

            prefix = jsonReq.getString("/prefix").value();

            if (prefix.empty())
            {
                throw std::runtime_error("Field /prefix is empty");
            }
        }
        catch (const std::exception& e)
        {
            res = adapter::userErrorResponse(e.what());
            return;
        }

        if (!kvdb->existsDB(name))
        {
            res = adapter::userErrorResponse(
                fmt::format(
                    MESSAGE_DB_NOT_EXISTS,
                    name
                )
            );
            return;
        }

        auto resultHandler = kvdb->getKVDBHandler(name, kvdbScopeName);

        if (base::isError(resultHandler))
        {
            res = adapter::userErrorResponse(
                base::getError(resultHandler).message
            );
            return;
        }

        auto handler = std::move(base::getResponse(resultHandler));

        const auto searchRes = handler->search(prefix, page, records);

        if (base::isError(searchRes))
        {
            res = adapter::userErrorResponse(
                base::getError(searchRes).message
            );
            return;
        }

        const auto& searchResult = base::getResponse(searchRes);

        json::Json resJson{{
            {"/status", schemas::engine::ReturnStatus::OK}
        }};

        resJson.setArray("/entries");

        for (const auto& [key, value] : searchResult)
        {
            resJson.appendJson(
                "/entries",
                json::Json{{
                    {"/key", key},
                    {"/value", json::Json{value}}
                }}
            );
        }

        res = adapter::userResponse(resJson);
    };
}

void registerHandlers(std::shared_ptr<kvdbManager::IKVDBManager> kvdbManager,
                      const std::shared_ptr<httpserver::Server>& server)
{
    server->addRoute(httpserver::Method::POST, "/kvdb/manager/get", managerGet(kvdbManager));
    server->addRoute(httpserver::Method::POST, "/kvdb/manager/post", managerPost(kvdbManager));
    server->addRoute(httpserver::Method::POST, "/kvdb/manager/delete", managerDelete(kvdbManager));
    server->addRoute(httpserver::Method::POST, "/kvdb/manager/dump", managerDump(kvdbManager, "kvdb"));

    server->addRoute(httpserver::Method::POST, "/kvdb/db/get", dbGet(kvdbManager, "kvdb"));
    server->addRoute(httpserver::Method::POST, "/kvdb/db/delete", dbDelete(kvdbManager, "kvdb"));
    server->addRoute(httpserver::Method::POST, "/kvdb/db/put", dbPut(kvdbManager, "kvdb"));
    server->addRoute(httpserver::Method::POST, "/kvdb/db/search", dbSearch(kvdbManager, "kvdb"));
}

} // namespace api::kvdb::handlers

