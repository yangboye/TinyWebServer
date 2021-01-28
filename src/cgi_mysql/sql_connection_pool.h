//
// Created by yangb on 2021/1/28.
//

#ifndef TINYWEBSERVER_SRC_CGI_MYSQL_SQL_CONNECTION_POOL_H
#define TINYWEBSERVER_SRC_CGI_MYSQL_SQL_CONNECTION_POOL_H

#include <list>
#include <string>
#include <mysql/mysql.h>
#include "../lock/locker.h"

namespace tiny {

class SqlConnectionPool {
 public:
  MYSQL* GetConnection();               // 获取数据库连接
  bool ReleaseConnection(MYSQL* conn);  // 释放连接
  int free_conn() const;                      // 获取连接数
  void DestroyPool();                   // 销毁所有的连接

  // 单例模式
  static SqlConnectionPool* GetInstance();

  void Init(std::string url, std::string user, std::string password, std::string db_name, int port, int max_conn, int close_log);

 private:
  SqlConnectionPool();

  ~SqlConnectionPool();

 private:
  int max_conn_;                  // 最大连接数
  int cur_conn_;                  // 当前已使用的连接数
  int free_conn_;                 // 当前空闲的连接数
  Locker lock_;
  std::list<MYSQL*> conn_list_;  // 连接池
  Sem reserve_;

 private:
  std::string url_;           // 主机地址
  std::string port_;          // 数据库端口号
  std::string user_;          // 登录数据库的用户名
  std::string password_;      // 登陆数据库的密码
  std::string database_name_; // 数据库名
  int close_log_;             // 日志开关
};

class ConnectionRAII {
 public:
  ConnectionRAII(MYSQL** conn, SqlConnectionPool* conn_pool);

  ~ConnectionRAII();

 private:
  MYSQL* conn_raii_;
  SqlConnectionPool* pool_raii_;
};

} // namespace tiny

#endif //TINYWEBSERVER_SRC_CGI_MYSQL_SQL_CONNECTION_POOL_H
