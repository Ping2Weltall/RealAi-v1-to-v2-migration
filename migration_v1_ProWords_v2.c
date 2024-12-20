/*****************************************************************************/
/* Projekt: RuteShi                                Version  0.0.1.           */
/*---------------------------------------------------------------------------*/
/* Modul       : RealAI_ProWords.c                                           */
/* Autor       : Alexander J. Herrmann                                       */
/* Erstellt    : Dezember 2024                                               */
/* Than you    : Instead of typing a lot myself on a Android Tablet I used   */
/*               AI coaching. So some of the Code is not up2 industrial      */
/*               strenght. Well, it is something to be used once - anyway:   */
/*               Thanks to Google Gemini, MicroSoft Bing Co-pilot and        */
/*               GitHub Co-Pilot for saving me a lot of typing.              */
/* The Oblivonburn RealAI Version 1 can be found at GitHub:                  */
/*      https://github.com/Oblivionburn/realai.                              */
/* These program has it's home in the Cloud at:                              */
/*      https://github.com/Ping2Weltall/RealAi-v1-to-v2-migration.           */
/*...........................................................................*/
/* Purpose:                                                                  */
/*      Transfers the data from RealAI v1 into SQLite3 Tables                */
/*---------------------------------------------------------------------------*/
/* Changes:                                                                  */
/* dd.mm.yyyy  : Author        : Modification                                */
/*.............+...............+.............................................*/
/*             :               :                                             */
/*.............+...............+.............................................*/
/* 19.12.2024  : AJH           : Added TRANSAKTION for faster processing     */
/*---------------------------------------------------------------------------*/
/*    THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED   */
/*    WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF   */
/*            MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.          */
/*                This program is NOT FREE SOFTWARE in common!               */
/* But as it has a dual Licence so it may still fit your needs as long as it */
/* is used for non-profit purposes including educational use. You're also    */
/* allowed to redistribute it and/or modify it under the included            */
/* "LESSER GNU GENERAL PUBLIC LICENCE" Version 2.1 - see COPYING.LESSER.     */
/* A exception is that any output like Webpages, Scripts do not automaticly  */
/* fall under the copyright of this package, but belong to whoever generated */
/* them and may be sold commercialy and may be aggregatet with this Software */
/* as long as the Software itself is distributed under the Licence terms.    */
/* C subroutines (or comparable compiled subroutines in other languages)     */
/* supplied by you and linked into this Software in order to extend the      */
/* functionality of this Software shall not be considered part of this       */
/* Software and should not alter the Software in any way that would cause it */
/* to fail the regression tests for this Software.                           */
/* Another exception to the above Licence is that you can modify the and use */
/* the modified Software without placing the modifications in the Public     */
/* Domain as long as this modifications are only used within your corporation*/
/* or organization.                                                          */
/*---------------------------------------------------------------------------*/
/* In case that you would like to use it in aggregate with commercial        */
/* programs and/or as part of a larger (possibly commercial) software        */
/* distribution than you should contact the Copyright Holder first.          */
/* Same if you have plans which would violate one or more of the included    */
/* "LESSER GNU GENERAL PUBLIC LICENCE" Version 2.1 Licence Terms.            */
/*---------------------------------------------------------------------------*/
/* (C) 2024.     Alexander Joerg Herrmann                                    */
/*               Email:    ping2weltall@gmail.com                            */
/*               http://fb.me/Computeralex                                   */
/*...........................................................................*/
/* Alle Rechte vorbehalten                               All rights reserved */
/*****************************************************************************/

// You may need to change MY_BRAIN to the directory where your RealAI Textfiles are stored
#define EXPORT_TYPE "Pro"
#define FILE_TYPE EXPORT_TYPE "-"
#define TABLE_NAME EXPORT_TYPE "Words"
#define FIELD_NAME EXPORT_TYPE "Word"

#define MY_BRAIN "../Brain5511/Brain"
#define DATABASE_NAME "../RuteShi.db"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>
#include <errno.h>

