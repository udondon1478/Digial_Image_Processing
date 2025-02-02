//(OpenCV4) g++ -std=c++11 dip07a.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip07a.cpp `pkg-config --cflags --libs opencv`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

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
    cv::Size imageSize(720, 405);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);
    cv::Mat backImage(imageSize, CV_8UC3);
    cv::Mat subImage(imageSize, CV_8UC3);
    cv::Mat subBinImage(imageSize, CV_8UC1);
    cv::Mat resultImage(imageSize, CV_8UC3);
    cv::Mat contImage(imageSize, CV_8UC3);
    cv::Mat recImage(imageSize, CV_8UC3);

    // 領域の輪郭を抽出
    std::vector<std::vector<cv::Point>> contours;
    double area = 0;

    // ③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Back");
    cv::moveWindow("Back", 50, 50);
    cv::namedWindow("Subtraction");
    cv::moveWindow("Subtraction", 100, 100);


    cv::namedWindow("Result");
    cv::moveWindow("Result", 150, 150);

    cv::VideoWriter writer("output.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, recImage.size());

    // ④動画処理用無限ループ
    while (1)
    {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        // ビデオが終了したら無限ループから脱出
        if (originalImage.data == NULL)
            break;
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);

        //frameImageのサイズを取得
        int width = frameImage.cols;
        int height = frameImage.rows;

        //(b)"frameImage"と"backImage"との差分画像"subImage"の生成
        cv::absdiff(frameImage, backImage, subImage);

        //(b')"subImage"をグレースケール化して"subBinImage"生成
        cv::cvtColor(subImage, subBinImage, cv::COLOR_BGR2GRAY);
        cv::threshold(subBinImage, subBinImage, 15, 255, cv::THRESH_BINARY);

        //(b")"frameImage"を"subBinImage"マスク付きで"resultImage"にコピー
        resultImage = cv::Scalar(0);

        cv::findContours(subBinImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

        // contImage.copyTo(resultImage);
        //輪郭の描画
        cv::drawContours(contImage, contours, -1, cv::Scalar(0, 0, 255), 2);



        //(c)"frameImage"，"backImage"，"subImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Back", backImage);
        cv::imshow("Subtraction", subImage);
        cv::imshow("Subtraction Binary", subBinImage);
        cv::imshow("Contour", contImage);
        cv::imshow("Result", resultImage);

        // //(d)"frameImage"で"backImage"を更新
        // frameImage.copyTo(backImage);

        //(e)キー入力待ち
        int key = cv::waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key == 'q')
            break;
        if (key == 'c')
        {
            frameImage.copyTo(backImage);
        }

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
