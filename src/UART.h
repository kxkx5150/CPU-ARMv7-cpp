#ifndef _H_UART
#define _H_UART
#include "IRQ.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include "utils/bitopts.h"

class UART {

  public:
    IRQ    *gic    = nullptr;
    BitOps *bitops = nullptr;

    int64_t     irq      = 0;
    int64_t     baseaddr = 0;
    int64_t     id       = 0;
    std::string name     = &"UART"[id];

    int64_t enabled        = false;
    int64_t fifo_length    = 16;
    int64_t rx_fifo_level  = fifo_length / 8;
    int64_t tx_fifo_level  = fifo_length / 8;
    int64_t tx_int_enabled = false;
    int64_t rx_int_enabled = false;
    int64_t tx_enabled     = false;
    int64_t rx_enabled     = false;
    int64_t fifo_enabled   = false;

    int64_t DR    = 0;
    int64_t FR    = 0;
    int64_t IBRD  = 0;
    int64_t FBRD  = 0;
    int64_t LCR_H = 0;
    int64_t CR    = 0;
    int64_t IFLS  = 0;
    int64_t IMSC  = 0;
    int64_t MIS   = 0;
    int64_t ICR   = 0;

    int64_t DR_data    = 0;
    int64_t CR_data    = 0;
    int64_t LCR_H_data = 0;
    int64_t IFLS_data  = 0;
    int64_t IMSC_data  = 0;
    int64_t ICR_data   = 0;

    int64_t rx_fifo[100] = {};
    int64_t tx_fifo[100] = {};

    int64_t rx_fifo_idx = 0;
    int64_t tx_fifo_idx = 0;

    std::string strbufs[1000];
    int         strbufs_idx = 0;
    std::string strbuf      = "";

  public:
    UART(int _id, int _baseaddr, int _irq_base, IRQ *_gic);
    ~UART();

    int64_t func_sDR(int64_t onebyte);
    int64_t func_lDR();
    int64_t func_IFR();
    int64_t func_sCR(int64_t halfword);
    int64_t func_lCR();
    int64_t func_sLCR_H(int64_t onebyte);
    int64_t func_lLCR_H();
    int64_t func_sIFLS(int64_t halfword);
    int64_t func_lIFLS();
    int64_t func_sIMSC(int64_t halfword);
    int64_t func_lIMSC();
    int64_t func_IMIS();
    int64_t func_sICR(int64_t halfword);
    int64_t enable();
    int64_t output_char(char str);
    int64_t input_char(char str);
    int64_t update_fifo_onoff(int64_t halfword);
    int64_t update_fifo_level(int64_t halfword);

    void store_char(int x);
};
#endif