#ifndef AnyTypeTest_hpp
#define AnyTypeTest_hpp

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class AnyTypeTest : public oatpp::test::UnitTest {
private:
  const char* TAG = "AnyTypeTest";
public:
  AnyTypeTest() : UnitTest("TEST[AnyTypeTest]") {}

  void onRun() override {
    testAnyCreation();
    testAnyRetrieval();
    testAnyCopy();
    testAnyTypeCheck();
  }

  void testAnyCreation() {
    oatpp::Int64 value = 42;
    oatpp::Any any(value);
    
    OATPP_ASSERT(any.getValueType() == oatpp::data::mapping::type::__class::Any::getType());
    OATPP_ASSERT(any.retrieve<oatpp::Int64>() == value);
  }

  void testAnyRetrieval() {
    oatpp::Int64 originalValue = 123;
    oatpp::Any any(originalValue);
    
    auto retrievedValue = any.retrieve<oatpp::Int64>();
    OATPP_ASSERT(retrievedValue == originalValue);
  }

  void testAnyCopy() {
    oatpp::Any original(oatpp::Int64(42));
    oatpp::Any copy(original);
    
    OATPP_ASSERT(copy.getValueType() == original.getValueType());
    OATPP_ASSERT(copy.retrieve<oatpp::Int64>() == original.retrieve<oatpp::Int64>());
  }

  void testAnyTypeCheck() {
    oatpp::Any any(oatpp::Int64(42));
    
    OATPP_ASSERT(any.getValueType() == oatpp::data::mapping::type::__class::Any::getType());
    
    bool thrown = false;
    try {
      any.retrieve<oatpp::String>();
    } catch (const std::runtime_error& e) {
      thrown = true;
    }
    OATPP_ASSERT(thrown);
  }
};

}}}}

#endif // AnyTypeTest_hpp
