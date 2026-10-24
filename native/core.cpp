#include "core.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <sstream>
#include <chrono>
#include <cmath>
#include <limits>
namespace systems {
PageRun paging(const std::vector<int>& pages,int capacity,int algorithm) {
 if(capacity<1||capacity>32||pages.empty()||pages.size()>4096||algorithm<0||algorithm>2) throw std::invalid_argument("Invalid paging configuration");
 if(std::any_of(pages.begin(),pages.end(),[](int n){return n<0||n>1000000;})) throw std::invalid_argument("Page numbers must be between 0 and 1000000");
 PageRun r; r.algorithm=std::vector<std::string>{"FIFO","LRU","OPT"}[algorithm];
 std::vector<int> frames(capacity,-1), loaded(capacity,-1), used(capacity,-1);
 std::unordered_map<int,std::vector<int>> occurrences;
 for(int i=0;i<(int)pages.size();i++) occurrences[pages[i]].push_back(i);
 for(int i=0;i<(int)pages.size();i++) {
  int page=pages[i], victim=-1, slot=(int)(std::find(frames.begin(),frames.end(),page)-frames.begin()); bool hit=slot<capacity;
  if(!hit) {
   r.faults++; slot=(int)(std::find(frames.begin(),frames.end(),-1)-frames.begin());
   if(slot==capacity) {
    slot=0;
    if(algorithm==0) slot=(int)(std::min_element(loaded.begin(),loaded.end())-loaded.begin());
    if(algorithm==1) slot=(int)(std::min_element(used.begin(),used.end())-used.begin());
    if(algorithm==2) { int farthest=-1; for(int k=0;k<capacity;k++) { const auto& seq=occurrences.at(frames[k]); auto it=std::upper_bound(seq.begin(),seq.end(),i); int next=it==seq.end()?std::numeric_limits<int>::max():*it; if(next>farthest) {farthest=next;slot=k;} } }
    victim=frames[slot];
   }
   frames[slot]=page; loaded[slot]=i;
  }
  used[slot]=i; r.steps.push_back({page,victim,hit,frames});
 }
 return r;
}
BankResult banker(int n,int m,const std::vector<int>& maximum,const std::vector<int>& allocation,const std::vector<int>& available,int pid,const std::vector<int>& request) {
 if(n<1||n>50||m<1||m>10||maximum.size()!=n*m||allocation.size()!=n*m||available.size()!=m||pid< -1||pid>=n) throw std::invalid_argument("Invalid matrix dimensions");
 auto valid=[](int v){return v>=0&&v<=1000000;};
 if(!std::all_of(maximum.begin(),maximum.end(),valid)||!std::all_of(allocation.begin(),allocation.end(),valid)||!std::all_of(available.begin(),available.end(),valid)) throw std::invalid_argument("Resource values must be between 0 and 1000000");
 BankResult r; r.available=available;r.allocation=allocation; r.need.resize(n*m);
 for(int i=0;i<n*m;i++){if(allocation[i]>maximum[i])throw std::invalid_argument("Allocation exceeds maximum");r.need[i]=maximum[i]-allocation[i];}
 if(pid>=0) {
  if(request.size()!=m||!std::all_of(request.begin(),request.end(),valid)) throw std::invalid_argument("Invalid request vector");
  for(int j=0;j<m;j++) { if(request[j]>r.need[pid*m+j]) {r.status="EXCEEDS_NEED";r.sequence=banker(n,m,maximum,allocation,available).sequence;return r;} if(request[j]>r.available[j]) {r.status="INSUFFICIENT";r.sequence=banker(n,m,maximum,allocation,available).sequence;return r;} }
  for(int j=0;j<m;j++) {r.available[j]-=request[j];r.allocation[pid*m+j]+=request[j];r.need[pid*m+j]-=request[j];}
 }
 std::vector<int> work=r.available; std::vector<bool> done(n,false);
 while((int)r.sequence.size()<n) { bool progress=false; for(int i=0;i<n;i++) if(!done[i]) {bool fits=true;for(int j=0;j<m;j++)if(r.need[i*m+j]>work[j])fits=false; if(fits){for(int j=0;j<m;j++)work[j]+=r.allocation[i*m+j];done[i]=true;r.sequence.push_back(i);progress=true;}} if(!progress)break; }
 r.safe=(int)r.sequence.size()==n; r.granted=pid>=0&&r.safe; r.status=r.safe?(pid<0?"SAFE":"GRANTED"):"UNSAFE";
 if(pid>=0&&!r.safe) {r.available=available;r.allocation=allocation;for(int i=0;i<n*m;i++)r.need[i]=maximum[i]-allocation[i];r.sequence=banker(n,m,maximum,allocation,available).sequence;}
 return r;
}
DeviceResult devices(const std::vector<DeviceRequest>& requests) {
 if(requests.size()>1000)throw std::invalid_argument("Too many requests");
 // Device 0/1 share controller 0 and channel 0. Device 2/3 have separate controllers, sharing channel 1.
 const int controllers[]{0,0,1,2}, channels[]{0,0,1,1};
 std::unordered_set<int> pids; std::vector<int> deviceOwner(4,-1), controllerOwner(3,-1), channelOwner(2,-1);
 DeviceResult r; r.owner.assign(4,-1);
 for(const auto& q:requests) {
  if(q.process<0||q.device<0||q.device>=4||!pids.insert(q.process).second)throw std::invalid_argument("Invalid or duplicate process request");
  int co=controllers[q.device],ch=channels[q.device]; bool grant=deviceOwner[q.device]<0&&controllerOwner[co]<0&&channelOwner[ch]<0;
  if(grant){deviceOwner[q.device]=q.process;controllerOwner[co]=q.process;channelOwner[ch]=q.process;r.owner[q.device]=q.process;}
  r.granted.push_back(grant);
 }
 return r;
}
PrimeResult primes(int limit,int method,Cancel cancel) {
 if(limit<0||limit>100000000||method<0||method>1)throw std::invalid_argument("Limit must be between 0 and 100000000");
 PrimeResult r; r.bins.assign(100,0);auto start=std::chrono::steady_clock::now();
 auto add=[&](int p){r.count++;r.last=p;if(r.first.size()<1000)r.first.push_back(p);int bin=std::min(99,(int)((std::int64_t)p*100/std::max(1,limit+1)));r.bins[bin]++;};
 auto stop=[&](){if(cancel&&cancel()){r.canceled=true;return true;}return false;};
 if(stop()){r.processed=std::min(1,limit);r.elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();return r;}
 if(method==0) {
  const int root=(int)std::sqrt(limit); std::vector<bool> base(root+1,true);std::vector<int> small;
  for(int p=2;p<=root;p++)if(base[p]){small.push_back(p);if(p<=root/p)for(int j=p*p;j<=root;j+=p)base[j]=false;}
  constexpr int segment=65536;
  for(int low=2;low<=limit;low+=segment) {if(stop())break;int high=std::min(limit,low+segment-1);std::vector<bool> mark(high-low+1,true);
   for(int p:small){std::int64_t begin=std::max((std::int64_t)p*p,((std::int64_t)low+p-1)/p*p);for(auto j=begin;j<=high;j+=p)mark[(size_t)(j-low)]=false;}
   for(int i=low;i<=high;i++)if(mark[i-low])add(i);r.processed=high;
  }
 } else {
  std::vector<int> discovered;
  for(int p=2;p<=limit;p+=(p==2?1:2)) {if((p&2047)==1&&stop())break;bool prime=true;for(int q:discovered){if(q>p/q)break;if(p%q==0){prime=false;break;}}if(prime){discovered.push_back(p);add(p);}r.processed=p;}
  if(!r.canceled)r.processed=limit;
 }
 if(limit<2)r.processed=limit; r.elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();return r;
}
}
#ifdef SYSTEMS_EXPORTS
namespace { thread_local std::string output;
std::string quote(const std::string& s){std::string t="\"";for(char c:s){if(c=='\"'||c=='\\')t+='\\';if((unsigned char)c>=32)t+=c;}return t+'\"';}
template<class T> std::string array(const std::vector<T>& a){std::ostringstream o;o<<'[';for(size_t i=0;i<a.size();i++){if(i)o<<',';o<<a[i];}o<<']';return o.str();}
template<class F> const char* guarded(F f){try{output=f();}catch(const std::exception& e){output="{\"error\":"+quote(e.what())+"}";}catch(...){output="{\"error\":\"Native computation failed\"}";}return output.c_str();}
std::vector<int> copy(const int* p,int n){if(!p||n<0)throw std::invalid_argument("Missing input");return {p,p+n};}
}
API const char* __cdecl sl_paging(const int* p,int n,int cap){return guarded([&]{if(n<1||n>4096)throw std::invalid_argument("Use 1 to 4096 page references");auto pages=copy(p,n);std::ostringstream o;o<<"{\"results\":[";for(int a=0;a<3;a++){if(a)o<<',';auto r=systems::paging(pages,cap,a);o<<"{\"algorithm\":"<<quote(r.algorithm)<<",\"faults\":"<<r.faults<<",\"steps\":[";for(size_t i=0;i<r.steps.size();i++){if(i)o<<',';auto&s=r.steps[i];o<<"{\"page\":"<<s.page<<",\"victim\":"<<s.victim<<",\"hit\":"<<(s.hit?"true":"false")<<",\"frames\":"<<array(s.frames)<<'}';}o<<"]}";}o<<"]}";return o.str();});}
API const char* __cdecl sl_banker(int n,int m,const int* mx,const int* al,const int* av,int pid,const int* req){return guarded([&]{if(n<1||n>50||m<1||m>10)throw std::invalid_argument("Invalid dimensions");auto r=systems::banker(n,m,copy(mx,n*m),copy(al,n*m),copy(av,m),pid,pid<0?std::vector<int>{}:copy(req,m));std::ostringstream o;o<<"{\"safe\":"<<(r.safe?"true":"false")<<",\"granted\":"<<(r.granted?"true":"false")<<",\"status\":"<<quote(r.status)<<",\"sequence\":"<<array(r.sequence)<<",\"available\":"<<array(r.available)<<",\"allocation\":"<<array(r.allocation)<<",\"need\":"<<array(r.need)<<'}';return o.str();});}
API const char* __cdecl sl_devices(const int* pids,const int* devs,int n){return guarded([&]{if(n<0||n>1000)throw std::invalid_argument("Invalid request count");std::vector<systems::DeviceRequest> q;for(int i=0;i<n;i++)q.push_back({pids[i],devs[i]});auto r=systems::devices(q);return "{\"granted\":"+array(r.granted)+",\"owner\":"+array(r.owner)+"}";});}
API const char* __cdecl sl_primes(int limit,int method,systems::Cancel cancel){return guarded([&]{auto r=systems::primes(limit,method,cancel);std::ostringstream o;o<<"{\"count\":"<<r.count<<",\"last\":"<<r.last<<",\"processed\":"<<r.processed<<",\"canceled\":"<<(r.canceled?"true":"false")<<",\"elapsed\":"<<r.elapsed<<",\"first\":"<<array(r.first)<<",\"bins\":"<<array(r.bins)<<'}';return o.str();});}
#endif
