/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_sema4.h"

#define SEMA4_0 ((void*) 0x30AC0000)

/* Number of available semaphore per instance */
#define GATE_NUMS (16)
#define PROC_NUMS (2)


/* Value that must be used to take a semaphore */
static uint8_t g_proc_id = -1;

uint8_t sema4_trylock(uint8_t id)
{
  return SEMA4_TryLock(SEMA4_0, id, g_proc_id) == kStatus_Success;
}

void sema4_lock(uint8_t id)
{
  SEMA4_Lock(SEMA4_0, id, g_proc_id);
}

void sema4_unlock(uint8_t id)
{
  SEMA4_Unlock(SEMA4_0, id);
}

void sema4_init(void)
{
  //int i;

  SEMA4_Init(SEMA4_0);
  SEMA4_ResetGate(SEMA4_0, 0);

  /*
   * Determine the proc id
   * This is not really required as the proc id should not change.
   * Still, this is a good way to check semaphore are operational.
   */
  /*for (i = 0; i < PROC_NUMS; i++)
    {
      if (SEMA4_TryLock(SEMA4_0, 0, i) == kStatus_Success)
        {
          SEMA4_Unlock(SEMA4_0, 0);
          g_proc_id = i;
          break;
        }
    }*/
g_proc_id = 0;
  if (g_proc_id < 0)
    {
      //_err("Failed to determine the proc id\n");
    }
}
