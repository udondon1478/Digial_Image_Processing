//(OpenCV4) g++ dip13a.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip13a.cpp `pkg-config --cflags --libs opencv`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main(int argc, char* argv[])
{
    //ビデオキャプチャの初期化
    cv::VideoCapture capture("senro.mov");  //ビデオファイルをオープン
    //cv::VideoCapture capture("senro.mov");  //ビデオファイルをオープン
    if (capture.isOpened()==0) {  //オープンに失敗した場合
        printf("Capture not found\n");
        return -1;
    }
    
    //画像格納用インスタンス準備
    cv::Mat frameImage;  //ビデオキャプチャ用
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size imageSize(width, height);  //ビデオ画像サイズ
    printf("imageSize = (%d, %d)\n", width, height);  //ビデオ画像サイズ表示

    //画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    
    std::vector<std::vector<std::vector<double>>> sum(height, std::vector<std::vector<double>>(width, std::vector<double>{0, 0, 0}));
    
    int frameCount = 0;
    //動画処理用無限ループ
    while (1) {
        frameCount++;
        //ビデオキャプチャから1フレーム"frameImage"に取り込み
        capture >> frameImage;
        //ビデオが終了したら無限ループから脱出
        if (frameImage.data==NULL) {
            break;
        }
        
        for(int i = 0; i < frameImage.rows; i++) {
            for(int j = 0; j < frameImage.cols; j++) {
                cv::Vec3b s = frameImage.at<cv::Vec3b>(i, j);
                for(int k = 0; k < 3; k++) {
                    sum[i][j][k] += (double)s[k];
                }
            }
        }

        //画像表示
        cv::imshow("Frame", frameImage);

        //キー入力待ち
        int key = cv::waitKey(33);
        //[Q]が押されたら無限ループ脱出
        if (key=='q')
            break;
    }
    
    cv::Mat resultImage(imageSize, CV_8UC3);
    for(int row = 0; row < height; row++) {
        for(int col = 0; col < width; col++) {
            cv::Vec3b s;
            for(int k = 0; k < 3; k++) {
                s[k] = sum[row][col][k] / frameCount;
            }

            resultImage.at<cv::Vec3b>(row, col) = s;
        }
    }
    
    cv::imshow("aa", resultImage);
    cv::imwrite("out.png", resultImage);
    cv::waitKey(0);
    
    //終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
