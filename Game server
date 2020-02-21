#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define BUFSIZE 2048
#define STDPORT 4443
#define LQ_LEN 20
#define SESS_SIZE 20
int global_value;
const char value_message[] = "Global value: ";
const char intro_message[] = "Welcome to server by AB!\n";
const char change_message[] = "Value has been changed!!!\n";
enum sess_states {
    sess_start,
    sess_action,
    sess_finish,
    sess_error
};

struct session {
    int fd;
    char buf[BUFSIZE];
    int buf_used;
    enum sess_states state;
};

struct server_str {
    int ls;
    struct session **sess_array;
    int sess_array_size;
};

void send_string(struct session *sess, char *str)
{
    write(sess->fd, str, strlen(str));
}

void send_global_value(struct session* sess)
{
    char value[256];
    write(sess->fd, value_message, sizeof(value_message));
    sprintf(value, "%d", global_value);
    send_string(sess, value);

    write(sess->fd, "\n", 1);
}

void change_value(char *line)
{
    if (strcmp(line, "incr") == 0)
        global_value++;
    if (strcmp(line, "decr") == 0)
        global_value--;
}

struct session *make_new_session(int fd, struct sockaddr_in *from)
{
    struct session *sess = malloc(sizeof(struct session));
    sess->fd = fd;
    sess->buf_used = 0;
    sess->state = sess_action;
    write(fd, intro_message, sizeof(intro_message));
    return sess;
}

void sess_step(struct session *sess, char *line)
{
    switch (sess->state) {
        case sess_start:
        case sess_action:
            change_value(line);
            send_global_value(sess);
        case sess_finish:
        case sess_error:
            free(line);   
    }
}

void session_check_lf(struct session *sess, struct server_str* serv)
{
    int pos = -1;
    int i, j;
    char *line;
    for (i = 0; i < sess->buf_used; i++) {
        if (sess->buf[i] == '\n') {
            for (j = 0; j < serv->sess_array_size; j++){
                if (serv->sess_array[j] != 0)
                    write(serv->sess_array[j]->fd, 
                          change_message,
                          sizeof(change_message));
            }
            pos = i;
            break;
        }
    }
    if (pos == -1)
        return;
    line = malloc(pos+1);
    memcpy(line, sess->buf, pos);
    line[pos] = 0;
    memmove(sess->buf, sess->buf+pos, pos+1);
    sess->buf_used -= (pos+1);
    if (line[pos-1] == '\r')
        line[pos-1] = 0;
    sess_step(sess, line); 
}

int session_do_read(struct session *sess, struct server_str* serv)
{
    int rc;
    rc = read(sess->fd, sess->buf + sess->buf_used, BUFSIZE - sess->buf_used);
    if (rc <= 0) {
        sess->state = sess_error;
        return 0; 
    }
    sess->buf_used += rc;
    session_check_lf(sess, serv);
    if (sess->buf_used >= BUFSIZE) {
        send_string(sess, "Buffer overflowed!\n");
        sess->state = sess_error;
        return 0;
    }
    if (sess->state == sess_finish)
        return 0;
    return 1;
}



int server_init(struct server_str *serv)
{
    int sock, i;
    struct sockaddr_in addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(STDPORT);
    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("bind");
        return -1;
    }
    if (listen(sock, LQ_LEN) != 0){
        perror("listen");
        return -1;
    }
    serv->ls = sock;
    serv->sess_array = malloc(sizeof(*serv->sess_array) * SESS_SIZE);
    serv->sess_array_size = SESS_SIZE;
    for (i = 0; i < SESS_SIZE; i++)
        serv->sess_array[i] = NULL;
    return 0;
}

void server_accept_client(struct server_str *serv)
{
    int sd, i;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    sd = accept(serv->ls, (struct sockaddr*) &addr, &len);
    if (sd == -1) {
        perror("accept");
        return;
    }
    if (sd >= serv->sess_array_size) {
        int newlen = serv->sess_array_size;
        while (sd >= newlen)
            newlen += SESS_SIZE;
        serv->sess_array =
            realloc(serv->sess_array, newlen * sizeof(struct session*));
        for (i = serv->sess_array_size; i < newlen; i++)
            serv->sess_array[i] = NULL;
        serv->sess_array_size = newlen;
    }
    serv->sess_array[sd] = make_new_session(sd, &addr);
}

void remove_session(struct server_str *serv, int sd)
{
    close(sd);
    serv->sess_array[sd]->fd = -1;
    free(serv->sess_array[sd]);
    serv->sess_array[sd] = NULL;
}

int main()
{ 
    struct server_str *serv = malloc(sizeof(struct server_str));
    fd_set readfds;
    int i, selres, readres, maxfd;  
    if (server_init(serv) == -1)
        return 1;
    for (;;) {
        FD_ZERO(&readfds);
        FD_SET(serv->ls, &readfds);
        maxfd = serv->ls;
        for (i = 0; i < serv->sess_array_size; i++) {
            if (serv->sess_array[i]) {
                FD_SET(i, &readfds);
                if (i > maxfd)
                    maxfd = i;
            }
        }
        selres = select(maxfd+1, &readfds, NULL, NULL, NULL);
        if (selres == -1) {
            perror("select");
            return 2;
        }
        if (FD_ISSET(serv->ls, &readfds))
            server_accept_client(serv);
        for (i = 0; i < serv->sess_array_size; i++) {
            if (serv->sess_array[i] && FD_ISSET(i, &readfds)) {
                readres = session_do_read(serv->sess_array[i],serv);
                if (!readres)
                    remove_session(serv, i);
            }
        }
    }
    return 0;
}
