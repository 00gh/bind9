/*
 * Copyright (C) 2000  Internet Software Consortium.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifndef ISC_ENTROPY_H
#define ISC_ENTROPY_H 1

/*****
 ***** Module Info
 *****/

/*
 * Entropy
 *
 * The entropy API 
 *
 * MP:
 *	The entropy object is locked internally.  All callbacks into
 *	application-provided functions (for setup, gathering, and
 *	shutdown of sources) are guaranteed to be called with the
 *	entropy API lock held.  This means these functions are
 *	not permitted to call back into the entropy API.
 *
 * Reliability:
 *	No anticipated impact.
 *
 * Resources:
 *	A buffer, used as an entropy pool.
 *
 * Security:
 *	While this code is believed to implement good entropy gathering
 *	and distribution, it has not been reviewed by a cryptographic
 *	expert.
 *
 *	Since the added entropy is only as good as the sources used,
 *	this module could hand out bad data and never know it.
 *
 * Standards:
 *	None.
 */

/***
 *** Imports
 ***/

#include <isc/lang.h>
#include <isc/magic.h>
#include <isc/types.h>

ISC_LANG_BEGINDECLS

/***
 *** Magic numbers
 ***/
#define ISC_ENTROPY_MAGIC		ISC_MAGIC('R', 'a', 'n', 'd')
#define ISC_ENTROPY_VALID(b)		ISC_MAGIC_VALID(b, ISC_ENTROPY_MAGIC)

/***
 *** Flags.
 ***/

/*
 * _GOODONLY
 *	Extract only "good" data; return failure if there is not enough
 *	data available and there are no sources which we can poll to get
 *	data, or those sources are empty.
 *
 * _POLLABLE
 *	The entropy source is pollable for more data.  This is most useful
 *	for things like files and devices.  It should not be used for
 *	tty/keyboard data, device timings, etc.
 */

#define ISC_ENTROPY_GOODONLY	0x00000001U
#define ISC_ENTROPY_POLLABLE	0x00000002U

/***
 *** Functions
 ***/

isc_result_t
isc_entropy_create(isc_mem_t *mctx, isc_entropy_t **entp);
/*
 * Create a new entropy object.
 */

isc_result_t
isc_entropy_createfilesource(isc_entropy_t *ent, const char *fname,
			     unsigned int flags,
			     isc_entropysource_t **sourcep);
/*
 * Create a new entropy source from a file.
 *
 * The file is assumed to contain good randomness, and will be mixed directly
 * into the pool with every byte adding 8 bits of entropy.
 *
 * The file will be put into non-blocking mode, so it may be a device file,
 * such as /dev/random.  /dev/urandom should not be used here if it can
 * be avoided, since it will always provide data even if it isn't good.
 * We will make as much pseudorandom data as we need internally if our
 * caller asks for it.
 *
 * If we hit end-of-file, we will stop reading from this source.  Callers
 * who require strong random data will get failure when our pool drains.
 * The file will never be opened/read again once EOF is reached.
 */

isc_result_t
isc_entropy_destroysource(isc_entropysource_t **sourcep);
/*
 * Removes an entropy source from the entropy system.
 */

isc_result_t
isc_entropy_getdata(isc_entropy_t *ent, void *data, unsigned int length,
		    unsigned int *returned, unsigned int flags);
/*
 * Extract data from the entropy pool.  This may load the pool from various
 * sources.
 */

ISC_LANG_ENDDECLS


#endif /* ISC_BUFFER_H */
