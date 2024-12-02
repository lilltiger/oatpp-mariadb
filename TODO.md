# TODO: Common Types Implementation

## Currently Implemented Types
- [x] Email (RFC 5322 compliant, extends oatpp::String)
- [x] Phone Number (International format with extensions, extends oatpp::String)
- [x] URL (Multiple protocols, IPv4/IPv6, IDN support, extends oatpp::String)
- [x] Currency Amount (Configurable decimal places, extends oatpp::Float64)

## Integration with oatpp Types
Our custom types extend oatpp's type system while adding:
- Validation rules
- Normalization logic
- Database constraints
- Format-specific features

### Base Types (provided by oatpp)
- Int32, Int64, UInt8, Float64 (numeric types)
- String (text types)
- Boolean
- Any (for complex types)
- Vector, List, UnorderedSet (collections)

## Types to Implement

### Status and State Types
- [ ] Status
  - Predefined status values
  - State transition validation
  - Custom status definitions
  - Integration with database ENUM

- [ ] Flag
  - Bit field support
  - Multiple flag combinations
  - Named flags
  - Integration with database SET or BIT

- [ ] Progress
  - Percentage validation (0-100)
  - Optional decimal precision
  - Integration with database DECIMAL

### Date and Time Types (extending oatpp::String)
- [ ] Date
  - ISO8601 format validation
  - Leap year checks
  - Range validation
  - Format normalization
  - Integration with database DATETIME

- [ ] Time
  - ISO8601 format validation
  - Timezone handling
  - Millisecond precision
  - Integration with database TIME

- [ ] DateTime
  - Full ISO8601 compliance
  - Timezone conversion
  - Range validation
  - Integration with database TIMESTAMP

- [ ] Duration
  - ISO8601 duration format
  - Period calculations
  - Integration with database INTERVAL

### Identifier Types (extending oatpp::String)
- [ ] UUID
  - RFC 4122 validation
  - Version support
  - Format normalization
  - Integration with database BINARY(16)

- [ ] SemVer
  - SemVer 2.0.0 validation
  - Version comparison
  - Integration with database VARCHAR

- [ ] Slug
  - URL-safe string
  - Unicode to ASCII conversion
  - Case normalization
  - Integration with database VARCHAR

- [ ] Reference
  - Foreign key validation
  - Referential integrity checks
  - Lazy loading support
  - Integration with database references

### Network Types (extending oatpp::String)
- [ ] IPv4
  - Format validation
  - Range checking
  - CIDR support
  - Integration with database VARCHAR

- [ ] IPv6
  - Format validation
  - Compression handling
  - CIDR support
  - Integration with database VARCHAR

- [ ] MAC Address
  - Format validation
  - Case normalization
  - Integration with database VARCHAR

- [ ] Port
  - Range validation (0-65535)
  - Well-known port checks
  - Integration with database SMALLINT

### Geographic Types
- [ ] GeoCoordinates
  - Format validation
  - Range checking
  - Integration with database POINT

- [ ] CountryCode
  - ISO 3166-1 validation
  - Case normalization
  - Integration with database CHAR

- [ ] Address
  - Street, city, state, postal code
  - Format validation by country
  - Integration with database JSON

- [ ] Region
  - Hierarchical structure
  - Parent-child relationships
  - Integration with database VARCHAR

### Document Types
- [ ] JSON
  - Structure validation
  - Schema support
  - Integration with database JSON

- [ ] MIME Type
  - Format validation
  - Parameters support
  - Integration with database VARCHAR

- [ ] Markdown
  - Format validation
  - Safe HTML conversion
  - Integration with database TEXT

- [ ] XML
  - Schema validation
  - Namespace support
  - Integration with database TEXT

### Security Types
- [ ] PasswordHash
  - Algorithm validation
  - Salt handling
  - Integration with database BINARY

- [ ] APIKey
  - Format validation
  - Strength checks
  - Integration with database VARCHAR

- [ ] EncryptedString
  - Encryption/decryption
  - Key management
  - Integration with database VARBINARY

