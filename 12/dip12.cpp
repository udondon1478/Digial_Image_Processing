/*
 g++ -std=c++11 dip12.cpp `pkg-config --cflags --libs opencv4`
 */
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

// 顔に画像をオーバーレイする関数
void overlay_image(cv::Mat& frame, const cv::Mat& overlay, cv::Rect face_rect) {
    // オーバーレイ画像のリサイズ
    cv::Mat resized_overlay;
    cv::resize(overlay, resized_overlay, cv::Size(face_rect.width, face_rect.height));

    // マスクを作成
    cv::Mat mask = cv::Mat::zeros(resized_overlay.size(), CV_8UC1);
    for (int y = 0; y < resized_overlay.rows; ++y) {
        for (int x = 0; x < resized_overlay.cols; ++x) {
            if (resized_overlay.at<cv::Vec3b>(y, x) != cv::Vec3b(255, 255, 255)) {
                mask.at<uchar>(y, x) = 255;
            }
        }
    }

    // 顔領域のROI
    cv::Mat roi = frame(face_rect);

    // マスク処理でオーバーレイ画像を合成
    resized_overlay.copyTo(roi, mask);
}

//main関数
int main(int argc, char* argv[])
{
    //OpenCV初期設定処理
    //カメラキャプチャの初期化
    cv::VideoCapture capture = cv::VideoCapture(0);
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

    // うさ耳画像の読み込み
    cv::Mat usamimi = cv::imread("inumimi.jpeg");
    if (usamimi.empty()) {
        std::cerr << "Failed to load usamimi.jpg" << std::endl;
        return -1;
    }

    // 背景を白に
    for (int y = 0; y < usamimi.rows; ++y) {
        for (int x = 0; x < usamimi.cols; ++x) {
            if (usamimi.at<cv::Vec3b>(y, x) == cv::Vec3b(0, 0, 0)) {
                usamimi.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
            }
        }
    }

    while(1){
        //ビデオキャプチャから1フレーム画像取得
        capture >> originalImage;
        cv::resize(originalImage, frameImage, imageSize);

        // ②検出情報を受け取るための配列を用意する
        std::vector<cv::Rect> faces;

                        // Faceウィンドウに犬耳を合成する前のフレームを表示
        cv::imshow("Frame", frameImage.clone());

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

            // 顔の位置を調整 (うさぎの耳の位置に合わせる)
            cv::Rect adjusted_face(face.x, face.y - face.height / 2, face.width, face.height);

            // 顔に画像をオーバーレイ
            overlay_image(frameImage, usamimi, adjusted_face);
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