import socket
import struct
import numpy as np
import cv2
import tensorflow as tf
import numpy as np
import warnings
import threading

warnings.filterwarnings('ignore')

from object_detection.utils import label_map_util

should_stop = False
active_threads = []

HOST = '127.0.0.1'
PORT = 8000
MAXIMUM_NUMBER_OF_CONNECTIONS = 4

PATH_TO_SAVED_MODEL= ".\\MLDataTraining\\Python\\Tensorflow\\models\\ssd_mobilenet_v2_fpnlite_320x320_coco17_tpu-8\\exports_2023-04-21_09-40-31\\saved_model"
LABEL_MAP = ".\\MLDataTraining\\Python\\Tensorflow\\training_data\\BDD100K\\train\\cars-pedestrians_label_map.pbtxt"
SCORE_THRESHOLD = 0.4
OVERLAP_THRESHOLD = 0.5

print("Loading model...")
detect_fn=tf.saved_model.load(PATH_TO_SAVED_MODEL)
category_index=label_map_util.create_category_index_from_labelmap(LABEL_MAP,use_display_name=True)
print("Finished loading model")

def detect_cars_inside_image(image_np):
    
    # cv2.imshow("img", image_np)
    # cv2.waitKey(0)
    # cv2.destroyAllWindows()
    
    input_tensor = tf.convert_to_tensor(image_np)
    input_tensor = input_tensor[tf.newaxis, ...]
    
    detections = detect_fn(input_tensor)
    
    num_detections = int(detections.pop('num_detections'))
    detections = {key: value[0, :num_detections].numpy() for key, value in detections.items()}
    detections['num_detections'] = num_detections
    
    car_indices = detections['detection_classes'] == 1
    car_boxes = detections['detection_boxes'][car_indices]
    car_scores = detections['detection_scores'][car_indices]
    
    indices = cv2.dnn.NMSBoxes(
        np.array(car_boxes).astype(np.int32),
        np.array(car_scores),
        score_threshold=SCORE_THRESHOLD,
        nms_threshold=OVERLAP_THRESHOLD
    )
    
    try:
        if indices:
            indices = indices.flatten()
    except:
        pass
    
    return len(indices)
    
class MessageHeader:
    data_type_format = "BH?Q"
    def __init__(self, message_type, message_id, has_priority, message_size):
        self.message_type = message_type
        self.message_id = message_id
        self.has_priority = has_priority
        self.message_size = message_size
        
    def pack(self):
        return struct.pack(MessageHeader.data_type_format, self.message_type, self.message_id, self.has_priority, self.message_size)
    
    @classmethod
    def unpack(cls, data):
        message_type, message_id, has_priority, message_size = struct.unpack(MessageHeader.data_type_format, data)
        return cls(message_type, message_id, has_priority, message_size)
        
    def __str__(self):
        return f"MessageHeader(message_type: {self.message_type}, message_id: {self.message_id}, \
        has_priority: {self.has_priority}, message_size: {self.message_size})"

def read_data(conn, bytes_to_read):
    received_bytes = 0
    data = b''
    while received_bytes < bytes_to_read and not should_stop:
        chunk = conn.recv(bytes_to_read - received_bytes)
        if not chunk:
            continue

        received_bytes += len(chunk)
        data += chunk

    return data

def read_message(conn):

    header_data = read_data(conn, struct.calcsize(MessageHeader.data_type_format))
    
    header = MessageHeader.unpack(header_data)
    
    print("Recieved message with header: ")
    print(header)

    if should_stop:
        return None, None
                
    body_data = read_data(conn, header.message_size)
    
    if should_stop:
        return None, None
    
    message_body = list(body_data)
    
    return header, message_body

def send_rez(conn, header, nr_cars_detected):
    
    header.message_size = 1
    
    print("Answear for:")
    print(header)
    print(f"Number cars: {nr_cars_detected}")
    #nr cars detected should be less then 256 so it's ok to convert it to uint8_t
    conn.sendall(header.pack() +  struct.pack('B', nr_cars_detected))

def handle_request(conn, addr):
    try:
        print(f"Connection from {addr}")
        while conn and not should_stop:
            header, message_body = read_message(conn)
            
            if should_stop:
                break

            message_body_bytes = bytes(message_body or {})
            
            image_data = np.frombuffer(message_body_bytes, dtype=np.uint8)
            image = cv2.imdecode(image_data, cv2.IMREAD_COLOR)
            
            nr_cars_detected = detect_cars_inside_image(image)
            send_rez(conn, header, nr_cars_detected)
        
        if conn:
            conn.close()
    except Exception as e:
        print(f"Failed to read/send data from/to the client {addr} err= {e}")
        if conn:
            conn.close()

def main(): 
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    
        s.bind((HOST, PORT))
        s.listen(MAXIMUM_NUMBER_OF_CONNECTIONS)
        s.settimeout(5)
        
        print(f"Listening on {HOST}:{PORT}")
        while not should_stop:
        
            try:
                conn, addr = s.accept()
                
                if conn and not should_stop:
                    client_thread = threading.Thread(target=handle_request, args=(conn, addr))
                    client_thread.start()
                    active_threads.append(client_thread)
            except socket.timeout:
                pass
            
            for thread in active_threads:
                if not thread.is_alive():
                    thread.join()
                    active_threads.remove(thread)

def shutdown():
    global should_stop
    should_stop = True
    for thread in active_threads:
        thread.join()
        
if __name__ == '__main__':
    main()
    shutdown()