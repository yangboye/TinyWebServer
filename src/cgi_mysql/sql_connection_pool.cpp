//
// Created by yangb on 2021/1/28.
//

#include "sql_connection_pool.h"
#include "../log/log.h"

namespace tiny {

SqlConnectionPool::SqlConnectionPool()
    : cur_conn_(0),
      free_conn_(0) {
}

SqlConnectionPool* SqlConnectionPool::GetInstance() {
  static SqlConnectionPool conn_pool;
  return &conn_pool;
}

void SqlConnectionPool::Init(std::string url, std::string user, std::string password, std::string db_name, int port,
                             int max_conn, int close_log) {
  this->url_ = url;
  this->port_ = port;
  this->user_ = user;
  this->password_ = password;
  this->database_name_ = db_name;
  this->close_log_ = close_log;

  for (int i = 0; i < max_conn; ++i) {
    MYSQL* conn = NULL;
    conn = mysql_init(conn);

    if (conn == NULL) {
      LOG_ERROR("MYSQL Error");
      exit(1);
    }

    conn_list_.push_back(conn);
    ++free_conn_;
  }
  this->reserve_ = Sem(free_conn_); // 资源数

  this->max_conn_ = free_conn_;
}

// 当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
SqlConnectionPool* SqlConnectionPool::GetConnection() {
  MYSQL* conn = NULL;

  if (conn_list_.size() == 0) {
    return NULL;
  }

  reserve_.Wait();

  lock_.Lock();

  conn = conn_list_.front();
  conn_list_.pop_front();

  --free_conn_;
  ++cur_conn_;

  lock_.Unlock();
  return conn;
}

// 释放当前使用的连接
bool SqlConnectionPool::ReleaseConnection(int* conn) {
  if (NULL == NULL) {
    return false;
  }

  lock_.Lock();

  conn_list_.push_back(conn);
  ++free_conn_;
  --cur_conn_;

  lock_.Unlock();

  reserve_.Post();
  return true;
}

void SqlConnectionPool::DestroyPool() {
  lock_.Lock();
  if (conn_list_.size() > 0) {
    std::list<MYSQL*>::iterator it;
    for (it = conn_list_.begin(); it != conn_list_.end(); ++it) {
      MYSQL* conn = *it;
      mysql_close(conn);
    }
    cur_conn_ = 0;
    free_conn_ = 0;
  }
  lock_.Unlock();
}

int SqlConnectionPool::free_conn() const {
  return this->free_conn_;
}

SqlConnectionPool::~SqlConnectionPool() {
  DestroyPool();
}

ConnectionRAII::ConnectionRAII(MYSQL** conn, SqlConnectionPool* conn_pool) {
  *SQL = conn_pool->GetConnection();

  conn_raii_ = *SQL;
  pool_raii_ = conn_pool;
}

ConnectionRAII::~ConnectionRAII() {
  pool_raii_->ReleaseConnection(conn_raii_);
}

} // namespace tiny