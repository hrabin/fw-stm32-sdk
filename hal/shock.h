#ifndef SHOCK_H
#define	SHOCK_H

#include "type.h"

#define SHOCK_ENABLE 1

#if SHOCK_ENABLE != 1
__inline bool shock_init(void) {OS_PUTTEXT("[OFF]");return (true);}
  #define	shock_detected() false
  #define	shock_task()
  #define	shock_get_activity() 0
  #define	shock_set_param(...)
#else // ! SHOCK_ENABLE
  extern bool shock_init (void);
  extern bool shock_reinit (void);
  extern bool shock_detected (void);
  extern void shock_task (void);
  extern u32  shock_get_activity (void);
  extern void shock_set_param (u8 sensitivity);
#endif // SHOCK_ENABLE

#endif // ~SHOCK_H

