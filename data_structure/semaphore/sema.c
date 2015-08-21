#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define SEMKEY 1234L
#define PERMS 0666

struct sembuf op_down[1] = {0, -1, 0};
struct sembuf op_up[1] = {0, 1, 0};

int semid = -1;
int res;

void init_sem()
{
    /*测试信号量是否已经存在*/
    semid = semget(SEMKEY, 0, IPC_CREAT | PERMS);
    if (semid < 0)
    {
        printf("Create the semaphore \n");

        semid = semget(SEMKEY, 1, IPC_CREAT | PERMS);

        if (semid < 0)
        {
            printf("Cound't create semaphore! \n");
            exit(-1);
        }
        res = semctl(semid, 0, SETVAL, 1);
    }

}

void down()
{
    res = semop(semid, &op_down[0], 1);
}

void up()
{
    res = semop(semid, &op_up[0], 1);
}

int main()
{
    init_sem();

    printf("Before critical code \n");

    down();
    /*临界区代码*/
    printf("In Critical code \n");
    sleep(10);

    up();

    return 0;

}
