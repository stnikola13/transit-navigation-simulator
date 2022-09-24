#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define PI 3.14
#define R 6371

enum PATHS {none, PATHA, PATHB};

typedef struct station {
    int no, zone;
    char name[257];
    double lat, lon;
} Station;

typedef struct node {
    Station *stat;
    struct node *next;
    struct node *prev;
} Node;

Station* readLine(FILE *fileName) {
    Station *stat;
    stat = malloc(1 * sizeof(Station));
    if (stat == NULL) {
        printf("MEM_GRESKA");
        return NULL;
    }
    if (fscanf(fileName, "%d!%[^!]!%lf!%lf!%d\n", &stat->no, stat->name, &stat->lat, &stat->lon, &stat->zone) != 5) {
        free(stat); //******
        return NULL;
    }
    else return stat;
}

Node* createNode(Station *stat) {
    Node *p;
    p = malloc(1 * sizeof(Node));
    if (p == NULL) {
        printf("MEM_GRESKA");
        return NULL;
    }
    p->next = NULL;
    p->prev = NULL;
    p->stat = stat;
    return p;
}

Node* createList(FILE *fileName) {
    Node *head = NULL, *tail = head, *p;
    Station *stat;
    while (1) {
        stat = readLine(fileName);
        if (stat == NULL) {
            free(stat);
            break;
        }
        p = createNode(stat);
        if (head == NULL) {
            head = p;
            head->prev = NULL;
        }
        else {
            tail->next = p;
            p->prev = tail;
        }
        tail = p;
    }
    return head;
}

double degToRad(double degrees) {
    return (degrees * PI / 180);
}

double distance(double lat1, double lon1, double lat2, double lon2) {
    double lat1r = degToRad(lat1), lat2r = degToRad(lat2);
    double lon1r = degToRad(lon1), lon2r = degToRad(lon2);
    double t1, t2, d;

    t1 = pow(sin((lat1r - lat2r) / 2), 2);
    t2 = pow(sin((lon1r - lon2r) / 2), 2);
    d = 2 * R * asin(sqrt(t1 + t2 * cos(lat1r) * cos(lat2r)));

    return d;
}

int findClosestStation(Node *head, double lat, double lon) {
    int num = 0;
    double lat1 = lat, lon1 = lon, d, min_dist = -1;
    Node *p = head;

    while (p != NULL) {
        d = distance(lat1, lon1, p->stat->lat, p->stat->lon);
        if (min_dist == -1) {
            min_dist = d;
            num = p->stat->no;
        }
        else if (d < min_dist) {
            min_dist = d;
            num = p->stat->no;
        }
        p = p->next;
    }
    return num;
}

Node* findPath(Node* pathA, Node* pathB, double lat1, double lon1, double lat2, double lon2, enum PATHS *path) {
    Node *p, *path_head = NULL, *path_tail = path_head, *q;
    int startStation, endStation;
    enum PATHS start = none, end = none;

    startStation = findClosestStation(pathA, lat1, lon1);
    endStation = findClosestStation(pathA, lat2, lon2);
    for (p = pathA; p != NULL; p = p->next) {
        if (p->stat->no == startStation && start == none) start = PATHA;
        else if (p->stat->no == endStation && start == PATHA) end = PATHA;
    }
    if (start == PATHA && end == PATHA) {
        for (p = pathA; p->stat->no != startStation; p = p->next);
        for (; p->stat->no != endStation; p = p->next) {
            q = createNode(p->stat);
            if (path_head == NULL) {
                path_head = q;
                path_head->prev = NULL;
            } else {
                path_tail->next = q;
                q->prev = path_tail;
            }
            path_tail = q;
        }
        q = createNode(p->stat);
        if (path_head == NULL) {
            path_head = q;
            path_head->prev = NULL;
        } else {
            path_tail->next = q;
            q->prev = path_tail;
        }
        path_tail = q;
        *path = PATHA;
        return path_head;
    }

    start = none;
    end = none;

    startStation = findClosestStation(pathB, lat1, lon1);
    endStation = findClosestStation(pathB, lat2, lon2);
    for (p = pathB; p != NULL; p = p->next) {
        if (p->stat->no == startStation && start == none) start = PATHB;
        else if (p->stat->no == endStation && start == PATHB) end = PATHB;
    }
    if (start == PATHB && end == PATHB) {
        for (p = pathB; p->stat->no != startStation; p = p->next);
        for (; p->stat->no != endStation; p = p->next) {
            q = createNode(p->stat);
            if (path_head == NULL) {
                path_head = q;
                path_head->prev = NULL;
            } else {
                path_tail->next = q;
                q->prev = path_tail;
            }
            path_tail = q;
        }
        q = createNode(p->stat);
        if (path_head == NULL) {
            path_head = q;
            path_head->prev = NULL;
        } else {
            path_tail->next = q;
            q->prev = path_tail;
        }
        path_tail = q;
        *path = PATHB;
        return path_head;
    }
    return NULL;
}

