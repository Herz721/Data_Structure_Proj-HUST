//
//  main.cpp
//  课设
//
//  Created by 刘浩哲 on 2019/3/5.
//  Copyright © 2019 刘浩哲. All rights reserved.
//

#include <stdio.h>      //提供FILE *类型
#include <time.h>       //提供clock_t类型
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define OK 1
#define ERROR 0
#define INFEASTABLE -3
#define OVERFLOW -2
#define LIST_INIT_SIZE 100
#define LISTINCREMENT  10
#define chessvalue(a,b,c) (( 100 * (a) ) + ( 10 * (b) ) + (c) )

typedef struct LNode{
    int num;
    struct LNode *nextclau;
    struct LNode *nextlite;
}LNode,*List;

typedef struct Snode{
    int x;
    bool backTrack;
    List data;
    struct Snode * next;
}Snode,*Stack;           //链式堆栈

/*-------DPLL相关函数-------*/
int DPLL(void);                             //DPLL核心算法
int SingleClause(void);                     //循环化简
int Simplify(int x);                        //利用x化简
int NextLiteral1(void);                     //当没有单句时取出一个未处理的变元
int NextLiteral2(void);
int NextLiteral3(void);
void CreateList(void);                   
void Deleteclause(List clause);
void Deleteliteral(List clause,int x);
int  JudgeClause(List clause);
void ListCopy(List & dest, const List src);
void ListDestroy(List delHead);
/*-------DPLL相关函数-------*/

/*-------文件相关函数-------*/
void OpenFile(void);
void LoadFile(void);
void SaveFile(int c);
/*-------文件相关函数-------*/

/*--------栈相关函数--------*/
void StackInit(void);
bool StackEmpty(void);
void Push(List backup, int x, bool backTrack);
List Pop(void);
void Clean(void);
/*--------栈相关函数--------*/

void ShowAnswer(int *value);

/*------数独游戏相关函数------*/
void Createcnf(void);
void FillNum(void);
void CreateBoard(void);
void Game(void);
/*------数独游戏相关函数------*/

clock_t Time(clock_t time1,clock_t time2);

List head;
FILE *fp;
int clausenum1;
int clausenum2;
int boolnum1;
int boolnum2;
int *value;
int chessclausenum;
bool chess[9][9][9];
char filename[500];
char filename2[500];
clock_t time1,time2;
int k=0;

Stack top;

int main(void){
    int op = 1;
    while(op){
        printf("---------主菜单---------\n");
        printf("1.求解SAT  2.数独游戏\n");
        printf("3.删除子句  4.添加子句\n");
        printf("5.删除文字  6.添加文字\n");
        printf("0.退出\n");
        printf("---------主菜单---------\n");
        scanf("%d",&op);
        if(op==1){
            CreateList();
            StackInit();
            time1 = clock();
            int c = DPLL();
            clock_t time;
            if(c == -1){
                printf("此样例不满足！\n");
                time2 = clock();
                SaveFile(c);
            }
            else{
                printf("该样例满足！\n");
                time2 = clock();
                SaveFile(c);
            }
            time = Time(time1, time2);
            fprintf(fp, "t:%lu",time);
            fclose(fp);
            if(c != -1){
                ShowAnswer(value);
            }
            getchar();
        }
        else if(op==2){
            StackInit();
            Game();
            getchar();
        }
        else if(op==0){
            return 0;
        }
        else{
            printf("输入错误!\n");
            getchar();
        }
    }
}

int DPLL(void){
    int x = 0;
    bool backTrack = false;                     //指示当前是否在回溯
    List backup;                                //当前栈中的个数
    int flag;
    flag = SingleClause();
    if(flag == -1)
        return -1;
    if(head->nextclau == NULL)
        return 1;
    while(1){
        if(head->nextclau == NULL)
            return OK;
        if (!backTrack)
            x = NextLiteral1();                  //找到一个未被处理的变元并讨论(链表中第一个句子的第一个元素)
        ListCopy(backup, head);
        Push(backup, x, backTrack);
        Simplify(x);
        flag = SingleClause();
        if (flag == -1){                        //化简后有空子句, 需要回溯
            ListDestroy(head);
            head = Pop();                       //将head恢复为之前的状态
            if (!backTrack){                    //若发生冲突前不是在回溯
                x = -x;                         //则改变决策变元的真值
                if(x>0){
                    value[x-1] = 1;
                }
                else
                    value[-x-1] = 0;
                backTrack = true;
            }
            else {                              //若发生冲突前是在回溯, 则需要
                while (top->backTrack) {        //一直向前, 找到没有回溯过的节点
                    ListDestroy(head);
                    head = Pop();
                    if (StackEmpty())
                        return -1;
                }
                ListDestroy(head);
                x = -(top->x);
                if(x > 0)
                    value[x-1] = 1;
                else
                    value[-x-1] = 0;
                head = Pop();
                backTrack = true;
            }
        }
        else {    //化简后没有空子句
            backTrack = false;
            if (head->nextclau == NULL) {
                ListDestroy(head);
                return 1;
            }
        }
    }
}

