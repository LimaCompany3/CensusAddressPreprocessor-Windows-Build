#define UNICODE
#define _UNICODE
#include <windows.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
static const char* Header = "RowID,ZIPCode,StreetName,StreetSoundex,StreetNumericKey,StreetNumericClass,PrefixDirectional,StreetSuffix,SuffixDirectional,HouseNumberType,LowHouseNumber,HighHouseNumber,LowPrimaryNumber,HighPrimaryNumber,LowSecondaryNumber,HighSecondaryNumber,LowAlphaSuffix,HighAlphaSuffix,LowFractionValue,HighFractionValue,DerivedParity,CentroidLatitude,CentroidLongitude,StreetCoreName,StreetTokenPrefixKey,StreetTokenPhoneticKey,StreetTokenSortedPrefixKey,StreetTokenSortedPhoneticKey";

struct Item { std::string line; std::vector<std::string> f; };

static std::vector<std::string> ParseCsv(const std::string& s) {
    std::vector<std::string> r; std::string v; bool q=false;
    for(size_t i=0;i<s.size();++i){char c=s[i]; if(q){if(c=='"'&&i+1<s.size()&&s[i+1]=='"'){v+='"';++i;}else if(c=='"')q=false;else v+=c;}else if(c=='"')q=true;else if(c==','){r.push_back(v);v.clear();}else v+=c;} r.push_back(v); return r;
}
static long long Num(const std::string& s) { if(s.empty()) return -1; try{return std::stoll(s);}catch(...){return -1;} }
static int CmpText(const std::string&a,const std::string&b){size_t n=std::min(a.size(),b.size());for(size_t i=0;i<n;++i){unsigned char x=(unsigned char)std::toupper((unsigned char)a[i]),y=(unsigned char)std::toupper((unsigned char)b[i]);if(x<y)return -1;if(x>y)return 1;}return a.size()<b.size()?-1:a.size()>b.size()?1:0;}
static bool Less(const Item&a,const Item&b){
    const int textKeys[]={1,2}; for(int k:textKeys){int c=CmpText(a.f[k],b.f[k]);if(c)return c<0;}
    const int numKeys[]={12,13,14,15}; for(int k:numKeys){long long x=Num(a.f[k]),y=Num(b.f[k]);if(x!=y)return x<y;}
    const int remainingKeys[]={16,17,18,19,10,11,20,6,7,8,9,3,4,5,21,22,23,24,25,26,27};
    for(int k:remainingKeys){int c=CmpText(a.f[k],b.f[k]);if(c)return c<0;}
    return Num(a.f[0])<Num(b.f[0]);
}
static bool SameSegment(const Item&a,const Item&b){if(a.f.size()!=28||b.f.size()!=28)return false;for(size_t i=1;i<28;++i)if(a.f[i]!=b.f[i])return false;return true;}
static std::wstring ErrorText(const std::wstring& s){return L"The national CSV was not created.\n\n"+s;}

