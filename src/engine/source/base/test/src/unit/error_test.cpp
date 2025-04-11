#include <base/error.hpp>
#include <gtest/gtest.h>

TEST(ErrorTest, NoError)
{
    base::OptError error = base::noError();
    ASSERT_FALSE(base::isError(error));
}

TEST(ErrorTest, IsError)
{
    base::Error error{"Test Error"};
    base::OptError optError = error;
    ASSERT_TRUE(base::isError(optError));
}

TEST(ErrorTest, IsErrorVariant)
{
    base::RespOrError<int> respOrError = base::Error{"Test Error"};
    ASSERT_TRUE(base::isError(respOrError));
}

TEST(ErrorTest, IsResponseVariant)
{
    base::RespOrError<int> respOrError = 42;
    ASSERT_FALSE(base::isError(respOrError));
    ASSERT_EQ(base::getResponse(respOrError), 42);
}

TEST(ErrorTest, GetResponseVariantThrows)
{
    base::RespOrError<int> respOrError = base::Error{"Test Error"};
    ASSERT_TRUE(base::isError(respOrError));
    ASSERT_THROW(base::getResponse(respOrError), std::bad_variant_access);
}

TEST(ErrorTest, GetErrorVariantThrows)
{
    base::RespOrError<int> respOrError = 42;
    ASSERT_FALSE(base::isError(respOrError));
    ASSERT_THROW(base::getError(respOrError), std::bad_variant_access);
}

TEST(ErrorTest, GetErrorVariant)
{
    base::Error error {"Test Error"};
    base::RespOrError<int> respOrError = error;
    ASSERT_TRUE(base::isError(respOrError));
    ASSERT_EQ(base::getError(respOrError).message, "Test Error");
}

TEST(ErrorTest, GetErrorOptional)
{
    base::Error error{"Test Error"};
    base::OptError optError = error;
    ASSERT_TRUE(base::isError(optError));
    ASSERT_EQ(base::getError(optError).message, "Test Error");
}

TEST(ErrorTest, GetErrorOptionalThrows)
{
    base::OptError optError = std::nullopt;
    ASSERT_FALSE(base::isError(optError));
    ASSERT_THROW(base::getError(optError), std::bad_optional_access);
}
