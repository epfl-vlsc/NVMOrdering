#pragma once

#include "Common.h"
#include <set>
#include <sstream>

namespace clang::ento::nvm
{

void maskTraverse(const Stmt *S, StringRef mask, bool &usesMask, bool &usesNegate)
{
    if (S)
    {
        if (const UnaryOperator *D = dyn_cast_or_null<UnaryOperator>(S))
        {
            usesNegate = true;
        }

        if (const DeclRefExpr *D = dyn_cast_or_null<DeclRefExpr>(S))
        {
            StringRef currentMask = D->getNameInfo()
                                        .getName()
                                        .getAsIdentifierInfo()
                                        ->getName();
            if (currentMask.equals(mask))
            {
                usesMask = true;
            }
        }

        for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
             I != E; ++I)
        {
            if (const Stmt *Child = *I)
            {
                maskTraverse(Child, mask, usesMask, usesNegate);
            }
        }
    }
}

bool usesMask(const Stmt *S, StringRef mask)
{
    bool usesMask = false;
    bool usesNegate = false;
    maskTraverse(S, mask, usesMask, usesNegate);
    return (usesMask && !usesNegate);
}

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

    bool isSameCheckName(const StringRef &otherCheckName)
    {
        return checkName_.equals(otherCheckName);
    }
};

class CheckInfo : public LabeledInfo
{
    bool hasMask_;
    StringRef maskStr_;

  public:
    CheckInfo(bool check, bool msk, StringRef maskVal)
        : LabeledInfo(check), hasMask_(msk), maskStr_(maskVal) {}

    bool hasMask()
    {
        return hasMask_;
    }

    StringRef getMask()
    {
        return maskStr_;
    }
};

class CheckDataInfo : public CheckInfo
{
    StringRef checkName_;
    bool isDcl_;

  public:
    CheckDataInfo(bool check, bool dcl, const StringRef &cName,
                  bool msk, StringRef maskVal)
        : CheckInfo(check, msk, maskVal), checkName_(cName), isDcl_(dcl) {}

    bool isDcl()
    {
        return isDcl_;
    }

    bool isSameCheckName(const StringRef &otherCheckName)
    {
        return checkName_.equals(otherCheckName);
    }

    //todo use smart pointer
    DataInfo* getDI(){
        return new DataInfo(false, false, checkName_);
    }
};

class NVMTypeInfo
{
    static constexpr const char *DCL = "dcl";
    static constexpr const char *SCL = "scl";
    static constexpr const char *CHECK = "check";
    static constexpr const char *SEP = "-";
    static constexpr const char *DOT_SEP = ".";
    static constexpr const StringRef emptySR;
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
                //todo use fully qualified name
                auto [_, fieldName] = annot.split(DOT_SEP);
                labels[D] = new DataInfo(false, true, fieldName);
                //llvm::outs() << "dcl " << annot << " " << fieldName << "\n";
            }
            else if (annotInfo.contains(SCL))
            {
                //todo use fully qualified name
                auto [_, fieldName] = annot.split(DOT_SEP);
                labels[D] = new DataInfo(false, false, fieldName);
                //llvm::outs() << "scl " << annot << "\n";
            }
            else if (annotInfo.contains(CHECK))
            {
                if (textInfo.empty())
                {
                    labels[D] = new CheckInfo(true, false, emptySR);
                    //llvm::outs() << "check " << annot << "\n";
                }
                else
                {
                    StringRef fieldName = D->getName();
                    labels[D] = new CheckDataInfo(true, false, fieldName, true, textInfo);
                    /*
                    llvm::outs() << "m: " << textInfo << " f: " << fieldName
                                 << " " << annot << "\n";
                    */
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