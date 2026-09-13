#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cwctype>
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
static const std::map<std::string,std::string> SUFFIXES={{"STREET","ST"},{"ST","ST"},{"AVENUE","AVE"},{"AVE","AVE"},{"BOULEVARD","BLVD"},{"BLVD","BLVD"},{"ROAD","RD"},{"RD","RD"},{"DRIVE","DR"},{"DR","DR"},{"LANE","LN"},{"LN","LN"},{"COURT","CT"},{"CT","CT"},{"PLACE","PL"},{"PL","PL"},{"PARKWAY","PKWY"},{"PKWY","PKWY"},{"HIGHWAY","HWY"},{"HWY","HWY"},{"TERRACE","TER"},{"TER","TER"},{"CIRCLE","CIR"},{"CIR","CIR"},{"TRAIL","TRL"},{"TRL","TRL"},{"WAY","WAY"}};
static const std::map<std::string,std::string> DIRS={{"NORTH","N"},{"N","N"},{"SOUTH","S"},{"S","S"},{"EAST","E"},{"E","E"},{"WEST","W"},{"W","W"},{"NORTHEAST","NE"},{"NE","NE"},{"NORTHWEST","NW"},{"NW","NW"},{"SOUTHEAST","SE"},{"SE","SE"},{"SOUTHWEST","SW"},{"SW","SW"}};

