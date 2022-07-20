#include "ARMv7_cpu.h"

std::string ARMV7_CPU::decode_uncond(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_sync_prim(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_dataproc_imm(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_msr_imm_and_hints(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_misc(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_dataproc_reg(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_dataproc_rsr(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_extra_ldst1(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_extra_ldst2(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_multi(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_datamisc(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode_media(int64_t inst, int64_t addr)
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
std::string ARMV7_CPU::decode(int64_t inst, int64_t addr)
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
