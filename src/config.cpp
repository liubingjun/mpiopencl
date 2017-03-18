/*************************************************************************
	> File Name: config.cpp
	> Author: LiuBingjun
	> Mail: 278732708@qq.com 
	> Created Time: 2014年05月14日 星期三 14时53分01秒
 ************************************************************************/
#define INT 1;
#define DOUBLE 2;
#define FLOAT 3;
#define STRING 4;
#include "config.h"
    void  Config::set(){
        Config::num=3;
        Config::parameterType=INT;
        Config::totalLength=30000;
        Config::unitLength=100;
        Config::kernelname="adds";
        Config::kernelfile="adds.cl";
    }
    int**  Config::init(){
        int **p;
        p=new int*[Config::num];
        for(int i=0; i<Config::num;i++)
        {
            p[i] = new int [Config::totalLength];
        }

        for(int i=0;i<Config::totalLength;i++)
        {
            p[0][i]=i;
            p[1][i]=i;
            p[2][i]=i;
        }
        return p;
    }
