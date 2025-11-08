#include "print.hpp"

namespace Log {
	void errf(const char* fmt, ...) {
		printf("[ERROR] ");
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}

	void err(const char* s) {
		errf("%s", s);
	}
	
	void infof(const char* fmt, ...) {
		printf("[ INFO ] ");
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}

	void info(const char* s) {
		infof("%s", s);
	}
	
	void printf_status(const char* status, const char* fmt, ...) {
		printf("[ %s ] ", status);
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}

	void print_status(const char* status, const char* s) {
		printf_status(status, "%s", s);
	}
}
