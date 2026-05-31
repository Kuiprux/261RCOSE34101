#pragma once

#define PROCESS_MIN 5
#define PROCESS_MAX 10

#define PID_MIN 0
#define PID_MAX 99
#define PID_AVAILABLE_AMOUNT PID_MAX - PID_MIN + 1

#define JOB_QUEUE_SIZE PROCESS_MAX

#define ARRIVAL_TIME_MIN 0
#define ARRIVAL_TIME_MAX 20
#define EXTRA_CPU_BURST_TIME_MIN 5
#define EXTRA_CPU_BURST_TIME_MAX 20

#define IO_BURST_AMOUNT_MIN 0
#define IO_BURST_AMOUNT_MAX 2

#define IO_GAP_MIN 2
#define IO_GAP_MAX 5

#define IO_BURST_TIME_MIN 5
#define IO_BURST_TIME_MAX 20

#define PRIORITY_MIN 0
#define PRIORITY_MAX 7

#define TIME_QUANTUM 3