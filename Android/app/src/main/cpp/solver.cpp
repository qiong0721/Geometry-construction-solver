#include <jni.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string.h>

#define epsilon (1e-8)
#define sq(a) ((a) * (a))
using namespace std;

typedef struct {
    double x, y;
} Point;

typedef struct {
    double a, b, c; // ax + by = c
} Line;

typedef struct {
    double a, b, r; // (x-a)^2 + (y-b)^2 = r^2
} Circle;

// 点的来源类型
#define SOURCE_LL 0
#define SOURCE_LC 1
#define SOURCE_CC 2

typedef struct {
    int type;
    int idx1;
    int idx2;
} Source;

typedef struct {
    int step_index;
    Point p1;
    Point p2;
    int point_index_1;
    int point_index_2;
    vector<int> PointList;
} ResultStep;
//typedef struct{
//    vector<ResultStep>finalStep;
//    int goalmode;
//    double target[3];
//    Point* initP;
//    Line* initL;
//    Circle* initC;
//}FinResult;
static int E, NumP, NumL, NumC, IsGoalMet,modeNote,GoalNote;
static Point PGoal;
static Line LGoal;
static Circle CGoal;
static Point* Ps;
static Source* PointSource;
static Line* Ls;
static Circle* Cs;
static int* Tool;
static int** Pair;
static int* Count;
static int Init[4];
static int limit;

static vector<ResultStep> finalResults;

int IsZero(double i) { return fabs(i) < epsilon; }

int IsSameP(Point P1, Point P2) { return IsZero(P1.x - P2.x) && IsZero(P1.y - P2.y); }

int IsSameL(Line L1, Line L2) { return IsZero(L1.a - L2.a) && IsZero(L1.b - L2.b) && IsZero(L1.c - L2.c); }

int IsSameC(Circle C1, Circle C2) { return IsZero(C1.a - C2.a) && IsZero(C1.b - C2.b) && IsZero(C1.r - C2.r); }

int IsOnL(Point P, Line L) { return IsZero(L.a * P.x + L.b * P.y - L.c); }

int IsOnC(Point P, Circle C) { return IsZero(sq(P.x - C.a) + sq(P.y - C.b) - sq(C.r)); }

Line DrawL(Point P1, Point P2) {
    double a = P2.y - P1.y;
    double b = P1.x - P2.x;
    double c = P1.x * P2.y - P1.y * P2.x;
    Line L;

    if (!IsZero(b)) {
        L = (Line){a / b, 1, c / b};
    } else if (!IsZero(a)) {
        L = (Line){1, 0, c / a};
    }
    return L;
}

Circle DrawC(Point P1, Point P2) {
    Circle C = {P1.x, P1.y, sqrt(sq(P1.x - P2.x) + sq(P1.y - P2.y))};
    return C;
}
void Clear(int i);
int IsRepP() {
    for (int i = 0; i < NumP; ++i)
        if (IsSameP(Ps[i], Ps[NumP])) return 1;
    ++NumP;
    ++Count[E];
    return 0;
}
void LandL(Line L1, Line L2, double det, int idx1, int idx2) {
    double x = (L1.c * L2.b - L1.b * L2.c) / det;
    double y = (L1.a * L2.c - L1.c * L2.a) / det;
    Ps[NumP] = {x, y};
    PointSource[NumP] = {SOURCE_LL, idx1, idx2};
    IsRepP();
}

void LandC(Line L1, Circle C1, int i, double delta, int idxL, int idxC) {
    double root = sqrt(delta);
    double q = sq(L1.a) + sq(L1.b);
    double x = (L1.a * L1.c + L1.b * (L1.b * C1.a - L1.a * C1.b) - i * L1.b * root) / q;
    double y = (L1.b * L1.c - L1.a * (L1.b * C1.a - L1.a * C1.b) + i * L1.a * root) / q;
    Ps[NumP] = (Point){x, y};
    PointSource[NumP] = (Source){SOURCE_LC, idxL, idxC};
    IsRepP();
}
bool IsDrawnP() {
    for (int i = 0; i < NumP; ++i) {
        if (IsSameP(PGoal, Ps[i])) {
            return true;
        }
    }
    return false;
}
bool w=false;
bool CanDrawL() {
    int CountP = 0,LP[2];
    for (int i = 0; i < NumP&&CountP<2; ++i) {
        if (IsOnL(Ps[i],LGoal)) {
            LP[CountP]=i;
            ++CountP;

        }
    }
    if(CountP>=2)
    {
        Pair[E][0]=LP[0];
        Pair[E][1]=LP[1];
        Tool[E]=2;
        ++E;
        w=true;
    }
    return CountP >= 2;
}

