OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

PHDRS
{
	text PT_LOAD FLAGS(5);
	rodata PT_LOAD FLAGS(4);
	data PT_LOAD FLAGS(6);
}

SECTIONS
{
	. = 0xffffffff80000000;
	__kernel_start = .;

	.text : {
		__text_start = .;
		*(.text .text.*)
		__text_end = .;
	} :text

	. = ALIGN(4096);

	.rodata : {
		__rodata_start = .;
		*(.rodata .rodata.*)
	} :rodata

	.note.gnu.build-id : {
		g_build_id_note = .;
		*(.note.gnu.build-id)
	} :rodata

	.eh_frame : {
		*(.eh_frame .eh_frame_hdr)
	} :rodata

	. = ALIGN(4096);
	__rodata_end = .;

	.data : {
		__data_start = .;
		KEEP(*(.limine_requests))
		*(.data .data.*)
	} :data

	.bss : {
		__bss_start = .;
		*(.bss .bss.*)
		*(COMMON)
		__bss_end = .;
	} :data

	. = ALIGN(4096);
	__kernel_end = .;

	/DISCARD/ : {
		*(.comment)
		*(.note.GNU-stack)
	}
}
