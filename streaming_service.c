#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "streaming_service.h"

/*functions to help the control flow*/
/*Function to check if a user already exists in the linked list*/
int user_exists(int uid) {
    struct user* current = userList;
    while (current && current->uid != SENTINEL_UID) {
        if (current->uid == uid) {
            return 1; /* user already exists */
        }
        current = current->next;
    }
    return 0; /* user does not exist */
}

/* The function find_user_by_uid searches for a user in a linked list based on their UID and returns a pointer to the user if found, or NULL if not found.*/
struct user* find_user_by_uid(int uid) {
    struct user* current = userList;

    while (current && current->uid != SENTINEL_UID) {
        if (current->uid == uid) {
            return current; /*User with matching UID found*/
        }
        current = current->next;
    }

    return NULL; /*User with the specified UID was not found*/
}

/*The function get_category_name takes a movie category as input and returns the corresponding category name as a string*/
const char* get_category_name(movieCategory_t category) {
    switch (category) {
        case HORROR: 
            return "Horror";
        case SCIFI: 
            return "Sci-Fi";
        case DRAMA: 
            return "Drama";
        case ROMANCE: 
            return "Romance";
        case DOCUMENTARY: 
            return "Documentary";
        case COMEDY: 
            return "Comedy";
        default: 
            return "Unknown"; /*Default case*/
    }
}

/*The function creates a linked list of suggested movies based on a given category and year.*/
struct suggested_movie* create_suggested_movie_list(movieCategory_t category, unsigned year) {
    struct suggested_movie *head = NULL;
    struct suggested_movie *tail = NULL;
    struct movie *currentMovie = categoryLists[category];
    while(currentMovie != NULL) {
        if(currentMovie->info.year >= year) {
            struct suggested_movie *newNode = (struct suggested_movie *)malloc(sizeof(struct suggested_movie));
            newNode->info = currentMovie->info;
            newNode->next = NULL;
            newNode->prev = tail;
            if (tail) {
                tail->next = newNode;
            } else {
                head = newNode;
            }
            tail = newNode;
        }
        currentMovie = currentMovie->next;
    }
    return head;
}

/*The function adds a suggested movie to a user's list of suggested movies.*/
void add_suggested_movie_to_user(struct user *user, struct suggested_movie *suggestion) {
    struct suggested_movie *newNode = (struct suggested_movie *)malloc(sizeof(struct suggested_movie));
    newNode->info = suggestion->info;
    newNode->next = NULL;
    newNode->prev = user->suggestedTail;
    if (user->suggestedTail) {
        user->suggestedTail->next = newNode;
    } else {
        user->suggestedHead = newNode;
    }
    user->suggestedTail = newNode;
}

/*The function merges two linked lists of suggested movies based on their movie IDs.*/
struct suggested_movie* merge_suggested_movie_lists(struct suggested_movie *list1, struct suggested_movie *list2) {
    struct suggested_movie *mergedHead = NULL, *mergedTail = NULL;

    while (list1 != NULL && list2 != NULL) {
        struct suggested_movie **nodeToAdd = NULL;
        if (list1->info.mid < list2->info.mid) {
            nodeToAdd = &list1;
        } else {
            nodeToAdd = &list2;
        }

        if (mergedTail) {
            mergedTail->next = *nodeToAdd;
            (*nodeToAdd)->prev = mergedTail;
            mergedTail = mergedTail->next;
        } else {
            mergedHead = *nodeToAdd;
            mergedTail = mergedHead;
        }

        *nodeToAdd = (*nodeToAdd)->next;
    }

    /*Append any remaining elements of list1 or list2*/
    if (list1) {
        if (mergedTail) {
            mergedTail->next = list1;
            list1->prev = mergedTail;
        } else {
            mergedHead = list1;
        }
    } else if (list2) {
        if (mergedTail) {
            mergedTail->next = list2;
            list2->prev = mergedTail;
        } else {
            mergedHead = list2;
        }
    }

    return mergedHead;
}

/*printing functions*/
/*print categorized movies list*/
void print_categorized_movies(){
    movieCategory_t category;
    int position;
    for (category = HORROR; category <= COMEDY; category++) {
        /*Get the category name using get_category_name function*/
        const char* categoryName = get_category_name(category);
        printf("%s: ", categoryName);

        /*Traverse the movie list for the current category*/
        struct movie* current = categoryLists[category];
        position = 1;
        while (current != NULL) {
            /*Print movie ID and category, followed by a comma if not the last movie*/
            printf("<%d,%d>", current->info.mid, position);
            if (current->next != NULL) {
                printf(", ");
            }
            current = current->next; /*Move to the next movie in the category*/
            position ++;
        }

        printf("\n"); /*Print a newline to separate categories*/
    }
    printf("DONE\n");
}

