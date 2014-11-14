#ifndef SYSTIME_H
#define SYSTIME_H

struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
  char *__tm_zone;
  int __tm_gmtoff;
};

typedef unsigned long time_t;

// System Time Utility
uint64 get_cmos_time(struct tm * ptm);
void init_boot_time();
uint64 get_current_millis();
uint64 get_boot_millis();
uint64 get_tick_count();


#endif