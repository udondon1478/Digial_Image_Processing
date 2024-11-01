/*
 g++ -std=c++11 dip12.cpp `pkg-config --cflags --libs opencv4`
 */
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

// 顔色の変更
void change_face_color(cv::Mat& faceImage, cv::Mat& hsvImage, cv::Rect rect)
{
    // 色解析しやすいようにHSV色空間に変換
    cv::cvtColor(faceImage, hsvImage, cv::COLOR_BGR2HSV);

    //肌色領域のみ変換
    for(int y=rect.y; y<rect.y+rect.height; y++){
        for(int x=rect.x; x<rect.x+rect.width; x++){
            cv::Vec3b hsv = hsvImage.at<cv::Vec3b>(y, x);
            if(hsv[0] > 0 && hsv[0] < 30 && hsv[1] > 30 && hsv[2] > 60){
                hsv[0] = 120;  // 色相を緑に変更
                hsvImage.at<cv::Vec3b>(y, x) = hsv;
            }
        }
    }
    cv::cvtColor(hsvImage, faceImage, cv::COLOR_HSV2BGR);
}

//main関数
int main(int argc, char* argv[])
{
    //OpenCV初期設定処理
    //カメラキャプチャの初期化
    cv::VideoCapture capture = cv::VideoCapture(1);
    if (capture.isOpened()==0) {
        //カメラが見つからないときはメッセージを表示して終了
        printf("Camera not found\n");
        exit(1);
    }

    cv::Mat originalImage, frameImage, hsvImage, tempImage;
    cv::Size imageSize(720, 405);  // 画像サイズ
    cv::CascadeClassifier faceClassifier;  // 顔認識用分類器

    //3チャンネル画像"hsvImage"と"tempImage"の確保（ビデオと同サイズ）
    hsvImage = cv::Mat(imageSize, CV_8UC3);
    tempImage = cv::Mat(imageSize, CV_8UC3);

    //OpenCVウィンドウ生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Face");
    cv::moveWindow("Face", imageSize.width, 0);

    // ①正面顔検出器の読み込み
    faceClassifier.load("haarcascades/haarcascade_frontalface_default.xml");
    
    while(1){
        //ビデオキャプチャから1フレーム画像取得
        capture >> originalImage;
        cv::resize(originalImage, frameImage, imageSize);

        //フレーム画像表示
        cv::imshow("Frame", frameImage);

        // ②検出情報を受け取るための配列を用意する
        std::vector<cv::Rect> faces;

        // ③画像中から検出対象の情報を取得する
        faceClassifier.detectMultiScale(frameImage, faces, 1.1, 3, 0, cv::Size(20,0));

        // ④顔領域の検出
        for (int i = 0; i < faces.size(); i++) {
            // 検出情報から顔の位置情報を取得
            cv::Rect face = faces[i];
            // 大きさによるチェック。
            if(face.width*face.height < 100*100){
                continue; // 小さい矩形は採用しない
            }
                        //faceのサイズを1.5倍に拡大
            cv::Rect face2(face.x-face.width/4, face.y-face.height/4, face.width*1.5, face.height*1.3);
            
            // ⑤画像の加工
            change_face_color(frameImage, hsvImage, face2);
            

            // 顔の位置情報に基づき、矩形描画を行う
            cv::rectangle(frameImage,
                              cv::Point(face2.x, face2.y),
                              cv::Point(face2.x + face2.width, face2.y + face2.height),
                              CV_RGB(255, 0, 0),
                              3, cv::LINE_AA);
            // 取得した顔の位置情報に基づき、矩形描画を行う
            }
        
        //認識結果画像表示
        cv::imshow("Face", frameImage);
            
        char key = cv::waitKey(10);
        if(key == 'q'){
            break;
        }
    }
    
    return 0;
}
