#include <iostream>
#include <opencv2/opencv.hpp>

// グリーンバックを透過させるための関数
cv::Mat applyGreenScreenTransparency(const cv::Mat &image)
{
    cv::Mat hsvImage, mask;
    cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);
    cv::inRange(hsvImage, cv::Scalar(35, 40, 40), cv::Scalar(85, 255, 255), mask);

    cv::Mat transparentImage;
    cv::cvtColor(image, transparentImage, cv::COLOR_BGR2BGRA);

    for (int y = 0; y < image.rows; y++)
    {
        for (int x = 0; x < image.cols; x++)
        {
            if (mask.at<uchar>(y, x) != 0)
            {
                transparentImage.at<cv::Vec4b>(y, x)[3] = 0; // アルファチャンネルを0にして透過
            }
        }
    }
    return transparentImage;
}

// 船の画像をアフィン変換する関数
cv::Mat affineTransformShipImage(const cv::Mat &source, const cv::Point2f &center, float angle)
{
    cv::Mat transformedImage;
    cv::Mat rotationMat = cv::getRotationMatrix2D(center, angle, 1.0);
    cv::Rect bbox = cv::RotatedRect(cv::Point2f(), source.size(), angle).boundingRect();
    rotationMat.at<double>(0, 2) += bbox.width / 2.0 - center.x;
    rotationMat.at<double>(1, 2) += bbox.height / 2.0 - center.y;
    cv::warpAffine(source, transformedImage, rotationMat, bbox.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0, 0));
    return transformedImage;
}

// 画像をフレームに合成する関数
void overlayImage(cv::Mat &background, const cv::Mat &foreground, cv::Point2f location)
{
    for (int y = std::max(location.y, 0.0f); y < background.rows; ++y)
    {
        int fY = y - location.y; // フォアグラウンドのy座標

        if (fY >= foreground.rows)
            break;

        for (int x = std::max(location.x, 0.0f); x < background.cols; ++x)
        {
            int fX = x - location.x; // フォアグラウンドのx座標

            if (fX >= foreground.cols)
                break;

            double opacity = ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3]) / 255.0;

            for (int c = 0; opacity > 0 && c < background.channels(); ++c)
            {
                unsigned char foregroundPx = foreground.data[fY * foreground.step + fX * foreground.channels() + c];
                unsigned char backgroundPx = background.data[y * background.step + x * background.channels() + c];
                background.data[y * background.step + background.channels() * x + c] =
                    backgroundPx * (1. - opacity) + foregroundPx * opacity;
            }
        }
    }
}

