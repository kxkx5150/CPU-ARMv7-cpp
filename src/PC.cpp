#include "PC.h"
#include <cstdint>
#include <string>

PC::PC()
{
    TTF_Init();
    font = TTF_OpenFont("bin/cp437.ttf", 14);
    if (font == NULL) {
        printf("error: font not found\n");
        exit(EXIT_FAILURE);
    }
}
PC::~PC()
{
    delete memctlr;
}
void PC::init()
{
    gic     = new IRQ(0x1e000000);
    timer0  = new DTimer(0x10011000, irq_base + 2, gic);
    uart0   = new UART(0, 0x10009000, irq_base + 5, gic);
    io      = new IO(gic, timer0, uart0);
    memctlr = new Mem(io);
    cpu     = new ARMV7_CPU(memctlr, timer0);

    load_binary("bin/vexpress-armjs.dtb", 0x100);
    load_binary("bin/initramfs.cpio.lzo", 0x00800000);
    load_binary("bin/Image-3.10.29", 0x00008000);
    printf("load_binary\n");

    cpu->regs[0]  = 0;
    cpu->regs[1]  = 2272;
    cpu->regs[2]  = 0x100;
    cpu->regs[15] = 0x00008000;
    cpu->cpsr.m   = 0x13;
    is_booted     = true;
    is_running    = true;
}
void PC::load_binary(std::string path, int64_t phyaddr)
{
    FILE *f = fopen(path.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    const int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    auto buffer = new uint8_t[size];
    auto ___    = fread(buffer, size, 1, f);

    for (int64_t i = 0; i < size; i++) {
        memctlr->st_byte(phyaddr + i, buffer[i]);
    }

    delete[] buffer;
    fclose(f);
}
void PC::start()
{
    printf("\n\n************************\n");
    printf("************************\n");
    printf("****** Boot Linux ******\n");
    printf("************************\n");
    printf("************************\n\n\n");
}
void PC::run_cpu()
{
    loop();
}
void PC::loop()
{
    int64_t     timeslice  = 100000;
    int64_t     n_executed = 0;
    int64_t     inst       = 0;
    std::string inst_name  = "";

    do {
        if (!cpu->cpsr.i && gic->is_pending()) {
            int64_t irq = gic->pick_interrupt();
            if (irq != 0) {
                n_interrupts += 1;
                cpu->is_halted = false;
                cpu->interrupt();
            }
        }

        cpu->branch_to = 0;
        int64_t pc     = cpu->regs[15];
        try {
            inst = cpu->fetch_instruction(pc);
        } catch (const int ecode) {
            if (ecode == 111) {
                // printf("PF steps: %zu\n", cpu->count);
                cpu->prefetch_abort();
                continue;
            } else {
                // throw 999;
            }
        }

        if (cpu->is_valid(inst)) {
            try {
                inst_name = cpu->decode(inst, pc);
            } catch (const int ecode) {
                if (ecode == 333) {
                    // printf("UF steps: %zu\n", cpu->count);
                    cpu->undefined_instruction();
                    continue;
                } else {
                    // throw 999;
                }
            }

            if (cpu->cond(inst)) {
                try {
                    cpu->exec(inst_name, inst, pc);
                } catch (const int ecode) {
                    if (ecode == 111) {
                        // printf("PF a\n");
                        // printf("PF steps: %zu\n", cpu->count);
                        cpu->data_abort();
                        continue;
                    } else if (ecode == 222) {
                        // printf("SUPERVISOR steps: %zu\n", cpu->count);
                        cpu->supervisor();
                        continue;
                    } else {
                        // throw 999;
                    }
                }
            }
        }

        if (cpu->branch_to) {
            cpu->regs[15] = cpu->branch_to;
        } else {
            cpu->regs[15] = pc + 4;
        }

        n_instructions += 1;
        n_executed += 1;
    } while (--timeslice > 0);

    tick += n_executed;
}
void PC::paint(SDL_Renderer *renderer, int widht, int height)
{
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 11, 11, 11, SDL_ALPHA_OPAQUE);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 560;
    rect.h = 300;

    SDL_RenderFillRect(renderer, &rect);
    int       stridx = uart0->strbufs_idx;
    SDL_Color color  = {50, 205, 50};

    for (int y = 0; y < 25; ++y) {
        std::string str = "";

        if (24 < stridx) {
            str = uart0->strbufs[stridx - 24 + y];
        } else {
            str = uart0->strbufs[y];
        }
        int strlen = str.length();
        if (strlen == 0)
            continue;

        SDL_Rect r1;
        r1.x = 0;
        r1.y = y * 14;
        r1.w = strlen * 9;
        r1.h = 14;

        SDL_Surface *text_surface = TTF_RenderUTF8_Solid(font, str.c_str(), color);
        SDL_Texture *Message      = SDL_CreateTextureFromSurface(renderer, text_surface);

        SDL_RenderCopy(renderer, Message, NULL, &r1);
        SDL_FreeSurface(text_surface);
        SDL_DestroyTexture(Message);
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(10);
}
