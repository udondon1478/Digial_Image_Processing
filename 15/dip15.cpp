#include <iostream>           //入出力関連ヘッダ
#include <GLUT/glut.h>        //OpenGL関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

// 関数名の宣言
void initCV(void);     // OpenCVの初期化
void initGL(void);     // OpenGLの初期化
void display(void);    // 描画関数
void timer(int value); // タイマー関数
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void drawFace(double scale, double rotateX, double rotateY, double rotateZ, double tiltAngle, double translateX); // 顔描画関数 (translateXを追加)
void drawEye(void);                                                          // 目描画関数
void drawNose(void);                                                         // 鼻描画関数
void drawMouth(double scale);                                                // 口描画関数

// グローバル変数
double eDist, eDegX, eDegY;  // 視点極座標
int mX, mY, mState, mButton; // マウス座標
int winW, winH;              // ウィンドウサイズ
double fr = 30.0;            // フレームレート
cv::VideoCapture capture;    // 映像キャプチャ用変数
cv::Mat originalImage, frameImage, hsvImage, tempImage;
cv::Size imageSize(640, 360);
cv::CascadeClassifier faceClassifier, eyeClassifier, mouthClassifier, noseClassifier; // 分類器

// main関数
int main(int argc, char *argv[])
{
    // OpenGL初期化
    glutInit(&argc, argv);

    // OpenCV初期設定処理
    initCV();

    // OpenGL初期設定処理
    initGL();

    // イベント待ち無限ループ
    glutMainLoop();

    return 0;
}

// OpenCV初期設定処理
void initCV()
{
    // カメラキャプチャの初期化
    capture = cv::VideoCapture(0);
    if (capture.isOpened() == 0)
    {
        // カメラが見つからないときはメッセージを表示して終了
        printf("Camera not found\n");
        exit(1);
    }
    capture >> originalImage;
    cv::resize(originalImage, frameImage, imageSize);

    // OpenCVウィンドウ生成
    cv::namedWindow("Frame");

    // 顔検出器の読み込み
    faceClassifier.load("haarcascade_frontalface_default.xml");
    eyeClassifier.load("haarcascade_lefteye_2splits.xml");
    mouthClassifier.load("haarcascade_mcs_mouth.xml");
    noseClassifier.load("haarcascade_mcs_nose.xml");
}

// OpenGL初期設定処理
void initGL(void)
{
    // 初期設定
    glutInitWindowSize(imageSize.width, imageSize.height);     // ウィンドウサイズ指定
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE); // ディスプレイモード設定

    // OpenGLウィンドウ生成
    glutCreateWindow("CG");

    // ウィンドウ消去色設定
    glClearColor(0.0, 0.0, 0.2, 1.0);

    // 機能有効化
    glEnable(GL_DEPTH_TEST); // デプスバッファ
    glEnable(GL_NORMALIZE);  // 法線ベクトル正規化
    glEnable(GL_LIGHTING);   // 陰影付け

    // 光原設定
    GLfloat col[4];      // パラメータ(RGBA)
    glEnable(GL_LIGHT0); // 光源0
    col[0] = 0.9;
    col[1] = 0.9;
    col[2] = 0.9;
    col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  // 光源0の拡散反射の強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col); // 光源0の鏡面反射の強度
    col[0] = 0.05;
    col[1] = 0.05;
    col[2] = 0.05;
    col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col); // 光源0の環境光の強度

    // コールバック関数
    glutDisplayFunc(display);           // ディスプレイコールバック関数の指定
    glutReshapeFunc(reshape);           // リシェイプコールバック関数の指定
    glutMouseFunc(mouse);               // マウスクリックコールバック関数の指定
    glutMotionFunc(motion);             // マウスドラッグコールバック関数の指定
    glutKeyboardFunc(keyboard);         // キーボードコールバック関数の指定
    glutTimerFunc(1000 / fr, timer, 0); // タイマーコールバック関数の指定

    // 視点極座標初期値
    eDist = 600;
    eDegX = 0.0;
    eDegY = 0.0;
}

