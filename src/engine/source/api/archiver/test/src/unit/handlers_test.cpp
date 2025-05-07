#include <gtest/gtest.h>

#include <api/adapter/baseHandler_test.hpp>
#include <api/archiver/handlers.hpp>
#include <archiver/mockArchiver.hpp>

using namespace api::adapter;
using namespace api::test;
using namespace api::archiver;
using namespace api::archiver::handlers;
using namespace ::archiver::mocks;

using ArchiverHandlerTest = BaseHandlerTest<::archiver::IArchiver, MockArchiver>;
using ArchiverHandlerT = Params<::archiver::IArchiver, MockArchiver>;

TEST_P(ArchiverHandlerTest, Handler)
{
    auto [reqGetter, handlerGetter, resGetter, mocker] = GetParam();
    handlerTest(reqGetter, handlerGetter, resGetter, iHandler_, mockHandler_, mocker);
}

INSTANTIATE_TEST_SUITE_P(
    Api,
    ArchiverHandlerTest,
    ::testing::Values(
        /***********************************************************************
         * ActivateArchiver
         **********************************************************************/
        // Success
        ArchiverHandlerT(
            []()
            {
                return createRequest(json::Json{"{}"});
            },
            [](const std::shared_ptr<::archiver::IArchiver>& archiver) { return activateArchiver(archiver); },
            []()
            {
                return userResponse(
                    json::Json{{
                        {"/status", schemas::engine::ReturnStatus::OK}
                    }}
                );
            },
            [](auto& mock) { EXPECT_CALL(mock, activate()); }),
        // Wrong request type
        ArchiverHandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json proto request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<::archiver::IArchiver>& archiver) { return activateArchiver(archiver); },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&) {}),
        /***********************************************************************
         * DeactivateArchiver
         **********************************************************************/
        // Success
        ArchiverHandlerT(
            []()
            {
                return createRequest(json::Json{"{}"});
            },
            [](const std::shared_ptr<::archiver::IArchiver>& archiver) { return deactivateArchiver(archiver); },
            []()
            {
                return userResponse(
                    json::Json{{
                        {"/status", schemas::engine::ReturnStatus::OK}
                    }}
                );
            },
            [](auto& mock) { EXPECT_CALL(mock, deactivate()); }),
        // Wrong request type
        ArchiverHandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json proto request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<::archiver::IArchiver>& archiver) { return deactivateArchiver(archiver); },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&) {}),
        /***********************************************************************
         * GetArchiverStatus
         **********************************************************************/
        // Success
        ArchiverHandlerT(
            []()
            {
                return createRequest(json::Json{"{}"});
            },
            [](const std::shared_ptr<::archiver::IArchiver>& archiver) { return getArchiverStatus(archiver); },
            []()
            {
                return userResponse(
                    json::Json{{
                        {"/status", schemas::engine::ReturnStatus::OK},
                        {"/active", true}
                    }}
                );
            },
            [](auto& mock) { EXPECT_CALL(mock, isActive()).WillOnce(testing::Return(true)); }),
        // Wrong request type
        ArchiverHandlerT(
            []()
            {
                httplib::Request req;
                req.body = "not json proto request";
                req.set_header("Content-Type", "text/plain");
                return req;
            },
            [](const std::shared_ptr<::archiver::IArchiver>& archiver) { return getArchiverStatus(archiver); },
            []()
            {
                return userErrorResponse(
                    "Failed to parse json request: Parse error at offset 1: Invalid value."
                );
            },
            [](auto&) {})));
