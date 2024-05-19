#include <arrow/api.h>
#include <arrow/flight/api.h>
#include <gflags/gflags.h>
#include <grpc++/grpc++.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <signal.h>

using namespace arrow;
using namespace arrow::ipc;
using namespace arrow::flight;

class CustomBatchReader : public RecordBatchReader {

public:
    CustomBatchReader(std::shared_ptr<Schema> schema): 
        _schema(std::move(schema)),
        _batch1(nullptr),
        _batch2(nullptr),
        _counter(0)
    {
        std::shared_ptr<arrow::Array> name_array1, name_array2;
        std::shared_ptr<arrow::Array> age_array1, age_array2;
        arrow::StringBuilder name_builder;
        arrow::Int32Builder age_builder;

        // Add some dummy data to the first table
        name_builder.Append("John");
        age_builder.Append(25);
        name_builder.Finish(&name_array1);
        age_builder.Finish(&age_array1);

        // Reset the builders
        name_builder.Reset();
        age_builder.Reset();

        // Add some dummy data to the second table
        name_builder.Append("Jane");
        age_builder.Append(30);
        name_builder.Finish(&name_array2);
        age_builder.Finish(&age_array2);

        // Create record batches
        _batch1 = arrow::RecordBatch::Make(_schema, 1, {name_array1, age_array1});
        _batch2 = arrow::RecordBatch::Make(_schema, 1, {name_array2, age_array2});
    }

    std::shared_ptr<Schema> schema() const override
    {
        std::cout << "Returning schema"<< std::endl;
        return _schema;
    }

    Status ReadNext(std::shared_ptr<RecordBatch>* out) override
    {
        if(_counter == 0)
        {
            std::cout << "returning batch1" << std::endl;
            *out = _batch1;
            _counter += 1;
        }
        else if(_counter == 1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(30));
            std::cout << "returning batch2" << std::endl;
            *out = _batch2;
            _counter += 1;
        }
        else
        {
            *out = nullptr;
        }
        std::cout << "Returning status OK" << std::endl;
        return Status::OK();
    }

private:
    std::shared_ptr<Schema> _schema;
    std::shared_ptr<RecordBatch> _batch1;
    std::shared_ptr<RecordBatch> _batch2;
    std::size_t _counter;
};

class MyFlightService : public arrow::flight::FlightServerBase {
public:
    arrow::Status DoGet(const arrow::flight::ServerCallContext& context,
                        const arrow::flight::Ticket& request,
                        std::unique_ptr<arrow::flight::FlightDataStream>* stream) override {
        std::cout << "Received a DoGet request\n";

        std::shared_ptr<arrow::Schema> schema =
            arrow::schema({
                arrow::field("name", arrow::utf8()),
                arrow::field("age", arrow::int32())
                }
            );
        std::cout << "Created schema\n" << std::to_string(schema->num_fields()) << std::endl;
        auto custom_reader = std::make_shared<CustomBatchReader>(std::move(schema));
        *stream = std::unique_ptr<arrow::flight::FlightDataStream>(
            new arrow::flight::RecordBatchStream(custom_reader));

        return arrow::Status::OK();
    }

};

arrow::Status RunFlightGrpc()
{
    std::unique_ptr<MyFlightService> service = std::make_unique<MyFlightService>();
    
    arrow::flight::Location bind_location;
    ARROW_RETURN_NOT_OK(
        arrow::flight::Location::ForGrpcTcp("0.0.0.0", 5555).Value(&bind_location)
    );
    arrow::flight::FlightServerOptions options(bind_location);

    ARROW_RETURN_NOT_OK(service->Init(options));
    ARROW_RETURN_NOT_OK(service->SetShutdownOnSignals({SIGTERM}));

    std::cout << "Server is running at " << bind_location.ToString() << std::endl;
    std::cout << "Hello, from arrow_service!\n";
    ARROW_RETURN_NOT_OK(service->Serve());
    
    return arrow::Status::OK();
}

int main(int, char**)
{
    auto status = RunFlightGrpc();
    if (!status.ok()) {
        std::cerr << "Error from here:" << status.ToString() << std::endl;
        return EXIT_FAILURE;
    }
}