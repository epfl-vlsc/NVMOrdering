#include "annot.h"
#include <stdio.h>
#include <utility>

struct SimpleRec {
  sentinelp(SimpleRec::dvalid+dcl) int ddata1;
  sentinelp(SimpleRec::dvalid+dcl) int ddata2;
  int dvalid;

  sentinelp(SimpleRec::svalid+scl) int sdata;
  int svalid;

  int dfix1;
  int dfix2;
  int sfix;

  void analyze_recovery correctSeperate() {
    if (dvalid) {
      dfix1 = ddata1;
      dfix2 = ddata2;
    }
    if (svalid) {
      sfix = sdata;
    }
  }

  void analyze_recovery correctCombined() {
    if (dvalid && svalid) {
      dfix1 = ddata1;
      dfix2 = ddata2;
      sfix = sdata;
    }
  }

  void analyze_recovery notChecked() {
    dfix1 = ddata1;
    dfix2 = ddata2;
    sfix = sdata;
  }
};
