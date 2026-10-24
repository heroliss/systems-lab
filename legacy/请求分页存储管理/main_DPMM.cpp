#include "os_DPMM.h"
#include "iostream"
#include <string>
#include <vector>
#include <stdlib.h>
using namespace std;
OS_DPMM *osDPMM;
vector<int> pages_sequence; //页号序列
int size;
map<int,PTE> pteMap;
vector<int> blocks;
string commond;
int blockSize;
void init()
{
	osDPMM->init(size,pages_sequence);
}
void showBitMap()
{
	if (osDPMM==NULL)
		return;
	cout<<"位示图："<<endl;
	bool *map=osDPMM->getBitMap();
	if(map==NULL)
		return;
	for (int i=0;i<osDPMM->getMemorySize();i++)
	{
		cout<<map[i]<<" ";	
		if(0==(i+1)%10)
			cout<<endl;
	}
}
void showBlocksSequence()
{
	if (osDPMM==NULL)
		return;
	int i;
	for (i=0;i<pages_sequence.size();i++)
	{
		cout<<pages_sequence.at(i)<<"  ";
	}
	cout<<endl;
	for (i=0;i<pages_sequence.size();i++)
	{
		cout<<"---";
	}
	cout<<endl;
	int **table=osDPMM->blocks_sequence;
	int rows=osDPMM->getPagesSequence().size();
	int columns=osDPMM->getBlocksSize()+1;
	
	int column,row;
	for (column=0;column<columns-1;column++)
	{
		for (row=0;row<rows;row++)
		{
			if(table[row][column]==-1)
				cout<<" ";
			else 
				cout<<table[row][column];
			cout<<"  ";
		}
		cout<<endl;
	}
	for (row=0;row<rows;row++)
	{
		if (table[row][column]>0) //缺页
		{
			cout<<"^";
		}
		else
			cout<<" ";
		cout<<"  ";
	}
	cout<<"\t<─ 缺页标记"<<endl;
	cout<<"\t缺页次数："<<osDPMM->getCount()<<" 置换次数："<<osDPMM->getCount()-osDPMM->getBlocksSize()<<" 缺页率："<<(float)osDPMM->getCount()/pages_sequence.size()*100<<"%"<<endl;
}
void showPage()
{	
	if (osDPMM==NULL)
		return;
	int i;
	cout<<"页号序列：";
	pages_sequence=osDPMM->getPagesSequence();
	for (i=0;i<pages_sequence.size();i++)
		cout<<pages_sequence.at(i)<<" ";
	cout<<endl;
}
void showBlock()
{
	if (osDPMM==NULL)
		return;
	int i;
	cout<<"页框号\t页号"<<endl;
	blocks=osDPMM->getBlocks();
	for (i=0;i<blocks.size();i++)
		cout<<i<<"\t"<<blocks.at(i)<<endl;
}
void showPTE()
{
	if (osDPMM==NULL)
		return;
	cout<<"有效页表信息："<<endl;
	cout<<"页号 物理块号  状态位  访问字段"<<endl;
	    //" 1	 2222	   2222		1		1		"
	pteMap= osDPMM->getPTE();
	for (map<int,PTE>::iterator it=pteMap.begin();it!=pteMap.end();it++)
		cout<<" "<<it->first<<"\t "<<it->second.pbn<<"\t "
		
		<<it->second.bit_status<<"\t"
		
		<<(int)it->second.bit_access<<endl;
}
void help()
{
	cout<<"\t\t<帮\t助>"<<endl
		<<"---------------------------------------------------------------------------"<<endl;
	cout<<"设置页框大小并添加页号序列：create      清空页号序列：clear"<<endl
		<<"---------------------------------------------------------------------------"<<endl;
	cout<<"显示页表：    spte  显示页框：sblock    显示页号队列：spage 显示位示图：sbit"<<endl
		<<"---------------------------------------------------------------------------"<<endl;
	cout<<"先进先出算法：fifo  最近最久未使用算法：lru           最佳算法：opt"<<endl
		<<"---------------------------------------------------------------------------"<<endl;
	cout<<"显示命令：    help  清空页号序列：      clear"<<endl
		<<"---------------------------------------------------------------------------"<<endl;
	cout<<"清屏：        cls   退出程序：          quit/exit"<<endl
		<<"---------------------------------------------------------------------------"<<endl;
	cout<<"当前内存块大小："<<blockSize<<"Byte"<<endl;
}
main()
{
	int pagesMaxSize;
	cout<<"\t<欢迎使用置换算法模拟交互系统!>\n\n->此程序在一个页号序列上进行置换算法实验"<<endl;
	cout<<"->程序根据可以根据页号最大值自动调整页表大小"<<endl;
	cout<<"->请用add命令建立页号序列，然后执行置换算法"<<endl<<endl;
	cout<<">>请预定内存块大小(字节)：";
	cin>>blockSize;
	cout<<">>请输入页表最大容量：";
	cin>>pagesMaxSize;
	help();
	osDPMM=NULL;
	while(1)
	{
		
		cout<<">>";
		cin>>commond;
		
		
		if (commond=="create") //增加页号
		{
			cout<<"页框大小：";
			cin>>commond;
			size=atoi(commond.data());
			cout<<"页号序列(用空格或回车隔开，'#'结束,输入'c'自动创建默认序列)："<<endl;
			while (true)
			{
				cin>>commond;
				if (commond=="#"||commond=="c")
					break;
				pages_sequence.push_back(atoi(commond.data()));
			}
			if (commond=="c"&&pages_sequence.empty())
			{
				pages_sequence.push_back(7);pages_sequence.push_back(0);pages_sequence.push_back(1);pages_sequence.push_back(2);
				pages_sequence.push_back(0);pages_sequence.push_back(3);pages_sequence.push_back(0);pages_sequence.push_back(4);
				pages_sequence.push_back(2);pages_sequence.push_back(3);pages_sequence.push_back(0);pages_sequence.push_back(3);
				pages_sequence.push_back(2);pages_sequence.push_back(1);pages_sequence.push_back(2);pages_sequence.push_back(0);
				pages_sequence.push_back(1);pages_sequence.push_back(7);pages_sequence.push_back(0);pages_sequence.push_back(1);
				cout<<"已经自动创建默认序列：70120304230321201701"<<endl;
			}
			//delete(osDPMM);
			osDPMM=new OS_DPMM(size,pages_sequence,100);
		}
		else if (commond=="step") //单步执行
		{
			if (osDPMM==NULL)
			{
				cout<<"请先创建一个实验组再单步执行"<<endl;
				continue;
			}
			cout<<"请选择置换方式(fifo,lru，输入q退出单步模式)：";
			string commond2;
			cin>>commond2;
			if (commond2=="q")
				continue;
			cout<<"<提示：单步执行模式下请输入16位逻辑地址或命令>"<<endl<<endl;
			while (1)
			{
				int num=-1;
				for (int i=0;i<osDPMM-> memorySize;i++)
				{
					osDPMM-> blocks_bitmap[i]=osDPMM-> blocks_bitmap_back[i];
				}
				osDPMM-> position=0;
				cout<<"(单步模式) >>";
				cin>>commond;
				if (commond=="q")
				{
					break;
				}	
				else if(commond=="spage")
				{
					showPage();
					continue;
				}
				else if(commond=="sblock")
				{
					showBlock();
					continue;
				}
				else if (commond=="spte")
				{
					showPTE();
					continue;
				}	
				else if (commond=="help")
				{
					help();
					continue;
				}
				else if (commond=="sbit")
				{
					showBitMap();
					continue;
				}
				else if(commond=="exit"||commond=="quit")
				{
					return;
				}
				else if (commond=="cls")
				{
					system("cls");
					continue;
				}else{
				num=strtol(commond.data(),NULL,16);
				}
				if (num==-1)
				{
					cout<<"您输入的地址或命令有误！"<<endl;
					continue;
				}
				int pageNum=num/blockSize;
				
				cout<<"\t<访问页号为"<<pageNum<<">"<<endl;	
				if (pageNum>pagesMaxSize)
				{
					cout<<"地址越界！"<<endl;
					continue;
				}
				
				if (commond2=="fifo")
				{
					if(osDPMM->FIFO(pageNum))
					{
						osDPMM->count++; //记录一次缺页
						cout<<"发生缺页！"<<endl;
					}
					//showBlocksSequence();
					cout<<"当前页框:";
					for (int i=0;i<osDPMM->getBlocks().size();i++)
					{
						cout<<" "<<osDPMM->getBlocks().at(i)<<" ";
					}
					char c[100];
					string s=itoa(num%blockSize+osDPMM->getPTE()[pageNum].pbn*blockSize,c,16);
					cout<<endl<<"物理地址："<<s<<endl;
				}else if (commond2=="lru")
				{
					if(osDPMM->LRU(pageNum))
					{
						osDPMM->count++; //记录一次缺页
						cout<<"发生缺页！"<<endl;
					}
					//showBlocksSequence();
					cout<<"当前页框:";
					for (int i=0;i<osDPMM->getBlocks().size();i++)
					{
						cout<<"\t"<<osDPMM->getBlocks().at(i)<<" ";
					}
					char c[100];
					string s=itoa(num%blockSize+osDPMM->getPTE()[pageNum].pbn*blockSize,c,16);
					cout<<endl<<"物理地址："<<s<<endl;
				}
				else 
				{cout<<"您输入的置换方式有误!程序将退出单步执行模式"<<endl;break;}
			}	
			
		}
		else if(commond=="spage")
		{
			showPage();
		}
		else if(commond=="sblock")
		{
			showBlock();
			
		}
		else if (commond=="spte")
		{
			showPTE();
		}
		else if (commond=="fifo")
		{
			if (osDPMM==NULL)
				continue;
			init();
			osDPMM->auto_FIFO();
			showBlocksSequence();
		}
		else if (commond=="lru")
		{
			if (osDPMM==NULL)
				continue;
			init();
			osDPMM->auto_LRU();
			showBlocksSequence();
		}
		else if(commond=="opt")
		{
			if (osDPMM==NULL)
				continue;
			init();
			osDPMM->auto_OPT();
			showBlocksSequence();
		}
		else if (commond=="clear")
		{
			pages_sequence.clear();
		}
		else if (commond=="help")
		{
			help();
		}
		else if (commond=="sbit")
		{
			showBitMap();
		}
		else if(commond=="exit"||commond=="quit")
		{
			return;
		}
		else if (commond=="cls")
		{
			system("cls");
		}
		else 
			cout<<"无此命令！"<<endl;
	}
	
}