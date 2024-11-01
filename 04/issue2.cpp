// g++ issue1.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

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
    cv::Mat hsvImage(cv::Size(width, height), CV_8UC3);            // 1チャンネル
    cv::Mat resultImage(cv::Size(width, height), CV_8UC3);         // 1チャンネル
    cv::Mat recImage(cv::Size(width, height), CV_8UC3);             // 3チャンネル

    // ③ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("HSV");
    cv::moveWindow("HSV", width, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, height);

    // ④ルックアップテーブルの作成
    //HSVの各チャンネルに応じたルックアップテーブル
    unsigned char lookupTableH[256];
    //8段階に分ける
    for (int i = 0; i < 256; i++)
    {
        lookupTableH[i] = (i / 32) * 32;
    }
    unsigned char lookupTableS[256];
    //8段階に分ける
    for (int i = 0; i < 256; i++)
    {
        lookupTableS[i] = (i / 32) * 32;
    }
    unsigned char lookupTableV[256];
    //8段階に分ける
    for (int i = 0; i < 256; i++)
    {
        lookupTableV[i] = (i / 32) * 32;
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

        //(c)"frameImage"をグレースケールに変換して"hsvImage"に格納
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);

        //(d)"hsvImage"の各画像を走査して，ルックアップテーブルに基づいて画素値変換して"resultImage"に格納
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {
                /*
                // 座標(i,j)の画素値"s"取得
                unsigned char s = hsvImage.at<unsigned char>(j, i);
                // ルックアップテーブルで画素値"s"を変換
                s = lookupTable[s];
                // 変換後の画素値"s"を"resultImage"の座標(i,j)に格納
                resultImage.at<unsigned char>(j, i) = s;
                */
                
                //上記の処理をHSV画像に適応するために変更
                cv::Vec3b s = hsvImage.at<cv::Vec3b>(j, i);

                s[0] = lookupTableH[s[0]];
                s[1] = lookupTableS[s[1]];
                s[2] = lookupTableV[s[2]];

                resultImage.at<cv::Vec3b>(j, i) = s;
            }
        }
        //テキストを出力
        printf("成功\n");
        //RGBに変換
        cv::cvtColor(resultImage, resultImage, cv::COLOR_HSV2BGR);
        //(e)ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("HSV", hsvImage);
        cv::imshow("Result", resultImage);

        //(f)動画ファイル書き出し
        rec << resultImage;                                          // ビデオライタに画像出力

        //(g)キー入力待ち
        char key = cv::waitKey(20); // 20ミリ秒待機
        if (key == 'q')
            break;
    }

    return 0;
}
