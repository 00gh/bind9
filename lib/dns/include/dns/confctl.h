/*
 * Copyright (C) 1999  Internet Software Consortium.
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

#ifndef DNS_CONFIG_CONFCTL_H
#define DNS_CONFIG_CONFCTL_H 1

/*****
 ***** Module Info
 *****/

/*
 * ADTs for the data defined by a named.conf ``control'' statement.
 */

/*
 * 
 * MP:
 *
 *	Caller must do necessary locking.
 *
 * Reliability:
 *
 *	No known problems.
 *
 * Resources:
 *
 *	Uses memory managers supplied by caller.
 *
 * Security:
 *
 *	N/A.
 *
 * Standards:
 *
 *	N/A.
 *      
 */


/***
 *** Imports
 ***/

#include <config.h>

#include <sys/types.h>
#include <netinet/in.h>

#include <isc/mem.h>

#include <dns/confip.h>


/***
 *** Types
 ***/

typedef struct dns_c_ctrl		dns_c_ctrl_t;
typedef struct dns_c_ctrl_list		dns_c_ctrl_list_t;

struct dns_c_ctrl
{
	isc_mem_t	*mem;		/* where it's memory came from */

	dns_c_control_t control_type;
	union {
		struct {
			dns_c_addr_t addr;
			short port;
			dns_c_ipmatch_list_t *matchlist;
		} inet_v; /* when control_type == dns_c_inet_control  */
		struct {
			char *pathname;
			int perm;
			uid_t owner;
			gid_t group;
		} unix_v; /* when control_type == dns_c_unix_control  */
	} u;
	
	ISC_LINK(dns_c_ctrl_t) next;
};


struct dns_c_ctrl_list
{
	isc_mem_t	       *mem;

	ISC_LIST(dns_c_ctrl_t)	elements;
};



/***
 *** Functions
 ***/


isc_result_t	dns_c_ctrl_inet_new(isc_mem_t *mem, dns_c_ctrl_t **control,
				    dns_c_addr_t addr, short port,
				    dns_c_ipmatch_list_t *iml,
				    isc_boolean_t copy);
/*
 * Creates a new INET control object. If COPY is true then a deep copy is
 * made of IML, otherwise the value of IML is stored directly in the new
 * object.
 *
 * Requires:
 *	mem be a valid memoery manager
 *	control be a valid non-NULL pointer.
 *
 * Returns:
 *	ISC_R_SUCCESS		-- all is well
 *	ISC_R_NOMEMORY		-- insufficient memory available
 */


isc_result_t	dns_c_ctrl_unix_new(isc_mem_t *mem, dns_c_ctrl_t **control,
				    const char *path,
				    int perm, uid_t uid, gid_t gid);
/*
 * Creates a new UNIX control object. A copy is made of the PATH argument.
 *
 * Requires:
 *	mem be a valid memoery manager
 *	control be a valid non-NULL pointer.
 *
 * Returns:
 *	ISC_R_SUCCESS		-- all is well
 *	ISC_R_NOMEMORY		-- insufficient memory available
 *	
 */


isc_result_t	dns_c_ctrl_delete(dns_c_ctrl_t **control);
/*
 * Deletes the object pointed to by *CONTROL. *CONTROL may be NULL.
 *
 * Requires:
 *	control be a valid non-NULL pointer.
 *
 * Returns:
 *	ISC_R_SUCCESS
 */


void		dns_c_ctrl_print(FILE *fp, int indent, dns_c_ctrl_t *ctl);
/*
 * Prints the control object ctl in standard named.conf format. The output
 * is indented by indent number of tabs.
 *
 * Requires:
 *	fp be a pointer to a valid stdio stream.
 *	indent be a non-negative number.
 *
 */


isc_result_t	dns_c_ctrl_list_new(isc_mem_t *mem,
				    dns_c_ctrl_list_t **newlist);
/*
 * Creates a new control object list using the MEM memory manager.
 *
 * Requires:
 * 	mem be a pointer to a valid memory manager,
 *	newlist be a valid non-NULL pointer.
 *
 * Returns:
 *	ISC_R_SUCCESS		-- all is well.
 *	ISC_R_NOMEMORY		-- insufficient memory available.
 */


isc_result_t	dns_c_ctrl_list_delete(dns_c_ctrl_list_t **list);
/*
 * Deletes the control list. The value of *list may be NULL. Sets *list to
 * NULL when done.
 *
 * Requires:
 *	list be a valid non-NULL pointer.
 *
 * Returns:
 *	ISC_R_SUCCESS
 *
 */

void		dns_c_ctrl_list_print(FILE *fp, int indent,
				      dns_c_ctrl_list_t *cl);
/*
 * Prints the control objects inside the list. The output is indented with
 * indent number of tabs.
 *
 * Requires:
 *	fp be a pointer to a valid stdio stream.
 *
 */


#endif /* DNS_CONFIG_CONFCTL_H */
