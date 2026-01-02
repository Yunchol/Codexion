#include <unistd.h>
#include "codexion.h"

void	*monitor_routine(void *arg)
{
	t_shared	*shared;
	int			i;
	long		now;

	shared = (t_shared *)arg;

	while (!shared->stop)
	{
		now = get_timestamp_ms();

		i = 0;
		while (i < 2)
		{
			pthread_mutex_lock(&shared->dongles[i].mutex);

			/* ★ deadline を超えたタスクが残っていないか */
			int j = 0;
			while (j < shared->dongles[i].queue.count)
			{
				if (shared->dongles[i].queue.items[j].deadline_ms < now)
				{
					log_msg(shared, "monitor: DEADLINE MISSED");
					shared->stop = 1;
				}
				j++;
			}

			pthread_mutex_unlock(&shared->dongles[i].mutex);
			i++;
		}
		usleep(1000); /* 1ms ごとに監視 */
	}
	return (NULL);
}
