#include "routines/is_root.h"

using namespace hapi;

#include <sys/types.h>
#include <unistd.h>

bool is_root() { return getuid() == 0 && geteuid() == 0; }
