#ifndef ERRNO_HPP
#define ERRNO_HPP 1

enum errno_t {
    E2BIG           = 1,   // Argument list too long
    EACCES          = 2,   // Permission denied
    EADDRINUSE      = 3,   // Address in use
    EADDRNOTAVAIL   = 4,   // Address not available
    EAFNOSUPPORT    = 5,   // Address family not supported
    EAGAIN          = 6,   // Resource unavailable, try again
    EALREADY        = 7,   // Connection already in progress
    EBADF           = 8,   // Bad file descriptor
    EBADMSG         = 9,   // Bad message
    EBUSY           = 10,  // Device or resource busy
    ECANCELED       = 11,  // Operation canceled
    ECHILD          = 12,  // No child processes
    ECONNABORTED    = 13,  // Connection aborted
    ECONNREFUSED    = 14,  // Connection refused
    ECONNRESET      = 15,  // Connection reset
    EDEADLK         = 16,  // Resource deadlock would occur
    EDESTADDRREQ    = 17,  // Destination address required
    EDOM            = 18,  // Mathematics argument out of domain of function
    EDQUOT          = 19,  // Reserved
    EEXIST          = 20,  // File exists
    EFAULT          = 21,  // Bad address
    EFBIG           = 22,  // File too large
    EHOSTUNREACH    = 23,  // Host is unreachable
    EIDRM           = 24,  // Identifier removed
    EILSEQ          = 25,  // Illegal byte sequence
    EINPROGRESS     = 26,  // Operation in progress
    EINTR           = 27,  // Interrupted function
    EINVAL          = 28,  // Invalid argument
    EIO             = 29,  // I/O error
    EISCONN         = 30,  // Socket is connected
    EISDIR          = 31,  // Is a directory
    ELOOP           = 32,  // Too many levels of symbolic links
    EMFILE          = 33,  // File descriptor value too large
    EMLINK          = 34,  // Too many links
    EMSGSIZE        = 35,  // Message too large
    EMULTIHOP       = 36,  // Reserved
    ENAMETOOLONG    = 37,  // Filename too long
    ENETDOWN        = 38,  // Network is down
    ENETRESET       = 39,  // Connection aborted by network
    ENETUNREACH     = 40,  // Network unreachable
    ENFILE          = 41,  // Too many files open in system
    ENOBUFS         = 42,  // No buffer space available
    ENODATA         = 43,  // No message is available on the STREAM head read queue
    ENODEV          = 44,  // No such device
    ENOENT          = 45,  // No such file or directory
    ENOEXEC         = 46,  // Executable file format error
    ENOLCK          = 47,  // No locks available
    ENOLINK         = 48,  // Reserved
    ENOMEM          = 49,  // Not enough space
    ENOMSG          = 50,  // No message of the desired type
    ENOPROTOOPT     = 51,  // Protocol not available
    ENOSPC          = 52,  // No space left on device
    ENOSR           = 53,  // No STREAM resources
    ENOSTR          = 54,  // Not a STREAM
    ENOSYS          = 55,  // Functionality not supported
    ENOTCONN        = 56,  // The socket is not connected
    ENOTDIR         = 57,  // Not a directory or a symbolic link to a directory
    ENOTEMPTY       = 58,  // Directory not empty
    ENOTRECOVERABLE = 59,  // State not recoverable
    ENOTSOCK        = 60,  // Not a socket
    ENOTSUP         = 61,  // Not supported
    ENOTTY          = 62,  // Inappropriate I/O control operation
    ENXIO           = 63,  // No such device or address
    EOPNOTSUPP      = 64,  // Operation not supported on socket
    EOVERFLOW       = 65,  // Value too large to be stored in data type
    EOWNERDEAD      = 66,  // Previous owner died
    EPERM           = 67,  // Operation not permitted
    EPIPE           = 68,  // Broken pipe
    EPROTO          = 69,  // Protocol error
    EPROTONOSUPPORT = 70,  // Protocol not supported
    EPROTOTYPE      = 71,  // Protocol wrong type for socket
    ERANGE          = 72,  // Result too large
    EROFS           = 73,  // Read-only file system
    ESPIPE          = 74,  // Invalid seek
    ESRCH           = 75,  // No such process
    ESTALE          = 76,  // Reserved
    ETIME           = 77,  // Stream ioctl() timeout
    ETIMEDOUT       = 78,  // Connection timed out
    ETXTBSY         = 79,  // Text file busy
    EWOULDBLOCK     = 80,  // Operation would block (may be same value as EAGAIN)
    EXDEV           = 81,  // Cross-device link
};

