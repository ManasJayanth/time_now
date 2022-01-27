#include <caml/memory.h>
#include <time.h>
#include "ocaml_utils.h"
#include "config.h"

#define NANOS_PER_SECOND 1000000000

#if defined(JSC_TIMESPEC)

/* Note: this is imported noalloc if (and only if) ARCH_SIXTYFOUR is defined.
 * This is OK because caml_alloc_int63 doesn't actually allocate in that case. */
CAMLprim value time_now_nanoseconds_since_unix_epoch_or_zero()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
    return caml_alloc_int63(0);
  else
    return caml_alloc_int63(NANOS_PER_SECOND * (uint64_t)ts.tv_sec + (uint64_t)ts.tv_nsec);
}

#else

#if (defined(WIN32) || defined(_WIN32))
// Credit: https://stackoverflow.com/questions/10905892/equivalent-of-gettimeday-for-windows
#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
  // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
  // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
  // until 00:00:00 January 1, 1970 
  static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

  SYSTEMTIME  system_time;
  FILETIME    file_time;
  uint64_t    time;

  GetSystemTime( &system_time );
  SystemTimeToFileTime( &system_time, &file_time );
  time =  ((uint64_t)file_time.dwLowDateTime )      ;
  time += ((uint64_t)file_time.dwHighDateTime) << 32;

  tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
  tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
  return 0;
}

#else

#include <sys/types.h>
#include <sys/time.h>

#endif

CAMLprim value time_now_nanoseconds_since_unix_epoch_or_zero()
{
  struct timeval tp;
  if (gettimeofday(&tp, NULL) == -1)
    return caml_alloc_int63(0);
  else
    return caml_alloc_int63(NANOS_PER_SECOND * (uint64_t)tp.tv_sec + (uint64_t)tp.tv_usec * 1000);
}

#endif
