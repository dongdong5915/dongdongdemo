#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

struct person
{
    int age;
    char name[20];
    struct list_head list;
};

void main(int argc, char* argv[])
{
    struct person *pperson;
    struct person person_head;
    struct list_head *pos, *next;
    int i;

    // ��ʼ��˫����ı�ͷ
    INIT_LIST_HEAD(&person_head.list);

    // ��ӽڵ�
    for (i=0; i<5; i++)
    {
        pperson = (struct person*)malloc(sizeof(struct person));
        pperson->age = (i+1)*10;
        sprintf(pperson->name, "%d", i+1);
        // ���ڵ����ӵ������ĩβ
        // �����ѽڵ����ӵ�����ı�ͷ���棬��ʹ�� list_add
        list_add_tail(&(pperson->list), &(person_head.list));
    }

    // ��������
    printf("==== 1st iterator d-link ====\n");
    list_for_each(pos, &person_head.list)
    {
        pperson = list_entry(pos, struct person, list);
        printf("name:%-2s, age:%d\n", pperson->name, pperson->age);
    }

    // ɾ���ڵ�ageΪ20�Ľڵ�
    printf("==== delete node(age:20) ====\n");
    list_for_each_safe(pos, next, &person_head.list)
    {
        pperson = list_entry(pos, struct person, list);
        if(pperson->age == 20)
        {
            list_del_init(pos);
            free(pperson);
        }
    }

    // �ٴα�������
    printf("==== 2nd iterator d-link ====\n");
    list_for_each(pos, &person_head.list)
    {
        pperson = list_entry(pos, struct person, list);
        printf("name:%-2s, age:%d\n", pperson->name, pperson->age);
    }

    // �ͷ���Դ
    list_for_each_safe(pos, next, &person_head.list)
    {
        pperson = list_entry(pos, struct person, list);
        list_del_init(pos);
        free(pperson);
    }

}
