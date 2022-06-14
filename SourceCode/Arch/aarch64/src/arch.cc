#include "aarch64/mp.hh"    // for SMP_MAX_CPUS
#include "libcxx/macros.hh" // for extern_C
#include "libcxx/types.hh"  // for uintptr_t, u64

struct arm64_sp_info_t {
  u64 mpid;
  void* sp;                  // Stack pointer points to arbitrary data.
  uintptr_t* shadow_call_sp; // SCS pointer points to array of addresses.

  // This part of the struct itself will serve temporarily as the
  // fake arch_thread in the thread pointer, so that safe-stack
  // and stack-protector code can work early.  The thread pointer
  // (TPIDR_EL1) points just past arm64_sp_info_t.
  uintptr_t stack_guard;
  void* unsafe_sp;
};

// one for each CPU
arm64_sp_info_t arm64_secondary_sp_list[SMP_MAX_CPUS];

extern_C void arm64_secondary_entry() {}
