#ifndef _H_ARMV7_MMU
#define _H_ARMV7_MMU
#include "ARMv7_cp15.h"
#include "../utils/bitopts.h"

class ARMV7_CPU;
class ARMV7_MMU {
  public:
    ARMV7_CPU  *cpu    = nullptr;
    ARMV7_CP15 *cp15   = nullptr;
    BitOps     *bitops = nullptr;

    int64_t enabled         = false;
    int64_t baseaddr0       = 0;
    int64_t baseaddr1       = 0;
    int64_t asid            = 0;
    int64_t width           = 0;
    int64_t mask            = (1 << (31 - width - 20 + 1)) - 1;
    int64_t check_unaligned = false;

  public:
    ARMV7_MMU(ARMV7_CPU *_cpu);
    ~ARMV7_MMU();

    int64_t trans_to_phyaddr(int64_t vaddr, bool is_write);
    int64_t ld_word(int64_t addr);
    int64_t get_1st_ptaddr(int64_t vaddr);
    int64_t get_2nd_ptaddr(int64_t vaddr, int64_t table);
    int64_t check_permission(int64_t vaddr, int64_t ap2, int64_t ap10, bool is_write, bool is_section);
    int64_t check_permission_table1(int64_t vaddr, int64_t table, bool is_write);
    int64_t check_permission_table2(int64_t vaddr, int64_t table, bool is_write);
    int64_t need_perm_check(int64_t table, bool is_supersection);
    int64_t walk_table(int64_t vaddr, bool is_write);

  private:
};
#endif
