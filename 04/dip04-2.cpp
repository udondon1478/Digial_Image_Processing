// g++ dip04-2.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           // 入出力関連ヘッダ
#include <opencv2/opencv.hpp> // OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{
    int width = 640, height = 480;

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
    cv::Mat hsvImage(cv::Size(width, height), CV_8UC3);             // HSVカラー画像
    cv::Mat resultImage(cv::Size(width, height), CV_8UC3);          // 結果画像
    cv::Mat recImage(cv::Size(width, height), CV_8UC3);             // 録画用

    // ③ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, 0);

    // ④ルックアップテーブルの作成
    unsigned char lookupTableH[256];
    unsigned char lookupTableS[256];
    unsigned char lookupTableV[256];
    for (int i = 0; i < 256; i++)
    {
        // H（色相）のルックアップテーブル：360度を6段階に
        lookupTableH[i] = (i / 43) * 43;
        // S（彩度）のルックアップテーブル：255を6段階に
        if (i < 43) lookupTableS[i] = 0;
        else if (i < 85) lookupTableS[i] = 51;
        else if (i < 128) lookupTableS[i] = 102;
        else if (i < 170) lookupTableS[i] = 153;
        else if (i < 213) lookupTableS[i] = 204;
        else lookupTableS[i] = 255;
        // V（明度）のルックアップテーブル：255を6段階に
        if (i < 43) lookupTableV[i] = 0;
        else if (i < 85) lookupTableV[i] = 51;
        else if (i < 128) lookupTableV[i] = 102;
        else if (i < 170) lookupTableV[i] = 153;
        else if (i < 213) lookupTableV[i] = 204;
        else lookupTableV[i] = 255;
    }

    // ⑤ビデオライタ生成(ファイル名，コーデック，フレームレート，フレームサイズ)
    cv::VideoWriter rec("rec.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, recImage.size());

    // ⑥動画像処理無限ループ
    while (1)
    {
        //(a)カメラから1フレームを"captureImage"に読み込み
        capture >> captureImage;
        if (captureImage.data == NULL)
            break;

        //(b)"captureImage"をリサイズして"frameImage"に格納
        cv::resize(captureImage, frameImage, frameImage.size());

        //(c)"frameImage"をHSVカラーに変換して"hsvImage"に格納
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);

        //(d)"hsvImage"の各画像を走査して，ルックアップテーブルに基づいて画素値変換して"resultImage"に格納
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {
                // 座標(i,j)の画素値取得
                cv::Vec3b pixel = hsvImage.at<cv::Vec3b>(j, i);
                // H, S, V成分の変換
                pixel[0] = lookupTableH[pixel[0]]; // H成分
                pixel[1] = lookupTableS[pixel[1]]; // S成分
                pixel[2] = lookupTableV[pixel[2]]; // V成分
                // 変換後の画素値を"resultImage"の座標(i,j)に格納
                resultImage.at<cv::Vec3b>(j, i) = pixel;
            }
        }

        //(e)HSVからBGRに変換
        cv::cvtColor(resultImage, resultImage, cv::COLOR_HSV2BGR);

        //(f)ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", resultImage);

        //(g)動画ファイル書き出し
        rec << resultImage; // ビデオライタに画像出力

        //(h)キー入力待ち
        char key = cv::waitKey(20); // 20ミリ秒待機
        if (key == 'q')
            break;
    }

    return 0;
}
