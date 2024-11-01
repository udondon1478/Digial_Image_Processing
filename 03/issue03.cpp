// g++ dip03.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{

    //動画の読み込み
    cv::VideoCapture capture("dance.mov"); // 動画ファイルをオープン
    cv::VideoCapture capture2("landscape.mov");  // 動画ファイルをオープン

    // ②画像格納用インスタンス準備
    int width = 640, height = 360;                                  // 処理画像サイズ
    cv::Mat captureImage;                                           // キャプチャ用
    cv::Mat captureImage2;                                           // キャプチャ用
    cv::Mat frameImage = cv::Mat(cv::Size(width, height), CV_8UC3); // 処理用
    cv::Mat frameImage2 = cv::Mat(cv::Size(width, height), CV_8UC3); // 処理用
    cv::Mat hsvImage;                                              // 処理用
    cv::Mat recImage = cv::Mat(cv::Size(width, height), CV_8UC3);   // 動画用
    cv::Mat binImage = cv::Mat(cv::Size(width, height), CV_8UC1);   // 処理用

    // ③ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 0, height);

    //(X)ビデオライタ生成
    cv::VideoWriter rec("rec.mpg", cv::VideoWriter::fourcc('P', 'I', 'M', '1'), 30, recImage.size());

    // 動画像処理無限ループ
    while (1)
    {
        // ④カメラから1フレーム読み込んでcaptureImageに格納（CV_8UC3）
        capture >> captureImage;
        if (captureImage.data == 0)
            break; // キャプチャ画像が空の場合は終了

        capture2 >> captureImage2;
        if (captureImage.data == 0)
            break; // キャプチャ画像が空の場合は終了
        // ⑤captureImageをframeImageに合わせてサイズ変換して格納
        cv::resize(captureImage, frameImage, frameImage.size());
        cv::resize(captureImage2, frameImage2, frameImage2.size());
        // ⑥画像処理
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);
        cv::cvtColor(frameImage2,frameImage2,cv::COLOR_BGR2HSV);
        //指定画素値以下を黒にする
        cv::Vec3b s; //画素値を格納する変数
        cv::Vec3b s2; //画素値を格納する変数
        for (int y = 0; y < frameImage.rows; y++)
        {
            for (int x = 0; x < frameImage.cols; x++)
            {
                s = hsvImage.at<cv::Vec3b>(y, x); //"hsvImage"の画素(x,y)の画素値を読み込んで"s"に格納
                s2 = frameImage2.at<cv::Vec3b>(y, x); //"hsvImage"の画素(x,y)の画素値を読み込んで"s2"に格納
                cv::Vec3i s1;
                int S_MAX = 163, S_MIN = 0;


                //S_MAXからS_MIN以外のHの値を取る画素は黒くする
                if (s[1] > S_MAX || s[1] < S_MIN)
                {
                    s1[0] = s2[0];
                    s1[1] = s2[1];
                    s1[2] = s2[2];
                }
                else
                {
                    s1[0] = s[0];
                    s1[1] = s[1];
                    s1[2] = s[2];
                }


                s = s1; //int型をunsigned char型に丸める
                hsvImage.at<cv::Vec3b>(y, x) = s; //"s"の値を"hsvImage"の画素(x,y)に書き込む
            }
        }

        //HSVからBGRに変換
        cv::cvtColor(hsvImage, hsvImage, cv::COLOR_HSV2BGR);

        // ⑦ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", hsvImage);

        //(Y)カラーの動画ファイル書き出し
        rec << hsvImage;                                       // ビデオライタに画像出力

        // ⑧キー入力待ち
        char key = cv::waitKey(1);
        if (key == 'q')
            break;
    }

    return 0;
}
