#ifndef CODEXION_H
#define CODEXION_H

#include <pthread.h>

#define MAX_CODERS 200

/* codexion.h */

#define MAX_CODERS 200

typedef struct s_task
{
	int   coder_id;
	long  deadline_ms;
}	t_task;

typedef struct s_queue
{
	t_task	items[MAX_CODERS];
	int		count;
}	t_queue;


typedef struct s_dongle
{
	pthread_mutex_t	mutex;
    pthread_cond_t	cond; 
	int				in_use;
    long			next_available_ms;
    t_queue			queue; /* ★ FIFO */
}	t_dongle;



typedef struct s_shared
{
	int		num_coders;
	int		num_dongles;
	int		finished;
    int			stop;
	long	start_time;
	t_dongle dongles[2];
}	t_shared;

typedef struct s_coder
{
	int			id;
	t_shared	*shared;
	pthread_t	thread;
}	t_coder;

long	get_timestamp_ms(void);
void	log_msg(t_shared *shared, const char *msg);

void	*coder_routine(void *arg);

void	*monitor_routine(void *arg);

#endif
