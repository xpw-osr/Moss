#pragma once

// clang-format off

//.macro push ra, rb
//stp \ra, \rb, [sp,#-16]!
//.endm
//
//.macro pop ra, rb
//ldp \ra, \rb, [sp], #16
//.endm
//
///* this macro ensures that mask must be value of 1<<n, if not, compile failed */
//.macro tbzmask, reg, mask, label, shift=0
//.if \shift >= 64
//    .error "tbzmask: unsupported mask, \mask"
//.elseif \mask == 1 << \shift
//    tbz     \reg, #\shift, \label
//.else
//    tbzmask \reg, \mask, \label, "(\shift + 1)"
//.endif
//.endm
//
//.macro tbnzmask, reg, mask, label, shift=0
//.if \shift >= 64
//    .error "tbnzmask: unsupported mask, \mask"
//.elseif \mask == 1 << \shift
//    tbnz     \reg, #\shift, \label
//.else
//    tbnzmask \reg, \mask, \label, "(\shift + 1)"
//.endif
//.endm
//
//.macro calloc_bootmem_aligned, new_ptr, new_ptr_end, tmp, size_shift, phys_offset
//.if \size_shift < 4
//    .error "calloc_bootmem_aligned: Unsupported size_shift, \size_shift"
//.endif
//
//    /* load boot_alloc_end */
//    adrp    \tmp, boot_alloc_end
//    ldr     \new_ptr, [\tmp, #:lo12:boot_alloc_end]
//
//    /* align to page */
//.if \size_shift > 12
//    add     \new_ptr, \new_ptr, #(1 << \size_shift)
//    sub     \new_ptr, \new_ptr, #1
//.else
//    add     \new_ptr, \new_ptr, #(1 << \size_shift) - 1
//.endif
//    and     \new_ptr, \new_ptr, #~((1 << \size_shift) - 1)
//
//    /* add one page and store boot_alloc_end */
//    add     \new_ptr_end, \new_ptr, #(1 << \size_shift)
//    str     \new_ptr_end, [\tmp, #:lo12:boot_alloc_end]
//
////.if \phys_offset != 0
//    /* clear page */
//    sub     \new_ptr, \new_ptr, \phys_offset
//    sub     \new_ptr_end, \new_ptr_end, \phys_offset
////.endif
//
//    /* clear page */
//    mov     \tmp, \new_ptr
//.Lcalloc_bootmem_aligned_clear_loop\@:
//    stp     xzr, xzr, [\tmp], #16
//    cmp     \tmp, \new_ptr_end
//    b.lo    .Lcalloc_bootmem_aligned_clear_loop\@
//.endm
//
///* Set fault handler for next instruction */
//.macro set_fault_handler, handler
//.Lfault_location\@:
//.pushsection .rodata.fault_handler_table
//    .quad    .Lfault_location\@
//    .quad    \handler
//.popsection
//.endm
//

.macro adr_g reg, symbol, hwaddress_sanitizer_enabled=0
.if \hwaddress_sanitizer_enabled != 0
  adrp \reg, :pg_hi21_nc:\symbol
  movk \reg, #:prel_g3:\symbol+0x100000000
  add \reg, \reg, #:lo12:\symbol
.else
  adrp \reg, \symbol
  add \reg, \reg, #:lo12:\symbol
.endif
.endm  // adr_g


.macro movlit reg, literal
  mov \reg, #((\literal) & 0xffff)
.ifne (((\literal) >> 16) & 0xffff)
  movk \reg, #(((\literal) >> 16) & 0xffff), lsl #16
.endif
.ifne (((\literal) >> 32) & 0xffff)
  movk \reg, #(((\literal) >> 32) & 0xffff), lsl #32
.endif
.ifne (((\literal) >> 48) & 0xffff)
  movk \reg, #(((\literal) >> 48) & 0xffff), lsl #48
.endif
.endm
