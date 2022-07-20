#ifndef _H_ARMV7_CP15
#define _H_ARMV7_CP15

#include <cstddef>
#include <cstdint>
#include "../utils/bitopts.h"

class ARMV7_MMU;
class ARMV7_CP15 {
  public:
    BitOps *bitops                   = nullptr;
    int64_t interrupt_vector_address = 0;
    int64_t TRANS_FAULT_SECTION      = 5;      // 0b00101
    int64_t TRANS_FAULT_PAGE         = 7;      // 0b00111
    int64_t PERMISSION_FAULT_SECTION = 0xd;    // 0b01101
    int64_t PERMISSION_FAULT_PAGE    = 0xf;    // 0b01111
    int64_t domains[16]              = {};

    int64_t MIDR       = 0;
    int64_t CTR        = 1;
    int64_t ID_PFR0    = 10;
    int64_t ID_MMFR0   = 14;
    int64_t ID_MMFR1   = 15;
    int64_t ID_ISAR0   = 20;
    int64_t CCSIDR     = 100;
    int64_t CLIDR      = 101;
    int64_t CSSELR     = 200;
    int64_t SCTLR      = 1000;
    int64_t CPACR      = 1002;
    int64_t TTBR0      = 2000;
    int64_t TTBR1      = 2001;
    int64_t TTBCR      = 2002;
    int64_t ICIALLU    = 7050;
    int64_t ICIMVAU    = 7051;
    int64_t BPIALL     = 7056;
    int64_t BPIMVA     = 7057;
    int64_t ISB        = 7054;
    int64_t DCCMVAC    = 7101;
    int64_t DSB        = 7104;
    int64_t DMB        = 7105;
    int64_t DCCMVAU    = 7111;
    int64_t DCCIMVAC   = 7141;
    int64_t DACR       = 3000;
    int64_t DFSR       = 5000;
    int64_t IFSR       = 5001;    // Instruction Fault Status Register (IFSR)
    int64_t DFAR       = 6000;    // Data Fault Address Register (DFAR)
    int64_t IFAR       = 6002;    // Instruction Fault Address Register (IFAR)
    int64_t ITLBIALL   = 8050;
    int64_t ITLBIMVA   = 8051;
    int64_t ITLBIASID  = 8052;
    int64_t DTLBIALL   = 8060;
    int64_t DTLBIMVA   = 8061;
    int64_t DTLBIASID  = 8062;
    int64_t UTLBIALL   = 8070;
    int64_t UTLBIMVA   = 8071;
    int64_t UTLBIASID  = 8072;
    int64_t PRRR       = 10020;    // c10, Primary Region Remap Register (PRRR)
    int64_t NMRR       = 10021;    // c10, Normal Memory Remap Register (NMRR)
    int64_t CONTEXTIDR = 13001;
    int64_t TPIDRURW   = 13002;    // User Read/Write Thread ID Register, TPIDRURW
    int64_t TPIDRURO   = 13003;    // User Read-only Thread ID Register, TPIDRURO

    int64_t DFARd      = 0;
    int64_t IFARd      = 0;
    int64_t DFSRd      = 0;
    int64_t IFSRd      = 0;
    int64_t PRRRd      = 0;
    int64_t NMRRd      = 0;
    int64_t CSSELRd    = 0;
    int64_t CPACRd     = 0;
    int64_t TPIDRURWd  = 0;
    int64_t TPIDRUROd  = 0;
    int64_t ICIALLUd   = 0;
    int64_t ICIMVAUd   = 0;
    int64_t BPIALLd    = 0;
    int64_t BPIMVAd    = 0;
    int64_t ISBd       = 0;
    int64_t DCCMVACd   = 0;
    int64_t DSBd       = 0;
    int64_t DMBd       = 0;
    int64_t DCCMVAUd   = 0;
    int64_t DCCIMVACd  = 0;
    int64_t ITLBIALLd  = 0;
    int64_t ITLBIMVAd  = 0;
    int64_t ITLBIASIDd = 0;
    int64_t DTLBIALLd  = 0;
    int64_t DTLBIMVAd  = 0;
    int64_t DTLBIASIDd = 0;
    int64_t UTLBIALLd  = 0;
    int64_t UTLBIMVAd  = 0;
    int64_t UTLBIASIDd = 0;

    int64_t SCTLRd      = 4194304;
    int64_t TTBR0d      = 0;
    int64_t TTBR1d      = 0;
    int64_t TTBCRd      = 0;
    int64_t DACRd       = 0;
    int64_t CONTEXTIDRd = 0;

  public:
    ARMV7_MMU *mmu = nullptr;

  public:
    ARMV7_CP15(ARMV7_MMU *_mmu);
    ~ARMV7_CP15();

    int64_t func_sTTBCR(int64_t word);
    int64_t func_sDACR(int64_t word);
    int64_t func_sCONTEXTIDR(int64_t word);
    int64_t func_sTTBR0(int64_t word);
    int64_t func_sTTBR1(int64_t word);
    int64_t func_sDCR(int64_t word);
    void    send_word(int64_t inst, int64_t word);
    int64_t get_word(int64_t inst);
    int64_t sctlr_get_nmfi();
    void    set_memory_abort(int64_t vaddr, int64_t status, bool is_write);
    int64_t func_read(int64_t addr);
    int64_t func_write(int64_t addr, int64_t data);

  private:
};
#endif