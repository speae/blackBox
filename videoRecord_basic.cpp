#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <pthread.h> 
#include <dirent.h> 
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> // O_WRONLY
#include <unistd.h> // write(), read()

using namespace cv;
using namespace std;

// "202107121433.avi"
// #define OUTPUT_VIDEO_NAME "test.avi"
#define VIDEO_WINDOW_NAME "record"

// mutex 초기화
static pthread_mutex_t locker;
  
// 전역 변수
int time_type;
int thread_number = 0;
int TIME_FILENAME = 0;
int FOLDER_NAME = 1;
int LOG_TIME = 2;

char tBUF[200];
char buff[200];
const char* BASEPATH = "/home/pi/blackBox/blackBox/daytime";
const char* LOGPATH = "/home/pi/blackBox/blackBox/blackBox.log";

// log 파일 작성 시 필요한 전역 변수들
int fd = open(LOGPATH, O_WRONLY | O_CREAT | O_TRUNC, 0644); 
int length;
int WRBytes;

void* getTime(void* data){
  
  time_t UTCtime;
  struct tm* tm;    
  
  int err;
  err = pthread_mutex_lock(&locker);
  if (err)
  {
    cout << "ERROR : getTime is unlocked." << endl;
  }
  time(&UTCtime);
  tm = localtime(&UTCtime);
  int ret_type = *((int*)data);
  
  if (ret_type == TIME_FILENAME)
  {
    strftime(tBUF, sizeof(tBUF), "%Y%m%d%H%M%S.avi", tm);
  }
  else if (ret_type == FOLDER_NAME)
  {
    strftime(tBUF, sizeof(tBUF), "%Y%m%d%H", tm);
  }
  else if (ret_type == LOG_TIME)
  {
    strftime(tBUF, sizeof(tBUF), "[%Y-%m-%d %H:%M:%S]", tm);
  }

  err = pthread_mutex_unlock(&locker);
  if (err)
  {
    cout << "ERROR : getTime is locked." << endl;
  }

  return NULL;
}

float dfgetRatio(){

  struct statfs lstatfs;
  const char* MMOUNT = "/";
  float result;
  
  int boolean;
  boolean = statfs(MMOUNT, &lstatfs);
  if (boolean == 0)
  {
    result = (lstatfs.f_bavail * 100.0) / lstatfs.f_blocks;
  }
  else
  {
    perror("ERROR! Unable mount to root.\n");
    exit(1);
  }

  return result;
}

static int filter(const struct dirent* dirent){
    
    int result;

    /* 현재 디렉토리, 이전 디렉토리 표시는 출력안함 */
    // strcmp()함수는 true면 0 반환 == !(not)
    // ==> .이나 ..이면 filter()함수에서 0을 반환하여 scandir에서 제외시킴 
    result = !(strcmp(dirent->d_name, ".")) || !(strcmp(dirent->d_name, "..")) ? 0 : 1;
    return result;
}

