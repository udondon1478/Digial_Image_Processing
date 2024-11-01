// g++ dip14.cpp -std=c++11 -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv4` -Wno-deprecated
#include <iostream>           //入出力関連ヘッダ
#include <GLUT/glut.h>        //OpenGL
#include <math.h>             //数学関数
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ
#include <vector>

// 関数名の宣言
void initGL(void);
void display(void);
void reshape(int w, int h);
void timer(int value);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void initCV(void);
void mouseCallback(int event, int x, int y, int flags, void *userdata);
// drawlines関数の修正
void drawlines(cv::Mat &frameImage, const std::vector<cv::Point> &points); // 引数の型をconstに変更
void createColorPickers();
void updateColorPickers();
void hueTrackbarCallback(int, void *);
void saturationTrackbarCallback(int, void *);
void valueTrackbarCallback(int, void *);
cv::Scalar getSelectedColor(); // この行を追加

// グローバル変数
double eDist, eDegX, eDegY;        // 視点極座標
int mX, mY, mState, mButton;       // マウス座標
int winW, winH;                    // ウィンドウサイズ
double fr = 30.0;                  // フレームレート
cv::VideoCapture capture;          // ビデオキャプチャ
cv::Size imageSize;                // 画像サイズ
cv::Mat originalImage, frameImage; // 画像格納用
double theta = 0.0;
double delta = 1.0;                // 回転の速度
int rotFlag = 1;                   // 回転フラグ
std::vector<cv::Point> linePoints; // 線の点のリスト
bool drawing = false;              // 線を描画中かどうか

cv::Mat prevFrame;                // 前フレーム
cv::Mat grayFrame, prevGrayFrame; // グレースケールの現在フレームと前フレーム
bool firstFrame = true;           // 最初のフレームフラグ

// オプティカルフローで検出する速度の閾値
const double SPEED_THRESHOLD = 5.0; // ピクセル/フレーム
bool readyToTransfer = false;     // 転送準備完了フラグ

cv::Mat hueImage, saturationImage, valueImage;
int selectedHue = 0, selectedSaturation = 0, selectedValue = 0;

// 新しいグローバル変数
struct ShapeData {  // 形状データと色を格納する構造体
    std::vector<cv::Point3f> points;
    cv::Vec3f color;
    cv::Vec3f speed;
};
std::vector<ShapeData> transferredShapes;

void detectSwipe(cv::Mat &prevGray, cv::Mat &gray)
{
    std::vector<cv::Point2f> prevPoints, points;
    cv::goodFeaturesToTrack(prevGray, prevPoints, 100, 0.3, 7); 
    if (prevPoints.empty())
        return;

    std::vector<uchar> status;
    std::vector<float> err;
    cv::calcOpticalFlowPyrLK(prevGray, gray, prevPoints, points, status, err);

    double sumX = 0;
    int count = 0;
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i])
        {
            double dx = points[i].x - prevPoints[i].x;
            double dy = points[i].y - prevPoints[i].y;
            double speed = sqrt(dx * dx + dy * dy);

            if (speed > SPEED_THRESHOLD)
            {
                sumX += dx;
                count++;
            }
        }
    }

    if (count > 0)
    {
        double avgX = sumX / count;
        if (avgX > 2.0)
        {
            std::cout << "右にスワイプしました" << std::endl;
            readyToTransfer = true; // 右スワイプで転送準備完了
        }
        else if (avgX < -2.0)
        {
            std::cout << "左にスワイプしました" << std::endl;
        }
    }
}

void drawlines(cv::Mat &frameImage, const std::vector<cv::Point> &points) 
{
    cv::Scalar selectedColor = getSelectedColor();
    if (points.size() >= 2)
    {
        for (size_t i = 0; i < points.size() - 1; ++i)
        {
            cv::line(frameImage, points[i], points[i + 1], selectedColor, 2);
        }
    }
    cv::imshow("Frame", frameImage);
}

void transferShape()
{
    if (linePoints.size() >= 2)
    {
        ShapeData shapeData;  // 形状データと色を格納する構造体
        shapeData.color = cv::Vec3f(getSelectedColor()[2] / 255.0f, getSelectedColor()[1] / 255.0f, getSelectedColor()[0] / 255.0f); // 色情報を保存

        for (const auto &point : linePoints)
        {
            shapeData.points.push_back(cv::Point3f(point.x, point.y, 0));
        }

        // ランダムな速度を割り当て
        shapeData.speed = cv::Vec3f((rand() % 100 - 50) / 10.0f, (rand() % 100 - 50) / 10.0f, (rand() % 100 - 50) / 10.0f);

        transferredShapes.push_back(shapeData);
        linePoints.clear(); 
    }
}

cv::Scalar getSelectedColor() {
    cv::Mat hsv(1, 1, CV_8UC3, cv::Scalar(selectedHue, selectedSaturation, selectedValue));
    cv::Mat rgb;
    cv::cvtColor(hsv, rgb, cv::COLOR_HSV2BGR);
    return cv::Scalar(rgb.at<cv::Vec3b>(0)[0], rgb.at<cv::Vec3b>(0)[1], rgb.at<cv::Vec3b>(0)[2]);
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    initCV();
    initGL();
    glutMainLoop();
    return 0;
}

