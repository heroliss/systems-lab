#pragma once
#include <vector>
#include <string>
#include <cstdint>
namespace systems {
struct PageStep { int page, victim; bool hit; std::vector<int> frames; };
struct PageRun { std::string algorithm; int faults=0; std::vector<PageStep> steps; };
PageRun paging(const std::vector<int>& pages, int capacity, int algorithm);
struct BankResult { bool safe=false, granted=false; std::string status; std::vector<int> sequence, available, allocation, need; };
BankResult banker(int processes,int resources,const std::vector<int>& maximum,const std::vector<int>& allocation,const std::vector<int>& available,int pid=-1,const std::vector<int>& request={});
struct DeviceRequest { int process, device; };
struct DeviceResult { std::vector<bool> granted; std::vector<int> owner; };
DeviceResult devices(const std::vector<DeviceRequest>& requests);
struct PrimeResult { std::uint64_t count=0; int last=0, processed=1; bool canceled=false; double elapsed=0; std::vector<int> first, bins; };
using Cancel = int(__cdecl*)();
PrimeResult primes(int limit,int method,Cancel canceled=nullptr);
}
#ifdef SYSTEMS_EXPORTS
#define API extern "C" __declspec(dllexport)
API const char* __cdecl sl_paging(const int*,int,int);
API const char* __cdecl sl_banker(int,int,const int*,const int*,const int*,int,const int*);
API const char* __cdecl sl_devices(const int*,const int*,int);
API const char* __cdecl sl_primes(int,int,systems::Cancel);
#endif
