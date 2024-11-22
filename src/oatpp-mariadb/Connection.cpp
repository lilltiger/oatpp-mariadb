#include "Connection.hpp"

namespace oatpp { namespace mariadb {

void Connection::setInvalidator(const std::shared_ptr<provider::Invalidator<Connection>>& invalidator) {
  m_invalidator = invalidator;
}

std::shared_ptr<provider::Invalidator<Connection>> Connection::getInvalidator() {
  return m_invalidator;
}

ConnectionImpl::ConnectionImpl(MYSQL* mysql)
  : m_connection(mysql)
{}

ConnectionImpl::~ConnectionImpl() {
  if (m_connection) {
    mysql_close(m_connection);
    m_connection = nullptr;
  }
}

MYSQL* ConnectionImpl::getHandle() {
  return m_connection;
}

}}