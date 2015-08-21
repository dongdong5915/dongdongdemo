#include <stdio.h>

/*
 * dist 每段运送(中途卸载)的距离
 * coal 每段需要运的煤数量
 * left_dist 每段与市场终点的距离
 * return 返回到达市场后剩余煤的数量
 * */
int trans(int dist, int coal, int left_dist)
{
    if(coal <= 1000)
    {
        return coal-left_dist;
        //如果当前段需要运煤的数量小于1000(小于火车最大装载量),则一次运到市场,剩余煤数量即为 当前段需要运煤数量减去当前段与市场的距离
    }
    int left=1000-dist*2; //每段中, 在每个来回后中途卸载煤的数量
    //printf("left is %d \n",left);
    int amount=(coal/1000)*left;// 每段中, 所有来回后送给卸煤的数量
    //printf("amount1 is %d\n",amount);

    if(coal%1000>2*dist)
    {
        amount += coal%1000-dist;
    }else{
        amount += dist;
    }
    //printf("amount2 is %d\n",amount);
    return trans(dist, amount, left_dist-dist); //递归计算下一段的运煤
}

int main()
{
    int best=0;
    int i = 1;
    for(i=1;i<500;i++)
    {
        int tmp=trans(i, 3000, 1000);//分别计算每段运送距离为1到500公里的运送方案的最后剩余煤数量
        printf("i = %d, tmp=%d\n",i,tmp);
        if(tmp>best)
        {
          best=tmp;
        }
    }
    printf("best is %d \n", best);
}
