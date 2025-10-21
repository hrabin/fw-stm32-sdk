#ifndef LIS2DH12_H
#define	LIS2DH12_H

#include "type.h"

bool lis2dh12_init(void);

bool lis2dh12_start(void);

bool lis2dh12_read_data(s16 *x, s16 *y, s16 *z);

#endif //  ! LIS2DH12_H

