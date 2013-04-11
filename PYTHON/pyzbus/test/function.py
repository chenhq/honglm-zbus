from pyzbus import Connection

conn = Connection()
conn.probe(1)

conn.destroy()