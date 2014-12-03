#include "queue_a.h"

#define BUFFER_SIZE 3
int idcnt = 1;
pthread_t s1, t1, t2, a1;

typedef struct connectentry {
   struct connectentry *next;
   pthread_t *thread;
   int first;
} ConnectEntry;

/* One of these per stream.
   Holds:  the mutex lock, and full and empty semaphores;
           a buffer of tokens taken from a producer
           a structure with information regarding the processing of tokens 
           an identity
   Note: tokens of a stream are implicitly numbered - the first token 
         has number 1, the second number 2 and so on
*/
typedef struct stream_struct {
   struct stream_struct *next;
   ConnectEntry *ce;       /* the database (linked list) of the connection
                              details of all connected consumers */
   pthread_t *thread;      /* needed to cancel the thread and in 'connect' */
   int last_pos;           /* token number of last buffer element */
   int first_pos;          /* token number of first buffer element */
   pthread_mutex_t mutex;  /* A lock on the critical sections of code */
   pthread_cond_t  full;   /* A producer will wait if its buffer is full */
   pthread_cond_t  empty;  /* A consumer will wait if the buffer it is getting
                              tokens from is empty or doesn't yet have its 
                              requested token */
   queue buffer;   /* a buffer of values of any type      */
   void *args;     /* arguments for the producing stream  */
   int id;         /* identity of this stream             */
} Stream;

/* prod: linked list of streams that this producer consumes tokens from
   self: the producer's output stream */
typedef struct {
   Stream *self, *prod;
} Args;

/* Return 'ret' which should have a value from a call to put 
   If p is the asker and the queue is empty, just return to the asker 
   The asker is the function that translates user console input to
   requests for tokens from the consumers connected to the producer */
int get (void *stream, pthread_t *p) {
   ConnectEntry *ce =((stream...
   bool flag;
   int ret;
   ...
   if (p != NULL && isEmpty(q)) return -2;

   pthread_mutex_lock(mutex);
   ...
   ret = peek(q, ce->first - *first_pos);
   ...
   pthread_mutex_unlock(mutex);
   return ret;
}
/* 'value' is the value to move to the consumer */
void put(void *stream,  int value) {
   ...
   pthread_mutex_lock(mutex);
   if (nelem(q) >= BUFFER_SIZE) pthread_cond_wait(full, mutex);
   enqueue(q, value);
   ...
   pthread_cond_broadcast(empty);
   pthread_mutex_unlock(mutex);
   return;
}
/* Put 1,2,3,4,5... into the self stream */
void *producer (void *streams) {
   Stream *self = ((Args*)streams)->self;
   int i;
   
   for (i=1 ; ; i++) {
      put(self, i);
     // ... some print statement here …
   }
   pthread_exit(NULL);
}
/* A consumer that gets connected to the producer's output stream 
   The consumer multiplies the input token by self->args which is
   set during initialization (see below) */
void *consumer (void *streams) {
   ...
   int in;
   
   ... some print statement here …

   while (true) {
      in = get(prod, NULL);
      if (in != -1) in = (in)*(int)(self->args); else in = -1;
      put(self, in);
      print(&self->buffer, "cons", self->id);
   }
   pthread_exit(NULL);
}


/* This is the function that translates user console input to requests
   for tokens from the consumers.  It assumes there are two consumers.
   If the user inputs 1, the next token is fetched from consumer 1, if 
   the user inputs 2, the next token is fetched from consumer 2, a 0 
   input quits, any other input does nothing.  */
void *asker (void *streams) {
   Stream *s1 = ((Args*)streams)->prod;
   Stream *s2 = (((Args*)streams)->prod)->next;
   char line[10000];
   int val;

   while (true) {
      sleep(1);
      printf("[1 or 2 or 0 to quit] >>> "); fflush(stdout);
      fgets(line, 9000, stdin);
      if (line[0] == '1') {
val = get(s1, &a1);
if (val >= 0) 
    printf("--------> Asker: got %d from Consumer(%d)\n", val, s1->id);
else if (val == -1) 
    printf("          Buffer full and request not present\n");
else 
    printf("          Buffer empty\n");
      } else if (line[0] == '2') {
val = get(s2, &a1);
if (val >= 0)
    printf("--------> Asker: got %d from Consumer(%d)\n", val, s2->id);
else if (val == -1)
    printf("          Buffer full and request not present\n");
else 
    printf("          Buffer empty\n");
      } else if (line[0] == '0') {
printf("--------> Asker: have a good day!\n");
break;
      } else
printf("--------> Asker: not valid\n");
   }
   pthread_exit(NULL);
}

/* initialize streams - see also queue_a.h and queue_a.c */
void init_stream (Args *args, pthread_t *thread, Stream *self, void *data) {
   if (self != NULL) {
      self->thread = thread;
      self->ce = NULL;
      self->next = NULL;
      self->args = data;
      self->id = idcnt++;
      self->last_pos = -1;
      self->first_pos = 0;
      init_queue(&self->buffer);
      pthread_mutex_init(&self->mutex, NULL);
      pthread_cond_init(&self->full, NULL);
      pthread_cond_init(&self->empty, NULL);
   }
   args->self = self;
   args->prod = NULL;
}

/* free allocated space in the queue - see queue_a.h and queue_a.c */
void kill_stream(Stream *stream) { 
   destroy_queue(&stream->buffer);    /* provided by queue_a.c */
   pthread_cancel(*stream->thread);
}

/* puts an initialized stream object onto the end of a stream's input list */
void connect (Args *arg, Stream *s) {  
   s->next = arg->prod;
   arg->prod = s;
   ConnectEntry *ptr = s->ce;
   s->ce = (ConnectEntry*)malloc(sizeof(ConnectEntry));
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

   init_stream(&ask1_args, &a1, &ask1, NULL);    /* initialize asker stream */
   connect(&ask1_args, &con1);                   /* connect to times 7 stream */
   connect(&ask1_args, &con2);                   /* connect to times 5 stream */

   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   pthread_create(&s1, &attr, producer, (void*)&suc1_args);
   pthread_create(&t1, &attr, consumer, (void*)&con1_args);
   pthread_create(&t2, &attr, consumer, (void*)&con2_args);
   pthread_create(&a1, &attr, asker,    (void*)&ask1_args);

   pthread_join(a1, NULL);
   kill_stream(&suc1);
   kill_stream(&con1);
   kill_stream(&con2);
   kill_stream(&ask1);
}

//Only changes need to made to the get and the put..


// code added gett
while (isEmpty(q)|| ce->first > *last_pos)
pthread_cond_wait(Empty, mutex);
ret = peek(Q, ce-> first


// code added to put 
ce->first++;
pthead_mutex_unlock(Mutex);
return ret;​