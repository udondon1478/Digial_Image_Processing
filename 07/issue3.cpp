//(OpenCV4) g++ -std=c++11 dip07-3.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip07-3.cpp `pkg-config --cflags --libs opencv`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ
#include <math.h>

int main(int argc, char *argv[])
{
    // ①ビデオキャプチャの初期化
    cv::VideoCapture capture("obj.mov"); // ビデオファイルをオープン
    if (capture.isOpened() == 0)
    {
        printf("Camera not found\n");
        return -1;
    }
    // ②画像格納用インスタンス準備
    cv::Size imageSize(540, 303);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);
    cv::Mat backImage(imageSize, CV_8UC3);
    cv::Mat subImage(imageSize, CV_8UC3);
    cv::Mat subBinImage(imageSize, CV_8UC1);
    cv::Mat resultImage(imageSize, CV_8UC3);
    cv::Mat contourImage(imageSize, CV_8UC3);
    cv::Mat recImage(imageSize, CV_8UC3);

    // ③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Back");
    cv::moveWindow("Back", 540, 0);
    // cv::namedWindow("Subtraction");
    // cv::moveWindow("Subtraction", 100, 100);
    cv::namedWindow("SubtractionBin");
    cv::moveWindow("SubtractionBin", 0, 303);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 540, 303);

    // videoWritterの生成
    cv::VideoWriter writer("output.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, resultImage.size());

    // 輪郭点格納用配列
    std::vector<std::vector<cv::Point>> contours;

    // ④動画処理用無限ループ
    while (1)
    {
        //(a)ビデオキャプチャから 1 フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        // ビデオが終了したら無限ループから脱出
        if (originalImage.data == NULL)
            break;
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);

        //(b)"frameImage"と"backImage"との差分画像"subImage"の生成
        cv::absdiff(frameImage, backImage, subImage);

        //(b')"subImage"をグレースケール変換→しきい値処理した画像"subBinImage"を生成
        cv::cvtColor(subImage, subBinImage, cv::COLOR_BGR2GRAY);
        cv::threshold(subBinImage, subBinImage, 30, 255, cv::THRESH_BINARY);

        //(b")"frameImage"を"subBinImage"マスク付きで"resultImage"にコピー
        resultImage = cv::Scalar(0);
        // frameImage.copyTo(resultImage, subBinImage);

        // "binImage"からの輪郭抽出処理
        cv::findContours(subBinImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

        // // 白領域の面積を計算
        // int whiteArea = cv::countNonZero(subBinImage);

        // // 面積に応じて円を描画
        // if (whiteArea > 0)
        // {
        //     int radius = static_cast<int>(sqrt(whiteArea / 3.14));             // 面積から適切な半径を計算
        //     cv::Point center(resultImage.cols / 2, resultImage.rows / 2);      // 中心座標（必要に応じて変更）
        //     cv::circle(resultImage, center, radius, cv::Scalar(0, 0, 255), 2); // 赤色の円を描画
        // }

        // 輪郭を描画
        for (size_t i = 0; i < contours.size(); i++)
        {
            // 輪郭線の長さを計算
            double length = cv::arcLength(contours[i], true);
            // 輪郭内部の面積を計算
            double area = cv::contourArea(contours[i]);
            // 円形どを計算
            double circularity = 4 * 3.14 * area / (length * length);
            // 免責に応じて色を変更
            if (circularity > 0.5)
            {
                cv::drawContours(resultImage, contours, (int)i, cv::Scalar(255, 0, 0), 2, 8);
            }
            else
            {
                cv::drawContours(resultImage, contours, (int)i, cv::Scalar(0, 255, 255), 2, 8);
            }
        }

        //(c)"frameImage"，"backImage"，"subImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Back", backImage);
        // cv::imshow("Subtraction", subImage);
        cv::imshow("SubtractionBin", subBinImage);
        cv::imshow("Result", resultImage);

        //(d)"frameImage"で"backImage"を更新
        // frameImage.copyTo(backImage);
        //(e)キー入力待ち
        int key = cv::waitKey(10);
        //[Q]が押されたら無限ループ脱出
        if (key == 'q')
            break;
        //[C]が押されたら"frameImage"で"backImage"を更新
        if (key == 'c')
            frameImage.copyTo(backImage);

        // 動画に書き出し
        cv::resize(resultImage, recImage, recImage.size());
        writer << recImage;
    }

    // ⑤終了処理
    // カメラ終了
    capture.release();
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
