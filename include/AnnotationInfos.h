#pragma once

#include "Common.h"

namespace clang::ento::nvm {

class BaseInfo {
protected:
  const ValueDecl* data;

public:
  BaseInfo(const ValueDecl* data_) : data(data_) {}
};

class PairInfo : public BaseInfo {
protected:
  const ValueDecl* check;

public:
  PairInfo(const ValueDecl* data_, const ValueDecl* check_)
      : BaseInfo(data_), check(check_) {}
};

class PCheckInfo : public BaseInfo {

public:
  PCheckInfo(const ValueDecl* data_) : BaseInfo(data_) {}
};

class PDclInfo : public PairInfo {

public:
  PDclInfo(const ValueDecl* data_, const ValueDecl* check_)
      : PairInfo(data_, check_) {}
};

class PSclInfo : public PairInfo {
public:
  PSclInfo(const ValueDecl* data_, const ValueDecl* check_)
      : PairInfo(data_, check_) {}
};

class MaskInfo : public PairInfo {
public:
  MaskInfo(const ValueDecl* data_, const ValueDecl* check_)
      : PairInfo(data_, check_) {}
};

class PDclmInfo : public MaskInfo {
public:
  PDclmInfo(const ValueDecl* data_, const ValueDecl* check_)
      : MaskInfo(data_, check_) {}
};

class PSclmInfo : public MaskInfo {
public:
  PSclmInfo(const ValueDecl* data_, const ValueDecl* check_)
      : MaskInfo(data_, check_) {}
};

} // namespace clang::ento::nvm