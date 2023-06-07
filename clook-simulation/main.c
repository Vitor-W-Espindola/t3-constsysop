#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct request {
	int sector;
	struct request *next;
};

void print_req(struct request *req) { printf("Request with sector -> %d\n", req->sector); fflush(stdout); }

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

void append_request_to_access(struct request_list *list, struct request *req, int *current_sector) {
	

	struct request *tmp_req_prev = NULL;
	struct request *tmp_req = list->first;
	struct request *tmp_req_next;
	
	if(tmp_req == NULL) {
		tmp_req_next = NULL;
		list->first = req;
		list->last = req;
		*current_sector = req->sector;
	} else {
		tmp_req_next = list->first->next;
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
			if(req->sector > tmp_req_prev->sector && req->sector <= tmp_req->sector) {
				tmp_req_prev->next = req;
				req->next = tmp_req;
				break;
			}
		}
	}
}

void recv_request(struct request_list *access_list, struct request_list *future_list, struct request *req, int *current_sector) {
	if(req->sector < *current_sector)
		append_request_to_future(future_list, req);
	else
		append_request_to_access(access_list, req, current_sector);
}

void dispatch_request(struct request_list *access_list, int *current_sector) {
	struct request *req_to_disp = access_list->first;
	if(req_to_disp == NULL) 
		return;
	access_list->first = req_to_disp->next;
	if(access_list->first == NULL) 
		access_list->last = NULL;
	else
		*current_sector = access_list->first->sector;
	printf("Dispatching request %d...\n", req_to_disp->sector);
	free(req_to_disp);
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
 * Two queues are built in order to receive requests:
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
	access_list->first = NULL;
	access_list->last = NULL;
	struct request_list *future_list = malloc(sizeof(struct request_list));
	
	srand(time(NULL));

	int i;
	for(i = 0; i < 10; i++) {
		struct request *req = malloc(sizeof(struct request));
		req->sector = rand() % 100;
		printf("Receving a request: sector %d...\n", req->sector);
		recv_request(access_list, future_list, req, &current_sector);
	}

	printf("Reading access request list...\n");
	print_list(access_list);
	printf("Reading future request list...\n");
	print_list(future_list);

	struct request *tmp_req = access_list->first;
	while(1) {
		if(tmp_req == NULL)
			break;
		dispatch_request(access_list, &current_sector);
		tmp_req = access_list->first;	
	}

	return 0;
}
