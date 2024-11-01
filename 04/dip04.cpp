// g++ issue1.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{
    int width = 640, height = 480;

    // 4*4のディザパターンを宣言
    unsigned char dither[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}};

    // ①カメラの初期化
    cv::VideoCapture capture(0); // カメラ0番をオープン
    // カメラがオープンできたかどうかをチェック
    if (capture.isOpened() == 0)
    {
        printf("Camera not found\n");
        return -1;
    }

    // ②画像格納用インスタンス準備
    cv::Mat captureImage;                                           // キャプチャ用
    cv::Mat frameImage = cv::Mat(cv::Size(width, height), CV_8UC3); // 処理用
    cv::Mat grayImage(cv::Size(width, height), CV_8UC1);            // 1チャンネル
    cv::Mat resultGImage(cv::Size(width, height), CV_8UC1);         // 1チャンネル
    cv::Mat recImage(cv::Size(width, height), CV_8UC3);             // 3チャンネル
    cv::Mat binImage(cv::Size(width, height), CV_8UC1);             // 1チャンネル

    // ③ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Gray");
    cv::moveWindow("Gray", width, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, height);
    cv::moveWindow("Bin", 0, height);

    // ④ルックアップテーブルの作成
    unsigned char lookupTable[256];
    // 17段階に分ける
    for (int i = 0; i < 256; i++)
    {
        lookupTable[i] = (i / 16) * 16;
    }

    // ⑤ビデオライタ生成(ファイル名，コーデック，フレームレート，フレームサイズ)
    cv::VideoWriter rec("rec.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, recImage.size());

    // ⑥動画像処理無限ループ
    while (1)
    {
        //(a)カメラから1フレームを" captureImage"に読み込み
        capture >> captureImage;
        if (captureImage.data == NULL)
            break;

        //(b)" captureImage"をリサイズして" frameImage"に格納
        cv::resize(captureImage, frameImage, frameImage.size());

        // frameImageを1/4にリサイズ
        cv::resize(frameImage, frameImage, cv::Size(width / 4, height / 4));

        cv::resize(binImage, binImage, cv::Size(width / 4, height / 4));

        //(c)"frameImage"をグレースケールに変換して"grayImage"に格納
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);

        //(d)"grayImage"の各画像を走査して，ルックアップテーブルに基づいて画素値変換して"resultGImage"に格納
        for (int j = 0; j < height / 4; j++)
        {
            for (int i = 0; i < width / 4; i++)
            {
                unsigned char s = grayImage.at<unsigned char>(j, i);
                s = lookupTable[s];
                binImage.at<unsigned char>(j, i) = s;

                int n = s/17; // 0~16の値を取得

                int sheight = height / 4;
                int swidth = width / 4;

                // ディザパターンに含まれる値とnを比較して2値化
                //resultGImageはbinImageの4倍のサイズ
                //binImageの1画素はresultGImageの4画素に対応
                for (int y = 0; y < 4; y++)
                {
                    for (int x = 0; x < 4; x++)
                    {
                        if (dither[y][x] > n)
                        {
                            resultGImage.at<unsigned char>(j * 4 + y, i * 4 + x) = 0;
                        }
                        else
                        {
                            resultGImage.at<unsigned char>(j * 4 + y, i * 4 + x) = 255;
                        }
                    }
                }
            }
        }

        //(e)ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Gray", grayImage);
        cv::imshow("Result", resultGImage);
        cv::imshow("Bin", binImage);

        //(f)動画ファイル書き出し
        cv::cvtColor(resultGImage, recImage, cv::COLOR_GRAY2BGR); // 動画用3チャンネル画像生成
        rec << recImage;                                          // ビデオライタに画像出力

        //(g)キー入力待ち
        char key = cv::waitKey(20); // 20ミリ秒待機
        if (key == 'q')
            break;
    }

    return 0;
}