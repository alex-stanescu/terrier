#include "catalog/catalog_accessor.h"

#include <string>
#include <utility>
#include <vector>

#include "catalog/catalog.h"
#include "catalog/postgres/pg_proc.h"

namespace terrier::catalog {
db_oid_t CatalogAccessor::GetDatabaseOid(std::string name) const {
  NormalizeObjectName(&name);
  return catalog_->GetDatabaseOid(txn_, name);
}

db_oid_t CatalogAccessor::CreateDatabase(std::string name) const {
  NormalizeObjectName(&name);
  return catalog_->CreateDatabase(txn_, name, true);
}

bool CatalogAccessor::DropDatabase(db_oid_t db) const { return catalog_->DeleteDatabase(txn_, db); }

void CatalogAccessor::SetSearchPath(std::vector<namespace_oid_t> namespaces) {
  TERRIER_ASSERT(!namespaces.empty(), "search path cannot be empty");

  default_namespace_ = namespaces[0];
  search_path_ = std::move(namespaces);

  // Check if 'pg_catalog is explicitly set'
  for (auto &ns : search_path_)
    if (ns == postgres::NAMESPACE_CATALOG_NAMESPACE_OID) return;

  search_path_.emplace(search_path_.begin(), postgres::NAMESPACE_CATALOG_NAMESPACE_OID);
}

namespace_oid_t CatalogAccessor::GetNamespaceOid(std::string name) const {
  if (name.empty()) return catalog::postgres::NAMESPACE_DEFAULT_NAMESPACE_OID;
  NormalizeObjectName(&name);
  return dbc_->GetNamespaceOid(txn_, name);
}

namespace_oid_t CatalogAccessor::CreateNamespace(std::string name) const {
  NormalizeObjectName(&name);
  return dbc_->CreateNamespace(txn_, name);
}

bool CatalogAccessor::DropNamespace(namespace_oid_t ns) const { return dbc_->DeleteNamespace(txn_, ns); }

table_oid_t CatalogAccessor::GetTableOid(std::string name) const {
  NormalizeObjectName(&name);
  for (auto &path : search_path_) {
    table_oid_t search_result = dbc_->GetTableOid(txn_, path, name);
    if (search_result != INVALID_TABLE_OID) return search_result;
  }
  return INVALID_TABLE_OID;
}

table_oid_t CatalogAccessor::GetTableOid(namespace_oid_t ns, std::string name) const {
  NormalizeObjectName(&name);
  return dbc_->GetTableOid(txn_, ns, name);
}

table_oid_t CatalogAccessor::CreateTable(namespace_oid_t ns, std::string name, const Schema &schema) const {
  NormalizeObjectName(&name);
  return dbc_->CreateTable(txn_, ns, name, schema);
}

bool CatalogAccessor::RenameTable(table_oid_t table, std::string new_table_name) const {
  NormalizeObjectName(&new_table_name);
  return dbc_->RenameTable(txn_, table, new_table_name);
}

bool CatalogAccessor::DropTable(table_oid_t table) const { return dbc_->DeleteTable(txn_, table); }

bool CatalogAccessor::SetTablePointer(table_oid_t table, storage::SqlTable *table_ptr) const {
  return dbc_->SetTablePointer(txn_, table, table_ptr);
}

common::ManagedPointer<storage::SqlTable> CatalogAccessor::GetTable(table_oid_t table) const {
  return dbc_->GetTable(txn_, table);
}

common::ManagedPointer<std::shared_mutex> CatalogAccessor::GetTableLock(table_oid_t table) const {
  return dbc_->GetTableLock(txn_, table);
}

bool CatalogAccessor::UpdateSchema(table_oid_t table, Schema *new_schema) const {
  return dbc_->UpdateSchema(txn_, table, new_schema);
}

const Schema &CatalogAccessor::GetSchema(table_oid_t table) const { return dbc_->GetSchema(txn_, table); }

std::vector<constraint_oid_t> CatalogAccessor::GetConstraints(table_oid_t table) const {
  return dbc_->GetConstraints(txn_, table);
}

std::vector<index_oid_t> CatalogAccessor::GetIndexOids(table_oid_t table, bool only_live) const {
  return dbc_->GetIndexOids(txn_, table, only_live);
}

std::vector<std::pair<common::ManagedPointer<storage::index::Index>, const IndexSchema &>> CatalogAccessor::GetIndexes(
    table_oid_t table) {
  return dbc_->GetIndexes(txn_, table);
}

index_oid_t CatalogAccessor::GetIndexOid(std::string name) const {
  NormalizeObjectName(&name);
  for (auto &path : search_path_) {
    index_oid_t search_result = dbc_->GetIndexOid(txn_, path, name);
    if (search_result != INVALID_INDEX_OID) return search_result;
  }
  return INVALID_INDEX_OID;
}

index_oid_t CatalogAccessor::GetIndexOid(namespace_oid_t ns, std::string name) const {
  NormalizeObjectName(&name);
  return dbc_->GetIndexOid(txn_, ns, name);
}

index_oid_t CatalogAccessor::CreateIndex(namespace_oid_t ns, table_oid_t table, std::string name,
                                         const IndexSchema &schema) const {
  NormalizeObjectName(&name);
  return dbc_->CreateIndex(txn_, ns, name, table, schema);
}

bool CatalogAccessor::SetIndexLive(index_oid_t index) const { return dbc_->SetIndexLive(txn_, index); }

const IndexSchema &CatalogAccessor::GetIndexSchema(index_oid_t index) const {
  return dbc_->GetIndexSchema(txn_, index);
}

bool CatalogAccessor::DropIndex(index_oid_t index) const { return dbc_->DeleteIndex(txn_, index); }

bool CatalogAccessor::SetIndexPointer(index_oid_t index, storage::index::Index *index_ptr) const {
  return dbc_->SetIndexPointer(txn_, index, index_ptr);
}

common::ManagedPointer<storage::index::Index> CatalogAccessor::GetIndex(index_oid_t index) const {
  return dbc_->GetIndex(txn_, index);
}

language_oid_t CatalogAccessor::CreateLanguage(const std::string &lanname) {
  return dbc_->CreateLanguage(txn_, lanname);
}

language_oid_t CatalogAccessor::GetLanguageOid(const std::string &lanname) {
  return dbc_->GetLanguageOid(txn_, lanname);
}

bool CatalogAccessor::DropLanguage(language_oid_t language_oid) { return dbc_->DropLanguage(txn_, language_oid); }

proc_oid_t CatalogAccessor::CreateProcedure(const std::string &procname, language_oid_t language_oid,
                                            namespace_oid_t procns, const std::vector<std::string> &args,
                                            const std::vector<type_oid_t> &arg_types,
                                            const std::vector<type_oid_t> &all_arg_types,
                                            const std::vector<postgres::ProArgModes> &arg_modes, type_oid_t rettype,
                                            const std::string &src, bool is_aggregate) {
  return dbc_->CreateProcedure(txn_, procname, language_oid, procns, args, arg_types, all_arg_types, arg_modes, rettype,
                               src, is_aggregate);
}

bool CatalogAccessor::DropProcedure(proc_oid_t proc_oid) { return dbc_->DropProcedure(txn_, proc_oid); }

proc_oid_t CatalogAccessor::GetProcOid(const std::string &procname, const std::vector<type_oid_t> &arg_types) {
  proc_oid_t ret;
  for (auto ns_oid : search_path_) {
    ret = dbc_->GetProcOid(txn_, ns_oid, procname, arg_types);
    if (ret != catalog::INVALID_PROC_OID) {
      return ret;
    }
  }
  return catalog::INVALID_PROC_OID;
}

bool CatalogAccessor::SetProcCtxPtr(proc_oid_t proc_oid, const execution::udf::UDFContext *udf_context) {
  return dbc_->SetProcCtxPtr(txn_, proc_oid, udf_context);
}

common::ManagedPointer<execution::udf::UDFContext> CatalogAccessor::GetProcCtxPtr(proc_oid_t proc_oid) {
  return dbc_->GetProcCtxPtr(txn_, proc_oid);
}

common::ManagedPointer<execution::udf::UDFContext> CatalogAccessor::GetUDFContext(proc_oid_t proc_oid) {
  return dbc_->GetUDFContext(txn_, proc_oid);
}

type_oid_t CatalogAccessor::GetTypeOidFromTypeId(type::TypeId type) { return dbc_->GetTypeOidForType(type); }

common::ManagedPointer<storage::BlockStore> CatalogAccessor::GetBlockStore() const {
  // TODO(Matt): at some point we may decide to adjust the source  (i.e. each DatabaseCatalog has one), stick it in a
  // pg_tablespace table, or we may eliminate the concept entirely. This works for now to allow CREATE nodes to bind a
  // BlockStore
  return catalog_->GetBlockStore();
}

common::ManagedPointer<transaction::TransactionContext> CatalogAccessor::GetTransactionContext() const { return txn_; }

}  // namespace terrier::catalog
