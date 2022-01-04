from socket import *
import sys

#define max_num_of_bytes_per_recv
MESSAGE_SIZE = 100



def read_get_request( _socket, counter):

    is_bad_req = True    #default assumption
    req = ''

    while(condition == -1):
        req += _socket.recv(MESSAGE_SIZE)
        condition = req.find('\r\n\r\n') 

    index = req.find('/counter') 
    if( index != -1):
        counter += 1
        is_bad_req = False 

    return counter, is_bad_req
   


def send_answer(_socket, counter, is_bad_req):
    if is_bad_req == True:
        ans = "HTTP/1.1 404 Not Found\r\nContent-type: text/html\r\nContent-length: 113\r\n<html><head><title>Not Found</title></head><body>\r\nSorry, the object you requested was not found.\r\n</body></html>"
    else:
        #good request
        ans = 'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: ' +str(len(counter))+'\r\n\r\n'+str(counter)+'\r\n\r\n' 
    
    _socket.send(ans)




req_counter = 0
port_num = int(sys.argv[1])
s = socket()
s.connect(('localhost', port_num))

while(True):
    req_counter, is_bad_request = read_get_request(s, req_counter)
    send_answer(s, req_counter, is_bad_request)


