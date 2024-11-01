//g++ main06.cpp -framework OpenGL -framework GLUT -Wno-deprecated

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h>  //OpenGL/GLUTの使用
#include <random>

//三次元ベクトル構造体: Vec_3D
typedef struct _Vec_3D
{
    double x, y, z;
} Vec_3D;

//関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void timer(int value);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
Vec_3D crossProduct(Vec_3D v1, Vec_3D v2); //ベクトル外積計算関数(正規化込み)
double vectorNormalize(Vec_3D* vec); //ベクトル正規化用関数
void mySolidCircle(double cx, double cy, double cz, double r, int p);  //円描画関数
void myTriangle(Vec_3D p0, Vec_3D p1, Vec_3D p2);  //三角形描画関数
void myTetrahedron(double scale);  //四面体描画関数
void myCylinder(double top, double bottom, double height, double slices);  //円柱描画関数
void myCone(double bottom, double height, double slices);  //円錐描画関数
void myArrow(double yajiri_bottom, double yajiri_height, double ya_bottom, double ya_height, int slices);  //矢印描画関数
void mySolidRobot(double scale);  //ロボット
void myLines(double );

//グローバル変数
double eDist, eDegX, eDegY;  //カメラ制御用
int mButton, mState, mX, mY;  //マウス情報
int autoRotFlag = 0;  //カメラ自動旋回フラグ
double t=0.0;  //時刻t
double dt=0.033;  //単位時間
Vec_3D vertex[100];  // 頂点列
int vertex_num = 0;  // 頂点の個数
std::random_device rnd;
std::mt19937 mt(rnd());
std::uniform_real_distribution<> RandXYZ(-1, 1);

//メイン関数
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);  //OpenGL/GLUTの初期化
    initGL();  //初期設定
    
    glutMainLoop();  //イベント待ち無限ループ
    
    return 0;
}

//初期化関数
void initGL()
{
    //描画ウィンドウ生成
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);  //ディスプレイモードの指定
    glutInitWindowSize(800, 600);  //ウィンドウサイズの指定
    glutCreateWindow("CG_SAMPLE");  //ウィンドウの生成
    
    //コールバック関数の指定
    glutDisplayFunc(display);  //ディスプレイコールバック関数の指定（"display()"）
    glutReshapeFunc(reshape);  //リシェイプコールバック関数の指定（"reshape()"）
    glutTimerFunc(33, timer, 0);  //タイマーコールバック関数（"timer"）
    glutMouseFunc(mouse);  //マウスクリックコールバック関数
    glutMotionFunc(motion);  //マウスドラッグコールバック関数
    glutKeyboardFunc(keyboard);  //キーボードコールバック関数

    //各種設定
    glClearColor(0.0, 0.0, 0.2, 1.0);  //ウィンドウクリア色の指定（RGBA）
    glEnable(GL_DEPTH_TEST);  //デプスバッファの有効化
    glEnable(GL_NORMALIZE);  //法線ベクトルの正規化の有効化
    
    //光源設定
    glEnable(GL_LIGHTING); //陰影付けの有効化
    glEnable(GL_LIGHT0); //光源0の有効化
    GLfloat col[] = { 0.8, 0.8, 0.8, 1.0 }; //パラメータ設定
    glLightfv(GL_LIGHT0,GL_DIFFUSE,col); //光源1の拡散反射に関する強度
    glLightfv(GL_LIGHT0,GL_SPECULAR,col); //光源1の鏡面反射に関する強度
    col[0]=0.2; col[1]=0.2; col[2]=0.2; col[3]=1.0;  //環境光用に設定し直し
    glLightfv(GL_LIGHT0,GL_AMBIENT,col); //光源1の環境光反射に関する強度
    glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,0.0); //光源1の一定減衰率の設定
    glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.05); //光源1の二次減衰率の設定
    
    //視点関係（極座標：距離，x軸周り回転角，y軸周り回転角）
    eDist = 15.0; eDegX = 20.0; eDegY = 0.0;
    
    //myLineの頂点列の初期化
    vertex_num = 10;
    for(int i =0; i<vertex_num; i++){
        double x = RandXYZ(mt);
        double y = RandXYZ(mt);
        double z = RandXYZ(mt);
        vertex[i].x = x;
        vertex[i].y = y;
        vertex[i].z = z;
    }
    
    glLineWidth(3.0);
}

