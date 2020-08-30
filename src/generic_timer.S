.globl get_generic_timer_freq
get_generic_timer_freq:
  mrs x0, cntfrq_el0
  ret

.globl set_cnthp_ctl
set_cnthp_ctl:
  msr cnthp_ctl_el2, x0
  ret

.globl get_cnthp_ctl
get_cnthp_ctl:
  mrs x0, cnthp_ctl_el2
  ret

.globl set_cnthp_cval
set_cnthp_cval:
  msr cnthp_cval_el2, x0
  ret

.globl get_cnthp_cval
get_cnthp_cval:
  mrs x0, cnthp_cval_el2
  ret

.globl set_cnthp_tval
set_cnthp_tval:
  msr cnthp_tval_el2, x0
  ret

.globl get_cnthp_tval
get_cnthp_tval:
  mrs x0, cnthp_tval_el2
  ret
