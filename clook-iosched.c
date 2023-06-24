/*
 * C-LOOK IO Scheduler
 *
 * For Kernel 4.13.9
 */

#include <linux/blkdev.h>
#include <linux/list.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

/* C-LOOK data structure. */
struct clook_data {
	struct list_head queue;
};

struct future_data {
	struct list_head queue;
};
	
struct future_data *fnd; // Future list: stores not ordered requests

int *current_sector;
int *access_list_size;
int *future_list_size;

static void clook_merged_requests(struct request_queue *q, struct request *rq, struct request *next)
{
	list_del_init(&next->queuelist);
}

static void refresh(struct request_queue *q) {
	struct clook_data *nd = q->elevator->elevator_data;
	
	struct list_head *current_lh;
	
	struct request *current_r;
	struct request *smaller_r = NULL;
	
	// Iterates over the future list N^2 times and keeps moving the request with smallest sector value to access list each time. 
	int i;
	for(i = 0; i < *future_list_size; i++) {
		list_for_each(current_lh, &fnd->queue) {
			current_r = list_entry(current_lh, struct request, queuelist);
			if(smaller_r == NULL)
				smaller_r = current_r;
			if(current_r->__sector < smaller_r->__sector)
				smaller_r = current_r;
		}
		list_move(&smaller_r->queuelist, &nd->queue);
		smaller_r = NULL;
	}
	*future_list_size = 0;
};

/* Esta função despacha o próximo bloco a ser lido. */
static int clook_dispatch(struct request_queue *q, int force)
{
	struct clook_data *nd = q->elevator->elevator_data;
	char direction = 'R';
	struct request *rq;

	/* Aqui deve-se retirar uma requisição da fila e enviá-la para processamento.
	 * Use como exemplo o driver noop-iosched.c. Veja como a requisição é tratada.
	 *
	 * Antes de retornar da função, imprima o sector que foi atendido.
	 */

	rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);

	if (rq) {

		list_del_init(&rq->queuelist);
		*access_list_size--;
		elv_dispatch_sort(q, rq);
		printk(KERN_EMERG "[C-LOOK] dsp %c %lu\n", direction, blk_rq_pos(rq));
		
		// Check access list and refresh if needed
		// Refreshing consists of ordering all elements in the future list by their sector values and moving then to access list
		// Access list is empty? Refresh it.
		// 	It is still empty? 
		// 		current_sector = -1
		// Access list is not empty?
		// 	Update current sector.
		if(list_empty(&nd->queue)) { 
			refresh(q);
			if(list_empty(&nd->queue)) {
				*current_sector = -1;
			} else {
				rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);
				*current_sector = rq->__sector;
			}
		} else {
			rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);
			*current_sector = rq->__sector; 
		}
		return 1;
	}

	return 0;
}

static void append_to_future_list(struct request *rq) {
	list_add_tail(&rq->queuelist, &fnd->queue);
}

static void append_to_access_list(struct request_queue *q, struct request *rq) {
	struct clook_data *nd = q->elevator->elevator_data;
	struct list_head *lh;
	struct list_head *lhn;

	// If the access queue is empty, the request is simply added to it.
	if(list_empty(&nd->queue)) {
		list_add_tail(&rq->queuelist, &nd->queue);
		*current_sector = rq->__sector;
	} else {
		// The request will end up being placed between a request with smaller sector value and a request with greater sector value than its sector value. 	
		
		list_for_each(lh, &nd->queue) {
			// Node lh will always a smaller sector number then node lhn
			lhn = lh->next; 
			if(rn == NULL) {
				list_add_tail(&rq->queuelist, &nd->queue);
				break;
			}
			// TODO: translate to struct request
			if(nd->__sector <= rn->__sector) {
				r->next = nd;
				nd->prev = r;
				nd->next = rn;
				rn->prev = nd;
				break;
			}
		}	
	};
}

// append_request(): Decides which list the request will be appended to
static void append_request(struct request_queue *q, struct request *rq) {
	
	// If the request sector is less then the current sector
	// to be read by the disk, the request is appended to
	// the future list. 
	//
	// Otherwise, the request is appended
	// to the access list.
	if(rq->__sector < *current_sector) {
		append_to_future_list(rq);
		*future_list_size++;
		printk(KERN_EMERG "[C-LOOK] appended to future list %lu\n", blk_rq_pos(rq));
	} else {
		append_to_access_list(q, rq);
		*access_list_size++;
		printk(KERN_EMERG "[C-LOOK] appended to access list %lu\n", blk_rq_pos(rq));
	}
}

/* Esta função adiciona uma requisição ao disco em uma fila */
static void clook_add_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;
	char direction = 'R';

	/* Aqui deve-se adicionar uma requisição na fila do driver.
	 * Use como exemplo o driver noop-iosched.c
	 *
	 * Antes de retornar da função, imprima o sector que foi adicionado na lista.
	 */
	
	// The request will be appended to either the access list or the future list depending on its sector
	append_request(q, rq);

	printk(KERN_EMERG "[C-LOOK] add %c %lu\n", direction, blk_rq_pos(rq));
}

/* Esta função inicializa as estruturas de dados necessárias para o escalonador */
static int clook_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct clook_data *nd;
	struct elevator_queue *eq; // Access list: stores requests to be dispatched
	
	*current_sector = 0;
	*access_list_size = 0;
	*future_list_size = 0;

	/* Implementação da inicialização da fila (queue).
	 *
	 * Use como exemplo a inicialização da fila no driver noop-iosched.c
	 *
	 */

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	// Future list
	fnd = kmalloc_node(sizeof(*fnd), GFP_KERNEL, q->node);
	INIT_LIST_HEAD(&fnd->queue);

	// Access list
	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;
	INIT_LIST_HEAD(&nd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);

	return 0;
}

static void clook_exit_queue(struct elevator_queue *e)
{
	
	// Freeing access list
	struct clook_data *nd = e->elevator_data;

	/* Implementação da finalização da fila (queue).
	 *
	 * Use como exemplo o driver noop-iosched.c
	 *
	 */
	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);

	// Freeing future list
	BUG_ON(!list_empty(&fnd->queue));
	kfree(fnd);

}

/* Estrutura de dados para os drivers de escalonamento de IO */
static struct elevator_type elevator_clook = {
	.ops.sq = {
		.elevator_merge_req_fn		= clook_merged_requests,
		.elevator_dispatch_fn		= clook_dispatch,
		.elevator_add_req_fn		= clook_add_request,
		.elevator_init_fn		= clook_init_queue,
		.elevator_exit_fn		= clook_exit_queue,
	},
	.elevator_name = "c-look",
	.elevator_owner = THIS_MODULE,
};

/* Inicialização do driver. */
static int __init clook_init(void)
{
	return elv_register(&elevator_clook);
}

/* Finalização do driver. */
static void __exit clook_exit(void)
{
	elv_unregister(&elevator_clook);
}

module_init(clook_init);
module_exit(clook_exit);

MODULE_AUTHOR("Sérgio Johann Filho");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("C-LOOK IO scheduler skeleton");