//ディスプレイコールバック関数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //画面消去
    
    //視点座標の計算（極座標→直交座標）
    Vec_3D e;
    e.x = eDist*cos(eDegX*M_PI/180.0)*sin(eDegY*M_PI/180.0);
    e.y = eDist*sin(eDegX*M_PI/180.0);
    e.z = eDist*cos(eDegX*M_PI/180.0)*cos(eDegY*M_PI/180.0);
    
    //モデルビュー変換の設定
    glMatrixMode(GL_MODELVIEW);  //変換行列の指定（設定対象はモデルビュー変換行列）
    glLoadIdentity();  //行列初期化
    gluLookAt(e.x, e.y, e.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);  //視点視線設定（視野変換行列を乗算）
    
    //光源0の位置指定
    GLfloat lightpos0[] = { 1.0, 3.0, 5.0, 0.0 }; //平行光源
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos0);
    
    GLfloat col[4], spe[4], shi[1];  //材質設定用変数
    
    //床
    col[0] = 0.0; col[1] = 1.0; col[2] = 0.0; col[3] = 1.0; //拡散反射係数，環境光反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0; //鏡面反射係数
    shi[0] = 100.0; //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col); //拡散反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col); //環境光反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe); //鏡面反射係数を設定
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi); //ハイライト係数を設定
    glPushMatrix();  //行列一時保存
    glBegin(GL_QUADS);  //物体（四角形）頂点配置開始
    glVertex3d(-4.0, -1.0, -4.0);  //頂点
    glVertex3d(4.0, -1.0, -4.0);  //頂点
    glVertex3d(4.0, -1.0, 4.0);  //頂点
    glVertex3d(-4.0, -1.0, 4.0);  //頂点
    glEnd();  //物体頂点配置終了
    glPopMatrix();  //行列復帰

    double dRot = 50.0*t;
    //球
    col[0] = 1.0; col[1] = 0.0; col[2] = 0.0;  col[3] = 1.0;  //拡散反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;  //鏡面反射係数
    shi[0] = 10.0;  //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  //鏡面反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);  //ハイライト
    glPushMatrix();
    glTranslated(-3.0, 0.0, -3.0);
    glRotated(dRot, 0, 1, 0);
    glutSolidSphere(1.0, 36, 36);
    glPopMatrix();
    
    //立方体
    col[0] = 0.0; col[1] = 1.0; col[2] = 0.0;  col[3] = 1.0;  //拡散反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;  //鏡面反射係数
    shi[0] = 10.0;  //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  //鏡面反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);  //ハイライト
    glPushMatrix();
    glTranslated(0.0, 0.0, -3.0);
    glRotated(dRot, 0, 1, 0);
    glutSolidCube(2.0);
    glPopMatrix();
    
    //ドーナツ
    col[0] = 0.0; col[1] = 0.0; col[2] = 1.0;  col[3] = 1.0;  //拡散反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;  //鏡面反射係数
    shi[0] = 10.0;  //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  //鏡面反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);  //ハイライト
    glPushMatrix();
    glTranslated(3.0, 0.2, -3.0);
    glRotated(dRot, 0, 1, 0);
    glutSolidTorus(0.4, 0.8, 36, 36);
    glPopMatrix();
    
    //三角形
    Vec_3D p1 = {-1.0, -1.0, 0.0};
    Vec_3D p2 = { 1.0, -1.0, 0.0};
    Vec_3D p3 = { 0.0,  1.0, 0.0};
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0;  col[3] = 1.0;  //拡散反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;  //鏡面反射係数
    shi[0] = 10.0;  //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  //鏡面反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);  //ハイライト
    glPushMatrix();
    glTranslated(-3.0, 0.0, 0.0);
    glRotated(dRot, 0, 1, 0);
    myTriangle(p1, p2, p3);
    glPopMatrix();

    //四面体
    col[0] = 1.0; col[1] = 1.0; col[2] = 0.0;  col[3] = 1.0;  //拡散反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;  //鏡面反射係数
    shi[0] = 10.0;  //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  //鏡面反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);  //ハイライト
    glPushMatrix();
    glTranslated(0.0, 0.0, 0.0);
    glRotated(dRot, 0, 1, 0);
    myTetrahedron(2.0);
    glPopMatrix();
    
    //円柱(多角柱)
    col[0] = 1.0; col[1] = 0.0; col[2] = 1.0;  col[3] = 1.0;  //拡散反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;  //鏡面反射係数
    shi[0] = 10.0;  //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  //鏡面反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);  //ハイライト
    glPushMatrix();
    glTranslated(3.0, 0.0, 0.0);
    glRotated(dRot, 0, 1, 0);
    myCylinder(1, 1, 2, 72);
    glPopMatrix();
    
    //円錐(多角錐)
    col[0] = 0.0; col[1] = 1.0; col[2] = 1.0;  col[3] = 1.0;  //拡散反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;  //鏡面反射係数
    shi[0] = 10.0;  //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  //鏡面反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);  //ハイライト
    glPushMatrix();
    glTranslated(-3.0, 0.0, 3.0);
    glRotated(dRot, 0, 1, 0);
    myCone(1, 2, 108);
    glPopMatrix();
    
    //矢印
    col[0] = 0.5; col[1] = 0.5; col[2] = 0.5;  col[3] = 1.0;  //拡散反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;  //鏡面反射係数
    shi[0] = 10.0;  //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  //鏡面反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);  //ハイライト
    glPushMatrix();
    glTranslated(0.0, 0.0, 3.0);
    glRotated(dRot, 0, 1, 0);
    myArrow(0.2, 0.5, 0.1, 0.5, 5);
    glPopMatrix();

    //myLines
    col[0] = 1.0; col[1] = 0.2; col[2] = 0.0;  col[3] = 1.0;  //拡散反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;  //鏡面反射係数
    shi[0] = 10.0;  //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  //鏡面反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);  //ハイライト
    glPushMatrix();
    glTranslated(3.0, 0.0, 3.0);
    glRotated(dRot, 0, 1, 0);
    myLines(1.0);
    glPopMatrix();

    glutSwapBuffers();  //描画実行
    
    if(autoRotFlag){
        eDegY += 1.0;  //視点移動
    }
}

//リシェイプコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);  //ビューポート設定
    //投影変換の設定
    glMatrixMode(GL_PROJECTION);  //変換行列の指定（設定対象は投影変換行列）
    glLoadIdentity();  //行列初期化
    gluPerspective(30.0, (double)w/(double)h, 1.0, 1000.0);  //透視投影ビューボリューム設定
}

//タイマーコールバック関数
void timer(int value)
{
    t += dt;
    
    glutPostRedisplay();  //ディスプレイイベント強制発生
    glutTimerFunc(33, timer, 0);  //タイマー再設定
}

//マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    //マウス情報をグローバル変数に格納
    mButton = button; mState = state; mX = x; mY = y;
}

//マウスドラッグコールバック関数
void motion(int x, int y)
{
    //左ボタンドラッグでカメラの位置変更
    if (mButton==GLUT_LEFT_BUTTON){
        eDegY += (mX - x)*0.5;  //マウス横方向→水平角
        eDegX += (y - mY)*0.5;  //マウス縦方向→垂直角
    }
    //右ボタンドラッグでズーム変更
    else if (mButton==GLUT_RIGHT_BUTTON){
        eDist -= (mX - x)*0.1;  //マウス横方向→水平角
    }
    
    //マウス座標をグローバル変数に保存
    mX = x; mY = y;
}

void keyboard(unsigned char key, int x, int y)
{
    printf("KEY: ASCII code=%d, x=%d, y=%d\n", key, x, y);
    switch (key) {
        case 27:  //[ESC]キー
        case 'q':  //[q]キー
        case 'Q':  //[Q]キー
            exit(0);  //プロセス終了
            break;
        case ' ':  //スペースキー
            autoRotFlag = (autoRotFlag+1)%2;
            break;
        default:
            break;
    }
}

//2 ベクトルの外積計算
Vec_3D crossProduct(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out;
    
    out.x = v1.y*v2.z-v1.z*v2.y;
    out.y = v1.z*v2.x-v1.x*v2.z;
    out.z = v1.x*v2.y-v1.y*v2.x;
    vectorNormalize(&out);
    
    return out; //戻り値は外積ベクトル
}

//ベクトル正規化
double vectorNormalize(Vec_3D* vec)
{
    double len;
    
    //ベクトル長
    len = sqrt(vec->x*vec->x+vec->y*vec->y+vec->z*vec->z);
    if (len>0) {
        //正規化
        vec->x = vec->x/len; vec->y = vec->y/len; vec->z = vec->z/len;
    }
    return len; //戻り値はベクトル長
}

