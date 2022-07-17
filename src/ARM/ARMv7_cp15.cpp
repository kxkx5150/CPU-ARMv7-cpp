#include "ARMv7_cp15.h"
#include "ARMv7_mmu.h"
#include <cstdint>

ARMV7_CP15::ARMV7_CP15(ARMV7_MMU *_mmu)
{
    mmu    = _mmu;
    bitops = new BitOps();

    for (int64_t i = 0; i < 16; i++)
        domains[i] = 0;
}
ARMV7_CP15::~ARMV7_CP15()
{
    delete bitops;
}
int64_t ARMV7_CP15::func_sTTBCR(int64_t word)
{
    TTBCRd         = word;
    mmu->baseaddr0 = bitops->clear_bits(word, 13 - mmu->width, 0);
    return 1;
}
int64_t ARMV7_CP15::func_sDACR(int64_t word)
{
    DACRd = word;
    for (int64_t i = 0; i < 16; i++) {
        domains[i] = bitops->get_bits(word, i * 2 + 1, i * 2);
    }
    return 1;
}
int64_t ARMV7_CP15::func_sCONTEXTIDR(int64_t word)
{
    int64_t asid = word & 0xff;
    mmu->asid    = asid;
    CONTEXTIDRd  = word;
    return 1;
}
int64_t ARMV7_CP15::func_sTTBR0(int64_t word)
{
    TTBR0d         = word;
    mmu->baseaddr0 = bitops->clear_bits(word, 13 - mmu->width, 0);
    return 1;
}
int64_t ARMV7_CP15::func_sTTBR1(int64_t word)
{
    TTBR1d         = word;
    mmu->baseaddr0 = bitops->clear_bits(word, 13 - mmu->width, 0);
    return 1;
}
int64_t ARMV7_CP15::func_sDCR(int64_t word)
{
    SCTLRd = word;
    if (word & 1) {
        mmu->enabled = true;
    } else {
        mmu->enabled = false;
    }
    if (!(word & 0x01000000)) {     // SCTLR.VE[24]
        if (word & 0x00002000) {    // SCTLR.V[13]
            interrupt_vector_address = 0xffff0000;
        } else {
            interrupt_vector_address = 0x00000000;
        }
    }
    if (word & 2) {    // SCTLR.A[1]
        mmu->check_unaligned = true;
        throw "Check unaligned access!";
    } else {
        mmu->check_unaligned = false;
    }
    SCTLRd = bitops->set_bit(SCTLRd, 22, 1);
    return 1;
}
void ARMV7_CP15::send_word(int64_t inst, int64_t word)
{
    uint64_t instu = inst;
    int64_t  crn   = (instu >> 16) & 0xf;    // the major register specifier
    int64_t  opc1  = (instu >> 21) & 0x7;
    int64_t  crm   = instu & 0xf;
    int64_t  opc2  = (instu >> 5) & 0x7;
    int64_t  idx   = crn * 1000 + opc1 * 100 + crm * 10 + opc2;
    int64_t  ret   = func_write(idx, word);
    if (-555 != ret)
        return;
    else
        throw "write: ";
};

