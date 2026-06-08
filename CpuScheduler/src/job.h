#pragma once
typedef struct
{
	int* first_priority;
	int* second_priority;

	int pid;
	int first_arrival_time;
	int burst_time;
	int io_amount;
	int io_start_times[20];
	int io_burst_times[20];
	int priority;
	int time_quantum_multiplier;
	
	int arr_index;

	int next_arrival_time;
	int run_start_time;
	int left_burst_time;
	int next_io_index;
} Job;

int job_compare(Job* a, Job* b);
int job_pid_compare(const void* a, const void* b);