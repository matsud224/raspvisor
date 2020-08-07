.section ".text.user"

.set HVC_NOTIFY_NUMBER, 0    // hvc numbers
.set HVC_EXIT_NUMBER, 1

.globl user_delay
user_delay:
  subs x0, x0, #1
  bne user_delay
  ret

.globl call_hvc_notify
call_hvc_notify:
  mov w8, #HVC_NOTIFY_NUMBER
  hvc #0
  ret

.globl call_hvc_exit
call_hvc_exit:
  mov w8, #HVC_EXIT_NUMBER
  hvc #0
  ret
