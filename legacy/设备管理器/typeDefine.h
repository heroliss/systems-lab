#ifndef TYPEDEFINE_H
#define TYPEDEFINE_H
#include <queue>
#include <string>
using namespace std;
enum PROPETRY{ReadOnly,WriteOnly,ReadAndWrite,Forbidden};
enum HEALTH{Normal,Fault};
enum STATUS{Free,Busy};
typedef struct dct_struct DCT;
typedef struct //定义进程控制块PCB
{
    string name;     //外部标识符
	DCT *dctPoint;    //占用的设备控制表指针
	bool holdingDevice;//是否正在占用设备

}PCB;

typedef struct //(channel control table,通道控制表) 
{
	string name;
	//int ID;	
	STATUS status;//状态（空闲，忙碌）
	//HEALTH health;//健康状况（正常，故障）	
	queue<PCB*> *waitingQueuePoint;//等待队列指针
}CHCT;

typedef struct  //(controller control table,控制器控制表)
{
	string name;
	//int ID;//内部标示符
	STATUS status;//状态（空闲，忙碌）
	//HEALTH health;//健康状况（正常，故障）
	queue<PCB*> *waitingQueuePoint;//等待队列指针
	CHCT* point_CHCT;//通道指针
}COCT;

typedef struct dct_struct//(device control table.设备控制表)
{
	//int ID_logical;//逻辑设备号
	//int ID;//物理设备号
	//PROPETRY propetry;//属性（读写性）
	STATUS status;//状态（空闲，忙碌）
	//HEALTH health;//健康状况（正常，故障）
	queue<PCB*> *waitingQueuePoint;//等待队列指针
	COCT* point_COCT;//对应的通道指针
}DCT;

typedef struct //(system device table,系统设备表)
{
	string name;//外部标示符
	//int ID;     //内部标志符
	string type;   //设备类型
	DCT* point_DCT; //对应的设备控制表指针
	PCB* point_PCB; //正在使用的设备进程标识（指针）
}SDT;
#endif // TYPEDEFINE_H