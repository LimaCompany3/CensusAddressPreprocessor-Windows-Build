using System;
using System.Collections.Generic;
using System.Data.SqlTypes;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.SqlServer.Server;

public static class DLL_703854028M
{
    static readonly Dictionary<string,string> Dirs=new Dictionary<string,string>(StringComparer.OrdinalIgnoreCase){{"NORTH","N"},{"SOUTH","S"},{"EAST","E"},{"WEST","W"},{"NORTHEAST","NE"},{"NORTHWEST","NW"},{"SOUTHEAST","SE"},{"SOUTHWEST","SW"},{"N","N"},{"S","S"},{"E","E"},{"W","W"},{"NE","NE"},{"NW","NW"},{"SE","SE"},{"SW","SW"}};
    static readonly Dictionary<string,string> Suffix=new Dictionary<string,string>(StringComparer.OrdinalIgnoreCase){{"STREET","ST"},{"ST","ST"},{"AVENUE","AVE"},{"AVE","AVE"},{"ROAD","RD"},{"RD","RD"},{"DRIVE","DR"},{"DR","DR"},{"BOULEVARD","BLVD"},{"BLVD","BLVD"},{"LANE","LN"},{"LN","LN"},{"COURT","CT"},{"CT","CT"},{"PLACE","PL"},{"PL","PL"},{"PARKWAY","PKWY"},{"PKWY","PKWY"},{"HIGHWAY","HWY"},{"HWY","HWY"},{"TERRACE","TER"},{"TER","TER"},{"CIRCLE","CIR"},{"CIR","CIR"},{"TRAIL","TRL"},{"TRL","TRL"},{"WAY","WAY"}};
    static readonly Dictionary<string,string> Ord=new Dictionary<string,string>(StringComparer.OrdinalIgnoreCase){{"FIRST","1ST"},{"SECOND","2ND"},{"THIRD","3RD"},{"FOURTH","4TH"},{"FIFTH","5TH"},{"SIXTH","6TH"},{"SEVENTH","7TH"},{"EIGHTH","8TH"},{"NINTH","9TH"},{"TENTH","10TH"},{"ELEVENTH","11TH"},{"TWELFTH","12TH"},{"THIRTEENTH","13TH"},{"FOURTEENTH","14TH"},{"FIFTEENTH","15TH"},{"SIXTEENTH","16TH"},{"SEVENTEENTH","17TH"},{"EIGHTEENTH","18TH"},{"NINETEENTH","19TH"},{"TWENTIETH","20TH"}};

    [SqlProcedure]
    public static void NormalizeAddress(SqlString rawAddress, SqlString postalCode,
        out SqlString streetName,out SqlString relaxedStreet,out SqlString numericKey,out SqlString streetSoundex,
        out SqlString houseNumber,out SqlInt32 primaryNumber,out SqlInt32 secondaryNumber,out SqlString alphaSuffix,
        out SqlString fraction,out SqlString streetCore,out SqlString tokenPrefixKey,out SqlString tokenPhoneticKey,out SqlString postalCodeOut)
    {
        streetName=relaxedStreet=numericKey=streetSoundex=houseNumber=alphaSuffix=fraction=streetCore=tokenPrefixKey=tokenPhoneticKey=postalCodeOut=SqlString.Null;
        primaryNumber=secondaryNumber=SqlInt32.Null;
        try {
            string zip=postalCode.IsNull?"":Regex.Replace(postalCode.Value,"[^0-9]",""); if(zip.Length>=5) zip=zip.Substring(0,5); postalCodeOut=zip.Length==5?new SqlString(zip):SqlString.Null;
            if(rawAddress.IsNull) return; string a=rawAddress.Value.ToUpperInvariant().Replace('–','-').Replace('—','-').Trim();
            a=a.Split(',')[0]; a=Regex.Replace(a,@"\s+(APT|APARTMENT|UNIT|SUITE|STE|FLOOR|FL|#)\s+.*$","",RegexOptions.IgnoreCase); a=Regex.Replace(a,@"\s+"," ").Trim();
            Match hm=Regex.Match(a,@"^(\d+(?:-\d+)?(?:[A-Z]+)?(?:\s+\d+/\d+)?)\s+(.+)$"); if(!hm.Success) return;
            string hn=hm.Groups[1].Value, sr=hm.Groups[2].Value; houseNumber=new SqlString(hn);
            Match hp=Regex.Match(hn,@"^(\d+)(?:-(\d+))?([A-Z]+)?(?:\s+(\d+/\d+))?$"); if(hp.Success){primaryNumber=new SqlInt32(Int32.Parse(hp.Groups[1].Value));if(hp.Groups[2].Success)secondaryNumber=new SqlInt32(Int32.Parse(hp.Groups[2].Value));if(hp.Groups[3].Success)alphaSuffix=new SqlString(hp.Groups[3].Value);if(hp.Groups[4].Success)fraction=new SqlString(hp.Groups[4].Value);}
            List<string> w=new List<string>(sr.Split(new[]{' '},StringSplitOptions.RemoveEmptyEntries)); for(int i=0;i<w.Count;i++)if(Ord.ContainsKey(w[i]))w[i]=Ord[w[i]];
            string pre="",suf="",sdir=""; if(w.Count>0&&Dirs.ContainsKey(w[0])){pre=Dirs[w[0]];w.RemoveAt(0);} if(w.Count>0&&Dirs.ContainsKey(w[w.Count-1])){sdir=Dirs[w[w.Count-1]];w.RemoveAt(w.Count-1);} if(w.Count>0&&Suffix.ContainsKey(w[w.Count-1])){suf=Suffix[w[w.Count-1]];w.RemoveAt(w.Count-1);}
            string core=String.Join(" ",w); string full=((pre.Length>0?pre+" ":"")+core+(suf.Length>0?" "+suf:"")+(sdir.Length>0?" "+sdir:"")).Trim();
            streetName=new SqlString(full); streetCore=new SqlString(core); relaxedStreet=new SqlString(core); streetSoundex=new SqlString(Soundex(full));
            Match nm=Regex.Match(core,@"(^| )(\d+)(ST|ND|RD|TH)?($| )"); if(nm.Success)numericKey=new SqlString(nm.Groups[2].Value);
            List<string> pk=new List<string>(),ph=new List<string>(); foreach(string x in w){pk.Add(x.Substring(0,Math.Min(3,x.Length)));ph.Add(Soundex(x));} tokenPrefixKey=new SqlString(String.Join("|",pk));tokenPhoneticKey=new SqlString(String.Join("|",ph));
        } catch { }
    }

    static string Soundex(string value){string s=Regex.Replace(value.ToUpperInvariant(),"[^A-Z]","");if(s.Length==0)return "";StringBuilder o=new StringBuilder();o.Append(s[0]);char p=Code(s[0]);for(int i=1;i<s.Length&&o.Length<4;i++){char d=Code(s[i]);if(d!='0'&&d!=p)o.Append(d);p=d;}while(o.Length<4)o.Append('0');return o.ToString();}
    static char Code(char c){if("BFPV".IndexOf(c)>=0)return '1';if("CGJKQSXZ".IndexOf(c)>=0)return '2';if("DT".IndexOf(c)>=0)return '3';if(c=='L')return '4';if("MN".IndexOf(c)>=0)return '5';if(c=='R')return '6';return '0';}
}