static std::vector<std::string> words(const std::string& value) {
    std::istringstream input(value); std::vector<std::string> result; std::string token;
    while(input>>token) result.push_back(token); return result;
}
static std::string tokenPrefixKey(const std::string& core) {
    std::string result;
    for(const auto& token:words(core)) {
        if(!result.empty()) result+='|';
        result+=token.substr(0,std::min<size_t>(3,token.size()));
    }
    return result;
}
static std::string tokenPhoneticKey(const std::string& core) {
    std::string result;
    for(const auto& token:words(core)) {
        if(!result.empty()) result+='|';
        std::string code=soundex(token);
        result+=code.empty()?token:code;
    }
    return result;
}
struct Street { std::string name,coreName,tokenPrefixKey,tokenPhoneticKey,numericKey,numericClass,prefix,suffix,suffixDir; };
static Street parseStreet(std::string raw) {
    raw=upper(trim(normalizeDashes(raw))); std::regex spaces("\\s+"); raw=std::regex_replace(raw,spaces," ");
    std::istringstream ss(raw); std::vector<std::string> w; std::string x; while(ss>>x){ auto n=WORD_NUMBERS.find(x); w.push_back(n==WORD_NUMBERS.end()?x:n->second); }
    Street r; if(!w.empty()){ auto d=DIRS.find(w.front()); if(d!=DIRS.end()){r.prefix=d->second; w.erase(w.begin());} }
    if(!w.empty()){ auto d=DIRS.find(w.back()); if(d!=DIRS.end()){r.suffixDir=d->second; w.pop_back();} }
    if(!w.empty()){ auto s=SUFFIXES.find(w.back()); if(s!=SUFFIXES.end()){r.suffix=s->second; w.pop_back();} }
    for(size_t i=0;i<w.size();i++){ if(i)r.coreName+=' '; r.coreName+=w[i]; }
    r.coreName=trim(r.coreName);
    r.tokenPrefixKey=tokenPrefixKey(r.coreName);
    r.tokenPhoneticKey=tokenPhoneticKey(r.coreName);
    std::smatch m; if(std::regex_search(r.coreName,m,std::regex("(^| )([0-9]+)(ST|ND|RD|TH)?($| )"))){ r.numericKey=m[2].str(); r.numericClass="N"; } else r.numericClass="T";
    std::string full; if(!r.prefix.empty()) full+=r.prefix+" "; full+=r.coreName; if(!r.suffix.empty()) full+=" "+r.suffix; if(!r.suffixDir.empty()) full+=" "+r.suffixDir; r.name=trim(full); return r;
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
static std::string getAny(const std::vector<std::string>& row,const std::map<std::string,size_t>& idx,const std::vector<std::string>& keys){for(const auto& key:keys){std::string value=get(row,idx,key);if(!value.empty())return value;}return "";}
static void writeRow(std::ostream& o,const std::vector<std::string>& v){for(size_t i=0;i<v.size();i++){if(i)o<<',';o<<csv(v[i]);}o<<"\r\n";}

static bool processFile(const fs::path& path, std::string& summary) {
    std::ifstream in(path,std::ios::binary); if(!in){summary="Could not open "+path.string();return false;}
    bool ok=false; auto header=parseCsvRecord(in,ok); if(!ok){summary="Invalid or empty CSV: "+path.string();return false;}
    if(!header.empty() && header[0].size()>=3 && (unsigned char)header[0][0]==0xEF) header[0].erase(0,3);
    std::map<std::string,size_t> idx; for(size_t i=0;i<header.size();i++)idx[upper(trim(header[i]))]=i;
    bool finalInput=idx.count("ROWID")&&idx.count("ZIPCODE")&&idx.count("STREETNAME")&&idx.count("LOWHOUSENUMBER")&&idx.count("HIGHHOUSENUMBER");
    const std::vector<std::string> required=finalInput?
        std::vector<std::string>{"ROWID","ZIPCODE","STREETNAME","LOWHOUSENUMBER","HIGHHOUSENUMBER","CENTROIDLATITUDE","CENTROIDLONGITUDE"}:
        std::vector<std::string>{"ZIP_CODE","ZIP3","FULL_STREET_NAME","FROM_HOUSE_NUMBER","TO_HOUSE_NUMBER","RANGE_CENTROID_LATITUDE","RANGE_CENTROID_LONGITUDE"};
    for(auto& k:required)if(!idx.count(k)){summary="Missing required column: "+k+" in "+path.filename().string();return false;}
    std::string digits;
    for (char ch : path.stem().string()) {
        if (std::isdigit((unsigned char)ch)) {
            digits += ch;
            if (digits.size() == 5) break;
        }
    }
    if (digits.size() != 5) {
        summary="The input filename must contain at least five numeric characters: "+path.filename().string();
        return false;
    }
    fs::path base=path.parent_path()/fs::path(digits+"_Complete.csv");
    fs::path work=path.parent_path()/fs::path(digits+"_Complete.tmp");
    fs::path rej=path.parent_path()/fs::path(digits+"_Rejected.csv");
    std::error_code staleError;
    if(fs::exists(rej) && !fs::remove(rej,staleError)) { summary="An older rejected file could not be removed. Close any program using it, then try again."; return false; }
    std::ofstream out(work,std::ios::binary); std::ofstream bad; if(!out){summary="Could not create the completed output file beside the input.";return false;}
    writeRow(out,{"RowID","ZIPCode","StreetName","StreetSoundex","StreetNumericKey","StreetNumericClass","PrefixDirectional","StreetSuffix","SuffixDirectional","HouseNumberType","LowHouseNumber","HighHouseNumber","LowPrimaryNumber","HighPrimaryNumber","LowSecondaryNumber","HighSecondaryNumber","LowAlphaSuffix","HighAlphaSuffix","LowFractionValue","HighFractionValue","DerivedParity","CentroidLatitude","CentroidLongitude","StreetCoreName","StreetTokenPrefixKey","StreetTokenPhoneticKey","ARID","TLID","SideIndicator","StateFIPS","CountyFIPS","SourceVintage","SegmentIdentityKey"});
    unsigned long long source=1,accepted=0,rejected=0;
    while(in.peek()!=EOF){ auto row=parseCsvRecord(in,ok); ++source; if(row.empty()&&!ok)break;
        bool blankRecord=true; for(const auto& value:row) if(!trim(value).empty()){blankRecord=false;break;} if(blankRecord)continue;
        if(finalInput){
            std::string streetName=get(row,idx,"STREETNAME");
            if(!ok||streetName.empty()||row.size()<23){
                if(!bad.is_open()){bad.open(rej,std::ios::binary);writeRow(bad,{"SourceFile","SourceRecord","Reason"});}
                writeRow(bad,{path.filename().string(),std::to_string(source),!ok?"Malformed CSV record":"Invalid final-schema record"});rejected++;continue;
            }
            Street st=parseStreet(streetName);
            std::string arid=getAny(row,idx,{"ARID"});
            std::string tlid=getAny(row,idx,{"TLID"});
            std::string side=getAny(row,idx,{"SIDEINDICATOR","SIDE","SIDE_IND"});
            std::string state=getAny(row,idx,{"STATEFIPS","STATEFP","STATEFP20"});
            std::string county=getAny(row,idx,{"COUNTYFIPS","COUNTYFP","COUNTYFP20"});
            std::string vintage=getAny(row,idx,{"SOURCEVINTAGE","VINTAGE"});
            row.resize(26);
            row[23]=st.coreName;
            row[24]=st.tokenPrefixKey;
            row[25]=st.tokenPhoneticKey;
            row.push_back(arid);
            row.push_back(tlid);
            row.push_back(side);
            row.push_back(state);
            row.push_back(county);
            row.push_back(vintage);
            std::string identity;
            if(!arid.empty()) identity="A|"+vintage+"|"+arid;
            else if(!tlid.empty()) identity="T|"+vintage+"|"+tlid+"|"+side+"|"+get(row,idx,"ZIPCODE")+"|"+get(row,idx,"LOWHOUSENUMBER")+"|"+get(row,idx,"HIGHHOUSENUMBER");
            else identity="F|"+get(row,idx,"ZIPCODE")+"|"+streetName+"|"+get(row,idx,"LOWHOUSENUMBER")+"|"+get(row,idx,"HIGHHOUSENUMBER")+"|"+get(row,idx,"CENTROIDLATITUDE")+"|"+get(row,idx,"CENTROIDLONGITUDE");
            row.push_back(identity);
            writeRow(out,row);accepted++;continue;
        }
        std::string zip=get(row,idx,"ZIP_CODE"), streetRaw=get(row,idx,"FULL_STREET_NAME"), loRaw=get(row,idx,"FROM_HOUSE_NUMBER"), hiRaw=get(row,idx,"TO_HOUSE_NUMBER"), lat=get(row,idx,"RANGE_CENTROID_LATITUDE"), lon=get(row,idx,"RANGE_CENTROID_LONGITUDE");
        std::string arid=getAny(row,idx,{"ARID"});
        std::string tlid=getAny(row,idx,{"TLID"});
        std::string side=getAny(row,idx,{"SIDEINDICATOR","SIDE","SIDE_IND"});
        std::string state=getAny(row,idx,{"STATEFIPS","STATEFP","STATEFP20"});
        std::string county=getAny(row,idx,{"COUNTYFIPS","COUNTYFP","COUNTYFP20"});
        std::string vintage=getAny(row,idx,{"SOURCEVINTAGE","VINTAGE"});
        std::string reason; if(!ok)reason="Malformed CSV record"; else if(!std::regex_match(zip,std::regex("[0-9]{5}(-[0-9]{4})?")))reason="Invalid ZIP code"; else if(streetRaw.empty())reason="Blank street name"; else if(loRaw.empty()||hiRaw.empty())reason="Blank house-number endpoint"; else {try{std::stod(lat);std::stod(lon);}catch(...){reason="Invalid latitude or longitude";}}
        Street st=parseStreet(streetRaw); House lo=parseHouse(loRaw),hi=parseHouse(hiRaw); if(reason.empty()&&(st.name.empty()||lo.text.empty()||hi.text.empty()))reason="Unusable lookup value";
        if(!reason.empty()){
            if(!bad.is_open()) {
                bad.open(rej,std::ios::binary);
                if(!bad) { summary="Could not create the rejected-record output file. The original CSV was preserved."; return false; }
                writeRow(bad,{"SourceFile","SourceRecord","Reason","ZIP_CODE","FULL_STREET_NAME","FROM_HOUSE_NUMBER","TO_HOUSE_NUMBER"});
            }
            writeRow(bad,{path.filename().string(),std::to_string(source),reason,zip,streetRaw,loRaw,hiRaw});rejected++;continue;
        }
        int rangeType=(lo.type==hi.type?lo.type:5); std::string parity=(lo.parity>=0&&hi.parity>=0&&lo.parity==hi.parity)?(lo.parity?"O":"E"):"B";
        std::string identity;
        if(!arid.empty()) identity="A|"+vintage+"|"+arid;
        else if(!tlid.empty()) identity="T|"+vintage+"|"+tlid+"|"+side+"|"+zip+"|"+lo.text+"|"+hi.text;
        else identity="F|"+zip+"|"+st.name+"|"+lo.text+"|"+hi.text+"|"+lat+"|"+lon;
        writeRow(out,{std::to_string(++accepted),zip,st.name,soundex(st.name),st.numericKey,st.numericClass,st.prefix,st.suffix,st.suffixDir,std::to_string(rangeType),lo.text,hi.text,lo.primary,hi.primary,lo.secondary,hi.secondary,lo.alpha,hi.alpha,lo.fraction,hi.fraction,parity,lat,lon,st.coreName,st.tokenPrefixKey,st.tokenPhoneticKey,arid,tlid,side,state,county,vintage,identity});
    }
    in.close();
    out.close();
    if(bad.is_open()) bad.close();
    if(!out || (rejected>0 && !bad)) { summary="Output write failed; the original CSV was preserved."; return false; }
    std::error_code ec;
    fs::path backup=path.parent_path()/fs::path(digits+"_Previous.csv");
    if(finalInput) {
        fs::remove(backup,ec); ec.clear();
        fs::rename(path,backup,ec);
        if(ec){summary="The prior lookup CSV could not be prepared for safe replacement.";return false;}
        fs::rename(work,base,ec);
        if(ec){std::error_code restore;fs::rename(backup,path,restore);summary="Completed CSV replacement failed; the prior file was restored.";return false;}
        fs::remove(backup,ec);
        summary=base.filename().string()+": "+std::to_string(accepted)+" upgraded, "+std::to_string(rejected)+" rejected.";return true;
    }
    fs::rename(work,base,ec);
    if(ec){summary="The completed output file could not be finalized; the original CSV was preserved.";return false;}
    fs::remove(path,ec);
    if(ec){summary="The completed file was created, but the original CSV could not be deleted.";return false;}
    summary=base.filename().string()+": "+std::to_string(accepted)+" accepted, "+std::to_string(rejected)+" rejected. Original CSV deleted.";return true;
}

static std::string numericPrefix(const fs::path& path) {
    const std::wstring name=path.filename().wstring();
    const size_t delimiter=name.find(L"_");
    if(delimiter==std::wstring::npos) return "";
    return std::string(name.begin(),name.begin()+delimiter);
}

static std::vector<fs::path> findInputFiles() {
    std::vector<wchar_t> modulePath(32768);
    DWORD length=GetModuleFileNameW(nullptr,modulePath.data(),(DWORD)modulePath.size());
    if(length==0 || length>=modulePath.size()) return {};
    fs::path folder=fs::path(std::wstring(modulePath.data(),length)).parent_path();
    const std::wregex inputPattern(L"^[0-9]+_{1,2}extracted_street_data\\.csv$",std::regex_constants::icase);
    std::vector<fs::path> files;
    for(const auto& entry:fs::directory_iterator(folder)) {
        if(entry.is_regular_file() && std::regex_match(entry.path().filename().wstring(),inputPattern))
            files.push_back(entry.path());
    }
    std::sort(files.begin(),files.end(),[](const fs::path& a,const fs::path& b) {
        std::string left=numericPrefix(a),right=numericPrefix(b);
        const size_t leftNonZero=left.find_first_not_of('0');
        const size_t rightNonZero=right.find_first_not_of('0');
        left=leftNonZero==std::string::npos?"0":left.substr(leftNonZero);
        right=rightNonZero==std::string::npos?"0":right.substr(rightNonZero);
        if(left.size()!=right.size()) return left.size()<right.size();
        if(left!=right) return left<right;
        return a.filename().wstring()<b.filename().wstring();
    });
    return files;
}

int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int) {
    auto files=findInputFiles();
    if(files.empty()) {
        MessageBoxW(nullptr,L"No matching input CSV file was found beside CensusAddressPreprocessor.exe.\n\nPlace files named like 18089_extracted_street_data.csv in the same folder, then double-click the executable again. Both one and two underscores are accepted.",L"No input files found",MB_OK|MB_ICONWARNING);
        return 1;
    }

    size_t good=0;
    std::string failures;
    for(const auto& file:files) {
        std::string summary;
        if(processFile(file,summary)) ++good;
        else failures+=summary+"\n";
    }

    const size_t failed=files.size()-good;
    std::string report="Files found: "+std::to_string(files.size())+
        "\nProcessed successfully: "+std::to_string(good)+
        "\nFailed: "+std::to_string(failed);
    if(!failures.empty()) report+="\n\nFailed files were preserved:\n"+failures;
    std::wstring wideReport(report.begin(),report.end());
    MessageBoxW(nullptr,wideReport.c_str(),failed==0?L"Processing complete":L"Processing finished with errors",
        MB_OK|(failed==0?MB_ICONINFORMATION:MB_ICONWARNING));
    return failed==0?0:1;
}
