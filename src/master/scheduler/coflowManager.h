//
// Created by he on 1/23/18.
//

#ifndef MASTER_COFLOWMANAGER_H
#define MASTER_COFLOWMANAGER_H

#include "../datastructures/coflow.h"

class coflowManager {
public:
  coflowManager();

private:
  vector<Coflow *> *mUnregisterCoflows;
  vector<Coflow *> *mRunningCoflows;
  vector<Coflow *> *mFinishedCoflows;
  vector<Coflow *> *mNotAdmittedCoflows;
};

#endif // MASTER_COFLOWMANAGER_H
