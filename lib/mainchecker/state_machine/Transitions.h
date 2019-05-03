#pragma once
#include "Common.h"
#include "TransDcl.h"
#include "TransScl.h"

namespace clang::ento::nvm {

class Transitions {
public:
  static void fenceFlush(StateInfo& SI) {
    if (SI.isScl()) {
      if (SI.isData()) {
        SclSpace::flushFenceData(SI);
      } else {
        SclSpace::flushFenceCheck(SI);
      }
    } else {
      if (SI.isData()) {
        DclSpace::flushFenceData(SI);
      } else {
        DclSpace::flushFenceCheck(SI);
      }
    }
  }
  static void pfence(StateInfo& SI) {
    if (SI.isScl()) {
      if (SI.isData()) {
        SclSpace::fence(SI);
      } else {
        SclSpace::fence(SI);
      }
    } else {
      if (SI.isData()) {
        DclSpace::pfence(SI);
      } else {
        DclSpace::pfence(SI);
      }
    }
  }
  static void vfence(StateInfo& SI) {
    if (SI.isScl()) {
      if (SI.isData()) {
        SclSpace::fence(SI);
      } else {
        SclSpace::fence(SI);
      }
    } else {
      // do not care
    }
  }
  static void flush(StateInfo& SI) {
    if (SI.isScl()) {
      // do not care
    } else {
      if (SI.isData()) {
        DclSpace::flushData(SI);
      } else {
        DclSpace::flushCheck(SI);
      }
    }
  }
  static void write(StateInfo& SI) {
    if (SI.isScl()) {
      if (SI.isData()) {
        SclSpace::writeData(SI);
      } else {
        SclSpace::writeCheck(SI);
      }
    } else {
      if (SI.isData()) {
        DclSpace::writeData(SI);
      } else {
        DclSpace::writeCheck(SI);
      }
    }
  }
};

} // namespace clang::ento::nvm
