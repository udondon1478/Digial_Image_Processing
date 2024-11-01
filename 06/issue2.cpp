// g++ -std=c++11 dip06.cpp `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{
    // ①ルートディレクトリの画像ファイル"col.jpg"を読み込んで"sourceImage"に格納
    cv::Mat sourceImage = cv::imread("kadai.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data == 0)
    { // 画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);

    // ②画像格納用インスタンスの生成
    cv::Mat grayImage(sourceImage.size(), CV_8UC1);    // グレースケール画像用（1チャンネル）
    cv::Mat binImage(sourceImage.size(), CV_8UC1);     // ２値画像用（1チャンネル）
    cv::Mat contourImage(sourceImage.size(), CV_8UC3); // 輪郭表示画像用（3チャンネル）

    // ③原画像をグレースケール画像に、グレースケール画像を２値画像に変換
    //"sourceImage"をグレースケール画像に変換して"grayImage"に出力
    cv::cvtColor(sourceImage, grayImage, cv::COLOR_BGR2GRAY);
    //"grayImage"を2値化して"grayImage"に出力
    cv::threshold(grayImage, binImage, 50, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    //"sourceImage"のコピーを"contourImage"に出力
    contourImage = sourceImage.clone();

    // ④輪郭点格納用配列、輪郭の階層用配列の確保
    // 輪郭点格納用配列
    std::vector<std::vector<cv::Point>> contours;

    // binImageを膨張処理
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 1));
    cv::Mat element2 = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(7, 7));
    cv::Mat element3 = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
    cv::Mat element4 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 3));

    for (int j = 0; j < 10; j++)
    {
        cv::dilate(binImage, binImage, element3, cv::Point(-1, -1), 1);
        cv::erode(binImage, binImage, element3, cv::Point(-1, -1), 1);
        cv::dilate(binImage, binImage, element4, cv::Point(-1, -1), 1);
        cv::erode(binImage, binImage, element4, cv::Point(-1, -1), 1);
        cv::imshow("Gray", binImage);
    }

    cv::erode(binImage, binImage, element2, cv::Point(-1, -1), 2);
    cv::dilate(binImage, binImage, element2, cv::Point(-1, -1), 2);
    // ⑤"binImage"からの輪郭抽出処理
    // findContours()では入力画像が壊れるので、tmpImage に一時退避。壊れても良いなら不要。
    cv::Mat tmpImage = binImage.clone();
    // 輪郭抽出処理．輪郭ごとに輪郭画素位置を"counter"に格納
    cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    // Enterを押すたび次の輪郭を表示
    for (int i = 0; i < contours.size(); i++)
    {

        // モーメントを計算
        cv::Moments mu = cv::moments(contours[i]);
        // 中心（重心）を計算
        int cx = int(mu.m10 / mu.m00);
        int cy = int(mu.m01 / mu.m00);

        double circularity = 0.0;
        double area = cv::contourArea(contours[i]);
        double length = cv::arcLength(contours[i], true);
        circularity = 4 * 3.14159265358979323846 * area / (length * length);

        // 中心の色を取得
        cv::Vec3b color = sourceImage.at<cv::Vec3b>(cy, cx);

        // 色をログに出力
        printf("Center color of contour[%d]=B:%d, G:%d, R:%d\n", i, color[0], color[1], color[2]);
        // 中心の色がB:255,G:255,R:167の場合かつ、円形度が80%以上の場合、drawContoursを行う
        if (0.8 < circularity)
        {
            cv::drawContours(contourImage, contours, i, cv::Scalar(0, 0, 255), 2);
            //周囲長と面積、円形度を表示
            printf("Length[%d]=%f\n", i, length);
            printf("Area[%d]=%f\n", i, area);
            printf("Circularity[%d]=%f\n", i, circularity);
        }

        cv::imshow("Gray", binImage);

        cv::imshow("Contour", contourImage);
        // 円形どをログに出力

        printf("Circularity[%d]=%f\n", i, circularity);
        cv::waitKey(0);
    }

    cv::imshow("Gray", binImage);


    // 全ての輪郭の円形度をログに出力
    for (int i = 0; i < contours.size(); i++)
    {
        double circularity = 0.0;
        double area = cv::contourArea(contours[i]);
        double length = cv::arcLength(contours[i], true);
        circularity = 4 * 3.14159265358979323846 * area / (length * length);
        printf("Circularity[%d]=%f\n", i, circularity);
    }

    // ⑥ウィンドウを生成して各画像を表示
    // 原画像
    cv::namedWindow("Source");         // ウィンドウの生成
    cv::moveWindow("Source", 0, 50);   // ウィンドウの表示位置の指定
    cv::imshow("Source", sourceImage); // ウィンドウに画像を表示
    // グレースケール(2値化)
    cv::namedWindow("Gray");         // ウィンドウの生成
    cv::moveWindow("Gray", 150, 50); // ウィンドウの表示位置の指定
    cv::imshow("Gray", binImage);    // ウィンドウに画像を表示
    // 輪郭画像(原画像に輪郭を追加)
    cv::namedWindow("Contour");          // ウィンドウの生成
    cv::moveWindow("Contour", 300, 50);  // ウィンドウの表示位置の指定
    cv::imshow("Contour", contourImage); // ウィンドウに画像を表示

    // png形式でcontourImageを保存
    cv::imwrite("x22004_02.png", contourImage);
    cv::imwrite("x22004_02_gray.png", binImage);

    // ⑦キー入力があるまでここでストップ
    cv::waitKey(0);

    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
