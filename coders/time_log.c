#include <stdio.h>
#include <sys/time.h>
#include "codexion.h"

long	get_timestamp_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	log_msg(t_shared *shared, const char *msg)
{
	long	now;

	now = get_timestamp_ms();
	printf("%ldms %s\n", now - shared->start_time, msg);
}
