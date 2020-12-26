/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV5640 driver.
 */
#ifndef __OV5640_H__
#define __OV5640_H__
#include "sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

int ov5640_init(sensor_t *sensor);

#ifdef __cplusplus
}
#endif

#endif // __OV5640_H__
