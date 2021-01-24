//
// Created by yangb on 2021/1/24.
//

#ifndef TINYWEBSERVER_LOCK_LOCKER_H
#define TINYWEBSERVER_LOCK_LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

class Sem {
 public:
  Sem() {
    if (0 != sem_init(&sem_, 0, 0)) {
      throw std::exception();
    }
  }

  explicit Sem(int num) {
    if (0 != sem_init(&sem_, 0, num)) {
      throw std::exception();
    }
  }

  ~Sem() {
    sem_destroy(&sem_);
  }

  bool wait() {
    return (0 == sem_wait(&sem_));
  }

  bool post() {
    return (0 == sem_post(&sem_));
  }

 private:
  sem_t sem_;
};

class Locker {
 public:
  Locker() {
    if (0 != pthread_mutex_init(&mutex_, NULL)) {
      throw std::exception();
    }
  }

  ~Locker() {
    pthread_mutex_destroy(&mutex_);
  }

  bool Lock() {
    return pthread_mutex_lock(&mutex_) == 0;
  }

  bool Unlock() {
    return pthread_mutex_unlock(&mutex_) == 0;
  }

  pthread_mutex_t *Get() {
    return &mutex_;
  }

 private:
  pthread_mutex_t mutex_;
};

class Cond {
 public:
  Cond() {
    if (pthread_cond_init(&cond_, NULL) != 0) {
      throw std::exception();
    }
  }

  ~Cond() {
    pthread_cond_destroy(&cond_);
  }

  bool Wait(pthread_mutex_t *mutex) {
    return pthread_cond_wait(&cond_, mutex) == 0;
  }

  bool TimeWait(pthread_mutex_t *mutex, struct timespec t) {
    return pthread_cond_timedwait(&cond_, mutex, &t) == 0;
  }

  bool Signal() {
    return pthread_cond_signal(&cond_) == 0;
  }

  bool Broadcast() {
    return pthread_cond_broadcast(&cond_) == 0;
  }

 private:
  pthread_cond_t cond_;
};

#endif //TINYWEBSERVER_LOCK_LOCKER_H
