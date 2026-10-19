#ifndef CONSOLE_H
# define CONSOLE_H

# include <stddef.h>
# include <stdint.h>
# include "velum/abi/abi_types.h"

# define CON_COLS_MAX 160
# define CON_ROWS_MAX 200
# define CON_ROW_BYTES 192
# define CON_LINE_MAX 256
# define CON_LIVE_BYTES 512
# define CON_HIST_MAX 32
# define CON_ARGS_MAX 16
# define CON_NAME_MAX 64
# define CON_LS_MAX 128
# define CON_PS_MAX 64
# define CON_CAT_MAX 4096
# define CON_SYS_DIR "/system"
# define CON_BIN_DIR CON_SYS_DIR "/bin"
# define CON_USER "utilisateur"
# define CON_HOST "velum"
# define CON_SEGS 7
# define CON_QUIET 1

typedef enum e_concolor
{
	CC_TEXT = 0,
	CC_USER,
	CC_AT,
	CC_PATH,
	CC_DIR,
	CC_EXEC,
	CC_DIM,
	CC_ERR,
	CC_COUNT
}	t_concolor;

typedef struct s_conrow
{
	char		text[CON_ROW_BYTES];
	uint8_t		color[CON_ROW_BYTES];
	uint32_t	bytes;
	uint32_t	cells;
}	t_conrow;

typedef struct s_conlive
{
	char		text[CON_LIVE_BYTES];
	uint8_t		color[CON_LIVE_BYTES];
	uint32_t	bytes;
	uint32_t	cells;
	uint32_t	cursor;
}	t_conlive;

typedef struct s_conscreen
{
	t_conrow	rows[CON_ROWS_MAX];
	uint32_t	first;
	uint32_t	count;
	uint32_t	cols;
}	t_conscreen;

typedef struct s_conline
{
	char		buf[CON_LINE_MAX];
	uint32_t	len;
	uint32_t	cur;
	char		hist[CON_HIST_MAX][CON_LINE_MAX];
	uint32_t	hist_n;
	uint32_t	hist_pos;
	char		draft[CON_LINE_MAX];
}	t_conline;

typedef struct s_conent
{
	char		name[CON_NAME_MAX];
	uint32_t	dir;
}	t_conent;

typedef struct s_conops
{
	int	(*stat)(void *env, const char *path, t_vstat *st);
	int	(*list)(void *env, const char *path, t_conent *out, uint32_t max);
	int	(*read)(void *env, const char *path, char *buf, size_t max,
			size_t *len);
	int	(*sysinfo)(void *env, t_sysinfo *si);
	int	(*procs)(void *env, t_procinfo *out, uint32_t max);
	int	(*spawn)(void *env, const char *path, const char *arg,
			uint32_t *pid);
	int	(*kill)(void *env, uint32_t pid);
}	t_conops;

typedef struct s_console
{
	t_conscreen		screen;
	t_conline		line;
	char			cwd[VFS_PATH_MAX];
	const char		*sep;
	const t_conops	*ops;
	void			*env;
	int				quit;
	t_conent		ents[CON_LS_MAX];
	char			io[CON_CAT_MAX + 1];
	t_procinfo		procs[CON_PS_MAX];
}	t_console;

typedef struct s_conseg
{
	const char	*text;
	uint8_t		color;
}	t_conseg;

typedef int	(*t_concmd)(t_console *c, int argc, char **argv);

typedef struct s_concmdent
{
	const char	*name;
	const char	*help;
	t_concmd	run;
}	t_concmdent;

void		scr_init(t_conscreen *s, uint32_t cols);
void		scr_clear(t_conscreen *s);
void		scr_set_cols(t_conscreen *s, uint32_t cols);
void		scr_putc(t_conscreen *s, char c, uint8_t color);
void		scr_puts(t_conscreen *s, const char *text, uint8_t color);
void		scr_newline(t_conscreen *s);
uint32_t	scr_count(const t_conscreen *s);
const t_conrow	*scr_row(const t_conscreen *s, uint32_t i);
void		scr_pad(t_conscreen *s, uint32_t from_cell, uint32_t to_cell);

void		line_init(t_conline *l);
int			line_insert(t_conline *l, const char *utf8, uint32_t n);
void		line_backspace(t_conline *l);
void		line_delete(t_conline *l);
void		line_left(t_conline *l);
void		line_right(t_conline *l);
void		line_home(t_conline *l);
void		line_end(t_conline *l);
void		line_kill_end(t_conline *l);
void		line_kill_all(t_conline *l);
void		line_hist_prev(t_conline *l);
void		line_hist_next(t_conline *l);
void		line_commit(t_conline *l, char *out);
uint32_t	line_cells(const char *s, uint32_t bytes);
uint32_t	utf8_put(uint32_t cp, char *out);

int			con_split(char *line, char **argv, int max);
int			con_path_join(const char *cwd, const char *arg, char *out,
				size_t size);
int			con_path_in_bin(const char *path);
int			con_confine_arg(const char *cwd, const char *arg, char *out,
				size_t size);
const char	*con_err_text(int err);

void		con_init(t_console *c, const t_conops *ops, void *env,
				uint32_t cols);
int			con_prompt_segs(const t_console *c, t_conseg *out);
void		con_echo(t_console *c, const char *typed);
void		con_exec(t_console *c, char *line);
void		con_submit(t_console *c);
void		con_say(t_console *c, const char *text, uint8_t color);
void		con_fail(t_console *c, const char *cmd, const char *why);
const t_concmdent	*con_table(size_t *count);
const t_concmdent	*con_find(const char *name);
void		con_live_row(const t_console *c, uint32_t cols, t_conlive *out);
void		con_fmt_num(char *out, size_t size, uint64_t v, uint32_t width);
void		con_fmt_duration(char *out, size_t size, uint64_t ns);

int			con_cmd_ls(t_console *c, int argc, char **argv);
int			con_cmd_cat(t_console *c, int argc, char **argv);
int			con_cmd_cd(t_console *c, int argc, char **argv);
int			con_cmd_pwd(t_console *c, int argc, char **argv);
int			con_cmd_echo(t_console *c, int argc, char **argv);
int			con_cmd_ps(t_console *c, int argc, char **argv);
int			con_cmd_kill(t_console *c, int argc, char **argv);
int			con_cmd_uptime(t_console *c, int argc, char **argv);
int			con_cmd_mem(t_console *c, int argc, char **argv);
int			con_cmd_clear(t_console *c, int argc, char **argv);
int			con_cmd_help(t_console *c, int argc, char **argv);
int			con_cmd_exit(t_console *c, int argc, char **argv);
int			con_run_program(t_console *c, int argc, char **argv);

#endif