#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#define MAX_WORD_LENGTH 100
#define INITIAL_CAPACITY 100

int dump_dir2sql(sqlite3 *db, sqlite3_stmt *stmt);
int dump_file2sql(const char *filename, char *type, sqlite3 *db, sqlite3_stmt *stmt);

typedef struct
{
    int position;
    char word[MAX_WORD_LENGTH];
    char preword[MAX_WORD_LENGTH];
    int priority;
    int distance;
} WordData;

WordData *read_words(const char *filename, const char *type, char *word, int *num_words);
struct stat *file_status(const char *filename);

int main()
{
    sqlite3 *db;
    int rc;

    rc = sqlite3_open(DATABASE_NAME, &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    const char *sql_drop = "DROP TABLE " TABLE_NAME ";";
    const char *sql_create =
        "CREATE TABLE " TABLE_NAME " ("
        "    ID INTEGER PRIMARY KEY,"
        "    Word TEXT NULLABLE,"
        "  " FIELD_NAME " TEXT NULLABLE,"
        "    Priority INTEGER NULLABLE,"
        "    Distance INTEGER NULLABLE"
        ");";
    rc = sqlite3_exec(db, sql_drop, NULL, 0, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL INFO: %s rc=%d\n", sqlite3_errmsg(db), rc);
    }

    rc = sqlite3_exec(db, sql_create, NULL, 0, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s rc=%d\n", sqlite3_errmsg(db), rc);
        sqlite3_close(db);
        return 1;
    }
    else
    {
        const char *sql_insert = "INSERT INTO " TABLE_NAME
                                 " (Id, Word, " FIELD_NAME ","
                                 "  Priority, "
                                 " Distance) VALUES (?, ?, ?, ?, ?)";

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
        if (rc == SQLITE_OK)
        {
            // Load Words ..
            printf("Database ready .. loading.\n");
            dump_dir2sql(db, stmt);
        }
        else
        {
            fprintf(stderr, "SQL error: %s [%s]\n", sqlite3_errmsg(db), sql_insert);
            sqlite3_close(db);
            return 1;
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return 0;
}

int dump_dir2sql(sqlite3 *db, sqlite3_stmt *stmt)
{
    DIR *d;
    struct dirent *dir;

    int rc = 0;
    int count = 0;

    d = opendir(MY_BRAIN);

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            char *file = dir->d_name;

            if (dir->d_type == DT_REG)
            {
                if ((strncmp(file, FILE_TYPE, 4) == 0))
                {
                    char input_file[1024];

                    sprintf(input_file, "%s/%s", MY_BRAIN, file);
                    printf("%5d \t", ++count);
                    rc = dump_file2sql(input_file, FILE_TYPE, db, stmt);
                }
            }
        }
    }
    else
    {
        printf("Error %d\n", errno);
        rc = -1;
    }
    return (rc);
}

int dump_file2sql(const char *filename, char *type, sqlite3 *db, sqlite3_stmt *stmt)
{
    WordData *words;

    int num_words = 0;
    static int sequence = 0;
    int rc = SQLITE_OK;
    static int idnr = 0;
    char *word = strdup(filename);
    char *sptr = (strstr(word, ".txt"));
    char *err_msg = NULL;
    if (sptr != NULL)
    {
        *((sptr = (word + strlen(word) - 4))) = '\0';
        while (sptr > word)
        {
            if (*sptr == '/' && (strncmp(sptr + 1, FILE_TYPE, 4) == 0))
            {
                free(word);
                word = strdup(sptr + 5);
                break;
            }
            sptr--;
        }
    }
    // printf("%s ", ++sequence, word);

    // printf("%s:", filename);

    words = read_words(filename, type, word, &num_words);
    if (words == NULL)
    {
        if (word == NULL)
        {
            printf("No Data found! %s\t%d\n", filename, sequence);
            exit(1);
        }
        sqlite3_bind_int(stmt, 1, ++idnr);
        sqlite3_bind_text(stmt, 2, word, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, NULL, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, 0);
        sqlite3_bind_int(stmt, 5, 0);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            // Handle error
            fprintf(stderr, "SQL error: %s : %s Sequence: %d\n", sqlite3_errmsg(db), word, sequence);
            sqlite3_close(db);
            exit(1);
        }
        sqlite3_reset(stmt);
    }
    else
    {
        // Begin transaction
        rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Failed to begin transaction: %s\n", err_msg);
            sqlite3_free(err_msg);
        }
        else
        {
            for (int i = 0; i < num_words; i++)
            {
                int len = strlen(words[i].word);
                printf("%5d:%s:L%d:P%0.3d:D:%0.3d\n", words[i].position, words[i].word, len, words[i].priority, words[i].distance);
                sqlite3_bind_int(stmt, 1, ++idnr);
                sqlite3_bind_text(stmt, 2, words[i].word, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, words[i].preword, -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 4, words[i].priority);
                sqlite3_bind_int(stmt, 5, words[i].distance);

                rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                {
                    // Handle error
                    fprintf(stderr, "SQL error: %s Position: %d\n", sqlite3_errmsg(db), i);
                    sqlite3_close(db);
                    exit(1);
                }
                sqlite3_reset(stmt);
            }
            // Commit transaction
            rc = sqlite3_exec(db, "COMMIT", NULL, NULL, &err_msg);
            if (rc != SQLITE_OK)
            {
                fprintf(stderr, "Failed to commit transaction: %s\n", err_msg);
                sqlite3_free(err_msg);
            }
            free(words);
        }
    }
    return (rc);
}

