//g++ dip02.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char* argv[]) {
    
    //①画像ファイルの読み込み
    cv::Mat sourceImage = cv::imread("color1.jpg", cv::IMREAD_COLOR);
    cv::Mat sourceImage2 = cv::imread("color2.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data==0) {  //画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);
    
    //②画像格納用オブジェクト"resultImage"の生成
    cv::Mat resultImage = cv::Mat(sourceImage.size(), CV_8UC3);
    //③ウィンドウの生成と移動
    cv::namedWindow("Source");
    cv::moveWindow("Source", 0,0);
    cv::namedWindow("Source2");
    cv::moveWindow("Source2", 0,0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 400,0);
    
    //④画像の画素単位の読み込み・処理・書き込み
    cv::Vec3b s; //画素値を格納する変数
    cv::Vec3b s2; //画素値を格納する変数



    for (int y=0; y<sourceImage.rows; y++) {
        for (int x=0; x<sourceImage.cols; x++) {
            s = sourceImage.at<cv::Vec3b>(y,x); //"sourceImage"の画素(x,y)の画素値を読み込んで"s"に格納
            cv::Vec3i s1;
            s2 = sourceImage2.at<cv::Vec3b>(y,x); //"sourceImage"の画素(x,y)の画素値を読み込んで"s"に格納

            int count = 0;

            //16(4*4)分割
            int xx= x/(sourceImage.cols/4);
            int yy= y/(sourceImage.rows/4);
            if(((xx%2)^(yy%2)) == 0){
                //何もしない
            }else{
                s[0]=s2[0];
                s[1]=s2[1];
                s[2]=s2[2];
            }

    
            resultImage.at<cv::Vec3b>(y,x) = s; //"s"の値を"resultImage"の画素(x,y)に書き込む
        }
    }
    //⑤ウィンドウへの画像の表示
    cv::imshow("Source", sourceImage);
    cv::imshow("Result", resultImage);
    
    //⑥キー入力待ち
    cv::waitKey(0);
    
    //⑦画像の保存
    cv::imwrite("result.jpg", resultImage);
    
    return 0;
}
