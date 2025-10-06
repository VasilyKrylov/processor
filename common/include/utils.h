/**
 * @file
 * @brief Usefull functions \n
 *        More detailed description will be in future
 */

#ifndef K_UTILS_H
#define K_UTILS_H

/**
 * @brief Swap values of two size_t's
 * @param [in] a First number to swap
 * @param [in] b Second number to swap
 */
void Swap (size_t *a, size_t *b);

size_t CountChrs (char *s, const char *elements);

#define MAX (x,y) ( (x) > (y) ? (x) : (y) )
#define MIN (x,y) ( (x) < (y) ? (x) : (y) )

struct str_t
{
    char *str = NULL;
    size_t size = 0;
};

#endif // K_UTILS_H
