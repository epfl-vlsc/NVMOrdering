#pragma once

#include "Common.h"
#include <set>
#include <sstream>

namespace clang::ento::nvm
{

int hasInt(const std::string &in)
{
    std::stringstream sstr(in);
    int val = 0;
    sstr >> val;
    return val;
}

class LabeledDecl
{
    bool isCheck_;

  public:
    LabeledDecl(bool check) : isCheck_(check) {}

    bool isCheck()
    {
        return isCheck_;
    }
};

class DataDecl : public LabeledDecl
{
    //todo if you have time optimize string comparison
    std::string checkName_;
    bool isDcl_;

  public:
    DataDecl(bool check, bool dcl, const std::string &cName) : LabeledDecl(check), checkName_(cName), isDcl_(dcl) {}

    bool isDcl()
    {
        return isDcl_;
    }
};

class CheckDecl : public LabeledDecl
{
    bool hasMask_;
    int maskValue_;

  public:
    CheckDecl(bool check, bool msk, int maskVal) : LabeledDecl(check), hasMask_(msk), maskValue_(maskVal) {}

    bool hasMask()
    {
        return hasMask_;
    }

    int getMaskValue()
    {
        return maskValue_;
    }
};

class NVMTypeInfo
{
    static constexpr const char *DCL = "dcl";
    static constexpr const char *SCL = "scl";
    static constexpr const char *CHECK = "check";
    static constexpr const char *SEP = "-";
    std::map<const DeclaratorDecl *, LabeledDecl *> labels;

  public:

    ~NVMTypeInfo(){
        for(auto& [_,LD] : labels){
            delete LD;
        }
    }

    void analyzeMemLabel(const DeclaratorDecl *D)
    {
        for (const auto *Ann : D->specific_attrs<AnnotateAttr>())
        {
            StringRef annot = Ann->getAnnotation();
            if (auto [annotInfo, textInfo] = annot.split(SEP);
                annotInfo.contains(DCL))
            {
                labels[D] = new DataDecl(false, true, textInfo);
                //llvm::outs() << "dcl " << annot << "\n";
            }
            else if (annotInfo.contains(SCL))
            {
                labels[D] = new DataDecl(false, false, textInfo);
                //llvm::outs() << "scl " << annot << "\n";
            }
            else if (annotInfo.contains(CHECK))
            {
                auto numStr = textInfo.str();

                if (auto numVal = hasInt(numStr); numVal)
                {
                    labels[D] = new CheckDecl(true, true, numVal);
                    //llvm::outs() << "check mask " << annot << "\n";
                }
                else
                {
                    labels[D] = new CheckDecl(true, false, 0);
                    //llvm::outs() << "check " << annot << "\n";
                }
            }
        }
    }

    bool inLabels(const DeclaratorDecl *D) const
    {
        return labels.count(D);
    }

    LabeledDecl* getDeclaratorInfo(const DeclaratorDecl *D) const
    {
        assert(labels.count(D));
        return labels.find(D)->second;
    }
};

} // namespace clang::ento::nvm