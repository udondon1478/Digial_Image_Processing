// g++ dip02.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{

    // ①画像ファイルの読み込み
    cv::Mat sourceImage = cv::imread("color1.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data == 0)
    { // 画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);

    // ②画像格納用オブジェクト"resultImage"の生成
    cv::Mat resultImage = cv::Mat(sourceImage.size(), CV_8UC3);
    // ③ウィンドウの生成と移動
    cv::namedWindow("Source");
    cv::moveWindow("Source", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 400, 0);

    // ④画像の画素単位の読み込み・処理・書き込み
    cv::Vec3b s; // 画素値を格納する変数

    int mx = sourceImage.cols / 2.0; // 画像の中心座標
    int my = sourceImage.rows / 2.0; // 画像の中心座標
    int r = 0;                       // 中心からの距離

    for (int y = 0; y < sourceImage.rows; y++)
    {
        for (int x = 0; x < sourceImage.cols; x++)
        {
            s = sourceImage.at<cv::Vec3b>(y, x); //"sourceImage"の画素(x,y)の画素値を読み込んで"s"に格納
            cv::Vec3i s1;
            // 現在の画素座標(x,y)から中心座標(mx,my)までの距離を計算
            r = (int)sqrt((x - mx) * (x - mx) + (y - my) * (y - my));
            // 中心から遠いほど画素値を暗くする
            s1[0] = s[0] - r / 2; // 青の画素値を暗くする
            s1[1] = s[1] - r / 2; // 緑の画素値を暗くする
            s1[2] = s[2] - r / 2; // 赤の画素値を暗くする

            /* 念の為書いた処理、エラー吐かなかったため不要
            if (s1[0]<0) s1[0] = 0; //青の画素値が負にならないようにする
            if (s1[1]<0) s1[1] = 0; //緑の画素値が負にならないようにする
            if (s1[2]<0) s1[2] = 0; //赤の画素値が負にならないようにする
            if (s1[0]>255) s1[0] = 255; //青の画素値が255を超えないようにする
            if (s1[1]>255) s1[1] = 255; //緑の画素値が255を超えないようにする
            if (s1[2]>255) s1[2] = 255; //赤の画素値が255を超えないようにする
            */

            s = s1;                              // int型をunsigned char型に丸める
            resultImage.at<cv::Vec3b>(y, x) = s; //"s"の値を"resultImage"の画素(x,y)に書き込む
        }
    }
    // ⑤ウィンドウへの画像の表示
    cv::imshow("Source", sourceImage);
    cv::imshow("Result", resultImage);

    // ⑥キー入力待ち
    cv::waitKey(0);

    // ⑦画像の保存
    cv::imwrite("result.jpg", resultImage);

    return 0;
}
