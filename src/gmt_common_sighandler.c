/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * gmt_common_sighandler.c contains code for catching SIGINT
 *                     and for generating a stack backtrace
 *
 * Author:  Florian Wobbe
 * Date:    5-SEP-2013
 * Version: 5
 */

/*
 *
 * A) List of exported gmt_* functions available to modules and libraries via gmt_dev.h:
 * gmt_sig_handler_unix
 * gmt_sig_handler_win32
 */

#ifndef NO_SIGHANDLER

#include "gmt_dev.h"
#include "gmt_internals.h"

#ifdef WIN32
#include <windows.h>
/* win32: Install Windows SIGINT handling only */
BOOL gmt_sig_handler_win32 (DWORD dwType) {
    if (dwType == CTRL_C_EVENT)
		gmtlib_terminate_session ();	/* Delete session dir, call GMT_Destroy_Session, and for CLI call exit */
	return TRUE;
}
/* install WIN32 signal handler like this: 
 *  SetConsoleCtrlHandler((PHANDLER_ROUTINE)gmt_sig_handler_win32,TRUE));
 */
#else
/* unix: Install broader sighandler via backtrace handling of SIGINT, SIGILL, SIGFPE, SIGBUS and SIGSEGV */
#include "gmt_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ucontext.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <signal.h>

#ifdef HAVE_EXECINFO_H_
#include <execinfo.h>
#else
#include <stdbool.h>
GMT_LOCAL int backtrace(void **buffer, int size) {
	return 0;
}

GMT_LOCAL void backtrace_symbols_fd(void *const *buffer, int size, int fd) {
	static bool once = false;
	if (once) return;
	once = true;
	fprintf (stderr, "(stack backtrace unavailable due to missing execinfo.h)\n");
}
#endif	/* HAVE_EXECINFO_H_ */

/* Macro for retrieving instruction pointer */
#if defined(__APPLE__)
# include <mach/mach.h>
# if __DARWIN_UNIX03
#  ifdef __x86_64__
#   define UC_IP(uc) ((void *) (uc)->uc_mcontext->__ss.__rip)
#  elif __arm64__	/* Apple Silicon, e.g. M1 */
#   define UC_IP(uc) ((void *) (uc)->uc_mcontext->__ss.__pc)
#  else
#   define UC_IP(uc) ((void *) (uc)->uc_mcontext->__ss.__eip)
#  endif
# else
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext->ss.eip)
# endif
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
# ifdef __x86_64__
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.mc_rip)
# elif defined( __arm__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.arm_pc)
# elif defined( __aarch64__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.mc_gpregs.gp_elr)
# elif defined(__ppc__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.mc_srr0)
# else
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.mc_eip)
# endif
#elif defined(SIZEOF_GREG_T)
# ifdef __x86_64__
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.gregs[REG_RIP])
# elif defined(__aarch64__) || defined(__mips__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.pc)
# elif defined( __alpha__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.sc_pc)
# elif defined( __arm__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.arm_pc)
# elif defined( __hppa__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.sc_iaoq[0])
# elif defined(__m68k__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.gregs[R_PC])
# elif defined(__riscv)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.__gregs[REG_PC])
# elif defined(__sh__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.pc)
# elif defined(__s390__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.psw.addr)
# elif defined(__sparc__)
#  if defined (__arch64__)
#   define UC_IP(uc) ((void *) (uc)->uc_mcontext.mc_gregs[MC_PC])
#  else
#   define UC_IP(uc) ((void *) (uc)->uc_mcontext.gregs[REG_PC])
#  endif
# else
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.gregs[REG_EIP])
# endif
#else
# ifdef __x86_64__
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.rip)
# elif defined(__aarch64__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.pc)
# elif defined(__arm__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.arm_pc)
# elif defined(__powerpc__) || defined(__powerpc64__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.regs->nip)
# elif defined(__ia64__)
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.sc_ip)
# else
#  define UC_IP(uc) ((void *) (uc)->uc_mcontext.eip)
# endif
#endif

