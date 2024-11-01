//(OpenCV4) g++ -std=c++11 dip05.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip05.cpp `pkg-config --cflags --libs opencv`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ


// アニメ調色変換関数の宣言
void myAnimeColor(const cv::Mat &src, cv::Mat &dst);
unsigned char lookupTable[3][256];

int main(int argc, char *argv[])
{
    // 4*4のディザパターンを宣言
    unsigned char dither[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}};

        int blockSize = 16; // 各領域のサイズ

    // アニメ変換用ルックアップテーブル
    for (int i = 0; i < 256; i++)
    {
        // H
        lookupTable[0][i] = i;
        // S
        lookupTable[1][i] = i;
        // V
        if (i < 20)
            lookupTable[2][i] = 0;
        else if (i < 60)
            lookupTable[2][i] = 85;
        else if (i < 180)
            lookupTable[2][i] = 170;
        else
            lookupTable[2][i] = 255;
    }

    // ①ビデオキャプチャを初期化して，映像を取り込む
    // cv::VideoCapture capture(0);  //内臓カメラをオープン
    cv::VideoCapture capture("scene.mov"); // 指定したビデオファイルをオープン
    // ビデオファイルがオープンできたかどうかをチェック
    if (capture.isOpened() == 0)
    {
        printf("Camera not found\n");
        return -1;
    }

    // ②画像格納用インスタンス準備
    int width = 720, height = 405;
    cv::Mat frameImage;
    cv::Mat originalImage(cv::Size(width, height), CV_8UC3);
    cv::Mat binImage(cv::Size(width, height), CV_8UC3);
    cv::Mat grayImage(cv::Size(width, height), CV_8UC1);
    cv::Mat resultImage(cv::Size(width, height), CV_8UC3);
    cv::Mat resizedImage(cv::Size(width, height), CV_8UC3);
    cv::Mat recImage(cv::Size(width, height), CV_8UC3);

    // ③画像表示用ウィンドウの生成
    cv::namedWindow("Original");
    cv::moveWindow("Original", 0, 50);
    cv::namedWindow("Gray");
    cv::moveWindow("Gray", 0, 200);
    cv::namedWindow("Bin");
    cv::moveWindow("Bin", 0, 350);
    cv::namedWindow("Resized");
    cv::moveWindow("Resized", 200, 50);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 200, 200);

    // ④ラプシアンフィルタの生成
    // ラプシアンフィルタの係数を要素とする1次元配列の作成(5x5の平均値フィルタ用)
    float fdata[] = {0, 0, 1, 0, 0,
                     0, 1, 2, 1, 0,
                     1, 2, -16, 2, 1,
                     0, 1, 2, 1, 0,
                     0, 0, 1, 0, 0};

    // 一次元配列の要素に基づき，線形空間フィルタをCvMat型の5x5行列"kernel"として生成
    cv::Mat kernel(cv::Size(5, 5), CV_32F, fdata);

    // ビデオライタ生成
    cv::VideoWriter rec("rec.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, recImage.size());

    // ⑤動画像処理無限ループ：「ビデオキャプチャから1フレーム取り込み」→「画像処理」→「表示」の繰り返し
    while (1)
    {
        //(a)ビデオキャプチャ"capture"から1フレームを取り込んで，"frameImage"に格納
        capture >> frameImage;
        // ビデオが終了したら無限ループから脱出
        if (frameImage.data == NULL)
            break;

        //(b)"frameImage"をリサイズして"originalImage"に格納
        cv::resize(frameImage, originalImage, originalImage.size());

        // originalImageをresultImageにコピー
        originalImage.copyTo(resultImage);

        /*
        myAnimeColor(originalImage, resultImage);
        */

        // 明度の平均値を取得するためにリサイズ
        cv::resize(resultImage, resizedImage, cv::Size(width / 4, height / 4));

        cv::cvtColor(resizedImage, resizedImage, cv::COLOR_BGR2HSV);

        // resizedImageの各画素を走査して，ディザパターンに基づいて画素値変換
        for (int j = 0; j < height / 4; j++)
        {
            for (int i = 0; i < width / 4; i++)
            {
                // nにresizedImageの現在画素のVの値を代入
                float n = resizedImage.at<cv::Vec3b>(j, i)[3];

                // nの値に応じてresultImageの画素に対して処理を行う
                // resultImageはresizedImageの4倍のサイズ
                // resizedImageの1画素はresultImageの4画素に対応
                for (int y = 0; y < 4; y++)
                {
                    for (int x = 0; x < 4; x++)
                    {
                        if (n > 50)
                        {
                            // continue;
                        }
                        else if (n > 40)
                        {

                            cv::line(resultImage, cv::Point(i * 4, j * 4), cv::Point(i * 4 + x + 3, j * 4 + x + 3), cv::Scalar(0, 0, 0), 1, 4);
                        }
                        else if (n > 10)
                        {
                            // 線を2本引く
                            cv::line(resultImage, cv::Point(i * 4, j * 4), cv::Point(i * 4 + x + 3, j * 4 + x + 3), cv::Scalar(0, 0, 0), 1, 4);
                            cv::line(resultImage, cv::Point(i * 4 + 2, j * 4 + 2), cv::Point(i * 4 + x + 1, j * 4 + x + 1), cv::Scalar(0, 0, 0), 1, 4);
                        }
                    }
                }
            }
        }

        // resultImageにメディアンフィルタを適応
        cv::medianBlur(resultImage, resultImage, 5);

        //(c)"frameImage"をグレースケールに変換して"grayImage"に格納
        cv::cvtColor(originalImage, grayImage, cv::COLOR_BGR2GRAY);

        //(d)"grayImage"に線形空間フィルタ"kernel"を適用して"binImage"を出力
        cv::filter2D(grayImage, binImage, grayImage.depth(), kernel);

        // binを二値化
        cv::threshold(binImage, binImage, 190, 255, cv::THRESH_BINARY);

        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {
                if (binImage.at<unsigned char>(j, i) == 255)
                {
                    // 画素値を0に変更
                    resultImage.at<cv::Vec3b>(j, i) = cv::Vec3b(0, 0, 0);
                }
                else
                {
                    resultImage.at<cv::Vec3b>(j, i) = resultImage.at<cv::Vec3b>(j, i);
                }
            }
        }
        //(e)ウィンドウに画像表示
        cv::imshow("Original", originalImage);
        cv::imshow("Gray", grayImage);
        cv::imshow("Bin", binImage);
        cv::imshow("Resized", resizedImage);
        cv::imshow("Result", resultImage);

        // 動画に書き出し
        cv::resize(resultImage, recImage, recImage.size());
        rec << recImage;

        //(f)[q]キーが押されたら無限ループから脱出
        int key = cv::waitKey(10);
        if (key == 'q')
            break;
    }

    // ⑥メッセージを出力して終了
    printf("Finished\n");
    return 0;
}

// アニメ調色変換関数（src：入力画像，dst：出力画像）
void myAnimeColor(const cv::Mat &src, cv::Mat &dst)
{
    // 作業用画像生成
    cv::Mat cImage(src.size(), CV_8UC3); // 3チャンネル

    // 色変換
    cv::cvtColor(src, cImage, cv::COLOR_BGR2HSV); // RGB→HSV
    cv::Vec3b s;
    for (int j = 0; j < src.rows; j++)
    {
        for (int i = 0; i < src.cols; i++)
        {
            // ルックアップテーブルで各画素値変換
            s = cImage.at<cv::Vec3b>(j, i);
            s[0] = lookupTable[0][s[0]];
            s[1] = lookupTable[1][s[1]];
            s[2] = lookupTable[2][s[2]];
            cImage.at<cv::Vec3b>(j, i) = s;
        }
    }

    cv::cvtColor(cImage, dst, cv::COLOR_HSV2BGR); // HSV→RGB
}