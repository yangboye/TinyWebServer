//
// Created by yangb on 2021/1/25.
//

#ifndef TINYWEBSERVER_SRC_LOG_LOG_H
#define TINYWEBSERVER_SRC_LOG_LOG_H

#include <cstdint>
#include <iostream>
#include <string>

#include "block_queue.h"

namespace tiny {

class Log {
 public:
  inline static Log* GetInstance() {
    static Log instance;
    return &instance;
  }

  inline static void* FlushLogThread(void* args) {
    Log::GetInstance()->AsyncWriteLog();
  }

  bool Init(const char* file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000,
            int max_queue_size = 0);

  void WriteLog(int level, const char* format, ...);

  void Flush();

 private:
  Log();

  virtual ~Log();

  inline void* AsyncWriteLog() {
    std::string single_log;
    // 从阻塞队列中取出一个日志string, 写入文件
    while (log_queue_->Pop(single_log)) {
      mutex_.Lock();
      fputs(single_log.c_str(), fp_);
      mutex_.Unlock();
    }
  }

 private:
  static const int kMaxLength = 128;

 private:
  char dir_name_[kMaxLength];         // 路径名
  char log_name_[kMaxLength];         // log文件名
  int32_t split_lines_;               // 日志最大行数
  int32_t log_buf_size_;              // 日志缓冲区大小
  int64_t count_;                     // 日志行数记录
  int32_t today_;                     // 按天分类，记录当前时间是哪一天
  FILE* fp_;                          // 打开log的文件指针
  char* buf_;
  BlockQueue<std::string>* log_queue_; // 阻塞队列
  bool is_async_;                     // 是否同步标志位
  Locker mutex_;                      // 互斥锁
  int32_t close_log_;                 // 关闭日志

};

} // namespace tiny

#endif //TINYWEBSERVER_SRC_LOG_LOG_H
