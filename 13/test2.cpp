// g++ ai2.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
    // ビデオキャプチャの初期化
    VideoCapture capture("pantora.mp4"); // ビデオファイルをオープン
    if (!capture.isOpened()) { // オープンに失敗した場合
        printf("Capture not found\n");
        return -1;
    }
    
    // パンの種類と対応する色
    enum BreadType { GORO_CHOCOLATE_SCONE, CHEESE_BUSSE, THICK_CHOCOLATE_CAKE, BANANA_BREAD, LEMON_BAUM };
    Scalar breadColors[] = { Scalar(255, 255, 0), Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(255, 0, 255), Scalar(0, 0, 255) };
    string breadNames[] = { "Goro Choco Scone", "Cheese Busse", "Thick Choco Cake", "Banana Bread", "Lemon Baum" };
    
    // ネットワーク読み込み（事前にトレーニングされたパンの種類識別モデルを利用）
    dnn::Net net = dnn::readNetFromONNX("bread_recognition_model.onnx");

    // 画像格納用インスタンス準備
    Mat frameImage;
    int width = capture.get(CAP_PROP_FRAME_WIDTH);
    int height = capture.get(CAP_PROP_FRAME_HEIGHT);
    Size imageSize(width, height);
    printf("imageSize = (%d, %d)\n", width, height);

    // 画像表示用ウィンドウの生成
    namedWindow("Frame");

    // インジケータの初期設定
    Mat indicator(50, imageSize.width, CV_8UC3, Scalar(0, 0, 0));
    for (int i = 0; i < 5; i++) {
        rectangle(indicator, Point(10 + i * 60, 10), Point(50 + i * 60, 40), breadColors[i], -1);
        putText(indicator, breadNames[i], Point(10 + i * 60, 45), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
    }

    // 動画処理用無限ループ
    while (true) {
        // ビデオキャプチャから1フレームを取り込み
        capture >> frameImage;
        if (frameImage.empty()) break; // ビデオが終了したら無限ループから脱出

        // パンの領域を検出し、種類を識別
        Mat blob = dnn::blobFromImage(frameImage, 1.0, Size(224, 224), Scalar(104, 117, 123), false, false);
        net.setInput(blob);
        Mat detections = net.forward();

        // 各検出領域に対して種類を特定し、輪郭を描画
        for (int i = 0; i < detections.size[2]; i++) {
            float confidence = detections.at
