// g++ dip10.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

// 配列の象限入れ替え用関数の宣言
void ShiftDFT(const cv::Mat &src_arr, cv::Mat &dst_arr);

int main(int argc, const char *argv[])
{
    // ①カメラのフレームイメージのグレースケール画像を"sourceImg"に格納

    // カメラをオープン
    cv::VideoCapture capture(0);
    if (!capture.isOpened())
    {
        std::cerr << "Camera not found" << std::endl;
        return -1;
    }

    cv::Mat sourceImg;

    // カメラから映像を取り込む
    cv::Mat frame;

    // 作業用配列領域、描画用画像領域の宣言
    cv::Mat cxMatrix(sourceImg.size(), CV_64FC2);  // 複素数用(実数 2 チャンネル)
    cv::Mat ftMatrix(sourceImg.size(), CV_64FC2);  // 複素数用(実数 2 チャンネル)
    cv::Mat spcMatrix(sourceImg.size(), CV_64FC1); // スペルトルデータ(実数)
    cv::Mat spcImg(sourceImg.size(), CV_8UC1);     // スペクトル画像(自然数)
    cv::Mat resultImg(sourceImg.size(), CV_8UC1);  // 逆変換画像(自然数)

    // ウィンドウ生成
    // 原画像
    cv::namedWindow("sourceImg");
    cv::moveWindow("sourceImg", 0, 0);
    // フーリエスペクトル画像
    cv::namedWindow("spcImg");
    cv::moveWindow("spcImg", sourceImg.cols, 0);
    // 逆変換画像
    cv::namedWindow("Result");
    cv::moveWindow("Result", sourceImg.cols * 2, 0);

    cv::Mat recImage(sourceImg.size(), CV_8UC1);
    cv::VideoWriter writer("output.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, resultImg.size());

    while (1)
    {
        // カメラから映像を取り込む
        capture >> frame;
        if (frame.empty())
        {
            std::cerr << "Frame not found" << std::endl;
            return -1;
        }
        // カメラから取り込んだ映像をグレースケール画像に変換
        cv::cvtColor(frame, sourceImg, cv::COLOR_BGR2GRAY);

        // ②原画像を複素数(実数部と虚数部)の 2 チャンネル配列(画像)として表現．虚数部はゼロ
        cv::Mat imgMatrix[] = {cv::Mat_<double>(sourceImg), cv::Mat::zeros(sourceImg.size(), CV_64FC1)};
        // 実数部と虚数部を一組にした 2 チャンネル画像 cxMatrix を生成
        cv::merge(imgMatrix, 2, cxMatrix);

        // ③フーリエ変換
        // フーリエ変換の実施（cxMatrix → ftMatrix）
        cv::dft(cxMatrix, ftMatrix);
        // 配列の象限入れ替え（低周波成分が画像中央部、高周波成分が画像周辺部
        ShiftDFT(ftMatrix, ftMatrix);

        // 四角領域でマスク処理
        for (int y = 0; y < ftMatrix.rows; y++)
        {
            for (int x = 0; x < ftMatrix.cols; x++)
            {
                /*
                //左半分除去
                if(x < ftMatrix.cols/2){
                    ftMatrix.at<cv::Vec2d>(y, x)[0] = 0;
                    ftMatrix.at<cv::Vec2d>(y, x)[1] = 0;
                }
                //右半分除去
                else{
                    ftMatrix.at<cv::Vec2d>(y, x)[0] = 0;
                    ftMatrix.at<cv::Vec2d>(y, x)[1] = 0;
                }
                */

                // 四角でマスキング
                int band_width = 50;
                // 中心(ftMatrix.cols/2, ftMatrix.rows/2), いっぺんの長さ=band_width
                if ((abs(x - ftMatrix.cols / 2) < band_width && abs(y - ftMatrix.rows / 2) < band_width))
                {
                    ftMatrix.at<cv::Vec2d>(y, x)[0] = 0;
                    ftMatrix.at<cv::Vec2d>(y, x)[1] = 0;
                }
            }
        }

        // ④フーリエスペクトル"spcMatrix"の計算
        // ftMatrix を実数部 imgMatrix[0] と虚数部 imgMatrix[1] に分解
        cv::split(ftMatrix, imgMatrix);
        // フーリエスペクトル各要素を計算して spcMatrix に格納(spc = sqrt(re^2+im^2))
        cv::magnitude(imgMatrix[0], imgMatrix[1], spcMatrix);

        // ⑤フーリエスペクトルからフーリエスペクトル画像を生成
        // 表示用にフーリエスペクトル spcMatrix の各要素の対数をとる(log(1+spc))
        spcMatrix += cv::Scalar::all(1);
        cv::log(spcMatrix, spcMatrix);
        // フーリエスペクトルを 0〜255 にスケーリングしてフーリエスペクトル画像 spcImg にコピー
        cv::normalize(spcMatrix, spcImg, 0, 255, cv::NORM_MINMAX, CV_8U);

        // ⑥フーリエ逆変換
        // 配列の象限入れ替え（低周波成分が画像周辺部、高周波成分が画像中央部）
        ShiftDFT(ftMatrix, ftMatrix);
        // フーリエ逆変換の実施（ftMatrix → cxMatrix）
        // cv::idft(ftMatrix, cxMatrix);
        cv::idft(ftMatrix, cxMatrix);
        // cxMatrix を実数部(imgMatrix[0])と虚数部(imgMatrix[1])に分解
        cv::split(cxMatrix, imgMatrix);
        // 実数部(imgMatrix[0])を 0〜255 にスケーリングして resultImg にコピー
        cv::normalize(imgMatrix[0], resultImg, 0, 255, cv::NORM_MINMAX, CV_8U);

        // resultを2値化
        cv::threshold(resultImg, resultImg, 128, 255, cv::THRESH_BINARY);

        // ⑦各画像を表示
        // 原画像
        cv::imshow("sourceImg", sourceImg);
        // フーリエスペクトル画像
        cv::imshow("spcImg", spcImg);
        // 逆変換画像
        cv::imshow("Result", resultImg);

        // キー入力を取得
        int key = cv::waitKey(1);
        if (key == 'q')
        {
            break;
        }

        // ビデオ保存
        writer << resultImg;
    }

    return 0;
}

// 画像の象限入れ替え用関数
void ShiftDFT(const cv::Mat &src_arr, cv::Mat &dst_arr)
{
    int cx = src_arr.cols / 2;
    int cy = src_arr.rows / 2;

    cv::Mat q1(src_arr, cv::Rect(cx, 0, cx, cy));
    cv::Mat q2(src_arr, cv::Rect(0, 0, cx, cy));
    cv::Mat q3(src_arr, cv::Rect(0, cy, cx, cy));
    cv::Mat q4(src_arr, cv::Rect(cx, cy, cx, cy));

    cv::Mat tmp;
    q1.copyTo(tmp);
    q3.copyTo(q1);
    tmp.copyTo(q3);

    q2.copyTo(tmp);
    q4.copyTo(q2);
    tmp.copyTo(q4);
}
