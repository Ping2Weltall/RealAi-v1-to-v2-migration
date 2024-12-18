import sqlite3
import logging


# Configure logging (optional: adjust log level and file path)
logging.basicConfig(filename='data_transfer.log', level=logging.INFO, 
                    format='%(asctime)s - %(levelname)s - %(message)s') 


def transfer_data(source_db, source_table, dest_db, dest_table, *fields):
    """
    Transfers selected data from one table to another.

    Args:
        source_db (str): Path to the source database.
        source_table (str): Name of the source table.
        dest_db (str): Path to the destination database.
        dest_table (str): Name of the destination table.
        *fields (str): List of field names to be transferred.
    """

    try:
        conn1 = sqlite3.connect(source_db, detect_types=sqlite3.PARSE_DECLTYPES)
        cursor1 = conn1.cursor()
        conn1.text_factory = lambda x: str(x, 'utf-8', 'ignore')

        conn2 = sqlite3.connect(dest_db, detect_types=sqlite3.PARSE_DECLTYPES)
        cursor2 = conn2.cursor()
        conn2.text_factory = lambda x: str(x, 'utf-8', 'ignore')

        with conn2:
            cursor2.execute(f"DELETE FROM {dest_table}") 

            select_query = f"SELECT {', '.join(fields)} FROM {source_table}"
            cursor1.execute(select_query)
            data = cursor1.fetchall()

            placeholders = ', '.join(['?'] * len(fields))
            insert_query = f"INSERT INTO {dest_table} VALUES ({placeholders})"
            cursor2.executemany(insert_query, data)

    except sqlite3.IntegrityError as e:
        logging.error(f"Integrity error: {e}")
    except sqlite3.OperationalError as e:
        logging.error(f"Operational error: {e}")
    except sqlite3.ProgrammingError as e:
        logging.error(f"Programming error: {e}")
    except sqlite3.Error as e:
        logging.error(f"General SQLite error: {e}")
    finally:
        if conn1:
            conn1.close()
        if conn2:
            conn2.close()

# ... (your data transfer calls) 

# Datentransfer
transfer_data('RuteShi.db', 'Words', 'test.db.brain', 'Words', 'Id', 'Word', 'Distance')
transfer_data('RuteShi.db', 'ProWords', 'test.db.brain', 'ProWords', 'Id', 'Word', 'ProWord', 'Priority', 'Distance')
transfer_data('RuteShi.db', 'PreWords', 'test.db.brain', 'PreWords', 'Id', 'Word', 'PreWord', 'Priority', 'Distance')