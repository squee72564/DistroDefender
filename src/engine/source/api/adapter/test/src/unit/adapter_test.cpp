#include <gtest/gtest.h>

#include <api/adapter/adapter.hpp>

using namespace api::adapter;

TEST(ApiAdapterTest, ERROR)
{
    Error err{};
    ASSERT_NO_THROW(err = Error{});
}

TEST(ApiAdapterTest, ResOrErrorResp)
{
    ASSERT_NO_THROW(ResOrErrorResp<json::Json>{});
    ASSERT_NO_THROW(ResOrErrorResp<Error>{});
}

TEST(ApiAdapterTest, IsError)
{
    auto res = ResOrErrorResp<json::Json>{};
    ASSERT_FALSE(isError(res));

    res = Error{};
    ASSERT_TRUE(isError(res));
}

TEST(ApiAdaptorTest, GetErrorResp)
{
    auto res = ResOrErrorResp<json::Json>{};
    ASSERT_THROW(getErrorResp(res), std::bad_variant_access);

    res = Error {};
    ASSERT_NO_THROW(getErrorResp(res));
}

TEST(ApiAdaptorTest, GetReq)
{
    json::Json json{};
    auto res = ResOrErrorResp<json::Json>{json};
    ASSERT_NO_THROW(getRes(res));

    res = Error{};
    ASSERT_THROW(getRes(res), std::bad_variant_access);
}

TEST(ApiAdaptorTEst, InternalErrorResponse)
{
    httplib::Response res;
    json::Json jsonRes{};

    jsonRes.setObject("");
    jsonRes.setTypeMany({
        {"/status", schemas::engine::ReturnStatus::ERROR},
        {"/error", "test"}
    });

    ASSERT_NO_THROW(res = internalErrorResponse("test"));
    ASSERT_EQ(res.status, httplib::StatusCode::InternalServerError_500);

    json::Json gotJsonRes = parseResponse(res);

    ASSERT_EQ(gotJsonRes.toStr(), jsonRes.toStr());
}

TEST(ApiAdapterTest, UserResponse)
{
    httplib::Response res;

    json::Json jsonRes{};
    jsonRes.setObject("");
    jsonRes.setType(
        "/status", schemas::engine::ReturnStatus::OK
    );

    ASSERT_NO_THROW(res = userResponse(jsonRes));

    ASSERT_EQ(res.status, httplib::StatusCode::OK_200);

    auto gotJsonRes = parseResponse(res);
    
    ASSERT_EQ(gotJsonRes.toStr(), jsonRes.toStr());

}

TEST(ApiAdapterTest, UserErrorResponse)
{
    httplib::Response res;

    json::Json jsonRes{};
    jsonRes.setObject("");

    jsonRes.setTypeMany({
        {"/status", schemas::engine::ReturnStatus::ERROR},
        {"/error", "test"}
    });

    ASSERT_NO_THROW(res = userErrorResponse("test"));

    ASSERT_EQ(res.status, httplib::StatusCode::BadRequest_400);

    auto gotJsonRes = parseResponse(res);

    ASSERT_EQ(jsonRes.toStr(), gotJsonRes.toStr());
}

TEST(ApiAdapterTest, ParseRequest)
{
    json::Json jsonReq{};
    jsonReq.setObject("");

    jsonReq.setType("/content", "test");

    auto req = createRequest(jsonReq);

    ResOrErrorResp<json::Json> res;
    ASSERT_NO_THROW(res = parseRequest(req));

    ASSERT_FALSE(isError(res));

    auto gotReq = getRes(res);

    ASSERT_EQ(gotReq.toStr(), jsonReq.toStr());

    req.body = "invalid";

    ASSERT_NO_THROW(res = parseRequest(req));

    ASSERT_TRUE(isError(res));

    auto error = getErrorResp(res);

    ASSERT_EQ(error.status, httplib::StatusCode::BadRequest_400);

    ASSERT_NO_THROW(parseResponse(error));
}

TEST(ApiAdapterTest, CreateRequest)
{
    json::Json jsonReq{};
    jsonReq.setObject("");
    jsonReq.setType("/content", "test");

    httplib::Request req;
    ASSERT_NO_THROW(req = createRequest(jsonReq));

    ASSERT_EQ(req.body, jsonReq.toStr());
    ASSERT_EQ(req.get_header_value("Content-Type"), "plain/text");
}

TEST(ApiAdapterTest, ParseResponse)
{
    httplib::Response res;

    json::Json jsonRes;
    jsonRes.setObject("");
    jsonRes.setType("/status", schemas::engine::ReturnStatus::OK);

    ASSERT_NO_THROW(res = userResponse(jsonRes));

    ASSERT_EQ(res.status, httplib::StatusCode::OK_200);

    auto gotJsonRes = parseResponse(res);

    ASSERT_EQ(gotJsonRes.toStr(), jsonRes.toStr());
}