//円描画関数
void mySolidCircle(double cx, double cy, double cz, double r, int p)
{
    double x, y, z;  //座標計算結果
    double t, delta;  //パラメータt 及び変位量 delta
    
    delta = 2.0*M_PI/p;
    
    //折れ線近所の開始
    glBegin(GL_POLYGON);
    glNormal3d(0, 0, 1);
    for(int i=0; i<=p; i++){
        t = delta*i;
        x = r*cos(t) + cx;
        y = r*sin(t) + cy;
        z = cz;
        glVertex3d(x, y, z);  //円周上の頂点s
    }
    glEnd();
}

//三角形描画関数
void myTriangle(Vec_3D p1, Vec_3D p2, Vec_3D p3)
{
    Vec_3D v1, v2, nv; //ベクトルv1, v2と法線nv
    v1.x = p2.x-p1.x; v1.y = p2.y-p1.y; v1.z = p2.z-p1.z;
    v2.x = p3.x-p1.x; v2.y = p3.y-p1.y; v2.z = p3.z-p1.z;
    nv = crossProduct(v1, v2);
    glPushMatrix();
        glNormal3d(nv.x, nv.y, nv.z);
        glBegin(GL_TRIANGLES);
        glVertex3d(p1.x, p1.y, p1.z);
        glVertex3d(p2.x, p2.y, p2.z);
        glVertex3d(p3.x, p3.y, p3.z);
        glEnd();
    glPopMatrix();
}

//四面体描画関数
void myTetrahedron(double scale)
{
    Vec_3D p1 = {-0.5, -0.3, 0.0};  //底面第1頂点
    Vec_3D p2 = { 0.5, -0.3, 0.0};  //底面第2頂点
    Vec_3D p3 = { 0.0,  0.5, 0.0};  //底面第3頂点
    Vec_3D p4 = { 0.0,  0.0, 0.5};  //錐の頂点

    glPushMatrix();
        glScaled(scale, scale, scale);
        myTriangle(p1, p3, p2);
        myTriangle(p2, p3, p4);
        myTriangle(p1, p4, p3);
        myTriangle(p1, p2, p4);
    glPopMatrix();
}

//円柱描画関数
void myCylinder(double top, double bottom, double height, double slices)
{
    glPushMatrix();
    //上面の円
    glPushMatrix();
    glTranslated(0, 0, height/2);
    mySolidCircle(0, 0, 0, top, slices);
    glPopMatrix();
    //底面の円
    glPushMatrix();
    glTranslated(0, 0, -height/2);
    glRotated(180, 0, 0, 1);
    glRotated(180, 0, 1, 0);
    mySolidCircle(0, 0, 0, bottom, slices);
    glPopMatrix();
    glPopMatrix();
    
    //胴体
    Vec_3D p1, p2, p3, p4;  //４頂点
    Vec_3D v1, v2, nv;
    double t1, t2, x, y, z;
    double delta = 2.0*M_PI/slices;
    //折れ線近似の開始
    glBegin(GL_QUADS);
    for(int i=0; i<(int)slices; i++){  //円周上のサンプリング
        t1 = delta*i;
        //p1の計算
        x = top*cos(t1);
        y = top*sin(t1);
        z = height/2;
        p1.x = x; p1.y = y; p1.z = z;
        //p2の計算
        x = bottom*cos(t1);
        y = bottom*sin(t1);
        z = -height/2;
        p2.x = x; p2.y = y; p2.z = z;
        t2 = delta*(i+1);
        //p4の計算
        x = top*cos(t2);
        y = top*sin(t2);
        z = height/2;
        p4.x = x; p4.y = y; p4.z = z;
        //p3の計算
        x = bottom*cos(t2);
        y = bottom*sin(t2);
        z = -height/2;
        p3.x = x; p3.y = y; p3.z = z;
        //法線の計算
        v1.x = p2.x-p1.x; v1.y = p2.y-p1.y; v1.z = p2.z-p1.z;
        v2.x = p3.x-p1.x; v2.y = p3.y-p1.y; v2.z = p3.z-p1.z;
//        v1 = diffVec(p2, p1);
//        v2 = diffVec(p3, p1);
        nv = crossProduct(v1, v2);
        glNormal3d(nv.x, nv.y, nv.z);
        //４頂点に四角形ポリゴンを貼り付け
        glVertex3d(p1.x, p1.y, p1.z);  //円周上の頂点s
        glVertex3d(p2.x, p2.y, p2.z);  //円周上の頂点s
        glVertex3d(p3.x, p3.y, p3.z);  //円周上の頂点s
        glVertex3d(p4.x, p4.y, p4.z);  //円周上の頂点s
    }
    glEnd();

}

