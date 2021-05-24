#include "../include/test.h"

#define MINOR_WAIT 1000000 // TODO: To prevent a process from flooding the screen
#define WAIT 10000000      // TODO: Long enough to see theese processes beeing run at least twice

void bussy_wait(uint64_t n)
{
  uint64_t i;
  for (i = 0; i < n; i++)
    ;
}

void endless_loop2()
{
  uint64_t pid = _getPid();

  while (1)
  {
    printInt(pid);
    bussy_wait(MINOR_WAIT);
  }
}

#define TOTAL_PROCESSES 3

void test_prio()
{
  uint64_t pids[TOTAL_PROCESSES];
  uint64_t i;

  for (i = 0; i < TOTAL_PROCESSES; i++)
  {
    char *argv[] = {"endless_loop"};
    pids[i] = _createProcess(&endless_loop2, 1, argv, 0, NULL); //En caso de ejecutar el test como background va a devolver error porque un bg no puede crear un fg
  }
  bussy_wait(WAIT);
  print("\nCHANGING PRIORITIES...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
  {
    switch (i % 3)
    { 
    case 0:
      _nice(pids[i], 1); //lowest priority
      break;
    case 1:
      _nice(pids[i], 6); //medium priority
      break;
    case 2:
      _nice(pids[i], 12); //highest priority
      break;
    }
  }

  bussy_wait(WAIT);
  print("\nBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _block(pids[i]);

  print("CHANGING PRIORITIES WHILE BLOCKED...\n");
  for (i = 0; i < TOTAL_PROCESSES; i++)
  {
    switch (i % 3)
    {
    case 0:
      _nice(pids[i], 6); //medium priority
      break;
    case 1:
      _nice(pids[i], 6); //medium priority
      break;
    case 2:
      _nice(pids[i], 6); //medium priority
      break;
    }
  }

  print("UNBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _unblock(pids[i]);

  bussy_wait(WAIT);
  print("\nKILLING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _kill(pids[i]);
}
