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

#include <isc/mem.h>
#include <isc/region.h>
#include <isc/result.h>
#include <isc/tls.h>
#include <isc/types.h>

/*
 * Replacement for isc_sockettype_t provided by socket.h.
 */
typedef enum {
	isc_socktype_tcp = 1,
	isc_socktype_udp = 2,
	isc_socktype_unix = 3,
	isc_socktype_raw = 4
} isc_socktype_t;

typedef void (*isc_nm_recv_cb_t)(isc_nmhandle_t *handle, isc_result_t eresult,
				 isc_region_t *region, void *cbarg);
/*%<
 * Callback function to be used when receiving a packet.
 *
 * 'handle' the handle that can be used to send back the answer.
 * 'eresult' the result of the event.
 * 'region' contains the received data, if any. It will be freed
 *          after return by caller.
 * 'cbarg'  the callback argument passed to isc_nm_listenudp(),
 *          isc_nm_listentcpdns(), or isc_nm_read().
 */
typedef isc_result_t (*isc_nm_accept_cb_t)(isc_nmhandle_t *handle,
					   isc_result_t result, void *cbarg);
/*%<
 * Callback function to be used when accepting a connection. (This differs
 * from isc_nm_cb_t below in that it returns a result code.)
 *
 * 'handle' the handle that can be used to send back the answer.
 * 'eresult' the result of the event.
 * 'cbarg'  the callback argument passed to isc_nm_listentcp() or
 * isc_nm_listentcpdns().
 */

typedef void (*isc_nm_cb_t)(isc_nmhandle_t *handle, isc_result_t result,
			    void *cbarg);
/*%<
 * Callback function for other network completion events (send, connect).
 *
 * 'handle' the handle on which the event took place.
 * 'eresult' the result of the event.
 * 'cbarg'  the callback argument passed to isc_nm_send(),
 *          isc_nm_tcp_connect(), or isc_nm_listentcp()
 */

typedef void (*isc_nm_opaquecb_t)(void *arg);
/*%<
 * Opaque callback function, used for isc_nmhandle 'reset' and 'free'
 * callbacks.
 */

typedef void (*isc_nm_workcb_t)(void *arg);
typedef void (*isc_nm_after_workcb_t)(void *arg, isc_result_t result);
/*%<
 * Callback functions for libuv threadpool work (see uv_work_t)
 */

void
isc_nm_attach(isc_nm_t *mgr, isc_nm_t **dst);
void
isc_nm_detach(isc_nm_t **mgr0);
/*%<
 * Attach/detach a network manager. When all references have been
 * released, the network manager is shut down, freeing all resources.
 * Destroy is working the same way as detach, but it actively waits
 * for all other references to be gone.
 */

/* Return thread ID of current thread, or ISC_NETMGR_TID_UNKNOWN */
int
isc_nm_tid(void);

void
isc_nmsocket_close(isc_nmsocket_t **sockp);
/*%<
 * isc_nmsocket_close() detaches a listening socket that was
 * created by isc_nm_listenudp(), isc_nm_listentcp(), or
 * isc_nm_listentcpdns(). Once there are no remaining child
 * sockets with active handles, the socket will be closed.
 */

#ifdef NETMGR_TRACE
#define isc_nmhandle_attach(handle, dest) \
	isc__nmhandle_attach(handle, dest, __FILE__, __LINE__, __func__)
#define isc_nmhandle_detach(handlep) \
	isc__nmhandle_detach(handlep, __FILE__, __LINE__, __func__)
#define FLARG , const char *file, unsigned int line, const char *func
#else
#define isc_nmhandle_attach(handle, dest) isc__nmhandle_attach(handle, dest)
#define isc_nmhandle_detach(handlep)	  isc__nmhandle_detach(handlep)
#define FLARG
#endif

void
isc__nmhandle_attach(isc_nmhandle_t *handle, isc_nmhandle_t **dest FLARG);
void
isc__nmhandle_detach(isc_nmhandle_t **handlep FLARG);
/*%<
 * Increment/decrement the reference counter in a netmgr handle,
 * but (unlike the attach/detach functions) do not change the pointer
 * value. If reference counters drop to zero, the handle can be
 * marked inactive, possibly triggering deletion of its associated
 * socket.
 *
 * (This will be used to prevent a client from being cleaned up when
 * it's passed to an isc_task event handler. The libuv code would not
 * otherwise know that the handle was in use and might free it, along
 * with the client.)
 */
