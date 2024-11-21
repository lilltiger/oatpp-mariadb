#ifndef oatpp_mariadb_Connection_hpp
#define oatpp_mariadb_Connection_hpp

#include "oatpp/orm/Connection.hpp"
#include "oatpp/core/provider/Pool.hpp"
#include "oatpp/core/Types.hpp"

#include <mysql/mysql.h>

namespace oatpp { namespace mariadb {

class Connection : public oatpp::orm::Connection {
private:
  std::shared_ptr<provider::Invalidator<Connection>> m_invalidator;
public:

  /**
   * Get MYSQL native connection handle.
   * @return - MYSQL native connection handle.
   */
  virtual MYSQL* getHandle() = 0;

  void setInvalidator(const std::shared_ptr<provider::Invalidator<Connection>>& invalidator);
  std::shared_ptr<provider::Invalidator<Connection>> getInvalidator();

};

class ConnectionImpl : public Connection {
private:
  MYSQL* m_connection;

public:

  ConnectionImpl(MYSQL* connection);
  ~ConnectionImpl();

  MYSQL* getHandle() override;

};

struct ConnectionAcquisitionProxy : public provider::AcquisitionProxy<Connection, ConnectionAcquisitionProxy> {
  ConnectionAcquisitionProxy(const provider::ResourceHandle<Connection>& resource,
                             const std::shared_ptr<PoolInstance>& pool)
    : provider::AcquisitionProxy<Connection, ConnectionAcquisitionProxy>(resource, pool)
  {}

  MYSQL* getHandle() override {
    return _handle.object->getHandle();
  }
};

}}

#endif /* oatpp_mariadb_Connection_hpp */