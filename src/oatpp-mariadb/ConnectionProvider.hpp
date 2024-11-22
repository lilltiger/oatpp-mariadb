#ifndef oatpp_mariadb_ConnectionProvider_hpp
#define oatpp_mariadb_ConnectionProvider_hpp

#include "Connection.hpp"

#include "oatpp/core/provider/Pool.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace mariadb {

struct ConnectionOptions {
  oatpp::String host;
  v_uint16 port;
  oatpp::String database;
  oatpp::String username;
  oatpp::String password;
};

class ConnectionProvider : public provider::Provider<Connection> {
private:

  class ConnectionInvalidator : public provider::Invalidator<Connection> {
  public:
    void invalidate(const std::shared_ptr<Connection>& connection) override;
  };

private:
  std::shared_ptr<ConnectionInvalidator> m_invalidator;
  ConnectionOptions m_options;

public:

  /**
   * Constructor.
   * @param options - connection options.
   */
  ConnectionProvider(const ConnectionOptions& options);

  /**
   * Get connection.
   * @return - resource handle to the connection.
   */
  provider::ResourceHandle<Connection> get() override;

  /**
   * Get connection asynchronously.
   * @return - coroutine handle to the connection.
   */
  async::CoroutineStarterForResult<const provider::ResourceHandle<Connection>&> getAsync() override;

  /**
   * Stop the provider.
   */
  void stop() override;

  /**
   * Clear all connections.
   */
  void clear();

};

/**
 * Connection pool.
 */
typedef oatpp::provider::Pool<
  provider::Provider<Connection>,
  Connection,
  ConnectionAcquisitionProxy
> ConnectionPool;

}}

#endif /* oatpp_mariadb_ConnectionProvider_hpp */