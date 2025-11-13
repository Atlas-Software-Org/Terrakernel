#include "print.hpp"

namespace Log {
	void errf(const char* fmt, ...) {
		printf("[ \x1b[1;31mERROR\x1b[0m ] ");
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		printf("\n\r");
	}

	void err(const char* s) {
		errf("%s", s);
	}

	void warnf(const char* fmt, ...) {
		printf("[ \x1b[1;mWARNING\x1b[0m ] ");
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		printf("\n\r");
	}

	void warn(const char* s) {
		warnf("%s", s);
	}
	
	void infof(const char* fmt, ...) {
		printf("[ \x1b[94mINFO\x1b[0m ]  ");
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		printf("\n\r");
	}

	void info(const char* s) {
		infof("%s", s);
	}
	
	void printf_status(const char* status, const char* fmt, ...) {
		printf("[ \x1b[92m%s\x1b[0m ] ", status);
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		printf("\n\r");
	}

	void print_status(const char* status, const char* s) {
		printf_status(status, "%s", s);
	}
}
