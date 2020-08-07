#include "user.h"
#include "printf.h"
#include "user_sys.h"

void user_process() {
  call_hvc_write("--- hvc from user process ---\n\r");
  call_hvc_exit();
}
