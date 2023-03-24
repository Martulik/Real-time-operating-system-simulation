#ifndef SYS_H
#define SYS_H
#include "defs.h"
#include <csetjmp>

enum TTaskState
{
    TASK_RUNNING = 0, // выполняется сейчас
    TASK_READY = 1,  // готов к выполнению
    TASK_SUSPENDED = 2, // ожидает когда станет которым к выполнению
    TASK_WAITING = 3 // ждёт ??
};

typedef unsigned int TEventMask;
typedef struct Type_Task
        {
                int next, prev;
        int priority;
        int ceiling_priority;
        void (*entry)(void);
        char *name;
        TTaskState task_state;
        int switch_count;
        jmp_buf context; //сохранение контекта
        TEventMask waiting_events;
        TEventMask working_events;
        } TTask;

typedef struct Type_resource
        {
                int task;
        char *name;
        } TResource;
extern jmp_buf InitStacks[MAX_TASK]; //массив сохраненных контекстов
extern TTask TaskQueue[MAX_TASK]; //MAX_TASK = 32 очередь готовых к выполнению задач
extern TResource ResourceQueue[MAX_RES]; //MAX_RES = 16
extern int RunningTask; // текущая выполняющаяся задача
extern int TaskHead; // первая задача в списке
extern int TaskCount; //количество готовых к выполнению задач
extern int FreeTask; // указатель на первую свободную (ожидающую) задачу
extern jmp_buf MainContext; // главный контекст
void Schedule(int task, int dont_show = 0); //второй параметр по умолчанию грит что конец очереди активных задач
void Dispatch();
#endif