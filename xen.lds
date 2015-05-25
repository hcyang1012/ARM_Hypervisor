ENTRY(start)
OUTPUT_ARCH(arm)
PHDRS
{
  text PT_LOAD ;
}
SECTIONS
{
  . = 0x80200000;
  _start = .;
  .text : {
        _stext = .;
       *(.start)
       *(.text)
       *(.text.cold)
       *(.text.unlikely)
       *(.fixup)
       *(.gnu.warning)
       _etext = .;
  } :text = 0x9090
  . = ALIGN((1 << 12));
  .rodata : {
        _srodata = .;
       __start_bug_frames = .;
       *(.bug_frames.0)
       __stop_bug_frames_0 = .;
       *(.bug_frames.1)
       __stop_bug_frames_1 = .;
       *(.bug_frames.2)
       __stop_bug_frames_2 = .;
       *(.rodata)
       *(.rodata.*)
        _erodata = .;
  } :text
  .data : {
       . = ALIGN((1 << 12));
       *(.data.page_aligned)
       *(.data)
       *(.data.rel)
       *(.data.rel.*)
       CONSTRUCTORS
  } :text
  . = ALIGN((1 << (7)));
  .data.read_mostly : {
       __start___ex_table = .;
       *(.ex_table)
       __stop___ex_table = .;
       __start___pre_ex_table = .;
       *(.ex_table.pre)
       __stop___pre_ex_table = .;
       *(.data.read_mostly)
       *(.data.rel.ro)
       *(.data.rel.ro.*)
  } :text
  . = ALIGN(8);
  .arch.info : {
      _splatform = .;
      *(.arch.info)
      _eplatform = .;
  } :text
  . = ALIGN(8);
  .dev.info : {
      _sdevice = .;
      *(.dev.info)
      _edevice = .;
  } :text
  . = ALIGN((1 << 12));
  __init_begin = .;
  .init.text : {
       _sinittext = .;
       *(.init.text)
       _einittext = .;
  } :text
  . = ALIGN((1 << 12));
  .init.data : {
       *(.init.rodata)
       *(.init.rodata.str*)
       *(.init.data)
       *(.init.data.rel)
       *(.init.data.rel.*)
       . = ALIGN(8);
       __ctors_start = .;
       *(.init_array)
       __ctors_end = .;
  } :text
  . = ALIGN(32);
  .init.setup : {
       __setup_start = .;
       *(.init.setup)
       __setup_end = .;
  } :text
  .init.proc.info : {
       __proc_info_start = .;
       *(.init.proc.info)
       __proc_info_end = .;
  } :text
  .initcall.init : {
       __initcall_start = .;
       *(.initcallpresmp.init)
       __presmp_initcall_end = .;
       *(.initcall1.init)
       __initcall_end = .;
  } :text
  .xsm_initcall.init : {
       __xsm_initcall_start = .;
       *(.xsm_initcall.init)
       __xsm_initcall_end = .;
  } :text
  __init_end_efi = .;
  . = ALIGN(((1 << 12) << 3));
  __init_end = .;
  .bss : {
       __bss_start = .;
       *(.bss.stack_aligned)
       . = ALIGN((1 << 12));
       *(.bss.page_aligned)
       *(.bss)
       . = ALIGN((1 << (7)));
       __per_cpu_start = .;
       *(.bss.percpu)
       . = ALIGN((1 << (7)));
       *(.bss.percpu.read_mostly)
       . = ALIGN((1 << (7)));
       __per_cpu_data_end = .;
       __bss_end = .;
  } :text
  _end = . ;
  /DISCARD/ : {
       *(.exit.text)
       *(.exit.data)
       *(.exitcall.exit)
       *(.eh_frame)
  }
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
}
ASSERT( _end_boot - start <= (1 << 12), "Boot code is larger than 4K")