// ディスプレイコールバック関数
void display(void)
{
    //------------------------------CV------------------------------
    // ビデオキャプチャから1フレーム画像取得
    capture >> originalImage;
    cv::resize(originalImage, frameImage, imageSize);

    // 検出情報を受け取るための配列を用意する
    std::vector<cv::Rect> faces, eyes, mouths, noses;

    // 画像中から検出対象の情報を取得する
    faceClassifier.detectMultiScale(frameImage, faces, 1.1, 3, 0, cv::Size(20, 20));   // 顔
    eyeClassifier.detectMultiScale(frameImage, eyes, 1.1, 3, 0, cv::Size(20, 20));     // 目
    mouthClassifier.detectMultiScale(frameImage, mouths, 1.1, 3, 0, cv::Size(20, 20)); // 口
    noseClassifier.detectMultiScale(frameImage, noses, 1.1, 3, 0, cv::Size(20, 20));   // 鼻

    // 顔
    double faceScale = 1.0, faceRotateX = 0.0, faceRotateY = 0.0, faceRotateZ = 0.0;
    double mouthScale = 1.0;
    double tiltAngle = 0.0; // 顔の傾き角度を格納する変数を追加
    double translateX = 0.0; // 顔の横方向への移動量を格納する変数を追加

    for (int i = 0; i < faces.size(); i++)
    {
        // 検出情報から位置情報を取得
        cv::Rect face = faces[i];
        // 大きさによるチェック。
        if (face.width * face.height < 100 * 100)
        {
            continue; // 小さい矩形は採用しない
        }
        // 取得した位置情報に基づき矩形描画
        cv::rectangle(frameImage, cv::Point(face.x, face.y), cv::Point(face.x + face.width, face.y + face.height), CV_RGB(255, 0, 0), 2, 8);

        // 顔のスケールと回転角度を計算
        faceScale = face.width / (double)imageSize.width;
        faceRotateX = (face.y + face.height / 2.0 - imageSize.height / 2.0) / (double)imageSize.height * 180.0;
        faceRotateY = -(face.x + face.width / 2.0 - imageSize.width / 2.0) / (double)imageSize.width * 180.0;

        // 顔の横方向への移動量を計算
        translateX = (face.x + face.width / 2.0 - imageSize.width / 2.0) / (double)imageSize.width; // -1.0 ~ 1.0

        // 顔の矩形内に口があるかチェック
        for (int j = 0; j < mouths.size(); j++)
        {
            cv::Rect mouth = mouths[j];
            if (mouth.x > face.x && mouth.x + mouth.width < face.x + face.width &&
                mouth.y > face.y && mouth.y + mouth.height < face.y + face.height)
            {
                // 口のスケールを計算
                mouthScale = mouth.height / (double)face.height * 4.0;
                cv::rectangle(frameImage, cv::Point(mouth.x, mouth.y), cv::Point(mouth.x + mouth.width, mouth.y + mouth.height), CV_RGB(0, 255, 0), 2, 8);
            }
        }

        // 顔の傾き角度の計算 (両目が検出できた場合のみ)
        if (eyes.size() == 2) {
            cv::Rect eyeLeft = eyes[0].x < eyes[1].x ? eyes[0] : eyes[1];  // 左目の矩形
            cv::Rect eyeRight = eyes[0].x < eyes[1].x ? eyes[1] : eyes[0]; // 右目の矩形
            tiltAngle = (eyeRight.y + eyeRight.height / 2.0) - (eyeLeft.y + eyeLeft.height / 2.0);
            tiltAngle = atan(tiltAngle / (double)imageSize.width) * 180.0 / M_PI; // ラジアンから度に変換
        }
    }

    // フレーム画像表示
    cv::imshow("Frame", frameImage);

    //------------------------------CG------------------------------
    GLfloat col[4]; // 色設定用

    // ウィンドウ内消去
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 行列初期化
    glLoadIdentity();

    // 視点座標の計算
    double ex = eDist * cos(eDegX * M_PI / 180.0) * sin(eDegY * M_PI / 180.0);
    double ey = eDist * sin(eDegX * M_PI / 180.0);
    double ez = eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0);

    // 視点視線の設定
    gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // 変換行列に視野変換行列を乗算

    // 光源0の位置指定
    GLfloat pos0[] = {200.0, 700.0, 200.0, 1.0}; //(x, y, z, 0(平行光源)/1(点光源))
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    //--------------------顔--------------------
    //顔の描画 (tiltAngleとtranslateXを渡す)
    drawFace(faceScale, faceRotateX, faceRotateY, faceRotateZ, tiltAngle, translateX); 

    // 描画実行
    glutSwapBuffers();
}

