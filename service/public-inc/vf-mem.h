/**
 **************************************************************************************************
 *  @file           : vf-mem.h
 *  @brief          : Vision Flow memory routine API
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @description:
 *  Vision Flow memory allocation and deallocation API
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_MEM__H
#define VF_MEM__H

#include "stdlib.h"

#define alloc_data(count, type) (type *)calloc((count), sizeof(type))

static inline
void clear_data(void *buf) {
        void **ptr = (void **)buf;

        if (NULL == ptr) {
                return;
        }

        free(*ptr);
        *ptr = NULL;
}

#endif /* VF_MEM__H */

