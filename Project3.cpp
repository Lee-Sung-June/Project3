#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <iostream>
#include <time.h>
using namespace std;
unsigned long long limit = 1000000000000LL ; //target: 1조까지
unsigned long long sqlimit, sqlimit2 ; //sqrt 취해서 각각 100만, 1천까지
unsigned char *block0 ; //첫번째  sieve block 만들어주기
#define min(a, b)       ((a)<(b)?(a):(b)) //각각 작은 block들의 시작부분을 판단
int nthreads = 10 ; //총 Thread의 개수 = 10개
pthread_mutex_t mutex ; //thread 선언
unsigned long long nextblock ;  //다음 block들을 만들어줄때 사용
unsigned long long numberOfPrime ;  //소수의 개수 선언

/* 각 쓰레드들이 어떻게 움직이는가에 대한 함수를 만든다 */
void *
workerThread(void *parm)
{
    unsigned long long low, high, multiplier ; //block들의 시작,끝,배수
    unsigned long long idx, idx2, count = 0LL ; //for문 돌릴때 index들, block count
    unsigned char *block1 = (unsigned char *) parm ; //각 block들 설정
    int i, rc ; //main에서 사용함
    for (;;) {
            pthread_mutex_lock(&mutex) ;
            low = nextblock ; //각 block의 시작점을 설정
            high = min(low+sqlimit, limit) ;   //각 block의 끝점을 설정
            nextblock = high  ; //끝을 그다음 block의 시작점으로 바꿔주기 위해
            pthread_mutex_unlock(&mutex) ;
            if (low >= limit)    //1조개를 넘어가면 멈춘다
                break ;
            /* now, sieve */
            //block1을 백만 사이즈의 0배열로 초기화한다
            memset(block1, 0, sizeof(*block1)*sqlimit) ;
            for (idx=2; idx<sqlimit; idx++) { //2부터 sqrt까지만 확인해도 된다
                if (block0[idx] == 0) { //소수의 배수들을 체크할 것이다
                    multiplier = low / idx ;
                    idx2 = multiplier * idx ;
                    if (idx2 < low) //만약 인덱스가 시작보다 작으면 크게 만들어준다
                        idx2 += idx ;
                    for ( ; idx2 < high; idx2 += idx) //소수의 배수들을 1로 체크한다
                        block1[idx2-low] = 1 ;
                }
            }
            for (idx=low, count=0LL; idx < high; idx ++) { //해당 block의 소수를 체크한다
                if (block1[idx-low] == 0) {
                    count ++ ; // 0표시된 소수들을 체크하고 개수를 센다
                }
            }
            pthread_mutex_lock(&mutex) ;
            numberOfPrime += count ; //각각의 block들의 소수 개수를 더한다
            pthread_mutex_unlock(&mutex) ;
    }
    return NULL ;
}

/* main에서 Thread들을 이용한다 */
int main(int argc, char *argv[])
{
    time_t start_time = time(NULL);
    unsigned long long idx, idx2, count = 0LL ;
    unsigned long long low, high, multiplier ;
    int i, rc ;
    if (argc > 1)
        limit = atoll(argv[1]) ; //문자 스트링을 long long 값으로 반환한다
    sqlimit = (unsigned long long) ceil(sqrt((double) limit)) ;
    sqlimit2 = (unsigned long long) ceil(sqrt((double) sqlimit)) ;

    block0 = (unsigned char *) calloc(sqlimit, sizeof(unsigned char)) ;
    /* 우선은 첫번째 sieve block의 소수들을 구한다 */
    for (idx=2; idx<=sqlimit2; idx++)
        if (block0[idx] == 0) {
            for (idx2 = idx*idx; idx2 < sqlimit; idx2 += idx)
                block0[idx2] = 1 ;
        }
    for (idx=2LL; idx < sqlimit; idx++)
        if (block0[idx] == 0) {
            count ++ ;
        }
    /* 이제 Thread를 이용하여 다른 block들의 소수의 개수를 구한다 */
    nextblock = sqlimit ;
    numberOfPrime = count ;
    pthread_mutex_init(&mutex, NULL) ;
    pthread_t *thread = (pthread_t *) calloc(nthreads, sizeof(pthread_t)) ;
    for (i=0; i<nthreads; i++) {
        rc = pthread_create(&thread[i], NULL, workerThread, (void *) calloc(sqlimit, sizeof(unsigned char))) ;
    }
    for (i=0; i<nthreads; i++) {
        pthread_join(thread[i], NULL) ;
    }

    cout << "소수의 개수:" << numberOfPrime << endl;
    time_t end_time = time(NULL);
    time_t elapsed_time = (double) (end_time - start_time);
    cout << "프로그램 수행 시간:" << elapsed_time << endl;
    return 0;
}
