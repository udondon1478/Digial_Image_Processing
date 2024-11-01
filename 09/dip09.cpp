#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ
#include <cmath>              // 数学関数

int main(int argc, char *argv[])
{
    // ①ビデオキャプチャの初期化
    cv::VideoCapture capture("swingcar.mp4"); // ビデオファイルをオープン
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
    cv::Mat rotatedImage;                   // 回転後の画像

    // ③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Edge");
    cv::moveWindow("Edge", 100, 100);

    // ビデオライタ
    cv::VideoWriter writer("output.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, imageSize);

    // ⑤動画処理用無限ループ
    while (1)
    {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        // ビデオが終了したら巻き戻し
        if (originalImage.data == NULL)
        {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);

        //(b)"frameImage"からグレースケール画像"grayImage"を生成
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);

        //(c)"grayImage"からエッジ画像"edgeImage"を生成
        cv::Canny(grayImage, edgeImage, 50, 150, 3);

        //(d) ハフ変換で直線検出
        std::vector<cv::Vec4i> lines;
        cv::HoughLinesP(edgeImage, lines, 1, CV_PI / 180, 50, 50, 10);

        // 最も長い線分を見つける
        double maxLen = 0;
        double angle = 0;
        for (size_t i = 0; i < lines.size(); i++)
        {
            cv::Vec4i l = lines[i];
            double dx = l[2] - l[0];
            double dy = l[3] - l[1];
            double len = std::sqrt(dx * dx + dy * dy);
            if (len > maxLen)
            {
                maxLen = len;
                angle = std::atan2(dy, dx) * 180.0 / CV_PI;
            }
        }

        // 画像の中心を基準に回転
        cv::Point2f center(frameImage.cols / 2.0, frameImage.rows / 2.0);
        cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, 1.0);
        cv::warpAffine(frameImage, rotatedImage, rotationMatrix, frameImage.size());

        // 検出した線を描画
        for (size_t i = 0; i < lines.size(); i++)
        {
            cv::Vec4i l = lines[i];
            cv::line(rotatedImage, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
        }

        // 検出した線の個数を"Line Count: "としてrotatedImageに表示
        cv::putText(rotatedImage, "Line Count: " + std::to_string(lines.size()), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

        //(f)"rotatedImage"，"edgeImage"の表示
        cv::imshow("Frame", rotatedImage);
        cv::imshow("Edge", edgeImage);

        //(g)キー入力待ち
        int key = cv::waitKey(10);
        //[Q]が押されたら無限ループ脱出
        if (key == 'q')
            break;

        // ビデオライタに書き込み
        writer << rotatedImage;
    }

    // ⑥終了処理
    // カメラ終了
    capture.release();
    writer.release();
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
