#include "os_interface.h"

#include "windows.h"

uint64 get_tick_count() {
  return GetTickCount();
}
