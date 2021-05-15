#include <user_registry.h>
#include <user.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>



typedef struct user_registry
{
    int user_count;
    USER** users;
} USER_REGISTRY;

pthread_mutex_t locker;

USER_REGISTRY *ureg_init(void){
    USER_REGISTRY* new_reg = malloc(sizeof(USER_REGISTRY));
    new_reg->users = malloc(sizeof(USER*));
    new_reg->users[0] = NULL;
    new_reg->user_count = 0;
    return new_reg;
}

void ureg_fini(USER_REGISTRY *ureg){
    for (int i = 0; i < 10; i++){
        if (ureg->users[i] == NULL){
            continue;
        } else {
            user_unref(ureg->users[i], "Registry Finished");
        }
    }
    free(ureg->users);
    free(ureg);
}

USER *ureg_register(USER_REGISTRY *ureg, char *handle){
    pthread_mutex_lock(&locker);
    for (int i = 0; i <= ureg->user_count; i++){
        if (ureg->users[i] == NULL) {
            continue;
        } else {
            if (strcmp(handle, user_get_handle(ureg->users[i])) == 0){
                user_ref(ureg->users[i], "User referenced.");
                pthread_mutex_unlock(&locker);
                return ureg->users[i];
            }
        }
    }
    USER* new_user = user_create(handle);
    user_ref(new_user, "New user registered");
    ureg->users[ureg->user_count] = new_user;
    ureg->user_count++;
    ureg = realloc(ureg, sizeof(*ureg) + sizeof(USER*));
    ureg->users[ureg->user_count] = NULL;
    pthread_mutex_unlock(&locker);
    return new_user;
}

void ureg_unregister(USER_REGISTRY *ureg, char *handle){
    for (int i = 0; i <= ureg->user_count; i++){
        if (strcmp(user_get_handle(ureg->users[i]), handle) == 0){
            user_unref(ureg->users[i], "Registry unreference.");
        }
    }
}
