#include "user.h"
#include "printf.h"
#include "user_sys.h"

void user_process() {
  call_hvc_notify();
  call_hvc_exit();
}
