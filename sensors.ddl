CREATE TABLE `sensors` (
  `sn_id` int NOT NULL AUTO_INCREMENT,
  `sn_datetime` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `sn_co2` double NOT NULL DEFAULT '0',
  `sn_tvoc` double NOT NULL DEFAULT '0',
  `sn_h2` double NOT NULL DEFAULT '0',
  `sn_eth` double NOT NULL DEFAULT '0',
  `sn_no2` double NOT NULL DEFAULT '0',
  `sn_nh3` double NOT NULL DEFAULT '0',
  `sn_co` double NOT NULL DEFAULT '0',
  PRIMARY KEY (`sn_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8_unicode_ci