int main(int argc, char *argv[])
{

    // ビデオキャプチャを初期化して、映像を取り込む
    cv::VideoCapture capture("water1.mov"); // 指定したビデオファイルをオープン
    if (!capture.isOpened())
    {
        std::cerr << "Camera not found" << std::endl;
        return -1;
    }
    // フレームサイズ取得
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    std::cout << "Frame Size = (" << width << ", " << height << ")" << std::endl;
    cv::Size imageSize(width, height); // フレームと同じ画像サイズ定義

    // 船画像"ship.jpg"の読み込みと透過処理
    cv::Mat shipImage = cv::imread("ship.jpg", cv::IMREAD_COLOR);
    if (shipImage.empty())
    {
        std::cerr << "Ship image not found" << std::endl;
        return -1;
    }
    cv::Mat transparentShipImage = applyGreenScreenTransparency(shipImage);

    // shipImageのサイズを測定
    int shipWidth = transparentShipImage.cols;
    int shipHeight = transparentShipImage.rows;

    // 画像格納用インスタンス準備
    cv::Mat frameImage;
    cv::Mat recImage(imageSize, CV_8UC3); // フレーム画像

    //recImageの半分のサイズでビデオライタを生成
    cv::Size halfSize(recImage.cols / 2, recImage.rows / 2);
    cv::VideoWriter writer("output.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, halfSize);
    

    // オプティカルフロー準備
    cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 10, 0.01); // 終了条件
    cv::Mat presentImage(imageSize, CV_8UC1), priorImage(imageSize, CV_8UC1);                                   // 現フレーム濃淡画像，前フレーム濃淡画像
    std::vector<cv::Point2f> presentFeature, priorFeature;                                                      // 現フレーム対応点，前フレーム追跡点
    std::vector<unsigned char> status;                                                                          // 処理用
    std::vector<float> errors;                                                                                  // 処理用

    // 船の初期位置
    cv::Point2f shipPoint(130, 190);

    // ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Ship");
    cv::moveWindow("Ship", width, 0);

    // 船画像の表示
    cv::imshow("Ship", shipImage);

    // 平滑化用移動量
    cv::Point2f smoothedMove(0, 0);
    float alpha = 0.1; // スムージング係数
    float angle = 0.0; // 初期角度

    // 動画像処理無限ループ
    while (true)
    {
        //===== カメラから1フレーム読み込み =====
        capture >> frameImage;
        if (frameImage.empty())
            break;

        // 以前の追跡点を削除
        priorFeature.clear();

        // shipPointを中心に5x5のグリッド状の追跡点を設定
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                priorFeature.push_back(cv::Point2f(shipPoint.x - 10 + 5 * i, shipPoint.y - 10 + 5 * j));
            }
        }

        //===== オプティカルフロー =====
        cv::cvtColor(frameImage, presentImage, cv::COLOR_BGR2GRAY); // 現フレーム濃淡画像"presentImage"を生成
        int opCnt = priorFeature.size(); // 追跡点の個数
        cv::Point2f totalMove(0, 0);
        int moveCnt = 0;

        if (opCnt > 0)
        {
            cv::calcOpticalFlowPyrLK(priorImage, presentImage, priorFeature, presentFeature, status, errors, cv::Size(10, 10), 3, criteria);

            for (int i = 0; i < opCnt; i++)
            {
                if (status[i])
                {
                    cv::Point2f move = presentFeature[i] - priorFeature[i];
                    totalMove += move;
                    moveCnt++;
                }
            }

            if (moveCnt > 0)
            {
                totalMove *= (1.0 / moveCnt); // 平均移動量
                smoothedMove = alpha * totalMove + (1 - alpha) * smoothedMove; // 移動量を平滑化

                // 壁との衝突を検出して修正
                cv::Rect shipRect(shipPoint.x - shipWidth / 2, shipPoint.y - shipHeight / 2, shipWidth, shipHeight);
                shipRect.x = std::max(shipRect.x, 0);
                shipRect.y = std::max(shipRect.y, 0);
                shipRect.width = std::min(shipRect.width, width - shipRect.x);
                shipRect.height = std::min(shipRect.height, height - shipRect.y);

                if ((shipRect & cv::Rect(0, 0, width, height)) == shipRect)
                {
                    shipPoint += smoothedMove;

                    // 移動方向に基づいて船の回転角度を計算
                    float newAngle = std::atan2(smoothedMove.y, smoothedMove.x) * -180 / CV_PI;
                    angle = 0.9 * angle + 0.1 * newAngle; // 激しい回転を防ぐために平滑化
                }
            }

        }
        presentImage.copyTo(priorImage);

        // 船の画像をアフィン変換して描画
        cv::Mat transformedShip = affineTransformShipImage(transparentShipImage, cv::Point2f(shipWidth / 2.0, shipHeight / 2.0), angle);
        overlayImage(frameImage, transformedShip, cv::Point(shipPoint.x - transformedShip.cols / 2, shipPoint.y - transformedShip.rows / 2));

        // ウィンドウに画像表示
        cv::imshow("Frame", frameImage);

        // キー入力待ち
        char key = cv::waitKey(20);
        if (key == 'q')
            break;
        
        //動画ファイルへの書き込み
        cv::resize(frameImage, recImage, halfSize);
        writer << recImage;
    }

    // メッセージを出力して終了
    std::cout << "Finished" << std::endl;
    return 0;
}
