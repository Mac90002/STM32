#ifndef __BSP_TASK_H
#define __BSP_TASK_H

#define START_TASK_PRIO         1
#define START_TASK_STACK_SIZE   128

/* TASK1 ���� ����
 * ����: ������ �������ȼ� ��ջ��С ��������
 */#define TASK1_PRIO       2
#define TASK1_STACK_SIZE   128


/* TASK2 ���� ����
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define TASK2_PRIO         3
#define TASK2_STACK_SIZE   128

/* TASK3 ���� ����
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define TASK3_PRIO         4
#define TASK3_STACK_SIZE   128



void OS_Task(void);

#endif /* __BSP_TASK_H */



