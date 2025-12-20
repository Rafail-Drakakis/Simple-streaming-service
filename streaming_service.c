#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "streaming_service.h"

/* ============================================
 * Helper Functions
 * ============================================ */

/**
 * Check if a user already exists in the linked list
 * @param uid User ID to check
 * @return 1 if user exists, 0 otherwise
 */
int user_exists(int uid) {
    struct user* current = userList;
    
    if (current == NULL) {
        return 0;
    }
    
    while (current && current->uid != SENTINEL_UID) {
        if (current->uid == uid) {
            return 1; /* User already exists */
        }
        current = current->next;
    }
    return 0; /* User does not exist */
}

/**
 * Find a user by UID
 * @param uid User ID to search for
 * @return Pointer to user if found, NULL otherwise
 */
struct user* find_user_by_uid(int uid) {
    struct user* current = userList;

    if (current == NULL) {
        return NULL;
    }

    while (current && current->uid != SENTINEL_UID) {
        if (current->uid == uid) {
            return current; /* User with matching UID found */
        }
        current = current->next;
    }

    return NULL; /* User with the specified UID was not found */
}

/**
 * Get category name as string
 * @param category Movie category enum value
 * @return Category name string
 */
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
            return "Unknown";
    }
}

/**
 * Free a suggested movie list
 * @param head Head of the suggested movie list to free
 */
static void free_suggested_movie_list(struct suggested_movie* head) {
    while (head != NULL) {
        struct suggested_movie* temp = head;
        head = head->next;
        free(temp);
    }
}

/**
 * Free a movie list
 * @param head Head of the movie list to free
 */
static void free_movie_list(struct movie* head) {
    while (head != NULL) {
        struct movie* temp = head;
        head = head->next;
        free(temp);
    }
}

/**
 * Create a suggested movie list based on category and year filter
 * @param category Movie category to filter
 * @param year Minimum year filter
 * @return Head of the created suggested movie list
 */
struct suggested_movie* create_suggested_movie_list(movieCategory_t category, unsigned year) {
    struct suggested_movie *head = NULL;
    struct suggested_movie *tail = NULL;
    struct movie *currentMovie = categoryLists[category];
    
