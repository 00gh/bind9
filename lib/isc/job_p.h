/*
 * Copyright (C) Internet Systems Consortium, Inc. ("ISC")
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * See the COPYRIGHT file distributed with this work for additional
 * information regarding copyright ownership.
 */

#pragma once

#include <isc/job.h>
#include <isc/loop.h>
#include <isc/uv.h>

typedef ISC_LIST(isc_job_t) isc_joblist_t;

void
isc__job_cb(uv_idle_t *handle);

void
isc__job_close(uv_handle_t *handle);
