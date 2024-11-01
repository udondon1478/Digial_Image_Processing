//g++ dip02.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char* argv[]) {
    
    //①画像ファイルの読み込み
    cv::Mat sourceImage = cv::imread("color2.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data==0) {  //画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);

    //HSV変換
    cv::Mat hsvImage;
    cv::cvtColor(sourceImage, hsvImage, cv::COLOR_BGR2HSV);

    
    //②画像格納用オブジェクト"resultImage"の生成
    cv::Mat resultImage = cv::Mat(sourceImage.size(), CV_8UC3);
    //③ウィンドウの生成と移動
    cv::namedWindow("Source");
    cv::moveWindow("Source", 0,0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 400,0);
    
    //④画像の画素単位の読み込み・処理・書き込み
    cv::Vec3b s; //画素値を格納する変数


    for (int y=0; y<sourceImage.rows; y++) {
        for (int x=0; x<sourceImage.cols; x++) {
            s = hsvImage.at<cv::Vec3b>(y,x); //"hsvImage"の画素(x,y)の画素値を読み込んで"s"に格納
            cv::Vec3i s1;
            int G_MAX = 88, G_MIN = 70;

            //G_MAXからG_MIN以外のHの値を取る画素は黒くする
            if (s[0] > G_MAX || s[0] < G_MIN) {
                s1[0] = 0;
                s1[1] = 0;
                s1[2] = 0;
            } else {
                s1[0] = s[0];
                s1[1] = s[1];
                s1[2] = s[2];
            }


            s = s1; //int型をunsigned char型に丸める
            resultImage.at<cv::Vec3b>(y,x) = s; //"s"の値を"resultImage"の画素(x,y)に書き込む
        }
    }

    //HSVからBGRに変換
    cv::cvtColor(resultImage, resultImage, cv::COLOR_HSV2BGR);

/*より詳細な塗りつぶし、時間があれば閾値調整
for (int y=0; y<sourceImage.rows; y++) {
        for (int x=0; x<sourceImage.cols; x++) {
            s = resultImage.at<cv::Vec3b>(y,x); //"hsvImage"の画素(x,y)の画素値を読み込んで"s"に格納
            cv::Vec3i s1;
            int G_MAX = 230, G_MIN = 25;

            //G_MAXからG_MIN以外のGreenの値を取る画素は黒くする
            if (s[1] > G_MAX || s[1] < G_MIN) {
                s1[0] = 0;
                s1[1] = 0;
                s1[2] = 0;
            } else {
                s1[0] = s[0];
                s1[1] = s[1];
                s1[2] = s[2];
            }

            s = s1; //int型をunsigned char型に丸める
            resultImage.at<cv::Vec3b>(y,x) = s; //"s"の値を"resultImage"の画素(x,y)に書き込む
        }
    }
*/

    //⑤ウィンドウへの画像の表示
    cv::imshow("Source", sourceImage);
    cv::imshow("Result", resultImage);
    
    //⑥キー入力待ち
    cv::waitKey(0);
    
    //⑦画像の保存
    cv::imwrite("result.jpg", resultImage);
    
    return 0;
}
