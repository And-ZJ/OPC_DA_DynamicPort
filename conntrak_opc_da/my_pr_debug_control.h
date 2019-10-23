#ifndef _MY_PR_DEBUG_CONTROL_H
#define _MY_PR_DEBUG_CONTROL_H

// Comment PR_DEBUG_ON for close the pr_debug output.
#define PR_DEBUG_ON

/**
* When defined PR_DBUG_ON,
*   In linux, pr_debug will take place by printk.
*   In windows, pr_debug will take place by printf.
* Otherwise,
*   In linux and windows, use the system definition or do nothing.
*/


#ifdef PR_DEBUG_ON

#ifdef pr_debug
#undef pr_debug // disable the existed pr_debug in system first, when you want to print
#endif // pr_debug

#ifdef __linux__

#include <linux/module.h>
#ifndef KERN_DEBUG
#define KERN_DEBUG ""
#endif // KERN_DEBUG

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif // pr_fmt
#define pr_debug(fmt, ...) \
		printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__) // In linux.

#else // other system, like windows

#include <stdio.h>
#define pr_debug(fmt, ...) \
		printf(fmt, ##__VA_ARGS__) // In windows.

#endif // __linux__

#else // when PR_DEBUG_ON not definition, if system don't have pr_debug, then define it do nothing.

#ifndef pr_debug
#define pr_debug(fmt, ...) \
		;  // do nothing
#endif // pr_debug

#endif // PR_DEBUG_ON

#endif // _MY_PR_DEBUG_CONTROL_H

