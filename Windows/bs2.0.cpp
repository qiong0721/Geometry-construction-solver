#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include<string>
#include <math.h>
#include<graphics.h>
#include<Windows.h>
#define epsilon (1e-8)
#define sq(a) ((a)*(a))
#pragma warning(disable:4996)
#pragma warning(disable:4576)
using namespace std;


typedef struct { double x, y; } Point;      // (x,y)
typedef struct { double a, b, c; } Line;    // ax+by=c
typedef struct { double a, b, r; } Circle;  // (x-a)^2+(y-b)^2=r^2


#define SOURCE_LL 0   
#define SOURCE_LC 1   
#define SOURCE_CC 2   


typedef struct {
    int type;
    int idx1;
    int idx2;
} Source;

Line   DrawL(Point P1, Point P2);
Circle DrawC(Point P1, Point P2);
void   LandL(Line L1, Line L2, double det, int idx1, int idx2);
void   LandC(Line L1, Circle C1, int i, double delta, int idxL, int idxC);
void   CandC(Circle C1, Circle C2, int idx1, int idx2);


void PrintP(Point P) { printf("(%f,%f)\n", P.x, P.y); }
void PrintL(Line L) { printf("%f*x+%f*y=%f\n", L.a, L.b, L.c); }
void PrintC(Circle C) { printf("(x-%f)^2+(y-%f)^2=%f^2\n", C.a, C.b, C.r); }
void PrintSol(int j);


int IsZero(double i) { return fabs(i) < epsilon; }
int IsSameP(Point P1, Point P2) { return IsZero(P1.x - P2.x) && IsZero(P1.y - P2.y); }
int IsSameL(Line L1, Line L2) { return IsZero(L1.a - L2.a) && IsZero(L1.b - L2.b) && IsZero(L1.c - L2.c); }
int IsSameC(Circle C1, Circle C2) { return IsZero(C1.a - C2.a) && IsZero(C1.b - C2.b) && IsZero(C1.r - C2.r); }
int IsOnL(Point P, Line L) { return IsZero(L.a * P.x + L.b * P.y - L.c); }
int IsOnC(Point P, Circle C) { return IsZero(sq(P.x - C.a) + sq(P.y - C.b) - sq(C.r)); }


int IsRepP();
int IsRepL();
int IsRepC();


void AllLandL();
void AllLandC();
void AllCandL();
void AllCandC();

void Next();
void Clear(int i);


int E, NumP, NumL, NumC,NumG,IsGoalMet;
Point P1, P2;
Point *PGoal;
Line  *LGoal;
Circle *CGoal;
Point* Ps;
Source* PointSource;         //source information
Line* Ls;
Circle* Cs;
int* Tool;         //0,1= circle/ 2= line  
int** Pair;
int* Count;
int Init[4];        //0=E/1=point/2=line/3=circle      



Line DrawL(Point P1, Point P2)
{
    double a = P2.y - P1.y;
    double b = P1.x - P2.x;
    double c = P1.x * P2.y - P1.y * P2.x;
    Line L;
    if (!IsZero(b)) { L = { a / b, 1, c / b }; }
    else if (!IsZero(a)) { L = { 1, 0, c / a }; }
    return L;
}

Circle DrawC(Point P1, Point P2)
{
    Circle C = { P1.x, P1.y, sqrt(sq(P1.x - P2.x) + sq(P1.y - P2.y)) };
    return C;
}

void LandL(Line L1, Line L2, double det, int idx1, int idx2)
{
    double x = (L1.c * L2.b - L1.b * L2.c) / det;
    double y = (L1.a * L2.c - L1.c * L2.a) / det;
    Ps[NumP] = { x, y };
    PointSource[NumP] = { SOURCE_LL, idx1, idx2 };
    IsRepP();
}

void LandC(Line L1, Circle C1, int i, double delta, int idxL, int idxC)
{
    double root = sqrt(delta);
    double q = sq(L1.a) + sq(L1.b);
    double x = (L1.a * L1.c + L1.b * (L1.b * C1.a - L1.a * C1.b) - i * L1.b * root) / q;
    double y = (L1.b * L1.c - L1.a * (L1.b * C1.a - L1.a * C1.b) + i * L1.a * root) / q;
    Ps[NumP] = { x, y };
    PointSource[NumP] = { SOURCE_LC, idxL, idxC };
    IsRepP();
}

