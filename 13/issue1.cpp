//(OpenCV4) g++ dip13a.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip13a.cpp `pkg-config --cflags --libs opencv`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, char *argv[]) {
    //ビデオキャプチャの初期化
    cv::VideoCapture capture("pantora.mp4"); //ビデオファイルをオープン
    // cv::VideoCapture capture("senro.mov");  //ビデオファイルをオープン
    if(capture.isOpened() == 0) { //オープンに失敗した場合
        printf("Capture not found\n");
        return -1;
    }

    //画像格納用インスタンス準備
    cv::Mat frameImage; //ビデオキャプチャ用
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size imageSize(width, height);               //ビデオ画像サイズ
    printf("imageSize = (%d, %d)\n", width, height); //ビデオ画像サイズ表示

    //画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    
    cv::VideoWriter rec("rec.mpg", cv::VideoWriter::fourcc('P','I','M','1'), 30, cv::Size(width,height));

    //動画処理用無限ループ
    int count = 0;
    while(1) {
        count++;
        //ビデオキャプチャから1フレーム"frameImage"に取り込み
        capture >> frameImage;
        //ビデオが終了したら無限ループから脱出
        if(frameImage.data == NULL) {
            break;
        }

        cv::Mat grayImage, binImage, contourImage;
        //"sourceImage"をグレースケール画像に変換して"grayImage"に出力
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);
        //"grayImage"を2値化して"grayImage"に出力
        cv::threshold(grayImage, binImage, 188, 255, cv::THRESH_BINARY);
        //"sourceImage"のコピーを"contourImage"に出力
        contourImage = frameImage.clone();
        
        for(int row = 0; row <= binImage.rows; row++) {
            for(int col = 0; col < 100; col++) {
                binImage.at<unsigned char>(row, col) = 255;
            }
        }

        //④輪郭点格納用配列、輪郭の階層用配列の確保
        std::vector<std::vector<cv::Point>> contours;

        cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
        cv::erode(binImage, binImage, element, cv::Point(-1, -1), 5);
        cv::dilate(binImage, binImage, element, cv::Point(-1, -1), 10);
        cv::erode(binImage, binImage, element, cv::Point(-1, -1), 5);

        //⑥ウィンドウを生成して各画像を表示
        cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
        bool donut = false;
        bool cake = false;
        bool flags[5]= {false, false, false, false, false};
        for(int i = 0; i < contours.size(); i++) {
            auto size = cv::contourArea(contours[i]);
            if(size >= 5000 && size <= 15000) {
                double cc = 4 * M_PI * cv::contourArea(contours[i]) / (cv::arcLength(contours[i], true) * cv::arcLength(contours[i], true));
                cv::Scalar color;
                if(cc >= 0.84) {
                    if(!donut) {
                        if(count >= 1800) {
                            color = cv::Scalar(255, 0, 0);
                        } else {
                            color = cv::Scalar(0, 0, 255);
                        }
                        
                        flags[0] = true;
                    } else {
                        if(count >= 1800) {
                            color = cv::Scalar(0, 0, 255);
                        } else {
                            color = cv::Scalar(255, 0, 0);
                        }
                        
                        flags[1] = true;
                    }
                    donut = true;
                } else {
                    if(cc <= 0.7) {
                        color = cv::Scalar(255, 255, 0);
                        flags[4] = true;
                    } else if(!cake){
                        color = cv::Scalar(255, 0, 255);
                        cake = true;
                        flags[3] = true;
                    } else {
                        color = cv::Scalar(0, 255, 0);
                        flags[2] = true;
                    }
                }
                cv::drawContours(contourImage, contours, i, color, 2, 8);
                std::cout << "i: " << i << ", size: " << size << ", cc: " << cc << std::endl;
                std::cout << count << std::endl;
            }
        }
        
        cv::Scalar colors[] = {cv::Scalar(0, 0, 255), cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), cv::Scalar(255, 0, 255), cv::Scalar(255, 255, 0)};
        for(int i = 0; i < 5; i++) {
            if(flags[i]) {
                cv::circle(contourImage, cv::Point(20 + i * 20, 20), 10, colors[i], -1);
            }
        }

        //画像表示
        cv::imshow("Frame", contourImage);

        //キー入力待ち
        int key = cv::waitKey(33);
        //[Q]が押されたら無限ループ脱出
        if(key == 'q')
            break;
        rec << contourImage;  //ビデオライタに画像出力
    }

    //終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
