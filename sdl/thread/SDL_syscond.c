/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

/* An implementation of condition variables using semaphores and mutexes */
/*
   This implementation borrows heavily from the BeOS condition variable
   implementation, written by Christopher Tate and Owen Smith.  Thanks!
 */

#include "SDL_thread.h"
#include <rthreads/rthreads.h>
#include <rthreads/rsemaphore.h>

struct SDL_cond
{
    slock_t *lock;
    int waiting;
    int signals;
    ssem_t *wait_sem;
    ssem_t *wait_done;
};

/* Create a condition variable */
SDL_cond *LRSDL_CreateCond(void)
{
    SDL_cond *cond;

    cond = (SDL_cond *) SDL_malloc(sizeof(SDL_cond));
    if (cond)
    {
        cond->lock      = slock_new();
        cond->wait_sem  = ssem_new(0);
        cond->wait_done = ssem_new(0);
        cond->waiting   = 0;
        cond->signals   = 0;

        if (!cond->lock || !cond->wait_sem || !cond->wait_done) {
            LRSDL_DestroyCond(cond);
            cond = NULL;
        }
    }
    else
        LRSDL_OutOfMemory();
    return (cond);
}

/* Destroy a condition variable */
void LRSDL_DestroyCond(SDL_cond * cond)
{
    if (cond)
    {
        if (cond->wait_sem)
            ssem_free(cond->wait_sem);
        if (cond->wait_done)
            ssem_free(cond->wait_done);
        if (cond->lock)
            LRSDL_DestroyMutex(cond->lock);
        LRSDL_free(cond);
    }
}

/* Restart one of the threads that are waiting on the condition variable */
int LRSDL_CondSignal(SDL_cond * cond)
{
    if (!cond)
    {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }

    /* If there are waiting threads not already signalled, then
       signal the condition and wait for the thread to respond.
     */
    slock_lock(cond->lock);
    if (cond->waiting > cond->signals)
    {
        ++cond->signals;
        ssem_signal(cond->wait_sem);
        slock_unlock(cond->lock);
        ssem_wait(cond->wait_done);
    }
    else
        slock_unlock(cond->lock);

    return 0;
}

/* Restart all threads that are waiting on the condition variable */
int LRSDL_CondBroadcast(SDL_cond * cond)
{
    if (!cond)
    {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }

    /* If there are waiting threads not already signalled, then
       signal the condition and wait for the thread to respond.
     */
    slock_lock(cond->lock);
    if (cond->waiting > cond->signals)
    {
        int i, num_waiting;

        num_waiting = (cond->waiting - cond->signals);
        cond->signals = cond->waiting;
        for (i = 0; i < num_waiting; ++i)
            ssem_signal(cond->wait_sem);

        /* Now all released threads are blocked here, waiting for us.
           Collect them all (and win fabulous prizes!) :-)
           */
        slock_unlock(cond->lock);
        for (i = 0; i < num_waiting; ++i)
            ssem_wait(cond->wait_done);
    }
    else
        slock_unlock(cond->lock);

    return 0;
}

/* Wait on the condition variable for at most 'ms' milliseconds.
   The mutex must be locked before entering this function!
   The mutex is unlocked during the wait, and locked again after the wait.

Typical use:

Thread A:
    SDL_LockMutex(lock);
    while ( ! condition ) {
        SDL_CondWait(cond, lock);
    }
    SDL_UnlockMutex(lock);

Thread B:
    SDL_LockMutex(lock);
    ...
    condition = true;
    ...
    SDL_CondSignal(cond);
    SDL_UnlockMutex(lock);
 */
int LRSDL_CondWaitTimeout(SDL_cond * cond, SDL_mutex * mutex, Uint32 ms)
{
    int retval;

    if (!cond) {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }

    /* Obtain the protection mutex, and increment the number of waiters.
       This allows the signal mechanism to only perform a signal if there
       are waiting threads.
     */
    slock_lock(cond->lock);
    ++cond->waiting;
    slock_unlock(cond->lock);

    /* Unlock the mutex, as is required by condition variable semantics */
    slock_unlock(mutex);

    /* Wait for a signal */
    if (ms == SDL_MUTEX_MAXWAIT)
        retval = ssem_wait(cond->wait_sem);
    else
        retval = SDL_SemWaitTimeout(cond->wait_sem, ms);

    /* Let the signaler know we have completed the wait, otherwise
       the signaler can race ahead and get the condition semaphore
       if we are stopped between the mutex unlock and semaphore wait,
       giving a deadlock.  See the following URL for details:
       http://www-classic.be.com/aboutbe/benewsletter/volume_III/Issue40.html
     */
    slock_lock(cond->lock);
    if (cond->signals > 0) {
        /* If we timed out, we need to eat a condition signal */
        if (retval > 0) {
            ssem_wait(cond->wait_sem);
        }
        /* We always notify the signal thread that we are done */
        ssem_signal(cond->wait_done);

        /* Signal handshake complete */
        --cond->signals;
    }
    --cond->waiting;
    slock_unlock(cond->lock);

    /* Lock the mutex, as is required by condition variable semantics */
    slock_lock(mutex);

    return retval;
}

/* Wait on the condition variable forever */
int
SDL_CondWait(SDL_cond * cond, SDL_mutex * mutex)
{
    return scond_wait_timeout((scond_t*)cond, (slock_t*)mutex, SDL_MUTEX_MAXWAIT);
}

/* vi: set ts=4 sw=4 expandtab: */
