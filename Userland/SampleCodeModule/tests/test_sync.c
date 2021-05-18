#include "../include/test.h"

#define TOTAL_PAIR_PROCESSES 2
#define SEM_ID "sem"

int64_t global; //shared memory

void slowInc(int64_t *p, int64_t inc)
{
  int64_t aux = *p;
  aux += inc;
  _yield();
  *p = aux;
}

void inc(int argc, char *argv[])
{
  int sem = atoi2(argv[1]);
  int value = atoi2(argv[2]);
  int N = atoi2(argv[3]);
  void *aux;
  uint64_t i;

  if (sem && (aux = _semOpen(SEM_ID, 1)) == NULL)
  {
    print("ERROR OPENING SEM\n");
    return;
  }

  for (i = 0; i < N; i++)
  {
    print("En el for\n");
    if (sem)
      _semWait(aux);

    print("Pase el semWait\n");

    slowInc(&global, value);
    if (sem)
      _semPost(aux);

    print("Pase el semPost\n");
  }

  if (sem){
    _semClose(SEM_ID);
    print("Pase semClose\n");
  }

  print("Final value: ");
  printInt(global);
  print("\n");
}

void test_sync()
{
  uint64_t i;

  global = 0;

  print("CREATING PROCESSES...(WITH SEM)\n");

  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    char *argv1[] = {"inc", "1", "1", "100000"};
    char *argv2[] = {"inc", "1", "-1", "100000"};
    _createProcess(&inc, 4, argv1, 0);
    _createProcess(&inc, 4, argv2, 0);
  }
}

void test_no_sync()
{
  uint64_t i;

  global = 0;

  print("CREATING PROCESSES...(WITHOUT SEM)\n");

  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    char *argv1[] = {"inc", "1", "1", "100000"};
    char *argv2[] = {"inc", "1", "-1", "100000"};
    _createProcess(&inc, 4, argv1, 0);
    _createProcess(&inc, 4, argv2, 0);
  }
}