/* 07-prod-cons.c

   Producer-consumer co-routining.  There are five producers and four 
   consumers.  Two successors put consecutive numbers into their respective 
   streams.  Two times consumers multiply all consumed numbers by 5 and 7
   respectively and put the results into their streams.  A merge consumer
   merges the two stream created by the times producers.  A consumer prints 
   tokens from the merge stream.  This  illustrates that producer-consumer 
   relationships can be formed into complex networks.

   Each stream has a buffer of size 5 - producers can put up to 5 numbers
   in their stream before waiting.

                              7,14,21,28...                1,2,3,4...
                                     /--- Times 7 <---- successor
                      5,7,10,14...  /
          consumer <--- merge <----<
                                    \
                                     \--- Times 5 <---- successor
                              5,10,15,20...              1,2,3,4...
*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "queue_a.h"

#define Q &((Stream*)stream)->buffer
#define Lock &((Stream*)stream)->lock
#define FullNotifier &((Stream*)stream)->full
#define EmptyNotifier &((Stream*)stream)->empty


#define BUFFER_SIZE 3
int idcnt = 1;
pthread_t s1, t1, t2, a1;

typedef struct connectentry {
   struct connectentry *next;
   pthread_t *thread;
   int first;
} ConnectionEntry;

/* One of these per stream.
   Holds:  the mutex lock and notifier condition variables
           a buffer of tokens taken from a producer
           a structure with information regarding the processing of tokens 
           an identity
  Note: some taken from lab document
*/
typedef struct stream_struct {
   struct stream_struct *next;
   ConnectionEntry *ce;		  /* linked list of connections */
   pthread_t *thread;         /* needed to cancel thread in connect */
   int first_pos; 			  /* token number of first element in buffer */
   int last_pos;             /* token number of last element in buffer */
   pthread_mutex_t lock;
   pthread_cond_t full;      /* producer will wait if buffer is full */
   pthread_cond_t empty;     /* consumer will wait for its token or until 
	   	                        buffer is empty */
   queue buffer;               /* a buffer of values of any type      */
   void *args;                 /* arguments for the producing stream */
   int id;                     /* identity of this stream            */
} Stream;


bool peek(queue *q, int pos) {
	
}

/* prod: linked list of streams that this producer consumes tokens from
   self: the producer's output stream */
typedef struct {
   Stream *self, *prod;
} Args;