#undef FLARG

void *
isc_nmhandle_getdata(isc_nmhandle_t *handle);

void *
isc_nmhandle_getextra(isc_nmhandle_t *handle);

bool
isc_nmhandle_is_stream(isc_nmhandle_t *handle);

void
isc_nmhandle_setdata(isc_nmhandle_t *handle, void *arg,
		     isc_nm_opaquecb_t doreset, isc_nm_opaquecb_t dofree);
/*%<
 * isc_nmhandle_t has a void* opaque field (for example, ns_client_t).
 * We reuse handle and `opaque` can also be reused between calls.
 * This function sets this field and two callbacks:
 * - doreset resets the `opaque` to initial state
 * - dofree frees everything associated with `opaque`
 */

void
isc_nmhandle_settimeout(isc_nmhandle_t *handle, uint32_t timeout);
void
isc_nmhandle_cleartimeout(isc_nmhandle_t *handle);
/*%<
 * Set/clear the read/recv timeout for the socket connected to 'handle'
 * to 'timeout' (in milliseconds), and reset the timer.
 *
 * When this is called on a 'wrapper' socket handle (for example,
 * a TCPDNS socket wrapping a TCP connection), the timer is set for
 * both socket layers.
 */
bool
isc_nmhandle_timer_running(isc_nmhandle_t *handle);
/*%<
 * Return true if the timer for the socket connected to 'handle'
 * is running.
 */

void
isc_nmhandle_keepalive(isc_nmhandle_t *handle, bool value);
/*%<
 * Enable/disable keepalive on this connection by setting it to 'value'.
 *
 * When keepalive is active, we switch to using the keepalive timeout
 * to determine when to close a connection, rather than the idle timeout.
 *
 * This applies only to TCP-based DNS connections (i.e., TCPDNS or
 * TLSDNS). On other types of connection it has no effect.
 */

isc_sockaddr_t
isc_nmhandle_peeraddr(isc_nmhandle_t *handle);
/*%<
 * Return the peer address for the given handle.
 */
isc_sockaddr_t
isc_nmhandle_localaddr(isc_nmhandle_t *handle);
/*%<
 * Return the local address for the given handle.
 */

isc_nm_t *
isc_nmhandle_netmgr(isc_nmhandle_t *handle);
/*%<
 * Return a pointer to the netmgr object for the given handle.
 */

isc_result_t
isc_nm_listenudp(isc_nm_t *mgr, isc_sockaddr_t *iface, isc_nm_recv_cb_t cb,
		 void *cbarg, size_t extrasize, isc_nmsocket_t **sockp);
/*%<
 * Start listening for UDP packets on interface 'iface' using net manager
 * 'mgr'.
 *
 * On success, 'sockp' will be updated to contain a new listening UDP socket.
 *
 * When a packet is received on the socket, 'cb' will be called with 'cbarg'
 * as its argument.
 *
 * When handles are allocated for the socket, 'extrasize' additional bytes
 * can be allocated along with the handle for an associated object, which
 * can then be freed automatically when the handle is destroyed.
 */

void
isc_nm_udpconnect(isc_nm_t *mgr, isc_sockaddr_t *local, isc_sockaddr_t *peer,
		  isc_nm_cb_t cb, void *cbarg, unsigned int timeout,
		  size_t extrahandlesize);
/*%<
 * Open a UDP socket, bind to 'local' and connect to 'peer', and
 * immediately call 'cb' with a handle so that the caller can begin
 * sending packets over UDP.
 *
 * When handles are allocated for the socket, 'extrasize' additional bytes
 * can be allocated along with the handle for an associated object, which
 * can then be freed automatically when the handle is destroyed.
 *
 * 'timeout' specifies the timeout interval in milliseconds.
 *
 * The connected socket can only be accessed via the handle passed to
 * 'cb'.
 */

isc_result_t
isc_nm_routeconnect(isc_nm_t *mgr, isc_nm_cb_t cb, void *cbarg,
		    size_t extrahandlesize);
