#ifndef _H_ARMV7_CPU
#define _H_ARMV7_CPU

#include "../Mem.h"
#include "ARMv7_mmu.h"
#include "ARMv7_cp15.h"
#include "../utils/bitopts.h"
#include "../dtimer.h"
#include <vector>
#include <string>

const int64_t USR_MODE = 0x10;
const int64_t FIQ_MODE = 0x11;
const int64_t IRQ_MODE = 0x12;
const int64_t SVC_MODE = 0x13;
const int64_t MON_MODE = 0x16;
const int64_t ABT_MODE = 0x17;
const int64_t UND_MODE = 0x1b;
const int64_t SYS_MODE = 0x1f;

const int64_t SRType_LSL = 0;
const int64_t SRType_LSR = 1;
const int64_t SRType_ASR = 2;
const int64_t SRType_RRX = 3;
const int64_t SRType_ROR = 4;

typedef struct Flags
{
    int n = 0;
    int z = 0;
    int c = 0;
    int v = 0;
    int q = 0;
    int e = 0;
    int a = 0;
    int i = 0;
    int f = 0;
    int t = 0;
    int m = 0;
} Flags;

class ARMV7_CPU {
    int64_t optmp = 0;

  public:
    ARMV7_MMU *mmu     = nullptr;
    Mem       *memctlr = nullptr;
    BitOps    *bitops  = nullptr;
    DTimer    *timer0  = nullptr;

    size_t      count     = 0;
    bool        is_halted = false;
    std::string current   = "";

    bool is_good_mode[0xff];

    int64_t regs[16]     = {};
    int64_t regs_usr[16] = {};

    int64_t regs_svc[16] = {};
    int64_t regs_mon[16] = {};
    int64_t regs_abt[16] = {};
    int64_t regs_und[16] = {};
    int64_t regs_irq[16] = {};
    int64_t regs_fiq[16] = {};

    int64_t shift_t   = 0;
    int64_t shift_n   = 0;
    int64_t carry_out = 0;
    int64_t overflow  = 0;

    int64_t branch_to = 0;

    ARMV7_CP15 *coprocs[16];

    Flags cpsr;
    Flags spsr_svc;
    Flags spsr_mon;
    Flags spsr_abt;
    Flags spsr_und;
    Flags spsr_irq;
    Flags spsr_fiq;

    bool saturated = false;

    bool                     logcheck        = true;
    std::string              filename        = "log0.txt";
    int                      filecheck_start = 0;
    int                      filecheck_end   = 100000;
    int                      fileoffset      = 0;
    bool                     stepinfo        = false;
    std::vector<std::string> lines;

  public:
    ARMV7_CPU(Mem *_mem, DTimer *_timer0);
    ~ARMV7_CPU();

    bool    is_priviledged();
    int64_t get_pc();
    int64_t reg(int i);
    int64_t is_user_or_system();
    int64_t is_secure();
    int64_t scr_get_aw();
    int64_t scr_get_fw();
    int64_t nsacr_get_rfr();
    int64_t sctlr_get_nmfi();

    Flags   parse_psr(int64_t value);
    int64_t psr_to_value(Flags *psr);
    Flags  *clone_psr_n(Flags *src);
    void    set_current_spsr(Flags spsr);
    Flags   get_current_spsr();
    Flags  *spsr_write_by_instr0(Flags *spsr, Flags *psr, int64_t bytemask);
    void    spsr_write_by_instr(Flags *psr, int64_t bytemask);
    int64_t cpsr_write_by_instr(Flags *psr, int64_t bytemask, bool affect_execstate);

    int64_t save_to_regs(int64_t mode);
    int64_t change_mode(int64_t mode);
    int64_t restore_from_regs(int64_t mode);
    void    set_apsr(int64_t val, bool set_overflow);
    int64_t coproc_accepted(int64_t cp);
    int64_t coproc_get_word(uint64_t cp, int64_t inst);
    void    coproc_send_word(uint64_t cp, int64_t inst, int64_t word);

