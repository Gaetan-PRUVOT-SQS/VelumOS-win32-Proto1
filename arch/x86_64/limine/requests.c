#include "limine.h"
#include "limine_req.h"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t g_base_revision[3] = LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request g_memmap = {
	.id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0, .response = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request g_hhdm = {
	.id = LIMINE_HHDM_REQUEST_ID, .revision = 0, .response = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request g_fb = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0, .response = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request g_rsdp = {
	.id = LIMINE_RSDP_REQUEST_ID, .revision = 0, .response = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request g_module = {
	.id = LIMINE_MODULE_REQUEST_ID, .revision = 0, .response = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_address_request g_addr = {
	.id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID, .revision = 0, .response = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_cmdline_request g_cmdline = {
	.id = LIMINE_EXECUTABLE_CMDLINE_REQUEST_ID, .revision = 0, .response = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_date_at_boot_request g_date = {
	.id = LIMINE_DATE_AT_BOOT_REQUEST_ID, .revision = 0, .response = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_efi_system_table_request g_efi = {
	.id = LIMINE_EFI_SYSTEM_TABLE_REQUEST_ID, .revision = 0, .response = 0
};

int limine_base_ok(void)
{
	return (LIMINE_BASE_REVISION_SUPPORTED(g_base_revision));
}

const void *lim_memmap(void) { return (g_memmap.response); }
const void *lim_hhdm(void) { return (g_hhdm.response); }
const void *lim_fb(void) { return (g_fb.response); }
const void *lim_rsdp(void) { return (g_rsdp.response); }
const void *lim_module(void) { return (g_module.response); }
const void *lim_addr(void) { return (g_addr.response); }
const void *lim_cmdline(void) { return (g_cmdline.response); }
const void *lim_date(void) { return (g_date.response); }
const void *lim_efi(void) { return (g_efi.response); }
