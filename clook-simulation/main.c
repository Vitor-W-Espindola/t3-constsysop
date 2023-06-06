#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct request {
	int sector;
	struct request *next;
};

struct request_list {
	int size;
	struct request *first;
	struct request *last;	
};

void append_request_to_future(struct request_list *list, struct request *req) {
	if(list->first == NULL) {
		list->first = req;
		list->last = req;
	} else {
		list->last->next = req;
		list->last = req;
	}
}

void append_request_to_access(struct request_list *list, struct request *req) {
	
	struct request tmp_req_prev = NULL;
	struct request tmp_req = list->first;
	struct request tmp_req_next = list->first->next;

	if(tmp_req == NULL) {
		list->first = req;
		list->last = req;
	} else {
		while(1) {
			tmp_req_prev = tmp_req;
			tmp_req = tmp_req_next;
			if(tmp_req != NULL) 
				tmp_req_next = tmp_req_next->next;
			
			// If it ended up at the tail
			if(tmp_req == NULL) {
				tmp_req_prev->next = req;
				list->last = req;
				break;
			}
			
			// If it found some proper position for it
			if(req->sector > tmp_req_prev->sector && req->sector <= tmp_req) {
				tmp_req_prev->next = req;
				req->next = tmp_req;
				break;
			}
		}
	}
}

void recv_request(struct request_list *access_list, struct request_list *future_list, struct request *req, int current_sector, int *current_sector) {
	if(req->sector < current_sector)
		append_request_to_future(future_list, req);
	else
		append_request_to_acces(access_list, req);	
}

void rearrange_awaiting_list(struct request_list await_req_list) {

}

void print_list(struct request_list *list) {
	struct request *tmp_req = list->first;
	while(1) {
		if(tmp_req == NULL) break;
		printf("Request sector: %d\n", tmp_req->sector);
		tmp_req = tmp_req->next;
	}
}


/*
 * Two queues are built in order to receive differente requests:
 *
 * 	i) access -> all requests with target sector greather or equal than current sector
 * 	ii) future -> all requests with target sector less than current sector
 *
 * 	Whenever a new request reaches the driver, if it is able to join the access queue,
 *  then it will be placed on a proper position on the queue.
 *  	Whenever the access queue is empty, the future queue is ordered and all its requests are
 *  copied to the access queue, leaving the future queue blank again for future requests.
 *
 * */
int main() {
	int current_sector = 0;
	struct request_list *access_list = malloc(sizeof(struct request_list));
	struct request_list *future_list = malloc(sizeof(struct request_list));
	
	srand(time(NULL));

	int i;
	for(i = 0; i < 10; i++) {
		struct request *req = malloc(sizeof(struct request));
		req->sector = rand() % 100;
		printf("Receving a request: sector %d...\n", req->sector);
		recv_request(req_list, awaiting_list, req);
	}

	printf("Reading current request list...\n");
	print_list(req_list);
	printf("Reading awaiting request list...\n");
	print_list(awaiting_list);

	return 0;
}
