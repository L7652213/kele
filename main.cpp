// ============================================================
//  可乐道具图鉴 —— QQ飞车道具代码查询器 (Win32 / C++ / x64)
//  数据来源: https://www.bnnb.cn  (站内 api.php + 官方图床)
//  编译:  MSVC x64  /utf-8 /EHsc /O2 /MD
// ============================================================
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <winhttp.h>
#include <shellapi.h>
#include <uxtheme.h>

#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <objidl.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "msimg32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace Gdiplus;

// ---------------- 常量与配色 ----------------
static const WCHAR* APP_TITLE = L"可乐道具图鉴 · QQ飞车道具查询器";
static const WCHAR* API_BASE  = L"https://www.bnnb.cn/api.php";
static const WCHAR* IMG_BASE  = L"https://iips.speed.qq.com/images/";
static const int    LIMIT     = 60;
static const int    LIST_ROW_H= 190;

// 主题色
static const COLORREF C_MAIN      = RGB(213,43,43);    // 可乐红
static const COLORREF C_MAIN_HOVER= RGB(232,64,56);
static const COLORREF C_MAIN_DOWN = RGB(174,24,31);
static const COLORREF C_MAIN_LIGHT= RGB(255,235,228);  // 可乐奶油浅底
static const COLORREF C_TOOLBAR   = RGB(255,247,241);
static const COLORREF C_BG        = RGB(252,244,238);
static const COLORREF C_CARD      = RGB(255,255,255);
static const COLORREF C_TEXT      = RGB(40,46,60);
static const COLORREF C_SUB       = RGB(140,150,165);
static const COLORREF C_BORDER    = RGB(226,231,240);
static const int TBH=80, TREEW=252;   // 两行工具栏高度、默认分类栏宽度
static const int SCROLLBAR_W=12;
static const COLORREF C_TREE_BG   = RGB(249,252,250);
static const COLORREF C_TREE_ROOT = RGB(240,248,246);
static const COLORREF C_TREE_SEL  = RGB(255,232,224);
static const COLORREF C_TREE_LINE = RGB(169,202,194);
static const COLORREF C_TREE_TEXT = RGB(52,66,70);
static const COLORREF C_TREE_HEAD = RGB(112,65,54);
static const COLORREF C_TREE_ACCENT = RGB(201,76,55);

// 控件 ID
enum { ID_SEARCH=1001, ID_MODE, ID_GO, ID_PREV, ID_NEXT, ID_PAGEINFO, ID_STAT,
       ID_TREE, ID_LIST, ID_DETAIL, ID_COPY_ID, ID_COPY_BOTH, ID_TREE_SCROLL, ID_LIST_SCROLL };
static const UINT_PTR ID_COPY_TIMER=3001;
// 自定义消息
enum { MSG_LOAD_DONE=WM_APP+1, MSG_IMG_READY=WM_APP+2 };

// ---------------- 分类树数据（宽字符字面量，规避源编码问题） ----------------
struct CCategory { const wchar_t* group; const wchar_t* fine; int count; };
static const CCategory g_cats[] = {
 {L"赛车",L"A级赛车",698},{L"赛车",L"B级赛车",508},{L"赛车",L"尾挂",317},{L"赛车",L"赛车皮肤",241},
 {L"赛车",L"K2级轮滑",176},{L"赛车",L"S级赛车",161},{L"赛车",L"胎印",156},{L"赛车",L"车牌",135},
 {L"赛车",L"C级赛车",114},{L"赛车",L"K1级轮滑",110},{L"赛车",L"漂焰",108},{L"赛车",L"T2机甲赛车",84},
 {L"赛车",L"赛车配件",80},{L"赛车",L"车辆炫光",79},{L"赛车",L"T1机甲赛车",55},{L"赛车",L"赛车改装卡",43},
 {L"赛车",L"车轮",41},{L"赛车",L"车头",39},{L"赛车",L"车尾",39},{L"赛车",L"车翼",39},{L"赛车",L"M2级摩托",27},
 {L"赛车",L"喷漆",13},{L"赛车",L"T3机甲赛车",12},{L"赛车",L"进阶改装配件",12},{L"赛车",L"L2级滑板",9},
 {L"赛车",L"进阶改装碎片",9},{L"赛车",L"D级赛车",5},{L"赛车",L"R级赛车",4},{L"赛车",L"L3级滑板",3},
 {L"赛车",L"M3级摩托",3},{L"赛车",L"能量源",3},{L"赛车",L"K0级轮滑",2},{L"赛车",L"M1级摩托",2},
 {L"赛车",L"L0级滑板",1},{L"赛车",L"L1级滑板",1},
 {L"装扮",L"套装",17837},{L"装扮",L"发饰",11172},{L"装扮",L"背饰",3769},{L"装扮",L"上装",3614},
 {L"装扮",L"下装",3349},{L"装扮",L"配饰",1950},{L"装扮",L"手杖",1924},{L"装扮",L"头饰",1810},
 {L"装扮",L"表情",1664},{L"装扮",L"面饰",974},{L"装扮",L"挂饰",744},{L"装扮",L"手套",652},
 {L"装扮",L"脚底炫光",411},{L"装扮",L"尾饰",375},{L"装扮",L"护镜",289},{L"装扮",L"左手手环",285},
 {L"装扮",L"右手手环",278},{L"装扮",L"坐骑",272},{L"装扮",L"VALANCE",191},{L"装扮",L"肩饰",132},
 {L"装扮",L"飞饰",92},{L"装扮",L"刺青",44},{L"装扮",L"鞋子",15},{L"装扮",L"DECORATION",14},
 {L"装扮",L"上装光效",7},{L"装扮",L"HEADWEAR",1},
 {L"宠物",L"宠物",558},{L"宠物",L"宝宝头饰",263},{L"宠物",L"宠物头饰",263},{L"宠物",L"宝宝服饰",100},
 {L"宠物",L"宠物道具",31},{L"宠物",L"精灵卡",17},{L"宠物",L"宝宝炫光",15},{L"宠物",L"精灵道具",14},
 {L"功能",L"宝箱",8430},{L"功能",L"戒指",1302},{L"功能",L"小喇叭",1165},{L"功能",L"钥匙",381},
 {L"功能",L"功能道具",294},{L"功能",L"名片夹",251},{L"功能",L"抽奖卡",62},{L"功能",L"聊天动作",54},
 {L"功能",L"代金券",48},{L"功能",L"户外特效",48},{L"功能",L"赛车觉醒经验卡",25},{L"功能",L"锤子",21},
 {L"功能",L"操纵魔法",20},{L"功能",L"头像卡",11},{L"功能",L"折扣卡",11},{L"功能",L"满减券",11},
 {L"功能",L"装备类糖果",11},{L"功能",L"变形卡",10},{L"功能",L"兑换类型",9},{L"功能",L"双倍经验卡",8},
 {L"功能",L"赠送类型",7},{L"功能",L"礼包",6},{L"功能",L"任务类糖果",4},{L"功能",L"更名卡",4},
 {L"功能",L"砸蛋卡",4},{L"功能",L"时间类糖果",3},{L"功能",L"倒计时加速卡",2},{L"功能",L"功能",2},
 {L"功能",L"新双倍经验卡",2},{L"功能",L"比赛道具",2},{L"功能",L"临时驾照",1},{L"功能",L"双倍排位积分卡",1},
 {L"功能",L"战绩清零",1},{L"功能",L"排位积分保护卡",1},{L"功能",L"积分清零",1},{L"功能",L"自动防御卡",1},
 {L"功能",L"车队徽章",1},{L"功能",L"飞行卡",1},{L"功能",L"高飞卡",1},
 {L"其他",L"精简名片",50},{L"其他",L"三角道具之钻",48},{L"其他",L"圆形竞速宝珠",48},{L"其他",L"趣味动作",37},
 {L"其他",L"氛围名片",19},{L"其他",L"鱼儿",19},{L"其他",L"鱼苗",19},{L"其他",L"氛围背景",18},
 {L"其他",L"类型ID:320",17},{L"其他",L"全景名片",14},{L"其他",L"语音包",12},{L"其他",L"类型ID:319",10},
 {L"其他",L"进房秀",7},{L"其他",L"乐器弹奏卡",5},{L"其他",L"车辆",5},{L"其他",L"个人信息场景",4},
 {L"其他",L"炫彩昵称",4},{L"其他",L"聊天气泡",4},{L"其他",L"幸运石",3},{L"其他",L"社交底板",3},
 {L"其他",L"祝福石",3},{L"其他",L"雕刻刀",3},{L"其他",L"鱼饲料",3},{L"其他",L"开场舞场景",2},
 {L"其他",L"结算场景",2},{L"其他",L"宝石拆除刀",1},{L"其他",L"类型ID:2380",1},
};

// ---------------- 编码工具 ----------------
static std::wstring utf8ws(const std::string& u){
    if(u.empty()) return L"";
    int n=MultiByteToWideChar(CP_UTF8,0,u.c_str(),(int)u.size(),NULL,0);
    std::wstring w(n,0);
    MultiByteToWideChar(CP_UTF8,0,u.c_str(),(int)u.size(),&w[0],n);
    return w;
}
static std::string urlEncode(const std::string& s){
    const char* hex="0123456789ABCDEF";
    std::string out;
    for(unsigned char c:s){
        if(isalnum(c)||c=='-'||c=='_'||c=='.'||c=='~') out+=(char)c;
        else{ out+='%'; out+=hex[c>>4]; out+=hex[c&15]; }
    }
    return out;
}
static std::string utf8ws_to_utf8(const std::wstring& w){
    if(w.empty())return "";
    int n=WideCharToMultiByte(CP_UTF8,0,w.c_str(),(int)w.size(),NULL,0,NULL,NULL);
    std::string s(n,0); WideCharToMultiByte(CP_UTF8,0,w.c_str(),(int)w.size(),&s[0],n,NULL,NULL); return s;
}

// ---------------- HTTP (WinHTTP) ----------------
static bool httpGet(const std::wstring& url, std::vector<BYTE>& out){
    HINTERNET hS=WinHttpOpen(L"QQFeiche/1.0 (KeleDaobao)",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);
    if(!hS) return false;
    WinHttpSetTimeouts(hS,3000,3000,4000,4000);
    bool ok=false;
    URL_COMPONENTS uc; ZeroMemory(&uc,sizeof(uc)); uc.dwStructSize=sizeof(uc);
    wchar_t host[256]={0}, path[4096]={0}, extra[4096]={0};
    uc.lpszHostName=host; uc.dwHostNameLength=256;
    uc.lpszUrlPath=path;  uc.dwUrlPathLength=4096;
    uc.lpszExtraInfo=extra; uc.dwExtraInfoLength=4096;
    if(WinHttpCrackUrl(url.c_str(),(DWORD)url.size(),0,&uc)){
        HINTERNET hC=WinHttpConnect(hS,uc.lpszHostName,uc.nPort,0);
        if(hC){
            DWORD flags=(uc.nScheme==INTERNET_SCHEME_HTTPS)?WINHTTP_FLAG_SECURE:0;
            std::wstring requestPath=std::wstring(path)+extra;
            HINTERNET hR=WinHttpOpenRequest(hC,L"GET",requestPath.c_str(),NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,flags);
            if(hR){
                DWORD prot=WINHTTP_FLAG_SECURE_PROTOCOL_TLS1|WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1|WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2|WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
                WinHttpSetOption(hR,WINHTTP_OPTION_SECURE_PROTOCOLS,&prot,sizeof(prot));
                DWORD redir=WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
                WinHttpSetOption(hR,WINHTTP_OPTION_REDIRECT_POLICY,&redir,sizeof(redir));
                if(WinHttpSendRequest(hR,WINHTTP_NO_ADDITIONAL_HEADERS,0,WINHTTP_NO_REQUEST_DATA,0,0,0)&&WinHttpReceiveResponse(hR,NULL)){
                    DWORD status=0,sz=sizeof(status);
                    WinHttpQueryHeaders(hR,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,NULL,&status,&sz,NULL);
                    if(status==200){ DWORD avail=0,read=0;
                        do{ if(!WinHttpQueryDataAvailable(hR,&avail)||avail==0)break;
                            std::vector<BYTE> buf(avail);
                            if(!WinHttpReadData(hR,buf.data(),avail,&read))break;
                            if(read>0)out.insert(out.end(),buf.data(),buf.data()+read);
                        }while(read>0); ok=true; }
                }
                WinHttpCloseHandle(hR);
            }
            WinHttpCloseHandle(hC);
        }
    }
    WinHttpCloseHandle(hS);
    return ok;
}

