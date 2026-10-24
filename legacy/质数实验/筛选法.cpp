#include <iostream>
#include <time.h>
#include <math.h>
using namespace std;
/*100000000
5761455time:3269
请按任意键继续. . .*/
void FilterPrime(int n)
{
	int count=0;
	bool* isPrimes = new bool[n+1];
	for(int i=2;i<=n;i++)
	{
		isPrimes[i] = true;
	}
	for(int j=2;j<=sqrt(n);++j)
	{
		if(isPrimes[j]==true)
		{
			for(int m=2;j*m<=n;++m)
			{
				isPrimes[j*m] = false;
			}
		}
	}
/*	for(int k=2;k<=n;++k)
	{
		if(isPrimes[k]==true)
		{
			count++;
		}
	}
	cout<<count;*/
	delete [] isPrimes;
}
int main()
{
	int num;
	cin>>num;
	clock_t t1,t2;
	t1=clock();
	FilterPrime(num);
	t2=clock();
	cout<<"time:"<<t2-t1<<endl;
	system("pause");
	return 0;
}