#define MY_BRAIN "../Brain5511/Brain"
#define MY_WORDS MY_BRAIN "/Words.txt"
#define DATABASE_NAME    "../RuteShi.db"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>
#include <errno.h>

#define MAX_WORD_LENGTH 100
#define INITIAL_CAPACITY 100

int dump2sql(sqlite3 *db, sqlite3_stmt *stmt);

typedef struct
{
    int position;
    char word[MAX_WORD_LENGTH];
    int frequency;
} WordData;

WordData *read_words(const char *filename, int *num_words); 

int main()
{
    // ... other code

    sqlite3 *db;
    int rc;

    rc = sqlite3_open(DATABASE_NAME, &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    const char *sql_drop = "DROP TABLE Words;";
    const char *sql_create = "CREATE TABLE Words ("
                      "    ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "    Word TEXT,"
                      "    Priority INTEGER"
                      ");";
    rc = sqlite3_exec(db, sql_drop, NULL, 0, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL INFO: %s rc=%d\n", sqlite3_errmsg(db), rc);
    }
    
    rc = sqlite3_exec(db, sql_create, NULL, 0, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    else
    {
        const char *sql_insert = "INSERT INTO Words (Word, Priority) VALUES (?, ?)";

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
        if (rc == SQLITE_OK)
        {
            // Load Words ..
            dump2sql(db, stmt);
        }
        else
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return 0;
}

int dump2sql(sqlite3 *db, sqlite3_stmt *stmt)
{
    WordData *words;
    int num_words;
    int rc = SQLITE_OK;

    words = read_words(MY_WORDS, &num_words);
    if (words == NULL)
    {
        return 1;
    }
    // fprintf(CREATE TABLE )
    // Use the words array for further processing
    for (int i = 0; i < num_words; i++)
    {
        int len = strlen(words[i].word);
        printf("%0.5xd:%s:L%0.3d:F:%0.3d\n", words[i].position, words[i].word, len, words[i].frequency);
        sqlite3_bind_text(stmt, 1, words[i].word, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, words[i].frequency);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            // Handle error
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
        }

        sqlite3_reset(stmt);
    }

    free(words);
    return(rc);
}

WordData *read_words(const char *filename, int *num_words)
{
    static int position = 0;
    WordData *words = (WordData *)malloc(INITIAL_CAPACITY * sizeof(WordData));
    if (words == NULL)
    {
        perror("Error allocating memory");
        return NULL;
    }

    int capacity = INITIAL_CAPACITY;
    int count = 0;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Error opening file");
        free(words);
        return NULL;
    }

    char line[MAX_WORD_LENGTH + 1];
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        char *word = strtok(line, "~");
        char *freq_str = strtok(NULL, "~");

        if (word != NULL && freq_str != NULL)
        {
            int frequency = atoi(freq_str);
            position++;
            if (count == capacity)
            {
                capacity *= 2;
                WordData *new_words = (WordData *)realloc(words, capacity * sizeof(WordData));
                if (new_words == NULL)
                {
                    perror("Error reallocating memory");
                    free(words);
                    return NULL;
                }
                words = new_words;
            }
            strlcpy(words[count].word, word, MAX_WORD_LENGTH);
            words[count].frequency = frequency;
            words[count].position = position;
            count++;
        }
    }

    fclose(fp);
    *num_words = count;
    return words;
}