int SingleClause(void){
    int temp;
    int num;
    int flag;
    List clausepoint = head->nextclau;          //头节点为空,从第二个节点开始
    while(clausepoint != NULL){
        num = JudgeClause(clausepoint);
        if(num == 0)
            return -1;
        else if(num == 1){                      //字句为单字句，进行化简
            temp = clausepoint->nextlite->num;
            if(temp>0)
                value[temp-1] = 1;
            else
                value[-temp-1] = 0;
            flag = Simplify(temp);
            if(flag == false)
                return -1;                      //该样例不满足，返回-1
            clausepoint = head;                 //化简完毕后重新循环，后面有一次next
        }
        clausepoint = clausepoint->nextclau;
    }                                           //通过单字句化简完毕，剩下都不为单字句
    return OK;
}

int Simplify(int x){
    int num;
    List clausepoint = head->nextclau;
    List literalpoint;
    while(clausepoint != NULL){
        num = JudgeClause(clausepoint);
        if(num != 0){
            literalpoint = clausepoint->nextlite;
            while(literalpoint->num != 0){
                if(literalpoint->num == x){             //删除该子句
                    Deleteclause(clausepoint);
                    clausepoint = head;
                    break;
                }
                else if(literalpoint->num == -x){       //删除该变元
                    Deleteliteral(clausepoint, -x);
                    if(JudgeClause(clausepoint) == 0)
                        return false;                   //不满足
                    literalpoint = clausepoint->nextlite;         //跳过后重新判断
                }
                else
                    literalpoint = literalpoint->nextlite;
            }
        }
        else
            return false;
        clausepoint = clausepoint->nextclau;
    }
    return OK;                                           //化简完成
}

int NextLiteral1(void){
    int i;
    i = head->nextclau->nextlite->num;
    if(i>0)
        value[i-1] = 1;
    else
        value[-i-1] = 0;
    return head->nextclau->nextlite->num;
}

int NextLiteral2(void){
    k++;
    value[k-1] = 1;
    return k;
}

int NextLiteral3(void){
    int i;
    List Clause,Literal;
    Clause=head->nextclau;
    Literal=Clause->nextlite;
    do{
    while(1){
        if(Literal->num>0)
            Literal=Literal->nextlite;
        else if(Literal->num==0)
            return Clause->nextlite->num;
        else
            break;
        }
        Clause=Clause->nextclau;
        Literal=Clause->nextlite;
    }while(Clause->nextclau!=NULL);
    i=NextLiteral1();
    return i;
}

void CreateList(void){
    List clauseTail, literalTail;     //尾指针
    int literal;                      //该字句的值
    OpenFile();                       //将文件读入并初始化
    LoadFile();
    head = (List)malloc(sizeof(LNode));
    if(!head){
        printf("内存不足！\n");
        exit(OVERFLOW);
    }
    head->nextclau = NULL;
    head->nextlite = NULL;
    clauseTail = head;                //尾指针指向第一个
    //开始将数据读链表
    for(int i = 0;i<clausenum1;i++){ //有头节点
        clauseTail->nextclau = (List)malloc(sizeof(LNode));
        clauseTail = clauseTail->nextclau;
        clauseTail->nextclau = NULL;
        clauseTail->nextlite = (List)malloc(sizeof(LNode));
        literalTail = clauseTail->nextlite;
        fscanf(fp, "%d",&literal);  //放入暂存
        while(literal != 0){        //读入值
            literalTail->num = literal;
            literalTail->nextlite = (List)malloc(sizeof(LNode));
            literalTail = literalTail->nextlite;
            literalTail->nextlite = NULL;
            fscanf(fp, "%d",&literal);
        }
    }
    fclose(fp);
}

