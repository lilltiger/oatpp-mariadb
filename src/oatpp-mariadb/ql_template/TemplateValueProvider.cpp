/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "TemplateValueProvider.hpp"

namespace oatpp { namespace mariadb { namespace ql_template {

// e.g. select * from t_user where id = :user.id and name = :user.name
//   -> select * from t_user where id = ? and name = ?
oatpp::String TemplateValueProvider::getValue(const data::share::StringTemplate::Variable& variable, v_uint32 index) {
  m_buffStream.setCurrentPosition(0);
  m_buffStream << "?";
  return m_buffStream.toString();
}

}}}
