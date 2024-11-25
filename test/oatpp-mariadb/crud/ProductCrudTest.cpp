#include "ProductCrudTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/TransactionGuard.hpp"
#include "oatpp-mariadb/mapping/JsonHelper.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace crud {

namespace {

const char* const TAG = "TEST[mariadb::crud::ProductCrudTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class ProductDto : public oatpp::DTO {
  DTO_INIT(ProductDto, DTO);
  DTO_FIELD(Int32, id);
  DTO_FIELD(String, name, "name");
  DTO_FIELD(Float32, price, "price");
  DTO_FIELD(Int32, stock, "stock");
  DTO_FIELD(Boolean, active, "active");
  DTO_FIELD(String, created_at, "created_at");
  DTO_FIELD(String, updated_at, "updated_at");
  DTO_FIELD(String, metadata, "metadata");
};

class CountResult : public oatpp::DTO {
  DTO_INIT(CountResult, DTO);
  DTO_FIELD(Int32, count, "count");
};

class TransactionState : public oatpp::DTO {
  DTO_INIT(TransactionState, DTO);
  DTO_FIELD(String, in_transaction);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class TestClient : public oatpp::orm::DbClient {
private:
  oatpp::parser::json::mapping::ObjectMapper m_objectMapper;

public:
  TestClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
    , m_objectMapper()
  {
    setEnabledInterpretations({"DEFAULT", "POSTGRES", "MARIADB"});
    m_objectMapper.getSerializer()->getConfig()->useBeautifier = true;
    oatpp::mariadb::mapping::JsonHelper::setupIntegerSerializers(m_objectMapper);
  }

  template<typename T>
  oatpp::String serializeToJson(const T& obj) {
    return m_objectMapper.writeToString(obj);
  }

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_products` ("
        "`id` INTEGER PRIMARY KEY AUTO_INCREMENT,"
        "`name` VARCHAR(255) NOT NULL,"
        "`price` FLOAT NOT NULL,"
        "`stock` INTEGER NOT NULL DEFAULT 0,"
        "`active` BOOLEAN DEFAULT TRUE,"
        "`created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "`updated_at` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
        "`metadata` JSON,"
        "UNIQUE INDEX `idx_name` (`name`)"
        ");")

  QUERY(dropTable,
        "DROP TABLE IF EXISTS `test_products`;")

  QUERY(createProduct,
        "INSERT INTO `test_products` "
        "(`name`, `price`, `stock`, `active`, `metadata`) "
        "VALUES "
        "(CAST(:product.name AS CHAR), CAST(:product.price AS FLOAT), "
        "CAST(:product.stock AS SIGNED INTEGER), :product.active, :product.metadata) "
        "RETURNING *;",
        PARAM(oatpp::Object<ProductDto>, product))

  QUERY(updateProduct,
        "UPDATE `test_products` SET "
        "`name` = CAST(:product.name AS CHAR), "
        "`price` = CAST(:product.price AS FLOAT), "
        "`stock` = CAST(:product.stock AS SIGNED INTEGER), "
        "`active` = :product.active, "
        "`metadata` = :product.metadata "
        "WHERE `id` = :id;",
        PARAM(oatpp::Int32, id),
        PARAM(oatpp::Object<ProductDto>, product))

  QUERY(getProductById,
        "SELECT * FROM `test_products` WHERE `id` = :id;",
        PARAM(oatpp::Int32, id))

  QUERY(getProductByName,
        "SELECT * FROM `test_products` WHERE `name` = :name;",
        PARAM(oatpp::String, name))

  QUERY(getAllProducts,
        "SELECT * FROM `test_products` ORDER BY `id`;")

  QUERY(getActiveProducts,
        "SELECT * FROM `test_products` WHERE `active` = TRUE ORDER BY `id`;")

  QUERY(deleteProduct,
        "DELETE FROM `test_products` WHERE `id` = :id;",
        PARAM(oatpp::Int32, id))

  QUERY(deleteAllProducts,
        "DELETE FROM `test_products`;")

  QUERY(countProducts,
        "SELECT COUNT(*) as count FROM `test_products`;")

  QUERY(searchProducts,
        "SELECT * FROM `test_products` "
        "WHERE `name` LIKE CONCAT('%', :search, '%') "
        "ORDER BY `id`;",
        PARAM(oatpp::String, search))

  QUERY(setIsolationLevel,
        "SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED;")

