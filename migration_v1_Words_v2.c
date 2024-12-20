/*****************************************************************************/
/* Projekt: RuteShi                                Version  0.0.1.           */
/*---------------------------------------------------------------------------*/
/* Modul       : RealAI_Words.c                                              */
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
/* 20.12.2024  : AJH           : Added TRANSAKTION for faster processing     */
/*---------------------------------------------------------------------------*/
/* 20.12.2024  : AJH           : Changed Collum Priority to Frequency Words.  */
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

#define MY_BRAIN "../Brain5511/Brain"
#define MY_WORDS MY_BRAIN "/Words.txt"
#define DATABASE_NAME "../RuteShi.db"

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
                             "    Frequency INTEGER"
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
        const char *sql_insert = "INSERT INTO Words (Word, Frequency) VALUES (?, ?)";

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
    char *err_msg = NULL;

    words = read_words(MY_WORDS, &num_words);
    if (words == NULL)
    {
        return 1;
    }
    // fprintf(CREATE TABLE )
    // Use the words array for further processing
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

        // Commit transaction
        rc = sqlite3_exec(db, "COMMIT", NULL, NULL, &err_msg);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Failed to commit transaction: %s\n", err_msg);
            sqlite3_free(err_msg);
        }
    }
    free(words);
    return (rc);
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
