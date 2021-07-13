#include<stdio.h>
#include<stdlib.h>

#define MAX_LIST 50

int searchOldFolder(void)

{
    // 숫자를 비교할 배열의 개수를 MAX_LIST
    int num[MAX_LIST] = {2021071322, 2021071321, 2021071323, 2021071400, 2021071401, 2021071402};

    int i;

    int min;


    min = num[0];     // min 초기화

    printf("sizeof(long unsigned int)=%d\n", sizeof(int));

    printf("==================================================\n");

    printf("■  최대값 구하는 프로그램\n");

    printf("==================================================\n");

    for(i=0;i<MAX_LIST;i++)

    {
        if (num[i] == '\0')
        {
            break;
        }
        

        printf("%d 번째 배열 요소 . . . . . . %d \n",i+1, num[i]);
    }
    for(i = 0;i<MAX_LIST;i++)

    {
        if (num[i] == '\0')
        {
            break;
        }
        else
        {
            if(num[i] < min ) //num[i]가 max보다 크다면

            min = num[i]; //max 에는 num[i]의 값이 들어감
    
        }
        
    }

    printf("---------------------------------------------------\n");

    printf(" >>> 5개의 숫자중에서 최소값은 [ %d ] 입니다.\n", min); //최대값 출력

    printf("==================================================\n");

    return min;
}


int main(void){
    int result;
    char folderName[30];
    result = searchOldFolder();
    sprintf(folderName, "%d", result);
    printf("가장 오래된 폴더는 %s\n", folderName);
}