/*%<
 * Open a route/netlink socket and call 'cb', so the caller can be
 * begin listening for interface changes.  This behaves similarly to
 * isc_nm_udpconnect().
 *
 * Returns ISC_R_NOTIMPLEMENTED on systems where route/netlink sockets
 * are not supported.
 */

void
isc_nm_stoplistening(isc_nmsocket_t *sock);
/*%<
 * Stop listening on socket 'sock'.
 */

void
isc_nm_pause(isc_nm_t *mgr);
/*%<
 * Pause all processing, equivalent to taskmgr exclusive tasks.
 * It won't return until all workers have been paused.
 */

void
isc_nm_resume(isc_nm_t *mgr);
/*%<
 * Resume paused processing. It will return immediately after signalling
 * workers to resume.
 */

void
isc_nm_read(isc_nmhandle_t *handle, isc_nm_recv_cb_t cb, void *cbarg);
/*
 * Begin (or continue) reading on the socket associated with 'handle', and
 * update its recv callback to 'cb', which will be called as soon as there
 * is data to process.
 */

void
isc_nm_pauseread(isc_nmhandle_t *handle);
/*%<
 * Pause reading on this handle's socket, but remember the callback.
 *
 * Requires:
 * \li	'handle' is a valid netmgr handle.
 */

void
isc_nm_cancelread(isc_nmhandle_t *handle);
/*%<
 * Cancel reading on a connected socket. Calls the read/recv callback on
 * active handles with a result code of ISC_R_CANCELED.
 *
 * Requires:
 * \li	'sock' is a valid netmgr socket
 * \li	...for which a read/recv callback has been defined.
 */

void
isc_nm_resumeread(isc_nmhandle_t *handle);
/*%<
 * Resume reading on the handle's socket.
 *
 * Requires:
 * \li	'handle' is a valid netmgr handle.
 * \li	...for a socket with a defined read/recv callback.
 */

void
isc_nm_send(isc_nmhandle_t *handle, isc_region_t *region, isc_nm_cb_t cb,
	    void *cbarg);
/*%<
 * Send the data in 'region' via 'handle'. Afterward, the callback 'cb' is
 * called with the argument 'cbarg'.
 *
 * 'region' is not copied; it has to be allocated beforehand and freed
 * in 'cb'.
 */

isc_result_t
isc_nm_listentcp(isc_nm_t *mgr, isc_sockaddr_t *iface,
		 isc_nm_accept_cb_t accept_cb, void *accept_cbarg,
		 size_t extrahandlesize, int backlog, isc_quota_t *quota,
		 isc_nmsocket_t **sockp);
/*%<
 * Start listening for raw messages over the TCP interface 'iface', using
 * net manager 'mgr'.
 *
 * On success, 'sockp' will be updated to contain a new listening TCP
 * socket.
 *
 * When connection is accepted on the socket, 'accept_cb' will be called with
 * 'accept_cbarg' as its argument. The callback is expected to start a read.
 *
 * When handles are allocated for the socket, 'extrasize' additional bytes
 * will be allocated along with the handle for an associated object.
 *
 * If 'quota' is not NULL, then the socket is attached to the specified
 * quota. This allows us to enforce TCP client quota limits.
 *
 */

void
isc_nm_tcpconnect(isc_nm_t *mgr, isc_sockaddr_t *local, isc_sockaddr_t *peer,
		  isc_nm_cb_t cb, void *cbarg, unsigned int timeout,
		  size_t extrahandlesize);
/*%<
 * Create a socket using netmgr 'mgr', bind it to the address 'local',
 * and connect it to the address 'peer'.
 *
 * When the connection is complete or has timed out, call 'cb' with
 * argument 'cbarg'. Allocate 'extrahandlesize' additional bytes along
 * with the handle to use for an associated object.
 *
 * 'timeout' specifies the timeout interval in milliseconds.
 *
 * The connected socket can only be accessed via the handle passed to
 * 'cb'.
 */

isc_result_t
isc_nm_listentcpdns(isc_nm_t *mgr, isc_sockaddr_t *iface,
		    isc_nm_recv_cb_t recv_cb, void *recv_cbarg,
		    isc_nm_accept_cb_t accept_cb, void *accept_cbarg,
		    size_t extrahandlesize, int backlog, isc_quota_t *quota,
		    isc_nmsocket_t **sockp);
