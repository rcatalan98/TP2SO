#include "../include/test.h"

#define TOTAL_PAIR_PROCESSES 2
#define SEM_ID "sem"
#define ITER "67"

int64_t global; //shared memory
int64_t currentPid;
void slowInc(int64_t *p, int64_t inc)
{
  int64_t aux = *p;
  // print("Soy el proceso: ");
  // printInt(_getPid());
  // print(" el valor antes del incremento es: ");
  // if (aux < 0)
  // {
  //   print("-");
  //   printInt(aux * (-1));
  // }
  // else
  // {
  //   printInt(aux);
  // }
  // print("\n");
  aux += inc;
  _yield();
  *p = aux;
  // print("Soy el proceso: ");
  // printInt(_getPid());
  // print(" el valor despues del incremento es: ");
  // if (aux < 0)
  // {
  //   print("-");
  //   printInt(aux * (-1));
  // }
  // else
  // {
  //   printInt(aux);
  // }
  // print("\n");
}

void inc(int argc, char *argv[])
{
  int sem = atoi2(argv[1]);
  int value = atoi2(argv[2]);
  // print("El inc es ");
  // if (value < 0)
  // {
  //   print("-");
  //   printInt(value * (-1));
  // }
  // else
  // {
  //   printInt(value);
  // }
  // print("Para el proceso ");
  // printInt(_getPid());
  // print(".\n");
  int N = atoi2(argv[3]);
  int semIndex;
  uint64_t i;
  currentPid = _getPid();
  if (sem && (semIndex = _semOpen(SEM_ID, 1)) == -1)
  {
    print("ERROR OPENING SEM\n");
    return;
  }
  //print("Mi sem es ");
  // printInt(semIndex);
  // print("\n");
  for (i = 0; i < N; i++)
  {
    if (sem)
    {
      // print("Soy el proceso: ");
      // printInt(_getPid());
      // print(" estoy por entrar al wait\n");
      _semWait(semIndex);
    }
    // print("Soy el proceso: ");
    // printInt(_getPid());
    // print(" sali del wait.\n");
    slowInc(&global, value);
    // print("Soy el proceso: ");
    // printInt(_getPid());
    // print(" sali del slowInc");
    // print("\n");
    if (sem)
    {
      if (_semPost(semIndex) == -1)
        print("Error en el post");
    }
    // print("Soy el proceso: ");
    // printInt(_getPid());
    // print(" sali del post\n");
   // _yield();
  }
  if (sem)
    _semClose(SEM_ID);
  print("Soy el proceso: ");
  printInt(_getPid());
  print(" el final value: ");
  if (global < 0)
  {
    print("-");
    printInt(global * (-1));
  }
  else
  {
    printInt(global);
  }
  print(".\n");
}

void test_sync()
{
  uint64_t i;

  global = 0;

  print("CREATING PROCESSES...(WITH SEM)\n");

  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    char *argv1[] = {"inc", "1", "1", ITER};
    char *argv2[] = {"inc", "1", "-1", ITER};
    _createProcess(&inc, 4, argv1, 0, NULL);
    _createProcess(&inc, 4, argv2, 0, NULL);
  }
  // sleep(atoi2(ITER));
  // _ps();
  // print("FINAL Global value: ");
  // if (global < 0)
  // {
  //   print("-");
  //   printInt(global * (-1));
  // }
  // else
  // {
  //   printInt(global);
  // }
  // print("\n");
  // print("Termino test_sync\n");
}

void test_no_sync()
{
  uint64_t i;

  global = 0;

  print("CREATING PROCESSES...(WITHOUT SEM)\n");

  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    char *argv1[] = {"inc", "0", "1", ITER}; // 100000
    char *argv2[] = {"inc", "0", "-1", ITER};
    _createProcess(&inc, 4, argv1, 0, NULL);
    _createProcess(&inc, 4, argv2, 0, NULL);
  }
}