- [ ] JWT
  - Token validation
  - Claims handling
  - Integration with database TEXT

### Measurement Types
- [ ] Length
  - Unit conversion
  - Range validation
  - Integration with database DECIMAL

- [ ] Weight
  - Unit conversion
  - Range validation
  - Integration with database DECIMAL

- [ ] Temperature
  - Unit conversion
  - Range validation
  - Integration with database DECIMAL

### File Types
- [ ] FilePath
  - Path validation
  - Security checks
  - Integration with database VARCHAR

- [ ] FileSize
  - Unit conversion
  - Range validation
  - Integration with database BIGINT

- [ ] FileType
  - MIME type validation
  - Extension validation
  - Integration with database VARCHAR

### Audit Types
- [ ] Timestamp
  - Automatic creation/update
  - Timezone handling
  - Integration with database TIMESTAMP

- [ ] UserReference
  - User tracking
  - Role validation
  - Integration with database references

- [ ] ChangeLog
  - Diff tracking
  - History management
  - Integration with database JSON

### Numeric Types
- [ ] Money
  - Multiple currency support
  - Exchange rate handling
  - Arithmetic operations
  - Integration with database DECIMAL

- [ ] Percentage
  - Range validation (0-100)
  - Decimal precision
  - Format options (with/without % symbol)
  - Integration with database DECIMAL

- [ ] Ratio
  - Fraction representation
  - Decimal conversion
  - Range validation
  - Integration with database DECIMAL

### Text Types
- [ ] Username
  - Format validation
  - Reserved word checking
  - Case sensitivity options
  - Integration with database VARCHAR

- [ ] Password
  - Strength validation
  - Hash generation
  - Policy enforcement
  - Integration with database VARCHAR

- [ ] Color
  - Multiple format support (HEX, RGB, HSL)
  - Name resolution
  - Validation
  - Integration with database VARCHAR

- [ ] Language
  - ISO 639-1/2/3 support
  - Name resolution
  - Validation
  - Integration with database CHAR

### Range Types
- [ ] DateRange
  - Start/end validation
  - Duration calculation
  - Overlap detection
  - Integration with database custom type

- [ ] NumberRange
  - Min/max validation
  - Overlap detection
  - Integration with database custom type

- [ ] TimeRange
  - Duration validation
  - Timezone support
  - Integration with database custom type

### Composite Types
- [ ] Name
  - First/Last/Middle parts
  - Cultural format support
  - Integration with database JSON

- [ ] PhoneWithRegion
  - Country code validation
  - Region-specific formats
  - Integration with database custom type

- [ ] MoneyWithCurrency
  - Amount and currency together
  - Exchange support
  - Integration with database custom type

### Media Types
- [ ] Image
  - Format validation
  - Dimension constraints
  - Size limits
  - Integration with database BLOB

- [ ] Audio
  - Format validation
  - Duration limits
  - Integration with database BLOB

- [ ] Video
  - Format validation
  - Duration/size limits
  - Integration with database BLOB

### Business Types
- [ ] TaxID
  - Country-specific formats
  - Checksum validation
  - Integration with database VARCHAR

- [ ] CompanyID
  - Country-specific formats
  - Validation rules
  - Integration with database VARCHAR

- [ ] InvoiceNumber
  - Format validation
  - Sequence management
  - Integration with database VARCHAR

### Configuration Types
- [ ] Setting
  - Type-safe values
  - Validation rules
  - Integration with database JSON

- [ ] Feature
  - Flag management
  - Dependency checking
  - Integration with database JSON

- [ ] Preference
  - User settings
  - Default values
  - Integration with database JSON

### Pricing Types
- [ ] TieredPrice (Base Class)
  - Common tiered pricing logic
  - Tier calculation methods
  - Validation rules
  - Integration with database JSON

- [ ] VolumeBasedTier
  - Quantity breakpoints
  - Per-unit pricing
  - Total price calculation
  - Minimum/maximum quantities
  - Integration with database JSON

