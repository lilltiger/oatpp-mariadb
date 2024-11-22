#ifndef oatpp_mariadb_QueryResult_hpp
#define oatpp_mariadb_QueryResult_hpp

#include "oatpp/core/provider/Pool.hpp"
#include "oatpp/core/Types.hpp"

#include <mysql/mysql.h>

#include "ConnectionProvider.hpp"
#include "mapping/Deserializer.hpp"
#include "mapping/ResultMapper.hpp"
#include "oatpp/orm/QueryResult.hpp"

namespace oatpp { namespace mariadb {

/**
 * Implementation of &id:oatpp::orm::QueryResult;. for mariadb.
 */
class QueryResult : public orm::QueryResult {
private:
  MYSQL_STMT* m_stmt;
  provider::ResourceHandle<orm::Connection> m_connection;
  std::shared_ptr<mapping::ResultMapper> m_resultMapper;
  mapping::ResultMapper::ResultData m_resultData;
  oatpp::String m_errorMessage;
  bool m_inTransaction;
  v_int64 m_lastInsertId;

  /**
   * Clean up statement resources safely
   * @return true if cleanup was successful
   */
  bool cleanupStatement();

public:

  QueryResult(MYSQL_STMT* stmt,
              const provider::ResourceHandle<orm::Connection>& connection,
              const std::shared_ptr<mapping::ResultMapper>& resultMapper,
              const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver);

  ~QueryResult();

  provider::ResourceHandle<orm::Connection> getConnection() const override;

  bool isSuccess() const override;

  oatpp::String getErrorMessage() const override;

  v_int64 getPosition() const override;

  v_int64 getKnownCount() const override;

  bool hasMoreToFetch() const override;

  oatpp::Void fetch(const oatpp::Type* const type, v_int64 count) override;

  /**
   * Get the ID generated for an AUTO_INCREMENT column by the previous INSERT query.
   * This will first try to get the ID from RETURNING clause if available,
   * otherwise fall back to mysql_insert_id().
   * @return The last insert ID
   */
  v_int64 getLastInsertId() const;

  /**
   * Get the number of rows affected by the last INSERT, UPDATE, REPLACE or DELETE query
   * @return Number of affected rows
   */
  v_int64 getAffectedRows() const;

  /**
   * Set the last insert ID from RETURNING clause
   * @param id The insert ID from RETURNING
   */
  void setLastInsertId(v_int64 id);

};

}}

#endif //oatpp_mariadb_QueryResult_hpp