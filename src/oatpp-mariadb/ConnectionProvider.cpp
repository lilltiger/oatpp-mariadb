#include "ConnectionProvider.hpp"

namespace oatpp { namespace mariadb {

void ConnectionProvider::ConnectionInvalidator::invalidate(const std::shared_ptr<Connection>& connection) {
  (void) connection; // unused
  // TODO: implement connection invalidation
}

ConnectionProvider::ConnectionProvider(const ConnectionOptions& options)
  : m_options(options)
  , m_invalidator(std::make_shared<ConnectionInvalidator>())
{}

provider::ResourceHandle<Connection> ConnectionProvider::get() {
  MYSQL* handle = mysql_init(nullptr);
  if (handle == nullptr) {
      throw std::runtime_error("[oatpp::mariadb::ConnectionProvider::get()]: " 
        "Failed to initialize MySQL connection. Error: " + std::string(mysql_error(handle)));
  }
    
  // Set max_allowed_packet to 16MB to handle large text fields
  unsigned long max_allowed_packet = 16 * 1024 * 1024;
  if (mysql_options(handle, MYSQL_OPT_MAX_ALLOWED_PACKET, &max_allowed_packet)) {
    throw std::runtime_error("[oatpp::mariadb::ConnectionProvider::get()]: " 
      "Failed to set max_allowed_packet. Error: " + std::string(mysql_error(handle)));
  }

  MYSQL* result = mysql_real_connect(handle, 
    m_options.host->c_str(), 
    m_options.username->c_str(), 
    m_options.password->c_str(), 
    m_options.database->c_str(), 
    m_options.port, 
    nullptr, 
    CLIENT_FOUND_ROWS | CLIENT_MULTI_STATEMENTS);  // Enable RETURNING support and multi-statement support

  if (result == nullptr) {
    throw std::runtime_error("[oatpp::mariadb::ConnectionProvider::get()]: " 
      "Failed to connect to MySQL server. Error: " + std::string(mysql_error(handle)));
  }

  if (mysql_set_character_set(handle, "utf8") != 0) {
    throw std::runtime_error("[oatpp::mariadb::ConnectionProvider::get()]: " 
      "Failed to set character set to utf8. Error: " + std::string(mysql_error(handle)));
  }

  return provider::ResourceHandle<Connection>(std::make_shared<ConnectionImpl>(handle), m_invalidator);
}

async::CoroutineStarterForResult<const provider::ResourceHandle<Connection>&> ConnectionProvider::getAsync() {
  throw std::runtime_error("[oatpp::mariadb::ConnectionProvider::getAsync()]: Not implemented!");
}

void ConnectionProvider::stop() {
  // Already implemented
}

void ConnectionProvider::clear() {
  // No need to do anything here as connections are automatically closed
  // when their shared_ptr is destroyed
}

}}