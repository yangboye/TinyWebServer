//
// Created by yangb on 2021/1/29.
//

#ifndef TINYWEBSERVER_SRC_THREADPOOL_THREADPOOL_H
#define TINYWEBSERVER_SRC_THREADPOOL_THREADPOOL_H

#include <cstdint>
#include <thread>
#include <list>
#include "../lock/locker.h"
#include "../cgi_mysql/sql_connection_pool.h"

namespace tiny {

template <typename T>
class ThreadPool {
 public:
  ThreadPool(int actor_model, SqlConnectionPool* conn_pool, int32_t thread_number = 8, int max_request = 10000);

  ~ThreadPool();

  bool Append(T* request, int32_t state);

  bool AppendP(T* request);

 private:
  // 工作线程运行的函数，它不断从工作队列中取出任务并执行
  static void* Worker(void* arg);

  void Run();

 private:
  int32_t thread_number_;         // 线程池中的线程数
  int32_t max_requests_;           // 请求队列中允许的最大请求数
  pthread_t* threads_;            // 描述线程池的数组，其大小为thread_number_
  std::list<T*> work_queue_;      // 请求队列
  Locker queue_locker_;           // 保护请求队列的互斥锁
  Sem queue_stat_;                // 是否有任务需要处理
  SqlConnectionPool* conn_pool_;  // 数据库
  int32_t actor_model_;           // 模型切换
};

template <typename T>
ThreadPool<T>::ThreadPool(int actor_model,
                          SqlConnectionPool* conn_pool,
                          int32_t thread_number,
                          int max_request)
    : actor_model_(actor_model),
      thread_number_(thread_number),
      max_requests_(max_request) {
  if (thread_number <= 0 || max_request <= 0) {
    throw std::exception();
  }

  threads_ = new pthread_t[thread_number];
  if (!threads_) {
    throw std::exception();
  }

  for (int i = 0; i < thread_number; ++i) {
    if (pthread_create(threads_ + i, NULL, Worker, this) != 0) {
      delete[] threads_;
      throw std::exception();
    }
    if (pthread_detach(threads_[i])) {
      delete[] threads_;
      throw std::exception();
    }
  }
}

template <typename T>
ThreadPool<T>::~ThreadPool() {
  delete[] threads_;
}

template <typename T>
bool ThreadPool<T>::Append(T* request, int32_t state) {
  queue_locker_.Lock();
  if (work_queue_.size() >= max_requests_) {
    queue_locker_.Unlock();
    return false;
  }
  request->state_ = state;
  work_queue_.push_back(request);
  queue_locker_.Unlock();
  queue_stat_.Post();
}

template <typename T>
bool ThreadPool<T>::AppendP(T* request) {
  queue_locker_.Lock();
  if (work_queue_.size() >= max_requests_) {
    queue_locker_.Unlock();
    return false;
  }
  work_queue_.push_back(request);
  queue_locker_.Unlock();
  queue_stat_.Post();
  return true;
}

template <typename T>
void* ThreadPool<T>::Worker(void* arg) {
  ThreadPool* pool = (ThreadPool*) arg;
  pool->Run();
  return pool;
}

template <typename T>
void ThreadPool<T>::Run() {
  while (true) {
    queue_stat_.Wait();
    queue_locker_.Lock();
    if(work_queue_.empty()) {
      queue_locker_.Unlock();
      continue;
    }
    T* request = work_queue_.front();
    work_queue_.pop_front();
    queue_locker_.Unlock();
    if(!request) {
      continue;
    }
    if(actor_model_ == 1) {
      if(request->state_) {
        if(request->read_once()) {
          request->improv = 1;
        }
      }
    }
  }
}

} // namespace tiny

#endif //TINYWEBSERVER_SRC_THREADPOOL_THREADPOOL_H
