/* Copyright (c) 2007-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */
 
#ifndef MC_MODEL_CHECKER_H
#define MC_MODEL_CHECKER_H

#include <simgrid_config.h>

#include "mc_forward.h"

SG_BEGIN_DECL()

/** @brief State of the model-checker (global variables for the model checker)
 *
 *  Each part of the state of the model chercker represented as a global
 *  variable prevents some sharing between snapshots and must be ignored.
 *  By moving as much state as possible in this structure allocated
 *  on the model-chercker heap, we avoid those issues.
 */
struct s_mc_model_checker {
  // This is the parent snapshot of the current state:
  mc_snapshot_t parent_snapshot;
  mc_pages_store_t pages;
  int fd_clear_refs;
  int fd_pagemap;
  xbt_dynar_t record;
};

mc_model_checker_t MC_model_checker_new(void);
void MC_model_checker_delete(mc_model_checker_t mc);

SG_END_DECL()

#endif