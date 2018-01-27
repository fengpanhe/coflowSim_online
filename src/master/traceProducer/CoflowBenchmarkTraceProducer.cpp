//
// Created by he on 1/21/18.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <utility>
#include "CoflowBenchmarkTraceProducer.h"

CoflowBenchmarkTraceProducer::CoflowBenchmarkTraceProducer(string coflowBenchFilePath, string machineDefineFilePath) {
    this->coflowBenchFilePath = std::move(coflowBenchFilePath);
    this->machineDefineFilePath = std::move(machineDefineFilePath);
    this->coflowNum = 0;
    this->logicMachinePortNum = 0;
}

bool CoflowBenchmarkTraceProducer::prepareCoflows(vector<Coflow*>* & coflows) {
    fstream CBfin;
    CBfin.open(this->coflowBenchFilePath);

    int coflowNum;

    CBfin >> this->logicMachinePortNum;
    CBfin >> coflowNum;

    char buffer[4096];
    CBfin.getline(buffer, 4096, '\n');
    int num;
    int i = 0;

    while(!CBfin.eof() && i < coflowNum){
        CBfin.getline(buffer, 4096, '\n');
        coflows->push_back(prepareOneCoflow(buffer));
        i++;
    }
    CBfin.close();
    return true;
}

vector<Machine*>* CoflowBenchmarkTraceProducer::prepareMachines() {

    return nullptr;
}

int CoflowBenchmarkTraceProducer::getCoflowNum() {
    if(this->coflowNum != 0)
        return this->coflowNum;
    int coflowNum;
    fstream CBfin;
    CBfin.open(this->coflowBenchFilePath);
    CBfin >> coflowNum;
    CBfin >> coflowNum;
    CBfin.close();
    this->coflowNum = coflowNum;
    return coflowNum;
}

int CoflowBenchmarkTraceProducer::getLogicMachinePortNum() {
    if(this->logicMachinePortNum != 0)
        return this->logicMachinePortNum;
    int logicMachinePortNum;
    fstream CBfin;
    CBfin.open(this->coflowBenchFilePath);
    CBfin >> logicMachinePortNum;
    CBfin.close();
    this->logicMachinePortNum = logicMachinePortNum;
    return logicMachinePortNum;
}

Coflow* CoflowBenchmarkTraceProducer::prepareOneCoflow(char* line) {
    stringstream ss(line);
    int coflowID;
    double startTime;
    int mapperNum, reducerNum;

    ss >> coflowID;
    ss >> startTime;
    auto* coflow = new Coflow(coflowID, startTime);

    ss >> mapperNum;
    int* mapperIDs = new int[mapperNum];
    for (int i = 0; i < mapperNum; ++i) {
        ss >> mapperIDs[i];
    }

    ss >> reducerNum;
    char buf[50];
    int reducerID;
    double sizeMB;
    vector<pair<int, double>> reducer_sizeMB;
    for (int j = 0; j < reducerNum; ++j) {
        ss >> buf;
        stringstream ss1(strtok(buf, ":"));
        stringstream ss2(strtok(nullptr, ":"));
        ss1 >> reducerID;
        ss2 >> sizeMB;
        reducer_sizeMB.emplace_back(make_pair(reducerID, sizeMB));
    }

    for (auto &it : reducer_sizeMB) {
        for (int i = 0; i < mapperNum; ++i) {
            coflow->addFlow(mapperIDs[i], it.first, it.second / mapperNum);
        }
    }
    return coflow;
}





