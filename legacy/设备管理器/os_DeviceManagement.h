#ifndef OS_DEVICEMANGEMENT_H
#define OS_DEVICEMANGEMENT_H
#include "typedefine.h"
#include <map>
using namespace std;

class OS_DeviceMangement
{
public:
	OS_DeviceMangement();
	~OS_DeviceMangement();
	bool addDevice(string name,string type,string co,string ch); //添加设备
	bool delDevice(string name); //删除设备
	bool applyDevice(PCB* process,string name);//进程申请设备
	bool applyDeviceOnType(PCB* process,string type);//进程按照类型申请设备
	bool releaseDevice(PCB* process,string name);//进程解除占用设备
	
	map<string,SDT> sdt;//系统设备表
private:
	
	map<string,COCT*> scot;//系统控制器表
	map<string,CHCT*> scht;//系统通道表
	map<string,PCB*> pcbt;//pcb表
	
	string ch1;
	string ch2;
	
	string findDeviceOnType(string type);//根据设备类型寻找最适设备算法
};

OS_DeviceMangement::OS_DeviceMangement()
{
	ch1="ch1";
	ch2="ch2";
	/*2个通道初始化*/
	scht[ch1]=new CHCT;
	scht[ch2]=new CHCT;
	scht[ch1]->name="ch1";
	scht[ch2]->name="ch2";
	scht[ch1]->status=Free;
	scht[ch2]->status=Free;
	scht[ch1]->waitingQueuePoint=new queue<PCB*>;
	scht[ch2]->waitingQueuePoint=new queue<PCB*>;
	
	/*添加4个设备和3个控制器*/

	addDevice("打印","打印设备","打印机控制器",ch2);
	addDevice("显示","显示设备","显示器适配器",ch2);
	addDevice("键盘","基本输入设备","基本输入控制器",ch1);
	addDevice("鼠标","基本输入设备","基本输入控制器",ch1);
}
OS_DeviceMangement::~OS_DeviceMangement()//（队列还没有释放）
{
	map<string,CHCT*>::iterator it1;
	for (it1=scht.begin();it1!=scht.end();it1++)
	{
		delete it1->second;
	}
	map<string,COCT*>::iterator it2;
	for (it2=scot.begin();it2!=scot.end();it2++)
	{
		delete it2->second;
	}
}

bool OS_DeviceMangement::addDevice(string name,string type,string co,string ch)
{
	map<string,SDT>::iterator it_sdt;
	map<string,COCT*>::iterator it_scot;
	map<string,CHCT*>::iterator it_scht;
	
	if (sdt.end()!=(it_sdt=sdt.find(name)))//如果找到设备名
	{
		return false;
	}
	else
	{
		it_scht=scht.find(ch);
		if (scht.end()==it_scht)//如果通道不存在
		{
			return false;
		}
		it_scot=scot.find(co);
		if (scot.end()==it_scot)//如果控制器不存在
		{
				scot[co]=new COCT;//添加控制器至控制器列表
				scot[co]->point_CHCT=it_scht->second;//为控制器链接至选定的通道
				scot[co]->status=Free;
				scot[co]->waitingQueuePoint=new queue<PCB*>;
				scot[co]->name=co;
				it_scot=scot.find(co);
		}
		
		//一下是控制器存在且通道存在的情况
		if (it_scot->second->point_CHCT->name!=it_scht->second->name) //已有控制器链接的通道与当前通道不同
		{
			return false;
		}
		sdt[name].name=name;
		sdt[name].type=type; 
		sdt[name].point_PCB=NULL;
		sdt[name].point_DCT=new DCT;
		sdt[name].point_DCT->point_COCT=it_scot->second;
		sdt[name].point_DCT->status=Free;
		sdt[name].point_DCT->waitingQueuePoint=new queue<PCB*>;
		return true;
	}
	return false;
}

