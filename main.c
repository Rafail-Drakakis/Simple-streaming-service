/*
 * ============================================
 * file: main.c
 * @Author John Malliotakis (jmal@csd.uoc.gr)
 * @Version 23/10/2023
 *
 * @e-mail hy240@csd.uoc.gr
 *
 * @brief Main function
 *        for CS240 Project Phase 1,
 *        Winter Semester 2023-2024
 * @see   Compile using supplied Makefile by running: make
 * ============================================
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "streaming_service.h"

/* Maximum input line size */
#define MAX_LINE 1024

/* 
 * Uncomment the following line to
 * enable debugging prints
 * or comment to disable it
 */
/* #define DEBUG */
#ifdef DEBUG
#define DPRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DPRINT(...)
#endif /* DEBUG */

/* Global data structures */
struct user *userList = NULL;
struct new_movie *newMoviesList = NULL;
struct movie *categoryLists[CATEGORY_COUNT] = { NULL };

/**
 * Initialize all data structures
 */
void init_structures(void) {
    int i;

    /* Initialize category-specific and new movies lists */
    newMoviesList = NULL;
    for (i = 0; i < CATEGORY_COUNT; i++) {
        categoryLists[i] = NULL;
    }

    /* Allocate memory for the sentinel node */
    userList = (struct user *)malloc(sizeof(struct user));
    if (userList == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for sentinel node\n");
        exit(EXIT_FAILURE);
    }

    /* Initialize the sentinel node */
    userList->uid = SENTINEL_UID;
    userList->suggestedHead = NULL;
    userList->suggestedTail = NULL;
    userList->watchHistory = NULL;
    userList->next = NULL;
}

/**
 * Clean up and free all allocated memory
 */
void destroy_structures(void) {
    int i;
    
    /* Free the user list */
    while (userList != NULL) {
        struct user *tempUser = userList;
        userList = userList->next;

        /* Free suggested movie list for each user */
        while (tempUser->suggestedHead != NULL) {
            struct suggested_movie *tempSuggested = tempUser->suggestedHead;
            tempUser->suggestedHead = tempUser->suggestedHead->next;
            free(tempSuggested);
        }

        /* Free watch history for each user */
        while (tempUser->watchHistory != NULL) {
            struct movie *tempMovie = tempUser->watchHistory;
            tempUser->watchHistory = tempUser->watchHistory->next;
            free(tempMovie);
        }

        /* Finally free the user itself */
        free(tempUser);
    }

    /* Free new movies list */
    while (newMoviesList != NULL) {
        struct new_movie *tempNewMovie = newMoviesList;
        newMoviesList = newMoviesList->next;
        free(tempNewMovie);
    }

    /* Free category-specific movie lists */
    for (i = 0; i < CATEGORY_COUNT; i++) {
        while (categoryLists[i] != NULL) {
            struct movie *tempCategoryMovie = categoryLists[i];
            categoryLists[i] = categoryLists[i]->next;
            free(tempCategoryMovie);
        }
    }
}

/**
 * Trim leading whitespace from a string
 * @param line String to trim
 * @return Pointer to first non-whitespace character, or NULL if all whitespace
 */
static char* trim_leading_whitespace(char* line) {
    if (line == NULL) {
        return NULL;
    }
    
    while (*line != '\0' && isspace((unsigned char)*line)) {
        line++;
    }
    
    if (*line == '\0') {
        return NULL;
    }
    
    return line;
}

/**
 * Parse and handle event R - Register user
 * @param line Input line to parse
 * @return 0 on success, -1 on error
 */
static int handle_register_user(const char* line) {
    int uid;
    if (sscanf(line, "R %d", &uid) != 1) {
        fprintf(stderr, "Event R parsing error\n");
        return -1;
    }
    return register_user(uid);
}

/**
 * Parse and handle event U - Unregister user
 * @param line Input line to parse
 * @return 0 on success, -1 on error
 */
static int handle_unregister_user(const char* line) {
    int uid;
    if (sscanf(line, "U %d", &uid) != 1) {
        fprintf(stderr, "Event U parsing error\n");
        return -1;
    }
    unregister_user(uid);
    return 0;
}

/**
 * Parse and handle event A - Add new movie
 * @param line Input line to parse
 * @return 0 on success, -1 on error
 */
static int handle_add_new_movie(const char* line) {
    unsigned mid, year;
    int category_int;
    movieCategory_t category;
    
    if (sscanf(line, "A %u %d %u", &mid, &category_int, &year) != 3) {
        fprintf(stderr, "Event A parsing error\n");
        return -1;
    }
    
    category = (movieCategory_t)category_int;
    if (category < HORROR || category > COMEDY) {
        fprintf(stderr, "Event A: Invalid category %d\n", category_int);
        return -1;
    }
    
    return add_new_movie(mid, category, year);
}

