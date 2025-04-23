#ifndef _CONF_MOCK_CONFLOADER_HPP
#define _CONF_MOCK_CONFLOADER_HPP
#include <gmock/gmock.h>

#include <conf/confLoader.hpp>

namespace conf::mock
{

class MockConfLoader : public IConfLoader
{
public:
    MOCK_METHOD(json::Json, load, (), (const, override));
};

} // namespace conf::mock

#endif // _CONF_MOCK_CONFLOADER_HPP
