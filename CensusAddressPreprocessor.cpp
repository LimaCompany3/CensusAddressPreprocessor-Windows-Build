#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string trim(std::string s) {
    auto notSpace=[](unsigned char c){ return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

static std::string upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::toupper(c); });
    return s;
}

static std::string normalizeDashes(std::string s) {
    const std::vector<std::string> dashes={"\xE2\x80\x90","\xE2\x80\x91","\xE2\x80\x92","\xE2\x80\x93","\xE2\x80\x94","\xE2\x88\x92"};
    for (const auto& d:dashes) { size_t p=0; while ((p=s.find(d,p))!=std::string::npos) { s.replace(p,d.size(),"-"); ++p; } }
    return s;
}

static std::vector<std::string> parseCsvRecord(std::istream& in, bool& ok) {
    std::vector<std::string> fields; std::string field; bool quoted=false, any=false; char c;
    while (in.get(c)) {
        any=true;
        if (quoted) {
            if (c=='"') { if (in.peek()=='"') { in.get(c); field.push_back('"'); } else quoted=false; }
            else field.push_back(c);
        } else {
            if (c=='"' && field.empty()) quoted=true;
            else if (c==',') { fields.push_back(field); field.clear(); }
            else if (c=='\n') { fields.push_back(field); ok=true; return fields; }
            else if (c!='\r') field.push_back(c);
        }
    }
    if (any) { fields.push_back(field); ok=!quoted; return fields; }
    ok=false; return {};
}

static std::string csv(const std::string& s) {
    if (s.find_first_of(",\"\r\n")==std::string::npos) return s;
    std::string r="\""; for(char c:s){ if(c=='\"') r+="\"\""; else r+=c; } return r+="\"";
}

static std::string soundex(const std::string& input) {
    std::string s; for(unsigned char c:upper(input)) if(std::isalpha(c)) s+=(char)c;
    if(s.empty()) return "";
    auto code=[](char c){ if(std::string("BFPV").find(c)!=std::string::npos)return '1'; if(std::string("CGJKQSXZ").find(c)!=std::string::npos)return '2'; if(std::string("DT").find(c)!=std::string::npos)return '3'; if(c=='L')return '4'; if(std::string("MN").find(c)!=std::string::npos)return '5'; if(c=='R')return '6'; return '0'; };
    std::string out(1,s[0]); char prev=code(s[0]);
    for(size_t i=1;i<s.size() && out.size()<4;i++){ char d=code(s[i]); if(d!='0' && d!=prev) out+=d; prev=d; }
    while(out.size()<4) out+='0'; return out;
}

static const std::map<std::string,std::string> WORD_NUMBERS={
    {"FIRST","1ST"},{"SECOND","2ND"},{"THIRD","3RD"},{"FOURTH","4TH"},{"FIFTH","5TH"},{"SIXTH","6TH"},{"SEVENTH","7TH"},{"EIGHTH","8TH"},{"NINTH","9TH"},{"TENTH","10TH"},
    {"ELEVENTH","11TH"},{"TWELFTH","12TH"},{"THIRTEENTH","13TH"},{"FOURTEENTH","14TH"},{"FIFTEENTH","15TH"},{"SIXTEENTH","16TH"},{"SEVENTEENTH","17TH"},{"EIGHTEENTH","18TH"},{"NINETEENTH","19TH"},{"TWENTIETH","20TH"}
};
static const std::map<std::string,std::string> SUFFIXES={{"STREET","ST"},{"AVENUE","AVE"},{"BOULEVARD","BLVD"},{"ROAD","RD"},{"DRIVE","DR"},{"LANE","LN"},{"COURT","CT"},{"PLACE","PL"},{"PARKWAY","PKWY"},{"HIGHWAY","HWY"},{"TERRACE","TER"},{"CIRCLE","CIR"},{"TRAIL","TRL"},{"WAY","WAY"}};
static const std::map<std::string,std::string> DIRS={{"NORTH","N"},{"SOUTH","S"},{"EAST","E"},{"WEST","W"},{"NORTHEAST","NE"},{"NORTHWEST","NW"},{"SOUTHEAST","SE"},{"SOUTHWEST","SW"}};

