#ifndef TYPEDEFINE_H
#define  TYPEDEFINE_H
typedef struct 
{

	int pbn;//物理块号 （physical block number）
	bool bit_status; //模拟 状态位
	char bit_access;//模拟 访问字段
	bool bit_modified;//模拟 修改位
	int external_memory_address;//外存地址
}PTE;//页表项 （page table entry）

typedef struct  
{
	bool status;//状态
	int size_PTE;//页表大小
	PTE* address_PTE;//页表首地址
}STE;//段表项（segment table entry）
#endif