// ---------------- 最小 JSON ----------------
struct Json{
    enum T{NUL,B,NUM,STR,ARR,OBJ} t=NUL;
    bool b=false; double n=0; std::string s;
    std::vector<Json> a; std::map<std::string,Json> o;
    const Json* get(const std::string& k)const{ auto it=o.find(k); return it==o.end()?nullptr:&it->second; }
};
static void skipWs(const char*& p){ while(*p==' '||*p=='\t'||*p=='\n'||*p=='\r')p++; }
static void appendUtf8(std::string& s,unsigned cp){
    if(cp<0x80)s+=(char)cp; else if(cp<0x800){s+=(char)(0xC0|(cp>>6));s+=(char)(0x80|(cp&63));}
    else if(cp<0x10000){s+=(char)(0xE0|(cp>>12));s+=(char)(0x80|((cp>>6)&63));s+=(char)(0x80|(cp&63));}
    else{s+=(char)(0xF0|(cp>>18));s+=(char)(0x80|((cp>>12)&63));s+=(char)(0x80|((cp>>6)&63));s+=(char)(0x80|(cp&63));}
}
static unsigned hexVal(char c){ if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return 0; }
static void parseString(const char*& p,std::string& out){
    if(*p=='"')p++;
    while(*p&&*p!='"'){
        if(*p=='\\'){ p++;
            switch(*p){
                case '"':out+='"';p++;break; case '\\':out+='\\';p++;break; case '/':out+='/';p++;break;
                case 'n':out+='\n';p++;break; case 't':out+='\t';p++;break; case 'r':out+='\r';p++;break;
                case 'b':out+='\b';p++;break; case 'f':out+='\f';p++;break;
                case 'u':{ unsigned cp=(hexVal(p[1])<<12)|(hexVal(p[2])<<8)|(hexVal(p[3])<<4)|hexVal(p[4]); p+=5;
                    if(cp>=0xD800&&cp<=0xDBFF&&p[0]=='\\'&&p[1]=='u'){ unsigned lo=(hexVal(p[2])<<12)|(hexVal(p[3])<<8)|(hexVal(p[4])<<4)|hexVal(p[5]); p+=6; cp=0x10000+((cp-0xD800)<<10)+(lo-0xDC00); }
                    appendUtf8(out,cp); break; }
                default:p++;break;
            }
        }else{ out+=*p; p++; }
    }
    if(*p=='"')p++;
}
static Json parseJson(const char*& p){
    skipWs(p); Json j;
    if(!*p)return j;
    if(*p=='{'){ j.t=Json::OBJ; p++; skipWs(p); if(*p=='}'){p++;return j;}
        while(*p){ skipWs(p); std::string key; parseString(p,key); skipWs(p); if(*p==':')p++; skipWs(p);
            j.o[key]=parseJson(p); skipWs(p);
            if(*p==','){p++;continue;} if(*p=='}'){p++;break;} } }
    else if(*p=='['){ j.t=Json::ARR; p++; skipWs(p); if(*p==']'){p++;return j;}
        while(*p){ j.a.push_back(parseJson(p)); skipWs(p);
            if(*p==','){p++;continue;} if(*p==']'){p++;break;} } }
    else if(*p=='"'){ j.t=Json::STR; parseString(p,j.s); }
    else if(*p=='t'){ j.t=Json::B;j.b=true;p+=4; }
    else if(*p=='f'){ j.t=Json::B;j.b=false;p+=5; }
    else if(*p=='n'){ j.t=Json::NUL;p+=4; }
    else { j.t=Json::NUM; const char* st=p; if(*p=='-')p++; while(isdigit((unsigned char)*p))p++;
        if(*p=='.'){p++;while(isdigit((unsigned char)*p))p++;} if(*p=='e'||*p=='E'){p++;if(*p=='+'||*p=='-')p++;while(isdigit((unsigned char)*p))p++;}
        j.n=atof(st); }
    return j;
}

// ---------------- 道具模型 & 全局状态 ----------------
struct Item{ int id=0; std::wstring name,type,sex,descr,funcs; };
static HWND g_hwnd,g_hGroup,g_hSearch,g_hMode,g_hGo,g_hPrev,g_hNext,g_hPage,g_hStat,g_hTree,g_hList,g_hDetail,g_hCopyId,g_hCopyBoth;
static HWND g_hTreeScroll,g_hListScroll;
static bool g_hovGo=false,g_hovPrev=false,g_hovNext=false,g_hovCopyId=false,g_hovCopyBoth=false;
static HINSTANCE g_inst;
static HFONT g_font,g_fontBold,g_fontSmall;
static HBRUSH g_hToolbarBrush;
static HBRUSH g_editBrush;
static HIMAGELIST g_hRowImages;
static Gdiplus::Font* g_gdFont=nullptr; static Gdiplus::Font* g_gdFontB=nullptr; static Gdiplus::Font* g_gdFontS=nullptr;

static std::vector<Item> g_items;
static std::atomic<long> g_seq{0};
static int g_total=0,g_page=1,g_pageCount=1;
static bool g_loading=false;
static std::wstring g_curType,g_curKw;
static int g_sel=-1;
static int g_cardCols=1;
static int g_copyFlashIndex=-1;
static UINT g_dpi=96;
static int g_treeItemHeight=0;
static bool g_inSizeMove=false;

static std::map<int,Image*> g_imgMem;
static Image* g_placeholder=NULL;
static std::set<int> g_imgReq;
static std::wstring g_cacheDir;
static std::mutex g_qmutex; static std::condition_variable g_qcv; static std::queue<int> g_imgQueue; static bool g_imgStop=false;

