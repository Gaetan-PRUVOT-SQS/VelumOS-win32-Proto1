#ifndef LOGON_FLOW_H
# define LOGON_FLOW_H

# include <stdint.h>

# define LF_NO_ACCOUNT 0xffffffffu

typedef enum e_logonmode
{
	LM_PICK = 0,
	LM_PASSWORD,
	LM_SHUTDOWN,
	LM_SESSION
}	t_logonmode;

typedef enum e_logonmsg
{
	LMSG_NONE = 0,
	LMSG_BAD,
	LMSG_WAIT,
	LMSG_ERROR,
	LMSG_POWER,
	LMSG_EMPTY
}	t_logonmsg;

typedef enum e_logonact
{
	LA_NONE = 0,
	LA_TRY_EMPTY,
	LA_START_SESSION,
	LA_POWER_OFF
}	t_logonact;

typedef struct s_logonflow
{
	t_logonmode	mode;
	t_logonmode	back;
	uint32_t	selected;
	t_logonmsg	msg;
	uint32_t	wait_s;
}	t_logonflow;

void	lf_init(t_logonflow *f);
int		lf_select(t_logonflow *f, uint32_t idx, int needs_password);
int		lf_result(t_logonflow *f, int authres, uint32_t wait_s);
int		lf_cancel(t_logonflow *f);
int		lf_session_ended(t_logonflow *f);
int		lf_ask_shutdown(t_logonflow *f);
int		lf_confirm_shutdown(t_logonflow *f);

#endif
