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
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

using namespace cv;
using namespace std;

// "202107121433.avi"
// #define OUTPUT_VIDEO_NAME "test.avi"
#define VIDEO_WINDOW_NAME "record"

char fileName[30];
char folderName[30];
char* folderBuffer;
char fileBuffer[30];
struct folderNameStruct{
  int folder_year;
  int folder_month;
  int folder_day;
  int folder_hour;
};

char* makefolderName(void){
  
  time_t UTCtime;
  struct tm* tm;
  struct folderNameStruct* folderNameSave;    
  time(&UTCtime);
  tm = localtime(&UTCtime);
  folderBuffer = (char*)malloc(sizeof(char32_t));
  sprintf(folderBuffer, "%d%d%d%d\n", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour);
  strftime(folderName, sizeof(folderName), "%Y%m%d%H", tm);
  return folderName; 
}

void makefileName(void){
  
  time_t UTCtime;
  struct tm* tm;    
  struct folderNameStruct* folderNameSave;
  time(&UTCtime);
  tm = localtime(&UTCtime);
  sprintf(fileBuffer, "%d%d%d%d\n", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour);
  strftime(fileName, sizeof(fileName), "%Y%m%d%H%M.avi", tm);
  printf("strftime : %s\n", fileName);  
}

int main(int, char**)
{
    // 1. VideoCapture("동영상 파일의 경로") 함수 사용
    struct tm* tm;    
    time_t UTCtime;      
    time(&UTCtime);
    tm = localtime(&UTCtime);

    struct folderNameStruct* folderNameSave;    
    VideoCapture cap;
    VideoWriter writer;
    Mat frame;

    // STEP 1. 카메라 장치 열기
    int deviceID = 0;
    int apiID = CAP_V4L2;
    int maxFrame = 1440;
    int frameCount;
    int exitFlag = 0;
    char* maked_folder_time = makefolderName();

    cap.open(deviceID, apiID);

    if (!cap.isOpened()) {
        perror("ERROR! Unable to open camera\n");
        return -1;
    }

    cap.set(CAP_PROP_FPS, 24);
    cap.set(CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CAP_PROP_FRAME_HEIGHT, 240);
    
    // Video Recording
    // 현재 카메라에서 초당 몇 프레임으로 출력하는지 확인
    // CAP_PROP_FPS 값을 5라고 정의 
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
    while (1)
    {
      // 시간정보를 읽어와서 파일명 생성
      // 전역변수 fileName에 저장
      makefileName();
      
      writer.open(fileName, VideoWriter::fourcc('D', 'I', 'V', 'X'), videoFPS, Size(videoWidth, videoHeight), true);

      if (!writer.isOpened())
      {
        perror("Can't write video");
        return -1;
      }

      frameCount = 0;
      
      // 창 생성
      namedWindow(VIDEO_WINDOW_NAME);

      while (frameCount < maxFrame)
      {
        // 카메라에서 매 프레임마다 이미지 읽기
        cap.read(frame);     
        frameCount++;

        if (frame.empty()) {
              perror("ERROR! blank frame grabbed\n");
              break;
        }  

        // 읽어온 한 장의 프레임을 writer에 쓰기
        writer << frame; // test.avi
        imshow(VIDEO_WINDOW_NAME, frame);

        // ESC => 27; 'ESC' 키가 입력되면 종료
        // 키 입력을 확인
        // 카메라 영상에서 esc 키를 누를것
        if (waitKey(10) == 27)
        {
          printf("Stop video record\n");
          exitFlag = 1;
          break;
        }   
      }

      cout << maked_folder_time << endl;

      char* currentBuffer;
      currentBuffer = (char*)malloc(sizeof(char32_t));
      int current_hour = tm->tm_hour;
      sprintf(currentBuffer, "current_hour = %d\n", current_hour);

      char basePath[] = {"/home/pi/blackBox/blackBox/"};
      char* filePath;
      filePath = strcat(basePath, maked_folder_time);
      cout << filePath << endl;
      cout << folderBuffer << endl;
      cout << currentBuffer << endl;

      if(access(filePath, F_OK) == -1)
      {
        mkdir(maked_folder_time, 0755);
      }
      else
      {
        if (currentBuffer == folderBuffer)
        {
          char* command;
          char mv[5] = {"mv "};
          char* srcFile = fileName;
          char* dstPath = folderBuffer;
          char* resultPath;
          resultPath = strcat(srcFile, dstPath);
          command = strcat(mv, resultPath);
          popen(command, "rb");
          free(currentBuffer);
        }
        else
        {
          mkdir(maked_folder_time, 0777);

          char* command;
          char mv[5] = {"mv "};
          char* srcFile = fileName;
          char* dstPath = currentBuffer;
          char* resultPath;
          resultPath = strcat(srcFile, dstPath);
          command = strcat(mv, resultPath);
          popen(command, "rb");
        }
      }

      writer.release();
      if (exitFlag == 1){
        break;
      }
    }

    cap.release();
    
    // 창 삭제
    destroyWindow(VIDEO_WINDOW_NAME);

    return 0;
}
