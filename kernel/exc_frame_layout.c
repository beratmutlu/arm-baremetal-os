#include <kernel/exc_frame_layout.h>


uint32_t exc_frame_get_lr(const struct exc_frame *f) {
    return f->lr;
}

uint32_t exc_frame_get_sp(const struct exc_frame *f) {
    return (uint32_t)((uintptr_t)f + EXC_FRAME_TOTAL_BYTES);
}

uint32_t exc_frame_get_spsr(const struct exc_frame *f) {
    return f->spsr;
}
