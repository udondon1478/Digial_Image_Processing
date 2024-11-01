//g++ dip13b.cpp -std=c++11 `pkg-config --cflags --libs opencv4` -framework OpenGL -framework GLUT -Wno-deprecated
//g++ dip13b.cpp `pkg-config --cflags --libs opencv` -framework OpenGL -framework GLUT -Wno-deprecated
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ
#include <GLUT/glut.h>  //OpenGL
#include <math.h>  //数学関数

//関数名の宣言
void initGL();
void initCV();
void display();
void reshape(int w, int h);
void timer(int value);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);

//グローバル変数
double eDist, eDegX, eDegY;  //視点極座標
int mX, mY, mState, mButton;  //マウス座標
int winW, winH;  //ウィンドウサイズ
double fr = 60.0;  //フレームレート
double boxPos[3][3];  //ボックス座標
cv::VideoCapture capture;
cv::Size imageSize;
cv::Mat frameImage;
cv::VideoWriter rec;
std::vector<cv::Point> prevPoints;
std::vector<std::deque<double>> prevAngles;

//main関数
int main(int argc, char* argv[])
{
    rec = cv::VideoWriter("rec.mpg", cv::VideoWriter::fourcc('P','I','M','1'), 30, cv::Size(720,405));
    prevAngles = std::vector<std::deque<double>>(3, std::deque<double>());
    //OpenGL初期化
    glutInit(&argc, argv);
    
    //OpenCV初期設定処理
    initCV();

    //OpenGL初期設定処理
    initGL();
    
    //イベント待ち無限ループ
    glutMainLoop();
    
    return 0;
}

void initCV()
{
    //ビデオキャプチャの初期化
    capture = cv::VideoCapture("movingobjects.mov");  //カメラ0番をオープン
    if (capture.isOpened()==0) {  //オープンに失敗した場合
        printf("Capture not found\n");
        exit(0);
    }
    
    //画像格納用インスタンス準備
    int imageWidth=720, imageHeight=405;
    imageSize = cv::Size(imageWidth, imageHeight);  //画像サイズ
    frameImage = cv::Mat(imageSize, CV_8UC3);  //3チャンネル

    //画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
}

//OpenGL初期設定処理
void initGL()
{
    //初期設定
    glutInitWindowSize(720, 405);  //ウィンドウサイズ指定
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);  //ディスプレイモード設定
    
    //OpenGLウィンドウ生成
    glutCreateWindow("GL");
    glutInitWindowPosition(0, 0);
    
    //ウィンドウ消去色設定
    glClearColor(0.0, 0.0, 0.2, 1.0);
    
    //機能有効化
    glEnable(GL_DEPTH_TEST);  //デプスバッファ
    glEnable(GL_NORMALIZE);  //法線ベクトル正規化
    glEnable(GL_LIGHTING);  //陰影付け
    
    //光原設定
    GLfloat col[4];  //パラメータ(RGBA)
    glEnable(GL_LIGHT0);  //光源0
    col[0] = 0.9; col[1] = 0.9; col[2] = 0.9; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  //光源0の拡散反射の強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);  //光源0の鏡面反射の強度
    col[0] = 0.05; col[1] = 0.05; col[2] = 0.05; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);  //光源0の環境光の強度

    //コールバック関数
    glutDisplayFunc(display);  //ディスプレイコールバック関数の指定
    glutReshapeFunc(reshape);  //リシェイプコールバック関数の指定
    glutMouseFunc(mouse);  //マウスクリックコールバック関数の指定
    glutMotionFunc(motion);  //マウスドラッグコールバック関数の指定
    glutKeyboardFunc(keyboard);  //キーボードコールバック関数の指定
    glutTimerFunc(1000/fr, timer, 0);  //タイマーコールバック関数の指定
    
    //視点極座標初期値
    eDist = 1400; eDegX = 30.0; eDegY = 10.0;
    
    //ボックス座標初期値
    boxPos[0][0] = -100.0; boxPos[0][1] = 0.0; boxPos[0][2] = 0.0;  //赤ボックス
    boxPos[1][0] = 0.0; boxPos[1][1] = 0.0; boxPos[1][2] = 0.0;  //緑ボックス
    boxPos[2][0] = 100.0; boxPos[2][1] = 0.0; boxPos[2][2] = 0.0;  //青ボックス
}

