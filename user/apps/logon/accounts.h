#ifndef ACCOUNTS_H
# define ACCOUNTS_H

# include <stddef.h>
# include <stdint.h>
# include "velum/crypto.h"

# define ACC_USERS_PATH "/system/etc/users"
# define ACC_NAME_MAX 32
# define ACC_SALT_MIN 8
# define ACC_SALT_MAX 32
# define ACC_ITER_MIN 10000
# define ACC_ITER_MAX 1000000
# define ACC_MAX 16
# define ACC_FIELDS 5
# define ACC_LINE_MAX 512
# define ACC_FILE_MAX 65536
# define ACC_PW_MAX 1024
# define ACC_FLAG_DISABLED 0x1
# define ACC_FLAGS_KNOWN 0x1

typedef struct s_field
{
	const char	*p;
	size_t		n;
}	t_field;

typedef struct s_account
{
	char		name[ACC_NAME_MAX];
	uint32_t	iterations;
	uint32_t	salt_len;
	uint32_t	flags;
	uint8_t		salt[ACC_SALT_MAX];
	uint8_t		hash[SHA256_LEN];
}	t_account;

typedef struct s_accounts
{
	t_account	list[ACC_MAX];
	uint32_t	count;
}	t_accounts;

int				acc_split(const char *line, size_t len, t_field *f);
int				acc_dec(const t_field *f, uint32_t *out);
int				acc_hex(const t_field *f, uint8_t *out, size_t max);
int				acc_name_ok(const char *s, size_t n);
int				acc_parse_line(const char *line, size_t len, t_account *out);
int				acc_parse_file(const char *text, size_t len, t_accounts *set);
const t_account	*acc_find(const t_accounts *set, const char *name);
int				acc_verify(const t_account *a, const char *pw, size_t pwlen);
int				acc_needs_password(const t_account *a);

#endif
