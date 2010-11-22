/* Copyright (c) 2010, RWTH Aachen University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or 
 * without modification, are permitted provided that the following
 * conditions are met:
 *   * Redistributions of source code must retain the above 
 *     copyright notice, this list of conditions and the following
 *     disclaimer.
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *   * Neither the name of the RWTH Aachen University nor the
 *     names of its contributors may be used to endorse or promote 
 *     products derived from this software without specific prior 
 *     written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL RWTH 
 * AACHEN UNIVERSITY BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
 *
 * Coded by Matthias Petschow (petschow@aices.rwth-aachen.de),
 * September 2010, Version 1.0
 *
 * This code was the result of a collaboration between 
 * Matthias Petschow and Paolo Bientinesi. When you use this 
 * code, kindly reference the paper:
 *
 * "MR3-SMP: A Symmetric Tridiagonal Eigensolver for Multi-Core 
 * Architectures" by Matthias Petschow and Paolo Bientinesi, 
 * RWTH Aachen, Technical Report AICES-2010/10-2 (submitted to 
 * Parallel Computing).
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "global.h"
#include "queue.h"



queue_t *PMR_create_empty_queue(void)
{
  int     info;
  queue_t *queue;
  
  queue = (queue_t *) malloc(sizeof(queue_t));
  assert(queue != NULL);
  
  queue->num_tasks = 0;
  queue->head      = NULL;
  queue->back      = NULL;
  
  info = pthread_spin_init(&queue->lock, PTHREAD_PROCESS_PRIVATE);
  assert(info == 0);

  return(queue);
}



void PMR_destroy_queue(queue_t *queue)
{
  pthread_spin_destroy(&queue->lock);
  free(queue);
}



int PMR_get_num_tasks(queue_t *queue)
{
  int info, num_tasks;

  info = pthread_spin_lock(&queue->lock);
  assert(info == 0);

  num_tasks = queue->num_tasks;

  info = pthread_spin_unlock(&queue->lock);
  assert(info == 0);

  return(num_tasks);
}



int PMR_insert_task_at_front(queue_t *queue, task_t *task)
{
  int info;
  
  info = pthread_spin_lock(&queue->lock);
  assert(info == 0);

  queue->num_tasks++;

  task->next = queue->head;
  if (queue->head == NULL)
    queue->back = task;
  else
    queue->head->prev = task;
  queue->head = task;

  info = pthread_spin_unlock(&queue->lock);
  assert( info == 0);

  return(info);
}



int PMR_insert_task_at_back(queue_t *queue, task_t *task)
{
  int info;

  info = pthread_spin_lock(&queue->lock);
  assert(info == 0);

  queue->num_tasks++;

  task->prev = queue->back;
  task->next = NULL;
  if (queue->head == NULL)
    queue->head = task;
  else
    queue->back->next = task;
  queue->back = task;

  info = pthread_spin_unlock(&queue->lock);
  assert(info == 0);
  
  return(info);
}



task_t *PMR_remove_task_at_front(queue_t *queue)
{
  int    info;
  task_t *task;
  
  info = pthread_spin_lock(&queue->lock);
  assert(info == 0);
  
  task = queue->head;

  if (queue->head != NULL) {
    /* at least one element */
    queue->num_tasks--;
    if (queue->head->next == NULL) {
      /* last task removed */
      queue->head = NULL;
      queue->back = NULL;
    } else {
      /* at least two tasks */
      queue->head->next->prev = NULL;
      queue->head = queue->head->next;
    }
  }
  
  info = pthread_spin_unlock(&queue->lock);
  assert(info == 0);
  
  return(task);
  /* returns NULL when empty */
}



task_t *PMR_remove_task_at_back (queue_t *queue)
{
  int    info;
  task_t *task;

  info = pthread_spin_lock(&queue->lock);
  assert(info == 0);
  
  task = queue->back;

  if (queue->back != NULL) {
    /* at least one element */
    queue->num_tasks--;
    if (queue->back->prev == NULL) {
      /* last task removed */
      queue->head = NULL;
      queue->back = NULL;
    } else {
      /* at least two tasks */
      queue->back->prev->next = NULL;
      queue->back = queue->back->prev;
    }
  }
  
  info = pthread_spin_unlock(&queue->lock);
  assert(info == 0);

  return(task);
  /* returns NULL when empty */
}
