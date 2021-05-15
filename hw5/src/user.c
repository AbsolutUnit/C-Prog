#include <user.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct user
{
    char* handle;
    int ref_count;
} USER;

pthread_mutex_t arr_lock;

USER *user_create(char *handle) {
    USER* new_user = malloc(sizeof(USER));
    new_user->handle = (char *)(malloc(sizeof(char) * strlen(handle) + 1));
    new_user->handle = strcpy(new_user->handle, handle);
    new_user->ref_count = 1;
    return new_user;
}

USER *user_ref(USER *user, char *why) {
//    pthread_mutex_lock(&arr_lock);
    user->ref_count += 1;
//    pthread_mutex_unlock(&arr_lock);
    return user;
}

void user_unref(USER *user, char *why) {
//    pthread_mutex_lock(&arr_lock);
    user->ref_count -= 1;
    if (user->ref_count == 0){
        free(user->handle);
        free(user);
    }
//    pthread_mutex_unlock(&arr_lock);
}

char *user_get_handle(USER *user) {
    return user->handle;
}