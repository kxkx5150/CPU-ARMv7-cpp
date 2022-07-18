#include "ARMv7_cpu.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>

using namespace std;

ARMV7_CPU::ARMV7_CPU(Mem *_mem, DTimer *_timer0)
{
    file_read();

    is_good_mode[USR_MODE] = true;
    is_good_mode[FIQ_MODE] = true;
    is_good_mode[IRQ_MODE] = true;
    is_good_mode[SVC_MODE] = true;
    is_good_mode[ABT_MODE] = true;
    is_good_mode[UND_MODE] = true;
    is_good_mode[SYS_MODE] = true;

    for (size_t i = 0; i < 16; i++) {
        regs[i]     = 0;
        regs_usr[i] = 0;
        coprocs[i]  = nullptr;
    }
    memctlr     = _mem;
    mmu         = new ARMV7_MMU(this);
    coprocs[15] = mmu->cp15;
    bitops      = new BitOps();
    timer0      = _timer0;

    regs_svc[13] = 0;
    regs_svc[14] = 0;

    regs_mon[13] = 0;
    regs_mon[14] = 0;

    regs_abt[13] = 0;
    regs_abt[14] = 0;

    regs_und[13] = 0;
    regs_und[14] = 0;

    regs_irq[13] = 0;
    regs_irq[14] = 0;

    regs_fiq[8]  = 0;
    regs_fiq[9]  = 0;
    regs_fiq[10] = 0;
    regs_fiq[11] = 0;
    regs_fiq[12] = 0;
    regs_fiq[13] = 0;
    regs_fiq[14] = 0;
}
ARMV7_CPU::~ARMV7_CPU()
{
}
int64_t ARMV7_CPU::get_pc()
{
    return regs[15] + 8;
};
int64_t ARMV7_CPU::reg(int i)
{
    if (i == 15)
        return get_pc();
    else
        return regs[i];
};
bool ARMV7_CPU::is_priviledged()
{
    int64_t mode = cpsr.m;
    if (mode == USR_MODE)
        return false;
    else
        return true;
};
int64_t ARMV7_CPU::is_user_or_system()
{
    int64_t mode = cpsr.m;
    if (mode == USR_MODE || mode == SYS_MODE)
        return true;
    else
        return false;
};
int64_t ARMV7_CPU::is_secure()
{
    return false;
};
int64_t ARMV7_CPU::scr_get_aw()
{
    return 1;
};
int64_t ARMV7_CPU::scr_get_fw()
{
    return 1;
};
int64_t ARMV7_CPU::nsacr_get_rfr()
{
    return 0;
};
int64_t ARMV7_CPU::sctlr_get_nmfi()
{
    return coprocs[15]->sctlr_get_nmfi();
};

