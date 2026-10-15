#ifndef ERRNO_LVALUE_H
# define ERRNO_LVALUE_H

int	*__velum_errno(void);

# define errno (*__velum_errno())

#endif