    void    st_word(int64_t addr, int64_t word);
    bool    allow_unaligned_access();
    int64_t ld_word(int64_t addr);
    int64_t ld_halfword(int64_t addr);
    void    st_halfword(int64_t addr, int64_t hw);
    int64_t ld_byte(int64_t addr);
    void    st_byte(int64_t addr, int64_t b);
    int64_t fetch_instruction(int64_t addr);

    int64_t shift(int64_t value, int64_t type, int64_t amount, int64_t carry_in);
    void    decode_imm_shift(int64_t type, int64_t imm5);
    int64_t shift_c(int64_t value, int64_t type, int64_t amount, int64_t carry_in);

    void    call_supervisor();
    int64_t align(int64_t value, int64_t _align);
    void    coproc_internal_operation(int64_t cp, int64_t inst);

    int64_t ror_c(int64_t value, int64_t amount, bool write);
    int64_t ror(int64_t val, int64_t rotation);

    int64_t is_zero_bit(int64_t val);
    int64_t expand_imm_c(int64_t imm12, int64_t carry_in);
    int64_t expand_imm(int64_t imm12);
    int64_t add_with_carry(int64_t x, int64_t y, int64_t carry_in);
    int64_t decode_reg_shift(int64_t type);

    bool is_valid(int64_t inst);
    bool cond(int64_t inst);

    void adc_imm(int64_t inst, int64_t addr);
    void add_imm(int64_t inst, int64_t addr);
    void adr_a1(int64_t inst, int64_t addr);
    void adr_a2(int64_t inst, int64_t addr);
    void and_imm(int64_t inst, int64_t addr);
    void asr_imm(int64_t inst, int64_t addr);
    void bic_imm(int64_t inst, int64_t addr);

    void b(int64_t inst, int64_t addr);
    void bl_imm(int64_t inst, int64_t addr);
    void cmn_imm(int64_t inst, int64_t addr);
    void cmp_imm(int64_t inst, int64_t addr);
    void eor_imm(int64_t inst, int64_t addr);
    void ldr_imm(int64_t inst, int64_t addr);
    void ldrb_imm(int64_t inst, int64_t addr);
    void ldrd_imm(int64_t inst, int64_t addr);
    void ldrsh_imm(int64_t inst, int64_t addr);
    void ldrsh_reg(int64_t inst, int64_t addr);
    void lsl_imm(int64_t inst, int64_t addr);
    void lsr_imm(int64_t inst, int64_t addr);
    void mov_imm_a1(int64_t inst, int64_t addr);
    void mov_imm_a2(int64_t inst, int64_t addr);
    void movt(int64_t inst, int64_t addr);
    void msr_imm_sys(int64_t inst, int64_t addr);
    void mvn_imm(int64_t inst, int64_t addr);
    void orr_imm(int64_t inst, int64_t addr);
    void pld_imm(int64_t inst, int64_t addr);
    void rsb_imm(int64_t inst, int64_t addr);
    void rsc_imm(int64_t inst, int64_t addr);
    void ror_imm(int64_t inst, int64_t addr);
    void rrx(int64_t inst, int64_t addr);
    void sbc_imm(int64_t inst, int64_t addr);
    void str_imm(int64_t inst, int64_t addr);
    void strb_imm(int64_t inst, int64_t addr);
    void sub_imm(int64_t inst, int64_t addr);
    void teq_imm(int64_t inst, int64_t addr);
    void tst_imm(int64_t inst, int64_t addr);
    void ldr_lit(int64_t inst, int64_t addr);
    void adc_reg(int64_t inst, int64_t addr);
    void add_reg(int64_t inst, int64_t addr);
    void and_reg(int64_t inst, int64_t addr);
    void asr_reg(int64_t inst, int64_t addr);
    void bic_reg(int64_t inst, int64_t addr);
    void bfc(int64_t inst, int64_t addr);
    void bfi(int64_t inst, int64_t addr);
    void blx_reg(int64_t inst, int64_t addr);

