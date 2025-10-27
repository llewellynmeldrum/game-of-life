#ifndef _log_H
#define _log_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <libgen.h>
#define adjust 10'000'000
#define msleep(ms) usleep(ms*1000)

// useful macro helpers
#define __MACRO_TO_STR(ARG) 	#ARG
#define MACRO_TO_STR(ARG)	__MACRO_TO_STR(ARG)

#define randf(max)  (((double)( (rand()*adjust)%(max*adjust) ))/10'000'000.0)
#ifdef DEBUG_PRINTING
#define potential_sleep(ms)do{\
/*		vec2 temp = getcurpos();\
//		_log("%0d,%0d",cury(),curx());\
		mvcurv(debug_pos);\
*/		usleep(ms*1000);\
		refresh();\
/*		mvcurv(temp);\ 
*/		}while(0)
#else
#define potential_sleep(ms) do{  }while(0);
#endif
	extern bool show_pretty_print_guide;
extern char copybuf[];


ssize_t GET_TERM_COLS();
void _log_UPPER_SEPARATOR();
void _log_LOWER_SEPARATOR();
void dprintbuf(const char* title, const char* buf, ssize_t sz, ssize_t linec_to_print);
#define pretty_print_buffer(title, buf, lncount) dprintbuf(title, buf, strlen(buf), lncount);


#define NOP do{}while(0)
#ifdef DISABLE__log
	#define _log(fmt, ...) NOP
#else
	#define _log(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#endif


#define _logexit(n)do {				\
	if(n == EXIT_SUCCESS){ _log(SET_GREEN);}	\
	else {_log(SET_RED);}			\
	_log("\nExiting...%s\n\n",SET_CLEAR);	\
	exit(n);				\
	} while(0)


#define _logerrno() do{			\
	_log("ERRNO(%d):",errno);	\
	perror("");			\
	}while(0)


/* functions can define FN_NAME to idenify themselves in _logging:
 * 	- any FATAL messages,
 *   	- any WARNING messages (planned)
 *   	- any CRIT_WARNING messages (planned)
 * */

#define ALLOC_FAILURE(resource) "Failed to allocate memory for '%s'!\n", #resource
#ifdef FN_NAME

#define _logfatal(fmt, ...) do{							\
	_log("\n\033[31;1m[FATAL ERROR IN " FN_NAME "] ->\033[0m \033[31m");	\
	_log(fmt, ##__VA_ARGS__);						\
	_log("\033[0m");								\
	}while(0)

#else
#define _logfatal(fmt, ...) do{					\
	_log("\n\033[31;1m[FATAL ERROR] ->\033[0m \033[31m");	\
	_log(fmt, ##__VA_ARGS__);				\
	_log("\033[0m");						\
	}while(0)

#endif
#define _logsdl(fmt, ...) do{					\
	_log("\n\033[31;1m[SDL ERROR] ->\033[0m \033[31m");	\
	_log(fmt, ##__VA_ARGS__);				\
	_log(": %s",SDL_GetError());				\
	_log("\033[0m");						\
	}while(0)

#define _logsdl_exit(fmt, ...) do{					\
	_log("\n\033[31;1m[SDL ERROR] ->\033[0m \033[31m");	\
	_log(fmt, ##__VA_ARGS__);				\
	_log(": %s",SDL_GetError());				\
	_log("\033[0m");						\
	_logexit(EXIT_FAILURE);						\
	}while(0)

#define _logfatalerrno(fmt, ...) do{	\
	_logfatal(fmt, ##__VA_ARGS__);	\
	_logerrno();			\
	_log("\033[0m");			\
	}while(0)			\

#define _logfatal_exit(fmt, ...) do{	\
	_logfatal(fmt, ##__VA_ARGS__);	\
	_logexit(1);			\
	}while(0)

#define _logfatalerrno_exit(fmt, ...) do{	\
	_logfatalerrno(fmt, ##__VA_ARGS__);	\
	_logexit(1);				\
     	exit(1);				\
	}while(0)

