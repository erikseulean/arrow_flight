import pyarrow as pa
import pyarrow.flight

client = pa.flight.connect("grpc://0.0.0.0:5555")

reader = client.do_get(pa.flight.Ticket(b"weather"))
for table in reader:
    print(table.to_pandas())
# print(read_table.to_pandas().head())
