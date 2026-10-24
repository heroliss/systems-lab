#ifndef OS_DPMM_H
#define OS_DPMM_H

#include "typedefine.h"
#include <vector>
#include <algorithm>
#include <map>
#include "iostream"
#include <stdlib.h>
#include <time.h>
using namespace std;
class OS_DPMM //Demand paging memory management 请求分页存储管理 
{
public:
	OS_DPMM(int blocks_size,vector<int> pages_sequence,int memorySize)
	{setMemory(memorySize);blocks_sequence=NULL;init(blocks_size,pages_sequence);}//参数不能为负数
	~OS_DPMM();
	init(int blocks_size,vector<int> pages_sequence);
	vector<int> getPagesSequence(){return pages_sequence;} //显示页号序列
	vector<int> getBlocks(){return blocks;}//显示页框
	map<int,PTE> getPTE(){return pte;}//显示页表
	int getCount(){return count;}
	int getBlocksSize(){return blocks_size;}
	void auto_FIFO();
	void auto_LRU();
	void auto_OPT();
	int** blocks_sequence;//页框序列
	void setMemory(int memorySize);
	bool *getBitMap(){return blocks_bitmap;}//获取位示图
	int getMemorySize(){return memorySize;}

//private:
	int count;
	map<int,PTE> pte;
	bool *blocks_bitmap;//物理块位示图
	bool *blocks_bitmap_back;//位示图备份
	vector<int> blocks; //物理块(页框)
	
	//int *blocks_virtual;//虚拟块
	int blocks_size;
	vector<int> pages_sequence;//页号序列


	//int search(int n);//在页框中查找页号，如果存在返回序号，否则返回-1
	void clear();

	bool FIFO(int n);//先进先出
	bool LRU(int n); //最近最久未使用
	bool OPT(int j); //最佳置换算法(注意：参数与上俩个有所不同，这个是页号序列号，而不是页号)
	int memorySize;
	int position; //记录位示图地址
	int searchPosition();
};
int OS_DPMM::searchPosition()
{
	for (;position<memorySize;position++)
	{
		if(blocks_bitmap[position]==0)
		{
			blocks_bitmap[position]=1;
			return position;
		}
	}

}
void OS_DPMM::setMemory(int memorySize)
{
	int i;
	this->memorySize=memorySize;
	srand(time(NULL));
	blocks_bitmap=new bool[memorySize];
	for (i=0;i<memorySize;i++)
	{
		blocks_bitmap[i]=rand()%2;
	}
	blocks_bitmap_back=new bool[memorySize];
	for (i=0;i<memorySize;i++)    
	{
		blocks_bitmap_back[i]=blocks_bitmap[i];
	}
}
void OS_DPMM::clear()
{
	blocks.clear();
	count=0;
	for (int i=0;i<pages_sequence.size();i++)
	{
		int n=pages_sequence.at(i);
		pte[n].bit_access=0;
		pte[n].bit_modified=false;
		pte[n].bit_status=false;
		pte[n].external_memory_address=0;
		pte[n].pbn=0;
	}
}
OS_DPMM::init(int blocks_size,vector<int> pages_sequence)
{
	int i;
//	for (i=0;i<this->pages_sequence.size();i++)
//	{
//		for (int j=0;j<this->blocks_size;j++)
//		{
//			delete (blocks_sequence[i]+j);
//		}
//			delete (blocks_sequence+i);
//	}
	if (blocks_sequence!=NULL)
	delete (blocks_sequence);//释放以前的空间

	blocks_sequence=new int*[pages_sequence.size()];
	for (i=0;i<pages_sequence.size();i++)
	{
		blocks_sequence[i]=new int[blocks_size+1];
		for (int j=0;j<blocks_size+1;j++)
		{
			blocks_sequence[i][j]=-1; //-1表示无内容
		}
	} //初始化块序列表

	this->blocks_size=blocks_size;
	this->pages_sequence.clear();
	this->pages_sequence=pages_sequence;
	pte.clear();
	clear();

}
void OS_DPMM::auto_FIFO()
{
	int i,j;
	for (i=0;i<memorySize;i++)
	{
		blocks_bitmap[i]=blocks_bitmap_back[i];
	}
	position=0;

	for (i=0;i<pages_sequence.size();i++)
	{
		if(FIFO(pages_sequence.at(i)))
		{
			count++; //记录一次缺页
			blocks_sequence[i][blocks_size]=1;//缺页标记
		}
		for (j=0;j<blocks.size();j++)
		{
			blocks_sequence[i][j] = blocks.at(j);
		}

	}	

}
void OS_DPMM::auto_LRU()
{
	int i,j;
	for (i=0;i<memorySize;i++)
	{
		blocks_bitmap[i]=blocks_bitmap_back[i];
	}
	position=0;

	for (i=0;i<pages_sequence.size();i++)
	{
		if(LRU(pages_sequence.at(i)))
		{
			count++; //记录一次缺页
			blocks_sequence[i][blocks_size]=1;//缺页标记
		}
		for (j=0;j<blocks.size();j++)
		{
			blocks_sequence[i][j] = blocks.at(j);
		}
	}

}
void OS_DPMM::auto_OPT()
{
	int i,j;
	for (i=0;i<memorySize;i++)
	{
		blocks_bitmap[i]=blocks_bitmap_back[i];
	}
	position=0;

	for (i=0;i<pages_sequence.size();i++)
	{
		if(OPT(i))
		{
			count++; //记录一次缺页
			blocks_sequence[i][blocks_size]=1;//缺页标记
		}
		for (j=0;j<blocks.size();j++)
		{
			blocks_sequence[i][j] = blocks.at(j);
		}
	}

}
bool OS_DPMM::FIFO(int n) //返回true表示发生一次缺页
{
	if(pte[n].bit_status==true)
		return false;
	else
	{
		if(blocks.size()<blocks_size)
		{
			blocks.push_back(n);
			pte[n].bit_status=true;
			pte[n].pbn=searchPosition();
		}

		else
		{
			pte[blocks.front()].bit_status=false;
			pte[n].pbn=pte[blocks.front()].pbn;
			blocks.erase(blocks.begin());
			blocks.push_back(n);	
			pte[n].bit_status=true;
		}
			return true;
	}				
}

