DROP TABLE IF EXISTS `t_user`;
CREATE TABLE `t_user` (
  `user_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '用户id',
  `number` varchar(20)   NOT NULL COMMENT '手机号',
  `nickname` varchar(20)   NOT NULL COMMENT '昵称',
  `password` varchar(20)   NOT NULL COMMENT '密码',
  `state` tinyint(1) unsigned NOT NULL DEFAULT 0 COMMENT '状态',
  `ip` varchar(20) NOT NULL COMMENT '账号ip',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `uk_number` (`number`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='账户';


DROP TABLE IF EXISTS `t_friend`;
CREATE TABLE `t_friend` (
  `user_id` bigint(20) unsigned NOT NULL COMMENT '账户id',
  `friend_id` bigint(20) unsigned NOT NULL COMMENT '好友账户id',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  UNIQUE KEY `uk_user_friend` (`user_id`,`friend_id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='好友关系';

DROP TABLE IF EXISTS `t_group`;
CREATE TABLE `t_group` (
  `group_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '群组id',
  `name` varchar(20)  NOT NULL COMMENT '组名',
  `user_id` bigint(20) unsigned NOT NULL COMMENT '群主', 
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`group_id`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='群组';

DROP TABLE IF EXISTS `t_group_user`;
CREATE TABLE `t_group_user` (
  `group_id` bigint(20) unsigned NOT NULL COMMENT '组id',
  `user_id` bigint(20) unsigned NOT NULL COMMENT '用户id',
  UNIQUE KEY `uk_group_user` (`group_id`,`user_id`) USING BTREE,
  KEY `idx_user_id` (`user_id`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='群组成员关系';

DROP TABLE IF EXISTS `t_send_message`;
CREATE TABLE `t_send_message` (
  `message_id` bigint(20) unsigned NOT NULL COMMENT '消息id',
  `from_id` bigint(20) unsigned NOT NULL COMMENT '用户id',
  `type` tinyint(3) NOT NULL COMMENT '接收者类型,1:个人；2：群组',
  `to_id` bigint(20) unsigned NOT NULL COMMENT '接收者id,如果是单聊信息，则为user_id，如果是群组消息，则为group_id',
  `group_id` bigint(20) unsigned COMMENT 'group_id',
  `content` text  NOT NULL COMMENT '内容',
  `send_time` datetime(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3) COMMENT '消息发送时间',
  UNIQUE KEY `idx_from_id_message_id` (`from_id`,`message_id`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='发送消息';

DROP TABLE IF EXISTS `t_push_message`;
CREATE TABLE `t_push_message` (
  `message_id` bigint(20) unsigned NOT NULL COMMENT '消息id',
  `from_id` bigint(20) unsigned NOT NULL COMMENT '用户id',
  `type` tinyint(3) NOT NULL COMMENT '接收者类型,1:个人；2：群组',
  `to_id` bigint(20) unsigned NOT NULL COMMENT '接收者id,如果是单聊信息，则为user_id，如果是群组消息，则为group_id',
  `group_id` bigint(20) unsigned COMMENT 'group_id',
  `content` text  NOT NULL COMMENT '内容',
  `send_time` datetime(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3) COMMENT '消息发送时间',
  `state` tinyint(1) unsigned NOT NULL DEFAULT 0 COMMENT '是否已接收到',
  UNIQUE KEY `idx_to_id_message_id` (`to_id`,`message_id`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='推送消息';