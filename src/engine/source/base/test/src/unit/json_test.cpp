#include <base/json.hpp>
#include <gtest/gtest.h>

const char* jsonStr = R"({
    "hello": "world",
    "t": true,
    "f": false,
    "n": null,
    "i": 123,
    "pi": 3.1416,
    "a": [1, 2, 3, 4]
})";

TEST(JsonTest, CStrConstructor)
{
    json::Json json(jsonStr);
    std::optional<base::Error> err = json.getError();
    ASSERT_FALSE(base::isError(err));
}
