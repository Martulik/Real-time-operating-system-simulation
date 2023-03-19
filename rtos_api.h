#include "sys.h"

#define DeclareTask(TaskID, priority) \
    TASK(TaskID);   \
    enum {TaskID##prior = priority}

#define DeclareResource(ResID, priority) \
    enum{ResID = priority}

#define TASK(TaskID) void TaskID(void)

typedef void TTaskCall(void);
int ActivateTask(TTaskCall entry, int priority, char *name);
void TerminateTask(void);
int StartOS(TTaskCall entry, int priority, char *name);
void ShutdownOS();
void InitRes(int ResNum, char *name);
void PIP_GetRes(int ResNum);
void PIP_ReleaseRes(int ResNum);
#define DeclareEvent(ID) \
const int Event ##ID = (ID) * (ID);
void SetEvent(int task, TEventMask mask);
void GetEvent(int task, TEventMask *mask);
void WaitEvent(TEventMask mask);
void ClearEvent(TEventMask mask);