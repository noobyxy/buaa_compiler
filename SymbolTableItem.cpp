//
// Created by yxy on 2020-10-22.
//

#include "SymbolTableItem.h"
#include "Utils.h"
#include <string>
#include <map>
#include <algorithm>

extern SyntaxSymbolTable symbolTable;

bool SymbolTableItem::compareParaListNum(const std::vector<ItemReturnType> &paraListTypeIn) const {
    int n1 = paraListTypes.size();
    int n2 = paraListTypeIn.size();
    if (n1 != n2) return false;
    return true;
}

bool SymbolTableItem::compareParaList(const std::vector<ItemReturnType>& paraListTypeIn) const {
    int n1 = paraListTypes.size();
    for (int i = 0; i < n1; i++)
    {
        if (paraListTypes[i] != paraListTypeIn[i]) return false;
    }
    return true;
}

bool SymbolTableItem::compareName(const std::string &nameIn) const {
    int l1 = this->name.length();
    int l2 = nameIn.length();
    if (l1 != l2) return false;
    for (int i = 0; i < l2;i++)
        if (this->name[i] != nameIn[i]) return false;
    return true;
}

extern FunctionSymbolTable* globalSymbolTable;

SymbolTableItem::SymbolTableItem() {}

FunctionSymbolTable::FunctionSymbolTable(const std::string& name):funcName(name),varNum(0) {
    if (name != "#global") {
        varInfo["$ra"] = itemInfo(LOCALVAR,varNum++);
        for (int i = 2; i <= 9; i++) {
            varInfo["$t" + int2string(i)] = itemInfo(LOCALVAR, 4*varNum++);
            varInfo["$s" + int2string(i - 2)] = itemInfo(LOCALVAR, 4 * varNum++);
        }
    }
    this->sRegPool = new SRegPool();
}

void FunctionSymbolTable::appendConst(const std::string &name,int value) {
    if (varInfo.find(name) != varInfo.end())
        varInfo[name].type = CONST;
    else {
        varInfo[name] = itemInfo(CONST,value);
    }
}

void FunctionSymbolTable::appendTempVar(const std::string &name) {

    varInfo[name] = itemInfo(TEMPVAR,4*varNum++);
}

void FunctionSymbolTable::appendLocalVar(const std::string &name) {
    if (varInfo.find(name) != varInfo.end())
    varInfo[name].type = LOCALVAR;
    else
    {
        varInfo[name] = itemInfo(LOCALVAR,4*varNum++);
    }
}

void FunctionSymbolTable::appendLocalVar(const std::string &name,int x) {
    varInfo[name] = itemInfo(LOCALARRAY1,4*varNum);
    varNum += x;
    arrayInfo[name] = arrayItemInfo(LOCALARRAY1, x);
}

void FunctionSymbolTable::appendLocalVar(const std::string &name,int x,int y) {
    varInfo[name] = itemInfo(LOCALARRAY2,4*varNum);
    varNum += x * y;
    arrayInfo[name] = arrayItemInfo(LOCALARRAY2, x, y);
}

//void FunctionSymbolTable::appendGlobalVar(const std::string &name) {
//    varInfo[name] = itemInfo(GLOBALVAR,4*varNum++);
//}

int FunctionSymbolTable::getOffset(const std::string& name) {
    auto it = varInfo.find(name);
    if (it != varInfo.end()) {
        return varInfo[name].offset;
    }
    return -1;
}

int FunctionSymbolTable::getOffset(const std::string& name,int x) {
    auto it = varInfo.find(name);
    if (it != varInfo.end()) {
        return varInfo[name].offset + x * 4;
    }
    return -1;
}

int FunctionSymbolTable::getOffset(const std::string& name,int x,int y) {
    auto it = varInfo.find(name);
    if (it != varInfo.end()) {
        return varInfo[name].offset + arrayInfo[name].y * x + y;
    }
    return -1;
}

int FunctionSymbolTable::getSubOffset() {
    return 4 * this->varNum;
}


bool FunctionSymbolTable::isConstValue(const std::string& name, int &value) {
    auto it = varInfo.find(name);
    if (it != varInfo.end()) {
        if ((it->second).type == CONST) {
            value = it->second.offset;
            return true;
        } else {
            return false;
        }
    }
    return globalSymbolTable->isConstValue(name,value);
}

void FunctionSymbolTable::appendPara(const std::string &name) {
    auto info = itemInfo(PARA,4*varNum++);
    paraInfo.push_back(info);
    varInfo[name] = info;
}

int FunctionSymbolTable::getRetOffset() {
    return 0;
}

int FunctionSymbolTable::getParaOffsetByIndex(int index) {
    return paraInfo[index].offset;
}

void FunctionSymbolTable::appendArrayPointer(const std::string &name, const std::string &arrayName, int x, int y) {

}

int FunctionSymbolTable::getArrayY(const std::string &name) {
    auto it = varInfo.find(name);
    if (it != varInfo.end()) {
        return arrayInfo[name].y;
    }
    return -1;
}

void FunctionSymbolTable::setIsLeaf(bool isLeaf) {
    this->isLeaf = isLeaf;
}

bool FunctionSymbolTable::getIsLeaf() {
    return isLeaf;
}

bool FunctionSymbolTable::isTmpValue(const std::string &name) {
    auto it = varInfo.find(name);
    if (it != varInfo.end()) {
        if (it->second.type == TEMPVAR) {
            return true;
        }
        return false;
    }
    return false;
}

void FunctionSymbolTable::addTimes(const std::string &name, int time) {
    if (this == globalSymbolTable) return;
    times[name] += time;
}

bool cmp(const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
    return a.second > b.second;
}

SRegPool *FunctionSymbolTable::getSRegPool() {
    std::vector<std::pair<std::string, int>> vec(times.begin(), times.end());
    std::sort(vec.begin(), vec.end(), cmp);
    if (times.size() <= 8) {
        for (int i = 0; i < times.size(); i++) {
            sRegPool->setReg(vec[i].first, i);
        }
    }
    else {
        for (int i = 0; i < 8; i++) {
            sRegPool->setReg(vec[i].first, i);
        }
    }
    return sRegPool;
}