inline const char* strerror(int errnum) {
    switch (errnum) {
        case E2BIG:           return "Argument list too long";
        case EACCES:          return "Permission denied";
        case EADDRINUSE:      return "Address in use";
        case EADDRNOTAVAIL:   return "Address not available";
        case EAFNOSUPPORT:    return "Address family not supported";
        case EAGAIN:          return "Resource unavailable, try again";
        case EALREADY:        return "Connection already in progress";
        case EBADF:           return "Bad file descriptor";
        case EBADMSG:         return "Bad message";
        case EBUSY:           return "Device or resource busy";
        case ECANCELED:       return "Operation canceled";
        case ECHILD:          return "No child processes";
        case ECONNABORTED:    return "Connection aborted";
        case ECONNREFUSED:    return "Connection refused";
        case ECONNRESET:      return "Connection reset";
        case EDEADLK:         return "Resource deadlock would occur";
        case EDESTADDRREQ:    return "Destination address required";
        case EDOM:            return "Mathematics argument out of domain of function";
        case EDQUOT:          return "Reserved";
        case EEXIST:          return "File exists";
        case EFAULT:          return "Bad address";
        case EFBIG:           return "File too large";
        case EHOSTUNREACH:    return "Host is unreachable";
        case EIDRM:           return "Identifier removed";
        case EILSEQ:          return "Illegal byte sequence";
        case EINPROGRESS:     return "Operation in progress";
        case EINTR:           return "Interrupted function";
        case EINVAL:          return "Invalid argument";
        case EIO:             return "I/O error";
        case EISCONN:         return "Socket is connected";
        case EISDIR:          return "Is a directory";
        case ELOOP:           return "Too many levels of symbolic links";
        case EMFILE:          return "File descriptor value too large";
        case EMLINK:          return "Too many links";
        case EMSGSIZE:        return "Message too large";
        case EMULTIHOP:       return "Reserved";
        case ENAMETOOLONG:    return "Filename too long";
        case ENETDOWN:        return "Network is down";
        case ENETRESET:       return "Connection aborted by network";
        case ENETUNREACH:     return "Network unreachable";
        case ENFILE:          return "Too many files open in system";
        case ENOBUFS:         return "No buffer space available";
        case ENODATA:         return "No message is available on the STREAM head read queue";
        case ENODEV:          return "No such device";
        case ENOENT:          return "No such file or directory";
        case ENOEXEC:         return "Executable file format error";
        case ENOLCK:          return "No locks available";
        case ENOLINK:         return "Reserved";
        case ENOMEM:          return "Not enough space";
        case ENOMSG:          return "No message of the desired type";
        case ENOPROTOOPT:     return "Protocol not available";
        case ENOSPC:          return "No space left on device";
        case ENOSR:           return "No STREAM resources";
        case ENOSTR:          return "Not a STREAM";
        case ENOSYS:          return "Functionality not supported";
        case ENOTCONN:        return "The socket is not connected";
        case ENOTDIR:         return "Not a directory or a symbolic link to a directory";
        case ENOTEMPTY:       return "Directory not empty";
        case ENOTRECOVERABLE: return "State not recoverable";
        case ENOTSOCK:        return "Not a socket";
        case ENOTSUP:         return "Not supported";
        case ENOTTY:          return "Inappropriate I/O control operation";
        case ENXIO:           return "No such device or address";
        case EOPNOTSUPP:      return "Operation not supported on socket";
        case EOVERFLOW:       return "Value too large to be stored in data type";
        case EOWNERDEAD:      return "Previous owner died";
        case EPERM:           return "Operation not permitted";
        case EPIPE:           return "Broken pipe";
        case EPROTO:          return "Protocol error";
        case EPROTONOSUPPORT: return "Protocol not supported";
        case EPROTOTYPE:      return "Protocol wrong type for socket";
        case ERANGE:          return "Result too large";
        case EROFS:           return "Read-only file system";
        case ESPIPE:          return "Invalid seek";
        case ESRCH:           return "No such process";
        case ESTALE:          return "Reserved";
        case ETIME:           return "Stream ioctl() timeout";
        case ETIMEDOUT:       return "Connection timed out";
        case ETXTBSY:         return "Text file busy";
        case EWOULDBLOCK:     return "Operation would block";
        case EXDEV:           return "Cross-device link";
        default:              return "Unknown error";
    }
}

#endif