Flags ARMV7_CPU::parse_psr(int64_t value)
{
    uint64_t valueu = value;
    Flags    psr    = {.n = 0, .z = 0, .c = 0, .v = 0, .q = 0, .e = 0, .a = 0, .i = 0, .f = 0, .t = 0, .m = 0};
    psr.n           = valueu >> 31;
    psr.z           = (valueu >> 30) & 1;
    psr.c           = (valueu >> 29) & 1;
    psr.v           = (valueu >> 28) & 1;
    psr.q           = (valueu >> 27) & 1;
    psr.e           = (valueu >> 9) & 1;
    psr.a           = (valueu >> 8) & 1;
    psr.i           = (valueu >> 7) & 1;
    psr.f           = (valueu >> 6) & 1;
    psr.t           = (valueu >> 5) & 1;
    psr.m           = value & 0x1f;
    return psr;
};
int64_t ARMV7_CPU::psr_to_value(Flags *psr)
{
    int64_t value = psr->m;
    value += psr->t << 5;
    value += psr->f << 6;
    value += psr->i << 7;
    value += psr->a << 8;
    value += psr->e << 9;
    value += psr->q << 27;
    value += psr->v << 28;
    value += psr->c << 29;
    value += psr->z << 30;
    value += psr->n << 31;
    return value;
};
Flags *ARMV7_CPU::clone_psr_n(Flags *src)
{

    Flags *dst = new Flags();
    dst->n     = src->n;
    dst->z     = src->z;
    dst->c     = src->c;
    dst->v     = src->v;
    dst->q     = src->q;
    dst->e     = src->e;
    dst->a     = src->a;
    dst->i     = src->i;
    dst->f     = src->f;
    dst->t     = src->t;
    dst->m     = src->m;
    return dst;
};
void ARMV7_CPU::set_current_spsr(Flags spsr)
{
    switch (cpsr.m) {
        case USR_MODE:
            throw " user";
        case FIQ_MODE:
            spsr_fiq = spsr;
            break;
        case IRQ_MODE:
            spsr_irq = spsr;
            break;
        case SVC_MODE:
            spsr_svc = spsr;
            break;
        case MON_MODE:
            spsr_mon = spsr;
            break;
        case ABT_MODE:
            spsr_abt = spsr;
            break;
        case UND_MODE:
            spsr_und = spsr;
            break;
        case SYS_MODE:
            throw " system user";
        default:
            throw " unknown";
    }
};
Flags ARMV7_CPU::get_current_spsr()
{
    switch (cpsr.m) {
        case USR_MODE:
            throw "get_current_spsr user";
        case FIQ_MODE:
            return spsr_fiq;
        case IRQ_MODE:
            return spsr_irq;
        case SVC_MODE:
            return spsr_svc;
        case MON_MODE:
            return spsr_mon;
        case ABT_MODE:
            return spsr_abt;
        case UND_MODE:
            return spsr_und;
        case SYS_MODE:
            throw "get_current_spsr system user";
        default:
            throw "get_current_spsr unknown";
    }
};
Flags *ARMV7_CPU::spsr_write_by_instr0(Flags *spsr, Flags *psr, int64_t bytemask)
{
    if (is_user_or_system())
        abort_unpredictable("", 0);
    if (bytemask & 8) {
        spsr->n = psr->n;
        spsr->z = psr->z;
        spsr->c = psr->c;
        spsr->v = psr->v;
        spsr->q = psr->q;
    }
    if (bytemask & 4) {
        // spsr->ge = psr->ge;
    }
    if (bytemask & 2) {
        spsr->e = psr->e;
        spsr->a = psr->a;
    }
    if (bytemask & 1) {
        spsr->i = psr->i;
        spsr->f = psr->f;
        spsr->t = psr->t;
        if (!is_good_mode[psr->m])
            abort_unpredictable("", psr->m);
        else
            spsr->m = psr->m;
    }
    return spsr;
};
void ARMV7_CPU::spsr_write_by_instr(Flags *psr, int64_t bytemask)
{
    Flags spsr = get_current_spsr();
    spsr_write_by_instr0(&spsr, psr, bytemask);
    set_current_spsr(spsr);    // XXX
};
int64_t ARMV7_CPU::cpsr_write_by_instr(Flags *psr, int64_t bytemask, bool affect_execstate)
{
    bool    is_pr = is_priviledged();
    int64_t nmfi  = sctlr_get_nmfi() == 1;
    if (bytemask & 8) {
        cpsr.n = psr->n;
        cpsr.z = psr->z;
        cpsr.c = psr->c;
        cpsr.v = psr->v;
        cpsr.q = psr->q;
    }
    if (bytemask & 2) {
        cpsr.e = psr->e;
        if (is_pr && (is_secure() || scr_get_aw() == 1))
            cpsr.a = psr->a;
    }
    if (bytemask & 1) {
        if (is_pr) {
            cpsr.i = psr->i;
        }
        if (is_pr && (is_secure() || scr_get_fw() == 1) && (!nmfi || psr->f == 0))
            cpsr.f = psr->f;
        if (affect_execstate)
            cpsr.t = psr->t;
        if (is_pr) {
            if (!is_good_mode[psr->m])
                abort_unpredictable("", psr->m);
            else {
                if (!is_secure() && psr->m == MON_MODE)
                    abort_unpredictable("", psr->m);
                if (!is_secure() && psr->m == FIQ_MODE && nsacr_get_rfr() == 1)
                    abort_unpredictable("", psr->m);
                if (cpsr.m != psr->m)
                    change_mode(psr->m);
            }
        }
    }
    return 0;
};
int64_t ARMV7_CPU::save_to_regs(int64_t mode)
{
    switch (mode) {
        case USR_MODE:
            regs_usr[13] = regs[13];
            regs_usr[14] = regs[14];
            break;
        case FIQ_MODE:
            regs_fiq[8]  = regs[8];
            regs_fiq[9]  = regs[9];
            regs_fiq[10] = regs[10];
            regs_fiq[11] = regs[11];
            regs_fiq[12] = regs[12];
            regs_fiq[13] = regs[13];
            regs_fiq[14] = regs[14];
            break;
        case IRQ_MODE:
            regs_irq[13] = regs[13];
            regs_irq[14] = regs[14];
            break;
        case SVC_MODE:
            regs_svc[13] = regs[13];
            regs_svc[14] = regs[14];
            break;
        case MON_MODE:
            regs_mon[13] = regs[13];
            regs_mon[14] = regs[14];
            break;
        case ABT_MODE:
            regs_abt[13] = regs[13];
            regs_abt[14] = regs[14];
            break;
        case UND_MODE:
            regs_und[13] = regs[13];
            regs_und[14] = regs[14];
            break;
        case SYS_MODE:
            throw " system";
        default:
            throw " unknown: ";
    }
    return 0;
};
int64_t ARMV7_CPU::restore_from_regs(int64_t mode)
{
    switch (mode) {
        case USR_MODE:
            regs[13] = regs_usr[13];
            regs[14] = regs_usr[14];
            break;
        case FIQ_MODE:
            regs[8]  = regs_fiq[8];
            regs[9]  = regs_fiq[9];
            regs[10] = regs_fiq[10];
            regs[11] = regs_fiq[11];
            regs[12] = regs_fiq[12];
            regs[13] = regs_fiq[13];
            regs[14] = regs_fiq[14];
            break;
        case IRQ_MODE:
            regs[13] = regs_irq[13];
            regs[14] = regs_irq[14];
            break;
        case SVC_MODE:
            regs[13] = regs_svc[13];
            regs[14] = regs_svc[14];
            break;
        case MON_MODE:
            regs[13] = regs_mon[13];
            regs[14] = regs_mon[14];
            break;
        case ABT_MODE:
            regs[13] = regs_abt[13];
            regs[14] = regs_abt[14];
            break;
        case UND_MODE:
            regs[13] = regs_und[13];
            regs[14] = regs_und[14];
            break;
        case SYS_MODE:
            throw "restore_from_regs system";
        default:
            throw "restore_from_regs unknown: ";
    }
    return 0;
};
int64_t ARMV7_CPU::change_mode(int64_t mode)
{
    if (!mode)
        throw "Invalid mode: ";
    save_to_regs(cpsr.m);
    cpsr.m = mode;
    restore_from_regs(cpsr.m);
    return 0;
};
void ARMV7_CPU::set_apsr(int64_t val, bool set_overflow)
{
    int32_t  vali32 = val;
    uint32_t valu32 = val;
    uint64_t valu   = val;
    cpsr.n          = valu32 >> 31;
    cpsr.z          = (val == 0) ? 1 : 0;
    cpsr.c          = carry_out;
    if (set_overflow)
        cpsr.v = overflow;
};
int64_t ARMV7_CPU::coproc_accepted(int64_t cp)
{
    return cp == 15;    // FIXME
};
int64_t ARMV7_CPU::coproc_get_word(uint64_t cp, int64_t inst)
{
    return coprocs[cp]->get_word(inst);
};
void ARMV7_CPU::coproc_send_word(uint64_t cp, int64_t inst, int64_t word)
{
    coprocs[cp]->send_word(inst, word);
};
void ARMV7_CPU::coproc_internal_operation(int64_t cp, int64_t inst)
{
    throw "coproc";
};
int64_t ARMV7_CPU::align(int64_t value, int64_t _align)
{
    return value;
};
void ARMV7_CPU::call_supervisor()
{
    throw 222;
};
bool ARMV7_CPU::allow_unaligned_access()
{
    if (!mmu->check_unaligned)
        return true;
    else
        return false;
};
int64_t ARMV7_CPU::ld_word(int64_t addr)
{
    int64_t phyaddr;
    if (addr & 3) {
        if (!allow_unaligned_access()) {
            throw "Unaligned ld_word: ";
        } else {
            int64_t val = 0;
            for (int64_t i = 0; i < 4; i++) {
                phyaddr = mmu->trans_to_phyaddr(addr + i, false);
                val     = bitops->set_bits(val, 8 * i + 7, 8 * i, memctlr->ld_byte(phyaddr));
            }
            return val;
        }
    } else {
        phyaddr = mmu->trans_to_phyaddr(addr, false);
        return memctlr->ld_word(phyaddr);
    }
};
void ARMV7_CPU::st_word(int64_t addr, int64_t word)
{
    int64_t phyaddr;
    if (addr & 3) {
        if (!allow_unaligned_access()) {
            throw "Unaligned st_word: ";
        } else {
            for (int64_t i = 0; i < 4; i++) {
                phyaddr = mmu->trans_to_phyaddr(addr + i, false);
                memctlr->st_byte(phyaddr, bitops->get_bits(word, 8 * i + 7, 8 * i));
            }
        }
    } else {
        phyaddr = mmu->trans_to_phyaddr(addr, true);
        memctlr->st_word(phyaddr, word);
    }
};
int64_t ARMV7_CPU::ld_halfword(int64_t addr)
{
    int64_t phyaddr;
    if (addr & 1) {
        if (!allow_unaligned_access()) {
            throw "Unaligned ld_halfword: ";
        } else {
            int64_t val = 0;
            for (int64_t i = 0; i < 2; i++) {
                phyaddr = mmu->trans_to_phyaddr(addr + i, false);
                val     = bitops->set_bits(val, 8 * i + 7, 8 * i, memctlr->ld_byte(phyaddr));
            }
            return val;
        }
    } else {
        phyaddr = mmu->trans_to_phyaddr(addr, false);
        return memctlr->ld_halfword(phyaddr);
    }
};
void ARMV7_CPU::st_halfword(int64_t addr, int64_t hw)
{
    int64_t phyaddr;
    if (addr & 1) {
        if (!allow_unaligned_access()) {
            throw "Unaligned st_halfword: ";
        } else {
            for (int64_t i = 0; i < 2; i++) {
                phyaddr = mmu->trans_to_phyaddr(addr + i, false);
                memctlr->st_byte(phyaddr, bitops->get_bits(hw, 8 * i + 7, 8 * i));
            }
        }
    } else {
        phyaddr = mmu->trans_to_phyaddr(addr, true);
        memctlr->st_halfword(phyaddr, hw);
    }
};
int64_t ARMV7_CPU::ld_byte(int64_t addr)
{
    int64_t phyaddr = mmu->trans_to_phyaddr(addr, false);
    return memctlr->ld_byte(phyaddr);
};
void ARMV7_CPU::st_byte(int64_t addr, int64_t b)
{
    int64_t phyaddr = mmu->trans_to_phyaddr(addr, true);
    memctlr->st_byte(phyaddr, b);
};
int64_t ARMV7_CPU::fetch_instruction(int64_t addr)
{
    int64_t phyaddr = mmu->trans_to_phyaddr(addr, false);
    return memctlr->ld_word_fast(phyaddr);
};
int64_t ARMV7_CPU::shift(int64_t value, int64_t type, int64_t amount, int64_t carry_in)
{
    return shift_c(value, type, amount, carry_in);
};
void ARMV7_CPU::decode_imm_shift(int64_t type, int64_t imm5)
{
    switch (type) {
        case 0:
            shift_t = type;
            shift_n = imm5;
            break;
        case 1:
        case 2:
            shift_t = type;
            if (imm5 == 0)
                shift_n = 32;
            else
                shift_n = imm5;
            break;
        case 3:
            if (imm5 == 0) {
                shift_t = type;
                shift_n = 1;
            } else {
                shift_t = SRType_ROR;
                shift_n = imm5;
            }
            break;
        default:
            throw "decode_imm_shift";
    }
};
int64_t ARMV7_CPU::shift_c(int64_t value, int64_t type, int64_t amount, int64_t carry_in)
{
    int64_t  res;
    int64_t  result;
    uint64_t valueu = value;

    if (amount == 0) {
        carry_out = carry_in;
        return value;
    } else {
        switch (type) {
            case 0: {    // LSL
                Number64 *val64    = new Number64(0, value);
                Number64 *extended = val64->lsl(amount);
                carry_out          = extended->high & 1;
                return extended->low;
            }
            case 1: {    // LSR
                carry_out = (amount == 32) ? 0 : ((valueu >> (amount - 1)) & 1);
                result    = bitops->lsr(value, amount);
                return result;
            }
            case 2:    // ASR
                carry_out = (amount == 32) ? 0 : ((valueu >> (amount - 1)) & 1);
                result    = bitops->asr(value, amount);
                return result;
            case 3:    // RRX
                carry_out = value & 1;
                result    = bitops->set_bit(valueu >> 1, 31, carry_in);
                return result;
            case 4:    // ROR
                return ror_c(value, amount, true);
            default:
                throw "shift_c";
        }
    }
};
int64_t ARMV7_CPU::ror_c(int64_t value, int64_t amount, bool write)
{
    uint64_t result = bitops->ror(value, amount);
    if (write)
        carry_out = result >> 31;
    return result;
};
int64_t ARMV7_CPU::ror(int64_t val, int64_t rotation)
{
    if (rotation == 0)
        return val;
    return ror_c(val, rotation, false);
};
int64_t ARMV7_CPU::is_zero_bit(int64_t val)
{
    if (val == 0)
        return 1;
    else
        return 0;
};
int64_t ARMV7_CPU::expand_imm_c(int64_t imm12, int64_t carry_in)
{
    uint64_t imm12u          = imm12;
    int64_t  unrotated_value = imm12 & 0xff;
    int64_t  amount          = 2 * (imm12u >> 8);
    if (!amount) {
        carry_out = carry_in;
        return unrotated_value;
    }
    return ror_c(unrotated_value, amount, true);
};
int64_t ARMV7_CPU::expand_imm(int64_t imm12)
{
    return expand_imm_c(imm12, cpsr.c);
};
int64_t ARMV7_CPU::add_with_carry(int64_t x, int64_t y, int64_t carry_in)
{
    uint64_t yu           = y >> 0;
    uint64_t xu           = x >> 0;
    uint64_t carry_inu    = carry_in;
    uint64_t unsigned_sum = xu + yu + carry_inu;

    int32_t yi        = y >> 0;
    int32_t xi        = x >> 0;
    int32_t carry_ini = carry_in >> 0;

    int64_t yi64        = yi;
    int64_t xi64        = xi;
    int64_t carry_ini64 = carry_ini;

    int64_t tmp        = yi + xi + carry_ini;
    int64_t signed_sum = 0;
    if (0 <= yi && 0 <= xi) {
        signed_sum = x + y + carry_in;
    } else if (yi < 0 && xi < 0) {
        signed_sum = yi64 + xi64 + carry_in;
    } else {
        signed_sum = tmp;
    }
    int64_t result = unsigned_sum % 0x100000000;
    if (result < 0)
        result += 0x100000000;

    int32_t resulti = result >> 0;
    carry_out       = (result == unsigned_sum) ? 0 : 1;
    overflow        = (resulti == signed_sum) ? 0 : 1;

    if (count == 1284037) {
        // fix me
        // uart map not complete
        result = -1;
    }
    return result;
};
int64_t ARMV7_CPU::decode_reg_shift(int64_t type)
{
    shift_t = type;
    return type;
};
bool ARMV7_CPU::is_valid(int64_t inst)
{
    return (inst != 0xe1a00000 && inst != 0);    // NOP or NULL?
};
bool ARMV7_CPU::cond(int64_t inst)
{
    uint64_t instu = inst;
    int64_t  cond  = instu >> 28;
    bool     ret   = false;
    switch (cond >> 1) {
        case 0:
            ret = cpsr.z == 1;    // EQ or NE
            break;
        case 1:
            ret = cpsr.c == 1;    // CS or CC
            break;
        case 2:
            ret = cpsr.n == 1;    // MI or PL
            break;
        case 3:
            ret = cpsr.v == 1;    // VS or VC
            break;
        case 4:
            ret = cpsr.c == 1 && cpsr.z == 0;    // HI or LS
            break;
        case 5:
            ret = cpsr.n == cpsr.v;    // GE or LT
            break;
        case 6:
            ret = cpsr.n == cpsr.v && cpsr.z == 0;    // GT or LE
            break;
        case 7:
            ret = true;    // AL
            break;
        default:
            break;
    }
    if ((cond & 1) && cond != 0xf)
        ret = !ret;
    return ret;
};
void ARMV7_CPU::adc_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm(imm12);
    int64_t  ret   = add_with_carry(reg(n), imm32, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::add_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm(imm12);
    int64_t  ret   = add_with_carry(reg(n), imm32, 0);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::adr_a1(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm(imm12);
    int64_t  ret   = align(get_pc(), 4) + imm32;
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
    }
};
void ARMV7_CPU::adr_a2(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  d     = (inst >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm(imm12);
    int64_t  ret   = align(get_pc(), 4) - imm32;
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
    }
};
void ARMV7_CPU::and_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm_c(imm12, cpsr.c);
    int64_t  valn  = reg(n);
    int64_t  ret   = bitops->and_b(valn, imm32);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::asr_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  m     = inst & 0xf;
    decode_imm_shift(2, imm5);
    int64_t ret = shift_c(reg(m), SRType_ASR, shift_n, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::bic_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  valn  = reg(n);
    int64_t  imm32 = expand_imm_c(imm12, cpsr.c);
    int64_t  ret   = bitops->and_b(valn, bitops->not_b(imm32));
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::b(int64_t inst, int64_t addr)
{
    int64_t imm24 = inst & 0x00ffffff;
    int64_t imm26 = imm24 << 2;
    int32_t imm32 = imm26 >> 0;
    if (imm26 & 0x02000000)
        imm32 = imm26 | 0xfc000000;
    branch_to = get_pc() + imm32;
    if (branch_to >= 0x100000000)
        branch_to -= 0x100000000;
};
void ARMV7_CPU::bl_imm(int64_t inst, int64_t addr)
{

    int64_t  imm24  = inst & 0x00ffffff;
    uint64_t imm24u = imm24;
    int64_t  imm26  = imm24u << 2;
    int64_t  imm32  = imm26;
    if (imm26 & 0x02000000)
        imm32 = imm26 | 0xfc000000;
    regs[14]      = get_pc() - 4;
    uint64_t rval = (get_pc());

    branch_to = align(bitops->lsl(rval >> 2, 2), 4) + imm32;
    if (branch_to >= 0x100000000)
        branch_to -= 0x100000000;
};
void ARMV7_CPU::cmn_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  valn  = reg(n);
    int64_t  imm32 = expand_imm(imm12);
    int64_t  ret   = add_with_carry(valn, imm32, 0);
    set_apsr(ret, true);
};
void ARMV7_CPU::cmp_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  valn  = reg(n);
    int64_t  imm32 = expand_imm(imm12);
    int64_t  ret   = add_with_carry(valn, bitops->not_b(imm32), 1);
    set_apsr(ret, true);
};
void ARMV7_CPU::eor_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm_c(imm12, cpsr.c);
    int64_t  valn  = reg(n);
    int64_t  ret   = bitops->xor_b(valn, imm32);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::ldr_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  p     = (instu >> 24) & 1;
    int64_t  u     = (instu >> 23) & 1;
    int64_t  w     = (instu >> 21) & 1;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  t     = (instu >> 12) & 0xf;
    int64_t  imm12 = (instu & 0xfff);
    if (n == 13 && p == 0 && u == 1 && w == 0 && imm12 == 4) {
        if (t == 15)
            branch_to = ld_word(regs[13]);
        else
            regs[t] = ld_word(regs[13]);
        regs[13] = regs[13] + 4;
        return;
    }
    int64_t imm32       = imm12;
    int64_t is_index    = p == 1;
    int64_t is_add      = u == 1;
    int64_t is_wback    = p == 0 || w == 1;
    int64_t valn        = reg(n);
    int64_t offset_addr = valn + (is_add ? imm32 : -imm32);
    int64_t address     = is_index ? offset_addr : valn;
    int64_t data        = ld_word(address);
    if (is_wback)
        regs[n] = offset_addr;
    if (t == 15)
        branch_to = data;
    else
        regs[t] = data;
};
void ARMV7_CPU::ldrb_imm(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  imm32       = inst & 0xfff;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset_addr = valn + (is_add ? imm32 : -imm32);
    int64_t  address     = is_index ? offset_addr : valn;
    int64_t  data        = ld_byte(address);
    regs[t]              = data;
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::ldrd_imm(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  imm4h       = (instu >> 8) & 0xf;
    int64_t  imm4l       = inst & 0xf;
    int64_t  t2          = t + 1;
    int64_t  imm32       = (imm4h << 4) + imm4l;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset_addr = valn + (is_add ? imm32 : -imm32);
    int64_t  address     = is_index ? offset_addr : valn;
    regs[t]              = ld_word(address);
    regs[t2]             = ld_word(address + 4);
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::ldrsh_imm(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  imm4h       = (instu >> 8) & 0xf;
    int64_t  imm4l       = inst & 0xf;
    int64_t  imm32       = (imm4h << 4) + imm4l;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset_addr = valn + (is_add ? imm32 : -imm32);
    int64_t  address     = is_index ? offset_addr : valn;
    int64_t  data        = ld_halfword(address);
    if (is_wback)
        regs[n] = offset_addr;
    regs[t] = bitops->sign_extend(data, 16, 32);
};
void ARMV7_CPU::ldrsh_reg(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  m           = inst & 0xf;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset      = shift(reg(m), SRType_LSL, 0, cpsr.c);
    int64_t  offset_addr = valn + (is_add ? offset : -offset);
    int64_t  address     = is_index ? offset_addr : valn;
    int64_t  data        = ld_halfword(address);
    if (is_wback)
        regs[n] = offset_addr;
    regs[t] = bitops->sign_extend(data, 16, 32);
};
void ARMV7_CPU::lsl_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  m     = inst & 0xf;
    int64_t  valm  = reg(m);
    decode_imm_shift(0, imm5);
    int64_t ret = shift_c(valm, SRType_LSL, shift_n, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::lsr_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  m     = inst & 0xf;
    int64_t  valm  = reg(m);
    decode_imm_shift(1, imm5);
    int64_t ret = shift_c(valm, SRType_LSR, shift_n, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::mov_imm_a1(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm_c(imm12, cpsr.c);
    int64_t  ret   = imm32;
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::mov_imm_a2(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  imm4  = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = (imm4 << 12) + imm12;
    int64_t  ret   = imm32;
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
    }
};
void ARMV7_CPU::movt(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  imm4  = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm16 = (imm4 << 12) + imm12;
    regs[d]        = bitops->set_bits(reg(d), 16, 31, imm16);
};
void ARMV7_CPU::msr_imm_sys(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  r     = inst & (1 << 22);
    int64_t  mask  = (instu >> 16) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm(imm12);
    Flags    flgp  = parse_psr(imm32);
    if (r) {
        spsr_write_by_instr(&flgp, mask);
    } else {
        cpsr_write_by_instr(&flgp, mask, false);
    }
};
void ARMV7_CPU::mvn_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm_c(imm12, cpsr.c);
    int64_t  ret   = bitops->not_b(imm32);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::orr_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  valn  = reg(n);
    int64_t  imm32 = expand_imm_c(imm12, cpsr.c);
    int64_t  ret   = bitops->or_b(valn, imm32);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::pld_imm(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  u       = (instu >> 23) & 1;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  imm12   = inst & 0xfff;
    int64_t  valn    = reg(n);
    int64_t  imm32   = imm12;
    int64_t  is_add  = u == 1;
    int64_t  base    = (n == 15) ? align(get_pc(), 4) : valn;
    int64_t  address = base + (is_add ? imm32 : -imm32);
};
void ARMV7_CPU::rsb_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm(imm12);
    int64_t  valn  = reg(n);
    int64_t  ret   = add_with_carry(bitops->not_b(valn), imm32, 1);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::rsc_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm(imm12);
    int64_t  valn  = reg(n);
    int64_t  ret   = add_with_carry(bitops->not_b(valn), imm32, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::ror_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  m     = inst & 0xf;
    int64_t  valm  = reg(m);
    decode_imm_shift(3, imm5);
    int64_t ret = shift_c(valm, SRType_ROR, shift_n, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::rrx(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  m     = inst & 0xf;
    int64_t  valm  = reg(m);
    int64_t  ret   = shift_c(valm, SRType_RRX, 1, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::sbc_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm(imm12);
    int64_t  valn  = reg(n);
    int64_t  ret   = add_with_carry(valn, bitops->not_b(imm32), cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::str_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  p     = (instu >> 24) & 1;
    int64_t  u     = (instu >> 23) & 1;
    int64_t  w     = (instu >> 21) & 1;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  t     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  address;
    if (n == 13 && p == 1 && u == 0 && w == 1 && imm12 == 4) {
        int64_t sp = reg(13);
        address    = sp - 4;
        st_word(address, reg(t));
        regs[13] = sp - 4;
        return;
    }
    int64_t imm32       = imm12;
    int64_t is_index    = p == 1;
    int64_t is_add      = u == 1;
    int64_t is_wback    = p == 0 || w == 1;
    int64_t valn        = reg(n);
    int64_t offset_addr = valn + (is_add ? imm32 : -imm32);
    address             = is_index ? offset_addr : valn;
    int64_t valt        = reg(t);
    st_word(address, valt);
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::strb_imm(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  imm32       = inst & 0xfff;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset_addr = valn + (is_add ? imm32 : -imm32);
    int64_t  address     = is_index ? offset_addr : valn;
    st_byte(address, reg(t) & 0xff);
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::sub_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  imm32 = expand_imm(imm12);
    int64_t  ret   = add_with_carry(reg(n), bitops->not_b(imm32), 1);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::teq_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  valn  = reg(n);
    int64_t  imm32 = expand_imm_c(imm12, cpsr.c);
    int64_t  ret   = bitops->xor_b(valn, imm32);
    set_apsr(ret, false);
};
void ARMV7_CPU::tst_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  imm12 = inst & 0xfff;
    int64_t  valn  = reg(n);
    int64_t  imm32 = expand_imm_c(imm12, cpsr.c);
    int64_t  ret   = bitops->and_b(valn, imm32);
    set_apsr(ret, false);
};
void ARMV7_CPU::ldr_lit(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  u       = inst & (1 << 23);
    int64_t  t       = (instu >> 12) & 0xf;
    int64_t  imm32   = inst & 0xfff;
    int64_t  base    = align(get_pc(), 4);
    int64_t  address = base + (u ? imm32 : -imm32);
    int64_t  data    = ld_word(address);
    if (t == 15)
        branch_to = data;
    else
        regs[t] = data;
};
void ARMV7_CPU::adc_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = add_with_carry(valn, shifted, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::add_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = add_with_carry(valn, shifted, 0);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::and_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift_c(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = bitops->and_b(valn, shifted);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::asr_reg(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  s       = inst & 0x00100000;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  m       = (instu >> 8) & 0xf;
    int64_t  n       = inst & 0xf;
    int64_t  shift_n = bitops->get_bits(reg(m), 7, 0);
    int64_t  ret     = shift_c(reg(n), SRType_ASR, shift_n, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::bic_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift_c(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = bitops->and_b(valn, bitops->not_b(shifted));
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::bfc(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  msbit = (instu >> 16) & 0x1f;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  lsbit = (instu >> 7) & 0x1f;
    if (msbit >= lsbit)
        regs[d] = bitops->clear_bits(regs[d], msbit, lsbit);
    else
        abort_unpredictable("BFC", inst);
};
void ARMV7_CPU::bfi(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  msbit = (instu >> 16) & 0x1f;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  lsbit = (instu >> 7) & 0x1f;
    int64_t  n     = inst & 0xf;
    if (msbit >= lsbit)
        regs[d] = bitops->set_bits(regs[d], msbit, lsbit, bitops->get_bits(reg(n), msbit - lsbit, 0));
    else
        abort_unpredictable("BFI", inst);
};
void ARMV7_CPU::blx_reg(int64_t inst, int64_t addr)
{
    int64_t m               = inst & 0xf;
    int64_t next_instr_addr = get_pc() - 4;
    regs[14]                = next_instr_addr;
    branch_to               = reg(m);
};
void ARMV7_CPU::bx(int64_t inst, int64_t addr)
{
    int64_t m = inst & 0xf;
    branch_to = reg(m);
};
void ARMV7_CPU::cdp_a1(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  t     = (instu >> 12) & 0xf;
    int64_t  cp    = (instu >> 8) & 0xf;
    if ((cp >> 1) == 5) {
        abort_simdvfp_inst(inst, addr);
    }
    if (!coproc_accepted(cp)) {
        throw "GenerateCoprocessorException(): ";
    } else {
        coproc_internal_operation(cp, inst);
    }
};
void ARMV7_CPU::clz(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  m     = inst & 0xf;
    regs[d]        = bitops->count_leading_zero_bits(reg(m));
};
void ARMV7_CPU::cmn_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = add_with_carry(valn, shifted, 0);
    set_apsr(ret, true);
};
void ARMV7_CPU::cmp_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = add_with_carry(valn, bitops->not_b(shifted), 1);
    set_apsr(ret, true);
};
void ARMV7_CPU::eor_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift_c(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = bitops->xor_b(valn, shifted);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::ldr_reg(int64_t inst, int64_t addr)
{
    uint64_t instu    = inst;
    int64_t  p        = (instu >> 24) & 1;
    int64_t  u        = (instu >> 23) & 1;
    int64_t  w        = (instu >> 21) & 1;
    int64_t  n        = (instu >> 16) & 0xf;
    int64_t  t        = (instu >> 12) & 0xf;
    int64_t  imm5     = (instu >> 7) & 0x1f;
    int64_t  type     = (instu >> 5) & 3;
    int64_t  m        = inst & 0xf;
    int64_t  is_index = p == 1;
    int64_t  is_add   = u == 1;
    int64_t  is_wback = p == 0 || w == 1;
    int64_t  valn     = reg(n);
    decode_imm_shift(type, imm5);
    int64_t offset      = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t offset_addr = valn + (is_add ? offset : -offset);
    int64_t address     = is_index ? offset_addr : valn;
    address             = bitops->get_bits64(address, 31, 0);    // XXX
    int64_t data        = ld_word(address);
    if (is_wback)
        regs[n] = offset_addr;
    if (t == 15)
        branch_to = data;
    else
        regs[t] = data;
};
void ARMV7_CPU::ldrb_reg(int64_t inst, int64_t addr)
{
    uint64_t instu    = inst;
    int64_t  p        = (instu >> 24) & 1;
    int64_t  u        = (instu >> 23) & 1;
    int64_t  w        = (instu >> 21) & 1;
    int64_t  n        = (instu >> 16) & 0xf;
    int64_t  t        = (instu >> 12) & 0xf;
    int64_t  imm5     = (instu >> 7) & 0x1f;
    int64_t  type     = (instu >> 5) & 3;
    int64_t  m        = inst & 0xf;
    int64_t  is_index = p == 1;
    int64_t  is_add   = u == 1;
    int64_t  is_wback = p == 0 || w == 1;
    decode_imm_shift(type, imm5);
    int64_t valn        = reg(n);
    int64_t offset      = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t offset_addr = valn + (is_add ? offset : -offset);
    int64_t address     = is_index ? offset_addr : valn;
    int64_t data        = ld_byte(address);
    regs[t]             = data;
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::ldrd_reg(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  m           = inst & 0xf;
    int64_t  t2          = t + 1;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  valm        = reg(m);
    int64_t  offset_addr = valn + (is_add ? valm : -valm);
    int64_t  address     = is_index ? offset_addr : valn;
    regs[t]              = ld_word(address);
    regs[t2]             = ld_word(address + 4);
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::ldrex(int64_t inst, int64_t addr)
{
    int64_t n       = bitops->get_bits(inst, 19, 16);
    int64_t t       = bitops->get_bits(inst, 15, 12);
    int64_t imm32   = 0;
    int64_t address = reg(n) + imm32;
    regs[t]         = ld_word(address);
};
void ARMV7_CPU::ldrexd(int64_t inst, int64_t addr)
{
    int64_t n       = bitops->get_bits(inst, 19, 16);
    int64_t t       = bitops->get_bits(inst, 15, 12);
    int64_t t2      = t + 1;
    int64_t address = reg(n);
    regs[t]         = ld_word(address);
    regs[t2]        = ld_word(address + 4);
};
void ARMV7_CPU::ldrt_a1(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  u       = (instu >> 23) & 1;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  t       = (instu >> 12) & 0xf;
    int64_t  imm32   = inst & 0xfff;
    int64_t  is_add  = u == 1;
    int64_t  valn    = reg(n);
    int64_t  offset  = imm32;
    int64_t  address = valn;
    address          = bitops->get_bits64(address, 31, 0);    // XXX
    int64_t data     = ld_word(address);
    if (t == 15)
        branch_to = data;
    else
        regs[t] = data;
};
void ARMV7_CPU::lsl_reg(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  s       = inst & 0x00100000;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  m       = (instu >> 8) & 0xf;
    int64_t  n       = inst & 0xf;
    int64_t  shift_n = bitops->get_bits(reg(m), 7, 0);
    int64_t  ret     = shift_c(reg(n), SRType_LSL, shift_n, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::lsr_reg(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  s       = inst & 0x00100000;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  m       = (instu >> 8) & 0xf;
    int64_t  n       = inst & 0xf;
    int64_t  shift_n = bitops->get_bits(reg(m), 7, 0);
    int64_t  ret     = shift_c(reg(n), SRType_LSR, shift_n, cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::mcr_a1(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  t     = (instu >> 12) & 0xf;
    int64_t  cp    = (instu >> 8) & 0xf;
    if ((cp >> 1) == 5) {
        abort_simdvfp_inst(inst, addr);
    }
    if (!coproc_accepted(cp)) {
        throw "GenerateCoprocessorException()";
    } else {
        coproc_send_word(cp, inst, regs[t]);
    }
};
void ARMV7_CPU::mla(int64_t inst, int64_t addr)
{
    uint64_t  instu      = inst;
    int64_t   s          = inst & 0x00100000;
    int64_t   d          = (instu >> 16) & 0xf;
    int64_t   a          = (instu >> 12) & 0xf;
    int64_t   m          = (instu >> 8) & 0xf;
    int64_t   n          = inst & 0xf;
    int64_t   ope1       = reg(n);
    int64_t   ope2       = reg(m);
    int64_t   addend     = reg(a);
    Number64 *n64_ope1   = new Number64(0, ope1);
    Number64 *n64_ope2   = new Number64(0, ope2);
    Number64 *n64_addend = new Number64(0, addend);
    Number64 *n64        = n64_ope1->mul_n(n64_ope2);
    Number64 *ret        = n64->add_n(n64_addend);
    regs[d]              = ret->low;
    if (s) {
        uint64_t lowu = ret->low;
        cpsr.n        = (lowu >> 31) & 1;
        cpsr.z        = (ret == 0) ? 1 : 0;
    }
    delete n64_ope1;
    delete n64_ope2;
    delete n64_addend;
    delete n64;
    delete ret;
};
void ARMV7_CPU::mls(int64_t inst, int64_t addr)
{
    uint64_t  instu      = inst;
    int64_t   d          = (instu >> 16) & 0xf;
    int64_t   a          = (instu >> 12) & 0xf;
    int64_t   m          = (instu >> 8) & 0xf;
    int64_t   n          = inst & 0xf;
    int64_t   ope1       = reg(n);
    int64_t   ope2       = reg(m);
    int64_t   addend     = reg(a);
    Number64 *n64_ope1   = new Number64(0, ope1);
    Number64 *n64_ope2   = new Number64(0, ope2);
    Number64 *n64_addend = new Number64(0, addend);
    Number64 *n64        = n64_ope1->mul_n(n64_ope2);
    Number64 *ret        = n64_addend->sub_n(n64);
    regs[d]              = ret->low;
    delete n64_ope1;
    delete n64_ope2;
    delete n64_addend;
    delete n64;
    delete ret;
};
void ARMV7_CPU::subs_pc_lr_a2(int64_t inst, int64_t addr)
{
    uint64_t instu  = inst;
    int64_t  opcode = (instu >> 21) & 0xf;
    int64_t  n      = (instu >> 16) & 0xf;
    int64_t  imm5   = (instu >> 7) & 0x1f;
    int64_t  type   = (instu >> 5) & 3;
    int64_t  m      = inst & 0xf;
    decode_imm_shift(type, imm5);
    int64_t operand2 = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t ret;
    switch (opcode) {
        case 0:
            ret = bitops->and_b(reg(n), operand2);
            break;
        case 1:
            ret = bitops->xor_b(reg(n), operand2);
            break;
        case 2:
            ret = add_with_carry(reg(n), bitops->not_b(operand2), 1);
            break;
        case 3:
            ret = add_with_carry(bitops->not_b(reg(n)), operand2, 1);
            break;
        case 4:
            ret = add_with_carry(reg(n), operand2, 0);
            break;
        case 5:
            ret = add_with_carry(reg(n), operand2, cpsr.c);
            break;
        case 6:
            ret = add_with_carry(reg(n), bitops->not_b(operand2), cpsr.c);
            break;
        case 7:
            ret = add_with_carry(bitops->not_b(reg(n)), operand2, cpsr.c);
            break;
        case 0xc:
            ret = bitops->or_b(reg(n), operand2);
            break;
        case 0xd:
            ret = operand2;
            break;
        case 0xe:
            ret = bitops->and_b(reg(n), bitops->not_b(operand2));
            break;
        case 0xf:
            ret = bitops->not_b(operand2);
            break;
        default:
            throw "subs_pc_lr_a2: unknown opcode";
            break;
    }
    auto flgobj = get_current_spsr();
    cpsr_write_by_instr(&flgobj, 15, true);
    branch_to = ret;
};
void ARMV7_CPU::mov_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  m     = inst & 0xf;
    if (d == 15 && s) {
        subs_pc_lr_a2(inst, addr);
        return;
    }
    int64_t  ret  = reg(m);
    uint64_t retu = ret;
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s) {
            cpsr.n = retu >> 31;
            cpsr.z = (ret == 0) ? 1 : 0;
        }
    }
};
void ARMV7_CPU::mrc_a1(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  t     = (instu >> 12) & 0xf;
    int64_t  cp    = (instu >> 8) & 0xf;
    if ((cp >> 1) == 5) {
        abort_simdvfp_inst(inst, addr);
    }
    if (!coproc_accepted(cp)) {
        throw "GenerateCoprocessorException()";
    } else {
        int64_t  value  = coproc_get_word(cp, inst);
        uint64_t valueu = value;
        if (t != 15) {
            regs[t] = value;
        } else {
            cpsr.n = (valueu >> 31) & 1;
            cpsr.z = (valueu >> 30) & 1;
            cpsr.c = (valueu >> 29) & 1;
            cpsr.v = (valueu >> 28) & 1;
        }
    }
};
void ARMV7_CPU::mrs(int64_t inst, int64_t addr)
{
    uint64_t instu     = inst;
    int64_t  read_spsr = inst & (1 << 22);
    int64_t  d         = (instu >> 12) & 0xf;
    if (read_spsr) {
        if (is_user_or_system())
            abort_unpredictable("MRS", inst);
        else {
            auto flgobj = get_current_spsr();
            regs[d]     = psr_to_value(&flgobj);
        }
    } else {
        regs[d] = bitops->and_b(psr_to_value(&cpsr), 0xf8ff03df);
    }
};
void ARMV7_CPU::msr_reg_sys(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  r     = inst & (1 << 22);
    int64_t  mask  = (instu >> 16) & 0xf;
    int64_t  n     = inst & 0xf;
    if (r) {
        auto flgobj = parse_psr(reg(n));
        spsr_write_by_instr(&flgobj, mask);
    } else {
        auto flgobj = parse_psr(reg(n));
        cpsr_write_by_instr(&flgobj, mask, false);
    }
};
void ARMV7_CPU::mul(int64_t inst, int64_t addr)
{
    uint64_t  instu    = inst;
    int64_t   s        = inst & 0x00100000;
    int64_t   d        = (instu >> 16) & 0xf;
    int64_t   m        = (instu >> 8) & 0xf;
    int64_t   n        = inst & 0xf;
    int64_t   ope1     = reg(n);
    int64_t   ope2     = reg(m);
    Number64 *n64_ope1 = new Number64(0, ope1);
    Number64 *n64_ope2 = new Number64(0, ope2);
    Number64 *ret      = n64_ope1->mul_n(n64_ope2);
    regs[d]            = ret->low;
    if (s) {
        uint64_t lowu = ret->low;
        cpsr.n        = lowu >> 31;
        cpsr.z        = (ret == 0) ? 1 : 0;
    }
    delete n64_ope1;
    delete n64_ope2;
    delete ret;
};
void ARMV7_CPU::mvn_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift_c(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = bitops->not_b(shifted);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::orr_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift_c(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = bitops->or_b(valn, shifted);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::rev(int64_t inst, int64_t addr)
{
    int64_t d    = bitops->get_bits(inst, 15, 12);
    int64_t m    = bitops->get_bits(inst, 3, 0);
    int64_t valm = reg(m);
    int64_t ret  = 0;
    ret          = bitops->set_bits(ret, 31, 24, bitops->get_bits(valm, 7, 0));
    ret          = bitops->set_bits(ret, 23, 16, bitops->get_bits(valm, 15, 8));
    ret          = bitops->set_bits(ret, 15, 8, bitops->get_bits(valm, 23, 16));
    ret          = bitops->set_bits(ret, 7, 0, bitops->get_bits(valm, 31, 24));
    regs[d]      = ret;
};
void ARMV7_CPU::rev16(int64_t inst, int64_t addr)
{
    int64_t d    = bitops->get_bits(inst, 15, 12);
    int64_t m    = bitops->get_bits(inst, 3, 0);
    int64_t valm = reg(m);
    int64_t ret  = 0;
    ret          = bitops->set_bits(ret, 31, 24, bitops->get_bits(valm, 23, 16));
    ret          = bitops->set_bits(ret, 23, 16, bitops->get_bits(valm, 31, 24));
    ret          = bitops->set_bits(ret, 15, 8, bitops->get_bits(valm, 7, 0));
    ret          = bitops->set_bits(ret, 7, 0, bitops->get_bits(valm, 15, 8));
    regs[d]      = ret;
};
void ARMV7_CPU::rsb_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = add_with_carry(bitops->not_b(valn), shifted, 1);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::sbc_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = add_with_carry(valn, bitops->not_b(shifted), cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::sbfx(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  widthminus1 = (instu >> 16) & 0x1f;
    int64_t  d           = (instu >> 12) & 0xf;
    int64_t  lsbit       = (instu >> 7) & 0x1f;
    int64_t  n           = inst & 0xf;
    int64_t  msbit       = lsbit + widthminus1;
    if (msbit <= 31)
        regs[d] = bitops->sign_extend(bitops->get_bits(reg(n), msbit, lsbit), msbit - lsbit + 1, 32);
    else
        abort_unpredictable("SBFX", inst);
};
void ARMV7_CPU::smlal(int64_t inst, int64_t addr)
{
    uint64_t  instu = inst;
    int64_t   s     = inst & 0x00100000;
    int64_t   dhi   = (instu >> 16) & 0xf;
    int64_t   dlo   = (instu >> 12) & 0xf;
    int64_t   m     = (instu >> 8) & 0xf;
    int64_t   n     = inst & 0xf;
    Number64 *n64_n = new Number64(0, reg(n));
    Number64 *n64_m = new Number64(0, reg(m));
    Number64 *n64   = new Number64(reg(dhi), reg(dlo));
    Number64 *_ret  = n64_n->mul_n(n64_m);
    Number64 *ret   = _ret->add_n(n64);
    regs[dhi]       = ret->high;
    regs[dlo]       = ret->low;
    if (s) {
        cpsr.n = bitops->get_bit(ret->high, 31, 0);
        cpsr.z = ret->is_zero() ? 1 : 0;
    }
    delete n64_n;
    delete n64_m;
    delete n64;
    delete _ret;
    delete ret;
};
void ARMV7_CPU::smull(int64_t inst, int64_t addr)
{
    uint64_t  instu = inst;
    int64_t   s     = inst & 0x00100000;
    int64_t   dhi   = (instu >> 16) & 0xf;
    int64_t   dlo   = (instu >> 12) & 0xf;
    int64_t   m     = (instu >> 8) & 0xf;
    int64_t   n     = inst & 0xf;
    Number64 *n64_n = new Number64(0, reg(n));
    Number64 *n64_m = new Number64(0, reg(m));
    Number64 *ret   = n64_n->mul_n(n64_m);
    regs[dhi]       = ret->high;
    regs[dlo]       = ret->low;
    if (s) {
        cpsr.n = bitops->get_bit(ret->high, 31, 0);
        cpsr.z = ret->is_zero() ? 1 : 0;
    }
    delete n64_n;
    delete n64_m;
    delete ret;
};
void ARMV7_CPU::swp(int64_t inst, int64_t addr)
{
    int64_t B  = (inst >> 22) & 0x1;
    int64_t Rn = (inst >> 16) & 0xF;
    int64_t Rd = (inst >> 12) & 0xF;
    int64_t Rm = inst & 0xF;

    int64_t valn    = reg(Rn);
    int64_t valm    = reg(Rm);
    int64_t address = valn;

    if (B) {
        int64_t data = ld_byte(address);
        st_byte(address, bitops->get_bits(valm, 7, 0));
        regs[Rd] = data;
    } else {
        int64_t data = ld_word(address);
        st_word(address, valm);
        regs[Rd] = data;
    }
};
void ARMV7_CPU::strex(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  t       = inst & 0xf;
    int64_t  imm32   = 0;
    int64_t  address = reg(n) + imm32;
    st_word(address, reg(t));
    regs[d] = 0;
};
void ARMV7_CPU::strexd(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  t       = inst & 0xf;
    int64_t  t2      = t + 1;
    int64_t  address = reg(n);
    st_word(address, reg(t));
    st_word(address + 4, reg(t2));
    regs[d] = 0;
};
void ARMV7_CPU::sub_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  s     = inst & 0x00100000;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  d     = (instu >> 12) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = add_with_carry(valn, bitops->not_b(shifted), 1);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (s)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::sxtb(int64_t inst, int64_t addr)
{
    uint64_t instu    = inst;
    int64_t  d        = (instu >> 12) & 0xf;
    int64_t  m        = inst & 0xf;
    int64_t  rotation = ((instu >> 10) & 3) << 3;
    int64_t  rotated  = ror(reg(m), rotation);
    regs[d]           = bitops->sign_extend(bitops->get_bits64(rotated, 7, 0), 8, 32);
};
void ARMV7_CPU::sxth(int64_t inst, int64_t addr)
{
    uint64_t instu    = inst;
    int64_t  d        = (instu >> 12) & 0xf;
    int64_t  m        = inst & 0xf;
    int64_t  rotation = ((instu >> 10) & 3) << 3;
    int64_t  rotated  = ror(reg(m), rotation);
    regs[d]           = bitops->sign_extend(bitops->get_bits64(rotated, 15, 0), 16, 32);
};
void ARMV7_CPU::sxtah(int64_t inst, int64_t addr)
{
    uint64_t  instu    = inst;
    int64_t   n        = (instu >> 16) & 0xf;
    int64_t   d        = (instu >> 12) & 0xf;
    int64_t   m        = inst & 0xf;
    int64_t   rotation = ((instu >> 10) & 3) << 3;
    int64_t   rotated  = ror(reg(m), rotation);
    Number64 *n64      = new Number64(0, reg(n));
    int64_t   bval     = bitops->get_bits64(rotated, 15, 0);
    int64_t   beval    = bitops->sign_extend(bval, 16, 32);
    //
    Number64 *nbv  = new Number64(0, beval);
    Number64 *nval = n64->add_n(nbv);
    regs[d]        = nval->low;
    delete n64;
    delete nbv;
    delete nval;
};
void ARMV7_CPU::teq_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    int64_t  valn  = reg(n);
    int64_t  valm  = reg(m);
    decode_imm_shift(type, imm5);
    int64_t shifted = shift(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = bitops->xor_b(valn, shifted);
    set_apsr(ret, false);
};
void ARMV7_CPU::tst_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  n     = (instu >> 16) & 0xf;
    int64_t  imm5  = (instu >> 7) & 0x1f;
    int64_t  type  = (instu >> 5) & 3;
    int64_t  m     = inst & 0xf;
    decode_imm_shift(type, imm5);
    int64_t valn    = reg(n);
    int64_t valm    = reg(m);
    int64_t shifted = shift_c(valm, shift_t, shift_n, cpsr.c);
    int64_t ret     = bitops->and_b(valn, shifted);
    set_apsr(ret, false);
};
void ARMV7_CPU::ubfx(int64_t inst, int64_t addr)
{
    int64_t widthminus1 = bitops->get_bits(inst, 20, 16);
    int64_t d           = bitops->get_bits(inst, 15, 12);
    int64_t lsbit       = bitops->get_bits(inst, 11, 7);
    int64_t n           = bitops->get_bits(inst, 3, 0);
    int64_t msbit       = lsbit + widthminus1;
    if (msbit <= 31)
        regs[d] = bitops->get_bits(reg(n), msbit, lsbit);
    else
        abort_unpredictable("UBFX", inst);
};
void ARMV7_CPU::umlal(int64_t inst, int64_t addr)
{
    int64_t s   = inst & 0x00100000;
    int64_t dhi = bitops->get_bits(inst, 19, 16);
    int64_t dlo = bitops->get_bits(inst, 15, 12);
    int64_t m   = bitops->get_bits(inst, 11, 8);
    int64_t n   = bitops->get_bits(inst, 3, 0);

    Number64 *n64_n = new Number64(0, reg(n));
    Number64 *n64_m = new Number64(0, reg(m));
    Number64 *n64_d = new Number64(reg(dhi), reg(dlo));
    Number64 *nval  = n64_n->mul_n(n64_m);
    Number64 *ret   = nval->add_n(n64_d);
    regs[dhi]       = ret->high;
    regs[dlo]       = ret->low;
    if (s) {
        cpsr.n = bitops->get_bit(ret->high, 31, 0);
        cpsr.z = ret->is_zero() ? 1 : 0;
    }
    delete n64_n;
    delete n64_m;
    delete n64_d;
    delete nval;
    delete ret;
};
void ARMV7_CPU::umull(int64_t inst, int64_t addr)
{
    int64_t   s     = inst & 0x00100000;
    int64_t   dhi   = bitops->get_bits(inst, 19, 16);
    int64_t   dlo   = bitops->get_bits(inst, 15, 12);
    int64_t   m     = bitops->get_bits(inst, 11, 8);
    int64_t   n     = bitops->get_bits(inst, 3, 0);
    Number64 *n64_n = new Number64(0, reg(n));
    Number64 *n64_m = new Number64(0, reg(m));
    Number64 *ret   = n64_n->mul_n(n64_m);
    regs[dhi]       = ret->high;
    regs[dlo]       = ret->low;
    if (s) {
        cpsr.n = bitops->get_bit(ret->high, 31, 0);
        cpsr.z = ret->is_zero() ? 1 : 0;
    }
    delete n64_n;
    delete n64_m;
    delete ret;
};
int64_t ARMV7_CPU::unsigned_satq(int64_t i, int64_t n)
{
    int64_t ret;
    if (i > (pow(2, n) - 1)) {
        ret       = pow(2, n) - 1;
        saturated = true;
    } else if (i < 0) {
        ret       = 0;
        saturated = true;
    } else {
        ret       = i;
        saturated = false;
    }
    return bitops->get_bits64(ret, 31, 0);
};
void ARMV7_CPU::usat(int64_t inst, int64_t addr)
{
    int64_t saturate_to = bitops->get_bits(inst, 20, 16);
    int64_t d           = bitops->get_bits(inst, 15, 12);
    int64_t imm5        = bitops->get_bits(inst, 11, 7);
    int64_t sh          = bitops->get_bit(inst, 6, 0);
    int64_t n           = bitops->get_bits(inst, 3, 0);
    decode_imm_shift(sh << 1, imm5);
    int64_t operand = shift(reg(n), shift_t, shift_n, cpsr.c);
    int64_t ret     = unsigned_satq(bitops->sint32(operand), saturate_to);
    regs[n]         = ret;
    if (saturated)
        cpsr.q = 1;
};
void ARMV7_CPU::uxtab(int64_t inst, int64_t addr)
{
    uint64_t instu    = inst;
    int64_t  n        = (instu >> 16) & 0xf;
    int64_t  d        = (instu >> 12) & 0xf;
    int64_t  rotation = ((instu >> 10) & 3) << 3;
    int64_t  m        = inst & 0xf;
    int64_t  rotated  = ror(reg(m), rotation);
    regs[d]           = reg(n) + bitops->get_bits64(rotated, 7, 0);
};
void ARMV7_CPU::uxtah(int64_t inst, int64_t addr)
{
    uint64_t instu    = inst;
    int64_t  n        = (instu >> 16) & 0xf;
    int64_t  d        = (instu >> 12) & 0xf;
    int64_t  m        = inst & 0xf;
    int64_t  rotation = ((instu >> 10) & 3) << 3;
    int64_t  rotated  = ror(reg(m), rotation);
    regs[d]           = reg(n) + bitops->get_bits64(rotated, 15, 0);
};
void ARMV7_CPU::uxtb(int64_t inst, int64_t addr)
{
    uint64_t instu    = inst;
    int64_t  d        = (instu >> 12) & 0xf;
    int64_t  m        = inst & 0xf;
    int64_t  rotation = ((instu >> 10) & 3) << 3;
    int64_t  rotated  = ror(reg(m), rotation);
    regs[d]           = bitops->get_bits64(rotated, 7, 0);
};
void ARMV7_CPU::uxth(int64_t inst, int64_t addr)
{
    uint64_t instu    = inst;
    int64_t  d        = (instu >> 12) & 0xf;
    int64_t  m        = inst & 0xf;
    int64_t  rotation = ((instu >> 10) & 3) << 3;
    int64_t  rotated  = ror(reg(m), rotation);
    regs[d]           = bitops->get_bits64(rotated, 15, 0);
};
void ARMV7_CPU::add_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  sf      = inst & 0x00100000;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  s       = (instu >> 8) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = add_with_carry(reg(n), shifted, 0);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (sf)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::and_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  sf      = inst & 0x00100000;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  s       = (instu >> 8) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift_c(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = bitops->and_b(reg(n), shifted);
    regs[d]          = ret;
    if (sf)
        set_apsr(ret, false);
};
void ARMV7_CPU::bic_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  sf      = inst & 0x00100000;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  s       = (instu >> 8) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift_c(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = bitops->and_b(reg(n), bitops->not_b(shifted));
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (sf)
            set_apsr(ret, false);
    }
};
void ARMV7_CPU::cmp_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  s       = (instu >> 8) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = add_with_carry(reg(n), bitops->not_b(shifted), 1);
    set_apsr(ret, true);
};
void ARMV7_CPU::eor_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  sf      = inst & 0x00100000;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  s       = (instu >> 8) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift_c(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = bitops->xor_b(reg(n), shifted);
    regs[d]          = ret;
    if (sf)
        set_apsr(ret, false);
};
void ARMV7_CPU::mvn_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  sf      = inst & 0x00100000;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  s       = (instu >> 8) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift_c(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = bitops->not_b(shifted);
    regs[d]          = ret;
    if (sf)
        set_apsr(ret, false);
};
void ARMV7_CPU::orr_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  sf      = inst & 0x00100000;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  s       = (instu >> 8) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift_c(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = bitops->or_b(reg(n), shifted);
    regs[d]          = ret;
    if (sf)
        set_apsr(ret, false);
};
void ARMV7_CPU::rsb_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  sf      = inst & 0x00100000;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  s       = (instu >> 8) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = add_with_carry(bitops->not_b(reg(n)), shifted, 1);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (sf)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::sbc_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  sf      = inst & 0x00100000;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  s       = (instu >> 8) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = add_with_carry(reg(n), bitops->not_b(shifted), cpsr.c);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (sf)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::sub_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  sf      = inst & 0x00100000;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  d       = (instu >> 12) & 0xf;
    int64_t  s       = (instu >> 8) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = add_with_carry(reg(n), bitops->not_b(shifted), 1);
    if (d == 15) {
        branch_to = ret;
    } else {
        regs[d] = ret;
        if (sf)
            set_apsr(ret, true);
    }
};
void ARMV7_CPU::tst_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  s       = inst & 0x00100000;
    int64_t  n       = (instu >> 16) & 0xf;
    int64_t  type    = (instu >> 5) & 3;
    int64_t  m       = inst & 0xf;
    int64_t  shift_t = decode_reg_shift(type);
    int64_t  shift_n = bitops->get_bits(reg(s), 7, 0);
    int64_t  shifted = shift_c(reg(m), shift_t, shift_n, cpsr.c);
    int64_t  ret     = bitops->and_b(reg(n), shifted);
    set_apsr(ret, false);
};
void ARMV7_CPU::ldrh_imm(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  imm4h       = (instu >> 8) & 0xf;
    int64_t  imm4l       = inst & 0xf;
    int64_t  imm32       = (imm4h << 4) + imm4l;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset_addr = valn + (is_add ? imm32 : -imm32);
    int64_t  address     = is_index ? offset_addr : valn;
    int64_t  data        = ld_halfword(address);
    if (is_wback)
        regs[n] = offset_addr;
    regs[t] = data;
};
void ARMV7_CPU::ldrh_reg(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  m           = inst & 0xf;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset      = shift(reg(m), SRType_LSL, 0, cpsr.c);
    int64_t  offset_addr = valn + (is_add ? offset : -offset);
    int64_t  address     = is_index ? offset_addr : valn;
    int64_t  data        = ld_halfword(address);
    if (is_wback)
        regs[n] = offset_addr;
    regs[t] = data;
};
void ARMV7_CPU::ldrsb_imm(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  imm4h       = (instu >> 8) & 0xf;
    int64_t  imm4l       = inst & 0xf;
    int64_t  imm32       = (imm4h << 4) + imm4l;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset_addr = valn + (is_add ? imm32 : -imm32);
    int64_t  address     = is_index ? offset_addr : valn;
    regs[t]              = bitops->sign_extend(ld_byte(address), 8, 32);
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::ldrsb_reg(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  m           = inst & 0xf;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  offset      = shift(reg(m), SRType_LSL, 0, cpsr.c);
    int64_t  valn        = reg(n);
    int64_t  offset_addr = valn + (is_add ? offset : -offset);
    int64_t  address     = is_index ? offset_addr : valn;
    regs[t]              = bitops->sign_extend(ld_byte(address), 8, 32);
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::str_reg(int64_t inst, int64_t addr)
{
    uint64_t instu    = inst;
    int64_t  p        = (instu >> 24) & 1;
    int64_t  u        = (instu >> 23) & 1;
    int64_t  w        = (instu >> 21) & 1;
    int64_t  n        = (instu >> 16) & 0xf;
    int64_t  t        = (instu >> 12) & 0xf;
    int64_t  imm5     = (instu >> 7) & 0x1f;
    int64_t  type     = (instu >> 5) & 3;
    int64_t  m        = inst & 0xf;
    int64_t  is_index = p == 1;
    int64_t  is_add   = u == 1;
    int64_t  is_wback = p == 0 || w == 1;
    decode_imm_shift(type, imm5);
    int64_t valn        = reg(n);
    int64_t offset      = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t offset_addr = valn + (is_add ? offset : -offset);
    int64_t address     = is_index ? offset_addr : valn;
    address             = bitops->get_bits64(address, 31, 0);    // XXX
    int64_t data        = reg(t);
    st_word(address, data);
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::strbt_a1(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  u           = inst & (1 << 23);
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  imm32       = inst & 0xfff;
    int64_t  is_add      = u == 1;
    int64_t  valn        = reg(n);
    int64_t  offset      = imm32;
    int64_t  offset_addr = valn + (is_add ? offset : -offset);
    st_byte(valn, bitops->get_bits(reg(t), 7, 0));
    regs[n] = offset_addr;
};
void ARMV7_CPU::strbt_a2(int64_t inst, int64_t addr)
{
    uint64_t instu  = inst;
    int64_t  u      = (instu >> 23) & 1;
    int64_t  n      = (instu >> 16) & 0xf;
    int64_t  t      = (instu >> 12) & 0xf;
    int64_t  imm5   = (instu >> 7) & 0x1f;
    int64_t  type   = (instu >> 5) & 3;
    int64_t  m      = inst & 0xf;
    int64_t  is_add = u == 1;
    decode_imm_shift(type, imm5);
    int64_t valn        = reg(n);
    int64_t offset      = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t offset_addr = valn + (is_add ? offset : -offset);
    st_byte(valn, bitops->get_bits(reg(t), 7, 0));
    regs[n] = offset_addr;
};
void ARMV7_CPU::strb_reg(int64_t inst, int64_t addr)
{
    uint64_t instu    = inst;
    int64_t  p        = (instu >> 24) & 1;
    int64_t  u        = (instu >> 23) & 1;
    int64_t  w        = (instu >> 21) & 1;
    int64_t  n        = (instu >> 16) & 0xf;
    int64_t  t        = (instu >> 12) & 0xf;
    int64_t  imm5     = (instu >> 7) & 0x1f;
    int64_t  type     = (instu >> 5) & 3;
    int64_t  m        = inst & 0xf;
    int64_t  is_index = p == 1;
    int64_t  is_add   = u == 1;
    int64_t  is_wback = p == 0 || w == 1;
    decode_imm_shift(type, imm5);
    int64_t valn        = reg(n);
    int64_t offset      = shift(reg(m), shift_t, shift_n, cpsr.c);
    int64_t offset_addr = valn + (is_add ? offset : -offset);
    int64_t address     = is_index ? offset_addr : valn;
    st_byte(address, bitops->get_bits(reg(t), 7, 0));
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::strd_reg(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  m           = inst & 0xf;
    int64_t  t2          = t + 1;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  valm        = reg(m);
    int64_t  offset_addr = valn + (is_add ? valm : -valm);
    int64_t  address     = is_index ? offset_addr : valn;
    st_word(address, reg(t));
    st_word(address + 4, reg(t2));
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::strd_imm(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  imm4h       = (instu >> 8) & 0xf;
    int64_t  imm4l       = inst & 0xf;
    int64_t  t2          = t + 1;
    int64_t  imm32       = (imm4h << 4) + imm4l;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset_addr = valn + (is_add ? imm32 : -imm32);
    int64_t  address     = is_index ? offset_addr : valn;
    st_word(address, reg(t));
    st_word(address + 4, reg(t2));
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::strh_imm(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  imm4h       = (instu >> 8) & 0xf;
    int64_t  imm4l       = inst & 0xf;
    int64_t  imm32       = (imm4h << 4) + imm4l;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset_addr = valn + (is_add ? imm32 : -imm32);
    int64_t  address     = is_index ? offset_addr : valn;
    st_halfword(address, bitops->get_bits(reg(t), 15, 0));
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::strh_reg(int64_t inst, int64_t addr)
{
    uint64_t instu       = inst;
    int64_t  p           = (instu >> 24) & 1;
    int64_t  u           = (instu >> 23) & 1;
    int64_t  w           = (instu >> 21) & 1;
    int64_t  n           = (instu >> 16) & 0xf;
    int64_t  t           = (instu >> 12) & 0xf;
    int64_t  m           = inst & 0xf;
    int64_t  is_index    = p == 1;
    int64_t  is_add      = u == 1;
    int64_t  is_wback    = p == 0 || w == 1;
    int64_t  valn        = reg(n);
    int64_t  offset      = shift(reg(m), SRType_LSL, 0, cpsr.c);
    int64_t  offset_addr = valn + (is_add ? offset : -offset);
    int64_t  address     = is_index ? offset_addr : valn;
    st_halfword(address, bitops->get_bits(reg(t), 15, 0));
    if (is_wback)
        regs[n] = offset_addr;
};
void ARMV7_CPU::ldm(int64_t inst, int64_t addr)
{
    uint64_t instu         = inst;
    int64_t  w             = (instu >> 21) & 1;
    int64_t  n             = (instu >> 16) & 0xf;
    int64_t  register_list = inst & 0xffff;
    int64_t  n_registers   = bitops->bit_count(register_list, 16);
    int64_t  is_pop        = false;
    if (w && n == 13 && n_registers >= 2) {
        is_pop = true;
    }
    int64_t  is_wback = w == 1;
    int64_t  valn     = reg(n);
    int64_t  address  = valn;
    int64_t  reglist[16];
    uint64_t register_listu = register_list;
    for (int64_t i = 0; i < 15; i++) {
        if ((register_listu >> i) & 1) {
            reglist[i] = i;
            regs[i]    = ld_word(address);
            address += 4;
        }
    }
    if (register_list & 0x8000) {
        reglist[15] = 15;
        branch_to   = ld_word(address);
    }
    if (is_wback)
        regs[n] = reg(n) + 4 * n_registers;
};
void ARMV7_CPU::ldm_er(int64_t inst, int64_t addr)
{
    uint64_t instu         = inst;
    int64_t  p             = (instu >> 24) & 1;
    int64_t  u             = (instu >> 23) & 1;
    int64_t  w             = (instu >> 21) & 1;
    int64_t  n             = (instu >> 16) & 0xf;
    int64_t  register_list = inst & 0x7fff;
    int64_t  n_registers   = bitops->bit_count(register_list, 15);
    int64_t  is_wback      = w == 1;
    int64_t  is_increment  = u == 1;
    int64_t  is_wordhigher = p == u;
    int64_t  valn          = reg(n);
    if (is_user_or_system())
        abort_unpredictable("LDM (exception return)", inst);
    int64_t length  = 4 * n_registers + 4;
    int64_t address = valn + (is_increment ? 0 : -length);
    if (is_wordhigher)
        address += 4;
    int64_t  reglist[16];
    uint64_t register_listu = register_list;

    for (int64_t i = 0; i < 15; i++) {
        if ((register_listu >> i) & 1) {
            reglist[i] = i;
            regs[i]    = ld_word(address);
            address += 4;
        }
    }
    int64_t new_pc = ld_word(address);
    if (is_wback)
        regs[n] = valn + (is_increment ? length : -length);

    auto flgobj = get_current_spsr();
    cpsr_write_by_instr(&flgobj, 15, true);
    branch_to = new_pc;
};
void ARMV7_CPU::ldm_ur(int64_t inst, int64_t addr)
{
    uint64_t instu         = inst;
    int64_t  p             = (instu >> 24) & 1;
    int64_t  u             = (instu >> 23) & 1;
    int64_t  n             = (instu >> 16) & 0xf;
    int64_t  register_list = inst & 0x7fff;
    int64_t  n_registers   = bitops->bit_count(register_list, 15);
    int64_t  is_increment  = u == 1;
    int64_t  is_wordhigher = p == u;
    int64_t  valn          = reg(n);
    if (is_user_or_system())
        abort_unpredictable("LDM (user registers)", inst);
    int64_t length  = 4 * n_registers;
    int64_t address = valn + (is_increment ? 0 : -length);
    if (is_wordhigher)
        address += 4;
    int64_t  reglist[16];
    uint64_t register_listu = register_list;

    for (int64_t i = 0; i < 15; i++) {
        if ((register_listu >> i) & 1) {
            reglist[i]  = i;
            regs_usr[i] = ld_word(address);
            if (cpsr.m == FIQ_MODE) {
                if (!(i >= 8 && i <= 14))
                    regs[i] = regs_usr[i];
            } else {
                if (!(i >= 13 && i <= 14))
                    regs[i] = regs_usr[i];
            }
            address += 4;
        }
    }
};
void ARMV7_CPU::ldmda(int64_t inst, int64_t addr)
{
    uint64_t instu         = inst;
    int64_t  w             = (instu >> 21) & 1;
    int64_t  n             = (instu >> 16) & 0xf;
    int64_t  register_list = inst & 0xffff;
    int64_t  n_registers   = bitops->bit_count(register_list, 16);
    int64_t  address       = reg(n) - 4 * n_registers + 4;
    int64_t  reglist[16];
    uint64_t register_listu = register_list;

    for (int64_t i = 0; i < 15; i++) {
        if ((register_listu >> i) & 1) {
            reglist[i] = i;
            regs[i]    = ld_word(address);
            address += 4;
        }
    }
    if (register_list & 0x8000) {
        reglist[15] = 15;
        branch_to   = ld_word(address);
    }
    if (w)
        regs[n] = reg(n) - 4 * n_registers;
};
void ARMV7_CPU::ldmdb(int64_t inst, int64_t addr)
{
    uint64_t instu         = inst;
    int64_t  w             = (instu >> 21) & 1;
    int64_t  n             = (instu >> 16) & 0xf;
    int64_t  register_list = inst & 0xffff;
    int64_t  n_registers   = bitops->bit_count(register_list, 16);
    int64_t  address       = reg(n) - 4 * n_registers;
    int64_t  reglist[16];
    uint64_t register_listu = register_list;

    for (int64_t i = 0; i < 15; i++) {
        if ((register_listu >> i) & 1) {
            reglist[i] = i;
            regs[i]    = ld_word(address);
            address += 4;
        }
    }
    if (register_list & 0x8000) {
        reglist[15] = 15;
        branch_to   = ld_word(address);
    }
    if (w)
        regs[n] = reg(n) - 4 * n_registers;
};
void ARMV7_CPU::ldmib(int64_t inst, int64_t addr)
{
    uint64_t instu         = inst;
    int64_t  w             = (instu >> 21) & 1;
    int64_t  n             = (instu >> 16) & 0xf;
    int64_t  register_list = inst & 0xffff;
    int64_t  n_registers   = bitops->bit_count(register_list, 16);
    int64_t  address       = reg(n) + 4;
    int64_t  reglist[16];
    uint64_t register_listu = register_list;

    for (int64_t i = 0; i < 15; i++) {
        if ((register_listu >> i) & 1) {
            reglist[i] = i;
            regs[i]    = ld_word(address);
            address += 4;
        }
    }
    if (register_list & 0x8000) {
        reglist[15] = 15;
        branch_to   = ld_word(address);
    }
    if (w)
        regs[n] = reg(n) + 4 * n_registers;
};
void ARMV7_CPU::stm(int64_t inst, int64_t addr)
{
    uint64_t instu         = inst;
    int64_t  w             = (instu >> 21) & 1;
    int64_t  n             = (instu >> 16) & 0xf;
    int64_t  register_list = inst & 0xffff;
    int64_t  n_registers   = bitops->bit_count(register_list, 16);
    int64_t  address       = reg(n);
    int64_t  reglist[16];
    uint64_t register_listu = register_list;

    for (int64_t i = 0; i < 15; i++) {
        if ((register_listu >> i) & 1) {
            reglist[i] = i;
            st_word(address, regs[i]);
            address += 4;
        }
    }
    if (register_list & 0x8000) {
        reglist[15] = 15;
        st_word(address, get_pc());
    }
    if (w)
        regs[n] = reg(n) + 4 * n_registers;
};
void ARMV7_CPU::stmdb(int64_t inst, int64_t addr)
{
    uint64_t instu         = inst;
    int64_t  w             = (instu >> 21) & 1;
    int64_t  n             = (instu >> 16) & 0xf;
    int64_t  register_list = inst & 0xffff;
    int64_t  n_registers   = bitops->bit_count(register_list, 16);
    int64_t  is_push       = false;
    int64_t  valn          = reg(n);
    if (w && n == 13 && n_registers >= 2) {
        is_push = true;
    }
    int64_t  address = valn - 4 * n_registers;
    int64_t  reglist[16];
    uint64_t register_listu = register_list;

    for (int64_t i = 0; i < 15; i++) {
        if ((register_listu >> i) & 1) {
            reglist[i] = i;
            st_word(address, regs[i]);
            address += 4;
        }
    }
    if (register_list & 0x8000) {
        reglist[15] = 15;
        st_word(address, get_pc());
    }
    if (w || is_push)
        regs[n] = reg(n) - 4 * n_registers;
};
void ARMV7_CPU::stmib(int64_t inst, int64_t addr)
{
    uint64_t instu         = inst;
    int64_t  w             = (instu >> 21) & 1;
    int64_t  n             = (instu >> 16) & 0xf;
    int64_t  register_list = inst & 0xffff;
    int64_t  n_registers   = bitops->bit_count(register_list, 16);
    int64_t  valn          = reg(n);
    int64_t  address       = valn + 4;
    int64_t  reglist[16];
    uint64_t register_listu = register_list;

    for (int64_t i = 0; i < 15; i++) {
        if ((register_listu >> i) & 1) {
            reglist[i] = i;
            st_word(address, regs[i]);
            address += 4;
        }
    }
    if (register_list & 0x8000) {
        reglist[15] = 15;
        st_word(address, get_pc());
    }
    if (w)
        regs[n] = reg(n) + 4 * n_registers;
};
void ARMV7_CPU::stm_ur(int64_t inst, int64_t addr)
{
    uint64_t instu         = inst;
    int64_t  p             = (instu >> 24) & 1;
    int64_t  u             = (instu >> 23) & 1;
    int64_t  n             = (instu >> 16) & 0xf;
    int64_t  register_list = inst & 0xffff;
    int64_t  n_registers   = bitops->bit_count(register_list, 16);
    int64_t  is_increment  = u == 1;
    int64_t  is_wordhigher = p == u;
    if (n == 15 || n_registers < 1)
        abort_unpredictable("STM (user registers)", inst);
    if (is_user_or_system())
        abort_unpredictable("STM (user registers)", 0);
    int64_t length  = 4 * n_registers;
    int64_t address = reg(n) + (is_increment ? 0 : -length);
    if (is_wordhigher)
        address += 4;

    int64_t  reglist[16];
    uint64_t register_listu = register_list;

    for (int64_t i = 0; i < 15; i++) {
        if ((register_listu >> i) & 1) {
            reglist[i] = i;
            if (cpsr.m == FIQ_MODE) {
                if (i >= 8 && i <= 14)
                    st_word(address, regs_usr[i]);
                else
                    st_word(address, regs[i]);
            } else {
                if (i >= 13 && i <= 14)
                    st_word(address, regs_usr[i]);
                else
                    st_word(address, regs[i]);
            }
            address += 4;
        }
    }
    if (register_list & 0x8000) {
        reglist[15] = 15;
        st_word(address, get_pc());
    }
};
void ARMV7_CPU::cps(int64_t inst, int64_t addr)
{
    uint64_t instu   = inst;
    int64_t  imod    = (instu >> 18) & 3;
    int64_t  m       = inst & (1 << 17);
    int64_t  a       = inst & (1 << 8);
    int64_t  i       = inst & (1 << 7);
    int64_t  f       = inst & (1 << 6);
    int64_t  mode    = inst & 0xf;
    int64_t  enable  = imod == 2;
    int64_t  disable = imod == 3;
    if (is_priviledged()) {
        auto new_cpsr = clone_psr_n(&cpsr);
        if (enable) {
            if (a)
                new_cpsr->a = 0;
            if (i)
                new_cpsr->i = 0;
            if (f)
                new_cpsr->f = 0;
        }
        if (disable) {
            if (a)
                new_cpsr->a = 1;
            if (i)
                new_cpsr->i = 1;
            if (f)
                new_cpsr->f = 1;
        }
        if (m)
            new_cpsr->m = mode;
        cpsr_write_by_instr(new_cpsr, 15, true);
    }
};
void ARMV7_CPU::svc(int64_t inst, int64_t addr)
{
    call_supervisor();
};
void ARMV7_CPU::clrex(int64_t inst, int64_t addr){};
void ARMV7_CPU::dsb(int64_t inst, int64_t addr){};
void ARMV7_CPU::dmb(int64_t inst, int64_t addr){};
void ARMV7_CPU::isb(int64_t inst, int64_t addr){};
void ARMV7_CPU::wfi(int64_t inst, int64_t addr)
{
    is_halted = true;
    cpsr.i    = 0;
};
void ARMV7_CPU::vmrs(string inst_name, int64_t inst, int64_t addr)
{
    regs[6] = 1 << 20;
};
void ARMV7_CPU::nop(string inst_name, int64_t inst, int64_t addr){};
void ARMV7_CPU::interrupt()
{
    spsr_irq     = *(clone_psr_n(&cpsr));
    regs_irq[14] = get_pc() - 4;
    change_mode(IRQ_MODE);
    cpsr.i   = 1;
    cpsr.a   = 1;
    regs[15] = coprocs[15]->interrupt_vector_address + 0x18;
};
void ARMV7_CPU::data_abort()
{
    spsr_abt     = *(clone_psr_n(&cpsr));
    regs_abt[14] = get_pc();
    change_mode(ABT_MODE);
    cpsr.i   = 1;
    regs[15] = coprocs[15]->interrupt_vector_address + 0x10;
};
void ARMV7_CPU::prefetch_abort()
{
    spsr_abt     = *(clone_psr_n(&cpsr));
    regs_abt[14] = get_pc() - 4;
    change_mode(ABT_MODE);
    cpsr.i   = 1;
    regs[15] = coprocs[15]->interrupt_vector_address + 0x0c;
};
void ARMV7_CPU::supervisor()
{
    spsr_svc     = *(clone_psr_n(&cpsr));
    regs_svc[14] = get_pc() - 4;
    change_mode(SVC_MODE);
    cpsr.i   = 1;
    regs[15] = coprocs[15]->interrupt_vector_address + 0x08;
};
void ARMV7_CPU::undefined_instruction()
{
    spsr_und     = *(clone_psr_n(&cpsr));
    regs_und[14] = get_pc() - 4;
    change_mode(UND_MODE);
    cpsr.i   = 1;
    regs[15] = coprocs[15]->interrupt_vector_address + 0x04;
};
int64_t ARMV7_CPU::abort_unknown_inst(int64_t inst, int64_t addr)
{
    throw "UNKNOWN";
};
int64_t ARMV7_CPU::abort_simdvfp_inst(int64_t inst, int64_t addr)
{
    throw "SIMD or VFP";
};
int64_t ARMV7_CPU::abort_not_impl(string name, int64_t inst, int64_t addr)
{
    throw "NOT IMPLEMENTED: ";
};
int64_t ARMV7_CPU::abort_undefined_instruction(string category, int64_t inst, int64_t addr)
{
    throw "UNDEFINED: ";
};
int64_t ARMV7_CPU::abort_unpredictable(string category, int64_t value)
{
    throw "UNPREDICTABLE: " + category;
};
int64_t ARMV7_CPU::abort_unpredictable_instruction(string category, int64_t inst, int64_t addr)
{
    throw "UNPREDICTABLE: ";
};
int64_t ARMV7_CPU::abort_decode_error(int64_t inst, int64_t addr)
{
    throw "Decode error";
};
string ARMV7_CPU::decode_uncond(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op    = 0;
    int64_t  op1   = 0;
    int64_t  op2   = 0;
    int64_t  tmp   = 0;
    op1            = (instu >> 20) & 0xff;
    uint64_t op1u  = op1;

    if ((op1 >> 7) == 0) {
        op1 = (instu >> 20) & 0x7f;
        op  = (instu >> 16) & 1;
        op2 = (instu >> 4) & 0xf;
        tmp = (op1u >> 5) & 3;
        switch (tmp) {
            case 0:
                if (op1 == 0x10 && (op2 & 2) == 0) {
                    if (op) {
                        abort_not_impl("SETEND", inst, addr);
                    } else {
                        return "cps";
                    }
                    break;
                }
                abort_unknown_inst(inst, addr);
                break;
            case 1:
                abort_simdvfp_inst(inst, addr);
                break;
            case 2:
                if ((op1 & 1) == 0) {
                    abort_simdvfp_inst(inst, addr);
                }
                switch (op1 >> 1 & 3) {
                    case 2:
                        if (op1 & 0x10) {
                            return "pld_imm";
                        } else {
                            abort_not_impl("PLI (immediate, literal)", inst, addr);
                        }
                        break;
                    case 3:
                        if ((op1 & 0x18) == 0x10) {
                            switch (op2) {
                                case 1:
                                    return "clrex";
                                case 4:
                                    return "dsb";
                                case 5:
                                    return "dmb";
                                case 6:
                                    return "isb";
                                default:
                                    abort_unpredictable_instruction(
                                        "Miscellaneous instructions, memory hints, and Advanced SIMD instructions",
                                        inst, addr);
                                    break;
                            }
                        } else {
                            abort_unpredictable_instruction(
                                "Miscellaneous instructions, memory hints, and Advanced SIMD instructions", inst, addr);
                        }
                        break;
                    default:
                        abort_unknown_inst(inst, addr);
                        break;
                }
                break;
            case 3:
                if ((op2 & 1) == 0) {
                    switch (op1 & 7) {
                        case 5:
                            if (op1 & 0x10) {
                                abort_not_impl("PLD (register)", inst, addr);
                            } else {
                                abort_not_impl("PLI (register)", inst, addr);
                            }
                            break;
                        case 7:
                            abort_unpredictable_instruction(
                                "Miscellaneous instructions, memory hints, and Advanced SIMD instructions", inst, addr);
                            break;
                        default:
                            abort_unknown_inst(inst, addr);
                            break;
                    }
                }
                break;
            default:
                abort_decode_error(inst, addr);
                break;
        }
    } else {
        switch (op1) {
            case 0xc4:
                abort_not_impl("MCRR, MCRR2", inst, addr);
                break;
            case 0xc5:
                abort_not_impl("MRRC, MRRC2", inst, addr);
                break;
            default:
                tmp = (op1u >> 5) & 7;
                switch (tmp) {
                    case 4:
                        if (op1 & 4) {
                            if (!(op1 & 1)) {
                                abort_not_impl("SRS", inst, addr);
                                break;
                            }
                        } else {
                            if (op1 & 1) {
                                abort_not_impl("RFE", inst, addr);
                                break;
                            }
                        }
                        abort_unknown_inst(inst, addr);
                        break;
                    case 5:
                        abort_not_impl("BL, BLX (immediate)", inst, addr);
                        break;
                    case 6:
                        if (op1 & 1) {
                            throw 333;
                        } else {
                            throw 333;
                        }
                        break;
                    case 7:
                        if (!(op1 & 1 << 4)) {
                            if (op & 1) {
                                if (op1 & 1) {
                                    abort_not_impl("MRC, MRC2", inst, addr);
                                } else {
                                    abort_not_impl("MCR, MCR2", inst, addr);
                                }
                            } else {
                                throw 333;
                            }
                            break;
                        }
                    default:
                        abort_unknown_inst(inst, addr);
                        break;
                }
        }
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
string ARMV7_CPU::decode_sync_prim(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op    = (instu >> 20) & 0xf;
    if ((op & 8) == 0) {
        if ((op & 3) == 0) {
            return "swp";
        } else {
            abort_unknown_inst(inst, addr);
        }
    } else {
        switch (op & 7) {
            case 0:
                return "strex";
            case 1:
                return "ldrex";
            case 2:
                return "strexd";
            case 3:
                return "ldrexd";
            case 4:
                abort_not_impl("STREXB", inst, addr);
                break;
            case 5:
                abort_not_impl("LDREXB", inst, addr);
                break;
            case 6:
                abort_not_impl("STREXH", inst, addr);
                break;
            case 7:
                abort_not_impl("LDREXH", inst, addr);
                break;
            default:
                break;
        }
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
string ARMV7_CPU::decode_dataproc_imm(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op    = (instu >> 20) & 0x1f;
    int64_t  rn;
    switch (op >> 1) {
        case 0:
            return "and_imm";
        case 1:
            return "eor_imm";
        case 2:
            rn = (instu >> 16) & 0xf;
            if (rn == 0xf) {
                return "adr_a2";
            } else {
                return "sub_imm";
            }
        case 3:
            return "rsb_imm";
        case 4:
            rn = (instu >> 16) & 0xf;
            if (rn == 0xf) {
                return "adr_a1";
            } else {
                return "add_imm";
            }
        case 5:
            return "adc_imm";
        case 6:
            return "sbc_imm";
        case 7:
            return "rsc_imm";
        case 8:
            if ((op & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            return "tst_imm";
        case 9:
            if ((op & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            return "teq_imm";
        case 0xa:
            if ((op & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            return "cmp_imm";
        case 0xb:
            if ((op & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            return "cmn_imm";
        case 0xc:
            return "orr_imm";
        case 0xd:
            return "mov_imm_a1";
        case 0xe:
            return "bic_imm";
        case 0xf:
            return "mvn_imm";
        default:
            break;
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
string ARMV7_CPU::decode_msr_imm_and_hints(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op    = inst & (1 << 22);
    int64_t  op1   = (instu >> 16) & 0xf;
    int64_t  op2   = inst & 0xff;
    if (op) {
        return "msr_imm_sys";
    } else {
        if ((op1 & 2)) {
            return "msr_imm_sys";
        } else {
            if ((op1 & 1)) {
                return "msr_imm_sys";
            } else {
                if (op1 & 8) {
                    abort_not_impl("MSR (immediate) (application level)", inst, addr);
                } else {
                    if (op1 & 4) {
                        abort_not_impl("MSR (immediate) (application level)", inst, addr);
                    } else {
                        if ((op2 & 0xf0) == 0xf0) {
                            abort_not_impl("DBG", inst, addr);
                        } else {
                            switch (op2) {
                                case 0:
                                    return "nop";
                                case 1:
                                    abort_not_impl("YIELD", inst, addr);
                                    break;
                                case 2:
                                    abort_not_impl("WFE", inst, addr);
                                    break;
                                case 3:
                                    return "wfi";
                                case 4:
                                    abort_not_impl("SEV", inst, addr);
                                    break;
                                default:
                                    abort_unknown_inst(inst, addr);
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
void ARMV7_CPU::decode_half_mul(int64_t inst, int64_t addr)
{
    throw "decode_half_mul";
};
string ARMV7_CPU::decode_misc(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op    = (instu >> 21) & 0x3;
    int64_t  op1   = (instu >> 16) & 0xf;
    int64_t  op2   = (instu >> 4) & 0x7;
    switch (op2) {
        case 0:
            if (op & 1) {
                if (!((op & 2) == 2) && (op1 & 3) == 0) {
                    abort_not_impl("MSR (register) (application level)", inst, addr);
                } else {
                    return "msr_reg_sys";
                }
            } else {
                return "mrs";
            }
            break;
        case 1:
            switch (op) {
                case 1:
                    return "bx";
                case 3:
                    return "clz";
                default:
                    abort_unknown_inst(inst, addr);
                    break;
            }
            break;
        case 2:
            if (op != 1) {
                abort_unknown_inst(inst, addr);
            }
            abort_not_impl("BXJ", inst, addr);
            break;
        case 3:
            if (op != 1) {
                abort_unknown_inst(inst, addr);
            }
            return "blx_reg";
        case 5:
            abort_not_impl("Saturating addition and subtraction", inst, addr);
            break;
        case 7:
            switch (op) {
                case 1:
                    abort_not_impl("BKPT", inst, addr);
                    break;
                case 3:
                    abort_not_impl("SMC (previously SMI)", inst, addr);
                    break;
                default:
                    abort_unknown_inst(inst, addr);
                    break;
            }
            break;
        default:
            abort_unknown_inst(inst, addr);
            break;
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
string ARMV7_CPU::decode_dataproc_reg(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op1   = (instu >> 20) & 0x1f;
    int64_t  op2   = (instu >> 7) & 0x1f;
    int64_t  op3   = (instu >> 5) & 0x3;
    switch (op1 >> 1) {
        case 0:
            return "and_reg";
        case 1:
            return "eor_reg";
        case 2:
            return "sub_reg";
        case 3:
            return "rsb_reg";
        case 4:
            return "add_reg";
        case 5:
            return "adc_reg";
        case 6:
            return "sbc_reg";
        case 7:
            abort_not_impl("RSC (register)", inst, addr);
            break;
        case 8:
            if ((op1 & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            return "tst_reg";
        case 9:
            if ((op1 & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            return "teq_reg";
        case 0xa:
            if ((op1 & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            return "cmp_reg";
        case 0xb:
            if ((op1 & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            return "cmn_reg";
        case 0xc:
            return "orr_reg";
        case 0xd:
            switch (op3) {
                case 0:
                    if (op2 == 0) {
                        return "mov_reg";
                    } else {
                        return "lsl_imm";
                    }
                case 1:
                    return "lsr_imm";
                case 2:
                    return "asr_imm";
                case 3:
                    if (op2 == 0) {
                        return "rrx";
                    } else {
                        return "ror_imm";
                    }
                default:
                    break;
            }
            break;
        case 0xe:
            return "bic_reg";
        case 0xf:
            return "mvn_reg";
        default:
            break;
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
string ARMV7_CPU::decode_dataproc_rsr(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op1   = (instu >> 20) & 0x1f;
    int64_t  op2   = (instu >> 5) & 0x3;
    switch (op1 >> 1) {
        case 0:
            return "and_rsr";
        case 1:
            return "eor_rsr";
        case 2:
            return "sub_rsr";
        case 3:
            return "rsb_rsr";
        case 4:
            return "add_rsr";
        case 5:
            abort_not_impl("ADC (register-shifted register)", inst, addr);
            break;
        case 6:
            return "sbc_rsr";
        case 7:
            abort_not_impl("RSC (register-shifted register)", inst, addr);
            break;
        case 8:
            if ((op1 & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            return "tst_rsr";
        case 9:
            if ((op1 & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            abort_not_impl("TEQ (register-shifted register)", inst, addr);
            break;
        case 0xa:
            if ((op1 & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            return "cmp_rsr";
        case 0xb:
            if ((op1 & 1) == 0) {
                abort_unknown_inst(inst, addr);
            }
            abort_not_impl("CMN (register-shifted register)", inst, addr);
            break;
        case 0xc:
            return "orr_rsr";
        case 0xd:
            switch (op2) {
                case 0:
                    return "lsl_reg";
                    break;
                case 1:
                    return "lsr_reg";
                case 2:
                    return "asr_reg";
                case 3:
                    abort_not_impl("ROR (register)", inst, addr);
                    break;
                default:
                    break;
            }
            break;
        case 0xe:
            return "bic_rsr";
        case 0xf:
            return "mvn_rsr";
        default:
            break;
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
void ARMV7_CPU::decode_extra_ldst_unpriv1(int64_t inst, int64_t addr)
{
    optmp = bitops->get_bit(inst, 20, 0);
    if (optmp) {
        abort_not_impl("LDRHT", inst, addr);
    } else {
        abort_not_impl("STRHT", inst, addr);
    }
};
void ARMV7_CPU::decode_extra_ldst_unpriv2(int64_t inst, int64_t addr)
{
    auto op2 = bitops->get_bits(inst, 6, 5);
    if (optmp) {
        switch (op2) {
            case 2:
                abort_not_impl("LDRSBT", inst, addr);
                break;
            case 3:
                abort_not_impl("LDRSHT", inst, addr);
                break;
            default:
                abort_unknown_inst(inst, addr);
                break;
        }
    } else {
        int64_t rt = bitops->get_bits(inst, 15, 12);
        if (rt & 1) {
            abort_undefined_instruction("Extra load/store instructions (unprivileged) #2", inst, addr);
        } else {
            abort_unpredictable_instruction("Extra load/store instructions (unprivileged) #2", inst, addr);
        }
    }
};
string ARMV7_CPU::decode_extra_ldst1(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op1   = (instu >> 20) & 0x1f;
    if (op1 & 1) {
        if (op1 & 4) {
            int64_t rn = (instu >> 16) & 0xf;
            if (rn == 0xf) {
                abort_not_impl("LDRH (literal)", inst, addr);
            } else {
                return "ldrh_imm";
            }
        } else {
            return "ldrh_reg";
        }
    } else {
        if (op1 & 4) {
            return "strh_imm";
        } else {
            return "strh_reg";
        }
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
string ARMV7_CPU::decode_extra_ldst2(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op1   = (instu >> 20) & 0x1f;
    int64_t  op2   = (instu >> 5) & 0x3;
    int64_t  rn    = (instu >> 16) & 0xf;
    if (op2 & 1) {
        if (op1 & 1) {
            if (op1 & 4) {
                if (rn == 0xf) {
                    abort_not_impl("LDRSH (literal)", inst, addr);
                } else {
                    return "ldrsh_imm";
                }
            } else {
                return "ldrsh_reg";
            }
        } else {
            if (op1 & 4) {
                return "strd_imm";
            } else {
                return "strd_reg";
            }
        }
    } else {
        if (op1 & 1) {
            if (op1 & 4) {
                if (rn == 0xf) {
                    abort_not_impl("LDRSB (literal)", inst, addr);
                } else {
                    return "ldrsb_imm";
                }
            } else {
                return "ldrsb_reg";
            }
        } else {
            if (op1 & 4) {
                if (rn == 0xf) {
                    abort_not_impl("LDRD (literal)", inst, addr);
                } else {
                    return "ldrd_imm";
                }
            } else {
                return "ldrd_reg";
            }
        }
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
string ARMV7_CPU::decode_multi(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op    = (instu >> 20) & 0xf;
    switch (op >> 1) {
        case 0:
            return "mul";
        case 1:
            return "mla";
        case 2:
            if (op & 1) {
                abort_undefined_instruction("Multiply and multiply-accumulate", inst, addr);
            } else {
                abort_not_impl("UMAAL", inst, addr);
            }
            break;
        case 3:
            if (op & 1) {
                abort_undefined_instruction("Multiply and multiply-accumulate", inst, addr);
            } else {
                return "mls";
            }
            break;
        case 4:
            return "umull";
        case 5:
            return "umlal";
        case 6:
            return "smull";
        case 7:
            return "smlal";
        default:
            break;
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
string ARMV7_CPU::decode_datamisc(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op    = (instu >> 25) & 1;
    int64_t  op1   = (instu >> 20) & 0x1f;
    int64_t  op2   = (instu >> 4) & 0xf;
    int64_t  rn    = 0;
    if (op) {
        if (op1 == 0x12 || op1 == 0x16) {    // 10x10
            return decode_msr_imm_and_hints(inst, addr);
        } else {
            switch (op1) {
                case 0x10:
                    return "mov_imm_a2";
                    break;
                case 0x14:
                    return "movt";
                    break;
                default:
                    if ((op1 >> 3) == 2 && (op1 & 1) == 0) {
                        abort_unknown_inst(inst, addr);
                        return "null";
                    } else {
                        return decode_dataproc_imm(inst, addr);
                    }
                    break;
            }
        }
    } else {
        if (op2 & 1) {
            if (op2 >> 3) {
                if ((op2 & 4) == 4) {
                    if ((op1 >> 4) == 0 && (op1 & 2) == 2) {    // 0xx1x
                        decode_extra_ldst_unpriv2(inst, addr);
                        return "null";
                    } else {
                        return decode_extra_ldst2(inst, addr);
                    }
                } else {
                    if (op2 & 2) {
                        if ((op1 >> 4) == 0 && (op1 & 2) == 2) {    // 0xx1x
                            decode_extra_ldst_unpriv1(inst, addr);
                            return "null";
                        } else {
                            return decode_extra_ldst1(inst, addr);
                        }
                    } else {
                        if (op1 >> 4) {
                            return decode_sync_prim(inst, addr);
                        } else {
                            return decode_multi(inst, addr);
                        }
                    }
                }
            } else {
                if ((op1 >> 3) == 2 && (op1 & 1) == 0) {    // 10xx0
                    return decode_misc(inst, addr);
                } else {
                    return decode_dataproc_rsr(inst, addr);
                }
            }
        } else {
            if ((op1 >> 3) == 2 && (op1 & 1) == 0) {    // 10xx0
                if (op2 >> 3) {
                    abort_not_impl("Halfword multiply and multiply-accumulate", inst, addr);
                } else {
                    return decode_misc(inst, addr);
                }
            } else {
                return decode_dataproc_reg(inst, addr);
            }
        }
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
string ARMV7_CPU::decode_media(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;
    int64_t  op1   = (instu >> 20) & 0x1f;
    int64_t  op2   = (instu >> 5) & 0x7;
    int64_t  tmp   = op1 >> 3;
    int64_t  rn    = 0;
    int64_t  a     = 0;
    switch (tmp) {
        case 0:
            if (op1 & 4) {
                op1 = bitops->get_bits(inst, 21, 20);
                op2 = bitops->get_bits(inst, 7, 5);
                switch (op1) {
                    case 1:
                        switch (op2) {
                            case 0:
                                abort_not_impl("UADD16", inst, addr);
                                break;
                            case 1:
                                abort_not_impl("UASX", inst, addr);
                                break;
                            case 2:
                                abort_not_impl("USAX", inst, addr);
                                break;
                            case 3:
                                abort_not_impl("USUB16", inst, addr);
                                break;
                            case 4:
                                abort_not_impl("UADD8", inst, addr);
                                break;
                            case 7:
                                abort_not_impl("USUB8", inst, addr);
                                break;
                            default:
                                abort_unknown_inst(inst, addr);
                                break;
                        }
                        break;
                    case 2:
                        switch (op2) {
                            case 0:
                                abort_not_impl("UQADD16", inst, addr);
                                break;
                            case 1:
                                abort_not_impl("UQASX", inst, addr);
                                break;
                            case 2:
                                abort_not_impl("UQSAX", inst, addr);
                                break;
                            case 3:
                                abort_not_impl("UQSUB16", inst, addr);
                                break;
                            case 4:
                                abort_not_impl("UQADD8", inst, addr);
                                break;
                            case 7:
                                abort_not_impl("UQSUB8", inst, addr);
                                break;
                            default:
                                abort_unknown_inst(inst, addr);
                                break;
                        }
                        break;
                    case 3:
                        switch (op2) {
                            case 0:
                                abort_not_impl("UHADD16", inst, addr);
                                break;
                            case 1:
                                abort_not_impl("UHASX", inst, addr);
                                break;
                            case 2:
                                abort_not_impl("UHSAX", inst, addr);
                                break;
                            case 3:
                                abort_not_impl("UHSUB16", inst, addr);
                                break;
                            case 4:
                                abort_not_impl("UHADD8", inst, addr);
                                break;
                            case 7:
                                abort_not_impl("UHSUB8", inst, addr);
                                break;
                            default:
                                abort_unknown_inst(inst, addr);
                                break;
                        }
                    default:
                        abort_unknown_inst(inst, addr);
                        break;
                }
            } else {
                op1 = bitops->get_bits(inst, 21, 20);
                op2 = bitops->get_bits(inst, 7, 5);
                switch (op1) {
                    case 1:
                        switch (op2) {
                            case 0:
                                abort_not_impl("SADD16", inst, addr);
                                break;
                            case 1:
                                abort_not_impl("SASX", inst, addr);
                                break;
                            case 2:
                                abort_not_impl("SSAX", inst, addr);
                                break;
                            case 3:
                                abort_not_impl("SSUB16", inst, addr);
                                break;
                            case 4:
                                abort_not_impl("SADD8", inst, addr);
                                break;
                            case 7:
                                abort_not_impl("SSUB8", inst, addr);
                                break;
                            default:
                                abort_unknown_inst(inst, addr);
                                break;
                        }
                        break;
                    case 2:
                        switch (op2) {
                            case 0:
                                abort_not_impl("QADD16", inst, addr);
                                break;
                            case 1:
                                abort_not_impl("QASX", inst, addr);
                                break;
                            case 2:
                                abort_not_impl("QSAX", inst, addr);
                                break;
                            case 3:
                                abort_not_impl("QSUB16", inst, addr);
                                break;
                            case 4:
                                abort_not_impl("QADD8", inst, addr);
                                break;
                            case 7:
                                abort_not_impl("QSUB8", inst, addr);
                                break;
                            default:
                                abort_unknown_inst(inst, addr);
                                break;
                        }
                        break;
                    case 3:
                        switch (op2) {
                            case 0:
                                abort_not_impl("SHADD16", inst, addr);
                                break;
                            case 1:
                                abort_not_impl("SHASX", inst, addr);
                                break;
                            case 2:
                                abort_not_impl("SHSAX", inst, addr);
                                break;
                            case 3:
                                abort_not_impl("SHSUB16", inst, addr);
                                break;
                            case 4:
                                abort_not_impl("SHADD8", inst, addr);
                                break;
                            case 7:
                                abort_not_impl("SHSUB8", inst, addr);
                                break;
                            default:
                                abort_unknown_inst(inst, addr);
                                break;
                        }
                        break;
                    default:
                        abort_unknown_inst(inst, addr);
                        break;
                }
            }
            break;
        case 1:
            op1 = (instu >> 20) & 0x7;
            op2 = (instu >> 5) & 0x7;
            tmp = op1 >> 1;
            switch (tmp) {
                case 0:
                    if (op1) {
                        abort_unknown_inst(inst, addr);
                    }
                    if (op2 & 1) {
                        switch (op2 >> 1) {
                            case 1:
                                a = bitops->get_bits(inst, 19, 16);
                                if (a == 0xf) {
                                    abort_not_impl("SXTB16", inst, addr);
                                } else {
                                    abort_not_impl("SXTAB16", inst, addr);
                                }
                                break;
                            case 2:
                                abort_not_impl("SEL", inst, addr);
                                break;
                            default:
                                abort_unknown_inst(inst, addr);
                                break;
                        }
                    } else {
                        throw "PKH";
                    }
                    break;
                case 1:
                    if (op2 & 1) {
                        switch (op1) {
                            case 2:
                                switch (op2) {
                                    case 1:
                                        abort_not_impl("SSAT16", inst, addr);
                                        break;
                                    case 3:
                                        a = bitops->get_bits(inst, 19, 16);
                                        if (a == 0xf) {
                                            return "sxtb";
                                        } else {
                                            abort_not_impl("SXTAB", inst, addr);
                                        }
                                        break;
                                    default:
                                        abort_unknown_inst(inst, addr);
                                        break;
                                }
                                break;
                            case 3:
                                switch (op2) {
                                    case 1:
                                        return "rev";
                                    case 3:
                                        a = (instu >> 16) & 0xf;
                                        if (a == 0xf) {
                                            return "sxth";
                                        } else {
                                            return "sxtah";
                                        }
                                    case 5:
                                        return "rev16";
                                    default:
                                        abort_unknown_inst(inst, addr);
                                        break;
                                }
                                break;
                            default:
                                abort_unknown_inst(inst, addr);
                                break;
                        }
                    } else {
                        abort_not_impl("SSAT", inst, addr);
                    }
                    break;
                case 2:
                    if (op2 != 3) {
                        abort_unknown_inst(inst, addr);
                    }
                    a = bitops->get_bits(inst, 19, 16);
                    if (a == 0xf) {
                        abort_not_impl("UXTB16", inst, addr);
                    } else {
                        abort_not_impl("UXTAB16", inst, addr);
                    }
                    break;
                case 3:
                    if (op2 & 1) {
                        switch (op1) {
                            case 6:
                                switch (op2) {
                                    case 1:
                                        abort_not_impl("USAT16", inst, addr);
                                        break;
                                    case 3:
                                        a = (instu >> 16) & 0xf;
                                        if (a == 0xf) {
                                            return "uxtb";
                                        } else {
                                            return "uxtab";
                                        }
                                    default:
                                        abort_unknown_inst(inst, addr);
                                        break;
                                }
                                break;
                            case 7:
                                switch (op2) {
                                    case 1:
                                        abort_not_impl("RBIT", inst, addr);
                                        break;
                                    case 3:
                                        a = (instu >> 16) & 0xf;
                                        if (a == 0xf) {
                                            return "uxth";
                                        } else {
                                            return "uxtah";
                                        }
                                    case 5:
                                        abort_not_impl("REVSH", inst, addr);
                                        break;
                                    default:
                                        abort_unknown_inst(inst, addr);
                                        break;
                                }
                                break;
                            default:
                                abort_unknown_inst(inst, addr);
                                break;
                        }
                    } else {
                        return "usat";
                    }
                    break;
                default:
                    break;
            }
            break;
        case 2:
            op1 = (instu >> 20) & 0x7;
            op2 = (instu >> 5) & 0x7;
            a   = (instu >> 12) & 0xf;
            switch (op1) {
                case 0:
                    switch (op2 >> 1) {
                        case 0:
                            if (a == 0xf) {
                                abort_not_impl("SMUAD", inst, addr);
                            } else {
                                abort_not_impl("SMLAD", inst, addr);
                            }
                            break;
                        case 1:
                            if (a == 0xf) {
                                abort_not_impl("SMUSD", inst, addr);
                            } else {
                                abort_not_impl("SMLSD", inst, addr);
                            }
                            break;
                        default:
                            abort_unknown_inst(inst, addr);
                            break;
                    }
                    break;
                case 4:
                    switch (op2 >> 1) {
                        case 0:
                            abort_not_impl("SMLALD", inst, addr);
                            break;
                        case 1:
                            abort_not_impl("SMLSLD", inst, addr);
                            break;
                        default:
                            abort_unknown_inst(inst, addr);
                            break;
                    }
                    break;
                case 5:
                    switch (op2 >> 1) {
                        case 0:
                            if (a == 0xf) {
                                abort_not_impl("SMMUL", inst, addr);
                            } else {
                                abort_not_impl("SMMLA", inst, addr);
                            }
                            break;
                        case 3:
                            abort_not_impl("SMMLS", inst, addr);
                            break;
                        default:
                            abort_unknown_inst(inst, addr);
                            break;
                    }
                    break;
                default:
                    abort_unknown_inst(inst, addr);
                    break;
            }
            break;
        case 3:
            if (op1 == 0x1f && op2 == 7) {
                abort_undefined_instruction("Signed multiplies", inst, addr);
            }
            switch (op1 >> 1 & 3) {
                case 0:
                    if ((op1 & 1) == 0 && op2 == 0) {
                        int64_t rd = bitops->get_bits(inst, 15, 12);
                        if (rd == 0xf) {
                            abort_not_impl("USAD8", inst, addr);
                        } else {
                            abort_not_impl("USADA8", inst, addr);
                        }
                        break;
                    }
                    abort_unknown_inst(inst, addr);
                    break;
                case 1:
                    if ((op2 & 3) == 2) {
                        return "sbfx";
                    }
                    abort_unknown_inst(inst, addr);
                    break;
                case 2:
                    if ((op2 & 3) == 0) {
                        rn = inst & 0xf;
                        if (rn == 0xf) {
                            return "bfc";
                        } else {
                            return "bfi";
                        }
                    }
                    abort_unknown_inst(inst, addr);
                    break;
                case 3:
                    if ((op2 & 3) == 2) {
                        return "ubfx";
                    }
                    abort_unknown_inst(inst, addr);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
string ARMV7_CPU::decode(int64_t inst, int64_t addr)
{
    uint64_t instu = inst;

    if (inst == 0xeef06a10)
        return "vmrs";

    if (instu >> 28 == 0xf) {
        return decode_uncond(inst, addr);
    } else {
        int64_t op1    = (instu >> 25) & 7;
        int64_t op     = (instu >> 4) & 1;
        int64_t op2    = 0;
        int64_t tmp    = 0;
        int64_t rn     = 0;
        int64_t coproc = 0;
        shift_t        = 0;
        shift_n        = 0;
        carry_out      = 0;
        overflow       = 0;

        switch (op1 >> 1) {
            case 0:
                return decode_datamisc(inst, addr);
            case 1:
                if (op1 & 1) {
                    if (op) {
                        return decode_media(inst, addr);
                    } else {
                        op1 = (instu >> 20) & 0x1f;
                        if (op1 & 1) {
                            if (op1 & 4) {                      // xx1x1
                                if (op1 == 7 || op1 == 15) {    // 0x111
                                    abort_not_impl("LDRBT", inst, addr);
                                } else {
                                    return "ldrb_reg";
                                }
                            } else {                            // xx0x1
                                if (op1 == 3 || op1 == 11) {    // 0x011
                                    abort_not_impl("LDRT A2", inst, addr);
                                } else {
                                    return "ldr_reg";
                                }
                            }
                        } else {
                            if (op1 & 4) {                      // xx1x0
                                if (op1 == 6 || op1 == 14) {    // 0x110
                                    return "strbt_a2";
                                } else {
                                    return "strb_reg";
                                }
                            } else {                            // xx0x0
                                if (op1 == 2 || op1 == 10) {    // 0x010
                                    abort_not_impl("STRT", inst, addr);
                                } else {
                                    return "str_reg";
                                }
                            }
                        }
                    }
                } else {
                    op1 = (instu >> 20) & 0x1f;
                    if (op1 & 1) {
                        if (op1 & 4) {                      // xx1x1
                            if (op1 == 7 || op1 == 15) {    // 0x111
                                abort_not_impl("LDRBT", inst, addr);
                            } else {
                                rn = (instu >> 16) & 0xf;
                                if (rn == 0xf) {
                                    abort_not_impl("LDRB (literal)", inst, addr);
                                } else {
                                    return "ldrb_imm";
                                }
                            }
                        } else {                             // xx0x1
                            if (op1 == 3 || op1 == 0xb) {    // 0x011
                                return "ldrt_a1";
                            } else {
                                rn = (instu >> 16) & 0xf;
                                if (rn == 0xf) {
                                    return "ldr_lit";
                                } else {
                                    return "ldr_imm";
                                }
                            }
                        }
                    } else {
                        if (op1 & 4) {                      // xx1x0
                            if (op1 == 6 || op1 == 14) {    // 0x110
                                return "strbt_a1";
                            } else {
                                return "strb_imm";
                            }
                        } else {                            // xx0x0
                            if (op1 == 2 || op1 == 10) {    // 0x010
                                abort_not_impl("STRT", inst, addr);
                            } else {
                                return "str_imm";
                            }
                        }
                    }
                }
                break;
            case 2:
                op = (instu >> 20) & 0x3f;
                if (op & 0x20) {
                    if (op & 0x10) {
                        return "bl_imm";
                    } else {
                        return "b";
                    }
                } else {
                    if (op & 4) {
                        if (op & 1) {
                            int64_t r = (instu >> 15) & 1;
                            if (r) {
                                return "ldm_er";
                            } else {
                                return "ldm_ur";
                            }
                        } else {
                            return "stm_ur";
                        }
                    } else {
                        if (op & 1) {
                            switch (op >> 2 & 7) {    // 0b11100
                                case 0:
                                    return "ldmda";
                                case 2:
                                    return "ldm";
                                case 4:
                                    return "ldmdb";
                                case 6:
                                    return "ldmib";
                                default:
                                    abort_unknown_inst(inst, addr);
                                    break;
                            }
                        } else {
                            switch (op >> 2 & 7) {    // 0b11100
                                case 0:
                                    abort_not_impl("STMDA / STMED", inst, addr);
                                    break;
                                case 2:
                                    return "stm";
                                case 4:
                                    return "stmdb";
                                case 6:
                                    return "stmib";
                                default:
                                    abort_unknown_inst(inst, addr);
                                    break;
                            }
                        }
                    }
                }
                break;
            case 3:
                op1 = (instu >> 20) & 0x3f;
                op  = (instu >> 4) & 1;
                if (op1 & 0x20) {
                    if (op1 & 0x10) {
                        return "svc";
                    } else {
                        coproc = (instu >> 8) & 0xf;
                        if (op) {
                            if ((coproc >> 1) == 5) {    // 0b101x
                                abort_simdvfp_inst(inst, addr);
                            } else {
                                if (op1 & 1) {
                                    return "mrc_a1";
                                } else {
                                    return "mcr_a1";
                                }
                            }
                        } else {
                            if ((coproc >> 1) == 5) {    // 0b101x
                                abort_simdvfp_inst(inst, addr);
                            } else {
                                throw 333;
                            }
                        }
                    }
                } else {
                    if ((op1 >> 3) == 0 && (op1 & 2) == 0) {    // 000x0x
                        switch (op1 >> 1) {
                            case 0:
                                abort_undefined_instruction("System call, and coprocessor instructions", inst, addr);
                                break;
                            case 2:
                                coproc = bitops->get_bits(inst, 11, 8);
                                if ((coproc >> 1) == 5) {    // 0b101x
                                    abort_simdvfp_inst(inst, addr);
                                } else {
                                    if (op1 & 1) {
                                        abort_not_impl("MRRC, MRRC2", inst, addr);
                                    } else {
                                        abort_not_impl("MCRR, MCRR2", inst, addr);
                                    }
                                }
                                break;
                            default:
                                abort_unknown_inst(inst, addr);
                                break;
                        }
                    } else {
                        coproc = bitops->get_bits(inst, 11, 8);
                        if ((coproc >> 1) == 5) {    // 0b101x
                            abort_simdvfp_inst(inst, addr);
                        } else {
                            if (op1 & 1) {
                                rn = bitops->get_bits(inst, 19, 16);
                                if (rn == 0xf) {
                                    throw 333;
                                } else {
                                    throw 333;
                                }
                            } else {
                                throw 333;
                            }
                        }
                    }
                }
                break;
            default:
                break;
        }
    }
    abort_unknown_inst(inst, addr);
    return "null";
};
int ARMV7_CPU::file_read()
{
    logcheck = true;
    stepinfo = false;
    filename = "linux_boot_logs/log13.txt";

    if (logcheck) {
        string   line;
        ifstream input_file(filename);
        if (!input_file.is_open()) {
            logcheck = false;
            stepinfo = false;
        }

        while (getline(input_file, line)) {
            lines.push_back(line);
        }

        input_file.close();

        if (filename == "linux_boot_logs/log0.txt") {
            filecheck_start = 1;
            filecheck_end   = 1000000;
        } else if (filename == "linux_boot_logs/log1.txt") {
            filecheck_start = 1000001;
            filecheck_end   = 2000000;
            fileoffset      = 1000000;
        } else if (filename == "linux_boot_logs/log2.txt") {
            filecheck_start = 2000001;
            filecheck_end   = 3000000;
            fileoffset      = 2000000;
        } else if (filename == "linux_boot_logs/log3.txt") {
            filecheck_start = 3000001;
            filecheck_end   = 4000000;
            fileoffset      = 3000000;
        } else if (filename == "linux_boot_logs/log4.txt") {
            filecheck_start = 4000001;
            filecheck_end   = 5000000;
            fileoffset      = 4000000;
        } else if (filename == "linux_boot_logs/log5.txt") {
            filecheck_start = 5000001;
            filecheck_end   = 6000000;
            fileoffset      = 5000000;
        } else if (filename == "linux_boot_logs/log6.txt") {
            filecheck_start = 6000001;
            filecheck_end   = 7000000;
            fileoffset      = 6000000;
        } else if (filename == "linux_boot_logs/log7.txt") {
            filecheck_start = 7000001;
            filecheck_end   = 8000000;
            fileoffset      = 7000000;
        } else if (filename == "linux_boot_logs/log8.txt") {
            filecheck_start = 8000001;
            filecheck_end   = 9000000;
            fileoffset      = 8000000;
        } else if (filename == "linux_boot_logs/log9.txt") {
            filecheck_start = 9000001;
            filecheck_end   = 10000000;
            fileoffset      = 9000000;
        } else if (filename == "linux_boot_logs/log10.txt") {
            filecheck_start = 10000001;
            filecheck_end   = 11000000;
            fileoffset      = 10000000;
        } else if (filename == "linux_boot_logs/log11.txt") {
            filecheck_start = 11000001;
            filecheck_end   = 12000000;
            fileoffset      = 11000000;
        } else if (filename == "linux_boot_logs/log12.txt") {
            filecheck_start = 12000001;
            filecheck_end   = 13000000;
            fileoffset      = 12000000;
        } else if (filename == "linux_boot_logs/log13.txt") {
            filecheck_start = 13000001;
            filecheck_end   = 13800000;
            fileoffset      = 13000000;
        }
    }
    return EXIT_SUCCESS;
}
void ARMV7_CPU::dump(string inst_name, int64_t inst, int64_t addr)
{
    count++;

    if (logcheck && filecheck_start <= count && count <= filecheck_end) {

        char buf1[1000];
        char buf2[1000];
        char buf3[1000];
        char buf4[1000];
        char buf5[1000];

        sprintf(buf1, "name:%s inst:%ld addr:%ld", inst_name.c_str(), inst, addr);

        sprintf(buf2, "cpsr:  a:%d e:%d f:%d i:%d m:%d n:%d q:%d t:%d v:%d z:%d", cpsr.a, cpsr.e, cpsr.f, cpsr.i,
                cpsr.m, cpsr.n, cpsr.q, cpsr.t, cpsr.v, cpsr.z);

        sprintf(buf3,
                "regs:  0:%ld 1:%ld 2:%ld 3:%ld 4:%ld 5:%ld 6:%ld 7:%ld 8:%ld 9:%ld 10:%ld 11:%ld 12:%ld 13:%ld 14:%ld "
                "15:%ld",
                regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7], regs[8], regs[9], regs[10],
                regs[11], regs[12], regs[13], regs[14], regs[15]);

        sprintf(buf4, "flg:  st:%ld sn:%ld co:%ld of:%ld", shift_t, shift_n, carry_out, overflow);

        sprintf(buf5, "irq:%ld", timer0->gic->pending_interrupts_idx);

        if (stepinfo) {
            printf("\n");
            printf("count : %zu\n", count);
            printf("%s\n", buf1);
            printf("%s\n", buf2);
            printf("%s\n", buf3);
            printf("%s\n", buf4);
            printf("%s\n", buf5);
        }

        if (count < filecheck_end) {
            int len = lines[0].size() + 1010;

            char *sbuf = new char[len];
            sprintf(sbuf, "%s", lines[count - 1 - fileoffset].c_str());
            string s = sbuf;

            char *tbf = new char[len];
            sprintf(tbf, "%s %s %s %s %s", buf1, buf2, buf3, buf4, buf5);
            string t = tbf;

            if (std::equal(t.begin(), t.end(), s.begin())) {
                // printf("ok !\n");
            } else {
                printf("\n\n\n***************\n");
                printf("*** Error ! ***\n");
                printf("***************\n\n\n");

                printf("count : %zu\n", count);
                printf("OK : %s\n", lines[count - 1 - fileoffset].c_str());
                printf("NG : %s %s %s %s %s\n\n", buf1, buf2, buf3, buf4, buf5);
                exit(1);
            }

            delete[] sbuf;
            delete[] tbf;
        } else {
            printf("\n\n\n\n\n-- Compare OK ! ---\n");
            printf("count : %zu\n\n", count);
            printf("\n\n\n\n\n\n");
            exit(1);
        }
    }
};

void ARMV7_CPU::exec(string inst_name, int64_t inst, int64_t addr)
{
    dump(inst_name, inst, addr);
    if (count == 12069110) {
        printf(" ");
    }
    // timer emulate for boot logs
    timer0->set_timer_func(count);
    current = inst_name;
    call_func(inst_name, inst, addr);
};
void ARMV7_CPU::call_func(string inst_name, int64_t inst, int64_t addr)
{
    if (inst_name == "set_apsr") {
        set_apsr(inst, addr);
    } else if (inst_name == "coproc_get_word") {
        coproc_get_word(inst, addr);
    } else if (inst_name == "coproc_internal_operation") {
        coproc_internal_operation(inst, addr);
    } else if (inst_name == "align") {
        align(inst, addr);
    } else if (inst_name == "st_word") {
        st_word(inst, addr);
    } else if (inst_name == "st_halfword") {
        st_halfword(inst, addr);
    } else if (inst_name == "st_byte") {
        st_byte(inst, addr);
    } else if (inst_name == "decode_imm_shift") {
        decode_imm_shift(inst, addr);
    } else if (inst_name == "ror") {
        ror(inst, addr);
    } else if (inst_name == "expand_imm_c") {
        expand_imm_c(inst, addr);
    } else if (inst_name == "adc_imm") {
        adc_imm(inst, addr);
    } else if (inst_name == "add_imm") {
        add_imm(inst, addr);
    } else if (inst_name == "adr_a1") {
        adr_a1(inst, addr);
    } else if (inst_name == "adr_a2") {
        adr_a2(inst, addr);
    } else if (inst_name == "and_imm") {
        and_imm(inst, addr);
    } else if (inst_name == "asr_imm") {
        asr_imm(inst, addr);
    } else if (inst_name == "bic_imm") {
        bic_imm(inst, addr);
    } else if (inst_name == "b") {
        b(inst, addr);
    } else if (inst_name == "bl_imm") {
        bl_imm(inst, addr);
    } else if (inst_name == "cmn_imm") {
        cmn_imm(inst, addr);
    } else if (inst_name == "cmp_imm") {
        cmp_imm(inst, addr);
    } else if (inst_name == "eor_imm") {
        eor_imm(inst, addr);
    } else if (inst_name == "ldr_imm") {
        ldr_imm(inst, addr);
    } else if (inst_name == "ldrb_imm") {
        ldrb_imm(inst, addr);
    } else if (inst_name == "ldrd_imm") {
        ldrd_imm(inst, addr);
    } else if (inst_name == "ldrsh_imm") {
        ldrsh_imm(inst, addr);
    } else if (inst_name == "ldrsh_reg") {
        ldrsh_reg(inst, addr);
    } else if (inst_name == "lsl_imm") {
        lsl_imm(inst, addr);
    } else if (inst_name == "lsr_imm") {
        lsr_imm(inst, addr);
    } else if (inst_name == "mov_imm_a1") {
        mov_imm_a1(inst, addr);
    } else if (inst_name == "mov_imm_a2") {
        mov_imm_a2(inst, addr);
    } else if (inst_name == "movt") {
        movt(inst, addr);
    } else if (inst_name == "msr_imm_sys") {
        msr_imm_sys(inst, addr);
    } else if (inst_name == "mvn_imm") {
        mvn_imm(inst, addr);
    } else if (inst_name == "orr_imm") {
        orr_imm(inst, addr);
    } else if (inst_name == "pld_imm") {
        pld_imm(inst, addr);
    } else if (inst_name == "rsb_imm") {
        rsb_imm(inst, addr);
    } else if (inst_name == "rsc_imm") {
        rsc_imm(inst, addr);
    } else if (inst_name == "ror_imm") {
        ror_imm(inst, addr);
    } else if (inst_name == "rrx") {
        rrx(inst, addr);
    } else if (inst_name == "sbc_imm") {
        sbc_imm(inst, addr);
    } else if (inst_name == "str_imm") {
        str_imm(inst, addr);
    } else if (inst_name == "strb_imm") {
        strb_imm(inst, addr);
    } else if (inst_name == "sub_imm") {
        sub_imm(inst, addr);
    } else if (inst_name == "teq_imm") {
        teq_imm(inst, addr);
    } else if (inst_name == "tst_imm") {
        tst_imm(inst, addr);
    } else if (inst_name == "ldr_lit") {
        ldr_lit(inst, addr);
    } else if (inst_name == "adc_reg") {
        adc_reg(inst, addr);
    } else if (inst_name == "add_reg") {
        add_reg(inst, addr);
    } else if (inst_name == "and_reg") {
        and_reg(inst, addr);
    } else if (inst_name == "asr_reg") {
        asr_reg(inst, addr);
    } else if (inst_name == "bic_reg") {
        bic_reg(inst, addr);
    } else if (inst_name == "bfc") {
        bfc(inst, addr);
    } else if (inst_name == "bfi") {
        bfi(inst, addr);
    } else if (inst_name == "blx_reg") {
        blx_reg(inst, addr);
    } else if (inst_name == "bx") {
        bx(inst, addr);
    } else if (inst_name == "cdp_a1") {
        cdp_a1(inst, addr);
    } else if (inst_name == "clz") {
        clz(inst, addr);
    } else if (inst_name == "cmn_reg") {
        cmn_reg(inst, addr);
    } else if (inst_name == "cmp_reg") {
        cmp_reg(inst, addr);
    } else if (inst_name == "eor_reg") {
        eor_reg(inst, addr);
    } else if (inst_name == "ldr_reg") {
        ldr_reg(inst, addr);
    } else if (inst_name == "ldrb_reg") {
        ldrb_reg(inst, addr);
    } else if (inst_name == "ldrd_reg") {
        ldrd_reg(inst, addr);
    } else if (inst_name == "ldrex") {
        ldrex(inst, addr);
    } else if (inst_name == "ldrexd") {
        ldrexd(inst, addr);
    } else if (inst_name == "ldrt_a1") {
        ldrt_a1(inst, addr);
    } else if (inst_name == "lsl_reg") {
        lsl_reg(inst, addr);
    } else if (inst_name == "lsr_reg") {
        lsr_reg(inst, addr);
    } else if (inst_name == "mcr_a1") {
        mcr_a1(inst, addr);
    } else if (inst_name == "mla") {
        mla(inst, addr);
    } else if (inst_name == "mls") {
        mls(inst, addr);
    } else if (inst_name == "subs_pc_lr_a2") {
        subs_pc_lr_a2(inst, addr);
    } else if (inst_name == "mov_reg") {
        mov_reg(inst, addr);
    } else if (inst_name == "mrc_a1") {
        mrc_a1(inst, addr);
    } else if (inst_name == "mrs") {
        mrs(inst, addr);
    } else if (inst_name == "mul") {
        mul(inst, addr);
    } else if (inst_name == "mvn_reg") {
        mvn_reg(inst, addr);
    } else if (inst_name == "orr_reg") {
        orr_reg(inst, addr);
    } else if (inst_name == "rev") {
        rev(inst, addr);
    } else if (inst_name == "rev16") {
        rev16(inst, addr);
    } else if (inst_name == "rsb_reg") {
        rsb_reg(inst, addr);
    } else if (inst_name == "sbc_reg") {
        sbc_reg(inst, addr);
    } else if (inst_name == "sbfx") {
        sbfx(inst, addr);
    } else if (inst_name == "smlal") {
        smlal(inst, addr);
    } else if (inst_name == "smull") {
        smull(inst, addr);
    } else if (inst_name == "swp") {
        swp(inst, addr);
    } else if (inst_name == "strex") {
        strex(inst, addr);
    } else if (inst_name == "strexd") {
        strexd(inst, addr);
    } else if (inst_name == "sub_reg") {
        sub_reg(inst, addr);
    } else if (inst_name == "sxtb") {
        sxtb(inst, addr);
    } else if (inst_name == "sxth") {
        sxth(inst, addr);
    } else if (inst_name == "sxtah") {
        sxtah(inst, addr);
    } else if (inst_name == "teq_reg") {
        teq_reg(inst, addr);
    } else if (inst_name == "tst_reg") {
        tst_reg(inst, addr);
    } else if (inst_name == "ubfx") {
        ubfx(inst, addr);
    } else if (inst_name == "umlal") {
        umlal(inst, addr);
    } else if (inst_name == "umull") {
        umull(inst, addr);
    } else if (inst_name == "unsigned_satq") {
        unsigned_satq(inst, addr);
    } else if (inst_name == "usat") {
        usat(inst, addr);
    } else if (inst_name == "uxtab") {
        uxtab(inst, addr);
    } else if (inst_name == "uxtah") {
        uxtah(inst, addr);
    } else if (inst_name == "uxtb") {
        uxtb(inst, addr);
    } else if (inst_name == "uxth") {
        uxth(inst, addr);
    } else if (inst_name == "add_rsr") {
        add_rsr(inst, addr);
    } else if (inst_name == "and_rsr") {
        and_rsr(inst, addr);
    } else if (inst_name == "bic_rsr") {
        bic_rsr(inst, addr);
    } else if (inst_name == "cmp_rsr") {
        cmp_rsr(inst, addr);
    } else if (inst_name == "eor_rsr") {
        eor_rsr(inst, addr);
    } else if (inst_name == "mvn_rsr") {
        mvn_rsr(inst, addr);
    } else if (inst_name == "orr_rsr") {
        orr_rsr(inst, addr);
    } else if (inst_name == "rsb_rsr") {
        rsb_rsr(inst, addr);
    } else if (inst_name == "sbc_rsr") {
        sbc_rsr(inst, addr);
    } else if (inst_name == "sub_rsr") {
        sub_rsr(inst, addr);
    } else if (inst_name == "tst_rsr") {
        tst_rsr(inst, addr);
    } else if (inst_name == "ldrh_imm") {
        ldrh_imm(inst, addr);
    } else if (inst_name == "ldrh_reg") {
        ldrh_reg(inst, addr);
    } else if (inst_name == "ldrsb_imm") {
        ldrsb_imm(inst, addr);
    } else if (inst_name == "ldrsb_reg") {
        ldrsb_reg(inst, addr);
    } else if (inst_name == "str_reg") {
        str_reg(inst, addr);
    } else if (inst_name == "strbt_a1") {
        strbt_a1(inst, addr);
    } else if (inst_name == "strbt_a2") {
        strbt_a2(inst, addr);
    } else if (inst_name == "strb_reg") {
        strb_reg(inst, addr);
    } else if (inst_name == "strd_reg") {
        strd_reg(inst, addr);
    } else if (inst_name == "strd_imm") {
        strd_imm(inst, addr);
    } else if (inst_name == "strh_imm") {
        strh_imm(inst, addr);
    } else if (inst_name == "strh_reg") {
        strh_reg(inst, addr);
    } else if (inst_name == "ldm") {
        ldm(inst, addr);
    } else if (inst_name == "ldm_er") {
        ldm_er(inst, addr);
    } else if (inst_name == "ldm_ur") {
        ldm_ur(inst, addr);
    } else if (inst_name == "ldmda") {
        ldmda(inst, addr);
    } else if (inst_name == "ldmdb") {
        ldmdb(inst, addr);
    } else if (inst_name == "ldmib") {
        ldmib(inst, addr);
    } else if (inst_name == "stm") {
        stm(inst, addr);
    } else if (inst_name == "stmdb") {
        stmdb(inst, addr);
    } else if (inst_name == "stmib") {
        stmib(inst, addr);
    } else if (inst_name == "stm_ur") {
        stm_ur(inst, addr);
    } else if (inst_name == "cps") {
        cps(inst, addr);
    } else if (inst_name == "svc") {
        svc(inst, addr);
    } else if (inst_name == "clrex") {
        clrex(inst, addr);
    } else if (inst_name == "dsb") {
        dsb(inst, addr);
    } else if (inst_name == "dmb") {
        dmb(inst, addr);
    } else if (inst_name == "isb") {
        isb(inst, addr);
    } else if (inst_name == "wfi") {
        wfi(inst, addr);
    } else if (inst_name == "abort_unknown_inst") {
        abort_unknown_inst(inst, addr);
    } else if (inst_name == "abort_simdvfp_inst") {
        abort_simdvfp_inst(inst, addr);
    } else if (inst_name == "abort_decode_error") {
        abort_decode_error(inst, addr);
    } else if (inst_name == "decode_uncond") {
        decode_uncond(inst, addr);
    } else if (inst_name == "decode_sync_prim") {
        decode_sync_prim(inst, addr);
    } else if (inst_name == "decode_dataproc_imm") {
        decode_dataproc_imm(inst, addr);
    } else if (inst_name == "decode_msr_imm_and_hints") {
        decode_msr_imm_and_hints(inst, addr);
    } else if (inst_name == "decode_half_mul") {
        decode_half_mul(inst, addr);
    } else if (inst_name == "decode_misc") {
        decode_misc(inst, addr);
    } else if (inst_name == "decode_dataproc_reg") {
        decode_dataproc_reg(inst, addr);
    } else if (inst_name == "decode_dataproc_rsr") {
        decode_dataproc_rsr(inst, addr);
    } else if (inst_name == "decode_extra_ldst_unpriv1") {
        decode_extra_ldst_unpriv1(inst, addr);
    } else if (inst_name == "decode_extra_ldst_unpriv2") {
        decode_extra_ldst_unpriv2(inst, addr);
    } else if (inst_name == "decode_extra_ldst1") {
        decode_extra_ldst1(inst, addr);
    } else if (inst_name == "decode_extra_ldst2") {
        decode_extra_ldst2(inst, addr);
    } else if (inst_name == "decode_multi") {
        decode_multi(inst, addr);
    } else if (inst_name == "decode_datamisc") {
        decode_datamisc(inst, addr);
    } else if (inst_name == "decode_media") {
        decode_media(inst, addr);
    } else if (inst_name == "decode") {
        decode(inst, addr);
    } else if (inst_name == "msr_reg_sys") {
        msr_reg_sys(inst, addr);
    } else {
        printf("\n\n\n\n\n-- Unkown function  ! ---\n");
        printf("\n\n\n\n\n\n");
        exit(1);
    }
}