/*print users list*/
void print_users_list(void){
    printf("Users = ");
    struct user *current = userList;
    while (current-> uid != SENTINEL_UID) {
        printf("<%d>,", current->uid);
        current = current->next;
    }
}

/*Starting the fucntions for the events*/
/*Event R- Function to register a new user and add them to the linked list*/
int register_user(int uid) {
    if (user_exists(uid)) {
        printf("\nThe user with uid %d already exists.\n", uid);
        return -1;
    }

    struct user *newUser = malloc(sizeof(*newUser));
    if (!newUser) {
        printf("\nMemory allocation failed.\n");
        return -1;
    }

    newUser->uid = uid;
    newUser->suggestedHead = NULL;
    newUser->suggestedTail = NULL;
    newUser->watchHistory = NULL;
    newUser->next = userList;
    userList = newUser;

    printf("R <%d>\n", uid);
    print_users_list();
    printf("\nDone\n");
    return 0;
}

/*Event U- Function to unregister a user and remove them from the linked list*/
void unregister_user(int uid) {
    struct user *current = userList;
    struct user *prev = NULL;
    /*Search for the user with the given UID*/
    if (!user_exists(uid)) {
        printf("\nUser with UID %d not found.\n", uid);
        return;
    }
    while (current && current->uid != SENTINEL_UID) {
        if (current->uid == uid) {
            /*Remove the user from the linked list*/
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                userList = current->next;
            }
            printf("U %d\n", uid);

            while (current->suggestedHead) {
                struct suggested_movie *tmp = current->suggestedHead;
                current->suggestedHead = tmp->next;
                free(tmp);
            }
            while (current->watchHistory) {
                struct movie *tmp = current->watchHistory;
                current->watchHistory = tmp->next;
                free(tmp);
            }
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }
    /*Print the updated list of users*/
    print_users_list();
    printf("\nDone\n");
}

/*Event A- Function to add a new movie to the sorted list of new releases*/
int add_new_movie(unsigned mid, movieCategory_t category, unsigned year) {
    struct new_movie *newMovie = (struct new_movie *)malloc(sizeof(struct new_movie));
    if (newMovie == NULL) {
        return -1; /*Memory allocation failed, return -1*/
    }
    newMovie->info.mid = mid;
    newMovie->category = category; 
    newMovie->info.year = year;
    newMovie->next = NULL;

    struct new_movie *current = newMoviesList;
    struct new_movie *prev = NULL;

    /*Find the correct position to insert the new movie in sorted order*/
    while (current != NULL && current->info.mid < mid) {
        prev = current;
        current = current->next;
    }

    if (prev == NULL) {
        /*Insert at the beginning of the list*/
        newMovie->next = newMoviesList;
        newMoviesList = newMovie;
    } else {
        /*Insert in the middle or at the end of the list*/
        prev->next = newMovie;
        newMovie->next = current;
    }

    printf("A <%u> <%d> <%u>\n", mid, category, year); /*Print the added new movie*/
    printf("New movies = "); /*print the list with updated list with the movies*/
    struct new_movie *temp = newMoviesList;
    while (temp != NULL) {
        printf(" <%u,%d,%u>", temp->info.mid, temp->category, temp->info.year);
        temp = temp->next;
    }
    printf("\nDONE\n");
    return 0;
}

/*Event D- The function distribute_new_movies categorizes new movies and inserts them into the appropriate category list.*/
void distribute_new_movies(void) {
    struct new_movie *current = newMoviesList;

    while (current != NULL) {
        int category = current->category;
        struct movie *newMovie = (struct movie *)malloc(sizeof(struct movie));
        newMovie->info = current->info;
        newMovie->next = NULL;

        struct movie *currentCategory = categoryLists[category];
        struct movie *prevCategory = NULL;

        /* Find the correct position to insert the new movie in the category list */
        while (currentCategory != NULL && currentCategory->info.mid < current->info.mid) {
            prevCategory = currentCategory;
            currentCategory = currentCategory->next;
        }

        if (prevCategory == NULL) {
            /* Insert at the beginning of the category list */
            newMovie->next = categoryLists[category];
            categoryLists[category] = newMovie;
        } else {
            /* Insert in the middle or at the end of the category list */
            prevCategory->next = newMovie;
            newMovie->next = currentCategory;
        }

        /* Move to the next new movie */
        struct new_movie *temp = current;
        current = current->next;
        free(temp);
    }
    newMoviesList = NULL;
    printf("D\nCategorized Movies:\n");
    print_categorized_movies();
}

