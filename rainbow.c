#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <string.h>

#define THREADS 4
#define LINES_SIZE 10000
#define BATCH_SIZE (LINES_SIZE / THREADS)

volatile unsigned found = 0;

typedef struct thread_conf {
    char** buffer;
    char* sha256_value;
} thread_conf;

void sha256(const char *str, char *output) {
    EVP_MD_CTX *context = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;

    EVP_DigestInit_ex(context, EVP_sha256(), NULL);
    EVP_DigestUpdate(context, str, strlen(str));
    EVP_DigestFinal_ex(context, hash, &length);
    EVP_MD_CTX_free(context);

    for(int i = 0; i < length; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }

    output[64] = '\0';
}

void* thread_process_line(void* arg) {

    thread_conf* conf = (thread_conf*) arg;

    for (unsigned int i = 0; i < BATCH_SIZE; i++) {
        if (found == 1) {
            return NULL;
        }
        if (conf->buffer[i] != NULL) {
            char sha256_result[65];
            sha256(conf->buffer[i], sha256_result);
            if (strcmp(sha256_result, conf->sha256_value) == 0) {
                printf("We have a match ! value %s : sha256: %s\n", conf->buffer[i], sha256_result);
                found = 1;
            }
        }
    }

    return NULL;
}

int main(int argc, char** argv) {

    if (argc < 3) {
        printf("Usage: %s <in> <sha256>\n", argv[0]);
        return 1;
    }


    for (unsigned short sha_count = 2; sha_count < argc; sha_count ++) {

        found = 0;

        FILE* in = fopen(argv[1], "r");

        if (in == NULL) {
            perror("fopen in");
            return 2;
        }

        char* lines[LINES_SIZE];
    
        size_t n = 0;

        unsigned int count = 0;

        for (int i = 0; i < LINES_SIZE; i++) {
            lines[i] = NULL;
        }

        while(!feof(in)) {

            count = 0;
            
            while(count < LINES_SIZE && (getline(&lines[count], &n, in) != -1)) {
                count ++;
                n = 0;
            }
            
            pthread_t threads[THREADS];
           
            for (unsigned int i = 0; i < THREADS; i ++) {
                thread_conf conf = {
                    .buffer = &lines[BATCH_SIZE * i],
                    .sha256_value = argv[sha_count]
                };
                pthread_create(&threads[i], NULL, thread_process_line, &conf);
            }
        
            for (unsigned int i = 0; i < THREADS; i ++) {
                pthread_join(threads[i], NULL);
            }

            for (unsigned int i = 0; i < count; i ++) {
                if (lines[i] != NULL) {
                    free(lines[i]);
                }
            }
            
        }

        fclose(in);
    }


    return 0;
}