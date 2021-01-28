#include <iostream>
#include <unistd.h>

#include "test/test_semaphore.h"
#include "src/log/block_queue.h"
#include "src/cgi_mysql/sql_connection_pool.h"

using std::cout;
using std::endl;

class A {
 public:
  ~A() {
    cout << __FUNCTION__ << endl;
  }
};

int main() {
//  tiny::test::Test();
  auto tmp = new A();
  delete tmp;
  return 0;

}