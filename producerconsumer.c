#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NPRODUCER 5
#define NCONSUMER 2
#define NBUFSIZE 2             // Length of the buffer
static int BUF[NBUFSIZE];
// static int in = 0;              // 用来跟踪生产者生产的情况，初值为0，指向下一个可用的缓冲区
// static int out = -1;            // 用来跟踪消费者的消费情况，初值为-1，因为没有产品可用
/* 将in和out设置为全局变量的原因是在生产者和消费者当中，都需要访问in和out来看缓冲区当时的情况，具体如下：
1、在生产者当中，如果in == out，则说明，消费者没有来得及取走产品，此时生产者需要等消费者将产品取走，然后
才能生产；生产者需要阻塞
2、在消费者当中，如果out == in，则说明out指向的是一个空的缓冲单元（in指向一个空的缓冲单元），说明消费的
速度太快，生产者没有来得及放入产品，此时消费者需要阻塞 */
static int empty_buf = NBUFSIZE;  // 初始化最初的可用缓冲单元个数
static int full_buf = 0;

pthread_mutex_t mutexi;         // Protect access of in between producers
pthread_mutex_t mutexo;         // Protect access of out between consumers

pthread_cond_t empty;           // if the producer can work; consumer free it
pthread_cond_t full;            // if the consumer can work; producer free it

void* producer(void* tid)
{

  static int in = 0;            // 在生产者之间共享的量，指向下一个空闲的缓冲单元
  static int Product = 0;       // 生产产品的编号
  int id = * (int *) tid;

  while (1) {
    pthread_mutex_lock(&mutexi);  // 需要对in指针操作，in指针在生产者之间共享，所以需要互斥
    sleep(random() % 3);
    while (empty_buf <= 0)            // 可用缓冲的个数不足
      pthread_cond_wait(&empty, &mutexi);  // 如果从此处返回，则说明empty_buf > 0，可以生产

    BUF[in] = Product;
    printf("Procduer: %d ---> product: %d IN = %d \n", id, Product, in);
    Product++;
    empty_buf--;                  // 可用缓冲个数减一个
    if (++in % NBUFSIZE == 0)
      in = in % NBUFSIZE;
    full_buf++;                   // 可以消费的缓冲个数增加了
    pthread_cond_signal(&full);   // 通知消费者，其可以执行了
    pthread_mutex_unlock(&mutexi);
  }
}

void* consumer(void* tid)
{
  static int out = 0;          // 在消费者之间共享的量，指向下一个可供使用的缓冲单元
  int id = * (int *) tid;

  while (1) {
    pthread_mutex_lock(&mutexo);
    sleep(random() % 4);
    while (full_buf <= 0)
      pthread_cond_wait(&full, &mutexo);

    int result = BUF[out];
    printf("Consumer: %d ---> consumer: %d OUT = %d\n", id, result, out);
    full_buf--;
    if (++out % NBUFSIZE == 0)
      out = out % NBUFSIZE;
    empty_buf++;
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutexo);
  }
}

int main()
{
  pthread_t producers[NPRODUCER];
  pthread_t consumers[NCONSUMER];

  mutexi = PTHREAD_MUTEX_INITIALIZER;
  mutexo = PTHREAD_MUTEX_INITIALIZER;
  empty = PTHREAD_COND_INITIALIZER;
  full = PTHREAD_COND_INITIALIZER;

  for (int i = 0; i < NPRODUCER; i++) {
    pthread_create(&producers[i], NULL, &producer, &i);
  }
  
  for (int i = 0; i < NCONSUMER; i++) {
    pthread_create(&consumers[i], NULL, &consumer, &i);
  }

  sleep(100);
}
