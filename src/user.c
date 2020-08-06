#include "user_sys.h"
#include "user.h"
#include "printf.h"

void user_process()
{
	call_hvc_write("User process\n\r");
  call_hvc_exit();
}

