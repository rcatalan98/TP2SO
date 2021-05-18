#include "../include/test.h"
#define MAX_BLOCKS 128
#define MAX_MEMORY (1024 * 1024 * 60) // Around 80% of memory managed by the MM (128 Mb)

typedef struct MM_rq
{
  void *address;
  uint32_t size;
} mm_rq;

void test_mm()
{
  mm_rq mm_rqs[MAX_BLOCKS];
  uint8_t rq;
  uint32_t total;
  while (1)
  {
    rq = 0;
    total = 0;

    // Request as many blocks as we can
    while (rq < MAX_BLOCKS && total < MAX_MEMORY)
    {
      mm_rqs[rq].size = GetUniform(MAX_MEMORY - total - 1) + 1;
      mm_rqs[rq].address = (void *)_mallocFF(mm_rqs[rq].size); // TODO: Port this call as required
                                                               // TODO: check if NULL
      if (mm_rqs[rq].address == NULL)
      {
        print("total:");
        printInt(total);
        print("\n");
        print("Returned Null\n");
      }
      total += mm_rqs[rq].size;
      rq++;
    }

    // // Set
    uint32_t i;
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address != NULL)
        memset(mm_rqs[i].address, i, mm_rqs[i].size); // TODO: Port this call as required

    // Check
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address != NULL)
        if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size))
          print("ERROR!\n"); // TODO: Port this call as required

    // Free
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address != NULL)
        _freeFF(mm_rqs[i].address); // TODO: Port this call as required
  }
}