void Deleteclause(List clause){
    List clauseTail,literalTail;
    List p,q;
    clauseTail = head;
    while(clauseTail->nextclau != clause){
        clauseTail = clauseTail->nextclau;
    }
    p = clause;
    clauseTail->nextclau = clause->nextclau;
    q = clause->nextlite;
    literalTail = clause->nextlite;
    while(literalTail != NULL){
        q = literalTail;
        literalTail = literalTail->nextlite;
        free(q);
        q = NULL;
    }
    free(p);
    p = NULL;
}

void Deleteliteral(List clause,int x){
    List literalTail = clause;
    List q;
    while(literalTail->nextlite->num != x){
        literalTail = literalTail->nextlite;
    }
    q = literalTail->nextlite;
    literalTail->nextlite = q->nextlite;
    free(q);
    q = NULL;
}

int JudgeClause(List clause){
    if(clause->nextlite->num == 0)
        return 0;
    else if(clause->nextlite->nextlite->num == 0)
        return 1;
    else return 2;
}

void ListCopy(List & dest, const List src) {
    List clauseTail1, literalTail1;     //源的尾指针
    List clauseTail2, literalTail2;     //目的地的尾指针
    dest = (List)malloc(sizeof(LNode));
    dest->nextclau = NULL;
    dest->nextlite = NULL;
    clauseTail2 = dest;
    clauseTail1 = src;
    while (clauseTail1->nextclau) {
        clauseTail1 = clauseTail1->nextclau;
        clauseTail2->nextclau = (List)malloc(sizeof(LNode));
        clauseTail2 = clauseTail2->nextclau;
        clauseTail2->nextclau = NULL;
        clauseTail2->nextlite = NULL;
        
        literalTail2 = clauseTail2;
        literalTail1 = clauseTail1;
        while (literalTail1->nextlite) {
            literalTail1 = literalTail1->nextlite;
            literalTail2->nextlite = (List)malloc(sizeof(LNode));
            literalTail2 = literalTail2->nextlite;
            literalTail2->nextlite = NULL;
            literalTail2->num = literalTail1->num;
        }//while
    }//while
}//listCopy

void ListDestroy(List delHead) {
    while (delHead->nextclau){
        Deleteclause(delHead->nextclau);
    }
    free(delHead);
    delHead = NULL;
}

void OpenFile(void){
    printf("请输入待读取的cnf文件:");
    scanf("%s", filename);
    if(!(fp = fopen(filename, "r"))){
        printf("文件打开失败!\n");
        exit(EXIT_FAILURE);
    }
    printf("文件打开成功\n正在求解……\n");
}

void LoadFile(void){
    char ch;
    int i;
    while((ch = getc(fp)) == 'c'){
        while((ch = getc(fp)) != '\n');
    }
    getc(fp);getc(fp);getc(fp);getc(fp);
    fscanf(fp, "%d",&boolnum1);
    boolnum2 = boolnum1;
    fscanf(fp, "%d",&clausenum1);
    clausenum2 = clausenum1;
    value = (int *)calloc(boolnum1, sizeof(int));    //为value分配空间
    for(i=0;i<boolnum1;i++){
        value[i] = 0;
    }
}

void SaveFile(int c){
    printf("请问你要存放的res文件名为:\n");
    scanf("%s",filename2);
    fp = fopen(filename2, "w+");
    if(c == -1){
        fprintf(fp, "s 0\n");
    }
    else{
        fprintf(fp, "s 1\nv ");
        for(int i = 0;i<boolnum1;i++){
            if(value[i])
                fprintf(fp, "%d ",i+1);
            else
                fprintf(fp, "-%d ",i+1);
        }
        fprintf(fp, "\n");
    }
}

void ShowAnswer(int *value){
    int i;
    printf("该样例的一个解为:");
    for(i = 0; i<boolnum1; i++){
        if(value[i] == 1)
            printf("%d ",i+1);
        else if(value[i] == 0)
            printf("-%d ",i+1);
        else
            printf("%d ",i+1);
    }
    printf("\n");
    getchar();
}

/*---------------栈相关------------------*/
void StackInit(void) {
    top = (Stack)malloc(sizeof(Snode));
    top->next = NULL;
    top->data = NULL;
}

bool StackEmpty(void) {
    return top->next ? false : true;
}

void Push(List backup, int x, bool backTrack) {
    Stack s = (Stack)malloc(sizeof(Snode));
    s->data = backup;
    s->x = x;
    s->backTrack = backTrack;
    s->next = top;
    top = s;
}

