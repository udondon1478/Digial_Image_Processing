//g++ dip11_gl.cpp -std=c++11 `pkg-config --cflags --libs opencv4` -framework OpenGL -framework GLUT -Wno-deprecated
#include <iostream>  //入出力関連ヘッダ
#include <GLUT/glut.h>  //OpenGL
#include <math.h>  //数学関数
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

//関数名の宣言
void initGL();
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
double fr = 30.0;  //フレームレート
cv::VideoCapture capture;
cv::Mat originalImage;
cv::Mat frameImage;
cv::Size imageSize;
// 四角形面の角度を格納する配列
float quadAngles[32][24][2]; // 32行 x 24列 x (x軸回転, y軸回転)

//main関数
int main(int argc, char* argv[])
{
    //--------------------
    //①ビデオキャプチャの初期化
    capture = cv::VideoCapture(0);  //カメラ0番をオープン
    if (capture.isOpened()==0) {  //オープンに失敗した場合
        printf("Capture not found\n");
        return -1;
    }
    
    //②画像格納用インスタンス準備
    int imageWidth=720, imageHeight=405;
    imageSize = cv::Size(imageWidth, imageHeight);  //画像サイズ
     //ビデオキャプチャ用
    frameImage = cv::Mat(imageSize, CV_8UC3);  //3チャンネル

    //③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    
    //--------------------------
    //OpenGL初期化
    glutInit(&argc, argv);
    
    //OpenGL初期設定処理
    initGL();
    
    //イベント待ち無限ループ
    glutMainLoop();
    
    return 0;
}

//OpenGL初期設定処理
void initGL()
{
    //初期設定
    glutInitWindowSize(imageSize.width, imageSize.height);  //ウィンドウサイズ指定
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
    eDist = 1200; eDegX = 0.0; eDegY = 0.0;
}

//ディスプレイコールバック関数
void display()
{
    //-------------------------------
    //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
    capture >> originalImage;
    //ビデオが終了したら無限ループから脱出
    if (originalImage.data==NULL) {
        exit(0);
    }
    //"originalImage"をリサイズして"frameImage"生成
    cv::resize(originalImage, frameImage, imageSize);
    
    //(b)"frameImage"の表示
    cv::imshow("Frame", frameImage);

    
    //------------------
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
    gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);  //変換行列に視野変換行列を乗算
    
    //光源0の位置指定
    GLfloat pos0[] = {200.0, 700.0, 200.0, 1.0};  //(x, y, z, 0(平行光源)/1(点光源))
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    // 四角形面の角度をビデオ画像の画素値に基づいて更新
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 24; j++) {
            // 画素値を取得
            int x = j * (imageSize.width / 24);
            int y = i * (imageSize.height / 32);
            cv::Vec3b pixel = frameImage.at<cv::Vec3b>(y, x);
            // 画素値から角度を計算 (例: 赤色成分を x軸回転、緑色成分を y軸回転に利用)
            quadAngles[i][j][0] = pixel[2] / 255.0 * 180.0; // 赤色成分
            quadAngles[i][j][1] = pixel[1] / 255.0 * 180.0; // 緑色成分
        }
    }

    // 四角形面を配置
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 24; j++) {
            // 色設定
            col[0] = 0.5; col[1] = 1.0; col[2] = 0.5;
            glMaterialfv(GL_FRONT, GL_DIFFUSE, col);  //拡散反射係数
            glMaterialfv(GL_FRONT, GL_AMBIENT, col);  //環境光反射係数
            col[0] = 0.5; col[1] = 0.5; col[2] = 0.5; col[3] = 1.0;
            glMaterialfv(GL_FRONT, GL_SPECULAR, col);
            glMaterialf(GL_FRONT, GL_SHININESS, 64);  //ハイライト係数

            glPushMatrix();  //行列一時保存
            // 四角形面の中心座標を計算 (frameImage のサイズに合わせて調整)
            glTranslated((j - 12) * (imageSize.width / 24.0), (i - 16) * (imageSize.height / 32.0), 0.0);
            // 角度を適用
            glRotated(quadAngles[i][j][0], 1.0, 0.0, 0.0);
            glRotated(quadAngles[i][j][1], 0.0, 1.0, 0.0);
            // 四角形面を描画
            glBegin(GL_QUADS);
            glVertex3f(-(imageSize.width / 48.0), -(imageSize.height / 64.0), 0.0); // 左下
            glVertex3f((imageSize.width / 48.0), -(imageSize.height / 64.0), 0.0); // 右下
            glVertex3f((imageSize.width / 48.0), (imageSize.height / 64.0), 0.0); // 右上
            glVertex3f(-(imageSize.width / 48.0), (imageSize.height / 64.0), 0.0); // 左上
            glEnd();
            glPopMatrix();  //行列復帰
        }
    }

    // 描画実行
    glutSwapBuffers();
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