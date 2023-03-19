#include <stdio.h>
#include "sys.h"
#include "rtos_api.h"
// Инициализация стеков
void InitializeStacks(int numStack)
{
    char cushionSpace[100000];
    cushionSpace[99999] = 1; // отключаем оптимизацию массивов
    for (int i = 0; i <= MAX_TASK; ++i) {
        if (!setjmp(InitStacks[i])) {
            continue;
        } else {
            TaskQueue[RunningTask].entry();
            break;
        }
    }
}
int StartOS(TTaskCall entry, int priority, char *name)
{
    RunningTask = TaskHead = -1;
    TaskCount = 0; //подсчет задач
    FreeTask = 0; //ожидающие задачи
    printf("StartOS!\n");
    InitializeStacks(0); //инициализация стека
    for (int i = 0; i < MAX_TASK; i++)
    {
        TaskQueue[i].next = i + 1; // номер массива на след элемент
        TaskQueue[i].prev = i - 1; // пред элемент
        TaskQueue[i].task_state = TASK_SUSPENDED; // пометка ожидания (неактивная задача)
        TaskQueue[i].switch_count = 0; // ключ
        TaskQueue[i].waiting_events = 0; // ожидание задачи
        TaskQueue[i].working_events = 0; // сработавшие задачи
    } // создание массива задач
    TaskQueue[MAX_TASK - 1].next = 0; // присваиваем последнему элементу 0
    TaskQueue[0].prev = MAX_TASK - 1; // присваиваем предпоследний
    if (!setjmp(MainContext)) {
        ActivateTask(entry, priority, name); // запускаем функцию активации задачи
    }
    return 0;
}
// Завершает работу системы(задача завершена)
void ShutdownOS()
{
    printf("ShutdownOS!\n");
}