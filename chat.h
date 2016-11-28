/** \file chat.h
*/
#ifndef __CHAT_H
#define __CHAT_H

// Return values
enum {
	CHAT_AGAIN,
	CHAT_ERROR,
	CHAT_DONE
};

#include "sys_timers.h"

typedef struct {
	int (*talk)(unsigned char); 	///< pointer to function to send a char (returns -1 if need to wait)
	int (*listen)(void); 		///< pointer to function to receive a char (returns available char, -1 if nothing)
#ifdef CHAT_FLUSHINPUTAFTERSEND
	void (*ignore)(void); 		///< pointer to function to flush receive buffer
#endif
} chatfunctions_t;

typedef struct {
	char *ptr;			///< point to current char in current string being sent or expected
	const chatfunctions_t *func;	///< pointer to I/O functions
	volatile sys_timer_t *timer;	///< pointer to timer (decremented outside of this function)
	char *expect;			///< structure containing pointers to strings
	sys_timer_t timeout;		///< time to wait for a response (expect)
	uint8_t state;			///< handler state
} chat_t;

#ifdef CHAT_FULLCHAT
typedef struct {
	char *send;		///< string to be sent
	char *expect;		///< string to expect in response
} sendexpect_t;

typedef struct {
	sendexpect_t *strings;		///< pointer to an array of sendexpect_t (send/expect pairs)
	chat_t chat;			///< internal send-expect chat type
	uint8_t state;			///< handler state
} fullchat_t;
#endif

/** Initializes control data
\param1	control type, to be initialized
\param2 pointer to I/O functions
\param3 pointer to externally handled time variable (timer), counting down to zero
*/
void chat_init(chat_t *chat, const chatfunctions_t *func, volatile sys_timer_t *timer);

/** Starts control data for this chat
\param1	control type, to be initialized
\param2 pointer to send string
\param3 pointer to expect string
\param4 time to wait (in timer units, depends on system handling)
\note "Send" strings are sent 'as is'
\note "Expect" strings have some rules:
	 'ç' matches any single character, including white space
	 ' ' matches some white space
	 no white space at the end of the string
	 all chars must be lower case (comparison is if(expectchar == tolower(char)))
*/
void chat_start(chat_t *chat, char *send, char *expect, unsigned int timeout);

/** Handles chat stuff
\param	control type, initialized by chat_init() and chat_start()
\retval	CHAT_AGAIN, call again
\retval	CHAT_ERROR, timeout or called without proper initialization + start
\retval	CHAT_DONE,  fine, job is done.
*/
int chat_tick(chat_t *chat);

#ifdef CHAT_FULLCHAT
/** Initializes control data
\param1	control type, to be initialized
\param2 pointer to I/O functions
\param3 pointer to externally handled time variable (timer), counting down to zero
*/
void chat_fullinit(fullchat_t *chat, const chatfunctions_t *func, volatile sys_timer_t *timer);

/** Starts control data for this chat
\param1	control type, to be initialized
\param2 pointer to sendexpect type (array of pairs, pointers to send/expect strings)
\param3 time to wait (in timer units, depends on system handling)
\note See chat_start for details regarding send/expect strings
*/
void chat_fullstart(fullchat_t *chat, sendexpect_t *strings, unsigned int timeout);

/** Handles chat stuff
\param	control type, initialized by chat_fullinit() and chat_fullstart()
\retval	CHAT_AGAIN, call again
\retval	CHAT_ERROR, timeout or called without proper initialization + start
\retval	CHAT_DONE,  fine, job is done.
*/
int chat_fulltick(fullchat_t *chat);
#endif

#endif
