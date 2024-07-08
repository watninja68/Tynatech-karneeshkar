import grpc
from chirpstack_api import api
import chirpstack_api.api.device_queue_pb2_grpc as device_queue_pb2_grpc

# Configuration.
server = "localhost:8080"
dev_eui = "0101010101010101"
api_token = "..."

if __name__ == "__main__":
    # Connect without using TLS.
    channel = grpc.insecure_channel(server)

    # Use the correct stub class for DeviceQueueService
    client = device_queue_pb2_grpc.DeviceQueueServiceStub(channel)

    # Define the API key meta-data.
    auth_token = [("authorization", "Bearer %s" % api_token)]

    # Construct request.
    req = api.EnqueueDeviceQueueItemRequest()
    req.queue_item.confirmed = False
    req.queue_item.data = bytes([0x01, 0x02, 0x03])
    req.queue_item.dev_eui = dev_eui
    req.queue_item.f_port = 10

    try:
        resp = client.Enqueue(req, metadata=auth_token)

        # Print the downlink id
        print(resp.id)
    except grpc.RpcError as e:
        print(f"gRPC error: {e.details()}")
        print(f"({e.code().name})")
