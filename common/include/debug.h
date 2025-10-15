#ifndef K_DEBUG_H
#define K_DEBUG_H

#define RED_BOLD_COLOR  "\33[1;31m"
#define BLUE_BOLD_COLOR "\33[1;34m" // blue
#define COLOR_END       "\33[0m" // To flush out prev colors

#define ERROR(format, ...) fprintf(stderr, RED_BOLD_COLOR "[ERROR] %s:%d:%s: " format "\n" COLOR_END, __FILE__, __LINE__, __func__, __VA_ARGS__); // NOTE: rename to ERROR_LOG, but it is too late...
#define ERROR_PRINT(format, ...) fprintf(stderr, RED_BOLD_COLOR format "\n" COLOR_END, __VA_ARGS__);

enum common_errors
{
    COMMON_OK                        = 0,
    COMMON_ERROR_ALLOCATING_MEMORY   = 1 << 0,
    COMMON_ERROR_REALLOCATING_MEMORY = 1 << 1,
    COMMON_ERROR_OPENING_FILE        = 1 << 2,
    COMMON_NULL_POINTER              = 1 << 3,
    COMMON_SSCANF_ERROR              = 1 << 4
};

#ifdef PRINT_DEBUG
    #define DEBUG_LOG(format, ...)  do { \
                                        fprintf(stderr, BLUE_BOLD_COLOR "[DEBUG] %s:%d:%s(): " format "\n" COLOR_END, \
                                                __FILE__, __LINE__, __func__, __VA_ARGS__); \
                                    } while(0) 

    #define DEBUG_PRINT(format, ...)    do { \
                                            fprintf(stderr, BLUE_BOLD_COLOR format COLOR_END, __VA_ARGS__); \
                                        } while(0)
    #define ON_DEBUG(...) __VA_ARGS__

#else
    #define DEBUG_LOG(format, ...) 
    #define DEBUG_PRINT(format, ...) 
    #define ON_DEBUG(...) 

#endif // PRINT_DEBUG

void StackPrintError (int error);

#endif // K_DEBUG_H