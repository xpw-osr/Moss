#include "aarch64/esr.hh"
#include "aarch64/exception.hh"
#include "libcxx/log.hh"
#include "libcxx/macro.hh"
#include "libcxx/types.hh"

static bool exception_vector_table_already_initialized = false;

/// this is a init hook function, so detect it's initialized or not
extern_C void init_exception_vector_table() {
  [[likely]] if (not exception_vector_table_already_initialized) {
    init_exception_vector_table_asm();
    exception_vector_table_already_initialized = true;
  }
}

constexpr const char* entry_error_messages[] = {
    "SYNC_INVALID_EL1t",   "IRQ_INVALID_EL1t",   "FIQ_INVALID_EL1t",   "ERROR_INVALID_EL1T",

    "SYNC_INVALID_EL1h",   "IRQ_INVALID_EL1h",   "FIQ_INVALID_EL1h",   "ERROR_INVALID_EL1h",

    "SYNC_INVALID_EL0_64", "IRQ_INVALID_EL0_64", "FIQ_INVALID_EL0_64", "ERROR_INVALID_EL0_64",

    "SYNC_INVALID_EL0_32", "IRQ_INVALID_EL0_32", "FIQ_INVALID_EL0_32", "ERROR_INVALID_EL0_32"};

extern_C void show_invalid_entry_message(int type, u64 far_el1, u64 sp, u64 esr_el1, u64 elr_el1) {
  log_e("[%s] far_el1: 0x%lx, sp: 0x%lx, esr_el1: 0x%lx, elr_el1: 0x%lx, exception class: %s",
        entry_error_messages[type], far_el1, sp, esr_el1, elr_el1, esr_get_class_string(esr_el1));
}
