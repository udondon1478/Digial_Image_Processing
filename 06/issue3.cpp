// g++ -std=c++11 issue3.cpp `pkg-config --cflags --libs opencv4`
#include <iostream>           // 入出力関連ヘッダ
#include <opencv2/opencv.hpp> // OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{
    // ビデオファイル"movie.mov"を取り込み
    cv::VideoCapture capture("movie.mov"); // 指定したビデオファイルをオープン
    // ビデオファイルがオープンできたかどうかをチェック
    if (!capture.isOpened())
    {
        printf("Specified video not found\n");
        return -1;
    }

    // フレームの大きさを取得
    int width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    printf("FRAME SIZE = (%d %d)\n", width, height);

    // 画像格納用インスタンス準備
    cv::Mat frameImage;
    cv::Mat hsvImage;
    cv::Mat resultImage;
    cv::Mat grayImage(cv::Size(width, height), CV_8UC1);
    cv::Mat binImage(cv::Size(width, height), CV_8UC1);

    std::vector<cv::Point> centers;
    std::vector<cv::Scalar> colors;
    int color = 0;

    std::vector<std::vector<cv::Point>> contours;

    cv::VideoWriter rec("rec.mpg", cv::VideoWriter::fourcc('P', 'I', 'M', '1'), 30, cv::Size(width, height));

    // ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Bin");
    cv::moveWindow("Bin", 0, height);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, 0);

    // 動画像処理無限ループ：「ビデオキャプチャから1フレーム取り込み」→「画像処理」→「表示」の繰り返し
    while (true)
    {
        // カメラから1フレーム読み込み（ストリーム入力）
        capture >> frameImage;
        if (frameImage.empty())
            break;

        // 画像処理
        // 色空間変換(BGR -> HSV)
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);

        // 色の抽出
        for (int y = 0; y < frameImage.rows; y++)
        {
            for (int x = 0; x < frameImage.cols; x++)
            {
                cv::Vec3b s = hsvImage.at<cv::Vec3b>(y, x);
                // 色相(H)と彩度(S)の値を用いてボール抽出
                if (s[0] > 60 && s[0] < 80 && s[1] > 100)
                {
                    // 条件に合致する場合はそのままにする
                }
                else
                {
                    s[0] = 0;
                    s[1] = 0;
                    s[2] = 0;
                }
                hsvImage.at<cv::Vec3b>(y, x) = s;
            }
        }

        // 色空間変換(HSV -> BGR)
        cv::cvtColor(hsvImage, resultImage, cv::COLOR_HSV2BGR);

        // 2値化
        cv::cvtColor(resultImage, grayImage, cv::COLOR_BGR2GRAY);
        cv::threshold(grayImage, binImage, 80, 255, cv::THRESH_BINARY);

        //膨張収縮処理
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));
        cv::dilate(binImage, binImage, kernel, cv::Point(-1, -1), 5);
        cv::erode(binImage, binImage, kernel, cv::Point(-1, -1), 5);

        // 領域抽出
        cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
        for (int i = 0; i < contours.size(); i++)
        {
            // 領域の中心を求める 領域がn個の要素で構成される場合、領域の中心は 0 番目の要素と n/2 番目の要素の中点
            cv::Point center = (contours[i][0] + contours[i][contours[i].size() / 2]) / 2;
            centers.push_back(center);

            // 新しい中心に円を描画
            int hue = (color + i * 10) % 180;
            cv::Mat hsv(1, 1, CV_8UC3, cv::Scalar(hue, 255, 255));
            cv::Mat rgb;
            cv::cvtColor(hsv, rgb, cv::COLOR_HSV2BGR);
            cv::Scalar colorScalar = rgb.at<cv::Vec3b>(0, 0);
            colors.push_back(colorScalar);
            cv::circle(resultImage, center, 5, colorScalar, -1);
        }

        // 過去の中心点に円を描画
        for (size_t j = 0; j < centers.size(); j++)
        {
            cv::circle(frameImage, centers[j], 5, colors[j], 2);
        }

        color += 5;

        // ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Bin", binImage);
        cv::imshow("Result", resultImage);

        // 動画に書き出し
        rec << frameImage;

        // キー入力待ち
        char key = cv::waitKey(30); // 30ミリ秒待機
        if (key == 'q')
            break;
    }

    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
