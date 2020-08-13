#include "fifo.h"
#include "mm.h"

#define FIFO_SIZE  256  // warning: DO NOT exceed page size

struct fifo {
  unsigned int head;
  unsigned int tail;
  unsigned int used;
  unsigned long buf[FIFO_SIZE];
};

#define NEXT_INDEX(i) (((i)+1) == FIFO_SIZE ? 0 : ((i)+1))

int is_empty_fifo(struct fifo *fifo) {
  return fifo->used == 0;
}

int is_full_fifo(struct fifo *fifo) {
  return fifo->used == FIFO_SIZE;
}

struct fifo *create_fifo() {
  struct fifo *fifo = (struct fifo *)allocate_page();
  fifo->head = 0;
  fifo->tail = 0;
  fifo->used = 0;

  return fifo;
}

void enqueue_fifo(struct fifo *fifo, unsigned long val) {
  fifo->buf[fifo->head] = val;
  if (is_full_fifo(fifo))
    fifo->tail = NEXT_INDEX(fifo->tail);
  else
    fifo->used++;

  fifo->head = NEXT_INDEX(fifo->head);
}

int dequeue_fifo(struct fifo *fifo, unsigned long *val) {
  if (is_empty_fifo(fifo))
    return -1;

  if (val)
    *val = fifo->buf[fifo->tail];

  fifo->tail = NEXT_INDEX(fifo->tail);
  fifo->used--;

  return 0;
}
