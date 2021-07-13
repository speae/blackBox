#include <dirent.h> 
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h>

#define MAX_LIST 50 

const char *path = "/home/pi/blackBox/blackBox/daytime"; 

static int filter(const struct dirent* dirent){
    
    int result;

    /* 현재 디렉토리, 이전 디렉토리 표시는 출력안함 */
    // strcmp()함수는 true면 0 반환 == !(not)
    // ==> .이나 ..이면 filter()함수에서 0을 반환하여 scandir에서 제외시킴 
    result = !(strcmp(dirent->d_name, ".")) || !(strcmp(dirent->d_name, "..")) ? 0 : 1;

    return result;
}

int searchOldFolder(void) 
{ 
    struct dirent **namelist; 
    int count; 
    int idx; 
    int min = 0;
    int num[MAX_LIST];
    
    // 1st : 내가 탐색하고자 하는 폴더
    // 2nd : namelist를 받아올 구조체 주소값
    // 3rd : filter
    // 4th : 알파벳 정렬
    // scandir()함수에서 namelist 메모리를 malloc
    if((count = scandir(path, &namelist, *filter, alphasort)) == -1) 
    { 
        fprintf(stderr, "%s Directory Scan Error: %s\n", path, strerror(errno)); 
        return 1; 
    } 
    printf("count = %d\n",count);    
    
    for(idx=0;idx<count;idx++)
    {
        num[idx] = atoi(namelist[idx]->d_name);
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
    printf("min = %d\n", min);

    // 건별 데이터 메모리 해제 
    for(idx = 0; idx < count; idx++) 
    { 
        free(namelist[idx]); 
    } 
    
    // namelist에 대한 메모리 해제 
    free(namelist); 
    
    return min; 
}

int main(void) { 
    
    int result; 
    char folderName[30];
    result = searchOldFolder();
    sprintf(folderName,"%d",result);
    printf("가장 오래된 폴더는 %s 이다.\n",folderName);

}