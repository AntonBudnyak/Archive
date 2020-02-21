#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct l_words {
    char *word;
    struct l_words *next;
    int sep;
};

struct pid_list {
    int pid;
    struct pid_list *next;
};

enum word_type {amp, space, enter, eof, pipee, more, less};

void print_list(struct l_words *list) 
{
    struct l_words *ptr = list;
    if (ptr == NULL)
        printf("empty list\n");
    else {
        while (ptr != NULL) {
            printf("%s\n", ptr->word);
            //printf("%i\n", ptr->sep);
            ptr = ptr->next;
        }
    }
}

void print_pid_list(struct pid_list *list) 
{
    struct pid_list *ptr = list;
    if (ptr == NULL)
        printf("empty list\n");
    else {
        while (ptr != NULL) {
            printf("%i\n", ptr->pid);
            //printf("%i\n", ptr->sep);
            ptr = ptr->next;
        }
    }
}

void del_list(struct l_words *list) 
{
    struct l_words *ptr;
    while (list != NULL) {
        ptr = list;
        list = list->next;
        free(ptr);
    }
}

struct l_words *add_word(struct l_words *list, char *word, int sep) 
{
    struct l_words *ptr = list;
    struct l_words *newword;
    newword = malloc(sizeof(struct l_words));
    newword->next = NULL;
    newword->word = word;
    newword->sep = sep;
    if (word[0] != 0){
        if (ptr == NULL)
            list = newword;
        else {
            while (ptr->next != NULL)
                ptr = ptr->next;
            ptr->next = newword;
        }
    }
    return list;
}

int is_a_sep(char c)
{
    int i=0;
    if ((c == ' ') || (c == '&') || (c == '>') || (c == '<') || (c == '|'))
        i=1;
    return i;
}

char *wordin(enum word_type *out)
{
    int i, c, txt;
    int wordlen = 20;
    char *word, *ptr;
    word = malloc(wordlen * sizeof(char) + 1);
    txt = 0;
    i = 0;
    while (((c = getchar()) != EOF) && (c != '\n') && (txt || !is_a_sep(c))) {
        if (c == '"') {
            txt = !txt;
            continue;
        }
        if (i == wordlen) {
            wordlen = 2 * wordlen;
            ptr=word;
            word = realloc(word, wordlen * sizeof(char) + 1);
            if (word == NULL) {
                free(ptr);
                fprintf(stderr, "not enought memory\n");
                break;                
            }
        }
        word[i] = c;
        i++;
    }
    switch (c) {
        case '>':
            *out = more;
            break;
        case '<':
            *out = less;
            break;
        case '&':
            *out = amp;
            break;
        case '|':
            *out = pipee;
            break;
        case ' ':
            *out = space;
            break;
        case '\n':
            *out = enter;
            break;
        default:
            *out = eof;
    }
    word[i] = 0;
    return word;
}


char **list2vec(struct l_words *list)
{
    struct l_words *ptr = list;
    int i,l = 0;
    while (ptr != NULL) {
        l++;
        ptr = ptr->next;
    }
    char **s;
    s = malloc(sizeof(char *) * (l + 1));
    ptr = list;
    for (i = 0; i < l; i ++) {
        s[i]=ptr -> word;
        ptr = ptr -> next;
    }
    s[l] = NULL;
    return s;
}

int strcd(char *str)
{
    return((str[0] == 'c') && (str[1] == 'd') && (str[2] == 0)); 
}

int cmd_is_correct(struct l_words *list, int *bg)
{
    int out_changed = 0, in_changed = 0, nc = 1;
    struct l_words *ptr = list, *prev = list;
    while (ptr) {
        if ((ptr->word[0] == '&') && (ptr->sep)) {
            if (ptr->next == NULL) {
                *bg = 1;
                free(ptr);
                prev->next = NULL;
                break;
            } else {
                printf("& missplaced\n");
                return 0;
            }
        }
        if ((ptr->word[0] == '|') && (ptr->sep)) 
            nc = -1;
        if ((ptr->word[0] == '>') && (ptr->sep)) {
            if ((out_changed) || (ptr->next == NULL) || (ptr->next->sep)) {
                printf("incorrect output changing\n");
                return 0;
            } else 
                out_changed = 1;
        }
        if ((ptr->word[0] == '<') && (ptr->sep)) {
            if ((in_changed) || (ptr->next == NULL) || (ptr->next->sep)) {
                printf("incorrect input changing\n");
                return 0;
            } else 
                in_changed = 1;
        }
        prev = ptr;
        ptr = ptr->next;
    }
    return nc;
}

void del_stream_ch(struct l_words* prev)
{
    struct l_words* next = prev->next->next->next;
    free(prev->next->next);
    free(prev->next);
    prev->next = next;
}

void l_found(struct l_words *ptr, struct l_words *prev)
{
    int fd;
    fd = open(ptr->next->word, O_RDONLY);
    if (fd == -1) {
        perror(ptr->next->word);
        exit(1);
    }
    dup2(fd,0);
    close(fd);
    del_stream_ch(prev);
}

