// g++ -std=c++11 dip12.cpp `pkg-config --cflags --libs opencv4`

#include <iostream>
#include <opencv2/opencv.hpp>

//パンの種類を識別し、輪郭を表示するとともに、画面左上に検出したパンに対応する色を表示するインジケーターを実装するプログラム
//パンの種類は5種類で、それぞれのパンに対応する色は以下の通り
//・ごろごろチョコチップスコーン：シアン
//・チーズブッセ：青
//・厚切りチョコケーキ：緑
//・バナナブレッド：マゼンタ
//・レモンバウム：赤

// パンの種類
enum BreadType
{
    CHOCOCHIPSCONE, // ごろごろチョコチップスコーン
    CHEESEBOUSSA,   // チーズブッセ
    CHOCOCAKE,      // 厚切りチョコケーキ
    BANANABREAD,    // バナナブレッド
    LEMONBAUM,      // レモンバウム
    UNKNOWN         // 不明
};

// パンの種類に対応する色
const cv::Scalar breadColors[] = {
    cv::Scalar(255, 255, 0),   // シアン
    cv::Scalar(255, 0, 0),     // 青
    cv::Scalar(0, 255, 0),     // 緑
    cv::Scalar(255, 0, 255),   // マゼンタ
    cv::Scalar(0, 0, 255),     // 赤
    cv::Scalar(255, 255, 255)  // 白
};

// パンの種類を判定する関数
BreadType detectBreadType(const cv::Mat &image)
{
    // 画像をグレースケールに変換
    cv::Mat grayImage;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);

    // 2値化
    cv::Mat binaryImage;
    cv::threshold(grayImage, binaryImage, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 輪郭抽出
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binaryImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 輪郭の数が0の場合はUNKNOWNを返す
    if (contours.size() == 0)
    {
        return BreadType::UNKNOWN;
    }

    // 最大の輪郭を取得
    double maxArea = 0;
    int maxAreaIdx = 0;
    for (int i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if (area > maxArea)
        {
            maxArea = area;
            maxAreaIdx = i;
        }
    }

    // 最大の輪郭を描画
    cv::Mat contourImage = cv::Mat::zeros(image.size(), CV_8UC3);
    cv::drawContours(contourImage, contours, maxAreaIdx, cv::Scalar(255, 255, 255), 2);

    // パンの種類を判定
    if (maxArea > 10000)
    {
        return BreadType::CHOCOCHIPSCONE;
    }
    else if (maxArea > 5000)
    {
        return BreadType::CHEESEBOUSSA;
    }
    else if (maxArea > 2000)
    {
        return BreadType::CHOCOCAKE;
    }
    else if (maxArea > 1000)
    {
        return BreadType::BANANABREAD;
    }
    else if (maxArea > 500)
    {
        return BreadType::LEMONBAUM;
    }
    else
    {
        return BreadType::UNKNOWN;
    }

    return BreadType::UNKNOWN;
}

int main(int argc, char *argv[])
{
    // カメラのキャプチャを初期化
    cv::VideoCapture capture(2);
    if (!capture.isOpened())
    {
        std::cerr << "Failed to open camera." << std::endl;
        return -1;
    }

    // ウィンドウを生成
    cv::namedWindow("Camera Feed");
    cv::moveWindow("Camera Feed", 0, 0);

    cv::namedWindow("Contour");
    cv::moveWindow("Contour", 640, 0);

    cv::namedWindow("Indicator");
    cv::moveWindow("Indicator", 0, 480);

    while (1)
    {
        // カメラから1フレーム取得
        cv::Mat frame;
        capture >> frame;
        if (frame.empty())
        {
            break;
        }

        // パンの種類を判定
        BreadType breadType = detectBreadType(frame);

        // パンの種類に応じて色を表示
        cv::Mat indicator = cv::Mat::zeros(100, 100, CV_8UC3);
        indicator.setTo(breadColors[breadType]);
        cv::putText(indicator, "Bread", cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);

        // ウィンドウに表示
        cv::imshow("Camera Feed", frame);
        cv::imshow("Contour", frame + contourImage);
        cv::imshow("Indicator", indicator);

        // キー入力を取得
        int key = cv::waitKey(1);
        if (key == 'q')
        {
            break;
        }
    }

    return 0;
}

//g++ -std=c++11 dip12.cpp `pkg-config --cflags --libs opencv4`
//./a.out