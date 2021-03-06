/* Copyright (c) 2008-2015. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <xbt/log.h>
#include <xbt/fifo.h>
#include <xbt/sysdep.h>
#include <simgrid/modelchecker.h>

#include "mc_record.h"
#include "mc_private.h"
#include "mc_mmalloc.h"
#include "mc_ignore.h"
#include "mc_protocol.h"
#include "mc_client.h"
#include "ModelChecker.hpp"

extern "C" {

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_client_api, mc,
  "Public API for the model-checked application");

void MC_assert(int prop)
{
  if (MC_is_active() && !prop) {
    MC_client_send_simple_message(MC_MESSAGE_ASSERTION_FAILED);
    MC_client_handle_messages();
  }
}

// TODO, MC_automaton_new_propositional_symbol

void *MC_snapshot(void)
{
  return simcall_mc_snapshot();
}

int simcall_HANDLER_mc_compare_snapshots(smx_simcall_t simcall,
                                   mc_snapshot_t s1, mc_snapshot_t s2)
{
  return snapshot_compare(s1, s2);
}

int MC_compare_snapshots(void *s1, void *s2)
{
  return simcall_mc_compare_snapshots(s1, s2);
}

void MC_cut(void)
{
  user_max_depth_reached = 1;
}

void MC_ignore(void* addr, size_t size)
{
  if (mc_mode == MC_MODE_CLIENT) {
    s_mc_ignore_memory_message_t message;
    message.type = MC_MESSAGE_IGNORE_MEMORY;
    message.addr = addr;
    message.size = size;
    MC_client_send_message(&message, sizeof(message));
  }
}

}
