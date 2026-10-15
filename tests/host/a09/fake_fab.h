#ifndef FAKE_FAB_H
# define FAKE_FAB_H

# include <stddef.h>
# include <stdint.h>

# define FAB_MAX_DEVS 300
# define FAB_LOG_MAX 512
# define FAB_BARS 6
# define FAB_NONE 0
# define FAB_MEM32 1
# define FAB_MEM64_LO 2
# define FAB_MEM64_HI 3
# define FAB_IO 4
# define FAB_ONES 5

typedef struct s_fabbar
{
	uint64_t	size;
	uint8_t		kind;
	uint8_t		pref;
}	t_fabbar;

typedef struct s_fabdev
{
	uint16_t	seg;
	uint8_t		bus;
	uint8_t		dev;
	uint8_t		fn;
	uint8_t		cfg[256];
	t_fabbar	bar[FAB_BARS];
	uint16_t	msi_off;
	int			cmd_ro;
	int			msi_ro;
}	t_fabdev;

typedef struct s_fabwr
{
	uint8_t		bus;
	uint8_t		dev;
	uint8_t		fn;
	uint8_t		width;
	uint16_t	off;
	uint32_t	val;
}	t_fabwr;

typedef struct s_fabspec
{
	uint8_t		bus;
	uint8_t		dev;
	uint8_t		fn;
	uint16_t	vendor;
	uint16_t	device;
	uint8_t		cls;
	uint8_t		sub;
	uint8_t		htype;
}	t_fabspec;

typedef struct s_fab
{
	t_fabdev	devs[FAB_MAX_DEVS];
	uint32_t	ndev;
	t_fabwr		log[FAB_LOG_MAX];
	uint32_t	nlog;
	uint32_t	reads;
	uint32_t	decode_violations;
}	t_fab;

extern t_fab	g_fab;

void		fab_reset(void);
t_fabdev	*fab_make(const t_fabspec *s);
t_fabdev	*fab_find(uint16_t seg, uint8_t bus, uint8_t dev, uint8_t fn);
void		fab_put(t_fabdev *d, uint16_t off, uint8_t w, uint32_t v);
uint32_t	fab_get(const t_fabdev *d, uint16_t off, uint8_t w);
void		fab_bar(t_fabdev *d, uint8_t idx, uint8_t kind, uint64_t size);
void		fab_bar_base(t_fabdev *d, uint8_t idx, uint64_t base);
void		fab_cap(t_fabdev *d, uint8_t off, uint8_t id, uint8_t next);
void		fab_bridge(t_fabdev *d, uint8_t sec, uint8_t sub);
void		fab_irq(t_fabdev *d, uint8_t line, uint8_t pin);
void		fab_bar_store(t_fabdev *d, uint8_t idx, uint32_t v);
uint32_t	fab_bar_reg(const t_fabdev *d, uint8_t idx);
int			fab_in_bars(const t_fabdev *d, uint16_t off);
uint32_t	fab_writes_at(uint16_t lo, uint16_t hi);
void		fab_endpoint(uint8_t bus, uint8_t dev, uint16_t vendor);
void		fab_pci_bridge(uint8_t bus, uint8_t dev, uint8_t sec, uint8_t sub);
void		fab_bridge_chain(int length);
void		fab_multifunction_slots(int slots);
t_fabdev	*fab_msi_dev(uint8_t dev, uint16_t ctrl);
void		fab_q35(void);

#endif
