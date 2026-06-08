#pragma once

#define PROCESS_MIN 50
#define PROCESS_MAX 100

#define PID_MIN 0
#define PID_MAX 999
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

#define TIME_QUANTUM_MULTIPLIER_MIN 1
#define TIME_QUANTUM_MULTIPLIER_MAX 5

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif