#include "cstring"
#include <mem/mem.hpp>

char* strcpy(char* dest, const char* src) {
    char* ptr = dest;
    while ((*ptr++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, unsigned int n) {
    char* ptr = dest;
    unsigned int i = 0;
    while (i < n && src[i]) {
        ptr[i] = src[i];
        i++;
    }
    while (i < n) {
        ptr[i++] = '\0';
    }
    return dest;
}

unsigned int strlen(const char* str) {
    unsigned int len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char* s1, const char* s2, unsigned int n) {
    unsigned int i = 0;
    while (i < n && s1[i] && (s1[i] == s2[i])) {
        i++;
    }
    if (i == n) return 0;
    return (unsigned char)s1[i] - (unsigned char)s2[i];
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest;
    while (*ptr) ptr++;
    while ((*ptr++ = *src++));
    return dest;
}

char* strncat(char* dest, const char* src, unsigned int n) {
    char* ptr = dest;
    unsigned int i = 0;
    while (*ptr) ptr++;
    while (i < n && src[i]) {
        *ptr++ = src[i++];
    }
    *ptr = '\0';
    return dest;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c) return (char*)str;
        str++;
    }
    return nullptr;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
        haystack++;
    }
    return nullptr;
}

char* strtok(char* str, const char* delim) {
    static char* next;
    if (str) next = str;
    if (!next) return nullptr;

    char* start = next;
    while (*start && strchr(delim, *start)) start++;
    if (!*start) {
        next = nullptr;
        return nullptr;
    }

    char* end = start;
    while (*end && !strchr(delim, *end)) end++;

    if (*end) {
        *end = '\0';
        next = end + 1;
    } else {
        next = nullptr;
    }

    return start;
}

char* strtok_r(char* str, const char* delim, char** saveptr) {
    char* token;

    if (str != nullptr)
        *saveptr = str;

    if (*saveptr == nullptr)
        return nullptr;

    char* s = *saveptr;
    while (*s && strchr(delim, *s))
        s++;

    if (*s == '\0') {
        *saveptr = nullptr;
        return nullptr;
    }

    token = s;

    while (*s && !strchr(delim, *s))
        s++;

    if (*s == '\0') {
        *saveptr = nullptr;
    } else {
        *s = '\0';
        *saveptr = s + 1;
    }

    return token;
}

char* strrchr(const char* s, int c) {
	const char* last = 0;
	while (*s) {
		if (*s == (char)c) last = s;
		s++;
	}
	return (char*)last;
}

char* strdup(const char* s) {
	size_t szs = strlen(s);
	char* new_str = (char*)mem::heap::malloc(szs);
	mem::memcpy((void*)new_str, (void*)s, szs);
	return new_str;
}

char* strndup(const char* s, unsigned int n) {
	size_t szs = strlen(s);
	if (szs > n) szs = n;
	char* new_str = (char*)mem::heap::malloc(szs);
	mem::memcpy((void*)new_str, (void*)s, szs);
	return new_str;
}
