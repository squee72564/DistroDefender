#ifndef _I_KVDB_HANDLER_COLLECTION_HPP
#define _I_KVDB_HANDLER_COLLECTION_HPP

#include <string>

namespace kvdbManager
{

class IKVDBHandlerCollection
{
public:
    virtual void addKVDBHandler(const std::string& dbName, const std::string& scopeName) = 0;

    virtual void removeKVDBHandler(const std::string& name, const std::string& scopeName) = 0;
};

} // namepsace kvdbManager

#endif // _I_KVDB_HANDLER_COLLECTION_HPP
