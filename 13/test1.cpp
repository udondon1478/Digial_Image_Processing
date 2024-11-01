//g++ ai1.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ
#include <vector>

using namespace cv;
using namespace std;

// パンの種類と対応する色
enum BreadType {
    GoroGoroChocoChipScone,
    CheeseBusse,
    ThickCutChocoCake,
    BananaBread,
    LemonBaum
};

const Scalar breadColors[] = {
    Scalar(255, 255, 0), // シアン
    Scalar(255, 0, 0),   // 青
    Scalar(0, 255, 0),   // 緑
    Scalar(255, 0, 255), // マゼンタ
    Scalar(0, 0, 255)    // 赤
};

// 円形度を計算する関数
double calculateCircularity(const RotatedRect& rect) {
    double area = rect.size.area();
    double perimeter = 2 * (CV_PI * (rect.size.width / 2) + CV_PI * (rect.size.height / 2));
    return 4 * CV_PI * area / (perimeter * perimeter);
}

// 輪郭をパンとして認識するかどうか判定する関数
bool isBreadContour(const vector<Point>& contour, double minAreaThreshold, double maxAreaThreshold, double minCircularityThreshold) {
    // 面積が小さい輪郭は無視
    if (contourArea(contour) < minAreaThreshold || contourArea(contour) > maxAreaThreshold) {
        return false;
    }

    // 輪郭を近似する
    vector<Point> approx;
    approxPolyDP(contour, approx, arcLength(contour, true) * 0.02, true);

    // 輪郭がほぼ円形かつ面積が大きい場合、パンとして認識
    RotatedRect rect = minAreaRect(approx);
    double circularity = calculateCircularity(rect);
    if (circularity > minCircularityThreshold) {
        return true;
    }

    return false;
}

int main(int argc, char* argv[]) {
    // ビデオキャプチャの初期化
    VideoCapture capture("pantora.mp4");  // ビデオファイルをオープン
    if (capture.isOpened() == 0) {  // オープンに失敗した場合
        printf("Capture not found\n");
        return -1;
    }

    // 画像格納用インスタンス準備
    Mat frameImage;  // ビデオキャプチャ用
    int width = capture.get(CAP_PROP_FRAME_WIDTH);
    int height = capture.get(CAP_PROP_FRAME_HEIGHT);
    Size imageSize(width, height);  // ビデオ画像サイズ
    printf("imageSize = (%d, %d)\n", width, height);  // ビデオ画像サイズ表示

    // 画像表示用ウィンドウの生成
    namedWindow("Frame");
    namedWindow("Processed Frame");

    // パンの種類別インジケータ表示のための円形 (Point のベクトル)
    vector<Point> indicators;
    for (int i = 0; i < 5; i++) {
        indicators.push_back(Point(20 + i * 40, 20)); 
    }

    // 動画処理用無限ループ
    while (1) {
        // ビデオキャプチャから1フレーム"frameImage"に取り込み
        capture >> frameImage;
        // ビデオが終了したら無限ループから脱出
        if (frameImage.data == NULL) {
            break;
        }

        // グレースケール変換
        Mat grayImage;
        cvtColor(frameImage, grayImage, COLOR_BGR2GRAY);

        // ガウシアンブラー
        GaussianBlur(grayImage, grayImage, Size(5, 5), 0);

        // 2値化 (閾値を 180 に変更)
        Mat thresholdImage;
        threshold(grayImage, thresholdImage, 190, 255, THRESH_BINARY_INV);

        // 輪郭抽出
        vector<vector<Point>> contours;
        findContours(thresholdImage, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // 認識結果格納用
        vector<vector<Point>> breadContours;
        vector<BreadType> breadTypes;

        // 輪郭の判定と認識
        for (size_t i = 0; i < contours.size(); i++) {
            // 面積が小さい輪郭は無視
            // 面積が小さい輪郭を無視する最小閾値
            double minAreaThreshold = 2000;
            // 面積が大きい輪郭を無視する最大閾値
            double maxAreaThreshold = 30000; // 調整してください
            // 円形度の最小閾値
            double minCircularityThreshold = 0.3; 

            // 一定の面積以下の領域は描画しない
            if (contourArea(contours[i]) >= minAreaThreshold && contourArea(contours[i]) <= maxAreaThreshold) { 
                // 円形度が一定値以上の輪郭のみ描画
                RotatedRect rect = minAreaRect(contours[i]);
                double circularity = calculateCircularity(rect);
                if (circularity >= minCircularityThreshold) {
                    drawContours(frameImage, contours, i, Scalar(0, 0, 255), 2); // 赤色で描画
                }
            }

            // パンの種類判定
            if (isBreadContour(contours[i], minAreaThreshold, maxAreaThreshold, minCircularityThreshold)) {
                breadContours.push_back(contours[i]);

                // パンの種類判定
                // ここでは単純に輪郭の中心座標によって判定している
                // より精度の高い判定には、特徴量抽出や機械学習などの手法が必要
                RotatedRect rect = minAreaRect(contours[i]);
                double centerX = rect.center.x;
                if (centerX < width / 3) {
                    breadTypes.push_back(GoroGoroChocoChipScone);
                } else if (centerX < 2 * width / 3) {
                    breadTypes.push_back(CheeseBusse);
                } else {
                    breadTypes.push_back(ThickCutChocoCake);
                }
            }
        }

        // 認識したパンの輪郭をbreadColorsに基づいて描画
        for (size_t i = 0; i < breadContours.size(); i++) {
            // パンの種類に対応する色を取得
            Scalar color = breadColors[breadTypes[i]];
            drawContours(frameImage, breadContours, i, color, 2); // FrameImage に描画
        }

        // 認識したパンのインジケータを描画
        for (size_t i = 0; i < breadTypes.size(); i++) {
            // 円を描画する (色を指定)
            circle(frameImage, indicators[breadTypes[i]], 10, breadColors[breadTypes[i]], -1);
        }

        // 画像表示
        imshow("Frame", frameImage);
        imshow("Processed Frame", thresholdImage);

        // キー入力待ち
        int key = waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key == 'q')
            break;
    }

    // 終了処理
    // カメラ終了
    capture.release();
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}