void CandC(Circle C1, Circle C2, int idx1, int idx2)
{
    double a = 2 * (C1.a - C2.a);
    double b = 2 * (C1.b - C2.b);
    if (IsZero(a) && IsZero(b)) return;
    double c = sq(C1.a) - sq(C2.a) + sq(C1.b) - sq(C2.b) - sq(C1.r) + sq(C2.r);
    Line L1 = { a, b, c };
    double q = sq(L1.a) + sq(L1.b);
    double delta = sq(C1.r) * q - sq(L1.a * C1.a + L1.b * C1.b - L1.c);
    if (delta < -epsilon) return;
    if (IsZero(delta)) delta = 0;

    double root = sqrt(delta);
    double x1 = (L1.a * L1.c + L1.b * (L1.b * C1.a - L1.a * C1.b) - L1.b * root) / q;
    double y1 = (L1.b * L1.c - L1.a * (L1.b * C1.a - L1.a * C1.b) + L1.a * root) / q;
    Ps[NumP] = { x1, y1 };
    PointSource[NumP] = { SOURCE_CC, idx1, idx2 };
    IsRepP();

    if (delta > epsilon)
    {
        double x2 = (L1.a * L1.c + L1.b * (L1.b * C1.a - L1.a * C1.b) + L1.b * root) / q;
        double y2 = (L1.b * L1.c - L1.a * (L1.b * C1.a - L1.a * C1.b) - L1.a * root) / q;
        Ps[NumP] = { x2, y2 };
        PointSource[NumP] = { SOURCE_CC, idx1, idx2 };
        IsRepP();
    }
}



void AllLandL()
{
    for (int i = 0; i < NumL - 1; ++i)
    {
        double det = Ls[i].a * Ls[NumL - 1].b - Ls[i].b * Ls[NumL - 1].a;
        if (IsZero(det)) continue;
        LandL(Ls[i], Ls[NumL - 1], det, i, NumL - 1);
    }
}

void AllLandC()
{
    for (int i = 0; i < NumC; ++i)
    {
        double q = sq(Ls[NumL - 1].a) + sq(Ls[NumL - 1].b);
        double delta = sq(Cs[i].r) * q - sq(Ls[NumL - 1].a * Cs[i].a + Ls[NumL - 1].b * Cs[i].b - Ls[NumL - 1].c);
        if (delta < -epsilon) continue;
        if (IsZero(delta)) delta = 0;
        LandC(Ls[NumL - 1], Cs[i], 1, delta, NumL - 1, i);
        if (delta > epsilon)
            LandC(Ls[NumL - 1], Cs[i], -1, delta, NumL - 1, i);
    }
}

void AllCandL()
{
    for (int i = 0; i < NumL; ++i)
    {
        double q = sq(Ls[i].a) + sq(Ls[i].b);
        double delta = sq(Cs[NumC - 1].r) * q - sq(Ls[i].a * Cs[NumC - 1].a + Ls[i].b * Cs[NumC - 1].b - Ls[i].c);
        if (delta < -epsilon) continue;
        if (IsZero(delta)) delta = 0;
        LandC(Ls[i], Cs[NumC - 1], 1, delta, i, NumC - 1);
        if (delta > epsilon)
            LandC(Ls[i], Cs[NumC - 1], -1, delta, i, NumC - 1);
    }
}

void AllCandC()
{
    for (int i = 0; i < NumC - 1; ++i)
    {
        CandC(Cs[i], Cs[NumC - 1], i, NumC - 1);
    }
}

int IsDrawnP(Point P)
{
    for (int i = 0; i < NumP; ++i)
        if (IsSameP(P, Ps[i]))
            return 1;
    return 0;
}
int CanDrawL(Line L)
{
    int CountP = 0;
    for (int i = 0; i < NumP; ++i)
        if (IsOnL(Ps[i], L))
            ++CountP;
    return CountP >= 2;
}
int CanDrawC(Circle C)
{
    for (int i = 0; i < NumP; ++i)
        if (IsZero(C.a - Ps[i].x) && IsZero(C.b - Ps[i].y))
            for (int j = 0; j < NumP; ++j)
                if (IsOnC(Ps[j], C))
                    return 1;
    return 0;
}


int IsRepP()
{
    for (int i = 0; i < NumP; ++i)
        if (IsSameP(Ps[i], Ps[NumP]))
            return 1;
    ++NumP;
    ++Count[E];
    return 0;
}

int IsRepL()
{
    for (int i = 0; i < NumL; ++i)
        if (IsSameL(Ls[i], Ls[NumL]))
            return 1;
    return 0;
}

int IsRepC()
{
    for (int i = 0; i < NumC; ++i)
        if (IsSameC(Cs[i], Cs[NumC]))
            return 1;
    return 0;
}