/*%<
 * Start listening for DNS messages over the TCP interface 'iface', using
 * net manager 'mgr'.
 *
 * On success, 'sockp' will be updated to contain a new listening TCPDNS
 * socket. This is a wrapper around a raw TCP socket, which sends and
 * receives DNS messages via that socket. It handles message buffering
 * and pipelining, and automatically prepends messages with a two-byte
 * length field.
 *
 * When a complete DNS message is received on the socket, 'cb' will be
 * called with 'cbarg' as its argument.
 *
 * When a new TCPDNS connection is accepted, 'accept_cb' will be called
 * with 'accept_cbarg' as its argument.
 *
 * When handles are allocated for the socket, 'extrasize' additional bytes
 * will be allocated along with the handle for an associated object
 * (typically ns_client).
 *
 * 'quota' is passed to isc_nm_listentcp() when opening the raw TCP socket.
 */

isc_result_t
isc_nm_listentlsdns(isc_nm_t *mgr, isc_sockaddr_t *iface,
		    isc_nm_recv_cb_t recv_cb, void *recv_cbarg,
		    isc_nm_accept_cb_t accept_cb, void *accept_cbarg,
		    size_t extrahandlesize, int backlog, isc_quota_t *quota,
		    isc_tlsctx_t *sslctx, isc_nmsocket_t **sockp);
/*%<
 * Same as isc_nm_listentcpdns but for an SSL (DoT) socket.
 */

void
isc_nm_settimeouts(isc_nm_t *mgr, uint32_t init, uint32_t idle,
		   uint32_t keepalive, uint32_t advertised);
/*%<
 * Sets the initial, idle, and keepalive timeout values (in milliseconds) to use
 * for TCP connections, and the timeout value to advertise in responses using
 * the EDNS TCP Keepalive option (which should ordinarily be the same
 * as 'keepalive'), in milliseconds.
 *
 * Requires:
 * \li	'mgr' is a valid netmgr.
 */

void
isc_nm_setnetbuffers(isc_nm_t *mgr, int32_t recv_tcp, int32_t send_tcp,
		     int32_t recv_udp, int32_t send_udp);
/*%<
 * If not 0, sets the SO_RCVBUF and SO_SNDBUF socket options for TCP and UDP
 * respectively.
 *
 * Requires:
 * \li	'mgr' is a valid netmgr.
 */

void
isc_nm_gettimeouts(isc_nm_t *mgr, uint32_t *initial, uint32_t *idle,
		   uint32_t *keepalive, uint32_t *advertised);
/*%<
 * Gets the initial, idle, keepalive, or advertised timeout values,
 * in milliseconds.
 *
 * Any integer pointer parameter not set to NULL will be updated to
 * contain the corresponding timeout value.
 *
 * Requires:
 * \li	'mgr' is a valid netmgr.
 */

void
isc_nm_maxudp(isc_nm_t *mgr, uint32_t maxudp);
/*%<
 * Simulate a broken firewall that blocks UDP messages larger than a given
 * size.
 */

void
isc_nm_setstats(isc_nm_t *mgr, isc_stats_t *stats);
/*%<
 * Set a socket statistics counter set 'stats' for 'mgr'.
 *
 * Requires:
 *\li	'mgr' is valid and doesn't have stats already set.
 *
 *\li	stats is a valid set of statistics counters supporting the
 *	full range of socket-related stats counter numbers.
 */

isc_result_t
isc_nm_checkaddr(const isc_sockaddr_t *addr, isc_socktype_t type);
/*%<
 * Check whether the specified address is available on the local system
 * by opening a socket and immediately closing it.
 *
 * Requires:
 *\li	'addr' is not NULL.
 */

void
isc_nm_tcpdnsconnect(isc_nm_t *mgr, isc_sockaddr_t *local, isc_sockaddr_t *peer,
		     isc_nm_cb_t cb, void *cbarg, unsigned int timeout,
		     size_t extrahandlesize);
void
isc_nm_tlsdnsconnect(isc_nm_t *mgr, isc_sockaddr_t *local, isc_sockaddr_t *peer,
		     isc_nm_cb_t cb, void *cbarg, unsigned int timeout,
		     size_t extrahandlesize, isc_tlsctx_t *sslctx);