void initCV(void)
{
    capture = cv::VideoCapture(0);
    if (capture.isOpened() == 0)
    { 
        printf("Capture not found\n");
        return;
    }

    int imageWidth = 720, imageHeight = 405;
    imageSize = cv::Size(imageWidth, imageHeight); 
    frameImage = cv::Mat(imageSize, CV_8UC3);     

    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    createColorPickers();
    cv::setMouseCallback("Frame", mouseCallback);
    createColorPickers(); 
}

void createColorPickers()
{
    hueImage = cv::Mat(200, 360, CV_8UC3);
    cv::namedWindow("Hue Picker");
    cv::createTrackbar("Hue", "Hue Picker", &selectedHue, 179, hueTrackbarCallback);

    saturationImage = cv::Mat(200, 256, CV_8UC3);
    cv::namedWindow("Saturation Picker");
    cv::createTrackbar("Saturation", "Saturation Picker", &selectedSaturation, 255, saturationTrackbarCallback);

    valueImage = cv::Mat(200, 256, CV_8UC3);
    cv::namedWindow("Value Picker");
    cv::createTrackbar("Value", "Value Picker", &selectedValue, 255, valueTrackbarCallback);

    updateColorPickers();
}

void updateColorPickers()
{
    for (int i = 0; i < hueImage.cols; i++)
    {
        cv::Vec3b color = cv::Vec3b(i / 2, 255, 255);
        cv::Mat hueCol = hueImage.col(i);
        hueCol = cv::Scalar(color[0], color[1], color[2]);
    }
    cv::cvtColor(hueImage, hueImage, cv::COLOR_HSV2BGR);
    cv::imshow("Hue Picker", hueImage);

    for (int i = 0; i < saturationImage.cols; i++)
    {
        cv::Vec3b color = cv::Vec3b(selectedHue, i, selectedValue);
        cv::Mat satCol = saturationImage.col(i);
        satCol = cv::Scalar(color[0], color[1], color[2]);
    }
    cv::cvtColor(saturationImage, saturationImage, cv::COLOR_HSV2BGR);
    cv::imshow("Saturation Picker", saturationImage);

    for (int i = 0; i < valueImage.cols; i++)
    {
        cv::Vec3b color = cv::Vec3b(selectedHue, selectedSaturation, i);
        cv::Mat valCol = valueImage.col(i);
        valCol = cv::Scalar(color[0], color[1], color[2]);
    }
    cv::cvtColor(valueImage, valueImage, cv::COLOR_HSV2BGR);
    cv::imshow("Value Picker", valueImage);
}

void hueTrackbarCallback(int, void *)
{
    updateColorPickers();
    glutPostRedisplay(); 
}

void saturationTrackbarCallback(int, void *)
{
    updateColorPickers();
    glutPostRedisplay(); 
}

void valueTrackbarCallback(int, void *)
{
    updateColorPickers();
    glutPostRedisplay(); 
}

// OpenGL初期設定処理
void initGL(void)
{
    glutInitWindowSize(600, 400);                              
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE); 

    glutInitWindowPosition(imageSize.width, 0);
    glutCreateWindow("GL");

    glClearColor(0.9, 0.95, 1.0, 1.0);

    glEnable(GL_DEPTH_TEST); 
    glEnable(GL_NORMALIZE); 
    glEnable(GL_LIGHTING);  
    glEnable(GL_LIGHT0);    

    GLfloat col[4];     
    glEnable(GL_LIGHT0); 
    col[0] = 0.9;
    col[1] = 0.9;
    col[2] = 0.9;
    col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col); 
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);
    col[0] = 0.05;
    col[1] = 0.05;
    col[2] = 0.05;
    col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col); 

    glutDisplayFunc(display);          
    glutReshapeFunc(reshape);          
    glutMouseFunc(mouse);              
    glutMotionFunc(motion);            
    glutKeyboardFunc(keyboard);        
    glutTimerFunc(1000 / fr, timer, 0); 

    eDist = 1500;
    eDegX = 10.0;
    eDegY = 0.0;

    glLineWidth(3.0);
}

