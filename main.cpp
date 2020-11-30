#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

enum ClockType {
  INCREMENT_SLEEP, // the clock increments while the computer is asleep/suspended
  INCREMENT_AWAKE // the clock increments while the computer is awake
};

// Windows things

#if defined(_MSC_VER)
#include <windows.h>
#include <realtimeapiset.h>
#pragma comment(lib, "mincore.lib")

uint64_t get_time_ms(ClockType type) {
  uint64_t rv = 0;
  ULONGLONG interrupt_time;
  const uint64_t HNS2MS = 10000;
  if (type == INCREMENT_SLEEP) {
    QueryInterruptTime(&interrupt_time);
  } else if (type == INCREMENT_AWAKE) {
    if (!QueryUnbiasedInterruptTime(&interrupt_time)) {
      fprintf(stderr, "QueryUnbiasedInterruptTime error: %lx\n", GetLastError());
      return 0;
    }
  }
  rv = interrupt_time / HNS2MS;
  return rv;
}
#endif

// Apple things
#if defined(__APPLE__) && defined(__MACH__)
#include <time.h>

uint64_t get_time_ms(ClockType type) {
  uint64_t rv = 0;
  if (type == INCREMENT_SLEEP) {
    // https://developer.apple.com/documentation/kernel/1462446-mach_absolute_time recommends this
    rv = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW) / 1e6;
  } else if (type == INCREMENT_AWAKE) {
    // https://developer.apple.com/documentation/kernel/1646199-mach_continuous_time recommends this
    rv = clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1e6;
  }
  return rv;
}
#endif

// Linux desktop + android
#ifdef __linux__
#include <time.h>

uint64_t get_time_ms(ClockType type) {
  timespec ts;
  uint64_t rv = 0;
  if (type == INCREMENT_SLEEP) {
    if (clock_gettime(CLOCK_MONOTONIC, &ts)) {
      perror("clock_gettime (MONOTONIC)");
    }
    rv += ts.tv_sec * 1e3;
    rv += ts.tv_nsec / 1e6;
  } else if (type == INCREMENT_AWAKE) {
    if (clock_gettime(CLOCK_BOOTTIME, &ts)) {
      perror("clock_gettime (BOOTTIME)");
    }
    rv += ts.tv_sec * 1e3;
    rv += ts.tv_nsec / 1e6;
  }
  return rv;
}
#endif // __linux__

int main()
{
  uint64_t program_start_no_sleep = get_time_ms(INCREMENT_AWAKE);
  uint64_t program_start_sleep = get_time_ms(INCREMENT_SLEEP);
  printf("Time is %" PRIu64 "ms (awake time %" PRIu64 "ms)\n", program_start_sleep, program_start_no_sleep);
  printf("Put the computer to sleep, come back in some time..., and type 'y'");
  while(getchar() != 'y') {
    printf("Didn't understand sorry.\n");
  }
  uint64_t program_end_no_sleep = get_time_ms(INCREMENT_AWAKE);
  uint64_t program_end_sleep = get_time_ms(INCREMENT_SLEEP);
  printf("Time is %" PRIu64 "ms (awake time %" PRIu64 "ms)\n", program_end_sleep, program_end_no_sleep);
  printf("Program duration was %" PRIu64 "ms (awake time %" PRIu64 "ms)\n", program_end_sleep - program_start_sleep, program_end_no_sleep - program_start_no_sleep);
  return 0;
}