WordData *read_words(const char *filename, const char *type, char *word, int *num_words)
{
    int position = 0;
    struct stat *status = NULL;
    int size = 0;

    WordData *words = NULL;

    if ((status = file_status(filename)) != NULL)
    {
        if ((size = status->st_size) > 0)
        {
            if (word != NULL)
            {
                words = (WordData *)malloc(INITIAL_CAPACITY * sizeof(WordData));
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
                    if (*line != '\0')
                    {
                        char *preword = strtok(line, "~");
                        char *s_distance = strtok(NULL, "~");

                        {
                            int priority = 0;
                            int distance = 0;
                            if (s_distance != NULL)
                            {
                                distance = atoi(s_distance);
                            }

                            position++;
                            if (count == capacity)
                            {
                                capacity *= 2;
                                WordData *new_words = (WordData *)realloc(words, capacity * sizeof(WordData));
                                if (new_words == NULL)
                                {
                                    perror("Error reallocating memory");
                                    free(words);
                                    exit(1);
                                }
                                words = new_words;
                            }
                            strlcpy(words[count].word, word, MAX_WORD_LENGTH);
                            if (preword != NULL)
                            {
                                strlcpy(words[count].preword, preword, MAX_WORD_LENGTH);
                            }
                            else
                            {
                                *(words[count].preword) = '\0';
                            }
                            words[count].priority = priority;
                            words[count].distance = distance;
                            words[count].position = ++position;
                            count++;
                            printf("\t%-20s\t%d\t%d\n", words[count].preword, priority, distance);
                        }
                    }
                }
                printf("%20s\t%d\n", word, count);
                free(word);
                fclose(fp);
                *num_words = count;
            }
            else
            {
                printf("Memmory %d - %s\n", errno, filename);
                exit(1);
            }
        }
        else
        {
            printf("Info Empty %s", word);
        }
    }
    else
    {
        printf("status error %d", errno);
    }
    return (words);
}

struct stat *file_status(const char *filename)
{
    static struct stat fileStat;
    static long n = 0;

    if (stat(filename, &fileStat) == 0)
    {
       // Get a segmentation fault here ..
       // printf("Last change: %s", ctime(&fileStat.st_mtime));
    }
    else
    {
        printf("stat error %d", errno);
        exit(1);
    }
    return (&fileStat);
}