  QUERY(getTransactionState,
        "SELECT IF(@@in_transaction, 'true', 'false') as in_transaction;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void ProductCrudTest::onRun() {
  OATPP_LOGD(TAG, "Running Product CRUD Tests...");

  auto env = utils::EnvLoader();
  
  auto options = oatpp::mariadb::ConnectionOptions();
  options.host = env.get("MARIADB_HOST", "127.0.0.1");
  options.port = env.getInt("MARIADB_PORT", 3306);
  options.username = env.get("MARIADB_USER", "root");
  options.password = env.get("MARIADB_PASSWORD", "root");
  options.database = env.get("MARIADB_DATABASE", "test");

  auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
  auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
  auto client = TestClient(executor);

  // Setup: create fresh table
  client.dropTable();
  client.createTable();

  // Test 1: Basic CRUD Operations with Transaction
  {
    OATPP_LOGD(TAG, "Test 1: Basic CRUD Operations with Transaction");
    
    auto conn = executor->getConnection();
    OATPP_ASSERT(conn);
    client.setIsolationLevel(conn);
    
    auto beginResult = executor->begin(conn);
    OATPP_ASSERT(beginResult->isSuccess());

    // Create
    auto product = ProductDto::createShared();
    product->name = "Test Product";
    product->price = 99.99f;
    product->stock = 100;
    product->active = true;
    product->metadata = "{\"category\": \"test\", \"tags\": [\"sample\", \"test\"]}";

    auto result = client.createProduct(product);
    OATPP_ASSERT(result->isSuccess());
    
    auto products = result->fetch<oatpp::Vector<oatpp::Object<ProductDto>>>();
    OATPP_ASSERT(products->size() == 1);
    OATPP_ASSERT(products[0]->id > 0);
    
    // Read
    result = client.getProductById(products[0]->id);
    OATPP_ASSERT(result->isSuccess());
    
    products = result->fetch<oatpp::Vector<oatpp::Object<ProductDto>>>();
    OATPP_ASSERT(products->size() == 1);
    
    auto readProduct = products[0];
    OATPP_ASSERT(readProduct->name == "Test Product");
    OATPP_ASSERT(readProduct->price == 99.99f);
    OATPP_ASSERT(readProduct->stock == 100);
    OATPP_ASSERT(readProduct->active == true);
    
    // Update
    product->name = "Updated Product";
    product->price = 149.99f;
    product->stock = 50;
    product->metadata = "{\"category\": \"updated\", \"tags\": [\"modified\"]}";

    result = client.updateProduct(readProduct->id, product);
    OATPP_ASSERT(result->isSuccess());

    // Verify update by fetching
    result = client.getProductById(readProduct->id);
    OATPP_ASSERT(result->isSuccess());
    products = result->fetch<oatpp::Vector<oatpp::Object<ProductDto>>>();
    OATPP_ASSERT(products->size() == 1);
    
    auto updatedProduct = products[0];
    OATPP_ASSERT(updatedProduct->name == "Updated Product");
    OATPP_ASSERT(updatedProduct->price == 149.99f);
    OATPP_ASSERT(updatedProduct->stock == 50);
    
    // Delete
    auto countResult = client.countProducts();
    OATPP_ASSERT(countResult->isSuccess());
    auto countBefore = countResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0]->count;
    
    result = client.deleteProduct(updatedProduct->id);
    OATPP_ASSERT(result->isSuccess());
    
    countResult = client.countProducts();
    OATPP_ASSERT(countResult->isSuccess());
    auto countAfter = countResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0]->count;
    OATPP_ASSERT(countBefore == countAfter + 1);

    auto commitResult = executor->commit(conn);
    OATPP_ASSERT(commitResult->isSuccess());
  }

  // Test 2: Search and Filter Operations
  {
    OATPP_LOGD(TAG, "Test 2: Search and Filter Operations");

    // Create multiple products
    auto product1 = ProductDto::createShared();
    product1->name = "Search Product 1";
    product1->price = 10.99f;
    product1->stock = 50;
    product1->active = true;
    
    auto product2 = ProductDto::createShared();
    product2->name = "Search Product 2";
    product2->price = 20.99f;
    product2->stock = 30;
    product2->active = false;

    auto result = client.createProduct(product1);
    OATPP_ASSERT(result->isSuccess());
    
    result = client.createProduct(product2);
    OATPP_ASSERT(result->isSuccess());

    // Test search
    result = client.searchProducts("Search");
    OATPP_ASSERT(result->isSuccess());
    auto products = result->fetch<oatpp::Vector<oatpp::Object<ProductDto>>>();
    OATPP_ASSERT(products->size() == 2);

    // Test active filter
    result = client.getActiveProducts();
    OATPP_ASSERT(result->isSuccess());
    products = result->fetch<oatpp::Vector<oatpp::Object<ProductDto>>>();
    OATPP_ASSERT(products->size() == 1);
    OATPP_ASSERT(products[0]->name == "Search Product 1");
  }

  OATPP_LOGD(TAG, "Product CRUD Tests completed successfully!");
}

}}}}
