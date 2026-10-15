#ifndef VSYNC_H
# define VSYNC_H

# include <stdint.h>

# define V_MUTEX_UNLOCKED 0
# define V_MUTEX_LOCKED 1
# define V_MUTEX_CONTENDED 2

typedef struct s_vspin
{
	uint32_t	locked;
}	t_vspin;

typedef struct s_vmutex
{
	uint32_t	state;
	uint32_t	owner;
	uint32_t	event;
	uint32_t	reserved;
}	t_vmutex;

typedef struct s_vevent
{
	uint32_t	handle;
	uint32_t	manual;
}	t_vevent;

void	v_spin_lock(t_vspin *spin);
int		v_spin_trylock(t_vspin *spin);
void	v_spin_unlock(t_vspin *spin);
void	v_mutex_init(t_vmutex *mutex);
int		v_mutex_lock(t_vmutex *mutex);
int		v_mutex_trylock(t_vmutex *mutex);
int		v_mutex_unlock(t_vmutex *mutex);
void	v_mutex_destroy(t_vmutex *mutex);
int		v_event_init(t_vevent *event, int manual, int initial);
int		v_event_set(t_vevent *event);
int		v_event_reset(t_vevent *event);
int		v_event_pulse(t_vevent *event);
int		v_event_wait(t_vevent *event, uint64_t timeout_ns);
void	v_event_destroy(t_vevent *event);

#endif