//ディスプレイコールバック関数
void display()
{
    //=====================OpenCV=====================
    //ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
    cv::Mat originalImage;
    capture >> originalImage;
    //ビデオが終了したら先頭に戻す
    if (originalImage.data==NULL) {
        exit(0);
    }
    //"originalImage"をリサイズして"frameImage"生成
    cv::resize(originalImage, frameImage, imageSize);
    
    cv::Scalar colors[] = {cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255)};
    std::vector<cv::Point> ansPoint;
    
    cv::Mat hsvImage;
    cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);
    
    std::vector<cv::Mat> splitedHSVImage;
    cv::split(hsvImage, splitedHSVImage);
    
    cv::Mat centerImage;
    
    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3,3));
    cv::erode(splitedHSVImage[2], centerImage, element, cv::Point(-1,-1), 15);
   
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(centerImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    if(prevPoints.size() == 0) {
        for(int i = 0; i < contours.size(); i++) {
            size_t n = contours[i].size();
            cv::Point center = cv::Point((contours[i][0].x + contours[i][n/2].x)/2, (contours[i][0].y + contours[i][n/2].y)/2);
            ansPoint.push_back(center);
            prevPoints = std::vector<cv::Point>(3);
        }
    } else {
        ansPoint = prevPoints;
        for(int j = 0; j < contours.size(); j++) {
            size_t n = contours[j].size();
            cv::Point center = cv::Point((contours[j][0].x + contours[j][n/2].x)/2, (contours[j][0].y + contours[j][n/2].y)/2);

            // search min distance
            double minDistance = 1000000;
            int minIdx = -1;
            for(int i = 0; i < prevPoints.size(); i++) {
                double distance = cv::norm(center - prevPoints[i]);
                if(distance < minDistance) {
                    minDistance = distance;
                    minIdx = i;
                }
            }
            ansPoint[minIdx] = center;
        }
    }
    
    for(int i = 0; i < 3; i++) {
        cv::circle(frameImage, ansPoint[i], 3, colors[i], -1);
    }
    
    cv::Mat contoursImage;
    cv::erode(splitedHSVImage[2], contoursImage, element, cv::Point(-1,-1), 5);
    cv::dilate(contoursImage, contoursImage, element, cv::Point(-1,-1), 5);
    
    cv::findContours(contoursImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    for(int i = 0; i < contours.size(); i++) {
        size_t n = contours[i].size();
        cv::Point center = cv::Point((contours[i][0].x + contours[i][n/2].x)/2, (contours[i][0].y + contours[i][n/2].y)/2);
        double minDistance = 1000000;
        int minIdx = -1;
        for(int i = 0; i < ansPoint.size(); i++) {
            double distance = cv::norm(center - ansPoint[i]);
            if(distance < minDistance) {
                minDistance = distance;
                minIdx = i;
            }
        }
        
        cv::drawContours(frameImage, contours, i, colors[minIdx]);
    }
    
    //表示
    cv::imshow("Frame", frameImage);
    
    rec << frameImage;
    
    std::vector<double> angles;
    for(int i = 0; i < ansPoint.size(); i++) {
        int idx = ansPoint.size() - 1 - i;
        cv::Point tmp = ansPoint[idx];
        tmp.x -= 720 / 2.0;
        tmp.x *= 1280 / 720.0;
        tmp.y -= 405 / 2.0;
        tmp.y *= 720 / 405.0;
        boxPos[i][0] = tmp.x; boxPos[i][1] = 0.0; boxPos[i][2] = tmp.y;
        
        double angle = atan2(ansPoint[idx].y - prevPoints[idx].y, ansPoint[idx].x - prevPoints[idx].x);
        angle = angle * 180 / M_PI;
        prevAngles[i].push_back(angle);
        if(prevAngles[i].size() >= 5) {
            prevAngles[i].pop_front();
        }
        
        double angleAvg = 0;
        for(int j = 0; j < prevAngles[i].size(); j++) {
            angleAvg += prevAngles[i].at(j);
        }
        angleAvg /= prevAngles[i].size();
        angles.push_back(angleAvg);
    }

    //=====================OpenGL=====================
    GLfloat col[4];  //色設定用
    
    //ウィンドウ内消去
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //行列初期化
    glLoadIdentity();
    
    //視点座標の計算
    double ex = eDist*cos(eDegX*M_PI/180.0)*sin(eDegY*M_PI/180.0);
    double ey = eDist*sin(eDegX*M_PI/180.0);
    double ez = eDist*cos(eDegX*M_PI/180.0)*cos(eDegY*M_PI/180.0);
    
    //視点視線の設定
    gluLookAt(ex, ey, ez, 0.0, 50.0, 0.0, 0.0, 1.0, 0.0);  //変換行列に視野変換行列を乗算
    
    //光源0の位置指定
    GLfloat pos0[] = {100.0, 300.0, 200.0, 1.0};  //(x, y, z, 0(平行光源)/1(点光源))
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    
    //--------------------床面--------------------
    //色設定
    col[0] = 0.5; col[1] = 0.5; col[2] = 0.5;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);  //拡散反射係数
    col[0] = 0.5; col[1] = 0.5; col[2] = 0.5; col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);  //ハイライト係数
    glPushMatrix();  //行列一時保存
    glTranslated(0.0, -20.0, 0.0);
    glScaled(1280, 1.0, 720);  //拡大縮小
    glNormal3d(0.0, 1.0, 0.0);
    glBegin(GL_QUADS);
    glVertex3d(-0.5, 0.0, -0.5);
    glVertex3d(-0.5, 0.0, 0.5);
    glVertex3d(0.5, 0.0, 0.5);
    glVertex3d(0.5, 0.0, -0.5);
    glEnd();
    glPopMatrix();  //行列復帰

    //--------------------赤ボックス(0)--------------------
    //色設定
    col[0] = 1.0; col[1] = 0.5; col[2] = 0.5;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);  //拡散反射係数
    col[0] = 0.5; col[1] = 0.5; col[2] = 0.5; col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);  //ハイライト係数
    //配置
    glPushMatrix();
    glTranslated(boxPos[0][0], boxPos[0][1], boxPos[0][2]);  //中心座標
    glRotated(angles[0], 0.0, 1.0, 0.0);
    glScaled(40.0, 40.0, 70.0);  //サイズ
    glutSolidCube(1.0);  //立方体の配置
    glPopMatrix();
    
    //--------------------緑ボックス(1)--------------------
    //色設定
    col[0] = 0.5; col[1] = 1.0; col[2] = 0.5;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);  //拡散反射係数
    col[0] = 0.5; col[1] = 0.5; col[2] = 0.5; col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);  //ハイライト係数
    //配置
    glPushMatrix();
    glTranslated(boxPos[1][0], boxPos[1][1], boxPos[1][2]);  //中心座標
    glRotated(angles[1], 0.0, 1.0, 0.0);
    glScaled(40.0, 40.0, 70.0);  //サイズ
    glutSolidCube(1.0);  //立方体の配置
    glPopMatrix();

    //--------------------青ボックス(2)--------------------
    //色設定
    col[0] = 0.5; col[1] = 0.5; col[2] = 1.0;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);
    col[0] = 0.5; col[1] = 0.5; col[2] = 0.5; col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);
    //配置
    glPushMatrix();
    glTranslated(boxPos[2][0], boxPos[2][1], boxPos[2][2]);  //中心座標
    glRotated(angles[2], 0.0, 1.0, 0.0);
    glScaled(40.0, 40.0, 70.0);  //サイズ
    glutSolidCube(1.0);  //立方体の配置
    glPopMatrix();

    //描画実行
    glutSwapBuffers();
    
    for(int i = 0; i < 3; i++) {
        prevPoints[i] = ansPoint[i];
    }
}

