#include <pebble.h>
  
#define KEY_CURRENT_INDEX 0
#define NUMBER_OF_ENTRIES 200 //total storage must be under 4kB, so with 7 bytes in each entry about 500 entries should be possible.

#define KEY_EVENT_TIME_STAMP_001 1
#define KEY_EVENT_STRING_001 1001
