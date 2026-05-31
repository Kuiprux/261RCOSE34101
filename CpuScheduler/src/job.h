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

int job_compare(Job* a, Job* b)
{
	if (*(a->first_priority) > *(b->first_priority))
		return 1;
	if (*(a->first_priority) > *(b->first_priority))
		return -1;

	if (*(a->second_priority) > *(b->second_priority))
		return 1;
	if (*(a->second_priority) > *(b->second_priority))
		return -1;

	return 0;
}

int job_pid_compare(Job* a, Job* b)
{
	if (a->pid > b->pid)
		return 1;
	if (a->pid > b->pid)
		return -1;

	return 0;
}