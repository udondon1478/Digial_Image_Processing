//(OpenCV4) g++ dip08.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{
    // ①画像ファイルの読み込み
    // 画像ファイル"ferarri.jpg"を読み込んで，画像データ"sourceImage"に格納
    cv::Mat sourceImage = cv::imread("ferarri.jpg", cv::IMREAD_COLOR);
    cv::Mat sourceImage2 = cv::imread("milano.jpg", cv::IMREAD_COLOR);
    cv::Mat recImage(sourceImage.size(), CV_8UC3);

    if (sourceImage.data == 0 || sourceImage2.data == 0)
    { // 画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);

    cv::VideoWriter writer("output.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, recImage.size());

    // ②画像表示用ウィンドウの生成
    cv::namedWindow("Translate");

    // ③3チャンネル画像"translateImage"の確保（画像ファイルと同サイズ）
    //"sourceImage"と同サイズ・3チャンネル・ゼロで初期化
    cv::Mat translateImage = cv::Mat::zeros(sourceImage.size(), CV_8UC3);

    // 背景にmilano.jpgを張り付け
    cv::Mat roi = translateImage(cv::Rect(0, 0, sourceImage2.cols, sourceImage2.rows));

    sourceImage2.copyTo(roi);

    // ④回転移動行列"rotateMat"の生成
    cv::Point2f center = cv::Point2f(sourceImage.cols / 2, sourceImage.rows / 2); // 回転中心
    double angle = 0.0;                                                          // 回転角度
    double scale = 1.0;                                                           // 拡大率
    cv::Mat rotateMat = cv::getRotationMatrix2D(center, angle, scale);            // 行列生成

    // 上記の処理をscaleが0を下回るまでループさせ、回転させながら収縮させる
    while ( scale > 0.0)
    {
        //背景にmilano.jpgを張り付け
        cv::Mat roi = translateImage(cv::Rect(0, 0, sourceImage2.cols, sourceImage2.rows));
        sourceImage2.copyTo(roi);

        // ④回転移動行列"rotateMat"の生成
        angle = angle + 1.0;
        scale = scale - 0.01;
        cv::Mat rotateMat = cv::getRotationMatrix2D(center, angle, scale); // 行列生成

        // 行列要素表示(確認用)
        printf("%f %f %f\n", rotateMat.at<double>(0, 0), rotateMat.at<double>(0, 1), rotateMat.at<double>(0, 2));
        printf("%f %f %f\n", rotateMat.at<double>(1, 0), rotateMat.at<double>(1, 1), rotateMat.at<double>(1, 2));

        // ⑤"sourceImage"に回転移動"rotateMat"を施して"translateImage"に張り付け
        // 値が決定されない画素はSourceImage2の画素で埋める
        cv::warpAffine(sourceImage, roi, rotateMat, roi.size(), cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);

        // ⑥"translateImage"の表示
        cv::imshow("Translate", translateImage);

        //(h)キー入力待ち
        int key = cv::waitKey(20);
        //'q'が押されたら無限ループ脱出
        if (key == 'q')
        {
            break;
        }

        // ⑦動画ファイルへの書き込み
        writer << translateImage;


    }



    return 0;
}
