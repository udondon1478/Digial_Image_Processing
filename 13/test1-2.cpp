// g++ main.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[])
{
    // ビデオキャプチャの初期化
    cv::VideoCapture capture("senro.mov");  // ビデオファイルをオープン
    if (!capture.isOpened()) {  // オープンに失敗した場合
        std::cerr << "Capture not found" << std::endl;
        return -1;
    }

    // 画像格納用インスタンス準備
    cv::Mat frameImage, background;
    int width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    cv::Size imageSize(width, height);
    std::cout << "imageSize = (" << width << ", " << height << ")" << std::endl;

    // 画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::namedWindow("Background");

    // 初期背景画像の設定
    capture >> background;
    if (background.empty()) {
        std::cerr << "Failed to read the first frame" << std::endl;
        return -1;
    }

    cv::Mat avgBackground;
    background.convertTo(avgBackground, CV_32FC3);

    // 動画処理用ループ
    while (true) {
        // ビデオキャプチャから1フレーム"frameImage"に取り込み
        capture >> frameImage;
        if (frameImage.empty()) {  // ビデオが終了したら無限ループから脱出
            break;
        }

        // フレームのグレースケール変換
        cv::Mat grayFrame, grayBackground, diffFrame;
        cv::cvtColor(frameImage, grayFrame, cv::COLOR_BGR2GRAY);
        cv::cvtColor(background, grayBackground, cv::COLOR_BGR2GRAY);

        // 現フレームと背景画像の差分を計算
        cv::absdiff(grayFrame, grayBackground, diffFrame);

        // 差分画像の二値化（動きがある部分を検出）
        cv::threshold(diffFrame, diffFrame, 30, 255, cv::THRESH_BINARY);

        // 動きがない領域（背景）を更新
        cv::Mat mask;
        cv::bitwise_not(diffFrame, mask);
        frameImage.copyTo(background, mask);

        // 過去のフレームを加算平均して背景を更新
        cv::accumulateWeighted(frameImage, avgBackground, 0.01, mask);
        avgBackground.convertTo(background, CV_8UC3);

        // 画像表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Background", background);

        // キー入力待ち
        int key = cv::waitKey(20);
        if (key == 'q')  // [Q]が押されたら無限ループ脱出
            break;
    }

    // 終了処理
    capture.release();
    cv::destroyAllWindows();

    // 背景画像を保存
    cv::imwrite("background.jpg", background);

    std::cout << "Finished" << std::endl;
    return 0;
}