/* return 'value' which should have a value from a call to put */
void *get(void *stream, pthread_t *p) {
	void *ret;  /* needed to take/save a value from the critical section */
	int first = -1; // first position in the buffer calling consumer needs
	ConnectionEntry *ce; // pointer for walking CE list
	// step 1
	if(isEmpty(Q) && p!= NULL) {
		return -2;
	}

	// step 2
   pthread_mutex_lock(Lock);      /* lock the section   */
   
   // step 3 have any connections been made
   if(stream->ce == NULL) {
	   return -3;
   }
   
   // step 4 look for calling thread in connect entry list
   ce = stream->ce;
   while(ce->next != NULL) {
	   if(ce->thread->id == p->id) {
		   // found consumer thread
		   first = ce->first;
		   break;
	   }
	   ce = ce->next;
   }
   
   if(first == -1) {
	   return -4;
   }
   
   /* step 5 Wait here (a mutex cond variable) as long as the buffer is empty or the request
    is for a token that has not yet been added to the buffer (this is where last_pos comes in). 
   Do not blindly cut-and-paste. Do not assume that "empty" means use the "empty" cond variable. 
   Think about it. Also, consider what happens if the buffer is still empty when the thread is 
   resumed at this point. This is probably the only tricky line in the whole program and illustrates
    an important principle in locking - which we will consider later in the course. */
	while (isEmpty(q)|| first > last_pos)
	pthread_cond_wait(Empty, mutex);
	ret = peek(Q, ce-> first
   
   
   
   ret = peek(q, ce->first - *first_pos);
   
   while (isEmpty(Q))             /* if no tokens are in the queue, wait     */
      pthread_cond_wait(EmptyNotifier, Lock);   /* and let other threads in       */
   ret = (void*)dequeue(Q);       /* take the next token from the queue      */
   pthread_cond_signal(EmptyNotifier); /* let the producer continue with 'put'    */
   pthread_mutex_unlock(Lock);    /* unlock the section */
   return ret;
}

/* 'value' is the value to move to the consumer */
void put(void *stream, void *value) {
   pthread_mutex_lock(Lock);       /* lock the section */
   while (nelem(Q) >= BUFFER_SIZE) /* if the queue is full, wait            */
      pthread_cond_wait(FullNotifier, Lock);   /* and let other threads in      */
   enqueue(Q, (long)value);        /* add the 'value' token to the queue    */
   pthread_cond_signal(EmptyNotifier);  /* let the consumer continue with 'get'  */
   pthread_mutex_unlock(Lock);     /* unlock the section */
   return;
}

/* Put 1,2,3,4,5... into the self stream */
void *producer (void *streams) {
   Stream *self = ((Args*)streams)->self;
   int id = ((Args*)streams)->self->id;
   int i;
   long *value;
   
   for (i=1 ; ; i++) {
      /* sleep(1); */
      printf("Producer(%d): sending %d\n", id, i);
      value = (long*)malloc(sizeof(long));
      *value = i;
      put(self, (void*)value);
      printf("Successor(%d): sent %d, buf_sz=%d\n",
             id, i, nelem(&self->buffer));
   }
   pthread_exit(NULL);
}

/* multiply all tokens from the self stream by (int)self->args and insert
   the resulting tokens into the self stream */
void *times (void *streams) {
   Stream *self = ((Args*)streams)->self;
   Stream *prod = ((Args*)streams)->prod;
   long *in;
   
   printf("Times(%d) connected to Successor (%d)\n", self->id, prod->id);
   while (true) {
      in = (long*)get(prod);

      printf("\t\tTimes(%d): got %ld from Successor %d\n",
             self->id, *(long*)in, prod->id);

      *in *= (long)(self->args);
      put(self, (void*)in);

      printf("\t\tTimes(%d): sent %ld buf_sz=%d\n",
             self->id, *in, nelem(&self->buffer));
   }
   pthread_exit(NULL);
}

/* merge two streams that containing tokens in increasing order 
   ex: stream 1:  3,6,9,12,15,18...  stream 2: 5,10,15,20,25,30...
       output stream: 3,5,6,9,10,12,15,15,18...
*/
void *merge (void *streams) {
   Stream *self = ((Args*)streams)->self;
   Stream *s1 = ((Args*)streams)->prod;
   Stream *s2 = (((Args*)streams)->prod)->next;
   void *a = get(s1);
   void *b = get(s2);

   while (true) {
      if (*(long*)a < *(long*)b) {
         put(self, a);
         a = get(s1);
         printf("\t\t\t\t\tMerge(%d): sent %ld from Times %d buf_sz=%d\n", 
                self->id, *(long*)a, s1->id, nelem(&self->buffer));
      } else {
         put(self, b);
         b = get(s2);
         printf("\t\t\t\t\tMerge(%d): sent %ld from Times %d buf_sz=%d\n", 
                self->id, *(long*)b, s2->id, nelem(&self->buffer));
      }
   }
   pthread_exit(NULL);
}

/* Final consumer in the network */
void *consumer (void *streams) {
   Stream *prod = ((Args*)streams)->prod;
   int i;
   void *value;
   
   for (i=0 ; i < 10 ; i++) {
      value = get(prod); 
      printf("\t\t\t\t\t\t\tConsumer: got %ld\n", *(long*)value);
      free(value);
   }

   exit(0);
}

/* initialize streams - see also queue_a.h and queue_a.c */
void init_stream (Args *args, pthread_t *thread, Stream *self, void *data) {
   if (self != NULL) {
      self->next = NULL;
      self->args = data;
      self->id = idcnt++;
	  self->thread = thread;
      init_queue(&self->buffer);
      pthread_mutex_init(&self->lock, NULL);
      pthread_cond_init (&self->full, NULL);
	  pthread_cond_init (&self->empty, NULL);
	  self->ce = NULL;
	  self->first_pos = 0;
	  self->last_pos = -1;
   }
   args->self = self;
   args->prod = NULL;
}

/* free allocated space in the queue - see queue_a.h and queue_a.c */
void kill_stream(Stream *stream) {
	destroy_queue(&stream->buffer); 
	pthread_cancel(*stream->thread);
}

/* puts an initialized stream object onto the end of a stream's input list */
void connect (Args *arg, Stream *s) {  
   s->next = arg->prod;
   arg->prod = s;
   ConnectionEntry *ptr = s->ce;
   s->ce = (ConnectionEntry*)malloc(sizeof(ConnectionEntry));
   s->ce->next = ptr;				
   s->ce->first = 0;
   s->ce->thread = arg->self->thread;
}


int main () {
    Stream suc1, con1, con2, ask1;
     Args suc1_args, con1_args, con2_args, ask1_args;
     pthread_attr_t attr;

     init_stream(&suc1_args, &s1, &suc1, NULL);   /* initialize producer stream */

     init_stream(&con1_args, &t1, &con1, (void*)7);/* initialize times 7 stream */
     connect(&con1_args, &suc1);                   /* connect to producer */

     init_stream(&con2_args, &t2, &con2, (void*)5);/* initialize times 5 stream */
     connect(&con2_args, &suc1);                   /* connect to producer */
	 
	 printf("Connected streams");

     //init_stream(&ask1_args, &a1, &ask1, NULL);    /* initialize asker stream */
     //connect(&ask1_args, &con1);                   /* connect to times 7 stream */
     //connect(&ask1_args, &con2);                   /* connect to times 5 stream */

     pthread_attr_init(&attr);
     pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
     pthread_create(&s1, &attr, producer, (void*)&suc1_args);
     pthread_create(&t1, &attr, consumer, (void*)&con1_args);
     pthread_create(&t2, &attr, consumer, (void*)&con2_args);
     //pthread_create(&a1, &attr, asker,    (void*)&ask1_args);

     pthread_join(a1, NULL);
     kill_stream(&suc1);
     kill_stream(&con1);
     kill_stream(&con2);
     //kill_stream(&ask1);
	  printf("Done\n");
}
   