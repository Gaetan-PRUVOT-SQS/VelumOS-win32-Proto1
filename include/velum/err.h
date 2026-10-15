#ifndef ERR_H
# define ERR_H

typedef enum e_err
{
	E_OK = 0,
	E_PERM = -1,
	E_NOENT = -2,
	E_IO = -5,
	E_BADF = -9,
	E_CHILD = -10,
	E_AGAIN = -11,
	E_NOMEM = -12,
	E_ACCES = -13,
	E_FAULT = -14,
	E_BUSY = -16,
	E_EXIST = -17,
	E_NODEV = -19,
	E_NOTDIR = -20,
	E_ISDIR = -21,
	E_INVAL = -22,
	E_NFILE = -23,
	E_MFILE = -24,
	E_NOSPC = -28,
	E_PIPE = -32,
	E_RANGE = -34,
	E_DEADLK = -35,
	E_NOSYS = -38,
	E_NOTEMPTY = -39,
	E_PROTO = -71,
	E_OVERFLOW = -75,
	E_NOTSUP = -95,
	E_TIMEOUT = -110,
	E_CANCELED = -125
}	t_err;

#endif
