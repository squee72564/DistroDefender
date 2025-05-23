#ifndef _API_TEST_BASE_HANDLER_TEST_HPP
#define _API_TEST_BASE_HANDLER_TEST_HPP

#include <gtest/gtest.h>

#include <api/adapter/adapter.hpp>
//#include <base/behaviour.hpp>
#include <base/json.hpp>
#include <base/logger.hpp>

namespace api::test
{
using ReqGetter = std::function<httplib::Request()>;
using ResGetter = std::function<httplib::Response()>;

template<typename IHandler>
using HandlerGetter = std::function<adapter::RouteHandler(const std::shared_ptr<IHandler>&)>;

template<typename MockHandler>
using Mocker = std::function<void(MockHandler& mock)>;

template<typename IHandler, typename MockHandler>
using Params = std::tuple<ReqGetter, HandlerGetter<IHandler>, ResGetter, Mocker<MockHandler>>;

template<typename IHandler, typename MockHandler>
class BaseHandlerTest : public testing::TestWithParam<Params<IHandler, MockHandler>>
{
protected:
    static_assert(
        std::is_base_of<IHandler, MockHandler>::value,
        "MockHandler must inherit from IHandler"
    );

    std::shared_ptr<MockHandler> mockHandler_;
    std::shared_ptr<IHandler> iHandler_;

    void SetUp() override
    {
        logger::testInit();

        mockHandler_ = std::make_shared<MockHandler>();
        iHandler_ = mockHandler_;
    }
};

template<typename IHandler, typename MockHandler>
void handlerTest(ReqGetter& reqGetter,
                 HandlerGetter<IHandler>& handlerGetter,
                 ResGetter& resGetter,
                 std::shared_ptr<IHandler>& handler,
                 std::shared_ptr<MockHandler>& mockHandler,
                 Mocker<MockHandler>& mocker)
{
    auto request = reqGetter();
    adapter::RouteHandler routeHandler;
    auto expectedResponse = resGetter();
    ASSERT_NO_THROW(routeHandler = handlerGetter(handler));

    mocker(*mockHandler);
    httplib::Response response;
    ASSERT_NO_THROW(routeHandler(request, response));

    EXPECT_EQ(response.status, expectedResponse.status);
    EXPECT_EQ(response.body, expectedResponse.body);

    // Expired handler test
    handler.reset();
    mockHandler.reset();
    httplib::Response expiredResponse;
    ASSERT_NO_THROW(routeHandler(request, expiredResponse));
    auto expectedExpiredResponse =
        adapter::internalErrorResponse("Error: Handler is not initialized");
    EXPECT_EQ(expiredResponse.status, expectedExpiredResponse.status);
    // TODO: find a way to compare the body problem is response type may be different than GenericStatus_Response
    // EXPECT_EQ(expiredResponse.body, expectedExpiredResponse.body);
}

} // namespace api::test

#endif // _API_TEST_BASE_HANDLER_TEST_HPP
