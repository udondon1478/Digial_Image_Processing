// g++ main.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  
#include <opencv2/opencv.hpp>  

int main(int argc, char* argv[])
{
    // ビデオファイルの読み込み
    cv::VideoCapture video("colorful.mp4");
    if (!video.isOpened()) {
        std::cerr << "ビデオファイルが見つかりません" << std::endl;
        return -1;
    }

    // 画像フレームの準備
    cv::Mat frame, result, mask, snow;
    int width = video.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = video.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size frameSize(width, height);
    std::cout << "フレームサイズ: (" << width << ", " << height << ")" << std::endl;

    // 初期状態の画像作成
    result = cv::Mat::zeros(frameSize, CV_8UC3);
    snow = cv::Mat::zeros(frameSize, CV_8UC3);

    bool isFirstFrame = true;

    // 色範囲の設定 (B, G, R)
    cv::Scalar whiteLower(230, 230, 230); 
    cv::Scalar whiteUpper(255, 255, 255); 

    // フレーム処理ループ
    while (true) {
        // フレームを取得
        video >> frame;
        if (frame.empty()) break;

        // 初回フレームでの処理
        if (isFirstFrame) {
            cv::inRange(frame, whiteLower, whiteUpper, mask);
            cv::bitwise_and(frame, frame, snow, mask);

            // 黄色の除去
            cv::Scalar yellowLower(20, 100, 100); 
            cv::Scalar yellowUpper(30, 255, 255); 
            cv::Mat yellowMask;
            cv::inRange(frame, yellowLower, yellowUpper, yellowMask);
            frame.setTo(cv::Scalar(0, 0, 0), yellowMask);

            isFirstFrame = false;
        }

        // 結果画像を更新
        cv::bitwise_or(result, frame, result);

        // 色変換処理
        for (int y = 0; y < frame.rows; y++) {
            for (int x = 0; x < frame.cols; x++) {
                cv::Vec3b& pixel = result.at<cv::Vec3b>(y, x);
                cv::Vec3b snowPixel = snow.at<cv::Vec3b>(y, x);

                // 白色をグレーに変換
                if (pixel[0] > 180 && pixel[1] > 180 && pixel[2] > 180) {
                    pixel[0] = 200;  
                    pixel[1] = 200;  
                    pixel[2] = 200;  
                }
                // 雪の部分を白色に設定
                if (snowPixel[0] > 230) {
                    pixel[0] = 255;
                    pixel[1] = 255;
                    pixel[2] = 255;
                }
            }
        }

        // 画像表示
        cv::imshow("Processed Frame", result);

        // キー入力処理
        int key = cv::waitKey(20);
        if (key == 'q') break;
    }

    //最後のフレームを画像として出力
    cv::imwrite("result.jpg", result);

    // リソース解放
    video.release();
    cv::destroyAllWindows();
    std::cout << "終了" << std::endl;

    return 0;
}
