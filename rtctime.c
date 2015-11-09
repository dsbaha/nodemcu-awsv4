// Module for RTC time keeping

#include "lauxlib.h"

#include "rtc/rtctime_internal.h"
#include "rtc/rtctime.h"


// ******* C API functions *************

void rtctime_early_startup (void)
{
  Cache_Read_Enable (0, 0, 1);
  rtc_time_register_bootup ();
  rtc_time_switch_clocks ();
  Cache_Read_Disable ();
}

void rtctime_late_startup (void)
{
  rtc_time_switch_system ();
}

void rtctime_gettimeofday (struct rtc_timeval *tv)
{
  rtc_time_gettimeofday (tv);
}

void rtctime_settimeofday (const struct rtc_timeval *tv)
{
  if (!rtc_time_check_magic ())
    rtc_time_prepare ();
  rtc_time_settimeofday (tv);
}

bool rtctime_have_time (void)
{
  return rtc_time_have_time ();
}

void rtctime_deep_sleep_us (uint32_t us)
{
  rtc_time_deep_sleep_us (us);
}

void rtctime_deep_sleep_until_aligned_us (uint32_t align_us, uint32_t min_us)
{
  rtc_time_deep_sleep_until_aligned (align_us, min_us);
}



// ******* Lua API functions *************

//  rtctime.set (sec, usec)
static int rtctime_set (lua_State *L)
{
  if (!rtc_time_check_magic ())
    rtc_time_prepare ();

  uint32_t sec = luaL_checknumber (L, 1);
  uint32_t usec = 0;
  if (lua_isnumber (L, 2))
    usec = lua_tonumber (L, 2);

  struct rtc_timeval tv = { sec, usec };
  rtctime_settimeofday (&tv);
  return 0;
}


// sec, usec = rtctime.get ()
static int rtctime_get (lua_State *L)
{
  struct rtc_timeval tv;
  rtctime_gettimeofday (&tv);
  lua_pushnumber (L, tv.tv_sec);
  lua_pushnumber (L, tv.tv_usec);
  return 2;
}

static void do_sleep_opt (lua_State *L, int idx)
{
  if (lua_isnumber (L, idx))
  {
    uint32_t opt = lua_tonumber (L, idx);
    if (opt < 0 || opt > 4)
      luaL_error (L, "unknown sleep option");
    deep_sleep_set_option (opt);
  }
}

// rtctime.dsleep (usec, option)
static int rtctime_dsleep (lua_State *L)
{
  uint32_t us = luaL_checknumber (L, 1);
  do_sleep_opt (L, 2);
  rtctime_deep_sleep_us (us); // does not return
  return 0;
}


// rtctime.dsleep_aligned (aligned_usec, min_usec, option)
static int rtctime_dsleep_aligned (lua_State *L)
{
  if (!rtctime_have_time ())
    return luaL_error (L, "time not available, unable to align");

  uint32_t align_us = luaL_checknumber (L, 1);
  uint32_t min_us = luaL_checknumber (L, 2);
  do_sleep_opt (L, 3);
  rtctime_deep_sleep_until_aligned_us (align_us, min_us); // does not return
  return 0;
}

#define LEAP_YEAR(_year) ((_year%4)==0)
static  int8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};

static void setfield (lua_State *L, const char *key, int value) {
  lua_pushinteger(L, value);
  lua_setfield(L, -2, key);
}

static void setboolfield (lua_State *L, const char *key, int value) {
  if (value < 0) return;
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}

static int rtctime_date (lua_State *L)
{
  struct rtc_timeval tv;
  rtctime_gettimeofday (&tv);
  unsigned long epoch = tv.tv_sec;
  bool isDst;

  lua_createtable(L, 0, 9);

  setfield(L, "sec", epoch%60);
  epoch/=60;
  setfield(L, "min", epoch%60);
  epoch/=60;
  setfield(L, "hour", epoch%24);
  epoch/=24;
  setfield(L, "wday", (epoch+4)%7);

  uint8_t year = 70;
  unsigned long days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch) {
    year++;
  }

  setfield(L, "year", year+1900);

  days -= LEAP_YEAR(year) ? 366 : 365;
  epoch -= days;

  setfield(L, "yday", epoch);

  days = 0;
  uint8_t month,monthLength = 0;

  for (month=0; month<12; month++) {
    if (month==1) {
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (epoch>=monthLength) {
      epoch-=monthLength;
    } else {
        break;
    }
  }
  setfield(L, "month", month+1);
  setfield(L, "day", epoch+1);
  setboolfield(L, "isdst", isDst);

  return 1;
}


// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE rtctime_map[] =
{
  { LSTRKEY("set"), LFUNCVAL(rtctime_set) },
  { LSTRKEY("get"), LFUNCVAL(rtctime_get) },
  { LSTRKEY("date"), LFUNCVAL(rtctime_date) },
  { LSTRKEY("dsleep"),  LFUNCVAL(rtctime_dsleep)  },
  { LSTRKEY("dsleep_aligned"), LFUNCVAL(rtctime_dsleep_aligned) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_rtctime (lua_State *L)
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else
  luaL_register (L, AUXLIB_RTCTIME, rtctime_map);
  return 1;
#endif
}
