#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>

using namespace cv;
using namespace std;

void myAnimeColor(const cv::Mat &src, cv::Mat &dst);
unsigned char lookupTable[3][256];

void drawShading(Mat &image, Mat &image2, int blockSize)
{
    int rows = image.rows;
    int cols = image.cols;

    // 乱数ジェネレーターのセットアップ
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(-blockSize / 4.0, blockSize / 4.0);

    for (int y = 0; y < rows; y += blockSize)
    {
        for (int x = 0; x < cols; x += blockSize)
        {
            Rect blockRect(x, y, blockSize, blockSize);
            Mat block = image(blockRect);

            // 4x4領域の平均明度を求める
            Scalar meanValue = mean(block);
            int meanIntensity = static_cast<int>(meanValue[0]);

            // 明度に応じて線を描画
            int numLines;
            if (meanIntensity < 100)
            {
                numLines = 3; // 非常に暗い
            }
            else if (meanIntensity < 150)
            {
                numLines = 2; // 普通に暗い
            }
            else if (meanIntensity < 192)
            {
                numLines = 1; // やや暗い
            }
            else
            {
                numLines = 0; // 明るい
            }

            for (int i = 0; i < numLines; ++i)
            {
                // ランダムなオフセットを計算
                float offsetX1 = dis(gen);
                float offsetY1 = dis(gen);
                float offsetX2 = dis(gen);
                float offsetY2 = dis(gen);

                // width = 720, height = 405のサイズに合わせて座標を変更
                //  斜め線を描画
                Point p1(x + offsetX1, y + offsetY1);
                Point p2(x + blockSize + offsetX2, y + blockSize + offsetY2);
                line(image2, p1, p2, Scalar(0), 1);
            }
        }
    }
}

int main(int argc, char **argv)
{
        // アニメ変換用ルックアップテーブル
    for (int i = 0; i < 256; i++)
    {
        // H
        lookupTable[0][i] = i;
        // S
        lookupTable[1][i] = i;
        // V
        if (i < 20)
            lookupTable[2][i] = 200;
        else if (i < 60)
            lookupTable[2][i] = 200;
        else if (i < 180)
            lookupTable[2][i] = 200;
        else
            lookupTable[2][i] = 200;
    }

    // 動画ファイルの読み込み
    VideoCapture cap("scene.mov");
    if (!cap.isOpened())
    {
        cerr << "Error opening video file" << endl;
        return -1;
    }

    int width = 720, height = 405;
    Mat frameImage;
    Mat originalImage;
    Mat grayImage;
    Mat resultImage(Size(width, height), CV_8UC3);
    Mat recImage(Size(width, height), CV_8UC3);

    int blockSize = 16; // 各領域のサイズ

    namedWindow("Original");
    moveWindow("Original", 0, 50);
    namedWindow("Gray");
    moveWindow("Gray", 0, 200);
    namedWindow("Result");
    moveWindow("Result", 0, 350);

    VideoWriter rec("rec.mp4", VideoWriter::fourcc('M', 'P', '4', 'V'), 30, recImage.size());

    while (true)
    {
        // フレームを取得
        cap >> frameImage;
        if (frameImage.empty())
        {
            break; // 動画の最後まで再生したらループを終了
        }

        // frameImageをoriginalImageにコピー
        frameImage.copyTo(originalImage);

        // グレースケール変換
        cvtColor(originalImage, grayImage, COLOR_BGR2GRAY);

        myAnimeColor(originalImage, originalImage);

        // resultImageにoriginal
        originalImage.copyTo(resultImage);

        // シェーディングを描画
        drawShading(grayImage, resultImage, blockSize);

        // resultImageをリサイズ
        resize(resultImage, recImage, recImage.size());
        // 結果を表示
        imshow("Original", originalImage);
        imshow("Gray", grayImage);
        imshow("Shaded Video", resultImage);

            // 動画に書き出し
                    cv::resize(resultImage, recImage, recImage.size());
                    rec << recImage;
        // 'q'キーが押されたらループを終了
        if (waitKey(30) == 'q')
        {
            break;
        }
    }



    cap.release();
    destroyAllWindows();

    return 0;
}

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