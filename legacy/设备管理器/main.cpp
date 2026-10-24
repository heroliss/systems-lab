#pragma warning(disable : 4786)
#include "os_DeviceManagement.h"
#include "iostream"
#include <algorithm>
#include "stdlib.h"
using namespace std;
OS_DeviceMangement *os;
map<string,PCB*> pcbList;
bool cmp(pair<string,string> &s1,pair<string,string> &s2)
{  
    return s1.first < s2.first;
}  
void show()
{
	map<string,map<string,vector<string> > > allMap;//通道，控制器,设备表
	map<string,map<string,vector<string> > >::iterator it2;
	map<string,vector<string> >::iterator it3;
	map<string,SDT>::iterator it;
	for (it=os->sdt.begin();it!=os->sdt.end();it++)
	{
		DCT *device=it->second.point_DCT;
		COCT *co=device->point_COCT;
		CHCT *ch=co->point_CHCT;
		allMap[ch->name][co->name].push_back(it->first);
	}
	for (it2=allMap.begin();it2!=allMap.end();it2++)//通道
	{
		int mark;
		switch(os->sdt[it2->second.begin()->second.at(0)].point_DCT->point_COCT->point_CHCT->status)
		{
		case Free:
			cout<<"空闲\t";
			break;
		case Busy:
			cout<<os->sdt[it2->second.begin()->second.at(0)].point_DCT->point_COCT->point_CHCT->waitingQueuePoint->front()->name<<"\t";
			break;
		}	
		cout<<it2->first<<":";
		queue<PCB*> *q=new queue<PCB*> (*(os->sdt[it2->second.begin()->second.at(0)].point_DCT->point_COCT->point_CHCT->waitingQueuePoint));
		cout<<"\t\t\t(";
		while (!q->empty())
		{
			cout<<q->front()->name<<",";
			q->pop();
		}
		cout<<")"<<endl;
		
		for (it3=it2->second.begin();it3!=it2->second.end();it3++)  //控制器
		{		
			switch(os->sdt[it3->second.at(0)].point_DCT->point_COCT->status)
			{
			case Free:
				cout<<"空闲\t";
				break;
			case Busy:
				cout<<os->sdt[it3->second.at(0)].point_DCT->point_COCT->waitingQueuePoint->front()->name<<"\t";
				break;
			}

			
			if (++it3==it2->second.end())
			{
				it3--;
				cout<<"  └─"<<it3->first;
				mark=0;
			}
			else
			{
				it3--;
				cout<<"  ├─"<<it3->first;
				mark=1;
			}

			queue<PCB*> *q=new queue<PCB*> (*(os->sdt[it3->second.at(0)].point_DCT->point_COCT->waitingQueuePoint));
			cout<<"\t(";
			while (!q->empty())
			{
				cout<<q->front()->name<<",";
				q->pop();
			}
			cout<<")"<<endl;

			for (int i=0;i<it3->second.size();i++)  //设备
			{
				switch(os->sdt[it3->second.at(i)].point_DCT->status)  
				{
				case Free:
					cout<<"空闲\t";
					break;
				case Busy:
					cout<<os->sdt[it3->second.at(i)].point_DCT->waitingQueuePoint->front()->name<<"\t";
					break;
				}
				
				if (mark==1)
				{
					cout<<"  │  ";
				}else
				{
					cout<<"      ";
				}

				if (i==(it3->second.size()-1))
				{
					cout<<"└─"<<it3->second.at(i);
				}
				else
				{
					cout<<"├─"<<it3->second.at(i);
				}

				queue<PCB*> *q=new queue<PCB*> (*(os->sdt[it3->second.at(i)].point_DCT->waitingQueuePoint));
				cout<<"\t\t(";
				while (!q->empty())
				{
					cout<<q->front()->name<<",";
					q->pop();
				}	
				cout<<")"<<"\t<"<< os->sdt[it3->second.at(i)].type <<">"<<endl;

			}
		}
		
	}
	
}

bool applyDevice(string process,string device)
{
	if (pcbList.find(process)!=pcbList.end())
	{
		return false;
	}
	PCB *p=pcbList[process]=new PCB;
	p->name=process;
	p->holdingDevice=NULL;
	return os->applyDevice(p,device);
}
bool applyDeviceOnType(string process,string type)
{
	if (pcbList.find(process)!=pcbList.end())
	{
		return false;
	}
	PCB *p=pcbList[process]=new PCB;
	p->name=process;
	p->holdingDevice=NULL;
	os->applyDeviceOnType(p,type);
}
void help()
{
	cout<<"显示：s 创建：c 删除（解除占用）：d  帮助：h"<<endl;
}
void main()
{
	os=new OS_DeviceMangement;
	string command;
	help();
	while (1)
	{
		cout<<">>";
		cin>>command;

		if (command=="c")
		{
			cout<<"您要创建？进程(p) 设备(d):";
			cin>>command;
			if (command=="p")//创建进程
			{
				cout<<"输入进程名:";
				cin>>command;
				string s=command;
				cout<<"输入需要的设备名(输入“type”按类型申请设备):";
				cin>>command;

				if (command=="type")
				{
					cout<<"输入要申请的设备类型：";
					cin>>command;
					if(applyDeviceOnType(s,command))
						cout<<"<进程申请设备成功！>"<<endl;
					else
						cout<<"<进程申请设备失败！>"<<endl;		
				}
				else
				{					
					if(applyDevice(s,command))
						cout<<"<进程申请设备成功！>"<<endl;
					else
						cout<<"<进程申请设备失败！>"<<endl;
				}
			}
			else if (command=="d")//创建设备
			{
				string s1,s2,s3;
				cout<<"输入要创建的设备名和设备类型：";
				cin>>s1;
				cin>>command;
				cout<<"输入要使用的控制器：";
				cin>>s2;
				cout<<"输入要连接的通道：";
				cin>>s3;
				
				if (os->addDevice(s1,command,s2,s3))
				{
					cout<<"<成功创建设备"<<s1<<"!>"<<endl;
				}
				else
				{
					cout<<"<创建设备失败！>"<<endl;
				}
			}

			show();
		}
		else if (command=="d")//解除
		{
			cout<<"您要？解除进程占用(p) 删除设备(d):";
			cin>>command;
			if (command=="p")
			{
				string s1,s2;
				cout<<"输入要解除占用的进程名和设备名：";
				cin>>s1;
				cin>>s2;
				map<string,PCB*>::iterator it;
				if(pcbList.end()==(it=pcbList.find(s1)))
				{
					cout<<"<没有找到该进程！>"<<endl;
					continue;
				}
				else
				{
					if(os->releaseDevice(it->second,s2))
						cout<<"<解除占用成功！>"<<endl;
					else
						cout<<"<解除占用失败！>"<<endl;
				}
				
			}else if (command=="d")
			{
				cout<<"输入要删除的设备名：";
				cin>>command;
				if (os->delDevice(command))
				{
					cout<<"<删除设备"<<command<<"成功！>"<<endl;
				}
			}
		}
		else if (command=="s") //show
		{
			show();
		}
		else if (command=="cls")
		{
			system("cls");
		}
		else if (command=="h")
		{
			help();
		}
		else
			cout<<"无此命令!"<<endl;
	}

}