// ディスプレイコールバック関数
void display()
{
    GLfloat col[4];

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    double ex = eDist * cos(eDegX * M_PI / 180.0) * sin(eDegY * M_PI / 180.0);
    double ey = eDist * sin(eDegX * M_PI / 180.0);
    double ez = eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0);

    gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    GLfloat pos0[] = {200.0, 700.0, 200.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    col[0] = 0.5;
    col[1] = 1.0;
    col[2] = 0.5;                                                 
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); 
    col[0] = 1.0;
    col[1] = 1.0;
    col[2] = 1.0;
    col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64); 
    glPushMatrix();                          
    glScaled(1000, 1, 1000);                
    glutSolidCube(1.0);                      
    glPopMatrix();                          

    // ------------------------------- OpenCV --------------------------------
    capture >> originalImage;
    if (originalImage.data == NULL)
    {
        exit(0);
    }
    cv::resize(originalImage, frameImage, imageSize);
    cv::cvtColor(frameImage, grayFrame, cv::COLOR_BGR2GRAY);

    if (!firstFrame)
    {
        detectSwipe(prevGrayFrame, grayFrame);
    }
    else
    {
        firstFrame = false;
    }

    prevGrayFrame = grayFrame.clone();

    drawlines(frameImage, linePoints);

    cv::imshow("Frame", frameImage);

    updateColorPickers(); 

    // ------------------------------- OpenGL --------------------------------

    // 転送準備が完了したら、図形を転送
    if (readyToTransfer) {
        transferShape();
        readyToTransfer = false; // フラグをリセット
    }

    // 転送された図形を描画
    for (const auto& shapeData : transferredShapes) {
        glBegin(GL_LINE_STRIP);
        glColor3f(shapeData.color[0], shapeData.color[1], shapeData.color[2]); // 色を適用
        for (const auto &point : shapeData.points) {
            glVertex3f(point.x, point.y, point.z);
        }
        glEnd();
    }

    glutSwapBuffers();
}

void moveShapes()
{
    for (auto& shapeData : transferredShapes)  // transferredShapesの要素を直接参照
    {
        for (auto &point : shapeData.points)
        {
            point.x += shapeData.speed[0];
            point.y += shapeData.speed[1];
            point.z += shapeData.speed[2];

            if (point.x < 0 || point.x > imageSize.width) shapeData.speed[0] *= -1;
            if (point.y < 0 || point.y > imageSize.height) shapeData.speed[1] *= -1;
            if (point.z < -500 || point.z > 500) shapeData.speed[2] *= -1;
        }
    }
}

void timer(int value)
{
    if (rotFlag)
    {
        eDegY += delta;
    }

    theta += delta;

    moveShapes();

    glutPostRedisplay();
    glutTimerFunc(1000 / fr, timer, 0);
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);                                   
    glMatrixMode(GL_PROJECTION);                              
    glLoadIdentity();                                         
    gluPerspective(30.0, (double)w / (double)h, 1.0, 10000.0); 
    glMatrixMode(GL_MODELVIEW);                               
}

void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        mX = x;
        mY = y;
        mState = state;
        mButton = button;

        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            drawing = true;
            linePoints.push_back(cv::Point(x, y));
        }
        else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
        {
            drawing = false;
            linePoints.clear();
        }
    }
}

void motion(int x, int y)
{
    if (mButton == GLUT_RIGHT_BUTTON)
    {
        eDegY = eDegY + (mX - x) * 0.5;
        eDegX = eDegX + (y - mY) * 0.5;
    }

    if (drawing)
    {
        linePoints.push_back(cv::Point(x, y));
    }
    mX = x;
    mY = y;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'q':
    case 'Q':
    case 27:
        exit(0);
    case 't':
    case 'T':
        // 't'キーの処理は不要になりました
        break;
    }
}

void mouseCallback(int event, int x, int y, int flags, void *userdata)
{
    std::cout << "x=" << x << ", y=" << y << " ";

    switch (event)
    {
    case cv::EVENT_MOUSEMOVE:
        std::cout << "マウスが動いた";
        break;
    case cv::EVENT_LBUTTONDOWN:
        std::cout << "左ボタンを押した";
        break;
    case cv::EVENT_RBUTTONDOWN:
        std::cout << "右ボタンを押した";
        break;
    case cv::EVENT_LBUTTONUP:
        std::cout << "左ボタンを離した";
        drawing = false; 
        break;
    case cv::EVENT_RBUTTONUP:
        std::cout << "右ボタンを離した";
        break;
    case cv::EVENT_RBUTTONDBLCLK:
        std::cout << "右ボタンをダブルクリック";
        break;
    case cv::EVENT_LBUTTONDBLCLK:
        std::cout << "左ボタンをダブルクリック";
        break;
    }

    std::string str;
    if (flags & cv::EVENT_FLAG_ALTKEY)
    {
        str += "Alt "; 
    }
    if (flags & cv::EVENT_FLAG_CTRLKEY)
    {
        str += "Ctrl "; 
    }
    if (flags & cv::EVENT_FLAG_SHIFTKEY)
    {
        str += "Shift "; 
    }
    if (flags & cv::EVENT_FLAG_LBUTTON)
    {
        str += "左ボタン "; 
    }
    if (flags & cv::EVENT_FLAG_RBUTTON)
    {
        str += "右ボタン"; 
    }
    if (!str.empty())
    {
        std::cout << "  押下: " << str;
    }
    std::cout << std::endl;

    if (event == cv::EVENT_LBUTTONDOWN)
    {
        drawing = true;
        linePoints.clear();                   
        linePoints.push_back(cv::Point(x, y));
    }

    if (event == cv::EVENT_MOUSEMOVE && drawing)
    {
        linePoints.push_back(cv::Point(x, y)); 
    }
}