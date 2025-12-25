#ifndef CONFIG_HPP
#define CONFIG_HPP 1

/*
    Configure whether the schedule uses the same PML4 as the kernel or not
*/
#define SCHED_CFG_USE_SAME_PML4
// #undef SCHED_CFG_USE_SAME_PML4

/*
	Configure the initial size of the (ps2k) keyboard read buffer
*/
#define PS2K_CFG_INITIAL_BUF_SIZE 8192

/*
	Configure whether the (ps2k) keyboard read buffer is allocated via valloc or malloc
		Helpful if you are using a big buffer
*/
#define PS2K_CFG_ALLOC_BUF_MALLOC
// #undef PS2K_ALLOC_BUF_MALLOC

/*
	Configure whether the (ps2k) keyboard driver handles \b (backspace) independently, meaning it wont just print it,
		it also handles it
*/
#define PS2K_CFG_HANDLE_BACKSPACE_INDEPENDENTLY
// #undef PS2K_CFG_HANDLE_BACKSPACE_INDEPENDENTLY

/*
	Configure whether the (ps2k) keyboard driver echoes back text to the terminal
*/
#define PS2K_CFG_ECHO
// #undef PS2K_CFG_ECHO

#endif
