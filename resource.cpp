#include "sys.h"
#include "rtos_api.h"
#include <stdio.h>

void InitRes(int ResNum, char *name) // инициализация ресурса
{
    ResourceQueue[ResNum].name = name;
    ResourceQueue[ResNum].task = -1;
}

void PIP_GetRes(int ResNum) // ресурс, запрашиваемый задачей
{
    while (ResourceQueue[ResNum].task != -1)
    { // ресурс заблокирован
        int rtask = ResourceQueue[ResNum].task;
        if (TaskQueue[rtask].ceiling_priority <=
            TaskQueue[RunningTask].ceiling_priority)
        {
// увеличив времен приоритет на 1
            TaskQueue[rtask].ceiling_priority = TaskQueue[RunningTask].ceiling_priority + 1;
            Schedule(rtask); // добавляем в очередь
        }
        Dispatch(); // переключение
    }
    ResourceQueue[ResNum].task = RunningTask; // ресурс занят
}

void PIP_ReleaseRes(int ResNum) // ресурс, подлежащий освобождению
{
    int rtask = ResourceQueue[ResNum].task;
    ResourceQueue[ResNum].task = -1;
// сбрасываем временный приоритет до базового
    if (TaskQueue[rtask].ceiling_priority != TaskQueue[rtask].priority) {
        TaskQueue[rtask].ceiling_priority = TaskQueue[rtask].priority;
        Schedule(rtask);
    }
    Dispatch(); // переход к следующей задаче
}