/*%<
 * Establish a DNS client connection via a TCP or TLS connection, bound to
 * the address 'local' and connected to the address 'peer'.
 *
 * When the connection is complete or has timed out, call 'cb' with
 * argument 'cbarg'. Allocate 'extrahandlesize' additional bytes along
 * with the handle to use for an associated object.
 *
 * 'timeout' specifies the timeout interval in milliseconds.
 *
 * The connected socket can only be accessed via the handle passed to
 * 'cb'.
 */

/*%<
 * Returns 'true' iff 'handle' is associated with a socket of type
 * 'isc_nm_tlsdnssocket'.
 */

bool
isc_nm_is_http_handle(isc_nmhandle_t *handle);
/*%<
 * Returns 'true' iff 'handle' is associated with a socket of type
 * 'isc_nm_httpsocket'.
 */

#if HAVE_LIBNGHTTP2

#define ISC_NM_HTTP_DEFAULT_PATH "/dns-query"

isc_result_t
isc_nm_listentls(isc_nm_t *mgr, isc_sockaddr_t *iface,
		 isc_nm_accept_cb_t accept_cb, void *accept_cbarg,
		 size_t extrahandlesize, int backlog, isc_quota_t *quota,
		 isc_tlsctx_t *sslctx, isc_nmsocket_t **sockp);

void
isc_nm_tlsconnect(isc_nm_t *mgr, isc_sockaddr_t *local, isc_sockaddr_t *peer,
		  isc_nm_cb_t cb, void *cbarg, isc_tlsctx_t *ctx,
		  unsigned int timeout, size_t extrahandlesize);

void
isc_nm_httpconnect(isc_nm_t *mgr, isc_sockaddr_t *local, isc_sockaddr_t *peer,
		   const char *uri, bool POST, isc_nm_cb_t cb, void *cbarg,
		   isc_tlsctx_t *ctx, unsigned int timeout,
		   size_t extrahandlesize);

isc_result_t
isc_nm_listenhttp(isc_nm_t *mgr, isc_sockaddr_t *iface, int backlog,
		  isc_quota_t *quota, isc_tlsctx_t *ctx,
		  isc_nm_http_endpoints_t *eps, uint32_t max_concurrent_streams,
		  isc_nmsocket_t **sockp);

isc_nm_http_endpoints_t *
isc_nm_http_endpoints_new(isc_mem_t *mctx);
/*%<
 * Create a new, empty HTTP endpoints set object.
 *
 * Requires:
 * \li 'mctx' a valid memory context object.
 */

isc_result_t
isc_nm_http_endpoints_add(isc_nm_http_endpoints_t *restrict eps,
			  const char *uri, const isc_nm_recv_cb_t cb,
			  void *cbarg, const size_t extrahandlesize);
/*%< Adds a new endpoint to the given HTTP endpoints set object.
 *
 * NOTE: adding an endpoint is allowed only if the endpoint object has
 * not been passed to isc_nm_listenhttp() yet.
 *
 * Requires:
 * \li 'eps' is a valid pointer to a valid isc_nm_http_endpoints_t
 * object;
 * \li 'uri' is a valid pointer to a string of length > 0;
 * \li 'cb' is a valid pointer to a read callback function.
 */

void
isc_nm_http_endpoints_attach(isc_nm_http_endpoints_t  *source,
			     isc_nm_http_endpoints_t **targetp);
/*%<
 * Attaches to an HTTP endpoints set object.
 *
 * Requires:
 * \li 'source' is a non-NULL pointer to a valid
 * isc_nm_http_endpoints_t object;
 * \li 'target' is a pointer to a pointer, containing NULL.
 */

void
isc_nm_http_endpoints_detach(isc_nm_http_endpoints_t **restrict epsp);
/*%<
 * Detaches from an HTTP endpoints set object. When reference count
 * reaches 0, the object get deleted.
 *
 * Requires:
 * \li 'epsp' is a pointer to a pointer to a valid
 * isc_nm_http_endpoints_t object.
 */

bool
isc_nm_http_path_isvalid(const char *path);
/*%<
 * Returns 'true' if 'path' matches the format requirements for
 * the path component of a URI as defined in RFC 3986 section 3.3.
 */

