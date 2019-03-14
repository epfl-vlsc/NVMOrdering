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

class LabeledInfo
{
    bool isCheck_;

  public:
    LabeledInfo(bool check)
        : isCheck_(check) {}

    bool isCheck()
    {
        return isCheck_;
    }
};

class DataInfo : public LabeledInfo
{
    //todo if you have time optimize string comparison
    StringRef checkName_;
    bool isDcl_;

  public:
    DataInfo(bool check, bool dcl, const StringRef &cName)
        : LabeledInfo(check), checkName_(cName), isDcl_(dcl) {}

    bool isDcl()
    {
        return isDcl_;
    }

    bool isSameCheckName(const StringRef& otherCheckName){
        return checkName_.equals(otherCheckName);
    }
};

class CheckInfo : public LabeledInfo
{
    bool hasMask_;
    int maskValue_;

  public:
    CheckInfo(bool check, bool msk, int maskVal)
        : LabeledInfo(check), hasMask_(msk), maskValue_(maskVal) {}

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
    static constexpr const char *DOT_SEP = ".";
    std::map<const DeclaratorDecl *, LabeledInfo *> labels;

  public:
    ~NVMTypeInfo()
    {
        for (auto &[_, LD] : labels)
        {
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
                //todo use fullname for field save
                auto [_, fieldName] = annot.split(DOT_SEP);
                labels[D] = new DataInfo(false, true, fieldName);
                //llvm::outs() << "dcl " << annot << " " << fieldName << "\n";
            }
            else if (annotInfo.contains(SCL))
            {
                //todo use fullname for field save
                auto [_, fieldName] = annot.split(DOT_SEP);
                labels[D] = new DataInfo(false, false, fieldName);
                //llvm::outs() << "scl " << annot << "\n";
            }
            else if (annotInfo.contains(CHECK))
            {
                auto numStr = textInfo.str();

                if (auto numVal = hasInt(numStr); numVal)
                {
                    labels[D] = new CheckInfo(true, true, numVal);
                    //llvm::outs() << "check mask " << annot << "\n";
                }
                else
                {
                    labels[D] = new CheckInfo(true, false, 0);
                    //llvm::outs() << "check " << annot << "\n";
                }
            }
        }
    }

    bool inLabels(const DeclaratorDecl *D) const
    {
        return labels.count(D);
    }

    LabeledInfo *getDeclaratorInfo(const DeclaratorDecl *D) const
    {
        assert(labels.count(D));
        return labels.find(D)->second;
    }
};

} // namespace clang::ento::nvm