/*Event W- Function for the user to wantch a movie*/
int watch_movie(int uid, unsigned mid) {
    struct user* user = find_user_by_uid(uid); /* Find the user with the specified uid */

    if (user == NULL) {
        printf("User %d does not exist\n", uid);
        return -1; /* User with the specified UID does not exist */
    }

    /* Create a new movie structure and initialize it */
    struct movie* new_movie = (struct movie*)malloc(sizeof(struct movie));
    if (new_movie == NULL) {
        return -1; /* Memory allocation failed */
    }
    new_movie->info.mid = mid;
    new_movie->info.year = 0;
    new_movie->next = NULL;

    /* Add the new movie to the top of the user's watch history stack */
    new_movie->next = user->watchHistory;
    user->watchHistory = new_movie;

    /* Print the watch history */
    printf("W <%d>, <%d>\n", uid, mid);
    printf("User %d Watch History = ", uid);
    struct movie* current_movie = user->watchHistory;
    while (current_movie != NULL) {
        printf("%d", current_movie->info.mid);
        current_movie = current_movie->next;
        if (current_movie != NULL) {
            printf(", ");
        }
    }
    printf("\nDONE\n");
    return 0; /* Successfully added the movie to the watch history and printed the history */
}

/* Event S- Function to suggest movies to user */
int suggest_movies(int uid){
    int counter = 1;
    struct user *current = userList;
    struct movie_info i;       
    while(current->uid != SENTINEL_UID){
        if(current->uid == uid){
            break;
        }
        current = current->next;
    }

    if(current->uid != uid){
        printf("User not found.\n");
        return -1;
    }

    struct suggested_movie *currFront = current->suggestedHead;
    struct suggested_movie *currBack = current->suggestedTail;
    struct user *temp = userList;

    while(temp->uid != SENTINEL_UID){
        if(temp->uid != uid){
            if(temp->watchHistory != NULL){
                struct movie *topMovie = temp->watchHistory;
                i = topMovie->info;
                struct suggested_movie *suggestedMovieNode = (struct suggested_movie *)malloc(sizeof(struct suggested_movie));
                if(suggestedMovieNode == NULL){
                    printf("Could not allocate memory");
                    return -1;
                }
                suggestedMovieNode->info = i;
                if(counter % 2 != 0){ /*Pseudocode from the tutorial*/
                    if(currFront != NULL){
                        currFront->next = suggestedMovieNode;
                    }
                    suggestedMovieNode->next = NULL;
                    suggestedMovieNode->prev = currFront;
                    currFront = suggestedMovieNode;
                    if(current->suggestedHead == NULL){
                        current->suggestedHead = currFront;
                    }
                } else {
                    suggestedMovieNode->prev = NULL;
                    suggestedMovieNode->next = currBack;
                    if(currBack != NULL){
                        currBack->prev = suggestedMovieNode;
                    }
                    currBack = suggestedMovieNode;
                    if(current->suggestedTail == NULL){
                        current->suggestedTail = currBack;
                    }
                }
                counter++;
            }
        }
        temp = temp->next;
    }
    if(currFront != NULL){
        if(currBack != NULL){
            currFront->next = currBack;
            currBack->prev = currFront;
        } else {
            currBack = currFront;
        }
    }
    
    struct suggested_movie *suggestedMovieIterator = current->suggestedHead;
    printf("\nS <%d>\n", uid);
    printf("User <%d> Suggested Movies = ", uid);
    while(suggestedMovieIterator != NULL){
        printf("<%d>", suggestedMovieIterator->info.mid);
        if(suggestedMovieIterator->next != NULL){
            printf(", ");
        }
        suggestedMovieIterator = suggestedMovieIterator->next;
    }
    printf("\nDONE\n");
    return 0;
}

