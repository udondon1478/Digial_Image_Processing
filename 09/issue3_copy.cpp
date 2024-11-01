#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV 関連ヘッダ
#include <cmath>              //数学関連ヘッダ

// 2つの直線の交点を計算する関数
cv::Point2f computeIntersection(cv::Vec2f line1, cv::Vec2f line2) {
    float rho1 = line1[0], theta1 = line1[1];
    float rho2 = line2[0], theta2 = line2[1];
    float sinTheta1 = sin(theta1), cosTheta1 = cos(theta1);
    float sinTheta2 = sin(theta2), cosTheta2 = cos(theta2);

    float x = (rho2 * sinTheta1 - rho1 * sinTheta2) / (cosTheta2 * sinTheta1 - cosTheta1 * sinTheta2);
    float y = (rho1 * cosTheta2 - rho2 * cosTheta1) / (cosTheta2 * sinTheta1 - cosTheta1 * sinTheta2);

    return cv::Point2f(x, y);
}

// 2つの直線の角度の差が90度に近いかどうかを判断する関数
bool isNear90Degrees(float theta1, float theta2, float tolerance = 3.0f) {
    float angleDifference = fabs(theta1 - theta2);
    return fabs(angleDifference - CV_PI / 2) <= tolerance * CV_PI / 180;
}

// 直線の長さを計算する関数
float lineLength(cv::Point2f p1, cv::Point2f p2) {
    return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

// 2つの点の距離を計算する関数
float pointDistance(cv::Point2f p1, cv::Point2f p2) {
    return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

int main(int argc, char *argv[]) {
    // ①ビデオキャプチャの初期化
    cv::VideoCapture capture("card.mov"); // ビデオファイルをオープン
    if (capture.isOpened() == 0) {
        printf("Capture not found\n");
        return -1;
    }
    // ②画像格納用インスタンス準備
    int w = capture.get(cv::CAP_PROP_FRAME_WIDTH);  // capture から動画横サイズ取得
    int h = capture.get(cv::CAP_PROP_FRAME_HEIGHT); // capture から動画縦サイズ取得
    cv::Size imageSize(w, h);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3); // 3 チャンネル
    cv::Mat grayImage(imageSize, CV_8UC1);  // 1 チャンネル
    cv::Mat edgeImage(imageSize, CV_8UC1);  // 1 チャンネル
    // ③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Edge");
    cv::moveWindow("Edge", 100, 100);
    // ④ハフ変換用変数
    std::vector<cv::Vec2f> lines; // ρ,θ の組で表現される直線群
    // ⑤動画処理用無限ループ
    while (1) {
        //(a)ビデオキャプチャから 1 フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        // ビデオが終了したら無限ループから脱出
        if (originalImage.data == NULL) {
            break;
        }
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);
        //(b)"frameImage"からグレースケール画像"grayImage"を生成
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);
        //(c)"grayImage"からエッジ画像"edgeImage"を生成
        cv::Canny(grayImage, edgeImage, 120, 160, 3); // ケニーのエッジ検出アルゴリズム
        //(d)"edgeImage"に直線検出ハフ変換を施して，しきい値(90)以上の得票数を得た直線群(ρ,θ)を"lines"に格納
        cv::HoughLines(edgeImage, lines, 1, M_PI / 180, 75);

        // 四角形の頂点となる交点を格納するベクトル
        std::vector<cv::Point2f> intersections;

        //(e)ハフ変換結果表示
        // 検出された直線の数("lines.size()")としきい値(200)の小さい方の数だけ繰り返し
        for (int i = 0; i < std::min(static_cast<int>(lines.size()), 200); i++) {
            // 直線パラメータ：(ρ,θ) → 直線数式：a(x-x0)=b(y-y0) → 2 端点"p1"，"p2"を計算
            float rho = lines[i][0]; //"ρ"
            float theta = lines[i][1]; //"θ"
            double a = cos(theta); //"θ"から"a"を計算
            double b = sin(theta); //"θ"から"b"を計算
            double x0 = a * rho; // 直線上の 1 点 p0(x0, y0)の"x0"を計算
            double y0 = b * rho; // 直線上の 1 点 p0(x0, y0)の"y0"を計算
            cv::Point2f p1, p2; // 直線描画用の端点"p1"，"p2"
            p1.x = x0 - 1000 * b; //"p1"の x 座標の計算
            p1.y = y0 + 1000 * a; //"p1"の y 座標の計算
            p2.x = x0 + 1000 * b; //"p2"の x 座標の計算
            p2.y = y0 - 1000 * a; //"p2"の y 座標の計算

            // 直線の長さを計算
            float length = lineLength(p1, p2);
            // 長さが一定範囲内の直線だけを描画
            if (length >= 1) { // ここで長さの範囲を設定

                // 他の直線との距離をチェック
                bool tooClose = false;
                for (int j = 0; j < i; j++) {
                    float rho2 = lines[j][0];
                    float theta2 = lines[j][1];
                    cv::Point2f p3, p4;
                    p3.x = cos(theta2) * rho2 - 1000 * sin(theta2);
                    p3.y = sin(theta2) * rho2 + 1000 * cos(theta2);
                    p4.x = cos(theta2) * rho2 + 1000 * sin(theta2);
                    p4.y = sin(theta2) * rho2 - 1000 * cos(theta2);
                    if (pointDistance(p1, p3) < 10 || pointDistance(p2, p4) < 10 || pointDistance(p1, p4) < 10 || pointDistance(p2, p3) < 10) {
                        tooClose = true;
                        break;
                    }
                }

                if (!tooClose) {

                    // 他の直線との交点を探す
                    for (int j = i + 1; j < std::min(static_cast<int>(lines.size()), 200); j++) {
                        if (isNear90Degrees(lines[i][1], lines[j][1])) {
                            cv::Point2f intersection = computeIntersection(lines[i], lines[j]);
                            if (intersection.x >= 0 && intersection.x < w && intersection.y >= 0 && intersection.y < h) {
                                cv::circle(frameImage, intersection, 5, cv::Scalar(0, 255, 0), -1);
                                intersections.push_back(intersection);
                            }
                        }
                    }
                }
            }
        }

        // 四角形を描画
        if (intersections.size() == 4) {
            for (int i = 0; i < 4; i++) {
                cv::line(frameImage, intersections[i], intersections[(i + 1) % 4], cv::Scalar(255, 0, 0), 2);
            }
            // 対角線を描画
            cv::line(frameImage, intersections[0], intersections[2], cv::Scalar(255, 255, 0), 2);
            cv::line(frameImage, intersections[1], intersections[3], cv::Scalar(255, 255, 0), 2);
        }

        //(f)"frameImage"，"edgeImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Edge", edgeImage);
        //(g)キー入力待ち
        int key = cv::waitKey(33);
        //[Q]が押されたら無限ループ脱出
        if (key == 'q')
            break;
    }
    // ⑥終了処理
    // カメラ終了
    capture.release();
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
