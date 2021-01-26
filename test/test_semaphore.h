//
// Created by yangb on 2021/1/24.
// @brief: 信号量`semaphore.h`的使用

#ifndef TINYWEBSERVER_TEST_TEST_SEMAPHORE_H
#define TINYWEBSERVER_TEST_TEST_SEMAPHORE_H

#include <cstdio>
#include "../src/lock/locker.h"

namespace tiny {
namespace test {

int number; // 被保护的全局变量
Sem sem_id(1);

static void* Func1(void* arg) {
  sem_id.Wait();
  printf("Func1 gets the semaphore.\n");
  number++;
  printf("Func1: number = %d.\n", number);

  sem_id.Post();

  return NULL;
}

static void* Func2(void* arg) {
  sem_id.Wait();
  printf("Func2 gets the semaphore.\n");
  number--;
  printf("Func2: number = %d.\n", number);

  sem_id.Post();

  return NULL;
}

static void Test() {
  number = 1;
  pthread_t id1, id2;

  pthread_create(&id1, NULL, Func1, NULL);
  pthread_create(&id2, NULL, Func2, NULL);

  pthread_join(id1, NULL);
  pthread_join(id2, NULL);

}

} // namespace test
} // namespace tiny

#endif //TINYWEBSERVER_TEST_TEST_SEMAPHORE_H
