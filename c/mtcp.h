#ifndef __MTCP_H__
#define __MTCP_H__

#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
typedef int ssize_t;
#define EMSGSIZE WSAEMSGSIZE
#endif

/** \file mtcp.h
 *
 * Message TCP (MTCP) and Transaction MTCP (TMTCP).
 *
 * Protocols based on TCP that provide a message (reliable datagram)
 * abstraction (MTCP) and an abstraction for a transactional sequence of
 * messages (TMTCP) on top of the continuous data streams of TCP.
 *
 * open(), listen(), bind(), connect(), close() are all the same as usual.
 *
 * The new functions mtcp_recv_message() and mtcp_send_message() wrap around
 * recv() and send() to manage message discretization. The new functions
 * tmtcp_recv_message() and tmtcp_send_message() additionally manage
 * transaction identity.
 *
 * These functions are the basic mechanism to send and receive messages
 * between app servers, RSUs, and the signage server, and between the
 * vehicle app code and the OBU proxy (running on the vehicle) which manages
 * roadside UDP transmissions.
 *
 * Transactions and TCP sessions are in a many-to-one relationship. On the
 * server side, it is likely that many transactions will be occurring (even
 * simultaneously) on the same session (thus the need for TMTCP). On the
 * vehicle side, there is one transaction per session (see obu.h), partly
 * because session setup costs are much lower locally than over GPRS backhaul.
 * (For this reason, we use MTCP rather than TMTCP on the vehicle side.)
 */

/** \brief MTCP state
 *
 * State of current MTCP transmission. Only the socket file descriptor member
 * should be accessed directly by callers of the MTCP API. The struct should
 * be initialized with the mtcp_init_state() function, which takes the socket
 * as an argument. A clean mtcp_state should be used for each message. The
 * struct should be reinitialized with mtcp_init_state() between messages
 * (but not between several calls to send or recv a single message with a
 * non-blocking socket). Each thread should use a different struct.
 *
 * <em> Note: non-blocking IO is \b not implemented on Windows (use threads). <\em>
 */
typedef struct {
    int             sock;
    size_t          msglen;
    size_t          bufpos;
    char            lenbuf[4];
    size_t          lenbufpos;
} mtcp_state;

/** \brief initialize MTCP state
 *
 * Should be called for each message sent. (If the socket is non-blocking,
 * do not reinitialize for repeated send/recv calls on the same message.)
 *
 * \param pmts      the mtcp_state to be used
 * \param s         the socket file descriptor that it will use.
 *
 * It is up to the caller to initialize the socket s itself (including the
 * connect() call, setsockopt() with O_NONBLOCK, etc.).
 */
extern void mtcp_init_state(mtcp_state *pmts, int s);

/** \brief receive a single message on a TCP socket using the MTCP protocol
 *
 * \param pmts          pointer to the MTCP params (struct given by caller)
 * \param buf           as in recv()
 * \param len           as in recv()
 * \return              as in recv(), but see below
 *
 * Arguments are as in the standard recv() (see 'man 2 recv'), except that the
 * socket argument is wrapped in a mtcp_state struct and that that no
 * flags argument is used. The same mtcp_state struct should be used for all 
 * mtcp_recv_message calls on this socket (but use a different one for send
 * calls).
 *
 * The difference is that the stream is chopped into discrete messages using
 * length fields (4 octets each, stored in network order). The value of length
 * field is the length of the subsequent message, exclusive of the length
 * field itself. Each call to mtcp_recv_message() returns at most one such
 * message, even if more data is available. If no complete message is
 * available, the call blocks until a complete message is available, unless
 * the socket was placed in non-blocking mode. The length field is not
 * accessible to callers of the MTCP API -- it is managed internally.
 *
 * The return value is as usual: 0 means peer has closed its socket, -1 means
 * an error, and a positive number is the length of the message copied into
 * buf. Error codes are in errno. One additional error code is generated,
 * beyond what recv() generates: EMSGSIZE is generated if the received message
 * has a length field that indicates that the message is too long for buf.
 * As usual, EAGAIN in the non-blocking case indicates that the function
 * should be called again to finish the receive operation, using the same pmts.
 */
extern ssize_t mtcp_recv_message(mtcp_state *pmts, void *buf, size_t len);

/** \brief send a single message on a TCP socket using the MTCP protocol
 *
 * \param pmts          pointer to the MTCP params (struct given by caller)
 * \param buf           as in send()
 * \param len           as in send()
 * \return              as in send()
 *
 * Arguments are as in the standard send() (see 'man 2 send'), except that the
 * socket argument is wrapped in a mtcp_state struct and that that no
 * flags argument is used.
 *
 * The difference is that the stream is chopped into discrete messages using
 * length fields (as described at mtcp_recv_message()). Each call to
 * mtcp_send_message() sends at most one such message. The length field is not
 * accessible to callers of the MTCP API--it is managed internally.
 *
 * The return value is as usual: -1 means an error, and a positive number is
 * the length of the message sent. Error codes are in errno.
 * As usual, EAGAIN in the non-blocking case indicates that the function
 * should be called again to finish the send operation, using the same pmts.
 */
extern ssize_t mtcp_send_message(mtcp_state *pmts, const void *buf, size_t len);

/** \brief Get the blocking/non-blocking state of the socket.
 *
 * \param pmts          pointer to the MTCP params (struct given by caller)
 * \return             1 if blocking, 0 if non-blocking.
 */
int mtcp_get_block_state(mtcp_state *pmts);

#endif // __MTCP_H__
