#include <arch/bsp/uart.h>
#include <stddef.h>
#include <stdint.h>

static const uint32_t UART_BASE_PHYS		   = 0x7E201000;
static const uint32_t PERIPH_OFFSET 		   = 0x3F000000;
static const uint32_t UART_BASE		           = (UART_BASE_PHYS - PERIPH_OFFSET);

enum {
    FR_RXFE = 1u << 4,  /* Receive FIFO empty */
    FR_TXFF = 1u << 5,  /* Transmit FIFO full */
};


struct uart {
    uint32_t DR;            /* 0x00 Data Register */
    uint32_t RSR_ECR;       /* 0x04 Receive Status / Error Clear */
    uint32_t unused0[4];    /* 0x08...0x17 */
    uint32_t FR;            /* 0x18 Flag Register */
};

static_assert(offsetof(struct uart, DR) == 0x0);
static_assert(offsetof(struct uart, RSR_ECR) == 0x4);
static_assert(offsetof(struct uart, FR) == 0x18);

static volatile struct uart *const uart_port = (struct uart *)UART_BASE;

void init_uart(void) {
    uart_port->RSR_ECR = 0xFFFFFFFFu;
}

void uart_putc(char c) {
    while (uart_port->FR & FR_TXFF) { /* busy wait */ }
    uart_port->DR = (uint32_t)(uint8_t)c;
}

char uart_getc(void) {
    while (uart_port->FR & FR_RXFE) { /* busy wait*/ }
    char c = (char)(uint8_t)uart_port->DR;
    return c;
}

void uart_puts(const char *s) {
    if (!s) {
        return;
    }
    while (*s) {
        uart_putc(*s++);
    }
}
