//
// Created by yangb on 2021/1/25.
// @brief: 循环数组实现的阻塞队列, back_ = (back_ + 1) % max_size_;
//         线程安全，每个操作前都要先加互斥锁, 操作完后再解锁

#ifndef TINYWEBSERVER_SRC_LOG_BLOCK_QUEUE_H
#define TINYWEBSERVER_SRC_LOG_BLOCK_QUEUE_H

#include <cstdlib>
#include <sys/time.h>
#include "../lock/locker.h"

namespace tiny {

template <typename T>
class BlockQueue {
 public:
  explicit BlockQueue(int max_size = 1000);

  ~BlockQueue();

  // @brief 清空队列
  void Clear();

  // @brief 判断队列是否满了
  bool IsFull();

  // @brief 判断队列是否空了
  bool IsEmpty();

  // @brief 返回队首元素
  bool Front(T& value);

  // @brief 返回队尾元素
  bool Back(T& value);

  // @brief 向队列中添加元素
  bool Push(const T& item);

  // @brief 获取队首元素
  bool Pop(T& item);

  // @brief 获取队首元素, 增加了超时处理
  bool Pop(T& item, int ms_timeout);

  // @brief 返回当前队列中元素的个数
  int size();

  // @brief 返回队列的最大长度
  int max_size();

 private:
  Locker mutex_;
  Cond cond_;

  T* array_;
  int size_;
  int max_size_;
  int front_;
  int back_;
};

template <typename T>
inline BlockQueue<T>::BlockQueue(int max_size) {
  if (max_size <= 0) {
    exit(-1);
  }

  max_size_ = max_size;
  array_ = new T[max_size];
  size_ = 0;
  front_ = -1;
  back_ = -1;
}

template <typename T>
inline BlockQueue<T>::~BlockQueue() {
  {
    mutex_.Lock();
    if (array_ != NULL) {
      delete[] array_;
    }
    mutex_.Unlock();
  }
}

template <typename T>
inline void BlockQueue<T>::Clear() {
  mutex_.Lock();
  size_ = 0;
  front_ = -1;
  back_ = -1;
  mutex_.Unlock();
}

template <typename T>
inline bool BlockQueue<T>::IsFull() {
  mutex_.Lock();
  if (size_ >= max_size_) {
    mutex_.Unlock();
    return true;
  }
  mutex_.Unlock();
  return false;
}

template <typename T>
inline bool BlockQueue<T>::IsEmpty() {
  mutex_.Lock();
  if (0 == size_) {
    mutex_.Unlock();
    return true;
  }
  mutex_.Unlock();
  return false;
}

template <typename T>
inline bool BlockQueue<T>::Front(T& value) {
  mutex_.Lock();
  if (0 == size_) {
    mutex_.Unlock();
    return false;
  }
  value = array_[front_];
  mutex_.Unlock();
  return true;
}

template <typename T>
inline bool BlockQueue<T>::Back(T& value) {
  mutex_.Lock();
  if (0 == size_) {
    mutex_.Unlock();
    return false;
  }
  value = array_[back_];
  mutex_.Unlock();
  return true;
}

template <typename T>
inline int BlockQueue<T>::size() {
  int tmp = 0;

  mutex_.Lock();
  tmp = size_;
  mutex_.Unlock();

  return tmp;
}

template <typename T>
inline int BlockQueue<T>::max_size() {
  int tmp = 0;

  mutex_.Lock();
  tmp = max_size_;
  mutex_.Unlock();

  return tmp;
}

template <typename T>
bool BlockQueue<T>::Push(const T& item) {
  mutex_.Lock();
  if (size_ >= max_size_) {  // 队列满了
    cond_.Broadcast();
    mutex_.Unlock();
    return false;
  }

  back_ = (back_ + 1) % max_size_;
  array_[back_] = item;

  ++size_;

  cond_.Broadcast();
  mutex_.Unlock();
  return true;
}

template <typename T>
bool BlockQueue<T>::Pop(T& item) {
  mutex_.Lock();
  while (size_ <= 0) {
    if (!cond_.Wait(mutex_.Get())) {
      mutex_.Unlock();
      return false;
    }
  }
  front_ = (front_ + 1) % max_size_;
  item = array_[front_];
  --size_;
  mutex_.Unlock();
  return true;
}

template <typename T>
bool BlockQueue<T>::Pop(T& item, int ms_timeout) {
  struct timespec t = {0, 0};
  struct timeval now = {0, 0};
  gettimeofday(&now, NULL);

  mutex_.Lock();

  if (size_ <= 0) {
    t.tv_sec = now.tv_sec + ms_timeout / 1000;
    t.tv_nsec = (ms_timeout % 1000) * 1000; // TODO: 存疑
    if (!cond_.TimeWait(mutex_.Get(), t)) {
      mutex_.Unlock();
      return false;
    }
  }

  if (size_ <= 0) {
    mutex_.Unlock();
    return false;
  }

  front_ = (front_ + 1) % max_size_;
  item = array_[front_];
  --size_;
  mutex_.Unlock();
  return true;
}

} // namespace tiny

#endif //TINYWEBSERVER_SRC_LOG_BLOCK_QUEUE_H
