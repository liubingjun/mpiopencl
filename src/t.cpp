/*************************************************************************
	> File Name: t.cpp
	> Author: LiuBingjun
	> Mail: 278732708@qq.com 
	> Created Time: 2014年05月14日 星期三 16时26分37秒
 ************************************************************************/

#include<iostream>
#include"config.h"
using namespace std;
int main()
   {
              cout<<"ss";
              Config c;
              int** p=c.init();
              cout<<p[1][20];
              return 0;
          }