bool OS_DeviceMangement::delDevice(string name)
{
	map<string,SDT>::iterator it;
	it=sdt.find(name);
	if (it==sdt.end())//没有找到设备
	{
		return false;
	}
	else
	{
		COCT *co=it->second.point_DCT->point_COCT;
		if(co->waitingQueuePoint->size()==1)
		{
			scot.erase(scot.find(co->name)->first);
			delete co->waitingQueuePoint;
			delete co;
		}
		delete it->second.point_DCT->waitingQueuePoint;
		delete it->second.point_DCT;
		sdt.erase(it);
		return true;
	}
	return false;
}

bool OS_DeviceMangement::applyDeviceOnType(PCB* process,string type)
{
	map<string,SDT>::iterator it;
	bool exist=false;
	for (it=sdt.begin();it!=sdt.end();it++)
	{
		if (it->second.type==type)
		{
			exist=true;
		}
	}
	if (exist==false)
	{
		return false;
	}
	else
	{
		string name;//设备名
		name=findDeviceOnType(type);
		return applyDevice(process,name);
	}
	return false;
}

bool OS_DeviceMangement::applyDevice(PCB* process,string name)
{
	map<string,SDT>::iterator it;
	it=sdt.find(name);
	if (it==sdt.end())
	{
		return false;
	}
	if(it->second.point_DCT->status==Busy)//设备被占用
	{
		it->second.point_DCT->waitingQueuePoint->push(process);
		process->dctPoint=it->second.point_DCT;
		return true;
	}
	
	it->second.point_DCT->status=Busy;
	it->second.point_DCT->waitingQueuePoint->push(process);
	process->dctPoint=it->second.point_DCT;

	if(it->second.point_DCT->point_COCT->status==Busy)//控制器被占用
	{
		it->second.point_DCT->point_COCT->waitingQueuePoint->push(process);
		return true;
	}
	
	it->second.point_DCT->point_COCT->status=Busy;
	it->second.point_DCT->point_COCT->waitingQueuePoint->push(process);
	if (it->second.point_DCT->point_COCT->point_CHCT->status==Busy)//通道被占用
	{
		it->second.point_DCT->point_COCT->point_CHCT->waitingQueuePoint->push(process);
		return true;
	}
				
	it->second.point_DCT->point_COCT->point_CHCT->status=Busy;
	it->second.point_DCT->point_COCT->point_CHCT->waitingQueuePoint->push(process);
	process->holdingDevice=true;
	return true;
				
}

bool OS_DeviceMangement::releaseDevice(PCB* process,string name)
{
	if (process->holdingDevice==false)
	{
		return false;
	}

	DCT *device=process->dctPoint;
	COCT *co=device->point_COCT;
	CHCT *ch=co->point_CHCT;
	ch->waitingQueuePoint->pop();
	
	if (ch->waitingQueuePoint->empty())
	{
		ch->status=Free;
	}
	else
	{
		ch->waitingQueuePoint->front()->holdingDevice=true;
	}
	co->waitingQueuePoint->pop();

	if (co->waitingQueuePoint->empty())
	{
		co->status=Free;
	}
	else
	{
		if(ch->status==Free)
		{
			ch->waitingQueuePoint->push(co->waitingQueuePoint->front());
			process->holdingDevice=true;
			ch->status=Busy;
		}
	}
	
	device->waitingQueuePoint->pop();
	if (device->waitingQueuePoint->empty())
	{
		device->status=Free;
		return true;
	}
	else
	{
		if (co->status==Free)
		{
			co->waitingQueuePoint->push(device->waitingQueuePoint->front());
			co->status=Busy;
		}
		if(ch->status==Free)
		{
			ch->waitingQueuePoint->push(co->waitingQueuePoint->front());
			process->holdingDevice=true;
			ch->status=Busy;
		}
		return true;
	}

	return false;

}
string OS_DeviceMangement::findDeviceOnType(string type)
{
	map<string,SDT>::iterator it;
    for(it=sdt.begin();it!=sdt.end();it++)
    {
        if(it->second.type==type)
            return it->second.name;
    }
    string s="";
    return s;
}

#endif 