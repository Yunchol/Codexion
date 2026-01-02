#include <unistd.h>
#include "codexion.h"

void *monitor_routine(void *arg)
{
    t_shared *shared;
    int i;
    long now;

    shared = (t_shared *)arg;

    while (!shared->stop)
    {
        /* ★ 正常終了チェック */
        if (shared->finished_coders >= shared->num_coders)
        {
            log_msg(shared, "monitor: all coders finished");
            shared->stop = 1;
        }

        now = get_timestamp_ms();

        i = 0;
        while (i < 2)
        {
            pthread_mutex_lock(&shared->dongles[i].mutex);

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

        /* ★ stop が立ったら全員起こす */
        if (shared->stop)
        {
            i = 0;
            while (i < 2)
            {
                pthread_cond_broadcast(&shared->dongles[i].cond);
                i++;
            }
        }

        usleep(1000);
    }
    return (NULL);
}
