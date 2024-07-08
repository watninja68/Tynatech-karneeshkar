import sqlite3

# Connect to the database file
conn = sqlite3.connect('test.db')  # replace with your.db file name

# Get a cursor object
cur = conn.cursor()

# Execute a query to retrieve all table names
cur.execute("SELECT name FROM sqlite_master WHERE type='table'")

# Fetch all the rows
tables = cur.fetchall()

# Iterate over each table and print its contents
for table in tables:
    table_name = table[0]
    print(f"Table: {table_name}")
    print("---------")

    # Execute a query to retrieve all columns and rows for this table
    cur.execute(f"SELECT * FROM {table_name}")
    rows = cur.fetchall()
    columns = [desc[0] for desc in cur.description]

    # Print the column names
    print("Columns:", columns)
    print("---------")

    # Print each row
    for row in rows:
        print(row)

    print()  # empty line for separation
    cur.execute(f"SELECT count(*) from {table_name}")
    count = cur.fetchall()
    print(count)
# Close the connection
conn.close()