#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

// The Tables have to exist inwide the ImportDatabase
#define EXPORT_DB "RuteShi.db"
#define IMPORT_DB "test123.brain.db"

#define TABLE_WORDS   "Words"
#define TABLE_PROWORDS   "ProWords"
#define TABLE_PREWORDS   "PreWords"

// Struktur zur Definition der Felder
typedef struct
{
    const char *name;
} Field;

Field words_fields[] = {{"Id"}, {"Word"}, {"Distance"}};
Field prowords_fields[] = {{"Id"}, {"Word"}, {"ProWord"}, {"Priority"}, {"Distance"}};
Field prewords_fields[] = {{"Id"}, {"Word"}, {"PreWord"}, {"Priority"}, {"Distance"}};

// Funktion zur Berechnung der Anzahl der Felder
#define FIELD_COUNT(fields) (sizeof(fields) / sizeof(fields[0]))


void transfer_data(const char *source_db, const char *dest_db, const char *table, Field *fields, int field_count)
{
    sqlite3 *conn1, *conn2;
    sqlite3_stmt *stmt1, *stmt2;
    char *err_msg = 0;
    int rc;

    char select_query[1024];
    char insert_query[1024];

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
    snprintf(select_query, sizeof(select_query), "DELETE FROM %s", table);
    rc = sqlite3_exec(conn2, select_query, 0, 0, &err_msg);
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
    transfer_data(EXPORT_DB, IMPORT_DB, TABLE_WORDS, words_fields, FIELD_COUNT(words_fields));

    transfer_data(EXPORT_DB, IMPORT_DB, TABLE_PROWORDS, prowords_fields, FIELD_COUNT(prowords_fields));

    transfer_data(EXPORT_DB, IMPORT_DB, TABLE_PREWORDS, prewords_fields, FIELD_COUNT(prewords_fields));

    return (0);
}