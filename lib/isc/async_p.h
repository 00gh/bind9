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

#include <isc/async.h>
#include <isc/job.h>
#include <isc/loop.h>
#include <isc/mem.h>
#include <isc/stack.h>
#include <isc/uv.h>

typedef ISC_ASTACK(isc_job_t) isc_asyncstack_t;

void
isc__async_cb(uv_async_t *handle);

void
isc__async_close(uv_handle_t *handle);
