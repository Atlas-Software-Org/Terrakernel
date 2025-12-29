#ifndef CONFIG_HPP
#define CONFIG_HPP 1

/*
    Configure whether the schedule uses the same PML4 as the kernel or not
*/
#define SCHED_CFG_USE_SAME_PML4
// #undef SCHED_CFG_USE_SAME_PML4

/*
	Initial (ps2k) keyboard driver buffer size
*/
#define PS2K_CFG_INITIAL_BUF_SIZE 64

/*
	Configure whether the (ps2k) keyboard driver prints key events (DBG ONLY!)
*/
#define PS2K_CFG_DEBUG
// #undef PS2K_CFG_DEBUG

/*
	Configure whether the (ps2k) keyboard driver bufer is allocated via malloc or valloc (valloc better for big buf size)
*/
#define PS2K_CFG_ALLOC_BUF_MALLOC
// #undef PS2K_CFG_ALLOC_BUF_MALLOC

/*
	Configure whether the (ps2k) keyboard driver buffer is allocated or not
*/
#define PS2K_CFG_ALLOC_BUF
// #undef PS2K_CFG_ALLOC_BUF 

/*
	Configure whether uACPI driver is verbose or not
*/
#define ACPI_CFG_VERBOSE
// #undef ACPI_CFG_VERBOSE

/*
	Configure whether the PCI driver is verbose or not
*/
// #define PCI_CFG_VERBOSE
#undef PCI_CFG_VERBOSE

#endif