bool CanDrawC() {
    bool hasCenter = false;
    int CP[2];
    for (int i = 0; i < NumP; ++i) {
        if (IsSameP(Ps[i],{CGoal.a,CGoal.b})) {
            hasCenter = true;
            CP[0]=i;
            break;
        }
    }
    if (!hasCenter) return false;

    for (int i = 0; i < NumP; ++i) {
        if (IsOnC(Ps[i],CGoal)) {
            CP[1]=i;
            w=true;
            Tool[E]=0;
                Pair[E][0]=CP[0];
                Pair[E][0]=CP[1];
            ++E;
            return true;
        }
    }
    return false;
}
void CandC(Circle C1, Circle C2, int idx1, int idx2)
{
    double a = 2 * (C1.a - C2.a);
    double b = 2 * (C1.b - C2.b);
    if (IsZero(a) && IsZero(b)) return;
    double c = sq(C1.a) - sq(C2.a) + sq(C1.b) - sq(C2.b) - sq(C1.r) + sq(C2.r);
    Line L1 = {a, b, c};
    double q = sq(L1.a) + sq(L1.b);
    double delta = sq(C1.r) * q - sq(L1.a * C1.a + L1.b * C1.b - L1.c);
    if (delta < -epsilon) return;
    if (IsZero(delta)) delta = 0;
    double root = sqrt(delta);
    double x1 = (L1.a * L1.c + L1.b * (L1.b * C1.a - L1.a * C1.b) - L1.b * root) / q;
    double y1 = (L1.b * L1.c - L1.a * (L1.b * C1.a - L1.a * C1.b) + L1.a * root) / q;
    Ps[NumP] = (Point){x1, y1};
    PointSource[NumP] = (Source){SOURCE_CC, idx1, idx2};
    IsRepP();
    if (delta > epsilon) {
        double x2 = (L1.a * L1.c + L1.b * (L1.b * C1.a - L1.a * C1.b) + L1.b * root) / q;
        double y2 = (L1.b * L1.c - L1.a * (L1.b * C1.a - L1.a * C1.b) - L1.a * root) / q;
        Ps[NumP] = (Point){x2, y2};
        PointSource[NumP] = (Source){SOURCE_CC, idx1, idx2};
        IsRepP();
    }
}

void AllLandL() {
    for (int i = 0; i < NumL - 1; ++i) {
        double det = Ls[i].a * Ls[NumL - 1].b - Ls[i].b * Ls[NumL - 1].a;
        if (IsZero(det)) continue;
        LandL(Ls[i], Ls[NumL - 1], det, i, NumL - 1);
    }
}

void AllLandC() {
    for (int i = 0; i < NumC; ++i) {
        double q = sq(Ls[NumL - 1].a) + sq(Ls[NumL - 1].b);
        double delta = sq(Cs[i].r) * q - sq(Ls[NumL - 1].a * Cs[i].a + Ls[NumL - 1].b * Cs[i].b - Ls[NumL - 1].c);
        if (delta < -epsilon) continue;
        if (IsZero(delta)) delta = 0;
        LandC(Ls[NumL - 1], Cs[i], 1, delta, NumL - 1, i);
        if (delta > epsilon) LandC(Ls[NumL - 1], Cs[i], -1, delta, NumL - 1, i);
    }
}

void AllCandL() {
    for (int i = 0; i < NumL; ++i) {
        double q = sq(Ls[i].a) + sq(Ls[i].b);
        double delta = sq(Cs[NumC - 1].r) * q - sq(Ls[i].a * Cs[NumC - 1].a + Ls[i].b * Cs[NumC - 1].b - Ls[i].c);
        if (delta < -epsilon) continue;
        if (IsZero(delta)) delta = 0;
        LandC(Ls[i], Cs[NumC - 1], 1, delta, i, NumC - 1);
        if (delta > epsilon) LandC(Ls[i], Cs[NumC - 1], -1, delta, i, NumC - 1);
    }
}

void AllCandC() {
    for (int i = 0; i < NumC - 1; ++i) {
        CandC(Cs[i], Cs[NumC - 1], i, NumC - 1);
    }
}




int IsRepL() {
    for (int i = 0; i < NumL; ++i)
        if (IsSameL(Ls[i], Ls[NumL])) return 1;
    return 0;
}

int IsRepC() {
    for (int i = 0; i < NumC; ++i)
        if (IsSameC(Cs[i], Cs[NumC])) return 1;
    return 0;
}

