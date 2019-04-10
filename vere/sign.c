/* vere/sign.c
**
*/
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <uv.h>
#include <termios.h>
#include <term.h>
#include <errno.h>
#include <libgen.h>
#include <ftw.h>

#include "all.h"
#include "vere/vere.h"

/*

   Overview:

       This code handles all unix / posix signals.  See 'man signal(7)', 'man kill(2)' for more info.

       Signal handling has two components:
          * using either sigaction() or uv_signal_init() and uv_signal_start() to setup handling
          * the callbacks that are invoked when a signal is received

       There are two processes, the king and the serf.  The serf deals
       with pure Nock computations.  The king primarily deals with
       messy real world things like terminals, sockets, etc.

       The king is based around a libuv loop.  Because of this, the
       king code does not use sigaction(), but instead uses
       uv_signal_init() and uv_signal_start().

       The serf is not structured around a libuv loop, and thus uses
       sigaction().  That code is in ../noun/manage.c

       No code relating to signals should ever appeal in a file other
       than this one and that one.

       Note that the king also does some Nock computations (for the
       benefit of king processes; not ship events).  As such it has a
       copy of the ../noun/manage.c code linked in and those signal
       handlers ALSO exist in king.

   Details about signal handling in the king (i.e. in this file)

        signal      key      meaning                    handling
        ------      -----    -------------              --------------------
        SIGTERM              terminate process          libuv -> _signal_cb -> uv_process_kill() / u3_pier_exit()
        SIGTSTP     ^Z       user initiated stop        libuv -> _signal_cb -> uv_process_kill() / u3_pier_exit()
        SIGWINCH             window resize              libuv -> _signal_cb -> u3_term_ef_winc()
        SIGINT      ^C       Interrupt from keyboard    libuv -> _signal_cb -> u3_term_ef_ctlc()
        SIGPROF              profiling                  block
       
*/

void _signal_cb(uv_signal_t* sil_u, c3_i num_i);


//  Block profiling signal, which should be delivered to exactly one thread.
//
void u3_signal_init_mainthread()
{
  if ( _(u3_Host.ops_u.pro) ) {
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGPROF);
    if ( 0 != pthread_sigmask(SIG_BLOCK, &set, NULL) ) {
      perror("pthread_sigmask");
      exit(1);
    }
  }

}

// Setup the complete list of signal handlers.  By keeping them in a
// datastructure, we can later turn them on or off as needed.
void
u3_signal_setup()
{
  {
    u3_usig* sig_u;

    sig_u = c3_malloc(sizeof(u3_usig));
    uv_signal_init(u3L, &sig_u->sil_u);

    sig_u->num_i = SIGTSTP;
    sig_u->nex_u = u3_Host.sig_u;
    u3_Host.sig_u = sig_u;
  }
  {
    u3_usig* sig_u;

    sig_u = c3_malloc(sizeof(u3_usig));
    uv_signal_init(u3L, &sig_u->sil_u);

    sig_u->num_i = SIGTERM;
    sig_u->nex_u = u3_Host.sig_u;
    u3_Host.sig_u = sig_u;
  }
  {
    u3_usig* sig_u;

    sig_u = c3_malloc(sizeof(u3_usig));
    uv_signal_init(u3L, &sig_u->sil_u);

    sig_u->num_i = SIGINT;
    sig_u->nex_u = u3_Host.sig_u;
    u3_Host.sig_u = sig_u;
  }
  {
    u3_usig* sig_u;

    sig_u = c3_malloc(sizeof(u3_usig));
    uv_signal_init(u3L, &sig_u->sil_u);

    sig_u->num_i = SIGWINCH;
    sig_u->nex_u = u3_Host.sig_u;
    u3_Host.sig_u = sig_u;
  }

}


// turn all the signal handling off
void
u3_signal_hold(void)
{
  u3_usig* sig_u;

  for ( sig_u = u3_Host.sig_u; sig_u; sig_u = sig_u->nex_u ) {
    uv_signal_stop(&sig_u->sil_u);
  }
}


// turn all the signal handling on
void
u3_signal_activate(void)
{
  u3_usig* sig_u;

  for ( sig_u = u3_Host.sig_u; sig_u; sig_u = sig_u->nex_u ) {
    uv_signal_start(&sig_u->sil_u, _signal_cb, sig_u->num_i);
  }
}



void
_signal_cb(uv_signal_t* sil_u, c3_i num_i)
{

  switch ( num_i ) {

  case SIGTSTP: 
  case SIGTERM:
    fprintf(stderr, "\r\ncaught signal %d\r\n", num_i);
    u3_Host.liv = c3n;

    // kill serf
    //    u3_pier* pir_u =   * u3K.tab_u;
    //    uv_process_kill(&pir_u->god_u->cub_u, SIGINT);

    // kill self (king), gently (can't believe Curtis didn't name this abdicate() !)
    u3_pier_exit();
    
    break;
  case SIGINT:
    fprintf(stderr, "\r\ninterrupt\r\n");
    u3_term_ef_ctlc();
    break;
  case SIGWINCH:
    u3_term_ef_winc();
    break;
  default:
    fprintf(stderr, "\r\nmysterious signal %d\r\n", num_i);
    break;


  }
}
