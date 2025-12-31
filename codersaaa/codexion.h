#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

# define CODEX_SCHED_FIFO 0
# define CODEX_SCHED_EDF 1

typedef struct s_shared t_shared;

typedef struct s_request
{
	int			coder_id; /* requester id */
	long			deadline; /* absolute deadline ms */
	long			seq; /* fifo order */
	int			heap_index; /* position in heap */
}t_request;

typedef struct s_heap
{
	t_request	**items; /* heap array */
	int			count; /* number of items */
	int			cap; /* capacity */
}t_heap;

typedef struct s_dongle
{
	pthread_mutex_t	mutex; /* protects dongle state */
	pthread_cond_t	cond; /* waiters */
	long			available_at; /* cooldown end */
	int			holder; /* current holder id */
	t_heap			heap; /* request heap */
}t_dongle;

typedef struct s_coder
{
	int			id; /* 1..n */
	pthread_t	thread; /* coder thread */
	pthread_mutex_t	mutex; /* protects coder state */
	long			last_compile_start; /* absolute ms */
	int			compile_count; /* completed compiles */
	int			done; /* reached required */
	t_shared		*shared; /* back-pointer */
}t_coder;

typedef struct s_shared
{
	int			num_coders; /* number of coders */
	long			t_burnout; /* ms */
	long			t_compile; /* ms */
	long			t_debug; /* ms */
	long			t_refactor; /* ms */
	int			required_compiles; /* stop target */
	long			dongle_cooldown; /* ms */
	int			scheduler; /* fifo or edf */
	long			start_ms; /* simulation start */
	int			stop; /* stop flag */
	long			seq_counter; /* fifo order */
	pthread_mutex_t	state_mutex; /* stop + seq */
	pthread_mutex_t	log_mutex; /* serialize logs */
	t_dongle		*dongles; /* dongles array */
	t_coder			*coders; /* coders array */
	pthread_t	monitor; /* monitor thread */
}t_shared;

int	parse_args(int ac, char **av, t_shared *shared);
int	init_sim(t_shared *shared);
void	cleanup_sim(t_shared *shared);

void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);

long	time_ms(void);
long	sim_time_ms(t_shared *shared);
void	precise_sleep(t_shared *shared, long ms);

void	log_state(t_shared *shared, int id, const char *msg);
int	is_stopped(t_shared *shared);
void	request_stop(t_shared *shared);

int	dongle_acquire(t_shared *shared, int idx, t_coder *coder);
void	dongle_release(t_shared *shared, int idx);

int		heap_init(t_heap *heap, int cap);
void	heap_destroy(t_heap *heap);
void	heap_push(t_heap *heap, t_request *req, int sched);
void	heap_remove(t_heap *heap, t_request *req, int sched);
int	heap_is_top(t_heap *heap, t_request *req);

t_request	*heap_peek(t_heap *heap);

#endif
