
/* sensor.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_THREAD_H__
#define __SENSOR_THREAD_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "modules.h"

typedef enum {
   SET_AUTOFOCUS,
   CANCEL_AUTOFOCUS,
}sensor_thread_msg_type_t;

typedef struct {
  pthread_t td;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int readfd;
  int writefd;
  boolean is_thread_started;
}sensor_thread_t;

typedef struct {
  sensor_thread_msg_type_t  msgtype;
  int fd;
  int sessionid;
  boolean stop_thread;
  void* module;
} sensor_thread_msg_t;

void sensor_cancel_autofocus_loop();
int32_t sensor_thread_create(mct_module_t *module);
#endif //__SENSOR_THREAD_H__