static void layout(HWND hwnd);
static LRESULT CALLBACK ScrollBarProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp);
static LRESULT CALLBACK ScrollTargetProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR uId,DWORD_PTR ref);
static int uiPx(int value){ return MulDiv(value,(int)g_dpi,96); }
static void rebuildFonts(){
    if(g_font)DeleteObject(g_font); if(g_fontBold)DeleteObject(g_fontBold); if(g_fontSmall)DeleteObject(g_fontSmall);
    if(g_gdFont)delete g_gdFont; if(g_gdFontB)delete g_gdFontB; if(g_gdFontS)delete g_gdFontS;
    g_font=CreateFontW(-uiPx(15),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Microsoft YaHei");
    g_fontBold=CreateFontW(-uiPx(15),0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Microsoft YaHei");
    g_fontSmall=CreateFontW(-uiPx(13),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Microsoft YaHei");
    g_gdFont=new Gdiplus::Font(L"Microsoft YaHei",(REAL)uiPx(14),FontStyleRegular,UnitPixel);
    g_gdFontB=new Gdiplus::Font(L"Microsoft YaHei",(REAL)uiPx(15),FontStyleBold,UnitPixel);
    g_gdFontS=new Gdiplus::Font(L"Microsoft YaHei",(REAL)uiPx(11),FontStyleRegular,UnitPixel);
}
static void setStatusText(const std::wstring& text){
    if(!g_hStat)return;
    SetWindowTextW(g_hStat,text.c_str());
    if(g_hwnd){ layout(g_hwnd); InvalidateRect(g_hStat,NULL,TRUE); }
}

struct Query{ long seq; std::wstring type,kw; int mode,page; };

// ---------------- 图片线程 ----------------
static void imageThread(){
    std::unique_lock<std::mutex> lk(g_qmutex);
    while(true){
        while(!g_imgStop&&g_imgQueue.empty())g_qcv.wait(lk);
        if(g_imgStop)return;
        int id=g_imgQueue.front(); g_imgQueue.pop(); lk.unlock();
        std::wstring path=g_cacheDir+std::to_wstring(id)+L".png";
        std::wstring idText=std::to_wstring(id);
        std::wstring url=std::wstring(IMG_BASE)+idText+L".png";
        std::vector<BYTE> bytes;
        // 使用站点图片代理优先，官方图床作为备用来源。
        if(!httpGet(L"https://www.bnnb.cn/img.php?id="+idText,bytes)||bytes.size()<=8){
            bytes.clear();
            httpGet(url,bytes);
        }
        bool looksLikeImage=bytes.size()>8&&
            ((bytes[0]==0x89&&bytes[1]==0x50&&bytes[2]==0x4E&&bytes[3]==0x47)||
             (bytes[0]==0xFF&&bytes[1]==0xD8&&bytes[2]==0xFF));
        if(looksLikeImage){
            HANDLE hf=CreateFileW(path.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
            if(hf!=INVALID_HANDLE_VALUE){ DWORD w=0; WriteFile(hf,bytes.data(),(DWORD)bytes.size(),&w,NULL); CloseHandle(hf); }
        }
        PostMessage(g_hwnd,MSG_IMG_READY,(WPARAM)id,0);
        lk.lock();
    }
}
static void requestImage(int id){
    if(id<=0)return;
    if(g_imgMem.count(id)||g_imgReq.count(id))return;
    g_imgReq.insert(id);
    std::lock_guard<std::mutex> lk(g_qmutex); g_imgQueue.push(id); g_qcv.notify_one();
}

// ---------------- 加载线程 ----------------
static void loadThread(Query q){
    std::wstring url=std::wstring(API_BASE)+L"?limit="+std::to_wstring(LIMIT);
    if(!q.kw.empty()){ std::string k=urlEncode(utf8ws_to_utf8(q.kw)); url+=std::wstring(L"&q=")+utf8ws(k)+(q.mode==0?L"&mode=fuzzy":L"&mode=exact"); }
    if(!q.type.empty()){ std::string t=urlEncode(utf8ws_to_utf8(q.type)); url+=std::wstring(L"&type=")+utf8ws(t); }
    url+=L"&sort=new&page="+std::to_wstring(q.page);
    std::vector<BYTE> raw; std::vector<Item>* items=new std::vector<Item>(); int total=0; bool ok=false;
    if(httpGet(url,raw)){
        std::string body(raw.begin(),raw.end()); const char* p=body.c_str(); Json j=parseJson(p);
        const Json* tot=j.get("total"); if(tot&&tot->t==Json::NUM)total=(int)tot->n;
        const Json* rows=j.get("rows");
        if(rows&&rows->t==Json::ARR){ for(auto& r:rows->a){ Item it;
            const Json* f;
            f=r.get("id");if(f&&f->t==Json::NUM)it.id=(int)f->n;
            f=r.get("name");if(f&&f->t==Json::STR)it.name=utf8ws(f->s);
            f=r.get("type");if(f&&f->t==Json::STR)it.type=utf8ws(f->s);
            f=r.get("sex");if(f&&f->t==Json::STR)it.sex=utf8ws(f->s);
            f=r.get("descr");if(f&&f->t==Json::STR)it.descr=utf8ws(f->s);
            f=r.get("funcs");if(f&&f->t==Json::STR)it.funcs=utf8ws(f->s);
            items->push_back(std::move(it));
            if((int)items->size()>=LIMIT)break;
        } ok=true; }
    }
    if(total<=0) total=(int)items->size();
    PostMessage(g_hwnd,MSG_LOAD_DONE,(WPARAM)q.seq,(LPARAM)(new std::pair<int,std::vector<Item>*>(total,items)));
    (void)ok;
}
static void startQuery(const std::wstring& type,const std::wstring& kw,int mode,int page){
    long seq=++g_seq; g_loading=true;
    g_copyFlashIndex=-1;
    KillTimer(g_hwnd,ID_COPY_TIMER);
    setStatusText(L"正在加载…");
    Query q{seq,type,kw,mode,page}; std::thread(loadThread,q).detach();
}

// ---------------- 分类树 ----------------
static HTREEITEM tvInsert(HWND tree,const wchar_t* text,HTREEITEM parent){
    TVINSERTSTRUCTW tvi{}; tvi.hParent=parent; tvi.hInsertAfter=TVI_LAST;
    tvi.item.mask=TVIF_TEXT; tvi.item.pszText=(LPWSTR)text;
    return TreeView_InsertItem(tree,&tvi);
}
static void buildTree(HWND tree){
    std::map<std::wstring,HTREEITEM> groups;
    tvInsert(tree,L"最新道具",TVI_ROOT);
    std::vector<const CCategory*> racing;
    std::vector<const CCategory*> other;
    for(const auto& c:g_cats){
        if(!c.fine[0]){ tvInsert(tree,L"最新道具",TVI_ROOT); continue; }
        (std::wcscmp(c.group,L"赛车")==0?racing:other).push_back(&c);
    }
    static const wchar_t* racingOrder[]={
        L"T3机甲赛车",L"T2机甲赛车",L"T1机甲赛车",L"S级赛车",L"M2级摩托",L"M1级摩托",
        L"A级赛车",L"B级赛车",L"C级赛车",L"D级赛车",L"R级赛车",L"L3级滑板",L"L2级滑板",
        L"L1级滑板",L"L0级滑板",L"K2级轮滑",L"K1级轮滑",L"K0级轮滑"
    };
    auto racingRank=[](const wchar_t* fine){
        for(size_t i=0;i<sizeof(racingOrder)/sizeof(racingOrder[0]);++i)
            if(std::wcscmp(fine,racingOrder[i])==0)return (int)i;
        return 1000;
    };
    std::stable_sort(racing.begin(),racing.end(),[&](const CCategory* a,const CCategory* b){
        int ar=racingRank(a->fine),br=racingRank(b->fine);
        return ar==br?std::wcscmp(a->fine,b->fine)<0:ar<br;
    });
    auto appendGroup=[&](const std::vector<const CCategory*>& cats){
        for(const auto* c:cats){
            if(groups.count(c->group)==0){ std::wstring gl=std::wstring(c->group); groups[c->group]=tvInsert(tree,gl.c_str(),TVI_ROOT); }
            std::wstring fl=std::wstring(c->fine)+L"  ("+std::to_wstring(c->count)+L")";
            tvInsert(tree,fl.c_str(),groups[c->group]);
        }
    };
    appendGroup(racing); appendGroup(other);
    auto it=groups.find(L"赛车");
    if(it!=groups.end())TreeView_Expand(tree,it->second,TVE_EXPAND);
}
static std::wstring treeToType(HTREEITEM item){
    wchar_t buf[128]; TVITEMW tvi{}; tvi.hItem=item; tvi.mask=TVIF_TEXT; tvi.pszText=buf; tvi.cchTextMax=128;
    if(!TreeView_GetItem(g_hTree,&tvi))return L"";
    std::wstring s(buf);
    if(s==L"最新道具")return L"";
    size_t pos=s.find(L"  ("); if(pos!=std::wstring::npos)s=s.substr(0,pos);
    pos=s.find(L" ▸"); if(pos!=std::wstring::npos)s=s.substr(0,pos);
    return s;
}

// ---------------- GDI+ 图片 ----------------
static Image* loadPlaceholder(){
    if(g_placeholder)return g_placeholder;
    HRSRC hr=FindResourceW(NULL,MAKEINTRESOURCEW(101),L"PNG");
    if(!hr)return NULL;
    HGLOBAL hg=LoadResource(NULL,hr);
    if(!hg)return NULL;
    void* data=LockResource(hg);
    DWORD sz=SizeofResource(NULL,hr);
    IStream* st=NULL;
    if(CreateStreamOnHGlobal(NULL,TRUE,&st)!=S_OK)return NULL;
    ULONG wr=0; st->Write(data,sz,&wr);
    LARGE_INTEGER li{}; li.QuadPart=0; st->Seek(li,STREAM_SEEK_SET,NULL);
    g_placeholder=Image::FromStream(st,FALSE);
    st->Release();
    return g_placeholder;
}
static Image* loadImage(int id){
    auto it=g_imgMem.find(id); if(it!=g_imgMem.end())return it->second;
    std::wstring path=g_cacheDir+std::to_wstring(id)+L".png";
    if(GetFileAttributesW(path.c_str())==INVALID_FILE_ATTRIBUTES)return NULL;
    Image* img=Image::FromFile(path.c_str(),FALSE);
    if(img&&img->GetLastStatus()!=Ok){ delete img; return NULL; }
    g_imgMem[id]=img; return img;
}
static Color colorFromRef(COLORREF c){
    return Color(GetRValue(c),GetGValue(c),GetBValue(c));
}
static void drawThumb(Graphics& g,int id,RECT rc,int radius=6){
    (void)radius;
    Image* img=loadImage(id);
    if(!img){
        Image* ph=loadPlaceholder();
        if(ph){
            int tw=rc.right-rc.left,th=rc.bottom-rc.top;
            int pw=ph->GetWidth(),pih=ph->GetHeight();
            float sc=(float)std::min((double)tw/(double)pw,(double)th/(double)pih);
            int dw=(int)(pw*sc),dh=(int)(pih*sc);
            int dx=rc.left+(tw-dw)/2,dy=rc.top+(th-dh)/2;
            Gdiplus::SolidBrush bg(Color(255,255,255));
            g.FillRectangle(&bg,(REAL)rc.left,(REAL)rc.top,(REAL)tw,(REAL)th);
            g.DrawImage(ph,dx,dy,dw,dh);
        }else{
            Gdiplus::SolidBrush bg(Color(255,239,231));
            g.FillRectangle(&bg,(REAL)rc.left,(REAL)rc.top,(REAL)(rc.right-rc.left),(REAL)(rc.bottom-rc.top));
            Gdiplus::SolidBrush bubble(Color(213,43,43));
            int d=std::min(rc.right-rc.left,rc.bottom-rc.top)-12;
            g.FillEllipse(&bubble,rc.left+6,rc.top+6,d,d);
            if(g_gdFontS){ SolidBrush white(Color(255,255,255));
                g.DrawString(L"图",1,g_gdFontS,PointF((REAL)(rc.left+d/2),(REAL)(rc.top+d/2-7)),&white); }
        }
        requestImage(id); return;
    }
    int w=img->GetWidth(),h=img->GetHeight();
    int tw=rc.right-rc.left,th=rc.bottom-rc.top;
    float sc=(float)std::min((double)tw/(double)w,(double)th/(double)h);
    int dw=(int)(w*sc),dh=(int)(h*sc);
    int dx=rc.left+(tw-dw)/2,dy=rc.top+(th-dh)/2;
    g.DrawImage(img,dx,dy,dw,dh);
}

// ---------------- 详情面板 ----------------
static void paintDetail(HWND hwnd){
    PAINTSTRUCT ps; HDC dc=BeginPaint(hwnd,&ps);
    Graphics g(dc); g.SetSmoothingMode(SmoothingModeAntiAlias);
    RECT cr; GetClientRect(hwnd,&cr);
    SolidBrush bg(Color(255,255,255)); g.FillRectangle(&bg,0,0,cr.right,cr.bottom);
    int selected=g_sel;
    if(selected<0||selected>=(int)g_items.size()){
        SolidBrush t(Color(150,120,110));
        g.DrawString(L"请在左侧选择分类，点击列表中的道具查看代码与图片",-1,g_gdFontS,PointF(24,24),&t);
        EndPaint(hwnd,&ps); return;
    }
    Item& it=g_items[selected];
    RECT irc{20,20,20+118,20+118};
    drawThumb(g,it.id,irc,8);
    SolidBrush black(Color(30,36,50));
    g.DrawString(it.name.c_str(),(INT)it.name.size(),g_gdFontB,PointF(154,20),&black);
    std::wstring idTxt=L"道具 ID   "+std::to_wstring(it.id);
    SolidBrush blue(Color(183,35,35));
    g.DrawString(idTxt.c_str(),(INT)idTxt.size(),g_gdFont,PointF(154,52),&blue);
    std::wstring tt=L"类型  "+it.type+L"       性别  "+it.sex;
    SolidBrush gray2(Color(96,106,122));
    g.DrawString(tt.c_str(),(INT)tt.size(),g_gdFont,PointF(154,78),&gray2);
    std::wstring d=L"描述：  "+it.descr;
    SolidBrush gray(Color(84,92,108));
    g.DrawString(d.c_str(),(INT)d.size(),g_gdFontS,PointF(20,140),&gray);
    std::wstring fn=L"功能：  "+it.funcs;
    g.DrawString(fn.c_str(),(INT)fn.size(),g_gdFontS,PointF(20,166),&gray);
    EndPaint(hwnd,&ps);
}

// ---------------- 列表自绘 ----------------
static void addRoundRect(Gdiplus::GraphicsPath& gp,float x,float y,float w,float h,float r);
static void paintRow(HDC dc,int rowIndex,RECT rc){
    int rowWidth=rc.right-rc.left,rowHeight=rc.bottom-rc.top;
    HDC mdc=CreateCompatibleDC(dc); HBITMAP mb=CreateCompatibleBitmap(dc,rowWidth,rowHeight);
    HGDIOBJ ob=SelectObject(mdc,mb);
    RECT canvas{0,0,rowWidth,rowHeight};
    HBRUSH bg=CreateSolidBrush(RGB(255,249,246)); FillRect(mdc,&canvas,bg); DeleteObject(bg);
    Graphics g(mdc); g.SetSmoothingMode(SmoothingModeAntiAlias);
    int gap=8;
    int cardWidth=(rowWidth-gap*(g_cardCols+1))/g_cardCols;
    for(int col=0;col<g_cardCols;++col){
        int itemIndex=rowIndex*g_cardCols+col;
        if(itemIndex>=(int)g_items.size())break;
        Item& it=g_items[itemIndex];
        int left=gap+col*(cardWidth+gap);
        RECT card{left,4,left+cardWidth,rowHeight-4};
        bool selected=(g_sel==itemIndex);
        GraphicsPath cardPath;
        addRoundRect(cardPath,(REAL)card.left,(REAL)card.top,(REAL)(card.right-card.left),(REAL)(card.bottom-card.top),8);
        SolidBrush cardBg(selected?Color(255,239,231):Color(255,255,255));
        g.FillPath(&cardBg,&cardPath);
        Pen cardBorder(selected?Color(213,43,43):Color(239,222,214),1.0f);
        g.DrawPath(&cardBorder,&cardPath);

        int imageSize=std::min(78,cardWidth-24);
        int imageLeft=card.left+(cardWidth-imageSize)/2;
        RECT imageRect{imageLeft,card.top+7,imageLeft+imageSize,card.top+7+imageSize};
        drawThumb(g,it.id,imageRect,8);

        SolidBrush label(Color(111,91,84));
        SolidBrush value(Color(42,39,38));
        SolidBrush idColor(Color(176,35,35));
        StringFormat format;
        format.SetTrimming(StringTrimmingEllipsisCharacter);
        RectF idRect((REAL)(card.left+10),(REAL)(card.top+94),(REAL)(cardWidth-76),22.0f);
        std::wstring idText=L"ID "+std::to_wstring(it.id);
        g.DrawString(idText.c_str(),(INT)idText.size(),g_gdFontB,idRect,&format,&idColor);

        RECT copyRect{card.right-64,card.top+91,card.right-8,card.top+116};
        GraphicsPath copyPath;
        float cRadius=6.0f;
        addRoundRect(copyPath,(REAL)copyRect.left,(REAL)copyRect.top,(REAL)(copyRect.right-copyRect.left),(REAL)(copyRect.bottom-copyRect.top),cRadius);
        // 柔和蓝灰渐变，降低视觉刺激并与详情区复制按钮保持一致。
        const bool copied=(g_copyFlashIndex==itemIndex);
        const Color copyTop=copied?Color(255,133,186,157):Color(255,143,177,183);
        const Color copyBottom=copied?Color(255,76,145,110):Color(255,91,128,137);
        LinearGradientBrush copyBg(RectF((REAL)copyRect.left,(REAL)copyRect.top,(REAL)(copyRect.right-copyRect.left),(REAL)(copyRect.bottom-copyRect.top)),
            copyTop,copyBottom,LinearGradientModeVertical);
        g.FillPath(&copyBg,&copyPath);
        // 圆角内顶部细高光条，提升质感
        GraphicsPath copyGlow;
        addRoundRect(copyGlow,(REAL)copyRect.left+2,(REAL)copyRect.top+2,(REAL)(copyRect.right-copyRect.left-4),(REAL)((copyRect.bottom-copyRect.top)*0.42f),cRadius-2);
        SolidBrush glowBr(Color(45,255,255,255)); g.FillPath(&glowBr,&copyGlow);
        // 低对比度边框与白字，保持按钮清晰但不刺眼。
        Pen copyBorder(copied?Color(255,58,123,91):Color(255,65,103,112),1.0f); g.DrawPath(&copyBorder,&copyPath);
        SolidBrush copyText(Color(255,255,255,255));
        StringFormat centered; centered.SetAlignment(StringAlignmentCenter); centered.SetLineAlignment(StringAlignmentCenter);
        RectF copyTextRect((REAL)copyRect.left,(REAL)copyRect.top,(REAL)(copyRect.right-copyRect.left),(REAL)(copyRect.bottom-copyRect.top));
        const wchar_t* copyLabel=copied?L"已复制":L"复制";
        g.DrawString(copyLabel,-1,g_gdFontS,copyTextRect,&centered,&copyText);

        RectF nameLabelRect((REAL)(card.left+10),(REAL)(card.top+122),34.0f,21.0f);
        RectF nameRect((REAL)(card.left+46),(REAL)(card.top+122),(REAL)(cardWidth-56),21.0f);
        g.DrawString(L"名称",-1,g_gdFontS,nameLabelRect,&format,&label);
        g.DrawString(it.name.c_str(),(INT)it.name.size(),g_gdFont,nameRect,&format,&value);

        std::wstring typeText=it.type;
        for(wchar_t& ch:typeText)if(ch==L',')ch=L'、';
        RectF typeLabelRect((REAL)(card.left+10),(REAL)(card.top+148),54.0f,21.0f);
        RectF typeRect((REAL)(card.left+66),(REAL)(card.top+148),(REAL)(cardWidth-76),21.0f);
        g.DrawString(L"道具类型",-1,g_gdFontS,typeLabelRect,&format,&label);
        g.DrawString(typeText.c_str(),(INT)typeText.size(),g_gdFontS,typeRect,&format,&value);
    }
    BitBlt(dc,rc.left,rc.top,rowWidth,rowHeight,mdc,0,0,SRCCOPY);
    SelectObject(mdc,ob);
    DeleteObject(mb); DeleteDC(mdc);
}

// ---------------- 自绘按钮 ----------------
static void addRoundRect(GraphicsPath& gp,float x,float y,float w,float h,float r){
    float d=r*2;
    gp.AddArc(x,y,d,d,180,90);
    gp.AddArc(x+w-d,y,d,d,270,90);
    gp.AddArc(x+w-d,y+h-d,d,d,0,90);
    gp.AddArc(x,y+h-d,d,d,90,90);
    gp.CloseFigure();
}
static bool btnHover(HWND btn){
    RECT r; POINT p; GetWindowRect(btn,&r); GetCursorPos(&p);
    return (p.x>=r.left&&p.x<=r.right&&p.y>=r.top&&p.y<=r.bottom);
}
static bool btnDown(HWND btn){ return btnHover(btn)&&(GetAsyncKeyState(VK_LBUTTON)&0x8000); }
static void drawButton(HDC dc,RECT r,HWND btn,bool primary){
    (void)primary;
    bool hover=btnHover(btn), down=btnDown(btn);
    int bw=r.right-r.left,bh=r.bottom-r.top;
    if(bw<=0||bh<=0)return;
    HDC mdc=CreateCompatibleDC(dc);
    HBITMAP mb=CreateCompatibleBitmap(dc,bw,bh);
    HBITMAP ob=(HBITMAP)SelectObject(mdc,mb);
    Graphics g(mdc); g.SetSmoothingMode(SmoothingModeAntiAlias);
    // Compatible bitmap 默认为黑色；先铺工具栏底色，避免半透明阴影在圆角外形成黑边。
    g.Clear(Color(255,255,247,241));
    RectF rr((REAL)0,(REAL)0,(REAL)bw,(REAL)bh);
    float radius=(float)std::min(10,std::max(4,bh/2));
    GraphicsPath gp; addRoundRect(gp,rr.X,rr.Y,rr.Width,rr.Height,radius);
    int id=GetDlgCtrlID(btn);
    // 顶部操作按钮保留各自的主题色；复制按钮使用柔和蓝灰，避免高饱和红色。
    Color t1,t2,tc;
    if(id==ID_GO){ t1=Color(255,122,120,255); t2=Color(255,76,106,255); tc=Color(255,255,255); }
    else if(id==ID_PREV){ t1=Color(255,98,205,186); t2=Color(255,56,170,152); tc=Color(255,255,255); }
    else if(id==ID_NEXT){ t1=Color(255,255,186,100); t2=Color(255,241,148,56); tc=Color(255,255,255); }
    else if(id==ID_COPY_ID||id==ID_COPY_BOTH){ t1=Color(255,143,177,183); t2=Color(255,91,128,137); tc=Color(255,255,255); }
    else { t1=Color(255,150,150,235); t2=Color(255,118,118,222); tc=Color(255,255,255); }
    auto adj=[=](Color c,int d){ return Color(255,(BYTE)std::max(0,std::min(255,(int)c.GetRed()+d)),(BYTE)std::max(0,std::min(255,(int)c.GetGreen()+d)),(BYTE)std::max(0,std::min(255,(int)c.GetBlue()+d))); };
    if(down){ t1=adj(t1,-24); t2=adj(t2,-24); }
    else if(hover){ t1=adj(t1,16); t2=adj(t2,16); }
    GraphicsPath shadow; addRoundRect(shadow,1.0f,2.0f,(REAL)bw-2.0f,(REAL)bh-2.0f,radius);
    SolidBrush shadowBrush(Color(42,80,53,70)); g.FillPath(&shadowBrush,&shadow);
    LinearGradientBrush sb(RectF(rr),t1,t2,LinearGradientModeVertical);
    g.FillPath(&sb,&gp);
    Pen border(down?Color(255,112,51,78):Color(190,255,255,255),1.0f); g.DrawPath(&border,&gp);
    GraphicsPath gloss; addRoundRect(gloss,1.5f,1.5f,(REAL)bw-3.0f,(REAL)std::max(4,bh/2-1),radius-1.0f);
    SolidBrush glossBrush(Color(38,255,255,255)); g.FillPath(&glossBrush,&gloss);
    // 文字
    wchar_t buf[64]; GetWindowTextW(btn,buf,64);
    SolidBrush tbs(tc);
    StringFormat sf; sf.SetAlignment(StringAlignmentCenter); sf.SetLineAlignment(StringAlignmentCenter); sf.SetTrimming(StringTrimmingEllipsisCharacter);
    RectF textRect(3.0f,1.0f,(REAL)bw-6.0f,(REAL)bh-2.0f);
    g.DrawString(buf,-1,g_gdFontS,textRect,&sf,&tbs);
    BitBlt(dc,r.left,r.top,bw,bh,mdc,0,0,SRCCOPY);
    SelectObject(mdc,ob); DeleteObject(mb); DeleteDC(mdc);
}

// 搜索框回车键直接触发搜索
static LRESULT CALLBACK SearchEditProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR uId,DWORD_PTR ref){
    static DWORD lastEnterTick=0;
    // 回车 = 按搜索按钮；吞掉随后产生的 WM_CHAR，避免一次回车触发两次请求。
    if(msg==WM_KEYDOWN&&wp==VK_RETURN){
        lastEnterTick=GetTickCount();
        if(g_hGo) SendMessage(g_hGo,BM_CLICK,0,0);
        return 0;
    }
    if(msg==WM_CHAR&&wp==VK_RETURN)return 0;
    // 输入法可能吞掉 WM_KEYDOWN，组合结束时补发一次；时间窗避免和普通回车重复。
    if(msg==WM_IME_ENDCOMPOSITION&&GetTickCount()-lastEnterTick>180){
        wchar_t text[256]={0}; GetWindowTextW(hwnd,text,256);
        if(text[0]&&g_hGo)SendMessage(g_hGo,BM_CLICK,0,0);
    }
    if(msg==WM_GETDLGCODE)return DefSubclassProc(hwnd,msg,wp,lp)|DLGC_WANTALLKEYS;
    if(msg==WM_NCDESTROY){ RemoveWindowSubclass(hwnd,SearchEditProc,uId); }
    return DefSubclassProc(hwnd,msg,wp,lp);
}
static void paintModeCombo(HWND hwnd,HDC dc){
    RECT rc; GetClientRect(hwnd,&rc); int w=rc.right-rc.left,h=rc.bottom-rc.top;
    Graphics g(dc); g.SetSmoothingMode(SmoothingModeAntiAlias); g.Clear(Color(255,255,247,241));
    GraphicsPath path; addRoundRect(path,1.0f,1.0f,(REAL)std::max(2,w-2),(REAL)std::max(2,h-2),(REAL)uiPx(8));
    SolidBrush fill(Color(255,255,255,255)); g.FillPath(&fill,&path);
    Pen edge(Color(255,190,198,218),(REAL)uiPx(1)); g.DrawPath(&edge,&path);
    int sel=(int)SendMessageW(hwnd,CB_GETCURSEL,0,0); wchar_t text[128]={0};
    if(sel>=0)SendMessageW(hwnd,CB_GETLBTEXT,sel,(LPARAM)text);
    RectF tr((REAL)uiPx(9),0.0f,(REAL)std::max(2,w-uiPx(31)),(REAL)h);
    StringFormat fmt; fmt.SetAlignment(StringAlignmentNear); fmt.SetLineAlignment(StringAlignmentCenter); fmt.SetTrimming(StringTrimmingEllipsisCharacter); fmt.SetFormatFlags(StringFormatFlagsNoWrap);
    SolidBrush ink(Color(255,55,62,78)); g.DrawString(text,-1,g_gdFont,tr,&fmt,&ink);
    PointF tri[3]={{(REAL)(w-uiPx(17)),(REAL)(h/2-uiPx(2))},{(REAL)(w-uiPx(9)),(REAL)(h/2-uiPx(2))},{(REAL)(w-uiPx(13)),(REAL)(h/2+uiPx(3))}};
    SolidBrush arrow(Color(255,87,105,132)); g.FillPolygon(&arrow,tri,3);
}
static LRESULT CALLBACK ModeComboProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR uId,DWORD_PTR ref){
    if(msg==WM_ERASEBKGND)return 1;
    if(msg==WM_PAINT){
        PAINTSTRUCT ps; HDC dc=BeginPaint(hwnd,&ps); paintModeCombo(hwnd,dc); EndPaint(hwnd,&ps); return 0;
    }
    if(msg==WM_NCDESTROY)RemoveWindowSubclass(hwnd,ModeComboProc,uId);
    return DefSubclassProc(hwnd,msg,wp,lp);
}


// 搜索框/下拉框圆角：父窗口 WM_PAINT 绘制背景 + 控件透明背景（见 WM_PAINT / WM_CTLCOLOREDIT）
// 滚动条：使用系统 Explorer 主题窄条（自动跟随 Windows DPI，不与系统条重叠）




// ---------------- 剪贴板 ----------------
static bool copyText(const std::wstring& t){
    if(!OpenClipboard(g_hwnd))return false;
    EmptyClipboard();
    size_t bytes=(t.size()+1)*sizeof(wchar_t);
    HGLOBAL hg=GlobalAlloc(GMEM_MOVEABLE,bytes);
    bool copied=false;
    if(hg){
        wchar_t* dst=(wchar_t*)GlobalLock(hg);
        if(dst){
            wcscpy_s(dst,t.size()+1,t.c_str()); GlobalUnlock(hg);
            copied=SetClipboardData(CF_UNICODETEXT,hg)!=NULL;
            if(copied)hg=NULL;
        }
        if(hg)GlobalFree(hg);
    }
    CloseClipboard();
    return copied;
}

// ---------------- 统一滚动条 ----------------
static HWND scrollTarget(HWND bar){
    return (HWND)GetWindowLongPtrW(bar,GWLP_USERDATA);
}
static bool scrollMetrics(HWND bar,int& thumbTop,int& thumbHeight,int& trackTop,int& trackHeight,SCROLLINFO& si){
    HWND target=scrollTarget(bar);
    if(!target)return false;
    si.cbSize=sizeof(si); si.fMask=SIF_RANGE|SIF_PAGE|SIF_POS|SIF_TRACKPOS;
    if(!GetScrollInfo(target,SB_VERT,&si))return false;
    RECT rc; GetClientRect(bar,&rc);
    trackTop=5; trackHeight=std::max(1,(int)(rc.bottom-rc.top)-10);
    int logicalRange=si.nMax-std::max(1,(int)si.nPage-1);
    if(logicalRange<=0||si.nPage<=0)return false;
    thumbHeight=std::max(24,(int)((long long)trackHeight*si.nPage/(si.nMax+1)));
    thumbHeight=std::min(thumbHeight,trackHeight);
    int travel=std::max(0,trackHeight-thumbHeight);
    int pos=std::max(si.nMin,std::min(si.nPos,si.nMax-std::max(1,(int)si.nPage-1)));
    thumbTop=trackTop+(travel>0?(int)((long long)(pos-si.nMin)*travel/logicalRange):0);
    return true;
}
static void scrollInvalidate(HWND target){
    HWND bar=target==g_hTree?g_hTreeScroll:(target==g_hList?g_hListScroll:NULL);
    if(bar)InvalidateRect(bar,NULL,FALSE);
}
static void paintScrollBar(HWND hwnd,HDC dc){
    RECT rc; GetClientRect(hwnd,&rc);
    HBRUSH bg=CreateSolidBrush(RGB(249,244,241)); FillRect(dc,&rc,bg); DeleteObject(bg);
    int top=0,height=0,trackTop=0,trackHeight=0; SCROLLINFO si{};
    if(!scrollMetrics(hwnd,top,height,trackTop,trackHeight,si))return;
    Graphics g(dc); g.SetSmoothingMode(SmoothingModeAntiAlias);
    GraphicsPath track; addRoundRect(track,2.0f,(REAL)trackTop,(REAL)(rc.right-rc.left-4),(REAL)trackHeight,4.0f);
    SolidBrush trackBrush(Color(255,238,231,226)); g.FillPath(&trackBrush,&track);
    GraphicsPath thumb; addRoundRect(thumb,2.0f,(REAL)top,(REAL)(rc.right-rc.left-4),(REAL)height,4.0f);
    SolidBrush thumbBrush(Color(255,205,107,91)); g.FillPath(&thumbBrush,&thumb);
    Pen thumbBorder(Color(255,184,83,69),1.0f); g.DrawPath(&thumbBorder,&thumb);
}
static LRESULT CALLBACK ScrollBarProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    static std::map<HWND,int> dragOffset;
    switch(msg){
    case WM_NCCREATE:{
        CREATESTRUCTW* cs=(CREATESTRUCTW*)lp;
        SetWindowLongPtrW(hwnd,GWLP_USERDATA,(LONG_PTR)cs->lpCreateParams);
        return TRUE;
    }
    case WM_ERASEBKGND:return 1;
    case WM_PAINT:{
        PAINTSTRUCT ps; HDC dc=BeginPaint(hwnd,&ps); paintScrollBar(hwnd,dc); EndPaint(hwnd,&ps); return 0;
    }
    case WM_LBUTTONDOWN:{
        HWND target=scrollTarget(hwnd); if(!target)return 0;
        int top=0,height=0,trackTop=0,trackHeight=0; SCROLLINFO si{};
        POINT pt{(int)(short)LOWORD(lp),(int)(short)HIWORD(lp)};
        if(!scrollMetrics(hwnd,top,height,trackTop,trackHeight,si))return 0;
        if(pt.y>=top&&pt.y<=top+height){
            dragOffset[hwnd]=pt.y-top; SetCapture(hwnd); return 0;
        }
        int code=pt.y<top?SB_PAGEUP:SB_PAGEDOWN;
        SendMessageW(target,WM_VSCROLL,MAKEWPARAM(code,0),(LPARAM)hwnd); scrollInvalidate(target); return 0;
    }
    case WM_MOUSEMOVE:{
        auto it=dragOffset.find(hwnd); if(it==dragOffset.end()||(GetCapture()!=hwnd))break;
        HWND target=scrollTarget(hwnd); if(!target)break;
        int top=0,height=0,trackTop=0,trackHeight=0; SCROLLINFO si{};
        if(!scrollMetrics(hwnd,top,height,trackTop,trackHeight,si))break;
        int y=(int)(short)HIWORD(lp)-it->second;
        int travel=std::max(1,trackHeight-height);
        int logicalRange=si.nMax-std::max(1,(int)si.nPage-1);
        int pos=si.nMin+(int)((long long)std::max(0,std::min(travel,y-trackTop))*logicalRange/travel);
        SendMessageW(target,WM_VSCROLL,MAKEWPARAM(SB_THUMBTRACK,pos),(LPARAM)hwnd); scrollInvalidate(target); return 0;
    }
    case WM_LBUTTONUP:{
        auto it=dragOffset.find(hwnd); if(it!=dragOffset.end()){
            HWND target=scrollTarget(hwnd); if(target)SendMessageW(target,WM_VSCROLL,MAKEWPARAM(SB_ENDSCROLL,0),(LPARAM)hwnd);
            dragOffset.erase(it); if(GetCapture()==hwnd)ReleaseCapture(); if(target)scrollInvalidate(target);
        }
        return 0;
    }
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
static LRESULT CALLBACK ScrollTargetProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR uId,DWORD_PTR ref){
    LRESULT result=DefSubclassProc(hwnd,msg,wp,lp);
    if(msg==WM_VSCROLL||msg==WM_MOUSEWHEEL||msg==WM_KEYDOWN||msg==WM_LBUTTONUP||msg==WM_SIZE||msg==WM_PAINT)
        scrollInvalidate(hwnd);
    if(msg==WM_NCDESTROY)RemoveWindowSubclass(hwnd,ScrollTargetProc,uId);
    return result;
}

// ---------------- 布局 ----------------
static bool moveChildIfChanged(HWND parent,HWND child,int x,int y,int w,int h,BOOL repaint){
    if(!child)return false;
    RECT current{}; GetWindowRect(child,&current);
    POINT origin{current.left,current.top}; ScreenToClient(parent,&origin);
    if(origin.x==x&&origin.y==y&&current.right-current.left==w&&current.bottom-current.top==h)return false;
    MoveWindow(child,x,y,w,h,repaint);
    return true;
}
static void populateListRows(){
    if(!g_hList)return;
    ListView_DeleteAllItems(g_hList);
    int rowCount=((int)g_items.size()+g_cardCols-1)/g_cardCols;
    for(int i=0;i<rowCount;++i){
        LVITEMW row{}; row.mask=LVIF_TEXT; row.iItem=i; row.pszText=(LPWSTR)L"";
        ListView_InsertItem(g_hList,&row);
    }
    InvalidateRect(g_hList,NULL,TRUE);
    scrollInvalidate(g_hList);
}
static int windowTextWidth(HWND hwnd,HFONT font){
    if(!hwnd)return 0;
    wchar_t text[512]={0}; GetWindowTextW(hwnd,text,(int)(sizeof(text)/sizeof(text[0])));
    HDC dc=GetDC(hwnd); if(!dc)return 0;
    HFONT old=(HFONT)SelectObject(dc,font?font:g_fontSmall); SIZE sz{};
    GetTextExtentPoint32W(dc,text,(int)wcslen(text),&sz);
    SelectObject(dc,old); ReleaseDC(hwnd,dc); return sz.cx;
}
static void paintStatBadge(HDC dc,RECT rc){
    Graphics g(dc); g.SetSmoothingMode(SmoothingModeAntiAlias);
    GraphicsPath path; addRoundRect(path,1.0f,1.0f,(REAL)(rc.right-rc.left-2),(REAL)(rc.bottom-rc.top-2),11.0f);
    Color bg=g_loading?Color(255,255,247,228):Color(255,239,247,244);
    Color border=g_loading?Color(255,242,193,113):Color(255,177,218,205);
    Color text=g_loading?Color(255,166,104,32):Color(255,35,116,95);
    SolidBrush fill(bg); g.FillPath(&fill,&path); Pen edge(border,1.0f); g.DrawPath(&edge,&path);
    wchar_t buf[512]={0}; GetWindowTextW(g_hStat,buf,(int)(sizeof(buf)/sizeof(buf[0])));
    RectF textRect((REAL)(rc.left+13),(REAL)rc.top,(REAL)(rc.right-rc.left-21),(REAL)(rc.bottom-rc.top));
    StringFormat fmt; fmt.SetAlignment(StringAlignmentNear); fmt.SetLineAlignment(StringAlignmentCenter); fmt.SetTrimming(StringTrimmingEllipsisCharacter);
    SolidBrush ink(text); g.DrawString(buf,-1,g_gdFontS,textRect,&fmt,&ink);
    SolidBrush dot(text); g.FillEllipse(&dot,(REAL)(rc.left+7),(REAL)(rc.top+10),4.0f,4.0f);
}
static void paintHeaderDecor(HWND hwnd,HDC dc,RECT cr){
    Graphics g(dc); g.SetSmoothingMode(SmoothingModeAntiAlias);
    const wchar_t* title=L"QQ交流群  782834246";
    // 标题使用固定的独立区域，窄窗口时分页/搜索会换到下一行，避免任何控件覆盖标题。
    StringFormat measureFmt; RectF measured;
    g.MeasureString(title,-1,g_gdFontB,PointF(0,0),&measureFmt,&measured);
    float pillW=std::max((float)uiPx(210),std::min((float)uiPx(240),measured.Width+(float)uiPx(38)));
    pillW=std::min(pillW,(float)std::max(1,(int)cr.right-uiPx(16)));
    float pillH=(float)uiPx(26), pillX=((float)cr.right-pillW)/2.0f, pillY=(float)uiPx(7);
    GraphicsPath pill; addRoundRect(pill,pillX,pillY,pillW,pillH,(float)uiPx(13));
    SolidBrush pillBg(Color(255,255,249,244)); g.FillPath(&pillBg,&pill);
    Pen pillBorder(Color(255,239,207,192),1.0f); g.DrawPath(&pillBorder,&pill);
    RectF tr(pillX+(float)uiPx(10),pillY,pillW-(float)uiPx(20),pillH);
    StringFormat centered; centered.SetAlignment(StringAlignmentCenter); centered.SetLineAlignment(StringAlignmentCenter); centered.SetTrimming(StringTrimmingEllipsisCharacter);
    SolidBrush red(Color(255,194,56,44)); g.DrawString(title,-1,g_gdFontB,tr,&centered,&red);
    // 统一绘制输入控件的圆角外框，子控件只负责文字和交互。
    HWND fields[2]={g_hSearch,g_hMode};
    for(HWND field:fields)if(field&&IsWindowVisible(field)){
        RECT er; GetWindowRect(field,&er); POINT lt{er.left,er.top}; ScreenToClient(hwnd,&lt);
        RECT box{lt.x-uiPx(3),lt.y-uiPx(2),lt.x+(er.right-er.left)+uiPx(3),lt.y+(er.bottom-er.top)+uiPx(2)};
        GraphicsPath fp; addRoundRect(fp,(REAL)box.left,(REAL)box.top,(REAL)(box.right-box.left),(REAL)(box.bottom-box.top),(REAL)uiPx(9));
        SolidBrush fill(Color(255,255,255,255)); g.FillPath(&fill,&fp);
        Pen border(Color(255,206,212,229),1.0f); g.DrawPath(&border,&fp);
    }
}
static void layout(HWND hwnd){
    RECT cr; GetClientRect(hwnd,&cr);
    int w=cr.right,h=cr.bottom;

    // 预留标题和三组分页控件的独立空间；中等窗口也使用三行紧凑布局。
    const bool compact=(w<uiPx(1000));
    const int titleLeft=std::max(0,(w-uiPx(210))/2);
    const bool narrowHeader=compact&&titleLeft<uiPx(120);
    const int toolbarH=uiPx(narrowHeader?176:(compact?142:TBH));
    const int treeW=std::max(uiPx(130),std::min(uiPx(TREEW),w-uiPx(205)));
    int treeItemHeight=uiPx(30);
    if(g_hTree&&g_treeItemHeight!=treeItemHeight){
        TreeView_SetItemHeight(g_hTree,treeItemHeight);
        g_treeItemHeight=treeItemHeight;
    }
    int prevW=std::max(uiPx(compact?66:82),windowTextWidth(g_hPrev,g_fontSmall)+uiPx(22));
    int nextW=std::max(uiPx(compact?66:82),windowTextWidth(g_hNext,g_fontSmall)+uiPx(22));
    int pageW=std::max(uiPx(compact?92:130),windowTextWidth(g_hPage,g_fontSmall)+uiPx(16));
    const int nextX=std::max(uiPx(8),w-nextW-uiPx(8)), pageX=std::max(uiPx(8),nextX-pageW-uiPx(8)), prevX=std::max(uiPx(8),pageX-prevW-uiPx(8));
    int statDesired=windowTextWidth(g_hStat,g_fontSmall)+uiPx(30);
    int statW=compact?std::max(uiPx(96),std::min(statDesired,std::max(uiPx(96),titleLeft-uiPx(16))))
                     :std::max(uiPx(96),std::min(statDesired,std::max(uiPx(96),prevX-uiPx(22))));
    const int statY=narrowHeader?uiPx(38):uiPx(7);
    const int topRowY=narrowHeader?uiPx(70):(compact?uiPx(38):uiPx(8));
    if(narrowHeader)statW=std::max(uiPx(96),std::min(statDesired,w-uiPx(16)));
    MoveWindow(g_hStat,uiPx(8),statY,statW,uiPx(28),TRUE); ShowWindow(g_hStat,SW_SHOW);
    MoveWindow(g_hPrev,prevX,topRowY,prevW,uiPx(26),TRUE);
    MoveWindow(g_hPage,pageX,topRowY+uiPx(4),pageW,uiPx(18),TRUE);
    MoveWindow(g_hNext,nextX,topRowY,nextW,uiPx(26),TRUE);
    // 标题由父窗口绘制，避免缩放时静态子窗口的残影相互覆盖。
    if(g_hGroup)ShowWindow(g_hGroup,SW_HIDE);

    int searchY=uiPx(narrowHeader?110:(compact?76:46));
    int modeW=112, goW=compact?64:78, gap=8;
    modeW=uiPx(modeW); goW=uiPx(goW); gap=uiPx(gap);
    int searchMax=w-uiPx(10)-gap-modeW-gap-goW-uiPx(4);
    int searchW=std::max(uiPx(72),std::min(uiPx(compact?156:320),searchMax));
    SendMessageW(g_hSearch,EM_SETCUEBANNER,FALSE,(LPARAM)(compact?L"输入道具名称":L"在此输入道具名进行搜索"));
    MoveWindow(g_hSearch,uiPx(10),searchY,searchW,uiPx(28),TRUE);
    MoveWindow(g_hMode,uiPx(10)+searchW+gap,searchY,modeW,uiPx(28),TRUE);
    MoveWindow(g_hGo,uiPx(10)+searchW+gap+modeW+gap,searchY,goW,uiPx(28),TRUE);
    moveChildIfChanged(hwnd,g_hTree,0,toolbarH,treeW,std::max(1,h-toolbarH),TRUE);
    moveChildIfChanged(hwnd,g_hList,treeW+uiPx(2),toolbarH,std::max(1,w-treeW-uiPx(2)),std::max(1,h-toolbarH),TRUE);
    if(g_hTreeScroll){MoveWindow(g_hTreeScroll,0,0,1,1,FALSE);ShowWindow(g_hTreeScroll,SW_HIDE);}
    if(g_hListScroll){MoveWindow(g_hListScroll,0,0,1,1,FALSE);ShowWindow(g_hListScroll,SW_HIDE);}
    ShowWindow(g_hDetail,SW_HIDE);
    ShowWindow(g_hCopyId,SW_HIDE);
    ShowWindow(g_hCopyBoth,SW_HIDE);
    if(g_hList){
        RECT lr; GetClientRect(g_hList,&lr);
        ListView_SetColumnWidth(g_hList,0,std::max(uiPx(100),(int)lr.right-(int)lr.left-uiPx(SCROLLBAR_W)));
        int newCols=std::max(1,std::min(6,((int)lr.right-(int)lr.left)/uiPx(230)));
        if(newCols!=g_cardCols){
            g_cardCols=newCols;
            populateListRows();
        }
    }
    if(g_hTreeScroll)InvalidateRect(g_hTreeScroll,NULL,FALSE);
    if(g_hListScroll)InvalidateRect(g_hListScroll,NULL,FALSE);
}
static void activateTreeCategory(HTREEITEM item){
    if(!item)return;
    g_curType=treeToType(item); g_curKw=L""; SetWindowTextW(g_hSearch,L""); g_page=1;
    startQuery(g_curType,L"",0,1);
}
struct TreePalette{ Color accent; Color accentDark; Color tint; Color border; Color text; };
static TreePalette treePalette(const wchar_t* text){
    std::wstring s=text?text:L"";
    // 统一白色控件底，仅用低饱和的彩色强调选中态和分类标记。
    if(s.find(L"最新道具")!=std::wstring::npos)return {Color(255,71,151,145),Color(255,45,112,110),Color(255,235,247,245),Color(255,180,216,211),Color(255,45,100,99)};
    if(s.find(L"赛车")!=std::wstring::npos)return {Color(255,202,113,99),Color(255,159,77,67),Color(255,251,241,238),Color(255,229,198,190),Color(255,115,74,67)};
    if(s.find(L"装扮")!=std::wstring::npos)return {Color(255,157,128,170),Color(255,112,87,128),Color(255,246,241,249),Color(255,215,201,223),Color(255,92,74,106)};
    if(s.find(L"宠物")!=std::wstring::npos)return {Color(255,102,157,121),Color(255,67,119,87),Color(255,239,248,241),Color(255,194,220,200),Color(255,61,104,76)};
    if(s.find(L"功能")!=std::wstring::npos)return {Color(255,105,143,184),Color(255,72,107,150),Color(255,240,245,251),Color(255,198,213,231),Color(255,62,88,123)};
    return {Color(255,201,151,82),Color(255,157,107,47),Color(255,252,246,235),Color(255,229,207,164),Color(255,112,81,40)};
}

// ---------------- 主窗口过程 ----------------
static LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    switch(msg){
    case WM_CREATE:{
        g_dpi=GetDpiForWindow(hwnd); if(!g_dpi)g_dpi=96;
        rebuildFonts();
        g_hToolbarBrush=CreateSolidBrush(C_TOOLBAR);
        g_editBrush=CreateSolidBrush(RGB(247,247,249));
        g_hGroup=CreateWindowW(L"STATIC",L"QQ交流群：782834246",WS_CHILD|WS_VISIBLE|SS_CENTER|SS_NOPREFIX,10,7,700,20,hwnd,NULL,g_inst,0);
        SendMessage(g_hGroup,WM_SETFONT,(WPARAM)g_fontBold,TRUE);
        g_hSearch=CreateWindowW(L"EDIT",L"",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,10,32,240,26,hwnd,(HMENU)ID_SEARCH,g_inst,0);
        SendMessage(g_hSearch,WM_SETFONT,(WPARAM)g_font,TRUE);
        SendMessage(g_hSearch,EM_SETMARGINS,EC_LEFTMARGIN|EC_RIGHTMARGIN,MAKELONG(uiPx(9),uiPx(9)));
        SendMessage(g_hSearch,EM_SETCUEBANNER,FALSE,(LPARAM)L"在此输入道具名进行搜索");
        SetWindowSubclass(g_hSearch,SearchEditProc,7,0);
        g_hMode=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS,258,32,112,200,hwnd,(HMENU)ID_MODE,g_inst,0);
        SendMessage(g_hMode,WM_SETFONT,(WPARAM)g_font,TRUE);
        SendMessage(g_hMode,CB_ADDSTRING,0,(LPARAM)L"模糊搜索");
        SendMessage(g_hMode,CB_ADDSTRING,0,(LPARAM)L"精确搜索");
        SendMessage(g_hMode,CB_SETCURSEL,0,0);
        SetWindowTheme(g_hMode,L"Explorer",NULL);
        SetWindowSubclass(g_hMode,ModeComboProc,8,0);
        g_hGo=CreateWindowW(L"BUTTON",L"搜 索",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,380,32,76,26,hwnd,(HMENU)ID_GO,g_inst,0);
        SendMessage(g_hGo,WM_SETFONT,(WPARAM)g_font,TRUE);
        g_hStat=CreateWindowW(L"STATIC",L"就绪",WS_CHILD|WS_VISIBLE|SS_OWNERDRAW,8,7,160,28,hwnd,(HMENU)ID_STAT,g_inst,0);
        SendMessage(g_hStat,WM_SETFONT,(WPARAM)g_fontSmall,TRUE);
        g_hPrev=CreateWindowW(L"BUTTON",L"◀ 上一页",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,200,33,78,24,hwnd,(HMENU)ID_PREV,g_inst,0);
        SendMessage(g_hPrev,WM_SETFONT,(WPARAM)g_fontSmall,TRUE);
        g_hPage=CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE|SS_CENTER,310,36,120,18,hwnd,(HMENU)ID_PAGEINFO,g_inst,0);
        SendMessage(g_hPage,WM_SETFONT,(WPARAM)g_fontSmall,TRUE);
        g_hNext=CreateWindowW(L"BUTTON",L"下一页 ▶",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,420,33,78,24,hwnd,(HMENU)ID_NEXT,g_inst,0);
        SendMessage(g_hNext,WM_SETFONT,(WPARAM)g_fontSmall,TRUE);

        g_hTree=CreateWindowExW(0,WC_TREEVIEWW,L"",WS_CHILD|WS_VISIBLE|TVS_SHOWSELALWAYS,
            0,TBH,252,500,hwnd,(HMENU)ID_TREE,g_inst,0);
        SendMessage(g_hTree,WM_SETFONT,(WPARAM)g_font,TRUE);
        TreeView_SetItemHeight(g_hTree,uiPx(30));
        TreeView_SetBkColor(g_hTree,C_TREE_BG);
        TreeView_SetLineColor(g_hTree,C_TREE_LINE);
        SetWindowTheme(g_hTree,L"Explorer",NULL);
        buildTree(g_hTree);

        g_hList=CreateWindowExW(0,WC_LISTVIEWW,L"",WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_NOCOLUMNHEADER|LVS_SINGLESEL|LVS_SHOWSELALWAYS,
            TREEW+2,TBH,800,400,hwnd,(HMENU)ID_LIST,g_inst,0);
        ListView_SetExtendedListViewStyle(g_hList,LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT);
        ListView_SetBkColor(g_hList,C_CARD);
        ListView_SetTextBkColor(g_hList,C_CARD);
        ListView_SetTextColor(g_hList,C_TEXT);
        g_hRowImages=ImageList_Create(1,LIST_ROW_H,ILC_COLOR32|ILC_MASK,1,1);
        HBITMAP blankRow=CreateBitmap(1,LIST_ROW_H,1,32,NULL);
        if(g_hRowImages&&blankRow)ImageList_AddMasked(g_hRowImages,blankRow,RGB(0,0,0));
        if(blankRow)DeleteObject(blankRow);
        if(g_hRowImages)ListView_SetImageList(g_hList,g_hRowImages,LVSIL_SMALL);
        LVCOLUMNW col{}; col.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
        col.fmt=LVCFMT_LEFT; col.cx=800; col.pszText=(LPWSTR)L"";
        ListView_InsertColumn(g_hList,0,&col);
        SendMessage(g_hList,WM_SETFONT,(WPARAM)g_font,TRUE);
        SetWindowTheme(g_hList,L"Explorer",NULL);
        SetWindowSubclass(g_hTree,ScrollTargetProc,21,0);
        SetWindowSubclass(g_hList,ScrollTargetProc,22,0);
        ShowScrollBar(g_hTree,SB_VERT,TRUE); ShowScrollBar(g_hTree,SB_HORZ,FALSE);
        ShowScrollBar(g_hList,SB_VERT,TRUE); ShowScrollBar(g_hList,SB_HORZ,FALSE);
        g_hTreeScroll=CreateWindowExW(0,L"KeleScrollBar",L"",WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,
            0,0,SCROLLBAR_W,80,hwnd,(HMENU)ID_TREE_SCROLL,g_inst,g_hTree);
        g_hListScroll=CreateWindowExW(0,L"KeleScrollBar",L"",WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,
            0,0,SCROLLBAR_W,80,hwnd,(HMENU)ID_LIST_SCROLL,g_inst,g_hList);

        g_hDetail=CreateWindowExW(0,L"STATIC",L"",WS_CHILD|SS_OWNERDRAW,TREEW+2,0,1,1,hwnd,(HMENU)ID_DETAIL,g_inst,0);
        g_hCopyId=CreateWindowW(L"BUTTON",L"复制代码(ID)",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,640,120,108,26,hwnd,(HMENU)ID_COPY_ID,g_inst,0);
        SendMessage(g_hCopyId,WM_SETFONT,(WPARAM)g_fontSmall,TRUE);
        g_hCopyBoth=CreateWindowW(L"BUTTON",L"复制 ID+名称",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,756,120,108,26,hwnd,(HMENU)ID_COPY_BOTH,g_inst,0);
        SendMessage(g_hCopyBoth,WM_SETFONT,(WPARAM)g_fontSmall,TRUE);

        wchar_t buf[MAX_PATH];
        SHGetFolderPathW(NULL,CSIDL_LOCAL_APPDATA,NULL,0,buf);
        g_cacheDir=std::wstring(buf)+L"\\KeleDaobao\\cache\\";
        CreateDirectoryW((std::wstring(buf)+L"\\KeleDaobao").c_str(),NULL);
        CreateDirectoryW(g_cacheDir.c_str(),NULL);
        std::thread(imageThread).detach();

        layout(hwnd);
        g_curType=L""; g_curKw=L""; startQuery(L"",L"",0,1);
        return 0;
    }
    case WM_ENTERSIZEMOVE:
        g_inSizeMove=true;
        return 0;
    case WM_SIZE:
        // 缩放过程中只批量调整窗口矩形，不强制所有子控件立即重绘，避免 GDI+ 卡顿。
        layout(hwnd);
        InvalidateRect(hwnd,NULL,FALSE);
        return 0;
    case WM_DPICHANGED:{
        g_dpi=HIWORD(wp); if(!g_dpi)g_dpi=96;
        RECT suggested=*(RECT*)lp;
        SetWindowPos(hwnd,NULL,suggested.left,suggested.top,suggested.right-suggested.left,suggested.bottom-suggested.top,
            SWP_NOZORDER|SWP_NOACTIVATE);
        rebuildFonts();
        HWND controls[]={g_hGroup,g_hSearch,g_hMode,g_hGo,g_hPrev,g_hNext,g_hPage,g_hStat,g_hTree,g_hList,g_hDetail,g_hCopyId,g_hCopyBoth};
        for(HWND c:controls)if(c)SendMessageW(c,WM_SETFONT,(WPARAM)((c==g_hGroup)?g_fontBold:((c==g_hPage||c==g_hStat||c==g_hPrev||c==g_hNext)?g_fontSmall:g_font)),TRUE);
        layout(hwnd); RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);
        return 0;
    }
    case WM_EXITSIZEMOVE:
        g_inSizeMove=false;
        layout(hwnd);
        RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN|RDW_ERASE);
        return 0;
    case WM_GETMINMAXINFO:{
        MINMAXINFO* info=(MINMAXINFO*)lp;
        info->ptMinTrackSize.x=350; info->ptMinTrackSize.y=370;
        return 0;
    }
    case WM_CTLCOLORSTATIC:
        if((HWND)lp==g_hGroup){
            HDC dc=(HDC)wp;
            SetTextColor(dc,C_MAIN);
            SetBkMode(dc,OPAQUE);
            SetBkColor(dc,C_TOOLBAR);
            return (LRESULT)g_hToolbarBrush;
        }
        if((HWND)lp==g_hMode){
            HDC dc=(HDC)wp;
            SetTextColor(dc,RGB(60,65,82));
            SetBkMode(dc,TRANSPARENT);
            SetBkColor(dc,RGB(247,247,249));
            return (LRESULT)g_editBrush;
        }
        if((HWND)lp==g_hPage){
            HDC dc=(HDC)wp;
            SetTextColor(dc,RGB(88,76,72));
            SetBkMode(dc,TRANSPARENT);
            return (LRESULT)g_hToolbarBrush;
        }
        break;
    case WM_CTLCOLOREDIT:
        if((HWND)lp==g_hSearch||(HWND)lp==g_hMode){
            HDC dc=(HDC)wp;
            SetTextColor(dc,RGB(60,65,82));
            SetBkMode(dc,TRANSPARENT);
            SetBkColor(dc,RGB(247,247,249));
            return (LRESULT)g_editBrush;
        }
        break;
    case WM_CTLCOLORLISTBOX:
        if((HWND)lp==g_hMode){
            HDC dc=(HDC)wp;
            SetTextColor(dc,RGB(60,65,82));
            SetBkMode(dc,TRANSPARENT);
            SetBkColor(dc,RGB(247,247,249));
            return (LRESULT)g_editBrush;
        }
        break;
    case WM_ERASEBKGND: return 1; // 防闪烁
    case WM_PAINT:{
        PAINTSTRUCT ps; HDC dc=BeginPaint(hwnd,&ps); RECT cr; GetClientRect(hwnd,&cr);
        int paintW=std::max(1,(int)cr.right), paintH=std::max(1,(int)cr.bottom);
        HDC mem=CreateCompatibleDC(dc); HBITMAP bmp=CreateCompatibleBitmap(dc,paintW,paintH);
        HGDIOBJ old=SelectObject(mem,bmp);
        HBRUSH tb=CreateSolidBrush(C_TOOLBAR); FillRect(mem,&cr,tb); DeleteObject(tb);
        RECT sr{0,0,cr.right,4}; TRIVERTEX tv[2]; ZeroMemory(tv,sizeof(tv));
        tv[0].x=sr.left; tv[0].y=sr.top; tv[0].Red=(110<<8); tv[0].Green=(142<<8); tv[0].Blue=(255<<8);
        tv[1].x=sr.right; tv[1].y=sr.bottom; tv[1].Red=(184<<8); tv[1].Green=(96<<8); tv[1].Blue=(255<<8);
        GRADIENT_RECT gr; gr.UpperLeft=0; gr.LowerRight=1; GradientFill(mem,tv,2,&gr,1,GRADIENT_FILL_RECT_H);
        paintHeaderDecor(hwnd,mem,cr);
        BitBlt(dc,0,0,cr.right,cr.bottom,mem,0,0,SRCCOPY);
        SelectObject(mem,old); DeleteObject(bmp); DeleteDC(mem); EndPaint(hwnd,&ps); return 0; }
    case WM_COMMAND:{
        int id=LOWORD(wp);
        if(id==ID_GO){ wchar_t k[256]; GetWindowTextW(g_hSearch,k,256);
            g_curKw=k; g_page=1; int mode=(int)SendMessage(g_hMode,CB_GETCURSEL,0,0);
            startQuery(g_curType,g_curKw,mode,1); }
        else if(id==ID_PREV){ if(g_page>1&&!g_loading){g_page--;startQuery(g_curType,g_curKw,(int)SendMessage(g_hMode,CB_GETCURSEL,0,0),g_page);} }
        else if(id==ID_NEXT){ if(g_page<g_pageCount&&!g_loading){g_page++;startQuery(g_curType,g_curKw,(int)SendMessage(g_hMode,CB_GETCURSEL,0,0),g_page);} }
        else if(id==ID_COPY_ID){ if(g_sel>=0&&g_sel<(int)g_items.size())copyText(std::to_wstring(g_items[g_sel].id)); }
        else if(id==ID_COPY_BOTH){ if(g_sel>=0&&g_sel<(int)g_items.size())copyText(std::to_wstring(g_items[g_sel].id)+L" "+g_items[g_sel].name); }
        return 0; }
    case WM_MOUSEMOVE:{
        auto upd=[&](HWND btn,bool& hov){ bool h=btnHover(btn); if(h!=hov){ hov=h; InvalidateRect(btn,NULL,TRUE); } };
        upd(g_hGo,g_hovGo); upd(g_hPrev,g_hovPrev); upd(g_hNext,g_hovNext);
        upd(g_hCopyId,g_hovCopyId); upd(g_hCopyBoth,g_hovCopyBoth);
        return 0; }
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        InvalidateRect(g_hGo,NULL,TRUE); InvalidateRect(g_hPrev,NULL,TRUE); InvalidateRect(g_hNext,NULL,TRUE);
        InvalidateRect(g_hCopyId,NULL,TRUE); InvalidateRect(g_hCopyBoth,NULL,TRUE);
        return 0;
    case WM_NOTIFY:{
        NMHDR* nm=(NMHDR*)lp;
        if(nm->idFrom==ID_TREE&&nm->code==NM_CUSTOMDRAW){
            NMTVCUSTOMDRAW* draw=(NMTVCUSTOMDRAW*)lp;
            if(draw->nmcd.dwDrawStage==CDDS_PREPAINT)return CDRF_NOTIFYITEMDRAW;
            if(draw->nmcd.dwDrawStage==CDDS_ITEMPREPAINT){
                // 统一浅色底 + 黑字，去掉系统选中高亮，避免圆角外露出蓝色角
                draw->clrText=RGB(0,0,0);
                draw->clrTextBk=C_TREE_BG;
                return CDRF_NOTIFYPOSTPAINT;
            }
            if(draw->nmcd.dwDrawStage==CDDS_ITEMPOSTPAINT){
                HDC hdc=draw->nmcd.hdc;
                HTREEITEM item=(HTREEITEM)draw->nmcd.dwItemSpec;
                RECT rc,trc;
                if(!TreeView_GetItemRect(g_hTree,item,&rc,FALSE))return 0;
                if(!TreeView_GetItemRect(g_hTree,item,&trc,TRUE))return 0;
                wchar_t buf[256]; TVITEMW tvi{}; tvi.hItem=item; tvi.mask=TVIF_TEXT; tvi.pszText=buf; tvi.cchTextMax=256;
                if(!TreeView_GetItem(g_hTree,&tvi))return 0;
                bool selected=(draw->nmcd.uItemState&CDIS_SELECTED)!=0;
                bool hot=(draw->nmcd.uItemState&CDIS_HOT)!=0;
                bool root=(TreeView_GetParent(g_hTree,item)==NULL);
                bool expandable=root&&TreeView_GetChild(g_hTree,item)!=NULL;

                Graphics g(hdc); g.SetSmoothingMode(SmoothingModeAntiAlias);
                Gdiplus::Font* f=root?g_gdFontB:g_gdFont;
                StringFormat fmt; fmt.SetAlignment(StringAlignmentNear); fmt.SetLineAlignment(StringAlignmentCenter);
                fmt.SetFormatFlags(StringFormatFlagsNoWrap);
                RectF szR;
                g.MeasureString(buf,-1,f,PointF(0,0),&fmt,&szR);
                wchar_t paletteText[256]={0};
                wcscpy_s(paletteText,buf);
                if(!root){
                    HTREEITEM parent=TreeView_GetParent(g_hTree,item);
                    if(parent){ TVITEMW pvi{}; pvi.hItem=parent; pvi.mask=TVIF_TEXT; pvi.pszText=paletteText; pvi.cchTextMax=256; TreeView_GetItem(g_hTree,&pvi); }
                }
                TreePalette palette=treePalette(paletteText);
                float pad=(float)uiPx(root?16:14);
                float btnW=szR.Width+pad*2.0f;
                RECT client{}; GetClientRect(g_hTree,&client);
                float maxW=(float)std::max(1,(int)client.right-(int)trc.left-uiPx(12));
                float minW=(float)uiPx(root?(expandable?112:120):104);
                btnW=std::min(maxW,std::max(btnW,minW));
                float btnH=std::max((float)uiPx(24),(float)(rc.bottom-rc.top)-uiPx(6));
                float bx=(REAL)trc.left, by=(REAL)rc.top+(REAL)uiPx(3);
                float radius=(float)uiPx(8);
                RectF brr(bx,by,btnW,btnH);
                GraphicsPath gp; addRoundRect(gp,bx,by,btnW,btnH,radius);
                GraphicsPath shadowPath; addRoundRect(shadowPath,bx+(REAL)uiPx(1),by+(REAL)uiPx(1),btnW,btnH,radius);
                SolidBrush shadow(Color(18,83,91,108)); g.FillPath(&shadow,&shadowPath);
                Color buttonBg=selected?palette.tint:(hot?Color(255,249,251,253):Color(255,255,255,255));
                SolidBrush bg(buttonBg); g.FillPath(&bg,&gp);
                Pen edge(selected?palette.accent:Color(255,206,212,229),selected?1.3f:1.0f); g.DrawPath(&edge,&gp);
                float textPad=(float)uiPx(root?(expandable?25:19):19);
                float rightPad=(float)uiPx(root&&expandable?26:8);
                RectF frr(bx+textPad,by,std::max(1.0f,btnW-textPad-rightPad),btnH);
                StringFormat cf; cf.SetAlignment(StringAlignmentNear); cf.SetLineAlignment(StringAlignmentCenter);
                cf.SetTrimming(StringTrimmingEllipsisCharacter); cf.SetFormatFlags(StringFormatFlagsNoWrap);
                SolidBrush ink(selected?palette.accentDark:Color(255,55,62,78));
                g.DrawString(buf,-1,f,frr,&cf,&ink);
                // 用低饱和彩色标记区分分类，按钮主体保持与搜索框一致的白色圆角样式。
                SolidBrush marker(palette.accent);
                if(root){
                    g.FillEllipse(&marker,bx+(REAL)uiPx(8),by+(btnH-(REAL)uiPx(7))/2.0f,(REAL)uiPx(7),(REAL)uiPx(7));
                    if(expandable){
                        bool expanded=(TreeView_GetItemState(g_hTree,item,TVIS_EXPANDED)&TVIS_EXPANDED)!=0;
                        // 一级分类使用直观的 +/- 展开标记：+ 表示可展开，- 表示已展开。
                        Pen expandMark(selected?palette.accentDark:palette.accent,(REAL)uiPx(2));
                        float markX=bx+btnW-(REAL)uiPx(14), centerY=by+btnH/2.0f;
                        g.DrawLine(&expandMark,markX-(REAL)uiPx(5),centerY,markX+(REAL)uiPx(5),centerY);
                        if(!expanded)g.DrawLine(&expandMark,markX,centerY-(REAL)uiPx(5),markX,centerY+(REAL)uiPx(5));
                    }
                }else{
                    GraphicsPath markerPath; addRoundRect(markerPath,bx+(REAL)uiPx(8),by+(btnH-(REAL)uiPx(10))/2.0f,(REAL)uiPx(4),(REAL)uiPx(10),(REAL)uiPx(2));
                    g.FillPath(&marker,&markerPath);
                }
                return 0;
            }
        } else if(nm->idFrom==ID_TREE&&nm->code==NM_CLICK){
            POINT pt; GetCursorPos(&pt); ScreenToClient(g_hTree,&pt);
            TVHITTESTINFO hit{}; hit.pt=pt; HTREEITEM item=TreeView_HitTest(g_hTree,&hit);
            if(item){
                bool root=TreeView_GetParent(g_hTree,item)==NULL;
                if(root){
                    bool expandable=TreeView_GetChild(g_hTree,item)!=NULL;
                    if(expandable){
                        bool expanded=(TreeView_GetItemState(g_hTree,item,TVIS_EXPANDED)&TVIS_EXPANDED)!=0;
                        TreeView_Expand(g_hTree,item,expanded?TVE_COLLAPSE:TVE_EXPAND);
                    }
                    if(TreeView_GetSelection(g_hTree)!=item)TreeView_SelectItem(g_hTree,item);
                }else if(!(hit.flags&TVHT_ONITEMBUTTON)){
                    if(TreeView_GetSelection(g_hTree)!=item)TreeView_SelectItem(g_hTree,item);
                }
            }
        } else if(nm->idFrom==ID_TREE&&nm->code==TVN_SELCHANGED){
            NMTREEVIEW* tv=(NMTREEVIEW*)lp; HTREEITEM it=tv->itemNew.hItem;
            activateTreeCategory(it);
        } else if(nm->idFrom==ID_LIST&&nm->code==NM_CLICK){
            NMITEMACTIVATE* click=(NMITEMACTIVATE*)lp;
            if(click->iItem>=0){
                RECT row{};
                if(ListView_GetItemRect(g_hList,click->iItem,&row,LVIR_BOUNDS)){
                    int gap=8;
                    int rowWidth=row.right-row.left;
                    int cardWidth=(rowWidth-gap*(g_cardCols+1))/g_cardCols;
                    for(int col=0;col<g_cardCols;++col){
                        int itemIndex=click->iItem*g_cardCols+col;
                        int cardLeft=gap+col*(cardWidth+gap);
                        RECT card{cardLeft,row.top+4,cardLeft+cardWidth,row.bottom-4};
                        if(itemIndex>=(int)g_items.size()||!PtInRect(&card,click->ptAction))continue;
                        RECT copyRect{card.right-64,card.top+91,card.right-8,card.top+116};
                        g_sel=itemIndex;
                        if(PtInRect(&copyRect,click->ptAction)){
                            bool copied=copyText(std::to_wstring(g_items[itemIndex].id));
                            if(copied){
                                g_copyFlashIndex=itemIndex;
                                SetTimer(hwnd,ID_COPY_TIMER,1000,NULL);
                                setStatusText(L"已复制道具 ID");
                            }else{
                                setStatusText(L"复制失败");
                            }
                        }else{
                            requestImage(g_items[itemIndex].id);
                        }
                        InvalidateRect(g_hDetail,NULL,TRUE);
                        InvalidateRect(g_hList,NULL,FALSE);
                        break;
                    }
                }
            }
        } else if(nm->idFrom==ID_LIST&&nm->code==NM_CUSTOMDRAW){
            NMLVCUSTOMDRAW* draw=(NMLVCUSTOMDRAW*)lp;
            if(draw->nmcd.dwDrawStage==CDDS_PREPAINT)return CDRF_NOTIFYITEMDRAW;
            if(draw->nmcd.dwDrawStage==CDDS_ITEMPREPAINT){
                int index=(int)draw->nmcd.dwItemSpec;
                RECT row{};
                if(ListView_GetItemRect(g_hList,index,&row,LVIR_BOUNDS)){
                    paintRow(draw->nmcd.hdc,index,row);
                }
                return CDRF_SKIPDEFAULT;
            }
        }
        return 0; }
    case WM_TIMER:
        if(wp==ID_COPY_TIMER){
            KillTimer(hwnd,ID_COPY_TIMER);
            g_copyFlashIndex=-1;
            InvalidateRect(g_hList,NULL,FALSE);
            return 0;
        }
        break;
    case WM_DRAWITEM:{
        DRAWITEMSTRUCT* dis=(DRAWITEMSTRUCT*)lp;
        if(dis->CtlID==ID_MODE&&dis->CtlType==ODT_COMBOBOX){
            RECT r=dis->rcItem; Graphics g(dis->hDC); g.SetSmoothingMode(SmoothingModeAntiAlias);
            bool selected=(dis->itemState&ODS_SELECTED)!=0;
            SolidBrush bg(selected?Color(255,255,236,226):Color(255,255,255,255)); g.FillRectangle(&bg,(INT)r.left,(INT)r.top,(INT)(r.right-r.left),(INT)(r.bottom-r.top));
            wchar_t text[128]={0}; if(dis->itemID!=(UINT)-1)SendMessageW(dis->hwndItem,CB_GETLBTEXT,dis->itemID,(LPARAM)text);
            RectF tr((REAL)(r.left+uiPx(9)),(REAL)r.top,(REAL)(r.right-r.left-uiPx(18)),(REAL)(r.bottom-r.top));
            StringFormat fmt; fmt.SetAlignment(StringAlignmentNear); fmt.SetLineAlignment(StringAlignmentCenter); fmt.SetTrimming(StringTrimmingEllipsisCharacter); fmt.SetFormatFlags(StringFormatFlagsNoWrap);
            SolidBrush ink(selected?Color(255,174,56,28):Color(255,55,62,78)); g.DrawString(text,-1,g_gdFont,tr,&fmt,&ink);
            return TRUE;
        }
        if(dis->CtlType==ODT_STATIC&&dis->CtlID==ID_STAT){ paintStatBadge(dis->hDC,dis->rcItem); return TRUE; }
        if(dis->CtlType==ODT_STATIC){ paintDetail(hwnd); return TRUE; }
        if(dis->CtlType==ODT_BUTTON){
            bool primary=(dis->CtlID==ID_GO);
            drawButton(dis->hDC,dis->rcItem,dis->hwndItem,primary); return TRUE;
        }
        return 0; }
    case WM_MEASUREITEM:{
        MEASUREITEMSTRUCT* mi=(MEASUREITEMSTRUCT*)lp;
        if(mi&&mi->CtlID==ID_MODE){ mi->itemHeight=(UINT)uiPx(26); return TRUE; }
        return 0; }
    case MSG_LOAD_DONE:{
        long seq=(long)wp;
        if(seq==g_seq.load()){
            std::pair<int,std::vector<Item>*>* res=(std::pair<int,std::vector<Item>*>*)lp;
            g_total=res->first; std::vector<Item>* items=res->second;
            g_items=std::move(*items);
            delete items; delete res;
            g_pageCount=std::max(1,(int)std::ceil((double)g_total/LIMIT));
            g_loading=false;
            std::wstring st=g_curKw.empty()?L"":(L"搜索  "+g_curKw+L"  · ");
            st+=L"共 "+std::to_wstring(g_total)+L" 条";
            setStatusText(st);
            std::wstring pg=L"第 "+std::to_wstring(g_page)+L" / "+std::to_wstring(g_pageCount)+L" 页";
            SetWindowTextW(g_hPage,pg.c_str());
            layout(hwnd);
            g_sel=-1; InvalidateRect(g_hDetail,NULL,TRUE);
            ListView_DeleteAllItems(g_hList);
            populateListRows();
        } else {
            std::pair<int,std::vector<Item>*>* stale=(std::pair<int,std::vector<Item>*>*)lp;
            delete stale->second; delete stale;
        }
        return 0; }
    case MSG_IMG_READY:{
        (void)wp;
        InvalidateRect(g_hList,NULL,FALSE); InvalidateRect(g_hDetail,NULL,TRUE);
        return 0; }
    case WM_DESTROY:
        { std::lock_guard<std::mutex> lk(g_qmutex); g_imgStop=true; g_qcv.notify_all(); }
        if(g_gdFont)delete g_gdFont; if(g_gdFontB)delete g_gdFontB; if(g_gdFontS)delete g_gdFontS;
        if(g_font)DeleteObject(g_font); if(g_fontBold)DeleteObject(g_fontBold); if(g_fontSmall)DeleteObject(g_fontSmall);
        if(g_hToolbarBrush)DeleteObject(g_hToolbarBrush);
        if(g_editBrush)DeleteObject(g_editBrush);
        if(g_placeholder){ delete g_placeholder; g_placeholder=NULL; }
        if(g_hRowImages)ImageList_Destroy(g_hRowImages);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}

