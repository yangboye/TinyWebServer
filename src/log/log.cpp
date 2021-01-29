//
// Created by yangb on 2021/1/25.
//

#include <cstring>
#include <stdarg.h>
#include "log.h"

namespace tiny {

Log::Log()
    : count_(0),
      is_async_(false) {
}

Log::~Log() {
  if (fp_ != NULL) {
    fclose(fp_);
  }
  delete log_queue_;
  delete[] buf_;
}

// 异步需要设置阻塞队列的长度, 同步不需要设置
bool Log::Init(const char* file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size) {
  // 如果设置了max_queue_size的值，则设置为异步
  if (max_queue_size >= 1) {
    is_async_ = true;
    log_queue_ = new BlockQueue<std::string>(max_queue_size);
    pthread_t tid;
    pthread_create(&tid, NULL, FlushLogThread, NULL);
  }

  close_log_ = close_log;
  log_buf_size_ = log_buf_size;
  buf_ = new char[log_buf_size];
  memset(buf_, '\0', log_buf_size);
  split_lines_ = split_lines;

  time_t t = time(NULL);
  struct tm* sys_tm = localtime(&t);
  struct tm my_tm = *sys_tm;

  const char* p = strrchr(file_name, '/');
  char log_full_name[256] = {0};

  if (NULL == p) {
    snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
  } else {
    strcpy(log_name_, p + 1);
    strncpy(dir_name_, file_name, p - file_name + 1);
    snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name_, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
             log_name_);
  }

  today_ = my_tm.tm_mday;

  fp_ = fopen(log_full_name, "a");
  if (fp_ == NULL) {
    return false;
  }
  return true;
}

void Log::WriteLog(int level, const char* format, ...) {
  struct timeval now = {0, 0};
  gettimeofday(&now, NULL);
  time_t t = now.tv_sec;
  struct tm* sys_tm = localtime(&t);
  struct tm my_tm = *sys_tm;
  char s[16] = {0};

  switch (level) {
    case 0: {
      strcpy(s, "[debug]:");
      break;
    }
    case 1: {
      strcpy(s, "[info]:");
      break;
    }
    case 2: {
      strcpy(s, "[warn]:");
      break;
    }
    case 3: {
      strcpy(s, "[error]:");
      break;
    }
    default: {
      strcpy(s, "[info]:");
      break;
    }
  } // switch

  // 写入一个log
  mutex_.Lock();
  ++count_;

  if (today_ != my_tm.tm_mday || count_ % split_lines_ == 0) { // everyday log
    char new_log[256] = {0};
    fflush(fp_);
    fclose(fp_);
    char tail[16] = {0};

    snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

    if (today_ != my_tm.tm_mday) {
      snprintf(new_log, 255, "%s%s%s", dir_name_, tail, log_name_);
      today_ = my_tm.tm_mday;
      count_ = 0;
    } else {
      snprintf(new_log, 255, "%s%s%s.%ld", dir_name_, tail, log_name_, count_ / split_lines_);
    }
    fp_ = fopen(new_log, "a");
  }

  mutex_.Unlock();

  va_list valst;
  va_start(valst, format);

  std::string log_str;
  mutex_.Lock();

  // 写入的具体时间内容格式
  int n = snprintf(buf_, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                   my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                   my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);

  int m = vsnprintf(buf_ + n, log_buf_size_ - 1, format, valst);
  buf_[n + m] = '\n';
  buf_[n + m + 1] = '\0';
  log_str = buf_;

  mutex_.Unlock();

  if (is_async_ && !log_queue_->IsFull()) {
    log_queue_->Push(log_str);
  } else {
    mutex_.Lock();
    fputs(log_str.c_str(), fp_);
    mutex_.Unlock();
  }

  va_end(valst);
}

void Log::Flush() {
  mutex_.Lock();
  fflush(fp_);  // 强制刷新写入流缓冲区
  mutex_.Unlock();
}

} // namespace tiny