void Next()
{
    ++Pair[E][1];
    for (int i = E; i > 0; --i)
    {
        if (Pair[i][1] > NumP)
        {
            ++Pair[i][0];
            Pair[i][1] = Pair[i][0] + 1;
        }
        if (Pair[i][0] > NumP - 1)
        {
            ++Pair[i - 1][1];
            Pair[i][0] = 1;
            Pair[i][1] = 2;
            Clear(i - 1);
        }
        else break;
    }
}

void Clear(int i)
{
    --E;
    if (Tool[i] > 1) --NumL;
    else --NumC;
    NumP -= Count[i];
    Count[i] = 0;
}

void drawLine(double a, double b, double c, COLORREF color) {
    double x1, y1, x2, y2;
    setlinecolor(color);
    if (a == 0) {
        y1 = c / b;
        y2 = y1;
        x1 = 0;
        x2 = 750;
    }
    else if (b == 0) {
        x1 = c / a;
        x2 = x1;
        y1 = 0;
        y2 = 600;
    }
    else {
        x1 = 0;
        y1 = c / b;

        x2 = 750;
        y2 = (c - a * x2) / b;
    }

    line(x1, y1, x2, y2);
}
double distance(Point p1, Point p2)
{
    double a = sq(p1.x - p2.x) + sq(p1.y - p2.y);
    return sqrt(a);
}
int ModeNote, ToolNote, GoalNote, limit, t = 0;
void PrintSol(int j)
{
    cleardevice();
    settextstyle(16, 0, NULL);
    wstring s1 = L"方法代码：" + to_wstring(j), s2 = L"总步数：" + to_wstring(E);
    setlinestyle(PS_SOLID | PS_JOIN_BEVEL, 5);

    while (1)
    {
        cleardevice();
        outtextxy(0, 0, L"已找到解");
        outtextxy(0, 20, s1.c_str());
        outtextxy(0, 40, s2.c_str());
        setlinecolor(BLUE);
        setfillcolor(BLUE);
        for (int i = 0; i < Init[1]; ++i)
        {
            solidcircle(Ps[i].x * 20 + 375, Ps[i].y * 20 + 300, 5);
        }
        for (int i = 0; i < Init[2]; ++i)
        {
            drawLine(Ls[i].a, Ls[i].b, 20 * Ls[i].c + 375 * Ls[i].a + 300 * Ls[i].b, BLUE);
        }
        for (int i = 0; i < Init[3]; ++i)
        {
            circle(Cs[i].a * 20 + 375, Cs[i].b * 20 + 300, Cs[i].r * 20);
        }
        Sleep(1000);
        int currentP = Init[1];   //the given numbers
        int currentL = Init[2];
        int currentC = Init[3];
        setlinecolor(BLACK);
        setfillcolor(BLACK);
        for (int step = 0; step < E; ++step)
        {
            if (Tool[step] == 0 || Tool[step] == 1)
            {
                int centerIdx, pointOnCircleIdx;
                if (Tool[step] == 0)
                {
                    centerIdx = currentP - Pair[step][0] + 1;
                    pointOnCircleIdx = currentP - Pair[step][1] + 1;
                }
                else
                {
                    centerIdx = currentP - Pair[step][1] + 1;
                    pointOnCircleIdx = currentP - Pair[step][0] + 1;
                }
                ++currentC;
                circle(Ps[centerIdx - 1].x * 20 + 375, Ps[centerIdx - 1].y * 20 + 300, distance(Ps[centerIdx - 1], Ps[pointOnCircleIdx - 1]) * 20);
                Sleep(1000);
            }
            else if (Tool[step] == 2)
            {
                int p1Idx = currentP - Pair[step][0] + 1;
                int p2Idx = currentP - Pair[step][1] + 1;
                ++currentL;
                drawLine(Ls[currentL - 1].a, Ls[currentL - 1].b, 20 * Ls[currentL - 1].c + 375 * Ls[currentL - 1].a + 300 * Ls[currentL - 1].b, BLACK);

                Sleep(1000);
            }

            if (Count[step] > 0)
            {
                for (int p = currentP; p < currentP + Count[step]; ++p)
                {
                    int num = p + 1;

                    solidcircle(Ps[num].x * 20 + 375, Ps[num].y * 20 + 300, 5);
                }
                currentP += Count[step];
            }
        }
        setlinecolor(YELLOW);
        setfillcolor(YELLOW);
        if (GoalNote == 0)
        {
            for (int i = 0; i < NumG; ++i)
            {
                circle(CGoal[i].a * 20 + 375, CGoal[i].b * 20 + 300, CGoal[i].r * 20);
            }
            
        }
        else if (GoalNote == 1)
        {
            for (int i = 0; i < NumG; ++i)
            {
                drawLine(LGoal[i].a, LGoal[i].b, 20 * LGoal[i].c + 375 * LGoal[i].a + 300 * LGoal[i].b, YELLOW);
            }
        }

        else if (GoalNote == 2)
        {
            for (int i = 0; i < NumG; ++i)
            {
                solidcircle(PGoal[i].x * 20 + 375, PGoal[i].y * 20 + 300, 5);
            }
        }
            
        Sleep(3000);
    }

}
POINT sb;
wchar_t s[10];
int main()
{

    initgraph(750, 600);
    HWND h = FindWindow(NULL, L"作图");
    setbkcolor(WHITE);
    cleardevice();
    setfillcolor(BLUE);
    fillrectangle(75, 250, 225, 350);
    fillrectangle(300, 250, 450, 350);
    fillrectangle(525, 250, 675, 350);
    settextcolor(BLACK);
    outtextxy(110, 300, L"尺规作图");
    outtextxy(335, 300, L"单尺作图");
    outtextxy(560, 300, L"单规作图");
    while (1)
    {
        GetCursorPos(&sb);
        ScreenToClient(h, &sb);
        if (sb.x <= 225 && sb.x >= 75 && sb.y >= 250 && sb.y <= 350 && GetAsyncKeyState(VK_LBUTTON))
        {
            ModeNote = 0;
            cleardevice();
            if (InputBox(s, 20, L"输入步数限制（包括条件中的步数，小于20步）", NULL, NULL, 300, 0, false))
            {
                limit = _wtoi(s);
                fillrectangle(75, 250, 225, 350);
                fillrectangle(300, 250, 450, 350);
                fillrectangle(525, 250, 675, 350);
                outtextxy(110, 300, L"点");
                outtextxy(335, 300, L"线");
                outtextxy(560, 300, L"圆");
                break;
            }
            else
            {
                fillrectangle(75, 250, 225, 350);
                fillrectangle(300, 250, 450, 350);
                fillrectangle(525, 250, 675, 350);
                outtextxy(110, 300, L"尺规作图");
                outtextxy(335, 300, L"单尺作图");
                outtextxy(560, 300, L"单规作图");
            }
        }
        else if (sb.x <= 450 && sb.x >= 300 && sb.y >= 250 && sb.y <= 350 && GetAsyncKeyState(VK_LBUTTON))
        {
            ModeNote = 1;
            cleardevice();
            if (InputBox(s, 20, L"输入步数限制（包括条件中的步数，小于20步）", NULL, NULL, 300, 0, false))
            {
                limit = _wtoi(s);
                fillrectangle(75, 250, 225, 350);
                fillrectangle(525, 250, 675, 350);
                outtextxy(110, 300, L"点");
                outtextxy(560, 300, L"线");
                break;
            }
            else
            {
                fillrectangle(75, 250, 225, 350);
                fillrectangle(300, 250, 450, 350);
                fillrectangle(525, 250, 675, 350);
                outtextxy(110, 300, L"尺规作图");
                outtextxy(335, 300, L"单尺作图");
                outtextxy(560, 300, L"单规作图");
            }
        }
        else if (sb.x <= 675 && sb.x >= 525 && sb.y >= 250 && sb.y <= 350 && GetAsyncKeyState(VK_LBUTTON))
        {
            ModeNote = 2;
            cleardevice();
            if (InputBox(s, 20, L"输入步数限制（包括条件中的步数，小于20步）", NULL, NULL, 300, 0, false))
            {
                limit = _wtoi(s);
                fillrectangle(75, 250, 225, 350);
                fillrectangle(525, 250, 675, 350);
                outtextxy(110, 300, L"点");
                outtextxy(560, 300, L"圆");
                break;
            }
            else
            {
                fillrectangle(75, 250, 225, 350);
                fillrectangle(300, 250, 450, 350);
                fillrectangle(525, 250, 675, 350);
                outtextxy(110, 300, L"尺规作图");
                outtextxy(335, 300, L"单尺作图");
                outtextxy(560, 300, L"单规作图");
            }
        }
    }
    outtextxy(300, 100, L"选择目标类型");
    while (1)
    {
        GetCursorPos(&sb);
        ScreenToClient(h, &sb);
        if (sb.x <= 225 && sb.x >= 75 && sb.y >= 250 && sb.y <= 350 && GetAsyncKeyState(VK_LBUTTON))
        {
            GoalNote = 2;
            cleardevice();
            if (InputBox(s, 20, L"输入目标个数",NULL, NULL, 300, 0, false))
            {
                NumG = wcstod(s, NULL);
                //cout << NumG;
                PGoal = new Point[NumG];
                for (int i = 0; i < NumG; ++i)
                {
                    wstring w0 = L"输入目标点P";
                    w0+=(char)(i + 1 + '0');
                    wstring wy =w0+ L"的y坐标",wx=w0+L"的x坐标";
                    if (InputBox(s, 20, wx.c_str(), NULL, NULL, 300, 0, false))
                    {
                        PGoal[i].x = wcstod(s, NULL);
                        InputBox(s, 20, wy.c_str());
                        PGoal[i].y = wcstod(s, NULL);
                        //break;
                    }
                    else
                    {
                        if (ModeNote == 0)
                        {
                            fillrectangle(75, 250, 225, 350);
                            fillrectangle(300, 250, 450, 350);
                            fillrectangle(525, 250, 675, 350);
                            outtextxy(110, 300, L"点");
                            outtextxy(335, 300, L"线");
                            outtextxy(560, 300, L"圆");
                        }
                        else
                        {
                            fillrectangle(75, 250, 225, 350);
                            fillrectangle(525, 250, 675, 350);
                            outtextxy(110, 300, L"点");
                            outtextxy(560, 300, L"线");
                        }
                    }
                }
                break;
            }
            
        }
        else if (sb.x <= 450 && sb.x >= 300 && sb.y >= 250 && sb.y <= 350 && GetAsyncKeyState(VK_LBUTTON) && ModeNote == 0)
        {
            GoalNote = 1;
            cleardevice();
            if (InputBox(s, 20, L"输入目标个数", NULL, NULL, 300, 0, false))
            {
                NumG = wcstod(s, NULL);
                LGoal = new Line[NumG];
                for (int i = 0; i < NumG; ++i)
                {
                    wstring w0 = L"输入目标直线L";
                    w0+=(char)(i + 1 + '0');
                    wstring wa = w0 + L"方程ax+by=c中的a", wb = w0 + L"方程ax+by=c中的b", wc = w0 + L"方程ax+by=c中的c";
                    if (InputBox(s, 20, wa.c_str(), NULL, NULL, 300, 0, false))
                    {
                        double a, b, c;
                        a = wcstod(s, NULL);
                        InputBox(s, 20, wb.c_str());
                        b = wcstod(s, NULL);
                        InputBox(s, 20, wc.c_str());
                        c = wcstod(s, NULL);
                        if (!IsZero(b)) { LGoal[i] = {a / b, 1, c / b}; }
                        else if (!IsZero(a)) { LGoal[i] = {1, 0, c / a}; }
                        --limit;
                        //break;
                    }
                    else
                    {
                        fillrectangle(75, 250, 225, 350);
                        fillrectangle(300, 250, 450, 350);
                        fillrectangle(525, 250, 675, 350);
                        outtextxy(110, 300, L"点");
                        outtextxy(335, 300, L"线");
                        outtextxy(560, 300, L"圆");
                    }
                }
                break;
            }
            
        }
        else if (sb.x <= 675 && sb.x >= 525 && sb.y >= 250 && sb.y <= 350 && GetAsyncKeyState(VK_LBUTTON))
        {
            if (ModeNote == 1)
            {
                GoalNote = 1;
                cleardevice();
                if (InputBox(s, 20, L"输入目标个数", NULL, NULL, 300, 0, false))
                {
                    NumG = wcstod(s, NULL);
                    LGoal = new Line[NumG];
                    for (int i = 0; i < NumG; ++i)
                    {
                        wstring w0 = L"输入目标直线L";
                        w0+=(char)(i + 1 + '0');
                        wstring wa = w0 + L"方程ax+by=c中的a", wb = w0 + L"方程ax+by=c中的b", wc = w0 + L"方程ax+by=c中的c";
                        if (InputBox(s, 20, wa.c_str(), NULL, NULL, 300, 0, false))
                        {
                            double a, b, c;
                            a = wcstod(s, NULL);
                            InputBox(s, 20, wb.c_str());
                            b = wcstod(s, NULL);
                            InputBox(s, 20, wc.c_str());
                            c = wcstod(s, NULL);
                            if (!IsZero(b)) { LGoal[i] = { a / b, 1, c / b }; }
                            else if (!IsZero(a)) { LGoal[i] ={ 1, 0, c / a }; }
                            --limit;
                            //break;
                        }
                        else
                        {
                            fillrectangle(75, 250, 225, 350);
                            fillrectangle(525, 250, 675, 350);
                            outtextxy(110, 300, L"点");
                            outtextxy(560, 300, L"线");
                        }
                    }
                    break;
                }
                
            }
            else
            {
                GoalNote = 0;
                cleardevice();
                if (InputBox(s, 20, L"输入目标个数", NULL, NULL, 300, 0, false))
                {
                    NumG = wcstod(s, NULL);
                    CGoal = new Circle[NumG];
                    for (int i = 0; i < NumG; ++i)
                    {
                        wstring w0 = L"输入目标圆C";
                        w0+=(char)(i + 1 + '0');
                        wstring wa = w0 + L"方程(x-a)^2+(y-b)^2=r^2中的a", wb = w0 + L"方程(x-a)^2+(y-b)^2=r^2中的b", wr = w0 + L"方程(x-a)^2+(y-b)^2=r^2中的r";
                        if (InputBox(s, 20, wa.c_str(), NULL, NULL, 300, 0, false))
                        {
                            CGoal[i].a = wcstod(s, NULL);
                            InputBox(s, 20, wb.c_str());
                            CGoal[i].b = wcstod(s, NULL);
                            InputBox(s, 20, wr.c_str());
                            CGoal[i].r = wcstod(s, NULL);
                            --limit;
                            //break;
                        }
                        else
                        {
                            if (ModeNote == 0)
                            {
                                fillrectangle(75, 250, 225, 350);
                                fillrectangle(300, 250, 450, 350);
                                fillrectangle(525, 250, 675, 350);
                                outtextxy(110, 300, L"点");
                                outtextxy(335, 300, L"线");
                                outtextxy(560, 300, L"圆");
                            }
                            else
                            {
                                fillrectangle(75, 250, 225, 350);
                                fillrectangle(525, 250, 675, 350);
                                outtextxy(110, 300, L"点");
                                outtextxy(560, 300, L"圆");
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    Ps = (Point*)malloc(limit * limit * sizeof(Point));
    PointSource = (Source*)malloc(limit * limit * sizeof(Source));
    Ls = (Line*)malloc(limit * sizeof(Line));
    Cs = (Circle*)malloc(limit * sizeof(Circle));
    Tool = (int*)malloc(limit * sizeof(int));
    Pair = (int**)malloc(limit * sizeof(int*));
    for (int i = 0; i < limit; ++i)
        Pair[i] = (int*)malloc(2 * sizeof(int));
    Count = (int*)malloc(limit * sizeof(int));
    InputBox(s, 20, L"输入条件中点的数量");
    Init[1] = _wtoi(s);
    for (int i = 0; i < Init[1]; ++i)
    {
        wstring str = L"输入点P";
        str += (char)(i + 1 + '0');
        wstring sx = str + L"的x坐标", sy = str + L"的y坐标";
        InputBox(s, 20, sx.c_str());
        Ps[i].x = wcstod(s, NULL);
        InputBox(s, 20, sy.c_str());
        Ps[i].y = wcstod(s, NULL);

    }
    InputBox(s, 20, L"输入条件中线的数量");
    Init[2] = _wtoi(s);
    for (int i = 0; i < Init[2]; ++i)
    {
        wstring str = L"输入线L";
        str += (char)(i + 1 + '0');
        wstring sa = str + L"方程ax+by=c中的a", sb0 = str + L"方程ax+by=c中的b", sc = str + L"方程ax+by=c中的c";
        double a, b, c;
        InputBox(s, 20, sa.c_str());
        a = wcstod(s, NULL);
        InputBox(s, 20, sb0.c_str());
        b = wcstod(s, NULL);
        InputBox(s, 20, sc.c_str());
        c = wcstod(s, NULL);
        if (!IsZero(b)) { Ls[i] = { a / b, 1, c / b }; }
        else if (!IsZero(a)) { Ls[i] = { 1, 0, c / a }; }
    }
    InputBox(s, 20, L"输入条件中圆的数量");
    Init[3] = _wtoi(s);
    for (int i = 0; i < Init[3]; ++i)
    {
        wstring str = L"输入圆C";
        str += (char)(i + 1 + '0');
        wstring sa = str + L"方程(x-a)^2+(y-b)^2=r^2中的a", sb0 = str + L"方程(x-a)^2+(y-b)^2=r^2中的b", sr = str + L"方程(x-a)^2+(y-b)^2=r^2中的r";
        InputBox(s, 20, sa.c_str());
        Cs[i].a = wcstod(s, NULL);
        InputBox(s, 20, sb0.c_str());
        Cs[i].b = wcstod(s, NULL);
        InputBox(s, 20, sr.c_str());
        Cs[i].r = wcstod(s, NULL);
    }
    Init[0] = Init[2] + Init[3];


    E = Init[0];
    NumP = Init[1];
    NumL = Init[2];
    NumC = Init[3];

    long long Toollimit;
    cleardevice();
    settextstyle(30, 0, NULL);
    if (ModeNote == 0)//lines and circles
    {
        Toollimit = (long long)pow(3, limit);

        wstring str = L"所有可能的方法数：" + to_wstring(Toollimit);
        outtextxy(200, 200, str.c_str());
        for (int j = 0; j < Toollimit; ++j)
        {
            ToolNote = j;
            wstring s2 = L"进度：" + to_wstring((j * 1.0 / Toollimit) * 100) + L"%";
            cleardevice();
            outtextxy(200, 200, str.c_str());
            outtextxy(200, 300, s2.c_str());
            for (int i = 0; i < limit; ++i)
            {
                Tool[i] = ToolNote % 3;
                Pair[i][0] = 1; Pair[i][1] = 2;
                Count[i] = 0;
                ToolNote /= 3;
            }

            while (E < limit && Pair[0][1] <= Init[1])
            {
                if (Tool[E] == 0)
                {
                    Cs[NumC] = DrawC(Ps[NumP - Pair[E][0]], Ps[NumP - Pair[E][1]]);
                    if (!IsRepC())
                    {
                        ++NumC; t = 1;
                        AllCandL(); AllCandC();
                    }
                    else Next();
                }
                else if (Tool[E] == 1)
                {
                    Cs[NumC] = DrawC(Ps[NumP - Pair[E][1]], Ps[NumP - Pair[E][0]]);
                    if (!IsRepC())
                    {
                        ++NumC; t = 1;
                        AllCandL(); AllCandC();
                    }
                    else Next();
                }
                else if (Tool[E] == 2)
                {
                    Ls[NumL] = DrawL(Ps[NumP - Pair[E][0]], Ps[NumP - Pair[E][1]]);
                    if (!IsRepL())
                    {
                        ++NumL; t = 1;
                        AllLandL(); AllLandC();
                    }
                    else Next();
                }
                if (t) { t = 0; ++E; }
                IsGoalMet = 1;
                for (int i = 0; i < NumG; ++i) //Checking
                {
                    int s = 0;
                    if (GoalNote == 0)s = CanDrawC(CGoal[i]);
                    if (GoalNote == 1)s = CanDrawL(LGoal[i]);
                    if (GoalNote == 2)s = IsDrawnP(PGoal[i]);
                    if (!s) { IsGoalMet = 0; break; }
                }
                if (IsGoalMet) { PrintSol(j); return 0; }

                /*if (GoalNote == 0)
                    for (int i = 0; i < NumP; ++i)
                        if (IsZero(CGoal.a - Ps[i].x) && IsZero(CGoal.b - Ps[i].y))
                            for (int k = 0; k < NumP; ++k)
                                if (IsOnC(Ps[k], CGoal))
                                {
                                    PrintSol(j); goto END;
                                }
                if (GoalNote == 1)
                {
                    int CountP = 0;
                    for (int i = 0; i < NumP; ++i)
                        if (IsOnL(Ps[i], LGoal)) ++CountP;
                    if (CountP >= 2) { PrintSol(j); goto END; }
                }
                if (GoalNote == 2)
                    for (int i = 0; i < NumP; ++i)
                        if (IsSameP(Ps[i], PGoal)) { PrintSol(j); goto END; }*/

                if (E == limit) { Clear(E - 1); Next(); }
            }
            E = Init[0]; NumP = Init[1]; NumL = Init[2]; NumC = Init[3];
        }
    }
    else if (ModeNote == 1)//only lines
    {
        wstring str = L"所有可能的方法数：" + to_wstring(limit);
        outtextxy(200, 200, str.c_str());
        for (int j = 0; j < limit; ++j)
        {
            Tool[j] = 2;
            Pair[j][0] = 1; Pair[j][1] = 2;
            Count[j] = 0;
        }
        while (E < limit && Pair[0][1] <= Init[1])
        {
            Ls[NumL] = DrawL(Ps[NumP - Pair[E][0]], Ps[NumP - Pair[E][1]]);
            if (!IsRepL())
            {
                ++NumL; t = 1;
                AllLandL(); AllLandC();
            }
            else Next();
            if (t) { t = 0; ++E; }
            IsGoalMet = 1;
            for (int i = 0; i < NumG; ++i) //Checking
            {
                int s = 0;
                //if (GoalNote == 0)s = CanDrawC(CGoal[i]);
                if (GoalNote == 1)s = CanDrawL(LGoal[i]);
                if (GoalNote == 2)s = IsDrawnP(PGoal[i]);
                if (!s) { IsGoalMet = 0; break; }
            }
            if (IsGoalMet) { PrintSol(limit); return 0; }
            /*if (GoalNote == 1)
            {
                int CountP = 0;
                for (int i = 0; i < NumP; ++i)
                    if (IsOnL(Ps[i], LGoal)) ++CountP;
                if (CountP >= 2) { PrintSol(limit); goto END; }
            }
            if (GoalNote == 2)
                for (int i = 0; i < NumP; ++i)
                    if (IsSameP(Ps[i], PGoal)) { PrintSol(limit); goto END; }*/

            if (E == limit) { Clear(E - 1); Next(); }
        }
        E = Init[0]; NumP = Init[1]; NumL = Init[2]; NumC = Init[3];
    }
    else if (ModeNote == 2)//only circles
    {
        Toollimit = (long long)pow(2, limit);
        wstring str = L"所有可能的方法数：" + to_wstring(Toollimit);
        outtextxy(200, 200, str.c_str());
        for (int j = 0; j < Toollimit; ++j)
        {
            ToolNote = j;
            wstring s2 = L"进度：" + to_wstring((j * 1.0 / Toollimit) * 100) + L"%";
            cleardevice();
            outtextxy(200, 200, str.c_str());
            outtextxy(200, 300, s2.c_str());
            //printf("\r%lf%%", (j * 1.0 / Toollimit) * 100);
            for (int i = 0; i < limit; ++i)
            {
                Tool[i] = ToolNote % 2;
                Pair[i][0] = 1; Pair[i][1] = 2;
                Count[i] = 0;
                ToolNote /= 2;
            }

            while (E < limit && Pair[0][1] <= Init[1])
            {
                if (Tool[E] == 0)
                {
                    Cs[NumC] = DrawC(Ps[NumP - Pair[E][0]], Ps[NumP - Pair[E][1]]);
                    if (!IsRepC())
                    {
                        ++NumC; t = 1;
                        AllCandL(); AllCandC();
                    }
                    else Next();
                }
                else if (Tool[E] == 1)
                {
                    Cs[NumC] = DrawC(Ps[NumP - Pair[E][1]], Ps[NumP - Pair[E][0]]);
                    if (!IsRepC())
                    {
                        ++NumC; t = 1;
                        AllCandL(); AllCandC();
                    }
                    else Next();
                }
                if (t) { t = 0; ++E; }
                IsGoalMet = 1;
                for (int i = 0; i < NumG; ++i) //Checking
                {
                    int s = 0;
                    if (GoalNote == 0)s = CanDrawC(CGoal[i]);
                    //if (GoalNote == 1)s = CanDrawL(LGoal[i]);
                    if (GoalNote == 2)s = IsDrawnP(PGoal[i]);
                    if (!s) { IsGoalMet = 0; break; }
                }
                if (IsGoalMet) { PrintSol(j); return 0; }
                /*if (GoalNote == 0)
                    for (int i = 0; i < NumP; ++i)
                        if (IsZero(CGoal.a - Ps[i].x) && IsZero(CGoal.b - Ps[i].y))
                            for (int k = 0; k < NumP; ++k)
                                if (IsOnC(Ps[k], CGoal))
                                {
                                    PrintSol(j); goto END;
                                }
                if (GoalNote == 2)
                    for (int i = 0; i < NumP; ++i)
                        if (IsSameP(Ps[i], PGoal)) { PrintSol(j); goto END; }*/

                if (E == limit) { Clear(E - 1); Next(); }
            }
            E = Init[0]; NumP = Init[1]; NumL = Init[2]; NumC = Init[3];
        }
    }
    cleardevice();
    outtextxy(300, 300, L"限定步数内未找到解");
    while (1);

END:
    for (int i = 0; i < limit; ++i) free(Pair[i]);
    free(Ps); free(PointSource); free(Ls); free(Cs); free(Tool); free(Pair); free(Count);
    while (1);
    return 0;
}
