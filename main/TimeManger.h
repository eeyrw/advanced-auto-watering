#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <time.h>

void obtain_time(void);
void initialize_sntp(void);
void time_init_task(void *arg);

#endif