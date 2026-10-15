#include "velum/libk.h"
#include "cpu_int.h"

static void	decode_vendor(const t_cpuid_raw *raw, t_cpufeat *feat)
{
	memcpy(feat->vendor, &raw->r[CPUID_L0][R_EBX], 4);
	memcpy(feat->vendor + 4, &raw->r[CPUID_L0][R_EDX], 4);
	memcpy(feat->vendor + 8, &raw->r[CPUID_L0][R_ECX], 4);
	feat->vendor[12] = '\0';
}

static void	decode_signature(const t_cpuid_raw *raw, t_cpufeat *feat)
{
	uint32_t	sig;
	uint32_t	base_family;

	sig = raw->r[CPUID_L1][R_EAX];
	base_family = (sig >> 8) & 0xf;
	feat->stepping = sig & 0xf;
	feat->model = (sig >> 4) & 0xf;
	feat->family = base_family;
	if (base_family == 0xf)
		feat->family += (sig >> 20) & 0xff;
	if (base_family == 0x6 || base_family == 0xf)
		feat->model += ((sig >> 16) & 0xf) << 4;
}

static void	decode_bits(const t_cpuid_raw *raw, t_cpufeat *feat)
{
	const t_featbit	*fb;
	bool			*field;

	fb = cpuid_featbits();
	while (fb->name)
	{
		field = (bool *)((char *)feat + fb->offset);
		*field = cpuid_bit(raw, fb->leaf, fb->reg, fb->bit);
		fb++;
	}
}

static void	decode_address_bits(const t_cpuid_raw *raw, t_cpufeat *feat)
{
	uint32_t	sizes;

	sizes = raw->r[CPUID_E8][R_EAX];
	feat->phys_bits = sizes & 0xff;
	feat->virt_bits = (sizes >> 8) & 0xff;
	if (!feat->phys_bits)
		feat->phys_bits = 36;
	if (!feat->virt_bits)
		feat->virt_bits = 48;
}

void	cpuid_decode(const t_cpuid_raw *raw, t_cpufeat *feat)
{
	memset(feat, 0, sizeof(*feat));
	decode_vendor(raw, feat);
	decode_signature(raw, feat);
	cpuid_brand(raw, feat->brand);
	decode_bits(raw, feat);
	decode_address_bits(raw, feat);
}
