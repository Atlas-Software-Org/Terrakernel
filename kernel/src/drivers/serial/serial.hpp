#ifndef SERIAL_HPP
#define SERIAL_HPP 1

namespace serial {
	extern "C" void serial_putc(char c);
	void serial_enable();
	void serial_disable();
}

#endif /* SERIAL_HPP */