#define _logwarning(fmt, ...)do {		\
	_log(SET_ORANGE);			\
	_log("\n[WARNING!]%s\n",SET_CLEAR);	\
	_log(SET_BOLD);				\
	_log(fmt, ##__VA_ARGS__);		\
	} while(0)




#define SET_CLEAR "\033[0m"

#define SET_WHITE "\033[37m"
#define SET_RED "\033[1;31m"
#define SET_GREEN "\033[1;32m"
#define SET_BLUE "\033[0;34m"
#define SET_PURPLE "\033[0;35m"
#define SET_ORANGE "\033[48:2:255:165:1m"

#define SET_BGWHITE 	"\033[51m"
#define SET_BGRED 	"\033[41m"
#define SET_BGGREEN 	"\033[42m"
#define SET_BGBLUE 	"\033[44m"
#define SET_BGPURPLE 	"\033[45m"

#define SET_REV "\033[7m"

#define SET_BOLD "\033[1m"
#define SET_UNDERLINE "\033[4m"
#define SET_UNDERBOLD "\033[1;4m"


#define SET_NOREV "\033[27m"
#define SET_NOUNDERLINE "\033[24m"
#define SET_NOBOLD "\033[22m"

#define SET(s) "\033["s"m"

#define _logln(fmt, ...) \
fprintf(stderr, SET_BOLD"[%s:%d] %s %s " fmt, __FILE__, __LINE__, SET_CLEAR, ##__VA_ARGS__)

#define _assert(truth) if(!(truth)) {__ASSERTION_FAILED(truth);}
#define __ASSERTION_FAILED(TRUTH) 	\
fprintf(stderr, SET_RED SET_NOBOLD\
	"\nASSERTION FAILED IN -> "	\
	SET_CLEAR SET_UNDERLINE "%s() @%s:%d\n"	\
	SET_CLEAR SET_BOLD "--> (%s) == " SET_RED SET_NOBOLD " FALSE\n\n",						\
	__func__, __FILE__, __LINE__, #TRUTH);				\
	_logexit(EXIT_FAILURE)

#define _logfatal_ln_exit(fmt, ...) \
fprintf(stderr, "%s%s%s[%s:%d] IN --> %s(): %s " fmt,		\
		(SET_BOLD), (SET_RED), (SET_NOBOLD),		\
		__FILE__, __LINE__, __func__, (SET_CLEAR)	\
		, ##__VA_ARGS__);				\
_logexit(EXIT_FAILURE)


#define _logfn(fmt, ...) \
fprintf(stderr,"[%s:%d:%s()]: " fmt, __FILE__, __LINE__, __func__,##__VA_ARGS__)

#endif // _log_H
/* PROPOSED _logGING LEVELS:

	FATAL;
	-> PREFIX: (bold, red) "[FATAL/UNRECOVERABLE ERROR] --> %s", body_text
	-> BODY TEXT: (red) "..."
		Something likely unrecoverable has happened to the program.
		If termination isnt instant, it should happen pretty soon, else there will be a crash.
		A function that reports a fatal error will almost certainly return a nullptr.
        <------------------------------------------------------------------------------------------>

	CRIT_WARNING;
	-> PREFIX: (bold, underlined(?), yellow) "[CRITICAL WARNING] --> %s", body_text
	-> BODY TEXT: (yellow) "..."
		A critical warning; i.e something which should stand out when reading _logs,
		but is not necessarily more pertinent/threatening than a warning.
        <------------------------------------------------------------------------------------------>

	WARNING;
	-> PREFIX: (yellow) "[WARNING] --> %s", body_text
	-> BODY TEXT: (yellow) "..."
		Something probably bad has happened, but we arent going to return null or do anything crazy.
		Just keep it in mind.
        <------------------------------------------------------------------------------------------>

	STANDARD;
	-> PREFIX: (n/a)
	-> BODY TEXT: (n/a) "..."
		The default _logging level, shows up no matter what.
        <------------------------------------------------------------------------------------------>

	DEBUG;
	-> PREFIX: (n/a)
	-> BODY TEXT: (blue!) "..."
		For information superfluous outside of specific debugging contexts, like when
		creating a new feature.
        <------------------------------------------------------------------------------------------>

*/
