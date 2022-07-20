#ifndef _H_IRQ
#define _H_IRQ
#include <cstddef>
#include <cstdint>
#include "utils/bitopts.h"

class IRQ {

  public:
    BitOps *bitops = nullptr;

    int64_t baseaddr               = 0;
    int64_t enabled                = false;
    int64_t ITLinesNumber          = 64 / 32 - 1;
    int64_t CPUNumber              = 0;
    int64_t DCR_data               = 0;
    int64_t ICTR_data              = 1;
    int64_t CICR_data              = 0;
    int64_t IPMR_data              = 0;
    int64_t IAR_data               = 0;
    int64_t EOIR_data              = 0;
    int64_t ICDIPTR_data           = 0;
    int64_t sent_irqs_idx          = 0;
    int64_t pending_interrupts_idx = 0;

    int64_t pending_interrupts[100] = {};
    int64_t sent_irqs[100]          = {};
    int64_t ICFR_datas[100]         = {};
    int64_t IPTR_datas[100]         = {};
    int64_t IPR_datas[100]          = {};
    int64_t ICER_datas[100]         = {};
    int64_t ISER_datas[100]         = {};

  private:
  public:
    IRQ(int _baseaddr);
    ~IRQ();

    int64_t func_sDCR(int64_t data);
    int64_t func_lDCR();
    int64_t func_ICTR();
    int64_t func_sICFR(int64_t data, int64_t idx);
    int64_t func_ICFR(int64_t idx);
    int64_t func_sIPTR(int64_t data, int64_t idx);
    int64_t func_IIPTR(int64_t idx);
    int64_t func_sIPR(int64_t data, int64_t idx);
    int64_t func_IIPR(int64_t idx);
    int64_t func_sICER(int64_t data, int64_t idx);
    int64_t func_IICER(int64_t idx);
    int64_t func_sISER(int64_t data, int64_t idx);
    int64_t func_IISER(int64_t idx);
    int64_t func_sCICR(int64_t data);
    int64_t func_lCICR();
    int64_t func_sIPMR(int64_t data);
    int64_t func_lIPMR();
    int64_t func_lIAR();
    int64_t func_sEOIR(int64_t data);
    int64_t func_sICDIPTR(int64_t data);
    int64_t func_lICDIPTR();
    int64_t is_pending();
    int64_t send_interrupt(int64_t irq);
    int64_t pick_interrupt();
};
#endif