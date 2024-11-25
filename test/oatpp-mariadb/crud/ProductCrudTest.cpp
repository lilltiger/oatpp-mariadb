#include "ProductCrudTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
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
};

class CountResult : public oatpp::DTO {
  DTO_INIT(CountResult, DTO);
  DTO_FIELD(Int32, count, "count");
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class TestClient : public oatpp::orm::DbClient {
public:
  TestClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    setEnabledInterpretations({"DEFAULT"});
  }

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_products` ("
        "`id` INTEGER PRIMARY KEY AUTO_INCREMENT,"
        "`name` VARCHAR(255) NOT NULL,"
        "`price` FLOAT NOT NULL,"
        "`stock` INTEGER NOT NULL DEFAULT 0,"
        "`active` BOOLEAN DEFAULT TRUE,"
        "`created_at` DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");")

  QUERY(dropTable,
        "DROP TABLE IF EXISTS `test_products`;")

  QUERY(createProduct,
        "INSERT INTO `test_products` (`name`, `price`, `stock`, `active`) "
        "VALUES (:product.name, :product.price, :product.stock, :product.active) "
        "RETURNING *;",
        PARAM(oatpp::Object<ProductDto>, product))

  QUERY(updateProduct,
        "UPDATE `test_products` "
        "SET `name` = :product.name, "
        "`price` = :product.price, "
        "`stock` = :product.stock, "
        "`active` = :product.active "
        "WHERE `id` = :id;",
        PARAM(oatpp::Int32, id),
        PARAM(oatpp::Object<ProductDto>, product))

  QUERY(getProductById,
        "SELECT * FROM `test_products` WHERE `id` = :id;",
        PARAM(oatpp::Int32, id))

  QUERY(getAllProducts,
        "SELECT * FROM `test_products` ORDER BY `id`;")

  QUERY(deleteProduct,
        "DELETE FROM `test_products` WHERE `id` = :id;",
        PARAM(oatpp::Int32, id))

  QUERY(countProducts,
        "SELECT COUNT(*) as count FROM `test_products`;")

  QUERY(deleteAllProducts,
        "DELETE FROM `test_products`;")
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

  // Test 1: Create
  {
    OATPP_LOGD(TAG, "Test 1: Create operation");
    
    auto product = ProductDto::createShared();
    product->name = "Test Product";
    product->price = 99.99f;
    product->stock = 100;
    product->active = true;

    auto result = client.createProduct(product);
    OATPP_ASSERT(result->isSuccess());
    
    auto products = result->fetch<oatpp::Vector<oatpp::Object<ProductDto>>>();
    OATPP_ASSERT(products->size() == 1);
    OATPP_ASSERT(products[0]->id > 0);
    OATPP_LOGD(TAG, "Created product with id=%d", *products[0]->id);
  }

  // Test 2: Read
  {
    OATPP_LOGD(TAG, "Test 2: Read operation");
    
    auto result = client.getProductById(1);
    OATPP_ASSERT(result->isSuccess());
    
    auto products = result->fetch<oatpp::Vector<oatpp::Object<ProductDto>>>();
    OATPP_ASSERT(products->size() == 1);
    
    auto product = products[0];
    OATPP_ASSERT(product->name == "Test Product");
    OATPP_ASSERT(product->price == 99.99f);
    OATPP_ASSERT(product->stock == 100);
    OATPP_ASSERT(product->active == true);
  }

  // Test 3: Update
  {
    OATPP_LOGD(TAG, "Test 3: Update operation");
    
    auto product = ProductDto::createShared();
    product->name = "Updated Product";
    product->price = 149.99f;
    product->stock = 50;
    product->active = true;

    auto result = client.updateProduct(1, product);
    OATPP_ASSERT(result->isSuccess());

    // Verify update
    result = client.getProductById(1);
    OATPP_ASSERT(result->isSuccess());
    
    auto products = result->fetch<oatpp::Vector<oatpp::Object<ProductDto>>>();
    OATPP_ASSERT(products->size() == 1);
    
    auto updatedProduct = products[0];
    OATPP_ASSERT(updatedProduct->name == "Updated Product");
    OATPP_ASSERT(updatedProduct->price == 149.99f);
    OATPP_ASSERT(updatedProduct->stock == 50);
  }

  // Test 4: Delete
  {
    OATPP_LOGD(TAG, "Test 4: Delete operation");
    
    // Get count before delete
    auto countResult = client.countProducts();
    OATPP_ASSERT(countResult->isSuccess());
    auto countBefore = countResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0]->count;
    
    auto result = client.deleteProduct(1);
    OATPP_ASSERT(result->isSuccess());
    
    // Verify count after delete
    countResult = client.countProducts();
    OATPP_ASSERT(countResult->isSuccess());
    auto countAfter = countResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0]->count;
    OATPP_ASSERT(countBefore == countAfter + 1);

    // Verify product is gone
    result = client.getProductById(1);
    OATPP_ASSERT(result->isSuccess());
    auto products = result->fetch<oatpp::Vector<oatpp::Object<ProductDto>>>();
    OATPP_ASSERT(products->size() == 0);
  }

  OATPP_LOGD(TAG, "Product CRUD Tests completed successfully!");
}

}}}}