    void bx(int64_t inst, int64_t addr);
    void cdp_a1(int64_t inst, int64_t addr);
    void clz(int64_t inst, int64_t addr);

    int64_t abort_unknown_inst(int64_t inst, int64_t addr);
    int64_t abort_simdvfp_inst(int64_t inst, int64_t addr);
    int64_t abort_not_impl(std::string name, int64_t inst, int64_t addr);
    int64_t abort_undefined_instruction(std::string category, int64_t inst, int64_t addr);
    int64_t abort_unpredictable(std::string category, int64_t value);
    int64_t abort_unpredictable_instruction(std::string category, int64_t inst, int64_t addr);
    int64_t abort_decode_error(int64_t inst, int64_t addr);

    void    cmn_reg(int64_t inst, int64_t addr);
    void    cmp_reg(int64_t inst, int64_t addr);
    void    eor_reg(int64_t inst, int64_t addr);
    void    ldr_reg(int64_t inst, int64_t addr);
    void    ldrb_reg(int64_t inst, int64_t addr);
    void    ldrd_reg(int64_t inst, int64_t addr);
    void    ldrex(int64_t inst, int64_t addr);
    void    ldrexd(int64_t inst, int64_t addr);
    void    ldrt_a1(int64_t inst, int64_t addr);
    void    lsl_reg(int64_t inst, int64_t addr);
    void    lsr_reg(int64_t inst, int64_t addr);
    void    mcr_a1(int64_t inst, int64_t addr);
    void    mla(int64_t inst, int64_t addr);
    void    mls(int64_t inst, int64_t addr);
    void    subs_pc_lr_a2(int64_t inst, int64_t addr);
    void    mov_reg(int64_t inst, int64_t addr);
    void    mrc_a1(int64_t inst, int64_t addr);
    void    mrs(int64_t inst, int64_t addr);
    void    msr_reg_sys(int64_t inst, int64_t addr);
    void    mul(int64_t inst, int64_t addr);
    void    mvn_reg(int64_t inst, int64_t addr);
    void    orr_reg(int64_t inst, int64_t addr);
    void    rev(int64_t inst, int64_t addr);
    void    rev16(int64_t inst, int64_t addr);
    void    rsb_reg(int64_t inst, int64_t addr);
    void    sbc_reg(int64_t inst, int64_t addr);
    void    sbfx(int64_t inst, int64_t addr);
    void    smlal(int64_t inst, int64_t addr);
    void    smull(int64_t inst, int64_t addr);
    void    swp(int64_t inst, int64_t addr);
    void    strex(int64_t inst, int64_t addr);
    void    strexd(int64_t inst, int64_t addr);
    void    sub_reg(int64_t inst, int64_t addr);
    void    sxtb(int64_t inst, int64_t addr);
    void    sxth(int64_t inst, int64_t addr);
    void    sxtah(int64_t inst, int64_t addr);
    void    teq_reg(int64_t inst, int64_t addr);
    void    tst_reg(int64_t inst, int64_t addr);
    void    ubfx(int64_t inst, int64_t addr);
    void    umlal(int64_t inst, int64_t addr);
    void    umull(int64_t inst, int64_t addr);
    int64_t unsigned_satq(int64_t i, int64_t n);
    void    usat(int64_t inst, int64_t addr);
    void    uxtab(int64_t inst, int64_t addr);
    void    uxtah(int64_t inst, int64_t addr);
    void    uxtb(int64_t inst, int64_t addr);
    void    uxth(int64_t inst, int64_t addr);
    void    add_rsr(int64_t inst, int64_t addr);
    void    and_rsr(int64_t inst, int64_t addr);
    void    bic_rsr(int64_t inst, int64_t addr);
    void    cmp_rsr(int64_t inst, int64_t addr);
    void    eor_rsr(int64_t inst, int64_t addr);
    void    mvn_rsr(int64_t inst, int64_t addr);
    void    orr_rsr(int64_t inst, int64_t addr);
    void    rsb_rsr(int64_t inst, int64_t addr);
    void    sbc_rsr(int64_t inst, int64_t addr);
    void    sub_rsr(int64_t inst, int64_t addr);
    void    tst_rsr(int64_t inst, int64_t addr);
    void    ldrh_imm(int64_t inst, int64_t addr);
    void    ldrh_reg(int64_t inst, int64_t addr);
    void    ldrsb_imm(int64_t inst, int64_t addr);
    void    ldrsb_reg(int64_t inst, int64_t addr);
    void    str_reg(int64_t inst, int64_t addr);
    void    strbt_a1(int64_t inst, int64_t addr);
    void    strbt_a2(int64_t inst, int64_t addr);
    void    strb_reg(int64_t inst, int64_t addr);
    void    strd_reg(int64_t inst, int64_t addr);
    void    strd_imm(int64_t inst, int64_t addr);
    void    strh_imm(int64_t inst, int64_t addr);
    void    strh_reg(int64_t inst, int64_t addr);
    void    ldm(int64_t inst, int64_t addr);
    void    ldm_er(int64_t inst, int64_t addr);
    void    ldm_ur(int64_t inst, int64_t addr);
    void    ldmda(int64_t inst, int64_t addr);
    void    ldmdb(int64_t inst, int64_t addr);
    void    ldmib(int64_t inst, int64_t addr);
    void    stm(int64_t inst, int64_t addr);
    void    stmdb(int64_t inst, int64_t addr);
    void    stmib(int64_t inst, int64_t addr);
    void    stm_ur(int64_t inst, int64_t addr);
    void    cps(int64_t inst, int64_t addr);
    void    svc(int64_t inst, int64_t addr);

