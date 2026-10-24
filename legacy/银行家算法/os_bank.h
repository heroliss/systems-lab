#ifndef OS_BANK_H
#define OS_BANK_H

#include <iostream>  
using namespace std;  
#define MAXPROCESS 50                          /*最大进程数*/  
#define MAXRESOURCE 100                        /*最大资源数*/  

class OS_Bank
{
public:
	OS_Bank(){};
	void init();  
	bool safe();  
	void bank();  
	void showdata(int,int);  
private:
	bool finish[MAXPROCESS];                        /*系统是否有足够的资源分配*/  
	int p[MAXPROCESS];                              /*记录序列*/  
	int m;                                          /*m个进程*/  
	int n;                                          /*n个资源*/
	int available[MAXRESOURCE];                     /*可用资源向量*/  
	int max[MAXPROCESS][MAXRESOURCE];               /*最大需求矩阵*/  
	int allocation[MAXPROCESS][MAXRESOURCE];        /*分配矩阵*/  
	int need[MAXPROCESS][MAXRESOURCE];              /*需求矩阵*/  
	int request[MAXPROCESS][MAXRESOURCE];           /*进程需要资源数*/ 
};
void OS_Bank:: init()                               /*初始化算法*/  
{  
    int i,j;  
	int max_t[MAXPROCESS][MAXRESOURCE] = {
		{7, 5, 3},
		{3, 2, 2},
		{9 ,0, 2},
		{2, 2 ,2},
		{4 ,3, 3}
	};
	int allocation_t[MAXPROCESS][MAXRESOURCE] = {
		{0, 1, 0},
		{2, 0, 0},
		{3, 0, 2},
		{2, 1, 1},
		{0, 0, 2}
	};
    cout<<"请输入进程的总数:";  
    cin>>m;  
    cout<<"请输入资源的种类数:";  
    cin>>n;  
    cout<<"请输入进程所需资源数表，（行：进程，列：资源类型，"<<m<<"x"<<n<<"）"<<endl;  
    for(i=0;i<m;i++)  
        for(j=0;j<n;j++)  
			max[i][j] = max_t[i][j]; //自动赋值
		//cin>>max[i][j];  
		cout<<"请输入每个进程已分配的各资源数,("<<m<<"x"<<n<<")"<<endl;  
		for(i=0;i<m;i++)  
		{  
			for(j=0;j<n;j++)  
			{  
				//cin>>allocation[i][j];  
				allocation[i][j] = allocation_t[i][j];//自动赋值
				need[i][j]=max[i][j]-allocation[i][j];  
				if(need[i][j]<0)  
				{  
					cout<<"您输入的第"<<i+1<<"个进程所拥有的第"<<j+1<<"个资源数 错误,请重新输入:"<<endl;  
					j--;  
					continue;  
				}  
			}  
		}  
		cout<<"请输入每个可用资源总数:"<<endl;  
		for(i=0;i<n;i++)  
		{  
			cin>>available[i];  
		}  
}  
void OS_Bank:: bank()                /*银行家算法*/  
{  
    int i,cusneed,flag = 0;  
    char again;  
    while(1)  
    {  
        showdata(n,m);
        cout<<endl;  
		cout<<"请输入要申请资源的进程号："<<endl;  
		cin>>cusneed;  
		if (cusneed > m)  
		{  
			cout<<"没有该进程，请重新输入"<<endl;
			continue;
		}
		
		cout<<"请输入进程所请求的各资源的数量:"<<endl;  
		for(i=0;i<n;i++)  
		{  
			cin>>request[cusneed][i];  
		}
		bool mark=0;
		for(i=0;i<n;i++)  
		{  
			
			if(request[cusneed][i]>need[cusneed][i])//如果用户选择的线程的第i个资源请求数＞该线程该资源所需的数量  
			{  
				cout<<"您输入的请求数超过进程的需求量!"<<endl<<"******* < 请求被拒绝 > ******* "<<endl;  
				mark=1;
				break;  
			}  
			if(request[cusneed][i]>available[i])//如果用户选择的线程的第i个资源请求数＞系统现有的第i个资源的数量  
			{  
				cout<<"您输入的请求数超过系统有的资源数!"<<endl<<"******* < 请求被拒绝 > ******* "<<endl;  
				mark=1;
				break;  
			}
			
		} 
		if (mark==1)
		{
			continue;
		}  
		
		
		for(i=0;i<n;i++)//如果请求合理，那么下面  
		{  
			available[i]-=request[cusneed][i];//系统可用资源减去申请了的  
			allocation[cusneed][i]+=request[cusneed][i];//线程被分配的资源加上已申请了的  
			need[cusneed][i]-=request[cusneed][i];//线程还需要的资源减去已申请得到的  
		}  
		if(safe())//available  allocation  need变动之后，是否会导致不安全  
		{  
			cout<<"******* < 资源已分配！> *******"<<endl;  
		}  
		else  
		{  
			cout<<"******* < 请求被拒绝 > *******"<<endl;  
			for(i=0;i<n;i++)  
			{  
				available[i]+=request[cusneed][i];  
				allocation[cusneed][i]-=request[cusneed][i];  
				need[cusneed][i]+=request[cusneed][i];  
			}  
		}  
		for (i=0;i<n;i++)  
		{  
			if (need[cusneed][i] <= 0)  
			{  
				flag++;  
			}  
		}  
		if (flag == n)//如果该进程各资源都已满足条件，则释放资源  
		{  
			for (i=0;i<n;i++)  
			{  
				available[i] += allocation[cusneed][i];  
				allocation[cusneed][i] = 0;  
				need[cusneed][i] = 0;  
			}  
			cout<<"线程"<<cusneed<<" 占有的资源被释放！"<<endl;  
			flag = 0;  
		}  
		for(i=0;i<m;i++)//分配好了以后将进程的标识finish改成false  
		{  
			finish[i]=false;  
		}   
        
    }  
}  
bool OS_Bank ::safe()                                    /*安全性算法*/  
{  
	int i,j,k,l=0;  
	int count = 0;
	int work[MAXRESOURCE];                    /*工作数组*/  
	for(i=0;i<n;i++)  
		work[i]=available[i];  
	for(i=0;i<m;i++)  
	{  
		finish[i]=false;//finish记录每个进程是否安全  
	}  
	for(i=0;i<m;i++)  
	{     
		for(j=0;j<n;j++)//循环查找第i个进程需要的各个资源数 是否 超过系统现有的对应的资源数  
		{  
			if(need[i][j]>work[j])//第i个进程需要的第j个资源数 ＞ 系统现有的第j个资源数  
			{  
				break;  
			}  
		}  
		if(j==n && finish[i] != true)//如果第i个进程所需的各个资源数都没有超过系统现有的对应资源数  
		{   
			finish[i]=true;//给该进程的finish标记为true  
			for(k=0;k<n;k++)  
			{  
				work[k]+=allocation[i][k];//将work赋值为 第i个进程各个已分配资源数＋系统现有的对应资源数(因为当改进程全部资源数都满足时线程结束并将资源返还给系统)  
			}  
			p[l++]=i;//记录进程号  
			if (i == m-1) {
				i = -1;
				if (++count == m) {
					cout<<"系统是不安全的"<<endl;  
					return false;  
				}
			}
		}  
		else//如果超过继续循环下一个进程  
		{  
			if (i == m-1) {
				i = -1;
				if (++count == m) {
					cout<<"系统是不安全的"<<endl;  
					return false;  
				}
			}
			else {
				continue;   
			}
		}  
		if(l==m)//当所有进程都能够被满足运行时  
		{  
			cout<<endl<<"系统是安全的"<<endl;  
			cout<<"安全序列: ";  
			for(i=0;i<l;i++)//改了146行的i值，显示资源分配给进程的顺序  
			{  
				cout<<p[i];  
				if(i!=l-1)  
				{  
					cout<<" ";  
				}  
			}  
			cout<<""<<endl;           
			return true;  
		}  
		
	}//for循环  
	cout<<endl<<"系统是不安全的"<<endl;  
	return false;  
}  

void OS_Bank:: showdata(int n,int m)   //显示  
{  
    int i,j;  
    cout<<endl;        
    cout<<"Available向量:"<<endl<<"        ";  
    for   (j=0;j<n;j++)         
        cout<<"     "<<available[j];        
    cout<<endl;     
    cout<<"Need表:"<<endl;   
    for   (i=0;i<m;i++)     
    {  
        cout<<"    "<<i<<" │";     
		
        for   (j=0;j<n;j++)  
            cout<<"     "<<need[i][j];     
        cout<<endl;     
    }     
	
    cout<<endl;     
    cout<<"Allocation表:    "<<endl<<endl;     
	
    for   (i=0;i<m;i++)     
    {  
        cout<<"    "<<i<<" │";     
		
        for   (j=0;j<n;j++)  
            cout<<"     "<<allocation[i][j];  
        cout<<endl;     
    }    
    cout<<endl;   
}  

#endif//OS_BANK_H


