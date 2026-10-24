#include "iostream"
#include "math.h"
#include "time.h"
using namespace std;

void show(int *p,int count)
{
	int n=0;
	while (n<count)
	{
		for (int i=0;(i<10)&&(n<count);i++)
		{
			cout<<p[n]<<"\t";
			n++;
		}
		cout<<endl;
	}
}

void main()
{
	clock_t t1,t2;
	int prime[200000];
	prime[0]=2;
	prime[1]=3;

	
	
	int n/*当前的数*/,count_need,count;/*找出的质数数量*/
	n=5; count=2;
	cin>>count_need;

	t1=clock();
	int i;
	while(count<count_need) 
	{
		for (i=0;;i++) //遍历所有小于根号n的质数
		{
			if(0==n%prime[i]) //不是质数
			{
				i=-1;
				break;
			}
			if((int)sqrt(n)<=prime[i]) //结束试除
			{
				count++;
				prime[count-1]=n;
				break;
			}
		}
		n++;
	}

	t2=clock();

	cout<<"用时："<<t2-t1<<"ms"<<endl;
	cout<<"共 "<<count<<" 个质数, 最大质数为 "<<n-1<<endl;
	system("pause");
	show(prime,count);
}