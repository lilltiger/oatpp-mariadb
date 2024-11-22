#ifndef oatpp_mariadb_mapping_ResultMapper_hpp
#define oatpp_mariadb_mapping_ResultMapper_hpp

#include "Deserializer.hpp"
#include "oatpp/core/data/mapping/TypeResolver.hpp"
#include "oatpp/core/Types.hpp"

#include <mysql/mysql.h>

namespace oatpp { namespace mariadb { namespace mapping {

/**
 * Mapper from mariadb result to oatpp objects.
 */
class ResultMapper {
public:

  /**
   * Result data. Get data row by row.
   */
  struct ResultData {

    /**
     * Constructor.
     * @param pStmt
     * @param pTypeResolver
     */
    ResultData(MYSQL_STMT* pStmt, const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver);

    /**
     * Destructor. Free mariadb resources.
     */
    ~ResultData();

    /**
     * mariadb statement.
     */
    MYSQL_STMT* stmt;

    /**
     * &id:oatpp::data::mapping::TypeResolver;.
     */
    std::shared_ptr<const data::mapping::TypeResolver> typeResolver;

    /**
     * Names of columns.
     */
    std::vector<oatpp::String> colNames;

    /**
     * Column indices.
     */
    std::unordered_map<data::share::StringKeyLabel, v_int32> colIndices;

    /**
     * Column count.
     */
    v_int64 colCount;

    /**
     * Current row index.
     */
    v_int64 rowIndex;

    /**
     * Has more to read.
     */
    bool hasMore;

    /**
     * Is success.
     */
    bool isSuccess;

    /**
     * Bind results for cache.
     */
    std::vector<MYSQL_BIND> bindResults;

    /**
     * Meta results, the null represents that it is no result.
     */
    MYSQL_RES* metaResults;

  public:

    /**
     * Initialize column names and indices.
     */
    void init();

    /**
     * Move to next row.
     */
    void next();

    /**
     * Bind results for cache.
     */
    void bindResultsForCache();

  };

private:
  typedef oatpp::data::mapping::type::Type Type;
  typedef oatpp::Void (*ReadOneRowMethod)(ResultMapper*, ResultData*, const Type*);
  typedef oatpp::Void (*ReadRowsMethod)(ResultMapper*, ResultData*, const Type*, v_int64);
private:

  // Read one row methods
  static oatpp::Void readOneRowAsCollection(ResultMapper* _this, ResultData* dbData, const Type* type);
  static oatpp::Void readOneRowAsMap(ResultMapper* _this, ResultData* dbData, const Type* type);
  static oatpp::Void readOneRowAsObject(ResultMapper* _this, ResultData* dbData, const Type* type);

  // Read rows methods
  static oatpp::Void readRowsAsCollection(ResultMapper* _this, ResultData* dbData, const Type* type, v_int64 count);
  static oatpp::Void readRowsAsObject(ResultMapper* _this, ResultData* dbData, const Type* type, v_int64 count);

private:
  Deserializer m_deserializer;
  std::vector<ReadOneRowMethod> m_readOneRowMethods;
  std::vector<ReadRowsMethod> m_readRowsMethods;
public:

  /**
   * Default constructor.
   */
  ResultMapper();

  /**
   * Set "read one row" method for class id.
   * @param classId
   * @param method
   */
  void setReadOneRowMethod(const data::mapping::type::ClassId& classId, ReadOneRowMethod method);

  /**
   * Set "read rows" method for class id.
   * @param classId
   * @param method
   */
  void setReadRowsMethod(const data::mapping::type::ClassId& classId, ReadRowsMethod method);

  /**
   * Read one row to oatpp object or collection. <br>
   * Allowed output type classes are:
   *
   * - &id:oatpp::Vector;
   * - &id:oatpp::List;
   * - &id:oatpp::UnorderedSet;
   * - &id:oatpp::Fields;
   * - &id:oatpp::UnorderedFields;
   * - &id:oatpp::Object;
   *
   * @param dbData
   * @param type
   * @return
   */
  oatpp::Void readOneRow(ResultData* dbData, const Type* type);

  /**
   * Read `count` of rows to oatpp collection. <br>
   * Allowed collections to store rows are:
   *
   * - &id:oatpp::Vector;
   * - &id:oatpp::List;
   * - &id:oatpp::UnorderedSet;.
   *
   * @param dbData
   * @param type
   * @param count
   * @return
   */
  oatpp::Void readRows(ResultData* dbData, const Type* type, v_int64 count);

};

}}}

#endif //oatpp_mariadb_mapping_ResultMapper_hpp
