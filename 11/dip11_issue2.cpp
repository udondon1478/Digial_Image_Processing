// g++ dip11_issue2.cpp -std=c++11 `pkg-config --cflags --libs opencv4` -framework OpenGL -framework GLUT -Wno-deprecated
#include <iostream>
#include <GLUT/glut.h>
#include <opencv2/opencv.hpp>
#include <random>

// ウィンドウサイズ
const int windowWidth = 800;
const int windowHeight = 600;

// 立方体の配置領域を制限するための変数
const float cubeRange = 2.0f;

// カメラの設定
cv::VideoCapture capture(0z);
cv::Mat frame;

// 光源の位置 (初期状態では無効)
GLfloat lightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f }; 

// 立方体の位置と色を格納する配列
GLfloat cubePositions[20][3];
GLfloat cubeColors[20][3];

// 輝度閾値 (0-255)
int brightnessThreshold = 100;

// 面積の閾値
int areaThreshold = 100; // 適切な値に調整

// マウス操作の状態
bool isDragging = false;
int previousMouseX, previousMouseY;

// 視点の回転角度
float cameraTheta = 0.0f;
float cameraPhi = 0.0f;

// ランダムな位置と色を生成する関数
void generateRandomCube(GLfloat* position, GLfloat* color) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> pos_dist(-cubeRange, cubeRange);
    std::uniform_real_distribution<> color_dist(0.0f, 1.0f);

    position[0] = pos_dist(gen);
    position[1] = pos_dist(gen);
    position[2] = pos_dist(gen);

    color[0] = color_dist(gen);
    color[1] = color_dist(gen);
    color[2] = color_dist(gen);
}

// 立方体を描画する関数
void drawCubes() {
    for (int i = 0; i < 20; ++i) {
        glPushMatrix();
        glTranslatef(cubePositions[i][0], cubePositions[i][1], cubePositions[i][2]);
        glColor3f(cubeColors[i][0], cubeColors[i][1], cubeColors[i][2]);
        glutSolidCube(0.5f);
        glPopMatrix();
    }
}

// ディスプレイコールバック関数
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 視点と視線方向を設定
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // 光源の設定
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // マテリアルの設定
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // 立方体を描画
    drawCubes();

    // 光源の位置に球体を描画
    glPushMatrix();
    glTranslatef(lightPosition[0], lightPosition[1], lightPosition[2]);
    glColor3f(1.0f, 1.0f, 1.0f); // 白色
    glutSolidSphere(0.1, 16, 16);
    glPopMatrix();

    glutSwapBuffers();
}

// リサイズコールバック関数
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
}

// カメラの画像から光源の位置を更新する関数
void updateLightPosition() {
    capture >> frame;
    if (frame.empty()) return;

    cv::Mat grayFrame;
    cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(grayFrame, grayFrame, cv::Size(5, 5), 0);

    // 閾値処理で明るい領域を抽出
    cv::Mat thresholdFrame;
    cv::threshold(grayFrame, thresholdFrame, brightnessThreshold, 255, cv::THRESH_BINARY);

    // 輪郭検出
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresholdFrame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 面積が閾値以上の輪郭を探す
    cv::Point2f lightCenter(0, 0);
    float maxArea = 0;
    for (const auto &contour : contours) {
        float area = cv::contourArea(contour);
        if (area > maxArea && area > areaThreshold) {
            maxArea = area;
            cv::Moments moments = cv::moments(contour);
            lightCenter = cv::Point2f(moments.m10 / moments.m00, moments.m01 / moments.m00);
        }
    }

    // 光源の位置を更新 (カメラ画像の座標系をOpenGLの座標系に変換)
    if (maxArea > 0) {
        lightPosition[0] = (lightCenter.x / (float)frame.cols) * 2.0f - 1.0f;
        lightPosition[1] = -((lightCenter.y / (float)frame.rows) * 2.0f - 1.0f);
        lightPosition[2] = 5.0f; // z位置は固定
        lightPosition[3] = 1.0f; // 点光源を有効化
    } else {
        lightPosition[3] = 0.0f; // 光源を無効化
    }

    // 光源の位置に円を描画
    cv::circle(frame, lightCenter, 10, cv::Scalar(0, 0, 255), 2);

    // カメラ映像を表示
    cv::imshow("Camera Feed", frame);
}


// アイドル時に呼ばれる関数
void idle() {
    updateLightPosition();
    glutPostRedisplay();
}

// マウスボタンコールバック関数
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = true;
            previousMouseX = x;
            previousMouseY = y;
        } else if (state == GLUT_UP) {
            isDragging = false;
        }
    }
}

// マウスモーションコールバック関数
void motion(int x, int y) {
    if (isDragging) {
        // マウスの移動量に応じて視点の回転角度を更新
        cameraTheta += (x - previousMouseX) * 0.01f;
        cameraPhi += (y - previousMouseY) * 0.01f;

        // カメラの角度を制限
        cameraPhi = std::max(std::min(cameraPhi, 1.57f), -1.57f); 

        previousMouseX = x;
        previousMouseY = y;

        glutPostRedisplay(); 
    }
}

// トラックバーのコールバック関数
void onTrackbar(int value, void* userdata) {
    brightnessThreshold = value;
}


int main(int argc, char** argv) {
    if (!capture.isOpened()) {
        std::cerr << "カメラを開けません" << std::endl;
        return -1;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Random Cubes with Light Tracking");
    glEnable(GL_DEPTH_TEST);

        // 光源の初期状態をオフにする
    glDisable(GL_LIGHTING);

    // 光源の有効化
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // 滑らかなシェーディングを有効化
    glShadeModel(GL_SMOOTH);

    // Cubeの位置と色を一度だけ生成
    for (int i = 0; i < 20; ++i) {
        generateRandomCube(cubePositions[i], cubeColors[i]);
    }

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    // マウスコールバック関数の登録 (glutMainLoop() より前に移動)
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    // トラックバーの作成
    cv::namedWindow("Camera Feed");
    cv::createTrackbar("Threshold", "Camera Feed", &brightnessThreshold, 255, onTrackbar);
    cv::createTrackbar("Area", "Camera Feed", &areaThreshold, 1000, onTrackbar);

    // カメラ映像表示用のウィンドウを作成
    cv::namedWindow("Camera Feed"); 

    glutMainLoop(); // これ以降の処理は実行されない
    return 0;
}