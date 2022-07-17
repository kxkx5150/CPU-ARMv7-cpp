#include "IRQ.h"

IRQ::IRQ(int _baseaddr)
{
    bitops   = new BitOps();
    baseaddr = _baseaddr;
    for (int i = 0; i < 2 * (ITLinesNumber + 1); i++) {
        ICFR_datas[i] = 0;
    }
    for (int i = 0; i < 8 * (ITLinesNumber + 1); i++) {
        IPTR_datas[i] = 0x01010101;
    }
    for (int i = 0; i < 8 * (ITLinesNumber + 1); i++) {
        IPR_datas[i] = 0;
    }
    for (int i = 0; i < (ITLinesNumber + 1); i++) {
        ICER_datas[i] = 0;
    }
    for (int i = 0; i < (ITLinesNumber + 1); i++) {
        ISER_datas[i] = 0;
    }
}
IRQ::~IRQ()
{
}
int64_t IRQ::func_sDCR(int64_t data)
{
    if (bitops->get_bit(data, 0, 0)) {
        enabled = true;
    } else {
        enabled = false;
    }
    if (data)
        DCR_data = data;
    return DCR_data;
};
int64_t IRQ::func_lDCR()
{
    return DCR_data;
};
int64_t IRQ::func_ICTR()
{
    return ICTR_data;
};
int64_t IRQ::func_sICFR(int64_t data, int64_t idx)
{
    ICFR_datas[idx] = data;
    return ICFR_datas[idx];
};
int64_t IRQ::func_ICFR(int64_t idx)
{
    return ICFR_datas[idx];
};
int64_t IRQ::func_sIPTR(int64_t data, int64_t idx)
{
    IPTR_datas[idx] = data;
    return IPTR_datas[idx];
};
int64_t IRQ::func_IIPTR(int64_t idx)
{
    return IPTR_datas[idx];
};
int64_t IRQ::func_sIPR(int64_t data, int64_t idx)
{
    IPR_datas[idx] = data;
    return IPR_datas[idx];
};
int64_t IRQ::func_IIPR(int64_t idx)
{
    return IPR_datas[idx];
};
int64_t IRQ::func_sICER(int64_t data, int64_t idx)
{
    ICER_datas[idx] = data;
    return ICER_datas[idx];
};
int64_t IRQ::func_IICER(int64_t idx)
{
    return ICER_datas[idx];
};
int64_t IRQ::func_sISER(int64_t data, int64_t idx)
{
    ISER_datas[idx] = data;
    return ISER_datas[idx];
};
int64_t IRQ::func_IISER(int64_t idx)
{
    return ISER_datas[idx];
};
int64_t IRQ::func_sCICR(int64_t data)
{
    CICR_data = data;
    return CICR_data;
};
int64_t IRQ::func_lCICR()
{
    return CICR_data;
};
int64_t IRQ::func_sIPMR(int64_t data)
{
    IPMR_data = data;
    return IPMR_data;
};
int64_t IRQ::func_lIPMR()
{
    return IPMR_data;
};
int64_t IRQ::func_lIAR()
{
    if (sent_irqs_idx == 0) {
        return 1023;
    }
    int64_t irq = sent_irqs[0];
    return irq;
};
int64_t IRQ::func_sEOIR(int64_t data)
{
    if (sent_irqs_idx == 0)
        return 0;
    int64_t eoi = data & 0x3ff;
    if (sent_irqs[0] == eoi)
        sent_irqs_idx--;
    else
        throw "irq != eoi";
    return 0;
};
int64_t IRQ::func_sICDIPTR(int64_t data)
{
    ICDIPTR_data = data;
    return ICDIPTR_data;
};
int64_t IRQ::func_lICDIPTR()
{
    return 0x01010101;
};
int64_t IRQ::is_pending()
{
    return enabled && pending_interrupts_idx > 0;
};
int64_t IRQ::send_interrupt(int64_t irq)
{
    if (pending_interrupts[pending_interrupts_idx - 1] == irq)
        return 0;
    pending_interrupts[pending_interrupts_idx] = (irq);
    pending_interrupts_idx++;
    return 0;
};
int64_t IRQ::pick_interrupt()
{
    pending_interrupts_idx--;
    int64_t irq = pending_interrupts[pending_interrupts_idx];
    if (!irq)
        throw "irq === null";
    sent_irqs[sent_irqs_idx] = irq;
    sent_irqs_idx++;
    return irq;
};