void m_found(struct l_words *ptr, struct l_words *prev)
{
    int fd;
    if (ptr->word[1] == '>')
        fd = open(ptr->next->word, 
                   O_WRONLY | O_APPEND,
                   0666);
    else 
        fd = open(ptr->next->word,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0666);
    if (fd == -1) {
        perror(ptr->next->word);
        exit(1);
    }
    dup2(fd, 1);
    close(fd);
    del_stream_ch(prev);
}

char ***list2ev(struct l_words *list)
{
    int i = 0;
    char ***ev = malloc(10*sizeof(char *));
    struct l_words* ptr = list, * prev = list, * tmp = list;
    while (ptr) {
        if (ptr->sep) {
            switch (ptr->word[0]) {
                case '<':
                    l_found(ptr,prev);
                    break;
                case '>':
                    m_found(ptr,prev);
                    break;
                case '|':
                    prev->next = NULL;
                    if (i > sizeof(ev)/sizeof(char *)-1) 
                        ev = realloc(ev, 2 * sizeof(char *));
                    ev[i]=list2vec(tmp);
                    del_list(tmp);
                    tmp = ptr->next;
                    i++;
                    break;
                default:
                    break;
            }
        }    
        prev = ptr;
        ptr = ptr->next;        
    }
    ev[i] = list2vec(tmp);
    ev[++i] = NULL;
    return(ev);
}

struct pid_list *addpid(struct pid_list *pl, int pid)
{
    struct pid_list *newpid;
    newpid = malloc(sizeof(struct pid_list));
    newpid->pid = pid;
    newpid->next = pl;
    return (newpid);
}

void wait_circ(struct pid_list *pl, int s)
{
    struct pid_list *ptr = pl;
    int pid;
    while (s) {
        ptr = pl;
        pid = wait(NULL);
        while (ptr) {
            if (ptr->pid == pid) {
                s -= pid;
                break;
            }
            ptr = ptr->next;
        }
    }
}

void execconv(char ***ev, int bg)
{
    struct pid_list *pl = NULL;
    int fd[2], i = 0, s = 0,
        pid,d_out = dup(1), d_in = dup(0);
    while (ev[i] != NULL) {
        if (i) {
            dup2(fd[0],0);
            close(fd[0]);
        }
        if (ev[i+1]) {
            pipe(fd);
            dup2(fd[1], 1);
            close(fd[1]);
        } else {
            dup2(d_out, 1);
            close(d_out);
        }
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {
            close(fd[0]);
            execvp(ev[i][0], ev[i]);
            perror(ev[i][0]);
            exit(1);           
        }
        pl = addpid(pl, pid);
        s += pid;
        i++;
    }

    dup2(d_in, 0);
    close(d_in);
    if (!bg)
        wait_circ(pl, s);
}

void execfunc(struct l_words *list)
{   
    if (strcd(list->word) == 0) {
        int pid, zpid=0, bg = 0;
        int nc = cmd_is_correct(list, &bg);
        if (nc) {
            if (nc == -1) {
                char ***ev = list2ev(list);
                execconv(ev, bg); 
            } else {
                pid = fork();
                if (pid == -1) {
                    perror("fork");
                   return;
                }
                if (pid == 0) {
                    char ***ev = list2ev(list);
                    execvp(ev[0][0], ev[0]);
                    perror(ev[0][0]);
                    exit(1); 
                }
                if (!bg)
                    while (zpid != pid)
                        zpid = wait(NULL);
            }
        }
    } else {         
        if (list -> next != NULL){
            int res;
            res = chdir(list->next->word);
            if (res == -1) {
                perror("dir");
            }
        } else {
            fprintf(stderr, "incorrect arg of cd\n");
        }
    }
}

int main() 
{
    char *word;
    int wr=1, m = 1;
    enum word_type out = space;
    struct l_words *list = NULL;
    printf(">>> ");
    while (out != eof) {
        if (m)
            word = wordin(&out);
        else
            m = 1;
        switch (out) {
            case amp:
                list = add_word(list, word, 0);
                list = add_word(list, "&", 1);
                break;
            case more:
                list = add_word(list, word, 0);
                word = wordin(&out);
                if ((out == more) && (word[0] == 0)){
                    list = add_word(list, ">>", 1);
                } else {
                    list = add_word(list, ">", 1);
                    m = 0;
                }
                break;
            case less:
                list = add_word(list, word, 0);
                list = add_word(list, "<", 1);
                break;
            case pipee:
                list = add_word(list, word, 0);
                list = add_word(list, "|", 1);
                break;
            case space:
                list = add_word(list, word, 0);
                break;
            case enter:
                list = add_word(list, word, 0);
                while (wr > 0)
                    wr = wait4(-1, NULL, WNOHANG, 0);
                execfunc(list);
                del_list(list);
                list = NULL;
                printf(">>> ");
                break;
            case eof:
                list = add_word(list, word, 0);
                del_list(list);
                break;
        }
    }
}