#ifdef _sys_siglist
#	define sys_siglist _sys_siglist
#elif defined(__sys_siglist)
#	define sys_siglist __sys_siglist
#endif

#if 0	/* Currently not used so commented out. PW 14.NOV.2021 */
static void process_cpu() {
	/* print current process accumulated cpu time */
	struct rusage ru;
	if (getrusage (RUSAGE_SELF, &ru) == -1 )
		return;
	fprintf (stderr, "Tuser: %.3lfs Tsys: %.3lfs ",
					 ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0,
					 ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1000000.0);
}

static void process_mem() {
	/* print current process memory usage */
	double rss, vsize;
#if defined(__APPLE__)
	struct task_basic_info t_info;
	mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

	if (KERN_SUCCESS != task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count))
		return;
	rss   = t_info.resident_size / 1024.0;
	vsize = t_info.virtual_size / 1024.0;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
	FILE* fp = NULL;
	if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
		return; /* Can't open */
	if ( fscanf( fp, "%lf %lf", &vsize, &rss ) != 2 ) {
		fclose( fp );
		return; /* Can't read */
	}
	fclose( fp );
	rss   *= sysconf( _SC_PAGESIZE) / 1024.0;
	vsize *= sysconf( _SC_PAGESIZE) / 1024.0;

#else
	fprintf (stderr, "\n");
	return; /* AIX, BSD, Solaris, Unknown */
#endif
	fprintf (stderr, "VmRSS: %.0lfkB VmSize: %.0lfkB\n", rss, vsize);
}

static void process_info() {
	process_cpu();
	process_mem();
}
#endif

void gmt_sig_handler_unix (int sig_num, siginfo_t *info, void *ucontext) {
	if (sig_num == SIGINT) {
		/* Catch Ctrl-c and remove modern session sub-directory (if it exists) before exit */
		struct sigaction act, oldact;
		sigemptyset (&act.sa_mask);
		act.sa_flags = 0;
		act.sa_handler = SIG_DFL; /* Restore default action (do not catch SIGINT again) */
		sigaction (SIGINT, &act, &oldact); /* Change signal handler */
		gmtlib_terminate_session ();	/* Delete session dir and call GMT_Destroy_Session */
	}
	else {	/* Report backtrace after a crash */
		int size;
		void *array[50];
		ucontext_t *uc = (ucontext_t *)ucontext;

		array [0] = UC_IP (uc); /* caller's address */
		array [1] = info->si_addr; /* address of faulting instruction */

#ifdef HAVE_STRSIGNAL
		fprintf (stderr, "ERROR: Caught signal number %d (%s) at\n", sig_num, strsignal(sig_num));
#else
		fprintf (stderr, "ERROR: Caught signal number %d (%s) at\n", sig_num, sys_siglist[sig_num]);
#endif
		backtrace_symbols_fd (array, 2, STDERR_FILENO); /* print function with faulting instruction */
		size = backtrace (array, 50); /* get void*'s for all entries on the stack */
		fprintf (stderr, "Stack backtrace:\n");
		backtrace_symbols_fd (array, size, STDERR_FILENO); /* print out all the frames to stderr */
		exit (EXIT_FAILURE);	/* Now exit process */
	}
}

/*
 * install signal handler like this:
 * int main (int argc, char **argv) {
 *   struct sigaction act;
 *   sigemptyset (&act.sa_mask);
 *   act.sa_flags = SA_NODEFER;
 *   act.sa_handler = gmt_sig_handler_unix;
 *   sigaction (SIGINT,  &act, NULL);
 *   act.sa_flags = 0;
 *   sigaction (SIGBUS,  &act, NULL);
 *   sigaction (SIGSEGV, &act, NULL);
 * }
 */
#endif	/* Unix */

#endif	/* !NO_SIGHANDLER */
