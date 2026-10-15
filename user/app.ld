OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
EXTERN(_start)

PHDRS
{
	text PT_LOAD FILEHDR PHDRS FLAGS(5);
	rodata PT_LOAD FLAGS(4);
	data PT_LOAD FLAGS(6);
	dynamic PT_DYNAMIC;
	note PT_NOTE;
	stack PT_GNU_STACK FLAGS(6);
}

SECTIONS
{
	. = SIZEOF_HEADERS;
	.text : { *(.text .text.*) } :text

	. = ALIGN(4096);
	.hash : { *(.hash) } :rodata
	.gnu.hash : { *(.gnu.hash) }
	.dynsym : { *(.dynsym) }
	.dynstr : { *(.dynstr) }
	.rela.dyn : { *(.rela.dyn .rela.*) }
	.rodata : { *(.rodata .rodata.*) }
	.note.gnu.build-id : { *(.note.gnu.build-id) } :rodata :note

	. = ALIGN(4096);
	.dynamic : { __relro_start = .; *(.dynamic) } :data :dynamic
	.init_array : { *(.init_array .init_array.* .ctors .ctors.*) } :data
	.fini_array : { *(.fini_array .fini_array.* .dtors .dtors.*) }
	.data.rel.ro : { *(.data.rel.ro .data.rel.ro.*) }
	.got : { *(.got .got.plt .igot .igot.plt) }
	.relro_pad : { . = ALIGN(4096); __relro_end = .; }
	.data : { *(.data .data.*) }
	.bss : { *(.bss .bss.*) *(COMMON) }

	/DISCARD/ : {
		*(.comment)
		*(.note.GNU-stack)
		*(.eh_frame .eh_frame_hdr)
		*(.interp)
	}
}
ASSERT(SIZEOF(.init_array) + SIZEOF(.fini_array) == 0, "constructeurs non pris en charge")
