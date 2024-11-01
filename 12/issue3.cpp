/*
 g++ -std=c++11 issue3.cpp `pkg-config --cflags --libs opencv4`
 */
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

// main関数
int main(int argc, char *argv[])
{
    // OpenCV初期設定処理
    // カメラキャプチャの初期化
    cv::VideoCapture capture = cv::VideoCapture(0);
    if (capture.isOpened() == 0)
    {
        // カメラが見つからないときはメッセージを表示して終了
        printf("Camera not found\n");
        exit(1);
    }

    cv::Mat originalImage, frameImage, grayImage;
    cv::Size imageSize(720, 405);         // 画像サイズ
    cv::CascadeClassifier faceClassifier; // 顔認識用分類器
    cv::CascadeClassifier eyeClassifier;  // 眼認識用分類器

    // OpenCVウィンドウ生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Face");
    cv::moveWindow("Face", imageSize.width, 0);

    // ①正面顔検出器の読み込み
    faceClassifier.load("haarcascades/haarcascade_frontalface_default.xml");
    // 眼検出器の読み込み
    eyeClassifier.load("haarcascades/haarcascade_mcs_eyepair_small.xml");

    while (1)
    {
        // ビデオキャプチャから1フレーム画像取得
        capture >> originalImage;
        cv::resize(originalImage, frameImage, imageSize);

        // グレースケールに変換
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);

        // フレーム画像表示
        cv::imshow("Frame", frameImage);

        // ②検出情報を受け取るための配列を用意する
        std::vector<cv::Rect> faces, eyes;

        // ③画像中から検出対象の情報を取得する
        faceClassifier.detectMultiScale(grayImage, faces, 1.1, 3, 0, cv::Size(20, 0));
        eyeClassifier.detectMultiScale(grayImage, eyes, 1.1, 3, 0, cv::Size(10, 10));

        // ④顔領域の検出
        for (int i = 0; i < faces.size(); i++)
        {
            // 検出情報から顔の位置情報を取得
            cv::Rect face = faces[i];
            // 大きさによるチェック。
            if (face.width * face.height < 100 * 100)
            {
                continue; // 小さい矩形は採用しない
            }

            // 眼領域の検出
            for (int j = 0; j < eyes.size(); j++) {
                cv::Rect eye = eyes[j];

                // 眼領域が顔領域内にあるかチェック
                if (eye.x >= face.x && eye.y >= face.y && 
                    eye.x + eye.width <= face.x + face.width && 
                    eye.y + eye.height <= face.y + face.height) 
                {
                    // 眼領域を黒く塗りつぶす
                    cv::rectangle(frameImage, eye, cv::Scalar(0, 0, 0), -1);
                }
            }
        }

        // 認識結果画像表示
        cv::imshow("Face", frameImage);

        char key = cv::waitKey(10);
        if (key == 'q')
        {
            break;
        }
    }

    return 0;
}