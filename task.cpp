#include <cstdio>
#include <cstring>
#include "sys.h"
#include "rtos_api.h"
// добавить задачу перед item
void InsertTaskBefore(int task, int item, int *head)
{
    if (TaskQueue[task].next != -1)
        return;
    if (item == *head)
        *head = task;
    TaskQueue[task].next = item;
    TaskQueue[task].prev = TaskQueue[item].prev;
    TaskQueue[TaskQueue[item].prev].next = task;
    TaskQueue[item].prev = task; // ИСКЛЮЧЕНИЕ
}
void RemoveTask(int task, int *head) // удаление задачи из списка
{
    if (TaskQueue[task].next == -1)
        return;
    if (*head == task)
    {
        if (TaskQueue[task].next == task)
            *head = -1;
        else
            *head = TaskQueue[task].next;
    }
    TaskQueue[TaskQueue[task].prev].next = TaskQueue[task].next;
    TaskQueue[TaskQueue[task].next].prev = TaskQueue[task].prev;
    TaskQueue[task].next = -1;
    TaskQueue[task].prev = -1;
}
void TerminateTask(void) // завершить задачу
{
    int task;
    TaskCount--; // вычитаем из количества задач
    task = RunningTask;
    TaskQueue[task].task_state = TASK_SUSPENDED; // вносим в ожидающие
    RemoveTask(task, &TaskHead); // удаляем задачу из списка
// добавляем задачу в список свободных
    InsertTaskBefore(task, FreeTask, &FreeTask);
    if (TaskCount == 0)
        longjmp(MainContext, 1);
    Dispatch(); // переходим на другую задачу
}
// Переводит задачу из состояния suspended в состояние ready
int ActivateTask(TTaskCall entry, int priority, char *name) //функция активации задачи
{
    int task, occupy;
    task = TaskHead; // указатель на голову
    occupy = FreeTask; // указатель на свободную задачу // изменяем список свободных задач
    RemoveTask(occupy, &FreeTask); // удаление задачи из списка
    TaskQueue[occupy].priority = priority;
    TaskQueue[occupy].ceiling_priority = priority;
    TaskQueue[occupy].name = name;
    TaskQueue[occupy].entry = entry;
    TaskQueue[occupy].switch_count = 0;
    TaskQueue[occupy].task_state = TASK_READY;
    TaskCount++;
    Schedule(occupy); // добавление задачи в очередь активных задач
    if (task != TaskHead) {
        Dispatch();
    }
    return occupy;
}

void InsertTaskAfter(int task, int item)
{
    if (TaskQueue[task].next != -1) {
        return;
    }
    TaskQueue[task].next = TaskQueue[item].next;
    TaskQueue[task].prev = item;
    TaskQueue[TaskQueue[item].next].prev = task;
    TaskQueue[item].next = task;
}

void Schedule(int task, int dont_show) // добавление в список активных задач
{
    int cur;
    int priority;
    if (TaskQueue[task].task_state == TASK_SUSPENDED) {
        return;
    }
    if (TaskHead == -1) {
        TaskHead = task;
        TaskQueue[task].next = task;
        TaskQueue[task].prev = task;
    } else if (TaskCount > 1) {
        cur = TaskHead;
        priority = TaskQueue[task].ceiling_priority;
        RemoveTask(task, &TaskHead); // удаление задачи из списка
        while (TaskQueue[cur].ceiling_priority >= priority) {
            cur = TaskQueue[cur].next;
            if (cur == TaskHead) {
                break;
            }
        }
        if (priority <= TaskQueue[TaskHead].ceiling_priority && cur == TaskHead) {
// вставляем в конец списка
            InsertTaskAfter(task, TaskQueue[TaskHead].prev);
        } else {
            InsertTaskBefore(task, cur, &TaskHead); // вставляем перед элементом
        }
    } if (!dont_show) {
        printf("End of Schedule %s\n", TaskQueue[task].name);
    }
}

void TaskSwitch(int nextTask) // восстановление задачи next text переключатель задач
{
    if (nextTask == -1)
    {
        return;
    }
    TaskQueue[nextTask].task_state = TASK_RUNNING;
    RunningTask = nextTask; // текущая выполняющаяся задача
    TaskQueue[nextTask].switch_count++; // ставим номер счетчика задачи
    if (TaskQueue[nextTask].switch_count == 1) {
        longjmp(InitStacks[nextTask], 1); // восстановление предыдущей задачи
    } else {
        longjmp(TaskQueue[nextTask].context, 1);
    }
}

void Dispatch() // переключение задач
{
    int cur = 0;
    if ((RunningTask != -1) && (TaskQueue[RunningTask].task_state == TASK_RUNNING)) {
        TaskQueue[RunningTask].task_state = TASK_READY; // меняем пометку
    }
    cur = TaskHead; // указатель на голову
    while (TaskCount) {
        if (TaskQueue[cur].task_state == TASK_READY) {
// переключаемся на следующую задачу
            if (RunningTask == -1 || TaskQueue[RunningTask].task_state == TASK_SUSPENDED) {
                TaskSwitch(cur);
            }
            break;
        }
        cur = TaskQueue[cur].next;
        if (cur == TaskHead) {
            longjmp(MainContext, 1);
            printf("debug");
        }
    }
    Schedule(cur, 1); // добавляем в список активных задач
}