void createDataFile(FILE *name, Node *head, char *line, char *direction) {
    Node *p;
    fprintf(name, "%s!%s\n", line, direction);
    for (p = head; p != NULL; p = p->next) {
        fprintf(name, "%d!%s!%lf!%lf!%d\n", p->stat->no, p->stat->name,
                p->stat->lat, p->stat->lon, p->stat->zone);
    }
}

void deallocate(Node* head) {
    Node *p, *q;
    q = NULL;
    p = head;
    while (p != NULL) {
        q = p;
        p = p->next;
        free(q->stat);
        free(q);
    }
}

void deallocateList(Node* head) {
    Node *p, *q;
    q = NULL;
    p = head;
    while (p != NULL) {
        q = p;
        p = p->next;
        free(q);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 7) {
        printf("ARG_GRESKA");
        return 0;
    }

    char *output_name = argv[1];
    char *dirAstr = "_dirA.txt";
    char *dirBstr = "_dirB.txt";

    double lat1, lon1, lat2, lon2;
    lat1 = atof(argv[2]);
    lon1 = atof(argv[3]);
    lat2 = atof(argv[4]);
    lon2 = atof(argv[5]);

    FILE *output = fopen(output_name, "w");
    if (output == NULL) {
        printf("DAT_GRESKA");
        return 0;
    }

    for (int j = 6; j < argc; j++) {
        char *line, *pathDir = "", *data1, *data2;
        enum PATHS chosen_path = none;
        Node *head1, *head2, *path;

        data1 = malloc(15 * sizeof(char));
        data2 = malloc(15 * sizeof(char));
        if (data1 == NULL || data2 == NULL) {
            printf("MEM_GRESKA");
            return 0;
        }

        data1 = strcpy(data1, argv[j]);
        data2 = strcpy(data2, argv[j]);
        data1 = strcat(data1, dirAstr);
        data2 = strcat(data2, dirBstr);

        FILE *input1 = fopen(data1, "r");
        if (input1 == NULL) {
            printf("DAT_GRESKA");
            return 0;
        }
        FILE *input2 = fopen(data2, "r");
        if (input2 == NULL) {
            printf("DAT_GRESKA");
            return 0;
        }

        head1 = createList(input1);
        head2 = createList(input2);

        path = findPath(head1, head2, lat1, lon1, lat2, lon2, &chosen_path);

        //Informacije
        line = argv[j];
        switch (chosen_path) {
            case PATHA:
                pathDir = "A\0"; break;
            case PATHB:
                pathDir = "B\0"; break;
            case none: break;
        }

        createDataFile(output, path, line, pathDir);
        deallocate(head1);
        deallocate(head2);
        deallocateList(path);
        free(data1);
        free(data2);
        fclose(input1);
        fclose(input2);
    }

    fclose(output);
    return 0;
}