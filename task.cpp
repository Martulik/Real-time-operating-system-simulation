#include <cstdio>
#include "sys.h"
#include "rtos_api.h"

void InsertTaskBefore(int task, int item, int *head) // добавить задачу task перед задачей item
{
    if (TaskQueue[task].next != -1) // если задача в TaskQueue, то ничего не делаем
        return;
    if (item == *head)
        *head = task;
    TaskQueue[task].next = item; //классическая вставка в середину списка
    TaskQueue[task].prev = TaskQueue[item].prev;
    TaskQueue[TaskQueue[item].prev].next = task;
    TaskQueue[item].prev = task; // ИСКЛЮЧЕНИЕ
}

void RemoveTask(int task, int *head) // удаление задачи из TaskQueue (номер задачи, указатель на первый элемент в очереди)
{
    if (TaskQueue[task].next == -1) // если задача уже не в TaskQueue, то ничего не делаем
        return;
    if (*head == task) //если удаляемая задача была первой в очереди, то
    {
        if (TaskQueue[task].next == task) //если задача была первой и единственной, то просто указатель на начало ставим -1 (пустая очередь)
            *head = -1;
        else
            *head = TaskQueue[task].next; //иначе переносим начало очереди на следующий элемент
    }
    TaskQueue[TaskQueue[task].prev].next = TaskQueue[task].next; //классическое удаление из середины списка
    TaskQueue[TaskQueue[task].next].prev = TaskQueue[task].prev;
    TaskQueue[task].next = -1;
    TaskQueue[task].prev = -1;
}
void TerminateTask() // завершить задачу (TODO но не доделать? Может значит преостановить?)
{
    TaskCount--; // вычитаем из количества готовых к выполнению задач
    int task = RunningTask; //получаем позицию выполняющейся задачи
    TaskQueue[task].task_state = TASK_SUSPENDED; // вносим в ожидающие
    RemoveTask(task, &TaskHead); // удаляем задачу из списка

    InsertTaskBefore(task, FreeTask, &FreeTask); // добавляем задачу в список свободных (ставим ее перед первой свободной задачей)
    if (TaskCount == 0)
        longjmp(MainContext, 1); //восстанавливаем окружение, сохраненное в точке setjmp
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
    TaskQueue[occupy].ceiling_priority = priority; // максимальный приоритет
    TaskQueue[occupy].name = name;
    TaskQueue[occupy].entry = entry; //TODO за что отвечает это поле?
    TaskQueue[occupy].switch_count = 0;
    TaskQueue[occupy].task_state = TASK_READY;
    TaskCount++;
    Schedule(occupy); // добавление задачи в очередь активных задач
    if (task != TaskHead) {  //TODO видимо будем переключаться до тех пор пока выбранная задача не окажется первой в очереди?
        Dispatch();
    }
    return occupy;
}

void InsertTaskAfter(int task, int item)
{
    if (TaskQueue[task].next != -1) { // если задача в TaskQueue, то ничего не делаем
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
    if (TaskQueue[task].task_state == TASK_SUSPENDED) { //если задача еще ждет своей очереди попасть в очередь готовых к выполнению задач, то ниче не делаем
        return;
    }
    if (TaskHead == -1) { // если очередь готовых к выполнению задач пуста, то вставляем задачу в список
        TaskHead = task;
        TaskQueue[task].next = task; //начало и конец указываем на себя же
        TaskQueue[task].prev = task;
    } else if (TaskCount > 1) {
        cur = TaskHead;
        priority = TaskQueue[task].ceiling_priority; // ставим задаче макс приоритет
        RemoveTask(task, &TaskHead); // удаление задачи из списка
        while (TaskQueue[cur].ceiling_priority >= priority) { //пока не найдется задача с меньшим приоритетом чем у нашей - двигаем
            cur = TaskQueue[cur].next;
            if (cur == TaskHead) {
                break;
            }
        } //TODO не оч понятно
        if (priority <= TaskQueue[TaskHead].ceiling_priority && cur == TaskHead) { // если наш приоритет меньше или равен приоритету начала списка и текущий равен началу, то
            InsertTaskAfter(task, TaskQueue[TaskHead].prev); // вставляем в конец списка
        } else {
            InsertTaskBefore(task, cur, &TaskHead); // вставляем перед элементом
        }
    } if (!dont_show) { //флаг, по умолчанию указывает что это конец расписания
        printf("End of Schedule %s\n", TaskQueue[task].name);
    }
}

//используется только в Dispatch
void TaskSwitch(int nextTask) // восстановление задачи next text переключатель задач
{
    if (nextTask == -1)
    {
        return;
    }
    TaskQueue[nextTask].task_state = TASK_RUNNING;
    RunningTask = nextTask; // текущая выполняющаяся задача
    TaskQueue[nextTask].switch_count++; // ставим номер счетчика задачи (ключ)
    if (TaskQueue[nextTask].switch_count == 1) { //если мы переключились на задачу в первый раз
        longjmp(InitStacks[nextTask], 1); // восстановление предыдущей задачи (из массива контекстов)
    } else {
        longjmp(TaskQueue[nextTask].context, 1); //если переключаемся не в первыый, то восстанавливаем ее контекст
    }
}

void Dispatch() // переключение задач
{
    if ((RunningTask != -1) && (TaskQueue[RunningTask].task_state == TASK_RUNNING)) { //если есть задача, которая выполняется в данный момент
        TaskQueue[RunningTask].task_state = TASK_READY; // меняем пометку обратно на готовность к выполнению
    }
    int cur = TaskHead; // указатель на голову
    while (TaskCount) { //пока TaskCount != 0
        if (TaskQueue[cur].task_state == TASK_READY) { //TODO если текущая задача готова к выполнению, а также у нас нет выполняющихся задач или выполняющаяся задача в ожидании?
            if (RunningTask == -1 || TaskQueue[RunningTask].task_state == TASK_SUSPENDED) { // переключаемся на следующую задачу
                TaskSwitch(cur);
            }
            break;
        }
        cur = TaskQueue[cur].next;
        if (cur == TaskHead) { //если вернулись в начало, то переключаемся на главный контекст
            longjmp(MainContext, 1);
        }
    }
    Schedule(cur, 1); // добавляем в список активных задач
}
