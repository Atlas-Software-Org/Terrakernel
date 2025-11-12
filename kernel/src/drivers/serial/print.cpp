#include "print.hpp"

namespace Log {
	void errf(const char* fmt, ...) {
		printf("[ \033[1;31mERROR\033[0m ] ");
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
		printf("[ \033[1;mWARNING\033[0m ] ");
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
		printf("[ \033[94mINFO\033[0m ]  ");
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
		printf("[ \033[92m%s\033[0m ] ", status);
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
