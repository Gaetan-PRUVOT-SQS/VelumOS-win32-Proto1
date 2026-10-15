#ifndef TIME_INT_H
# define TIME_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/ksyscall.h"
# include "velum/proc.h"
# include "velum/timer.h"
# include "../irq/a05_lock.h"

# define NS_PER_S 1000000000ull
# define NS_PER_MS 1000000ull
# define NS_PER_US 1000ull
# define CONV_SHIFT 32
# define TIMERS_MAX 1024
# define TIMER_GEN_MASK 0x7fffffffu
# define WALL_MIN_NS 946684800000000000ull
# define WALL_MAX_NS 4102444800000000000ull
# define UNIX_MIN_YEAR 1970
# define UNIX_MAX_YEAR 9999
# define RTC_MIN_YEAR 2000
# define RTC_MAX_YEAR 2099
# define TSC_HZ_MIN 50000000ull
# define TSC_HZ_MAX 20000000000ull
# define HZ_TOLERANCE_PPM 10000
# define PIT_HZ 1193182ull
# define CLOCK_SRC_NONE 0
# define CLOCK_SRC_TSC 1
# define CLOCK_SRC_HPET 2
# define TIMER_MODE_NONE 0
# define TIMER_MODE_DEADLINE 1
# define TIMER_MODE_ONESHOT 2
# define WAIT_STEP_NS 1000ull
# define RTC_B_24H 0x02
# define RTC_B_BINARY 0x04
# define RTC_B_SET 0x80
# define RTC_A_UIP 0x80
# define RTC_HOUR_PM 0x80

typedef unsigned __int128	t_u128;

typedef struct s_clockconv
{
	uint64_t	mult;
	uint32_t	shift;
	uint32_t	reserved;
}	t_clockconv;

typedef struct s_civil
{
	int64_t		year;
	uint32_t	mon;
	uint32_t	day;
	uint32_t	hour;
	uint32_t	min;
	uint32_t	sec;
	uint32_t	reserved;
}	t_civil;

typedef struct s_rtc_raw
{
	uint8_t	sec;
	uint8_t	min;
	uint8_t	hour;
	uint8_t	wday;
	uint8_t	day;
	uint8_t	mon;
	uint8_t	year;
	uint8_t	cent;
	uint8_t	regb;
}	t_rtc_raw;

typedef struct s_tslot
{
	uint64_t	deadline;
	uint64_t	seq;
	t_timerfn	fn;
	void		*ctx;
	uint32_t	gen;
	int32_t		pos;
}	t_tslot;

typedef struct s_theap
{
	t_tslot		*slot;
	uint32_t	*heap;
	uint32_t	*freestk;
	uint32_t	cap;
	uint32_t	n;
	uint32_t	nfree;
	uint32_t	reserved;
	uint64_t	seq;
}	t_theap;

typedef struct s_clock
{
	t_clockconv	to_ns;
	t_clockconv	tsc_per_ns;
	uint64_t	origin;
	uint64_t	src_hz;
	uint64_t	tsc_hz;
	uint32_t	source;
	bool		ready;
}	t_clock;

typedef struct s_hpet
{
	volatile uint8_t	*base;
	uint64_t			hz;
	uint64_t			high;
	uint32_t			last_lo;
	bool				wide;
	bool				present;
	t_a05lock			lock;
}	t_hpet;

typedef struct s_timers
{
	t_theap		heap;
	t_tslot		slots[TIMERS_MAX];
	uint32_t	idx[2 * TIMERS_MAX];
	t_clockconv	lapic_per_ns;
	t_a05lock	lock;
	uint32_t	mode;
	bool		ready;
}	t_timers;

typedef struct s_probe
{
	uint64_t	at;
	uint32_t	fired;
	uint32_t	reserved;
}	t_probe;

int			clockconv_init(t_clockconv *c, uint64_t num, uint64_t den);
uint64_t	clockconv_apply(const t_clockconv *c, uint64_t v);
uint64_t	hz_from_ref(uint64_t dcycles, uint64_t dref, uint64_t ref_hz);
bool		hz_coherent(uint64_t a, uint64_t b, uint32_t ppm);
uint64_t	hz_from_cpuid15(uint32_t eax, uint32_t ebx, uint32_t ecx);
int64_t		civil_days(int64_t y, uint32_t m, uint32_t d);
void		civil_from_days(int64_t z, t_civil *out);
bool		civil_valid(const t_civil *c);
int64_t		civil_to_unix(const t_civil *c);
void		unix_to_civil(int64_t s, t_civil *out);
uint32_t	civil_weekday(int64_t days);
bool		rtc_field_dec(uint8_t v, uint8_t regb, uint32_t *out);
uint8_t		rtc_field_enc(uint32_t v, uint8_t regb);
bool		rtc_hour_dec(uint8_t h, uint8_t regb, uint32_t *out);
uint8_t		rtc_hour_enc(uint32_t h24, uint8_t regb);
int			rtc_decode(const t_rtc_raw *r, bool has_century,
				t_civil *out);
int			rtc_encode(const t_civil *c, uint8_t regb, bool has_century,
				t_rtc_raw *out);
void		theap_init(t_theap *h, t_tslot *slots, uint32_t *idx,
				uint32_t cap);
int64_t		theap_insert(t_theap *h, uint64_t deadline, t_timerfn fn,
				void *ctx);
bool		theap_cancel(t_theap *h, int64_t id);
uint64_t	theap_peek(const t_theap *h);
bool		theap_pop(t_theap *h, uint64_t now, t_tslot *out);
void		theap_sift_up(t_theap *h, uint32_t pos);
void		theap_sift_down(t_theap *h, uint32_t pos);
void		theap_remove_at(t_theap *h, uint32_t pos);
bool		clock_ready(void);
int			a05_check_priv(const t_process *p, uint32_t need);
int			a05_check_power(uint64_t op);
t_clock		*clock_state(void);
void		clock_install(uint32_t source, uint64_t src_hz,
				uint64_t tsc_hz);
uint64_t	clock_tsc_at(uint64_t deadline_ns);
uint64_t	tsc_read(void);
uint64_t	tsc_hz_cpuid(void);
uint64_t	tsc_calibrate_hpet(void);
uint64_t	tsc_calibrate_pit(void);
int			hpet_init(uint64_t phys);
bool		hpet_present(void);
uint64_t	hpet_read(void);
uint64_t	hpet_hz(void);
bool		hpet_is_wide(void);
t_hpet		*hpet_state(void);
int			rtc_read(t_rtc_raw *out, uint8_t century_reg);
t_a05lock	*rtc_lock(void);
uint8_t		rtc_cmos_rd(uint8_t idx);
int			rtc_write(const t_civil *c, uint8_t century_reg);
int			wall_init(void);
int			timer_hw_init(void);
uint32_t	timer_mode(void);
void		power_triple_fault(void) __attribute__((noreturn));
void		a05_syscalls_register(void);
t_process	*a05_caller(void);
int			timer_selftest(void);
int			timer_test_arm(void);
t_timers	*timers_state(void);
void		timer_irq(t_regs *regs, void *ctx);
int64_t		sys_time_mono(const t_sysargs *a);
int64_t		sys_time_wall(const t_sysargs *a);
int64_t		sys_time_set_wall(const t_sysargs *a);

#endif