int searchOldFolder(){

    struct dirent **namelist; 
    int count; 
    int idx; 
    long long min = 0;
    long long num[100];
    int missionSuccess;
    
    pthread_t t_id;
    int err;

    pthread_mutex_init(&locker, NULL);
  
    // 1st : 내가 탐색하고자 하는 폴더
    // 2nd : namelist를 받아올 구조체 주소값
    // 3rd : filter
    // 4th : 알파벳 정렬
    // scandir()함수에서 namelist 메모리를 malloc
    if((count = scandir(BASEPATH, &namelist, *filter, alphasort)) == -1) 
    { 
        fprintf(stderr, "%s Directory Scan Error: %s\n", BASEPATH, strerror(errno)); 
        time_type = LOG_TIME;
        err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
        if (err != 0)
        {
          perror("getTime is not created.");
          exit(1);
        }
        pthread_join(t_id, NULL);
        length = sprintf(buff, "%s %s 폴더를 찾는데 실패했습니다.\n", tBUF, BASEPATH);
        WRBytes = write(fd, buff, length);
        pthread_detach(t_id);
        return 1; 
    } 
    printf("count = %d\n",count);   

    for(idx = 0; idx < count; idx++)
    {
        num[idx] = atoll(namelist[idx]->d_name);
    }

    min = num[0];
    
    for(idx = 0; idx < count; idx++)
    {
      if(num[idx] < min) //num[idx]가 min보다 작다면
        min = num[idx]; //min 에는 num[idx]의 값이 들어감
      else
      {
        continue;
      }              
    }
  
    char deletePath[100];
    sprintf(deletePath, "%s/%lld", BASEPATH, min);  
    printf("deletePath = %s\n", deletePath);
    if((count = scandir(deletePath, &namelist, *filter, alphasort)) == -1) 
    { 
      fprintf(stderr, "%s File Scan Error: %s\n", deletePath, strerror(errno)); 
      time_type = LOG_TIME;
      err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
      if (err != 0)
      {
        perror("getTime is not created.");
        exit(1);
      }
      pthread_join(t_id, NULL);
      length = sprintf(buff, "%s %s 파일을 찾는데 실패했습니다.\n", tBUF, deletePath);
      WRBytes = write(fd, buff, length);
      pthread_detach(t_id);
      return 1; 
    } 
    printf("count = %d\n",count);   

    for(idx = 0; idx < count; idx++)
    {
      num[idx] = atoll(namelist[idx]->d_name);
    }
    
    char deleteFile[100];
    idx = 0;
    while(idx != count)
    {
      sprintf(deleteFile, "%s/%lld/%lld.avi", BASEPATH, min, num[idx]);  
      if (unlink(deleteFile) == -1)
      {
        printf("파일 삭제 실패.\n");
        break;
      }
      else
      {
        printf("파일 삭제 성공.\n");
        idx++;
      }
     
    }
    
    if (access(deletePath, F_OK) == 0)
    {
      if (rmdir(deletePath) == -1)
      {
        printf("%s 폴더 삭제 실패\n", deletePath);
        missionSuccess = 0; 
      }
      else
      {
        printf("%s 폴더 삭제 성공\n", deletePath);
        missionSuccess = 1;
      }
      
    }
    
    // 건별 데이터 메모리 해제 
    for(idx = 0; idx < count; idx++) 
    { 
      free(namelist[idx]); 
    } 
    
    // namelist에 대한 메모리 해제 
    free(namelist); 
    
    return missionSuccess; 
}

