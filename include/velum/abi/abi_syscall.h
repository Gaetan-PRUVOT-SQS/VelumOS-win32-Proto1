#ifndef ABI_SYSCALL_H
# define ABI_SYSCALL_H

# define VELUM_ABI_VERSION 1

# define SYS_EXIT 0x00
# define SYS_PROC_SPAWN 0x01
# define SYS_PROC_KILL 0x02
# define SYS_PROC_INFO 0x03
# define SYS_THREAD_CREATE 0x04
# define SYS_THREAD_EXIT 0x05
# define SYS_THREAD_PRIO 0x06
# define SYS_GETTID 0x07
# define SYS_SET_FSBASE 0x08
# define SYS_PROC_LIST 0x09
# define SYS_LOG 0x0a

# define SYS_CLOSE 0x10
# define SYS_DUP 0x11
# define SYS_WAIT 0x12
# define SYS_WAIT_MANY 0x13
# define SYS_EVENT_CREATE 0x14
# define SYS_EVENT_OP 0x15
# define SYS_SECTION_CREATE 0x16
# define SYS_SECTION_MAP 0x17
# define SYS_PORT_LISTEN 0x19
# define SYS_PORT_CONNECT 0x1a
# define SYS_PORT_ACCEPT 0x1b
# define SYS_CHAN_SEND 0x1c
# define SYS_CHAN_RECV 0x1d
# define SYS_TIMER_CREATE 0x1e
# define SYS_TIMER_SET 0x1f

# define SYS_OPEN 0x30
# define SYS_READ 0x31
# define SYS_WRITE 0x32
# define SYS_PREAD 0x33
# define SYS_PWRITE 0x34
# define SYS_SEEK 0x35
# define SYS_STAT 0x36
# define SYS_FSTAT 0x37
# define SYS_READDIR 0x38
# define SYS_MKDIR 0x39
# define SYS_UNLINK 0x3a
# define SYS_RENAME 0x3b
# define SYS_FSYNC 0x3c
# define SYS_TRUNCATE 0x3d

# define SYS_YIELD 0x50
# define SYS_SLEEP 0x51
# define SYS_TIME_MONO 0x52
# define SYS_TIME_WALL 0x53
# define SYS_TIME_SET_WALL 0x54
# define SYS_POWER 0x58

# define SYS_DISPLAY_INFO 0x60
# define SYS_DISPLAY_MAP 0x61
# define SYS_DISPLAY_SET_MODE 0x62
# define SYS_DISPLAY_MODES 0x63
# define SYS_KCON 0x64

# define SYS_INPUT_OPEN 0x70
# define SYS_INPUT_READ 0x71
# define SYS_INPUT_LAYOUT 0x72
# define SYS_INPUT_LEDS 0x73
# define SYS_INPUT_MOUSE_CFG 0x74

# define SYS_GETRANDOM 0x80
# define SYS_SYSINFO 0x81

# define SYS_VALLOC 0xa0
# define SYS_VFREE 0xa1
# define SYS_VPROTECT 0xa2
# define SYS_VQUERY 0xa3

# define SYS_MAX 0xff

# define PF_SPAWN 0x01
# define PF_DISPLAY 0x02
# define PF_INPUT 0x04
# define PF_LISTEN 0x08
# define PF_FSWRITE 0x10
# define PF_POWER 0x20
# define PF_ADMIN 0x40
# define PF_ALL 0x7f

# define PROT_R 0x01
# define PROT_W 0x02
# define PROT_X 0x04

# define O_RDONLY 0x0000
# define O_WRONLY 0x0001
# define O_RDWR 0x0002
# define O_CREAT 0x0040
# define O_EXCL 0x0080
# define O_TRUNC 0x0200
# define O_APPEND 0x0400
# define O_DIRECTORY 0x10000

# define SEEK_SET_ 0
# define SEEK_CUR_ 1
# define SEEK_END_ 2

# define WAIT_ANY 0x0
# define WAIT_ALL 0x1
# define EV_SET 0
# define EV_RESET 1
# define EV_PULSE 2
# define POWER_OFF 0
# define POWER_REBOOT 1
# define TIMEOUT_INF 0xffffffffffffffffull

#endif
