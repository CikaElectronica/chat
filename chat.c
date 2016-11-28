/** \file chat.c
*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "chat.h"

enum {
// Chat states
	CHAT_SEND,
	CHAT_EXPECT,
// fullChat states
	CHAT_NEXT,
	CHAT_CHAT,
// common states
	CHAT_IDLE,
};

int chat_tick(chat_t *chat)
{
char c, e;
const chatfunctions_t *func = chat->func;

	switch(chat->state){
	case CHAT_SEND:
		while((c = *chat->ptr) != '\0'){		// pump while there are chars
			if((*func->talk)(c) == -1)		// and room to put them
				return CHAT_AGAIN;		// wait till room
			++(chat->ptr);
		}
		if((chat->ptr = chat->expect) != NULL){		// point to expected string (if there is something to expect)
#ifdef CHAT_FLUSHINPUTAFTERSEND
			(*func->ignore)();			// flush input
#endif
			*chat->timer = chat->timeout;		// start timer
			chat->state = CHAT_EXPECT;		// and do it
		}
		else {
			goto done;
		}
		break;	
	case CHAT_EXPECT:
		if(*chat->timer == 0) {				// exit on timeout
			chat->state = CHAT_IDLE;
			return CHAT_ERROR;
		}
		c = *chat->ptr;
		while(c != '\0'){				// compare our string
			int ret;
			if((ret = (*func->listen)()) == -1)
				return CHAT_AGAIN;		// wait if nothing comes out from the outside world
			e = (char)tolower(ret);			// to received chars (case insensitive)
			if(c == ' '){				// space matches any amount of white space
				if(!isspace(e))			// if not white space any more
					c = *(++chat->ptr);	// then eval next character
				else
					continue;		// discard white space
			}
			if((c != 'ç') && (c != e)) {		// 'ç' matches any character, including white space
				chat->ptr = chat->expect;	// restart comparison after a difference
				return CHAT_AGAIN;
			}
			c = *(++(chat->ptr));
		}		
done:		chat->state = CHAT_IDLE;
		return CHAT_DONE;
	case CHAT_IDLE:
	default:
		return CHAT_ERROR;
	}
	return CHAT_AGAIN;
}

void chat_init(chat_t *chat, const chatfunctions_t *func, volatile sys_timer_t *timer)
{
	chat->func = func;
	chat->timer = timer;
}

void chat_start(chat_t *chat, char *send, char *expect, unsigned int timeout)
{
	chat->state = CHAT_SEND;
	chat->expect = expect;
	chat->ptr = send;
	chat->timeout = (sys_timer_t) timeout;
}

#ifdef CHAT_FULLCHAT
int chat_fulltick(fullchat_t *chat)
{
int ret;
sendexpect_t *strings = chat->strings;

	switch(chat->state){
	case CHAT_NEXT:
		if(strings->send == NULL || strings-> expect == NULL) { // get next sendexpect pair
			chat->state = CHAT_IDLE;		// no more pairs, done
			return CHAT_DONE;
		}
		chat->state = CHAT_CHAT;			// send it
		chat_start(&chat->chat, strings->send, strings->expect, chat->chat.timeout);
		/*FALLTHROUGH*/
	case CHAT_CHAT:
		while((ret = chat_tick(&chat->chat)) ==  CHAT_AGAIN)
			return CHAT_AGAIN;
		if(ret == CHAT_ERROR) {
			chat->state = CHAT_IDLE;		// no more strings, done
			return CHAT_ERROR;
		}			
		++chat->strings;				// next string to expect 
		chat->state = CHAT_NEXT;
		break;
	case CHAT_IDLE:
	default:
		return CHAT_ERROR;
	}
	return CHAT_AGAIN;
}

void chat_fullinit(fullchat_t *chat, const chatfunctions_t *func, volatile sys_timer_t *timer)
{
	chat->chat.func = func;
	chat->chat.timer = timer;
	chat_init(&chat->chat, func, timer);
}

void chat_fullstart(fullchat_t *chat, sendexpect_t *strings, unsigned int timeout)
{
	chat->state = CHAT_NEXT;
	chat->strings = strings;
	chat_start(&chat->chat, strings->send, strings->expect, timeout);
}
#endif
