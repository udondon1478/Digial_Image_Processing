/*
 g++ -std=c++11 issue2.cpp `pkg-config --cflags --libs opencv4`
 */
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

// 肌色判定
bool isSkinColor(const cv::Vec3b& hsv) {
    return (hsv[0] > 0 && hsv[0] < 30 && hsv[1] > 60 && hsv[2] > 80); 
}

// 美肌加工
void smoothSkin(cv::Mat& faceImage, cv::Mat& hsvImage, cv::Rect rect, int blurAmount = 15) {
    cv::Mat blurredImage;
    cv::GaussianBlur(faceImage(rect), blurredImage, cv::Size(blurAmount, blurAmount), 0);

    // 肌色領域だけを合成
    for (int y = rect.y; y < rect.y + rect.height; y++) {
        for (int x = rect.x; x < rect.x + rect.width; x++) {
            if (isSkinColor(hsvImage.at<cv::Vec3b>(y, x))) {
                faceImage.at<cv::Vec3b>(y, x) = blurredImage.at<cv::Vec3b>(y - rect.y, x - rect.x);
            }
        }
    }
}

// 酔っ払い加工
void drunkFilter(cv::Mat& faceImage, cv::Mat& hsvImage, cv::Rect rect) {
    for(int y = rect.y; y < rect.y + rect.height; y++) {
        for(int x = rect.x; x < rect.x + rect.width; x++) {
            cv::Vec3b& hsv = hsvImage.at<cv::Vec3b>(y, x);
            if (isSkinColor(hsv)) {
                // 彩度を少し上げて赤らめる
                hsv[1] = std::min(255, hsv[1] + 30); 
            }
        }
    }
    cv::cvtColor(hsvImage, faceImage, cv::COLOR_HSV2BGR);
}

// 日焼け加工
void tanFilter(cv::Mat& faceImage, cv::Mat& hsvImage, cv::Rect rect) {
    for(int y = rect.y; y < rect.y + rect.height; y++) {
        for(int x = rect.x; x < rect.x + rect.width; x++) {
            cv::Vec3b& hsv = hsvImage.at<cv::Vec3b>(y, x);
            if (isSkinColor(hsv)) {
                // 彩度を下げて、明度を大幅に下げて黒ずませる
                hsv[1] = std::max(0, hsv[1] - 60); // 彩度を下げる
                hsv[2] = std::max(0, hsv[2] - 50); // 明度を大きく下げる
            }
        }
    }
    cv::cvtColor(hsvImage, faceImage, cv::COLOR_HSV2BGR);
}

// ネガポジ反転
void negativePositive(cv::Mat& faceImage, cv::Mat& hsvImage, cv::Rect rect) {
    for(int y = rect.y; y < rect.y + rect.height; y++) {
        for(int x = rect.x; x < rect.x + rect.width; x++) {
            if (isSkinColor(hsvImage.at<cv::Vec3b>(y, x))) {
                cv::Vec3b& bgr = faceImage.at<cv::Vec3b>(y, x);
                bgr[0] = 255 - bgr[0];
                bgr[1] = 255 - bgr[1];
                bgr[2] = 255 - bgr[2];
            }
        }
    }
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

    int filterMode = 0; // 0: 加工なし, 1: 美肌, 2: 酔っ払い, 3: 日焼け, 4: ネガポジ
    
    while(1){
        //ビデオキャプチャから1フレーム画像取得
        capture >> originalImage;
        cv::resize(originalImage, frameImage, imageSize);
        frameImage.copyTo(hsvImage); // HSVに変換する前にコピー

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
            
            cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV); // HSV変換
            
            // ⑤画像の加工
            switch (filterMode) {
                case 1: smoothSkin(frameImage, hsvImage, face2); break;
                case 2: drunkFilter(frameImage, hsvImage, face2); break;
                case 3: tanFilter(frameImage, hsvImage, face2); break;
                case 4: negativePositive(frameImage, hsvImage, face2); break;
                default: break; 
            }

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
        } else if (key == '1') {
            filterMode = 1; // 美肌
        } else if (key == '2') {
            filterMode = 2; // 酔っ払い
        } else if (key == '3') {
            filterMode = 3; // 日焼け
        } else if (key == '4') {
            filterMode = 4; // ネガポジ
        } else if (key == '0') {
            filterMode = 0; // 加工なし
        }
    }
    
    return 0;
}