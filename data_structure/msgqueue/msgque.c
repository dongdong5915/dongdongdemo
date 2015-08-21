#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>

#define MESSAGE_LEN 100
//������Ϣ�����е�һ���������͵Ŀռ�
struct msg_buff
{
    long m_type;
    char m_data[MESSAGE_LEN];
};
//��ʼ����Ϣ����
int init_message(char *fileName)
{
    int msgid;
    key_t key;

    key = ftok(fileName, 1);
    if (key < 0) {
        perror("ftok()");
    }

    msgid = msgget(key, IPC_CREAT | 0600);
    if (msgid < 0) {
        perror("msgget");
    }

    return msgid;
}


int main(int argc, char **argv)
{
    pid_t pid;

    if ((pid = fork()) < 0) {
        perror("fork()");
    } else if (pid == 0) { //child
        int msgid;
        struct msg_buff sb, rb;
        //��ʼ��
        msgid = init_message("/tmp/1");
        sprintf(sb.m_data, "wwww in child");
        sb.m_type = 1;
        while (1) {
            //��������
            if (msgsnd(msgid, &sb, MESSAGE_LEN, 0) < 0) {
                perror("msgsnd()");
            }
            //�������ݣ����������������͵����ݣ�û�����ݻ�����
            if (msgrcv(msgid, &rb, MESSAGE_LEN, 0, 0) < 0) {
                printf("do not read in child\n");
            } else {
                printf("child read: %ld, %s\n", rb.m_type, rb.m_data);
            }

            sleep(1);
            sb.m_type++;
        }
    } else { //parent
        int msgid;
        struct msg_buff sb, rb;

        msgid = init_message("/tmp/1");
        sprintf(sb.m_data, "wwww in parent");
        sb.m_type = 1;
        while (1) {
            msgsnd(msgid, &sb, MESSAGE_LEN, 0);
            //������typeΪ1���������͵����ݣ���������ʽ����
            if (msgrcv(msgid, &rb, MESSAGE_LEN, 1, IPC_NOWAIT) < 0) {
                perror("recv");
            } else {
                printf("parent read: %ld, %s\n", rb.m_type, rb.m_data);
            }

            sleep(3);
            sb.m_type++;
        }
    }

    return 0;
}