//タイマーコールバック関数
void timer(int value)
{
    glutPostRedisplay();  //ディスプレイイベント強制発生
    glutTimerFunc(1000/fr, timer, 0);  //タイマー再設定
}

//リシェイプコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);  //ウィンドウ全体が描画対象
    glMatrixMode(GL_PROJECTION);  //投影変換行列を計算対象に設定
    glLoadIdentity();  //行列初期化
    gluPerspective(30.0, (double)w/(double)h, 1.0, 10000.0);  //変換行列に透視投影を乗算
    glMatrixMode(GL_MODELVIEW);  //モデルビュー変換行列を計算対象に設定
}

//マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    if (state==GLUT_DOWN) {
        //マウス情報格納
        mX = x; mY = y;
        mState = state; mButton = button;
    }
}

//マウスドラッグコールバック関数
void motion(int x, int y)
{
    if (mButton==GLUT_RIGHT_BUTTON) {
        //マウスの移動量を角度変化量に変換
        eDegY = eDegY+(mX-x)*0.5;  //マウス横方向→水平角
        eDegX = eDegX+(y-mY)*0.5;  //マウス縦方向→垂直角
    }
    
    //マウス座標格納
    mX = x; mY = y;
}

//キーボードコールバック関数(key:キーの種類，x,y:座標)
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 'q':
        case 'Q':
        case 27:
            exit(0);
    }
}