- [ ] StairstepTier
  - Fixed price per tier
  - All-units pricing
  - Tier threshold handling
  - Integration with database JSON

- [ ] GraduatedTier
  - Different rates for each tier
  - Marginal pricing
  - Cumulative calculations
  - Integration with database JSON

- [ ] PackageTier
  - Fixed units per package
  - Partial package handling
  - Overflow pricing
  - Integration with database JSON

- [ ] SubscriptionTier
  - Usage-based tiers
  - Feature-based tiers
  - Hybrid tier models
  - Upgrade/downgrade rules
  - Integration with database JSON

- [ ] TimePeriodTier
  - Duration-based pricing
  - Commitment discounts
  - Auto-renewal rules
  - Integration with database JSON

- [ ] CustomerSegmentTier
  - Group-based pricing
  - Qualification rules
  - Tier migration logic
  - Integration with database JSON

- [ ] GeographicTier
  - Region-based pricing
  - Currency conversion
  - Tax handling
  - Integration with database JSON

- [ ] SeasonalTier
  - Time-based tier changes
  - Peak/off-peak pricing
  - Calendar-based rules
  - Integration with database JSON

### Tier Rule Types
- [ ] TierQualification
  - Eligibility criteria
  - Qualification period
  - Entry/exit rules
  - Integration with database JSON

- [ ] TierBenefit
  - Discount structures
  - Reward systems
  - Benefit stacking
  - Integration with database JSON

- [ ] TierTransition
  - Upgrade paths
  - Downgrade rules
  - Grace periods
  - Integration with database JSON

### Localization Types
- [ ] LocalizedString
  - Multiple language support
  - Fallback chains
  - Placeholder handling
  - Integration with database JSON

- [ ] LocalizedContent
  - Rich content localization
  - Media asset localization
  - Integration with database JSON

- [ ] TranslationKey
  - Namespace management
  - Version control
  - Context tracking
  - Integration with database VARCHAR

### Versioning Types
- [ ] Version
  - Semantic versioning
  - Build numbers
  - Release channels
  - Integration with database VARCHAR

- [ ] ChangeSet
  - Version differences
  - Migration paths
  - Rollback support
  - Integration with database JSON

- [ ] Revision
  - Sequential versioning
  - Branch handling
  - Merge tracking
  - Integration with database JSON

### Scheduling Types
- [ ] Schedule
  - Recurring patterns
  - Exception handling
  - Timezone support
  - Integration with database JSON

- [ ] TimeSlot
  - Duration management
  - Availability rules
  - Conflict detection
  - Integration with database JSON

- [ ] Calendar
  - Event management
  - Holiday handling
  - Working hours
  - Integration with database JSON

### Analytics Types
- [ ] Metric
  - Value validation
  - Unit handling
  - Aggregation rules
  - Integration with database JSON

- [ ] Dimension
  - Category management
  - Hierarchy support
  - Filter rules
  - Integration with database JSON

- [ ] TimeSeries
  - Data point validation
  - Interpolation rules
  - Aggregation periods
  - Integration with database JSON

### Workflow Types
- [ ] State
  - Transition rules
  - Validation logic
  - History tracking
  - Integration with database JSON

- [ ] Action
  - Permission checks
  - Validation rules
  - Effect tracking
  - Integration with database JSON

- [ ] Condition
  - Rule evaluation
  - Complex logic
  - Dynamic updates
  - Integration with database JSON

### Integration Types
- [ ] WebhookPayload
  - Schema validation
  - Retry logic
  - Security signing
  - Integration with database JSON

- [ ] APIResponse
  - Status codes
  - Error handling
  - Rate limiting
  - Integration with database JSON

- [ ] MessageQueue
  - Queue management
  - Delivery guarantees
  - Dead letter handling
  - Integration with database JSON

### Compliance Types
- [ ] Consent
  - Purpose tracking
  - Expiration handling
  - Audit trail
  - Integration with database JSON

- [ ] DataRetention
  - Policy enforcement
  - Deletion rules
  - Archive handling
  - Integration with database JSON