struct Street { std::string name,numericKey,numericClass,prefix,suffix,suffixDir; };
static Street parseStreet(std::string raw) {
    raw=upper(trim(normalizeDashes(raw))); std::regex spaces("\\s+"); raw=std::regex_replace(raw,spaces," ");
    std::istringstream ss(raw); std::vector<std::string> w; std::string x; while(ss>>x){ auto n=WORD_NUMBERS.find(x); w.push_back(n==WORD_NUMBERS.end()?x:n->second); }
    Street r; if(!w.empty()){ auto d=DIRS.find(w.front()); if(d!=DIRS.end()){r.prefix=d->second; w.erase(w.begin());} }
    if(!w.empty()){ auto d=DIRS.find(w.back()); if(d!=DIRS.end()){r.suffixDir=d->second; w.pop_back();} }
    if(!w.empty()){ auto s=SUFFIXES.find(w.back()); if(s!=SUFFIXES.end()){r.suffix=s->second; w.pop_back();} }
    for(size_t i=0;i<w.size();i++){ if(i)r.name+=' '; r.name+=w[i]; }
    std::smatch m; if(std::regex_search(r.name,m,std::regex("(^| )([0-9]+)(ST|ND|RD|TH)?($| )"))){ r.numericKey=m[2].str(); r.numericClass="N"; } else r.numericClass="T";
    std::string full; if(!r.prefix.empty()) full+=r.prefix+" "; full+=r.name; if(!r.suffix.empty()) full+=" "+r.suffix; if(!r.suffixDir.empty()) full+=" "+r.suffixDir; r.name=trim(full); return r;
}

struct House { std::string text,primary,secondary,alpha,fraction; int type=5; bool comparable=false; long long parity=-1; };
static House parseHouse(std::string s) {
    House h; h.text=upper(trim(normalizeDashes(s))); std::smatch m;
    if(std::regex_match(h.text,m,std::regex("([0-9]+)\\s+([0-9]+)\\/([0-9]+)"))){ h.primary=m[1]; double d=std::stod(m[2])/std::stod(m[3]); std::ostringstream q;q<<std::fixed<<std::setprecision(6)<<d;h.fraction=q.str();h.type=4;h.comparable=true;h.parity=std::stoll(h.primary)%2; }
    else if(std::regex_match(h.text,m,std::regex("([0-9]+)-([0-9]+)"))){h.primary=m[1];h.secondary=m[2];h.type=2;h.comparable=true;h.parity=std::stoll(h.primary)%2;}
    else if(std::regex_match(h.text,m,std::regex("([0-9]+)([A-Z]+)"))){h.primary=m[1];h.alpha=m[2];h.type=3;h.comparable=true;h.parity=std::stoll(h.primary)%2;}
    else if(std::regex_match(h.text,m,std::regex("([0-9]+)"))){h.primary=m[1];h.type=1;h.comparable=true;h.parity=std::stoll(h.primary)%2;}
    return h;
}

static std::string get(const std::vector<std::string>& row,const std::map<std::string,size_t>& idx,const std::string& key){auto i=idx.find(key);return i==idx.end()||i->second>=row.size()?"":trim(row[i->second]);}
static void writeRow(std::ostream& o,const std::vector<std::string>& v){for(size_t i=0;i<v.size();i++){if(i)o<<',';o<<csv(v[i]);}o<<"\r\n";}

