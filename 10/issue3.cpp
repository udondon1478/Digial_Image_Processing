#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

// 配列の象限入れ替え用関数の宣言
void ShiftDFT(const cv::Mat& src_arr, cv::Mat& dst_arr);

double gaussian(double x, double y, double sigma);

void applyFilter(cv::Mat& ftMatrix, double sigma, bool isLowPass);

int main(int argc, const char* argv[])
{
    // 原画像のグレースケール画像を"sourceImage"に格納
    cv::Mat sourceImage = cv::imread("lenna_g.jpg", cv::IMREAD_GRAYSCALE);
    if (sourceImage.empty()) {
        std::cerr << "Source not found\n";
        return -1;
    }

    // 作業用配列領域、描画用画像領域の宣言
    cv::Mat cxMatrix(sourceImage.size(), CV_64FC2); // 複素数用(実数 2 チャンネル)
    cv::Mat ftMatrix(sourceImage.size(), CV_64FC2); // 複素数用(実数 2 チャンネル)
    cv::Mat spcMatrix(sourceImage.size(), CV_64FC1); // スペクトルデータ(実数)
    cv::Mat spcImage(sourceImage.size(), CV_8UC1); // スペクトル画像(自然数)
    cv::Mat resultImage(sourceImage.size(), CV_8UC1); // 逆変換画像(自然数)

    // 原画像を複素数(実数部と虚数部)の 2 チャンネル配列(画像)として表現．虚数部はゼロ
    cv::Mat RealImaginary[] = {cv::Mat_<double>(sourceImage), cv::Mat::zeros(sourceImage.size(), CV_64FC1)};
    cv::merge(RealImaginary, 2, cxMatrix);

    // フーリエ変換
    cv::dft(cxMatrix, ftMatrix);
    ShiftDFT(ftMatrix, ftMatrix);

    // ローパスフィルタの適用
    cv::Mat lowPassMatrix = ftMatrix.clone();
    applyFilter(lowPassMatrix, 30.0, true);

    // ハイパスフィルタの適用
    cv::Mat highPassMatrix = ftMatrix.clone();
    applyFilter(highPassMatrix, 30.0, false);

    // フィルタ適用後のスペクトル画像の計算と表示
    auto processSpectrumImage = [](const cv::Mat& matrix, cv::Mat& spcImage) {
        cv::Mat RealImaginary[2];
        cv::split(matrix, RealImaginary);
        cv::magnitude(RealImaginary[0], RealImaginary[1], spcImage);
        spcImage += cv::Scalar::all(1);
        cv::log(spcImage, spcImage);
        cv::normalize(spcImage, spcImage, 0, 1, cv::NORM_MINMAX);
    };

    processSpectrumImage(lowPassMatrix, spcImage);
    cv::imshow("Low Pass Filter Spectrum", spcImage);
    cv::imwrite("low_pass_spectrum.png", spcImage * 255);

    processSpectrumImage(highPassMatrix, spcImage);
    cv::imshow("High Pass Filter Spectrum", spcImage);
    cv::imwrite("high_pass_spectrum.png", spcImage * 255);

    // 逆フーリエ変換と画像の復元
    auto inverseDFT = [](cv::Mat& ftMatrix, cv::Mat& resultImage) {
        cv::Mat cxMatrix;
        ShiftDFT(ftMatrix, ftMatrix);
        cv::idft(ftMatrix, cxMatrix);
        cv::Mat RealImaginary[2];
        cv::split(cxMatrix, RealImaginary);
        cv::normalize(RealImaginary[0], resultImage, 0, 1, cv::NORM_MINMAX);
        resultImage.convertTo(resultImage, CV_8U, 255);
    };



    inverseDFT(lowPassMatrix, resultImage);
    cv::imshow("Low Pass Filter Result", resultImage);
    cv::imwrite("low_pass_result.png", resultImage);

    inverseDFT(highPassMatrix, resultImage);

    //resultImageを2値化
    cv::threshold(resultImage, resultImage, 120, 255, cv::THRESH_BINARY);
    cv::imshow("High Pass Filter Result", resultImage);
    cv::imwrite("high_pass_result.png", resultImage);

    // オリジナル画像の保存
    cv::imwrite("original.png", sourceImage);

    cv::waitKey(0);
    return 0;
}

void ShiftDFT(const cv::Mat& src_arr, cv::Mat& dst_arr)
{
    src_arr.copyTo(dst_arr);
    int cx = dst_arr.cols / 2;
    int cy = dst_arr.rows / 2;

    cv::Mat q1(dst_arr, cv::Rect(cx, 0, cx, cy));
    cv::Mat q2(dst_arr, cv::Rect(0, 0, cx, cy));
    cv::Mat q3(dst_arr, cv::Rect(0, cy, cx, cy));
    cv::Mat q4(dst_arr, cv::Rect(cx, cy, cx, cy));

    cv::Mat tmp;
    q1.copyTo(tmp);
    q3.copyTo(q1);
    tmp.copyTo(q3);

    q2.copyTo(tmp);
    q4.copyTo(q2);
    tmp.copyTo(q4);
}

double gaussian(double x, double y, double sigma)
{
    return exp(-0.5 * (x * x + y * y) / (sigma * sigma));
}

void applyFilter(cv::Mat& ftMatrix, double sigma, bool isLowPass)
{
    int centerX = ftMatrix.cols / 2;
    int centerY = ftMatrix.rows / 2;
    for (int y = 0; y < ftMatrix.rows; y++) {
        for (int x = 0; x < ftMatrix.cols; x++) {
            double dx = x - centerX;
            double dy = y - centerY;
            double gaussianValue = gaussian(dx, dy, sigma);
            if (!isLowPass) {
                gaussianValue = 1.0 - gaussianValue;
            }
            ftMatrix.at<cv::Vec2d>(y, x)[0] *= gaussianValue;
            ftMatrix.at<cv::Vec2d>(y, x)[1] *= gaussianValue;
        }
    }
}
