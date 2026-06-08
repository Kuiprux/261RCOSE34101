#include <math.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "data_structures.h"
#include "color_codes.h"
#include "job.h"
#include "settings.h"

void print_gantt(Job* job_list, schedule_result* sch, int proc_amount, ColorMode color_mode)
{
	if (!sch->last)
		return;

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	int min_process_term_width = log10(PID_MAX) + 2; //P00
	int min_run_time = sch->min_run_time <= 0 ? 1 : sch->min_run_time;
	//printf("#%d#", min_run_time);
	double min_gantt_width = sch->last->time * min_process_term_width / (double)min_run_time;
	int required_lines = ceil(min_gantt_width / w.ws_col);
	if (required_lines < 1)
		required_lines = 1;
	double width_per_time = w.ws_col * required_lines / (double)sch->last->time;
	double time_per_line = sch->last->time / (double)required_lines;

	schedule_result_part* cur_part = sch->start;

	int start_times[PROCESS_MAX] = { 0 };
	int term_offset = 0;
	char buffer[5000] = { 0 };

	int log_times_index = 0;
	int log_times_term_x[30] = { 0 };
	int log_times[30] = { 0 };

	char ansi_color_str[25] = { 0 };
	int last_end_term_x = 0;

	while (cur_part != NULL) {
		//arrive is not useful for printing gantt chart
		if(cur_part->type == ARRIVE) {
			cur_part = cur_part->next;
			continue;
		}

		int index = cur_part->arr_index;
		int pid = job_list[index].pid;
		int time = cur_part->time;

		int start_term_x = round(start_times[index] * width_per_time);
		int end_term_x = round(time * width_per_time);
		int term_width = end_term_x - start_term_x;
		if (term_width < 0)
			term_width = 0;
		if (term_width >= (int)sizeof(buffer))
			term_width = (int)sizeof(buffer) - 1;

		if (log_times_index < 0 || log_times[log_times_index] != time) {
			log_times_index++;
			log_times[log_times_index] = time; //In this way, 0 is always included in log_times
			log_times_term_x[log_times_index] = end_term_x - term_offset;
		}

		if (last_end_term_x < start_term_x) {
			printf("%s%*s", ANSI_BLACK, start_term_x - last_end_term_x, "");
			last_end_term_x = start_term_x;
		}

		switch (cur_part->type) {
		case START:
			start_times[index] = time;
			cur_part = cur_part->next;
			continue;
		case LEAVE:
			if (start_times[index] < 0) {
				cur_part = cur_part->next;
				continue;
			}
			//no break; is intentional
		case END:
			start_times[index] = -1;
			break;
		}



		//here runs only when the process is paused(or ended)
		//otherwise, continue; is called
		memset(buffer, ' ', term_width);
		buffer[term_width] = 0;
		int label_term_width = log10(pid) + 2; //why +2 instead of +1? Because of the space for 'P' ex) P13
		int label_start_term_x = (term_width - label_term_width) / 2;
		if (label_start_term_x < 0)
			label_start_term_x = 0;

		char char_after_label = buffer[label_start_term_x+label_term_width];
		sprintf(&(buffer[label_start_term_x]), "P%d", pid);
		buffer[label_start_term_x+label_term_width] = char_after_label;

		get_ansi_color(index, proc_amount, color_mode, ansi_color_str);

		printf("%s", ansi_color_str);
		int buffer_offset = 0;
		while (end_term_x - term_offset > w.ws_col)
		{
			//print the last part of the line of chart
			int left_term_width = end_term_x - term_offset - w.ws_col;
			int cutoff_term_width = term_width - left_term_width;
			printf("%.*s\n", cutoff_term_width, buffer + buffer_offset);

			printf("%s", ANSI_DEFAULT);

			//print the time line
			int prev_time_end_term_x = 0;
			for (int i = 0; i <= log_times_index; i++)
			{
				if (log_times_term_x[i] >= w.ws_col)
					break;
				int time_term_width = log_times[i] == 0 ? 1 : (int)(log10(log_times[i]) + 1);
				//try to center the time label, but make sure it does not poke out
				int time_term_x = min(max(prev_time_end_term_x, log_times_term_x[i] - time_term_width / 2), w.ws_col - time_term_width);
				int time_pad = time_term_x - prev_time_end_term_x;
				if (time_pad < 0)
					time_pad = 0;
				printf("%*s", time_pad, "");
				printf("%d", log_times[i]);

				prev_time_end_term_x = time_term_x + time_term_width;
			}
			//print the rest of the time line. Without this, the rest of the line will have the color of the next line
			printf("%*s", w.ws_col - prev_time_end_term_x, "");
			printf("\n");

			buffer_offset = cutoff_term_width;
			term_offset += w.ws_col;

			int carried_log_times[30] = { 0 };
			int carried_log_times_term_x[30] = { 0 };
			int carried_log_times_index = -1;
			for (int i = 0; i <= log_times_index; i++)
			{
				if (log_times_term_x[i] >= w.ws_col)
				{
					carried_log_times_index++;
					carried_log_times[carried_log_times_index] = log_times[i];
					carried_log_times_term_x[carried_log_times_index] = log_times_term_x[i] - w.ws_col;
				}
			}

			memset(log_times, 0, sizeof(log_times));
			memset(log_times_term_x, 0, sizeof(log_times_term_x));
			log_times_index = carried_log_times_index;
			for (int i = 0; i <= log_times_index; i++)
			{
				log_times[i] = carried_log_times[i];
				log_times_term_x[i] = carried_log_times_term_x[i];
			}
		}

		if (buffer_offset > 0)
			printf("%s", ansi_color_str);
		printf("%s", buffer + buffer_offset);

		last_end_term_x = end_term_x;
		cur_part = cur_part->next;
	}

	printf("%s\n", ANSI_DEFAULT);
	int prev_time_end_term_x = 0;
	for (int i = 0; i <= log_times_index; i++)
	{
		int time_term_width = log_times[i] == 0 ? 1 : (int)(log10(log_times[i]) + 1);
		//try to center the time label, but make sure it does not poke out
		int time_term_x = min(max(prev_time_end_term_x, log_times_term_x[i] - time_term_width / 2), w.ws_col - time_term_width);
		int time_pad = time_term_x - prev_time_end_term_x;
		if (time_pad < 0)
			time_pad = 0;
		printf("%*s", time_pad, "");
		printf("%d", log_times[i]);

		prev_time_end_term_x = time_term_x + time_term_width;
	}
	printf("\n");
}

void print_eval_result(evaluation_result* result)
{
	printf("Pid: waiting time, turnaround time\n");
	for (int i = 0; i < PROCESS_MAX; i++) {
		if (result->job_eval_result[i].pid < 0)
			continue;
		printf("P%d: %f, %f\n", result->job_eval_result[i].pid, result->job_eval_result[i].waiting_time, result->job_eval_result[i].turnaround_time);
	}
	printf("\nAverage: %f, %f\n", result->avg.waiting_time, result->avg.turnaround_time);
}

void print_eval_results(evaluation_result* result, int len)
{
	double avg_waiting = 0;
	double avg_turnaround = 0;

	printf("Name: waiting time, turnaround time\n");
	for (int i = 0; i < len; i++)
	{
		printf("%s: %f, %f\n", scheduling_methods[i].name, result[i].avg.waiting_time, result[i].avg.turnaround_time);
		avg_waiting += result[i].avg.waiting_time;
		avg_turnaround += result[i].avg.turnaround_time;
	}
	if (len > 0) {
		avg_waiting /= len;
		avg_turnaround /= len;
	}
	printf("\nAVG: %f, %f\n", avg_waiting, avg_turnaround);
}