static bool processFile(const fs::path& path, std::string& summary) {
    std::ifstream in(path,std::ios::binary); if(!in){summary="Could not open "+path.string();return false;}
    fs::path base=path.parent_path()/(path.stem().wstring()+L"_Final.csv"); fs::path rej=path.parent_path()/(path.stem().wstring()+L"_Rejected.csv");
    std::ofstream out(base,std::ios::binary), bad(rej,std::ios::binary); if(!out||!bad){summary="Could not create output files beside the input.";return false;}
    bool ok=false; auto header=parseCsvRecord(in,ok); if(!ok){summary="Invalid or empty CSV: "+path.string();return false;}
    if(!header.empty() && header[0].size()>=3 && (unsigned char)header[0][0]==0xEF) header[0].erase(0,3);
    std::map<std::string,size_t> idx; for(size_t i=0;i<header.size();i++)idx[upper(trim(header[i]))]=i;
    const std::vector<std::string> required={"ZIP_CODE","ZIP3","FULL_STREET_NAME","FROM_HOUSE_NUMBER","TO_HOUSE_NUMBER","RANGE_CENTROID_LATITUDE","RANGE_CENTROID_LONGITUDE"};
    for(auto& k:required)if(!idx.count(k)){summary="Missing required column: "+k+" in "+path.filename().string();return false;}
    writeRow(out,{"RowID","ZIPCode","StreetName","StreetSoundex","StreetNumericKey","StreetNumericClass","PrefixDirectional","StreetSuffix","SuffixDirectional","HouseNumberType","LowHouseNumber","HighHouseNumber","LowPrimaryNumber","HighPrimaryNumber","LowSecondaryNumber","HighSecondaryNumber","LowAlphaSuffix","HighAlphaSuffix","LowFractionValue","HighFractionValue","DerivedParity","CentroidLatitude","CentroidLongitude"});
    writeRow(bad,{"SourceFile","SourceRecord","Reason","ZIP_CODE","FULL_STREET_NAME","FROM_HOUSE_NUMBER","TO_HOUSE_NUMBER"});
    unsigned long long source=1,accepted=0,rejected=0;
    while(in.peek()!=EOF){ auto row=parseCsvRecord(in,ok); ++source; if(row.empty()&&!ok)break; std::string zip=get(row,idx,"ZIP_CODE"), streetRaw=get(row,idx,"FULL_STREET_NAME"), loRaw=get(row,idx,"FROM_HOUSE_NUMBER"), hiRaw=get(row,idx,"TO_HOUSE_NUMBER"), lat=get(row,idx,"RANGE_CENTROID_LATITUDE"), lon=get(row,idx,"RANGE_CENTROID_LONGITUDE");
        std::string reason; if(!ok)reason="Malformed CSV record"; else if(!std::regex_match(zip,std::regex("[0-9]{5}(-[0-9]{4})?")))reason="Invalid ZIP code"; else if(streetRaw.empty())reason="Blank street name"; else if(loRaw.empty()||hiRaw.empty())reason="Blank house-number endpoint"; else {try{std::stod(lat);std::stod(lon);}catch(...){reason="Invalid latitude or longitude";}}
        Street st=parseStreet(streetRaw); House lo=parseHouse(loRaw),hi=parseHouse(hiRaw); if(reason.empty()&&(st.name.empty()||lo.text.empty()||hi.text.empty()))reason="Unusable lookup value";
        if(!reason.empty()){writeRow(bad,{path.filename().string(),std::to_string(source),reason,zip,streetRaw,loRaw,hiRaw});rejected++;continue;}
        int rangeType=(lo.type==hi.type?lo.type:5); std::string parity=(lo.parity>=0&&hi.parity>=0&&lo.parity==hi.parity)?(lo.parity?"O":"E"):"B";
        writeRow(out,{std::to_string(++accepted),zip,st.name,soundex(st.name),st.numericKey,st.numericClass,st.prefix,st.suffix,st.suffixDir,std::to_string(rangeType),lo.text,hi.text,lo.primary,hi.primary,lo.secondary,hi.secondary,lo.alpha,hi.alpha,lo.fraction,hi.fraction,parity,lat,lon});
    }
    summary=path.filename().string()+": "+std::to_string(accepted)+" accepted, "+std::to_string(rejected)+" rejected"; return true;
}

static std::vector<fs::path> chooseFiles() {
    std::vector<wchar_t> buffer(65536); OPENFILENAMEW ofn{}; ofn.lStructSize=sizeof(ofn); ofn.lpstrFilter=L"CSV files (*.csv)\0*.csv\0All files (*.*)\0*.*\0"; ofn.lpstrFile=buffer.data(); ofn.nMaxFile=(DWORD)buffer.size(); ofn.Flags=OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT;
    if(!GetOpenFileNameW(&ofn))return{}; std::vector<fs::path> files; wchar_t* p=buffer.data(); fs::path first=p; p+=wcslen(p)+1; if(!*p)files.push_back(first); else while(*p){files.push_back(first/p);p+=wcslen(p)+1;} return files;
}

int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int) {
    MessageBoxW(nullptr,L"Select one or more Census CSV files. The originals will not be changed.\n\nThe program creates a _Final.csv and _Rejected.csv beside each input file.",L"Census Address Preprocessor",MB_OK|MB_ICONINFORMATION);
    auto files=chooseFiles(); if(files.empty())return 0; std::string report; int good=0; for(auto& f:files){std::string s;if(processFile(f,s))good++;report+=s+"\n";}
    std::wstring w(report.begin(),report.end()); MessageBoxW(nullptr,w.c_str(),good==(int)files.size()?L"Processing complete":L"Processing finished with errors",MB_OK|(good==(int)files.size()?MB_ICONINFORMATION:MB_ICONWARNING)); return good==(int)files.size()?0:1;
}
