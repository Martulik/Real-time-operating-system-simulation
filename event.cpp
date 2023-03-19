#include <cstdio>
#include "sys.h"
#include "rtos_api.h"

// Устанавливает для задачи события, задаваемые маской
void SetEvent(int task, TEventMask mask)
{
    TaskQueue[task].working_events |= mask;
    if (TaskQueue[task].task_state == TASK_WAITING &&
        (TaskQueue[task].working_events & TaskQueue[task].waiting_events))
    {
        TaskQueue[task].waiting_events &= !mask;
        TaskQueue[task].working_events &= !mask;
        TaskQueue[task].task_state = TASK_READY;
        Dispatch();
    }
}
// Возвращаем текущее состояние маски установленных событий для задачи
void GetEvent(int task, TEventMask *mask)
{
    *mask = TaskQueue[task].working_events;
}
// Задача переводится в состояние ожидания
void WaitEvent(TEventMask mask)
{
    TaskQueue[RunningTask].waiting_events = mask;
    if ((TaskQueue[RunningTask].working_events & mask) == 0)
    {
        TaskQueue[RunningTask].task_state = TASK_WAITING;
        Dispatch();
    }
}
// Очистка признаков событий(в маске установленных событий задачи) в соответствии с параметром маски
void ClearEvent(TEventMask mask)
{
    TaskQueue[RunningTask].working_events &= !mask;
}