int main(int argc, char* argv[]){
  
  char filePath[100];
  char folderPath[100];
  char threadBuff[200];
  
  // 1. VideoCapture("동영상 파일의 경로") 함수 사용
  VideoCapture cap;
  VideoWriter writer;
  Mat frame;
  
  pthread_t t_id;
  int err;

  pthread_mutex_init(&locker, NULL);
  
  // 로그파일을 기록하기 위해 파일열기
  time_type = LOG_TIME;
  err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
  if (err)
  {
    perror("getTime is not created.");
    exit(1);
  }
  err = pthread_join(t_id, NULL);
  if (err)
  {
    perror("getTime is not joined.");
    exit(1);
  }
  length = sprintf(buff, "%sblackbox log파일 저장을 시작합니다.\n", tBUF);
  WRBytes = write(fd, buff, length);
  pthread_detach(t_id);

  // STEP 1. 카메라 장치 열기
  int deviceID = 0;
  int apiID = CAP_V4L2;
  int maxFrame = 1440;
  int frameCount;
  int exitFlag = 0;
  
  cap.open(deviceID, apiID);

  if (!cap.isOpened()) {
      perror("ERROR! Unable to open camera.\n");
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
    float ratio = dfgetRatio();  
    
    while (ratio <= 53.0)
    { 
      printf("용량이 부족합니다.\n");
      printf("현재 용량 : %5f%%\n", ratio);
      printf("==================\n\n");
      time_type = LOG_TIME;
      err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
      if (err != 0)
      {
        perror("getTime is not created.");
        exit(1);
      }
      pthread_join(t_id, NULL);
      length = sprintf(buff, "%s %s 녹화 시작 가능까지 %5f%%만큼 용량이 부족합니다.\n", tBUF, BASEPATH, 53.0 - ratio);
      WRBytes = write(fd, buff, length);
      pthread_detach(t_id);

      int rewinder;
      if ((rewinder = searchOldFolder()) == 1)
      {
        ratio = dfgetRatio(); 
        printf("용량 확보 중...\n");
        printf("%5f\n", ratio);
        printf("...\n\n");
        time_type = LOG_TIME;
        err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
        if (err != 0)
        {
          perror("getTime is not created.");
          exit(1);
        }
        pthread_join(t_id, NULL);
        length = sprintf(buff, "%s %s 용량 확보 중... 현재 %5f%%\n", tBUF, BASEPATH, ratio);
        WRBytes = write(fd, buff, length);
        pthread_detach(t_id);
      }
      else
      {
        printf("용량 확보 실패.\n");
        printf("%5f\n", ratio);
        printf("==================\n\n");
        time_type = LOG_TIME;
        err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
        if (err != 0)
        {
          perror("getTime is not created.");
          exit(1);
        }
        pthread_join(t_id, NULL);
        length = sprintf(buff, "%s %s 용량 확보 실패... %5f%%\n", tBUF, BASEPATH, ratio);
        WRBytes = write(fd, buff, length);
        pthread_detach(t_id);
        break;
      }
    }
    
    printf("용량이 충분합니다.\n");
    printf("%5f\n", ratio);
    printf("==================\n\n");
    time_type = LOG_TIME;
    err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
    if (err != 0)
    {
      perror("getTime is not created.");
      exit(1);
    }
    pthread_join(t_id, NULL);
    length = sprintf(buff, "%s %s 용량 : %5f%% --> 충분합니다.\n", tBUF, BASEPATH, ratio);
    WRBytes = write(fd, buff, length);
    pthread_detach(t_id);

    // 시간정보를 읽어와서 파일명 생성
    // 전역변수 fileName에 저장
    time_type = FOLDER_NAME;
    err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
    if (err != 0)
    {
      perror("getTime is not created.");
      exit(1);
    }
    pthread_join(t_id, NULL);
    sprintf(folderPath, "%s/%s", BASEPATH, tBUF);
    printf("folderPath : %s\n", folderPath);
    pthread_detach(t_id);

    time_type = LOG_TIME;
    err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
    if (err != 0)
    {
      perror("getTime is not created.");
      exit(1);
    }
    pthread_join(t_id, NULL);
    length = sprintf(buff, "%s %s 명으로 폴더가 생성되거나 파일이 이동합니다.\n", tBUF, folderPath);
    WRBytes = write(fd, buff, length);
    pthread_detach(t_id);

    time_type = TIME_FILENAME;
    err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
    if (err != 0)
    {
      perror("getTime is not created.");
      exit(1);
    }
    pthread_join(t_id, NULL);
    sprintf(filePath, "%s/%s", folderPath, tBUF);
    printf("filePath : %s\n", filePath);
    pthread_detach(t_id);

    time_type = LOG_TIME;
    err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
    if (err != 0)
    {
      perror("getTime is not created.");
      exit(1);
    }
    pthread_join(t_id, NULL);
    length = sprintf(buff, "%s %s 명으로 녹화를 시작합니다.\n", tBUF, filePath);
    WRBytes = write(fd, buff, length);
    pthread_detach(t_id);

    if(access(folderPath, F_OK) == -1)
    {
      mkdir(folderPath, 0755);    
      writer.open(filePath, VideoWriter::fourcc('D', 'I', 'V', 'X'), videoFPS, Size(videoWidth, videoHeight), true);
    }
    else
    {
      writer.open(filePath, VideoWriter::fourcc('D', 'I', 'V', 'X'), videoFPS, Size(videoWidth, videoHeight), true);
    }
    
    if (!writer.isOpened())
    {
      perror("Can't write video");
      time_type = LOG_TIME;
      err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
      if (err != 0)
      {
        perror("getTime is not created.");
        exit(1);
      }
      pthread_join(t_id, NULL);
      length = sprintf(buff, "%s %s 파일 생성 실패.\n", tBUF, filePath);
      WRBytes = write(fd, buff, length);
      pthread_detach(t_id);
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
        time_type = LOG_TIME;
        err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
        if (err != 0)
        {
          perror("getTime is not created.");
          exit(1);
        }
        pthread_join(t_id, NULL);
        length = sprintf(buff, "%s %s 이미지 읽기 실패.\n", tBUF, filePath);
        WRBytes = write(fd, buff, length);
        pthread_detach(t_id);
        break;
      } 

      writer << frame;
      imshow(VIDEO_WINDOW_NAME, frame);

      if (waitKey(10) == 27)
      {
        printf("Stop video record\n");
        exitFlag = 1;
        time_type = LOG_TIME;
        err = pthread_create(&t_id, NULL, getTime, (void*)&time_type);
        if (err != 0)
        {
          perror("getTime is not created.");
          exit(1);
        }
        pthread_join(t_id, NULL);
        length = sprintf(buff, "%s %s 동영상 종료.\n", tBUF, filePath);
        WRBytes = write(fd, buff, length);
        pthread_detach(t_id);
        break;
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