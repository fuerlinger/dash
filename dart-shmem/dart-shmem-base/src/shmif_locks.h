/*
 * shmif_locks.h
 *
 *  Created on: Apr 7, 2013
 *      Author: maierm
 */

#ifndef SHMIF_LOCKS_H_
#define SHMIF_LOCKS_H_
#ifdef __cplusplus
extern "C" {
#endif

int shmif_lock_create_at(void* addr);

int shmif_lock_destroy(void* addr);

int shmif_lock_acquire(void* addr, int is_blocking);

int shmif_lock_release(void* addr);

int shmif_lock_size_of();

#ifdef __cplusplus
}
#endif
#endif /* SHMIF_LOCKS_H_ */