// ---------------- 入口 ----------------
int WINAPI wWinMain(HINSTANCE hI,HINSTANCE,LPWSTR,int){
    g_inst=hI;
    // 让窗口按当前显示器 DPI 渲染，并在跨显示器移动时接收 WM_DPICHANGED。
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    GdiplusStartupInput gsi; ULONG_PTR token;
    if(GdiplusStartup(&token,&gsi,NULL)!=Ok)return 1;
    INITCOMMONCONTROLSEX icc{sizeof(icc),ICC_WIN95_CLASSES};
    InitCommonControlsEx(&icc);
    WNDCLASSW wc{}; wc.lpfnWndProc=WndProc; wc.hInstance=hI; wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1); wc.hIcon=LoadIconW(hI,MAKEINTRESOURCEW(100));
    wc.lpszClassName=L"KeleDaobaoMain";
    RegisterClassW(&wc);
    WNDCLASSW sbwc{}; sbwc.lpfnWndProc=ScrollBarProc; sbwc.hInstance=hI; sbwc.hCursor=LoadCursor(NULL,IDC_ARROW);
    sbwc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1); sbwc.lpszClassName=L"KeleScrollBar";
    RegisterClassW(&sbwc);
    RECT r{0,0,760,520}; AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,FALSE);
    HWND hwnd=CreateWindowW(L"KeleDaobaoMain",APP_TITLE,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_VISIBLE,
        CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,NULL,NULL,hI,NULL);
    if(!hwnd){ GdiplusShutdown(token); return 1; }
    g_hwnd=hwnd;
    MSG msg;
    while(GetMessageW(&msg,NULL,0,0)>0){
        if(!IsDialogMessageW(hwnd,&msg)){ TranslateMessage(&msg); DispatchMessageW(&msg); }
    }
    for(auto& p:g_imgMem)if(p.second)delete p.second;
    GdiplusShutdown(token);
    return (int)msg.wParam;
}

// TEST_MARKER_12345
