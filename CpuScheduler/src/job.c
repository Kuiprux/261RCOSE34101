#include "job.h"

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