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

// "202107121433.avi"
// #define OUTPUT_VIDEO_NAME "test.avi"
#define VIDEO_WINDOW_NAME "record"
    
char fileName[30];

void makefileName(void){
  time_t UTCtime;
  struct tm* tm;
  time(&UTCtime);
  tm = localtime(&UTCtime);
  strftime(fileName, sizeof(fileName), "%Y%m%d%H%M.avi", tm);
  printf("strftime : %s\n", fileName);  
}

int main(int, char**)
{
    // 1. VideoCapture("동영상 파일의 경로") 함수 사용
    VideoCapture cap;
    VideoWriter writer;
    Mat frame;

    // STEP 1. 카메라 장치 열기
    int deviceID = 0;
    int apiID = cv::CAP_V4L2;
    cap.open(deviceID, apiID);

    if (!cap.isOpened()) {
        perror("ERROR! Unable to open camera\n");
        return -1;
    }

    cap.set(CAP_PROP_FPS, 30);
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    

    // Video Recording
    // 현재 카메라에서 초당 몇 프레임으로 출력하는지 확인
    float videoFPS = cap.get(CAP_PROP_FPS);

    // 해상도
    int videoWidth = cap.get(CAP_PROP_FRAME_WIDTH);
    int videoHeight = cap.get(CAP_PROP_FRAME_HEIGHT);
    
    printf("videoFPGS = %f\n", videoFPS);
    printf("width = %d, height = %d\n", videoWidth, videoHeight);
    
    // 1st : 저장하고자 하는 파일명
    // 2nd : 코덱 지정
    // 3rd : FPS
    // 4th : ImageSize
    // 5th : isColor = True
    makefileName();
    writer.open(fileName, VideoWriter::fourcc('D', 'I', 'V', 'X'), videoFPS, Size(videoWidth, videoHeight), true);

    if (!writer.isOpened())
    {
      perror("Can't write video");
      return -1;
    }
    
    // 창 생성
    namedWindow(VIDEO_WINDOW_NAME);

    while (1)
    {
      // 카메라에서 매 프레임마다 이미지 읽기
      cap.read(frame);     

      if (frame.empty()) {
            perror("ERROR! blank frame grabbed\n");
            break;
      }  

      // 읽어온 한 장의 프레임을 writer에 쓰기
      writer << frame; // test.avi
      imshow(VIDEO_WINDOW_NAME, frame);

      // ESC => 27; 'ESC' 키가 입력되면 종료
      // 키 입력을 확인
      if (waitKey(1000/videoFPS) == 27)
      {
        printf("Stop video record\n");
        break;
      }
      
    }
    cap.release();
    writer.release();

    // 창 삭제
    destroyWindow(VIDEO_WINDOW_NAME);

    return 0;
}
