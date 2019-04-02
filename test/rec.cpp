#include "annot.h"
#include <stdio.h>
#include <utility>

struct SimpleRec {
  pdcl(SimpleRec::dvalid) int ddata1;
  pdcl(SimpleRec::dvalid) int ddata2;
  int dvalid;

  pscl(SimpleRec::svalid) int sdata;
  int svalid;

  int dfix1;
  int dfix2;
  int sfix;

  void recovery_code correctSeperate() {
    if (dvalid) {
      dfix1 = ddata1;
      dfix2 = ddata2;
    }
    if (svalid) {
      sfix = sdata;
    }
  }

  void recovery_code correctCombined() {
    if (dvalid && svalid) {
      dfix1 = ddata1;
      dfix2 = ddata2;
      sfix = sdata;
    }
  }

  void recovery_code notChecked() {
    dfix1 = ddata1;
    dfix2 = ddata2;
    sfix = sdata;
  }
};