//円錐描画関数
void myCone(double bottom, double height, double slices)
{
    myCylinder(0, bottom, height, slices);
}

//矢印描画関数
void myArrow(double yajiri_bottom, double yajiri_height, double ya_bottom, double ya_height, int slices)
{
    glPushMatrix();
    glTranslated(0.0, 0.0, ya_height/2);
    myCylinder(ya_bottom, ya_bottom, ya_height, slices);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0.0, 0.0, ya_height+yajiri_height/2);
    myCone(yajiri_bottom, yajiri_height, slices);
    glPopMatrix();
}

//ロボット
void mySolidRobot(double scale)
{
    double doutai = 1.0;  //胴体の大きさ
    double ude = doutai/4;  //腕の太さ
    double ashi = ude;  //脚の太さ
    double atama = doutai/4;  //頭の大きさ
    double hana = atama/4;  //鼻の大きさ
    
    glPushMatrix();
    glScaled(scale, scale, scale);
    glTranslated(0.0, doutai*1.5, 0.0);  //脚を地面に付ける

    //胴体
    glPushMatrix();
    glScaled(doutai, doutai, doutai/2);
    glutSolidCube(1.0);
    glPopMatrix();
    
    //右腕
    glPushMatrix();
    glTranslated((doutai+ude)/2, doutai/2, 0.0);  //肩に接続
    glRotated(30.0*sin(2.0*t), 1.0, 0.0, 0.0);
    glScaled(ude, doutai, ude);
    glTranslated(0.0, -0.5, 0.0);  //腕の上面を原点に
    glutSolidCube(1.0);
    glPopMatrix();

    //左腕
    glPushMatrix();
    glTranslated(-(doutai+ude)/2, doutai/2, 0.0);  //肩に接続
    glRotated(30.0*sin(2.0*t), -1.0, 0.0, 0.0);
    glScaled(ude, doutai, ude);
    glTranslated(0.0, -0.5, 0.0);  //腕の上面を原点に
    glutSolidCube(1.0);
    glPopMatrix();
    
    //右脚
    glPushMatrix();
    glTranslated((doutai-ashi)/2, -doutai/2, 0.0);  //肩に接続
    glRotated(30.0*sin(2.0*t), -1.0, 0.0, 0.0);
    glScaled(ashi, doutai, ashi);
    glTranslated(0.0, -0.5, 0.0);  //腕の上面を原点に
    glutSolidCube(1.0);
    glPopMatrix();

    //左脚
    glPushMatrix();
    glTranslated(-(doutai-ashi)/2, -doutai/2, 0.0);  //肩に接続
    glRotated(30.0*sin(2.0*t), 1.0, 0.0, 0.0);
    glScaled(ashi, doutai, ashi);
    glTranslated(0.0, -0.5, 0.0);  //腕の上面を原点に
    glutSolidCube(1.0);
    glPopMatrix();

    //頭
    glPushMatrix();
    glTranslated(0.0, doutai/2, 0.0);  //腕の上面を原点に
    glScaled(atama, atama, atama);
    glTranslated(0.0, 1.0, 0.0);  //腕の上面を原点に
    glutSolidSphere(1.0, 12, 12);
    glPopMatrix();
    //鼻
    glColor4d(1.0, 0.0, 0.0, 1.0);
    glPushMatrix();
    glTranslated(0.0, doutai/2+atama, atama);  //腕の上面を原点に
    glScaled(hana, hana, hana);
    glutSolidSphere(1.0, 12, 12);
    glPopMatrix();

    glPopMatrix();
}

void myLines(double scale)
{
    glPushMatrix();
    glScaled(scale, scale, scale);
    
    glBegin(GL_LINE_STRIP);
    for(int i=0; i<vertex_num; i++){
        glVertex3d(vertex[i].x, vertex[i].y, vertex[i].z);
    }
    glEnd();
    
    glPopMatrix();
}
