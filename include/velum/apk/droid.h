#ifndef DROID_H
# define DROID_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/dvm.h"

# define DROID_VIEWS_MAX 64
# define DROID_KIDS_MAX 16
# define DROID_TEXT_MAX 256
# define DROID_HORIZONTAL 0
# define DROID_VERTICAL 1
# define DROID_LOG_DEBUG 3
# define DROID_LOG_INFO 4
# define DROID_LOG_WARN 5
# define DROID_LOG_ERROR 6

enum e_droidkind
{
	DV_NONE = 0,
	DV_VIEW,
	DV_TEXT,
	DV_BUTTON,
	DV_LINEAR
};

typedef void				(*t_droidlog)(void *user, uint32_t level,
								const char *tag, const char *msg);
typedef uint64_t			(*t_droidclock)(void *user);

typedef struct s_droidview
{
	uint32_t	kind;
	uint32_t	id;
	uint32_t	parent;
	uint32_t	orientation;
	uint32_t	nkids;
	uint32_t	kids[DROID_KIDS_MAX];
	t_dref		self;
	t_dref		listener;
	char		text[DROID_TEXT_MAX];
}	t_droidview;

typedef struct s_droid
{
	t_dvm			*vm;
	t_dref			activity;
	uint32_t		root;
	uint32_t		nviews;
	uint32_t		dirty;
	uint32_t		finished;
	t_droidlog		log;
	t_droidclock	clock;
	void			*user;
	char			title[DROID_TEXT_MAX];
	char			error[DROID_TEXT_MAX];
	t_droidview		views[DROID_VIEWS_MAX];
}	t_droid;

int					droid_install(t_droid *d, t_dvm *vm);
int					droid_start(t_droid *d, const char *activity_desc);
int					droid_click(t_droid *d, uint32_t view_id);
int					droid_stop(t_droid *d);
const t_droidview	*droid_view(const t_droid *d, uint32_t view_id);

#endif
