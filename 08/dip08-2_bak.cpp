#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, char *argv[])
{
    // ビデオキャプチャを初期化して，映像を取り込む
    cv::VideoCapture capture("water1.mov"); // 指定したビデオファイルをオープン
    if (capture.isOpened() == 0)
    {
        printf("Camera not found\n");
        return -1;
    }
    // フレームサイズ取得
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    printf("Frame Size = (%d %d)\n", width, height);
    cv::Size imageSize(width, height); // フレームと同じ画像サイズ定義

    // 船画像"ship.jpg"の読み込み
    cv::Mat shipImage = cv::imread("ship.jpg", cv::IMREAD_COLOR);

    // 画像格納用インスタンス準備
    cv::Mat frameImage, presentImage, priorImage, flow;

    // 船の初期位置
    cv::Point2f shipPoint(130, 190);

    // ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Ship");
    cv::moveWindow("Ship", width, 0);

    // 船画像の表示
    cv::imshow("Ship", shipImage);

    // ビデオライタ生成(ファイル名，コーデック(mp4v/mov)，フレームレート，フレームサイズ)
    // cv::VideoWriter rec("rec.mov", cv::VideoWriter::fourcc('m','p','4','v'), 30, cv::Size(width, height));

    // 動画像処理無限ループ
    while (1)
    {
        // カメラから1フレーム読み込み
        capture >> frameImage;
        if (frameImage.data == NULL)
            break;

        cv::Point2f shipMovement(0, 0);

        // オプティカルフロー計算のためにグレースケール変換
        cv::cvtColor(frameImage, presentImage, cv::COLOR_BGR2GRAY);

        if (!priorImage.empty())
        {
            // Farneback法でオプティカルフローを計算
            cv::calcOpticalFlowFarneback(priorImage, presentImage, flow, 0.5, 5, 15, 10, 10, 1.5, 0);

            // オプティカルフローの描画
            for (int y = 0; y < frameImage.rows; y += 10)
            {
                for (int x = 0; x < frameImage.cols; x += 10)
                {
                    const cv::Point2f flowAtXY = flow.at<cv::Point2f>(y, x);
                    cv::line(frameImage, cv::Point(x, y), cv::Point(cvRound(x + flowAtXY.x), cvRound(y + flowAtXY.y)),
                             cv::Scalar(0, 255, 0));
                    cv::circle(frameImage, cv::Point(x, y), 1, cv::Scalar(0, 255, 0), -1);

                    // 3x3のオプティカルフロー周辺の平均を求める
                    float sumX = 0.0f;
                    float sumY = 0.0f;
                    for (int dy = -3; dy <= 3; dy++)
                    {
                        for (int dx = -3; dx <= 3; dx++)
                        {
                            if (y + dy >= 0 && y + dy < frameImage.rows && x + dx >= 0 && x + dx < frameImage.cols)
                            {
                                const cv::Point2f flowNeighbor = flow.at<cv::Point2f>(y + dy, x + dx);
                                sumX += flowNeighbor.x;
                                sumY += flowNeighbor.y;
                            }
                        }
                    }
                    const cv::Point2f avgFlow = cv::Point2f(sumX / 36.0f, sumY / 36.0f);

                    // 船の移動量を更新
                    shipMovement = avgFlow;
                }
            }
        }

        // 船の位置を更新
        shipPoint += shipMovement;


        // 現在のフレームを保存して次のフレームに備える
        presentImage.copyTo(priorImage);

        // 船の位置に円を表示
        cv::circle(frameImage, shipPoint, 5, cv::Scalar(0, 255, 0), -1, 8);

        // ウィンドウに画像表示
        cv::imshow("Frame", frameImage);

        // キー入力待ち
        char key = cv::waitKey(20); // 20ミリ秒待機
        if (key == 'q')
            break;

        // 動画ファイル書き出し
        // rec << frameImage;  //ビデオライタに画像出力
    }

    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