void drawFace(double scale, double rotateX, double rotateY, double rotateZ, double tiltAngle, double translateX) // translateXを追加
{
    GLfloat col[4]; // 色設定用
    // 色設定
    col[0] = 1.0;
    col[1] = 0.8;
    col[2] = 0.5;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col); // 拡散反射係数
    glMaterialfv(GL_FRONT, GL_AMBIENT, col); // 環境光反射係数
    col[0] = 0.5;
    col[1] = 0.5;
    col[2] = 0.5;
    col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64); // ハイライト係数

    glPushMatrix();                               // 行列一時保存

    // 顔の移動と回転
    glTranslated(translateX * 200, 0.0, 0.0);    // 顔の横方向への移動
    glRotated(tiltAngle, 0.0, 0.0, 1.0);           // 顔の傾き
    glRotated(rotateX, 1.0, 0.0, 0.0);            // X軸周りの回転
    glRotated(rotateY, 0.0, 1.0, 0.0);            // Y軸周りの回転
    glRotated(rotateZ, 0.0, 0.0, 1.0);            // Z軸周りの回転
    glScaled(250.0 * scale, 250.0 * scale, 60.0); // 拡大縮小

    glutSolidCube(1.0);                           // 立方体の配置

    // 目
    glPushMatrix();
    glTranslated(0.3, 0.2, 0.51);
    glScaled(0.2, 0.2, 0.1);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glTranslated(-0.3, 0.2, 0.51);
    glScaled(0.2, 0.2, 0.1);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();

    // 鼻
    glPushMatrix();
    glTranslated(0.0, 0.0, 0.51);
    glScaled(0.15, 0.15, 0.1);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();

    // 口
    glPushMatrix();
    glTranslated(0.0, -0.2, 0.51);
    glScaled(0.3, 0.1, 0.1);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();

    glPopMatrix(); // 行列復帰
}

// タイマーコールバック関数
void timer(int value)
{
    glutPostRedisplay();                // ディスプレイイベント強制発生
    glutTimerFunc(1000 / fr, timer, 0); // タイマー再設定
}

// リサイズコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);                                    // ウィンドウ全体が描画対象
    glMatrixMode(GL_PROJECTION);                               // 投影変換行列を計算対象に設定
    glLoadIdentity();                                          // 行列初期化
    gluPerspective(30.0, (double)w / (double)h, 1.0, 10000.0); // 変換行列に透視投影を乗算
    glMatrixMode(GL_MODELVIEW);                                // モデルビュー変換行列を計算対象に設定

    winW = w;
    winH = h;
}

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        // マウス情報格納
        mX = x;
        mY = y;
        mState = state;
        mButton = button;
    }
}

// マウスドラッグコールバック関数
void motion(int x, int y)
{
    if (mButton == GLUT_RIGHT_BUTTON)
    {
        // マウスの移動量を角度変化量に変換
        eDegY = eDegY + (mX - x) * 0.5; // マウス横方向→水平角
        eDegX = eDegX + (y - mY) * 0.5; // マウス縦方向→垂直角
    }

    // マウス座標格納
    mX = x;
    mY = y;
}

// キーボードコールバック関数(key:キーの種類，x,y:座標)
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'q':
    case 'Q':
    case 27:
        exit(0);
    }
}