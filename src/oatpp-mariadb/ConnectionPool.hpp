#ifndef oatpp_mariadb_ConnectionPool_hpp
#define oatpp_mariadb_ConnectionPool_hpp

#include "ConnectionProvider.hpp"

namespace oatpp { namespace mariadb {

/**
 * Convenience typedef for ConnectionPool.
 * Uses the pool implementation from ConnectionProvider.hpp.
 */
using ConnectionPool = oatpp::provider::Pool<
  provider::Provider<Connection>,
  Connection,
  ConnectionAcquisitionProxy
>;

}}

#endif /* oatpp_mariadb_ConnectionPool_hpp */
