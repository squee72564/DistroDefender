#ifndef _STORE_ISTORE_HPP
#define _STORE_ISTORE_HPP

#include <list>
#include <string>
#include <utility>

#include <store/idriver.hpp>
#include <store/namespaceId.hpp>

namespace store
{

class IStoreReader
{
public:
    virtual ~IStoreReader() = default;

    virtual base::RespOrError<Doc> readDoc(const base::Name& name) const = 0;

    virtual base::RespOrError<Col> readCol(const base::Name& name, const NamespaceId& namespaceId) const = 0;

    virtual bool existsDoc(const base::Name& name) const = 0;

    virtual bool existsCol(const base::Name& name, const NamespaceId& namepspaceId) const = 0;

    virtual std::vector<NamespaceId> listNamespaces() const = 0;

    virtual std::optional<NamespaceId> getNamespace(const base::Name& name) const = 0;
};

class IStoreInternal
{
public:
    virtual ~IStoreInternal() = default;

    /**
     * @brief Create a Internal Document in the store.
     *
     * @param name name of the document.
     * @param content document content.
     * @return base::OptError with the error or empty if no error.
     */
    virtual base::OptError createInternalDoc(const base::Name& name, const Doc& content) = 0;

    /**
     * @brief Read a Internal document from the store.
     *
     * @param name name of the document.
     * @return base::RespOrError<Doc> with the document or error.
     */
    virtual base::RespOrError<Doc> readInternalDoc(const base::Name& name) const = 0;

    /**
     * @brief Update a Internal document in the store.
     *
     * @param name name of the document.
     * @param content document content.
     * @return base::OptError with the error or empty if no error.
     */
    virtual base::OptError updateInternalDoc(const base::Name& name, const Doc& content) = 0;

    /**
     * @brief Upsert a Internal document in the store.
     *
     * @param name name of the document.
     * @param content document content.
     * @return base::OptError with the error or empty if no error.
     */
    virtual base::OptError upsertInternalDoc(const base::Name& name, const Doc& content) = 0;

    /**
     * @brief Delete a Internal document from the store.
     *
     * @param name name of the document.
     * @return base::OptError with the error or empty if no error.
     */
    virtual base::OptError deleteInternalDoc(const base::Name& name) = 0;

    /**
     * @brief Get collection of Internal documents from the store.
     *
     * @param name name of the collection.
     */
    virtual base::RespOrError<Col> readInternalCol(const base::Name& name) const = 0;

    /**
     * @brief Check if a Internal document exists in the store.
     *
     * @param name name of the document.
     * @return true if the document exists, false otherwise.
     */
    virtual bool existsInternalDoc(const base::Name& name) const = 0;
};

class IStore
    : public IStoreReader
    , public IStoreInternal
{
public:
    /**
     * @brief Add a document to the store.
     *
     * If the document already exists, the document is not added and returns error.
     * @param name The document name.
     * @param namespaceId The namespace identifier.
     * @param content The document content.
     * @return base::OptError The error if the document already exists or cannot be added.
     */
    virtual base::OptError createDoc(const base::Name& name, const NamespaceId& namespaceId, const Doc& content) = 0;

    /**
     * @brief Update a document in the store.
     *
     * If the document does not exist, the document is not updated and returns error.
     * The namespace cannot be changed.
     * @param name The document name.
     * @param content The document content.
     * @return base::OptError The error if the document does not exist or cannot be updated.
     */
    virtual base::OptError updateDoc(const base::Name& name, const Doc& content) = 0;

    /**
     * @brief Upsert a document in the store.
     *
     * If the document does not exist, the document is added.
     * If the document already exists, the document is updated.
     * If the document already exists and has a different namespace, the function returns error.
     * @param name The document name.
     * @param namespaceId The namespace identifier.
     * @param content The document content.
     * @return base::OptError The error if the document already exists and has a different namespace.
     */
    virtual base::OptError upsertDoc(const base::Name& name, const NamespaceId& namespaceId, const Doc& content) = 0;

    /**
     * @brief delete a document from the store.
     *
     * @param name The document name.
     * @return base::OptError The error if the document does not exist or cannot be deleted.
     */
    virtual base::OptError deleteDoc(const base::Name& name) = 0;

    /**
     * @brief Delete a collection in a namespace from the store.
     *
     * @param name The collection name.
     * @param namespaceId The namespace identifier of the collection.
     * @return base::OptError The error if the collection does not exist or cannot be deleted.
     */
    virtual base::OptError deleteCol(const base::Name& name, const NamespaceId& namespaceId) = 0;
};

} // namespace store

#endif // _STORE_ISTORE_HPP
