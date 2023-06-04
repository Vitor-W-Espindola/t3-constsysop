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

void append_request(struct request_list *list, struct request *req) {
	if(list->first == NULL) {
		list->first = req;
		list->last = req;
	} else {
		list->last->next = req;
		list->last = req;
	}
}

void recv_request(struct request_list *curr_req_list, struct request_list *await_req_list, struct request *req) {
	if(curr_req_list->first == NULL) {
		append_request(curr_req_list, req);	
	} else {
		if(req->sector >= curr_req_list->last->sector)
			append_request(curr_req_list, req);
		else
			append_request(await_req_list, req);
	}
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

int main() {
	struct request_list *req_list = malloc(sizeof(struct request_list));
	struct request_list *awaiting_list = malloc(sizeof(struct request_list));
	
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
