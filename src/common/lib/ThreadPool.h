//
// Created by he on 4/20/18.
//

#ifndef COFLOWSIM_THREADPOOL_H
#define COFLOWSIM_THREADPOOL_H
#include "locker.h"
#include "spdlog/spdlog.h"
#include <cstdio>
#include <exception>
#include <list>
#include <vector>
#include <pthread.h>

class ThreadClass;

class ThreadPool {
public:
  explicit ThreadPool(int max_thread_num = 8, int max_task_num = 10000);
  ~ThreadPool();
  bool append(ThreadClass *task);

private:
  static void *worker(void *arg);
  void run();
  bool increase_a_thread();

private:
  int max_thread_num_;    // 最大线程数量，若为零，则表示不限制线程的数量
  int max_task_num_;
//  pthread_t *threads_;
  std::vector<pthread_t> threads_;
  int created_thread_num_;
  locker threads_locker_;
  sem free_thread_stat_;

  std::list<ThreadClass *> task_queue_;
  locker task_queue_locker_;
  sem task_queue_stat_;
  bool stop_;
};


class ThreadClass {
public:
  ThreadClass(){
    run_stop = false;
  }
  void stop(){
    run_stop = true;
  }
  virtual void run() = 0;
protected:
  bool run_stop;
};
#endif //COFLOWSIM_THREADPOOL_H
