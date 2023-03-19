#include "sys.h"

TTask TaskQueue[MAX_TASK];
TResource ResourceQueue[MAX_RES];
int RunningTask;
int FreeTask;
int FreeResource;
int TaskHead;
int TaskCount;
jmp_buf MainContext;
jmp_buf InitStacks[MAX_TASK];