- [ ] AccessControl
  - Permission rules
  - Role inheritance
  - Scope management
  - Integration with database JSON

### Machine Learning Types
- [ ] Feature
  - Value normalization
  - Missing data handling
  - Type conversion
  - Integration with database JSON

- [ ] Model
  - Version management
  - Parameter storage
  - Performance metrics
  - Integration with database JSON

- [ ] Prediction
  - Confidence scores
  - Explanation data
  - Feedback loop
  - Integration with database JSON

### Health Check Types
- [ ] ServiceStatus
  - Health indicators
  - Dependency status
  - Performance metrics
  - Integration with database JSON

- [ ] Alert
  - Severity levels
  - Notification rules
  - Resolution tracking
  - Integration with database JSON

- [ ] Incident
  - Impact assessment
  - Resolution steps
  - Post-mortem data
  - Integration with database JSON

### E-Commerce Types

- [ ] Product
  - SKU handling
  - Variant management
  - Stock tracking
  - Digital/Physical flags
  - Integration with database JSON

- [ ] Inventory
  - Stock levels
  - Reserved quantities
  - Reorder points
  - Location tracking
  - Integration with database JSON

- [ ] Cart
  - Item management
  - Quantity validation
  - Price calculation
  - Session handling
  - Integration with database JSON

- [ ] Order
  - Status workflow
  - Payment tracking
  - Fulfillment status
  - Return handling
  - Integration with database JSON

- [ ] Shipping
  - Carrier integration
  - Rate calculation
  - Tracking numbers
  - Delivery estimates
  - Integration with database JSON

- [ ] PaymentMethod
  - Provider integration
  - Security tokens
  - Validation rules
  - Expiry handling
  - Integration with database JSON

- [ ] Promotion
  - Coupon codes
  - Discount rules
  - Usage limits
  - Stacking rules
  - Integration with database JSON

- [ ] CustomerSegment
  - Membership rules
  - Benefit tracking
  - Tier management
  - Integration with database JSON

- [ ] ProductCategory
  - Hierarchy management
  - Attribute inheritance
  - Navigation structure
  - Integration with database JSON

- [ ] ProductAttribute
  - Value validation
  - Unit conversion
  - Search indexing
  - Integration with database JSON

- [ ] Review
  - Rating validation
  - Moderation status
  - Helpful votes
  - Integration with database JSON

- [ ] Wishlist
  - Item management
  - Sharing rules
  - Privacy settings
  - Integration with database JSON

- [ ] Bundle
  - Component products
  - Pricing rules
  - Stock validation
  - Integration with database JSON

- [ ] GiftCard
  - Balance tracking
  - Redemption rules
  - Expiry handling
  - Integration with database JSON

- [ ] ReturnRequest
  - Reason tracking
  - Status workflow
  - Refund handling
  - Integration with database JSON

- [ ] Subscription
  - Billing cycle
  - Delivery schedule
  - Pause/Resume rules
  - Integration with database JSON

- [ ] LoyaltyPoints
  - Earning rules
  - Redemption rules
  - Expiry tracking
  - Integration with database JSON

- [ ] AbandonedCart
  - Recovery rules
  - Reminder schedule
  - Incentive tracking
  - Integration with database JSON

- [ ] DigitalDownload
  - Access control
  - Expiry rules
  - Download limits
  - Integration with database JSON

- [ ] StoreLocation
  - Inventory levels
  - Operating hours
  - Service capabilities
  - Integration with database JSON

## Implementation Notes
1. All types should:
   - Extend appropriate oatpp base types
   - Implement MariaDBTypeWrapper interface
   - Provide database-specific mappings
   - Include comprehensive tests

2. Consider:
   - Performance implications
   - Memory usage
   - Database compatibility
   - Type conversion rules
   - Internationalization support
   - Validation customization
   - Default values
   - Null handling

3. Documentation:
   - Clear usage examples
   - Integration patterns
   - Performance guidelines
   - Migration guides
   - Best practices
   - Common pitfalls