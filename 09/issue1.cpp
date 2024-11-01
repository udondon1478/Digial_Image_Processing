/*
g++ dip09.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
g++ dip09.cpp `pkg-config --cflags --libs opencv`
 */

#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, char *argv[])
{
    // ①ビデオキャプチャの初期化
    cv::VideoCapture capture("balls.mov"); // ビデオファイルをオープン
    if (capture.isOpened() == 0)
    {
        printf("Capture not found\n");
        return -1;
    }

    // ②画像格納用インスタンス準備
    int w = capture.get(cv::CAP_PROP_FRAME_WIDTH);  // captureから動画横サイズ取得
    int h = capture.get(cv::CAP_PROP_FRAME_HEIGHT); // captureから動画縦サイズ取得
    cv::Size imageSize(w, h);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3); // 3チャンネル
    cv::Mat grayImage(imageSize, CV_8UC1);  // 1チャンネル
    cv::Mat edgeImage(imageSize, CV_8UC1);  // 1チャンネル
    cv::Mat recImage(imageSize, CV_8UC3);   // 3チャンネル

    // ③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Edge");
    cv::moveWindow("Edge", 100, 100);

    // ④ハフ変換用変数
    std::vector<cv::Point3f> circles;

    // ビデオライタ
    cv::VideoWriter writer("output.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, recImage.size());

    // ⑤動画処理用無限ループ
    while (1)
    {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        // ビデオが終了したら巻き戻し
        if (originalImage.data == NULL)
        {
            break;
        }
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);

        //(b)"frameImage"からグレースケール画像"grayImage"を生成
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);

        //(c)"grayImage"からエッジ画像"edgeImage"を生成
        cv::Canny(grayImage, edgeImage, 120, 160, 3);

        //(d')"grayImage"に円検出ハフ変換を施して，しきい値(90)以上の得票数を得た円群(x0,y0,r)を"circles"に格納
        cv::HoughCircles(grayImage, circles, cv::HOUGH_GRADIENT, 1, 30, 20, 15, 5, 30);

        //(e')検出された円の数("circles.size()")としきい値(200)の小さい方の数だけ繰り返し
        for (int i = 0; i < MIN(circles.size(), 200); i++)
        {
            cv::Point3f circle = circles[i];                                               //"circles"から円(x0, y0, r)を 1 組取り出し
            float x0 = circle.x;                                                           // 円の中心座標(x0, y0)の x 座標"x0"
            float y0 = circle.y;                                                           // 円の中心座標(x0, y0)の y 座標"y0"
            float r = circle.z;                                                            // 円の半径"r"
            cv::circle(frameImage, cv::Point(x0, y0), 3, cv::Scalar(0, 255, 0), -1, 8, 0); // 中心点の描画
            cv::circle(frameImage, cv::Point(x0, y0), r, cv::Scalar(0, 0, 255), 2, 8, 0);  // 円の描画
        }

        // 検出した円の個数を"Circle Count: "としてframeImageに表示
        cv::putText(frameImage, "Circle Count: " + std::to_string(circles.size()), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

        //(f)"frameImage"，"edgeImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Edge", edgeImage);

        //(g)キー入力待ち
        int key = cv::waitKey(10);
        //[Q]が押されたら無限ループ脱出
        if (key == 'q')
            break;

        // ビデオライタに書き込み
        cv::resize(frameImage, recImage, recImage.size());
        writer << recImage;
    }

    // ⑥終了処理
    // カメラ終了
    capture.release();
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
