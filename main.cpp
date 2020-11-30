#include <stdio.h>
#include <stdint.h>

enum ClockType {
  INCREMENT_SLEEP, // the clock increments while the computer is asleep/suspended
  INCREMENT_AWAKE // the clock increments while the computer is awake
};

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
  int program_start_no_sleep = get_time_ms(INCREMENT_AWAKE);
  int program_start_sleep = get_time_ms(INCREMENT_SLEEP);
  printf("Time is %dms (awake time %dms)\n", program_start_sleep, program_start_no_sleep);
  printf("Put the computer to sleep, come back in some time..., and type 'y'");
  while(getchar() != 'y') {
    printf("Didn't understand sorry.\n");
  }
  int program_end_no_sleep = get_time_ms(INCREMENT_AWAKE);
  int program_end_sleep = get_time_ms(INCREMENT_SLEEP);
  printf("Time is %dms (awake time %dms)\n", program_end_sleep, program_end_no_sleep);
  printf("Program duration was %dms (awake time %dms)\n", program_end_sleep - program_start_sleep, program_end_no_sleep - program_start_no_sleep);
}
