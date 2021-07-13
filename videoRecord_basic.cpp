#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
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

#define TIME_FILENAME 0
#define FOLDER_NAME 1
#define LOG_TIME 2

char tBUF[100];
char filePath[100];
char folderPath[100];
char resultPath[100];
int savePath[100] = {2021071317, 2021071319};
int i = 0;
const char* MMOUNT = "/proc/mounts";
const char* BASEPATH = "/home/pi/blackBox/blackBox/daytime";
void getTime(int ret_type){
  
  time_t UTCtime;
  struct tm* tm;    
  time(&UTCtime);
  tm = localtime(&UTCtime);
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
}

struct f_size
{
    long blocks;
    long avail; 
    float ratio;
};

typedef struct _mountinfo 
{
    FILE *fp;                // 파일 스트림 포인터    
    char devname[80];        // 장치 이름
    char mountdir[80];        // 마운트 디렉토리 이름
    char fstype[12];        // 파일 시스템 타입
    struct f_size size;        // 파일 시스템의 총크기/사용율 
} MOUNTP;

MOUNTP *dfopen()
{
    MOUNTP *MP;

    // /proc/mounts 파일을 연다.
    MP = (MOUNTP *)malloc(sizeof(MOUNTP));
    if(!(MP->fp = fopen(MMOUNT, "r")))
    {
        return NULL;
    }
    else
        return MP;
}

MOUNTP *dfget(MOUNTP *MP)
{
    char buf[256];
    char *bname;
    char null[16];
    struct statfs lstatfs;
    struct stat lstat; 
    int is_root = 0;

    // /proc/mounts로 부터 마운트된 파티션의 정보를 얻어온다.
    while(fgets(buf, 255, MP->fp))
    {
        is_root = 0;
        sscanf(buf, "%s%s%s",MP->devname, MP->mountdir, MP->fstype);
        
        // /dev/root
        if (strcmp(MP->mountdir,"/") == 0) is_root=1;
        // if (stat(MP->devname, &lstat) == 0 || is_root)
        if(is_root)
        {
            if (strstr(buf, MP->mountdir) && S_ISBLK(lstat.st_mode) || is_root)
            {
                // 파일시스템의 총 할당된 크기와 사용량을 구한다.        
                statfs(MP->mountdir, &lstatfs);
                MP->size.blocks = lstatfs.f_blocks * (lstatfs.f_bsize/1024); 
                MP->size.avail  = lstatfs.f_bavail * (lstatfs.f_bsize/1024); 
                MP->size.ratio = (MP->size.avail * 100) / MP->size.blocks;
                return MP;
            }
            break;
        }
    }
    rewind(MP->fp);
    return NULL;
}

int dfclose(MOUNTP *MP)
{
    fclose(MP->fp);
    return 0;
}

static int filter(const struct dirent* dirent){
    
    int result;

    /* 현재 디렉토리, 이전 디렉토리 표시는 출력안함 */
    // strcmp()함수는 true면 0 반환 == !(not)
    // ==> .이나 ..이면 filter()함수에서 0을 반환하여 scandir에서 제외시킴 
    result = !(strcmp(dirent->d_name, ".")) || !(strcmp(dirent->d_name, "..")) ? 0 : 1;
    return result;
}

int searchOldFolder(int savePath) 
{ 
    struct dirent **namelist; 
    int count; 
    int idx; 
    long long min = 0;
    long long num[100];
    int missionSuccess;
    
    // 1st : 내가 탐색하고자 하는 폴더
    // 2nd : namelist를 받아올 구조체 주소값
    // 3rd : filter
    // 4th : 알파벳 정렬
    // scandir()함수에서 namelist 메모리를 malloc
    
    if((count = scandir(BASEPATH, &namelist, *filter, alphasort)) == -1) 
    { 
        fprintf(stderr, "%s Directory Scan Error: %s\n", BASEPATH, strerror(errno)); 
        return 1; 
    } 
    printf("count = %d\n",count);    
    
    for(idx=0; idx<count; idx++)
    {
        num[idx] = atoll(namelist[idx]->d_name);
        printf("num[idx] = %lld\n", num[idx]);
    }

    min = num[0];
    
    for(idx = 0; idx < count; idx++)
    {
        printf("num[idx] = %lld\n", num[idx]);
        printf("count = %d\n", count);
        if(num[idx] < min) //num[idx]가 min보다 작다면
            min = num[idx]; //min 에는 num[idx]의 값이 들어감
        else
        {
            continue;
        }
                
    }
    printf("min = %lld\n", min);

    idx = 0;
    while(count != 0)
    {
      sprintf(filePath, "%s/%lld", BASEPATH, num[idx]);
      if (unlink(filePath) == -1)
      {
        printf("파일 삭제 실패\n");
        missionSuccess = 0;
      }
      else
      {
        printf("파일 삭제 성공\n");
        idx++;
      }
      
    }
    
    if (count == 0)
    {
      if (rmdir(folderPath) == -1)
      {
        printf("폴더 삭제 실패\n");
        missionSuccess = 0; 
      }
      else
      {
        printf("폴더 삭제 성공\n");
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

int main(int, char**)
{
    // 1. VideoCapture("동영상 파일의 경로") 함수 사용
    VideoCapture cap;
    VideoWriter writer;
    Mat frame;
    int fd;
    char buff[200];
      
    int length;
    int WRBytes;

    FILE* fp;   
    
    // 로그파일을 기록하기 위해 파일열기
    fd = open("/home/pi/blackBox/blackBox/blackBox.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    getTime(LOG_TIME);
    length = sprintf(buff, "%sblackbox log파일 저장을 시작합니다.\n", tBUF);
    WRBytes = write(fd, buff, length);

    // STEP 1. 카메라 장치 열기
    int deviceID = 0;
    int apiID = CAP_V4L2;
    int maxFrame = 1440;
    int frameCount;
    int exitFlag = 0;
    
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

    float limit_size = 0.0f;

    while (1)
    {
      MOUNTP *MP;
      if ((MP=dfopen()) == NULL)
      {
          perror("error");
          exit(1);
      }
 
      if (dfget(MP))
      {
        if ((limit_size = MP->size.ratio) <= 53)
        { 
          printf("용량이 부족합니다.\n");
          printf("%5f\n", limit_size);
          printf("==================\n\n");
          while (limit_size <= 53)
          {
            int rewinder;
            rewinder = searchOldFolder(savePath[i++]);
            if (rewinder == 0)
            {
              printf("용량 확보 실패.\n");
            }
            
          }
        }
        else
        {
          printf("용량을 확보했습니다.\n");
          printf("%5f\n", limit_size);
          printf("==================\n\n");
        } 
      }

      // 시간정보를 읽어와서 파일명 생성
      // 전역변수 fileName에 저장
      getTime(TIME_FILENAME);
      printf("filePath : %s\n", tBUF);
      sprintf(filePath, "%s/%s", BASEPATH, tBUF);

      getTime(LOG_TIME);
      length = sprintf(buff, "%s %s 명으로 녹화를 시작합니다.\n", tBUF, filePath);
      
      getTime(FOLDER_NAME);
      printf("folderPath : %s\n", tBUF);
      sprintf(folderPath, "%s/%s", BASEPATH, tBUF);
      
      getTime(LOG_TIME);
      length = sprintf(buff, "%s %s 명으로 폴더가 생성되거나 파일이 이동합니다.\n", tBUF, folderPath);
      WRBytes = write(fd, buff, length);
      
      WRBytes = write(fd, buff, length);

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