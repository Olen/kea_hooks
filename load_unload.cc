#include <hooks/hooks.h>
#include "library_common.h"
using namespace isc::hooks;
// "Interesting clients" log file handle definition.
std::fstream interesting;
extern "C" {
int load(LibraryHandle&) {
    interesting.open("/tmp/interesting.log",
                     std::fstream::out | std::fstream::app);
    return (interesting ? 0 : 1);
}
int unload() {
    if (interesting) {
        interesting.close();
    }
    return (0);
}
}
