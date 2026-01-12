#ifndef MMU_H
#define MMU_H

/** \brief L1 Tabellen Eintrag */
typedef unsigned int l1_entry;

/**
 * \brief Zugriffsberechtigungen für Sektionen
 *
 *  |   |            |
 *  |:--|-----------:|
 *  |NA | No Access  |
 *  |R  | Read Only  |
 *  |RW | Read Write |
 *
 *  Zuerst ist die Berechtigung für den privilegierten Modus angegeben.
 *  Danach die Berechtigung für den unprivilegierten Modus.
 *
 *  Bsp: PERM_RW_R, priv kann lesen und schreiben, unpriv kann nur lesen.
 *
 */
enum mmu_permission {
	PERM_NO_ACCESS         = 0b000,
	PERM_RW_NA             = 0b001,
	PERM_R_NA              = 0b101,
	PERM_R_R               = 0b111,
	PERM_RW_R              = 0b010,
	PERM_FULL_ACCESS       = 0b011
};

/**
 * \brief Erstellt einen L1 Tabellen Sektions Eintrag
 *
 * \param phy_addr Die Physikalische Adresse auf die zugegriffen werden soll
 * \param perm Die Zugriffsberechtigungen die gesetzt werden soll
 * \param xn Execute Never
 * \param pxn Privileged Execute Never
 * \return L1 Tabellen Sektions Eintrag
 */
[[nodiscard]] l1_entry mmu_l1_section(void * phy_addr, enum mmu_permission perm, bool xn, bool pxn);


/**
 * \brief Erstellt einen L1 Tabellen Fault Eintrag
 * \return L1 Tabellen Fault Eintrag
 */
[[nodiscard]] l1_entry mmu_l1_fault [[gnu::const]] (void);

/**
 * \brief Setzt einen L1 Tabellen Eintrag
 * \param virt_addr Die virtuelle Adresse die gesetzt werden soll
 * \param entry Der L1 Tabellen Eintrag
 */
void mmu_set_l1_entry(void * virt_addr, l1_entry entry);

/**
 * \brief Initialisiert die MMU
 * Initialisiert die MMU ohne diese anzuschalten. Alle Einträge werden auf
 * Fault gesetzt.
 */
void mmu_init(void);

/**
 * \brief Schaltet die MMU an
 */
void mmu_enable(void);
#endif