/**
 * Parse and handle event W - Watch movie
 * @param line Input line to parse
 * @return 0 on success, -1 on error
 */
static int handle_watch_movie(const char* line) {
    int uid;
    unsigned mid;
    
    if (sscanf(line, "W %d %u", &uid, &mid) != 2) {
        fprintf(stderr, "Event W parsing error\n");
        return -1;
    }
    return watch_movie(uid, mid);
}

/**
 * Parse and handle event S - Suggest movies
 * @param line Input line to parse
 * @return 0 on success, -1 on error
 */
static int handle_suggest_movies(const char* line) {
    int uid;
    
    if (sscanf(line, "S %d", &uid) != 1) {
        fprintf(stderr, "Event S parsing error\n");
        return -1;
    }
    return suggest_movies(uid);
}

/**
 * Parse and handle event F - Filtered movie search
 * @param line Input line to parse
 * @return 0 on success, -1 on error
 */
static int handle_filtered_search(const char* line) {
    int uid;
    int category1_int, category2_int;
    movieCategory_t category1, category2;
    unsigned year;
    
    if (sscanf(line, "F %d %d %d %u", &uid, &category1_int, &category2_int, &year) != 4) {
        fprintf(stderr, "Event F parsing error\n");
        return -1;
    }
    
    category1 = (movieCategory_t)category1_int;
    category2 = (movieCategory_t)category2_int;
    
    if (category1 < HORROR || category1 > COMEDY ||
        category2 < HORROR || category2 > COMEDY) {
        fprintf(stderr, "Event F: Invalid category\n");
        return -1;
    }
    
    return filtered_movie_search(uid, category1, category2, year);
}

/**
 * Parse and handle event T - Take off movie
 * @param line Input line to parse
 * @return 0 on success, -1 on error
 */
static int handle_take_off_movie(const char* line) {
    unsigned mid;
    
    if (sscanf(line, "T %u", &mid) != 1) {
        fprintf(stderr, "Event T parsing error\n");
        return -1;
    }
    take_off_movie(mid);
    return 0;
}

/**
 * Process a single event line
 * @param line Input line to process
 * @return 0 on success, -1 on error
 */
static int process_event(const char* line) {
    char event;
    
    if (line == NULL || *line == '\0') {
        return 0; /* Empty line, skip */
    }
    
    /* Find the event type */
    if (sscanf(line, "%c", &event) != 1) {
        fprintf(stderr, "Could not parse event type out of input line:\n\t%s", line);
        return -1;
    }

    switch (event) {
        case '#':
            /* Comment, ignore this line */
            break;
            
        case 'R':
            return handle_register_user(line);
            
        case 'U':
            return handle_unregister_user(line);
            
        case 'A':
            return handle_add_new_movie(line);
            
        case 'D':
            distribute_new_movies();
            break;
            
        case 'W':
            return handle_watch_movie(line);
            
        case 'S':
            return handle_suggest_movies(line);
            
        case 'F':
            return handle_filtered_search(line);
            
        case 'T':
            return handle_take_off_movie(line);
            
        case 'M':
            print_movies();
            break;
            
        case 'P':
            print_users();
            break;
            
        default:
            fprintf(stderr, "WARNING: Unrecognized event %c. Continuing...\n", event);
            break;
    }
    
    return 0;
}

/**
 * Main function
 * @param argc Argument count
 * @param argv Argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
int main(int argc, char *argv[]) {
    FILE *event_file;
    char line_buffer[MAX_LINE];
    int line_number = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    event_file = fopen(argv[1], "r");
    if (event_file == NULL) {
        fprintf(stderr, "Error: Cannot open file '%s': %s\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    init_structures();
    
    while (fgets(line_buffer, MAX_LINE, event_file)) {
        char *trimmed_line;
        line_number++;
        
        /* Remove trailing newline if present */
        size_t len = strlen(line_buffer);
        if (len > 0 && line_buffer[len - 1] == '\n') {
            line_buffer[len - 1] = '\0';
        }
        
        /* Trim leading whitespace */
        trimmed_line = trim_leading_whitespace(line_buffer);
        if (trimmed_line == NULL) {
            continue; /* Empty or whitespace-only line */
        }
        
        DPRINT("Processing line %d: %s\n", line_number, trimmed_line);
        
        if (process_event(trimmed_line) != 0) {
            fprintf(stderr, "Error processing line %d: %s\n", line_number, trimmed_line);
            /* Continue processing other lines */
        }
    }
    
    if (ferror(event_file)) {
        fprintf(stderr, "Error reading from file '%s'\n", argv[1]);
        fclose(event_file);
        destroy_structures();
        return EXIT_FAILURE;
    }
    
    fclose(event_file);
    destroy_structures();
    return EXIT_SUCCESS;
}
