/*****************************************************************************/
/* Projekt: RuteShi                                Version  0.0.1.           */
/*---------------------------------------------------------------------------*/
/* Modul       : ImportBrainTables2db.c                                      */
/* Autor       : Alexander J. Herrmann                                       */
/* Erstellt    : Dezember 2024                                               */
/* Than you    : Instead of typing a lot myself on a Android Tablet I used   */
/*               AI coaching. So some of the Code is not up2 industrial      */
/*               strenght. Well, it is something to be used once - anyway:   */
/*               Thanks to Google Gemini, MicroSoft Bing Co-pilot and        */
/*               GitHub Co-Pilot for saving me a lot of typing.              */
/* The Oblivonburn RealAI Version 2 can be found at GitHub:                  */
/*      https://github.com/Oblivionburn/realAI2.                             */
/* These program has it's home in the Cloud at:                              */
/*      https://github.com/Ping2Weltall/RealAi-v1-to-v2-migration.           */
/*...........................................................................*/
/* Purpose:                                                                  */
/*      Transfers the data from RuteShi.db into SQLite3 Tables for RealAI v2 */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <time.h>

#include <stdarg.h>
#include <sys/types.h>

// The Tables have to exist inwide the ImportDatabase
// Change this to the RealAIv2 Eport/Import Directory
#define REALAI2_HOME_DIR "../../RealAi.v2"
// The Database which contains the data exported from RealAIv1
#define EXPORT_DB "../RuteShi.db"
// Exported Brains usually have *.brain as default export extension
// I rename them to *.brain.db To view them inside some some Android SQLite3 Viewers which insist on the .db
#define IMPORT_DB REALAI2_HOME_DIR "/" \
                                   "test123.brain.db"

#define TABLE_WORDS "Words"
#define TABLE_PROWORDS "ProWords"
#define TABLE_PREWORDS "PreWords"

// Struktur zur Definition der Felder
struct field
{
    const char *name;
} Field;

struct field words_fields[] = {{"Id"}, {"Word"}, {"Frequency"}};
struct field prowords_fields[] = {{"Id"}, {"Word"}, {"ProWord"}, {"Priority"}, {"Distance"}};
struct field prewords_fields[] = {{"Id"}, {"Word"}, {"PreWord"}, {"Priority"}, {"Distance"}};

// Funktion zur Berechnung der Anzahl der Felder
#define FIELD_COUNT(fields) (sizeof(fields) / sizeof(struct field))
struct stat *fragment_status(char *file);

void transfer_data(const char *source_db, const char *dest_db, const char *table, struct field *fields, int field_count)
{
    sqlite3 *conn1, *conn2;
    sqlite3_stmt *stmt1, *stmt2;
    char *err_msg = NULL;
    int rc;

    char select_query[1024];
    char *delete_query = select_query;
    char insert_query[1024];

printf("Importing Tables from %s to %s\n", source_db, dest_db );

    // Open source database
    rc = sqlite3_open(source_db, &conn1);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open source database: %s\n", sqlite3_errmsg(conn1));
        sqlite3_close(conn1);
        return;
    }

    // Open destination database
    rc = sqlite3_open(dest_db, &conn2);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open destination database: %s\n", sqlite3_errmsg(conn2));
        sqlite3_close(conn1);
        sqlite3_close(conn2);
        return;
    }

    // Delete existing records in destination table
    snprintf(delete_query, sizeof(select_query), "DELETE FROM %s", table);
    rc = sqlite3_exec(conn2, delete_query, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(conn1);
        sqlite3_close(conn2);
        return;
    }

    // Prepare select query
    snprintf(select_query, sizeof(select_query), "SELECT %s", fields[0].name);
    for (int i = 1; i < field_count; i++)
    {
        strncat(select_query, ", ", sizeof(select_query) - strlen(select_query) - 1);
        strncat(select_query, fields[i].name, sizeof(select_query) - strlen(select_query) - 1);
    }
    strncat(select_query, " FROM ", sizeof(select_query) - strlen(select_query) - 1);
    strncat(select_query, table, sizeof(select_query) - strlen(select_query) - 1);

    rc = sqlite3_prepare_v2(conn1, select_query, -1, &stmt1, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare select statement: %s\n", sqlite3_errmsg(conn1));
        sqlite3_close(conn1);
        sqlite3_close(conn2);
        return;
    }

    // Prepare insert query
    snprintf(insert_query, sizeof(insert_query), "INSERT INTO %s VALUES (?", table);
    for (int i = 1; i < field_count; i++)
    {
        strncat(insert_query, ", ?", sizeof(insert_query) - strlen(insert_query) - 1);
    }
    strncat(insert_query, ")", sizeof(insert_query) - strlen(insert_query) - 1);

    rc = sqlite3_prepare_v2(conn2, insert_query, -1, &stmt2, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare insert statement: %s\n", sqlite3_errmsg(conn2));
        sqlite3_close(conn1);
        sqlite3_close(conn2);
        return;
    }

    // Begin transaction
    rc = sqlite3_exec(conn2, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to begin transaction: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(conn1);
        sqlite3_close(conn2);
        return;
    }

    // Execute data transfer
    while (sqlite3_step(stmt1) == SQLITE_ROW)
    {
        for (int i = 0; i < field_count; i++)
        {
            sqlite3_bind_text(stmt2, i + 1, (const char *)sqlite3_column_text(stmt1, i), -1, SQLITE_STATIC);
        }
        if (sqlite3_step(stmt2) != SQLITE_DONE)
        {
            fprintf(stderr, "Failed to execute insert statement: %s\n", sqlite3_errmsg(conn2));
        }
        sqlite3_reset(stmt2);
    }

    // Commit transaction
    rc = sqlite3_exec(conn2, "COMMIT", NULL, NULL, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to commit transaction: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    // Cleanup
    sqlite3_finalize(stmt1);
    sqlite3_finalize(stmt2);
    sqlite3_close(conn1);
    sqlite3_close(conn2);
}

int main()
{
    struct stat *status;
    int rc = 0;
    if ((status = fragment_status(IMPORT_DB)) != NULL)
    {
        if (status->st_size == 0)
        {
            // Sorry Folks
            fprintf(stderr, "Sorry folks - database [%s] seems to be empty.\n", IMPORT_DB);
        }
        else
        {
            transfer_data(EXPORT_DB, IMPORT_DB, TABLE_WORDS, words_fields, FIELD_COUNT(words_fields));

            transfer_data(EXPORT_DB, IMPORT_DB, TABLE_PROWORDS, prowords_fields, FIELD_COUNT(prowords_fields));

            transfer_data(EXPORT_DB, IMPORT_DB, TABLE_PREWORDS, prewords_fields, FIELD_COUNT(prewords_fields));
        }
    }
    else
    {
        fprintf(stderr, "Database [%s] not found.\n", IMPORT_DB);
    }
    return (0);
}

struct stat *fragment_status(char *file)
{
    static struct stat fileStat;

    if (stat(file, &fileStat) == 0)
    {
        // printf("Letzte Änderung: %s", ctime(&fileStat.st_mtime));
    }
    else
    {
        // fprintf(stderr, "stat error");
        return (NULL);
    }
    return (&fileStat);
}