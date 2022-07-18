#include "ARMv7_mmu.h"
#include "ARMv7_cpu.h"
#include <cstdint>

ARMV7_MMU::ARMV7_MMU(ARMV7_CPU *_cpu)
{
    cpu    = _cpu;
    cp15   = new ARMV7_CP15(this);
    bitops = new BitOps();
}
ARMV7_MMU::~ARMV7_MMU()
{
    delete cp15;
    delete bitops;
}
int64_t ARMV7_MMU::trans_to_phyaddr(int64_t vaddr, bool is_write)
{

    if (enabled) {
        return walk_table(vaddr, is_write);
    } else {
        return vaddr;
    }
    return 0;
};
int64_t ARMV7_MMU::ld_word(int64_t addr)
{
    return cpu->memctlr->ld_word(addr);
};
int64_t ARMV7_MMU::get_1st_ptaddr(int64_t vaddr)
{
    uint64_t vaddru = vaddr;
    int64_t  index  = (vaddru >> 20) & mask;
    int64_t  ptaddr;
    if (width) {
        bool is_zero = bitops->get_bits(vaddr, 31, 32 - width) == 0;
        if (is_zero)
            ptaddr = bitops->set_bits(baseaddr0, 13 - width, 2, index);
        else
            ptaddr = bitops->set_bits(baseaddr1, 13, 2, index);
        return bitops->clear_bits(ptaddr, 1, 0);
    } else {
        return baseaddr0 + (index << 2);
    }
};
int64_t ARMV7_MMU::get_2nd_ptaddr(int64_t vaddr, int64_t table)
{
    uint64_t vaddru = vaddr;
    int64_t  index  = (vaddru >> 12) & 0xff;
    int64_t  tmp    = table & 0xfffffc00;
    if (tmp < 0)
        tmp += 0x100000000;
    return tmp + (index << 2);
};
int64_t ARMV7_MMU::check_permission(int64_t vaddr, int64_t ap2, int64_t ap10, bool is_write, bool is_section)
{
    if (ap2) {
        switch (ap10) {
            case 0:
                throw "Reserved";
            case 1:
                if (is_write || !cpu->is_priviledged())
                    throw "Permission Fault: ap2 == 1, ap10 == 1";
                break;
            case 2:
                if (is_write)
                    throw "Permission Fault: ap2 == 1, ap10 == 2";
                break;
            case 3:
                if (is_write) {
                    if (is_section)
                        cp15->set_memory_abort(vaddr, cp15->PERMISSION_FAULT_SECTION, is_write);
                    else
                        cp15->set_memory_abort(vaddr, cp15->PERMISSION_FAULT_PAGE, is_write);
                    throw 111;
                }
                break;
            default:
                throw "Unknown ap10";
        }
    } else {
        switch (ap10) {
            case 0:
                if (is_section)
                    cp15->set_memory_abort(vaddr, cp15->PERMISSION_FAULT_SECTION, is_write);
                else
                    cp15->set_memory_abort(vaddr, cp15->PERMISSION_FAULT_PAGE, is_write);
                break;
            case 1:
                if (!cpu->is_priviledged())
                    throw "Permission Fault: ap2 == 0, ap10 == 1";
                break;
            case 2:
                if (is_write && !cpu->is_priviledged())
                    throw "Permission Fault: ap2 == 0, ap10 == 2";
                break;
            case 3:
                break;
            default:
                throw "Unknown ap10";
        }
    }
    return 0;
};
int64_t ARMV7_MMU::check_permission_table1(int64_t vaddr, int64_t table, bool is_write)
{
    uint64_t tableu = table;
    int64_t  ap2    = (tableu >> 15) & 1;
    int64_t  ap10   = (tableu >> 10) & 3;
    check_permission(vaddr, ap2, ap10, is_write, true);
    return 0;
};

int64_t ARMV7_MMU::check_permission_table2(int64_t vaddr, int64_t table, bool is_write)
{
    uint64_t tableu = table;
    int64_t  ap2    = (tableu >> 9) & 1;
    int64_t  ap10   = (tableu >> 4) & 3;
    check_permission(vaddr, ap2, ap10, is_write, false);
    return 0;
};
int64_t ARMV7_MMU::need_perm_check(int64_t table, bool is_supersection)
{
    uint64_t tableu = table;
    int64_t  domain;
    if (is_supersection)
        domain = cp15->domains[0];
    else
        domain = cp15->domains[(tableu >> 5) & 0xf];
    switch (domain) {
        case 0:
            throw "Domain Fault";
        case 1:
            return true;
        case 2:
            throw "Domain Reserved";
        case 3:
            return false;
        default:
            throw "Unknown Domain";
    }
};
int64_t ARMV7_MMU::walk_table(int64_t vaddr, bool is_write)
{
    int64_t paddr;
    int64_t ptaddr1 = get_1st_ptaddr(vaddr);
    int64_t table1  = ld_word(ptaddr1);
    int64_t format  = table1 & 3;
    switch (format) {
        case 0:
            cp15->set_memory_abort(vaddr, cp15->TRANS_FAULT_SECTION, false);
            throw 111;
            break;
        case 1:
            break;
        case 2: {
            uint64_t tableu          = table1;
            int64_t  is_supersection = (tableu >> 18) & 1;
            if (is_supersection) {
                if (need_perm_check(table1, true))
                    check_permission(vaddr, table1, is_write, false, false);
                throw "Supersection";
            } else {
                if (need_perm_check(table1, false))
                    check_permission_table1(vaddr, table1, is_write);
                int64_t tmp = table1 & 0xfff00000;
                if (tmp < 0)
                    tmp += 0x100000000;
                paddr = tmp + (vaddr & 0x000fffff);
            }
            return paddr;
        }
        case 3:
            throw "Translation fault (1st 3): ";
        default:
            throw "Unknown format: ";
    }

    int64_t ptaddr2 = get_2nd_ptaddr(vaddr, table1);
    int64_t table2  = ld_word(ptaddr2);

    if (need_perm_check(table1, false))    // table1 is correct
        check_permission_table2(vaddr, table2, is_write);
    int64_t format2 = table2 & 3;
    switch (format2) {
        case 0:
            cp15->set_memory_abort(vaddr, cp15->TRANS_FAULT_PAGE, false);
            throw 111;
            break;
        case 1:
            throw "Large page: ";
        case 2:
        case 3:
            break;
        default:
            throw "Unknown format: ";
    }
    int64_t tmp2 = table2 & 0xfffff000;
    if (tmp2 < 0)
        tmp2 += 0x100000000;

    paddr = tmp2 + (vaddr & 0x00000fff);
    return paddr;
};
