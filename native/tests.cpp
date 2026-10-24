#include "core.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <functional>
int checks=0;
void check(bool condition,const char* message){checks++;if(!condition)throw std::runtime_error(message);}
void rejects(std::function<void()> f){bool failed=false;try{f();}catch(const std::invalid_argument&){failed=true;}check(failed,"Expected input rejection");}
int __cdecl cancel(){return 1;}
int main(){try {
 std::vector<int> seq{7,0,1,2,0,3,0,4,2,3,0,3,2};
 check(systems::paging(seq,3,0).faults==10,"FIFO known reference");check(systems::paging(seq,3,1).faults==9,"LRU known reference");check(systems::paging(seq,3,2).faults==7,"OPT known reference");
 check(systems::paging({1,1,1},1,0).faults==1,"Repeated page");check(systems::paging({0},1,2).steps[0].frames[0]==0,"Zero valid page");
 std::mt19937 rng(42);for(int k=0;k<60;k++){std::vector<int> pages(30);for(int&v:pages)v=rng()%9;int cap=1+rng()%8;auto opt=systems::paging(pages,cap,2);for(int a=0;a<2;a++){auto r=systems::paging(pages,cap,a);check(opt.faults<=r.faults,"OPT lower bound");for(auto&s:r.steps)check(std::count(s.frames.begin(),s.frames.end(),s.page)==1,"Page present once");}}
 rejects([]{systems::paging({},3,0);});rejects([]{systems::paging({-1},3,0);});rejects([]{systems::paging({1},0,0);});
 std::vector<int> mx{7,5,3,3,2,2,9,0,2,2,2,2,4,3,3}, al{0,1,0,2,0,0,3,0,2,2,1,1,0,0,2},av{3,3,2};
 auto safe=systems::banker(5,3,mx,al,av);check(safe.safe&&safe.sequence.size()==5,"Bank safe sequence");check(av==std::vector<int>({3,3,2}),"Input immutable");
 auto granted=systems::banker(5,3,mx,al,av,1,{1,0,2});check(granted.granted&&granted.available==std::vector<int>({2,3,0}),"Safe request");
 auto unsafe=systems::banker(5,3,mx,al,av,4,{3,3,0});check(!unsafe.granted&&unsafe.status=="UNSAFE"&&unsafe.allocation==al&&unsafe.available==av,"Unsafe rollback");
 check(unsafe.sequence==safe.sequence,"Rejected request sequence matches retained state");
 check(systems::banker(5,3,mx,al,av,0,{8,0,0}).status=="EXCEEDS_NEED","Exceeded need");check(systems::banker(5,3,mx,al,av,0,{4,0,0}).status=="INSUFFICIENT","Unavailable request");
 rejects([&]{systems::banker(5,3,mx,al,av,5,{0,0,0});});auto bad=al;bad[0]=100;rejects([&]{systems::banker(5,3,mx,bad,av);});
 auto d=systems::devices({{1,0},{2,1},{3,2},{4,3}});check(d.granted==std::vector<bool>({true,false,true,false}),"Hierarchical exclusion");
 auto after=systems::devices({{2,1},{4,3}});check(after.granted==std::vector<bool>({true,true}),"Release advances queue");rejects([]{systems::devices({{1,0},{1,2}});});rejects([]{systems::devices({{1,4}});});
 for(int n: {0,1,2,3,10,100,1000,100000}){auto a=systems::primes(n,0),b=systems::primes(n,1);check(a.count==b.count&&a.last==b.last&&a.first==b.first,"Prime algorithms agree");}
 auto million=systems::primes(1000000,0);check(million.count==78498&&million.last==999983,"Million pi(n)");check(systems::primes(1000000,0,cancel).canceled,"Sieve cancel");check(systems::primes(1000000,1,cancel).canceled,"Trial cancel");rejects([]{systems::primes(-1,0);});
 check(systems::primes(100,1,cancel).canceled&&systems::primes(100,1,cancel).count==0,"Already canceled small trial");auto hundredMillion=systems::primes(100000000,0);check(hundredMillion.count==5761455&&hundredMillion.last==99999989,"Hundred million upper boundary");
 std::cout<<"{\"passed\":true,\"assertions\":"<<checks<<"}\n";return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<" at assertion "<<checks<<'\n';return 1;}}