/*Event F- filterd movie search*/
int filtered_movie_search(int uid, movieCategory_t category1, movieCategory_t category2, unsigned year) {
    printf("F %d %d %d %u\n", uid, category1, category2, year);
    struct user* user = find_user_by_uid(uid); /* Find the user with the specified uid */
    if (user == NULL) {
        printf("User %d does not exist\n", uid);
        return -1; /* User with the specified UID does not exist */
    }
    
    struct suggested_movie *first_category_suggestions = create_suggested_movie_list(category1, year);
    struct suggested_movie *second_category_suggestions = create_suggested_movie_list(category2, year);

    if (first_category_suggestions == NULL && second_category_suggestions == NULL) {
        printf("No suggestions available.\n");
    } else if (first_category_suggestions == NULL) {
        while(second_category_suggestions != NULL) {
            add_suggested_movie_to_user(user, second_category_suggestions);
            second_category_suggestions = second_category_suggestions->next;
        }
    } else if (second_category_suggestions == NULL) {
        while(first_category_suggestions != NULL) {
            add_suggested_movie_to_user(user, first_category_suggestions);
            first_category_suggestions = first_category_suggestions->next;
        }
    } else {
        struct suggested_movie *merged_suggestions = merge_suggested_movie_lists(first_category_suggestions, second_category_suggestions);
        while(merged_suggestions != NULL) {
            add_suggested_movie_to_user(user, merged_suggestions);
            merged_suggestions = merged_suggestions->next;
        }
    }
    printf("User <%d> Suggested Movies = ", user->uid);
    struct suggested_movie *currentSuggestedMovie = user->suggestedHead;
    while (currentSuggestedMovie != NULL) {
        printf("<%d>", currentSuggestedMovie->info.mid);
        if (currentSuggestedMovie->next != NULL) {
            printf(", ");
        }
        currentSuggestedMovie = currentSuggestedMovie->next;
    }
    printf("\nDONE\n");
return 0;
}

/*Event T- takeoff a movie from the service*/
void take_off_movie(unsigned mid) {
    int i; /*initialize the variable here because of the ansi standard*/
    int position;
    printf("T %u\n", mid);

    /* Step 1: Remove the movie from every user's suggested list */
    struct user* current_user = userList;
    while (current_user != NULL) {
        struct suggested_movie* suggested = current_user->suggestedHead;
        struct suggested_movie* prev_suggested = NULL;

        while (suggested != NULL) {
            if (suggested->info.mid == mid) {
                printf("%u removed from %u suggested list.\n", mid, current_user->uid);

                if (prev_suggested != NULL) {
                    prev_suggested->next = suggested->next;
                } else {
                    current_user->suggestedHead = suggested->next;
                }
                break;
            }

            prev_suggested = suggested;
            suggested = suggested->next;
        }

        current_user = current_user->next;
    }

    /* Step 2: Remove the movie from the corresponding category list */
    for (i = 0; i < CATEGORY_COUNT; i++) {
        struct movie* current_movie = categoryLists[i];
        struct movie* prev_movie = NULL;

        while (current_movie != NULL) {
            if (current_movie->info.mid == mid) {
                if (prev_movie != NULL) {
                    prev_movie->next = current_movie->next;
                } else {
                    categoryLists[i] = current_movie->next;
                }

                printf("%u removed from %s category list.\n", mid, get_category_name(i));
                position = i;        
                break;
            }

            prev_movie = current_movie;
            current_movie = current_movie->next;
        }
    }
    /* Printing the remaining movies in the category list */
    printf("Category list %d = ", position);
    struct movie* temp_movie = categoryLists[position];
    while (temp_movie != NULL) {
        printf("%u, ", temp_movie->info.mid);
        temp_movie = temp_movie->next;
    }
    printf("\nDONE\n");
}

/*Event M- Function to print information about movies in category lists*/
void print_movies(void) {
    printf("M\nCategorized Movies:\n");
    print_categorized_movies();
}

/*Event P- Function to print information about users and their suggested movies and watch history*/
void print_users() {
    printf("P\nUsers:\n");

    struct user* current = userList;
    while (current-> uid != SENTINEL_UID) {
        printf("<%d>:\nSuggested: ", current->uid);

        /*Print suggested movies*/
        struct suggested_movie* suggested = current->suggestedHead;
        while (suggested != NULL) {
            printf("<%u,%d>, ", suggested->info.mid, suggested->info.year);
            suggested = suggested->next;
        }

        /*Print watch history*/
        printf("\nWatch History: ");
        struct movie* watchHistory = current->watchHistory;
        while (watchHistory != NULL) {
            printf("<%u,%d>, ", watchHistory->info.mid, watchHistory->info.year);
            watchHistory = watchHistory->next;
        }

        printf("\n");
        current = current->next;
    }

    printf("DONE\n");
}
