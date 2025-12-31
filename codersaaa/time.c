#include "codexion.h"

long	time_ms(void)
{
	struct timeval			tv;

	gettimeofday(&tv, NULL); /* system time */
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000L); /* ms */
}

long	sim_time_ms(t_shared *shared)
{
	return (time_ms() - shared->start_ms); /* relative timestamp */
}

void	precise_sleep(t_shared *shared, long ms)
{
	long	end;
	long	now;
	long	left;

	end = time_ms() + ms; /* absolute deadline */
	while (!is_stopped(shared))
	{
		now = time_ms();
		if (now >= end)
			break;
		left = end - now; /* remaining time */
		if (left > 5)
			usleep(1000); /* 1ms slice */
		else
			usleep(200); /* finer slice */
	}
}