int64_t ARMV7_CP15::get_word(int64_t inst)
{
    uint64_t instu = inst;
    int64_t  crn   = (instu >> 16) & 0xf;    // the major register specifier
    int64_t  opc1  = (instu >> 21) & 0x7;
    int64_t  crm   = instu & 0xf;
    int64_t  opc2  = (instu >> 5) & 0x7;
    int64_t  idx   = crn * 1000 + opc1 * 100 + crm * 10 + opc2;

    int64_t ret = func_read(idx);
    if (ret != -555)
        return ret;
    else
        throw "read: ";
    return 0;
};
int64_t ARMV7_CP15::sctlr_get_nmfi()
{
    uint64_t sctlr = SCTLRd;
    return (sctlr >> 27) & 1;
};
void ARMV7_CP15::set_memory_abort(int64_t vaddr, int64_t status, bool is_write)
{
    DFARd        = vaddr;
    IFARd        = vaddr;    // XXX
    int64_t dfsr = is_write ? (1 << 11) : 0;
    dfsr         = dfsr + status;
    DFSRd        = dfsr;
    IFSRd        = dfsr;    // XXX
};
int64_t ARMV7_CP15::func_read(int64_t addr)
{
    switch (addr) {
        case 0:
            return 1092612097;
        case 1:
            return 2147500032;
        case 10:
            return 1;
        case 14:
            return 65539;
        case 15:
            return 0;
        case 20:
            return 65539;
        case 100:
            return 4026531848;
        case 101:
            return 0;
        case 7050:
            return ICIALLUd;
        case 7051:
            return ICIMVAUd;
        case 7056:
            return BPIALLd;
        case 7057:
            return BPIMVAd;
        case 7054:
            return ISBd;
        case 7101:
            return DCCMVACd;
        case 7104:
            return DSBd;
        case 7105:
            return DMBd;
        case 7111:
            return DCCMVAUd;
        case 7141:
            return DCCIMVACd;
        case 8050:
            return ITLBIALLd;
        case 8051:
            return ITLBIMVAd;
        case 8052:
            return ITLBIASIDd;
        case 8060:
            return DTLBIALLd;
        case 8061:
            return DTLBIMVAd;
        case 8062:
            return DTLBIASIDd;
        case 8070:
            return UTLBIALLd;
        case 8071:
            return UTLBIMVAd;
        case 8072:
            return UTLBIASIDd;
        case 5000:
            return DFSRd;
        case 5001:
            return IFSRd;
        case 6000:
            return DFARd;
        case 6002:
            return IFARd;
        case 10020:
            return PRRRd;
        case 10021:
            return NMRRd;
        case 200:
            return CSSELRd;
        case 1002:
            return CPACRd;
        case 13002:
            return TPIDRURWd;
        case 13003:
            return TPIDRUROd;
        case 1000:
            return SCTLRd;
        case 2000:
            return TTBR0d;
        case 2001:
            return TTBR1d;
        case 2002:
            return TTBCRd;
        case 3000:
            return DACRd;
        case 13001:
            return CONTEXTIDRd;
    }
    return -555;
}
int64_t ARMV7_CP15::func_write(int64_t addr, int64_t data)
{
    switch (addr) {
        case 7050:
            return ICIALLUd = data;
        case 7051:
            return ICIMVAUd = data;
        case 7056:
            return BPIALLd = data;
        case 7057:
            return BPIMVAd = data;
        case 7054:
            return ISBd = data;
        case 7101:
            return DCCMVACd = data;
        case 7104:
            return DSBd = data;
        case 7105:
            return DMBd = data;
        case 7111:
            return DCCMVAUd = data;
        case 7141:
            return DCCIMVACd = data;
        case 8050:
            return ITLBIALLd = data;
        case 8051:
            return ITLBIMVAd = data;
        case 8052:
            return ITLBIASIDd = data;
        case 8060:
            return DTLBIALLd = data;
        case 8061:
            return DTLBIMVAd = data;
        case 8062:
            return DTLBIASIDd = data;
        case 8070:
            return UTLBIALLd = data;
        case 8071:
            return UTLBIMVAd = data;
        case 8072:
            return UTLBIASIDd = data;
        case 5000:
            return DFSRd = data;
        case 5001:
            return IFSRd = data;
        case 6000:
            return DFARd = data;
        case 6002:
            return IFARd = data;
        case 10020:
            return PRRRd = data;
        case 10021:
            return NMRRd = data;
        case 200:
            return CSSELRd = data;
        case 1002:
            return CPACRd = data;
        case 13002:
            return TPIDRURWd = data;
        case 13003:
            return TPIDRUROd = data;
        case 1000:
            return func_sDCR(data);
        case 2000:
            return func_sTTBR0(data);
        case 2001:
            return func_sTTBR1(data);
        case 2002:
            return func_sTTBCR(data);
        case 3000:
            return func_sDACR(data);
        case 13001:
            return func_sCONTEXTIDR(data);
    }
    return -555;
}
