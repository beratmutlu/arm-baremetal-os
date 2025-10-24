#ifndef UART_H
#define UART_H


void init_uart(void);
char uart_getc(void);
void uart_putc(char c);
void uart_loopback [[noreturn]] (void);
void uart_puts(const char *s);

#endif