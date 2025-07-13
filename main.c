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
#define DPRINT(...) fprintf(stderr, __VA_ARGS__);
#else
#define DPRINT(...)
#endif /* DEBUG */

struct user *userList = NULL; /*Initialize the users list*/
struct new_movie *newMoviesList = NULL; /*Initialize the list of new movies*/
struct movie *categoryLists[CATEGORY_COUNT] = { NULL }; /*Initialize category-specific lists*/

void init_structures(void) {
    int i;

    /*Initialize category-specific and new movies lists*/
    newMoviesList = NULL;
    for (i = 0; i < CATEGORY_COUNT; i++) {
        categoryLists[i] = NULL;
    }

    /*Allocate memory for the sentinel node*/
    userList = (struct user *)malloc(sizeof(struct user));
    if (userList == NULL) {
        /*Handle the memory allocation error*/
        return;
    }

    /*Initialize the sentinel node*/
    userList->uid = SENTINEL_UID;
    userList->suggestedHead = NULL;
    userList->suggestedTail = NULL;
    userList->watchHistory = NULL;
    userList->next = NULL;
}

void destroy_structures(void) {
    int i; /*initialize the variable here because of ansi standard*/
	/*Free the user list*/
    while (userList != NULL) {
        struct user *tempUser = userList;
        userList = userList->next;

        /*Free suggested movie list for each user*/
        while (tempUser->suggestedHead != NULL) {
            struct suggested_movie *tempSuggested = tempUser->suggestedHead;
            tempUser->suggestedHead = tempUser->suggestedHead->next;
            free(tempSuggested);
        }

        /*Free watch history for each user*/
        while (tempUser->watchHistory != NULL) {
            struct movie *tempMovie = tempUser->watchHistory;
            tempUser->watchHistory = tempUser->watchHistory->next;
            free(tempMovie);
        }

        /*Finally free the user itself*/
        free(tempUser);
    }

    /*Free new movies list*/
    while (newMoviesList != NULL) {
        struct new_movie *tempNewMovie = newMoviesList;
        newMoviesList = newMoviesList->next;
        free(tempNewMovie);
    }

    /*Free category-specific movie lists*/
    for (i = 0; i < CATEGORY_COUNT; i++) {
        while (categoryLists[i] != NULL) {
            struct movie *tempCategoryMovie = categoryLists[i];
            categoryLists[i] = categoryLists[i]->next;
            free(tempCategoryMovie);
        }
    }
}

int main(int argc, char *argv[])
{
	FILE *event_file;
	char line_buffer[MAX_LINE];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	event_file = fopen(argv[1], "r");
	if (!event_file) {
		perror("fopen error for event file open");
		exit(EXIT_FAILURE);
	}

	init_structures();
	while (fgets(line_buffer, MAX_LINE, event_file)) {
		char *trimmed_line;
		char event;
		int uid;
		unsigned mid, year;
		movieCategory_t category1, category2;
		/*
		 * First trim any whitespace
		 * leading the line.
		 */
		trimmed_line = line_buffer;
		while (trimmed_line && isspace(*trimmed_line))
			trimmed_line++;
		if (!trimmed_line)
			continue;
		/* 
		 * Find the event,
		 * or comment starting with #
		 */
		if (sscanf(trimmed_line, "%c", &event) != 1) {
			fprintf(stderr, "Could not parse event type out of input line:\n\t%s", trimmed_line);
			fclose(event_file);
			exit(EXIT_FAILURE);
		}

		switch (event) {
			/* Comment, ignore this line */
			case '#':
				break;
			case 'R':
				if (sscanf(trimmed_line, "R %d", &uid) != 1) {
					fprintf(stderr, "Event R parsing error\n");
					break;
				}
				register_user(uid);
				break;
			case 'U':
				if (sscanf(trimmed_line, "U %d", &uid) != 1) {
					fprintf(stderr, "Event U parsing error\n");
					break;
				}
				unregister_user(uid);
				break;
			case 'A':
				if (sscanf(trimmed_line, "A %u %d %u", &mid, (int *)&category1, &year) != 3) {
					fprintf(stderr, "Event A parsing error\n");
					break;
				}
				add_new_movie(mid, category1, year);
				break;
			case 'D':
				distribute_new_movies();
				break;
			case 'W':
				if (sscanf(trimmed_line, "W %d %u", &uid, &mid) != 2) {
					fprintf(stderr, "Event W parsing error\n");
					break;
				}
				watch_movie(uid, mid);
				break;
			case 'S':
				if (sscanf(trimmed_line, "S %d", &uid) != 1) {
					fprintf(stderr, "Event S parsing error\n");
					break;
				}
				suggest_movies(uid);
				break;
			case 'F':
				if (sscanf(trimmed_line, "F %d %d %d %u", &uid, (int *)&category1, (int *)&category2, &year) != 4) {
					fprintf(stderr, "Event F parsing error\n");
					break;
				}
				filtered_movie_search(uid, category1, category2, year);
				break;
			case 'T':
				if (sscanf(trimmed_line, "T %u", &mid) != 1) {
					fprintf(stderr, "Event T parsing error\n");
					break;
				}
				take_off_movie(mid);
				break;
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
	}
	fclose(event_file);
	destroy_structures();
	return 0;
}