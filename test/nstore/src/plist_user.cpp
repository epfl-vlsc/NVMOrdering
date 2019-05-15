#include "plist.h"
#include <iostream>
#include "annot.h"
namespace storage {

void analyze_writes use_plist() {
  plist<const char*> p;
  const char* data = "c";
  p.push_back(data);
}

} // namespace storage
