/**
 **************************************************************************************************
 *  @file           : vf-generic.h
 *  @brief          : VF generic API
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Contains generic API used through project
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#ifndef VF_GENERIC__H
#define VF_GENERIC__H

#include <unistd.h>

#define SAFE_STR(str) (char *)((str) == NULL ? "(null)" : (str))

#endif /* VF_GENERIC__H */