bool OS_DPMM::LRU(int n)
{
	if(pte[n].bit_status==true)
	{
		blocks.erase(find(blocks.begin(),blocks.end(),n));
		blocks.push_back(n);
		return false;
	}
	else
	{
		if(blocks.size()<blocks_size)
		{
			blocks.push_back(n);
			pte[n].bit_status=true;
			pte[n].pbn=searchPosition();
		}
		else
		{
			pte[blocks.front()].bit_status=false;
			pte[n].pbn=pte[blocks.front()].pbn;
			blocks.erase(blocks.begin());
			blocks.push_back(n);	
			pte[n].bit_status=true;
			
		}
		return true;
	}			
}
bool OS_DPMM::OPT(int j)
{
	int tn;//临时存放要在页框内删除的页号
	int n = pages_sequence.at(j);
	if(pte[n].bit_status==true)
		return false;
	else
	{
		if(blocks.size()<blocks_size)
		{
			blocks.push_back(n);
			pte[n].bit_status=true;
			pte[n].pbn=searchPosition();
		}
		else
		{
			int index;
			int t=j;
			
			for (int i=0;i<blocks.size();i++)
			{
				int bn=blocks.at(i);
				pages_sequence.push_back(bn);//设置哨兵
				for (index=j+1;index<pages_sequence.size();index++)
				{
					if(pages_sequence.at(index)==bn)
					{
						if (index>t)
						{	
							t=index;
							tn=pages_sequence.at(t);
						}
						break;
					}
				}
				
				pages_sequence.pop_back();
			}
					
			pte[n].pbn=pte[tn].pbn;
			blocks.erase(find(blocks.begin(),blocks.end(),tn));
			pte[tn].bit_status=false;
			blocks.push_back(n);	
			pte[n].bit_status=true;
			
		}
		return true;
	}			
}

#endif