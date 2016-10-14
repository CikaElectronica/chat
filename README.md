# chat
Send-Expect type chat, as in modem AT command session

define CHAT_FULLCHAT for a multiple send-expect pair dialog engine

check chat.h for wildcards and documentation

###Timeout:
To handle a timeout situation, the library requires a timer. As each developer will
probably handle timers its own way... and different micros have different atomic access
capabilities, there is an external type for a timer: sys_timer_t. Each one will
define it at will. It can be found (and modified) in sys_timers.h
The library handles this timer as a volatile variable and requires a pointer to it;
that is, it expects "someone from the outside world" to change its value.

###Quick start:
chat_init() initializes the handler structure
chat_start() resets the pointers, starts the process
chat_tick() handles the chat process

Same with 'full' (chat_fullinit(), etc) for full dialog

####E.g.:
```
chat_t mychat;

	chat_init(&mychat, &myfuncs, &CHAT_STIMER);
	chat_start(&mychat, "Waiting for 'resp'", "resp", CHAT_MAXTIME);
	...
	switch(chat_tick(&mychat)){
	case CHAT_AGAIN:
		// call again..
		break;
	case CHAT_ERROR:
		// oops, timeout
		break;
	case CHAT_DONE:
		// sent what you asked, got what you expected
		break;
	}

sendexpect_t multipledialog[] = {
	{"Waiting for 'resp'", "resp"},
	{send2, expect2},
	{0,0}
};
fullchat_t myfullchat;

	chat_fullinit(&myfullchat, &myfuncs, &CHAT_STIMER);
	chat_fullstart(&myfullchat, (sendexpect_t *)&multipledialog, CHAT_MAXTIME);
	call chat_fulltick(&myfullchat) same as you did with chat_tick()
```
