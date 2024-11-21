CREATE TABLE IF NOT EXISTS `test_numerics` (
 `f_number` INTEGER,
 `f_decimal` DOUBLE PRECISION,
 `f_number_unchar` INTEGER,
 `f_date` DATE,
 `f_datetime` DATETIME,
 `f_string` VARCHAR(255)
) ENGINE=InnoDB;

INSERT INTO test_numerics
(f_number, f_decimal, f_number_unchar, f_date, f_datetime, f_string) VALUES (null, null, null, null, null, null);

INSERT INTO test_numerics
(f_number, f_decimal, f_number_unchar, f_date, f_datetime, f_string) VALUES (0, 0, 0, '2020-09-03', '2020-09-03 23:59:59', 'hello');

INSERT INTO test_numerics
(f_number, f_decimal, f_number_unchar, f_date, f_datetime, f_string) VALUES (1, 1, 1, '2020-09-03', '2020-09-03 23:59:59', 'world');

INSERT INTO test_numerics
(f_number, f_decimal, f_number_unchar, f_date, f_datetime, f_string) VALUES (1, 3.14, 1, '2020-09-03', '2020-09-03 23:59:59', 'foo');