    while (currentMovie != NULL) {
        if (currentMovie->info.year >= year) {
            struct suggested_movie *newNode = (struct suggested_movie *)malloc(sizeof(struct suggested_movie));
            if (newNode == NULL) {
                /* Memory allocation failed, free what we've created so far */
                free_suggested_movie_list(head);
                return NULL;
            }
            
            newNode->info = currentMovie->info;
            newNode->next = NULL;
            newNode->prev = tail;
            
            if (tail != NULL) {
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

/**
 * Add a suggested movie to a user's suggested movies list
 * @param user User to add suggestion to
 * @param suggestion Suggested movie to add
 */
void add_suggested_movie_to_user(struct user *user, struct suggested_movie *suggestion) {
    if (user == NULL || suggestion == NULL) {
        return;
    }
    
    struct suggested_movie *newNode = (struct suggested_movie *)malloc(sizeof(struct suggested_movie));
    if (newNode == NULL) {
        return; /* Memory allocation failed */
    }
    
    newNode->info = suggestion->info;
    newNode->next = NULL;
    newNode->prev = user->suggestedTail;
    
    if (user->suggestedTail != NULL) {
        user->suggestedTail->next = newNode;
    } else {
        user->suggestedHead = newNode;
    }
    user->suggestedTail = newNode;
}

/**
 * Merge two sorted suggested movie lists
 * @param list1 First sorted list
 * @param list2 Second sorted list
 * @return Head of merged list
 */
struct suggested_movie* merge_suggested_movie_lists(struct suggested_movie *list1, struct suggested_movie *list2) {
    struct suggested_movie *mergedHead = NULL;
    struct suggested_movie *mergedTail = NULL;

    while (list1 != NULL && list2 != NULL) {
        struct suggested_movie **nodeToAdd = NULL;
        
        if (list1->info.mid < list2->info.mid) {
            nodeToAdd = &list1;
        } else {
            nodeToAdd = &list2;
        }

        if (mergedTail != NULL) {
            mergedTail->next = *nodeToAdd;
            (*nodeToAdd)->prev = mergedTail;
            mergedTail = mergedTail->next;
        } else {
            mergedHead = *nodeToAdd;
            mergedTail = mergedHead;
        }

        *nodeToAdd = (*nodeToAdd)->next;
    }

    /* Append any remaining elements */
    if (list1 != NULL) {
        if (mergedTail != NULL) {
            mergedTail->next = list1;
            list1->prev = mergedTail;
        } else {
            mergedHead = list1;
        }
    } else if (list2 != NULL) {
        if (mergedTail != NULL) {
            mergedTail->next = list2;
            list2->prev = mergedTail;
        } else {
            mergedHead = list2;
        }
    }

    return mergedHead;
}

/* ============================================
 * Printing Functions
 * ============================================ */

/**
 * Print categorized movies list
 */
void print_categorized_movies(void) {
    movieCategory_t category;
    int position;
    
    for (category = HORROR; category <= COMEDY; category++) {
        const char* categoryName = get_category_name(category);
        printf("%s: ", categoryName);

        struct movie* current = categoryLists[category];
        position = 1;
        
        while (current != NULL) {
            printf("<%d,%d>", current->info.mid, position);
            if (current->next != NULL) {
                printf(", ");
            }
            current = current->next;
            position++;
        }

        printf("\n");
    }
    printf("DONE\n");
}

/**
 * Print users list
 */
void print_users_list(void) {
    printf("Users = ");
    struct user *current = userList;
    
    if (current == NULL) {
        return;
    }
    
    while (current->uid != SENTINEL_UID) {
        printf("<%d>", current->uid);
        current = current->next;
        if (current != NULL && current->uid != SENTINEL_UID) {
            printf(",");
        }
    }
}

/* ============================================
 * Event Handlers
 * ============================================ */

/**
 * Event R - Register a new user
 * @param uid User ID to register
 * @return SUCCESS on success, ERROR_FAILURE on failure
 */
int register_user(int uid) {
    if (user_exists(uid)) {
        printf("\nThe user with uid %d already exists.\n", uid);
        return ERROR_FAILURE;
    }

    struct user *newUser = malloc(sizeof(*newUser));
    if (newUser == NULL) {
        printf("\nMemory allocation failed.\n");
        return ERROR_FAILURE;
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
    return SUCCESS;
}

/**
 * Event U - Unregister a user
 * @param uid User ID to unregister
 */
void unregister_user(int uid) {
    struct user *current = userList;
    struct user *prev = NULL;
    
    if (!user_exists(uid)) {
        printf("\nUser with UID %d not found.\n", uid);
        return;
    }
    
    while (current != NULL && current->uid != SENTINEL_UID) {
        if (current->uid == uid) {
            /* Remove the user from the linked list */
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                userList = current->next;
            }
            
            printf("U %d\n", uid);

            /* Free suggested movies */
            free_suggested_movie_list(current->suggestedHead);
            
            /* Free watch history */
            free_movie_list(current->watchHistory);
            
            /* Free the user */
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }
    
    /* Print the updated list of users */
    print_users_list();
    printf("\nDone\n");
}

/**
 * Event A - Add a new movie to the new movies list
 * @param mid Movie ID
 * @param category Movie category
 * @param year Release year
 * @return SUCCESS on success, ERROR_FAILURE on failure
 */
int add_new_movie(unsigned mid, movieCategory_t category, unsigned year) {
    struct new_movie *newMovie = (struct new_movie *)malloc(sizeof(struct new_movie));
    if (newMovie == NULL) {
        return ERROR_FAILURE; /* Memory allocation failed */
    }
    
    newMovie->info.mid = mid;
    newMovie->category = category; 
    newMovie->info.year = year;
    newMovie->next = NULL;

    struct new_movie *current = newMoviesList;
    struct new_movie *prev = NULL;

    /* Find the correct position to insert in sorted order */
    while (current != NULL && current->info.mid < mid) {
        prev = current;
        current = current->next;
    }

    if (prev == NULL) {
        /* Insert at the beginning */
        newMovie->next = newMoviesList;
        newMoviesList = newMovie;
    } else {
        /* Insert in the middle or at the end */
        prev->next = newMovie;
        newMovie->next = current;
    }

    printf("A <%u> <%d> <%u>\n", mid, category, year);
    printf("New movies = ");
    
    struct new_movie *temp = newMoviesList;
    while (temp != NULL) {
        printf(" <%u,%d,%u>", temp->info.mid, temp->category, temp->info.year);
        temp = temp->next;
    }
    printf("\nDONE\n");
    return SUCCESS;
}

/**
 * Event D - Distribute new movies to category lists
 */
void distribute_new_movies(void) {
    struct new_movie *current = newMoviesList;

    while (current != NULL) {
        movieCategory_t category = current->category;
        struct movie *newMovie = (struct movie *)malloc(sizeof(struct movie));
        
        if (newMovie == NULL) {
            /* Memory allocation failed, but continue with other movies */
            current = current->next;
            continue;
        }
        
        newMovie->info = current->info;
        newMovie->next = NULL;

        struct movie *currentCategory = categoryLists[category];
        struct movie *prevCategory = NULL;

        /* Find the correct position to insert in sorted order */
        while (currentCategory != NULL && currentCategory->info.mid < current->info.mid) {
            prevCategory = currentCategory;
            currentCategory = currentCategory->next;
        }

        if (prevCategory == NULL) {
            /* Insert at the beginning */
            newMovie->next = categoryLists[category];
            categoryLists[category] = newMovie;
        } else {
            /* Insert in the middle or at the end */
            prevCategory->next = newMovie;
            newMovie->next = currentCategory;
        }

        /* Move to the next new movie and free the old one */
        struct new_movie *temp = current;
        current = current->next;
        free(temp);
    }
    
    newMoviesList = NULL;
    printf("D\nCategorized Movies:\n");
    print_categorized_movies();
}

/**
 * Event W - User watches a movie
 * @param uid User ID
 * @param mid Movie ID
 * @return SUCCESS on success, ERROR_FAILURE on failure
 */
int watch_movie(int uid, unsigned mid) {
    struct user* user = find_user_by_uid(uid);
    
    if (user == NULL) {
        printf("User %d does not exist\n", uid);
        return ERROR_FAILURE;
    }

    /* Create a new movie structure and initialize it */
    struct movie* new_movie = (struct movie*)malloc(sizeof(struct movie));
    if (new_movie == NULL) {
        return ERROR_FAILURE; /* Memory allocation failed */
    }
    
    new_movie->info.mid = mid;
    new_movie->info.year = 0; /* Year not needed for watch history */
    new_movie->next = NULL;

    /* Add to the top of the watch history stack */
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
    return SUCCESS;
}

/**
 * Event S - Suggest movies to user based on other users' watch history
 * @param uid User ID to suggest movies to
 * @return SUCCESS on success, ERROR_FAILURE on failure
 */
int suggest_movies(int uid) {
    int counter = 1;
    struct user *current = find_user_by_uid(uid);
    
    if (current == NULL) {
        printf("User not found.\n");
        return ERROR_FAILURE;
    }

    /* Clear existing suggestions first */
    free_suggested_movie_list(current->suggestedHead);
    current->suggestedHead = NULL;
    current->suggestedTail = NULL;

    struct suggested_movie *currFront = NULL;
    struct suggested_movie *currBack = NULL;
    struct user *temp = userList;

    while (temp != NULL && temp->uid != SENTINEL_UID) {
        if (temp->uid != uid && temp->watchHistory != NULL) {
            struct movie *topMovie = temp->watchHistory;
            struct movie_info i = topMovie->info;
            
            struct suggested_movie *suggestedMovieNode = (struct suggested_movie *)malloc(sizeof(struct suggested_movie));
            if (suggestedMovieNode == NULL) {
                printf("Could not allocate memory\n");
                return ERROR_FAILURE;
            }
            
            suggestedMovieNode->info = i;
            
            if (counter % 2 != 0) {
                /* Add to front */
                suggestedMovieNode->next = NULL;
                suggestedMovieNode->prev = currFront;
                
                if (currFront != NULL) {
                    currFront->next = suggestedMovieNode;
                }
                
                currFront = suggestedMovieNode;
                if (current->suggestedHead == NULL) {
                    current->suggestedHead = currFront;
                }
            } else {
                /* Add to back */
                suggestedMovieNode->prev = NULL;
                suggestedMovieNode->next = currBack;
                
                if (currBack != NULL) {
                    currBack->prev = suggestedMovieNode;
                }
                
                currBack = suggestedMovieNode;
                if (current->suggestedTail == NULL) {
                    current->suggestedTail = currBack;
                }
            }
            counter++;
        }
        temp = temp->next;
    }
    
    /* Connect front and back if both exist */
    if (currFront != NULL && currBack != NULL) {
        currFront->next = currBack;
        currBack->prev = currFront;
    } else if (currFront != NULL && current->suggestedTail == NULL) {
        current->suggestedTail = currFront;
    } else if (currBack != NULL && current->suggestedHead == NULL) {
        current->suggestedHead = currBack;
    }
    
    /* Print suggestions */
    struct suggested_movie *suggestedMovieIterator = current->suggestedHead;
    printf("\nS <%d>\n", uid);
    printf("User <%d> Suggested Movies = ", uid);
    
    while (suggestedMovieIterator != NULL) {
        printf("<%d>", suggestedMovieIterator->info.mid);
        if (suggestedMovieIterator->next != NULL) {
            printf(", ");
        }
        suggestedMovieIterator = suggestedMovieIterator->next;
    }
    printf("\nDONE\n");
    return SUCCESS;
}

/**
 * Event F - Filtered movie search
 * @param uid User ID
 * @param category1 First category filter
 * @param category2 Second category filter
 * @param year Minimum year filter
 * @return SUCCESS on success, ERROR_FAILURE on failure
 */
int filtered_movie_search(int uid, movieCategory_t category1, movieCategory_t category2, unsigned year) {
    printf("F %d %d %d %u\n", uid, category1, category2, year);
    
    struct user* user = find_user_by_uid(uid);
    if (user == NULL) {
        printf("User %d does not exist\n", uid);
        return ERROR_FAILURE;
    }
    
    /* Clear existing suggestions */
    free_suggested_movie_list(user->suggestedHead);
    user->suggestedHead = NULL;
    user->suggestedTail = NULL;
    
    struct suggested_movie *first_category_suggestions = create_suggested_movie_list(category1, year);
    struct suggested_movie *second_category_suggestions = create_suggested_movie_list(category2, year);

    if (first_category_suggestions == NULL && second_category_suggestions == NULL) {
        printf("No suggestions available.\n");
    } else if (first_category_suggestions == NULL) {
        while (second_category_suggestions != NULL) {
            add_suggested_movie_to_user(user, second_category_suggestions);
            second_category_suggestions = second_category_suggestions->next;
        }
    } else if (second_category_suggestions == NULL) {
        while (first_category_suggestions != NULL) {
            add_suggested_movie_to_user(user, first_category_suggestions);
            first_category_suggestions = first_category_suggestions->next;
        }
    } else {
        struct suggested_movie *merged_suggestions = merge_suggested_movie_lists(first_category_suggestions, second_category_suggestions);
        while (merged_suggestions != NULL) {
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
    return SUCCESS;
}

/**
 * Remove a movie from a user's suggested list
 * @param user User whose suggested list to modify
 * @param mid Movie ID to remove
 * @return 1 if movie was found and removed, 0 otherwise
 */
static int remove_movie_from_user_suggestions(struct user* user, unsigned mid) {
    struct suggested_movie* suggested = user->suggestedHead;
    struct suggested_movie* prev_suggested = NULL;

    while (suggested != NULL) {
        if (suggested->info.mid == mid) {
            /* Update head if needed */
            if (prev_suggested != NULL) {
                prev_suggested->next = suggested->next;
            } else {
                user->suggestedHead = suggested->next;
            }
            
            /* Update tail if needed */
            if (suggested->next != NULL) {
                suggested->next->prev = prev_suggested;
            } else {
                user->suggestedTail = prev_suggested;
            }
            
            free(suggested);
            return 1; /* Movie found and removed */
        }
        prev_suggested = suggested;
        suggested = suggested->next;
    }
    
    return 0; /* Movie not found */
}

/**
 * Remove a movie from a category list
 * @param category Category to search in
 * @param mid Movie ID to remove
 * @return 1 if movie was found and removed, 0 otherwise
 */
static int remove_movie_from_category(movieCategory_t category, unsigned mid) {
    struct movie* current_movie = categoryLists[category];
    struct movie* prev_movie = NULL;

    while (current_movie != NULL) {
        if (current_movie->info.mid == mid) {
            if (prev_movie != NULL) {
                prev_movie->next = current_movie->next;
            } else {
                categoryLists[category] = current_movie->next;
            }
            
            free(current_movie);
            return 1; /* Movie found and removed */
        }
        prev_movie = current_movie;
        current_movie = current_movie->next;
    }
    
    return 0; /* Movie not found */
}

/**
 * Event T - Take off a movie from the service
 * @param mid Movie ID to remove
 */
void take_off_movie(unsigned mid) {
    int movie_found = 0;
    movieCategory_t found_category = HORROR; /* Default, will be set if movie found */
    
    printf("T %u\n", mid);

    /* Step 1: Remove the movie from every user's suggested list */
    struct user* current_user = userList;
    while (current_user != NULL && current_user->uid != SENTINEL_UID) {
        if (remove_movie_from_user_suggestions(current_user, mid)) {
            printf("%u removed from %d suggested list.\n", mid, current_user->uid);
        }
        current_user = current_user->next;
    }

    /* Step 2: Remove the movie from the corresponding category list */
    for (movieCategory_t i = HORROR; i <= COMEDY; i++) {
        if (remove_movie_from_category(i, mid)) {
            printf("%u removed from %s category list.\n", mid, get_category_name(i));
            found_category = i;
            movie_found = 1;
            break; /* Movie can only be in one category */
        }
    }
    
    /* Print the remaining movies in the category list */
    if (movie_found) {
        printf("Category list %d = ", found_category);
        struct movie* temp_movie = categoryLists[found_category];
        int first = 1;
        while (temp_movie != NULL) {
            if (!first) {
                printf(", ");
            }
            printf("%u", temp_movie->info.mid);
            temp_movie = temp_movie->next;
            first = 0;
        }
    }
    printf("\nDONE\n");
}

/**
 * Event M - Print information about movies in category lists
 */
void print_movies(void) {
    printf("M\nCategorized Movies:\n");
    print_categorized_movies();
}

/**
 * Event P - Print information about users and their suggested movies and watch history
 */
void print_users(void) {
    printf("P\nUsers:\n");

    struct user* current = userList;
    if (current == NULL) {
        printf("DONE\n");
        return;
    }
    
    while (current->uid != SENTINEL_UID) {
        printf("<%d>:\nSuggested: ", current->uid);

        /* Print suggested movies */
        struct suggested_movie* suggested = current->suggestedHead;
        int first = 1;
        while (suggested != NULL) {
            if (!first) {
                printf(", ");
            }
            printf("<%u,%d>", suggested->info.mid, suggested->info.year);
            suggested = suggested->next;
            first = 0;
        }

        /* Print watch history */
        printf("\nWatch History: ");
        struct movie* watchHistory = current->watchHistory;
        first = 1;
        while (watchHistory != NULL) {
            if (!first) {
                printf(", ");
            }
            printf("<%u,%d>", watchHistory->info.mid, watchHistory->info.year);
            watchHistory = watchHistory->next;
            first = 0;
        }

        printf("\n");
        current = current->next;
    }

    printf("DONE\n");
}
