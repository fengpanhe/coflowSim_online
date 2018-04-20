//
// Created by he on 4/20/18.
//

#include "ThreadPool.h"

ThreadPool::ThreadPool(int max_thread_num, int max_task_num){
  this->max_thread_num_ = max_thread_num;
  this->max_task_num_ = max_task_num;
  this->stop_ = false;
  created_thread_num_ = 0;
  if ((this->max_thread_num_ < 0) || (this->max_task_num_ <= 0)) {
    throw std::exception();
  }

//  threads_ = new pthread_t[this->max_thread_num_];
//  if (!threads_) {
//    throw std::exception();
//  }
//
//  for (int i = 0; i < this->max_thread_num_; ++i) {
//    if (pthread_create(threads_ + i, nullptr, worker, this) != 0) {
//      delete[] threads_;
//      throw std::exception();
//    }
//    if (pthread_detach(threads_[i])) {
//      delete[] threads_;
//      throw std::exception();
//    }
//  }
}

ThreadPool::~ThreadPool() {
  stop_ = true;
}

bool ThreadPool::append(ThreadClass *task) {
  task_queue_locker_.lock();
  if (task_queue_.size() > max_task_num_) {
    printf("warning: task_queue_.size(%ld) > max_task_num_(%d)", task_queue_.size(), max_task_num_);
    task_queue_locker_.unlock();
    return false;
  }
  task_queue_.push_back(task);
  task_queue_locker_.unlock();

  // 已经没有空闲线程，则试图去创建新线程。
  while (!free_thread_stat_.tryWait()){
    //如果已达到最大线程数量，则放弃创建新线程。
    if (max_thread_num_ > 0 && created_thread_num_ >= max_thread_num_){
      break;
    }
    this->increase_a_thread();
  }
  task_queue_stat_.post();
  return true;
}

void *ThreadPool::worker(void *arg) {
  auto *pool = (ThreadPool *)arg;
  pool->run();
  return pool;
}

void ThreadPool::run() {
  while (!stop_) {
    free_thread_stat_.post();
    task_queue_stat_.wait();

    task_queue_locker_.lock();
    if (task_queue_.empty()) {
      task_queue_locker_.unlock();
      continue;
    }
    ThreadClass *task = task_queue_.front();
    task_queue_.pop_front();
    task_queue_locker_.unlock();
    if (!task) {
      continue;
    }
    task->run();
  }
}

bool ThreadPool::increase_a_thread(){
  pthread_t thread;
  if (pthread_create(&thread, nullptr, worker, this) != 0) {
    throw std::exception();
  }
  if (pthread_detach(thread)) {
    throw std::exception();
  }
  threads_locker_.lock();
  threads_.push_back(thread);
  created_thread_num_++;
  threads_locker_.unlock();
}