List Pop(void) {
    List r = top->data;
    Stack s = top;
    top = top->next;
    free(s);
    s = NULL;
    return r;
}

void Clean(void) {
    Stack s;
    while (top->next) {
        s = top;
        ListDestroy(s->data);
        top = top->next;
        free(s);
    }
    free(top);
    top = NULL;
}
/*---------------栈相关------------------*/

void Game(void){
    int flag1,flag2;
    printf("请问你是要创建一个新的数独游戏(输入1)还是使用已有的数独游戏(输入0)？\n");
    scanf("%d",&flag1);
    if(flag1==1) Createcnf();
    getchar();
    CreateList();
    clausenum1 = chessclausenum;
    if(DPLL() != -1){
        printf("是否查看此数独结果？(1查看,0不看)\n");
        scanf("%d",&flag2);
        getchar();
        if(flag2==1){
            printf("此数独结果为：\n");
        for(int x=0;x<9;x++){
            for(int y=0;y<9;y++){
                for(int z=1;z<=9;z++){
                    if(value[100*x+10*y+z-1] == 1){
                        printf("%d ",z);
                        break;
                    }
                }
            }
            printf("\n");
        }
        }
    }
    else{
        printf("此数独无解\n");
    }
}

void FillNum(void){
    int flag;
    int addnum;
    printf("请问有多少待添加数字?(数目必须大于17)\n");
    scanf("%d",&addnum);
    chessclausenum = addnum;
    fprintf(fp, "p cnf 889 %d\n",addnum+11745);
    for(flag = addnum;flag>0;flag--){
        int m,n,v;
        printf("请问待添加的数字在几排、几列、数字为多少？\n");
        scanf("%d%d%d",&m,&n,&v);
        fprintf(fp, "%d 0\n",(100*m+10*n+v-110));
        chessclausenum++;
    }
}

void Createcnf(void){
    printf("请问你要创建数独游戏的文件名为:\n");
    scanf("%s",filename);
    fp = fopen(filename, "w+");
    FillNum();
    CreateBoard();
    fclose(fp);
}

//创建棋盘
void CreateBoard(void){
    chessclausenum = 0;
    for(int x=0; x<9 ;x++){                 //在一个格子中间至少有一个数字
        for(int y=0;y<9;y++){
            for(int z=1;z<=9;z++){
                fprintf(fp, "%d ",chessvalue(x, y, z));
            }
            chessclausenum++;
            fprintf(fp, "0\n");
        }
    }
    for(int x=0;x<9;x++){                   //在一个格子中间最多有一个数字
        for(int y=0;y<9;y++){
            for(int z=1;z<9;z++){
                for(int i=z+1;i<=9;i++){
                    fprintf(fp, "%d %d 0\n",-chessvalue(x, y, z),-chessvalue(x, y, i));
                    chessclausenum++;
                }
            }
        }
    }
    for(int x=0;x<9;x++){                    //在一排中间不能重复
        for(int z=1;z<=9;z++){
            for(int y=0;y<8;y++){
                for(int i=y+1;i<9;i++){
                    fprintf(fp, "%d %d 0\n",-chessvalue(x, y, z),-chessvalue(x, i, z));
                    chessclausenum++;
                }
            }
        }
    }
    for(int y=0;y<9;y++){                    //在一列中间不能重复
        for(int z=1;z<=9;z++){
            for(int x=0;x<8;x++){
                for(int i=x+1;i<9;i++){
                    fprintf(fp, "%d %d 0\n",-chessvalue(x, y, z),-chessvalue(i, y, z));
                    chessclausenum++;
                }
            }
        }
    }
    for(int x=0;x<9;x = x+3){               //在一个九宫格中不能重复
        for(int y=0;y<9;y = y+3){
            for(int z=1;z<=9;z++){
                int a[9];
                int temp=0;
                for(int m=x;m<(x+3);m++){
                    for(int n=y;n<(y+3);n++){
                        a[temp] = chessvalue(m, n, z);
                        temp++;
                    }
                }
                for(temp = 0;temp<8;temp++){
                    for(int temp2 = temp+1;temp2<9;temp2++){
                        fprintf(fp, "%d %d 0\n",-a[temp],-a[temp2]);
                        chessclausenum++;
                    }
                }
            }
        }
    }
}

clock_t Time(clock_t time1,clock_t time2){
    clock_t timeused;
    timeused = (unsigned)(time2 - time1) / CLOCKS_PER_SEC *1000;
    printf("用时:%lums\n",timeused);
    return timeused;
}
