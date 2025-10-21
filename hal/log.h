#ifndef LOG_H
#define LOG_H

#include "os.h"
#include "log_select.h"

#if LOG_ENABLE == 1

  #define LOG_DEBUG_LEVEL_DEFAULT 2

  #define LOG_DEF(tag)       static const char __attribute__((unused))  *TAG = tag

  extern int log_debug_level;
  extern int log_debug_select;
  #define LOG_DEBUG( ... )   log_msg_debug(1, TAG, __VA_ARGS__)
  #define LOG_DEBUGL(level, ... ) log_msg_debug(level, TAG, __VA_ARGS__)
  #define LOG_INFO( ... )    log_msg("I", TAG, __VA_ARGS__)
  #define LOG_WARNING( ... ) log_msg("W", TAG, __VA_ARGS__)
  #define LOG_ERROR( ... )   log_msg("E", TAG, __VA_ARGS__)

  void log_msg(const ascii *type, const ascii *id, const ascii *fmt, ... );
  void log_msg_debug(int debug_level, const ascii *id, const ascii *fmt, ... );

  void log_dump(const ascii *id, const ascii *text, const u8 *data, int data_len);

  #if LOG_LOCK == 1
    void log_init(void);
    void log_lock(void);
    void log_unlock(void);

  #else  // LOG_LOCK == 1
    #define log_init()
    #define log_lock()
    #define log_unlock()
  #endif // LOG_LOCK != 1

#else // LOG_ENABLE

  #define LOG_DEF(x)

  #define LOG_DEBUG( ... )   {OS_PRINTF(__VA_ARGS__); OS_PRINTF(NL);}
  #define LOG_DEBUGL(x, ... )   {OS_PRINTF(__VA_ARGS__); OS_PRINTF(NL);}

  #define LOG_INFO( ... )    {OS_PRINTF(__VA_ARGS__); OS_PRINTF(NL);}
  #define LOG_WARNING( ... ) {OS_PRINTF( __VA_ARGS__); OS_PRINTF(NL);}
  #define LOG_ERROR( ... )   {OS_PRINTF("ERROR:" __VA_ARGS__); OS_PRINTF(NL);}

  #define log_dump( ... )

  #define log_init()
  #define log_lock()
  #define log_unlock()

#endif // not LOG_ENABLE

#define LOG_DUMP( ... ) log_dump(TAG, __VA_ARGS__)

#endif //  LOG_H

