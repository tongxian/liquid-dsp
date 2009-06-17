/*
 * Copyright (c) 2007, 2009 Joseph Gaeddert
 * Copyright (c) 2007, 2009 Virginia Polytechnic Institute & State University
 *
 * This file is part of liquid.
 *
 * liquid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * liquid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with liquid.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LIQUID_GPORT2_THREADED_BENCHMARK_H__
#define __LIQUID_GPORT2_THREADED_BENCHMARK_H__

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/resource.h>

#include "liquid.h"

// prototype for thread routines
void gport2_producer_handler ( void *ptr );
void gport2_consumer_handler ( void *ptr );

typedef struct {
    gport2 p;
    unsigned int producer_size;
    unsigned int consumer_size;
    unsigned long int num_trials;
} gport2_threaded_bench_data_t;

#define GPORT2_THREADED_BENCH_API(N)    \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ gport2_threaded_bench(_start, _finish, _num_iterations, N); }

// Helper function to keep code base small
void gport2_threaded_bench(
    struct rusage *_start,
    struct rusage *_finish,
    unsigned long int *_num_iterations,
    unsigned int _n)
{
    // adjust number of iterations
    *_num_iterations = (*_num_iterations *_n)/10;

    // initialize port
    gport2_threaded_bench_data_t data;
    data.p = gport2_create(8*_n,sizeof(int));
    data.producer_size = _n;
    data.consumer_size = _n;
    data.num_trials = *_num_iterations;

    // threads
    pthread_t producer_thread;
    pthread_t consumer_thread;
    pthread_attr_t thread_attr;
    void * status;

    // set thread attributes
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_JOINABLE);

    // create threads
    pthread_create(&producer_thread, &thread_attr, (void*) &gport2_producer_handler, (void*) &data);
    pthread_create(&consumer_thread, &thread_attr, (void*) &gport2_consumer_handler, (void*) &data);

    // destroy attributes object (no longer needed)
    pthread_attr_destroy(&thread_attr);

    // start trials:
    getrusage(RUSAGE_SELF, _start);

    // join threads
    pthread_join(producer_thread, &status);
    pthread_join(consumer_thread, &status);

    getrusage(RUSAGE_SELF, _finish);
    //*_num_iterations *= _n;

    // clean up memory
    gport2_destroy(data.p);
}

void gport2_producer_handler(void * _data)
{
    gport2_threaded_bench_data_t * data = (gport2_threaded_bench_data_t*)_data;
    unsigned long int i;
    int w[data->producer_size];
    for (i=0; i<data->num_trials; i+=data->producer_size) {
        gport2_produce(data->p,(void*)w,data->producer_size);
    }
    pthread_exit(0);
}


void gport2_consumer_handler(void * _data)
{
    gport2_threaded_bench_data_t * data = (gport2_threaded_bench_data_t*)_data;
    unsigned long int i;
    int r[data->consumer_size];
    for (i=0; i<data->num_trials; i+=data->consumer_size) {
        gport2_consume(data->p,(void*)r,data->consumer_size);
    }
    pthread_exit(0);
}

// 
void benchmark_gport2_threaded_n1   GPORT2_THREADED_BENCH_API(1)
void benchmark_gport2_threaded_n4   GPORT2_THREADED_BENCH_API(4)
void benchmark_gport2_threaded_n16  GPORT2_THREADED_BENCH_API(16)
void benchmark_gport2_threaded_n64  GPORT2_THREADED_BENCH_API(64)

#endif // __LIQUID_GPORT2_THREADED_BENCHMARK_H__

