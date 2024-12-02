#include "FlagTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp-mariadb/types/Flag.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::FlagTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class FlagRow : public oatpp::DTO {
    DTO_INIT(FlagRow, DTO);
    DTO_FIELD(Int64, id);
    DTO_FIELD(UInt64, permissions);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class TestClient : public oatpp::orm::DbClient {
public:
    TestClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
        : oatpp::orm::DbClient(executor)
    {}
    
    QUERY(createTable,
          "CREATE TABLE IF NOT EXISTS `flag_test` ("
          "`id` BIGINT NOT NULL AUTO_INCREMENT,"
          "`permissions` BIGINT UNSIGNED NOT NULL,"
          "PRIMARY KEY (`id`)"
          ") ENGINE=InnoDB;")
    
    QUERY(dropTable,
          "DROP TABLE IF EXISTS `flag_test`;")
    
    QUERY(insertRow,
          "INSERT INTO `flag_test` (`permissions`) VALUES (:row.permissions) RETURNING *;",
          PARAM(oatpp::Object<FlagRow>, row))
    
    QUERY(selectAll,
          "SELECT * FROM `flag_test` ORDER BY `id`;")
    
    QUERY(deleteAll,
          "DELETE FROM `flag_test`;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void FlagTest::testFlag8() {
    using Flag8 = oatpp::mariadb::types::Flag<8>;
    
    Flag8::registerFlag("READ", 1);
    Flag8::registerFlag("WRITE", 2);
    Flag8::registerFlag("EXECUTE", 4);
    
    Flag8 flags;
    OATPP_ASSERT(!flags.hasFlag("READ"));
    
    flags.setFlag("READ");
    OATPP_ASSERT(flags.hasFlag("READ"));
    OATPP_ASSERT(!flags.hasFlag("WRITE"));
    
    flags.setFlag("WRITE");
    OATPP_ASSERT(flags.hasFlag("READ"));
    OATPP_ASSERT(flags.hasFlag("WRITE"));
    
    flags.clearFlag("READ");
    OATPP_ASSERT(!flags.hasFlag("READ"));
    OATPP_ASSERT(flags.hasFlag("WRITE"));
    
    flags.toggleFlag("EXECUTE");
    OATPP_ASSERT(flags.hasFlag("EXECUTE"));
    
    auto str = flags.toString();
    OATPP_ASSERT(str == "WRITE|EXECUTE");
    
    auto parsed = Flag8::fromString(str);
    OATPP_ASSERT(parsed.hasFlag("WRITE"));
    OATPP_ASSERT(parsed.hasFlag("EXECUTE"));
    OATPP_ASSERT(!parsed.hasFlag("READ"));
}

void FlagTest::testFlag16() {
    using Flag16 = oatpp::mariadb::types::Flag<16>;
    
    Flag16::registerFlag("USER", 1);
    Flag16::registerFlag("GROUP", 2);
    Flag16::registerFlag("OTHER", 4);
    Flag16::registerFlag("SPECIAL", 256);
    
    Flag16 flags;
    flags.setFlag("USER");
    flags.setFlag("SPECIAL");
    
    OATPP_ASSERT(flags.hasFlag("USER"));
    OATPP_ASSERT(!flags.hasFlag("GROUP"));
    OATPP_ASSERT(flags.hasFlag("SPECIAL"));
    
    auto str = flags.toString();
    OATPP_ASSERT(str == "USER|SPECIAL");
}

void FlagTest::testFlag32() {
    using Flag32 = oatpp::mariadb::types::Flag<32>;
    
    Flag32::registerFlag("LOW", 1);
    Flag32::registerFlag("MEDIUM", 0x10000);
    Flag32::registerFlag("HIGH", 0x1000000);
    
    Flag32 flags;
    flags.setFlag("LOW");
    flags.setFlag("HIGH");
    
    OATPP_ASSERT(flags.hasFlag("LOW"));
    OATPP_ASSERT(!flags.hasFlag("MEDIUM"));
    OATPP_ASSERT(flags.hasFlag("HIGH"));
    
    auto str = flags.toString();
    OATPP_ASSERT(str == "LOW|HIGH");
}

void FlagTest::testFlag64() {
    using Flag64 = oatpp::mariadb::types::Flag<64>;
    
    Flag64::registerFlag("BIT0", 1ULL);
    Flag64::registerFlag("BIT32", 1ULL << 32);
    Flag64::registerFlag("BIT63", 1ULL << 63);
    
    Flag64 flags;
    flags.setFlag("BIT0");
    flags.setFlag("BIT63");
    
    OATPP_ASSERT(flags.hasFlag("BIT0"));
    OATPP_ASSERT(!flags.hasFlag("BIT32"));
    OATPP_ASSERT(flags.hasFlag("BIT63"));
    
    auto str = flags.toString();
    OATPP_ASSERT(str == "BIT0|BIT63");
}

void FlagTest::testInvalidValues() {
    using Flag8 = oatpp::mariadb::types::Flag<8>;
    
    bool exceptionCaught = false;
    try {
        Flag8::registerFlag("INVALID", 256); // Too large for 8 bits
    } catch (const std::runtime_error& e) {
        exceptionCaught = true;
        OATPP_ASSERT(std::string(e.what()).find("exceeds maximum") != std::string::npos);
    }
    OATPP_ASSERT(exceptionCaught);
    
    Flag8 flags;
    exceptionCaught = false;
    try {
        flags.setFlag(0xFF + 1); // Too large for 8 bits
    } catch (const std::runtime_error& e) {
        exceptionCaught = true;
        OATPP_ASSERT(std::string(e.what()).find("exceeds maximum") != std::string::npos);
    }
    OATPP_ASSERT(exceptionCaught);
}

void FlagTest::onRun() {
    testFlag8();
    testFlag16();
    testFlag32();
    testFlag64();
    testInvalidValues();
}

}}}}

void runFlagTests() {
    OATPP_RUN_TEST(oatpp::test::mariadb::types::FlagTest);
} 