    void clrex(int64_t inst, int64_t addr);
    void dsb(int64_t inst, int64_t addr);
    void dmb(int64_t inst, int64_t addr);
    void isb(int64_t inst, int64_t addr);
    void wfi(int64_t inst, int64_t addr);
    void vmrs(std::string inst_name, int64_t inst, int64_t addr);
    void nop(std::string inst_name, int64_t inst, int64_t addr);
    void exec(std::string inst_name, int64_t inst, int64_t addr);

    void interrupt();
    void data_abort();
    void prefetch_abort();
    void supervisor();
    void undefined_instruction();

    std::string decode_uncond(int64_t inst, int64_t addr);
    std::string decode_sync_prim(int64_t inst, int64_t addr);
    std::string decode_dataproc_imm(int64_t inst, int64_t addr);
    std::string decode_msr_imm_and_hints(int64_t inst, int64_t addr);
    void        decode_half_mul(int64_t inst, int64_t addr);
    std::string decode_misc(int64_t inst, int64_t addr);
    std::string decode_dataproc_reg(int64_t inst, int64_t addr);
    std::string decode_dataproc_rsr(int64_t inst, int64_t addr);
    void        decode_extra_ldst_unpriv1(int64_t inst, int64_t addr);
    void        decode_extra_ldst_unpriv2(int64_t inst, int64_t addr);
    std::string decode_extra_ldst1(int64_t inst, int64_t addr);
    std::string decode_extra_ldst2(int64_t inst, int64_t addr);
    std::string decode_multi(int64_t inst, int64_t addr);
    std::string decode_datamisc(int64_t inst, int64_t addr);
    std::string decode_media(int64_t inst, int64_t addr);
    std::string decode(int64_t inst, int64_t addr);

    void dump(std::string inst_name, int64_t inst, int64_t addr);
    int  file_read();
    void call_func(std::string inst_name, int64_t inst, int64_t addr);

  private:
};
#endif
