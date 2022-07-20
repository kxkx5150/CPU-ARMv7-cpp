#include "ARMv7_cpu.h"

void ARMV7_CPU::call_func(std::string inst_name, int64_t inst, int64_t addr)
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
