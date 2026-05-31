#include <math.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>
#include <term.h>

#include "data_structures.h"


void print_gantt(Job* jobs, schedule_result* sch, int process_amount, ColorMode color_mode)
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	int min_process_width = log10(PID_MAX) + 2;
	double min_gantt_width = sch->last->time * min_process_width / (double)sch->min_run_time;
	int required_lines = ceil(min_gantt_width / w.ws_col);
	double width_per_time = w.ws_col * required_lines / (double)sch->last->time;
	double time_per_line = sch->last->time / (double)required_lines;

	schedule_result_part* cur_part = sch->start;

	int start_times[PROCESS_MAX] = { 0 };
	int term_offset = 0;
	char buffer[100] = { 0 };

	int log_times_index = 0;
	int log_times_term_x[30] = { 0 };
	int log_times[30] = { 0 };

	char ansi_color_str[25] = { 0 };

	while (cur_part != NULL)
	{
		int index = cur_part->arr_index;
		int pid = jobs[index].pid;
		int time = cur_part->time;

		int start_term_x = round(start_times[index] * width_per_time);
		int end_term_x = round(time * width_per_time);
		int term_width = end_term_x - start_term_x;

		if (cur_part->type != ARRIVE)
		{
			if (log_times[log_times_index] != time)
			{
				log_times_index++;
				log_times[log_times_index] = time; //In this way, 0 is always included in log_times
				log_times_term_x[log_times_index] = start_term_x;
			}
		}

		switch (cur_part->type)
		{
		case START:
			start_times[index] = time;
			continue;
		case LEAVE:
			if (start_times[index] < 0)
				continue;
			//no break; is intentional
		case END:
			start_times[index] = -1;
			break;
		default:
			continue;
		}

		//here runs only when the process is paused(or ended)
		//otherwise, continue; is called
		memset(buffer, ' ', term_width);
		buffer[term_width] = 0;
		int label_term_width = log10(pid) + 2; //why +2 instead of +1? Because of the space for 'P' ex) P13
		int label_start_term_x = (term_width - label_term_width) / 2;

		sprintf(&(buffer[label_start_term_x]), "P%d", pid);

		get_ansi_color(index, process_amount, color_mode, ansi_color_str);

		printf("%s", ansi_color_str);
		if (end_term_x >= w.ws_col)
		{
			//print the last part of the line of chart
			int left_term_width = end_term_x - w.ws_col;
			int cutoff_term_width = term_width - (left_term_width);
			printf("%.*s\n", cutoff_term_width, buffer);

			printf("%s", ANSI_DEFAULT);

			//print the time line
			int prev_time_end_term_x = 0;
			for (int i = 0; i <= log_times_index; i++)
			{
				int time_term_width = log10(log_times[i]) + 1;
				//try to center the time label, but make sure it does not poke out
				int time_term_x = min(max(prev_time_end_term_x, log_times_term_x - time_term_width / 2), w.ws_col - time_term_width);
				printf("*s", time_term_x - prev_time_end_term_x , "");
				printf(log_times[i]);

				prev_time_end_term_x = time_term_x + time_term_width;
			}
			printf("\n");

			//print the rest of the process on the new line
			printf("%s", ansi_color_str);

			printf("%s", &(buffer[left_term_width]));


			int end_log_time = log_times[log_times_index];
			int end_log_time_term_x = log_times_term_x[log_times_index];

			memset(log_times, 0, sizeof(int) * (log_times_index + 1));
			memset(log_times_term_x, 0, sizeof(int) * (log_times_index + 1));
			log_times_index = 0;

			log_times[log_times_index] = end_log_time;
			log_times_term_x[log_times_index] = end_log_time - w.ws_col;
		}
		else
		{
			printf("%s", buffer);
		}
		
		cur_part = cur_part->next;
	}

	printf("%s", ANSI_DEFAULT);
}

void print_eval_result(evaluation_result* result)
{
	printf("Average waiting time: %f\n", result->avg.waiting_time);
	printf("Average turnaround time: %f\n", result->avg.turnaround_time);
}

void print_eval_result_all(evaluation_result* result, int len)
{
	printf("Name: waiting time, turnaround time");
	for (int i = 0; i < len; i++)
	{
		printf("%s: %f, %f", result->job_eval_result[i], result->job_eval_result[i].waiting_time, result->job_eval_result[i].turnaround_time);
	}
	printf("\nAVG: %f, %f", result->avg.waiting_time, result->avg.turnaround_time);
}