void Next() {
    ++Pair[E][1];
    for (int i = E; i > 0; --i) {
        if (Pair[i][1] > NumP) {
            ++Pair[i][0];
            Pair[i][1] = Pair[i][0] + 1;
        }
        if (Pair[i][0] > NumP - 1) {

            ++Pair[i - 1][1];
            Pair[i][0] = 1;
            Pair[i][1] = 2;
            Clear(i - 1);
        }
        else break;
    }
}
void Clear(int i) {
    --E;
    if (Tool[i] > 1)
        --NumL;
    else
        --NumC;
    NumP -= Count[i];
    Count[i] = 0;
}
void AssembleResult() {
    finalResults.clear();
    int currentPointIndex = Init[1];

    for (int step = Init[0]; step < E; ++step) {
        ResultStep rs;
        rs.step_index = Tool[step];
        int idx1 = currentPointIndex+1 - Pair[step][0];
        int idx2 = currentPointIndex+1 - Pair[step][1];
        if(w&&step==E-1)
        {
            idx1=Pair[step][0];
            idx2=Pair[step][1];
        }
        rs.p1 = Ps[idx1-1];
        rs.p2 = Ps[idx2-1];
        rs.point_index_1 = idx1 ;
        rs.point_index_2 = idx2 ;

        for (int k = 0; k < Count[step]; ++k) {
            rs.PointList.push_back(currentPointIndex + k + 1); // 1-based
        }
        currentPointIndex += Count[step];

        finalResults.push_back(rs);
    }

}

