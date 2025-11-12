#ifndef PANIC_H_
#define PANIC_H_

#include <kernel/kprintf.h>
#include <arch/bsp/uart.h>
/**
 * \brief Funktionalitäten um das System kontrolliert zum Stoppen zu bringen.
 * \file panic.h
 */

/**
 * \brief Macro um kernel panic auszugeben
 */
#define panic(msg) _panic(__func__, msg)

/**
 * \brief Hält das System an
 * \param func_name Name der Funktion, die den Fehler verursacht hat
 * \param msg Optionale Nachricht die mit ausgegebn wird
 *
 * Blockiert Interrupts und initialisiert den
 * UART minimal um eine panic Nachricht auszugeben.
 * Anschließend wird das System angehalten.
 *
 */
void _panic [[noreturn]] (const char *func_name, const char *msg);
#endif
