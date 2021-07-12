/**
  @file videocapture_basic.cpp
  @brief A very basic sample for using VideoCapture and VideoWriter
  @author PkLab.net
  @date Aug 24, 2016
*/

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

int main(int, char**)
{
    // Mat : 이미지를 저장할 수 있는 배열
    Mat frame;

    // 라즈베리파이에서 카메라를 열 수 있게 해줌
    // open 함수의 fd와 같은 역할
    //--- INITIALIZE VIDEOCAPTURE
    VideoCapture cap;

    // open the default camera using default API
    // cap.open(0);
    // OR advance usage: select any API backend
    int deviceID = 0;             // 0 = open default camera
    // int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    int apiID = cv::CAP_V4L2;
    
    // 0번부터 카메라 open
    // open selected camera using selected API
    cap.open(deviceID, apiID);

    // check if we succeeded
    if (!cap.isOpened()) {
        perror("ERROR! Unable to open camera\n");
        return -1;
    }

    //--- GRAB AND WRITE LOOP
    printf("Start grabbing\n");
    printf("Press any key to terminate\n");

    while(1)
    {
        // open 이후 읽어올 매트릭스
        // 읽은 이후 매트릭스 구조체에 저장
        // wait for a new frame from camera and store it into 'frame'
        cap.read(frame);

        // 프레임이 비어있으면 empty
        // check if we succeeded
        if (frame.empty()) {
            perror("ERROR! blank frame grabbed\n");
            break;
        }

        // 프레임이 정상적으로 읽힌 경우
        // Live라는 이름의 창 생성 -> 그 창에 읽어온 frame 이미지 출력
        // show live and wait for a key with timeout long enough to show images
        imshow("Live", frame);

        // waitKey 함수 ms단위로 대기
        // 만일 ()가 비어있으면 키보드 입력이 들어오기 전까지 무한 대기
        // 5ms 단위로 반복
        if (waitKey(5) >= 0)
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