int t=0;
void SolveCore() {
    if (modeNote == 0) {
        long long Toollimit = 1;
        for (int i = 0; i < limit; ++i) Toollimit *= 3;

        bool found = false;
        for (long long j = 0; j < Toollimit && !found; ++j) {
            long long toolNote = j;
            for (int i = 0; i < limit; ++i) {
                Tool[i] = toolNote % 3;
                Pair[i][0] = 1;
                Pair[i][1] = 2;
                Count[i] = 0;
                toolNote /= 3;
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
                int s = 0;
                if (GoalNote == 2)s = CanDrawC();
                if (GoalNote == 1)s = CanDrawL();
                if (GoalNote == 0)s = IsDrawnP();
                if (!s) { IsGoalMet = 0; }
                if (IsGoalMet) {AssembleResult();return; }

                if (E == limit) { Clear(E - 1); Next(); }
            }
            E = Init[0]; NumP = Init[1]; NumL = Init[2]; NumC = Init[3];

        }
    }
    else if (modeNote == 1)//only lines
    {

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

            int s = 0;
            //if (GoalNote == 0)s = CanDrawC(CGoal[i]);
            if (GoalNote == 1)s = CanDrawL();
            if (GoalNote == 0)s = IsDrawnP();
            if (!s) { IsGoalMet = 0; }

            if (IsGoalMet) { AssembleResult();return; }

            if (E == limit) { Clear(E - 1); Next(); }
        }
        E = Init[0]; NumP = Init[1]; NumL = Init[2]; NumC = Init[3];
    }
    else if (modeNote == 2)//only circles
    {
        long long Toollimit = (long long)pow(2, limit),ToolNote;

        for (int j = 0; j < Toollimit; ++j)
        {
            ToolNote = j;

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

                int s = 0;
                if (GoalNote == 2)s = CanDrawC();
                if (GoalNote == 0)s = IsDrawnP();
                if (!s) { IsGoalMet = 0;  }

                if (IsGoalMet) { AssembleResult();return; }
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

}

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_myapplication2_Solver_freeSolution(JNIEnv *env, jobject, jobjectArray solution) {

}

// 核心求解函数
JNIEXPORT jobjectArray JNICALL
Java_com_example_myapplication2_Solver_solveGeometryProblem(
        JNIEnv *env, jobject ,
        jdoubleArray initPoints, jint initPointCount,
        jdoubleArray initLines, jint initLineCount,
        jdoubleArray initCircles, jint initCircleCount,
        jdoubleArray targetPoints,
        jdoubleArray targetLines,
        jdoubleArray targetCircles,
        jstring mode,
        jint stepLimit,
        jint modenote,
        jint goalNote) {

    limit = stepLimit;
    modeNote=modenote;
    GoalNote=goalNote;
    int a=initLineCount+initCircleCount;

    Ps = new Point[(a+1+limit) * (1+limit+a)];
    PointSource = new Source[(a+1+limit) * (a+1+limit)];
    Ls = new Line[limit+a+1];
    Cs = new Circle[limit+a+1];
    Tool = new int[limit+a+1];
    Pair = new int*[limit+a+1];
    for (int i = 0; i < limit+a+1; ++i) Pair[i] = new int[2];
    Count = new int[limit+a+1];
    w=false;
    for(int i=0; i<limit+a+1; ++i) {
        Count[i] = 0;
        Pair[i][0] = 1;
        Pair[i][1] = 2;
    }
    NumP=0;
    NumL=0;
    NumC=0;
    t=0;
    E=0;
    PGoal={0,0};
    LGoal={0,0,0};
    CGoal={0,0,0};
//
    jdouble* pPoints = env->GetDoubleArrayElements(initPoints, nullptr);
    for (int i = 0; i < initPointCount; ++i) {
        Ps[i].x = pPoints[i * 2];
        Ps[i].y = pPoints[i * 2 + 1];
    }
    env->ReleaseDoubleArrayElements(initPoints, pPoints, 0);

    jdouble* pLines = env->GetDoubleArrayElements(initLines, nullptr);
    for (int i = 0; i < initLineCount; ++i) {
        double a = pLines[i * 3];
        double b = pLines[i * 3 + 1];
        double c = pLines[i * 3 + 2];
        if (!IsZero(b)) Ls[i] = (Line){a / b, 1, c / b};
        else if (!IsZero(a)) Ls[i] = (Line){1, 0, c / a};
        else Ls[i] = (Line){0,0,0};
    }
    env->ReleaseDoubleArrayElements(initLines, pLines, 0);

    jdouble* pCircles = env->GetDoubleArrayElements(initCircles, nullptr);
    for (int i = 0; i < initCircleCount; ++i) {
        Cs[i].a = pCircles[i * 3];
        Cs[i].b = pCircles[i * 3 + 1];
        Cs[i].r = pCircles[i * 3 + 2];
    }
    env->ReleaseDoubleArrayElements(initCircles, pCircles, 0);

    if(GoalNote==0)
    {
        jdouble* pTargets = env->GetDoubleArrayElements(targetPoints, nullptr);
        PGoal.x= pTargets[0];
        PGoal.y=pTargets[1];
        env->ReleaseDoubleArrayElements(targetPoints, pTargets, 0);
    }
    else if(GoalNote == 1)
    {
        jdouble* lTargets = env->GetDoubleArrayElements(targetLines, nullptr);
        LGoal.a= lTargets[0];
        LGoal.b=lTargets[1];
        LGoal.c=lTargets[2];
        env->ReleaseDoubleArrayElements(targetLines, lTargets, 0);
        limit--;
    }
    else if(GoalNote == 2)
    {
        jdouble* cTargets = env->GetDoubleArrayElements(targetCircles, nullptr);
        CGoal.a= cTargets[0];
        CGoal.b=cTargets[1];
        CGoal.r=cTargets[2];
        env->ReleaseDoubleArrayElements(targetCircles, cTargets, 0);
        limit--;
    }
//
    Init[1] = initPointCount;
    Init[2] = initLineCount;
    Init[3] = initCircleCount;
    Init[0] = Init[2] + Init[3];

    E = Init[0];
    limit+=E;
    NumP = Init[1];
    NumL = Init[2];
    NumC = Init[3];
//    limit=5;
//    PGoal={1,0};
//    goalNote=0;
//    modeNote=0;
//    Ps[0]={0,0};
//    Ps[1]={2,0};
//    Init[1]=2;
//    Init[2]=0;
//    Init[3]=0;
//    Init[0]=Init[2]+Init[3];
//    E = Init[0];
//    NumP = Init[1];
//    NumL = Init[2];
//    NumC = Init[3];
    finalResults.clear();
    SolveCore();

    jclass resultStepClass = env->FindClass("com/example/myapplication2/ResultStep");
    jmethodID constructor = env->GetMethodID(resultStepClass, "<init>", "(I[D[DIILjava/util/List;)V");

    jobjectArray resultArray = env->NewObjectArray(finalResults.size(), resultStepClass, nullptr);

    for (size_t i = 0; i < finalResults.size(); ++i) {
        const ResultStep& rs = finalResults[i];

        jdoubleArray p1Array = env->NewDoubleArray(2);
        jdoubleArray p2Array = env->NewDoubleArray(2);
        jdouble p1Coords[2] = {rs.p1.x, rs.p1.y};
        jdouble p2Coords[2] = {rs.p2.x, rs.p2.y};
        env->SetDoubleArrayRegion(p1Array, 0, 2, p1Coords);
        env->SetDoubleArrayRegion(p2Array, 0, 2, p2Coords);

        jclass arrayListClass = env->FindClass("java/util/ArrayList");
        jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
        jmethodID addMethod = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
        jobject pointList = env->NewObject(arrayListClass, arrayListConstructor);

        for (int idx : rs.PointList) {
            jobject intObj = env->NewObject(env->FindClass("java/lang/Integer"),
                                            env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"),
                                            idx);
            env->CallBooleanMethod(pointList, addMethod, intObj);
        }

        jobject resultStep = env->NewObject(resultStepClass, constructor,
                                            rs.step_index,
                                            p1Array,
                                            p2Array,
                                            rs.point_index_1,
                                            rs.point_index_2,
                                            pointList);

        env->SetObjectArrayElement(resultArray, i, resultStep);

        env->DeleteLocalRef(p1Array);
        env->DeleteLocalRef(p2Array);
        env->DeleteLocalRef(pointList);
        env->DeleteLocalRef(resultStep);
    }


    return resultArray;
}

} // extern "C"