void
isc_nm_http_makeuri(const bool https, const isc_sockaddr_t *sa,
		    const char *hostname, const uint16_t http_port,
		    const char *abs_path, char *outbuf,
		    const size_t outbuf_len);
/*%<
 * Makes a URI connection string out of na isc_sockaddr_t object 'sa'
 * or the specified 'hostname' and 'http_port'.
 *
 * Requires:
 * \li 'abs_path' is a valid absolute HTTP path string;
 * \li 'outbuf' is a valid pointer to a buffer which will get the result;
 * \li 'outbuf_len' is a size of the result buffer and is greater than zero.
 */
#endif /* HAVE_LIBNGHTTP2 */

void
isc_nm_bad_request(isc_nmhandle_t *handle);
/*%<
 * Perform a transport protocol specific action on the handle in case of a
 * bad/malformed incoming DNS message.
 *
 * NOTE: The function currently is no-op for any protocol except HTTP/2.
 *
 * Requires:
 *  \li 'handle' is a valid netmgr handle object.
 */

bool
isc_nm_xfr_allowed(isc_nmhandle_t *handle);
/*%<
 * Check if it is possible to do a zone transfer over the given handle.
 *
 * Requires:
 * \li	'handle' is a valid connection handle.
 */

void
isc_nm_set_maxage(isc_nmhandle_t *handle, const uint32_t ttl);
/*%<
 * Set the minimal time to live from the server's response Answer
 * section as a hint to the underlying transport.
 *
 * NOTE: The function currently is no-op for any protocol except HTTP/2.
 *
 * Requires:
 *
 * \li 'handle' is a valid netmgr handle object associated with an accepted
 * connection.
 */

isc_nmsocket_type
isc_nm_socket_type(const isc_nmhandle_t *handle);
/*%<
 * Returns the handle's underlying socket type.
 *
 * Requires:
 *  \li 'handle' is a valid netmgr handle object.
 */

bool
isc_nm_has_encryption(const isc_nmhandle_t *handle);
/*%<
 * Returns 'true' iff the handle's underlying transport does encryption.
 *
 * Requires:
 *  \li 'handle' is a valid netmgr handle object.
 */

void
isc_nm_task_enqueue(isc_nm_t *mgr, isc_task_t *task, int tid);
/*%<
 * Enqueue the 'task' onto the netmgr ievents queue.
 *
 * Requires:
 * \li 'mgr' is a valid netmgr object
 * \li 'task' is a valid task
 * \li 'tid' is either the preferred netmgr tid or -1, in which case
 *     tid will be picked randomly. The tid is capped (by modulo) to
 *     maximum number of 'workers' as specifed in isc_nm_start()
 */

void
isc_nm_work_offload(isc_nm_t *mgr, isc_nm_workcb_t work_cb,
		    isc_nm_after_workcb_t after_work_cb, void *data);
/*%<
 * Schedules a job to be handled by the libuv thread pool (see uv_work_t).
 * The function specified in `work_cb` will be run by a thread in the
 * thread pool; when complete, the `after_work_cb` function will run.
 *
 * Requires:
 * \li 'mgr' is a valid netmgr object.
 * \li We are currently running in a network manager thread.
 */

void
isc__nm_force_tid(int tid);
/*%<
 * Force the thread ID to 'tid'. This is STRICTLY for use in unit
 * tests and should not be used in any production code.
 */

uint32_t
isc_nm_getnworkers(const isc_nm_t *);
/*%<
 * Return the number of active workers
 */

void
isc_nmhandle_setwritetimeout(isc_nmhandle_t *handle, uint64_t write_timeout);

/*
 * Timer related functions
 */

typedef struct isc_nm_timer isc_nm_timer_t;

typedef void (*isc_nm_timer_cb)(void *, isc_result_t);

void
isc_nm_timer_create(isc_nmhandle_t *, isc_nm_timer_cb, void *,
		    isc_nm_timer_t **);

void
isc_nm_timer_attach(isc_nm_timer_t *, isc_nm_timer_t **);

void
isc_nm_timer_detach(isc_nm_timer_t **);

void
isc_nm_timer_start(isc_nm_timer_t *, uint64_t);

void
isc_nm_timer_stop(isc_nm_timer_t *);