int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int){
    try{
        fs::path base=fs::current_path(), output=base/L"DLL_703854028M_National_Complete.csv", temp=base/L"DLL_703854028M_Merge_Temporary";
        std::vector<fs::path> inputs;
        for(const auto&e:fs::directory_iterator(base))if(e.is_regular_file()){std::wstring n=e.path().filename().wstring();if(n.size()>=13&&n.substr(n.size()-13)==L"_Complete.csv"&&e.path()!=output)inputs.push_back(e.path());}
        std::sort(inputs.begin(),inputs.end());
        if(inputs.empty()){MessageBoxW(nullptr,L"Place this application in the folder containing the *_Complete.csv files, then double-click it again.",L"DLL_703854028M CSV Merger",MB_OK|MB_ICONINFORMATION);return 1;}
        if(MessageBoxW(nullptr,(L"Merge and externally sort "+std::to_wstring(inputs.size())+L" Complete CSV files?\n\nOutput: DLL_703854028M_National_Complete.csv").c_str(),L"DLL_703854028M CSV Merger",MB_OKCANCEL|MB_ICONQUESTION)!=IDOK)return 0;
        fs::create_directories(temp); std::vector<fs::path> runs; std::vector<Item> chunk; chunk.reserve(500000); size_t runNo=0;
        auto flush=[&](){if(chunk.empty())return;std::sort(chunk.begin(),chunk.end(),Less);fs::path p=temp/(L"run_"+std::to_wstring(runNo++)+L".csv");std::ofstream o(p,std::ios::binary);for(auto&x:chunk)o<<x.line<<'\n';o.close();runs.push_back(p);chunk.clear();};
        for(const auto&p:inputs){std::ifstream in(p,std::ios::binary);if(!in)throw std::runtime_error("Unable to open an input CSV.");std::string line;if(!std::getline(in,line))continue;if(!line.empty()&&line.back()=='\r')line.pop_back();if(line.size()>=3&&(unsigned char)line[0]==0xEF&&(unsigned char)line[1]==0xBB&&(unsigned char)line[2]==0xBF)line.erase(0,3);if(line!=Header)throw std::runtime_error("A CSV has an unexpected column header.");while(std::getline(in,line)){if(!line.empty()&&line.back()=='\r')line.pop_back();if(line.empty())continue;auto f=ParseCsv(line);if(f.size()!=28)throw std::runtime_error("A CSV row does not contain exactly 28 columns.");chunk.push_back({line,std::move(f)});if(chunk.size()>=500000)flush();}} flush();
        struct Node{Item item;size_t run;};struct Greater{bool operator()(const Node&a,const Node&b)const{return Less(b.item,a.item);}};
        std::vector<std::ifstream> streams(runs.size());std::priority_queue<Node,std::vector<Node>,Greater> heap;
        for(size_t i=0;i<runs.size();++i){streams[i].open(runs[i],std::ios::binary);std::string line;if(std::getline(streams[i],line))heap.push({{line,ParseCsv(line)},i});}
        fs::path partial=output;partial+=L".partial";std::ofstream out(partial,std::ios::binary|std::ios::trunc);out<<Header<<'\n';unsigned long long rows=0,duplicates=0;Item previous;bool havePrevious=false;
        while(!heap.empty()){Node n=heap.top();heap.pop();if(havePrevious&&SameSegment(previous,n.item)){++duplicates;}else{++rows;size_t comma=n.item.line.find(',');if(comma==std::string::npos)throw std::runtime_error("A CSV row is malformed.");out<<rows<<n.item.line.substr(comma)<<'\n';previous=n.item;havePrevious=true;}std::string line;if(std::getline(streams[n.run],line))heap.push({{line,ParseCsv(line)},n.run});}out.close();if(!out)throw std::runtime_error("The output file could not be completed.");
        if(fs::exists(output))fs::remove(output);fs::rename(partial,output);for(auto&p:runs)fs::remove(p);fs::remove(temp);for(const auto&p:inputs)fs::remove(p);
        MessageBoxW(nullptr,(L"Completed successfully.\n\nNational rows: "+std::to_wstring(rows)+L"\nExact duplicates removed: "+std::to_wstring(duplicates)+L"\nSource CSV files deleted: "+std::to_wstring(inputs.size())+L"\nFile: DLL_703854028M_National_Complete.csv").c_str(),L"DLL_703854028M CSV Merger",MB_OK|MB_ICONINFORMATION);return 0;
    }catch(const std::exception&e){std::wstring w(e.what(),e.what()+strlen(e.what()));MessageBoxW(nullptr,ErrorText(w).c_str(),L"DLL_703854028M CSV Merger",MB_OK|MB_ICONERROR);return 1;}catch(...){MessageBoxW(nullptr,L"The national CSV was not created because an unexpected error occurred.",L"DLL_703854028M CSV Merger",MB_OK|MB_ICONERROR);return 1;}
}
