//
// Created by he on 1/22/18.
//

#ifndef MASTER_THREAD_H
#define MASTER_THREAD_H

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

#endif // MASTER_THREAD_H
