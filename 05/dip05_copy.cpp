#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void drawLines(Mat& img, int grid_size) {
    // 画像を縮小して領域内の明度の平均を計算
    Mat small_img;
    resize(img, small_img, Size(img.cols / grid_size, img.rows / grid_size));

    // 各領域に線を描画
    for (int y = 0; y < small_img.rows; y++) {
        for (int x = 0; x < small_img.cols; x++) {
            int brightness = small_img.at<uchar>(y, x);

            // 明度に応じて線の本数を決定
            int num_lines = 0;
            if (brightness < 85) {
                num_lines = 3; // 非常に暗い
            } else if (brightness < 170) {
                num_lines = 2; // 普通に暗い
            } else if (brightness < 255) {
                num_lines = 1; // やや暗い
            }

            // 線を描画する領域の位置
            int start_x = x * grid_size;
            int start_y = y * grid_size;

            for (int i = 0; i < num_lines; i++) {
                // 斜め線を描画
                Point pt1(start_x, start_y + i * (grid_size / 3));
                Point pt2(start_x + (grid_size - 1), start_y + (i + 1) * (grid_size / 3) - 1);
                line(img, pt1, pt2, Scalar(0, 0, 0), 1);
            }
        }
    }
}

int main() {
    VideoCapture cap("scene.mov");
    if (!cap.isOpened()) {
        cerr << "Error opening video file" << endl;
        return -1;
    }

    int grid_size = 4;
    Mat frame;
    while (cap.read(frame)) {
        // グレースケールに変換
        cvtColor(frame, frame, COLOR_BGR2GRAY);

        // フレームに線描を適用
        drawLines(frame, grid_size);

        // 結果を表示
        imshow("Frame", frame);

        // 'q'キーが押されたら終了
        if (waitKey(30) == 'q') {
            break;
        }
    }

    cap.release();
    destroyAllWindows();

    return 0;
}
