#include "IRCode.h"
#include "IRInst.h"
#include <iostream>

InterCode::InterCode(){}

InterCode::~InterCode(){}

void InterCode::addInst(InterCode& block)
{
    std::vector<IRInst*>& addInists = block.getInsts();
    code.insert(code.end(), addInists.begin(), addInists.end());
}

int InterCode::getCodeSize()
{
    return code.size();
}

void InterCode::addInst(IRInst* inst)
{
    code.push_back(inst);
}

std::vector<IRInst*>& InterCode::getInsts()
{
    return code;
}

void InterCode::outputIR(const std::string& filepath)
{
    FILE *fp = fopen(filepath.c_str(), "w");
    if (nullptr == fp) {
        std::cout << "fopen fail!!!" << std::endl;
        return;
    }

    for (auto &inst : code) {

        std::string instStr;
        inst->toString(instStr);
        std::cout << instStr << std::endl;
        if (!instStr.empty()) {
            fprintf(fp, "%s\n", instStr.c_str());
        }
    }

    fclose(fp);
}